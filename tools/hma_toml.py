#!/usr/bin/env python3
"""Write the Hex Maniac Advance metadata (a .toml next to the ROM) for the demo build, from baserom.toml.

  python3 tools/hma_toml.py [--rom eds_demo.gba] [--sym eds_demo.sym] [--base baserom.toml] [--out eds_demo.toml]

baserom.toml (repo root, not committed) is HMA's metadata for the original ROM. Run by `make demo` after the ROM is
linked; it does nothing without baserom.toml. For every entry it finds what the original address was in the
source of this repo, and where that thing is in the demo ROM:

  NamedAnchors  the original address lies in a data label of data/*.s (`Name: @ 0x08xxxxxx`, its extent up to the
                next such label) or in a card table that moved to the end of the ROM (config/assets.tsv gives the
                original range of cards/art, stats.csv, id_to_number.json, passwords.csv, descriptions.json;
                MOVED_TABLES names their new symbols). The new address is that symbol's address in the .sym plus the
                same offset. A table's length that counts cards follows the demo: data.cards.numbers (by card ID)
                gets CARD_ID_COUNT, data.cards.internalnumbers (by card number) the highest card number in use + 1.
  MatchedWords  the original address lies in a function (config/functions.tsv); the function is found in the .sym
                and the constant in its demo code: the same instruction at the same offset when the function did
                not change, else the same instruction kind with the same immediate (closest to the old offset), or
                for the card count limits a literal holding the demo's value (Length 2). A site whose function no
                longer has the constant (rewritten without it) is left out, listed in a comment. Data constants
                (in a data label) move like anchors. The original instruction comes from baserom.gba (read only).
  Added         anchors baserom.toml lacks, by symbol (EXTRA_ANCHORS): every card's art icon (Deck Edit's ring art).
  Lists         cardnumbers (by card ID) and cardinternalnumbers (by card number) get the added cards
                (build/new_cards.tsv: their in-game names at their IDs and numbers); the other lists are kept.
  DefaultHash   recomputed (HMA's StoredList.GenerateHash) for every format and list this changes.

Anything not found is kept as it was and reported.
"""
import argparse
import bisect
import glob
import hashlib
import os
import re
import struct
import sys

ROM_BASE = 0x08000000
ORIGINAL_CARD_IDS = 821
ART_SIZE, PAL_SIZE = 0x10E0, 0x80
# config/assets.tsv path -> the symbol of the moved table (data/rodata_08087FD0.s, "The card tables with the added
# cards"); cards/art is the gfx of every card, then their palettes.
MOVED_TABLES = {
    'cards/descriptions.json': 'gCardDescriptions',
    'cards/stats.csv': 'gCardStats',
    'cards/id_to_number.json': 'gCardIdToNumber',
    'cards/passwords.csv': 'gCardPasswords',
}
CARD_LIMIT_WORDS = ('data.constants.cards.limit', 'data.constants.cards.limit2')
# Anchors baserom.toml doesn't have, added by symbol (their addresses come from the .sym): (name, symbol, offset
# from it given the card count, format), in HMA's formats as baserom.toml uses them.
#   Every card's own art icon, Deck Edit's ring art (tools/cardicons.py; gModFieldIconData's DATA_DECK_EDIT block in
#   src/mod_field_icons.c), after the field icons' palette, frames, pose maps and per-card 15x16 miniatures (those are
#   8bpp and untiled, which HMA's sprite formats can't show): two arrays by card ID, every 16-colour palette (0x20
#   bytes, entry 0 transparent) then every 16x16 4bpp image (2x2 row-major tiles, 0x80 bytes). They are HMA-CE's
#   inline image tables (ArrayRun.TryParseInlineImageTable: a row is one bare `ucp`/`ucs` format and nothing
#   else), as graphics.cards.images / graphics.cards.palettes; the images' palette hint is the palettes table, whose
#   row of the same index HMA uses.
FIELD_ICON_MINIS = 0x7980
MINI_SIZE = 15 * 16
ICON_PALETTE_SIZE = 0x20


def icon_palettes(count):
    return (FIELD_ICON_MINIS + count * MINI_SIZE + 3) & ~3


