#!/usr/bin/env python3
"""The cards added to EDS beyond the original 820 (config/new_cards.tsv): fetch their data, build the tables.

  python3 tools/newcards.py fetch [--force]   # YGOPRODeck -> assets/new_cards/ (needs network and Pillow; once)
  python3 tools/newcards.py build             # build/assets/cards__*.bin + assets/new_cards -> build/assets/newcards__*.bin

`fetch` asks the YGOPRODeck API (https://ygoprodeck.com/api-guide/) for every passcode in config/new_cards.tsv
and writes assets/new_cards/cards.json (type, race, attribute, level, ATK/DEF and text of each card) and, from
the cropped card image, assets/new_cards/art/NNN.png: a 72x80 indexed PNG with at most 64 colours, the format
of assets/cards/art/. An existing PNG is kept (so hand-edited art survives) unless --force is given. The
downloads are cached in assets/new_cards/cropped/. Like the rest of assets/, none of this is committed.

`build` runs on every `make` (stdlib only). It takes the original 821-entry tables that tools/assets.py built
and appends the new cards (IDs 821..920), giving 921-entry tables, and fills gCardNumberToId for the new card
numbers. It also writes newcards__name_order.bin: every card ID in name order (the order the Deck Edit lists
show), ending with 0. The data layout that places these files is in data/rodata_08087FD0.s.
"""
import json
import os
import re
import struct
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402  (text encoding, PNG reader/writer, 6bpp art packing)

TSV = 'config/new_cards.tsv'
DIR = 'assets/new_cards'
IN = 'build/assets'
OUT = 'build/assets'
API = 'https://db.ygoprodeck.com/api/v7/cardinfo.php?id='

OLD_COUNT = 821                     # original tables: IDs 0..820
NAME_SIZE, DESC_SIZE, ART_SIZE, PAL_SIZE = 0x40, 0x1E0, 0x10E0, 0x80
NUMBER_COUNT = 2048                 # gCardNumberToId
MAX_NAME = 41                       # a Deck Edit list row has 21 tiles of 2 characters (the longest original name)
FIRST_APPENDED = 814                # IDs 814..820 (Tickets, Gods, Token) are appended out of name order

# YGOPRODeck race -> EDS monster type (gCardTypeNames), attribute -> EDS attribute, Spell/Trap race -> subtype.
MONSTER_TYPES = {
    'Dragon': 1, 'Zombie': 2, 'Fiend': 3, 'Pyro': 4, 'Sea Serpent': 5, 'Rock': 6, 'Machine': 7, 'Fish': 8,
    'Dinosaur': 9, 'Insect': 10, 'Beast': 11, 'Beast-Warrior': 12, 'Plant': 13, 'Aqua': 14, 'Warrior': 15,
    'Winged Beast': 16, 'Fairy': 17, 'Spellcaster': 18, 'Thunder': 19, 'Reptile': 20,
}
CARD_TYPE_TRAP, CARD_TYPE_MAGIC = 21, 22
ATTRIBUTES = {'LIGHT': 1, 'DARK': 2, 'WATER': 3, 'FIRE': 4, 'EARTH': 5, 'WIND': 6}
SUBTYPES = {'Normal': 0, 'Counter': 1, 'Field': 2, 'Equip': 3, 'Continuous': 4, 'Quick-Play': 5, 'Ritual': 6}
TEXT_FIXES = {'●': '', '’': "'", '‘': "'", '“': '"', '”': '"', '–': '-',
              '—': '-', '•': '', ' ': ' '}


def load_tsv():
    rows = []
    for line in open(TSV, encoding='utf-8'):
        if line.startswith('#') or line.startswith('id\t') or not line.strip():
            continue
        f = line.rstrip('\n').split('\t')
        cid, number, passcode, name = f[:4]
        game_name = f[4] if len(f) > 4 and f[4] else name
        if len(game_name) > MAX_NAME:
            sys.exit(f'{TSV}: "{game_name}" is longer than {MAX_NAME} characters; give it a shorter game_name')
        rows.append({'id': int(cid), 'number': int(number), 'passcode': passcode, 'name': name,
                     'game_name': game_name})
    for k, r in enumerate(rows):
        if r['id'] != OLD_COUNT + k:
            sys.exit(f'{TSV}: ids must run from {OLD_COUNT} without gaps ({r["name"]} has {r["id"]})')
    return rows


# --------------------------------------------------------------------------------------------------- fetch
def http_get(url):
    req = urllib.request.Request(url, headers={'User-Agent': 'eds-decomp tools/newcards.py'})
    with urllib.request.urlopen(req, timeout=60) as r:
        return r.read()


