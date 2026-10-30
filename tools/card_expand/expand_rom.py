#!/usr/bin/env python3
"""Expand a binary ROM: repoint its card tables, add every card with leftover effect code at its own card number,
then add the cards listed in new_cards.txt.

  python3 tools/card_expand/expand_rom.py [LAST_ID] --rom eds.gba --out build/card_expand/expanded.gba
          [--list new_cards.txt] [--new-dir assets/new_cards] [--offline] [--refresh] [--no-leftovers]
          [--src-TABLE ADDR ...] [--TABLE ADDR ...] [--no-code] [--move-scratch] [--fill 0xFF] [--force]

Run from the repo root. Needs Pillow, and network access the first time (the card data is then cached).
tools/card_expand/run_expand.sh is a run script with every offset spelled out.

The card tables. TABLE is one of names, descs, art, palettes, stats, id2n, passwords, n2id:
  table      entry       original address   default destination
  names      0x40        0x0822C720         in place (grows into the unused name bank: up to ID 1641)
  descs      0x1E0       0x082461A0         in place (grows into the old art)
  art        0x10E0      0x082A6500         the first free space at or after 0x08800000 (--base is an alias)
  palettes   0x80        0x08608360         right after the art
  stats      4           0x08621DE0         the old palettes (--src-palettes)
  passwords  4           0x08623120         right after the stats
  id2n       2           0x08622AB4         in place (grows into the old passwords)
  n2id       2 (2048)    0x08623DF4         in place
--src-TABLE says where the table is in the input ROM (an edited ROM may have moved it); --TABLE says where to
write it. The 821 original entries (IDs 0..820) are read from the source addresses, so this needs nothing from
`make setup`, and every code pointer to a table is moved from its source to its destination. Each pointer is first
checked: it must point at its table's source address (plus its offset into the table), as the original code does.
A ROM whose editor moved a table works when --src-TABLE gives the new address. Pointers that point elsewhere
stop the run (--force patches the others and leaves those alone).

Edited ROMs: any ROM of 8 to 32 MiB whose card code is the original one works; its SHA-1 is only reported. The
sites card_expand.py patches (the Deck Edit list, the card count) must hold their original bytes, otherwise the
run stops: that is a ROM that is already expanded or whose code was changed there. The new tables are written
only over the old tables and over blank space (runs of 0x00 or 0xFF). Anything else in the way stops the run
with the address, so data an edit added (for example after 0x08800000) is not overwritten; move a table with
--TABLE, or pass --force. The default art address is the first address at or after 0x08800000 after which the
ROM is blank, so data appended to an edited ROM is kept.

The game has duel code for card numbers that no EDS card uses (LEFTOVER_EFFECTS.md). LEFTOVER below gives each of
those numbers its real card, so the effect works in a duel: the card IDs after 820 go to these cards in card
number order, each with its own card number. Their stats, text, password and artwork come from YGOPRODeck through
write_tables.py's code, exactly like a line of new_cards.txt (same cache in --new-dir, same encodings, same art
conversion; replace --new-dir/art/PASSCODE.png to use other art). The list includes the Fusion materials, ritual
monster and Tribute monsters other leftover cards name, and the three tokens their effects summon (card numbers
1921-1923, which DUEL_CMD_SUMMON_TOKEN looks up). It leaves out 230 (only an AI hint) and the Dark Magician and
Harpie Lady variants 1210 and 1249 (no effect of their own). The identifications come from the behaviour in the
code and the OCG release sets; see LEFTOVER_EFFECTS.md. --no-leftovers skips them.

The cards of new_cards.txt follow, with the same syntax and behaviour as in write_tables.py. A listed card that is
already a leftover card is skipped (it keeps its leftover number and effect), and the leftover numbers are never
handed out to listed cards. The list of every added card (ID, card number, password, stats) is written next to
the output ROM as <out>_cards.csv.

LAST_ID, the placeholders, --move-scratch and --fill work as in write_tables.py. --no-code leaves the code alone.
"""
import argparse, csv, hashlib, importlib.util, os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


