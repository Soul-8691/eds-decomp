#!/usr/bin/env python3
"""Stats for format-pack roulette tables (build/assets/format_pack_roulettes.bin)."""
import re
import struct
import sys
from collections import Counter

ROULETTE_SIZE = 8192
header = open('build/include/format_pack_data.h', encoding='utf-8').read()
m = re.search(r'#define FORMAT_PACK_COUNT (\d+)', header)
if not m:
    sys.exit('FORMAT_PACK_COUNT not found')
format_count = int(m.group(1))
names = re.findall(r'/\* (.+?) \(\d+ ROM cards\) \*/', header)

path = 'build/assets/format_pack_roulettes.bin'
data = open(path, 'rb').read()
expect = ROULETTE_SIZE * format_count * 2
if len(data) != expect:
    print(f'expected {expect} bytes, got {len(data)}', file=sys.stderr)
    sys.exit(1)
for i in range(format_count):
    off = i * ROULETTE_SIZE * 2
    nums = struct.unpack(f'<{ROULETTE_SIZE}H', data[off : off + ROULETTE_SIZE * 2])
    c = Counter(nums)
    label = names[i] if i < len(names) else str(i)
    print(f'{label}: unique={len(c)} top={c.most_common(2)}')
