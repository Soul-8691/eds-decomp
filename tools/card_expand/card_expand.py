#!/usr/bin/env python3
"""Patch map for expanding EDS (AY5E) beyond 820 card IDs.

  tools/dr python3 tools/card_expand/card_expand.py LAST_ID [--rom eds.gba --out build/card_expand/patched.gba]

Run from the repo root (tools/dr does that). The default CSV is build/card_expand/patches.csv.

LAST_ID is the new highest card ID (820 today, at most 2047). Writes every ROM change as a CSV row
(offset, old bytes, new bytes, what) and optionally applies them to a copy of the ROM. It only
repoints code; the caller must still write the grown tables at their new addresses (see layout()).
The pointer list comes from card_table_refs.csv (refs.py). The derivation and the emulator test
(test_relocation.py) are described in wiki/questions/card-capacity.md and wiki/tools/card-expand.md.
"""
import argparse, csv, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
def hexb(b): return ' '.join('%02X' % x for x in b)

ROM = 0x08000000

# ---- RAM: Deck Edit trunk-list rows -> 0x02028380, 0x1000 bytes (2048 ids) per row ----------------
DECKEDIT = 0x0201DB20
NEW_ROW0 = 0x02028380
STRIDE = 0x1000                     # movs #1; lsls #12
NEW_ROW1 = NEW_ROW0 + STRIDE        # ends 0x0202A380
NEW_SCRATCH = NEW_ROW1 + STRIDE     # optional: List Filter non-monster scratch, 0x0202A380..0x0202B380

def u32(v): return struct.pack('<I', v & 0xFFFFFFFF)
def u16(v): return struct.pack('<H', v)
def movs(rd, imm): return 0x2000 | (rd << 8) | imm
def lsls(rd, rm, sh): return (sh << 6) | (rm << 3) | rd

P = []  # (address, old bytes, new bytes, note)
def word(a, old, new, note): P.append((a, u32(old), u32(new), note))
def half(a, old, new, note): P.append((a, u16(old), u16(new), note))
def stride(a, rd, note):  # movs rd,#0xE5; lsls rd,rd,#3 (0x728)  ->  movs rd,#1; lsls rd,rd,#12 (0x1000)
    half(a, movs(rd, 0xE5), movs(rd, 1), note + ': row stride 0x728 -> 0x1000')
    half(a + 2, lsls(rd, rd, 3), lsls(rd, rd, 12), note + ': row stride 0x728 -> 0x1000')

OFF = NEW_ROW0 - DECKEDIT           # 0xA860, replaces the +0x644 trunk-list offset
for fn, a_st, rd, a_lit in [
        ('DeckEdit_GetListCard', 0x08068D44, 1, 0x08068D5C),
        ('DeckEdit_SetListCard', 0x08068DD0, 1, 0x08068DE0),
        ('DeckEdit_GetActiveListRow', 0x08068688, 0, 0x0806869C),
        ('DeckEdit_UpdateNameIndexLetters', 0x08068BD0, 1, 0x08068BDC),
        ('DeckEdit_GetSelectedCardCopies (list 0)', 0x0806848C, 1, 0x080684B0),
        ('DeckStats_CountCategory', 0x0806C5D4, 0, 0x0806C5E0)]:
    stride(a_st, rd, fn)
    word(a_lit, 0x644, OFF, fn + ': trunk offset gDeckEdit+0x644 -> new rows')
# GetSelectedCardCopies, lists 1/2: r6 = 0x728 is shared with the deck/side cases, so the trunk case
# replaces `adds r1,r3,#0; muls r1,r6` by `lsls r1,r3,#12; movs r1,r1` (row is 0 or 1).
for a, a_lit in [(0x08068534, 0x08068540), (0x080685C0, 0x080685CC)]:
    half(a, 0x1C19, lsls(1, 3, 12), 'DeckEdit_GetSelectedCardCopies: row*0x728 -> row<<12')
    half(a + 2, 0x4371, lsls(1, 1, 0), 'DeckEdit_GetSelectedCardCopies: drop muls (movs r1,r1)')
    word(a_lit, 0x644, OFF, 'DeckEdit_GetSelectedCardCopies: trunk offset -> new rows')
