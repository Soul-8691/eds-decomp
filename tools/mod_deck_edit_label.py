#!/usr/bin/env python3
"""Build the Deck Edit command label "Empty Main/Side" (gModDeckEditEmptyMainSideLabelGfx, command 6 in Deck Edit).

  python3 tools/mod_deck_edit_label.py [out.bin]

The command labels (0x086EF3B0: 0x400-byte blocks, two rows of 16 4bpp tiles = 128 x 16 pixels each, palette
0x086F1C60) are pre-drawn pills with anti-aliased, shaded lettering. This one is put together from the original
labels so it matches them: the pill from the widest label's ends ("Select A Swapping Card"), narrowed around the
text; "Main" and "Side" whole from "To Main Deck" and "To Side Deck"; E, t from "Exit"; p from "Swapping"; m from
the n of "Main" (its arch twice); y from the u of "Trunk" with the tail of the g of "Swapping"; and the / drawn in
the letters' stem colours (no label has one).
"""
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
GFX = REPO / 'build/assets/gfx__bank_a.bin'
OUT = REPO / 'build/assets/mod_deck_edit_empty_label.bin'
LABELS = 0xB9128                # gfx bank A offset of the labels (ROM 0x086EF3B0)
FILL = 1                        # the pill's inside
W, H = 128, 16
TEXT_ROWS = range(3, 14)        # the rows the lettering uses (inside the pill's border)

TRUNK, MAIN, SIDE, EXIT, SWAP = 1, 2, 3, 6, 7       # label numbers (To Trunk, To Main Deck, ...)


def load():
    data = GFX.read_bytes()
    out = []
    for i in range(9):
        b = data[LABELS + i * 0x400:LABELS + (i + 1) * 0x400]
        out.append([[pixel(b, x, y) for x in range(W)] for y in range(H)])
    return out


def pixel(b, x, y):
    o = ((y // 8) * 16 + x // 8) * 32 + (y % 8) * 4 + (x % 8) // 2
    return (b[o] >> 4) & 15 if x & 1 else b[o] & 15


def encode(px):
    b = bytearray(0x400)
    for y in range(H):
        for x in range(W):
            o = ((y // 8) * 16 + x // 8) * 32 + (y % 8) * 4 + (x % 8) // 2
            b[o] |= px[y][x] << (4 if x & 1 else 0)
    return bytes(b)


def glyphs(L):
    """Each glyph: a list of columns, each a {row: pixel} of its lettering (fill left out)."""
    def cols(label, xs, rows=TEXT_ROWS):
        return [{y: L[label][y][x] for y in rows if L[label][y][x] != FILL} for x in xs]

    def merge(a, b):
        return [{**p, **q} for p, q in zip(a, b)]

    g = {}
    g['Main'] = cols(MAIN, range(47, 71))
    g['Side'] = cols(SIDE, range(47, 70))
    g['E'] = cols(EXIT, range(54, 61))
    g['t'] = merge(cols(EXIT, [69], range(6, 8)), [{}]) + cols(EXIT, range(70, 75))
    g['p'] = cols(SWAP, range(75, 82))
    g['m'] = cols(MAIN, [65, 66, 67, 68, 69, 66, 67, 68, 69, 70])
    # y: Trunk's u above row 12, the g's bottom (its bowl's end and the tail) below, the g's stem over the u's.
    g['y'] = merge(cols(TRUNK, range(68, 75), range(3, 12)), cols(SWAP, range(90, 97), range(12, 14)))
    # /: a stem in the letters' colours (5 light edge, 2 core, A shadow), 4 columns over its 9 rows.
    slash = [dict() for _ in range(9)]
    for k, y in enumerate(range(4, 13)):
        x = 4 - (k * 4 + 4) // 8
        for dx, c in ((0, 5), (1, 2), (2, 0xA)):
            if 0 <= x + dx < 9:
                slash[x + dx].setdefault(y, c)
    g['/'] = slash
    for k in ('E', 'm', 'p', 't', 'y', '/'):     # set tight: no blank columns of their own at either side
        while not g[k][0]:
            g[k] = g[k][1:]
        while not g[k][-1]:
            g[k] = g[k][:-1]
    g[' '] = [{}] * 4
    return g


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    out = Path(args[0]) if args else OUT
    if not GFX.is_file():
        sys.exit(f'mod_deck_edit_label: missing {GFX} (run make setup)')
    L = load()
    g = glyphs(L)
    text = ['E', 'm', 'p', 't', 'y', ' ', 'Main', '/', 'Side']
    gap = {}
    lettering = []
    for k, part in enumerate(text):
        if k:
            lettering += [{}] * gap.get((text[k - 1], part), 0)
        lettering += g[part]
    tw = len(lettering)

    # The pill: Select A Swapping Card's (columns 2-126) narrowed to the text, its notch kept at the middle (62-65).
    swap = L[SWAP]
    left = [[swap[y][x] for y in range(H)] for x in range(2, 6)]       # the left end and its border
    right = [[swap[y][x] for y in range(H)] for x in range(123, 127)]
    left[-1] = [FILL if 4 <= y <= 12 else c for y, c in enumerate(left[-1])]     # (no lettering of the original)
    right[0] = [FILL if 4 <= y <= 12 else c for y, c in enumerate(right[0])]
    mid = [swap[y][30] for y in range(H)]
    mid = [FILL if y in TEXT_ROWS else c for y, c in enumerate(mid)]
    notch = {x: [swap[y][x] if y < 3 else mid[y] for y in range(H)] for x in range(62, 66)}
    inner = tw + 4                                                    # 2 fill columns each side of the text
    x0 = 64 - (inner + len(left) + len(right)) // 2
    px = [[0] * W for _ in range(H)]
    columns = left + [mid] * inner + right
    for i, c in enumerate(columns):
        x = x0 + i
        c = notch.get(x, c)
        for y in range(H):
            px[y][x] = c[y]
    tx = x0 + len(left) + 2
    for i, c in enumerate(lettering):
        for y, v in c.items():
            px[y][tx + i] = v
    if '--show' in sys.argv:
        for row in px:
            print(''.join('.123456789ABCDEF'[v] for v in row))
    out.parent.mkdir(parents=True, exist_ok=True)
    data = encode(px)
    if not out.is_file() or out.read_bytes() != data:
        out.write_bytes(data)


if __name__ == '__main__':
    main()