wt = load('write_tables', os.path.join(HERE, 'write_tables.py'))
ROM = wt.ROM
ORIGINAL_SHA1 = '510fbba212aca9bab95ea12f8fd933e62ee34dea'     # USA AY5E
ORIGINAL_CARDS = 821                                              # IDs 0..820
ART_MIN = 0x08800000                                              # the end of the original ROM

# option name: (key in the card dict, entry size, original address, table name in card_table_refs.csv)
TABLES = {
    'names': ('names', wt.NAME_SIZE, 0x0822C720, 'gCardNames'),
    'descs': ('descs', wt.DESC_SIZE, 0x082461A0, 'gCardDescriptions'),
    'art': ('art', wt.ART_SIZE, 0x082A6500, 'gCardArt'),
    'palettes': ('pal', wt.PAL_SIZE, 0x08608360, 'gCardArtPalettes'),
    'stats': ('stats', 4, 0x08621DE0, 'gCardStats'),
    'id2n': ('id2n', 2, 0x08622AB4, 'gCardIdToNumber'),
    'passwords': ('pw', 4, 0x08623120, 'gCardPasswords'),
    'n2id': ('n2id', 2, 0x08623DF4, 'gCardNumberToId'),
}
END_POINTER = 0x08063D98      # IsCardCollectionComplete: &gCardIdToNumber[LAST_ID + 1]
SKIP_SITES = {0x0806D004}     # DeckStats_Init: &gCardIdToNumber[0x439], an out-of-range read; left as it is

