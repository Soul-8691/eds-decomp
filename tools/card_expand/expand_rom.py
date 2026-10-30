#!/usr/bin/env python3
"""Expand a binary ROM: repoint its card tables, add every card with leftover effect code at its own card number,
then add the cards listed in new_cards.txt.

  python3 tools/card_expand/expand_rom.py [LAST_ID] --rom eds.gba --out build/card_expand/expanded.gba
          [--list new_cards.txt] [--new-dir assets/new_cards] [--offline] [--refresh] [--no-leftovers]
          [--no-code] [--move-scratch] [--fill 0xFF] [--base 0x08800000]

Run from the repo root. Needs Pillow, and network access the first time (the card data is then cached).

Unlike write_tables.py, this script needs nothing from `make setup`: the 821 existing card entries (names, texts,
stats, passwords, art, palettes and both card-number maps) are copied byte for byte from the ROM's own tables,
and only moved to the addresses card_expand.py points the code to.

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

LAST_ID, the placeholders, --move-scratch, --no-code, --base and --fill work as in write_tables.py.
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
ROM, NAMES, DESCS, ID2N, N2ID, OLD = wt.ROM, wt.NAMES, wt.DESCS, wt.ID2N, wt.N2ID, wt.OLD
NAME_SIZE, DESC_SIZE, ART_SIZE, PAL_SIZE = wt.NAME_SIZE, wt.DESC_SIZE, wt.ART_SIZE, wt.PAL_SIZE
ORIGINAL_SHA1 = '510fbba212aca9bab95ea12f8fd933e62ee34dea'     # USA AY5E
ORIGINAL_CARDS = 821                                              # IDs 0..820

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


def read_rom_tables(rom):
    """The 821 original entries of every card table, as raw bytes per ID, read from the ROM itself."""
    n = ORIGINAL_CARDS
    rows = lambda addr, size: [bytes(rom[addr - ROM + i * size:addr - ROM + (i + 1) * size]) for i in range(n)]
    t = {'names': rows(NAMES, NAME_SIZE), 'descs': rows(DESCS, DESC_SIZE), 'stats': rows(OLD['stats'], 4),
         'pw': rows(OLD['pw'], 4), 'art': rows(OLD['art'], ART_SIZE), 'pal': rows(OLD['pal'], PAL_SIZE),
         'id2n': list(struct.unpack_from(f'<{n}H', rom, ID2N - ROM)),
         'n2id': list(struct.unpack_from('<2048H', rom, N2ID - ROM))}
    t['text'] = [r.split(b'\0')[0].decode('cp1252', 'replace') for r in t['names']]
    return t


def check_input(rom, path):
    """The tables are read at their original addresses, so the ROM must not be expanded already."""
    if len(rom) != 0x800000:
        die(f'{path}: expected the 8 MiB ROM, got {len(rom):#x} bytes')
    sha = hashlib.sha1(rom).hexdigest()
    if sha == ORIGINAL_SHA1:
        return 'original USA ROM'
    ce = load('card_expand_check', os.path.join(HERE, 'card_expand.py'))
    ce.counts(820)
    ce.pointers(820, os.path.join(HERE, 'card_table_refs.csv'))
    moved = [ad for ad, old, new, _ in ce.P if bytes(rom[ad - ROM:ad - ROM + len(old)]) != old]
    if moved:
        die(f'{path} is not the original ROM and {len(moved)} card-table sites are already changed (e.g. 0x{moved[0]:08X}): '
            'run this on an unexpanded ROM')
    return f'modified ROM (SHA-1 {sha[:8]}), card tables at their original addresses'


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
        for k, v in (('names', A.field_bytes(card['name'], NAME_SIZE)), ('descs', A.field_bytes(card['desc'], DESC_SIZE)),
                     ('stats', card['stats']), ('pw', card['pw']), ('id2n', number), ('art', art), ('pal', pal),
                     ('text', card['name'])):
            t[k].append(v)
        t['n2id'][number] = cid
        known[card['name'].lower()] = cid
        rows.append([cid, number, 'leftover' if number in leftover_numbers else 'new_cards.txt',
                     card['pw'].hex() if card['pw'] != b'\xff' * 4 else '', card['name'], card['class'], c['name'], c['id']])
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('last', type=lambda s: int(s, 0), nargs='?', help='highest card ID (default: the smallest that fits)')
    ap.add_argument('--rom', default='eds.gba', help='the original 8 MiB ROM')
    ap.add_argument('--out', default='build/card_expand/expanded.gba')
    ap.add_argument('--list', default='new_cards.txt', help='more cards to add after the leftover ones (empty: none)')
    ap.add_argument('--new-dir', default='assets/new_cards', help='where the downloaded card data is kept')
    ap.add_argument('--offline', action='store_true', help='use only the data already downloaded')
    ap.add_argument('--refresh', action='store_true', help='ask the API again for every card')
    ap.add_argument('--no-leftovers', action='store_true', help='do not add the cards with leftover effects')
    ap.add_argument('--no-code', action='store_true', help='leave the code alone (apply card_expand.py yourself)')
    ap.add_argument('--move-scratch', action='store_true', help='always move the List Filter non-monster buffer')
    ap.add_argument('--fill', type=lambda s: int(s, 0), default=0xFF, help='byte for the added ROM space (default 0xFF)')
    ap.add_argument('--base', type=lambda s: int(s, 0), default=0x08800000,
                    help='ROM address where the moved art starts, the palettes right after it (default 0x08800000)')
    a = ap.parse_args()

    A = wt.load('eds_assets', os.path.join(REPO, 'tools', 'assets.py'))
    ce = wt.load('card_expand_layout', os.path.join(HERE, 'card_expand.py'))
    ce.check_base(a.base)
    rom = bytearray(open(a.rom, 'rb').read())
    kind = check_input(rom, a.rom)
    t = read_rom_tables(rom)

    leftover = [] if a.no_leftovers else LEFTOVER
    leftover_numbers = {no for no, _, _ in leftover}
    clash = sorted(no for no in leftover_numbers if no < 2000 and (no in t['id2n'] or t['n2id'][no]))
    if clash:
        die(f'leftover card numbers {clash} are already used in this ROM')
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
    for k, v in (('names', bytes(NAME_SIZE)), ('descs', bytes(DESC_SIZE)), ('stats', bytes(4)), ('pw', b'\xff' * 4),
                 ('id2n', wt.PLACEHOLDER_NUMBER), ('art', bytes(ART_SIZE)), ('pal', bytes(PAL_SIZE))):
        t[k] += [v] * pad
    others = sum((struct.unpack('<I', s)[0] >> 20 & 31) >= wt.TYPE_TRAP for s in t['stats'][1:real])
    scratch = a.move_scratch or others > wt.SCRATCH_LIMIT
    try:
        moves, rom_end = ce.layout(last, a.base)
    except AssertionError as e:
        die(str(e))
    new = {k: moves[tb][1] for k, tb in [('art', 'gCardArt'), ('pal', 'gCardArtPalettes'), ('stats', 'gCardStats'),
                                         ('pw', 'gCardPasswords')]}
    data = {k: b''.join(t[k]) for k in ('names', 'descs', 'stats', 'pw', 'art', 'pal')}
    data['id2n'] = struct.pack(f'<{n + 1}H', *t['id2n'], 0)        # [LAST_ID + 1] = 0: IsCardCollectionComplete's gate
    data['n2id'] = struct.pack('<2048H', *t['n2id'])
    regions = [('names', NAMES, data['names']), ('descs', DESCS, data['descs']), ('id2n', ID2N, data['id2n']),
               ('n2id', N2ID, data['n2id']), ('stats', new['stats'], data['stats']), ('pw', new['pw'], data['pw']),
               ('art', new['art'], data['art']), ('pal', new['pal'], data['pal'])]
    limits = {'names': 0x082461A0, 'descs': new['stats'], 'id2n': N2ID}
    for k, start, b in regions:
        if k in limits and start + len(b) > limits[k]:
            die(f'{k} would run past 0x{limits[k]:08X}')
    spans = sorted((start, start + len(b), k) for k, start, b in regions)
    for (s1, e1, k1), (s2, e2, k2) in zip(spans, spans[1:]):
        if e1 > s2:
            die(f'{k1} (ends 0x{e1:08X}) overlaps {k2} (starts 0x{s2:08X})')

    original = {k: bytes(rom[s - ROM:s - ROM + sz * ORIGINAL_CARDS]) for k, (s, sz) in
                {'names': (NAMES, NAME_SIZE), 'descs': (DESCS, DESC_SIZE), 'stats': (OLD['stats'], 4),
                 'pw': (OLD['pw'], 4), 'art': (OLD['art'], ART_SIZE), 'pal': (OLD['pal'], PAL_SIZE)}.items()}
    status = 'skipped (--no-code)' if a.no_code else wt.apply_code(rom, last, scratch, a.base)
    size = 0x800000
    while ROM + size < rom_end:
        size *= 2
    if size > 0x2000000:
        die('the tables need more than 32 MiB')
    rom += bytes([a.fill]) * (size - len(rom))
    for k, start, b in regions:
        rom[start - ROM:start - ROM + len(b)] = b
    differ = [k for k, old in original.items() if data[k][:len(old)] != old]
    if differ:
        die(f'internal error: the copied original entries differ in {differ}')

    os.makedirs(os.path.dirname(a.out) or '.', exist_ok=True)
    open(a.out, 'wb').write(rom)
    csv_path = os.path.splitext(a.out)[0] + '_cards.csv'
    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['id', 'number', 'source', 'password', 'name', 'card', 'ygoprodeck_name', 'passcode'])
        w.writerows(rows)
    n_left = sum(r[2] == 'leftover' for r in rows)
    print(f'{a.out}: {size >> 20} MiB from the {kind}; card IDs 0..{last}: 820 original + {n_left} leftover + '
          f'{len(rows) - n_left} from {a.list or "no list"}' + (f' + {pad} placeholders' if pad else '')
          + f'; code: {status}' + (f' (non-monster buffer moved: {others} non-monsters)' if scratch else ''))
    for k, start, b in regions:
        print(f'  {k:6} 0x{start:08X}-0x{start + len(b):08X}')
    print('original cards 0..820: copied byte for byte from the ROM; every added card is listed in', csv_path)
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
