#!/usr/bin/env python3
"""Emulator check of the leftover cards in a ROM written by expand_rom.py.

  python3 tools/card_expand/test_leftovers.py [--rom build/card_expand/expanded.gba]

Needs `unicorn`. Reads the card list expand_rom.py wrote next to the ROM (<rom>_cards.csv) and checks, by running
the game's own code where there is one:
  - card number <-> card ID: gCardNumberToId and gCardIdToNumber agree with the list (tokens included);
  - FindCardByPassword finds each card with a password from its password;
  - DeckEdit_IsFusionMonster is true exactly for the Fusion monsters (they go to the Fusion Deck);
  - IsMaterialOfFusion accepts both materials of the five Fusion recipes that use leftover numbers;
  - FindRitualRecipe finds the recipe of Curse of the Masked Beast (1416, ritual monster 1411).
"""
import argparse, csv, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import test_tables as tt                                     # Gba (Unicorn wrapper), addresses

IS_FUSION_DECK, IS_MATERIAL, FIND_RITUAL = 0x080668DC, 0x0803CDEC, 0x0804353C
ID2N, N2ID = 0x08622AB4, 0x08623DF4
RECIPES = {1241: (1240, None), 1334: (1333, None), 1404: (1402, 1403), 1445: (1444, 1446), 1526: (1523, 1525)}
EDS_MATERIALS = {1241: 'Gazelle the King of Mythical Beasts', 1334: 'Relinquished'}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--rom', default='build/card_expand/expanded.gba')
    a = ap.parse_args()
    rom = open(a.rom, 'rb').read()
    rows = [r for r in csv.DictReader(open(os.path.splitext(a.rom)[0] + '_cards.csv', encoding='utf-8'))
            if r['source'] == 'leftover']
    by_number = {int(r['number']): r for r in rows}
    g = tt.Gba(rom)
    pw_base = struct.unpack_from('<I', rom, 0x07C35C)[0]       # FindCardByPassword's table literal
    bad = []

    for r in rows:
        cid, no = int(r['id']), int(r['number'])
        if struct.unpack_from('<H', rom, N2ID - 0x08000000 + no * 2)[0] != cid:
            bad.append(f'{no}: gCardNumberToId does not give ID {cid}')
        if struct.unpack_from('<H', rom, ID2N - 0x08000000 + cid * 2)[0] != no:
            bad.append(f'ID {cid}: gCardIdToNumber does not give {no}')
        if r['password']:
            pw = bytes.fromhex(r['password'])
            if rom[pw_base - 0x08000000 + cid * 4:pw_base - 0x08000000 + cid * 4 + 4] != pw:
                bad.append(f'ID {cid}: password table entry differs')
            g.w(tt.PASSWORD + 8, bytes(d for b in pw for d in (b >> 4, b & 15)))
            found = g.call(tt.F['FindPassword']) & 0xFFFF
            if found != cid:
                bad.append(f"{r['name']}: FindCardByPassword gives {found}, not {cid}")
        fusion = 'Fusion' in r['card']
        if bool(g.call(IS_FUSION_DECK, cid) & 0xFFFF) != fusion:
            bad.append(f"{r['name']}: DeckEdit_IsFusionMonster says {not fusion}")
    print(f'{len(rows)} leftover cards: numbers, IDs, passwords and Fusion Deck flags checked')

    names = {}
    base = struct.unpack_from('<I', rom, 0x001020)[0] - 0x08000000   # gCardNames, from Bustup_UpdateTextBox's literal
    for i in range(821):
        names[rom[base + i * 0x40:base + i * 0x40 + 0x40].split(b'\0')[0].decode('cp1252')] = i
    for result, mats in RECIPES.items():
        fid = int(by_number[result]['id'])
        ids = [int(by_number[m]['id']) for m in mats if m] + ([names[EDS_MATERIALS[result]]] if result in EDS_MATERIALS else [])
        for m in ids:
            if not g.call(IS_MATERIAL, fid, m) & 0xFF:
                bad.append(f"IsMaterialOfFusion({by_number[result]['name']}, ID {m}) is false")
    print('Fusion recipes: ' + ', '.join(by_number[r]['name'] for r in RECIPES))
    curse = int(by_number[1416]['id'])
    row = g.call(FIND_RITUAL, curse)
    row = row - (1 << 32) if row >= 1 << 31 else row
    if row != 15:
        bad.append(f'FindRitualRecipe(Curse of the Masked Beast) = {row}, not 15')
    print(f'FindRitualRecipe(Curse of the Masked Beast) = {row}')
    for b in bad:
        print('FAIL', b)
    print('PASS' if not bad else f'FAIL ({len(bad)})')
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
