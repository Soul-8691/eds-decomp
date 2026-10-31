#!/usr/bin/env python3
"""Booster packs for the main-menu shop (src/mod_format_packs.c).

  python3 tools/format_packs.py build/include/format_pack_data.h

TCG sets from packs/*_tcg.json interleaved with usage-weighted format packs (BANLISTS order):
each format's real sets in release order, then that format's usage pack (LOB / Yugi / Kaiba / Yugi-Kaiba / MRD / …).
Usage packs list only cards present in usage/<stem>.json (weighted like cardprices.py), not the full card pool.
Format event dates come from packs/formats.json field \"date\".
"""
import bisect
import glob
import json
import os
import random
import re
import struct
import sys

sys.path.insert(0, 'tools')
import banlist  # noqa: E402
from cardprices import BASE, EXTRA_DIVISOR, LIMIT_FACTOR, share  # noqa: E402

FORMAT_PACK_ID_FIRST = 601
VANILLA_PACK_INFO_COUNT = 23
# 50 packs × ROULETTE_SIZE × 2 bytes (mod_format_pack_roulettes @rodata); keep .text.added headroom.
ROULETTE_SIZE = 8192
WEIGHT_SCALE = 10000
# Usage-pack pull weight ∝ 1/share^POWER (before banlist limits). 1.0 = full inverse usage;
# lower values flatten pulls so staples (high usage) show up more and the tail repeats less.
USAGE_PACK_SCORE_POWER = 0.4
# Baseline weight so every card in the pool keeps some chance (reduces “same six cards” loops).
USAGE_PACK_UNIFORM_WEIGHT = 120
FORMAT_PACK_FLAG_USAGE = 1
FORMAT_PACK_USAGE_WINS = 10

FORMATS_JSON = 'packs/formats.json'

# TCG English release dates (Yugipedia Set chronology); used for shop sort order only.
TCG_RELEASE_DATE = {
    'Legend of Blue Eyes White Dragon': '2002-03-08',
    'Starter Deck- Yugi': '2002-03-29',
    'Starter Deck- Kaiba': '2002-03-29',
    'Metal Raiders': '2002-06-26',
    'Spell Ruler': '2002-09-16',
    'Pharaoh\'s Servant': '2002-10-20',
    'Labyrinth of Nightmare': '2003-03-01',
    'Starter Deck- Joey': '2003-03-30',
    'Starter Deck- Pegasus': '2003-03-30',
    'Legacy of Darkness': '2003-06-06',
    'Pharaonic Guardian': '2003-07-18',
    'Magician\'s Force': '2003-10-10',
    'Dark Crisis': '2003-12-01',
    'Invasion of Chaos': '2004-03-01',
    'Starter Deck- Yugi Evolution': '2004-03-01',
    'Starter Deck- Kaiba Evolution': '2004-03-01',
    'Ancient Sanctuary': '2004-06-01',
    'Soul of the Duelist': '2004-10-01',
    'Dark Beginning 1': '2004-10-12',
    'Rise of Destiny': '2004-12-01',
    'Structure Deck- Dragon\'s Roar': '2005-01-01',
    'Structure Deck- Zombie Madness': '2005-01-01',
    'Flaming Eternity': '2005-03-01',
    'Dark Revelation Volume 1': '2005-03-19',
    'Structure Deck- Blaze of Destruction': '2005-05-06',
    'Structure Deck- Fury from the Deep': '2005-05-06',
    'The Lost Millennium': '2005-06-01',
    'Dark Beginning 2': '2005-07-27',
    'Cybernetic Revolution': '2005-08-17',
    'Dark Revelation Volume 2': '2005-10-20',
    'Structure Deck- Warrior\'s Triumph': '2005-10-28',
    'Elemental Energy': '2005-11-16',
    'Structure Deck- Spellcaster\'s Judgment': '2006-01-18',
    'Shadow of Infinity': '2006-02-18',
    'Starter Deck 2006': '2006-03-23',
}

