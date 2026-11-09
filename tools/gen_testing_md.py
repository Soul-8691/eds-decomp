#!/usr/bin/env python3
"""Write repo-root TESTING.md: high-usage demo cards, coded effects, YGOPRODeck text, test scenarios.

  python3 tools/gen_testing_md.py              # writes TESTING.md (YGOPRODeck API)
  python3 tools/gen_testing_md.py --no-api     # use cards.json cache only (faster)
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
from ai_coverage import ai_scenarios, ai_status  # noqa: E402

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


def fetch_ygo(name: str, cache: dict, *, use_api: bool = True) -> str:
    cached = (cache.get(name) or {}).get('desc', '')
    if not use_api:
        return cached.strip()
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


def _first_in_pool(pool: set[str], candidates: list[str]) -> str | None:
    for n in candidates:
        if n in pool:
            return n
    return None


def _read_pool_name_lines(pool_stems: list[str]) -> set[str]:
    names: set[str] = set()
    for ps in pool_stems:
        path = root / f'card_pools/{ps}.txt'
        if not path.is_file():
            continue
        for line in path.read_text(encoding='utf-8').splitlines():
            n = line.strip()
            if n:
                names.add(n)
    return names


def _legal_pool_names(
    pool_stems: list[str], stem: str, lookup, missing: list[str]
) -> tuple[set[str], dict[int, int]]:
    """Pool file names that exist in EDS and are not Forbidden (limit 0) in this format's banlist."""
    limits, _ = banlist.read_limits(root / f'banlists/{stem}_ban.json', lookup, missing)
    legal: set[str] = set()
    for name in _read_pool_name_lines(pool_stems):
        numbers = lookup(name)
        if numbers is None:
            continue
        if any(limits.get(n) == 0 for n in numbers):
            continue
        legal.add(name)
    return legal, limits


# Pool files list many vanilla cards; YGOPRO cache covers added cards only.
_SPELL_TRAP_NAMES = {
    'call of the haunted', 'card destruction', 'ceasefire', 'change of heart', 'confiscation',
    'dark hole', 'graceful charity', 'harpie\'s feather duster', 'heavy storm', 'jar of greed',
    'magic jammer', 'mirror force', 'monster reborn', 'mystical space typhoon', 'pot of greed',
    'raigeki', 'solemn judgment', 'trap hole', 'fissure', 'smashing ground', 'book of moon',
    'creature swap', 'delinquent duo', 'snatch steal', 'mystical refill', 'cyber jar',
    'tribute to the doomed', 'offerings to the doomed', 'premature burial', 'scorched earth',
    'hammer shot', 'brain control', 'the forceful sentry', 'painful choice', 'mage power',
    'united we stand', 'mystical elf', 'shrink', 'swords of revealing light',
}


def _card_type(name: str, cache: dict) -> str:
    return (cache.get(name) or {}).get('type', '')


def _is_monster(name: str, cache: dict) -> bool:
    t = _card_type(name, cache)
    if t:
        return 'Monster' in t
    return name.casefold() not in _SPELL_TRAP_NAMES


def _is_spell(name: str, cache: dict) -> bool:
    if _is_trap(name, cache):
        return False
    t = _card_type(name, cache)
    if t:
        return t == 'Spell Card'
    nl = name.casefold()
    return nl in _SPELL_TRAP_NAMES


_TRAP_NAMES = {
    'call of the haunted', 'mirror force', 'magic jammer', 'solemn judgment', 'trap hole',
    'bottomless trap hole', 'ceasefire', 'sakuretsu armor', 'dust tornado', 'torrential tribute',
}


def _is_trap(name: str, cache: dict) -> bool:
    t = _card_type(name, cache)
    if t:
        return t == 'Trap Card'
    nl = name.casefold()
    if nl in _TRAP_NAMES:
        return True
    return any(x in nl for x in ('trap hole', 'mirror force', 'magic jammer', 'solemn judgment'))


