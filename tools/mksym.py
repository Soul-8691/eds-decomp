#!/usr/bin/env python3
"""Write a pokeemerald-style .sym file from a linked ELF.

  python3 tools/mksym.py build/eds.elf eds.sym [build/eds.map]

Format (one line per entry): ``address binding size name``
  address and size are 8-digit hex; binding is the first character from objdump (g/l).

Symbol list follows pret's ``objdump -t`` filter (addresses ``^0[2389]``), but drops linker/compiler
noise with no extent, and fills in sizes objdump leaves at zero (ABS RAM labels in symbols.ld,
rodata/asm globals). Inserts ``free space`` for holes inside each loaded ROM section, linker *fill*
padding from the map file, and gaps between sections.
Only rewrites the output when its content changes.
"""
import os
import re
import subprocess
import sys
from dataclasses import dataclass

elf = sys.argv[1] if len(sys.argv) > 1 else 'build/eds.elf'
out = sys.argv[2] if len(sys.argv) > 2 else 'eds.sym'
map_path = sys.argv[3] if len(sys.argv) > 3 else os.path.join(
    os.path.dirname(elf), os.path.basename(elf).replace('.elf', '.map'))

LINE = re.compile(
    r'^([0-9a-f]{8}) (\w).{6} \S+\t([0-9a-f]{8}) (\S+)$', re.I)
SEC = re.compile(
    r'^\s+\d+\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)', re.I)
MAP_FILL = re.compile(r'^\s+\*fill\*\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)', re.I)
MAP_OBJECT = re.compile(
    r'^\s+\.(?:text(?:\.added)?|rodata(?:\.added|\.pinned)?)\s+'
    r'(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s+', re.I)
MAP_SYM = re.compile(r'^\s+(0x[0-9a-f]+)\s+(\S+)\s*$', re.I)
ROM_GAP_SECTIONS = frozenset({
    '.text', '.rodata', '.text.added', '.rodata.added', '.rodata.pinned',
})
ABS_BOUNDS = {
    'ewram': (0x02000000, 0x02040000),
    'iwram': (0x03000000, 0x03008000),
}
ADDR_LABEL = re.compile(r'^[0-9A-Fa-f]{8}$')


@dataclass
class Sym:
    addr: int
    addr_s: str
    bind: str
    size: int
    name: str
    section: str


def parse_objdump(elf_path):
    text = subprocess.run(
        ['arm-none-eabi-objdump', '-t', elf_path],
        capture_output=True, text=True, check=True).stdout
    out = []
    for line in sorted(set(text.splitlines())):
        m = LINE.match(line)
        if not m or not re.match(r'^0[2389]', m.group(1), re.I):
            continue
        addr_s, bind, size_s, name = m.group(1), m.group(2), m.group(3), m.group(4)
        parts = line.split('\t')
        left = parts[0].split()
        section = left[-1] if left else ''
        out.append(Sym(int(addr_s, 16), addr_s, bind, int(size_s, 16), name, section))
    return out


def parse_sections(elf_path):
    text = subprocess.run(
        ['arm-none-eabi-objdump', '-h', elf_path],
        capture_output=True, text=True, check=True).stdout
    secs = {}
    for line in text.splitlines():
        m = SEC.match(line)
        if not m:
            continue
        name, size, vma = m.group(1), int(m.group(2), 16), int(m.group(3), 16)
        if name in ROM_GAP_SECTIONS and size:
            secs[name] = (vma, vma + size)
    return secs


def parse_map_objects(map_file):
    """Per-symbol extents from linker map (.rodata 0xADDR 0xSIZE file.o lines)."""
    if not os.path.exists(map_file):
        return {}
    by_name = {}
    for line in open(map_file, encoding='utf-8', errors='replace'):
        m = MAP_OBJECT.match(line)
        if not m:
            continue
        addr, size = int(m.group(1), 16), int(m.group(2), 16)
        if size:
            by_name[addr] = size
    # Attach names from the following indented symbol lines.
    names_at = {}
    for line in open(map_file, encoding='utf-8', errors='replace'):
        m = MAP_SYM.match(line)
        if m:
            names_at[int(m.group(1), 16)] = m.group(2)
    return {names_at[addr]: size for addr, size in by_name.items() if addr in names_at}


def parse_map_fills(map_file):
    """Linker *fill* padding (alignment gaps) from the GNU ld map."""
    if not os.path.exists(map_file):
        return []
    rows = []
    for line in open(map_file, encoding='utf-8', errors='replace'):
        m = MAP_FILL.match(line)
        if not m:
            continue
        addr, size = int(m.group(1), 16), int(m.group(2), 16)
        if size and re.match(r'^0[2389]', f'{addr:08x}', re.I):
            rows.append((addr, 'l', f'{size:08x}', 'free space'))
    return rows


