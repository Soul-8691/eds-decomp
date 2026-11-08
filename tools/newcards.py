#!/usr/bin/env python3
"""The cards added to EDS beyond its 820 original cards: one card name per line (default config/new_cards.txt).

  NEW_CARDS_LIST=new_cards.txt python3 tools/newcards.py build   # demo list (repo root; stops at a ------ line)
  python3 tools/newcards.py build             # the Makefile runs it: fetch what is new, then build the tables
  python3 tools/newcards.py fetch [--force]   # (re)download every listed card; --force also redoes the art
  python3 tools/newcards.py capacity          # how many cards fit

The list is all there is to edit. Each name is looked up on YGOPRODeck (https://ygoprodeck.com/api-guide/), and
its type, race, attribute, level, ATK/DEF, text and passcode are cached in assets/new_cards/cards.json (by name);
the cropped card image becomes assets/new_cards/art/<passcode>.png, a 72x80 indexed PNG with at most 64 colours
(the format of assets/cards/art/; an existing PNG is kept, so hand-edited art survives, unless --force). Only
names not yet cached are downloaded (network and Pillow are needed then). None of this is committed.

`build` (stdlib only once everything is cached) gives the cards IDs 821, 822, ... in list order and card numbers
from the free numbers (free_numbers); a token takes the fixed number of its token kind instead (TOKEN_NUMBERS), and a card the engine knows by an effect
key takes that key (ENGINE_NUMBERS); then it appends them to the original 821-entry tables that tools/assets.py
built: build/assets/newcards__*.bin, plus newcards__name_order.bin (every card ID in name order, then 0). It also
writes the card count for C and assembly (build/include/card_count.h and .inc), build/new_cards.tsv (the
assignment) and the CARD_ constants of the added cards in include/constants/cards.h. Files are only rewritten
when their content changes. The data layout that places the tables is in data/rodata_08087FD0.s.
"""
import difflib
import json
import os
import re
import struct
import sys
import time
import urllib.error
import urllib.parse
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import assets as A  # noqa: E402  (text encoding, PNG reader/writer, 6bpp art packing)
from gencards import ident  # noqa: E402  (card name -> CARD_ identifier)

DEFAULT_LIST = 'config/new_cards.txt'
LIST = DEFAULT_LIST
DIR = 'assets/new_cards'
CACHE = f'{DIR}/cards.json'
IN = 'build/assets'
OUT = 'build/assets'
GEN_TSV = 'build/new_cards.tsv'
COUNT_H = 'build/include/card_count.h'
COUNT_INC = 'build/include/card_count.inc'
SYNCHRO_H = 'build/include/synchro_rules.h'
XYZ_H = 'build/include/xyz_rules.h'
LINK_H = 'build/include/link_rules.h'
CARDS_H = 'include/constants/cards.h'
API = 'https://db.ygoprodeck.com/api/v7/cardinfo.php?'

OLD_COUNT = 821                     # original tables: IDs 0..820
NAME_SIZE, DESC_SIZE, ART_SIZE, PAL_SIZE = 0x40, 0x1E0, 0x10E0, 0x80
FIRST_APPENDED = 814                # IDs 814..820 (Tickets, Gods, Token) are appended out of name order
MAX_NAME = 41                       # a Deck Edit list row has 21 tiles of 2 characters (the longest original name)
MAX_IDS = min(0xCD40 * 2 // NAME_SIZE,  # gCardNames grows in place into the unused name bank
              0x7FF + 1)            # card IDs are masked with 0x7FF
