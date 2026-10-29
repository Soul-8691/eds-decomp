#!/usr/bin/env python3
"""Add the cards listed in new_cards.txt to a binary ROM, with their data ripped from the YGOPRODeck API.

  python3 tools/card_expand/write_tables.py [LAST_ID] --rom eds.gba --out build/card_expand/expanded.gba
          [--list new_cards.txt] [--cards assets/cards] [--new-dir assets/new_cards] [--offline] [--refresh]
          [--no-code] [--move-scratch] [--fill 0xFF]

Run from the repo root. Needs network access the first time and Pillow (pip install pillow) to convert the art.

The existing cards come from the files `make setup` extracted to assets/cards/ (names.json, descriptions.json,
stats.csv, passwords.csv, id_to_number.json, number_to_id.json, art/NNN.png). The cards named in new_cards.txt
follow them, one card ID each in list order (821, 822, ...). Each name is looked up on YGOPRODeck
(https://ygoprodeck.com/api-guide/, cardinfo.php v7) in batches, and everything downloaded is kept under
--new-dir so the API and the image server are asked only once per card:
  cardinfo.json      the API fields used, per listed name (editable: fix a text or a stat here and rerun)
  img/PASSCODE.jpg   the cropped artwork
  art/PASSCODE.png   the artwork as a 72x80 indexed PNG, 64 colours (made from the JPG if missing; replace it
                     with your own drawing to override)
  cards.csv          what was written: ID, card number, password and stats of every new card
--offline uses only these files; --refresh asks the API again for every listed name.

The API data becomes the game's encodings (see wiki/data/card-table.md): ATK/DEF / 10, the monster type and
attribute enums, kind (normal / effect / fusion / ritual), the spell/trap subtype, the 8-digit passcode as the
password, and the description reduced to the plain ASCII the game's font has (at most 479 bytes). Card numbers
are not in the API: each new card gets the next free number from 0-1209 (then 1210-1899, which Card Trading
hides), skipping every number game code still reacts to; tokens get 1921-1999 like Insect Monster Token. A line
in new_cards.txt can set values the API lacks or that should differ, after a second "|":
    Sheep Token | | attr=EARTH level=1 atk=0 def=0
    Scapegoat | | number=1245
Keys: atk, def, level, attr, race, number, password ("none" for no password).

LAST_ID defaults to the smallest value the code patch can encode (imm8 << n) that holds every card; the IDs past
the last real card become hidden placeholders (card number 1999, no name, blank art). IsCardCollectionComplete
counts every ID up to LAST_ID, placeholders included. --move-scratch (moving the List Filter's non-monster
buffer) is switched on by itself when there are more than 310 spell/trap/ticket/god cards. Unless --no-code,
the code patches from card_expand.py are applied first (or recognised as already applied). The art starts at
--base (default 0x08800000, the end of the original ROM) with the palettes right after it; a later base leaves
the space before it free for other data. The ROM grows to the next power of two that holds the art. The script checks that the existing cards come out byte-identical to the
ROM, and warns about card numbers that clash or are reserved and about duplicate passwords.
New effect monsters, spells and traps have no row in the effect table, so in a duel they do nothing.
"""
import argparse, csv, importlib.util, io, json, os, re, struct, sys, time, unicodedata
import urllib.error, urllib.parse, urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
ROM = 0x08000000
NAMES, DESCS, ID2N, N2ID = 0x0822C720, 0x082461A0, 0x08622AB4, 0x08623DF4
NAME_SIZE, DESC_SIZE, ART_SIZE, PAL_SIZE = 0x40, 0x1E0, 0x10E0, 0x80
OLD = {'art': 0x082A6500, 'pal': 0x08608360, 'stats': 0x08621DE0, 'pw': 0x08623120}
PLACEHOLDER_NUMBER = 1999
# Non-EDS numbers that only the Fusion (0x0819A7C8) and ritual (0x0819A990) recipe tables use; cards.h has no
# constant for them, so they are added to the reserved numbers by hand (LEFTOVER_EFFECTS.md).
RECIPE_ONLY_NUMBERS = {1333: 'Fusion material of 1334', 1402: 'Fusion material of 1404',
                       1403: 'Fusion material of 1404', 1411: 'ritual monster of 1416'}