# Campaign format index (BANLISTS order) for each TCG set file basename.
PACK_FORMAT_INDEX = {
    'Legend of Blue Eyes White Dragon': 0,
    'Starter Deck- Yugi': 0,
    'Starter Deck- Kaiba': 0,
    'Metal Raiders': 1,
    'Spell Ruler': 2,
    'Pharaoh\'s Servant': 3,
    'Labyrinth of Nightmare': 4,
    'Starter Deck- Joey': 5,
    'Starter Deck- Pegasus': 5,
    'Legacy of Darkness': 6,
    'Pharaonic Guardian': 7,
    'Magician\'s Force': 8,
    'Dark Crisis': 9,
    'Invasion of Chaos': 10,
    'Starter Deck- Yugi Evolution': 10,
    'Starter Deck- Kaiba Evolution': 10,
    'Ancient Sanctuary': 11,
    'Soul of the Duelist': 11,
    'Dark Beginning 1': 11,
    'Rise of Destiny': 11,
    'Structure Deck- Dragon\'s Roar': 11,
    'Structure Deck- Zombie Madness': 11,
    'Flaming Eternity': 12,
    'Dark Revelation Volume 1': 12,
    'Structure Deck- Blaze of Destruction': 12,
    'Structure Deck- Fury from the Deep': 12,
    'The Lost Millennium': 12,
    'Dark Beginning 2': 12,
    'Dark Revelation Volume 2': 12,
    'Structure Deck- Warrior\'s Triumph': 12,
    'Cybernetic Revolution': 13,
    'Elemental Energy': 14,
    'Structure Deck- Spellcaster\'s Judgment': 14,
    'Shadow of Infinity': 14,
    'Starter Deck 2006': 14,
}

PORTRAIT_NAMES = [
    'La Jinn the Mystical Genie of the Lamp',
    'Sangan',
    'Confiscation',
    'Imperial Order',
    'Jinzo',
    'Graceful Charity',
    'Fiber Jar',
    'Yata-Garasu',
    'Magical Scientist',
    'Vampire Lord',
    'Chaos Emperor Dragon - Envoy of the End',
    'D.D. Warrior Lady',
    'Scapegoat',
    'Cyber Dragon',
    'Spirit Reaper',
]

RARITY_WEIGHT = {
    0: 10000,
    1: 7000,
    2: 2500,
    3: 600,
    4: 100,
    5: 25,
    6: 10,
}

RARITY_TOKEN_RANK = {
    'common': 0, 'c': 0,
    'short print': 1, 'super short print': 1, 'sp': 1, 'ssp': 1,
    'rare': 2, 'r': 2,
    'super rare': 3, 'sr': 3,
    'ultra rare': 4, 'ur': 4,
    'secret rare': 5, 'scr': 5,
    'ultimate rare': 6, 'utr': 6,
}


def date_ordinal(iso):
    """YYYY-MM-DD -> YYYYMMDD u32, or 0 when missing."""
    if not iso:
        return 0
    y, m, d = iso.split('-')
    return int(y) * 10000 + int(m) * 100 + int(d)


def load_format_event_dates():
    """Event dates for BANLISTS order from packs/formats.json \"date\"."""
    raw = json.load(open(FORMATS_JSON, encoding='utf-8'))
    by_name = {}
    for entry in raw:
        if entry.get('category') != 'TCG':
            continue
        name = entry.get('name')
        if name and entry.get('date'):
            by_name[name] = entry['date']
    ordinals = []
    for fmt_name, _stem, _pools in banlist.BANLISTS:
        iso = by_name.get(fmt_name)
        if iso is None:
            print(f'format_packs: no formats.json date for {fmt_name!r}', file=sys.stderr)
            ordinals.append(0)
        else:
            ordinals.append(date_ordinal(iso))
    return ordinals