# Vanilla quick-play Spells (added cards use cache `race`: Quick-Play).
_QUICK_PLAY_NAMES = {
    'book of moon', 'scapegoat', 'enemy controller', 'limiter removal', 'offerings to the doomed',
    'poison of the old man', 'emergency provisions', 'de-fusion', 'mystik wok', 'reload',
    'malice dispersion', 'monster recovery', 'my body as a shield', 'cold wave', 'serial spell',
}


def _is_quick_play_spell(name: str, cache: dict) -> bool:
    if _is_trap(name, cache):
        return False
    t = _card_type(name, cache)
    if t == 'Spell Card':
        return (cache.get(name) or {}).get('race') == 'Quick-Play'
    return name.casefold() in _QUICK_PLAY_NAMES


def _can_chain_card(name: str, cache: dict) -> bool:
    """Trap Cards and Quick-Play Spells only (not Normal Spells)."""
    return _is_trap(name, cache) or _is_quick_play_spell(name, cache)


def _pick_chainable(pool: set[str], cache: dict, candidates: list[str]) -> str | None:
    return _first_in_pool(pool, [n for n in candidates if _can_chain_card(n, cache)])


def _pick_normal_spell(pool: set[str], cache: dict, *, skip: str | None = None) -> str | None:
    for n in [
        'Change of Heart', 'Dark Hole', 'Monster Reborn', 'Pot of Greed', 'Graceful Charity', 'Card Destruction',
        'Mystical Space Typhoon', 'Fissure',
    ] + sorted(pool):
        if n == skip or n not in pool:
            continue
        if _is_spell(n, cache) and not _is_quick_play_spell(n, cache):
            return n
    return None


def _p_normal_spell(ctx: dict, cache: dict, *, skip: str | None = None) -> str:
    n = _pick_normal_spell(ctx['pool'], cache, skip=skip)
    if n:
        return f'**{n}**'
    return 'a Normal Spell from the pool'


def _p_chainable(ctx: dict, cache: dict, *, alt_candidates: list[str] | None = None) -> str:
    pool = ctx['pool']
    prefs = alt_candidates or [
        'Magic Jammer', 'Trap Hole', 'Call of the Haunted', 'Bottomless Trap Hole', 'Mirror Force',
        'Solemn Judgment', 'Book of Moon', 'Enemy Controller', 'Scapegoat',
    ]
    n = _pick_chainable(pool, cache, prefs)
    if n:
        return f'**{n}**'
    for name in sorted(pool):
        if _can_chain_card(name, cache):
            return f'**{name}**'
    return 'a face-down Trap or Quick-Play Spell from the pool'


def _has_monster_activation(coded: str) -> bool:
    cl = coded.lower().replace(' ', '')
    return (
        'activatedeffect' in cl
        or 'canactivatemonstereffect' in cl
        or "card menu's activate" in cl
        or 'card menu' in cl and 'activate' in cl
    )


def _is_mandatory_passive(name: str, coded: str) -> bool:
    """Mandatory battle/turn follow-ups and other effects that are not activations opponents can chain to."""
    cl = coded.lower()
    if _has_monster_activation(coded):
        return False
    if any(
        m in cl
        for m in (
            'end of the battle phase',
            'at the end of the battle phase',
            'cannot change position',
            'after it attacks',
            'after this card attacks',
            'lives in the battle code',
            'battlestage_',
            'damagecalc',
            'damage step',
        )
    ):
        return True
    if 'engine effect key' in cl and 'vanilla engine' in cl:
        if any(m in cl for m in ('after it attacks', 'battle phase', 'turn-start')):
            return True
    return False