NAME_SHOWN = 41                                     # longest name a Deck Edit list row shows
SCRATCH_LIMIT = 310                                 # List Filter non-monster buffer before --move-scratch

API = 'https://db.ygoprodeck.com/api/v7/cardinfo.php'
API_BATCH = 30                                      # names per request
API_DELAY, IMG_DELAY = 0.2, 0.25                    # seconds between requests (the API allows 20 per second)
USER_AGENT = 'eds-decomp card_expand/write_tables.py'

RACES = {r: i + 1 for i, r in enumerate([
    'Dragon', 'Zombie', 'Fiend', 'Pyro', 'Sea Serpent', 'Rock', 'Machine', 'Fish', 'Dinosaur', 'Insect', 'Beast',
    'Beast-Warrior', 'Plant', 'Aqua', 'Warrior', 'Winged Beast', 'Fairy', 'Spellcaster', 'Thunder', 'Reptile'])}
TYPE_TRAP, TYPE_MAGIC = 21, 22
ATTRS = {a: i + 1 for i, a in enumerate(['LIGHT', 'DARK', 'WATER', 'FIRE', 'EARTH', 'WIND'])}
SPELL_SUB = {'Normal': 0, 'Field': 2, 'Equip': 3, 'Continuous': 4, 'Quick-Play': 5, 'Ritual': 6}
TRAP_SUB = {'Normal': 0, 'Counter': 1, 'Continuous': 4}
KEEP = ('id', 'name', 'type', 'frameType', 'desc', 'atk', 'def', 'level', 'race', 'attribute')
TEXT_MAP = {'\u2018': "'", '\u2019': "'", '\u201c': '"', '\u201d': '"', '\u2013': '-', '\u2014': '-', '\u2212': '-',
            '\u2026': '...', '\u25cf': '-', '\u2022': '-', '\u00d7': 'x', '\u2605': '*', '\u2606': '*'}


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod                         # tools/assets.py looks itself up while loading its plugins
    spec.loader.exec_module(mod)
    return mod


def die(msg):
    sys.exit('write_tables: ' + msg)


def encodable(v):
    return any(v >> s < 256 and v >> s << s == v for s in range(32))


# ---------------------------------------------------------------------------------------- existing cards
def read_cards(d, A):
    """The extracted tables as per-ID lists (IDs 0..N-1)."""
    rd = lambda rel: open(os.path.join(d, rel), 'rb').read()
    names = json.loads(rd('names.json'))
    descs = json.loads(rd('descriptions.json'))
    id2n = json.loads(rd('id_to_number.json'))
    n2id = json.loads(rd('number_to_id.json'))
    pws = list(csv.DictReader(io.StringIO(rd('passwords.csv').decode())))
    stats = A.b_card_stats(rd, {'path': 'stats.csv'})
    n = len(names)
    if len(id2n) == n + 1:
        id2n = id2n[:-1]                               # the extracted list ends with the padding entry
    if len(n2id) != 2048:
        die(f'{d}/number_to_id.json must have 2048 entries')
    if [int(r['id']) for r in pws] != list(range(len(pws))):
        die(f'{d}/passwords.csv: ids must be 0, 1, 2, ... in order')
    have = {'descriptions.json': len(descs), 'id_to_number.json': len(id2n), 'stats.csv': len(stats) // 4,
            'passwords.csv': len(pws)}
    if any(v != n for v in have.values()):
        die(f'{d}: names.json has {n} entries but ' + ', '.join(f'{k} {v}' for k, v in have.items() if v != n))
    pw = []
    for r in pws:
        p = (r['password'] or '').strip()
        if p == '':
            pw.append(b'\xff' * 4)
        elif p.startswith('raw:'):
            pw.append(bytes.fromhex(p[4:]))
        elif re.fullmatch(r'\d{8}', p):
            pw.append(bytes.fromhex(p))
        else:
            die(f'{d}/passwords.csv: card {r["id"]}: password must be 8 digits or empty, not {p!r}')
    hi = json.loads(rd('art/palette_bit15.json')) if os.path.exists(os.path.join(d, 'art', 'palette_bit15.json')) else {}
    arts, pals = [], []
    for i in range(n):
        art, pal = art_from_png(rd(f'art/{i:03d}.png'), f'{d}/art/{i:03d}.png', A)
        if hi.get(f'{i:03d}'):
            cols = list(struct.unpack('<64H', pal))
            for j in hi[f'{i:03d}']:
                cols[j] |= 0x8000
            pal = struct.pack('<64H', *cols)
        arts.append(art)
        pals.append(pal)
    return {'names': names, 'descs': descs, 'stats': [stats[i:i + 4] for i in range(0, len(stats), 4)], 'pw': pw,
            'id2n': id2n, 'n2id': n2id, 'art': arts, 'pal': pals}