# FilterAndSortList: src/dst rows as offsets from gDeckEditSortScratch (0x0201EFC4)
word(0x080692D8, 0xFFFFF1A0, NEW_ROW0 - 0x0201EFC4, 'DeckEdit_FilterAndSortList: trunk row 0 (src)')
word(0x080692DC, 0xFFFFF8C8, NEW_ROW1 - 0x0201EFC4, 'DeckEdit_FilterAndSortList: trunk row 1 (dst)')
# BuildCardLists, mode 1 (Card Trading): r6 = row 0; listCount[0][1] and side row 0 were r6-relative
word(0x080687BC, 0x0201E164, NEW_ROW0, 'DeckEdit_BuildCardLists mode 1: trunk row 0')
word(0x080687C0, 0x00000E52, 0x0201EFB6 - NEW_ROW0, 'DeckEdit_BuildCardLists mode 1: keep r3 = &listCount[0][1]')
word(0x080687C4, 0x0000070A, 0x0201E86E - NEW_ROW0, 'DeckEdit_BuildCardLists mode 1: keep ip = side row 0')
half(0x08068756, movs(0, 0xE5), 0x1E98, 'DeckEdit_BuildCardLists mode 1: r9 = &listCount[0][0] as subs r0,r3,#2')
half(0x08068758, lsls(0, 0, 4), 0x46C0, 'DeckEdit_BuildCardLists mode 1: nop')
half(0x0806875A, 0x1980, 0x46C0, 'DeckEdit_BuildCardLists mode 1: nop')
# mode 0 (Deck Edit) and mode 2 (Prohibition)
word(0x080688C4, 0x0201E164, NEW_ROW0, 'DeckEdit_BuildCardLists mode 0: trunk row 0')
word(0x080689BC, 0xFFFFF1B0, NEW_ROW0 - 0x0201EFB4, 'DeckEdit_BuildCardLists mode 2: trunk row 0 (from listCount)')
# re-filter loop: r9 = trunk row 0 (from &listRow = 0x0201EFC0), row 1 = r9 + stride
word(0x080689D4, 0xFFFFF1A4, NEW_ROW0 - 0x0201EFC0, 'DeckEdit_BuildCardLists refilter: r9 = trunk row 0')
for a, rd in [(0x080689EA, 1), (0x08068A24, 2), (0x08068A8A, 2), (0x08068AD4, 2), (0x08068B32, 5)]:
    stride(a, rd, 'DeckEdit_BuildCardLists refilter (trunk)')

# ---- card count -------------------------------------------------------------------------------------
def imm_pair(a, rd, value, note):
    for sh in range(0, 32):
        if value % (1 << sh) == 0 and (value >> sh) < 256 and value >> sh << sh == value:
            imm, s = value >> sh, sh
            break
    else:
        sys.exit(f'{value:#x} cannot be built by movs+lsls at {a:#x}')
    half(a, movs(rd, 0xCD), movs(rd, imm), note)
    half(a + 2, lsls(rd, rd, 2), lsls(rd, rd, s), note)

def counts(last):
    for a, rd, fn in [(0x08007058, 0, 'DebugCardDetail_Browse'), (0x0801AE0E, 0, 'GiveMissingCards'),
                      (0x08063D86, 0, 'IsCardCollectionComplete'), (0x08068826, 0, 'DeckEdit_BuildCardLists mode 1'),
                      (0x08068916, 0, 'DeckEdit_BuildCardLists mode 0'), (0x0806892E, 3, 'DeckEdit_BuildCardLists mode 2'),
                      (0x0806AB6C, 0, 'SideDeckSwap_Init'), (0x0806D3CE, 0, 'DeckEdit_Init'),
                      (0x0806F232, 0, 'ProhibitCardSelect_Init'), (0x08070C38, 0, 'TradeCardSelect_Init'),
                      (0x08070D9A, 0, 'TradeCardSelect_Init'), (0x0807715E, 0, 'DebugGetAllCards'),
                      (0x0807C362, 0, 'FindCardByPassword')]:
        imm_pair(a, rd, last, fn + ': last card id 0x334')
    for a in (0x08062BC0, 0x08062C84, 0x08062D5C):
        word(a, 0x335, last + 1, 'GeneratePackCards: random id range 0x335')
    word(0x08007050, 0x333, last - 1, 'DebugCardDetail_Browse: last id - 1')
    word(0x08007054, 0x329, last - 10, 'DebugCardDetail_Browse: last id - 10')
    word(0x0800726C, 0x333, last - 1, 'DebugAutoDetail_Cycle: last id - 1')

