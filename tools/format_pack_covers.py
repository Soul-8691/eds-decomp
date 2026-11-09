#!/usr/bin/env python3
"""Pre-bake format-pack shop covers (56x112 8bpp tiles, gPackListPal indices).

  python3 tools/format_pack_covers.py build/assets/format_pack_covers.bin

Same crop and palette mapping as ModFormatPackLoadCoverGfx / BlitArtCover (src/mod_format_packs.c).
Portrait card numbers come from build/include/format_pack_data.h (tools/format_packs.py).
"""
import os
import re
import struct
import sys

PACK_COVER_W = 56
PACK_COVER_H = 112
PACK_COVER_TILES = 98
PACK_COVER_BYTES = PACK_COVER_TILES * 64
ART_SIZE = 0x10E0
ART_PALETTE_SIZE = 0x80
BANK_A_BASE = 0x08636288
PACK_LIST_PAL_ROM = 0x0863CC7C

HEADER = 'build/include/format_pack_data.h'
ART_GFX = 'build/assets/newcards__art_gfx.bin'
ART_PAL = 'build/assets/newcards__art_palettes.bin'
NUMBER_TO_ID = 'build/assets/newcards__number_to_id.bin'
GFX_BANK_A = 'build/assets/gfx__bank_a.bin'


def colour_dist(a, b):
    return sum(((a >> s & 31) - (b >> s & 31)) ** 2 for s in (0, 5, 10))


def load_pack_list_pal():
    off = PACK_LIST_PAL_ROM - BANK_A_BASE
    data = open(GFX_BANK_A, 'rb').read()
    return struct.unpack_from('<256H', data, off)


def nearest_pal_index(rgb, shop_pal):
    best_d = None
    best_i = 1
    for i in range(1, 256):
        d = colour_dist(rgb, shop_pal[i])
        if best_d is None or d < best_d:
            best_d, best_i = d, i
    return best_i


def art_pixel(gfx, card_id, x, y):
    base = card_id * ART_SIZE
    i = ((y >> 3) * 9 + (x >> 3)) * 64 + (y & 7) * 8 + (x & 7)
    bit = i * 6
    return ((gfx[base + (bit >> 3)] | gfx[base + (bit >> 3) + 1] << 8) >> (bit & 7)) & 63


def blit_art_cover(card_id, gfx, pals, shop_pal):
    art_pal = struct.unpack_from('<64H', pals, card_id * ART_PALETTE_SIZE)
    pal_map = [nearest_pal_index(art_pal[x], shop_pal) for x in range(64)]
    tiles = bytearray(PACK_COVER_BYTES)
    for y in range(PACK_COVER_H):
        for x in range(PACK_COVER_W):
            sx = 16 + x * 40 // PACK_COVER_W
            sy = y * 80 // PACK_COVER_H
            if sx > 71:
                sx = 71
            if sy > 79:
                sy = 79
            idx = pal_map[art_pixel(gfx, card_id, sx, sy)]
            tile_idx = (y >> 3) * 7 + (x >> 3)
            tiles[tile_idx * 64 + (y & 7) * 8 + (x & 7)] = idx
    return bytes(tiles)


def parse_header(path):
    text = open(path, encoding='utf-8').read()
    m = re.search(r'#define\s+FORMAT_PACK_COUNT\s+(\d+)', text)
    if not m:
        sys.exit(f'{path}: FORMAT_PACK_COUNT not found')
    count = int(m.group(1))
    m = re.search(
        r'static const u16 sFormatPackPortraitNumber\[FORMAT_PACK_COUNT\] = \{\s*([^}]+)\}',
        text,
        re.DOTALL,
    )
    if not m:
        sys.exit(f'{path}: sFormatPackPortraitNumber not found')
    nums = [int(x.strip().rstrip(',')) for x in m.group(1).split(',') if x.strip()]
    if len(nums) != count:
        sys.exit(f'{path}: expected {count} portrait numbers, got {len(nums)}')
    return count, nums


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else 'build/assets/format_pack_covers.bin'
    for path in (HEADER, ART_GFX, ART_PAL, NUMBER_TO_ID, GFX_BANK_A):
        if not os.path.isfile(path):
            sys.exit(f'format_pack_covers: missing {path} (run make setup / newcards build first)')

    count, portraits = parse_header(HEADER)
    n2i = struct.unpack('<2048H', open(NUMBER_TO_ID, 'rb').read())
    gfx = open(ART_GFX, 'rb').read()
    pals = open(ART_PAL, 'rb').read()
    shop_pal = load_pack_list_pal()

    covers = bytearray()
    baked = 0
    for fi, number in enumerate(portraits):
        if number == 0:
            covers.extend(bytes(PACK_COVER_BYTES))
            continue
        if number >= len(n2i):
            sys.exit(f'portrait card number {number} out of range (pack index {fi})')
        card_id = n2i[number]
        if card_id == 0:
            sys.exit(f'no card ID for portrait number {number} (pack index {fi})')
        covers.extend(blit_art_cover(card_id, gfx, pals, shop_pal))
        baked += 1

    os.makedirs(os.path.dirname(out_path) or '.', exist_ok=True)
    # Write only when changed (same idea as banlist.write_if_changed).
    try:
        old = open(out_path, 'rb').read()
    except OSError:
        old = None
    if old != bytes(covers):
        open(out_path, 'wb').write(covers)
    print(
        f'format_pack_covers: {baked} covers, {count} slots, {len(covers)} bytes -> {out_path}',
        file=sys.stderr,
    )


if __name__ == '__main__':
    main()
