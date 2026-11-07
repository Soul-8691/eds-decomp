---
title: Hex Maniac Advance metadata for the demo (tools/hma_toml.py)
type: tool
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-07
---
# Hex Maniac Advance metadata for the demo

`tools/hma_toml.py` writes `eds_demo.toml` next to `eds_demo.gba` on every `make demo` (not `make release-demo`),
from `baserom.toml`, the [Hex Maniac Advance](https://github.com/haven1433/HexManiacAdvance) metadata of the
original ROM (repo root). Without `baserom.toml` it does nothing. `eds_demo.toml` is git-ignored.

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
- **Added anchors** (`EXTRA_ANCHORS`, by symbol plus an offset, so they follow the `.sym` and the card count):
  - `graphics.cards.icons.palettes` (``[`ucp4`]cardnumbers``) and `graphics.cards.icons`
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
    tables of PACKS.md show), `data.packs.format.roulettes` (``[roulette<[card:cardinternalnumbers]8192>]...``: what
    a pull actually reads, 8192 card numbers per pack, built from the weights by `tools/format_packs.py`; editing a
    pool's weights in HMA does not change the roulette), `.names` (pointers to the `` `asc` `` strings
    `text.packs.format.pack<i>`), `.flags` (1 = usage pack), `.formats` (an index of the `formats` list),
    `.releasedates` (yyyymmdd), `.portraits` (the cover card).
  - Card prices (`src/mod_dp.c`, `build/include/card_prices.h`): `data.prices.cards`
    (``[costs<[card:cardinternalnumbers price:]/count> count: unused:]formats``), the DP of a password per format;
    the new list `formats` holds the 15 formats in banlist order.
  - Decks (`src/mod_decks.c`): for every `DeckList` built in, `data.decks.mod.<deck>` (``[cards<> count: unused:]1``)
    with its card array `data.decks.mod.<deck>.main` (main + Fusion Deck, card numbers) and `.side`; the format
    opponents' decks, the duelist decks and the Test Duel deck. `data.decks.recipes`
    (``[name<> list<> side<> sidecount::]45``) is the debug menu's deck recipes (`GetRecipePreset`; empty in the
    release), pointing at those decks and at their names `text.decks.recipes.recipe<i>` (`` `asc` ``); its last
    row is the terminator.
  - Lengths are fixed per build (the TOML is written again each `make demo`): a deck or pool made longer in HMA
    needs its count field changed too.
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
  2/10000, ...), the pack names, roulette 0 (0x4000 bytes per pack), the dates and portraits, the Yugi-Kaiba price
  table (75 cards, 0/223, 1/451, ...), Joey's DeckList pointing at its 41-card main deck, the 44 recipes (every
  DeckList and side they point to is anchored); no two anchors share an address.
- Not opened in HMA yet.
