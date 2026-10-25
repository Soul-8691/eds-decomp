---
title: Added cards (config/new_cards.txt)
type: game
status: draft
confidence: high
sources: [rom-analysis, web-card-references]
updated: 2026-10-24
---
# Added cards

The game is expanded with the cards listed in **`config/new_cards.txt`**, one English card name per line (100 on
2026-10-24: the first 100 of `missing.txt`, Goat-format cards that EDS lacks). The ROM no longer matches the
original, so `make compare` fails by design; `make` builds the expanded ROM.

## Changing the added cards
Edit `config/new_cards.txt` and run `make`. That is the whole workflow:
- Names must be spelled as on YGOPRODeck. An unknown name stops the build with suggestions ("did you mean: Pot of
  Greed; ...").
- A name longer than 41 characters needs a shorter in-game name after a `|`, e.g.
  `Black Luster Soldier - Envoy of the Beginning | Black Luster Soldier - Envoy of Beginning`. The reason:
  a Deck Edit list row draws its name into a 21-tile slot of 2 characters per tile
  (`DeckEdit_GetListRowTile`: `slot * 21 + 0x30C`, ring of 7). The longest original name is 41 characters
  ("Winged Dragon,Guardian of the Fortress #1", which Konami shortened the same way).
- Cards EDS cannot show stop the build: monster types or attributes it lacks, and Link/XYZ/Synchro/Pendulum cards.
- **IDs follow the list order** (821, 822, ...), so inserting or removing a line changes the IDs after it, and a
  save made with another list shows other cards. Append to keep existing saves meaningful.
- **Capacity:** `python3 tools/newcards.py capacity`; 615 cards with the current data. The limit is the free card
  numbers (below). The name bank would allow 821 cards and the 0x7FF ID mask 1227. RAM checks are compile-time
  asserts in `include/deck_edit.h`. The whole of `missing.txt` (291 cards, ROM 13.9 MB) was built as a test.

## How it works (`tools/newcards.py build`, run by `make`)
- **Data:** for each name not yet in `assets/new_cards/cards.json`, the build asks the YGOPRODeck API
  (https://ygoprodeck.com/api-guide/) and caches the type, race, attribute, level, ATK/DEF, text and passcode
  by name. The cropped image becomes `assets/new_cards/art/<passcode>.png`: centre-cropped to 9:10, scaled
  to 72×80, reduced to BGR555 precision and quantized to 64 colours ([[card-art]] format). Only new names need the
  network and Pillow, and an edited PNG is kept. `tools/newcards.py fetch [--force]` refreshes every card. Like all
  of `assets/`, none of it is committed.
- **Card numbers** come from the free numbers, in order. A free number has no original card, is not a
  `CARD_` constant of `cards.h` (the cards and the numbers the code tests), is not an effect-table key, and is not
  any integer in the extracted game tables (deck lists, packs, AI lists; excluding unrelated integers costs
  nothing). They are drawn from two bands:
  - 1–1209;
  - 1600–1899, above the effect keys (1211–1552) and every number the duel code tests (up to 1526).

  Card Trading hides numbers 1210–1899, so it now lists the added cards by ID instead
  (`DeckEdit_BuildCardLists`, `TradeCardSelect_Init`). The assignment is in `build/new_cards.tsv`.
- **Generated files** (rewritten only when they change):
  - `build/include/card_count.h` (`CARD_ID_COUNT_ADDED`, included by `include/global.h` when present, so tools
    that preprocess without it, such as `check.py`, see the original game);
  - `build/include/card_count.inc` (`CARD_ID_COUNT` for `data/rodata_08087FD0.s`);
  - the "Added cards" block at the end of enum CardNumber in `include/constants/cards.h`. `tools/gencards.py`
    keeps the block when it regenerates the header.
- **Mapping:** YGOPRODeck race maps to an EDS type (Dragon 1 … Reptile 20), and attribute LIGHT..WIND to 1..6. Monsters
  get their kind from the card type. Spell/Trap race maps to a subtype (Normal 0, Counter 1, Field 2, Equip 3,
  Continuous 4, Quick-Play 5, Ritual 6). Text loses line breaks and `●` bullets, and a text over 479 characters
  is cut at a word with "...".

> [!warning] Ring of Destruction and Shining Angel are already in EDS
> They are "Bell of Destruction" (ID 61) and "Shining Fairy" (ID 628), with the same passcodes (83555666,
> 95956346). `FindCardByPassword` returns the first match, so those passwords give the original cards. Other
> renamed EDS cards may also be in `missing.txt`.

## ROM layout
- `gCardNames` (`0x0822C720`) grows in place into the unused second name bank (`CARD_ID_COUNT` × 0x40; the
  assembler stops if it does not fit).
- `gCardNumberToId` stays at `0x08623DF4` (2048 entries, unchanged size). Its address-suffixed entry aliases keep working.
- The other per-card tables move to the end of the ROM, after `0x08800000`, each with `CARD_ID_COUNT` entries. Their old ranges become
  zero: `gCardDescriptions` 0x08800000, `gCardArtGfx`, `gCardArtPalettes`, `gCardStats`, `gCardIdToNumber` (+
  `gCardIdToNumber_821` padding and the out-of-range `gCardIdToNumber_1081`), `gCardPasswords` and `gCardSortKeys`
  ([[card-id-map]]). The final addresses are in `build/eds.map`.
- New table `gCardIdsByName`: every card ID in name order, then 0. Sort key as in [[card-name-table]]:
  lower case, hyphens dropped. The original IDs keep their relative order, and the Tickets, Gods and Token stay last.
- The data layout is hand-edited in `data/rodata_08087FD0.s` (a `tools/assets.py gen-data` run would undo it).
  `tools/newcards.py build` makes the CARD_ID_COUNT-entry tables from the original 821-entry ones plus `assets/new_cards/`.
- The ROM is 12.9 MB with 100 added cards, plus about 5 KB per added card.

### Keeping the rest of the ROM valid
The data tables hold absolute pointers: function pointers come from `config/functions.tsv`, and data pointers are
raw. Changing C code changes function sizes, so:
- `tools/mkld.py` pins `.rodata` at `0x08080A20` (an `ASSERT` stops the build if the code grows past it). Every data
  symbol except the moved card tables keeps its original address (verified against a build of the original commit).
- `tools/fixptrs.py` runs after `objcopy`. It finds every function pointer in the built table assets (a 4-aligned
  word equal to a `functions.tsv` address with the right Thumb bit) and writes the function's address from the ELF.
  There are 1163 slots, the same 1163 that a scan of the original ROM's table assets finds; outside the table assets, only 10 coincidental hits in
  sound and card-art data, which are left alone.
- Units that read card tables through integer addresses (`((const u32 *)0x08621DE0)[id]`, 221 places) use
  `CARD_*_ADDR` from `include/global.h`, which name the moved symbols.

## Code and RAM changes
- `CARD_ID_COUNT` = `CARD_ID_COUNT_ORIGINAL` (821) + `CARD_ID_COUNT_ADDED` (`include/card_data.h`,
  `include/global.h`). Bounds that
  were `0x334` use it: `IsCardCollectionComplete`, `FindCardByPassword`, `DebugGetAllCards`. Simon's gift and the
  Card Detail debug browser already used `CARD_ID_COUNT`, and so does `GeneratePackCards`'s random packs.
- **Deck Edit lists moved to RAM `0x0202A000`** (`gDeckEditCardLists[2]`, `struct DeckEditListRow` of
  `CARD_ID_COUNT` + 80 + 15 IDs per row, at most 0x2200 bytes for both rows). The original rows (`gDeckEdit` +0x644, 821 trunk entries, 0x728 per row) could not grow
  without moving every later `gDeckEdit` field into `gCardTrading` (`0x0201F780`). The old block is now
  `gDeckEdit.unusedLists`. `0x0202A000` is after `gSceneWork`'s largest view (0x8070 bytes from `0x02020310`), and
  no code or data refers to `0x02028000`–`0x02030000` ([[ram-map]]). The save needs no change:
  `gSaveData.trunk` already has 0x800 entries ([[save-game]]).
- The list builders walk `gCardIdsByName` instead of IDs 1..820, so the added cards appear in alphabetical
  order in the Trunk/Deck/Side lists, Card Trading, the Prohibition picker and the side-deck swap:
  `DeckEdit_BuildCardLists`, `DeckEdit_Init`, `ProhibitCardSelect_Init`, `TradeCardSelect_Init`,
  `SideDeckSwap_Init`. The 'Name' sort keeps this order.
- The 'All' filter's held-back non-monsters moved from `gDeckEdit.sortScratch` (310 slots, enough for only 84
  added Spells/Traps) to `gDeckEditHeldBack` (`0x0202C200`, one slot per card ID).

## Duels
The added cards have no rows in `gCardEffects`, and `FindCardEffect` returns -1 for them. Monsters therefore play as
vanilla monsters. Spells and traps cannot be activated (`CanActivateEffect` needs a row); they can still be set.
Every `FindCardEffect` caller handles -1 except the two proxy-resolve paths, which only run for cards that already
have an effect.

## Verification (2026-10-24)
- The 821 original entries of every moved table are byte-identical to the original ROM. Number↔ID round-trips for all
  920 IDs. `gCardIdsByName` is a permutation of 1..920 that keeps the original order. Stats, passwords and text of
  sampled cards match YGOPRODeck.
- Data symbols: all at their original addresses except the moved card tables. Outside the card block, 1122 data
  words differ from the original ROM: 1118 relocated function pointers and 4 in AgbSram's `.rodata`.
- Compiler warnings: the same 16 distinct warnings as the original build.
- **Not yet run in an emulator.** `tools/emu.py` needs mGBA ([[emulator]]); it was not available natively.
- `tools/check.py`/`check_all.py` and `make compare` compare against the original ROM and report the changed units.

Related: [[cards]], [[card-table]], [[card-id-map]], [[deck-edit-cards-c]], [[ram-map]].