# Card numbers for the added cards. Card Trading hides 1210-1899 (except added cards, which it lists by ID);
# 1211-1552 are effect-table keys and the numbers the duel code tests, and 1900-1999 are Tickets, Gods and tokens.
NUMBER_BANDS = [(1, 1210), (1600, 1900)]
# Tokens (YGOPRODeck type "Token") take the fixed number of their token kind instead: DUEL_CMD_SUMMON_TOKEN places
# the card of number 1920 + kind (enum TokenKind, include/constants/duel_cmds.h). Like the original Insect Monster
# Token (1920) they have no password, and the token number range keeps them out of the trunk and the card lists.
# YGOPRODeck leaves a token's stats to the card that summons it, so they are given here, with the text.
TOKEN_NUMBERS = {
    'Ojama Token': {**dict(number=1921, race='Beast', attribute='LIGHT', level=2, atk=0,  # TOKEN_KIND_1
                            desc='A token summoned by Ojama Trio.'),
                    'def': 1000},
    'Sheep Token': dict(number=1922,    # TOKEN_KIND_2: Scapegoat (EffectSummonFourTokensResolve)
                        race='Beast', attribute='EARTH', level=1, atk=0, desc='A token summoned by Scapegoat.'),
}
# Cards the game engine already knows by an effect-table key with no EDS card: they take that number, and with it the
# engine's own code and data for it (effect row, Fusion recipe, the special cases that test the number).
ENGINE_NUMBERS = {
    'Thousand-Eyes Idol': 1333,     # the second material of recipe 1334 (gFusionRecipes2: 730 + 1333)
    'Thousand-Eyes Restrict': 1334, # key 1334: Relinquished's absorb, the attack/position lock, recipe 730 + 1333
    'Soul Exchange': 1220,          # key 1220: EffectCanTributeOpponentMonsterPrepare, EffectSkipBattlePhaseChainA,
                                    # EffectTributeOpponentMonsterChainB / Resolve (CPU path: src/added_effects.c)
    'Bazoo the Soul-Eater': 1513,   # key 1513: menu ACTIVATE once per turn, EffectBanishGraveForAtkResolve (up to 3 own
                                    # Graveyard monsters, +300 ATK each), the link ends at its controller's next turn
    'Magic Cylinder': 1214,         # key 1214: EffectAttackResponsePrepare / Resolve (negate the attack, the
                                    # attacker's player loses its ATK); the CPU's trap logic knows it (ai_summon.c)
    'Fairy Box': 1424,              # key 1424: chained at once when the opponent declares an attack
                                    # (BattleStage_RespondToAttack), EffectCoinTossZeroAttackerResolve (call a coin;
                                    # right: the attacker's ATK is 0 for the battle), 500 LP each Standby Phase
                                    # (GetMaintenanceLpCost); the CPU's trap logic knows it (ai_summon.c)
    'Goblin Attack Force': 1344,    # key 1344: attacking uses up effectUnused (BattleStage_DamageCalc), at the end of
                                    # the Battle Phase a face-up Attack Position one turns to Defense Position
                                    # (BattleStage_EndBattlePhase), and its controller's next turn start locks its
                                    # position for that turn and re-arms it (DuelCmd_TurnStart)
    'United We Stand': 1449,        # key 1449: an Equip (EffectEquipResolve and the equip target rules), +800 ATK/DEF
                                    # per face-up monster of the equip card's controller (CalcZoneCardStats)
    "Nightmare's Steelcage": 1230,  # key 1230: a Continuous Spell with no effect row of its own (EffectNopResolve);
                                    # while it is face up neither player can attack (BuildAttackableMask), its turn
                                    # counter (battle_scene.c) and the opponent's 2nd End Phase destroy it
                                    # (DuelPhase_End's spell/trap counters, battle_phase3.c)
    'Big Bang Shot': 1313,          # key 1313: an Equip (EffectEquipResolve and the equip target rules) that gives piercing
                                    # (CalcBattle); +400 ATK (CalcZoneCardStats) and the banish (BigBangShotLeaves) are added
    'Mage Power': 1450,             # key 1450: an Equip, +500 ATK/DEF per Spell/Trap of the equip card's controller,
                                    # itself included (CalcZoneCardStats; changed to count the Field Spell too)
    'Gearfried the Iron Knight': 1351,  # key 1351: EquipCard destroys an Equip put on it (and the monster too for
                                    # Premature Burial); Snatch Steal cannot target it
    'The Fiend Megacyber': 1350,    # key 1350: Special Summoned from the hand while the opponent has 2 or more monsters
                                    # more (CanSummonFromHand, the hand menu, the summon steps)
    'Gilasaurus': 1520,             # key 1520: the hand menu's Special Summon commands, then (effect_hooks.c) a
                                    # RESPONSE_SPECIAL_SUMMONED trigger whose resolve (EffectOpponentGraveSummonResolve)
                                    # asks the opponent, who may Special Summon 1 monster from their Graveyard
}

# YGOPRODeck race -> EDS monster type (gCardTypeNames), attribute -> EDS attribute, Spell/Trap race -> subtype.
MONSTER_TYPES = {
    'Dragon': 1, 'Zombie': 2, 'Fiend': 3, 'Pyro': 4, 'Sea Serpent': 5, 'Rock': 6, 'Machine': 7, 'Fish': 8,
    'Dinosaur': 9, 'Insect': 10, 'Beast': 11, 'Beast-Warrior': 12, 'Plant': 13, 'Aqua': 14, 'Warrior': 15,
    'Winged Beast': 16, 'Fairy': 17, 'Spellcaster': 18, 'Thunder': 19, 'Reptile': 20,
}
CARD_TYPE_TRAP, CARD_TYPE_MAGIC = 21, 22
# Races EDS does not have: the stats word keeps the closest EDS monster type (its icon, and what the engine's monster
# checks see: they treat types 1-20 as monsters), and gCardTypeEx keeps the real race (enum RaceEx in
# include/constants/card_stats.h) for the type line and the List Filter.
EXTRA_RACES = {
    'Psychic': (1, 'Spellcaster'), 'Wyrm': (2, 'Dragon'), 'Cyberse': (3, 'Machine'), 'Illusion': (4, 'Spellcaster'),
    'Creator-God': (5, 'Fairy'), 'Divine-Beast': (6, 'Fairy'),
}
# gCardTypeEx, one byte per card ID (0 for the original cards): bits 0-1 the extended kind (enum CardKindEx), bit 2
# has an effect, bit 3 Tuner, bits 4-7 the extended race (EXTRA_RACES; 0 = the stats word's type).
KIND_EX_SYNCHRO = 1
KIND_EX_XYZ = 2
KIND_EX_LINK = 3
# Link Arrows: gCardLinkMarkers, one byte per card ID (src/mod_link.c), from YGOPRODeck's linkmarkers.
LINK_MARKER_BITS = {'Top-Left': 0x01, 'Top': 0x02, 'Top-Right': 0x04, 'Left': 0x08, 'Right': 0x10,
                    'Bottom-Left': 0x20, 'Bottom': 0x40, 'Bottom-Right': 0x80}
TYPE_EX_EFFECT = 0x04
TYPE_EX_TUNER = 0x08
ATTRIBUTES = {'LIGHT': 1, 'DARK': 2, 'WATER': 3, 'FIRE': 4, 'EARTH': 5, 'WIND': 6}
SUBTYPES = {'Normal': 0, 'Counter': 1, 'Field': 2, 'Equip': 3, 'Continuous': 4, 'Quick-Play': 5, 'Ritual': 6}
# Cards whose EDS subtype is not their YGOPRODeck race: Traps that become Equip Cards are Equip Traps in EDS (as the
# original's Kunai with Chain), so they stay on the field after resolving (Chain_CardGoesToGrave).
SUBTYPE_OVERRIDES = {'Blast with Chain': 'Equip'}
TEXT_FIXES = {'●': '', '’': "'", '‘': "'", '“': '"', '”': '"', '–': '-',
              '—': '-', '•': '', ' ': ' '}