# Card number -> YGOPRODeck name, [settings as in new_cards.txt]. LEFTOVER_EFFECTS.md has each effect and its status.
TOKEN = {'level': '1', 'password': 'none'}
LEFTOVER = [
    (1211, 'Thousand Knives'), (1212, 'Dark Magic Curtain'), (1213, 'Mystic Box'), (1214, 'Magic Cylinder'),
    (1220, 'Soul Exchange'), (1221, 'Card Destruction'), (1230, "Nightmare's Steelcage"), (1231, 'Rocket Warrior'),
    (1232, 'Multiply'), (1240, 'Berfomet'), (1241, 'Chimera the Flying Mythical Beast'),
    (1242, 'Archfiend of Gilfer'), (1243, 'Ancient Lamp'), (1244, 'Shadow Spell'), (1245, 'Scapegoat'),
    (1246, 'Copycat'), (1247, 'Mystical Refpanel'), (1248, 'Makiu, the Magical Mist'),
    (1250, 'Swift Gaia the Fierce Knight'), (1251, 'Obnoxious Celtic Guard'), (1252, 'Fairy King Truesdale'),
    (1253, 'Cipher Soldier'), (1254, 'Slate Warrior'), (1255, 'Electromagnetic Bagworm'), (1256, 'Rigorous Reaver'),
    (1257, 'Masked Beast Des Gardius'), (1258, 'The Mask of Remnants'),
    (1301, 'Michizure'), (1302, 'Minor Goblin Official'), (1303, 'Gamble'), (1304, 'Attack and Receive'),
    (1305, 'Solemn Wishes'), (1306, 'Skull Invitation'), (1307, 'Bubonic Vermin'), (1311, 'Burning Land'),
    (1312, 'Cold Wave'), (1313, 'Fairy Meteor Crush'), (1314, 'Limiter Removal'), (1315, 'Rain of Mercy'),
    (1316, 'Monster Recovery'), (1317, 'Shift'), (1318, 'Insect Imitation'),
    (1319, 'Interdimensional Matter Transporter'), (1320, 'Ground Collapse'), (1321, 'Magic Drain'),
    (1322, 'Infinite Dismissal'), (1323, 'Gravity Bind'), (1324, 'Type Zero Magic Crusher'),
    (1325, 'Shadow of Eyes'), (1326, 'The Legendary Fisherman'), (1327, 'Sword Hunter'), (1328, 'Drill Bug'),
    (1329, 'Deepsea Warrior'), (1330, 'Bite Shoes'), (1332, 'Invitation to a Dark Sleep'),
    (1333, 'Thousand-Eyes Idol'), (1334, 'Thousand-Eyes Restrict'), (1336, 'Hayabusa Knight'),
    (1337, 'Bombardment Beetle'), (1338, '4-Starred Ladybug of Doom'), (1339, 'Gradius'), (1340, 'Vampire Baby'),
    (1341, 'Mad Sword Beast'), (1344, 'Goblin Attack Force'), (1350, 'The Fiend Megacyber'),
    (1351, 'Gearfried the Iron Knight'), (1352, 'Insect Barrier'),
    (1402, 'Humanoid Slime'), (1403, 'Worm Drake'), (1404, 'Humanoid Worm Drake'), (1405, 'Revival Jam'),
    (1410, 'Grand Tiki Elder'), (1411, 'The Masked Beast'), (1412, 'Melchid the Four-Face Beast'),
    (1413, 'Nuvia the Wicked'), (1414, 'Chosen One'), (1415, 'Mask of Weakness'),
    (1416, 'Curse of the Masked Beast'), (1417, 'Mask of Dispel'), (1418, 'Mask of Restrict'),
    (1419, 'Mask of the Accursed'), (1420, 'Mask of Brutality'), (1421, 'Return of the Doomed'),
    (1422, 'Lightning Blade'), (1423, 'Tornado Wall'), (1424, 'Fairy Box'), (1425, 'Torrential Tribute'),
    (1426, 'Jam Breeding Machine'), (1427, 'Infinite Cards'), (1428, 'Jam Defender'), (1429, 'Card of Safe Return'),
    (1430, 'Lady Panther'), (1431, 'The Unfriendly Amazon'), (1432, 'Amazoness Archer'), (1433, 'Crimson Sentry'),
    (1434, 'Fire Princess'), (1435, 'Lady Assailant of Flames'), (1436, 'Fire Sorcerer'),
    (1437, 'Spirit of the Breeze'), (1438, 'Dancing Fairy'), (1439, 'Fairy Guardian'), (1441, 'Cure Mermaid'),
    (1442, 'Hysteric Fairy'), (1444, 'The Forgiving Maiden'), (1445, 'St. Joan'), (1446, 'Darklord Marie'),
    (1447, 'Jar of Greed'), (1448, 'Scroll of Bewitchment'), (1449, 'United We Stand'), (1450, 'Mage Power'),
    (1451, 'Offerings to the Doomed'),
    (1510, 'Jowgen the Spiritualist'), (1511, 'Kycoo the Ghost Destroyer'), (1512, 'Summoner of Illusions'),
    (1513, 'Bazoo the Soul-Eater'), (1514, 'Dark Necrofear'), (1515, 'Soul of Purity and Light'),
    (1516, 'Spirit of Flames'), (1517, 'Aqua Spirit'), (1518, 'The Rock Spirit'), (1519, 'Garuda the Wind Spirit'),
    (1520, 'Gilasaurus'), (1521, 'Tornado Bird'), (1522, 'Dreamsprite'), (1523, 'Zombyra the Dark'),
    (1524, 'Supply'), (1525, 'Maryokutai'), (1526, 'The Last Warrior from Another Planet'),
    (1527, 'Collected Power'), (1528, 'Destiny Board'), (1529, 'Dark Spirit of the Silent'),
    (1530, 'Royal Command'), (1531, 'Riryoku Field'), (1532, 'Skull Lair'), (1533, "Graverobber's Retribution"),
    (1534, 'Deal of Phantom'), (1535, 'Destruction Punch'), (1536, 'Blind Destruction'),
    (1537, "The Emperor's Holiday"), (1538, 'The Dark Door'), (1539, 'Spiritualism'), (1540, 'Cyclon Laser'),
    # placed by Destiny Board in this order (PlaceNextSpiritMessage), spelling F-I-N-A-L with the board
    (1541, 'Spirit Message "I"'), (1542, 'Spirit Message "N"'), (1543, 'Spirit Message "A"'),
    (1544, 'Spirit Message "L"'),
    (1545, 'Bait Doll'), (1546, 'De-Fusion'), (1547, 'Fusion Gate'), (1548, 'Ekibyo Drakmord'),
    (1549, 'Miracle Dig'), (1550, 'Dragonic Attack'), (1551, 'Spirit Elimination'), (1552, 'Vengeful Bog Spirit'),
    # tokens: card number 1920 + DUEL_CMD_SUMMON_TOKEN's kind (Multiply, Scapegoat, Jam Breeding Machine)
    (1921, 'Kuriboh Token', dict(TOKEN, race='Fiend', attr='DARK', atk='300', **{'def': '200'})),
    (1922, 'Sheep Token', dict(TOKEN, race='Beast', attr='EARTH', atk='0', **{'def': '0'})),
    (1923, 'Slime Token', dict(TOKEN, race='Aqua', attr='WATER', atk='500', **{'def': '500'})),
]
LEFTOVER = [(e[0], e[1], e[2] if len(e) > 2 else {}) for e in LEFTOVER]