def art_from_png(data, what, A):
    w, h, px, plte = A.png_read(data)
    if (w, h) != (A.ART_W, A.ART_H):
        die(f'{what} must be {A.ART_W}x{A.ART_H}')
    try:
        art = A._art_pack(px)
    except ValueError as e:
        die(f'{what}: {e}')
    return art, struct.pack('<64H', *[A.rgb_to_bgr555(c) for c in (plte + [(0, 0, 0)] * 64)[:64]])


# -------------------------------------------------------------------------------------------- new_cards.txt
def read_list(path):
    """[(line number, API name, name shown in the game, {override: value})]"""
    out = []
    for no, line in enumerate(open(path, encoding='utf-8-sig'), 1):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        parts = [p.strip() for p in line.split('|', 2)]
        name, shown = parts[0], (parts[1] if len(parts) > 1 else '') or parts[0]
        opts = {}
        for kv in (parts[2].split() if len(parts) > 2 else []):
            k, sep, v = kv.partition('=')
            if not sep or k not in ('atk', 'def', 'level', 'attr', 'race', 'number', 'password'):
                die(f'{path}:{no}: unknown setting {kv!r} (atk, def, level, attr, race, number, password)')
            opts[k] = v
        out.append((no, name, shown, opts))
    seen = {}
    for no, name, _, _ in out:
        if name.lower() in seen:
            die(f'{path}:{no}: {name!r} is already listed on line {seen[name.lower()]}')
        seen[name.lower()] = no
    return out


# ----------------------------------------------------------------------------------------------- YGOPRODeck
def http_get(url):
    req = urllib.request.Request(url, headers={'User-Agent': USER_AGENT})
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=60) as r:
                return r.read()
        except urllib.error.HTTPError as e:
            if e.code == 400:                          # the API's "no card matching your query"
                return e.read()
            err = e
        except (urllib.error.URLError, OSError) as e:
            err = e
        time.sleep(2 * (attempt + 1))
    die(f'{url}: {err}')


def api(params):
    time.sleep(API_DELAY)
    return json.loads(http_get(API + '?' + urllib.parse.urlencode(params)))


def fetch_info(entries, cache_path, offline, refresh):
    """API data for every listed name, from the cache or in batches of exact-name queries."""
    cache = json.load(open(cache_path, encoding='utf-8')) if os.path.exists(cache_path) else {}
    want = [name for _, name, _, _ in entries if refresh or name not in cache]
    if want and offline:
        die(f'--offline: not in {cache_path}: ' + ', '.join(want[:10]) + (' ...' if len(want) > 10 else ''))
    if want:
        print(f'asking YGOPRODeck for {len(want)} cards ...')
    for b in range(0, len(want), API_BATCH):
        batch = want[b:b + API_BATCH]
        got = {c['name'].lower(): c for c in api({'name': '|'.join(batch)}).get('data', [])}
        for name in batch:
            c = got.get(name.lower())
            if c:
                img = next((im for im in c.get('card_images', []) if im['id'] == c['id']), (c.get('card_images') or [{}])[0])
                cache[name] = dict({k: c[k] for k in KEEP if k in c}, image=img.get('image_url_cropped'))
        os.makedirs(os.path.dirname(cache_path), exist_ok=True)
        with open(cache_path, 'w', encoding='utf-8') as f:
            json.dump(cache, f, ensure_ascii=False, indent=1)
    missing = [(no, name) for no, name, _, _ in entries if name not in cache]
    if missing:
        msg = []
        for no, name in missing:
            sug = api({'fname': name.split(' - ')[0][:30]}).get('data', [])[:5] if not offline else []
            msg.append(f'  line {no}: {name!r}' + (' (did you mean: ' + '; '.join(c['name'] for c in sug) + '?)' if sug else ''))
        die('YGOPRODeck has no card with these exact names:\n' + '\n'.join(msg))
    return cache


