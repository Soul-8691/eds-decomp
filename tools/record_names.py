#!/usr/bin/env python3
"""The Records screen's name plates for this build's pages (src/mod_opponents.c ModRecordDrawPage).

  python3 tools/record_names.py build/assets/record_names.bin

The original Records screen draws each page's column of name plates from one image pack
(gRecordPageNameImages[page], bank B): black plates with a blue edge and the duelists' names in white, in a 5-pixel
font. This build's pages are the format opponents in unlock order (5 a page) and a page of the deck opponents
(Solomon, Debug, Empty Deck, Test Duel), so their names are drawn here: the plate of the first original page with its name
removed, and each name in the original font, whose letters are cut from the five original images
(assets/gfx/bank_b, from `make setup`). F and - are not in any original name and are drawn here in the same style.
The names come from src/mod_opponents.c (the FORMAT(order, ..., sName...) rows and the deck opponents' names).

Output: u32 offsets[4] (from the start of the file), then 4 image packs in the game's format (LoadBgImage4bppToMap,
tools/assetfmt/gfx_banks.py b_pack: 4bpp, the first page's 5 colours), each 4-byte aligned.
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402  (png_read, rgb_to_bgr555)

BANK = 'assets/gfx/bank_b'
ORIGINAL = [('087E6610', ['YUGI', 'TEA', 'JOEY', 'TRISTAN', 'BAKURA']),
            ('087E6974', ['REX', 'ROBA', 'WEEVIL', 'MAKO', 'MAI']),
            ('087E6C38', ['RARE HUNTER', 'ARKANA', 'STRINGS', 'UMBRA&LUMIS', 'MARIK']),
            ('087E7078', ['SETO', 'ISHIZU', 'SHADI', 'YAMI BAKURA', 'YAMI YUGI']),
            ('087E743C', ['COMPUTER', 'SIMON', 'PEGASUS', 'TRUSDALE'])]
PLATE_PACK = '087E6610'             # the first page: its plate, palette and layout
PLATE_COLORS, PLATE_BIT15 = 5, [4]  # its pack's colour count and bit-15 colours (assets/gfx/bank_b/index.json)
ROW_STRIDE = 24                     # a plate every 24 pixels (3 map rows)
TEXT_TOP = 2                        # the name's 5 rows: plate rows 2-6
TEXT_X = 8                          # left edge, as the originals; a long name moves left until it fits
EXTRA_GLYPHS = {
    'F': ('###', '#..', '###', '#..', '#..'),
    '-': ('..', '..', '##', '..', '..'),
}
DECK_PAGE = ['Solomon', 'Debug', 'Empty Deck', 'Test Duel']   # ModRecordDuelist's page 3 order (src/mod_opponents.c)


def white_index(pal, px=None, w=0):
    """The text colour: the white entry the first plate's name rows use (two pages have two white entries)."""
    whites = [i for i, c in enumerate(pal) if i > 1 and c == (255, 255, 255)]
    if px is None:
        return whites[0]
    used = [px[y * w + x] for y in range(TEXT_TOP, TEXT_TOP + 5) for x in range(w)]
    return max(whites, key=used.count)


def load_glyphs():
    """{letter: rows of 0/1}, cut from the original name images (the first version of each letter)."""
    glyphs = {}
    for addr, names in ORIGINAL:
        w, h, px, pal = A.png_read(open(f'{BANK}/{addr}.png', 'rb').read())
        text = white_index(pal, px, w)
        for k, name in enumerate(names):
            y0 = k * ROW_STRIDE + TEXT_TOP
            cols = [x for x in range(w) if any(px[(y0 + y) * w + x] == text for y in range(5))]
            runs, cur = [], []
            for x in cols:
                if cur and x != cur[-1] + 1:
                    runs.append(cur)
                    cur = []
                cur.append(x)
            runs.append(cur)
            letters = name.replace(' ', '')
            if len(runs) != len(letters):
                sys.exit(f'record_names: {addr} "{name}": {len(runs)} letters found, {len(letters)} expected')
            for ch, run in zip(letters, runs):
                glyphs.setdefault(ch, tuple(tuple(int(px[(y0 + y) * w + x] == text) for x in run)
                                            for y in range(5)))
    for ch, rows in EXTRA_GLYPHS.items():
        glyphs.setdefault(ch, tuple(tuple(int(c == '#') for c in r) for r in rows))
    return glyphs