def skip_sym(s: Sym) -> bool:
    n = s.name
    if n in ('$t', '$d', '$a', 'sp_usr', 'sp_irq', 'PCM_DATA'):
        return True
    if n.endswith('.o'):
        return True
    if n.startswith('.gcc2_compiled'):
        return True
    if n.startswith('.'):
        return True
    if n.startswith('_') and ADDR_LABEL.match(n[1:]):
        return True
    if s.bind == 'l' and s.size == 0:
        return True
    return False


def size_group(s: Sym):
    if s.section == '*ABS*':
        hi = s.addr >> 24
        if hi == 0x02:
            return ('abs', 'ewram')
        if hi == 0x03:
            return ('abs', 'iwram')
    return ('sec', s.section)


def infer_sizes(syms, sections):
    groups = {}
    for s in syms:
        groups.setdefault(size_group(s), []).append(s)
    for key, entries in groups.items():
        if key[0] == 'abs':
            lo, hi = ABS_BOUNDS[key[1]]
        elif key[1] in sections:
            lo, hi = sections[key[1]]
        else:
            lo, hi = 0, 0x100000000
        addrs = sorted({e.addr for e in entries})
        for e in entries:
            if e.size:
                continue
            nxt = hi
            for a in addrs:
                if a > e.addr:
                    nxt = a
                    break
            e.size = max(0, nxt - e.addr)
    return syms


def merge_intervals(ranges):
    if not ranges:
        return []
    ranges = sorted(ranges)
    out = [list(ranges[0])]
    for lo, hi in ranges[1:]:
        if lo <= out[-1][1]:
            out[-1][1] = max(out[-1][1], hi)
        else:
            out.append([lo, hi])
    return [(a, b) for a, b in out]


def free_space_rows(syms, sections):
    by_sec = {n: [] for n in sections}
    for s in syms:
        if s.section not in by_sec or not s.size:
            continue
        by_sec[s.section].append((s.addr, s.addr + s.size))
    rows = []
    for _sec, (lo, hi) in sorted(sections.items(), key=lambda x: x[1][0]):
        covered = merge_intervals(by_sec.get(_sec, []))
        pos = lo
        for a, b in covered:
            if a > pos:
                rows.append((pos, 'l', f'{a - pos:08x}', 'free space'))
            pos = max(pos, b)
        if pos < hi:
            rows.append((pos, 'l', f'{hi - pos:08x}', 'free space'))
    return rows


def inter_section_gap_rows(sections):
    """ROM holes between loaded sections (e.g. .text.added ends before .rodata.pinned)."""
    ordered = sorted(sections.values(), key=lambda r: r[0])
    rows = []
    for (_lo1, hi1), (lo2, _hi2) in zip(ordered, ordered[1:]):
        if lo2 > hi1:
            rows.append((hi1, 'l', f'{lo2 - hi1:08x}', 'free space'))
    return rows


def dedupe_gap_rows(rows):
    """Drop identical (addr, size) free-space rows; merge overlaps from fills vs section scan."""
    by_addr = {}
    for row in rows:
        addr, bind, size_s, name = row
        size = int(size_s, 16)
        prev = by_addr.get(addr)
        if prev is None or int(prev[2], 16) < size:
            by_addr[addr] = row
    return list(by_addr.values())


def format_row(addr, bind, size, name):
    size_s = size if isinstance(size, str) else f'{int(size):08x}'
    return f'{int(addr):08x} {bind} {size_s} {name}'


def apply_map_sizes(syms, map_sizes):
    for s in syms:
        if s.name in map_sizes:
            s.size = map_sizes[s.name]


def main():
    syms = [s for s in parse_objdump(elf) if not skip_sym(s)]
    sections = parse_sections(elf)
    fills = parse_map_fills(map_path)
    map_sizes = parse_map_objects(map_path)
    apply_map_sizes(syms, map_sizes)
    infer_sizes(syms, sections)
    syms = [s for s in syms if s.size > 0]
    rows = [format_row(s.addr, s.bind, s.size, s.name) for s in syms]
    gaps = dedupe_gap_rows(
        free_space_rows(syms, sections) + inter_section_gap_rows(sections) + fills)
    rows.extend(format_row(a, b, s, n) for a, b, s, n in gaps)
    rows.sort(key=lambda line: (int(line.split()[0], 16), line.split()[3] == 'free space', line))
    text = '\n'.join(rows) + '\n'
    if not os.path.exists(out) or open(out, encoding='utf-8').read() != text:
        open(out, 'w', encoding='utf-8').write(text)
        print(f'wrote {out} ({len(rows)} lines)')


if __name__ == '__main__':
    main()
