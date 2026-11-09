#!/usr/bin/env python3
"""Write repo-root TESTING.md: high-usage demo cards, coded effects, YGOPRODeck text, test scenarios.

  python3 tools/gen_testing_md.py              # writes TESTING.md
  python3 tools/gen_testing_md.py --list-only  # print counts only

Cards: every name in `demo_cards.txt` except those listed under `******` in `new_cards.txt` (even if the same
name appears above `------`), >= 10% usage in some format
(`usage/<format>.json` root `total` + `extra` only; limited x3, semi-limited x1.5; capped at 100%),
each listed once under the earliest qualifying format
(tools/banlist.py BANLISTS order). Effect text is traced from src/added_effects.c, engine keys, and
LEFTOVER_EFFECTS.md; official text from YGOPRODeck (https://ygoprodeck.com/api-guide/).
"""
import argparse
import json
import re
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

sys.path.insert(0, 'tools')
import banlist  # noqa: E402
from cardprices import share  # noqa: E402

LIMIT_FACTOR = {1: 3.0, 2: 1.5}  # banlist limited / semi-limited multipliers on inclusion rate
from gencards import ident  # noqa: E402

THRESH = 0.10
YGO_URL = 'https://db.ygoprodeck.com/api/v7/cardinfo.php'

root = Path(__file__).resolve().parents[1]
AE_PATH = root / 'src/added_effects.c'
LEFTOVER_MD = root / 'LEFTOVER_EFFECTS.md'
CACHE = root / 'assets/new_cards/cards.json'


STAR_SEP = '******'


def names_below_stars() -> set[str]:
    """Names in new_cards.txt on or after the `******` line (full-build / duplicate list)."""
    names: set[str] = set()
    past = False
    for line in (root / 'new_cards.txt').read_text(encoding='utf-8').splitlines():
        line = line.strip()
        if line == STAR_SEP:
            past = True
            continue
        if not past or not line or line.startswith('#'):
            continue
        names.add(line.split('|', 1)[0].strip())
    return names


def demo_card_names() -> list[str]:
    """Alphabetical order from demo_cards.txt (header comments skipped)."""
    out: list[str] = []
    for line in (root / 'demo_cards.txt').read_text(encoding='utf-8').splitlines():
        line = line.strip()
        if line and not line.startswith('#'):
            out.append(line.split('|', 1)[0].strip())
    return out


def testing_pool_names() -> list[str]:
    """demo_cards.txt minus any name also listed under `******` in new_cards.txt."""
    exclude = names_below_stars()
    return [n for n in demo_card_names() if n not in exclude]


def _eds_name(raw: str) -> str:
    name = raw
    for alias, eds in banlist.new_card_aliases().items():
        if name == alias:
            return eds
    for alias_v, eds in banlist.ALIASES.items():
        if name == alias_v:
            return eds
    return name


def name_usage_pct(stem: str, lookup, missing) -> dict[str, float]:
    """Deck inclusion rate per card name from the format's usage JSON only (root `total` + `extra`).

    share = (decks running the card) / decks_processed. Main and extra lists add; result is capped at 1.0 before
    and after banlist limited (x3) / semi-limited (x1.5) multipliers.
    """
    usage = json.load(open(root / f'usage/{stem}.json', encoding='utf-8'))
    decks = usage['decks_processed']
    limits, _ = banlist.read_limits(root / f'banlists/{stem}_ban.json', lookup, missing)
    inclusion: dict[str, float] = {}
    for key in ('total', 'extra'):
        block = usage.get(key)
        if not isinstance(block, list):
            continue
        for entry in block:
            name = _eds_name(entry['name'])
            inclusion[name] = inclusion.get(name, 0.0) + share(entry, decks)
    by_name: dict[str, float] = {}
    for name, base in inclusion.items():
        base = min(1.0, base)
        numbers = lookup(name)
        if numbers is None:
            continue
        lim = 1.0
        for number in numbers:
            lim = max(lim, LIMIT_FACTOR.get(limits.get(number), 1.0))
        by_name[name] = min(1.0, base * lim)
    return by_name


def assign_cards(names: list[str]) -> dict[str, tuple[str, str, float]]:
    lookup = banlist.card_lookup()
    missing = []
    assigned: dict[str, tuple[str, str, float]] = {}
    for fmt_name, stem, _ in banlist.BANLISTS:
        pct = name_usage_pct(stem, lookup, missing)
        for name in names:
            if name in assigned:
                continue
            rate = pct.get(name, 0.0)
            if rate >= THRESH:
                assigned[name] = (fmt_name, stem, rate)
    return assigned