def rarity_rank(rarity_str):
    if not rarity_str or not isinstance(rarity_str, str):
        return None
    s = rarity_str.strip()
    if not s or len(s) > 48:
        return None
    low = s.lower()
    if 'tip card' in low:
        return None
    if any(x in low for x in ('deck', 'strategy', 'phase', 'summon', 'attack', 'defense', 'shuffling')):
        if low not in RARITY_TOKEN_RANK:
            return None
    parts = re.split(r'[,/]+', low)
    rank = None
    for part in parts:
        part = part.strip()
        if part in RARITY_TOKEN_RANK:
            r = RARITY_TOKEN_RANK[part]
            rank = r if rank is None else max(rank, r)
    return rank


def pack_entries_tcg(path, lookup, missing):
    data = json.load(open(path, encoding='utf-8'))
    by_number = {}
    for name, rarities in data.items():
        if not rarities:
            continue
        rank = None
        for r in rarities:
            rr = rarity_rank(r)
            if rr is not None:
                rank = rr if rank is None else max(rank, rr)
        if rank is None:
            continue
        numbers = lookup(name)
        if numbers is None:
            missing.append(name)
            continue
        weight = RARITY_WEIGHT[rank]
        for number in numbers:
            if number == 0:
                continue
            prev = by_number.get(number)
            if prev is None or weight < prev:
                by_number[number] = weight
    return sorted(by_number.items())


def usage_maps(stem, lookup, missing):
    path = f'usage/{stem}.json'
    try:
        usage = json.load(open(path, encoding='utf-8'))
    except FileNotFoundError:
        return {}, {}, 1
    decks = max(usage.get('decks_processed', 1), 1)
    total, extra = {}, {}
    for entry in usage.get('total', []):
        numbers = lookup(entry['name'])
        if numbers is None:
            missing.append(entry['name'])
            continue
        s = share(entry, decks)
        for number in numbers:
            total[number] = total.get(number, 0.0) + s
    for entry in usage.get('extra', []):
        numbers = lookup(entry['name'])
        if numbers is None:
            missing.append(entry['name'])
            continue
        s = share(entry, decks)
        for number in numbers:
            extra[number] = extra.get(number, 0.0) + s
    return total, extra, decks


def usage_card_numbers(stem, lookup, missing):
    """Card numbers that appear in usage/<stem>.json (main, side, or extra lists)."""
    numbers = set()
    path = f'usage/{stem}.json'
    try:
        usage = json.load(open(path, encoding='utf-8'))
    except FileNotFoundError:
        return numbers
    for key in ('total', 'extra'):
        for entry in usage.get(key, []):
            nums = lookup(entry['name'])
            if nums is None:
                missing.append(entry['name'])
                continue
            numbers |= nums
    return numbers


def card_weight(number, total, extra, decks, limits):
    if limits.get(number) == 0:
        return None
    s = total.get(number, 0.0) + extra.get(number, 0.0) / EXTRA_DIVISOR
    if s <= 0.0:
        return None
    score = s * BASE * LIMIT_FACTOR.get(limits.get(number), 1.0)
    skew = WEIGHT_SCALE / (score ** USAGE_PACK_SCORE_POWER)
    w = int((1.0 - USAGE_PACK_UNIFORM_WEIGHT / WEIGHT_SCALE) * skew + USAGE_PACK_UNIFORM_WEIGHT + 0.5)
    return max(1, min(w, 65535))


def portrait_number(entries):
    if not entries:
        return 0
    min_w = min(w for _, w in entries)
    return min(n for n, w in entries if w == min_w)


def pack_display_name(path):
    base = os.path.basename(path)
    if base.endswith('_tcg.json'):
        base = base[:-9]
    return base


def build_roulette(entries, seed):
    numbers = [n for n, _ in entries]
    cum = []
    total = 0
    for _, weight in entries:
        total += weight
        cum.append(total)
    rng = random.Random(seed)
    table = []
    for _ in range(ROULETTE_SIZE):
        r = rng.randrange(total)
        idx = bisect.bisect_right(cum, r)
        if idx >= len(numbers):
            idx = len(numbers) - 1
        table.append(numbers[idx])
    return table


