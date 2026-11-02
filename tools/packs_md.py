#!/usr/bin/env python3
"""Generate PACKS.md: TCG/usage pack pull rates and usage-stat card prices.

  python3 tools/pack_chances_md.py [PACKS.md]   # after new cards or packs/*_tcg.json edits
  python3 tools/pack_prices_md.py [PACKS.md]    # after usage/*.json or banlist edits
  python3 tools/packs_md.py [PACKS.md]          # both sections

Pull math matches tools/format_packs.py and src/mod_format_packs.c (weighted pool, 8192-entry roulette).
"""
from __future__ import annotations

import glob
import os
import sys

sys.path.insert(0, 'tools')
import banlist  # noqa: E402
import format_packs as fp  # noqa: E402
from cardprices import (  # noqa: E402
    BASE,
    EXTRA_DIVISOR,
    NORM_MAX,
    NORM_MIN,
    NO_USAGE_PRICE,
    format_prices,
    card_names,
)

DEFAULT_OUT = 'PACKS.md'

TCG_SORT_TIE = {
    'Starter Deck- Yugi': 0,
    'Starter Deck- Kaiba': 1,
    'Starter Deck- Joey': 0,
    'Starter Deck- Pegasus': 1,
    'Starter Deck- Yugi Evolution': 0,
    'Starter Deck- Kaiba Evolution': 1,
}


def tcg_sort_key(spec):
    name = spec['name']
    tie = TCG_SORT_TIE.get(name, 99)
    return (spec['release'], tie, name.lower())

MARK_CHANCES_START = '<!-- pack-chances -->'
MARK_CHANCES_END = '<!-- /pack-chances -->'
MARK_PRICES_START = '<!-- pack-prices -->'
MARK_PRICES_END = '<!-- /pack-prices -->'


def static_intro() -> list[str]:
    rw = fp.RARITY_WEIGHT
    lines = [
        '# Booster packs and card prices',
        '',
        'TCG set packs and usage-weighted format packs for the main-menu **Booster Packs** shop '
        '(and the free pick after a Campaign win). Generated documentation; do not edit the marked sections by hand.',
        '',
        'Regenerate when data changes:',
        '',
        '- `python3 tools/pack_chances_md.py` — new cards, new `packs/*_tcg.json` entries, or rarity edits',
        '- `python3 tools/pack_prices_md.py` — `usage/*.json` or `banlists/*_ban.json` changes',
        '- `python3 tools/packs_md.py` — both sections',
        '',
        'Build-time sources: `tools/format_packs.py` → `build/include/format_pack_data.h`; '
        '`tools/cardprices.py` → `build/include/card_prices.h`.',
        '',
        '## How pulls work',
        '',
        'Each opened pack gives **five cards**. For format packs (IDs 601+), `ModGenerateFormatPackCards` '
        '(`src/mod_format_packs.c`) picks five **independent** indices into a fixed **8192-entry roulette** '
        '(built at compile time with a per-pool seed). Each index maps to one card number; weights match the tables below. '
        'The same card is not duplicated in one pack (up to 64 re-rolls per slot). A final shuffle permutes the five slots.',
        '',
        '**Single-card probability** (what the tables list): `weight / sum(weights)` for that pack\'s pool. '
        'With five draws without replacement, a card\'s chance to appear **at least once** in a pack is slightly higher '
        'than `1 - (1 - p)^5` would suggest for independent trials; for rare cards the table value is a good approximation.',
        '',
        'Only card numbers present in the ROM are in a pool. Names resolve through `tools/banlist.py` (including aliases). '
        'TCG JSON names with no ROM match are skipped at build time.',
        '',
        'Shop prices: **250 DP** per TCG set pack, **500 DP** per usage-weighted pack (`include/format_packs.h`). '
        'Vanilla campaign boosters stay **1000 DP**.',
        '',
        '### TCG set packs (`packs/*_tcg.json`)',
        '',
        'Each card name maps to a **rarity rank** (best token among its printings in that JSON). Ranks map to weights:',
        '',
        '| Rank | Printings | Weight |',
        '|---:|---|---:|',
        f'| 0 | Common | {rw[0]} |',
        f'| 1 | Short Print / Super Short Print | {rw[1]} |',
        f'| 2 | Rare | {rw[2]} |',
        f'| 3 | Super Rare | {rw[3]} |',
        f'| 4 | Ultra Rare | {rw[4]} |',
        f'| 5 | Secret Rare | {rw[5]} |',
        f'| 6 | Ultimate Rare | {rw[6]} |',
        '',
        'If one card number appears under several names or rarities, the build keeps the **lowest weight** (rarest). '
        'Pack cover art uses the card with the minimum weight in the pool (ties: lowest card number).',
        '',
        '### Usage-weighted packs (one per Campaign format)',
        '',
        'The pool is every card listed in `usage/<stem>.json` (main, side, or extra), not the full legal pool. '
        f'For each card, share = decks running it / `decks_processed`. Score = `(share_main + share_extra/{EXTRA_DIVISOR}) × {BASE}` '
        '× Limited (×3) or Semi-Limited (×1.5) from that format\'s banlist. Pull weight:',
        '',
        f'`weight = max(1, min(65535, round((1 - {fp.USAGE_PACK_UNIFORM_WEIGHT}/{fp.WEIGHT_SCALE}) × '
        f'{fp.WEIGHT_SCALE}/score^{fp.USAGE_PACK_SCORE_POWER} + {fp.USAGE_PACK_UNIFORM_WEIGHT})))`',
        '',
        'Forbidden cards (limit 0) are omitted. Lower usage ⇒ higher weight ⇒ more common in the pack.',
        '',
    ]
    return lines


