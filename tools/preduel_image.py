#!/usr/bin/env python3
"""A pre-duel screen picture (src/mod_preduel.c): a 240x96 PNG for the bustup portrait area, as game data.

  python3 tools/preduel_image.py yugi-kaiba.png build/assets/preduel_yugi_kaiba.bin [yugi-kaiba.svg]
  python3 tools/preduel_image.py --qr-only fiber.svg build/assets/preduel_qr/fiber.bin

The screen is the bustup screen's Mode-4 layout: the picture in rows 0-95 over the empty dialogue box (rows 96-159).
The box uses BG colours 16-31, so the picture has the other 224: colours 32-255. The PNG may be indexed or RGB(A)
with up to 224 colours. An indexed PNG whose pixels already use only indices 32-255 keeps them (and its palette
there); otherwise its colours are numbered 32, 33, ... in order of first use (index order for an indexed PNG).

The optional SVG is a QR code drawn in the dialogue box (src/mod_preduel.c): its dark modules as <rect> elements on a
grid (QR_ORIGIN + QR_MODULE * n, as the formatlibrary.com QR generator writes them) and its three finder patterns,
which are redrawn here (no <rect> may fall inside one). The format information is checked (BCH) so a misread grid
fails the build. At most QR_MAX_SIZE modules a side.

Output (4-byte aligned), laid out so Hex Maniac Advance can show and edit it (tools/hma_toml.py):
  +0x000  u16 colours[224]   BGR555, for BG palette entries 32-255 (unused ones 0); HMA `ucp4:0123456789ABCD`
  +0x1C0  u32 qrSize         modules per side, 0 = no QR code
  +0x1C4  u16 qrColours[16]  the QR sprite's palette: 0 white, 1 black (only for HMA; the game draws its own)
  +0x1E4  u8 qr[512]         the QR code as a 32x32 4bpp sprite (4x4 tiles, row-major; pixel (c, r) = module
                             (r, c), 1 = dark); HMA `ucs4x4x4`
  +0x3E4  u8 picture[23040]  the 240x96 picture as 8bpp tiles (30x12, row-major), each pixel its colour - 32 (0-223:
                             HMA indexes the palette from 0); HMA `ucs8x30x12`. src/mod_preduel.c turns it back into
                             the Mode 4 bitmap (colours 32-255).
--qr-only writes the QR code of an opponent with no picture (it shows its duelist's scene): the same layout up to
+0x3E4 (no picture after it; the colours all 0), so src/mod_preduel.c reads its QR code at the same offsets.
"""
import os
import re
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402  (png_read, rgb_to_bgr555)

WIDTH, HEIGHT = 240, 96
FIRST, COLORS = 32, 224
QR_ORIGIN, QR_MODULE = 173, 57      # the SVG's first module and module size (viewBox units)
QR_MAX_SIZE, QR_ROWS = 30, 32       # 30 modules at 2 px plus a 2 px border fill the box's 64 rows
QR_PALETTE = [0x7FFF, 0x0000] + [0] * 14


def tiles(pixels, width, height, bpp):
    """Row-major pixels as GBA tiles (row-major tiles), 4 or 8 bits per pixel."""
    out = bytearray()
    for ty in range(height // 8):
        for tx in range(width // 8):
            for y in range(8):
                row = pixels[(ty * 8 + y) * width + tx * 8:(ty * 8 + y) * width + tx * 8 + 8]
                if bpp == 8:
                    out += bytes(row)
                else:
                    out += bytes(row[i] | row[i + 1] << 4 for i in range(0, 8, 2))
    return bytes(out)


def read_qr(path):
    """The QR matrix of the SVG as a list of rows of 0/1."""
    s = open(path, encoding='utf-8').read()
    rects = [tuple(map(int, m)) for m in re.findall(
        r'<rect x="(\d+)" y="(\d+)" width="(\d+)" height="(\d+)" fill="#000000"', s)]
    if not rects:
        sys.exit(f'preduel_image: no dark <rect> modules in {path}')
    n = max(max(x, y) for x, y, _, _ in rects)
    n = (n - QR_ORIGIN) // QR_MODULE + 1
    if (n - 17) % 4 or not 21 <= n <= QR_MAX_SIZE:
        sys.exit(f'preduel_image: {path}: {n} modules a side is not a QR size that fits (21-{QR_MAX_SIZE})')
    g = [[0] * n for _ in range(n)]
    for x, y, w, h in rects:
        if (x - QR_ORIGIN) % QR_MODULE or (y - QR_ORIGIN) % QR_MODULE or (w, h) != (QR_MODULE, QR_MODULE):
            sys.exit(f'preduel_image: {path}: a <rect> off the {QR_MODULE}-unit grid at ({x}, {y})')
        g[(y - QR_ORIGIN) // QR_MODULE][(x - QR_ORIGIN) // QR_MODULE] = 1
    for r0, c0 in ((0, 0), (0, n - 7), (n - 7, 0)):
        for r in range(7):
            for c in range(7):
                if g[r0 + r][c0 + c]:
                    sys.exit(f'preduel_image: {path}: a module inside a finder pattern')
                g[r0 + r][c0 + c] = int(r in (0, 6) or c in (0, 6) or (2 <= r <= 4 and 2 <= c <= 4))
    bits = [g[8][c] for c in (0, 1, 2, 3, 4, 5, 7, 8)] + [g[r][8] for r in (7, 5, 4, 3, 2, 1, 0)]
    f = int(''.join(map(str, bits)), 2) ^ 0b101010000010010
    v = f >> 10 << 10
    for i in range(14, 9, -1):
        if v >> i & 1:
            v ^= 0b10100110111 << (i - 10)
    if (f >> 10 << 10 | v) != f:
        sys.exit(f'preduel_image: {path}: the QR format information does not check out (grid misread?)')
    return g


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
    if sys.argv[1:2] == ['--qr-only']:
        if len(sys.argv) != 4:
            sys.exit(__doc__)
        qr = read_qr(sys.argv[2])
        qr_pixels = [qr[r][c] if r < len(qr) and c < len(qr) else 0 for r in range(QR_ROWS) for c in range(QR_ROWS)]
        out = (bytes(COLORS * 2) + struct.pack('<I', len(qr)) + struct.pack('<16H', *QR_PALETTE)
               + tiles(qr_pixels, QR_ROWS, QR_ROWS, 4))
        os.makedirs(os.path.dirname(sys.argv[3]), exist_ok=True)
        if not os.path.exists(sys.argv[3]) or open(sys.argv[3], 'rb').read() != out:
            open(sys.argv[3], 'wb').write(out)
        return
    if len(sys.argv) not in (3, 4):
        sys.exit(__doc__)
    qr = read_qr(sys.argv[3]) if len(sys.argv) == 4 else []
    qr_pixels = [qr[r][c] if r < len(qr) and c < len(qr) else 0 for r in range(QR_ROWS) for c in range(QR_ROWS)]
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
    out = (struct.pack(f'<{COLORS}H', *colours) + struct.pack('<I', len(qr)) + struct.pack('<16H', *QR_PALETTE)
           + tiles(qr_pixels, QR_ROWS, QR_ROWS, 4) + tiles([p - FIRST for p in bitmap], WIDTH, HEIGHT, 8))
    out += b'\0' * (-len(out) % 4)
    try:
        if open(sys.argv[2], 'rb').read() == out:
            return
    except FileNotFoundError:
        pass
    open(sys.argv[2], 'wb').write(out)


if __name__ == '__main__':
    main()
