---
title: Hex Maniac Advance metadata for the demo (tools/hma_toml.py)
type: tool
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-09
---
# Hex Maniac Advance metadata for the demo

`tools/hma_toml.py` writes a companion `.toml` next to each **demo-profile** ROM the Makefile links
(`eds_demo.toml`, `eds_release_demo.toml`, or `eds_release_demo_extra.toml`; see below), from `baserom.toml`,
the [Hex Maniac Advance](https://github.com/haven1433/HexManiacAdvance) metadata of the original ROM (repo root).
Without `baserom.toml` it does nothing. Those TOMLs are git-ignored.

## How each entry moves
- **NamedAnchors.** The original address is placed in the source: a data label of `data/*.s` (`Name: @ 0x08xxxxxx`,
  its extent up to the next such label), or one of the card tables that moved to the end of the ROM
  (`config/assets.tsv` gives their original ranges; `MOVED_TABLES` names `gCardDescriptions`, `gCardStats`,
  `gCardIdToNumber`, `gCardPasswords`, and `cards/art` splits into `gCardArtGfx` / `gCardArtPalettes`). The new
  address is that symbol in `eds_demo.sym` plus the same offset.
  - Counts: `data.cards.numbers` (by card ID) gets CARD_ID_COUNT; `data.cards.internalnumbers` (by card number) the
    highest card number in use + 1 (from `newcards__number_to_id.bin`). Tables sized by a list (`]cardnumbers`)
    follow the list.
- **MatchedWords.** The original address is placed in a function (`config/functions.tsv`), found again in the
  `.sym`; the original instruction is read from `baserom.gba` (read only).
  - An unchanged function: the same instruction at the same offset.
  - A changed one: the same opcode bits and immediate (any register), the closest to the old offset.
  - The card count limits (`data.constants.cards.limit`, `.limit2`): the original forms (`movs #0xCD; lsls #2` =
    820, or a literal 821) can't hold the demo's values, so the script finds the literal holding CARD_ID_COUNT - 1 or
    CARD_ID_COUNT (Length 2), or else CARD_ID_COUNT_REAL - 1 / CARD_ID_COUNT_REAL (the code that stops before
    `make demo`'s blank slots, `CARD_ID_LAST_REAL`). Functions rewritten without the constant (Deck Edit's and Card Trading's list
    builders now walk `gCardIdsByName`) are left out and listed in a comment at the top of the file.
  - Constants in data (the pack rarity weights) move like anchors.
- **MatchedWords the mod adds** (appended after baserom.toml's; listed in `ADDED_WORDS_GUIDE` at the top of the TOML):
  - `data.constants.title.flamewave` (Length 1, `gModTitleFlameWave`): `1` runs the original flame wave on the title
    fade-in, `0` keeps the flames still.
  - `data.constants.title.flamepriority` (Length 1, `gModTitleFlamePriority`): BG1 layer priority for the flame sheet
    (`0`–`3`; default `1` in front of the coin and copyright).
  - `data.constants.preduel.qrsquare` (Length 1, `gModPreDuelQrSquare`): `1` draws the white square behind QR modules,
    `0` leaves the decorated box only (modules in white).
  - `data.constants.packs.format.usagewins` and `data.constants.formats.unlock.<format>` (documented below under pack
    unlock wins).
- **Added anchors** (`EXTRA_ANCHORS`, by symbol plus an offset, so they follow the `.sym` and the card count):
  - `data.cards.names` (``[name`asc`64]cardnumbers``), `data.cards.descriptions`
    (``[description`asc`480]cardnumbers``), and `data.cards.nameorder`
    (``[card:cardnumbers]!0000``): every card's name and effect text in fixed ASCII slots (`gCardNames`,
    `gCardDescriptions`), and the name-sorted ID list Deck Edit / the Trunk walk (`gCardIdsByName`; `make demo`'s blank
    slots are omitted until they have a name). At most **41** characters fit a Deck Edit name row.
  - `graphics.cards.icons.palettes` (``[`ucp4`]cardnumbers``) and `graphics.cards.icons.images`
    (``[`ucs4x2x2|graphics.cards.icons.palettes`]cardnumbers``): every card's own art icon, the art Deck Edit's
    ring shows. `gModFieldIconData`'s `DATA_DECK_EDIT` block (`tools/cardicons.py`, `src/mod_field_icons.c`), at
    after 0x7980 + 256 * CARD_ID_COUNT (rounded up to 4): every card ID's 16-colour palette (0x20 bytes, entry 0
    transparent), then every card ID's 16x16 4bpp image (2x2 row-major tiles, 0x80 bytes) at + CARD_ID_COUNT * 0x20.
    These are HMA-CE's inline image tables (`ArrayRun.TryParseInlineImageTable`, `InlineSpriteTableRun` /
    `InlinePaletteTableRun`): a row is one bare `` `ucs` `` / `` `ucp` `` format and nothing else, the same form
    as `graphics.cards.images` / `graphics.cards.palettes`; the palette hint names the palettes table and HMA uses
    its row of the same index. Checked by decoding Blue-Eyes, Dark Magician and Proxy Dragon from the demo ROM.
    - History (2026-11-06): the data used to be one 160-byte entry per card (palette, then tiles). In HMA, neither
      one table with named inline fields (the general table parser takes backtick formats only behind pointers)
      nor two tables over the same rows (an anchor clears every run it overlaps, `ClearFormat`) worked; a
      hand-made pair for Blue-Eyes did. So `tools/cardicons.py` now writes the two arrays and `ModDeckRingArt`
      reads them (`DATA_DECK_EDIT_IMAGES`); the data's size is the same.
  - `graphics.cards.fieldicons.artpalette` (`` `ucp4:0123` ``): the 64 BGR555 colours at `gModFieldIconData` +0
    (`DATA_PALETTE`, 128 bytes, one palette run — not ``[`ucp4:0123`]4``, which would be four table rows).
    `graphics.cards.fieldicons.miniatures`
    (``[`ucs8x2x2|graphics.cards.fieldicons.artpalette`]cardnumbers``): 256-byte GBA 8bpp 2×2 tiles per card;
    art indices **0–63** (HMA `SpriteTool.Render`), **0xFF** padding. In-game, `ModFieldIconMiniColor` adds
    `ART_PALETTE_ENTRY`.