BLOCK_BEGIN = '    /* ---- Added cards (new_cards list), written by tools/newcards.py ---- */'
BLOCK_END = '    /* ---- end of the added cards ---- */'
MOD_DECKS_C = 'src/mod_decks.c'


def strip_added_blocks(text):
    """Remove every added-cards block from cards.h (handles duplicate markers from older runs)."""
    while BLOCK_BEGIN in text:
        start = text.index(BLOCK_BEGIN)
        end = text.index(BLOCK_END, start) + len(BLOCK_END)
        if end < len(text) and text[end] == '\n':
            end += 1
        text = text[:start] + text[end:]
    return text


def original_card_symbols():
    """CARD_* names in the base game (before any added-cards block in cards.h)."""
    text = strip_added_blocks(open(CARDS_H, encoding='utf-8').read())
    return {m.group(1) for m in re.finditer(r'\b(CARD_\w+)\s*=', text)}


def all_card_symbols():
    """Every CARD_* in cards.h (base game plus the current added-cards block)."""
    text = open(CARDS_H, encoding='utf-8').read()
    return {m.group(1) for m in re.finditer(r'\b(CARD_\w+)\s*=', text)}


def mod_decks_guard_has():
    """Every HAS_CARD_* tested in mod_decks.c #if defined(...) deck guards."""
    text = open(MOD_DECKS_C, encoding='utf-8').read()
    return sorted(set(re.findall(r'defined\((HAS_CARD_\w+)\)', text)))


def in_game_has_defines(added_card_syms):
    """HAS_CARD_* for mod_decks guards when the card exists in cards.h but is not in the added-card list."""
    added_has = {f'HAS_{sym}' for sym in added_card_syms}
    in_game = all_card_symbols()
    out = []
    for has in mod_decks_guard_has():
        if has in added_has:
            continue
        card = has[4:] if has.startswith('HAS_') else has  # HAS_CARD_FOO -> CARD_FOO
        if card in in_game:
            out.append(f'#define {has} 1\n')
    return ''.join(out)


def fail(msg):
    sys.exit(f'{LIST}: {msg}')


def load_list():
    """[(name, game name)] in list order. Lines after a ------ separator are ignored (root new_cards.txt demo)."""
    rows, seen = [], set()
    for n, line in enumerate(open(LIST, encoding='utf-8'), 1):
        line = line.strip()
        if line.startswith('------'):
            break
        if not line or line.startswith('#'):
            continue
        name, _, game_name = (s.strip() for s in line.partition('|'))
        game_name = game_name or name
        if name.lower() in seen:
            fail(f'line {n}: {name} is listed twice')
        seen.add(name.lower())
        if len(game_name) > MAX_NAME:
            fail(f'line {n}: "{game_name}" is longer than {MAX_NAME} characters; add a shorter in-game name '
                 f'after a "|"')
        try:
            A.enc(game_name)
        except ValueError as e:
            fail(f'line {n}: {e}')
        rows.append((name, game_name))
    return rows


def load_leftovers(rows):
    """NEWCARDS_LEFTOVERS (make demo): [(name, game name, card number)] of the leftover effect cards
    (config/leftover_cards.txt: "number | name [| game name]") that are not in the card list already."""
    path = os.environ.get('NEWCARDS_LEFTOVERS', '')
    if not path:
        return []
    listed = {n.lower() for n, _ in rows}
    out = []
    for n, line in enumerate(open(path, encoding='utf-8'), 1):
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        parts = [s.strip() for s in line.split('|')]
        number, name = int(parts[0]), parts[1]
        game_name = parts[2] if len(parts) > 2 and parts[2] else name
        if name.lower() in listed:
            continue
        if len(game_name) > MAX_NAME:
            sys.exit(f'{path}: line {n}: "{game_name}" is longer than {MAX_NAME} characters')
        listed.add(name.lower())
        out.append((name, game_name, number))
    return out


def write_if_changed(path, data):
    if isinstance(data, str):
        data = data.encode()
    if os.path.exists(path) and open(path, 'rb').read() == data:
        return False
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, 'wb').write(data)
    return True


# --------------------------------------------------------------------------------------------- numbers
def cards_h_numbers():
    """Values of the CARD_ constants outside the added-cards block (cards and numbers the code uses)."""
    text = strip_added_blocks(open(CARDS_H, encoding='utf-8').read())
    return {int(v) for v in re.findall(r'^\s+CARD_\w+ = (\d+),(?!.*added card)', text, re.M)}


def free_numbers():
    """Card numbers the added cards may use, in order: in NUMBER_BANDS, with no original card, not a constant of
    include/constants/cards.h (the cards and the numbers the code tests), not an effect-table key, and not any
    integer of the extracted game tables (deck lists, packs, AI lists, ...; numbers that are something else there
    are excluded too, which costs nothing)."""
    taken = {n for n, i in enumerate(json.load(open('assets/cards/number_to_id.json'))) if i}
    taken |= cards_h_numbers()
    taken |= {e['number'] for e in json.load(open('assets/tables/card_effect_handlers.json'))}

    def walk(o):
        if isinstance(o, bool):
            return
        if isinstance(o, int):
            taken.add(o)
        elif isinstance(o, list):
            for x in o:
                walk(x)
        elif isinstance(o, dict):
            for x in o.values():
                walk(x)
    for root, _, files in os.walk('assets/tables'):
        for fn in sorted(files):
            if fn.endswith('.json'):
                try:
                    walk(json.load(open(os.path.join(root, fn), encoding='utf-8')))
                except ValueError:
                    pass
    return [n for lo, hi in NUMBER_BANDS for n in range(lo, hi) if n not in taken]