def parse_ae_blocks(text: str) -> dict[str, str]:
    """Normalized title -> first comment paragraph from added_effects.c."""
    out: dict[str, str] = {}
    for m in re.finditer(r'/\* ---- ([^\n]+)\n((?: \*[^\n]*\n)+)', text):
        title = m.group(1).strip().rstrip('-').strip()
        lines = []
        for ln in m.group(2).splitlines():
            ln = re.sub(r'^ \* ?', '', ln).strip()
            if ln.endswith('*/'):
                ln = ln[:-2].strip()
            if ln.startswith('---') or ln.startswith('#if') or ln.startswith('The prepare'):
                break
            if ln.startswith('*/'):
                break
            if ln:
                lines.append(ln)
        para = ' '.join(lines[:15]).strip()
        para = para.replace('*/', '').strip()
        if para:
            out[banlist.norm(title)] = para
    return out


def parse_engine_comments() -> dict[str, str]:
    text = (root / 'tools/newcards.py').read_text(encoding='utf-8')
    out: dict[str, str] = {}
    start = text.index('ENGINE_NUMBERS = {')
    end = text.index('\n}\n', start) if '\n}\n' in text[start:] else text.index('}', start)
    block = text[start:end]
    lines = block.splitlines()
    i = 0
    while i < len(lines):
        m = re.match(r"\s*'([^']+)':\s*(\d+),\s*(?:#\s*(.*))?", lines[i])
        if not m:
            i += 1
            continue
        name, num, c0 = m.group(1), m.group(2), m.group(3) or ''
        parts = [c0] if c0 else []
        j = i + 1
        while j < len(lines) and re.match(r'\s*#\s+', lines[j]):
            parts.append(re.sub(r'^\s*#\s*', '', lines[j]).strip())
            j += 1
        comment = ' '.join(parts).strip()
        out[name] = f"Engine effect key {num}. {comment}" if comment else f"Engine effect key {num}."
        i = j
    return out


def leftover_numbers() -> dict[str, int]:
    out = {}
    for line in (root / 'config/leftover_cards.txt').read_text(encoding='utf-8').splitlines():
        line = line.strip()
        if not line or line.startswith('#') or '|' not in line:
            continue
        num_s, name = line.split('|', 2)[:2]
        out[name.strip()] = int(num_s.strip())
    return out


def leftover_table(number: int) -> tuple[str, str]:
    if not LEFTOVER_MD.is_file():
        return '', ''
    for line in LEFTOVER_MD.read_text(encoding='utf-8').splitlines():
        if not line.startswith(f'| {number} |'):
            continue
        parts = [p.strip() for p in line.split('|')]
        if len(parts) >= 6:
            return parts[4], parts[5]
    return '', ''


def coded_summary(name: str, ae_text: str, ae_blocks: dict, engine: dict, leftovers: dict[str, int]) -> tuple[str, str]:
    """Return (implementation pointer, behavior paragraph)."""
    n = banlist.norm(name)
    # mod-specific block comment
    for key, para in ae_blocks.items():
        if key == n or n in key or key in n:
            tag = f'HAS_{ident(name)}'
            loc = f'`src/added_effects.c` (`{tag}`, `gAddedCardEffects`)'
            return loc, para
    tag = f'HAS_{ident(name)}'
    if tag in ae_text:
        loc = f'`src/added_effects.c` (`{tag}`, `gAddedCardEffects`)'
        return loc, '(See handlers in `src/added_effects.c`; no block comment was matched.)'
    if name in engine:
        return f'Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)', engine[name]
    if name in leftovers:
        num = leftovers[name]
        eff, status = leftover_table(num)
        loc = f'Vanilla engine (effect key **{num}**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))'
        body = eff
        if status:
            body += f' Status in ROM: {status}.'
        return loc, body
    return 'Unknown / stats only', 'No `gAddedCardEffects` row or engine key found for this demo card; verify in Deck Edit and trunk.'


def fetch_ygo(name: str, cache: dict) -> str:
    cached = (cache.get(name) or {}).get('desc', '')
    q = urllib.parse.quote(name)
    try:
        req = urllib.request.urlopen(f'{YGO_URL}?name={q}', timeout=30)
        data = json.loads(req.read().decode())
        if data.get('data'):
            desc = data['data'][0].get('desc', '')
            return desc.strip()
    except (urllib.error.URLError, json.JSONDecodeError, KeyError, IndexError):
        pass
    return cached.strip()


