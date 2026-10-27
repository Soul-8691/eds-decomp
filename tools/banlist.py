#!/usr/bin/env python3
"""Banlists as card numbers, for the mod's C code.

  python3 tools/banlist.py build/include/format_yugi_kaiba.h          # Tristan's format (src/mod_banlist.c)
  python3 tools/banlist.py --all build/include/banlists.h             # the selectable banlists (src/mod_banlists.c)

Inputs (repository root):
  card_pools/yugi-kaiba.txt, one card name per line (the legal card pool of Tristan's format), and banlists/yugi-kaiba_ban.json;
  banlists/<name>_ban.json and card_pools/<name>.txt, the Deck Edit banlists and their card pools, in the order
  of BANLISTS (a card outside the pool may not be in a deck).
The banlists are YGOPRODeck-style ("forbidden", "limited", "semiLimited" arrays of entries with a "cardName"; the
other arrays, such as "unlimited", list cards that came off the list and are ignored). Names are matched against the
built card names (build/assets/newcards__names.bin, original and added cards), first exactly, then ignoring case,
spacing and punctuation, then through ALIASES and the "TCG name | EDS name" lines of config/new_cards.txt. A name with no EDS card is reported and skipped: such a card cannot
be in a deck. The headers hold sorted tables of card numbers (alternate-art numbers are the same card, so only the
base number is listed).
"""
import json
import struct
import sys

POOL = 'card_pools/yugi-kaiba.txt'
BANLIST = 'banlists/yugi-kaiba_ban.json'
NAMES = 'build/assets/newcards__names.bin'
ID_TO_NUMBER = 'build/assets/newcards__id_to_number.bin'
NAME_SIZE = 0x40
ALT_ART = 2000
# TCG name -> EDS name, where the two differ beyond case and punctuation.
ALIASES = {
    'Red-Eyes Black Dragon': 'Red-Eyes B. Dragon',
    'Ring of Destruction': 'Bell of Destruction',  # not in the game; its stand-in takes its limit
    'Black Skull Dragon': 'B. Skull Dragon',
    'Kuwagata α': 'Kuwagata a',
}
LIMITS = {'forbidden': 0, 'limited': 1, 'semiLimited': 2}
# The selectable banlists: (menu name, banlist file stem, card pool file stems: the pool is their union).
BANLISTS = [
    ('Yugi-Kaiba', 'yugi-kaiba', ['yugi-kaiba']),
    ('Critter', 'critter', ['critter']),
    ('Treasure', 'treasure', ['treasure']),
    ('Imperial', 'imperial', ['imperial']),
    ('Android', 'android', ['android']),
    ('Joey-Pegasus', 'joey-pegasus', ['joey-pegasus']),
    ('Fiber', 'fiber', ['fiber']),
    ('Yata', 'yata', ['yata']),
    ('Scientist', 'scientist', ['scientist']),
    ('Vampire', 'vampire', ['vampire']),
    ('Chaos', 'chaos', ['chaos']),
    ('Warrior', 'warrior', ['warrior']),
    ('Goat', 'goat', ['goat']),
    ('Cyber', 'cyber', ['cyber']),     # the same Forbidden/Limited list as Goat (April 2005), a larger pool
    ('Reaper', 'reaper', ['reaper']),
]
DEFAULT_BANLIST = 'Yugi-Kaiba'  # new games start with it


def norm(s):
    return ''.join(c for c in s.lower() if c.isalnum())


def new_card_aliases():
    """The "TCG name | EDS name" lines of config/new_cards.txt (added cards shown under a shorter name)."""
    aliases = {}
    for line in open('config/new_cards.txt', encoding='utf-8'):
        line = line.strip()
        if line and not line.startswith('#') and '|' in line:
            tcg, eds = (part.strip() for part in line.split('|', 1))
            aliases[tcg] = eds
    return aliases