def capacity():
    return min(len(free_numbers()), MAX_IDS - OLD_COUNT)


# --------------------------------------------------------------------------------------------------- fetch
def http_get(url):
    req = urllib.request.Request(url, headers={'User-Agent': 'eds-decomp tools/newcards.py'})
    with urllib.request.urlopen(req, timeout=60) as r:
        return r.read()


def api(**query):
    """YGOPRODeck card list for the query; [] when nothing matches (the API answers 400)."""
    try:
        return json.loads(http_get(API + urllib.parse.urlencode(query)))['data']
    except urllib.error.HTTPError as e:
        if e.code == 400:
            return []
        raise


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


def load_cache():
    try:
        cache = json.load(open(CACHE, encoding='utf-8'))
    except FileNotFoundError:
        return {}
    if isinstance(cache, list):             # the earlier per-ID format
        cache = {c['name']: c for c in cache}
    return cache


def fetch(names, cache, force_art=False):
    """Download the cards `names` into the cache and make their art."""
    if names:
        print(f'newcards: fetching {len(names)} card(s) from YGOPRODeck')
    found = {}
    for k in range(0, len(names), 40):
        for c in api(name='|'.join(names[k:k + 40])):
            found[c['name'].lower()] = c
    unknown = [n for n in names if n.lower() not in found]
    if unknown:
        lines = []
        for n in unknown:
            guesses = [c['name'] for c in api(fname=n)]
            for word in sorted(set(re.findall(r'[\w.]{3,}', n)), key=len, reverse=True):
                if guesses:                 # a typo: search by its words, longest first, keep the closest names
                    break
                guesses = [c['name'] for c in api(fname=word)]
            guesses = difflib.get_close_matches(n, guesses, 3, 0.5) or guesses[:3]
            lines.append(f'  {n}' + (f'   (did you mean: {"; ".join(guesses)}?)' if guesses else ''))
            time.sleep(0.1)
        fail('YGOPRODeck has no card with these names:\n' + '\n'.join(lines))
    os.makedirs(f'{DIR}/cropped', exist_ok=True)
    os.makedirs(f'{DIR}/art', exist_ok=True)
    for n in names:
        c = found[n.lower()]
        passcode = f"{c['id']:08d}"
        cache[n] = {'name': n, 'passcode': passcode, 'type': c['type'], 'race': c['race'],
                    'attribute': c.get('attribute'), 'level': c.get('level', 0), 'atk': c.get('atk', 0),
                    'def': c.get('def', 0), 'desc': c['desc']}
        if 'Link' in c['type']:
            cache[n]['linkval'] = c.get('linkval', 0)
            cache[n]['linkmarkers'] = c.get('linkmarkers', [])
        image = next((i for i in c['card_images'] if i['id'] == c['id']), c['card_images'][0])
        jpg = f'{DIR}/cropped/{passcode}.jpg'
        if not os.path.exists(jpg):
            open(jpg, 'wb').write(http_get(image['image_url_cropped']))
            time.sleep(0.1)                 # YGOPRODeck allows 20 requests per second
    write_if_changed(CACHE, json.dumps(cache, ensure_ascii=False, indent=1, sort_keys=True))
    make_art([cache[n] for n in names], force_art)


def make_art(cards, force=False):
    for c in cards:
        png = f'{DIR}/art/{c["passcode"]}.png'
        if force or not os.path.exists(png):
            px, pal = to_art(f'{DIR}/cropped/{c["passcode"]}.jpg')
            open(png, 'wb').write(A.png_write(A.ART_W, A.ART_H, px, pal))


def cmd_fetch(force):
    cache = load_cache()
    names = [n for n, _ in load_list()]
    fetch(names, cache, force)
    print(f'newcards: {len(names)} cards in {DIR}/')


# --------------------------------------------------------------------------------------------------- build
def clean_text(s):
    for a, b in TEXT_FIXES.items():
        s = s.replace(a, b)
    s = re.sub(r'\s+', ' ', s).strip()
    if any(ord(ch) > 126 for ch in s):
        raise ValueError(f'unsupported character in {s[:60]!r}')
    if len(s) >= DESC_SIZE:
        s = s[:DESC_SIZE - 4].rsplit(' ', 1)[0] + '...'
    return s


