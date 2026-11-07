#!/usr/bin/env python3
"""Fill the Records screen's opponent faces (gModRecordFaces, src/mod_opponents.c DrawRecordFace) in a linked ROM.

  python3 tools/record_faces.py eds_demo.gba eds_demo.sym build/eds.elf      (the Makefile's ROM rule, after fixptrs)

For each opponent whose portrait is card art (sOpponents' portrait, a card number: the format opponents, Debug, the
Test Duel), the face is the centre of that card's art in this ROM: its full width and the 24 rows around its middle
(rows 28-51) scaled to 48 x 16 (an art pixel every 1.5 screen pixels), in the 15 colours that strip uses most
(colours 1-15; the first-counted colour wins a tie), every other art colour mapped to the nearest of them (squared
distance in 5-bit channels; the first wins a tie). Each entry (by duelist ID): u16 colours[16], then 6 x 2 4bpp
tiles, row-major. They are data so Hex Maniac Advance can edit them (tools/hma_toml.py graphics.opponents.<name>.face);
an opponent without card art keeps a zero entry, and the game draws its original face.
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from hma_toml import elf_symbols, load_sym  # noqa: E402

ROM_BASE = 0x08000000
FACE_SIZE = 16 * 2 + 6 * 2 * 32
ART_SIZE, ART_PALETTE_SIZE = 0x10E0, 0x80
CARD_NUMBER_ALT_ART = 2000
OPPONENT_SIZE = 12          # struct ModOpponent: kind, order, banlist, pad, name, portrait (u16), pad


def art_pixel(rom, gfx, card_id, x, y):
    base = gfx + card_id * ART_SIZE
    i = ((y >> 3) * 9 + (x >> 3)) * 64 + (y & 7) * 8 + (x & 7)
    bit = i * 6
    return ((rom[base + (bit >> 3)] | rom[base + (bit >> 3) + 1] << 8) >> (bit & 7)) & 63


def colour_dist(a, b):
    return sum(((a >> s & 31) - (b >> s & 31)) ** 2 for s in (0, 5, 10))


def face(rom, gfx, pals, card_id):
    art_pal = struct.unpack_from('<64H', rom, pals + card_id * ART_PALETTE_SIZE)
    strip = [art_pixel(rom, gfx, card_id, x * 3 // 2, 28 + y * 3 // 2) for y in range(16) for x in range(48)]
    count = [strip.count(c) for c in range(64)]
    pick = []
    for _ in range(15):
        best = -1
        for i in range(64):
            if count[i] and (best < 0 or count[i] > count[best]):
                best = i
        if best < 0:
            break
        pick.append(best)
        count[best] = 0
    remap = []
    for i in range(64):
        best_d, m = None, 1
        for j, pc in enumerate(pick):
            d = colour_dist(art_pal[i], art_pal[pc])
            if best_d is None or d < best_d:
                best_d, m = d, 1 + j
        remap.append(m)
    colours = [0] + [art_pal[pc] & 0x7FFF for pc in pick] + [0] * (15 - len(pick))
    tiles = bytearray(6 * 2 * 32)
    for y in range(16):
        for x in range(48):
            c = remap[strip[y * 48 + x]]
            tiles[((y >> 3) * 6 + (x >> 3)) * 32 + (y & 7) * 4 + ((x & 7) >> 1)] |= c << 4 if x & 1 else c
    return struct.pack('<16H', *colours) + bytes(tiles)


def main():
    rom_path, sym_path, elf_path = sys.argv[1:4]
    sym = load_sym(sym_path)
    elf = elf_symbols(elf_path)
    rom = bytearray(open(rom_path, 'rb').read())
    opponents = elf.get('mod_opponents.o:sOpponents')
    if 'gModRecordFaces' not in sym or opponents is None:
        sys.exit('record_faces: no gModRecordFaces or sOpponents')
    faces = sym['gModRecordFaces'][0] - ROM_BASE
    gfx, pals = sym['gCardArtGfx'][0] - ROM_BASE, sym['gCardArtPalettes'][0] - ROM_BASE
    n2i = sym['gCardNumberToId'][0] - ROM_BASE
    count = 0
    for duelist in range(opponents[1] // OPPONENT_SIZE):
        portrait = struct.unpack_from('<H', rom, opponents[0] - ROM_BASE + duelist * OPPONENT_SIZE + 8)[0]
        if portrait in (0, 0xFFFF) or portrait & 0x8000:
            continue                        # no portrait, or an original page's portrait (Solomon)
        number = portrait if portrait < CARD_NUMBER_ALT_ART else portrait - CARD_NUMBER_ALT_ART
        card_id = struct.unpack_from('<H', rom, n2i + 2 * (number & 0x7FF))[0]
        if portrait >= CARD_NUMBER_ALT_ART:
            card_id += 1
        if card_id == 0:
            continue
        rom[faces + duelist * FACE_SIZE:faces + (duelist + 1) * FACE_SIZE] = face(rom, gfx, pals, card_id)
        count += 1
    open(rom_path, 'wb').write(rom)
    print(f'record_faces: {count} opponent faces')


if __name__ == '__main__':
    main()
