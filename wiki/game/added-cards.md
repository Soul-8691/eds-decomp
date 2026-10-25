---
title: Added cards (100 cards beyond the original 820)
type: game
status: draft
confidence: high
sources: [rom-analysis, web-card-references]
updated: 2026-10-24
---
# Added cards

On 2026-10-24 the game was expanded with 100 cards: the first 100 entries of `missing.txt` (Goat-format cards that EDS
lacks). The ROM no longer matches the original, so `make compare` fails by design; `make` builds the expanded ROM.
Static checks are below; the expanded ROM has not been run in an emulator yet.

## Card identities
- `config/new_cards.tsv` (committed) lists each added card: **card ID** 821–920 (in `missing.txt` order), **card
  number**, the 8-digit **passcode**, the name, and an optional shorter in-game name.
- Card numbers 846–938 and 813–819. They are free numbers below 1210, so Card Trading lists them (it hides
  1210–1899 and 1920–1999, see [[deck-edit-cards-c]]). None of them is an effect-table key, a deck or pack entry, or a number the code tests
  (checked against `card_effect_handlers.json`, every table asset and the C sources).
- Constants: `include/constants/cards.h` has `CARD_<NAME> = <number>` for each, marked "added card".
- Stats, text, passcodes and art come from the YGOPRODeck API (https://ygoprodeck.com/api-guide/):
  `tools/newcards.py fetch` (run by `make setup`, or by the first build that needs it) writes
  `assets/new_cards/cards.json` and `assets/new_cards/art/NNN.png`. Art is the cropped card image, centre-cropped to 9:10,
  scaled to 72×80, reduced to BGR555 precision and quantized to 64 colours ([[card-art]] format). Like all of
  `assets/`, none of it is committed.
- Mapping: YGOPRODeck race to EDS type (Dragon 1 … Reptile 20), attribute LIGHT..WIND to 1..6. Every added monster
  is an effect monster (kind 1), and Spell/Trap race to subtype (Normal 0, Counter 1, Field 2, Equip 3, Continuous
  4, Quick-Play 5). Text loses line breaks and `●` bullets. All descriptions fit the 0x1E0-byte record; the longest is 394 characters.
- **Names are at most 41 characters.** A Deck Edit list row draws its name into a 21-tile slot of 2 characters
  per tile (`DeckEdit_GetListRowTile`: `slot * 21 + 0x30C`, ring of 7). The longest original name is 41
  characters ("Winged Dragon,Guardian of the Fortress #1", shortened by Konami the same way). "Black Luster Soldier -
  Envoy of the Beginning" (45) is shown as "Black Luster Soldier - Envoy of Beginning".

> [!warning] Two added cards duplicate original cards
> EDS already has **Ring of Destruction** as "Bell of Destruction" (ID 61) and **Shining Angel** as "Shining Fairy"
> (ID 628), with the same passcodes (83555666, 95956346). The added copies are IDs 821 and 890. `FindCardByPassword`
> returns the first match, so those passwords still give the original cards.

## ROM layout
- `gCardNames` (`0x0822C720`) grows in place into the unused second name bank (921 × 0x40).
- `gCardNumberToId` stays at `0x08623DF4` (2048 entries, unchanged size). Its address-suffixed entry aliases keep working.
- The other per-card tables move to the end of the ROM, after `0x08800000`, each with 921 entries. Their old ranges become
  zero: `gCardDescriptions` 0x08800000, `gCardArtGfx`, `gCardArtPalettes`, `gCardStats`, `gCardIdToNumber` (+
  `gCardIdToNumber_821` padding and the out-of-range `gCardIdToNumber_1081`), `gCardPasswords` and `gCardSortKeys`
  ([[card-id-map]]). The final addresses are in `build/eds.map`.
- New table `gCardIdsByName`: every card ID in name order, then 0. Sort key as in [[card-name-table]]:
  lower case, hyphens dropped. The original IDs keep their relative order, and the Tickets, Gods and Token stay last.
- The data layout is hand-edited in `data/rodata_08087FD0.s` (a `tools/assets.py gen-data` run would undo it).
  `tools/newcards.py build` makes the 921-entry tables from the original 821-entry ones plus `assets/new_cards/`.
- The ROM is 12.9 MB.

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
- `CARD_ID_COUNT` is 921 (`CARD_ID_LAST` 920, `CARD_ID_COUNT_ORIGINAL` 821) in `include/card_data.h`. Bounds that
  were `0x334` use it: `IsCardCollectionComplete`, `FindCardByPassword`, `DebugGetAllCards`. Simon's gift and the
  Card Detail debug browser already used `CARD_ID_COUNT`, and so does `GeneratePackCards`'s random packs.
- **Deck Edit lists moved to RAM `0x0202A000`** (`gDeckEditCardLists[2]`, `struct DeckEditListRow` of
  921 + 80 + 15 IDs = 0x7F0 bytes per row). The original rows (`gDeckEdit` +0x644, 821 trunk entries, 0x728 per row) could not grow
  without moving every later `gDeckEdit` field into `gCardTrading` (`0x0201F780`). The old block is now
  `gDeckEdit.unusedLists`. `0x0202A000` is after `gSceneWork`'s largest view (0x8070 bytes from `0x02020310`), and
  no code or data refers to `0x02028000`–`0x02030000` ([[ram-map]]). The save needs no change:
  `gSaveData.trunk` already has 0x800 entries ([[save-game]]).
- The list builders walk `gCardIdsByName` instead of IDs 1..820, so the added cards appear in alphabetical
  order in the Trunk/Deck/Side lists, Card Trading, the Prohibition picker and the side-deck swap:
  `DeckEdit_BuildCardLists`, `DeckEdit_Init`, `ProhibitCardSelect_Init`, `TradeCardSelect_Init`,
  `SideDeckSwap_Init`. The 'Name' sort keeps this order.
- The 'All' filter's held-back non-monsters (`gDeckEdit.sortScratch`, 310 slots) still fit: 226 original
  non-monsters plus 47 added.

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