def differs(coded: str, ygo: str) -> str:
    if not coded or not ygo:
        return ''
    if 'rom bug' in coded.lower() or 'two fixes in effect_resolve' in coded.lower():
        return 'Documented implementation quirk in this build (see **Coded behavior**); do not expect full modern TCG timing.'
    # Pull quoted TCG line from mod comments when present
    q = re.search(r'"([^"]{20,})"', coded)
    coded_pub = q.group(1) if q else coded
    c = banlist.norm(coded_pub)
    y = banlist.norm(ygo)
    if c == y or (len(c) > 40 and (c in y or y in c)):
        return ''
    # Equip / passive engine notes are not full card text
    if coded_pub.startswith('Engine effect key') or 'CalcBattle' in coded or 'CalcZoneCardStats' in coded:
        return ''
    if banlist.norm(coded_pub)[:120] == banlist.norm(ygo)[:120]:
        return ''
    return 'Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.'


def make_scenarios(name: str, fmt: str, coded: str, ygo: str) -> list[str]:
    blob = f'{coded} {ygo}'.lower()
    out: list[str] = []
    if 'tribute' in blob or 'soul exchange' in name.lower():
        out.append(
            f'**{fmt}** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate '
            f'**{name}**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.')
    if 'opponent' in blob and 'graveyard' in blob:
        out.append(
            f'Fill the opponent\'s Graveyard, attack or trigger **{name}**, and confirm whether/how many '
            'opponent Graveyard cards are banished or removed.')
    if 'destroy' in blob and ('all' in blob or 'every' in blob or 'torrential' in name.lower()):
        out.append(
            f'Summon monsters on both sides, activate **{name}** at legal timing, and confirm board-wide destruction '
            'and whether FLIP or floating effects still queue.')
    elif 'destroy' in blob or 'destroys' in blob:
        out.append(
            f'Give **{name}** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the '
            'target leaves the field and chained effects resolve in the right order.')
    if 'draw' in blob and 'discard' in blob:
        out.append(
            f'Both players with uneven hands: activate **{name}** and verify discard-then-draw counts for each side.')
    elif 'draw' in blob:
        out.append(
            f'Activate or trigger **{name}** when its draw effect should run (draw phase, chain, etc.) and count cards '
            'in hand before and after.')
    if 'attack' in blob and ('negate' in blob or 'cylinder' in name.lower()):
        out.append(
            'Declare an attack against you with a high-ATK monster, chain **Magic Cylinder** or equivalent, and '
            'confirm the attack stops and LP changes match the attacker\'s ATK.')
    if 'token' in blob or 'scapegoat' in name.lower():
        out.append(
            'With four open zones and no prior summon, activate **Scapegoat** (or similar), confirm four tokens and '
            'that further summons are blocked for the turn.')
    if 'flip' in blob:
        out.append(
            f'Set **{name}** face-down, have the opponent attack or flip it, and test the FLIP effect once.')
    if 'equip' in blob or 'united we stand' in name.lower() or 'mage power' in name.lower():
        out.append(
            f'Equip **{name}** to a legal monster, change the number of face-up monsters or Spells/Traps on your field, '
            'and watch ATK/DEF updates.')
    if ('monarch' in name.lower() or 'when this card is tribute summoned' in blob) and 'soul exchange' not in name.lower():
        out.append(
            f'Tribute Summon **{name}** (or Special Summon if coded), trigger its on-summon effect, and confirm '
            'hand/triggered effects.')
    if 'chaos' in name.lower() and 'emperor' in name.lower():
        out.append(
            'Reach 7+ LIGHT/DARK in hand or field, Special Summon **Chaos Emperor Dragon**, pay costs, and confirm '
            'mass hand/field return and damage.')
    if 'fiber jar' in name.lower():
        out.append(
            'Activate **Fiber Jar**\'s effect if coded, or verify deck-shuffle / reset behavior matches the engine key.')
    if 'virus' in name.lower():
        out.append(
            'Declare an allowed type with **Tribe-Infecting Virus**, pay the cost, and confirm opponent monsters of '
            'that type are destroyed while yours remain.')
    if 'yata' in name.lower():
        out.append(
            'Attack directly with **Yata-Garasu**, confirm hand size locked at zero, and attempt to draw during '
            'upkeep.')
    if 'cold wave' in name.lower():
        out.append(
            'Open Main Phase 1 with no prior plays: activate **Cold Wave** and confirm both players cannot activate '
            'Spells/Traps for the coded duration.')
    if not out:
        out.append(
            f'In a **{fmt}**-rules duel, add **{name}** to the deck, draw it in a realistic line, and run the effect '
            'once as human and once watching CPU handling.')
        out.append(
            'Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with '
            'an activation.')
    return out[:3]