def fetch_image(c, img_dir, offline):
    path = os.path.join(img_dir, f"{c['id']}.jpg")
    if not os.path.exists(path):
        if offline or not c.get('image'):
            die(f"{c['name']}: no artwork in {img_dir}" + ('' if offline else ' and none in the API data'))
        time.sleep(IMG_DELAY)
        data = http_get(c['image'])
        if data[:2] != b'\xff\xd8':
            die(f"{c['image']}: not a JPEG")
        os.makedirs(img_dir, exist_ok=True)
        open(path, 'wb').write(data)
    return path


def convert_art(jpg, png, A):
    """Cropped artwork -> 72x80 indexed PNG with 64 colours, centre-cropped to the game's aspect ratio."""
    try:
        from PIL import Image
    except ImportError:
        die('converting the artwork needs Pillow: pip install pillow')
    im = Image.open(jpg).convert('RGB')
    w, h = im.size
    if w * A.ART_H > h * A.ART_W:                      # too wide: cut the sides
        cw = h * A.ART_W // A.ART_H
        im = im.crop(((w - cw) // 2, 0, (w - cw) // 2 + cw, h))
    else:
        ch = w * A.ART_H // A.ART_W
        im = im.crop((0, (h - ch) // 2, w, (h - ch) // 2 + ch))
    lanczos = getattr(Image, 'Resampling', Image).LANCZOS
    im = im.resize((A.ART_W, A.ART_H), lanczos)
    im = im.point(lambda v: v & 0xF8 | v >> 5)       # the 15-bit colours the GBA can show
    median = getattr(Image, 'Quantize', Image).MEDIANCUT
    q = im.quantize(colors=64, method=median)
    pal = q.getpalette()[:64 * 3]
    pal = [tuple(pal[i:i + 3]) for i in range(0, len(pal), 3)]
    pal += [(0, 0, 0)] * (64 - len(pal))
    os.makedirs(os.path.dirname(png), exist_ok=True)
    open(png, 'wb').write(A.png_write(A.ART_W, A.ART_H, q.tobytes(), pal))


def plain_text(s):
    """API text -> the printable ASCII the game's font has, on one line."""
    s = ''.join(TEXT_MAP.get(ch, ch) for ch in s)
    s = unicodedata.normalize('NFKD', s).encode('ascii', 'ignore').decode()
    return re.sub(r'\s+', ' ', s).strip()


def fit_text(s, size):
    if len(s) < size:
        return s, False
    cut = s[:size - 4]
    return cut[:cut.rfind(' ')].rstrip(' ,;:') + '...', True


# ----------------------------------------------------------------------------------------------- new cards
def free_numbers(id2n):
    """Card numbers no existing card has and no game code tests, in the order they are handed out."""
    hdr = open(os.path.join(REPO, 'include/constants/cards.h')).read()
    reserved = {int(m.group(1)) for m in re.finditer(r'=\s*(\d+),\s*/\*\s*no EDS card', hdr)} | set(RECIPE_ONLY_NUMBERS)
    used ={no if no < 2000 else no - 2000 for no in id2n if no != 0xFFFF}
    return [no for no in range(1900) if no not in used and no not in reserved]


def new_card(c, shown, opts, where):
    """One card's table entries from its API data (and the line's overrides)."""
    def opt(k, default):
        v = opts.get(k, default)
        if v is None:
            die(f"{where}: YGOPRODeck gives no {k} for {c['name']}; set it on the line ({k}=...)")
        return v

    kind_name = c['type']
    if kind_name in ('Spell Card', 'Trap Card'):
        subs = SPELL_SUB if kind_name == 'Spell Card' else TRAP_SUB
        sub = subs.get(c.get('race'))
        if sub is None:
            die(f"{where}: {c['name']}: unknown {kind_name} subtype {c.get('race')!r}")
        typ, attr = (TYPE_MAGIC, 0) if kind_name == 'Spell Card' else (TYPE_TRAP, 1)
        word = (sub << 17) | (typ << 20) | (attr << 29)
        cls = f"{kind_name.split()[0]}/{c.get('race')}"
    elif 'Monster' in kind_name or kind_name == 'Token':
        if re.search(r'Pendulum|Synchro|XYZ|Link', kind_name):
            die(f"{where}: {c['name']} is a {kind_name}, which EDS has no frame for")
        race = opt('race', c.get('race'))
        attr = opt('attr', c.get('attribute'))
        if race not in RACES:
            die(f'{where}: {c["name"]}: type {race!r} has no EDS equivalent ({", ".join(RACES)})')
        if attr not in ATTRS:
            die(f'{where}: {c["name"]}: attribute {attr!r} has no EDS equivalent ({", ".join(ATTRS)})')
        atk, dfn, level = int(opt('atk', c.get('atk'))), int(opt('def', c.get('def'))), int(opt('level', c.get('level')))
        warn = []
        for k, v in (('ATK', atk), ('DEF', dfn)):
            if v < 0 or v % 10 or v > 5110:
                warn.append(f'{k} {v} stored as {max(0, min(v, 5110)) // 10 * 10}')
        if not 1 <= level <= 12:
            die(f"{where}: {c['name']}: level {level} is not 1..12")
        kind = (2 if 'Fusion' in kind_name else 3 if 'Ritual' in kind_name
                else 0 if kind_name in ('Normal Monster', 'Token') else 1)
        word = (max(0, min(dfn, 5110)) // 10 | max(0, min(atk, 5110)) // 10 << 9 | kind << 18 | RACES[race] << 20
                | level << 25 | ATTRS[attr] << 29)
        cls = f"{race}/{attr}/L{level}/{['Normal', 'Effect', 'Fusion', 'Ritual'][kind]} {atk}/{dfn}"
        for w in warn:
            print(f"warning: {c['name']}: {w}")
    else:
        die(f"{where}: {c['name']}: card type {kind_name!r} is not supported")

    pw = opts.get('password', '%08d' % c['id'])
    if pw == 'none':
        pwb = b'\xff' * 4
    elif re.fullmatch(r'\d{8}', pw):
        pwb = bytes.fromhex(pw)
    else:
        die(f'{where}: password must be 8 digits or none, not {pw!r}')
    name = plain_text(shown)
    if len(name) > NAME_SHOWN:
        die(f'{where}: {name!r} has {len(name)} characters; a list row shows {NAME_SHOWN}. '
            'Give the name to show after a "|"')
    desc, cut = fit_text(plain_text(c.get('desc', '')), DESC_SIZE)
    if cut:
        print(f"warning: {c['name']}: description shortened to {len(desc)} characters")
    return {'name': name, 'desc': desc, 'stats': struct.pack('<I', word), 'pw': pwb, 'class': cls,
            'token': kind_name == 'Token', 'number': opts.get('number')}


def add_new_cards(t, entries, a, A):
    """Append the listed cards to the tables in t. Returns the rows for cards.csv."""
    info = fetch_info(entries, os.path.join(a.new_dir, 'cardinfo.json'), a.offline, a.refresh)
    free = free_numbers(t['id2n'])
    tokens = [no for no in range(1921, 1999) if not t['n2id'][no]]
    taken = {no for no in t['id2n']}
    known = {n.lower() if isinstance(n, str) else n['text'].lower(): i for i, n in enumerate(t['names'])}
    known_pw = {p: i for i, p in enumerate(t['pw']) if i and p != b'\xff' * 4}
    rows = []
    for no, name, shown, opts in entries:
        c, where = info[name], f'{a.list}:{no}'
        card = new_card(c, shown, opts, where)
        if card['name'].lower() in known:
            die(f"{where}: {card['name']!r} is already card ID {known[card['name'].lower()]}")
        if card['pw'] in known_pw:                     # the same card under another name (TCG vs. EDS)
            i = known_pw[card['pw']]
            die(f"{where}: {c['name']!r} has the password of card ID {i}, {t['names'][i]!r}: it is already in the game")
        known_pw[card['pw']] = len(t['names'])
        cid = len(t['names'])
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
            convert_art(fetch_image(c, os.path.join(a.new_dir, 'img'), a.offline), png, A)
        art, pal = art_from_png(open(png, 'rb').read(), png, A)
        for k, v in (('names', card['name']), ('descs', card['desc']), ('stats', card['stats']), ('pw', card['pw']),
                     ('id2n', number), ('art', art), ('pal', pal)):
            t[k].append(v)
        t['n2id'][number] = cid
        known[card['name'].lower()] = cid
        rows.append([cid, number, card['pw'].hex() if card['pw'] != b'\xff' * 4 else '', card['name'], card['class'],
                     c['name'], c['id']])
    with open(os.path.join(a.new_dir, 'cards.csv'), 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['id', 'number', 'password', 'name', 'card', 'ygoprodeck_name', 'passcode'])
        w.writerows(rows)
    return rows


def check_numbers(id2n, n2id, real, pws):
    """Warnings for card numbers the game would resolve to another card or treat specially."""
    warn = []
    hdr = open(os.path.join(REPO, 'include/constants/cards.h')).read()
    reserved = {int(m.group(1)): m.group(0) for m in re.finditer(r'=\s*(\d+),\s*/\*\s*no EDS card[^\n]*', hdr)}
    reserved.update({no: f'= {no}, /* {what} */' for no, what in RECIPE_ONLY_NUMBERS.items()})
    seen = {}
    for i in range(1, real):
        no = id2n[i]
        if no == 0xFFFF:
            warn.append(f'ID {i}: no card number')
        elif no >= 2000:
            if no - 2000 >= 2000 or n2id[no - 2000] + 1 != i:
                warn.append(f'ID {i}: alternate number {no} needs number_to_id[{no - 2000}] == {i - 1} (ID before it)')
        else:
            if n2id[no] != i and not 1920 <= no <= 1999:
                warn.append(f'ID {i}: number {no} maps back to ID {n2id[no]} in number_to_id.json')
            if no in seen and not 1920 <= no <= 1999:
                warn.append(f'ID {i}: number {no} is also ID {seen[no]}')
            seen[no] = i
            if no in reserved:
                warn.append(f'ID {i}: number {no} is used by game code ({reserved[no].split("/*")[1].strip(" */")})')
            if i > 820 and 1210 <= no <= 1899:
                warn.append(f'ID {i}: number {no} is hidden from Card Trading (1210-1899)')
            if i > 820 and 1900 <= no <= 1919:
                warn.append(f'ID {i}: number {no} is in the Ticket/God range 1900-1919')
    dup = {}
    for i, p in enumerate(pws[:real]):
        if p != b'\xff' * 4 and i:
            dup.setdefault(p, []).append(i)
    warn += [f'IDs {ids}: same password {p.hex()}' for p, ids in dup.items() if len(ids) > 1]
    return warn


# ----------------------------------------------------------------------------------------------- code
def apply_code(rom, last, scratch, base):
    ce = load('card_expand', os.path.join(HERE, 'card_expand.py'))
    ce.counts(last)
    ce.pointers(last, os.path.join(HERE, 'card_table_refs.csv'), base)
    if scratch:
        ce.word(0x080693D8, 0x0201EFC4, ce.NEW_SCRATCH, 'scratch')
        ce.word(0x08069A88, 0x0201EFC4, ce.NEW_SCRATCH, 'scratch')
    at = lambda ad, b: bytes(rom[ad - ROM:ad - ROM + len(b)])
    old = sum(at(ad, o) == o for ad, o, nw, _ in ce.P)
    new = sum(at(ad, nw) == nw for ad, o, nw, _ in ce.P)
    if new == len(ce.P):
        return 'already applied'
    if old != len(ce.P):
        bad = [hex(ad) for ad, o, nw, _ in ce.P if at(ad, o) != o and at(ad, nw) != nw][:5]
        die(f'code patch: {len(ce.P) - old} sites hold neither the original nor the patched bytes (e.g. {bad}); '
            'start from the original ROM, or patch it with the same LAST_ID / --move-scratch / --base')
    for ad, o, nw, _ in ce.P:
        rom[ad - ROM:ad - ROM + len(nw)] = nw
    return f'{len(ce.P)} sites patched'


# ----------------------------------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('last', type=lambda s: int(s, 0), nargs='?', help='highest card ID (default: the smallest that fits)')
    ap.add_argument('--rom', default='eds.gba')
    ap.add_argument('--out', default='build/card_expand/expanded.gba')
    ap.add_argument('--list', default='new_cards.txt', help='the cards to add (empty: none)')
    ap.add_argument('--cards', default='assets/cards', help='the existing cards, as extracted by make setup')
    ap.add_argument('--new-dir', default='assets/new_cards', help='where the downloaded card data is kept')
    ap.add_argument('--offline', action='store_true', help='use only the data already downloaded')
    ap.add_argument('--refresh', action='store_true', help='ask the API again for every listed card')
    ap.add_argument('--no-code', action='store_true', help='the ROM already has the code patches (or apply them yourself)')
    ap.add_argument('--move-scratch', action='store_true', help='always move the List Filter non-monster buffer')
    ap.add_argument('--fill', type=lambda s: int(s, 0), default=0xFF, help='byte for the added ROM space (default 0xFF)')
    ap.add_argument('--base', type=lambda s: int(s, 0), default=0x08800000,
                    help='ROM address where the moved art starts, the palettes right after it (default 0x08800000); '
                         'the space between 0x08800000 and it is left as --fill bytes')
    a = ap.parse_args()

    A = load('eds_assets', os.path.join(REPO, 'tools', 'assets.py'))
    ce = load('card_expand_layout', os.path.join(HERE, 'card_expand.py'))
    ce.check_base(a.base)
    t = read_cards(a.cards, A)
    base = len(t['names'])
    entries = read_list(a.list) if a.list else []
    rows = add_new_cards(t, entries, a, A) if entries else []
    real = len(t['names'])
    last = a.last
    if last is None:
        last = next(v for v in range(max(real - 1, 820), 2048) if encodable(v))
    if not encodable(last):
        die(f'LAST_ID {last} cannot be encoded (must be imm8 << n)')
    if not 820 <= last <= 2047:
        die('LAST_ID must be 820..2047')
    if last < real - 1:
        die(f'LAST_ID {last} is too small for {real - 1} cards')
    n = last + 1
    pad = n - real
    for k, v in (('names', ''), ('descs', ''), ('stats', bytes(4)), ('pw', b'\xff' * 4), ('id2n', PLACEHOLDER_NUMBER),
                 ('art', bytes(ART_SIZE)), ('pal', bytes(PAL_SIZE))):
        t[k] += [v] * pad
    others = sum((struct.unpack('<I', s)[0] >> 20 & 31) >= TYPE_TRAP for s in t['stats'][1:real])
    scratch = a.move_scratch or others > SCRATCH_LIMIT
    try:
        moves, rom_end = ce.layout(last, a.base)
    except AssertionError as e:
        die(str(e))
    new = {k: moves[tb][1] for k, tb in [('art', 'gCardArt'), ('pal', 'gCardArtPalettes'), ('stats', 'gCardStats'),
                                         ('pw', 'gCardPasswords')]}

    data = {'names': b''.join(A.field_bytes(v, NAME_SIZE) for v in t['names']),
            'descs': b''.join(A.field_bytes(v, DESC_SIZE) for v in t['descs']),
            'stats': b''.join(t['stats']), 'pw': b''.join(t['pw']), 'art': b''.join(t['art']), 'pal': b''.join(t['pal']),
            'id2n': struct.pack(f'<{n + 1}H', *t['id2n'], 0),     # [LAST_ID + 1] = 0: IsCardCollectionComplete's gate
            'n2id': struct.pack('<2048H', *t['n2id'])}
    regions = [('names', NAMES, data['names']), ('descs', DESCS, data['descs']), ('id2n', ID2N, data['id2n']),
               ('n2id', N2ID, data['n2id']), ('stats', new['stats'], data['stats']), ('pw', new['pw'], data['pw']),
               ('art', new['art'], data['art']), ('pal', new['pal'], data['pal'])]
    # in-place growth limits and overlaps between the new tables
    limits = {'names': 0x082461A0, 'descs': new['stats'], 'id2n': N2ID}
    for k, start, b in regions:
        if k in limits and start + len(b) > limits[k]:
            die(f'{k} would run past 0x{limits[k]:08X}')
    spans = sorted((start, start + len(b), k) for k, start, b in regions)
    for (s1, e1, k1), (s2, e2, k2) in zip(spans, spans[1:]):
        if e1 > s2:
            die(f'{k1} (ends 0x{e1:08X}) overlaps {k2} (starts 0x{s2:08X})')

    rom = bytearray(open(a.rom, 'rb').read())
    if len(rom) != 0x800000:
        die(f'{a.rom}: expected the 8 MiB ROM, got {len(rom):#x} bytes')
    # the original tables, read before anything is overwritten, to report which existing cards differ
    orig = {'names': (NAMES, NAME_SIZE), 'descs': (DESCS, DESC_SIZE), 'stats': (OLD['stats'], 4), 'pw': (OLD['pw'], 4),
            'art': (OLD['art'], ART_SIZE), 'pal': (OLD['pal'], PAL_SIZE), 'id2n': (ID2N, 2)}
    old_bytes = {k: bytes(rom[s - ROM:s - ROM + sz * 821]) for k, (s, sz) in orig.items()}
    old_n2id = bytes(rom[N2ID - ROM:N2ID - ROM + 4096])

    status = 'skipped (--no-code)' if a.no_code else apply_code(rom, last, scratch, a.base)
    size = 0x800000
    while ROM + size < rom_end:
        size *= 2
    if size > 0x2000000:
        die('the tables need more than 32 MiB')
    rom += bytes([a.fill]) * (size - len(rom))
    for k, start, b in regions:
        rom[start - ROM:start - ROM + len(b)] = b

    changed = {}
    for k, (s, sz) in orig.items():
        nb = data[k]
        ids = [i for i in range(821) if nb[i * sz:(i + 1) * sz] != old_bytes[k][i * sz:(i + 1) * sz]]
        if ids:
            changed[k] = ids
    n2id_changed = [no for no in range(2048) if data['n2id'][no * 2:no * 2 + 2] != old_n2id[no * 2:no * 2 + 2]]

    os.makedirs(os.path.dirname(a.out) or '.', exist_ok=True)
    open(a.out, 'wb').write(rom)
    print(f'{a.out}: {size >> 20} MiB, card IDs 0..{last}: {base - 1} existing + {len(rows)} new cards'
          + (f' + {pad} placeholders' if pad else '') + f'; code: {status}'
          + (f' (non-monster buffer moved: {others} non-monsters)' if scratch else ''))
    for k, start, b in regions:
        print(f'  {k:6} 0x{start:08X}-0x{start + len(b):08X}')
    if changed:
        print('existing cards that differ from the original ROM:',
              '; '.join(f'{k} {v[:12]}{"..." if len(v) > 12 else ""}' for k, v in changed.items()))
    else:
        print('existing cards 0..820: byte-identical to the original tables')
    print(f'number_to_id: {len(n2id_changed)} numbers changed {n2id_changed[:12]}{"..." if len(n2id_changed) > 12 else ""}')
    if rows:
        print(f'new cards: IDs {rows[0][0]}..{rows[-1][0]}, numbers {min(r[1] for r in rows)}..{max(r[1] for r in rows)}; '
              f"see {os.path.join(a.new_dir, 'cards.csv')}")
    for w in check_numbers(t['id2n'], t['n2id'], real, t['pw']):
        print('warning:', w)


if __name__ == '__main__':
    main()
