#!/usr/bin/env python3
"""Fill the pre-duel screens' scene copies (gModPreDuelScenes, src/mod_preduel.c) in a linked ROM.

  python3 tools/preduel_scenes.py eds_demo.gba eds_demo.sym         (the Makefile's ROM rule, after record_faces)

An opponent without a pre-duel picture of its own shows its duelist's scene from the original pre-duel dialogue:
the scene set GetSceneSet (src/bustup_runner.c) gives that duelist ID, whose bitmap and eye sprites are LZSS in the
ROM. Hex Maniac Advance can't read those, so each such opponent's gModPreDuelTable entry points at a copy (zeros in
data/mod_preduel_data.s) that this fills, uncompressed: u16 bgColours[256], the 240x96 bitmap as 8bpp tiles (30 x
12, row-major), u16 objColours[256], the 0x2000-byte OBJ tile sheet (16 x 16 4bpp tiles, as CopyTileSheetTo2D
copies it). The game draws from the copy (tools/hma_toml.py: graphics.duelists.<duelist>.scene / .eyes), and the
eyes still blink with the scene set's own animation tracks.
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), 'assetfmt'))
from hma_toml import load_sym  # noqa: E402
from gfx_scenes import lz_decode  # noqa: E402

ROM_BASE = 0x08000000
ENTRY = 256 * 2 + 240 * 96 + 256 * 2 + 0x2000
TABLE_ROW = 12                  # struct PreDuelEntry: picture, qr, scene
SCENE_SET = 20                  # struct SceneSet: bitmapLz, bgPal, objPal, objTilesLz, anim


def scene_sets():
    """{duelist ID: scene set index} from GetSceneSet's switch (src/bustup_runner.c); its default is set 6."""
    src = open('src/bustup_runner.c', encoding='utf-8').read()
    body = src[src.index('const struct SceneSet *GetSceneSet(u32 charId)'):]
    body = body[:body.index('\n}\n')]
    ids = {n: int(v) for n, v in re.findall(r'(DUELIST_\w+) = (\d+)',
                                            open('include/constants/game.h', encoding='utf-8').read())}
    out, pending = {}, []
    for token in re.findall(r'case (\w+):|return &gSceneSets\[(\d+)\];', body):
        if token[0]:
            pending.append(ids[token[0]] if token[0] in ids else int(token[0], 0))
        else:
            for c in pending:
                out[c] = int(token[1])
            pending = []
    return out


def scene_set_of(duelist):
    return scene_sets().get(duelist, 6)


def tiles8(bitmap, w, h):
    out = bytearray()
    for ty in range(h // 8):
        for tx in range(w // 8):
            for y in range(8):
                out += bitmap[(ty * 8 + y) * w + tx * 8:(ty * 8 + y) * w + tx * 8 + 8]
    return bytes(out)


def ptr(rom, at):
    return struct.unpack_from('<I', rom, at)[0]


def main():
    rom_path, sym_path = sys.argv[1:3]
    sym = load_sym(sym_path)
    rom = bytearray(open(rom_path, 'rb').read())
    table = sym['gModPreDuelTable'][0] - ROM_BASE
    sets = sym['gSceneSets'][0] - ROM_BASE
    count = 0
    for duelist in range(25):
        scene = ptr(rom, table + duelist * TABLE_ROW + 8)
        if not scene:
            continue
        at = scene - ROM_BASE
        s = sets + scene_set_of(duelist) * SCENE_SET
        bitmap_lz, bg_pal, obj_pal, obj_tiles_lz = (ptr(rom, s + 4 * k) for k in range(4))
        bitmap = lz_decode(bytes(rom[bitmap_lz - ROM_BASE:bitmap_lz - ROM_BASE + 0x10000]))[0][:240 * 96]
        data = bytes(rom[bg_pal - ROM_BASE:bg_pal - ROM_BASE + 512]) + tiles8(bitmap, 240, 96)
        data += bytes(rom[obj_pal - ROM_BASE:obj_pal - ROM_BASE + 512]) if obj_pal else bytes(512)
        sheet = lz_decode(bytes(rom[obj_tiles_lz - ROM_BASE:obj_tiles_lz - ROM_BASE + 0x8000]))[0][:0x2000] \
            if obj_tiles_lz else b''
        data += sheet + bytes(0x2000 - len(sheet))
        assert len(data) == ENTRY
        rom[at:at + ENTRY] = data
        count += 1
    open(rom_path, 'wb').write(rom)
    print(f'preduel_scenes: {count} scenes')


if __name__ == '__main__':
    main()
