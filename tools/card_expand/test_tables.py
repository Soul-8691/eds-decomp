#!/usr/bin/env python3
"""Emulator check of a ROM written by write_tables.py: the game's own table readers must see the same cards.

  python3 tools/card_expand/test_tables.py --orig eds.gba --new build/card_expand/expanded.gba [--clone NEW=OLD ...]

Needs `unicorn`. For every original card ID (1..820) it runs, on both ROMs, the code that reads each relocated
table and compares the results:
  LoadCardPicture          art + palette -> VRAM / palette RAM            (gCardArt, gCardArtPalettes)
  UnpackCardArt8bpp        art -> 8bpp pixels                             (gCardArt)
  IsEffectMonster, IsFusionMonster, IsSpecialSummonOnly                   (gCardStats, gCardIdToNumber)
  FindCardByPassword       each card's password typed into gPassword      (gCardPasswords)
  IsSameCardName           random pairs                                   (gCardIdToNumber)
  DeckEdit lists           BuildCardLists modes 0-2, then a sort by ATK   (stats, numbers, the moved list rows)
--clone NEW=OLD checks a new card that was made as a copy of OLD (same art, stats and kind; its own number and
password): the readers must give OLD's results, and its password must find NEW.
"""
import argparse, os, random, struct, sys
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_HOOK_INTR
from unicorn.arm_const import (UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_SP,
                               UC_ARM_REG_LR, UC_ARM_REG_PC)

STOP = 0x0E100000
F = dict(LoadPicture=0x08061D24, Unpack8bpp=0x0807AEF0, IsEffect=0x08007730, IsSpecialOnly=0x08007834,
         IsFusion=0x08007994, SameName=0x080074A0, FindPassword=0x0807C304, Build=0x080686E8,
         Get=0x08068D1C, Filter=0x08069284)
DE, SAVE, GMAIN, PASSWORD = 0x0201DB20, 0x02011C20, 0x03000040, 0x0201F7B0


class Gba:
    def __init__(self, rom):
        self.u = u = Uc(UC_ARCH_ARM, UC_MODE_THUMB)
        u.mem_map(0x08000000, 0x02000000); u.mem_write(0x08000000, rom[:0x02000000])
        for base, size in [(0x02000000, 0x40000), (0x03000000, 0x8000), (0x04000000, 0x1000),
                           (0x05000000, 0x1000), (0x06000000, 0x20000), (0x07000000, 0x1000), (STOP & ~0xFFF, 0x1000)]:
            u.mem_map(base, size)
        u.hook_add(UC_HOOK_INTR, self.swi)

    def swi(self, u, intno, _):
        pc = u.reg_read(UC_ARM_REG_PC)
        n = struct.unpack('<H', u.mem_read(pc - 2, 2))[0] & 0xFF
        src, dst, c = (u.reg_read(r) for r in (UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2))
        if n not in (0x0B, 0x0C):
            raise RuntimeError('unemulated SWI %#x' % n)
        cnt = c & 0x1FFFFF
        w = 4 if (n == 0x0C or c >> 26 & 1) else 2
        if n == 0x0C:
            cnt = (cnt + 7) & ~7
        for i in range(cnt):
            u.mem_write(dst + i * w, bytes(u.mem_read(src + (0 if c >> 24 & 1 else i * w), w)))

    def call(self, fn, *args):
        u = self.u
        for r, v in zip((UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3), args):
            u.reg_write(r, v)
        u.reg_write(UC_ARM_REG_SP, 0x03007B00); u.reg_write(UC_ARM_REG_LR, STOP | 1)
        u.emu_start(fn | 1, STOP, count=50000000)
        return u.reg_read(UC_ARM_REG_R0)

    def r(self, a, n): return bytes(self.u.mem_read(a, n))
    def w(self, a, b): self.u.mem_write(a, b)


def readers(g, rom, ids, pw_base):
    """Per-ID results of every table reader."""
    out = {}
    for i in ids:
        g.w(0x06000000, bytes(0x20000)); g.w(0x05000000, bytes(0x400))
        g.call(F['LoadPicture'], i)
        pic = g.r(0x06000000, 0x18000) + g.r(0x05000000, 0x400)
        g.w(0x02030000, bytes(0x2000))
        g.call(F['Unpack8bpp'], i, 0x02030000, 0)
        art8 = g.r(0x02030000, 0x1680)
        flags = (g.call(F['IsEffect'], i), g.call(F['IsFusion'], i), g.call(F['IsSpecialOnly'], i))
        pw = rom[pw_base - 0x08000000 + i * 4:pw_base - 0x08000000 + i * 4 + 4]
        found = None
        if pw != b'\xff' * 4:
            digits = bytes(d for b in pw for d in (b >> 4, b & 15))
            g.w(PASSWORD + 8, digits)
            found = g.call(F['FindPassword']) & 0xFFFF
        out[i] = (pic, art8, flags, found)
    return out


