#!/usr/bin/env python3
"""The card command menu's Mulligan icon and label (CARDMENU_CMD_MULLIGAN, src/mod_mulligan.c).

  python3 tools/mulligan_menu_gfx.py [out.bin] [--show]

The menu's sheets (gCardMenuIconsGfx: 16 x 16 icons, 4 tiles each; gCardMenuLabelsGfx: 32 x 16 labels, 8 tiles
each; OBJ 1D mapping) hold 16 slots, of which the original uses 13; slots 13-15 hold a blank icon frame and a blank
label pill. This draws the Mulligan command on slot 13's blanks, in the sheets' own palettes (OBJ palettes 2 and 3):
  icon   the orange frame of the position icons (Def Pos, Atk Pos) around a card back inside a green circular
         arrow (shuffle and draw again);
  label  "Mulligan" in the labels' style: white letters with a dark shadow to the lower right, on the blank pill.
Output: the icon's 0x80 bytes, then the label's 0x100 (gModMulliganMenuGfx, data/mod_mulligan_data.s), which
ModLoadMulliganMenuGfx copies over slot 13 after LoadDuelUiGfx loads the sheets.
"""
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
GFX = REPO / 'build/assets/gfx__bank_a.bin'
OUT = REPO / 'build/assets/mulligan_menu_gfx.bin'
ICONS = 0x499D4                 # gCardMenuIconsGfx in gfx bank A (ROM 0x0867FC5C)
LABELS = 0x4A1F4                # gCardMenuLabelsGfx (ROM 0x0868047C)
SLOT = 13                       # CARDMENU_CMD_MULLIGAN

# The icon (palette gCardMenuIconsPal): . transparent; frame 5 (dark brown), A (orange), D (yellow) as Def Pos;
# E white inside; a card back in the middle (5 outline, F brown, D marks) inside a clockwise circular arrow
# (2 green, 3 dark green edge) whose head, at the bottom, points back to its start.
ICON = """
..555555555555..
.5AAAAAAAAAAAA5.
5ADDDDDDDDDDDDA5
5ADEEEE33EEEEDA5
5ADEE322223EEDA5
5ADE32EEEE23EDA5
5ADE2E5555E2EDA5
5AD32E5FD5E23DA5
5ADEEE5DF5E23DA5
5ADEEE5555E2EDA5
5ADEEEE3EE23EDA5
5ADEEE32223EEDA5
5ADEEEE33EEEEDA5
5ADDDDDDDDDDDDA5
.5AAAAAAAAAAAA5.
..555555555555..
"""

# "Mulligan": 1 = a letter pixel. Capitals from row 5, lowercase from row 7, g's tail to row 13.
LETTERS = {
    'M': ['1...1', '11.11', '1.1.1', '1.1.1', '1...1', '1...1', '1...1'],
    'u': ['', '', '1.1', '1.1', '1.1', '1.1', '.11'],
    'l': ['1', '1', '1', '1', '1', '1', '1'],
    'i': ['1', '', '1', '1', '1', '1', '1'],
    'g': ['', '', '.11', '1.1', '1.1', '.11', '..1', '11.'],
    'a': ['', '', '11.', '..1', '.11', '1.1', '.11'],
    'n': ['', '', '11.', '1.1', '1.1', '1.1', '1.1'],
}
TEXT, SHADOW, TOP = 1, 0xB, 5   # label palette: 1 white, B the dark shadow; the letters' top row


def read(data, base, w, h):
    tw = w // 8
    px = [[0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            o = base + ((y // 8) * tw + x // 8) * 32 + (y % 8) * 4 + (x % 8) // 2
            px[y][x] = (data[o] >> 4) & 15 if x & 1 else data[o] & 15
    return px


def write(px):
    h, w = len(px), len(px[0])
    tw = w // 8
    b = bytearray(w * h // 2)
    for y in range(h):
        for x in range(w):
            o = ((y // 8) * tw + x // 8) * 32 + (y % 8) * 4 + (x % 8) // 2
            b[o] |= px[y][x] << (4 if x & 1 else 0)
    return bytes(b)


def label(blank):
    px = [row[:] for row in blank]
    fill = px[8][15]                                            # the pill's inside
    word = 'Mulligan'
    width = sum(len(LETTERS[c][0] or LETTERS[c][-1]) for c in word) + len(word) - 1
    x = (31 - width) // 2                                       # the pill spans columns 0-30
    on = set()
    for c in word:
        rows = LETTERS[c]
        w = max(len(r) for r in rows)
        for dy, r in enumerate(rows):
            for dx, v in enumerate(r):
                if v == '1':
                    on.add((x + dx, TOP + dy))
        x += w + 1
    for (x, y) in on:
        for sx, sy in ((x + 1, y), (x, y + 1), (x + 1, y + 1)):
            if (sx, sy) not in on and px[sy][sx] == fill:
                px[sy][sx] = SHADOW
    for (x, y) in on:
        px[y][x] = TEXT
    return px


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    out = Path(args[0]) if args else OUT
    if not GFX.is_file():
        sys.exit(f'mulligan_menu_gfx: missing {GFX} (run make setup)')
    data = GFX.read_bytes()
    icon = [[int(c, 16) if c != '.' else 0 for c in row] for row in ICON.split()]
    assert len(icon) == 16 and all(len(r) == 16 for r in icon)
    lab = label(read(data, LABELS + SLOT * 0x100, 32, 16))
    if '--show' in sys.argv:
        for row in icon:
            print(''.join('.123456789ABCDEF'[v] for v in row))
        for row in lab:
            print(''.join('.123456789ABCDEF'[v] for v in row))
    blob = write(icon) + write(lab)
    out.parent.mkdir(parents=True, exist_ok=True)
    if not out.is_file() or out.read_bytes() != blob:
        out.write_bytes(blob)


if __name__ == '__main__':
    main()