EXTRA_ANCHORS = [
    ('graphics.cards.icons.palettes', 'gModFieldIconData', icon_palettes, '[`ucp4`]cardnumbers'),
    ('graphics.cards.icons', 'gModFieldIconData', lambda count: icon_palettes(count) + count * ICON_PALETTE_SIZE,
     '[`ucs4x2x2|graphics.cards.icons.palettes`]cardnumbers'),
]


def gen_hash(items, seed=0xCAFEBABE):
    """HMA's StoredList.GenerateHash (src/HexManiac.Core/Models/StoredMetadata.cs)."""
    h = seed
    for item in items:
        h = ((h << 3) | (h >> 29)) & 0xFFFFFFFF
        if item is None:
            continue
        h ^= struct.unpack('<I', hashlib.sha256(item.encode('utf-8')).digest()[:4])[0]
    return '%08X' % h


# ---- inputs ----

def load_sym(path):
    sym = {}
    for line in open(path):
        p = line.split()
        if len(p) >= 4:
            sym[p[3]] = (int(p[0], 16), int(p[2], 16))
    return sym


def data_labels():
    """(original address, symbol) of every data label with an address comment, sorted."""
    labels = {}
    for path in glob.glob('data/*.s'):
        for line in open(path, encoding='utf-8', errors='replace'):
            m = re.match(r'(\w+):\s*@\s*0x([0-9A-Fa-f]{8})', line)
            if m:
                labels[int(m.group(2), 16)] = m.group(1)
    return sorted(labels.items())


def moved_ranges():
    """(original start, original end, symbol) of the card tables that moved, from config/assets.tsv."""
    out = []
    for line in open('config/assets.tsv'):
        p = line.split()
        if len(p) < 4 or line.startswith('#'):
            continue
        start, end, path = int(p[0], 16), int(p[1], 16), p[3]
        if path in MOVED_TABLES:
            out.append((start, end, MOVED_TABLES[path]))
        elif path == 'cards/art':
            split = start + ORIGINAL_CARD_IDS * ART_SIZE
            out.append((start, split, 'gCardArtGfx'))
            out.append((split, end, 'gCardArtPalettes'))
    return out


def functions():
    out = []
    for line in open('config/functions.tsv'):
        if line.startswith('#') or not line.strip():
            continue
        p = line.split('\t')
        out.append((int(p[0], 16), int(p[2], 16), p[3]))
    out.sort()
    return out


def card_count():
    text = open('build/include/card_count.h').read()
    return ORIGINAL_CARD_IDS + int(re.search(r'CARD_ID_COUNT_ADDED (\d+)', text).group(1))


def added_cards():
    """(id, number, in-game name) of the added cards."""
    out = []
    lines = open('build/new_cards.tsv', encoding='utf-8').read().splitlines()
    for line in lines[1:]:
        p = line.split('\t')
        if len(p) >= 5:
            out.append((int(p[0]), int(p[1]), p[4] or p[3]))
    return out


def highest_number_in_use():
    data = open('build/assets/newcards__number_to_id.bin', 'rb').read()
    ids = struct.unpack(f'<{len(data) // 2}H', data)
    last = max(i for i, v in enumerate(ids) if v)
    return last


# ---- the TOML as blocks ----

def split_blocks(text):
    """[(header, lines)]; the text before the first header is block 0 with header ''."""
    blocks, header, lines = [], '', []
    for line in text.splitlines():
        if line.startswith('[') and not line.startswith('[ ') and line.rstrip().endswith(']') and '=' not in line:
            blocks.append((header, lines))
            header, lines = line.strip(), [line]
        else:
            lines.append(line)
    blocks.append((header, lines))
    return blocks


def field(lines, key):
    for line in lines:
        m = re.match(rf"{key} = (?:'''(.*)'''|(0x[0-9A-Fa-f]+|\d+))\s*$", line)
        if m:
            return m.group(1) if m.group(1) is not None else m.group(2)
    return None


def set_field(lines, key, value):
    for i, line in enumerate(lines):
        if line.startswith(f'{key} = '):
            lines[i] = f'{key} = {value}'
            return
    lines.append(f'{key} = {value}')