# ---- ROM tables -------------------------------------------------------------------------------------
# Kept in place, grown forward: names (into the unused all-zero name bank, up to id 1641),
# descriptions (into the old art), ID->number (into the old passwords, up to id 2463), number->ID.
# Moved: stats and passwords (into the old palettes), art and palettes (past the 8 MiB ROM end, from `base`).
ART_BASE = 0x08800000

def check_base(base):
    if base < ART_BASE or base % 4:
        sys.exit(f'base {base:#x} must be word-aligned and at least {ART_BASE:#x} (the end of the original ROM)')

def layout(last, base=ART_BASE):
    check_base(base)
    n = last + 1
    art = base
    pal = art + n * 0x10E0
    stats = 0x08608360
    pw = stats + n * 4
    assert pw + n * 4 <= 0x08621DE0 + 0xCD4
    assert 0x0822C720 + n * 0x40 <= 0x082461A0, 'names no longer fit in place: move gCardNames too'
    return {'gCardArt': (0x082A6500, art), 'gCardArtPalettes': (0x08608360, pal),
            'gCardStats': (0x08621DE0, stats), 'gCardPasswords': (0x08623120, pw)}, pal + n * 0x80

def pointers(last, refs_csv, base=ART_BASE):
    moves, rom_end = layout(last, base)
    for r in csv.DictReader(open(refs_csv)):
        if r['function'] == '(data)':
            continue  # u16 pairs in a game table, not pointers
        a, v, t = int(r['address'], 16), int(r['value'], 16), r['table']
        if a == 0x0806D004:
            continue  # &gCardIdToNumber[0x439]: an out-of-range read in DeckStats_Init; leave it
        if t in moves:
            old, new = moves[t]
            word(a, v, new + (v - old), f"{r['function']}: {t}")
    word(0x08063D98, 0x0862311E, 0x08622AB4 + (last + 1) * 2, 'IsCardCollectionComplete: &gCardIdToNumber[last+1]')
    return moves, rom_end

if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('last', type=lambda s: int(s, 0))
    ap.add_argument('--refs', default=os.path.join(HERE, 'card_table_refs.csv'))
    ap.add_argument('--csv', default='build/card_expand/patches.csv')
    ap.add_argument('--rom'); ap.add_argument('--out')
    ap.add_argument('--ram-only', action='store_true', help='only the Deck Edit list relocation (for testing)')
    ap.add_argument('--move-scratch', action='store_true', help='also move the List Filter non-monster scratch (needed past 310 spell/trap/ticket/god cards)')
    ap.add_argument('--base', type=lambda s: int(s, 0), default=ART_BASE,
                    help=f'ROM address of the moved art; the palettes follow it (default {ART_BASE:#x})')
    a = ap.parse_args()
    assert 821 <= a.last + 1 <= 2048
    moves, rom_end = {}, None
    if not a.ram_only:
        counts(a.last)
        moves, rom_end = pointers(a.last, a.refs, a.base)
    if a.move_scratch:
        word(0x080693D8, 0x0201EFC4, NEW_SCRATCH, 'DeckEdit_FilterAndSortList: non-monster scratch')
        word(0x08069A88, 0x0201EFC4, NEW_SCRATCH, 'DeckEdit_FilterAndSortList: non-monster scratch copy-back')
    P.sort()
    os.makedirs(os.path.dirname(a.csv) or '.', exist_ok=True)
    with open(a.csv, 'w', newline='') as f:
        w = csv.writer(f); w.writerow(['rom_offset', 'address', 'old', 'new', 'what'])
        for ad, o, n, note in P:
            w.writerow([f'ROM+0x{ad - ROM:06X}', f'0x{ad:08X}', hexb(o), hexb(n), note])
    print(f'{len(P)} patches -> {a.csv}; tables:', {k: hex(v[1]) for k, v in moves.items()}, 'ROM end', rom_end and hex(rom_end))
    if a.rom:
        d = bytearray(open(a.rom, 'rb').read())
        for ad, o, n, note in P:
            off = ad - ROM
            assert d[off:off + len(o)] == o, f'{ad:#x}: expected {o.hex()} found {d[off:off+len(o)].hex()} ({note})'
            d[off:off + len(n)] = n
        if a.out:
            os.makedirs(os.path.dirname(a.out) or '.', exist_ok=True)
            open(a.out, 'wb').write(d)
        print('all old bytes verified against', a.rom)
