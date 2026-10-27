#!/usr/bin/env python3
"""List every pointer into the per-card ROM tables (0x0822C720-0x08625460).

  tools/dr python3 tools/card_expand/refs.py [--elf build/eds.elf --rom eds.gba --out tools/card_expand/card_table_refs.csv]

Literal-pool words come from `arm-none-eabi-objdump -d` of the ELF (with function names); the .rodata and
game-table regions of the ROM are also scanned for aligned words, since data tables can hold pointers too
(the sound region is skipped: its sample bytes match by chance). Rows tagged `(data)` that land mid-entry are
u16 pairs, not pointers; card_expand.py skips them. Needs a matching build (`make compare`).
"""
import argparse, csv, os, re, struct, subprocess

TABLES = [(0x0822C720, 0x40, 'gCardNames'), (0x08239460, 0x40, '(unused name bank)'),
          (0x082461A0, 0x1E0, 'gCardDescriptions'), (0x082A6500, 0x10E0, 'gCardArt'),
          (0x08608360, 0x80, 'gCardArtPalettes'), (0x08621DE0, 4, 'gCardStats'),
          (0x08622AB4, 2, 'gCardIdToNumber'), (0x08623120, 4, 'gCardPasswords'),
          (0x08623DF4, 2, 'gCardNumberToId'), (0x08624DF4, 2, 'gCardJpSortKey')]
BLOCK_END = 0x08625460
DATA_SCAN = [(0x08080A20, 0x08087FD0), (0x08139F5C, 0x081ABE4C)]   # .rodata 1, game tables (rom-map)


def table_of(v):
    for (s, size, name), nxt in zip(TABLES, [t[0] for t in TABLES[1:]] + [BLOCK_END]):
        if s <= v < nxt:
            return s, size, name
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--elf', default='build/eds.elf')
    ap.add_argument('--rom', default='eds.gba')
    ap.add_argument('--objdump', default='arm-none-eabi-objdump')
    ap.add_argument('--out', default=os.path.join(os.path.dirname(os.path.abspath(__file__)), 'card_table_refs.csv'))
    a = ap.parse_args()

    rows = set()
    fn = None
    dis = subprocess.run([a.objdump, '-d', a.elf], check=True, stdout=subprocess.PIPE, universal_newlines=True).stdout
    for line in dis.splitlines():
        m = re.match(r'^([0-9a-f]{8}) <(.+)>:', line)
        if m:
            fn = m.group(2)
            continue
        m = re.match(r'^ ([0-9a-f]{7,8}):\s+[0-9a-f]+\s+\t\.word\s+0x([0-9a-f]+)', line)
        if m:
            addr, v = int(m.group(1), 16), int(m.group(2), 16)
            if TABLES[0][0] <= v < BLOCK_END:
                rows.add((addr, v, fn))
    rom = open(a.rom, 'rb').read()
    for lo, hi in DATA_SCAN:
        for addr in range(lo, hi, 4):
            v = struct.unpack_from('<I', rom, addr - 0x08000000)[0]
            if TABLES[0][0] <= v < BLOCK_END:
                rows.add((addr, v, '(data)'))

    with open(a.out, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['rom_offset', 'address', 'value', 'table', 'index', 'byte_in_entry', 'function'])
        for addr, v, fn in sorted(rows):
            s, size, name = table_of(v)
            idx, rem = divmod(v - s, size)
            w.writerow(['ROM+0x%06X' % (addr - 0x08000000), '0x%08X' % addr, '0x%08X' % v, name, idx, rem, fn])
    print(len(rows), 'pointers ->', a.out)


if __name__ == '__main__':
    main()