def pack_chances_section() -> list[str]:
    lookup = banlist.card_lookup()
    missing: list[str] = []
    names = card_names()

    tcg_by_format: list[list[dict]] = [[] for _ in banlist.BANLISTS]
    for path in glob.glob('packs/*_tcg.json'):
        name = fp.pack_display_name(path)
        if name not in fp.PACK_FORMAT_INDEX:
            continue
        iso = fp.TCG_RELEASE_DATE.get(name)
        fmt = fp.PACK_FORMAT_INDEX[name]
        tcg_by_format[fmt].append({
            'kind': 'tcg',
            'path': path,
            'name': name,
            'format_index': fmt,
            'release': fp.date_ordinal(iso),
        })

    pack_specs: list[dict] = []
    for i, (fmt_name, stem, pools) in enumerate(banlist.BANLISTS):
        for spec in sorted(tcg_by_format[i], key=tcg_sort_key):
            pack_specs.append(spec)
        pack_specs.append({
            'kind': 'usage',
            'path': None,
            'name': fmt_name,
            'format_index': i,
            'release': 0,
            'stem': stem,
            'pools': pools,
        })

    lines = [
        MARK_CHANCES_START,
        '',
        '## Pack pull rates',
        '',
        f'{len(pack_specs)} format packs (TCG sets interleaved with usage packs per Campaign format). '
        f'Roulette size: {fp.ROULETTE_SIZE} entries per pool.',
        '',
    ]
    if missing:
        pass  # filled per-pack below

    for spec in pack_specs:
        display = spec['name']
        if spec['kind'] == 'usage':
            stem = spec['stem']
            limits, _ = banlist.read_limits(f'banlists/{stem}_ban.json', lookup, missing)
            total, extra, decks = fp.usage_maps(stem, lookup, missing)
            entries = []
            for number in sorted(fp.usage_card_numbers(stem, lookup, missing)):
                if number == 0:
                    continue
                w = fp.card_weight(number, total, extra, decks, limits)
                if w is not None:
                    entries.append((number, w))
            kind = 'Usage'
            fmt_label = display
        else:
            entries = fp.pack_entries_tcg(spec['path'], lookup, missing)
            kind = 'TCG'
            fmt_label = banlist.BANLISTS[spec['format_index']][0]

        total_w = sum(w for _, w in entries) or 1
        lines += [
            f'### {display}',
            '',
            f'- **Type:** {kind} · **Format:** {fmt_label} · **Cards in pool:** {len(entries)} · '
            f'**Weight sum:** {total_w}',
            '',
            '| Card | Weight | Pull % |',
            '|---|---:|---:|',
        ]
        for number, weight in sorted(entries, key=lambda kv: (-kv[1], names.get(kv[0], ''))):
            pct = 100.0 * weight / total_w
            card = names.get(number, f'#{number}')
            lines.append(f'| {card} | {weight} | {pct:.4f} |')
        lines.append('')

    if missing:
        uniq = sorted(set(missing))
        lines += [
            f'_{len(uniq)} card names in JSON/usage had no ROM match (omitted from pools)._',
            '',
        ]

    lines.append(MARK_CHANCES_END)
    return lines


