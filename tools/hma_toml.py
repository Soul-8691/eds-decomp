#!/usr/bin/env python3
"""Write the Hex Maniac Advance metadata (a .toml next to the ROM) for the demo build, from baserom.toml.

  python3 tools/hma_toml.py [--rom eds_demo.gba] [--sym eds_demo.sym] [--base baserom.toml] [--out eds_demo.toml]

baserom.toml (repo root, not committed) is HMA's metadata for the original ROM. Run by `make demo` and
`make release-demo` after the ROM is linked (eds_demo.toml, eds_release_demo.toml); it does nothing without baserom.toml. For every entry it finds what the original address was in the
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
  Added         anchors baserom.toml lacks, by symbol (EXTRA_ANCHORS): field miniature palette + ucs8 sprites,
                Deck Edit ring art (graphics.cards.icons*). And the mod's own tables (mod_anchors), by the ELF's
                symbols (static ones too: build/eds.elf's .symtab): the format packs (data.packs.format.*: each
                pack's pool of card and weight, which is all a pull reads, flags, format, release date,
                portrait, name; list formatpacks), the card prices (data.prices.cards: per format, card and DP; list
                formats), every deck of src/mod_decks.c (data.decks.mod.<deck>: main + Fusion, side, DeckList) and
                the debug deck recipes (data.decks.recipes: name, DeckList, side, side count), and the pre-duel
                pictures (graphics.preduel.<format>: palette, QR code sprite and palette, 8bpp picture).
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
#   src/mod_field_icons.c), after the field icons' palette, frames, pose maps and per-card miniature sprites
#   (ucs8x2x2 at DATA_MINIS, palette-relative 1-64 in graphics.cards.fieldicons.artpalette): two arrays by card ID,
#   every 16-colour palette (0x20
#   bytes, entry 0 transparent) then every 16x16 4bpp image (2x2 row-major tiles, 0x80 bytes). They are HMA-CE's
#   inline image tables (ArrayRun.TryParseInlineImageTable: a row is one bare `ucp`/`ucs` format and nothing
#   else), as graphics.cards.images / graphics.cards.palettes; the images' palette hint is the palettes table, whose
#   row of the same index HMA uses.
FIELD_ICON_MINIS = 0x7980
MINI_STRIDE = 256             # 8bpp 2x2 tiles (tools/cardicons.py, src/mod_field_icons.c)
ICON_PALETTE_SIZE = 0x20


def icon_palettes(count):
    return (FIELD_ICON_MINIS + count * MINI_STRIDE + 3) & ~3


def field_mini_artpalette(_count):
    """The 64 shared BGR555 colours at DATA_PALETTE (one ucp4:0123 run, 128 bytes; not a multi-row table)."""
    return 0


EXTRA_ANCHORS = [
    ('graphics.cards.fieldicons.artpalette', 'gModFieldIconData', field_mini_artpalette, '`ucp4:0123`'),
    ('graphics.cards.fieldicons.miniatures', 'gModFieldIconData', lambda _c: FIELD_ICON_MINIS,
     '[`ucs8x2x2|graphics.cards.fieldicons.artpalette`]cardnumbers'),
    ('graphics.cards.icons.palettes', 'gModFieldIconData', icon_palettes, '[`ucp4`]cardnumbers'),
    ('graphics.cards.icons.images', 'gModFieldIconData', lambda count: icon_palettes(count) + count * ICON_PALETTE_SIZE,
     '[`ucs4x2x2|graphics.cards.icons.palettes`]cardnumbers'),
]


# The header's guide: (anchor name or prefix, what it is, whether the game uses it). Keep it in step with
# EXTRA_ANCHORS, mod_anchors and baserom.toml. The status is a string, or (release build, debug build) when it differs
# (make release / release-demo vs make / make demo: build/include/mod_config.h MOD_RELEASE); a more specific prefix
# comes before a general one. No anchor name may also be the start of another (a.b and a.b.c): HMA's Goto menu then
# shows a.b but not a.b.*, so every anchor is a leaf (.picture, .list, .images, .image).
USED, UNUSED = 'used', 'UNUSED'
ADDED_GUIDE = [
    ('graphics.cards.icons', "every card's art icon (Deck Edit's ring): .palettes, .images (16x16 4bpp)", USED),
    ('graphics.cards.fieldicons', "the field icons: .artpalette (64 shared colours), .miniatures (each card's "
                                  "16x16 8bpp miniature)", USED),
    ('data.cards.typeex', "each card ID's extended type (gCardTypeEx): kind (none, Synchro, Xyz, Link; the stats "
                          "word stores those as Fusion), effect, Tuner, and race (a race EDS lacks: Psychic, Wyrm, "
                          "Cyberse, Illusion, Creator-God, Divine-Beast; statstype = the stats word's)", USED),
    ('data.cards.linkmarkers', "each card ID's Link Arrows (gCardLinkMarkers): top-left ... bottom-right", USED),
    ('data.cards.addedeffects', "the added cards' effect rows (gAddedCardEffects): card, then the resolve, check, "
                                "prepare, chainA, chainB handlers; searched after data.cards.effects; last row 0xFFFF",
     USED),
    ('data.packs.format.pools', "the shop's TCG and usage format packs: each pack's cards and pull weights (all a "
                                "pull reads; chance = weight / total)", USED),
    ('data.packs.format', "per format pack: .names, .flags (1 = usage pack), .formats, .releasedates (yyyymmdd), "
                          ".portraits (cover card)", USED),
    ('text.packs.format', 'the format pack names (ASCII)', USED),
    ('data.prices.cards', "the DP to buy a card by password, per format: card and price", USED),
    ('data.decks.mod.yugideck', "Yugi's deck (30 7 Colored Fish)", 'UNUSED (Yugi is a hidden duelist)'),
    ('data.decks.mod.testdeck', "the Test Duel's deck (test.ydk)", (UNUSED, 'used (the Test Duel, debug builds only)')),
    ('data.decks.mod.reddeck', "Tea's deck (LR-Chaos Turbo)",
     (UNUSED, 'debug recipe only (Tea is a hidden duelist)')),
    ('data.decks.mod.joeydeck', "Joey's deck (sdlkilla-Warrior)",
     (UNUSED, 'debug recipe only (Joey is a hidden duelist)')),
    ('data.decks.mod.bakuradeck', "Bakura's deck (geistD-Goat Control)",
     (UNUSED, 'debug recipe only (Bakura is a hidden duelist)')),
    ('data.decks.mod', "the format opponents' decks (data.decks.mod.<format><deck>): .list the DeckList (cards, "
                       "count), .main the main + Fusion Deck, .side the Side Deck",
     ('used (the format opponents)', 'used (the format opponents, and the debug recipes)')),
    ('data.decks.recipes', "the debug menu's deck recipes: name, DeckList, Side Deck, side count; last row the "
                           "terminator", (UNUSED + ' (GetRecipePreset returns 0 in the release)', USED)),
    ('text.decks.recipes', 'the deck recipe names (ASCII)', (UNUSED, USED)),
    ('graphics.preduel', "the pre-duel pictures per format: <format>.picture the 240x96 picture, .palette its 224 "
                         "colours, .qr.image the QR code (1 = dark module), .qr.palette", USED),
    ('data.preduel', 'per pre-duel picture: .qrsize, the QR code modules a side (0 = none)', USED),
    ('data.opponents', "the opponents by duelist ID (list opponents): kind (hidden, format, deck), unlock order, format, "
                       ".name, and .portrait: the card whose art the opponent select and the Records screen show (0: "
                       "the original portrait; 0x8000 | page << 4 | slot: an original page's portrait, e.g. Solomon)",
     USED),
    ('text.opponents', 'the opponent names (ASCII)', USED),
]
# MatchedWords the mod adds (same guide).
ADDED_WORDS_GUIDE = [
    ('data.constants.packs.format.usagewins', "wins against a format that open its usage pack",
     (USED, UNUSED + ' (debug builds open every pack)')),
    ('data.constants.formats.unlock', "per format: wins against the format before it that open it, and with it its "
                                      "TCG packs", (USED, UNUSED + ' (debug builds open every format)')),
]
ADDED_LISTS_GUIDE = [
    ('formatpacks', 'the 50 format pack names (indexes data.packs.format.*)', USED),
    ('formats', 'the 15 Campaign formats in banlist order (indexes data.prices.cards, data.packs.format.formats, '
                'data.opponents)', USED),
    ('opponents', 'the duelist IDs 0-24 with the opponent each is in this build (indexes data.opponents)', USED),
    ('opponentkinds', 'hidden, format, deck (data.opponents\' kind)', USED),
    ('cardkindsex', 'none, synchro, xyz, link (data.cards.typeex\'s kind)', USED),
    ('racesex', 'statstype and the races EDS lacks (data.cards.typeex\'s race)', USED),
]

# What baserom.toml (the original game) brings, what this script does to it, and whether the mod still uses it.
VANILLA_GUIDE = [
    ('graphics.cards.images', "every card's 72x80 art, by card ID, with graphics.cards.palettes (moved to the end of "
                              "the ROM with the added cards; one row per card ID)", USED),
    ('graphics.cards.palettes', "the card art palettes, by card ID (moved, one row per card ID)", USED),
    ('data.cards.stats', "every card's stats word (ATK, DEF, kind, type, Level, attribute), by card ID (moved)", USED),
    ('data.cards.passwords', "every card's password (BCD), by card ID (moved; FFFFFFFF = none)", USED),
    ('data.cards.numbers', "card ID -> card number (moved; one row per card ID)", USED),
    ('data.cards.internalnumbers', "card number -> card ID (one row per card number in use)", USED),
    ('data.cards.effects', "the original effect table: card number or effect key, then the resolve, check, prepare, "
                           "chainA, chainB handlers (the added cards' rows: data.cards.addedeffects)", USED),
    ('data.cards.fusion', "the Fusion recipes", USED),
    ('data.cards.ritual', "the Ritual recipes", USED),
    ('data.decks.all', "every original duelist's deck (pointer, count), by the duelists list",
     'mostly UNUSED (every format opponent plays a data.decks.mod deck; only Solomon and Debug fall back to theirs '
     'until a recipe is picked)'),
    ('data.decks.initial', "the original starter deck's pools: .cards and .stats per pool",
     'UNUSED (the three starter decks are the mod\'s: BuildModStarterDeck)'),
    ('data.packs', "the original Campaign booster packs: per pack, a card list per rarity (by the rarities list)",
     'UNUSED (not in the shop, and Campaign rewards open format packs: ModRewardPack)'),
    ('data.specialpacks', "the special packs (Weekly Yu-Gi-Oh! A and B, the magazine, the Grandpa Cup prize): their "
                          "cards per rarity", 'UNUSED (not in the shop, and Campaign rewards open format packs: ModRewardPack)'),
    ('graphics.packs', "the original booster pack covers, with graphics.palettes.packs",
     'UNUSED (as data.packs)'),
    ('graphics.palettes.packs', "the pack covers' palette", 'UNUSED (as data.packs)'),
    ('graphics.titlescreen.gamelogo', "the title screen's game logo",
     'UNUSED (ModTitle_Load hides it: the title is "Yak\'s YGO Hack")'),
    ('graphics.titlescreen', "the title screen: .background, .medal, with graphics.palettes.titlescreen.*", USED),
    ('graphics.palettes.titlescreen.logo', "the game logo's palette", UNUSED + ' (as the logo)'),
    ('graphics.palettes.titlescreen', "the title screen palettes", USED),
]
VANILLA_WORDS_GUIDE = [
    ('data.constants.packs.rarities', "the original packs' rarity odds (.total and each rarity's weight)",
     'UNUSED (as data.packs)'),
    ('data.constants.packs.unlocks.2wins*', "IsCampaignLevel2Unlocked: more than this many wins against each of "
                                            "duelists 1-5 (Yugi, Tea, Joey, Tristan, Bakura) raise the campaign level: "
                                            "the original pack unlocks and the \"New Duelists\" notice (text 350); the "
                                            "Records pages now follow the formats (ModRecordHasNextPage)",
     'effectively UNUSED (four of those duelists are hidden, so it never passes)'),
    ('data.constants.packs.unlocks', "IsPackUnlocked: the wins that unlock the original Campaign packs",
     'UNUSED (only the original pack list reads it, and nothing shows that list; the "new packs" notice follows the '
     'format packs: data.constants.packs.format.usagewins, data.constants.formats.unlock.*)'),
    ('data.constants.fusion2', "a Fusion constant of the original", USED),
    ('data.constants.cards.limit2', "the card ID count in GeneratePackCards (random pack cards): the last real card",
     USED),
    ('data.constants.cards.limit', "the card ID count limits in the code: now the build's count (or the last real "
                                   "card's, before make demo's blank slots); sites whose code no longer holds it are "
                                   "listed below", 'used (two sites are debug screens: DebugCardDetail_Browse, '
                                                   'DebugGetAllCards)'),
    ('data.constants.effects.limit', "the original effect table's row count", USED),
]
VANILLA_LISTS_GUIDE = [
    ('cardnumbers', "card names by card ID: the original cards, then the added ones (and make demo's blank slots, "
                    "'Blank <id>')", USED),
    ('cardinternalnumbers', "card names by card number, with the added cards at their numbers", USED),
    ('rarities', "the pack rarities", USED),
    ('types', "the card types", USED),
    ('races', "the monster types", USED),
    ('attributes', "the attributes", USED),
    ('duelists', "the original duelists", USED),
]
GUIDE_RELEASE = False       # set by main from build/include/mod_config.h


def guide_lines(entries, names, suffix=''):
    """'#   prefix(.*) [N] (status): what' for each (prefix, what, status) that names hold, wrapped at 118 columns."""
    import textwrap
    out = []
    taken = set()
    for prefix, what, status in entries:
        if prefix.endswith('*'):        # 'a.b*': every name that starts with a.b
            hits = [n for n in names if n.startswith(prefix[:-1]) and n not in taken]
        else:
            hits = [n for n in names if (n == prefix or n.startswith(prefix + '.')) and n not in taken]
        if not hits:
            continue
        taken.update(hits)
        if isinstance(status, tuple):
            status = status[0] if GUIDE_RELEASE else status[1]
        star = '' if prefix.endswith('*') else '.*' if any(n != prefix for n in hits) else ''
        count = f' [{len(hits)}]' if len(hits) > 1 else ''
        out += textwrap.wrap(f'{prefix}{star}{count}{suffix} -- {status}: {what}', 112, initial_indent='#   ',
                             subsequent_indent='#       ')
    return out


def gen_hash(items, seed=0xCAFEBABE):
    """HMA's StoredList.GenerateHash (src/HexManiac.Core/Models/StoredMetadata.cs)."""
    h = seed
    for item in items:
        h = ((h << 3) | (h >> 29)) & 0xFFFFFFFF
        if item is None:
            continue
        h ^= struct.unpack('<I', hashlib.sha256(item.encode('utf-8')).digest()[:4])[0]
    return '%08X' % h