def die(msg):
    sys.exit('expand_rom: ' + msg)


def addr(s):
    v = int(s, 0)
    if not ROM <= v < ROM + 0x2000000:
        raise argparse.ArgumentTypeError(f'{s} is not a ROM address (0x08000000-0x09FFFFFF)')
    return v


def word_at(rom, a):
    return struct.unpack_from('<I', rom, a - ROM)[0]


# ------------------------------------------------------------------------------------------- input ROM
def read_rom_tables(rom, src):
    """The 821 original entries of every card table, as raw bytes per ID, read at the source addresses."""
    n = ORIGINAL_CARDS
    for opt, (key, size, _, _) in TABLES.items():
        length = 2048 * 2 if opt == 'n2id' else n * size
        if src[opt] + length > ROM + len(rom):
            die(f'--src-{opt} 0x{src[opt]:08X}: the table would run past the end of the ROM')
    rows = lambda opt: [bytes(rom[src[opt] - ROM + i * TABLES[opt][1]:src[opt] - ROM + (i + 1) * TABLES[opt][1]])
                        for i in range(n)]
    t = {TABLES[o][0]: rows(o) for o in ('names', 'descs', 'stats', 'passwords', 'art', 'palettes')}
    t['id2n'] = list(struct.unpack_from(f'<{n}H', rom, src['id2n'] - ROM))
    t['n2id'] = list(struct.unpack_from('<2048H', rom, src['n2id'] - ROM))
    t['text'] = [r.split(b'\0')[0].decode('cp1252', 'replace') for r in t['names']]
    if t['id2n'][0] != 0xFFFF or any(v >= 4096 and v != 0xFFFF for v in t['id2n']):
        die(f'0x{src["id2n"]:08X} does not look like the ID->number table (check --src-id2n)')
    if any(v > 2047 for v in t['n2id']):
        die(f'0x{src["n2id"]:08X} does not look like the number->ID table (check --src-n2id)')
    return t


def blank_end(rom):
    """First address at or after ART_MIN from which the ROM holds only 0x00 or 0xFF bytes."""
    end = len(rom)
    while end > ART_MIN - ROM and rom[end - 1] in (0x00, 0xFF):
        end -= 1
    return ROM + max(ART_MIN - ROM, (end + 3) & ~3)


def in_use(rom, start, end, free):
    """Sub-ranges of [start, end) that are inside the ROM, outside the free ranges, and not blank."""
    bad, a = [], start
    for fs, fe in sorted(free) + [(end, end)]:
        if fe <= a:
            continue
        b = min(fs, end)
        if a < b:
            lo, hi = a - ROM, min(b, ROM + len(rom)) - ROM
            if lo < hi:
                chunk = rom[lo:hi]
                for blank in (0x00, 0xFF):
                    chunk = chunk.strip(bytes([blank]))
                if chunk:
                    bad.append((a, b))
        a = max(a, fe)
        if a >= end:
            break
    return bad


