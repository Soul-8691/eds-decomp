#!/usr/bin/env python3
"""Generate mod_decks.c snippets from .ydk files (same resolution as testdeck.py)."""
import os
import re
import struct
import sys
from collections import Counter, OrderedDict

ALIASES = {
    11901678: 'B. Skull Dragon',
    98502113: 'Dark Paladin',
    70095154: 'Cyber Dragon',
    1546123: 'Cyber End Dragon',
    23995346: 'Manticore of Darkness',
}
ASSETS = 'build/assets'
CARDS_H = 'include/constants/cards.h'


def bcd(b):
    s = ''.join(f'{x >> 4}{x & 15}' for x in b)
    return int(s) if s.isdigit() else None


def tables():
    pw = open(os.path.join(ASSETS, 'newcards__passwords.bin'), 'rb').read()
    i2n = open(os.path.join(ASSETS, 'newcards__id_to_number.bin'), 'rb').read()
    names = open(os.path.join(ASSETS, 'newcards__names.bin'), 'rb').read()
    by_pass, by_name = {}, {}
    for i in range(1, len(pw) // 4):
        p = bcd(pw[i * 4:i * 4 + 4])
        if p is not None and p not in by_pass:
            by_pass[p] = i
        name = names[i * 0x40:(i + 1) * 0x40].split(b'\0')[0].decode('latin-1')
        by_name.setdefault(name, i)
    for p, name in ALIASES.items():
        if p not in by_pass and name in by_name:
            by_pass[p] = by_name[name]
    number = lambda cid: struct.unpack_from('<H', i2n, cid * 2)[0]
    name = lambda cid: names[cid * 0x40:(cid + 1) * 0x40].split(b'\0')[0].decode('latin-1')
    return by_pass, number, name


def read_ydk(path):
    sections = {'#main': [], '#extra': [], '!side': []}
    sec = None
    for line in open(path, encoding='utf-8', errors='replace'):
        line = line.strip()
        if line in sections:
            sec = line
        elif line.isdigit() and sec:
            sections[sec].append(int(line))
    return sections


def card_macros():
    m = {}
    for line in open(CARDS_H, encoding='utf-8'):
        mm = re.match(r'\s*CARD_(\w+)\s*=\s*(\d+)', line)
        if mm:
            m[int(mm.group(2))] = mm.group(1)
    return m


def compress_runs(nums, macros):
    """Collapse consecutive identical CARD_* into X2/X3 where possible."""
    out = []
    i = 0
    while i < len(nums):
        n = nums[i]
        macro = macros.get(n)
        if macro is None:
            j = i + 1
            while j < len(nums) and nums[j] == n:
                j += 1
            cnt = j - i
            for _ in range(cnt):
                out.append(str(n))
            i = j
            continue
        sym = f'CARD_{macro}'
        j = i + 1
        while j < len(nums) and nums[j] == n:
            j += 1
        cnt = j - i
        if cnt == 1:
            out.append(sym)
        elif cnt == 2:
            out.append(f'X2({sym})')
        elif cnt == 3:
            out.append(f'X3({sym})')
        else:
            for _ in range(cnt):
                out.append(sym)
        i = j
    return out


def resolve(path):
    by_pass, number, cname = tables()
    macros = card_macros()
    s = read_ydk(path)
    missing = []
    main_extra = []
    side = []
    for p in s['#main'] + s['#extra']:
        if p in by_pass:
            main_extra.append(number(by_pass[p]))
        else:
            missing.append(p)
    for p in s['!side']:
        if p in by_pass:
            side.append(number(by_pass[p]))
        else:
            missing.append(p)
    has = sorted({f'HAS_CARD_{macros[n]}' for n in main_extra + side if n in macros})
    return main_extra, side, missing, has, macros


def emit_deck(label, path, ydk_comment):
    main_extra, side, missing, has, macros = resolve(path)
    print(f'/* {ydk_comment} */')
    if missing:
        print(f'/* missing passcodes: {missing} */')
    print('#if ' + ' \\\n    && '.join(f'defined({h})' for h in has))
    print(f'#define MOD_{label.upper()} 1')
    print(f'static const u16 s{label}[] = {{')
    print('    /* Main + Fusion (' + str(len(main_extra)) + ') */')
    for line in compress_runs(main_extra, macros):
        print(f'    {line},')
    print('};')
    print(f'STATIC_ASSERT(ARRAY_COUNT(s{label}) == {len(main_extra)}, {label}Size);')
    print(f'static const struct DeckList s{label}List = {{ s{label}, ARRAY_COUNT(s{label}), 0 }};')
    if side:
        print(f'static const u16 s{label}Side[] = {{')
        print('    /* Side (' + str(len(side)) + ') */')
        for line in compress_runs(side, macros):
            print(f'    {line},')
        print('};')
        print(f'STATIC_ASSERT(ARRAY_COUNT(s{label}Side) == {len(side)}, {label}SideSize);')
    print('#endif')
    print()


def main():
    if len(sys.argv) < 2:
        decks = [
            ('ReaperBazooReturn', 'decks/brahimpoke-Bazoo Return.ydk', 'brahimpoke-Bazoo Return.ydk'),
            ('ReaperTomatoControl', 'decks/Roland Gress-Tomato Control.ydk', 'Roland Gress-Tomato Control.ydk'),
        ]
    else:
        decks = [(sys.argv[1], sys.argv[2], sys.argv[3])]
    for label, path, comment in decks:
        emit_deck(label, path, comment)


if __name__ == '__main__':
    main()