def list_items(lines):
    """The items of a [[List]] block (one 0 = [ ... ] run) and the index range of those lines."""
    start = next(i for i, l in enumerate(lines) if re.match(r'\d+ = \[', l))
    end = next(i for i in range(start, len(lines)) if lines[i].strip() == ']')
    items = [re.match(r"\s*'''(.*)''',?\s*$", l).group(1) for l in lines[start + 1:end]]
    return items, start, end


def set_list_items(lines, items):
    _, start, end = list_items(lines)
    lines[start + 1:end] = ["   '''%s'''," % s.replace("'''", "'") for s in items]
    set_field(lines, 'DefaultHash', "'''%s'''" % gen_hash(items))


# ---- moving things ----

class Mover:
    def __init__(self, sym, rom_old, rom_new):
        self.sym, self.old, self.new = sym, rom_old, rom_new
        self.labels = data_labels()
        self.label_addrs = [a for a, _ in self.labels]
        self.moved = moved_ranges()
        self.funcs = functions()
        self.func_addrs = [a for a, _, _ in self.funcs]

    def data_symbol(self, addr):
        """(symbol, offset) of an original ROM address in the data, or None."""
        for start, end, name in self.moved:
            if start <= addr < end:
                return name, addr - start
        i = bisect.bisect_right(self.label_addrs, addr) - 1
        if i < 0:
            return None
        start, name = self.labels[i]
        return name, addr - start

    def function(self, addr):
        i = bisect.bisect_right(self.func_addrs, addr) - 1
        if i >= 0 and addr < self.funcs[i][0] + self.funcs[i][1]:
            return self.funcs[i]
        return None

    def move_data(self, offset):
        """The demo file offset of an original file offset in the data, or None."""
        found = self.data_symbol(ROM_BASE + offset)
        if found is None or found[0] not in self.sym:
            return None
        name, delta = found
        return self.sym[name][0] - ROM_BASE + delta

    def move_word(self, name, offset, length, new_value):
        """(new offset, length) of a MatchedWords site in code, or None (with a reason)."""
        f = self.function(ROM_BASE + offset)
        if f is None:
            new = self.move_data(offset)
            return (new, length) if new is not None else (None, 'not in a data label or a function')
        fstart, fsize, fname = f
        if fname not in self.sym:
            return None, f'{fname} is not in the demo ROM'
        nstart, nsize = self.sym[fname]
        nstart -= ROM_BASE
        rel = offset - (fstart - ROM_BASE)
        code = self.new[nstart:nstart + nsize]
        if self.old is None:
            return None, 'no baserom.gba to read the original instruction from'
        old_code = self.old[fstart - ROM_BASE:fstart - ROM_BASE + fsize]
        hw_at = rel & ~1
        old_hw = old_code[hw_at:hw_at + 2]
        if name in CARD_LIMIT_WORDS:
            # the count limits: a literal (or a halfword) with the demo's value; the original byte form
            # (movs #0xCD; lsls #2) cannot hold it
            old_value = int.from_bytes(self.old[offset:offset + length], 'little') if length == 2 else old_code[rel] << 2
            want = old_value + new_value
            hits = [i for i in range(0, len(code) - 1, 2) if struct.unpack_from('<H', code, i)[0] == want]
            if not hits:
                return None, f'{fname} no longer holds the card count limit'
            best = min(hits, key=lambda i: abs(i - rel * nsize // fsize))
            return (nstart + best, 2)
        # an unchanged function: the same instruction at the same offset
        if nsize == fsize and code[hw_at:hw_at + 2] == old_hw:
            return (nstart + rel, length)
        # else the same opcode bits and immediate (another register is fine), the closest to the old offset
        hits = [i for i in range(0, len(code) - 1, 2)
                if code[i] == old_hw[0] and (code[i + 1] & 0xF8) == (old_hw[1] & 0xF8)]
        if not hits:
            return None, f'{fname} no longer has the instruction'
        best = min(hits, key=lambda i: abs(i - rel * nsize // fsize))
        return (nstart + best + (rel - hw_at), length)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--rom', default='eds_demo.gba')
    ap.add_argument('--sym', default='eds_demo.sym')
    ap.add_argument('--base', default='baserom.toml')
    ap.add_argument('--baserom', default='baserom.gba')
    ap.add_argument('--out', default=None)
    a = ap.parse_args()
    if not os.path.exists(a.base):
        print(f'hma_toml: no {a.base}; skipped')
        return
    out = a.out or os.path.splitext(a.rom)[0] + '.toml'
    sym = load_sym(a.sym)
    new_rom = open(a.rom, 'rb').read()
    old_rom = open(a.baserom, 'rb').read() if os.path.exists(a.baserom) else None
    mover = Mover(sym, old_rom, new_rom)
    count = card_count()
    numbers = highest_number_in_use() + 1
    blocks = split_blocks(open(a.base, encoding='utf-8').read())
    report, dropped = [], []

    # MatchedWords: the card count limits go from 820/821 to CARD_ID_COUNT - 1 / CARD_ID_COUNT
    limit_delta = count - ORIGINAL_CARD_IDS
    keep = []
    for header, lines in blocks:
        if header == '[[MatchedWords]]':
            name, addr, length = field(lines, 'Name'), int(field(lines, 'Address'), 16), int(field(lines, 'Length'))
            new, info = mover.move_word(name, addr, length, limit_delta)
            if new is None:
                dropped.append(f'#   {name} at 0x{addr:06X}: {info}')
                continue
            set_field(lines, 'Address', f'0x{new:06X}')
            set_field(lines, 'Length', str(info))
        elif header == '[[NamedAnchors]]':
            name, addr, fmt = field(lines, 'Name'), int(field(lines, 'Address'), 16), field(lines, 'Format')
            new = mover.move_data(addr)
            if new is None:
                report.append(f'anchor {name} (0x{addr:06X}): not found, kept')
            else:
                set_field(lines, 'Address', f'0x{new:06X}')
            new_fmt = fmt
            if name == 'data.cards.numbers':
                new_fmt = re.sub(r'\](\d+)$', f']{count}', fmt)
            elif name == 'data.cards.internalnumbers':
                new_fmt = re.sub(r'\](\d+)$', f']{numbers}', fmt)
            if new_fmt != fmt:
                set_field(lines, 'Format', "'''%s'''" % new_fmt)
                set_field(lines, 'DefaultHash', "'''%s'''" % gen_hash([new_fmt]))
        elif header == '[[List]]':
            name = field(lines, 'Name')
            items, _, _ = list_items(lines)
            if name == 'cardnumbers':
                items = items[:ORIGINAL_CARD_IDS] + ['None'] * max(0, count - len(items))
                for cid, _, game_name in added_cards():
                    items[cid] = game_name
                set_list_items(lines, items[:count])
            elif name == 'cardinternalnumbers':
                for _, number, game_name in added_cards():
                    while len(items) <= number:
                        items.append('None')
                    items[number] = game_name
                set_list_items(lines, items)
        keep.append((header, lines))

    # the anchors baserom.toml lacks, after its last anchor
    names = {field(lines, 'Name') for header, lines in keep if header == '[[NamedAnchors]]'}
    extra = []
    for name, symbol, offset, fmt in EXTRA_ANCHORS:
        if name in names:
            continue
        if symbol not in sym:
            report.append(f'extra anchor {name}: no {symbol} in {a.sym}, left out')
            continue
        extra.append(('[[NamedAnchors]]', ['[[NamedAnchors]]', f"Name = '''{name}'''",
                                           f'Address = 0x{sym[symbol][0] - ROM_BASE + offset(count):06X}',
                                           f"Format = '''{fmt}'''", f"DefaultHash = '''{gen_hash([fmt])}'''", '']))
    last = max(i for i, (header, _) in enumerate(keep) if header == '[[NamedAnchors]]')
    keep[last + 1:last + 1] = extra

    text = []
    for header, lines in keep:
        text.extend(lines)
    head = ['# Generated by tools/hma_toml.py from baserom.toml for %s (%d card IDs, card numbers 0-%d). Do not edit;'
            % (os.path.basename(a.rom), count, numbers - 1),
            '# it is written again by every `make demo`.']
    if dropped:
        head.append('# MatchedWords left out (their code no longer holds the constant):')
        head.extend(dropped)
    with open(out, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(head + text) + '\n')
    for r in report:
        print('hma_toml: ' + r)
    print(f'hma_toml: {out} ({count} card IDs, {numbers} card numbers; {len(dropped)} matched word(s) left out)')


if __name__ == '__main__':
    main()
