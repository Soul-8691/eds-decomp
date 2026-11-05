#!/usr/bin/env python3
"""A pre-duel screen picture (src/mod_preduel.c): a 240x96 PNG for the bustup portrait area, as game data.

  python3 tools/preduel_image.py yugi-kaiba.png build/assets/preduel_yugi_kaiba.bin

The screen is the bustup screen's Mode-4 layout: the picture in rows 0-95 over the empty dialogue box (rows 96-159).
The box uses BG colours 16-31, so the picture has the other 224: colours 32-255. The PNG may be indexed or RGB(A)
with up to 224 colours. An indexed PNG whose pixels already use only indices 32-255 keeps them (and its palette
there); otherwise its colours are numbered 32, 33, ... in order of first use (index order for an indexed PNG).

Output (4-byte aligned):
  +0x000  u16 colours[224]   BGR555, for BG palette entries 32-255 (unused ones 0)
  +0x1C0  u32 size, LZSS stream   the 240x96 bitmap (one byte per pixel, values 32-255), the game's LZSS format
                                  (LZSSDecompress; tools/assetfmt/gfx_scenes.py lz_encode)
"""
import os
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402  (png_read, rgb_to_bgr555)
from assetfmt.gfx_scenes import lz_encode  # noqa: E402

WIDTH, HEIGHT = 240, 96
FIRST, COLORS = 32, 224


def read_png(path):
    """(pixel indices, palette) or (None, RGB pixels) for a non-indexed PNG."""
    data = open(path, 'rb').read()
    try:
        w, h, px, pal = A.png_read(data)
        if (w, h) != (WIDTH, HEIGHT):
            sys.exit(f'preduel_image: {path} is {w}x{h}, not {WIDTH}x{HEIGHT}')
        return px, pal
    except ValueError:
        pass
    # RGB / RGBA, 8 bits per channel, not interlaced
    pos, idat = 8, b''
    while pos < len(data):
        n, t = struct.unpack('>I4s', data[pos:pos + 8])
        d = data[pos + 8:pos + 8 + n]
        if t == b'IHDR':
            w, h, depth, ctype, _, _, interlace = struct.unpack('>IIBBBBB', d)
        elif t == b'IDAT':
            idat += d
        pos += 12 + n
    if (w, h) != (WIDTH, HEIGHT) or depth != 8 or ctype not in (2, 6) or interlace:
        sys.exit(f'preduel_image: {path} must be a {WIDTH}x{HEIGHT} indexed, RGB or RGBA PNG (8 bits, not interlaced)')
    bpp = 3 if ctype == 2 else 4
    raw, stride = zlib.decompress(idat), w * bpp
    rows, prev, i = [], bytearray(stride), 0
    for _ in range(h):
        f, line = raw[i], bytearray(raw[i + 1:i + 1 + stride])
        i += 1 + stride
        for x in range(stride):
            a = line[x - bpp] if x >= bpp else 0
            b, c = prev[x], prev[x - bpp] if x >= bpp else 0
            if f == 1:
                line[x] = (line[x] + a) & 255
            elif f == 2:
                line[x] = (line[x] + b) & 255
            elif f == 3:
                line[x] = (line[x] + (a + b) // 2) & 255
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        prev = line
        rows += [tuple(line[x:x + 3]) for x in range(0, stride, bpp)]
    return None, rows


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    px, pal = read_png(sys.argv[1])
    colours = [0] * COLORS
    if px is not None and min(px) >= FIRST:
        bitmap = bytes(px)                              # already in 32-255
        for i in set(px):
            colours[i - FIRST] = A.rgb_to_bgr555(pal[i])
    else:
        if px is not None:
            keys = sorted(set(px))
            source = px
            rgb = {i: pal[i] for i in keys}
        else:
            keys = list(dict.fromkeys(pal))             # RGB pixels, in order of first use
            source = pal
            rgb = {c: c for c in keys}
        if len(keys) > COLORS:
            sys.exit(f'preduel_image: {sys.argv[1]} has {len(keys)} colours; at most {COLORS} fit (BG colours 32-255)')
        slot = {k: FIRST + n for n, k in enumerate(keys)}
        bitmap = bytes(slot[k] for k in source)
        for k, n in slot.items():
            colours[n - FIRST] = A.rgb_to_bgr555(rgb[k])
    stream = lz_encode(bitmap)
    out = struct.pack(f'<{COLORS}H', *colours) + struct.pack('<I', len(stream)) + stream
    out += b'\0' * (-len(out) % 4)
    try:
        if open(sys.argv[2], 'rb').read() == out:
            return
    except FileNotFoundError:
        pass
    open(sys.argv[2], 'wb').write(out)


if __name__ == '__main__':
    main()