def pack_prices_section() -> list[str]:
    lookup = banlist.card_lookup()
    names = card_names()
    lines = [
        MARK_PRICES_START,
        '',
        '## Password machine prices (DP)',
        '',
        'Same formula as `tools/cardprices.py` / `src/mod_dp.c`. Each table lists only cards that format\'s '
        'usage file mentions. At runtime, **GetCardCost** uses the newest unlocked format whose decks use the card.',
        '',
        f'- **Usage score:** main+side share × {BASE} + extra share × {BASE}/{EXTRA_DIVISOR}; '
        f'×3 if Limited, ×1.5 if Semi-Limited.',
        f'- **Scaled price:** linear map per format so the cheapest listed card is **{NORM_MIN} DP** '
        f'and the priciest is **{NORM_MAX} DP**.',
        f'- **Not in any usage table:** **{NO_USAGE_PRICE} DP** (`POOL_CARD_COST` / `DEFAULT_CARD_COST`).',
        '',
        '| Format | Cards priced |',
        '|---|---:|',
    ]
    tables: list[str] = []
    for name, stem, _pools in banlist.BANLISTS:
        prices = format_prices(stem, lookup, [])
        limits, _ = banlist.read_limits(f'banlists/{stem}_ban.json', lookup, [])
        lines.append(f'| [{name}](#{stem}) | {len(prices)} |')
        tables += [f'### {name}', '', '| Card | DP | Limit |', '|---|---:|---|']
        rows = []
        for n, c in sorted(prices.items(), key=lambda kv: (-kv[1], names.get(kv[0], ''))):
            limit = {1: 'Limited', 2: 'Semi-Limited'}.get(limits.get(n), '')
            row = f'| {names.get(n, n)} | {c} | {limit} |'
            if row not in rows:
                rows.append(row)
        tables += rows
        tables.append('')
    lines.append('')
    lines.extend(tables)
    lines.append(MARK_PRICES_END)
    return lines


def _replace_marked_block(text: str, start: str, end: str, new_block: list[str]) -> str:
    block = '\n'.join(new_block)
    if start in text and end in text:
        pre, rest = text.split(start, 1)
        _, post = rest.split(end, 1)
        return pre + block + post
    return text


def write_section(path: str, section: str) -> None:
    if section not in ('chances', 'prices', 'all'):
        raise SystemExit(f'unknown section {section!r}')
    if os.path.isfile(path):
        text = open(path, encoding='utf-8').read()
    else:
        text = '\n'.join(static_intro()) + '\n\n'

    if section in ('chances', 'all'):
        text = _replace_marked_block(text, MARK_CHANCES_START, MARK_CHANCES_END, pack_chances_section())
    if section in ('prices', 'all'):
        if section == 'prices' and MARK_CHANCES_START not in text:
            text = '\n'.join(static_intro()) + '\n\n'
            text = _replace_marked_block(text, MARK_CHANCES_START, MARK_CHANCES_END, pack_chances_section())
        text = _replace_marked_block(text, MARK_PRICES_START, MARK_PRICES_END, pack_prices_section())

    if MARK_CHANCES_START not in text or MARK_PRICES_START not in text:
        # First-time full assembly
        body = static_intro()
        body.extend([''] + pack_chances_section())
        body.extend([''] + pack_prices_section())
        text = '\n'.join(body)

    open(path, 'w', encoding='utf-8', newline='\n').write(text.rstrip('\n') + '\n')
    print(f'packs_md: wrote {section} section → {path}', file=sys.stderr)


def main(argv: list[str] | None = None) -> None:
    argv = argv if argv is not None else sys.argv[1:]
    section = 'all'
    path = DEFAULT_OUT
    for arg in argv:
        if arg in ('--chances', '--pack-chances'):
            section = 'chances'
        elif arg in ('--prices', '--pack-prices'):
            section = 'prices'
        elif arg == '--all':
            section = 'all'
        elif not arg.startswith('-'):
            path = arg
        else:
            raise SystemExit(f'usage: packs_md.py [--chances|--prices] [PACKS.md]')
    write_section(path, section)


if __name__ == '__main__':
    main()