# ---- the mod's tables, by the ELF's symbols ----

def elf_symbols(path):
    """{name: (address, size)} of every data symbol in the ELF's .symtab, the static ones too, keyed also as
    'file.o:name' for the static ones (their file is the STT_FILE symbol before them)."""
    data = open(path, 'rb').read()
    shoff, = struct.unpack_from('<I', data, 0x20)
    shentsize, shnum = struct.unpack_from('<HH', data, 0x2E)
    sections = [struct.unpack_from('<IIIIIIIIII', data, shoff + i * shentsize) for i in range(shnum)]
    out = {}
    for sh in sections:
        if sh[1] != 2:                      # SHT_SYMTAB
            continue
        strtab = sections[sh[6]]
        def name_at(off, base=strtab[4]):
            end = data.index(b'\0', base + off)
            return data[base + off:end].decode('latin-1')
        current = ''
        for i in range(sh[5] // 16):
            st_name, value, size, info, _, shndx = struct.unpack_from('<IIIBBH', data, sh[4] + i * 16)
            kind, bind = info & 0xF, info >> 4
            name = name_at(st_name)
            if kind == 4:                   # STT_FILE
                current = os.path.basename(name)
            elif kind == 1 and size:        # STT_OBJECT
                out.setdefault(name, (value, size))
                if bind == 0:
                    out[f'{current}:{name}'] = (value, size)
    return out


def c_string(rom, address):
    off = address - ROM_BASE
    return rom[off:rom.index(b'\0', off)].decode('latin-1')


def anchor_id(text):
    """An HMA anchor name part: lower case letters and digits."""
    return re.sub(r'[^a-z0-9]', '', text.lower())


def mod_anchors(elf, rom, rom_sym):
    """([(name, address, format)], [(list name, items)], [report], [(matched word name, address, length)]) for the
    mod's tables."""
    anchors, lists, report, words = [], [], [], []

    def sym(name, unit=None):
        return elf.get(f'{unit}:{name}') if unit else elf.get(name)

    def add(name, s, fmt):
        anchors.append((name, s[0] - ROM_BASE, fmt))

    def word(address):
        return struct.unpack_from('<I', rom, address - ROM_BASE)[0]

    # Format packs (src/mod_format_packs.c, build/include/format_pack_data.h): the shop's TCG and usage packs.
    pools = sym('sFormatPackPools', 'mod_format_packs.o')
    names = sym('sFormatPackNames', 'mod_format_packs.o')
    if pools and names:
        count = pools[1] // 8
        packs = [c_string(rom, word(names[0] + 4 * i)) for i in range(count)]
        lists.append(('formatpacks', packs))
        add('data.packs.format.pools', pools,
            '[pool<[card:cardinternalnumbers weight:]/count> count: unused:]formatpacks')
        add('data.packs.format.names', names, '[name<>]formatpacks')
        for i in range(count):
            p = word(names[0] + 4 * i)
            anchors.append((f'text.packs.format.pack{i}', p - ROM_BASE, '`asc`%d' % (len(packs[i]) + 1)))
        for field_name, symbol, fmt in (('flags', 'sFormatPackFlags', '[usage.]formatpacks'),
                                        ('formats', 'sFormatPackFormatIndex', '[format.formats]formatpacks'),
                                        ('releasedates', 'sFormatPackReleaseDate', '[yyyymmdd::]formatpacks'),
                                        ('portraits', 'sFormatPackPortraitNumber',
                                         '[card:cardinternalnumbers]formatpacks')):
            s = sym(symbol, 'mod_format_packs.o')
            if s:
                add(f'data.packs.format.{field_name}', s, fmt)
    else:
        report.append('no sFormatPackPools / sFormatPackNames in the ELF: the format packs are left out')

    # Card prices (src/mod_dp.c, build/include/card_prices.h): per format, { card number, DP }.
    costs = sym('sFormatCardCosts', 'mod_dp.o')
    if costs:
        text = open('build/include/card_prices.h', encoding='utf-8').read()
        formats = re.findall(r'/\* (.+?) \*/\s*static const struct CardCost sCardCosts\d+', text)
        if len(formats) != costs[1] // 8:
            formats = [f'Format {i}' for i in range(costs[1] // 8)]
        lists.append(('formats', formats))
        add('data.prices.cards', costs, '[costs<[card:cardinternalnumbers price:]/count> count: unused:]formats')
    else:
        report.append('no sFormatCardCosts in the ELF: the card prices are left out')

    # Decks (src/mod_decks.c): every DeckList (main deck + Fusion Deck, card numbers), its side deck.
    decks = sorted(k.split(':', 1)[1] for k in elf if k.startswith('mod_decks.o:') and k.endswith('List'))
    deck_names = {}
    for list_sym in decks:
        base = list_sym[:-4]
        s = sym(list_sym, 'mod_decks.o')
        if s[1] != 8:
            continue
        name = 'data.decks.mod.' + anchor_id(base[1:])
        cards = word(s[0])
        main = next((elf[k] for k in elf if k.startswith('mod_decks.o:') and elf[k][0] == cards), None)
        if main is None:
            report.append(f'deck {list_sym}: its card array is not a symbol, left out')
            continue
        deck_names[s[0]] = name
        add(name + '.main', main, '[card:cardinternalnumbers]%d' % (main[1] // 2))
        add(name + '.list', s, '[cards<> count: unused:]1')
        for side_sym in (base + 'Side', base.replace('StarterDeck', 'Deck') + 'Side'):
            side = sym(side_sym, 'mod_decks.o')
            if side:
                add(name + '.side', side, '[card:cardinternalnumbers]%d' % (side[1] // 2))
                break
    recipes = sym('sRecipePresets', 'mod_decks.o')
    if recipes:
        add('data.decks.recipes', recipes, '[name<> list<> side<> sidecount::]%d' % (recipes[1] // 16))
        for i in range(recipes[1] // 16):
            p = word(recipes[0] + 16 * i)
            if p:
                n = c_string(rom, p)
                anchors.append((f'text.decks.recipes.recipe{i}', p - ROM_BASE, '`asc`%d' % (len(n) + 1)))

    # The opponents (src/mod_opponents.c sOpponents, struct ModOpponent, 12 bytes, by duelist ID): kind, unlock
    # order, banlist (format), name, and the portrait: the card whose art the opponent select and the Records screen
    # draw (a card number; 0 = the original page's portrait; 0x8000 | page << 4 | slot = an original page's portrait).
    opponents = sym('sOpponents', 'mod_opponents.o')
    if opponents:
        ids = {int(v): n for n, v in re.findall(r'DUELIST_(\w+) = (\d+)',
                                                open('include/constants/game.h', encoding='utf-8').read())}
        labels, seen = [], set()
        for i in range(opponents[1] // 12):
            p = word(opponents[0] + 12 * i + 4)
            name = c_string(rom, p) if p else ''
            labels.append(ids.get(i, 'none').lower() + (f' - {name}' if name else ''))
            if p and p not in seen:
                seen.add(p)
                anchors.append((f'text.opponents.{anchor_id(name)}', p - ROM_BASE, '`asc`%d' % (len(name) + 1)))
        lists.append(('opponents', labels))
        lists.append(('opponentkinds', ['hidden', 'format', 'deck']))
        add('data.opponents', opponents, '[kind.opponentkinds order. format.formats unused. name<> '
                                         'portrait:cardinternalnumbers padding:]opponents')
    else:
        report.append('no sOpponents in the ELF: the opponents table is left out')

    # The extended card data by card ID (tools/newcards.py, data/rodata_08087FD0.s), one byte each:
    #   gCardTypeEx: bits 0-1 the Extra Deck kind the stats word stores as Fusion (enum CardKindEx), bit 2 has an
    #   effect, bit 3 Tuner, bits 4-7 a race EDS lacks (enum RaceEx; 0 = the stats word's type);
    #   gCardLinkMarkers: a Link Monster's arrows, bits TL, T, TR, L, R, BL, B, BR.
    type_ex = rom_sym.get('gCardTypeEx')
    if type_ex:
        lists.append(('cardkindsex', ['none', 'synchro', 'xyz', 'link']))
        lists.append(('racesex', ['statstype', 'psychic', 'wyrm', 'cyberse', 'illusion', 'creatorgod',
                                  'divinebeast']))
        anchors.append(('data.cards.typeex', type_ex[0] - ROM_BASE,
                        '[typeex.|t|kind:cardkindsex|effect.|tuner.|race::racesex]cardnumbers'))
    else:
        report.append('no gCardTypeEx in the .sym: the extended card types are left out')
    markers = rom_sym.get('gCardLinkMarkers')
    if markers:
        anchors.append(('data.cards.linkmarkers', markers[0] - ROM_BASE,
                        '[arrows.|t|topleft.|top.|topright.|left.|right.|bottomleft.|bottom.|bottomright.]cardnumbers'))

    # The added cards' effect rows (src/added_effects.c gAddedCardEffects): struct CardEffect, 24 bytes, as
    # baserom.toml's data.cards.effects (gCardEffects), which FindCardEffect searches first; the last row is the
    # 0xFFFF terminator.
    effects = rom_sym.get('gAddedCardEffects')
    if effects:
        anchors.append(('data.cards.addedeffects', effects[0] - ROM_BASE,
                        '[card:cardinternalnumbers padding: routine1::|h routine2::|h routine3::|h routine4::|h '
                        'routine5::|h]%d' % (effects[1] // 24)))
    else:
        report.append('no gAddedCardEffects in the .sym: the added effect rows are left out')

    # The wins that unlock packs: a usage pack opens at gModUsagePackWins[0] wins against its format
    # (src/mod_format_packs.c), a TCG pack with its format, which opens at sWinsToUnlockNext[k] wins against the format
    # before it (src/mod_opponents.c; named by the format it opens, from the table's comments).
    usage_wins = rom_sym.get('gModUsagePackWins')
    if usage_wins:
        words.append(('data.constants.packs.format.usagewins', usage_wins[0] - ROM_BASE, 1))
    else:
        report.append('no gModUsagePackWins in the .sym: the usage pack wins are left out')
    unlock = sym('sWinsToUnlockNext', 'mod_opponents.o')
    if unlock:
        src = open('src/mod_opponents.c', encoding='utf-8').read()
        table = src[src.index('sWinsToUnlockNext[FORMAT_COUNT - 1] = {'):]
        opens = re.findall(r'/\*\s*[^*]*?->\s*([^*]+?)\s*\*/', table[:table.index('};')])
        for k in range(unlock[1]):
            what = anchor_id(opens[k]) if k < len(opens) else f'format{k + 1}'
            words.append((f'data.constants.formats.unlock.{what}', unlock[0] - ROM_BASE + k, 1))

    # The pre-duel pictures (src/mod_preduel.c, data/mod_preduel_data.s, layout in tools/preduel_image.py): the
    # picture's 224 colours, the QR code's size, palette and 4bpp sprite, the picture's 8bpp tiles (colour - 32).
    labels = re.findall(r'^(gModPreDuel\w+):', open('data/mod_preduel_data.s', encoding='utf-8').read(), re.M)
    for label in labels:
        if label not in rom_sym:
            report.append(f'pre-duel picture {label}: not in the .sym, left out')
            continue
        base = rom_sym[label][0] - ROM_BASE
        name = 'graphics.preduel.' + anchor_id(label[len('gModPreDuel'):])
        anchors += [
            (name + '.palette', base, '`ucp4:0123456789ABCD`'),
            (f'data.preduel.{anchor_id(label[len("gModPreDuel"):])}.qrsize', base + 0x1C0, '[modules::]1'),
            (name + '.qr.palette', base + 0x1C4, '`ucp4`'),
            (name + '.qr.image', base + 0x1E4, f'`ucs4x4x4|{name}.qr.palette`'),
            (name + '.picture', base + 0x3E4, f'`ucs8x30x12|{name}.palette`'),
        ]
    return anchors, lists, report, words


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


def real_card_count():
    """CARD_ID_COUNT_REAL: the card IDs up to the last real card (make demo's blank slots follow them)."""
    m = re.search(r'CARD_ID_COUNT_REAL (\d+)', open('build/include/card_count.h').read())
    return int(m.group(1)) if m else card_count()


def added_cards():
    """(id, number, in-game name) of the added cards."""
    out = []
    lines = open('build/new_cards.tsv', encoding='utf-8').read().splitlines()
    for line in lines[1:]:
        p = line.split('\t')
        if len(p) >= 5:
            out.append((int(p[0]), int(p[1]), p[4] or p[3] or f'Blank {p[0]}'))     # make demo's blank slots
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
    ap.add_argument('--elf', default='build/eds.elf')
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
    real_delta = real_card_count() - ORIGINAL_CARD_IDS
    keep = []
    for header, lines in blocks:
        if header == '[[MatchedWords]]':
            name, addr, length = field(lines, 'Name'), int(field(lines, 'Address'), 16), int(field(lines, 'Length'))
            new, info = mover.move_word(name, addr, length, limit_delta)
            if new is None and name in CARD_LIMIT_WORDS and real_delta != limit_delta:
                # the code that stops at the last real card (CARD_ID_LAST_REAL: not the blank slots)
                new, info = mover.move_word(name, addr, length, real_delta)
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
    mod_lists = []
    elf = elf_symbols(a.elf) if os.path.exists(a.elf) else {}
    # build/eds.elf is shared by every build (make, make release, make release-demo): use it only when it is this
    # ROM's (every global data symbol of the .sym at the same address).
    local = {k.split(':', 1)[1] for k in elf if ':' in k}     # (static names can repeat across files)
    stale = [k for k, (address, _) in elf.items()
             if ':' not in k and k not in local and k in sym and sym[k][0] != address]
    if elf and stale:
        report.append(f'{a.elf} is not the ELF of {a.rom} (another build since; e.g. {stale[0]}): the mod tables '
                      f'(packs, prices, decks, recipes) are left out until {a.rom} is built again')
        elf = {}
    elif not elf:
        report.append(f'no {a.elf}: the mod tables (packs, prices, decks, recipes) are left out')
    # (the tables found by the .sym, the added effect rows and the pre-duel pictures, are there either way)
    anchors, mod_lists, mod_report, mod_words = mod_anchors(elf, new_rom, sym)
    if elf:
        report.extend(mod_report)
    for name, address, fmt in anchors:
        if name in names:
            continue
        names.add(name)
        extra.append(('[[NamedAnchors]]', ['[[NamedAnchors]]', f"Name = '''{name}'''", f'Address = 0x{address:06X}',
                                           f"Format = '''{fmt}'''", f"DefaultHash = '''{gen_hash([fmt])}'''", '']))
    last = max(i for i, (header, _) in enumerate(keep) if header == '[[NamedAnchors]]')
    keep[last + 1:last + 1] = extra
    # the mod's MatchedWords, after the last of baserom.toml's
    word_names = {field(lines, 'Name') for header, lines in keep if header == '[[MatchedWords]]'}
    new_words = [('[[MatchedWords]]', ['[[MatchedWords]]', f"Name = '''{name}'''", f'Address = 0x{address:06X}',
                                       f'Length = {length}', ''])
                 for name, address, length in mod_words if name not in word_names]
    last = max(i for i, (header, _) in enumerate(keep) if header == '[[MatchedWords]]')
    keep[last + 1:last + 1] = new_words
    list_names = {field(lines, 'Name') for header, lines in keep if header == '[[List]]'}
    for list_name, items in mod_lists:
        if list_name in list_names:
            continue
        lines = ['[[List]]', f"Name = '''{list_name}'''", "DefaultHash = ''''''", '0 = [', ']', '']
        set_list_items(lines, items)
        keep.append(('[[List]]', lines))

    text = []
    for header, lines in keep:
        text.extend(lines)
    head = ['# Generated by tools/hma_toml.py from baserom.toml for %s (%d card IDs, card numbers 0-%d). Do not edit;'
            % (os.path.basename(a.rom), count, numbers - 1),
            '# it is written again by every build of that ROM (`make demo`: eds_demo.toml, `make release-demo`: '
            'eds_release_demo.toml, `make release-demo-extra`: eds_release_demo_extra.toml).']
    # A guide to the file (only the groups present in this build): first what baserom.toml (the original game)
    # brings, then what the mod adds. [N] is how many entries a group has.
    added = {lines[1][len("Name = '''"):-3] for header, lines in extra}
    added_words = {n for n, _, _ in mod_words}
    added_lists = {n for n, _ in mod_lists}
    base_anchors = [field(lines, 'Name') for header, lines in keep
                    if header == '[[NamedAnchors]]' and field(lines, 'Name') not in added]
    base_words = [field(lines, 'Name') for header, lines in keep
                  if header == '[[MatchedWords]]' and field(lines, 'Name') not in added_words]
    base_lists = [field(lines, 'Name') for header, lines in keep
                  if header == '[[List]]' and field(lines, 'Name') not in added_lists]
    global GUIDE_RELEASE
    GUIDE_RELEASE = bool(re.search(r'#define MOD_RELEASE 1', open('build/include/mod_config.h').read())) \
        if os.path.exists('build/include/mod_config.h') else False
    head += ['#', '# Each entry: name -- whether the game uses it in this build (%s build) : what it is. [N] = entries.'
             % ('release' if GUIDE_RELEASE else 'debug'),
             '#', '# The original game (from baserom.toml). The card tables that moved, their lengths and the card lists',
             '# follow this build; everything else is at its original address:',
             '#  NamedAnchors (Goto menu):']
    head += guide_lines(VANILLA_GUIDE, base_anchors)
    head.append('#  MatchedWords (constants in the code and data):')
    head += guide_lines(VANILLA_WORDS_GUIDE, base_words)
    head.append('#  Lists (the names of table rows and enum values):')
    head += guide_lines(VANILLA_LISTS_GUIDE, base_lists)
    mod = (guide_lines(ADDED_GUIDE, sorted(added)), guide_lines(ADDED_WORDS_GUIDE, sorted(added_words)),
           guide_lines(ADDED_LISTS_GUIDE, sorted(added_lists)))
    if any(mod):
        head += ['#', "# Added by Yak's YGO Hack (not in baserom.toml); by symbol, so they follow every build:"]
        for title, lines_ in zip(('#  NamedAnchors (Goto menu):', '#  MatchedWords:', '#  Lists:'), mod):
            if lines_:
                head.append(title)
                head += lines_
    head.append('#')
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
