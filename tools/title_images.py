#!/usr/bin/env python3
"""The title screen's images in plain layouts (src/mod_title.c), so Hex Maniac Advance can show and edit them.

  python3 tools/title_images.py build/assets/title_images.bin

The original title loads image packs (deduplicated tiles and a cell list, which HMA can't show as pictures); the mod
draws these copies instead, each its colours then its tiles in row-major order (tools/hma_toml.py anchors them):
  text       title/yaks_ygo_hack.png: the whole screen (240 x 160) of BG0 (the original logo's layer, now 4bpp),
             "Yak's YGO Hack" at y 40 at its final size (the 2x is in the image), 16 colours (0 transparent; BG
             palette 8, entries 128-143), 30 x 20 4bpp tiles.
  flame      the whole screen (240 x 160) of BG1 (now 256-colour), the flames (assets/gfx/bank_b/087C1DCC.png, 96 x
             96) at x 72, y 0 as before: 128 colours, BG palette entries 0-127 (entry 0 is the screen's backdrop; the
             flames' own colours keep their indices 1-14), 30 x 20 8bpp tiles, each pixel its palette entry (0
             transparent).
  The game loads only the distinct tiles of text and flame (src/mod_title.c: blank tiles share one empty tile,
  identical ones are stored once): the two full-screen images don't fit in the title's background VRAM otherwise.
  coin       the coin (087C0CD4_coin.png, 96 x 96): 16 colours, 12 x 12 4bpp tiles; BG2, palette 11.
  copyright  the "(c)1996 KAZUKI TAKAHASHI" line (087C056C.png, 224 x 16): 16 colours, 28 x 2 4bpp tiles; BG2,
             palette 9.
Output: u32 offsets[4] (text, flame, coin, copyright, from the start), then the four images, each 4-byte aligned.
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402  (png_read, rgb_to_bgr555)

FLAME_COLORS = 128              # the flames' palette entries 0-127 (0: the backdrop)


def tiles(px, w, h, bpp):
    out = bytearray()
    for ty in range(h // 8):
        for tx in range(w // 8):
            for y in range(8):
                row = px[(ty * 8 + y) * w + tx * 8:(ty * 8 + y) * w + tx * 8 + 8]
                out += bytes(row) if bpp == 8 else bytes(row[i] | row[i + 1] << 4 for i in range(0, 8, 2))
    return bytes(out)


def colours(pal, n):
    return struct.pack(f'<{n}H', *[A.rgb_to_bgr555(c) for c in (list(pal) + [(0, 0, 0)] * n)[:n]])


def image(path, w, h, bpp, max_colour):
    pw, ph, px, pal = A.png_read(open(path, 'rb').read())
    if (pw, ph) != (w, h):
        sys.exit(f'title_images: {path} is {pw}x{ph}, not {w}x{h}')
    if max(px) > max_colour:
        sys.exit(f'title_images: {path} uses colour {max(px)}; at most {max_colour}')
    return px, pal


def main():
    text_px, text_pal = image('title/yaks_ygo_hack.png', 240, 160, 4, 15)
    text = colours(text_pal, 16) + tiles(text_px, 240, 160, 4)

    flame_px, flame_pal = image('assets/gfx/bank_b/087C1DCC.png', 96, 96, 4, FLAME_COLORS - 1)
    flame_cols = [(0, 0, 0)] + list(flame_pal[1:FLAME_COLORS])       # entry 0: the black backdrop
    screen = [0] * (240 * 160)
    for y in range(96):
        for x in range(96):
            screen[y * 240 + 72 + x] = flame_px[y * 96 + x]
    flame = colours(flame_cols, FLAME_COLORS) + tiles(screen, 240, 160, 8)

    coin_px, coin_pal = image('assets/gfx/bank_b/087C0CD4_coin.png', 96, 96, 4, 15)
    coin = colours(coin_pal, 16) + tiles(coin_px, 96, 96, 4)

    cr_px, cr_pal = image('assets/gfx/bank_b/087C056C.png', 224, 16, 4, 15)
    copyright = colours(cr_pal, 16) + tiles(cr_px, 224, 16, 4)

    body, head = b'', b''
    for part in (text, flame, coin, copyright):
        head += struct.pack('<I', 16 + len(body))
        body += part + bytes(-len(part) % 4)
    data = head + body
    os.makedirs(os.path.dirname(sys.argv[1]), exist_ok=True)
    if not os.path.exists(sys.argv[1]) or open(sys.argv[1], 'rb').read() != data:
        open(sys.argv[1], 'wb').write(data)


if __name__ == '__main__':
    main()
