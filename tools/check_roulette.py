#!/usr/bin/env python3
"""Stats for format-pack roulette tables (build/assets/format_pack_roulettes.bin)."""
import struct
import sys
from collections import Counter

ROULETTE_SIZE = 65536
FORMAT_COUNT = 15

path = 'build/assets/format_pack_roulettes.bin'
data = open(path, 'rb').read()
expect = ROULETTE_SIZE * FORMAT_COUNT * 2
if len(data) != expect:
    print(f'expected {expect} bytes, got {len(data)}', file=sys.stderr)
    sys.exit(1)
for i in range(FORMAT_COUNT):
    off = i * ROULETTE_SIZE * 2
    nums = struct.unpack(f'<{ROULETTE_SIZE}H', data[off : off + ROULETTE_SIZE * 2])
    c = Counter(nums)
    print(f'format {i}: unique={len(c)} hitotsu(2)={c[2]} top={c.most_common(3)}')