def main():
    out = sys.argv[1]
    bin_out = os.path.join(os.path.dirname(os.path.dirname(out)), 'assets', 'format_pack_roulettes.bin')
    lookup = banlist.card_lookup()
    missing = []
    format_event_dates = load_format_event_dates()

    tcg_by_format = [[] for _ in banlist.BANLISTS]
    for path in glob.glob('packs/*_tcg.json'):
        name = pack_display_name(path)
        if name not in PACK_FORMAT_INDEX:
            print(f'format_packs: no format index for {name!r}', file=sys.stderr)
            continue
        iso = TCG_RELEASE_DATE.get(name)
        if not iso:
            print(f'format_packs: no TCG release date for {name!r}', file=sys.stderr)
        fmt = PACK_FORMAT_INDEX[name]
        tcg_by_format[fmt].append({
            'kind': 'tcg',
            'path': path,
            'name': name,
            'format_index': fmt,
            'release': date_ordinal(iso),
        })

    def tcg_sort_key(spec):
        name = spec['name']
        # Same-day starters: Yugi before Kaiba; Joey before Pegasus.
        tie = {
            'Starter Deck- Yugi': 0,
            'Starter Deck- Kaiba': 1,
            'Starter Deck- Joey': 0,
            'Starter Deck- Pegasus': 1,
            'Starter Deck- Yugi Evolution': 0,
            'Starter Deck- Kaiba Evolution': 1,
        }.get(name, 99)
        return (spec['release'], tie, name.lower())

    pack_specs = []
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
        '/* Generated by tools/format_packs.py. Do not edit. */',
        f'#define FORMAT_PACK_COUNT {len(pack_specs)}',
        f'#define FORMAT_PACK_ID_FIRST {FORMAT_PACK_ID_FIRST}',
        f'#define VANILLA_PACK_INFO_COUNT {VANILLA_PACK_INFO_COUNT}',
        f'#define FORMAT_PACK_ROULETTE_SIZE {ROULETTE_SIZE}',
        f'#define FORMAT_EVENT_COUNT {len(banlist.BANLISTS)}',
        f'#define FORMAT_PACK_FLAG_USAGE {FORMAT_PACK_FLAG_USAGE}',
        f'#define FORMAT_PACK_USAGE_WINS {FORMAT_PACK_USAGE_WINS}',
        '',
        'struct FormatPackEntry { u16 number; u16 weight; };',
        '',
        f'static const u32 sFormatEventDate[FORMAT_EVENT_COUNT] = {{',
        '    ' + ', '.join(str(d) for d in format_event_dates) + ',',
        '};',
        '',
    ]

    portrait_nums = []
    flags = []
    format_indices = []
    release_dates = []
    pool_lines = []
    roulette_ptrs = []
    roulette_blob = bytearray()
    names = []

    usage_portraits = []
    for name in PORTRAIT_NAMES:
        nums = lookup(name)
        usage_portraits.append(min(nums) if nums else 0)

    for i, spec in enumerate(pack_specs):
        display = spec['name']
        names.append(display)
        format_indices.append(spec['format_index'])
        release_dates.append(spec['release'])

        if spec['kind'] == 'usage':
            flags.append(FORMAT_PACK_FLAG_USAGE)
            stem = spec['stem']
            limits, _ = banlist.read_limits(f'banlists/{stem}_ban.json', lookup, missing)
            total, extra, decks = usage_maps(stem, lookup, missing)
            entries = []
            for number in sorted(usage_card_numbers(stem, lookup, missing)):
                if number == 0:
                    continue
                w = card_weight(number, total, extra, decks, limits)
                if w is not None:
                    entries.append((number, w))
            seed = stem
            portrait_nums.append(usage_portraits[spec['format_index']])
            label = f'{display} usage'
        else:
            flags.append(0)
            entries = pack_entries_tcg(spec['path'], lookup, missing)
            seed = os.path.basename(spec['path'])
            portrait_nums.append(portrait_number(entries))
            label = f'{display} ({len(entries)} ROM cards)'

        lines.append(f'/* {label} */')
        lines.append(f'static const struct FormatPackEntry sFormatPackPool{i}[] = {{')
        for j in range(0, len(entries), 6):
            chunk = entries[j:j + 6]
            lines.append('    ' + ', '.join(f'{{ {n}, {w} }}' for n, w in chunk) + ',')
        lines.append('};')
        roulette = build_roulette(entries, seed)
        for n in roulette:
            roulette_blob += struct.pack('<H', n)
        pool_lines.append(f'    {{ sFormatPackPool{i}, {len(entries)} }},')
        roulette_ptrs.append(f'    &gFormatPackRouletteBlob[{i} * FORMAT_PACK_ROULETTE_SIZE],')

    os.makedirs(os.path.dirname(bin_out), exist_ok=True)
    try:
        if open(bin_out, 'rb').read() != roulette_blob:
            open(bin_out, 'wb').write(roulette_blob)
    except FileNotFoundError:
        open(bin_out, 'wb').write(roulette_blob)

    lines.append('static const u8 sFormatPackFlags[FORMAT_PACK_COUNT] = {')
    lines.append('    ' + ', '.join(str(f) for f in flags) + ',')
    lines.append('};')
    lines.append('')
    lines.append('static const u8 sFormatPackFormatIndex[FORMAT_PACK_COUNT] = {')
    lines.append('    ' + ', '.join(str(f) for f in format_indices) + ',')
    lines.append('};')
    lines.append('')
    lines.append('static const u32 sFormatPackReleaseDate[FORMAT_PACK_COUNT] = {')
    lines.append('    ' + ', '.join(str(d) for d in release_dates) + ',')
    lines.append('};')
    lines.append('')

    lines.append('static const u16 sFormatPackPortraitNumber[FORMAT_PACK_COUNT] = {')
    lines.append('    ' + ', '.join(str(n) for n in portrait_nums) + ',')
    lines.append('};')
    lines.append('')

    lines.append(
        'extern const u16 gFormatPackRouletteBlob[FORMAT_PACK_COUNT * FORMAT_PACK_ROULETTE_SIZE];'
    )
    lines.append('')
    lines.append('static const u16 *const sFormatPackRoulettes[FORMAT_PACK_COUNT] = {')
    lines.extend(roulette_ptrs)
    lines.append('};')
    lines.append('')

    lines.append('static const struct FormatPackPool {')
    lines.append('    const struct FormatPackEntry *entries;')
    lines.append('    u16 count;')
    lines.append('} sFormatPackPools[FORMAT_PACK_COUNT] = {')
    lines.extend(pool_lines)
    lines.append('};')
    lines.append('')

    lines.append('static const char *const sFormatPackNames[FORMAT_PACK_COUNT] = {')
    for n in names:
        esc = n.replace('\\', '\\\\').replace('"', '\\"')
        lines.append(f'    "{esc}",')
    lines.append('};')
    lines.append('')

    banlist.write_if_changed(out, '\n'.join(lines))
    write_make_deps(out)
    print(
        f'format_packs: {len(pack_specs)} packs (TCG + usage interleaved per format), '
        f'{len(set(missing))} names not in ROM (skipped)',
        file=sys.stderr,
    )


def write_make_deps(header_path):
    """GNU make splits prerequisites on spaces; TCG JSON paths contain spaces."""
    dep_path = os.path.splitext(header_path)[0] + '.d'
    tcg = sorted(glob.glob('packs/*_tcg.json'))
    usage = sorted(glob.glob('usage/*.json'))

    def esc(path):
        return path.replace('\\', '/').replace(' ', r'\ ')

    prereqs = [
        'tools/format_packs.py',
        'tools/banlist.py',
        'packs/formats.json',
        *[esc(p) for p in tcg],
        *[esc(p) for p in usage],
        'build/assets/.newcards',
    ]
    line = f'{esc(header_path)}: {" ".join(prereqs)}\n'
    try:
        if open(dep_path, encoding='utf-8').read() == line:
            return
    except FileNotFoundError:
        pass
    open(dep_path, 'w', encoding='utf-8').write(line)


if __name__ == '__main__':
    main()
