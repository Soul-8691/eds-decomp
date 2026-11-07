#!/usr/bin/env python3
"""The original duelists' opponent-select portraits as images of their own (src/mod_opponents.c DrawSlotPortrait).

  python3 tools/duelist_portraits.py build/assets/duelist_portraits.bin

The original opponent select has no portrait images: each page is one 240x160 Mode 4 picture
(gOpponentSelectPageBgs, assets/gfx/mode4_bitmaps from `make setup`) with the duelists' round portraits drawn in,
centred at their slot's position + (31, 33) (gOpponentSelectSlotPos; a circle of radius 23.5). Hex Maniac Advance
cannot show those linear bitmaps, so this cuts each portrait out: a 56x56 box around the centre, as 8bpp tiles
(7 x 7, row-major), with its page's 256 colours, which HMA shows and edits as `ucs8x7x7` with a `ucp4` palette
(tools/hma_toml.py). The game draws every portrait slot from these copies (DrawSlotPortrait), so an edit shows.

Output: 25 entries, one per original page and slot (index page * 5 + slot; page 4's slot 0 is empty but kept so
the index stays simple), each 4-byte aligned and PORTRAIT_ENTRY bytes: u16 colours[256] (BGR555), then the tiles.
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402  (png_read, rgb_to_bgr555)

PAGES = ['0871CE50', '08726650', '0872FE50', '08739650', '08742E50']   # gOpponentSelectPageBgs[0..4].bitmap
SLOTS = [(89, 91), (36, 58), (1, 5), (177, 5), (142, 58)]             # gOpponentSelectSlotPos (0x081983AC)
CENTRE = (31, 33)                                                     # PORTRAIT_DX, PORTRAIT_DY (mod_opponents.c)
BOX = 56                                                              # 7 x 7 tiles around the centre
HALF = 28
PORTRAIT_ENTRY = 256 * 2 + BOX * BOX


def main():
    out = bytearray()
    for page in PAGES:
        w, h, px, pal = A.png_read(open(f'assets/gfx/mode4_bitmaps/{page}.png', 'rb').read())
        if (w, h) != (240, 160):
            sys.exit(f'duelist_portraits: {page}.png is {w}x{h}, not 240x160')
        colours = [A.rgb_to_bgr555(c) for c in (list(pal) + [(0, 0, 0)] * 256)[:256]]
        for sx, sy in SLOTS:
            x0, y0 = sx + CENTRE[0] - HALF, sy + CENTRE[1] - HALF
            tiles = bytearray()
            for ty in range(BOX // 8):
                for tx in range(BOX // 8):
                    for y in range(8):
                        for x in range(8):
                            X, Y = x0 + tx * 8 + x, y0 + ty * 8 + y
                            tiles.append(px[Y * w + X] if 0 <= X < w and 0 <= Y < h else 0)
            out += struct.pack('<256H', *colours) + tiles
    os.makedirs(os.path.dirname(sys.argv[1]), exist_ok=True)
    if not os.path.exists(sys.argv[1]) or open(sys.argv[1], 'rb').read() != bytes(out):
        open(sys.argv[1], 'wb').write(out)


if __name__ == '__main__':
    main()
