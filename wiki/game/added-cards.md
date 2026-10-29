---
title: Added cards (config/new_cards.txt)
type: game
status: draft
confidence: high
sources: [rom-analysis, web-card-references]
updated: 2026-10-28
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

## Duels and card effects
An added card without an effect plays as a vanilla monster, or as a Spell/Trap that can be set but not activated:
`FindCardEffect` finds no row and returns -1, which every caller handles except the two proxy-resolve paths, and
those run only for cards that have an effect.

**Effects of added cards** live in `src/added_effects.c`:
- **Table.** `gAddedCardEffects` holds rows like `gCardEffects` (`struct CardEffect`, keyed by `CARD_<NAME>`,
  ending with number 0xFFFF). `FindCardEffect` searches it after the original 426 rows and returns
  `CARD_EFFECT_ROWS_ORIGINAL` + its index. Every row read goes through `CARD_EFFECT_ROW(row)` /
  `GetCardEffectRow` (`include/effect.h`): duel_main, duel_response, duel_setup, duel_turn_end,
  effect_activation, effect_hooks, effect_resolve5 and effect_resolve9. The rows point at their handlers by symbol,
  so `tools/fixptrs.py` is not involved.
- **Placement.** The unit is `@farcode` in `units.txt`: its code goes into the range the moved card tables left
  free (from `0x082461A0`, after its `.rodata`; within `bl` range of all game code). The rest of
  `data/rodata_08087FD0.s` is section `.rodata.pinned` at `0x08623DF4`. So added effects cost none of the game
  code's slack before the pinned data at `0x08080A20`. Each hook added inside the game's units still costs a
  few bytes there (0x558 left on 2026-10-25, after moving the rewritten saved-deck functions to `src/mod_save.c`).