def _has_activation_window(name: str, coded: str, cache: dict) -> bool:
    """Spell/Trap activation or monster effect explicitly invoked from the card menu (not a mandatory battle step)."""
    if _is_mandatory_passive(name, coded):
        return False
    t = _card_type(name, cache)
    if t in ('Spell Card', 'Trap Card'):
        return True
    if 'Monster' in t and _has_monster_activation(coded):
        return True
    if coded.lower().startswith('normal spell:') or coded.lower().startswith('normal trap'):
        return True
    return False


def format_staples(stem: str, pool: set[str], target: str, lookup, missing: list, k: int = 6) -> list[str]:
    usage = name_usage_pct(stem, lookup, missing)
    ranked = [(n, r) for n, r in usage.items() if n in pool and n.casefold() != target.casefold()]
    ranked.sort(key=lambda x: (-x[1], x[0].casefold()))
    return [n for n, _ in ranked[:k]]


def format_test_context(
    stem: str, pool_stems: list[str], target: str, lookup, missing: list, cache: dict
) -> dict:
    """Legal pool names and typical opponent pieces from card_pools + usage staples."""
    pool_files: list[str] = []
    for ps in pool_stems:
        if (root / f'card_pools/{ps}.txt').is_file():
            pool_files.append(f'card_pools/{ps}.txt')
    pool, _limits = _legal_pool_names(pool_stems, stem, lookup, missing)
    staples = format_staples(stem, pool, target, lookup, missing)
    mon_staples = [s for s in staples if _is_monster(s, cache)]
    spell_staples = [s for s in staples if _is_spell(s, cache)]
    trap_staples = [s for s in staples if _is_trap(s, cache)]
    ctx = {
        'pool': pool,
        'pool_files': pool_files,
        'staples': staples,
        'opp_fatk': _first_in_pool(pool, [
            'Summoned Skull', 'Gemini Elf', 'Berserk Gorilla', 'Battle Ox', 'La Jinn the Mystical Genie of the Lamp',
            'Vorse Raider', 'Battle Ox', 'Alexandrite Dragon',
        ] + mon_staples),
        'flip_set': _first_in_pool(pool, ['Man-Eater Bug', 'Morphing Jar', 'Penguin Soldier', 'Night Assailant']),
        'manipulation': _first_in_pool(pool, mon_staples + [
            'Mystic Tomato', 'Sangan', 'Witch of the Black Forest', 'Giant Rat',
        ]),
        'tribute_body': _first_in_pool(pool, [
            'Summoned Skull', 'Vorse Raider', 'Launcher Spider', 'Judge Man', 'Alexandrite Dragon',
            'Beast of Talwar', 'Curse of Dragon',
        ] + mon_staples),
        'wall': _first_in_pool(pool, mon_staples + [
            'Stone Statue of the Aztecs', 'Spirit Reaper', 'Battleguard King',
        ]),
        'removal': _first_in_pool(pool, spell_staples + [
            'Mystical Space Typhoon', 'Heavy Storm', 'Raigeki', 'Harpie\'s Feather Duster',
        ]),
        'trap_response': _first_in_pool(pool, trap_staples + [
            'Trap Hole', 'Mirror Force', 'Magic Jammer', 'Solemn Judgment',
        ]),
        'draw_spell': _first_in_pool(pool, spell_staples + [
            'Pot of Greed', 'Graceful Charity', 'Jar of Greed', 'Card Destruction',
        ]),
        'machine': _first_in_pool(pool, ['Cyber Dragon', 'Machina Fortress', 'Gearfried the Iron Knight', 'Bazoo the Soul-Eater']),
        'warrior': _first_in_pool(pool, ['Marauding Captain', 'Reinforcement of the Army', 'Blade Knight']),
    }
    return ctx


