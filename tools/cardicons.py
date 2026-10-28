#!/usr/bin/env python3
"""Duel field card icons with a miniature of each card's art (for the planned per-card field icons).

  python3 tools/cardicons.py [OUT]          # OUT defaults to build/card_icons; needs a built tree (make)
  python3 tools/cardicons.py --data FILE    # the duel's data (the Makefile: build/assets/field_icons.bin)

For every card ID (original and added cards, alternate arts and tokens included) it takes the card type's 32x32
field icon, card_icons/<type>.png (effect_monster, normal_monster, ritual_monster, spell_card, trap_card; a missing
one, such as fusion_monster, comes from the game's own icon graphics, pose 0 of gCardIcon*Gfx with gCardIconPal),
and draws a 17x17 miniature of the card's art over it at x 7-23, y 7-23: one pixel left of the black square
(x 8-22, y 7-22), top aligned with it. The miniature is the centre 72x72 of the 72x80 art (build/assets/
newcards__art_gfx.bin and __art_palettes.bin, by card ID), area-averaged in linear light.
Every miniature uses one shared 64-colour palette (k-means over all the miniatures' pixels, colours on the GBA's
15-bit grid), meant for the four BG palettes 12-15 the duel leaves unused. Output, in OUT:
  NNNN.png       card ID NNNN: an indexed 32x32 PNG, palette entries 0-15 the icon's (gCardIconPal, 0 transparent)
                 and 16-79 the shared art palette
  palette.bin    the shared palette as 64 BGR555 colours (128 bytes: BG palettes 12-15)
  palette.png    the shared palette as an 8x8 swatch image
  sheet.png      every icon on one contact sheet (32 a row)
  index.tsv      card ID, card number, icon type, name
The art comes from the ROM, so OUT stays out of git (build/).

--data FILE writes what the duel draws the field cards with (src/mod_field_icons.c, through
data/mod_field_icons_data.s), for BG2 in 256-colour mode:
  +0x0000   the shared palette: 64 BGR555 colours, for BG palette entries 192-255 (palettes 12-15)
  +0x0080   the frames: 8bpp 4x4-tile icons, 1 KiB each, by type (normal, effect, fusion and ritual monster,
            spell, trap: the order of GetCardIconBgTile) then pose (upright, sideways: rotated clockwise, as
            the game's pose 3); the icon's colours are entries 32-47 (BG palette 2, gCardIconPal), 0 transparent
  +0x3080   the miniatures: 17x17 bytes per card ID (CARD_ID_COUNT of them), row-major, palette entries
            192-255; an ID without a card is all 0
"""
import os
import struct
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402  (png_read, png_write, bgr555_to_rgb)

ICON_DIR = 'card_icons'
GFX_BANK = 'build/assets/gfx__bank_a.bin'
ART_GFX = 'build/assets/newcards__art_gfx.bin'
ART_PAL = 'build/assets/newcards__art_palettes.bin'
STATS = 'build/assets/newcards__stats.bin'
NAMES = 'build/assets/newcards__names.bin'
ID_TO_NUMBER = 'build/assets/newcards__id_to_number.bin'

ICON = 32
MINI = 17
MINI_X, MINI_Y = 7, 7           # one pixel left of the black square (x 8-22, y 7-22), top aligned
COLORS = 64
ART_PALETTE_BASE = 16           # the art's indices in the output PNGs follow the icon's 16 colours
KMEANS_PASSES = 12
WEIGHTS = (3, 4, 2)             # colour distance weights (R, G, B)

# gfx__bank_a.bin offsets (data/rodata_08087FD0.s): gCardIconPal and the 32x32 icons (4 poses of 16 tiles each).
ICON_PAL_OFFSET = 0x416D4
ICON_GFX_OFFSET = {
    'normal_monster': 0x426F4,
    'effect_monster': 0x42EF4,
    'fusion_monster': 0x436F4,
    'ritual_monster': 0x43EF4,
    'trap_card': 0x446F4,
    'spell_card': 0x44EF4,
}
CARD_TYPE_TRAP, CARD_TYPE_MAGIC = 21, 22
KIND_ICON = {0: 'normal_monster', 1: 'effect_monster', 2: 'fusion_monster', 3: 'ritual_monster'}
NAME_SIZE = 0x40