# ------------------------------------------------------------------------------------------- code patches
def code_patches(rom, last, scratch, src, dst, force):
    """card_expand.py's patches (Deck Edit list, card count, optional scratch) plus the table pointers, moved from
    their source to their destination. Returns (patches, notes)."""
    ce = load('card_expand_patches', os.path.join(HERE, 'card_expand.py'))   # fresh module: P has the RAM patches
    ce.counts(last)
    if scratch:
        ce.word(0x080693D8, 0x0201EFC4, ce.NEW_SCRATCH, 'scratch')
        ce.word(0x08069A88, 0x0201EFC4, ce.NEW_SCRATCH, 'scratch')
    fixed = list(ce.P)
    at = lambda ad, b: bytes(rom[ad - ROM:ad - ROM + len(b)])
    if all(at(ad, nw) == nw for ad, o, nw, _ in fixed) and fixed:
        die('the ROM is already expanded (the card count and Deck Edit list sites hold patched bytes)')
    changed = [ad for ad, o, nw, _ in fixed if at(ad, o) != o]
    if changed:
        die(f'{len(changed)} code sites card_expand.py patches do not hold the original bytes (e.g. '
            + ', '.join(f'0x{x:08X}' for x in changed[:5]) + '): the code there was edited, or the ROM is already expanded')

    by_name = {t[3]: opt for opt, t in TABLES.items()}
    pointers, odd = [], []
    for r in csv.DictReader(open(os.path.join(HERE, 'card_table_refs.csv'))):
        site, value, table = int(r['address'], 16), int(r['value'], 16), r['table']
        if r['function'] == '(data)' or site in SKIP_SITES or table not in by_name:
            continue
        opt = by_name[table]
        offset = value - TABLES[opt][2]
        expect, new = src[opt] + offset, dst[opt] + offset
        cur = word_at(rom, site)
        if cur != expect:
            odd.append(f"0x{site:08X} ({r['function']}: {table}) holds 0x{cur:08X}, not 0x{expect:08X}")
        elif new != cur:
            pointers.append((site, struct.pack('<I', cur), struct.pack('<I', new), f"{r['function']}: {table}"))
    expect = src['id2n'] + ORIGINAL_CARDS * 2
    cur = word_at(rom, END_POINTER)
    if cur != expect:
        odd.append(f'0x{END_POINTER:08X} (IsCardCollectionComplete: end of gCardIdToNumber) holds 0x{cur:08X}, not 0x{expect:08X}')
    else:
        pointers.append((END_POINTER, struct.pack('<I', cur), struct.pack('<I', dst['id2n'] + (last + 1) * 2),
                         'IsCardCollectionComplete: &gCardIdToNumber[last+1]'))
    if odd and not force:
        die(f'{len(odd)} table pointers do not point where the --src-* addresses say:\n  ' + '\n  '.join(odd[:20])
            + ('\n  ...' if len(odd) > 20 else '') + '\nGive the right --src-TABLE, or --force to leave these alone')
    return fixed + pointers, odd