- **Slack (2026-10-25).** The Tsukuyomi hooks pushed the code 0x80 bytes past `0x08080A20` ("the code has grown into
  the data"). The original unit `dice_scene` (0x145C bytes: the Dice, Destiny Board and Exodia scenes) is now
  `@farcode` too, which leaves 0x13DC bytes of slack (code end `0x0807F644`). Any unit can move this way; mkld.py
  takes `@farcode` for any unit. Its callers reach it through `bl` and symbols, and `tools/fixptrs.py` fixes its
  pointers in the data tables by name. Before moving one, check that no code refers to its functions by raw address
  (only the asm comments did here), and that it holds no ARM or IWRAM code. A short `b` across units would fail at
  link time. `check.py dice_scene` gives the same result with or without the move.
- **Guards.** Each card's code is under `#ifdef HAS_CARD_<NAME>`, which `tools/newcards.py` defines in
  `build/include/card_count.h` for every listed card. Taking a card off the list removes its effect instead of
  breaking the build.
- **Hooks keyed by card number.** Effects that the duel code triggers by number also need a `case` there, under the
  same guard:
  - "when Normal Summoned" triggers: `ExecuteSummonAction` and `ExecuteSummonActionAskPosition` (summon_checks.c);
  - Flip Summon triggers: `SummonStep_Flip` (summon_action.c);
  - the card menu's ACTIVATE for a face-up monster: `CanActivateMonsterEffect` (card_command_menu.c);
  - stat changes: card_stats.c.

### Breaker the Magical Warrior (implemented 2026-10-24)
"When this card is Normal Summoned: place 1 Spell Counter on it (max. 1). Gains 300 ATK for each Spell Counter on
it. You can remove 1 Spell Counter from this card; target 1 Spell/Trap on the field; destroy that target."
- **Spell Counter.** A `ZONE_LINK_ATK_300_PER_VALUE` link from Breaker's own card ID, count 1. The stat code already
  gives 300 ATK per count (card_stats.c). `DuelCmd_TurnStart` only strips such links from its own list of
  once-per-turn monsters, and `ClearZone` drops the link when the card leaves the field.
- **One effect row, two effects**, told apart by the chain link's `event`:
  - `RESPONSE_SUMMONED` with `loc0` = its own zone: the trigger that the two Normal Summon paths queue (not for a
    Set, a Flip Summon or a Special Summon). Resolve queues `QueueAddZoneLink` if Breaker is still face up there
    and has no counter.
  - `RESPONSE_NONE`: the activated effect from the Main Phase card menu (`CanActivateMonsterEffect` → prepare:
    face up with a counter; check: `EffectAnySpellTrapCheck`, so a Spell/Trap must be on the field). ChainA (the
    cost) removes the counter (`QueueRemoveZoneLink`). ChainB picks the target (`EffectSpellTrapTargetChainB`) and
    resolve destroys it (`EffectDestroyTargetResolve`). These are Mystical Space Typhoon's handlers.
- **Face down (2026-10-25).** A monster turned face down loses its counters. `DuelCmd_FlipCard` (duel_cmd_field.c)
  calls `OnMonsterTurnedFaceDown` (src/added_effects.c) when it leaves a monster face down. That function removes
  the zone's `ZONE_LINK_ATK_300_PER_VALUE` link at once (`RemoveZoneLink`). It covers Book of Moon and any later
  face-down effect. Magical Hats does not use the flip command: it parks the attacked monster's links and
  moves them back to its hat zone (`DUEL_CMD_MOVE_ZONE_LINKS`). In `EffectMagicalHatsResolve` (effect_resolve3.c)
  that move is now followed by `QueueRemoveZoneLink(..., ZONE_LINK_ATK_300_PER_VALUE)` on the hat zone, unless
  Light of Intervention makes the hats face up.
- The CPU never has Breaker in its decks.
- **Decks of this build** (`src/mod_decks.c`; keep to the Forbidden/Limited list):
  - **Red starter deck (2026-10-25):** the main deck of `LR-Chaos Turbo.ydk` (repository root): BLS - Envoy of the
    Beginning, Chaos Sorcerer, flip monsters, Thunder Dragon, the discard/draw Spells (`BuildModStarterDeck`, called
    by `BuildStarterDeck`). It replaces the earlier 8-BLS deck.
  - **Yugi Muto:** a competitive 40-card deck (`GetOpponentDeckOverride`): Jinzo, Summoned Skull, Vorse Raider,
    floaters and searchers, the limited staples, Mystical Space Typhoon, Trap Hole.
  - **Tea (`DUELIST_TEA`):** the Red starter deck's LR-Chaos Turbo list (`sRedDeckList`).
  - **Tristan (`DUELIST_TRISTAN`), weighted random per duel:** `TristanDeck` rolls `Random() % 10` when
    `LoadOpponentDeck` (its only caller, once per duel) asks. Soul Control 5 in 10 (`El Duelista DEX-Soul
    Control.ydk`), Vanilla Beat 3 in 10 (`ellietincan-Vanilla Beat.ydk`), Stall Mill 2 in 10 (`DK-ZERO-Stall
    Mill.ydk`, 43 cards). Main decks only. All counts are within `gCardCopyLimits`; verified from
    `build/src/mod_decks.o` against the three main decks.
  - **Green and Black starter decks (2026-10-25):** Green is Joey's Warrior list, Black is Bakura's Goat Control list
    (the user asked for "the other 2 ydks"; the colours were matched by Claude). `BuildModStarterDeck` now picks the
    list by `choice`. It sends a Fusion Monster to the saved Fusion Deck (`AddCardToSavedFusionDeck`), so Black also
    gets Bakura's Fusion Deck, and everything else to the saved Deck.
  - **Joey Wheeler (`DUELIST_JOEY`):** the main deck of `sdlkilla-Warrior.ydk`.
  - **Bakura (`DUELIST_BAKURA`):** the main and extra deck of `geistD-Goat Control.ydk` (40 + 15). A `struct
    DeckList` has one card list; `LoadOpponentDeck` adds each card with `AddCardNumberToDeckTop` (ai_summon.c), which
    puts a Fusion Monster into the Fusion Deck. So the 15 extra-deck cards are appended to `sBakuraDeck`, and its
    guard includes the added Fusion monsters. Verified from `build/src/mod_decks.o`: the 40 main cards and the 15
    Fusion monsters match the .ydk's `#main` and `#extra`.
  - **How the .ydk decks were made.** The .ydk numbers are passcodes. Each was mapped through the built
    `gCardPasswords` (original and added cards) to a card ID, then to its card number and `CARD_*` constant; all 120
    resolved. Only the `#main` sections were used (40 each), and all their counts are within `gCardCopyLimits`.
    Each deck is guarded on the `HAS_CARD_*` of its added cards (`MOD_RED_DECK`, `MOD_JOEY_DECK`,
    `MOD_BAKURA_DECK`), so taking one of them off the list drops that deck instead of breaking the build. Verified:
    the arrays decoded from `build/src/mod_decks.o` match the three main decks card for card.
  - **Limits.** The Red starter deck and Joey use their main decks only. Bakura's Fusion Deck gives Metamorphosis its
    targets (Thousand-Eyes Restrict at Level 1, Dark Balter, and others). The CPU plays added cards only through the generic logic.
- **Other rules.** Decks of fewer than 40 cards may duel (`DECK_MIN_CARDS` 1), and the human player cannot deck
  out outside Link Battles (`DuelCmd_DrawCards`).

### No 3-copy limit (2026-10-25)
`GetCardCopyLimit` returns `CARD_COPY_LIMIT_NONE` (99) for a card that is not on the Forbidden/Limited list; that
list's limits (0/1/2) still apply. In the original game the per-deck copy counters of `struct TrunkEntry` are 2
bits. They keep those bits as their low bits and have high bits in the bytes the original never used: deck +4 bits
(up to 63), side +2 (up to 15), fusion +3 (up to 31). **Every reader and writer goes through
`Get/SetTrunk{Deck,Side,Fusion}Copies`** (`src/mod_save.c`): collection.c (whose add/remove/trim functions were
rewritten), Deck Edit (cards, list, prohibit, stats, widgets, filter steps), booster code, the debug give-all. Saves
from the original game load unchanged; the format is a superset.

### Banishing and "sent to the Graveyard" triggers
A monster that the added cards banish does not run "sent to the Graveyard" effects such as Sangan's or Witch of the
Black Forest's. Those triggers are queued only by `SendFieldCardToGrave`, `SendBattleDestroyedCardToGraveyard`,
`TributeMonster` and `SendFusionMaterialToGrave`. `BanishFieldCard` (Chaos Sorcerer, BLS, D.D. Warrior Lady) and
`BanishBattleDestroyedCard` (D.D. Warrior Lady) queue none. Checked in the source on 2026-10-25.

**Revived monsters (fixed 2026-10-25).** A monster revived by Call of the Haunted or Premature Burial was an
exception. The banish calls `DestroyLinkedCards`, which destroys the reviving card while the monster still holds
its link to it (`ZONE_LINK_CONTINUOUS` or `ZONE_LINK_EQUIP`). That card's `SendFieldCardToGrave` case then finds the
monster, which is still in its zone because the banish is only queued, and calls `SendFieldCardToGrave(monster, 1, 1)`.
That queues the Graveyard trigger. Now `BanishFieldCard` and `BanishBattleDestroyedCard` first call
`DetachRevivingCards` (src/added_effects.c). It removes the link (`RemoveZoneLinkAt`, which takes effect at once)
and then destroys the Call of the Haunted (unless disabled) or Premature Burial itself, so nothing finds the monster.
The rest goes through `DestroyLinkedCards` as before. Checked in the source and the build; not tested in an emulator.

### Chaos Sorcerer (2026-10-25)
- **Summon.** The same as Black Luster Soldier - Envoy of the Beginning's: Special Summon only, banishing 1 LIGHT and
  1 DARK monster from the Graveyard. The hooks are shared under `HAS_LIGHT_DARK_BANISH_SUMMON` (global.h; defined when
  either card is listed), with `CanPayLightDarkBanishCost` and `LightDarkBanishAttribute`.
- **Effect.** Main Phase ACTIVATE, once per turn: it targets a face-up monster (`EffectTargetableFaceUpMonsterCheck` /
  `EffectTargetableFaceUpMonsterChainB`) and banishes it; Chaos Sorcerer cannot attack that turn
  (`EffectBanishTargetNoAttackResolve`, shared with BLS's banish effect).

### Tsukuyomi (2026-10-25)
- **Cannot be Special Summoned** (`IsCannotSpecialSummon` blocks `QueueSpecialSummon*` in summon_builders.c).
- **Field effect** when Normal Summoned face-up, Flip Summoned, or flipped face-up in battle: same as Book of
  Moon (`EffectBookOfMoonResolve`, face-up monster target handlers). `EffectTsukuyomiPrepare` gates events;
  face-up Normal Summon queues `RESPONSE_SUMMONED` from `TsukuyomiAfterNormalSummon` (summon_action.c).
- **End Phase:** if it was Normal Summoned or flipped face-up this turn, `EndPhase_ReturnTsukuyomi` returns it
  to the hand (set-only Normal Summon still returns, without firing the field effect).
- `HasFlipEffect` (card_detail.c).
- Checked in the source; not tested in an emulator.

### Zombyra the Dark (2026-10-25)
- "This card cannot attack directly. Each time this card destroys a monster by battle, it loses 200 ATK."
- **No effect row.**
  - **Direct attacks.** A direct attack is what happens when the opponent has no monster (`CanAttackDirectly` only
    lists the monsters that may attack directly past the opponent's monsters). So `CanMonsterAttack`
    (card_menu_input.c) refuses Zombyra while the opponent has no monster. The CPU's attack pick (ai_picks.c)
    lists its attackers with the same test.
  - **ATK loss.** `BattleStage_DestroyMonsters` now calls one `AddedCardsAfterBattle` for the added cards'
    battle-kill effects (Mystic Swordsman LV2 and Zombyra). A Zombyra that survives a battle which destroyed the
    other monster queues a self link `ZONE_LINK_KIND(ZONE_LINK_ATK_DOWN_200, 1)` from its own card ID. `AddZoneLink`
    raises the count of an existing link with the same target, and the stat code takes 200 per count. The link goes
    with the card when it leaves the field.
- **Not modelled:** turned face down, it keeps the lost ATK; on the real card its effects would reset.
- Checked in the source and the build; not tested in an emulator.

### Smashing Ground (2026-10-25)
- Normal Spell: "Destroy the 1 face-up monster your opponent controls that has the highest DEF (your choice, if
  tied)."
- **Prepare:** the opponent has a face-up monster.
- **Resolve** (`EffectSmashingGroundResolve`, like Widespread Ruin's highest-ATK search): no target.
  - It takes the highest current DEF (`GetZoneCardDef`) among the opponent's face-up monsters.
  - A single one is destroyed at once. On a tie the human picks among them with the cursor (another pick plays the
    error sound), and the CPU takes the first.
  - The destruction is `DestroyFieldCard` + `OnCardDestroyedByEffect`. Face-down monsters are ignored.
- Checked in the source and the build; not tested in an emulator.

### Magical Merchant (2026-10-25)
- "FLIP: Excavate cards from the top of your Deck until you excavate a Spell/Trap. Add that card to your hand, also
  send the remaining cards to the GY."
- **Flip:** a `HasFlipEffect` case. `EffectMagicalMerchantResolve` walks the Deck from the top (`deck[0]`) and
  sends each monster before the first Spell/Trap to the Graveyard (`DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD`).
- **The Spell/Trap:** shown (`ShowPickedCard`) and added with `AddDeckCardToHand`, Sangan's add. That takes the
  first card of its number, which is this one, since everything above it is a monster.
- **No Spell/Trap:** the whole Deck goes to the Graveyard. A player whose Deck is then empty decks out at the next
  draw (player 0 cannot). The Deck is not shuffled.
- Checked in the source and the build; not tested in an emulator.

### Magic Cylinder (2026-10-25)
- Normal Trap: "When an opponent's monster declares an attack: Target the attacking monster; negate the attack, and
  if you do, inflict damage to your opponent equal to its ATK."
- **The engine already has it** as effect key 1214. Its row is `EffectAttackResponsePrepare` (an opponent's attack
  declaration, from the field) and `EffectAttackResponseResolve`: `DUEL_CMD_NEGATE_ATTACK` (the battle goes to
  `BATTLE_STAGE_END_ATTACK` and the attacker is marked), then the attacker's controller loses LP equal to its ATK.
  The CPU's Set-Trap logic has a case for it (ai_summon.c: when the attacker's ATK is high enough).
- `tools/newcards.py` `ENGINE_NUMBERS` gives Magic Cylinder number 1214; no code change. Verified in the built
  tables: ID 866 = number 1214, Normal Trap.
- **Side effect.** The added cards after it in the list moved down one free number, as for Thousand-Eyes Restrict.
  This is harmless: the `CARD_*` constants are regenerated, and the saved trunk is indexed by card ID.
- Checked in the source and the build; not tested in an emulator.

### D.D. Assailant (2026-10-25)
- "After damage calculation, when this card is destroyed by battle with an opponent's monster: Banish that monster,
  also banish this card."
- **No effect row.** `BattleStage_DestroyMonsters` (battle_phase2.c), right after D.D. Warrior Lady's question: when
  a D.D. Assailant is on either side and the battle destroyed it, it shows the card and sets `gBattle.ddWarriorLady =
  DD_WARRIOR_LADY_YES`. The battle then goes on as after a Yes to D.D. Warrior Lady: every destroyed monster is
  banished instead of sent to the Graveyard (`BanishBattleDestroyedCard`, Dimensional Warrior's path), and a survivor
  is banished at the end (`BanishFieldCard`). That last block and the reset of the state are now guarded on either
  card being listed. The effect is mandatory, so nobody is asked.
- The banished monsters do not run Graveyard effects ([[added-cards#Banishing and "sent to the Graveyard" triggers]]).
- Checked in the source and the build; not tested in an emulator.

### Zaborg the Thunder Monarch (2026-10-25)
- "When this card is Tribute Summoned: Target 1 monster on the field; destroy that target."
- **Trigger.** It joins Breaker's case in both Normal Summon paths (`ExecuteSummonAction` /
  `ExecuteSummonActionAskPosition`, summon_checks.c), which queue a `RESPONSE_SUMMONED` trigger. It is Level 5, so
  that summon always has a Tribute. A Set, a Flip Summon and a Special Summon do not trigger it.
- **Row:** Man-Eater Bug's handlers (`EffectTargetableMonsterCheck`, `EffectTargetableMonsterChainB`,
  `EffectDestroyTargetResolve`). The effect is mandatory, as on the card, so Zaborg targets itself when it is the
  only monster.
- **Edge (as Man-Eater Bug):** the target step re-checks `CanActivateEffect` against the previous chain link. If
  another trigger starts the same chain first, the spell-speed rule refuses the monster and Zaborg picks no target.
- Checked in the source and the build; not tested in an emulator.

### Spirit Reaper (2026-10-25)
- "Cannot be destroyed by battle. After this card is targeted by an effect, destroy this card. When this card
  inflicts battle damage to your opponent: Discard 1 random card from their hand."
- **No effect row.** Three hooks:
  - **Battle.** `CalcBattle` (campaign.c), at the end with the other card rules, clears its `destroyed` flag as
    attacker or target. A face-down Reaper attacked is flipped first (`BattleStage_RevealDefender`), so it survives
    too. Battle damage is still calculated.
  - **Targeted.** `TryAddEffectTarget` (effect_targets1.c, the routine that records a picked field target and points
    at it) calls `OnFieldCardTargeted`. A face-up Spirit Reaper there is destroyed at once (`DestroyFieldCard`), so
    the targeting effect finds it gone when it resolves. Not for picks that Tribute rather than target (Catapult
    Turtle, Cannon Soldier, The Little Swordsman of Aile, Metamorphosis).
  - **Battle damage.** White Magical Hat's two cases in `BattleStage_InflictDamage`, as attacker and as target:
    `DuelPrompt_PostRandomDiscard` for the damaged player.
- **Not covered:** a handler that adds a field target with `AddEffectTarget` directly instead of
  `TryAddEffectTarget` does not destroy it.
- Checked in the source and the build; not tested in an emulator.

### Soul Exchange (2026-10-25)
- Normal Spell: "Target 1 monster your opponent controls; this turn, if you Tribute a monster, you must Tribute that
  monster, as if you controlled it. You cannot conduct your Battle Phase the turn you activate this card."
- **The engine already has it** as effect key 1220, and `ENGINE_NUMBERS` gives the card that number (ID 913 = 1220).
  The row:
  - prepare `EffectCanTributeOpponentMonsterPrepare`: Main Phase 1, and a level 5+ monster in hand (with a free
    zone, or for level 7+ another Tribute) or a face-up Catapult Turtle / The Little Swordsman of Aile / Cannon
    Soldier;
  - chainA `EffectSkipBattlePhaseChainA`;
  - chainB `EffectTributeOpponentMonsterChainB`;
  - resolve `EffectTributeOpponentMonsterResolve`: a Tribute Summon from the hand (it uses up the Normal Summon), or
    the target as the Tribute of one of those effects.
- **Engine bug fixed** (effect_resolve8.c). Both summon paths called `TributeMonster(link->targets[0], 0)`, passing
  the packed DUEL_LOC as the player and zone 0, so the opponent's zone 0 was Tributed whatever the target. They now
  pass `(u8)targets[0], targets[0] >> 8`.
- **CPU path.** Both steps were cursor UI, and Tristan's decks run Soul Exchange.
  - `EffectTributeOpponentMonsterChainB` (effect_targets3.c) calls `SoulExchangeCpuChainB` for the CPU: the human's
    strongest monster.
  - The resolve calls `SoulExchangeCpuResolve`: it Tribute Summons the CPU's strongest level 5+ hand monster (level 5-6
    with a free zone; level 7+ also Tributes its own weakest monster) and uses up its Normal Summon.
  - `AiPlayAddedCards` plays the card when its prepare passes.
- Number side effect as for Magic Cylinder: the added cards after it in the list moved down one free number.
- **Prepare fixed** (effect_prepare2.c, under `HAS_CARD_SOUL_EXCHANGE`). The engine's key 1220 required a monster of
  the *player's* (`CountMonsters(card->player)`), so the card couldn't be used on an empty field, its usual play. It
  never checked the opponent's side either. It now requires an opponent's monster (`CountMonsters(1 - player)`).
  The other conditions are unchanged: Main Phase 1, Normal Summon unused, a level 5-6 with a free zone or a level 7+
  with one own Tribute (or a face-up Catapult Turtle / Little Swordsman of Aile / Cannon Soldier).
- Checked in the source and the build; not tested in an emulator.

### Dimension Fusion (2026-10-26)
- Normal Spell: "Pay 2000 Life Points. Both players Special Summon as many of their removed from play monsters as
  possible."
- Shares Return from the Different Dimension's helpers (`DimensionReturnOne`, `StrongestDimensionReturn`), without
  the End Phase mark.
- **Prepare.** At least 2000 LP, plus a free zone and an eligible banished monster for either player. **Cost:**
  ChainA pushes `DUEL_CMD_LOSE_LP` 2000.
- **Resolve.** One monster per step, strongest ATK first: the activating player's until none can come, then the
  opponent's. Each player picks their own monsters' positions.
- **CPU.** Activates it when at least 2 of its monsters would come back (capped by its free zones), more than the
  human gets, and it has more than 3000 LP.
- Checked in the source and the build; not tested in an emulator.

### Yata-Garasu (2026-10-26)
- Spirit monster: "This card cannot be Special Summoned. This card returns to its owner's hand during the End Phase of
  the turn it is Normal Summoned or flipped face-up. When this card inflicts Battle Damage to your opponent, they skip
  their next Draw Phase."
- **No Special Summon.** `IsCannotSpecialSummon` lists it.
- **End Phase.** `EndPhase_ReturnYataGarasu` (DuelPhase_End, next to the Dark Magician of Chaos / Return from the
  Different Dimension hooks) returns every face-up Yata-Garasu to its owner's hand (`ReturnFieldCardToHand`). No
  turn record is kept: a face-up one at an End Phase was Normal Summoned or flipped that turn, since it always
  leaves at the End Phase and can't be Special Summoned. A Set one stays.
- **Battle damage.** `BattleStage_InflictDamage` steps 0 and 2 (battle_phase2.c), as Masked Sorcerer.
  `YataGarasuDealtDamage` shows the card and pushes `DUEL_CMD_SKIP_NEXT_DRAW_PHASE` (Time Seal's command) for the
  damaged player; `DuelPhase_Draw` consumes the flag.
- Checked in the source and the build; not tested in an emulator.

### Fiber Jar (2026-10-26)
- FLIP: "Each player shuffles all cards from their hand, field, and Graveyard into the Deck, then draws 5 cards."
  `EffectFiberJarResolve` (a resolve-only row; card_detail.c `HasFlipEffect` lists it).
- **For each player:**
  - **Field.** `ReturnFieldCardToDeck` on every zone, Spell/Trap and Field zones first. Tokens are destroyed;
    `DuelCmd_ReturnToDeck` skips a zone emptied meanwhile and sends Fusion Monsters to the Fusion Deck.
  - **Hand.** `DUEL_CMD_RETURN_HAND_CARD_TO_DECK`, last card first.
  - **Graveyard.** Its Fusion Monsters move to the Fusion Deck at once (`TakeGraveyardCardAt` +
    `AddCardToFusionDeck`), because `DUEL_CMD_RETURN_GRAVEYARD_TO_DECK` (Penguin Knight's) uses
    `AddCardToDeckTop` for every card; then that command takes the rest.
- **Then** both Decks are shuffled (`DUEL_CMD_SHUFFLE_DECK`) and both players draw 5 (`DrawCards`). Banished cards
  stay banished.
- Checked in the source and the build; not tested in an emulator.

### Dark Ruler Ha Des (fixed 2026-10-26)
- Effect Monster: "Cannot be Special Summoned from the GY. Negate the effects of monsters destroyed by battle with
  Fiend monsters you control."
- **Before.** The earlier code gave it a different effect: Flip effects locked for the turn after its Tribute
  Summon (summon_checks.c, `LockFlipEffectsForTurn`). That is removed; the lock's machinery is still there, unused.
- **Not from the Graveyard.** `CanReviveGraveyardCard` (duel_ritual.c) refuses it, so Monster Reborn, Premature
  Burial, Call of the Haunted and the others cannot pick it.
- **Negating battle-destroyed monsters.** `HaDesNegatesBattleDestroyed(player)`: the monster of `player` was
  destroyed in battle by a Fiend (`gBattle.side[other].cardId`) whose controller has a face-up Ha Des.
  - `SendBattleDestroyedCardToGraveyard` (duel_field_moves.c) then skips its "destroyed in battle" switch: no
    Sangan, Witch, Mystic Tomato, Giant Rat, Pyramid Turtle, Spear Cretin... The card still goes to the Graveyard.
  - `HaDesNegatesBattleFlipEffect` stops the Flip effect of a face-down target that the attacking Fiend destroys,
    next to Blade Knight's checks (battle_phase1.c / battle_phase2.c).
- **Caveat.** In a mutual destruction where Ha Des itself is the attacker, it may leave the field before the target
  is checked.
- Checked in the source and the build; not tested in an emulator.

### Skull Lair (2026-10-26)
- Continuous Trap: "Remove from play any number of Monster Cards in your Graveyard to destroy 1 face-up monster on the
  field whose Level is equal to the number of the cards you removed from play."
- **Reuse.** Like Ultimate Offering it can be activated again while face up. `CARD_SKULL_LAIR` joins
  Ultimate Offering's face-up case in `CardMenu_GetSpellTrapCommands`, the chain-response check
  (effect_activation.c) and `CanActivateFieldCard` (effect_targets4.c).
- **Check** `EffectSkullLairCheck`. A face-up monster the card may target (`EffectTargetableFaceUpMonsterCheck`) of
  Level 1 to the monsters in the player's Graveyard; it is the target filter of the pick too. **Prepare:** such a
  monster exists on either side.
- **ChainB** `EffectSkullLairChainB`.
  - Human: `EffectTargetableFaceUpMonsterChainB` with Bell of Destruction's "designate one monster to destroy"
    prompt (effect_targets2.c). An unpayable Level is refused with `SE_ERROR`.
  - CPU: the human's strongest legal monster (`SkullLairBestTarget`).
- **Resolve** `EffectSkullLairResolve`. If the target is still legal, Level-many Graveyard monsters are banished
  (`DUEL_CMD_BANISH_GRAVEYARD_CARD`), then it is destroyed (`DestroyFieldCardByEffect`, `OnCardDestroyedByEffect`).
  - Human: picks each banished monster in the Graveyard list, as Bazoo does (`CardListView_Open` with
    `CARD_SKULL_LAIR`; `CollectEffectTargets` shares Bazoo's key-1513 case, the Graveyard monsters).
  - CPU: banishes its weakest.
- **CPU policy.** Activates it on a human monster of 1000+ ATK.
- Checked in the source and the build; not tested in an emulator.

### Apprentice Magician (2026-10-28)
- Effect Monster (Level 2, DARK Spellcaster, 400/800). The fetched text: "If this card is Summoned: Target 1 face-up
  card on the field that you can place a Spell Counter on; place 1 Spell Counter on that target. If this card is
  destroyed by battle: You can Special Summon 1 Level 2 or lower Spellcaster monster from your Deck in face-down Defense
  Position."
- **Counter.** `AddedCardsSummoned` (summon_action.c, every summon kind) calls `ApprenticeMagicianPlaceCounter` when it
  is summoned face up. Of the added cards Breaker the Magical Warrior (max. 1: a `ZONE_LINK_ATK_300_PER_VALUE` link
  from Breaker's own card ID, so Breaker's cost removes it as its own) and Skilled Dark Magician (max. 3, see below)
  hold Spell Counters, so the counter goes on the first face-up one with room: the summoner's first, else the
  opponent's (mandatory); with none, nothing. Applied at once, not as a chain link. Summons that bypass the summon action (from the Deck, tokens) do not
  trigger it.
- **Battle.** A floater like Pyramid Turtle: the `SendBattleDestroyedCardToGraveyard` trigger list, a
  `CollectEffectTargets` case (Deck Spellcasters of Level 2 or lower) and `EffectSpecialSummonFromDeckResolve`, which
  for it queues `QueueSpecialSummon(..., faceUp 0, isDefense 1)`: face-down Defense Position. The human is asked
  first; the CPU takes `AiPickCardListEntry`'s pick.
- Checked in the source and the build; not tested in an emulator.

### Skilled Dark Magician (2026-10-28)
- Effect Monster (Level 4, DARK Spellcaster, 1900/1700). The fetched text: "Each time a Spell Card is activated, place
  1 Spell Counter on this card when that Spell resolves (max. 3). You can Tribute this card with 3 Spell Counters on
  it; Special Summon 1 "Dark Magician" from your hand, Deck, or GY."
- **Counters.** A new link kind, `ZONE_LINK_SPELL_COUNTER` (20, include/constants/duel.h): a link from the card's own
  ID whose high byte is the count; `card_stats.c` ignores it, and `AddZoneLink` raises the count of an existing link
  with the same target. `CountSpellCounters` counts it as well as Breaker's kind; `OnMonsterTurnedFaceDown` removes
  it. `Chain_Resolve` (duel_main.c) calls `ModChainLinkResolved` on entering `RESOLVE_STAGE_NEXT_LINK`, when the
  link is done and its commands have run (`DuelMainStep` drains the command queue before `Chain_Update`): a Magic
  card's link that was not negated gives each face-up Skilled Dark Magician on either side with fewer than 3 a
  counter (with `ShowCardEffect`). Apprentice Magician can place one too.
- **Counter badge.** `ModDrawCounterBadges` (src/mod_field_icons.c, called by `DrawFieldOverlay` each frame) draws an
  8x8 sprite with the count (purple, white digit, black border) at the top-right corner of each face-up Breaker or
  Skilled Dark Magician with counters (`ModZoneSpellCounters`, added_effects.c; other cards' kind-11 links are not
  counters). Its tiles are OBJ tiles 0x3F1-0x3F9 (above the duel banners' 0x364-0x3C3) and its colours OBJ palette
  12; both are written while drawing, because the chain list and battle screens reuse OBJ VRAM.
- **Effect.** The card menu's ACTIVATE in the Main Phase (`CanActivateMonsterEffect` in card_command_menu.c): the
  prepare needs it face up with 3 counters and a Dark Magician (either art) in the hand, Deck or Graveyard (a
  `CollectEffectTargets` case). The cost (chainA) Tributes it. The resolve, in Elegant Egotist's style: the human
  picks in the card list; the CPU takes the Graveyard's first, else the Deck's, else the hand's. The card is
  removed from its pile (compacting the hand), Special Summoned with `QueueSpecialSummonChoosePosition`, and the
  Deck is shuffled if it came from there. The CPU uses it through `AiPlayAddedCards` whenever it can.
- Checked in the source and the build; not tested in an emulator.

### Golem Sentry (2026-10-28)
- Effect Monster (Level 4, EARTH Rock, 800/1800). The fetched text: "Once per turn: You can change this card to
  face-down Defense Position. When this card is Flip Summoned: Target 1 monster your opponent controls; return that
  target to the hand."
- One row, told apart by the link's event:
  - **Flip Summon** (`RESPONSE_FLIP_SUMMONED`): `AddedCardsSummoned` queues the trigger for `SUMMON_ACTION_FLIP`, as
    for Sacred Crane. It isn't added to the vanilla Flip Summon list in summon_action.c (Sonic Bird's), because that
    list gates on `CanActivateEffectOfCard`, which leaves the entry's zone and event unset. The target is an
    opponent's monster (Crass Clown's `EffectOpponentMonsterCheck` / `EffectOpponentMonsterChainB`; the CPU takes
    the human's strongest), returned to its owner's hand by `EffectReturnTargetToHandResolve`.
  - **Face down** (`RESPONSE_NONE`): the card menu's ACTIVATE in the Main Phase (`CanActivateMonsterEffect`), once
    per turn through `effectUnused`, which chainB clears and `DuelCmd_TurnStart` sets again. The target is the card
    itself; Book of Moon's `EffectBookOfMoonResolve` turns it to face-down Defense Position (its Equip Cards are
    destroyed, its counters removed). The CPU uses it while the human controls a monster (`AiWantsMonsterEffect`),
    so its next Flip Summon has a target.
- Checked in the source and the build; not tested in an emulator.

### The CPU chains summon traps to summon triggers (2026-10-28)
- **Problem.** A monster's own summon trigger (Mobius, Zaborg, Breaker, Sonic Bird, ...) goes on the chain as soon as
  the summon finishes; the summon's own response window (`EventResponse_Request`, where the CPU uses Trap Hole via
  `AiShouldActivateSetCard`) only opens after that chain resolved. The human could already chain Trap Hole to the
  trigger (a response keeps the link's event and locations, `Chain_AskResponse`), but the CPU's chain answers
  (`AiTryChainResponse`, ai_summon.c) are a list by card number that never offered a summon trap, so Mobius could
  destroy the CPU's Set Trap Hole before it was ever asked.
- **Fix.** `AiChainSummonTriggerResponse` (added_effects.c), asked first by `AiTryChainResponse`: for a link of the
  human's with a summon event, the first Set card of the CPU that the summon window would use (spell speed 2 or
  more, `CanActivateFieldCard` against a copy of the link, `AiShouldActivateSetCard`) is chained (`AiChainSetCard`).
  Covers Trap Hole and the other summon traps (Bottomless Trap Hole, Torrential Tribute by its own rule, ...). As in
  the TCG, the trigger still resolves after the trap destroyed the monster.
- Checked in the source and the build; not tested in an emulator.

### Mobius the Frost Monarch (2026-10-28)
- Effect Monster (Level 6, WATER Aqua, 2400/1000): "If this card is Tribute Summoned: You can target up to 2
  Spells/Traps on the field; destroy them."
- **Trigger.** Zaborg's case in both Normal Summon paths (summon_checks.c) queues its `RESPONSE_SUMMONED` trigger; Level
  6, so that summon always has a Tribute. A Set, a Flip Summon or a Special Summon does not trigger it.
- **Row.** Check `EffectAnySpellTrapCheck` (a Spell/Trap is on the field, as Mystical Space Typhoon); targets
  `EffectMobiusChainB`; resolve `EffectMobiusResolve` (each target still there: the destroyed-by-opponent flag for an
  opponent's card, `DestroyFieldCardByEffect`, `OnCardDestroyedByEffect`).
  - Human: the first card (B: no target, the effect is not used), then a second one (B: only the first; skipped when
    no other Spell/Trap is on the field), with Greenkappa's "1st/2nd card to destroy" prompts.
  - CPU: up to 2 of the human's Spells/Traps, face-up ones first, then face-down; never its own.
- Checked in the source and the build; not tested in an emulator.

### King Tiger Wanghu (2026-10-28)
- Effect Monster (Level 4, EARTH Beast, 1700/1000): "When a monster with 1400 or less ATK is Normal or Special
  Summoned: Destroy it."
- **Rule.** `KingTigerWanghuOnSummon(player, zone)` (added_effects.c): the summoned monster, face-up with 1400 or less ATK
  (`GetZoneCardAtk`), is destroyed (`ShowCardEffect` of the Wanghu, `DestroyFieldCard`, `OnCardDestroyedByEffect`) while
  a face-up, non-negated (`isDisabled`) King Tiger Wanghu is on either side. Face-down summons (Sets) show no ATK and
  stay; Flip Summons do not count.
- **Hooks.**
  - `ExecuteSummonAction` (summon_action.c): Normal Summons and the summon-action Special Summons (from the hand, and
    the effects that use `QueueSpecialSummon*`: Monster Reborn and the like), before the opponent's response window,
    which is not opened for a monster it destroys.
  - `DuelCmd_SummonFromDeck` (duel_cmd_deck.c): Special Summons from the Deck (Mystic Tomato, Giant Rat, ...).
  - `DuelCmd_SummonToken` (duel_cmd_status.c): tokens (Scapegoat's Sheep Tokens and the others).
- **Not modelled.** It is applied at once rather than as a chain link; the CPU does not avoid summoning into it.
- Checked in the source and the build; not tested in an emulator.

### Berserk Gorilla (2026-10-28)
- Effect Monster (Level 4, EARTH Beast, 2000/1000): "If this card is in Defense Position: Destroy it. This card must
  attack, if able."
- **Defense Position.** `ModDuelStateCheck` (added_effects.c), new hook in `DuelMainStep` (duel_main.c) right after
  `UpdateSpellTrapNegation` while nothing else is busy: a face-up Berserk Gorilla in Defense Position is destroyed
  (`ShowCardEffect`, `DestroyFieldCard`). A face-down one is left alone (its effect does not apply). Not during an
  attack (Battle Phase stages `BATTLE_STAGE_SELECT_TARGET`-`BATTLE_STAGE_END_ATTACK`): a Set one flipped by an attack
  goes once that attack is over.
- **Must attack.** In the attacker selection (card_menu_input.c, battle step 1):
  - human: B (the End Battle Phase / end turn menu) is refused with "Berserk Gorilla must attack." while a face-up
    Attack Position one has its `attackableMask` bit and not its `attackedMask` bit
    (`BerserkGorillaBlocksBattleEnd`). "Complete turn" in the Main Phase menu (shown when the Battle Phase can be
    entered) is refused the same way (`BerserkGorillaBlocksTurnEnd`, duel_phases.c `DuelPhase_Main`; it builds the
    attackable mask first, as the Battle Phase does): the player has to enter the Battle Phase. The plain "End your
    turn?" (no Battle Phase possible, e.g. after it) still ends the turn.
  - CPU: when `AiPlanAttack` finds no attack, `AiPlanForcedAttack` plans the Gorilla's: a winning one
    (`AiFindAttackTarget`), else the human's weakest monster (`AiFindWeakestMonster`), else direct. The plan view
    (`gAiWork` +0xC bits 4-6) is the one the battle step reads.
  - `AiShouldRetreatToDefense` never picks it.
- Checked in the source and the build; not tested in an emulator.

### United We Stand (2026-10-29)
- Equip Spell: "The equipped monster gains 800 ATK/DEF for each face-up monster you control."
- **The engine already has it** as effect key 1449 (no EDS card): `ENGINE_NUMBERS` in tools/newcards.py gives the card
  that number (ID 875 = 1449), as for Goblin Attack Force. What key 1449 does, all in matched code:
  - **Activation:** `EffectEquipResolve` and the equip target rules. Key 1449 is in the equip lists of
    effect_prepare1.c and card_list_viewer.c.
  - **Stats:** `CalcZoneCardStats` (card_stats.c) gives +800 ATK and DEF per face-up monster of the equip card's
    controller (`CountMonstersFiltered(sourcePlayer, 1, 0)`), counting the equipped monster too.
- The other added cards keep their IDs; their card numbers after it move down one.
- Checked in the source and the build; not tested in an emulator.

### Goblin Attack Force (2026-10-28)
- Effect Monster (Level 4, EARTH Warrior, 2300/0): "If this card attacks, it is changed to Defense Position at the end
  of the Battle Phase, and its battle position cannot be changed until the end of your next turn."
- **The engine already has it** as effect key 1344 (no EDS card); `ENGINE_NUMBERS` gives the card that number (ID 892
  = 1344), as for Fairy Box. What key 1344 does, all in matched code:
  - **Attack:** `BattleStage_DamageCalc` (battle_phase1.c) clears its `effectUnused` when it attacks.
  - **End of the Battle Phase:** `BattleStage_EndBattlePhase` (battle_phase3.c) turns it to Defense Position
    (`ChangeBattlePosition`) when it is face-up in Attack Position with `effectUnused` clear.
  - **Next turn:** `DuelCmd_TurnStart` (duel_cmd_turn.c) gives it `positionLocked` for its controller's next turn and
    sets `effectUnused` again.
  - `PlaceMonsterCard` sets `effectUnused` on every summon, so it only turns after attacking.
- The other added cards keep their IDs; their card numbers after it move down one (numbers are regenerated, saves keep
  IDs). Giant Orc has the same text but needs its own number, so it is not covered.
- Checked in the source and the build; not tested in an emulator.

### Gigantes (2026-10-28)
- Effect Monster (Level 4, EARTH Rock, 1900/1300): "Cannot be Normal Summoned/Set. Must be Special Summoned (from your
  hand) by banishing 1 EARTH monster from your GY. If this card is destroyed by battle: Destroy all Spells and Traps on
  the field."
- **Summon.** The Black Luster Soldier / Chaos Sorcerer banish-summon machinery (`HAS_LIGHT_DARK_BANISH_SUMMON` now
  also covers it): `IsSpecialSummonOnly` lists it; `CanSummonFromHand` needs `CanPayEarthBanishCost` (a free zone,
  an EARTH monster in the Graveyard); `SUMMON_STEP_BLS_*` (effect_hooks.c) banish one monster, whose attribute
  `LightDarkBanishAttribute` gives as EARTH for it (by `gDuel.cardMenuCard`), and `BanishSummonCount` (1) ends the
  steps after it. The CPU summons it too (`AiLightDarkBanishSummon`: banishes its weakest EARTH monster).
- **Effect.** `SendBattleDestroyedCardToGraveyard` (duel_field_moves.c) queues its trigger with Sangan's; the row's
  resolve is Heavy Storm's (`EffectHeavyStormResolve`): every Spell/Trap and Field zone card of both sides, the
  opponent's first. Banisher of the Light (banished, not destroyed) and Dark Ruler Ha Des stop it, as for Sangan.
- Checked in the source and the build; not tested in an emulator.

### Airknight Parshath (2026-10-28)
- Effect Monster (Level 5, LIGHT Fairy, 1900/1400): "If this card attacks a Defense Position monster, inflict piercing
  battle damage. If this card inflicts battle damage to your opponent: Draw 1 card."
- **Piercing.** `CalcBattle` (campaign.c) already had a piercing switch (Attack Position, the key-1313 and key-1540
  links, card 1341); Parshath as the attacker joins it, so the defender's player takes ATK minus DEF.
- **Draw.** `BattleStage_InflictDamage` (battle_phase2.c) gives it Masked Sorcerer's case in both steps: when its damage
  is dealt (attacking, or defending in Attack Position), its controller draws 1. No damage dealt (Kuriboh,
  battle protection, card 1423), no draw.
- Checked in the source and the build; not tested in an emulator.

### CPU flip targets in the Battle Phase (2026-10-28)
- **Rule.** The CPU's flip monsters that remove or stop one of the human's monsters (Man-Eater Bug, Hane-Hane, Penguin
  Soldier, Dark-Eyes Illusionist, Night Assailant, Tsukuyomi) normally pick the human's strongest monster. In the
  human's Battle Phase (`gDuel.phase == PHASE_BATTLE`, `gDuel.turnPlayer == 0`) they now prefer the strongest (ATK) of
  the human's Attack Position monsters whose bit in `gDuelPlayers[0].attackedMask` is clear: those can still attack.
  With none, the usual pick.
- **Code.** `AiFlipEffectPreferredTarget` / `AiFlipEffectTarget` (added_effects.c), called by the CPU branches of
  `EffectTargetableMonsterChainB` (Man-Eater Bug, Hane-Hane), `EffectOpponentMonsterChainB` (Dark-Eyes Illusionist,
  Night Assailant), `EffectPenguinSoldierChainB` (both picks; the second skips the first's zone) and
  `EffectTargetableFaceUpMonsterChainB` (Tsukuyomi). The other flips that hit a monster have no choice (Cyber Jar,
  4-Starred Ladybug of Doom) or hit Spells/Traps.
- Checked in the source and the build; not tested in an emulator.

### 4-Starred Ladybug of Doom (2026-10-28)
- Flip Effect Monster (Level 3, 800/1200): "FLIP: Destroy all Level 4 monsters your opponent controls."
- Added as ID 1136 (the last line of config/new_cards.txt) for paoloNS-Vanilla Beat's Side Deck
  (`sAndroidVanillaBeatSide`).
- **Effect.** `EffectLadybugOfDoomResolve`, a resolve-only row as Fiber Jar's; card_detail.c `HasFlipEffect` lists it,
  so it resolves when flipped and the CPU treats it as a flip monster (ai_picks.c / ai_steps.c ask `HasFlipEffect`).
  Every face-up monster of the opponent whose Level (`CARD_STATS_LEVEL`) is 4 is destroyed (`DestroyFieldCard`,
  `OnCardDestroyedByEffect`, as Tribe-Infecting Virus). A face-down monster has no Level, so it is not destroyed (the
  TCG ruling).
- Gradius' Option (GBO-Recruiter Control's Side Deck) was added at the same time and removed again: it needs
  Gradius, so that side deck stays at 14.
- Checked in the source and the build; not tested in an emulator.

### Fairy Box (2026-10-26)
- Continuous Trap: "When your opponent's monster declares an attack: Toss a coin and call it. If you call it right,
  the attacking monster's ATK becomes 0 until the end of the Battle Phase. During each of your Standby Phases, pay
  500 Life Points or destroy this card."
- **The engine already has it** as effect key 1424 (the source's "OCG Fairy Box" hypothesis is confirmed by its
  behaviour). `ENGINE_NUMBERS` gives the card that number (ID 1135 = 1424).
  - **Activation:** prepare `EffectAttackResponsePrepare`.
  - **Each attack:** face up on the defender's side, it is chained at once when the opponent declares an attack
    (`BattleStage_RespondToAttack`, `RESPOND_STEP_CHAIN_AUTO`).
  - **Resolve:** `EffectCoinTossZeroAttackerResolve`. The human calls heads/tails, the CPU at random;
    `DUEL_CMD_TOSS_COIN` shows the toss, and a right call pushes `DUEL_CMD_ZERO_ATTACKER_ATK`. The engine zeroes
    the ATK for that battle rather than to the end of the Battle Phase: the same in practice, since a monster
    attacks once.
  - **Upkeep:** 500 LP in its controller's Standby Phase (`GetMaintenanceLpCost`, `STANDBY_STEP_ASK_PAY_LP`); it
    is destroyed when unpaid or unaffordable.
  - **CPU:** its trap logic knows the key (ai_summon.c).
- Added to `config/new_cards.txt` for the Android opponent's Tomato Control deck.
- Checked in the source and the build; not tested in an emulator.

### Return from the Different Dimension (2026-10-26)
- Normal Trap: "Pay half your Life Points; Special Summon as many of your banished monsters as possible. During the
  End Phase, banish all monsters that were Special Summoned by this effect."
- **Prepare.** A free monster zone and an eligible monster in the player's banished pile: banished normally
  (`BANISH_NORMAL`, not face down or until the End Phase) and not a token.
- **Cost.** ChainA pushes `DUEL_CMD_LOSE_LP` for half the LP.
- **Resolve.** One monster per step while zones and monsters remain, the strongest ATK first. Each one leaves the
  pile (`RemoveBanishedCardAt`) and is Special Summoned face up in the position the player picks
  (`QueueSpecialSummonChoosePosition`). Its zone and card ID go to `gModDuel.dimensionReturn` /
  `dimensionReturnIds` (cleared by `ModDuelInit`).
- **End Phase.** `EndPhase_ReturnFromTheDifferentDimension`, called next to the Dark Magician of Chaos hook in
  `DuelPhase_End` (battle_phase3.c), banishes each marked monster still in its zone (same card ID), then clears the
  marks.
- **CPU.** Activates it with two or more eligible monsters, two free zones and more than 2000 LP.
- A monster that cannot be Special Summoned (`IsCannotSpecialSummon`) stays banished:
  `QueueSpecialSummonChoosePosition` would drop it after it left the pile.
- Fixed: the resolve returned `EFFECT_STEP_END` (a step number) when nothing was left, a soft-lock; it now returns
  `EFFECT_STEP_DONE`.
- Checked in the source and the build; not tested in an emulator.

### Reasoning (2026-10-26)
- Normal Spell: "Your opponent declares a monster Level from 1 to 12. Excavate cards from the top of your Deck until
  you excavate a monster that can be Normal Summoned/Set, then, if that monster is the same Level as the one declared
  by your opponent, send all excavated cards to the GY. If not, Special Summon the excavated monster, also send the
  remaining cards to the GY."
- **Excavation.** Monster Gate's (`IsGateMonster`), so the row is guarded on both cards being listed. Prepare: an
  eligible monster in the Deck.
- **Declaration.** The engine has no Level prompt.
  - When the CPU activates it, the human gets "Declare a monster Level." with a new Level menu: `LevelMenu_Draw` /
    `LevelMenu_HandleInput`, made like the Monster Type menu of `DuelPrompt_SelectType` (LEFT/RIGHT cycle 1-12,
    A confirms, starting at 4). It draws "Level N" into the Type menu's OBJ label tiles (0x364, palette 15).
  - When the human activates it, the CPU declares Level 4 (the most common Level; it cannot see the Deck) and a text
    box says so.
  - The Level is kept in `gChain.effectSubStep`.
- **Result.** The monster is shown. At the declared Level, it and the cards before it go to the Graveyard. Otherwise
  the cards before it do, and the monster leaves the Deck; the next step Special Summons it face up in the position
  the player picks.
- Checked in the source and the build; not tested in an emulator.

### Monster Gate (2026-10-26)
- Normal Spell: "Tribute 1 monster; excavate cards from the top of your Deck until you excavate a monster that can be
  Normal Summoned/Set. Special Summon it, also send the other excavated cards to the GY." No engine key does this.
- **Eligible monster** (`IsGateMonster`): not Fusion, Ritual, Special-Summon-only (`IsSpecialSummonOnly`) or a token.
- **Prepare:** a monster the player may Tribute (`CountTributableMonsters`), and such a monster in the Deck.
- **Cost (chainB).** Catapult Turtle's `EffectTributeTargetChainB` pick; a monster that cannot be Tributed
  (`IsTributableMonster`) is refused. It is Tributed at once, as for Metamorphosis, and not kept as a target. Spirit
  Reaper's `OnFieldCardTargeted` ignores Monster Gate's pick, since a Tribute is not a target. The CPU Tributes its
  weakest monster.
- **Resolve.** From the top of the Deck (`deck[0]`), the cards before the first eligible monster go to the Graveyard
  (`DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD`). The monster is shown and leaves the Deck. The next step, once those
  commands ran, Special Summons it face up in the position the player picks (`QueueSpecialSummonChoosePosition`;
  the text names none). With no eligible monster left, every card goes to the Graveyard.
- **CPU:** `AiWantsSpell` plays it when its weakest Tributable monster has less than 1500 ATK.
- Checked in the source and the build; not tested in an emulator.

### Sacred Crane (2026-10-26)
- "If this card is Special Summoned: Draw 1 card."
- **Hook.** `SummonAction_Update` (summon_action.c) now calls one `AddedCardsSummoned(player, zone, id, kind, faceUp)`
  for every summon once the monster is placed. It replaces the Dark Magician of Chaos call there and calls
  `DarkMagicianOfChaosSummoned` itself.
- **Trigger.** For a Sacred Crane with a Special Summon kind (`SUMMON_ACTION_SPECIAL*`, any position), it queues a
  monster trigger with `RESPONSE_SPECIAL_SUMMONED`. The resolve `EffectSacredCraneResolve` draws 1 unless negated.
  It is mandatory, as written.
- Checked in the source and the build; not tested in an emulator.

### Bazoo the Soul-Eater (2026-10-26)
- "Once per turn: You can banish up to 3 monsters from your Graveyard; this card gains 300 ATK for each, until the end
  of your opponent's turn."
- **The engine already has it** as effect key 1513 (key 1344, which the battle code also names, is a different,
  handler-less key). `ENGINE_NUMBERS` gives the card that number: ID 917 = 1513. The engine's version:
  - **Activation:** the card menu's ACTIVATE (`CanActivateMonsterEffect`: once per turn by `effectUnused`, and a
    monster in the player's Graveyard). Prepare `EffectGravediggerGhoulPrepare` refuses while the opponent has an
    active Kycoo.
  - **Resolve:** `EffectBanishGraveForAtkResolve` asks before each of up to 3 picks (`CollectEffectTargets` case 1513:
    the player's Graveyard monsters), then adds a `ZONE_LINK_ATK_300_PER_VALUE` link with the count and marks the
    effect used.
  - **Duration:** `DuelCmd_TurnStart` re-arms it and removes the link at its controller's next turn, so the ATK lasts
    through the opponent's turn.
- **CPU path.** That resolve is UI only. `effect_resolve11.c` now hands the CPU's activation to `BazooCpuResolve`,
  which banishes its 3 weakest Graveyard monsters (or fewer), then adds the same link and mark. The CPU activates it
  through `AiPlayAddedCards`.
- Checked in the source and the build; not tested in an emulator.

### Pyramid Turtle (2026-10-26)
- The user's text (2026-10-26): "When this card is sent to the Graveyard as a result of battle, you may select 1
  Zombie-Type monster with a DEF of 2000 or less from your Deck and Special Summon it to the field in face up Attack or
  Defense Position (no Tribute is required for monsters of Level 5 or more). The Deck is then shuffled." It is a
  floater like Mystic Tomato:
  - **Trigger:** a case in the "destroyed in battle" trigger list of `SendBattleDestroyedCardToGraveyard`
    (duel_field_moves.c). A Pyramid Turtle banished instead (D.D. Warrior Lady, Dark Magician of Chaos) does not
    reach that list, so it does not trigger.
  - **List:** a `CollectEffectTargets` case for Deck Zombie monsters with DEF 2000 or less.
  - **Resolve:** `EffectSpecialSummonFromDeckResolve`. The human is asked and picks in the card-list viewer; the CPU
    takes `AiPickCardListEntry`'s best ATK+DEF. The summon is face up in either position, through Gravekeeper's Spy's
    branch of the resolve (`QueueSpecialSummonChoosePosition`: the human's position menu, the CPU's
    `AiShouldSetMonster`). Then the Deck is shuffled. Any Level (a Special Summon needs no Tribute).
- Checked in the source and the build; not tested in an emulator.

### Bottomless Trap Hole (2026-10-26)
- Normal Trap: "When your opponent Summons a monster(s) with 1500 or more ATK: Destroy that monster(s) with 1500 or
  more ATK, and if you do, banish it."
- **Window.** It answers Torrential Tribute's summon window: `RESPONSE_SUMMONED`, `RESPONSE_FLIP_SUMMONED` or
  `RESPONSE_SPECIAL_SUMMONED`, with the summoned monster at `loc0`. Trap Hole's own handlers test their card numbers
  and only Normal / Flip Summons, so they are not reused.
- **Condition** (`BottomlessTarget`, at activation and again at resolution): the monster is the opponent's, face up,
  with 1500 or more ATK.
- **Resolve:** `BanishFieldCard`; the monster never reaches the Graveyard, so Graveyard effects (Sangan, Witch) do not
  run.
- **CPU:** a Set copy goes off whenever the human's summon qualifies (`AiShouldActivateAddedCard`'s default yes).
- Checked in the source and the build; not tested in an emulator.

### Dark Mimic LV1 (2026-10-26)
- The card data's text: "FLIP: Draw 1 card. Once per turn, during your Standby Phase: You can send this card to the GY;
  Special Summon 1 'Dark Mimic LV3' from your hand or Deck." (The "draw 2 if flipped by an attack" clause is LV3's.)
- **One row, told apart by the event.** A flip (`HasFlipEffect`) draws 1. `RESPONSE_OWN_STANDBY` is the level-up.
  `EffectDarkMimicLv1Prepare` passes a flip, and for the level-up needs the card face up in its zone and an LV3 in the
  Deck or hand.
- **Level-up**, as Jigen Bakudan's Standby effect. The human activates it from the card menu during the Standby
  Phase's `STANDBY_STEP_FIELD`; it joins the Standby cases of `CanActivateMonsterEffect`, `HasActivatableStandbyCard`
  (duel_phases.c) and the card menu's Activate in duel_cmd_queue.c, which queues the event `RESPONSE_OWN_STANDBY`.
  It is also in the Standby case of `CardMenu_GetMonsterCommands` (card_command_menu.c), the list that gives a
  monster Activate in the Standby Phase; it was missing at first, so the menu offered nothing (fixed 2026-10-26).
  The resolve is `LevelUpResolve` with no question.
- **Shared level-up.** `LevelUpResolve` is Mystic Swordsman LV2's former resolve, made general (`from`, `to`, an
  optional Yes/No). LV2 now calls it with its question.
- **CPU.** `STANDBY_STEP_FIELD` waited for the human's cursor even on the CPU's turn whenever the CPU had an
  activatable Standby card. On the CPU's turn it now calls `AiStandbyActivations` (the CPU's first face-up LV1 that
  may level up, once per turn by `gModDuel.aiStandbyTurn`) and then skips the step. This also ends the waits the
  original game had with the CPU's Jigen Bakudan, Blast Juggler, Patrol Robo or Curse of Fiend, which the CPU never
  activated anyway.
- Checked in the source and the build; not tested in an emulator.

### Xing Zhen Hu (2026-10-26)
- Continuous Trap, the card data's (older) text: "Activate by selecting 2 Set Spell or Trap Cards on the field. The
  selected Spell or Trap Cards cannot be activated." Engine key 1312 (`EffectLockMagicTrapResolve`) is a different
  card (Cold Wave), so this is new code.
- **Lock.** As Spellbinding Circle does for a monster, the resolve gives each target still Set a
  `ZONE_LINK_CONTINUOUS` link whose value is Xing Zhen Hu's DUEL_LOC. `IsZoneLockedByXingZhenHu(player, zone)`
  is true for a Set card whose zone holds such a link from a face-up, enabled Xing Zhen Hu. It is asked where the
  zone is known:
  - the card menu, `CardMenu_GetSpellTrapCommands` (card_command_menu.c), drops Activate;
  - the response windows, `CanActivateFieldCard` (effect_targets4.c, the human's and the CPU's), refuse the card;
  - `CanActivateEffect` (`IsLockedByXingZhenHu`) refuses it when the entry's zone holds that card (the CPU's
    `AiTryPlaySpellTrap`).
- **Fix (2026-10-26).** The first version only asked `CanActivateEffect`. But the menu tests a Set Spell with
  `CanActivateEffectOfCard`, whose entry leaves the zone unset (stack garbage), so locked Set Spells could still be
  activated.
- **Ending the lock.** The lock ends when Xing Zhen Hu leaves or is disabled, since the source check fails. A target
  that leaves takes the link with it (`ClearZone`).
- **Prepare:** two Set Spell/Trap cards besides itself (a Set Xing Zhen Hu is face-down too).
- **ChainB:** two different Set cards with the cursor (the error sound for a repeat; B starts again). The CPU takes
  the human's Set cards first.
- **CPU:** `AiShouldActivateAddedCard` activates it only when the human has two Set cards.
- Checked in the source and the build; not tested in an emulator.

### Compulsory Evacuation Device (2026-10-26)
- Normal Trap: "Target 1 monster on the field; return that target to the hand."
- **Row:** Hane-Hane's handlers. `EffectTargetableMonsterCheck` / `EffectTargetableMonsterChainB` take any monster
  (the CPU takes the human's strongest first); `EffectReturnTargetToHandResolve` returns it with
  `ReturnFieldCardToHand` (owner's hand, a Fusion Monster to the Fusion Deck, a token destroyed).
- With no prepare, a Set copy may answer any response window that has a monster to target.
- The target prompt is Hane-Hane's ("Designate the monster to have returned"), from its case in
  `EffectTargetableMonsterChainB`.
- **CPU:** `AiShouldActivateAddedCard` uses it only against an attack or a summon, when the human has a monster.
- Checked in the source and the build; not tested in an emulator.

### Tristan's format check (2026-10-25)
- Tristan's duels require the Yugi-Kaiba format (May 2002). `tools/banlist.py` turns the repository's
  `yugi-kaiba.txt` (card pool) and `yugi-kaiba_ban.json` (Forbidden/Limited list) into
  `build/include/format_yugi_kaiba.h` (a Makefile rule).
  - Names are matched to EDS cards exactly, then ignoring case and punctuation, then through `ALIASES`
    ("Red-Eyes Black Dragon" is EDS's "Red-Eyes B. Dragon").
  - Four pool cards are not in EDS and are skipped: Baron of the Fiend Sword, Dark Titan of Terror, Mystic Clown,
    Sorcerer of the Doomed.
  - Result: 181 pool cards and 12 limits (10 limited, 2 semi-limited).
- **Check** (`src/mod_banlist.c`, a far-code unit). `OpponentSelect_HandleInput` (campaign_select.c) calls
  `BanlistBlocksDuel` when an opponent is picked. For Tristan it sums the saved Deck, Side Deck and Fusion Deck copies
  of each card (alternate art counted as the same card). A card is illegal if it is not in the pool or exceeds its
  limit (3 when the list does not name it).
- **Warning.** With any illegal card the duel does not start: the error sound plays and a scrollable list opens
  (UP/DOWN, A/B to close), with a header and one line per card ("x3 Pot of Greed - limit 1"). It is drawn into the
  name-banner OBJ tiles (0x2C0) as 8 x 5 sprites over the darkened background, and closing reloads the page.
  `BanlistView_Run` runs first in `OpponentSelect_HandleInput` while it is open. The state is `gBanlistView`, at
  EWRAM 0x0202E000 (symbols.ld).

### The CPU and the added cards (2026-10-25)
- **Why the CPU did not use them.** Its logic is a set of per-card-number scripts, and no added card is named in
  them. Only added cards that took an engine number the scripts know (Magic Cylinder 1214, Thousand-Eyes Restrict
  1334) were covered.
  - `AiPlaySpells` (ai_deck.c) only tries its fixed lists (`gAiGenericSpells`, `gAiEquipSpells` and the named
    cards).
  - `AiShouldActivateSetCard` (ai_summon.c) returns 0 for every card it does not name, so Set added Traps were never
    activated. `AiStepSetSpellTraps` already Set any Trap or Quick-Play it does not name.
  - `AiActivateMonsterEffects` only knows `gAiEffectMonsters` (Time Wizard, Cannon Soldier, Relinquished, Barrel
    Dragon).
  - The summon choice skips Special-Summon-only monsters, and the LIGHT/DARK summon steps of BLS and Chaos Sorcerer
    are a card-list UI with no CPU branch.
  - `PROMPT_SELECT_TYPE` (Tribe-Infecting Virus' declaration) has no CPU branch, so it would have shown the menu to
    the human.
- **The fix** (src/added_effects.c, "The CPU and the added cards"), for card IDs from `CARD_ID_COUNT_ORIGINAL` on:
  - `AiPlayAddedCards`, called at the end of `AiPlaySpells`' `AI_SPELLS_LISTS`, one action per call:
    - it Special Summons BLS or Chaos Sorcerer from the hand (banishing the weakest LIGHT and DARK Graveyard
      monsters, `BanishGraveyardCard`, `QueueSpecialSummonFromHand`);
    - it activates the card-menu effects of face-up added monsters (`CanActivateMonsterEffect`, then point at the
      zone and `Chain_AddPending`, as `AiActivateMonsterEffects` does);
    - it plays added Magic cards from the hand or Set (`AiTryPlaySpellTrap`, which makes the game's own checks).
  - `AiShouldActivateAddedCard`, returned at the end of `AiShouldActivateSetCard` instead of 0: yes for an added card,
    whose prepare handler has already accepted the window.
  - `EffectTribeInfectingVirusChainB`: the CPU declares the Type that costs the human the most face-up monsters
    (theirs minus its own); the human keeps DNA Surgery's prompt.
- **Policies** (the target steps' CPU branches take the human's side first, so each effect waits for a human target):
  - Breaker: the human has a Spell/Trap.
  - BLS: the human has a monster. Chaos Sorcerer: the human has a face-up monster.
  - Virus: a Type gains more than it costs, and the CPU has a card to discard.
  - Exiled Force: the human has a stronger face-up monster or a face-down one.
  - Abyss Soldier: the human has a card. Thousand-Eyes Restrict: the human has a monster.
  - Card Destruction: the human holds more cards than the CPU would discard.
  - Creature Swap: the CPU's weakest monster is weaker than the human's strongest face-up one.
  - Book of Moon (Main Phase): both sides have monsters. Scapegoat: never from the hand in the Main Phase; it is kept
    to be Set.
  - Set cards: Torrential Tribute when the human has more monsters; Scapegoat against an attack or with no CPU
    monster; Book of Moon against an attack; every other added card always.
- Checked in the source and the build; not tested in an emulator.

### Dark Magician of Chaos (2026-10-25, the 2005 text)
- Coded as the card was printed in 2005 (at the user's request), not the current errata: "When this card is Normal or
  Special Summoned, select 1 Spell Card from your Graveyard during the End Phase, and add it to your hand. If this card
  destroys a monster by battle, remove that monster from play. If this face-up card is removed from the field, remove
  it from play." So there is no once-per-turn limit, and the Spell is added even if the card has left the field by the
  End Phase.
- **Summon mark.** `SummonAction_Update` (summon_action.c, next to Tsukuyomi's hook) calls
  `DarkMagicianOfChaosSummoned` (through `AddedCardsSummoned` since 2026-10-26). A face-up Normal Summon or any Special Summon sets bit `player * 5 + zone` in
  `gModDuel.chaosSummoned`; a Set or a Flip Summon does not. `ModDuelInit` clears it at the start of a duel.
- **End Phase.** `DuelPhase_End` (battle_phase3.c, next to the Tsukuyomi and Mystic Swordsman hooks) calls
  `EndPhase_DarkMagicianOfChaos`, which queues a trigger for each mark whose player has a Spell in the Graveyard,
  then clears the marks. The row is Magician of Faith's: `EffectCardListTargetChainB` over the Graveyard Spells
  (Magician of Faith's `CollectEffectTargets` case, and its `AiPickCardListEntry` case for the CPU), then
  `EffectMaskOfDarknessResolve`. The text says "select", so it is mandatory and the list has no way to decline.
- **Battle.** In `BattleStage_DestroyMonsters`, with Dark Magician of Chaos on either side, `gBattle.ddWarriorLady`
  becomes the new `DD_BANISH_DESTROYED` (4), unless D.D. Warrior Lady's Yes is in force. Destroyed monsters are then
  banished (`BanishBattleDestroyedCard`) instead of sent to the Graveyard, and survivors stay. This covers both the
  monster it destroys and itself when it is destroyed.
- **Leaving the field.** `SendFieldCardToGrave`, `TributeMonster` (still returns 1), `ReturnFieldCardToHand` and
  `ReturnFieldCardToDeck` banish a face-up Dark Magician of Chaos instead (`IsFaceUpDarkMagicianOfChaos`,
  `BanishFieldCard`).
- **Not covered:** use as a Fusion material from the field (`SendFusionMaterialToGrave`).
- Checked in the source and the build; not tested in an emulator.

### Abyss Soldier (2026-10-25)
- "Once per turn: You can discard 1 WATER monster; return 1 card on the field to the hand."
- **Activation.** The card menu's ACTIVATE in the Main Phase (`CanActivateMonsterEffect`), once per turn through
  `effectUnused` (re-armed by `DuelCmd_TurnStart`), as for Tribe-Infecting Virus.
- **Prepare:** face up in its zone, effect unused, a WATER monster in hand (not planted or graverobbed), and a card
  on the field it may target.
- **ChainA (cost).** The human gets `PROMPT_DISCARD_COST` with the new flag `DISCARD_FLAG_WATER` (4) beside
  monsters-only (1). `DiscardPrompt_TryDiscardSelected` (duel_turn_end.c) refuses a card that is not a WATER monster
  (`IsWaterMonster`). The CPU discards its first WATER monster directly, since the prompt's CPU pick ignores its flags.
  `DUEL_CMD_SET_EFFECT_UNUSED` 0 uses up the turn's effect.
- **ChainB:** a cursor pick of any card on either field (monster, Spell/Trap or Field zone; `CanCardTargetZone`).
  There is no `CanActivateEffect` re-check, since the cost is paid by then. The CPU returns the human's strongest
  monster, else their first card.
- **Resolve:** `ReturnFieldCardToHand` (owner's hand; a Fusion Monster to the Fusion Deck; a token is destroyed).
- Checked in the source and the build; not tested in an emulator.

### Brain Control (2026-10-25)
- Normal Spell: "Pay 800 LP, then target 1 face-up monster your opponent controls that can be Normal Summoned/Set;
  take control of it until the End Phase."
- It takes control the Change of Heart way, as Mind Control does. `EffectChangeOfHeartResolve` moves the monster and
  links it to Brain Control. `DuelCmd_TurnEnd` returns it (`ReturnControlAtTurnEnd(player, i, CARD_BRAIN_CONTROL)`,
  next to Change of Heart's and Mind Control's). No attack or Tribute restriction.
- **Prepare:** more than 800 LP, a free monster zone, and a monster that `IsBrainControlTarget` accepts. That means
  the opponent's, face up and targetable, and not a Fusion, Ritual, Special-Summon-only monster or token.
- **ChainA:** `DUEL_CMD_LOSE_LP` 800 (a cost).
- **ChainB:** `EffectTakeControlChainB`, with `CARD_BRAIN_CONTROL` added to Snatch Steal's cases there (face-up
  prompt and cursor mask). A wrapper refuses an invalid pick: the human hears the error sound and stays on the
  cursor while a valid monster remains. The CPU's pick, the human's strongest monster, is replaced by the strongest
  valid one.
- Checked in the source and the build; not tested in an emulator.

### Mind Control (2026-10-25)
- Normal Spell: "Target 1 monster your opponent controls; take control of it until the End Phase, but it cannot
  declare an attack or be Tributed."
- **Row:** Change of Heart's handlers (`EffectChangeOfHeartPrepare`, `EffectOpponentMonsterCheck`,
  `EffectTakeControlChainB`, `EffectChangeOfHeartResolve`). The resolve moves the target to a free zone of the
  player and links it to the activating card (`ZONE_LINK_CARD_EFFECT` from `link->card`), here Mind Control.
- **Return:** `DuelCmd_TurnEnd` (duel_cmd_turn.c) now also calls `ReturnControlAtTurnEnd(player, i,
  CARD_MIND_CONTROL)` next to Change of Heart's: the monster goes back, or is destroyed if the owner has no free zone.
- **Restrictions:** `IsMindControlled` (the zone has a link from Mind Control) makes `CanMonsterAttack`
  (card_menu_input.c; the CPU's attack pick uses it too) and `IsTributableMonster` (duel_zones.c; the summon and
  cost tribute checks) refuse the monster.
- **Target step fix (2026-10-25).** `EffectTakeControlChainB` (effect_targets2.c) sets its cursor mask in a
  `switch` on the card number with no default (only five original numbers used it). For Mind Control the mask stayed
  unset and no target could be picked; the cursor stayed on the card. `CARD_MIND_CONTROL` is now in Change of Heart's
  case there, and in the prompt switch above it.
- **Not added:** Mind Control is not in the single-target redirect lists (Fairy's Hand Mirror and similar:
  `CanRedirectEffectToZone` and two others) that name Change of Heart.
- Checked in the source and the build; not tested in an emulator.

### Lightning Vortex (2026-10-25)
- Normal Spell: "Discard 1 card; destroy all face-up monsters your opponent controls."
- **Prepare:** a card to discard (2 in hand when played from the hand) and a face-up monster on the opponent's side.
- **ChainA:** the discard cost `DuelPrompt_PostDiscardCost`.
- **Resolve:** no target. Every face-up monster the opponent then controls is destroyed (`DestroyFieldCard` +
  `OnCardDestroyedByEffect`, as Tribe-Infecting Virus does); face-down monsters stay.
- Checked in the source and the build; not tested in an emulator.

### Don Zaloog (2026-10-25)
- "When this card inflicts battle damage to your opponent: You can activate 1 of these effects. Discard 1 random
  card from your opponent's hand; or send the top 2 cards of your opponent's Deck to the GY."
- **Trigger.** It is in both of `BattleStage_InflictDamage`'s battle-damage trigger lists, next to Kycoo: as the
  attacker (`RESPONSE_BATTLE_DAMAGE`), and as the target when the attacker's player takes the damage
  (`RESPONSE_BATTLE_DEFLECTED_DAMAGE`). The low nibble of `loc1` is the damaged player.
- **Resolve** (`EffectDonZaloogResolve`). `Chain_Build` never calls a prepare handler for pending triggers (checked:
  it only runs chainA/chainB), so the resolve checks the event and the damaged player itself.
  - **Human:** Yes/No for the random discard (when the opponent has a hand), then Yes/No for the Deck; No to both
    uses nothing.
  - **CPU:** it discards if it can, else sends the Deck cards.
  - **Discard:** White Magical Hat's `DuelPrompt_PostRandomDiscard(opponent, 1, 1)`, a discard by an opponent's
    effect.
  - **Deck:** `DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD` for `deck[0]` and `deck[1]` (the top is index 0).
- Checked in the source and the build; not tested in an emulator.

### Creature Swap (2026-10-25)
- Normal Spell: "Each player chooses 1 monster they control and switches control of those monsters with each other.
  Those monsters cannot change their battle positions for the rest of this turn."
- **Prepare:** both players control a monster.
- **Resolve** (`EffectCreatureSwapResolve`): nothing is targeted, so the choices are made here.
  - **Choices.** The human picks one of their monsters with the cursor, in steps like
    `EffectGainOpponentMonsterStatsResolve`; a pick off their side plays the error sound. The CPU gives its weakest
    monster (`AiFindWeakestMonster`).
  - **Swap.** Invader of the Throne's `SwapFieldCards` (`DUEL_CMD_SWAP_ZONES`, with the swapped cards'
    control-change rules such as Ameba's).
  - **Lock.** `DUEL_CMD_SET_POSITION_LOCKED` on both zones. The card menu refuses a position change for a locked
    zone, and `DuelCmd_TurnStart` clears the flag at the start of its controller's next turn.
- **Not modelled:** in a Link Battle, the remote player's monster is chosen by the CPU rule.
- Checked in the source and the build; not tested in an emulator.

### Mystic Swordsman LV2 (2026-10-25)
- "If this card attacks a face-down Defense Position monster: Destroy the monster immediately with this card's
  effect without flipping it face-up or applying damage calculation. At the End Phase of a turn in which this card
  destroyed a monster by battle: You can send this card to the GY; Special Summon 1 'Mystic Swordsman LV4' from your
  hand or Deck."
- **The attack.** It uses Sasuke's `DestroyTargetBeforeDamageStep`, which now has a per-card condition: Sasuke takes
  any Defense Position target, LV2 only a face-down one. That destruction is by effect, so it is not a battle kill.
- **Battle kills.** `BattleStage_DestroyMonsters` calls `MysticSwordsmanAfterBattle` (through `AddedCardsAfterBattle`) once per battle, after D.D.
  Warrior Lady's question. An LV2 on either side whose opponent's monster the battle destroyed gets bit
  `player * 5 + zone` in `gModDuel.battleKills`.
- **Level-up.** `DuelPhase_End` (next to Tsukuyomi's hook) calls `EndPhase_MysticSwordsmanLevelUp`. Each marked zone
  that still holds a face-up LV2 queues a pending trigger (kind monster, `RESPONSE_NONE`, its zone); then the marks
  are cleared. `EffectMysticSwordsmanLv2Resolve` runs in steps:
  - **Ask.** The human gets Yes/No; the CPU always says Yes.
  - **Pay.** It sends the LV2 to the Graveyard (`SendFieldCardToGrave`, not destroyed) and takes an LV4 from the
    Deck (then shuffles), else from the hand. The card is kept in `gChain.effectCard`.
  - **Summon.** The next step, after those commands ran, calls `QueueSpecialSummonChoosePosition`. That function
    picks the free zone when it is called, and the LV2's zone must be empty by then.
  - The row is guarded on both LV2 and LV4 being listed.
- **Not modelled:** the hand/Deck choice. The Deck is taken first.
- **Duel state.** `gModDuel` is at EWRAM `0x0202FF04` (symbols.ld). `DuelPhase_Init` (duel_main.c) now calls
  `ModDuelInit`, which clears it and Injection Fairy Lily's state at the start of every duel.
- **No `static` variables in mod code (2026-10-25).** The linker script puts `.bss` at 0x02000000, on top of
  `gTextCanvas`. Two such variables (`sTsukuyomiEndReturnMask[2]` and `sFlipEffectsLockedForTurn`, 3 bytes) were
  overwritten by text drawing and wrote into it. They are now `gModDuel.tsukuyomiEndReturn[2]` and
  `gModDuel.flipEffectsLocked`, cleared by `ModDuelInit`. The `.ewram` section of build/eds.map must stay 0 bytes long.
- Checked in the source and the build; not tested in an emulator.

### Ninja Grandmaster Sasuke (2026-10-25)
- "If this card attacks a Defense Position monster, destroy that monster immediately with this card's effect without
  applying damage calculation."
- **No effect row.** `BattleStage_RevealDefender` (battle_phase1.c) calls `DestroyTargetBeforeDamageStep` (shared
  with Mystic Swordsman LV2) at its
  first step, before a face-down target is flipped, so the target's flip effect does not run. For a face-up Sasuke
  attacking a monster in Defense Position (face up or down), it does three things:
  - pushes `DUEL_CMD_MARK_ATTACKED`, which the skipped `DAMAGE_STEP_MARK_ATTACKER` would push;
  - shows the card and destroys the target by effect (`DestroyFieldCard` with triggers, so Sangan and Witch search;
    `OnCardDestroyedByEffect`);
  - sets the stage to `BATTLE_STAGE_END_ATTACK`.
- So there is no Damage Step response window, no damage calculation, no battle damage and no "destroyed by battle"
  effects. D.D. Warrior Lady's prompt and Black Luster Soldier's second attack are in the skipped destroy stage.
- Checked in the source and the build; not tested in an emulator.

### Injection Fairy Lily (2026-10-25)
- "During either player's Damage Step, when this card attacks or is attacked: You can pay 2000 LP; this card gains
  3000 ATK during the Damage Step only."
- **No effect row.** EDS offers no face-up monster in a response window, only Set Spell/Trap cards and the hand
  (`EventResponse_Update`). So, as for D.D. Warrior Lady, the effect lives in the battle code:
  - `BattleStage_DamageCalc`, `DAMAGE_STEP_MARK_ATTACKER` (once per attack): `InjectionFairyLilyResetBattle`.
  - `DAMAGE_STEP_CALC`, before `CalcBattle`: `InjectionFairyLilyDamageStep` checks the attacker, then the target
    (not for a direct attack). It acts for a face-up Lily whose controller has more than 2000 LP. The human is
    asked through `PROMPT_CONFIRM_CARD_EFFECT` (text in duel_prompts.c). The CPU pays when the 3000 ATK would turn
    the battle; on a direct attack, when it has more than 4000 LP. Paying shows the card, queues `DUEL_CMD_LOSE_LP`
    2000 (as a cost) and a `ZONE_LINK_ATK_BONUS` link of 3000, then returns 1. The stage waits, because
    `DuelMainStep` runs the phase handlers only when the command queue and the prompts are idle, so `CalcBattle`
    sees the new ATK.
  - `BattleStage_EndAttack`: `InjectionFairyLilyEndAttack` removes that exact link (kind 4, value 3000; Riryoku's
    bonus has the same kind) from a Lily that was boosted.
- **State:** `gInjectionFairyLily` (step and boosted sides) at EWRAM `0x0202FF00` in `symbols.ld`. It is free RAM
  below `gScratchBuffer`, like the Deck Edit lists at `0x0202A000`; nothing in src/, asm/ or data/ refers to
  `0x0202D000`-`0x0202FFFF`.
- **Not modelled:** in a Link Battle, the remote player's Lily is decided by the CPU rule.
- Checked in the source and the build; not tested in an emulator.

### Phoenix Wing Wind Blast (2026-10-25)
- Normal Spell: "Discard 1 card, then target 1 card your opponent controls; place that target on the top of the
  Deck."
- **Prepare:** a card to discard (2 in hand when played from the hand), and a card the opponent controls that it may
  target.
- **ChainA:** the discard cost `DuelPrompt_PostDiscardCost`, as for Raigeki Break and Tribe-Infecting Virus.
- **ChainB** (`EffectPhoenixWingWindBlastChainB`): a cursor pick among the opponent's monster, Spell/Trap and Field
  zones (`PICK_PLAYER1(...)`, `EffectPhoenixWingWindBlastCheck`). The CPU takes the human's strongest monster, else
  their first card. Unlike Raigeki Break's ChainB it does not call `CanActivateEffect` again: the discard is paid by
  then, and the hand count of the prepare could fail.
- **Resolve:** `ReturnFieldCardToDeck`, which puts the card on top of its owner's Deck. A Fusion Monster goes to the
  Fusion Deck and a token is destroyed, the game's own rules for that command.
- Checked in the source and the build; not tested in an emulator.

### Thousand-Eyes Restrict and Thousand-Eyes Idol (2026-10-25)
- **The engine already has the card.** Effect key 1334 (no EDS card) is the old Thousand-Eyes Restrict of the engine
  EDS was built on, and the game still carries all of it:
  - **Recipe:** gFusionRecipes2 holds 1334 = 730 (Relinquished) + 1333, and 1334 is in Polymerization's
    2-material list (effect_fusion.c).
  - **Effect row:** Relinquished's `EffectRelinquishedResolve`, `EffectOpponentMonsterCheck` and `ChainB`.
  - **Absorb:** the menu's ACTIVATE (card_command_menu.c: no absorbed monster yet, a free Spell/Trap zone, once
    per turn through effectUnused, re-armed in duel_cmd_turn.c). Its ATK/DEF become the absorbed monster's
    (card_stats.c, `ZONE_LINK_ABSORBED`), and the absorbed monster is destroyed in its place in battle
    (battle_phase2.c). It counts as an Effect Monster (card_detail.c).
  - **Lock:** while a 1334 is face up, every other monster can neither attack (`CanMonsterAttack`,
    card_menu_input.c, which the CPU uses too) nor change position (the card menu, card_command_menu.c).
- **Card numbers.** `tools/newcards.py` gives such cards the engine's number (`ENGINE_NUMBERS`): Thousand-Eyes
  Restrict 1334 and Thousand-Eyes Idol 1333, the recipe's second material. Thousand-Eyes Idol was added to
  config/new_cards.txt, appended last. No added effect row and no recipe change was needed. Verified in the built
  tables: ID 1111 = number 1334 (Fusion), ID 1129 = number 1333 (Normal), both DARK Spellcaster Level 1, 0/0.
- **One fix.** `EffectRelinquishedResolve` (effect_resolve3.c) only absorbed when the card was Relinquished, so
  key 1334 never absorbed. It now accepts `CARD_1334` too.
- The engine's text, like the card's, takes an opponent's monster (max. 1).
- Checked in the source and the build; not tested in an emulator.

### Metamorphosis (2026-10-25)
- Normal Spell: "Tribute 1 monster. Special Summon 1 Fusion Monster from your Extra Deck with the same Level as the
  Tributed monster."
- **Prepare** (`EffectMetamorphosisPrepare`): one of the player's monsters has the Level of a Fusion Monster in the
  Fusion Deck.
- **Tribute** (`EffectMetamorphosisChainB`): the human picks with Catapult Turtle's `EffectTributeTargetChainB`. A
  monster whose Level matches no Fusion Monster is refused (an error sound, and the cursor stays). The CPU takes its
  highest-Level qualifying monster. The monster is then Tributed (`TributeMonster`) as soon as the pick is made, as
  for Exiled Force. Its Level goes to `targets[1]`, so `numTargets` is 2.
- **Resolve** (`EffectMetamorphosisResolve`): Cyber-Stein's steps and prompt. A `CollectEffectTargets` case lists
  the Fusion Deck's Fusion Monsters of that Level (`IsMetamorphosisFusion`, the Level passed as `param`, which
  `CardListView_Open` passes through). Then `DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK` and `QueueSpecialSummon`, face
  up in Attack Position. The CPU takes the highest ATK. It does not use `AiPickCardListEntry`, which lists with
  `param` 0.
- The Level is the card's printed Level (gCardStats).
- Checked in the source and the build; not tested in an emulator.

### Scapegoat and the Sheep Token (2026-10-25)
- Quick-Play Spell: "Special Summon 4 'Sheep Tokens' (Beast/EARTH/Level 1/ATK 0/DEF 0) in Defense Position. You
  cannot Summon other monsters the turn you activate this card." EDS still holds the handlers of the OCG Scapegoat
  of the engine it was built on, under effect key 1245. The prepare is `EffectNoSummonFourFreeZonesPrepare`: 4 free
  monster zones and no summon this turn. The resolve is `EffectSummonFourTokensResolve`: four
  `DUEL_CMD_SUMMON_TOKEN` of `TOKEN_KIND_2` in Defense Position, then `DUEL_CMD_SET_SUMMON_LOCKS` (Normal and
  Special). Scapegoat's row reuses both; it is guarded on `HAS_CARD_SCAPEGOAT` and `HAS_CARD_SHEEP_TOKEN`.
- **The token card.** `DuelCmd_SummonToken` places `gCardNumberToId[1920 + kind]`, and EDS has a card only at 1920
  (Insect Monster Token). So `Sheep Token` is in config/new_cards.txt, appended last so the other IDs keep their
  places. `tools/newcards.py` gives a token (YGOPRODeck type "Token") the fixed number of its kind from
  `TOKEN_NUMBERS` (Sheep Token: 1922) instead of a free number, with the stats and text given there (YGOPRODeck
  leaves them to the summoning card). It gets Normal-Monster stats (kind 0) and no password (0xFFFFFFFF), as the
  Insect Monster Token has. Verified in the built tables: ID 1110, number 1922, Beast/EARTH/Level 1/0/0.
- The token number range (1920-1999) keeps it out of the starting trunk (`GiveStartingTrunk`), the Deck Edit and
  Card Trading lists (unowned), the prohibition picker and the Password screen. Its other token behaviour (it
  vanishes when it leaves the field) is the game's own, by number.
- The CPU: its strategy code still names key 1245 (ai_strategy.c, ai_deck.c, ai_turn_steps.c), not
  `CARD_SCAPEGOAT`, so it does not play Scapegoat on purpose.
- Checked in the source and the build; not tested in an emulator.

### Book of Moon (2026-10-25)
- Quick-Play Spell: "Target 1 face-up monster on the field; change that target to face-down Defense Position." It
  targets like Chaos Sorcerer (`EffectTargetableFaceUpMonsterCheck` / `ChainB`). `EffectBookOfMoonResolve` first
  destroys the target's Equip Cards, because they cannot equip a face-down monster; a destroyed Premature Burial
  takes its monster with it. A monster in Attack Position then gets `DUEL_CMD_CHANGE_POSITION`, and every target gets
  `DUEL_CMD_FLIP_CARD` (it toggles face up / face down). The flip removes its counters (`OnMonsterTurnedFaceDown`).
- Continuous links (Spellbinding Circle, Call of the Haunted) stay, as in the real rules.
- Checked in the source and the build; not tested in an emulator.

### Sakuretsu Armor (2026-10-25)
"When an opponent's monster declares an attack: Target the attacking monster; destroy that target." Normal Trap
(`gAddedCardEffects` row in `added_effects.c`).
- **Prepare:** set on the field (`fromHand == 0`), `RESPONSE_ATTACK_DECLARED`, opponent's attacking monster still
  face-up in Attack Position at `loc0`, and `CanCardTargetZone` allows it.
- **Check:** only `loc0` (the attacker).
- **Target (chainB):** auto-sets `targets[0] = loc0` (no cursor).
- **Resolve:** `EffectDestroyTargetResolve`.
- Checked in the source; not tested in an emulator.

### Raigeki Break (2026-10-25)
"Discard 1 card, then target 1 card on the field; destroy it." Normal Trap (`gAddedCardEffects` row in
`added_effects.c`).
- **Prepare:** at least `1 + fromHand` cards in hand (one to discard; from hand, the trap itself counts toward
  `fromHand`), another card on the field that `CanCardTargetZone` allows (monsters and Spell/Trap/Field zones).
- **Cost (chainA):** `DuelPrompt_PostDiscardCost` for 1 card (same as Tribe-Infecting Virus / Magic Jammer).
- **Target (chainB):** any occupied zone on either side; human cursor `PICK_ANY_MONSTER | PICK_ANY_SPELL_TRAP`;
  prompt `gStrDesignateFirstCardToDestroy` (Greenkappa's "card" wording, not monster-only text).
- **Resolve:** `EffectDestroyTargetResolve`.
- Checked in the source; not tested in an emulator.

### Reinforcement of the Army (2026-10-25)
- Normal Spell: "Add 1 Level 4 or lower Warrior monster from your Deck to your hand." The resolve is Sangan's own
  `EffectSanganResolve`, which is generic: it lists `CollectEffectTargets` for the card number, lets the human pick
  in the card-list viewer (the CPU: `AiPickCardListEntry`'s best ATK+DEF), adds the card and shuffles the deck.
- The list is a `CollectEffectTargets` case using `IsReinforcementTarget` (type Warrior, Level 4 or lower; Ritual
  Warriors count). `EffectReinforcementOfTheArmyPrepare` needs one in the deck, so the Spell cannot be wasted.
- Checked in the source and the build; not tested in an emulator.

### Card Destruction (2026-10-25)
- Normal Spell: "Both players discard as many cards as possible from their hands, then each player draws the same
  number of cards they discarded." `EffectCardDestructionResolve`; no prepare, so it activates like Pot of Greed.
- Each hand (the activating player's first) goes through `DiscardHandCard` by index. The "compact the hand" operand
  is set only on the last card, so the indices stay valid. The discard triggers therefore run: Night Assailant, and
  Electric Snake and Minar for the opponent's hand (`byOpponentEffect` set). Then each player draws their count; a
  CPU with too short a deck decks out (player 0 cannot).
- Checked in the source and the build; not tested in an emulator.

### Jar of Greed (2026-10-25)
- Normal Trap, "Draw 1 card." `EffectJarOfGreedResolve` (DrawCards 1, unless negated). The row has no prepare or
  check, like Gift of the Mystical Elf's, so `CanActivateEffect` passes it: a Set Jar of Greed can be activated in
  any response window, at Trap speed.
- Checked in the source and the build; not tested in an emulator.

### Night Assailant (2026-10-25)
- **Flip.** "Target 1 monster your opponent controls; destroy that target." Listed in `HasFlipEffect`
  (card_detail.c). It uses Crass Clown's opponent-monster target (`EffectOpponentMonsterCheck` /
  `EffectOpponentMonsterChainB`) and Man-Eater Bug's `EffectDestroyTargetResolve`.
- **Discarded.** "When this card is sent from your hand to the GY: target 1 Flip monster in your GY, except
  Night Assailant; add it to your hand." `DiscardHandCard` (duel_field_moves.c) calls `QueueNightAssailantTrigger`,
  which queues an off-field `RESPONSE_DISCARDED` trigger, built like card 1242's. It is queued only when the
  Graveyard holds a target, because pending triggers enter the chain without a prepare or check. The target is picked
  in Mask of Darkness's card-list viewer (`EffectCardListTargetChainB`; the list is a `CollectEffectTargets` case
  using `IsNightAssailantTarget`). It resolves with `EffectMaskOfDarknessResolve`. The CPU's pick is
  `AiPickCardListEntry`'s default (best ATK+DEF).
- **"Flip monster"** means a monster that `HasFlipEffect(number, 0)` lists, i.e. the flip effects the game codes.
- **Covered:** every `DiscardHandCard` caller (discard costs, Card Destruction, the end-of-turn hand-size discard). **Not covered:** a Night Assailant sent from the hand as a Fusion material
  (`DUEL_CMD_SEND_HAND_FUSION_MATERIAL_TO_GRAVEYARD` in effect_fusion.c), and the direct
  `DUEL_CMD_SEND_HAND_CARD_TO_GRAVEYARD` pushes in duel_card_actions.c (Crush Card on draw) and duel_ritual.c.
- Checked in the source and the build; not tested in an emulator.

### Exiled Force (2026-10-25)
- **Effect.** "You can Tribute this card, then target 1 monster on the field; destroy that target." Main Phase
  ACTIVATE from the card menu (CanActivateMonsterEffect), with no once-per-turn flag. `EffectExiledForcePrepare`
  needs the card face up in its zone and another monster on the field.
- **Target.** Man-Eater Bug's `EffectTargetableMonsterChainB`, wrapped by `EffectExiledForceChainB`. The pick goes
  through `TryAddEffectTarget`, which does not consult the row's check, so the wrapper itself refuses Exiled Force
  (an error sound, and the cursor stays). `EffectExiledForceCheck` refuses it too, for CanActivateEffect.
- **Cost.** `TributeMonster` is called in the chainB wrapper once the target is set, not in chainA. The target step
  runs `CanActivateEffect` again, and the prepare handler would fail once the card had left its zone. A responder
  still sees the card Tributed before the effect resolves. Resolve: `EffectDestroyTargetResolve`.
- Checked in the source and the build; not tested in an emulator.

### Dekoichi the Battlechanted Locomotive (2026-10-25)
A flip effect (added to `HasFlipEffect`). It draws 1 card (`DrawCards`), plus 1 per "Bokoichi the Freightening Car" its
controller has. Bokoichi counts only if it is added to `config/new_cards.txt` (`HAS_CARD_` guard); it is not now.

### Trap Dustshoot (2026-10-25)
- **Activation.** Only while the opponent holds 4 or more cards (prepare).
- **Resolve.** It uses Confiscation's prompt `PROMPT_PICK_OPPONENT_HAND_CARD`, which reveals the opponent's hand for a
  cursor pick. A pick that is not a monster plays `SE_ERROR` and the prompt runs again. The CPU takes the first monster
  in the hand. The card goes to the bottom of the Deck (`ReturnHandCardToDeck`), and the Deck is shuffled.
- **No monster.** If the hand holds no monster, nothing happens, and the hand is not shown either.

### D.D. Warrior Lady (2026-10-25)
There is no effect row. The effect lives in `BattleStage_DestroyMonsters` (battle_phase2.c). When D.D. Warrior Lady is
in a battle with a monster, the stage first asks her controller, before any destroyed monster leaves the field. The
human gets `PROMPT_CONFIRM_CARD_EFFECT`, with its text case in `DuelPrompt_ConfirmCardEffect`; the CPU always says Yes.
The answer is kept in `gBattle.ddWarriorLady` (+0x5, a byte the original never used). On Yes the stage runs like a
Dimensional Warrior battle: a destroyed monster is banished (`BanishBattleDestroyedCard`) instead of sent to the
Graveyard, and a monster that survived is banished too (`BanishFieldCard`). It can't be chained to, unlike the real
card's effect.

### Gravekeeper's Spy (2026-10-25)
A flip effect: it is added to `HasFlipEffect` (card_detail.c), so it triggers on a Flip Summon and when it is flipped
by an attack. It resolves with the floaters' `EffectSpecialSummonFromDeckResolve`: Yes/No for the human, a Deck list,
then a Special Summon in a face-up position of the player's choice (`QueueSpecialSummonChoosePosition`, the
position menu; the CPU uses `AiShouldSetMonster`) and a shuffle. The other users of that handler (the floaters)
keep Attack Position. Its `CollectEffectTargets` case lists deck monsters whose name
starts with "Gravekeeper's" (`IsGravekeeperMonster`), with ATK 1500 or less, excluding Fusion and Ritual monsters.

### Blade Knight (2026-10-25)
"Gains 400 ATK while you have 1 or less cards in your hand. If you control no other monsters, the effects of
Flip monsters destroyed by battle with this card are negated." No `gAddedCardEffects` row (passive rules only).
- **+400 ATK:** `GetZoneCardStats` (`card_stats.c`) adds 400 ATK when its controller's `handCount` is 0 or 1.
- **Flip negation:** `IsBladeKnightCard` matches `CARD_BLADE_KNIGHT` **or** in-game name `"Blade Knight"`.
  `BladeKnightNegatesBattleFlipEffect` uses `gDuel.turnPlayer`,
  `gBattle.atkSlot`, exactly one monster on that side, `battleValue` showing the attacker won, and
  `gBattle.flipCardId` matching the defender. Hooks: `BattleStage_TriggerFlipEffect`, `CanActivateEffect`,
  clear `flipEffectPending` after `CalcBattle`.
- No piercing battle damage (not part of this card's effect).
- Checked in the source; not tested in an emulator.

### Kycoo the Ghost Destroyer (2026-10-25)
"When this card inflicts Battle Damage to your opponent's Life Points, you can remove from play up to 2 Monster
Cards from your opponent's Graveyard." The original duel code already models this as effect key **`CARD_1511`**
(`EffectBanishOpponentGraveMonstersResolve`, battle-damage pending triggers in `BattleStage_InflictDamage`).
The added card reuses that resolve handler and the same `CollectEffectTargets` body (opponent's graveyard
monsters).
- **Row** (`gAddedCardEffects`): `CARD_KYCOO_THE_GHOST_DESTROYER`, resolve
  `EffectBanishOpponentGraveMonstersResolve`, prepare `EffectKycooTheGhostDestroyerPrepare` (battle damage to the
  opponent and at least one valid graveyard target).
- **Battle hooks:** `battle_phase2.c` queues the same pending triggers as `CARD_1511` when Kycoo inflicts battle
  damage (direct or deflected).
- **Graveyard lock:** `CountActiveKycooOnField` counts face-up `CARD_1511` and `CARD_KYCOO_THE_GHOST_DESTROYER`;
  Gravedigger Ghoul, Soul Release, key 1532 and banish-from-grave Special Summon costs use it instead of testing
  only `CARD_1511`.
- **Resolve target list:** `EffectBanishOpponentGraveMonstersResolve` asks `CollectEffectTargets` with
  `CARD_NUMBER(link->card)` so the added card and key 1511 share one implementation.
- Checked in the source; not tested in an emulator.

### Torrential Tribute (2026-10-25)
A Normal Trap for the response window a summon opens: Normal, Flip or Special Summon by either player (not a Set),
with the summoned monster at `loc0`, like Trap Hole's prepare but for any monster. It resolves with Dark Hole's
handler (`EffectDarkHoleResolve`), which destroys every monster, the opponent's first.

### Tribe-Infecting Virus (2026-10-25)
Main Phase menu effect, once per turn: discard 1 card (Magic Jammer's cost prompt), declare a Monster Type (DNA
Surgery's type prompt), destroy every face-up monster of that type on the field, the opponent's first. It does not
target. Hooks: `CanActivateMonsterEffect`, `DuelCmd_TurnStart` (re-arms `effectUnused`).

### Starting trunk (2026-10-25)
A new game puts 8 copies of every card in the trunk (`GiveStartingTrunk`, called by `BuildStarterDeck`): every
original and added card except the monster tokens. Owning every card also meets the collection conditions, such as
Grandpa's and Pegasus' unlocks.

### Black Luster Soldier - Envoy of the Beginning (2026-10-25)
- **Summon.** Special Summon only (`IsSpecialSummonOnly`, so the hand menu offers only Sp. Summon / Sp. Set).
  `CanSummonFromHand` needs a free zone and a LIGHT and a DARK monster in the Graveyard
  (`CanPayBlackLusterSoldierCost`). The summon steps `SUMMON_STEP_BLS_*` (effect_hooks.c) banish one LIGHT, then
  one DARK monster through the Graveyard list (`CollectEffectTargets` case, `BlackLusterSoldierBanishAttribute`).
- **Effects** (one row, once per turn via `effectUnused`; `DuelCmd_TurnStart` re-arms it):
  - Main Phase, from the menu (`CanActivateMonsterEffect`): target a monster on the field (Man-Eater Bug's check and
    target pick), banish it (`BanishFieldCard`); BLS cannot attack this turn (`DUEL_CMD_SET_CANNOT_ATTACK`).
  - Attack again: `BattleStage_DestroyMonsters` queues a `RESPONSE_BATTLE_DESTROYED` trigger when the attacking BLS
    destroyed the target and survived. Resolve asks the human Yes/No (the CPU always takes it), then clears its
    `attackedMask` bit.
- **Not modelled.** The CPU never Special Summons it (no CPU deck has it). The text box string is new.

- **Verified statically.** The row is in the ROM with card number 143 and five Thumb handler pointers. The pinned
  data keeps its addresses, and the build has no new warnings. Not yet run in an emulator.

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