def write_testing_md(assigned: dict[str, tuple[str, str, float]]) -> None:
    ae_text = AE_PATH.read_text(encoding='utf-8')
    ae_blocks = parse_ae_blocks(ae_text)
    engine = parse_engine_comments()
    leftovers = leftover_numbers()
    cache = json.loads(CACHE.read_text(encoding='utf-8')) if CACHE.is_file() else {}

    by_fmt: dict[str, list[tuple[str, float]]] = {}
    stems: dict[str, str] = {}
    for name, (fmt, stem, rate) in assigned.items():
        by_fmt.setdefault(fmt, []).append((name, rate))
        stems[name] = stem
    for fmt in by_fmt:
        by_fmt[fmt].sort(key=lambda x: (-x[1], x[0].casefold()))

    lines = [
        '# Effect testing scenarios (high-usage demo cards)',
        '',
        'Manual play-test checklist for **demo-build** cards in [`demo_cards.txt`](demo_cards.txt), **excluding** any '
        'name that also appears under the `******` line in [`new_cards.txt`](new_cards.txt) (those duplicates are '
        'tracked for the full list, not for this pass). Cards must appear in at least **10%** of decks '
        'in some Campaign format\'s usage statistics. Inclusion is read only from each file\'s top-level `total` and '
        '`extra` arrays in [`usage/`](usage/) (fraction of `decks_processed` that run the card, main plus extra), '
        'then multiplied by **3** for limited cards and **1.5** for semi-limited in that format\'s banlist (capped at '
        '**100%**). Each card appears **once**, under the **earliest** format (in '
        '[`tools/banlist.py`](tools/banlist.py) `BANLISTS` order from **Yugi-Kaiba** through **Reaper**) where it '
        'meets that threshold.',
        '',
        'For each card: **Implementation** and **Coded behavior** come from this repo (`src/added_effects.c`, engine '
        'effect keys, [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md), and related files). **YGOPRODeck text** is loaded '
        f'from [YGOPRODeck API v7]({YGO_URL}) (`name` query). If the build differs from TCG/OCG, scenarios still follow '
        'the code—differences are noted, not treated as bugs to fix here.',
        '',
        f'**{len(assigned)}** cards qualify (of {len(testing_pool_names())} in '
        f'`demo_cards.txt` minus `new_cards.txt` below `******`; {len(names_below_stars())} names excluded). '
        'Regenerate with `python3 tools/gen_testing_md.py`.',
        '',
    ]

    for fmt_name, _stem, _ in banlist.BANLISTS:
        items = by_fmt.get(fmt_name)
        if not items:
            continue
        lines.append(f'## {fmt_name}')
        lines.append('')
        for name, rate in items:
            impl, coded = coded_summary(name, ae_text, ae_blocks, engine, leftovers)
            ygo = fetch_ygo(name, cache)
            note = differs(coded, ygo)
            lines.append(f'### {name}')
            lines.append('')
            lines.append(
                f'- **Usage:** {rate * 100:.1f}% weighted inclusion in `usage/{stems[name]}.json` '
                f'(earliest qualifying format).'
            )
            lines.append(f'- **Implementation:** {impl}')
            lines.append(f'- **Coded behavior:** {coded}')
            if ygo:
                lines.append(f'- **YGOPRODeck (TCG) text:** {ygo}')
            else:
                lines.append('- **YGOPRODeck (TCG) text:** *(API lookup failed; see [ygoprodeck.com](https://ygoprodeck.com/).)*')
            if note:
                lines.append(f'- **TCG vs this ROM:** {note}')
            lines.append('- **Scenarios:**')
            for i, sc in enumerate(make_scenarios(name, fmt_name, coded, ygo), 1):
                lines.append(f'  {i}. {sc}')
            lines.append('')
            time.sleep(0.12)

    out = root / 'TESTING.md'
    text = '\n'.join(lines).rstrip() + '\n'
    if out.exists() and out.read_text(encoding='utf-8') == text:
        print('TESTING.md unchanged', file=sys.stderr)
    else:
        out.write_text(text, encoding='utf-8', newline='\n')
        print(f'Wrote {out} ({len(assigned)} cards)', file=sys.stderr)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--list-only', action='store_true')
    args = ap.parse_args()
    names = testing_pool_names()
    assigned = assign_cards(names)
    if args.list_only:
        print(f'{len(assigned)} cards')
        return
    write_testing_md(assigned)


if __name__ == '__main__':
    main()