def lists(g, last):
    g.w(SAVE + 8, b''.join(struct.pack('<I', 1 if 1 <= i <= last else 0) for i in range(0x800)))
    res = []
    for mode in range(3):
        g.w(GMAIN + 0x4874, bytes([mode])); g.w(DE + 0x14A0, b'\0\0\0')
        g.call(F['Build'])
        n = struct.unpack('<H', g.r(DE + 0x1494, 2))[0]
        res.append([g.call(F['Get'], 0, 0, k) & 0xFFFF for k in range(n)])
    g.w(GMAIN + 0x4874, b'\0'); g.w(DE + 0x14A0, b'\0\0\0')
    g.call(F['Filter'], 0, 0, 1)
    n = struct.unpack('<H', g.r(DE + 0x149A, 2))[0]
    res.append([g.call(F['Get'], 0, 1, k) & 0xFFFF for k in range(n)])
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--orig', default='eds.gba')
    ap.add_argument('--new', default='build/card_expand/expanded.gba')
    ap.add_argument('--clone', action='append', default=[], help='NEW=OLD: new card ID made as a copy of OLD')
    ap.add_argument('--last', type=lambda s: int(s, 0), help='the LAST_ID the new ROM was built with')
    a = ap.parse_args()
    orig, new = open(a.orig, 'rb').read(), open(a.new, 'rb').read()
    # where the new ROM keeps its passwords: FindCardByPassword's literal
    pw_new = struct.unpack_from('<I', new, 0x07C35C)[0]
    pw_orig = struct.unpack_from('<I', orig, 0x07C35C)[0]   # an edited ROM can have its passwords elsewhere
    last = a.last or None
    if last is None:   # the pack generator's range literal holds LAST_ID + 1
        last = struct.unpack_from('<I', new, 0x062BC0)[0] - 1
    go, gn = Gba(orig), Gba(new)
    ids = range(1, 821)
    print('comparing the table readers for IDs 1..820 ...')
    ro, rn = readers(go, orig, ids, pw_orig), readers(gn, new, ids, pw_new)
    names = ['LoadCardPicture', 'UnpackCardArt8bpp', 'Is*Monster flags', 'FindCardByPassword']
    bad = 0
    for k, name in enumerate(names):
        diff = [i for i in ids if ro[i][k] != rn[i][k]]
        bad += bool(diff)
        print('  %-20s %s' % (name, 'identical' if not diff else 'DIFFERS for IDs %s' % diff[:10]))
    rng = random.Random(1)
    pairs = [(rng.randrange(1, 821), rng.randrange(1, 821)) for _ in range(300)] + [(83, 82), (151, 150), (553, 552)]
    diff = [p for p in pairs if go.call(F['SameName'], *p) != gn.call(F['SameName'], *p)]
    bad += bool(diff)
    print('  %-20s %s' % ('IsSameCardName', 'identical' if not diff else 'DIFFERS %s' % diff[:5]))
    lo = lists(Gba(orig), 820)
    ln = lists(Gba(new), last)
    extra = set(range(821, last + 1))
    kept = [[x for x in l if x not in extra] for l in ln]
    # list building keeps ID order; the ATK sort (QuickSortS16) is unstable, so extra cards may reorder ties.
    # An edited input ROM with more than 821 cards and the original 821-slot Deck Edit list overflows that list in
    # the Prohibition picker (which lists every card, owned or not), so mode 2 cannot be compared there.
    h1, h2 = struct.unpack_from('<HH', orig, 0x068916)                   # Deck Edit's loop bound: the last card ID
    orig_last = (h1 & 0xFF) << (h2 >> 6 & 31)
    modes = [0, 1] if orig_last > 820 else [0, 1, 2]
    same = all(kept[m] == lo[m] for m in modes) and sorted(kept[3]) == sorted(lo[3])
    bad += not same
    print('  %-20s %s' % ('Deck Edit lists', ('identical for the original cards' if same else 'DIFFER')
                          + ('' if len(modes) == 3 else f' (Prohibition picker skipped: the input has {orig_last + 1} '
                                                        'cards in an 821-slot list)')))
    print('  new IDs listed (Deck Edit / Trading / Prohibition):',
          [len([x for x in l if x in extra]) for l in ln[:3]], 'of', len(extra))

    for spec in a.clone:
        nid, oid = (int(x, 0) for x in spec.split('='))
        r_old, r_new = readers(go, orig, [oid], pw_orig)[oid], readers(gn, new, [nid], pw_new)[nid]
        # palette bit 15 is ignored by the hardware and kept per ID in art/palette_bit15.json: mask it
        unmask = lambda pic: pic[:0x18000] + bytes(b & 0x7F if k % 2 else b for k, b in enumerate(pic[0x18000:]))
        checks = [('art', unmask(r_old[0]) == unmask(r_new[0]) and r_old[1] == r_new[1]), ('flags', r_old[2] == r_new[2]),
                  ('password finds it', r_new[3] == nid)]
        bad += not all(ok for _, ok in checks)
        print('  clone %d of %d: %s' % (nid, oid, ', '.join('%s %s' % (k, 'ok' if ok else 'FAIL') for k, ok in checks)))
    print('PASS' if not bad else 'FAIL')
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