# ------------------------------------------------------------------------------------------- cards
def add_cards(t, entries, a, A, leftover_numbers):
    """Append the cards of entries [(where, API name, shown name, settings)] to t; returns the rows for the CSV."""
    info = wt.fetch_info(entries, os.path.join(a.new_dir, 'cardinfo.json'), a.offline, a.refresh)
    free = wt.free_numbers(t['id2n'] + sorted(leftover_numbers))
    tokens = [no for no in range(1921, 1999) if not t['n2id'][no] and no not in leftover_numbers]
    taken = set(t['id2n'])
    known = {name.lower(): i for i, name in enumerate(t['text']) if name}
    known_pw = {p: i for i, p in enumerate(t['pw']) if i and p != b'\xff' * 4}
    rows = []
    for where, name, shown, opts in entries:
        c = info[name]
        card = wt.new_card(c, shown, opts, where)
        cid = len(t['names'])
        if card['name'].lower() in known:
            die(f"{where}: {card['name']!r} is already card ID {known[card['name'].lower()]}")
        if card['pw'] in known_pw:
            i = known_pw[card['pw']]
            die(f"{where}: {c['name']!r} has the password of card ID {i}, {t['text'][i]!r}: it is already in the game")
        if card['pw'] != b'\xff' * 4:
            known_pw[card['pw']] = cid
        if card['number'] is not None:
            number = int(card['number'])
            if not 0 <= number < 2000 or number in taken:
                die(f'{where}: card number {number} is out of range or already used')
        else:
            pool = tokens if card['token'] else free
            while pool and pool[0] in taken:
                pool.pop(0)
            if not pool:
                die(f'{where}: no free card numbers left')
            number = pool.pop(0)
        taken.add(number)
        png = os.path.join(a.new_dir, 'art', f"{c['id']}.png")
        if not os.path.exists(png):
            wt.convert_art(wt.fetch_image(c, os.path.join(a.new_dir, 'img'), a.offline), png, A)
        art, pal = wt.art_from_png(open(png, 'rb').read(), png, A)
        for k, v in (('names', A.field_bytes(card['name'], wt.NAME_SIZE)),
                     ('descs', A.field_bytes(card['desc'], wt.DESC_SIZE)), ('stats', card['stats']), ('pw', card['pw']),
                     ('id2n', number), ('art', art), ('pal', pal), ('text', card['name'])):
            t[k].append(v)
        t['n2id'][number] = cid
        known[card['name'].lower()] = cid
        rows.append([cid, number, 'leftover' if number in leftover_numbers else 'new_cards.txt',
                     card['pw'].hex() if card['pw'] != b'\xff' * 4 else '', card['name'], card['class'], c['name'], c['id']])
    return rows