- **The mod's tables** (`mod_anchors`): found by the ELF's symbol table (`build/eds.elf` `.symtab`, read by the
  script itself; the static symbols too, keyed by their object file, e.g. `mod_dp.o:sFormatCardCosts`), so the
  addresses follow every build:
  - Format packs (`src/mod_format_packs.c`, `build/include/format_pack_data.h`), indexed by the new list
    `formatpacks` (the 50 pack names): `data.packs.format.pools`
    (``[pool<[card:cardinternalnumbers weight:]/count> count: unused:]formatpacks``, the pool and weights the
    tables of PACKS.md show, and all a pull reads: a weight edited in HMA is that card's new pull weight; a pool
    can be made shorter or, with room or a repoint, longer through its `count`), `.names` (pointers to the
    `` `asc` `` strings
    `text.packs.format.pack<i>`), `.flags` (1 = usage pack), `.formats` (an index of the `formats` list),
    `.releasedates` (yyyymmdd), `.portraits` (the cover card).
  - Format pack covers (by the `.sym`): `graphics.packs.format.<pack>` (the pack's name in lower case, letters and
    digits; a repeated name gets its index) is pack i's shop cover in `gFormatPackCovers` (i × 0x1880; baked at
    link time by `tools/format_pack_covers.py` from `data.packs.format.portraits`, same crop as the old runtime
    blit): `` `ucs8x7x14|graphics.palettes.packs` ``, the vanilla cover tile format. Changing `.portraits` needs a
    rebuild to refresh the cover blob; you can repaint a cover in HMA without rebuilding. Palette
    `graphics.palettes.packs` / `gPackListPal` (0x0863CC7C, 256 colours) is **used** for format covers, the vanilla
    cover gfx still in the ROM, and the shop background — an edit there retints all of them.
  - Card prices (`src/mod_dp.c`, `build/include/card_prices.h`): `data.prices.cards`
    (``[costs<[card:cardinternalnumbers price:]/count> count: unused:]formats``), the DP of a password per format;
    the new list `formats` holds the 15 formats in banlist order.
  - Decks (`src/mod_decks.c`): for every `DeckList` built in, `data.decks.mod.<deck>.list`
    (``[cards<> count: unused:]1``) with `data.decks.mod.<deck>.main` (main + Fusion Deck;
    ``[card:cardinternalnumbers|data.cards.passwords]…`` so HMA's **Import .ydk** on a card resolves passwords into
    `.main` and updates `.list`; `.side` is fixed-size and only its slots are filled) and `.side`. Format opponents'
    decks, duelist fallbacks, and the Test Duel deck. `data.decks.recipes`
    (``[name<> list<> side<> sidecount::]45``) is the debug menu's deck recipes (`GetRecipePreset`; empty in the
    release), pointing at those decks and at their names `text.decks.recipes.recipe<i>` (`` `asc` ``); its last
    row is the terminator.
  - Pre-duel pictures (`data/mod_preduel_data.s`, by the `.sym`; layout in `tools/preduel_image.py`), for each
    `gModPreDuel<Format>`: `graphics.preduel.<format>.palette` (`` `ucp4:0123456789ABCD` ``, the 224 colours of BG
    entries 32-255), `data.preduel.<format>.qrsize`, `graphics.preduel.<format>.qr.palette` (`` `ucp4` ``) and
    `.qr.image` (`` `ucs4x4x4|...qr.palette` ``, the QR code, 1 = dark module), and `graphics.preduel.<format>.picture`
    (`` `ucs8x30x12|...palette` ``, the 240x96 picture). An edited picture keeps 224 colours; an edited QR code must
    stay at its module count (`qrsize`), drawn from the top left.
  - Added effect rows (`src/added_effects.c`, by the `.sym`): `data.cards.addedeffects` is `gAddedCardEffects`
    in baserom.toml's `data.cards.effects` format (``[card:cardinternalnumbers padding: routine1::|h ... routine5::|h]``,
    resolve, check, prepare, chainA, chainB; 24 bytes a row), one row per added card with an effect row plus the
    0xFFFF terminator (127 rows now). `FindCardEffect` (effect_hooks.c) searches it after `gCardEffects`, so a row's
    card number gives that card the five handlers.
  - Title screen (by the `.sym`): `graphics.title.text` / `.flame` / `.coin` / `.copyright`, each `.image` and
    `.palette` (`gModTitleImages`, `tools/title_images.py`; [[title-screen]]: text `ucs4x30x20`, 4bpp, BG palette 8;
    flame `ucs8x30x20` with a 128-colour palette, BG entries 0-127), `text.title.version` / `.release` /
    `.debug` (`` `asc`16 ``). One kept original anchor is rewritten: `graphics.titlescreen.background` uses
    `ANCHOR_FIXES` (plain `` `ucs4x4x4|graphics.palettes.titlescreen.background` `` instead of an image pack HMA
    could not parse).
  - Duel backgrounds (by the `.sym`): `data.duel.backgrounds` (``[background<>]fieldbackgrounds``) is
    `gFieldBackgroundImages` (original data at 0x08086550), the BG0 texture per field: the normal field, then the 14
    Field Spells in `GetFieldMagicIndex`'s order (new list `fieldbackgrounds`). Its 15 pointers lead to 9 image packs,
    each 16 colours and 16 tiles in row-major order (checked), anchored as `graphics.duel.backgrounds.<name>.image`
    (`` `ucs4x4x4|...palette` ``, 32 x 32) and `.palette`, named after the first field that uses it (Mountain shares
    Wasteland's, Gaia Power Forest's, Umiiruka Umi's, Luminous Spark Chorus of Sanctuary's, Mystic Plasma Zone
    Yami's); repointing an entry gives a field another texture. Listed with the original game. The mod's board
    (`gModDuelFieldImage`, `tools/duel_board.py`) is a pack with deduplicated tiles and a cell list, which HMA can't
    show as a picture: `graphics.duel.board.palette` (`` `ucp4` ``) and `.tiles` (`` `uct4x<n>|...palette` ``, 73
    tiles now) are anchored, the cell list is not. Checked by rendering five textures.
  - Pre-duel screens (by the `.sym`, by duelist ID, `]opponents`): `data.preduel.table` (``[picture<> qr<> scene<>]``:
    the picture data, or 0; the data whose QR code it shows, or 0 for none; the scene copy for an opponent with no
    picture) and `data.preduel.text` (``[line1`asc`48 ... line5`asc`48]``, up to 38 characters a line inside the box's
    border). The scene copies are `graphics.duelists.<duelist>.scene` (`` `ucs8x30x12|...scenepalette` ``) and
    `.scenepalette` (`` `ucp4:0123456789ABCDEF` ``), and the eye sprites `.eyes` (`` `ucs4x16x16|...eyespalette` ``,
    the OBJ sheet) with `.eyespalette` (`` `ucp4` ``, the OBJ palette bank the sheet uses, from the extracted
    `sprites.png`). Edits show on the pre-duel screen; the eyes keep the scene set's animation tracks.
    Source: `duelists/preduel_text.txt` ([[yugi-kaiba-opponent]]).
  - Opponent faces on the Records screen (by the `.sym`'s `gModRecordFaces`, `data/mod_record_faces.s`): for each
    opponent with a card art portrait (the 15 formats and Debug; the Test Duel when its portrait is a card),
    `graphics.opponents.<name>.face` (`` `ucs4x6x2|...facepalette` ``, 48 x 16) and `.facepalette` (`` `ucp4` ``,
    entries 1-15). The table is zeros in the source; `tools/record_faces.py` fills it in the linked ROM (the
    Makefile's ROM rule, after fixptrs and before the TOML) from each opponent's portrait card and that ROM's card
    art, with the sampling the game used to do at run time (art rows 28-51 at 2/3, the 15 most used colours).
    `ModRecordDrawPage` now copies the face (`DrawRecordFace`); a zero entry draws the duelist's original face. A new
    portrait card set in HMA does not redraw the face: edit it too, or rebuild. Checked by rendering Critter
    (Sangan), Android (Jinzo) and Reaper (Spirit Reaper) from the anchors.
  - Original duelist portraits: `graphics.duelists.<duelist>.portrait` (`` `ucs8x7x7|...portraitpalette` ``, 56 x 56)
    and `.portraitpalette` (`` `ucp4:0123456789ABCDEF` ``, the page's 256 colours). The originals are round portraits
    drawn into the 5 opponent-select page pictures (240 x 160 linear 8bpp, which HMA can't show), so
    `tools/duelist_portraits.py` cuts each (a 56 x 56 box around slot position + (31, 33)) from
    `assets/gfx/mode4_bitmaps` into `gModDuelistPortraits` (`data/mod_duelist_portraits.s`, 25 entries by original
    page * 5 + slot, 3648 bytes each), and the game draws every portrait it copies from there (`DrawSlotPortrait`;
    an opponent with portrait 0 now also draws its page's own portrait from the copy), so an HMA edit shows. Checked
    by rendering Yugi's and Solomon's from the anchors.
  - Original duelist faces (by the `.sym`'s `gRecordPortraitImages`, original data in bank B): for duelist IDs
    1-24, `graphics.duelists.<duelist>.face` (`` `ucs4x6x2|...palette` ``, 48 x 16) and `.palette` (`` `ucp4` ``).
    Each is a 4bpp image pack whose 12 tiles are stored in row-major order with no deduplication (checked for all 24),
    so the pack's palette (+8) and tiles (after the tile-count header) are a plain HMA palette and sprite; a pack with
    14 or 15 colours has its tile count inside the 16-colour palette's span (colours no pixel uses). The 25th entry
    (the "???" face) is deduplicated and left out. Listed with the original game in the header guide
    (`VANILLA_ADDED_PREFIXES`); mostly unused now (the Records screen draws card art), except Solomon's face.
    Checked by rendering Yugi, Joey, Kaiba and Grandpa from the anchors.
  - Extended card data (by the `.sym`, one byte per card ID, `]cardnumbers`): `data.cards.typeex` is `gCardTypeEx`
    as an HMA bit tuple, ``[typeex.|t|kind:cardkindsex|effect.|tuner.|race::racesex]`` (bits 0-1 the kind: none,
    Synchro, Xyz, Link, which the stats word stores as Fusion; bit 2 effect; bit 3 Tuner; bits 4-7 a race EDS lacks,
    0 = the stats word's), with the new lists `cardkindsex` and `racesex`; `data.cards.linkmarkers` is
    `gCardLinkMarkers`, one bit per Link Arrow (top-left, top, top-right, left, right, bottom-left, bottom,
    bottom-right). HMA's tuple widths: `.` 1 bit, `:` 2 bits, from bit 0 (as baserom.toml's `data.cards.stats`).
    Checked: Stardust Dragon Synchro, Junk Synchron Tuner, Utopia Xyz, Link Spider Link/Cyberse with the bottom
    arrow, Proxy Dragon left and right.
  - Opponents (`src/mod_opponents.c` `sOpponents`, by the ELF): `data.opponents`
    (``[kind.opponentkinds order. format.formats unused. name<> portrait:cardinternalnumbers padding:]opponents``,
    12 bytes a row, one per duelist ID 0-24): kind (hidden, format, deck), unlock order, format (banlist), the name
    (`text.opponents.<name>`, ASCII), and the portrait: the card whose art the opponent select
    (`ModOpponentSelectPortraits`) and the Records screen (`DrawRecordArtPortrait`) draw for that opponent; 0 keeps
    the original page's portrait, `0x8000 | page << 4 | slot` takes an original page's (Solomon: 0x8043). New lists:
    `opponents` (duelist IDs with this build's opponent, e.g. "tristan - Yugi-Kaiba"; baserom.toml's `duelists` is in
    another order) and `opponentkinds`. Checked: La Jinn for Yugi-Kaiba, Sangan for Critter, Fiber Jar for Fiber.
  - Pack unlock wins (`MatchedWords`; see also title and pre-duel constants above): `data.constants.packs.format.usagewins` (Length 1) is
    `gModUsagePackWins[0]` (src/mod_format_packs.c, 10 = `FORMAT_PACK_USAGE_WINS` of tools/format_packs.py): the wins
    against a format that open its usage pack. It is a byte of data so it exists in every build (`ModIsFormatPackUnlocked`
    is `return 1` in make / make demo, and an immediate in the release). `data.constants.formats.unlock.<format>`
    (Length 1, 14 of them) are `sWinsToUnlockNext` (src/mod_opponents.c, 20 each): the wins against the format before
    it that open that format, and with it its TCG packs; named from the table's comments. They only take effect in
    `make release`: make and make demo open every format and pack. Checked in the demo ROM: 10, and 20 each.
  - **Leaf names only (2026-11-06):** HMA's Goto menu shows an anchor `a.b` but not the anchors under it (`a.b.c`),
    so no added anchor name is also the start of another: the pictures are `.picture`, the QR sprites `.qr.image`,
    the DeckLists `.list`, the icon images `graphics.cards.icons.images`, and the added effect rows
    `data.cards.addedeffects` (baserom.toml's `data.cards.effects` is itself an anchor). Checked by listing every
    anchor name of eds_demo.toml that starts another: none.
  - **The header guide (2026-11-06):** the top of each TOML lists, in two sections, what baserom.toml brings (the
    original game: anchors, MatchedWords, lists, and what the script changes in them) and what the mod adds; each
    group with its entry count and whether the game uses it in that build (release or debug, from
    `build/include/mod_config.h`). From the code: unused are the original starter-deck pools (`BuildModStarterDeck`
    replaces all three), the title logo and its palette (`ModTitle_Load`), Yugi's deck (hidden duelist); mostly
    unused the original opponent decks (every format opponent has a mod deck; Solomon and Debug fall back to theirs
    without a picked recipe);     partly used the original boosters and rarity odds (not in the shop;
    `ModPackList_AddUnlockedPacks` lists only format packs; still the Campaign event rewards, `gMain.rewardPack`);
    `graphics.palettes.packs` and the vanilla `graphics.packs` cover blobs are partly used — the palette tints format
    covers and the shop BG, the vanilla tiles stay in the ROM but no table points at them in the mod shop)
    (correction, 2026-11-06: the original pack-unlock wins are unused: `unlocks.10winseach`, `.20wins` and
    `.10winstotal` are in `IsPackUnlocked`, read only by the original pack list `PackList_AddUnlockedPacks`, which
    nothing calls; `unlocks.2wins1`-`5` are in `IsCampaignLevel2Unlocked`, "more than 1 win against each of duelists
    1-5", which can never pass with Yugi, Tea, Joey and Bakura hidden (the Records pages no longer use it:
    `ModRecordHasNextPage`, [[main-menu-c]]). The "new packs" notice, `unlockNotices` bit 1
    in `Campaign_RecordDuelResult`, compares `ModCountUnlockedFormatPacks` before and after a duel, so it follows the
    mod's usagewins and formats.unlock.*);
    debug-only the recipes, the Test Duel deck, and the Tea / Joey / Bakura decks (hidden duelists, reachable through
    the recipes); release-only the mod's unlock wins. The texts are `VANILLA_GUIDE` / `ADDED_GUIDE` (and the WORDS and
    LISTS variants) in the script.
  - The ELF is `build/eds.elf`, which every build shares: the script uses it only when its global symbols sit at
    the `.sym`'s addresses (else, e.g. after a `make release-demo`, it reports that and leaves the ELF-only tables out;
    run inside `make demo` the two always match).
  - Lengths are fixed per build (the TOML is written again each `make demo`): a deck or pool made longer in HMA
    needs its count field changed too.
- **Which builds:** `make demo` writes `eds_demo.toml`, `make release-demo` `eds_release_demo.toml` and
  `make release-demo-extra` `eds_release_demo_extra.toml` (release rules with the blank card slots) (the
  Makefile's ROM rule, for the demo card list either way). The release-demo one is where the unlock MatchedWords
  take effect (make demo opens every format and pack); it has no blank card slots (so 1127 card IDs) and one deck fewer; the recipe table is still in the ROM
  (`data.decks.recipes`) but unused there (`GetRecipePreset` returns 0 in the release).
- **The original duelists in `cardinternalnumbers` (2026-11-07):** after the last card (index 2389, the alternate
  arts from 2000) and a gap, the 24 original duelists at `PORTRAIT_FROM_SLOT(page, slot)` = `0x8000 | page << 4 |
  slot` of their original opponent-select slot (`gOpponentSelectDuelists`): "Duelist - Yugi Muto" = 32768 ...
  "Duelist - Trusdale (Grandpa)" = 32835. These are the values `data.opponents`' portrait takes for an original
  portrait, so its card picker names them (Solomon's 0x8043 reads "Duelist - Trusdale (Grandpa)"). The gap is HMA's
  sparse list form (`set_list_items` writes `N = [ ... ]` blocks; the hash rotates over the gaps, as
  `StoredList.GenerateHash` does).
- **Lists.** `cardnumbers` (by card ID) is cut or grown to CARD_ID_COUNT and `cardinternalnumbers` (by card number)
  gets the added cards, both with their in-game names from `build/new_cards.tsv` (a blank slot: `Blank <id>`). The
  other lists are kept.
- **DefaultHash** is recomputed for every changed format and list with HMA's `StoredList.GenerateHash` (SHA-256 of
  each item, folded into a rotating seed 0xCAFEBABE; checked against three of `baserom.toml`'s hashes). HMA keeps an
  entry whose hash doesn't match its own reference as a user edit.

## Checked (2026-11-06; field miniatures format 2026-11-07)
- Five anchors moved (the card images, palettes, stats, ID-to-number table, passwords); every other anchor kept its
  address, and the data there is the same as the original's but for tables the mod changes (the effect table's
  function pointers, two starter-deck stats).
- Every kept MatchedWord reads the expected value in the demo ROM (the limits 1006 / 1007, the pack and Fusion
  constants unchanged); 7 card-limit sites were left out.
- HMA's metadata parser cuts every line at `#`, so the header comments are fine (no card name holds a `#`).
- With the blank slots (2026-11-06): 1436 card IDs, the limits found as 1435/1436 or 1006/1007; the same 7
  Deck Edit / Card Trading sites left out.
- The mod tables (2026-11-06): decoded from the demo ROM at the TOML's addresses: pool 0 (126 cards, 1/600,
  2/10000, ...), the pack names, the dates and portraits, the Yugi-Kaiba price
  table (75 cards, 0/223, 1/451, ...), Joey's DeckList pointing at its 41-card main deck, the 44 recipes (every
  DeckList and side they point to is anchored); no two anchors share an address.
- Not opened in HMA yet.
