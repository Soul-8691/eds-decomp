#!/usr/bin/env python3
"""The Synchro and Xyz Monster card frames (gCardFrameSynchroGfx, gCardFrameXyzGfx): the Fusion frame with its
purple made white, or black (xyz).

  python3 tools/synchro_frame.py build/assets/synchro_frame.bin
  python3 tools/synchro_frame.py build/assets/xyz_frame.bin xyz

Reads the Fusion frame's image pack (gCardFrameFusionGfx in data/rodata_08087FD0.s, from the extracted asset bank)
and writes the same pack with its palette changed: every purple colour (red and blue both clearly above green)
becomes a light grey of about its brightness, raised toward white. The other colours (black, the orange star and
the greys of the text and art windows) stay. The pack layout is the one LoadCardFrame and LoadBgImageMap1 read:
a 16-byte header with the 32-colour BGR555 palette at +8, then the tiles.
"""
import re
import struct
import sys

LAYOUT = 'data/rodata_08087FD0.s'
PALETTE_AT, COLORS = 8, 32
LIFT = 7                # added to the grey of a recoloured entry (towards white)


def fusion_frame():
    text = open(LAYOUT).read()
    m = re.search(r'gCardFrameFusionGfx:.*\n\s*\.incbin "([^"]+)", (0x[0-9A-Fa-f]+), (0x[0-9A-Fa-f]+)', text)
    if not m:
        sys.exit(f'synchro_frame: gCardFrameFusionGfx not found in {LAYOUT}')
    start, size = int(m.group(2), 16), int(m.group(3), 16)
    return bytearray(open(m.group(1), 'rb').read()[start:start + size])


def whiten(c):
    r, g, b = c & 31, (c >> 5) & 31, (c >> 10) & 31
    if min(r, b) - g < 3:
        return c                # not purple
    grey = min(31, (r + g + b) // 3 + LIFT)
    return grey | grey << 5 | min(31, grey + 1) << 10    # a touch of blue keeps it from looking yellowish


def blacken(c):
    """A purple colour as a dark grey of a quarter of its brightness (the Xyz frame's black, still shaded)."""
    r, g, b = c & 31, (c >> 5) & 31, (c >> 10) & 31
    if min(r, b) - g < 3:
        return c                # not purple
    grey = (r + g + b) // 12 + 1
    return grey | grey << 5 | grey << 10


def main():
    if len(sys.argv) not in (2, 3) or (len(sys.argv) == 3 and sys.argv[2] != 'xyz'):
        sys.exit(__doc__)
    recolour = blacken if len(sys.argv) == 3 else whiten
    pack = fusion_frame()
    pal = struct.unpack_from(f'<{COLORS}H', pack, PALETTE_AT)
    struct.pack_into(f'<{COLORS}H', pack, PALETTE_AT, *[recolour(c) for c in pal])
    data = bytes(pack)
    try:
        if open(sys.argv[1], 'rb').read() == data:
            return
    except FileNotFoundError:
        pass
    open(sys.argv[1], 'wb').write(data)


if __name__ == '__main__':
    main()