# ------------------------------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0],
                                 epilog='Table addresses: see the table in the module docstring (python3 -m pydoc).')
    ap.add_argument('last', type=lambda s: int(s, 0), nargs='?', help='highest card ID (default: the smallest that fits)')
    ap.add_argument('--rom', default='eds.gba', help='the ROM to expand (original or edited, 8-32 MiB)')
    ap.add_argument('--out', default='build/card_expand/expanded.gba')
    ap.add_argument('--list', default='new_cards.txt', help='more cards to add after the leftover ones (empty: none)')
    ap.add_argument('--new-dir', default='assets/new_cards', help='where the downloaded card data is kept')
    ap.add_argument('--offline', action='store_true', help='use only the data already downloaded')
    ap.add_argument('--refresh', action='store_true', help='ask the API again for every card')
    ap.add_argument('--no-leftovers', action='store_true', help='do not add the cards with leftover effects')
    ap.add_argument('--no-code', action='store_true', help='leave the code alone (patch it yourself)')
    ap.add_argument('--move-scratch', action='store_true', help='always move the List Filter non-monster buffer')
    ap.add_argument('--fill', type=lambda s: int(s, 0), default=0xFF, help='byte for the added ROM space (default 0xFF)')
    ap.add_argument('--force', action='store_true',
                    help='write over non-blank data in the way and skip table pointers that do not match')
    g = ap.add_argument_group('table addresses (ROM addresses, e.g. 0x0822C720)')
    for opt, (_, size, orig, _) in TABLES.items():
        g.add_argument(f'--src-{opt}', type=addr, default=orig, metavar='ADDR',
                       help=f'where the input ROM has it (default 0x{orig:08X})')
        g.add_argument(f'--{opt}', type=addr, default=None, metavar='ADDR', help='where to write it')
    g.add_argument('--base', type=addr, default=None, metavar='ADDR', help='same as --art')
    a = ap.parse_args()
    src = {opt: getattr(a, f'src_{opt}') for opt in TABLES}
    if a.base is not None:
        if a.art is not None and a.art != a.base:
            die('--base and --art disagree')
        a.art = a.base

    A = wt.load('eds_assets', os.path.join(REPO, 'tools', 'assets.py'))
    rom = bytearray(open(a.rom, 'rb').read())
    if not 0x800000 <= len(rom) <= 0x2000000:
        die(f'{a.rom}: {len(rom):#x} bytes; expected 8 to 32 MiB')
    sha = hashlib.sha1(rom).hexdigest()
    kind = 'original USA ROM' if sha == ORIGINAL_SHA1 else f'edited ROM (SHA-1 {sha[:8]}, {len(rom) >> 20} MiB)'
    t = read_rom_tables(rom, src)

    leftover = [] if a.no_leftovers else LEFTOVER
    leftover_numbers = {no for no, _, _ in leftover}
    clash = sorted(no for no in leftover_numbers if no in t['id2n'] or t['n2id'][no])
    if clash:
        die(f'leftover card numbers {clash[:10]} are already used in this ROM (--no-leftovers to skip them)')
    entries = [(f'leftover {no}', name, name, dict(opts, number=str(no))) for no, name, opts in leftover]
    leftover_names = {name.lower(): no for no, name, _ in leftover}
    skipped = []
    for no, name, shown, opts in (wt.read_list(a.list) if a.list else []):
        if name.lower() in leftover_names:
            skipped.append(f'{name} (line {no}, leftover number {leftover_names[name.lower()]})')
            continue
        if 'number' in opts and int(opts['number']) in leftover_numbers:
            die(f'{a.list}:{no}: card number {opts["number"]} belongs to a leftover card')
        entries.append((f'{a.list}:{no}', name, shown, opts))
    rows = add_cards(t, entries, a, A, leftover_numbers) if entries else []

    real = len(t['names'])
    last = a.last
    if last is None:
        last = next(v for v in range(max(real - 1, 820), 2048) if wt.encodable(v))
    if not wt.encodable(last):
        die(f'LAST_ID {last} cannot be encoded (must be imm8 << n)')
    if not 820 <= last <= 2047:
        die('LAST_ID must be 820..2047')
    if last < real - 1:
        die(f'LAST_ID {last} is too small for {real - 1} cards')
    n = last + 1
    pad = n - real
    for k, v in (('names', bytes(wt.NAME_SIZE)), ('descs', bytes(wt.DESC_SIZE)), ('stats', bytes(4)),
                 ('pw', b'\xff' * 4), ('id2n', wt.PLACEHOLDER_NUMBER), ('art', bytes(wt.ART_SIZE)),
                 ('pal', bytes(wt.PAL_SIZE))):
        t[k] += [v] * pad
    others = sum((struct.unpack('<I', s)[0] >> 20 & 31) >= wt.TYPE_TRAP for s in t['stats'][1:real])
    scratch = a.move_scratch or others > wt.SCRATCH_LIMIT

    data = {opt: b''.join(t[TABLES[opt][0]]) for opt in ('names', 'descs', 'stats', 'passwords', 'art', 'palettes')}
    data['id2n'] = struct.pack(f'<{n + 1}H', *t['id2n'], 0)      # [LAST_ID + 1] = 0: IsCardCollectionComplete's gate
    data['n2id'] = struct.pack('<2048H', *t['n2id'])
    dst = {opt: getattr(a, opt) for opt in TABLES}
    for opt in ('names', 'descs', 'id2n', 'n2id'):
        dst[opt] = dst[opt] or src[opt]
    dst['stats'] = dst['stats'] or src['palettes']
    dst['passwords'] = dst['passwords'] or dst['stats'] + len(data['stats'])
    dst['art'] = dst['art'] or blank_end(rom)
    dst['palettes'] = dst['palettes'] or dst['art'] + len(data['art'])
    for opt, v in dst.items():
        if v % 4 and opt not in ('id2n', 'n2id') or v % 2:
            die(f'--{opt} 0x{v:08X} is not aligned')

    regions = sorted((dst[o], dst[o] + len(data[o]), o) for o in TABLES)
    for (s1, e1, k1), (s2, e2, k2) in zip(regions, regions[1:]):
        if e1 > s2:
            die(f'{k1} (0x{s1:08X}-0x{e1:08X}) overlaps {k2} (from 0x{s2:08X}); move one with --{k1} or --{k2}')
    rom_end = max(e for _, e, _ in regions)
    if rom_end > ROM + 0x2000000:
        die(f'the tables end at 0x{rom_end:08X}, past the 32 MiB limit')
    # the old tables are free (everything in them is rewritten or moved); anything else must be blank
    free = [(src[o], src[o] + (4096 if o == 'n2id' else ORIGINAL_CARDS * TABLES[o][1] + (2 if o == 'id2n' else 0)))
            for o in TABLES]
    used = [(o, s, e) for s0, e0, o in regions for s, e in in_use(rom, s0, e0, free)]
    if used and not a.force:
        die('the new tables would overwrite data that is not blank:\n  '
            + '\n  '.join(f'{o}: 0x{s:08X}-0x{e:08X}' for o, s, e in used[:10])
            + '\nMove the table with --TABLE, or pass --force')

    patches, odd = ([], []) if a.no_code else code_patches(rom, last, scratch, src, dst, a.force)
    size = 0x800000
    while size < max(len(rom), rom_end - ROM):
        size *= 2
    rom += bytes([a.fill]) * (size - len(rom))
    for ad, old, new, note in patches:
        if rom[ad - ROM:ad - ROM + len(old)] != old:
            die(f'internal error: 0x{ad:08X} ({note}) changed')
        rom[ad - ROM:ad - ROM + len(new)] = new
    for s, e, o in regions:
        rom[s - ROM:e - ROM] = data[o]

    os.makedirs(os.path.dirname(a.out) or '.', exist_ok=True)
    open(a.out, 'wb').write(rom)
    csv_path = os.path.splitext(a.out)[0] + '_cards.csv'
    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['id', 'number', 'source', 'password', 'name', 'card', 'ygoprodeck_name', 'passcode'])
        w.writerows(rows)
    n_left = sum(r[2] == 'leftover' for r in rows)
    code = 'left alone (--no-code)' if a.no_code else f'{len(patches)} sites patched'
    print(f'{a.out}: {size >> 20} MiB from the {kind}; card IDs 0..{last}: 820 original + {n_left} leftover + '
          f'{len(rows) - n_left} from {a.list or "no list"}' + (f' + {pad} placeholders' if pad else '')
          + f'; code: {code}' + (f' (non-monster buffer moved: {others} non-monsters)' if scratch else ''))
    print('  table      source       destination')
    for opt in TABLES:
        print(f'  {opt:10} 0x{src[opt]:08X}   0x{dst[opt]:08X}-0x{dst[opt] + len(data[opt]):08X}')
    print('original cards 0..820: copied byte for byte from the source tables; every added card is listed in', csv_path)
    for msg in odd:
        print('warning: left alone (--force):', msg)
    if used:
        print('warning: overwritten (--force): ' + ', '.join(f'{o} 0x{s:08X}-0x{e:08X}' for o, s, e in used))
    for s in skipped:
        print('skipped from the list, already a leftover card:', s)
    expected = ('is used by game code', 'is hidden from Card Trading', 'is in the Ticket/God range')
    hidden = 0
    for msg in wt.check_numbers(t['id2n'], t['n2id'], real, t['pw']):
        cid = int(msg.split()[1].rstrip(':')) if msg.startswith('ID ') else None
        if cid is not None and t['id2n'][cid] in leftover_numbers and any(e in msg for e in expected):
            hidden += 'Card Trading' in msg
            continue                                     # the reason these cards have their numbers
        print('warning:', msg)
    if hidden:
        print(f'note: {hidden} leftover cards have numbers in 1210-1899, which Card Trading hides (as the game does)')


if __name__ == '__main__':
    main()
