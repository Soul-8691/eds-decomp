#!/usr/bin/env python3
"""Point the function pointers in the data tables at the functions' addresses in this build.

  python3 tools/fixptrs.py build/eds.elf eds.gba       # the Makefile runs it after objcopy

The data tables are built from assets/ with absolute addresses: a function pointer is resolved through
config/functions.tsv, i.e. to where the function is in the original ROM. The data itself stays at its original
addresses (tools/mkld.py pins it), but code that changed size moves the functions after it. This step finds
every function pointer in the built table assets (a 4-aligned word equal to a function address of
config/functions.tsv, with the Thumb bit for a Thumb function) and writes the function's address from the ELF
into the ROM image at that place. Only table assets are scanned; graphics and sound data are left alone.
"""
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402

TABLE_TYPES = ('tables_', 'gfx_scenes_ptrs')


def main():
    elf, rom_path = sys.argv[1], sys.argv[2]
    funcs = {}
    for line in open('config/functions.tsv'):
        if line.startswith('#') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        funcs[int(f[0], 16) | (1 if f[1] == 't' else 0)] = f[3]
    syms = {}
    out = subprocess.run(['arm-none-eabi-nm', elf], capture_output=True, text=True, check=True).stdout
    for line in out.splitlines():
        p = line.split()
        if len(p) == 3 and p[1] in 'tT':
            syms[p[2]] = int(p[0], 16)
    rom = bytearray(open(rom_path, 'rb').read())
    slots = moved = 0
    for p in A.manifest():
        if not p['type'].startswith(TABLE_TYPES) or p['path'].startswith('cards/'):
            continue                # (the card tables hold no pointers and moved, see tools/newcards.py)
        data = open(A.out_path(p), 'rb').read()
        for off in range(-p['start'] % 4, len(data) - 3, 4):
            v = int.from_bytes(data[off:off + 4], 'little')
            name = funcs.get(v)
            if name is None:
                continue
            if name not in syms:
                sys.exit(f'fixptrs: {name} (pointer at 0x{p["start"] + off:08X}) is not in {elf}')
            new = syms[name] | (v & 1)
            at = p['start'] + off - A.BASE
            slots += 1
            if new != v:
                rom[at:at + 4] = new.to_bytes(4, 'little')
                moved += 1
    open(rom_path, 'wb').write(rom)
    print(f'fixptrs: {slots} function pointers in the data, {moved} moved')


if __name__ == '__main__':
    main()