def card_lookup():
    """A function from a card name to its EDS card numbers (None when there is no such card). A card can have more
    than one number (the two Polymerizations); alternate-art numbers are folded into the base number."""
    aliases = dict(ALIASES, **new_card_aliases())
    names = open(NAMES, 'rb').read()
    i2n = open(ID_TO_NUMBER, 'rb').read()
    exact, folded = {}, {}
    for cid in range(1, len(i2n) // 2):
        name = names[cid * NAME_SIZE:(cid + 1) * NAME_SIZE].split(b'\0')[0].decode('latin-1')
        number = struct.unpack_from('<H', i2n, cid * 2)[0]
        if number == 0xFFFF or not name:
            continue
        if number >= ALT_ART:
            number -= ALT_ART
        exact.setdefault(name, set()).add(number)
        folded.setdefault(norm(name), set()).add(number)

    def find(name):
        return exact.get(name, folded.get(norm(name)))

    def lookup(name):
        # The name's own card and its alias's: both count, should a TCG name match an added card as well as its
        # EDS stand-in.
        numbers = set()
        for n in (name, aliases.get(name)):
            if n is not None and find(n):
                numbers |= find(n)
        return numbers or None
    return lookup


def read_limits(path, lookup, missing):
    """The banlist file's {card number: copies allowed}, and its "id" (the date)."""
    ban = json.load(open(path, encoding='utf-8'))
    limits = {}
    for key, limit in LIMITS.items():
        for entry in ban.get(key, []):
            numbers = lookup(entry['cardName'])
            if numbers is None:
                missing.append(entry['cardName'])
            else:
                for number in numbers:
                    limits[number] = limit
    return limits, ban.get('id', '')


def write_if_changed(out, data):
    try:
        if open(out, encoding='utf-8').read() == data:
            return
    except FileNotFoundError:
        pass
    open(out, 'w', encoding='utf-8').write(data)


def report(missing):
    for name in sorted(set(missing)):
        print(f'banlist: no EDS card named "{name}" (skipped)', file=sys.stderr)


def read_pool(path, lookup, missing):
    """The card numbers of a card pool file (one card name per line)."""
    pool = set()
    for line in open(path, encoding='utf-8'):
        name = line.strip()
        if not name:
            continue
        numbers = lookup(name)
        if numbers is None:
            missing.append(name)
        else:
            pool |= numbers
    return pool


def format_header(out):
    lookup = card_lookup()
    missing = []
    pool = read_pool(POOL, lookup, missing)
    limits, date = read_limits(BANLIST, lookup, missing)
    report(missing)

    lines = ['/* Generated by tools/banlist.py from yugi-kaiba.txt and yugi-kaiba_ban.json. Do not edit. */',
             f'#define FORMAT_NAME "{date}"',
             f'#define FORMAT_POOL_COUNT {len(pool)}',
             f'#define FORMAT_LIMIT_COUNT {len(limits)}',
             '/* The legal card pool, sorted card numbers. */',
             'static const u16 sFormatPool[FORMAT_POOL_COUNT] = {']
    nums = sorted(pool)
    for i in range(0, len(nums), 12):
        lines.append('    ' + ', '.join(str(n) for n in nums[i:i + 12]) + ',')
    lines += ['};', '/* The Forbidden/Limited list: { card number, copies allowed }. */',
              'static const struct FormatLimit sFormatLimits[FORMAT_LIMIT_COUNT] = {']
    lines += [f'    {{ {n}, {l} }},' for n, l in sorted(limits.items())]
    lines += ['};', '']
    write_if_changed(out, '\n'.join(lines))


def banlists_header(out):
    lookup = card_lookup()
    missing = []
    pool_missing = []
    lines = ['/* Generated by tools/banlist.py --all from banlists/*_ban.json and card_pools/*.txt. Do not edit. */',
             f'#define BANLIST_COUNT {len(BANLISTS)}',
             f'#define BANLIST_DEFAULT {[b[0] for b in BANLISTS].index(DEFAULT_BANLIST)}   /* {DEFAULT_BANLIST} */', '']
    rows = []
    for i, (name, stem, pools) in enumerate(BANLISTS):
        limits, date = read_limits(f'banlists/{stem}_ban.json', lookup, missing)
        pool = set()
        for pool_stem in pools:
            pool |= read_pool(f'card_pools/{pool_stem}.txt', lookup, pool_missing)
        counts = [sum(1 for l in limits.values() if l == k) for k in range(3)]
        lines.append(f'/* {name} ({date}) */')
        lines.append(f'static const struct FormatLimit sBanlist{i}[] = {{')
        lines += [f'    {{ {n}, {l} }},' for n, l in sorted(limits.items())]
        lines += ['};', f'/* {name}: the card pool, sorted card numbers. */',
                  f'static const u16 sBanlistPool{i}[] = {{']
        nums = sorted(pool)
        for k in range(0, len(nums), 16):
            lines.append('    ' + ', '.join(str(n) for n in nums[k:k + 16]) + ',')
        lines += ['};', '']
        rows.append(f'    {{ "{name}", "{date}", sBanlist{i}, {len(limits)}, {{ {counts[0]}, {counts[1]}, {counts[2]} }}, '
                    f'sBanlistPool{i}, {len(nums)} }},')
    lines.append('static const struct Banlist sBanlists[BANLIST_COUNT] = {')
    lines += rows
    lines += ['};', '']
    report(missing)
    # The pools name every real card of their time; most are not in EDS, so only the count is reported.
    print(f'banlist: {len(set(pool_missing))} card pool names have no EDS card (not playable)', file=sys.stderr)
    write_if_changed(out, '\n'.join(lines))


if __name__ == '__main__':
    if sys.argv[1] == '--all':
        banlists_header(sys.argv[2])
    else:
        format_header(sys.argv[1])