def stats_word(c):
    """struct CardStats: def:9 atk:9 kind:2 type:5 level:4 attr:3; Magic/Trap keep their subtype in bits 17-19."""
    if c['type'] in ('Spell Card', 'Trap Card'):
        if c['race'] not in SUBTYPES:
            fail(f'{c["name"]}: {c["race"]} {c["type"]}s do not exist in EDS')
        sub = SUBTYPES[SUBTYPE_OVERRIDES.get(c['name'], c['race'])]
        if c['type'] == 'Spell Card':
            return CARD_TYPE_MAGIC << 20 | sub << 17
        return 1 << 29 | CARD_TYPE_TRAP << 20 | sub << 17
    if c['type'] == 'Token':
        c = dict(c, type='Normal Monster')     # stats like the Insect Monster Token's: kind 0
    race = EXTRA_RACES[c['race']][1] if c['race'] in EXTRA_RACES else c['race']
    if race not in MONSTER_TYPES or c.get('attribute') not in ATTRIBUTES:
        fail(f'{c["name"]}: a {c["race"]}/{c.get("attribute")} {c["type"]} cannot be shown in EDS (no such '
             f'monster type or attribute)')
    # An Xyz Monster's Rank (YGOPRODeck's level) and a Link Monster's Link Rating go in the Level field; a Link
    # Monster has no DEF.
    if 'Link' in c['type']:
        c = dict(c, level=c.get('linkval') or 0, **{'def': 0})
    if 'Pendulum' in c['type'] or not 1 <= (c['level'] or 0) <= 12:
        fail(f'{c["name"]}: {c["type"]} cards do not exist in EDS')
    # Synchro, Xyz and Link Monsters are Extra Deck monsters: kind FUSION in the stats word (gCardTypeEx tells them
    # apart).
    kind = (2 if any(k in c['type'] for k in ('Fusion', 'Synchro', 'XYZ', 'Link')) else 3 if 'Ritual' in c['type']
            else 0 if c['type'].startswith('Normal') else 1)
    atk, dfn = max(c['atk'] or 0, 0), max(c['def'] or 0, 0)
    if atk % 10 or dfn % 10 or atk > 5110 or dfn > 5110:
        fail(f'{c["name"]}: ATK/DEF {atk}/{dfn} cannot be stored (multiples of 10 up to 5110)')
    return (ATTRIBUTES[c['attribute']] << 29 | c['level'] << 25 | MONSTER_TYPES[race] << 20 | kind << 18
            | (atk // 10) << 9 | dfn // 10)


def type_ex(c):
    """gCardTypeEx byte of an added card (0 for Spells, Traps and plain monsters)."""
    if c['type'] in ('Spell Card', 'Trap Card', 'Token'):
        return 0
    b = (KIND_EX_SYNCHRO if 'Synchro' in c['type'] else KIND_EX_XYZ if 'XYZ' in c['type']
         else KIND_EX_LINK if 'Link' in c['type'] else 0)
    if 'Tuner' in c['type']:
        b |= TYPE_EX_TUNER
    # Effect: YGOPRODeck says "Effect" for most; Tuners are "Tuner Monster" ("Normal Tuner Monster" without), and an
    # Extra Deck monster's text has its effect after the materials line.
    extra = any(k in c['type'] for k in ('Synchro', 'Fusion', 'XYZ', 'Link'))
    if ('Effect' in c['type'] or c['type'] in ('Tuner Monster', 'Synchro Tuner Monster')
            or (extra and len(c['desc'].strip().splitlines()) > 1)):
        b |= TYPE_EX_EFFECT
    if c['race'] in EXTRA_RACES:
        b |= EXTRA_RACES[c['race']][0] << 4
    return b


SYNCHRO_TUNER_TYPE = 0x8000     # a rule's Tuner of one Type: 0x8000 | the EDS monster type (src/mod_synchro.c)
# Pre-errata Synchro material lines YGOPRODeck no longer carries (the API text is usually the current errata).
SYNCHRO_TUNER_OVERRIDES = {
    'Goyo Guardian': 0,           # pre-errata: 1 Tuner + 1+ non-Tuner (errata: 1 EARTH Tuner + …)
}


def synchro_tuner(c, numbers):
    """The card number a Synchro needs as its Tuner (0: any Tuner), from the first sentence of its text:
    '"Junk Synchron" + 1+ non-Tuner monsters' names one; '1 Tuner + 1+ non-Tuner monsters' takes any;
    '1 Fiend-Type Tuner + 1 or more non-Tuner monsters' takes a Tuner of that Type (SYNCHRO_TUNER_TYPE | type).
    SYNCHRO_TUNER_OVERRIDES fixes pre-errata material lines when the cached desc is errata."""
    name = c['name']
    if name in SYNCHRO_TUNER_OVERRIDES:
        return SYNCHRO_TUNER_OVERRIDES[name]
    m = re.match(r'\s*"([^"]+)"\s*\+', c['desc'])
    if m:
        if m.group(1) not in numbers:
            fail(f'{c["name"]}: its Synchro Material "{m.group(1)}" is not an added or original card')
        return numbers[m.group(1)]
    if re.match(r'\s*1 Tuner \+ 1(?:\+| or more) non-Tuner monsters', c['desc']):
        return 0
    m = re.match(r'\s*1 ([A-Za-z -]+?)(?:-Type)? Tuner \+ 1(?:\+| or more) non-Tuner monsters', c['desc'])
    if m:
        race = m.group(1)
        race = EXTRA_RACES[race][1] if race in EXTRA_RACES else race    # the closest EDS type
        if race not in MONSTER_TYPES:
            fail(f'{c["name"]}: its Tuner Type "{m.group(1)}" is not a monster Type')
        return SYNCHRO_TUNER_TYPE | MONSTER_TYPES[race]
    print(f'newcards: warning: {c["name"]}: Synchro Materials read as "1 Tuner + 1+ non-Tuner monsters"')
    return 0


def xyz_rule(c):
    """An Xyz Monster's materials from the first line of its text: (count, or more, Level, type, attribute), the type
    an EDS monster type and the attribute an EDS attribute, 0 for any. '2 Level 4 monsters', '2+ Level 4 monsters',
    '3 Level 4 Warrior monsters', '2 Level 4 LIGHT monsters'."""
    m = re.match(r'\s*(\d+)(\+| or more)? Level (\d+) (?:([A-Za-z -]+?)(?:-Type)? )?monsters', c['desc'])
    if not m:
        print(f'newcards: warning: {c["name"]}: Xyz Materials read as "2 Level {c["level"]} monsters"')
        return 2, 0, c['level'], 0, 0
    kind, typ, attr = m.group(4), 0, 0
    if kind:
        race = EXTRA_RACES[kind][1] if kind in EXTRA_RACES else kind
        if kind in ATTRIBUTES:
            attr = ATTRIBUTES[kind]
        elif race in MONSTER_TYPES:
            typ = MONSTER_TYPES[race]
        else:
            fail(f'{c["name"]}: its Xyz Materials "{kind}" are not a monster Type or Attribute')
    return int(m.group(1)), 1 if m.group(2) else 0, int(m.group(3)), typ, attr


def link_markers(c):
    """gCardLinkMarkers byte of a card (0 for all but Link Monsters)."""
    if 'Link' not in c['type']:
        return 0
    b = 0
    for m in c.get('linkmarkers') or []:
        if m not in LINK_MARKER_BITS:
            fail(f'{c["name"]}: unknown Link Arrow "{m}"')
        b |= LINK_MARKER_BITS[m]
    return b


# Link material kinds (src/mod_link.c): any monster, a Normal / Effect Monster, a Type, an Attribute, a Level.
LINK_MAT_ANY, LINK_MAT_NORMAL, LINK_MAT_EFFECT, LINK_MAT_TYPE, LINK_MAT_ATTRIBUTE, LINK_MAT_LEVEL = range(6)


def link_rule(c):
    """A Link Monster's materials from the first line of its text: (count, or more, kind, value). '1 Normal
    Monster', '2 monsters', '2+ Effect Monsters', '1 Level 1 monster', '2 Cyberse monsters', '2 LIGHT monsters'."""
    first = c['desc'].strip().splitlines()[0]
    m = re.match(r'\s*(\d+)(\+| or more)? (.*?)\s*[Mm]onsters?\s*$', first)
    if not m or ',' in first:
        print(f'newcards: warning: {c["name"]}: Link Materials read as "2 monsters" (from "{first}")')
        return 2, 0, LINK_MAT_ANY, 0
    count, more, what = int(m.group(1)), 1 if m.group(2) else 0, m.group(3).strip()
    what = re.sub(r'-Type$', '', what)
    lv = re.match(r'Level (\d+)$', what)
    if not what:
        return count, more, LINK_MAT_ANY, 0
    if what == 'Normal':
        return count, more, LINK_MAT_NORMAL, 0
    if what == 'Effect':
        return count, more, LINK_MAT_EFFECT, 0
    if lv:
        return count, more, LINK_MAT_LEVEL, int(lv.group(1))
    if what in ATTRIBUTES:
        return count, more, LINK_MAT_ATTRIBUTE, ATTRIBUTES[what]
    race = EXTRA_RACES[what][1] if what in EXTRA_RACES else what
    if race in MONSTER_TYPES:
        return count, more, LINK_MAT_TYPE, MONSTER_TYPES[race]
    print(f'newcards: warning: {c["name"]}: Link Materials read as "{count} monsters" (from "{first}")')
    return count, more, LINK_MAT_ANY, 0


def bcd(passcode):
    return bytes(int(passcode[i]) << 4 | int(passcode[i + 1]) for i in range(0, 8, 2))


def name_key(name):
    return name.lower().replace('-', '')     # the order of IDs 1..813 (wiki data/card-name-table.md)


def read_old(name, size):
    data = open(f'{IN}/cards__{name}.bin', 'rb').read()
    return [data[i * size:(i + 1) * size] for i in range(OLD_COUNT)], data


def pad4(b):
    return b + bytes(-len(b) % 4)


def update_cards_h(new):
    """Rewrite the added-cards block of enum CardNumber (and drop entries from before the block existed)."""
    text = strip_added_blocks(open(CARDS_H, encoding='utf-8').read())
    lines = [l for l in text.split('\n') if 'added card */' not in l]
    existing = {m.group(1) for l in lines for m in [re.match(r'\s+(CARD_\w+) =', l)] if m}
    block, syms = [BLOCK_BEGIN], []
    for c in sorted(new, key=lambda c: c['number']):
        sym = ident(c['name'])
        while sym in existing:
            sym += '_ADDED'
        existing.add(sym)
        block.append(f'    {sym} = {c["number"]},'.ljust(58) + f'/* id {c["id"]}, added card */')
        syms.append(sym)
    block.append(BLOCK_END)
    end = max(i for i, l in enumerate(lines) if l.startswith('};'))
    lines[end:end] = block
    write_if_changed(CARDS_H, '\n'.join(lines))
    return syms


def cmd_build():
    rows = load_list()
    leftovers = load_leftovers(rows)
    cache = load_cache()
    # (a Link Monster cached before its Link Rating and Arrows were kept is fetched again)
    missing = [n for n, _ in rows if n not in cache or ('Link' in cache[n]['type'] and 'linkval' not in cache[n])]
    missing += [n for n, _, _ in leftovers if n not in cache]
    if missing:
        fetch(missing, cache)
    make_art([cache[n] for n, _ in rows] + [cache[n] for n, _, _ in leftovers])
    numbers = free_numbers()
    cap = min(len(numbers), MAX_IDS - OLD_COUNT)
    n2i_old = struct.unpack('<2048H', open(f'{IN}/cards__number_to_id.json.bin', 'rb').read())
    if (sum(1 for n, _ in rows if cache[n]['type'] != 'Token' and n not in ENGINE_NUMBERS) > len(numbers)
            or len(rows) > MAX_IDS - OLD_COUNT):
        fail(f'{len(rows)} cards listed, but only {cap} fit (free card numbers: {len(numbers)}, card IDs: '
             f'{MAX_IDS - OLD_COUNT})')
    new = []
    free = iter(numbers)
    for k, (name, game_name) in enumerate(rows):
        c = dict(cache[name])
        if c['type'] == 'Token':
            if name not in TOKEN_NUMBERS:
                fail(f'{name}: a token needs the number of its token kind in TOKEN_NUMBERS (tools/newcards.py); '
                     f'known: {", ".join(TOKEN_NUMBERS)}')
            token = dict(TOKEN_NUMBERS[name])
            number = token.pop('number')
            c.update({'def': 0, **token})   # DEF 0 unless the token gives it (Ojama Token: 1000)
            if n2i_old[number]:
                fail(f'{name}: card number {number} already has a card')
        elif name in ENGINE_NUMBERS:
            number = ENGINE_NUMBERS[name]
            if n2i_old[number]:
                fail(f'{name}: card number {number} already has a card')
        else:
            number = next(free)
        c.update(id=OLD_COUNT + k, number=number, name=name, game_name=game_name)
        new.append(c)
    # NEWCARDS_LEFTOVERS (make demo): the leftover effect cards after the listed ones, each at its own card number
    # (the engine's effect key), so the original effect code plays them.
    used = {c['number'] for c in new}
    for name, game_name, number in leftovers:
        if n2i_old[number]:
            fail(f'{name}: card number {number} already has a card')
        if number in used:
            continue        # a listed card took the number (Big Bang Shot: 1313's piercing Equip)
        used.add(number)
        c = dict(cache[name])
        c.update(id=OLD_COUNT + len(new), number=number, name=name, game_name=game_name, leftover=True)
        new.append(c)
    if len(new) > cap:
        fail(f'{len(new)} cards with the leftover effect cards, but only {cap} fit')
    real = list(new)
    # NEWCARDS_PAD_TO (make demo): blank slots after the real cards until that many cards are added (at most the
    # capacity), for filling in later (e.g. in Hex Maniac Advance). Each takes a free card number; everything else is
    # zero (no name, text, stats or art; no password). They are left out of gCardIdsByName (the Deck Edit, Trunk and
    # Card Trading lists) and of cards.h, and the code that walks card IDs stops at CARD_ID_LAST_REAL.
    pad_to = min(int(os.environ.get('NEWCARDS_PAD_TO', '0') or 0), cap)
    while len(new) < pad_to:
        new.append({'id': OLD_COUNT + len(new), 'number': next(free), 'name': '', 'game_name': '', 'blank': True})
    total = OLD_COUNT + len(new)

    names, _ = read_old('names.json', NAME_SIZE)
    descs, _ = read_old('descriptions.json', DESC_SIZE)
    _, art = read_old('art', 1)
    stats, _ = read_old('stats.csv', 4)
    types_ex = [0] * OLD_COUNT
    markers = [0] * OLD_COUNT
    i2n, _ = read_old('id_to_number.json', 2)
    pws, _ = read_old('passwords.csv', 4)
    keys, _ = read_old('sort_keys.json', 2)
    n2i = list(struct.unpack('<2048H', open(f'{IN}/cards__number_to_id.json.bin', 'rb').read()))
    gfx = [art[i * ART_SIZE:(i + 1) * ART_SIZE] for i in range(OLD_COUNT)]
    pal_base = OLD_COUNT * ART_SIZE
    pals = [art[pal_base + i * PAL_SIZE:pal_base + (i + 1) * PAL_SIZE] for i in range(OLD_COUNT)]
    next_key = max(struct.unpack('<H', k)[0] for k in keys[1:]) + 1

    for c in new:
        if c.get('blank'):
            names.append(bytes(NAME_SIZE))
            descs.append(bytes(DESC_SIZE))
            gfx.append(bytes(ART_SIZE))
            pals.append(bytes(PAL_SIZE))
            stats.append(bytes(4))
            types_ex.append(0)
            markers.append(0)
            i2n.append(struct.pack('<H', c['number']))
            pws.append(bytes([0xFF] * 4))
            keys.append(struct.pack('<H', next_key))
            next_key += 1
            n2i[c['number']] = c['id']
            continue
        names.append(A.field_bytes(c['game_name'], NAME_SIZE))
        descs.append(A.field_bytes(clean_text(c['desc']), DESC_SIZE))
        w, h, px, plte = A.png_read(open(f'{DIR}/art/{c["passcode"]}.png', 'rb').read())
        if (w, h) != (A.ART_W, A.ART_H):
            fail(f'{DIR}/art/{c["passcode"]}.png ({c["name"]}) must be {A.ART_W}x{A.ART_H}')
        gfx.append(A._art_pack(px))
        pals.append(struct.pack('<64H', *[A.rgb_to_bgr555(p) for p in (plte + [(0, 0, 0)] * 64)[:64]]))
        stats.append(struct.pack('<I', stats_word(c)))
        types_ex.append(type_ex(c))
        markers.append(link_markers(c))
        i2n.append(struct.pack('<H', c['number']))
        pws.append(bytes([0xFF] * 4) if c['type'] == 'Token' else bcd(c['passcode']))   # tokens: no password
        keys.append(struct.pack('<H', next_key))
        next_key += 1
        n2i[c['number']] = c['id']

    all_names = [A.dec(n.split(b'\0')[0]) for n in names]
    order = sorted(list(range(1, FIRST_APPENDED)) + list(range(OLD_COUNT, OLD_COUNT + len(real))),
                   key=lambda i: (name_key(all_names[i]), i))     # (not the blank slots)
    order += list(range(FIRST_APPENDED, OLD_COUNT)) + [0]

    out = {
        'names': b''.join(names),
        'descriptions': b''.join(descs),
        'art_gfx': b''.join(gfx),
        'art_palettes': b''.join(pals),
        'stats': b''.join(stats),
        'id_to_number': pad4(b''.join(i2n) + b'\0\0'),   # + the 0 entry gCardIdToNumber_821 reads
        'passwords': b''.join(pws),
        'number_to_id': struct.pack('<2048H', *n2i),
        'sort_keys': pad4(b''.join(keys)),
        'name_order': pad4(struct.pack(f'<{len(order)}H', *order)),
        'type_ex': pad4(bytes(types_ex)),
        'link_markers': pad4(bytes(markers)),
    }
    for name, data in out.items():
        write_if_changed(f'{OUT}/newcards__{name}.bin', data)
    note = f'Generated by tools/newcards.py from {LIST}. Do not edit.'
    syms = update_cards_h(real)
    # HAS_CARD_<NAME> for every added card, so code for one card (src/added_effects.c) can be left out when the
    # card is not in the list. mod_decks.c also uses HAS_CARD_* for cards that exist in cards.h but are not in the
    # added-card list (base-game numbers re-used by the mod); define those too so format decks are not dropped.
    write_if_changed(COUNT_H, f'/* {note} */\n#define CARD_ID_COUNT_ADDED {len(new)}\n'
                     f'/* card IDs up to the last real card (the blank slots of NEWCARDS_PAD_TO follow) */\n'
                     f'#define CARD_ID_COUNT_REAL {OLD_COUNT + len(real)}\n'
                     + ''.join(f'#define HAS_{s} 1\n' for s in sorted(syms))
                     + in_game_has_defines(syms))
    # The Synchro Monsters and their Tuner (card number, 0 = any Tuner) for src/mod_synchro.c.
    numbers = {A.dec(names[i].split(b'\0')[0]): struct.unpack('<H', i2n[i])[0] for i in range(1, total)}
    numbers.update({c['name']: c['number'] for c in real})
    rules = [(c['number'], synchro_tuner(c, numbers)) for c in real if 'Synchro' in c['type']]
    write_if_changed(SYNCHRO_H, f'/* {note} */\n#define SYNCHRO_RULE_COUNT {len(rules)}\n'
                     '/* { Synchro card number, Tuner card number (0: any Tuner; 0x8000 | type: a Tuner of that '
                     'Type) } */\n'
                     '#define SYNCHRO_RULES { ' + ''.join(f'{{ {s}, {t} }}, ' for s, t in rules) + '{ 0, 0 } }\n')
    # The Link Monsters' material rules for src/mod_link.c.
    links = [(c['number'],) + link_rule(c) for c in real if 'Link' in c['type']]
    write_if_changed(LINK_H, f'/* {note} */\n#define LINK_RULE_COUNT {len(links)}\n'
                     '/* { Link card number, material count, 1: or more, kind (0 any, 1 Normal, 2 Effect, 3 Type, '
                     '4 Attribute, 5 Level), its value } */\n'
                     '#define LINK_RULES { ' + ''.join('{ %d, %d, %d, %d, %d }, ' % r for r in links)
                     + '{ 0, 0, 0, 0, 0 } }\n')
    # The Xyz Monsters' material rules for src/mod_xyz.c.
    xyz = [(c['number'],) + xyz_rule(c) for c in real if 'XYZ' in c['type']]
    write_if_changed(XYZ_H, f'/* {note} */\n#define XYZ_RULE_COUNT {len(xyz)}\n'
                     '/* { Xyz card number, material count, 1: or more, Level, type (0 any), attribute (0 any) } */\n'
                     '#define XYZ_RULES { ' + ''.join('{ %d, %d, %d, %d, %d, %d }, ' % r for r in xyz)
                     + '{ 0, 0, 0, 0, 0, 0 } }\n')
    write_if_changed(COUNT_INC, f'@ {note}\n\t.set CARD_ID_COUNT_ADDED, {len(new)}\n'
                                f'\t.set CARD_ID_COUNT, {total}\n')
    write_if_changed(GEN_TSV, 'id\tnumber\tpasscode\tname\tgame_name\n' + ''.join(
        f'{c["id"]}\t{c["number"]}\t{c.get("passcode", "")}\t{c["name"]}\t{c["game_name"]}\n' for c in new))
    stamp = f'{total}\n{os.path.abspath(LIST)}\n'
    stamp_path = os.environ.get('NEWCARDS_STAMP', f'{OUT}/.newcards')
    with open(stamp_path, 'w') as f:    # always: make compares its time (what depends on it must rebuild)
        f.write(stamp)
    # The tables are shared by every list (make vs make demo): the other lists' stamps no longer describe them.
    for other in os.listdir(OUT):
        path = os.path.join(OUT, other)
        if other.startswith('.newcards') and os.path.abspath(path) != os.path.abspath(stamp_path):
            os.remove(path)
    print(f'newcards: {len(new)} added cards (IDs {OLD_COUNT}-{total - 1}), room for {cap - len(new)} more '
          f'from {LIST}')


def main():
    global LIST
    LIST = os.environ.get('NEW_CARDS_LIST', DEFAULT_LIST)
    a = sys.argv[1:]
    if a[:1] == ['fetch']:
        cmd_fetch('--force' in a)
    elif a[:1] == ['build']:
        cmd_build()
    elif a[:1] == ['capacity']:
        print(f'{capacity()} cards fit ({len(load_list())} listed)')
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main()
