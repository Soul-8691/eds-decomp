#!/usr/bin/env python3
"""Emulator check of the Deck Edit list relocation made by card_expand.py.

  python3 tools/card_expand/test_relocation.py [--rom eds.gba] [--seeds 200]

Needs the `unicorn` module (pip install unicorn). Two checks, run on the CPU alone (no PPU, no BIOS: the one
SWI the list code makes, CpuSet, is done in Python):

1. Equivalence: the original ROM and a copy with only the RAM patches start from the same random, valid save
   (deck <= 80, side <= 15 cards) and run DeckEdit_BuildCardLists (all modes), GetSelectedCardCopies,
   DeckStats_CountCategory, FilterAndSortList, GetActiveListRow and UpdateNameIndexLetters. Every list (read
   through DeckEdit_GetListCard) and return value must be equal, the old trunk rows must stay zero and the deck
   and side rows must be byte-identical.
2. Capacity: with the full patch for last ID 1020 and every ID owned, Deck Edit must list every ID outside card
   numbers 1920-1999 (IDs above 820 read whatever the not-yet-written tables hold), without touching the deck
   row or the memory past the new rows, and the List Filter must keep the whole set.
"""
import argparse, importlib.util, os, random, struct, sys
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_HOOK_INTR
from unicorn.arm_const import (UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_SP,
                               UC_ARM_REG_LR, UC_ARM_REG_PC)

HERE = os.path.dirname(os.path.abspath(__file__))
STOP = 0x0E100000                   # return address: outside every mapped region the code uses
F = dict(GetSelected=0x08068434, ActiveRow=0x08068668, Build=0x080686E8, NameIdx=0x08068B90,
         Get=0x08068D1C, Set=0x08068DA4, Filter=0x08069284, Stats=0x0806C590)
DE, SAVE, GMAIN = 0x0201DB20, 0x02011C20, 0x03000040
OLD_ROW0, OLD_STRIDE, OLD_DECK0 = 0x0201E164, 0x728, 0x0201E7CE


def patched(rom, last, ram_only):
    spec = importlib.util.spec_from_file_location('card_expand', os.path.join(HERE, 'card_expand.py'))
    ce = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(ce)       # fresh module: its patch list P starts empty
    if not ram_only:
        ce.counts(last)
        ce.pointers(last, os.path.join(HERE, 'card_table_refs.csv'))
    ce.word(0x080693D8, 0x0201EFC4, ce.NEW_SCRATCH, 'scratch')
    ce.word(0x08069A88, 0x0201EFC4, ce.NEW_SCRATCH, 'scratch')
    d = bytearray(rom)
    for ad, old, new, note in ce.P:
        off = ad - 0x08000000
        assert d[off:off + len(old)] == old, '%#x: unexpected bytes (%s)' % (ad, note)
        d[off:off + len(new)] = new
    return bytes(d)


class Gba:
    def __init__(self, rom):
        self.u = u = Uc(UC_ARCH_ARM, UC_MODE_THUMB)
        u.mem_map(0x08000000, 0x02000000); u.mem_write(0x08000000, rom)
        for base, size in [(0x02000000, 0x40000), (0x03000000, 0x8000), (0x04000000, 0x1000),
                           (0x05000000, 0x1000), (0x06000000, 0x20000), (STOP & ~0xFFF, 0x1000)]:
            u.mem_map(base, size)
        u.hook_add(UC_HOOK_INTR, self.swi)

    def swi(self, u, intno, _):
        pc = u.reg_read(UC_ARM_REG_PC)
        assert struct.unpack('<H', u.mem_read(pc - 2, 2))[0] & 0xFF == 0x0B, 'only CpuSet is emulated'
        src, dst, c = (u.reg_read(r) for r in (UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2))
        n, fill, w = c & 0x1FFFFF, c >> 24 & 1, 4 if c >> 26 & 1 else 2
        for i in range(n):
            u.mem_write(dst + i * w, bytes(u.mem_read(src + (0 if fill else i * w), w)))

    def call(self, fn, *args):
        u = self.u
        for r, v in zip((UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3), args):
            u.reg_write(r, v)
        u.reg_write(UC_ARM_REG_SP, 0x03007B00); u.reg_write(UC_ARM_REG_LR, STOP | 1)
        u.emu_start(fn | 1, STOP, count=50000000)
        return u.reg_read(UC_ARM_REG_R0)

    def r(self, a, n): return bytes(self.u.mem_read(a, n))
    def w(self, a, b): self.u.mem_write(a, b)
    def count(self, row, lst): return struct.unpack('<H', self.r(DE + 0x1494 + (row * 3 + lst) * 2, 2))[0]
    def entries(self, lst, row): return [self.call(F['Get'], lst, row, i) & 0xFFFF for i in range(self.count(row, lst))]
    def snapshot(self): return [self.entries(l, row) for row in (0, 1) for l in (0, 1, 2)]