def _p(ctx: dict, key: str) -> str:
    """Pool pick or a short fallback phrase."""
    v = ctx.get(key)
    if v:
        return f'**{v}**'
    fallbacks = {
        'opp_fatk': 'a high-ATK monster from the format pool',
        'flip_set': 'a FLIP monster from the pool',
        'manipulation': 'a search/swap monster from the pool',
        'tribute_body': 'a Level 5+ monster from the pool',
        'wall': 'a stall monster from the pool',
        'removal': 'a format-legal removal Spell from the pool',
        'trap_response': 'a Trap from the pool',
        'draw_spell': 'a draw Spell from the pool',
        'machine': 'a Machine monster from the pool',
        'warrior': 'a Warrior monster from the pool',
    }
    return fallbacks.get(key, 'a staple from the format pool')


def make_scenarios(
    name: str,
    fmt: str,
    stem: str,
    ctx: dict,
    coded: str,
    ygo: str,
    cache: dict,
) -> tuple[str, list[str]]:
    blob = f'{coded} {ygo}'.lower()
    ctype = (cache.get(name) or {}).get('type', '')
    pool_files = ', '.join(f'`{p}`' for p in ctx['pool_files']) or '`card_pools/`'
    staple_join = ', '.join(f'**{s}**' for s in ctx['staples'][:5]) or '*(build from pool)*'
    context = (
        f'**{fmt}** legal pool: {pool_files}. Build the opponent from these pool staples '
        f'({staple_join}, all from the same files).'
    )
    out: list[str] = []
    opp = _p(ctx, 'opp_fatk')
    flip = _p(ctx, 'flip_set')
    trib = _p(ctx, 'tribute_body')
    manip = _p(ctx, 'manipulation')
    wall = _p(ctx, 'wall')
    removal = _p(ctx, 'removal')
    trap = _p(ctx, 'trap_response')
    draw_sp = _p(ctx, 'draw_spell')
    chain1_raw = _pick_chainable(
        ctx['pool'],
        cache,
        ['Magic Jammer', 'Trap Hole', 'Call of the Haunted', 'Bottomless Trap Hole', 'Mirror Force', 'Book of Moon'],
    )
    chain1 = f'**{chain1_raw}**' if chain1_raw else _p_chainable(ctx, cache)
    chain2_raw = _pick_chainable(
        ctx['pool'],
        cache,
        ['Book of Moon', 'Enemy Controller', 'Scapegoat', 'Solemn Judgment', 'Magic Jammer', 'Call of the Haunted'],
    )
    if chain2_raw and chain2_raw == chain1_raw:
        chain2_raw = _pick_chainable(
            ctx['pool'],
            cache,
            [n for n in sorted(ctx['pool']) if _can_chain_card(n, cache) and n != chain1_raw],
        )
    chain2 = f'**{chain2_raw}**' if chain2_raw else 'another face-down Trap or Quick-Play Spell from the pool'

    if 'soul exchange' in name.lower():
        out.append(
            f'You: **{name}** plus a Level 5+ Tribute from the pool (e.g. {trib}). Opponent fields {opp} in Attack Position. '
            f'Main Phase 1: activate **{name}** on {opp}, Tribute Summon the high-Level card, confirm you skip the Battle '
            f'Phase, and confirm the tribute removed {opp} even though they controlled it.'
        )
    elif 'card destruction' in name.lower():
        night = _first_in_pool(ctx['pool'], ['Night Assailant'])
        flip_note = (
            f', and any FLIP-on-discard effects (e.g. **{night}**) fire on the opponent\'s discard'
            if night
            else ''
        )
        out.append(
            f'Your hand: **{name}**, {draw_sp}, and another pool card; opponent has four cards including {opp}. '
            f'Activate **{name}**: both hands discard, both draw the same count{flip_note}.'
        )
    elif 'torrential' in name.lower():
        out.append(
            f'You control {manip}; opponent Normal Summons {opp} and sets {flip}. Activate set **{name}** when that '
            f'summon resolves (Trap in response to Summon): every monster on the field is destroyed and any FLIP '
            f'effects from {flip} still resolve.'
        )
    elif 'scapegoat' in name.lower():
        out.append(
            f'Opponent attacks your {manip} with {opp}. With four open zones and no summon yet, activate **{name}**, '
            f'block the attack with tokens, then try to summon {trib} the same turn (should fail).'
        )
    elif 'magic cylinder' in name.lower():
        out.append(
            f'Opponent attacks your face-down {flip} with {opp} (note its ATK). Activate **{name}** in the attack step: '
            f'negate the attack and inflict that ATK as damage; {flip} should not be destroyed by battle.'
        )
    elif 'bottomless' in name.lower():
        out.append(
            f'Opponent Special Summons or Normal Summons {opp} with 1500 or less ATK. Flip **{name}** in response: '
            f'{opp} is destroyed; opponent cannot chain a Normal Spell (e.g. {draw_sp})—only a Trap or Quick-Play Spell '
            f'such as {chain1}.'
        )
    elif 'tribe-infecting virus' in name.lower():
        mach = _p(ctx, 'machine')
        out.append(
            f'Your field: **{name}** plus discard fodder; opponent fields {mach}, {opp}, and {manip}. Declare Machine, '
            f'discard a Machine from the pool, and confirm only the opponent\'s Machines are destroyed.')
    elif 'chaos emperor dragon' in name.lower():
        out.append(
            f'Your hand/field: enough LIGHT and DARK monsters from the pool ({opp}, {wall}, etc.). Special Summon '
            f'**{name}**, pay 1000 LP, return all field cards and hands to the Deck, inflict 300 per returned card, '
            f'and confirm {draw_sp} cannot be played mid-resolution.')
    elif 'yata-garasu' in name.lower():
        out.append(
            f'You: empty field except **{name}**; opponent has 6 cards including {draw_sp}. Attack directly with **{name}**, '
            f'then pass turns until the opponent would draw: hand stays empty while **{name}** is face-up.'
        )
    elif 'fiber jar' in name.lower():
        out.append(
            f'Both players have damaged fields ({manip} vs {opp}) and uneven graveyards. Flip **{name}** and resolve '
            f'the FLIP effect; opponent may only respond with {chain1} (Trap or Quick-Play Spell), not a Normal Spell like '
            f'{removal}. Confirm timing matches the ROM.'
        )
    elif 'cyber dragon' in name.lower():
        out.append(
            f'Opponent controls {opp} and you hold **{name}** with an empty field. Special Summon **{name}** from hand, '
            f'attack into {opp}, then overlay with format-legal Fusion/Synchro pieces if present in the pool.')
    elif 'breaker the magical warrior' in name.lower():
        out.append(
            f'Normal Summon **{name}**; opponent sets {flip} and {trap}. Battle Phase: attack {opp}, gain the Spell/Trap '
            f'destruction trigger, then destroy the set {trap} and confirm once-per-turn tracking.')
    elif 'kycoo' in name.lower() or 'bazoo' in name.lower():
        gyc = _first_in_pool(
            ctx['pool'],
            ['Monster Reincarnation', 'Call of the Haunted', 'Graceful Charity', 'Premature Burial'],
        )
        gyc_p = f'**{gyc}**' if gyc else 'a revival card from the pool'
        out.append(
            f'Opponent Graveyard: two high-ATK monsters from the pool ({opp}, {trib}). Battle with **{name}**: after battle '
            f'damage, banish up to two opponent Graveyard cards (or activate {gyc_p} first to refill targets).')
    elif 'spirit reaper' in name.lower() or 'don zaloog' in name.lower():
        out.append(
            f'Opponent attacks **{name}** with {opp}; confirm destruction immunity and discards or battle trigger. '
            f'Use {wall} or {removal} from the pool to test interactions with non-destruction removal.')
    elif 'gravekeeper' in name.lower() and 'spy' in name.lower():
        out.append(
            f'Set **{name}** face-down. Opponent attacks with {opp}; after FLIP, Special Summon a Gravekeeper from deck '
            f'(another legal Gravekeeper from the pool) and confirm {opp} cannot destroy **{name}** by battle alone.'
        )
    elif 'cold wave' in name.lower():
        out.append(
            f'First turn of Main Phase 1, before any summon: activate **{name}**. Both players try to Set/activate '
            f'{chain1} or play Quick-Play Spells; Normal Spells such as {draw_sp} must be blocked for two turns. '
            f'Opponent later Normal Summons {opp} under the lock-out.')
    elif 'draw' in blob and 'discard' in blob:
        out.append(
            f'You hold **{name}** and three pool cards; opponent holds {draw_sp}, {opp}, and two others. Resolve **{name}** '
            'and verify each side\'s hand size after equal discard and draw.')
    elif 'destroy' in blob and ('all' in blob or 'every' in blob or 'field' in blob):
        out.append(
            f'You: {manip} + {wall}; opponent: {opp} + set {flip}. Activate **{name}** and list every monster destroyed; '
            f'check whether {flip} FLIP triggers when applicable.')
    elif 'destroy' in blob:
        if _has_activation_window(name, coded, cache):
            out.append(
                f'Opponent controls {opp}; you activate **{name}** on it (or on their set {trap}). If opponent chains '
                f'{chain1}, confirm stack order and resolution. {removal} is a Normal Spell and cannot be chained on '
                f'that stack—only Traps and Quick-Play Spells can.'
            )
        else:
            out.append(
                f'Opponent controls {opp}; use **{name}**\'s destruction effect per **Coded behavior**. Confirm targets; '
                f'use {chain1} in a separate response window if the ROM allows, not a Normal Spell on the same chain.'
            )
    elif 'flip' in blob or 'flip effect' in blob:
        out.append(
            f'Set **{name}**; opponent attacks with {opp} or flips with {removal}. Resolve the FLIP effect against '
            f'a legal target still in the pool ({trib} or {manip}).')
    elif 'equip' in blob or 'united we stand' in name.lower() or 'mage power' in name.lower():
        out.append(
            f'Equip **{name}** to {manip}. Add or remove monsters/spells ({opp}, {draw_sp}) on your field and confirm '
            'ATK/DEF changes match the coded counters.')
    elif 'attack' in blob and ('negate' in blob or 'trap' in ctype.lower()):
        out.append(
            f'Opponent attacks {manip} with {opp}. Activate **{name}** during the attack step; opponent may chain '
            f'{chain1} (or {chain2}) on top—do not use Normal Spells like {removal} as chain links.'
        )
    elif 'monarch' in name.lower() or 'tribute summon' in blob:
        out.append(
            f'Tribute {manip} and {trib} (or one Level 4 from the pool) for **{name}**. On summon, hit the effect against '
            f'opponent card(s) {opp} / {draw_sp} / hand, and test on your next Standby if a Monarch trigger repeats.')
    elif 'draw' in blob:
        out.append(
            f'Open with **{name}** while opponent controls only {opp}. Count hand size before/after relative to '
            f'{draw_sp} in the opponent list; repeat when your Standby/Draw Phase trigger applies.')
    else:
        atk = _p(ctx, 'opp_fatk')
        out.append(
            f'Campaign duel vs the **{fmt}** opponent: put **{name}** in your deck with pool backrow ({trap}) and '
            f'monsters ({manip}, {trib}). Opponent line: Normal Summon {atk}, set {flip}, pass. Play toward **{name}** '
            'as you would in the usage decks and verify effect once as human and once watching CPU.')
        if _has_activation_window(name, coded, cache):
            norm_ex = _p_normal_spell(ctx, cache)
            out.append(
                f'Same board: when you activate **{name}**, opponent may chain {chain1} (or {chain2}). Confirm stack '
                f'order. Normal Spells such as {norm_ex} cannot be chained—only Traps and Quick-Play Spells.'
            )
        elif _is_mandatory_passive(name, coded):
            out.append(
                f'Same duel: after **{name}**\'s mandatory effect runs (see **Coded behavior**—not an optional Trigger '
                f'you activate), confirm the ROM applies it automatically. Opponent cannot chain {chain1} to that '
                f'mandatory step; they may still use it in a later open window.'
            )
        elif 'flip' in ctype.lower() or 'flip:' in coded.lower():
            out.append(
                f'Set and flip **{name}**; when the FLIP effect resolves, opponent may respond with {chain1} only if '
                f'the ROM opens a chain window (Trap / Quick-Play Spell—not Normal Spells like {draw_sp}).'
            )
        else:
            out.append(
                f'Same board: use **{name}** per **Coded behavior**. If the ROM offers a chain window to the opponent, '
                f'it should accept only {chain1}-type responses (Trap or Quick-Play Spell), not Normal Spells.'
            )

    return context, out[:3]