def png_transparent0(png):
    """An indexed PNG from A.png_write with palette entry 0 transparent (a tRNS chunk after PLTE), as the
    card_icons PNGs have it."""
    i = png.index(b'IDAT') - 4
    body = b'tRNS\x00'
    return png[:i] + struct.pack('>I', 1) + body + struct.pack('>I', zlib.crc32(body) & 0xFFFFFFFF) + png[i:]


def icon_type(stats):
    """The icon a card's stats word picks, as GetCardIconGfx does: Magic, Trap, else by frame kind."""
    kind_type = (stats >> 20) & 0x1F
    if kind_type == CARD_TYPE_MAGIC:
        return 'spell_card'
    if kind_type == CARD_TYPE_TRAP:
        return 'trap_card'
    return KIND_ICON[(stats >> 18) & 3]


def rom_icon(bank, name):
    """Pose 0 of the game's icon `name` as 32x32 indices (4x4 tiles of 8x8, 4bpp)."""
    g = bank[ICON_GFX_OFFSET[name]:ICON_GFX_OFFSET[name] + 0x200]
    img = bytearray(ICON * ICON)
    for t in range(16):
        tx, ty = t % 4, t // 4
        for y in range(8):
            for x in range(8):
                b = g[t * 32 + y * 4 + x // 2]
                img[(ty * 8 + y) * ICON + tx * 8 + x] = (b >> 4) if x & 1 else (b & 15)
    return bytes(img)


def load_icons(bank):
    """{icon type: 32x32 indices}: card_icons/<type>.png where present, else the game's own icon."""
    icons = {}
    for name in ICON_GFX_OFFSET:
        path = f'{ICON_DIR}/{name}.png'
        if os.path.exists(path):
            w, h, px, _ = A.png_read(open(path, 'rb').read())
            if (w, h) != (ICON, ICON):
                sys.exit(f'cardicons: {path} must be {ICON}x{ICON}')
            icons[name] = bytes(px)
        else:
            icons[name] = rom_icon(bank, name)
    return icons


# sRGB <-> linear light, for the area averaging.
TO_LIN = [((c / 255) / 12.92 if c / 255 <= 0.04045 else ((c / 255 + 0.055) / 1.055) ** 2.4) for c in range(256)]


def to_srgb(v):
    v = 12.92 * v if v <= 0.0031308 else 1.055 * v ** (1 / 2.4) - 0.055
    return max(0, min(255, int(v * 255 + 0.5)))


def area_weights(src, dst):
    """For each output pixel, [(source index, weight)] covering its share of `src` pixels (weights sum to 1)."""
    scale = src / dst
    out = []
    for o in range(dst):
        a, b = o * scale, (o + 1) * scale
        taps = []
        i = int(a)
        while i < b:
            w = min(b, i + 1) - max(a, i)
            if w > 1e-9:
                taps.append((i, w / scale))
            i += 1
        out.append(taps)
    return out


WX = area_weights(A.ART_W, MINI)            # 72 -> 17
WY = area_weights(A.ART_W, MINI)            # the centre 72 rows -> 17
CROP_Y = (A.ART_H - A.ART_W) // 2           # rows 4-75 of 80


def miniature(px, pal):
    """The 17x17 RGB miniature (row-major list of (r, g, b)) of a 72x80 indexed art image."""
    lin = [(TO_LIN[r], TO_LIN[g], TO_LIN[b]) for r, g, b in pal]
    rows = []
    for y in range(A.ART_W):                # horizontal pass over the cropped rows
        base = (CROP_Y + y) * A.ART_W
        row = []
        for taps in WX:
            r = g = b = 0.0
            for i, w in taps:
                c = lin[px[base + i]]
                r += c[0] * w
                g += c[1] * w
                b += c[2] * w
            row.append((r, g, b))
        rows.append(row)
    out = []
    for taps in WY:                         # vertical pass
        for x in range(MINI):
            r = g = b = 0.0
            for i, w in taps:
                c = rows[i][x]
                r += c[0] * w
                g += c[1] * w
                b += c[2] * w
            out.append((to_srgb(r), to_srgb(g), to_srgb(b)))
    return out


def gba(c):
    """An 8-bit colour on the GBA's 5-bit grid, as the hardware shows it."""
    return tuple((v >> 3 << 3) | (v >> 5) for v in c)


def dist(a, b):
    return (WEIGHTS[0] * (a[0] - b[0]) ** 2 + WEIGHTS[1] * (a[1] - b[1]) ** 2
            + WEIGHTS[2] * (a[2] - b[2]) ** 2)


def median_cut(hist, n):
    """Initial palette: split the box with the largest weighted spread until there are n boxes."""
    boxes = [list(hist.items())]
    while len(boxes) < n:
        best, axis, spread = None, 0, -1
        for k, box in enumerate(boxes):
            if len(box) < 2:
                continue
            for ax in range(3):
                vals = [c[ax] for c, _ in box]
                s = (max(vals) - min(vals)) * WEIGHTS[ax] * sum(cnt for _, cnt in box) ** 0.5
                if s > spread:
                    best, axis, spread = k, ax, s
        if best is None:
            break
        box = sorted(boxes.pop(best), key=lambda e: e[0][axis])
        total, run = sum(cnt for _, cnt in box), 0
        for cut, (_, cnt) in enumerate(box):
            run += cnt
            if run * 2 >= total:
                break
        cut = max(1, min(len(box) - 1, cut + 1))
        boxes += [box[:cut], box[cut:]]
    return [mean(box) for box in boxes]


def mean(entries):
    total = sum(cnt for _, cnt in entries)
    return tuple(sum(c[ax] * cnt for c, cnt in entries) / total for ax in range(3))


def nearest(c, palette):
    best, bd = 0, None
    for k, p in enumerate(palette):
        d = dist(c, p)
        if bd is None or d < bd:
            best, bd = k, d
    return best


def kmeans(hist, palette):
    """Refine the palette over the colour histogram (Lloyd's algorithm), then snap it to the GBA grid."""
    items = list(hist.items())
    for _ in range(KMEANS_PASSES):
        groups = [[] for _ in palette]
        for c, cnt in items:
            groups[nearest(c, palette)].append((c, cnt))
        palette = [mean(g) if g else p for g, p in zip(groups, palette)]
    snapped = []
    for p in palette:
        c = gba(tuple(int(v + 0.5) for v in p))
        if c in snapped:                    # two colours on the same grid point: keep the slot, shift it
            c = gba(tuple(min(255, v + 8) for v in c))
        snapped.append(c)
    while len(snapped) < COLORS:
        snapped.append((0, 0, 0))
    return snapped


FRAME_TYPES = ['normal_monster', 'effect_monster', 'fusion_monster', 'ritual_monster', 'spell_card', 'trap_card']
ICON_PALETTE_ENTRY = 32         # BG palette 2 (gCardIconPal) as 8bpp entries 32-47
ART_PALETTE_ENTRY = 192         # BG palettes 12-15


def rotate_cw(img):
    """A 32x32 image turned clockwise, as the game's sideways pose (pose 3)."""
    out = bytearray(ICON * ICON)
    for y in range(ICON):
        for x in range(ICON):
            out[x * ICON + ICON - 1 - y] = img[y * ICON + x]
    return bytes(out)


def tiles8(img):
    """A 32x32 image of 4bpp palette indices as 16 8bpp tiles (4x4, row-major) in BG palette 2's entries."""
    out = bytearray()
    for t in range(16):
        tx, ty = t % 4, t // 4
        for y in range(8):
            for x in range(8):
                v = img[(ty * 8 + y) * ICON + tx * 8 + x]
                out.append(ICON_PALETTE_ENTRY + v if v else 0)
    return bytes(out)


def main():
    data_out = None
    if len(sys.argv) > 2 and sys.argv[1] == '--data':
        data_out = sys.argv[2]
    out = sys.argv[1] if len(sys.argv) > 1 and not data_out else 'build/card_icons'
    for path in (GFX_BANK, ART_GFX, ART_PAL, STATS, NAMES, ID_TO_NUMBER):
        if not os.path.exists(path):
            sys.exit(f'cardicons: {path} not found; run make first')
    bank = open(GFX_BANK, 'rb').read()
    icon_pal = [A.bgr555_to_rgb(v) for v in struct.unpack_from('<16H', bank, ICON_PAL_OFFSET)]
    icons = load_icons(bank)
    art, art_pal = open(ART_GFX, 'rb').read(), open(ART_PAL, 'rb').read()
    stats = open(STATS, 'rb').read()
    names = open(NAMES, 'rb').read()
    i2n = open(ID_TO_NUMBER, 'rb').read()
    count = len(stats) // 4

    cards = []                              # (id, number, icon type, name, miniature)
    hist = {}
    for cid in range(1, count):
        number = struct.unpack_from('<H', i2n, cid * 2)[0]
        if number == 0xFFFF:
            continue
        px = A._art_pixels(art[cid * A.ART_SIZE:(cid + 1) * A.ART_SIZE])
        pal = [A.bgr555_to_rgb(v & 0x7FFF) for v in struct.unpack_from('<64H', art_pal, cid * 0x80)]
        mini = miniature(px, pal)
        for c in mini:
            g = gba(c)
            hist[g] = hist.get(g, 0) + 1
        name = names[cid * NAME_SIZE:(cid + 1) * NAME_SIZE].split(b'\0')[0].decode('latin-1')
        cards.append((cid, number, icon_type(struct.unpack_from('<I', stats, cid * 4)[0]), name, mini))
    palette = kmeans(hist, median_cut(hist, COLORS))

    if data_out:
        blob = bytearray(struct.pack('<64H', *((c[0] >> 3) | (c[1] >> 3) << 5 | (c[2] >> 3) << 10 for c in palette)))
        for kind in FRAME_TYPES:
            blob += tiles8(icons[kind]) + tiles8(rotate_cw(icons[kind]))
        minis = bytearray(count * MINI * MINI)
        cache = {}
        for cid, _, _, _, mini in cards:
            for k, c in enumerate(mini):
                g = gba(c)
                if g not in cache:
                    cache[g] = nearest(g, palette)
                minis[cid * MINI * MINI + k] = ART_PALETTE_ENTRY + cache[g]
        blob += minis
        os.makedirs(os.path.dirname(data_out) or '.', exist_ok=True)
        open(data_out, 'wb').write(bytes(blob))
        print(f'cardicons: {len(cards)} miniatures, {len(hist)} source colours -> {COLORS}, in {data_out}')
        return

    os.makedirs(out, exist_ok=True)
    full_pal = icon_pal + palette
    cache = {}
    sheet_cols = 32
    sheet_rows = (len(cards) + sheet_cols - 1) // sheet_cols
    sheet = bytearray(sheet_cols * ICON * sheet_rows * ICON)
    index = ['id\tnumber\ticon\tname']
    for k, (cid, number, kind, name, mini) in enumerate(cards):
        img = bytearray(icons[kind])
        for y in range(MINI):
            for x in range(MINI):
                c = gba(mini[y * MINI + x])
                if c not in cache:
                    cache[c] = nearest(c, palette)
                img[(MINI_Y + y) * ICON + MINI_X + x] = ART_PALETTE_BASE + cache[c]
        open(f'{out}/{cid:04d}.png', 'wb').write(png_transparent0(A.png_write(ICON, ICON, bytes(img), full_pal)))
        sx, sy = (k % sheet_cols) * ICON, (k // sheet_cols) * ICON
        for y in range(ICON):
            row = (sy + y) * sheet_cols * ICON + sx
            sheet[row:row + ICON] = img[y * ICON:(y + 1) * ICON]
        index.append(f'{cid}\t{number}\t{kind}\t{name}')
    open(f'{out}/sheet.png', 'wb').write(png_transparent0(
        A.png_write(sheet_cols * ICON, sheet_rows * ICON, bytes(sheet), full_pal)))
    swatch = bytes(y // 4 * 8 + x // 4 for y in range(32) for x in range(32))
    open(f'{out}/palette.png', 'wb').write(A.png_write(32, 32, swatch, palette))
    open(f'{out}/palette.bin', 'wb').write(struct.pack('<64H', *(
        (c[0] >> 3) | (c[1] >> 3) << 5 | (c[2] >> 3) << 10 for c in palette)))
    open(f'{out}/index.tsv', 'w', encoding='utf-8', newline='\n').write('\n'.join(index) + '\n')
    print(f'cardicons: {len(cards)} icons, {len(hist)} source colours -> {COLORS}, in {out}/')


if __name__ == '__main__':
    main()