def to_art(path):
    """Cropped card image -> (72x80 pixel indices, 64-colour palette), centre-cropped to the 9:10 frame."""
    from PIL import Image
    img = Image.open(path).convert('RGB')
    w, h = img.size
    if w * A.ART_H > h * A.ART_W:           # too wide: crop the sides
        cw = h * A.ART_W // A.ART_H
        img = img.crop(((w - cw) // 2, 0, (w - cw) // 2 + cw, h))
    else:                                   # too tall: crop top and bottom
        ch = w * A.ART_H // A.ART_W
        img = img.crop((0, (h - ch) // 2, w, (h - ch) // 2 + ch))
    img = img.resize((A.ART_W, A.ART_H), Image.LANCZOS)
    img = img.point(lambda v: v & 0xF8)     # BGR555 precision, so the 64 colours stay distinct on hardware
    q = img.quantize(colors=64, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.FLOYDSTEINBERG)
    pal = q.getpalette()[:64 * 3]
    pal = [tuple(pal[i:i + 3]) for i in range(0, len(pal), 3)]
    pal += [(0, 0, 0)] * (64 - len(pal))
    return q.tobytes(), pal


def cmd_fetch(force):
    rows = load_tsv()
    os.makedirs(f'{DIR}/cropped', exist_ok=True)
    os.makedirs(f'{DIR}/art', exist_ok=True)
    data = {}
    for k in range(0, len(rows), 50):
        ids = ','.join(r['passcode'] for r in rows[k:k + 50])
        for c in json.loads(http_get(API + ids))['data']:
            data[c['id']] = c
    cards = []
    for r in rows:
        c = data.get(int(r['passcode']))
        if c is None:
            sys.exit(f'YGOPRODeck has no card with passcode {r["passcode"]} ({r["name"]})')
        if c['name'] != r['name']:
            print(f'note: passcode {r["passcode"]} is "{c["name"]}" on YGOPRODeck, "{r["name"]}" in {TSV}')
        images = c['card_images']
        image = next((i for i in images if i['id'] == c['id']), images[0])
        jpg = f'{DIR}/cropped/{r["passcode"]}.jpg'
        if not os.path.exists(jpg):
            open(jpg, 'wb').write(http_get(image['image_url_cropped']))
            time.sleep(0.1)                 # YGOPRODeck allows 20 requests per second
        png = f'{DIR}/art/{r["id"]:03d}.png'
        if force or not os.path.exists(png):
            px, pal = to_art(jpg)
            open(png, 'wb').write(A.png_write(A.ART_W, A.ART_H, px, pal))
        cards.append({'id': r['id'], 'number': r['number'], 'passcode': r['passcode'], 'name': r['name'],
                      'type': c['type'], 'race': c['race'], 'attribute': c.get('attribute'),
                      'level': c.get('level', 0), 'atk': c.get('atk', 0), 'def': c.get('def', 0),
                      'desc': c['desc']})
    json.dump(cards, open(f'{DIR}/cards.json', 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    print(f'fetched {len(cards)} cards into {DIR}/')


# --------------------------------------------------------------------------------------------------- build
def clean_text(s):
    for a, b in TEXT_FIXES.items():
        s = s.replace(a, b)
    s = re.sub(r'\s+', ' ', s).strip()
    if any(ord(ch) > 126 for ch in s):
        raise ValueError(f'unsupported character in {s[:60]!r}')
    return s


def stats_word(c):
    """struct CardStats: def:9 atk:9 kind:2 type:5 level:4 attr:3; Magic/Trap keep their subtype in bits 17-19."""
    if c['type'] in ('Spell Card', 'Trap Card'):
        sub = SUBTYPES[c['race']]
        if c['type'] == 'Spell Card':
            return CARD_TYPE_MAGIC << 20 | sub << 17
        return 1 << 29 | CARD_TYPE_TRAP << 20 | sub << 17
    if c['race'] not in MONSTER_TYPES:
        raise ValueError(f'{c["name"]}: monster type {c["race"]} does not exist in EDS')
    kind = 0 if c['type'] == 'Normal Monster' else 2 if 'Fusion' in c['type'] else 3 if 'Ritual' in c['type'] else 1
    atk, dfn = max(c['atk'] or 0, 0), max(c['def'] or 0, 0)
    if atk % 10 or dfn % 10 or atk > 5110 or dfn > 5110:
        raise ValueError(f'{c["name"]}: ATK/DEF {atk}/{dfn} cannot be stored (multiples of 10 up to 5110)')
    return (ATTRIBUTES[c['attribute']] << 29 | c['level'] << 25 | MONSTER_TYPES[c['race']] << 20 | kind << 18
            | (atk // 10) << 9 | dfn // 10)


def bcd(passcode):
    return bytes(int(passcode[i]) << 4 | int(passcode[i + 1]) for i in range(0, 8, 2))


def name_key(name):
    return name.lower().replace('-', '')     # the order of IDs 1..813 (wiki data/card-name-table.md)


def read_old(name, size):
    data = open(f'{IN}/cards__{name}.bin', 'rb').read()
    return [data[i * size:(i + 1) * size] for i in range(OLD_COUNT)], data


def pad4(b):
    return b + bytes(-len(b) % 4)


def cmd_build():
    rows = load_tsv()
    try:
        cards = {c['id']: c for c in json.load(open(f'{DIR}/cards.json', encoding='utf-8'))}
    except FileNotFoundError:
        sys.exit(f'{DIR}/cards.json is missing: run `python3 tools/newcards.py fetch` (needs network and Pillow)')
    new = []
    for r in rows:
        c = dict(cards[r['id']])
        c.update(number=r['number'], name=r['game_name'], passcode=r['passcode'])
        new.append(c)
    total = OLD_COUNT + len(new)

    names, _ = read_old('names.json', NAME_SIZE)
    descs, _ = read_old('descriptions.json', DESC_SIZE)
    _, art = read_old('art', 1)
    stats, _ = read_old('stats.csv', 4)
    i2n, _ = read_old('id_to_number.json', 2)
    pws, _ = read_old('passwords.csv', 4)
    keys, _ = read_old('sort_keys.json', 2)
    n2i = list(struct.unpack('<2048H', open(f'{IN}/cards__number_to_id.json.bin', 'rb').read()))
    gfx = [art[i * ART_SIZE:(i + 1) * ART_SIZE] for i in range(OLD_COUNT)]
    pal_base = OLD_COUNT * ART_SIZE
    pals = [art[pal_base + i * PAL_SIZE:pal_base + (i + 1) * PAL_SIZE] for i in range(OLD_COUNT)]
    next_key = max(struct.unpack('<H', k)[0] for k in keys[1:]) + 1

    for c in new:
        if n2i[c['number']]:
            sys.exit(f'card number {c["number"]} ({c["name"]}) already belongs to card ID {n2i[c["number"]]}')
        names.append(A.field_bytes(c['name'], NAME_SIZE))
        descs.append(A.field_bytes(clean_text(c['desc']), DESC_SIZE))
        w, h, px, plte = A.png_read(open(f'{DIR}/art/{c["id"]:03d}.png', 'rb').read())
        if (w, h) != (A.ART_W, A.ART_H):
            sys.exit(f'{DIR}/art/{c["id"]:03d}.png must be {A.ART_W}x{A.ART_H}')
        gfx.append(A._art_pack(px))
        pals.append(struct.pack('<64H', *[A.rgb_to_bgr555(p) for p in (plte + [(0, 0, 0)] * 64)[:64]]))
        stats.append(struct.pack('<I', stats_word(c)))
        i2n.append(struct.pack('<H', c['number']))
        pws.append(bcd(c['passcode']))
        keys.append(struct.pack('<H', next_key))
        next_key += 1
        n2i[c['number']] = c['id']

    all_names = [A.dec(n.split(b'\0')[0]) for n in names]
    order = sorted(list(range(1, FIRST_APPENDED)) + list(range(OLD_COUNT, total)),
                   key=lambda i: (name_key(all_names[i]), i))
    old = [i for i in order if i < FIRST_APPENDED]
    if old != sorted(old):
        sys.exit('the name order would move original cards relative to each other')
    order += list(range(FIRST_APPENDED, OLD_COUNT)) + [0]

    out = {
        'names': b''.join(names),
        'descriptions': b''.join(descs),
        'art_gfx': b''.join(gfx),
        'art_palettes': b''.join(pals),
        'stats': b''.join(stats),
        'id_to_number': pad4(b''.join(i2n)),
        'passwords': b''.join(pws),
        'number_to_id': struct.pack('<2048H', *n2i),
        'sort_keys': pad4(b''.join(keys)),
        'name_order': pad4(struct.pack(f'<{len(order)}H', *order)),
    }
    for name, data in out.items():
        dst = f'{OUT}/newcards__{name}.bin'
        if not os.path.exists(dst) or open(dst, 'rb').read() != data:
            open(dst, 'wb').write(data)
    open(f'{OUT}/.newcards', 'w').write(f'{total}\n')


def main():
    a = sys.argv[1:]
    if a[:1] == ['fetch']:
        cmd_fetch('--force' in a)
    elif a[:1] == ['build']:
        cmd_build()
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main()