def equivalence_case(roms, seed):
    rng = random.Random(seed)
    gs = [Gba(r) for r in roms]
    deck = set(rng.sample(range(1, 821), rng.randrange(0, 61)))
    fusion = set(rng.sample(range(1, 821), rng.randrange(0, 21)))
    side = set(rng.sample(range(1, 821), rng.randrange(0, 16)))
    deck_or_fusion = set(list(deck | fusion)[:80])

    def entry(i):
        if not 1 <= i <= 820:
            return 0
        v = rng.choice([0, 0, 1, 2, 3, 50])
        if i in deck_or_fusion:
            v |= rng.randrange(1, 4) << (10 if i in deck else 14)
        if i in side:
            v |= rng.randrange(1, 4) << 12
        return v
    trunk = b''.join(struct.pack('<I', entry(i)) for i in range(0x800))
    mode, rows_shown = rng.randrange(3), bytes(rng.randrange(2) for _ in range(3))
    flags, cursor = rng.getrandbits(8), rng.randrange(3)
    lst, flt, srt = rng.randrange(3), rng.randrange(8), rng.randrange(6)
    pre = [sorted(rng.sample(range(1, 821), rng.randrange(0, 200)))[:cap] for cap in (821, 80, 15)]
    out = []
    for g in gs:
        g.w(SAVE + 8, trunk); g.w(GMAIN + 0x4874, bytes([mode])); g.w(DE + 0x14A0, rows_shown)
        g.w(DE + 0x1C5A, bytes([flags])); g.w(DE + 0x1C1C, bytes([cursor]))
        for l in range(3):                          # row 1 starts filled, through each ROM's own SetListCard
            for i, c in enumerate(pre[l]):
                g.call(F['Set'], c, l, 1, i)
            g.w(DE + 0x149A + l * 2, struct.pack('<H', len(pre[l])))
        res = []
        g.call(F['Build']); res.append(('build', g.snapshot()))
        for l in range(3):
            g.w(DE + 0x14A0, rows_shown)            # the stats code reuses these bytes; reset per call
            res.append(('copies', l, g.call(F['GetSelected'])))
            res += [('stats', l, cat, g.call(F['Stats'], l, cat)) for cat in range(1, 7)]
        g.w(DE + 0x14A0, rows_shown)
        g.call(F['Filter'], lst, flt, srt); res.append(('filter', (lst, flt, srt), g.snapshot()))
        for l in range(3):
            g.w(DE + 0x14A0 + l, rows_shown[l:l + 1])
            p = g.call(F['ActiveRow'], l)
            res.append(('activerow', l, g.r(p, 2 * g.count(rows_shown[l], l))))
        g.w(DE + 0x1C1C, bytes([lst])); g.call(F['NameIdx']); res.append(('nameidx', g.r(0x0201F775, 2)))
        out.append(res)
    problems = ['%s differs' % a[0] for a, b in zip(*out) if a != b][:1]
    if any(gs[1].r(OLD_ROW0 + row * OLD_STRIDE, 821 * 2) != bytes(821 * 2) for row in (0, 1)):
        problems.append('old trunk rows written')
    if any(gs[0].r(OLD_DECK0 + row * OLD_STRIDE, 95 * 2) != gs[1].r(OLD_DECK0 + row * OLD_STRIDE, 95 * 2) for row in (0, 1)):
        problems.append('deck/side rows differ')
    return problems


def capacity_case(rom, last=1020):
    g = Gba(rom)
    g.w(SAVE + 8, b''.join(struct.pack('<I', 1 if 1 <= i <= last else 0) for i in range(0x800)))
    sentinel = bytes(range(256))
    g.w(OLD_DECK0, sentinel[:0xBE]); g.w(0x0202B380, sentinel[:0x40])
    g.w(GMAIN + 0x4874, b'\0'); g.w(DE + 0x14A0, b'\0\0\0')
    g.call(F['Build'])
    number = lambda i: struct.unpack_from('<H', rom, 0x622AB4 + i * 2)[0]
    expect = [i for i in range(1, last + 1) if not 0 <= number(i) - 1920 <= 0x4F]
    problems = []
    if g.entries(0, 0) != expect:
        problems.append('Deck Edit list: %d of %d' % (g.count(0, 0), len(expect)))
    if g.r(OLD_DECK0, 0xBE) != sentinel[:0xBE] or g.r(0x0202B380, 0x40) != sentinel[:0x40]:
        problems.append('wrote outside the new rows')
    g.call(F['Filter'], 0, 0, 1)                   # trunk, All, by ATK: uses the moved non-monster scratch
    if sorted(g.entries(0, 1)) != expect:
        problems.append('List Filter lost cards')
    return len(expect), problems


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--rom', default='eds.gba')
    ap.add_argument('--seeds', type=int, default=200)
    a = ap.parse_args()
    rom = open(a.rom, 'rb').read()
    roms = (rom, patched(rom, 820, ram_only=True))
    bad = 0
    for seed in range(a.seeds):
        p = equivalence_case(roms, seed)
        if p:
            bad += 1
            print('seed %d: %s' % (seed, ', '.join(p)))
    print('equivalence: %d / %d seeds identical' % (a.seeds - bad, a.seeds))
    n, p = capacity_case(patched(rom, 1020, ram_only=False))
    print('capacity (last ID 1020): %d cards listed%s' % (n, '; ' + ', '.join(p) if p else ', no stray writes'))
    sys.exit(1 if bad or p else 0)


if __name__ == '__main__':
    main()