def page_names():
    """The names of the 4 pages: the format opponents in unlock order, 5 a page, then the deck opponents."""
    src = open('src/mod_opponents.c', encoding='utf-8').read()
    strings = dict(re.findall(r'static const char (sName\w+)\[\] = "([^"]*)";', src))
    order = sorted((int(o), strings[s]) for o, s in re.findall(r'FORMAT\((\d+), \w+, (sName\w+),', src))
    formats = [name for _, name in order]
    return [formats[0:5], formats[5:10], formats[10:15], DECK_PAGE]


def render(name, glyphs, w, plate):
    """The plate (w x 8 pixels, name removed) with `name` drawn in white."""
    text_index = 3
    img = list(plate)
    chars = name.upper()

    def layout(tight):
        """[(x offset, glyph)] and the width; tight: no gap around a dash (for a name too long otherwise)."""
        out, x = [], 0
        for i, c in enumerate(chars):
            if c == ' ':
                x += 2
                continue
            if tight and i and (c == '-' or chars[i - 1] == '-'):
                x -= 1
            out.append((x, glyphs[c]))
            x += len(glyphs[c][0]) + 1
        return out, x - 1

    def fits(placed, x0):
        """Every letter pixel on the plate's black inside (colour 2), so none touches its blue edge."""
        return all(plate[(TEXT_TOP + y) * w + x0 + dx0 + dx] == 2
                   for dx0, g in placed for y, row in enumerate(g) for dx, on in enumerate(row) if on
                   and 0 <= x0 + dx0 + dx < w)

    for tight in (False, True):
        placed, width = layout(tight)
        x0 = next((x for x in range(TEXT_X, 0, -1) if x + width <= w and fits(placed, x)), None)
        if x0 is not None:
            break
    else:
        sys.exit(f'record_names: "{name}" is {width} pixels wide; it does not fit on a name plate')
    for dx0, g in placed:
        for y, row in enumerate(g):
            for dx, on in enumerate(row):
                if on:
                    img[(TEXT_TOP + y) * w + x0 + dx0 + dx] = text_index
    return img


def pack(px, w, h, pal):
    """An image pack (gfx_banks.b_pack): blank 8x8 blocks have no cell, identical blocks share a tile."""
    tiles, index, cells = [], {}, []
    for ty in range(h // 8):
        for tx in range(w // 8):
            blk = tuple(px[(ty * 8 + y) * w + tx * 8 + x] for y in range(8) for x in range(8))
            if not any(blk):
                continue
            if blk not in index:
                index[blk] = len(tiles)
                tiles.append(blk)
            cells.append((tx | ty << 8, index[blk]))
    cols = [A.rgb_to_bgr555(c) for c in pal[:PLATE_COLORS]]
    for i in PLATE_BIT15:
        cols[i] |= 0x8000
    out = struct.pack('<4H', *[PLATE_COLORS] * 4) + struct.pack(f'<{PLATE_COLORS}H', *cols)
    out += struct.pack('<4H', *[len(tiles)] * 4)
    out += b''.join(bytes(t[2 * i] | t[2 * i + 1] << 4 for i in range(32)) for t in tiles)
    out += struct.pack('<4H', *[len(cells)] * 4) + b''.join(struct.pack('<HH', *c) for c in cells)
    return out + bytes(-len(out) % 4)


def main():
    out_path = sys.argv[1]
    glyphs = load_glyphs()
    w, h, px, pal = A.png_read(open(f'{BANK}/{PLATE_PACK}.png', 'rb').read())
    text = white_index(pal, px, w)
    plate = [2 if p == text else p for p in px[:w * 8]]    # the first plate, its name painted over in black
    packs = []
    for names in page_names():
        missing = sorted({c for n in names for c in n.upper() if c != ' ' and c not in glyphs})
        if missing:
            sys.exit(f'record_names: no letter {"".join(missing)} in the original name images')
        height = (len(names) - 1) * ROW_STRIDE + 8
        img = [0] * (w * height)
        for k, name in enumerate(names):
            img[k * ROW_STRIDE * w:(k * ROW_STRIDE + 8) * w] = render(name, glyphs, w, plate)
        packs.append(pack(img, w, height, pal))
    offset = 16
    head, body = b'', b''
    for p in packs:
        head += struct.pack('<I', offset + len(body))
        body += p
    data = head + body
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    if not os.path.exists(out_path) or open(out_path, 'rb').read() != data:
        open(out_path, 'wb').write(data)


if __name__ == '__main__':
    main()
