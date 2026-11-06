#!/usr/bin/env python3
"""The duel board picture with Extra Monster Zones (gModDuelFieldImage, src/mod_duel_hud.c; wiki/game/duel-hud.md).

  python3 tools/duel_board.py build/assets/duel_board.bin

Reads the game's board (gDuelFieldImage in data/rodata_08087FD0.s, from the extracted asset bank) and writes it with
its middle row changed. The pack layout is the one LoadBgImage4bppMap1 reads: u16 colour count, palette at +8, then u16
tile count (8-byte header) and the tiles, then u16 cell count (8-byte header) and the cells, each u16 position (column
in bits 0-5, row in bits 8-15) and u16 tile. The tiles stay the same; only the cells change:
  - rows 18-21, columns 4-25 (the YOU and OPPONENT boxes, where the life points and phase letters were) are cleared;
  - the Extra Monster Zones: empty zone frames at columns 5-8 and 21-24 (above the first and fifth Main Monster
    Zones, at the edges), drawn with the zone frame tiles;
  - the shared phase panel between them (columns 10-19): a 10x4 box from the old boxes' edge and fill tiles; the
    turn player's name (row 19) and the phase letters (row 20) are sprites over it (src/mod_duel_hud.c);
  - row 17, columns 17-24: the opponent's zone bottoms where the OPPONENT label cut into them.
"""
import re
import struct
import sys

LAYOUT = 'data/rodata_08087FD0.s'

# zone frame tiles (a 4x4 block: top row, side columns, bottom row)
ZONE_TOP = [0x006, 0x007, 0x007, 0x008]
ZONE_LEFT, ZONE_RIGHT = 0x009, 0x00A
ZONE_BOTTOM = [0x011, 0x012, 0x012, 0x013]
# the old boxes' frame: corners, edges and the striped fill
PANEL_WIDTH = 10
PANEL = [
    [0x01E] + [0x01F] * (PANEL_WIDTH - 2) + [0x02E],
    [0x02F] + [0x030] * (PANEL_WIDTH - 2) + [0x033],
    [0x02F] + [0x030] * (PANEL_WIDTH - 2) + [0x033],
    [0x036] + [0x039] * (PANEL_WIDTH - 2) + [0x03B],
]
MIDDLE_ROWS = range(18, 22)
OLD_BOX_COLUMNS = range(4, 26)
EMZ_COLUMNS = (5, 21)
PANEL_COLUMN = 10


def field_image():
    text = open(LAYOUT).read()
    m = re.search(r'gDuelFieldImage:.*\n\s*\.incbin "([^"]+)", (0x[0-9A-Fa-f]+), (0x[0-9A-Fa-f]+)', text)
    if not m:
        sys.exit(f'duel_board: gDuelFieldImage not found in {LAYOUT}')
    start, size = int(m.group(2), 16), int(m.group(3), 16)
    return open(m.group(1), 'rb').read()[start:start + size]


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    pack = field_image()
    colours = struct.unpack_from('<H', pack, 0)[0]
    tile_count = struct.unpack_from('<H', pack, 8 + colours * 2)[0]
    cells_at = 0x10 + colours * 2 + tile_count * 32
    count = struct.unpack_from('<H', pack, cells_at)[0]
    cells = {}
    order = []
    for i in range(count):
        pos, tile = struct.unpack_from('<2H', pack, cells_at + 8 + i * 4)
        key = (pos >> 8, pos & 0x3F)
        cells[key] = tile
        order.append(key)

    for r in MIDDLE_ROWS:
        for c in OLD_BOX_COLUMNS:
            cells.pop((r, c), None)
    for c0 in (17, 21):                     # the OPPONENT label's bite out of the zone bottoms
        for i, t in enumerate(ZONE_BOTTOM):
            cells[(17, c0 + i)] = t
    for c0 in EMZ_COLUMNS:
        for i in range(4):
            cells[(18, c0 + i)] = ZONE_TOP[i]
            cells[(21, c0 + i)] = ZONE_BOTTOM[i]
        for r in (19, 20):
            cells[(r, c0)] = ZONE_LEFT
            cells[(r, c0 + 3)] = ZONE_RIGHT
    for dr, row in enumerate(PANEL):
        for dc, t in enumerate(row):
            cells[(18 + dr, PANEL_COLUMN + dc)] = t

    keys = sorted(cells)
    head = pack[:cells_at]
    out = bytearray(head)
    out += struct.pack('<4H', len(keys), len(keys), len(keys), len(keys))
    for r, c in keys:
        out += struct.pack('<2H', r << 8 | c, cells[(r, c)])
    data = bytes(out)
    try:
        if open(sys.argv[1], 'rb').read() == data:
            return
    except FileNotFoundError:
        pass
    open(sys.argv[1], 'wb').write(data)


if __name__ == '__main__':
    main()