def write_testing_md(assigned: dict[str, tuple[str, str, float]], *, use_api: bool = True) -> None:
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

    stem_pools = {stem: pools for _, stem, pools in banlist.BANLISTS}
    lookup = banlist.card_lookup()
    pool_missing: list[str] = []

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
        'Scenarios only cite card names that appear in that format\'s [`card_pools/`](card_pools/) file, exist as '
        'playable EDS cards, and are not **Forbidden** in [`banlists/<format>_ban.json`](banlists/) (`forbidden`, '
        'limit 0). Staple picks also come from high-usage entries in `usage/<format>.json` that pass the same checks.',
        '',
        '**Chaining:** only **Trap Cards** and **Quick-Play Spell Cards** are described as chain responses. Normal '
        'Spells are never chained onto an existing chain. Mandatory battle follow-ups (e.g. Goblin Attack Force\'s '
        'Defense Position change) are not optional Trigger activations—scenarios do not ask you to chain cards to those '
        'steps.',
        '',
        '**AI:** each card lists **AI coverage** traced from `src/added_effects.c` (`AiPlayAddedCards`, '
        '`AiWantsMonsterEffect`, `AiWantsSpell`, `AiShouldActivateAddedCard`, `AiChain*`, and `*Cpu*` effect steps). '
        '**Yes — generalized routine** means the default added-card fallbacks apply; **dedicated** means a named hook. '
        '**Automatic** means the CPU does not activate an effect from the card menu, but battle/engine code still runs.',
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
            ygo = fetch_ygo(name, cache, use_api=use_api)
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
            ctx = format_test_context(
                stems[name],
                stem_pools.get(stems[name], [stems[name]]),
                name,
                lookup,
                pool_missing,
                cache,
            )
            fmt_context, scenarios = make_scenarios(
                name, fmt_name, stems[name], ctx, coded, ygo, cache)
            ai_stat, ai_detail = ai_status(name, ae_text, engine, coded, impl)
            ai_bullets = ai_scenarios(name, ai_stat, fmt_name, detail=ai_detail)
            lines.append(f'- **Format context:** {fmt_context}')
            lines.append('- **Scenarios:**')
            for i, sc in enumerate(scenarios, 1):
                lines.append(f'  {i}. {sc}')
            lines.append(f'- **AI coverage:** {ai_stat} — {ai_detail}')
            lines.append('- **AI scenarios:**')
            for i, sc in enumerate(ai_bullets, 1):
                lines.append(f'  {i}. {sc}')
            lines.append('')
            if use_api:
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
    ap.add_argument('--no-api', action='store_true', help='Skip YGOPRODeck HTTP; use assets/new_cards/cards.json cache only.')
    args = ap.parse_args()
    names = testing_pool_names()
    assigned = assign_cards(names)
    if args.list_only:
        print(f'{len(assigned)} cards')
        return
    write_testing_md(assigned, use_api=not args.no_api)


if __name__ == '__main__':
    main()
