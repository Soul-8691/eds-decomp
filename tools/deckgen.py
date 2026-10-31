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
    10248389: 'Cyber Blader',
    86099788: 'The Last Warrior from Another Planet',
}
PASSWORDS_CSV = 'assets/cards/passwords.csv'
CARD_COUNT_H = 'build/include/card_count.h'
ASSETS = 'build/assets'
CARDS_H = 'include/constants/cards.h'


def bcd(b):
    s = ''.join(f'{x >> 4}{x & 15}' for x in b)
    return int(s) if s.isdigit() else None


def load_has_card_macros():
    if not os.path.exists(CARD_COUNT_H):
        return set()
    text = open(CARD_COUNT_H, encoding='utf-8').read()
    return {line.split()[1] for line in text.splitlines() if line.startswith('#define HAS_CARD_')}


def tables():
    pw = open(os.path.join(ASSETS, 'newcards__passwords.bin'), 'rb').read()
    i2n = open(os.path.join(ASSETS, 'newcards__id_to_number.bin'), 'rb').read()
    names = open(os.path.join(ASSETS, 'newcards__names.bin'), 'rb').read()
    by_pass_id, by_pass_num, by_name = {}, {}, {}
    for i in range(1, len(pw) // 4):
        p = bcd(pw[i * 4:i * 4 + 4])
        if p is not None and p not in by_pass_id:
            by_pass_id[p] = i
        name = names[i * 0x40:(i + 1) * 0x40].split(b'\0')[0].decode('latin-1')
        by_name.setdefault(name, i)
    for p, name in ALIASES.items():
        if p not in by_pass_id and name in by_name:
            by_pass_id[p] = by_name[name]
    if os.path.exists(PASSWORDS_CSV):
        import csv

        for row in csv.DictReader(open(PASSWORDS_CSV, encoding='utf-8')):
            if not row.get('password'):
                continue
            p = int(row['password'])
            n = int(row['number'])
            if p not in by_pass_num:
                by_pass_num[p] = n
    number = lambda cid: struct.unpack_from('<H', i2n, cid * 2)[0]
    cname = lambda cid: names[cid * 0x40:(cid + 1) * 0x40].split(b'\0')[0].decode('latin-1')
    return by_pass_id, by_pass_num, number, cname, by_name


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
    by_pass_id, by_pass_num, number, cname, by_name = tables()
    macros = card_macros()
    has_macros = load_has_card_macros()
    s = read_ydk(path)
    missing = []
    main_extra = []
    side = []

    def pass_to_number(p):
        if p in by_pass_id:
            return number(by_pass_id[p])
        if p in by_pass_num:
            return by_pass_num[p]
        if p in ALIASES and ALIASES[p] in by_name:
            return number(by_name[ALIASES[p]])
        return None

    for p in s['#main'] + s['#extra']:
        n = pass_to_number(p)
        if n is None:
            missing.append(p)
        else:
            main_extra.append(n)
    for p in s['!side']:
        n = pass_to_number(p)
        if n is None:
            missing.append(p)
        else:
            side.append(n)
    has = sorted(
        {
            f'HAS_CARD_{macros[n]}'
            for n in main_extra + side
            if n in macros and f'HAS_CARD_{macros[n]}' in has_macros
        }
    )
    return main_extra, side, missing, has, macros


def emit_deck(label, path, ydk_comment):
    main_extra, side, missing, has, macros = resolve(path)
    sym = label.replace('_', '')
    print(f'/* {ydk_comment} */')
    if missing:
        print(f'/* missing passcodes: {missing} */')
    if has:
        print('#if ' + ' \\\n    && '.join(f'defined({h})' for h in has))
    else:
        print('#if 1')
    mod = label.upper()
    if not mod.startswith('MOD_'):
        mod = 'MOD_' + mod
    print(f'#define {mod} 1')
    print(f'static const u16 s{sym}[] = {{')
    print('    /* Main + Fusion (' + str(len(main_extra)) + ') */')
    for line in compress_runs(main_extra, macros):
        print(f'    {line},')
    print('};')
    print(f'STATIC_ASSERT(ARRAY_COUNT(s{sym}) == {len(main_extra)}, {sym}Size);')
    print(f'static const struct DeckList s{sym}List = {{ s{sym}, ARRAY_COUNT(s{sym}), 0 }};')
    if side:
        print(f'static const u16 s{sym}Side[] = {{')
        print('    /* Side (' + str(len(side)) + ') */')
        for line in compress_runs(side, macros):
            print(f'    {line},')
        print('};')
        print(f'STATIC_ASSERT(ARRAY_COUNT(s{sym}Side) == {len(side)}, {sym}SideSize);')
    print('#endif')
    print()


def main():
    if len(sys.argv) < 2:
        decks = [
            ('Reaper_Stein_Monarch', 'decks/Jazz-Stein Monarch.ydk', 'Jazz-Stein Monarch.ydk'),
            ('Reaper_Warrior', 'decks/tobitomatenkopf-Warrior.ydk', 'tobitomatenkopf-Warrior.ydk'),
            ('Reaper_Bazoo_Return', 'decks/brahimpoke-Bazoo Return.ydk', 'brahimpoke-Bazoo Return.ydk'),
            ('Reaper_Chaos_Control', 'decks/jcvd93-Chaos Control.ydk', 'jcvd93-Chaos Control.ydk'),
            ('Reaper_Tomato_Control', 'decks/Roland Gress-Tomato Control.ydk', 'Roland Gress-Tomato Control.ydk'),
        ]
    else:
        decks = [(sys.argv[1], sys.argv[2], sys.argv[3])]
    for label, path, comment in decks:
        emit_deck(label, path, comment)


if __name__ == '__main__':
    main()
