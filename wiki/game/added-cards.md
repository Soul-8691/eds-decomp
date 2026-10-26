---
title: Added cards (config/new_cards.txt)
type: game
status: draft
confidence: high
sources: [rom-analysis, web-card-references]
updated: 2026-10-25
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
  - **Red starter deck:** 8 BLS - Envoy of the Beginning, 3 Breaker, 3 Shining Fairy, 3 Mystic Tomato, 1 Dark Hole,
    and 22 strong LIGHT/DARK Normal monsters (40). Dark Hole is limited to 1, so the other 2 go to the trunk
    (`BuildModStarterDeck`, called by `BuildStarterDeck`).
  - **Yugi Muto:** a competitive 40-card deck (`GetOpponentDeckOverride`): Jinzo, Summoned Skull, Vorse Raider,
    floaters and searchers, the limited staples, Mystical Space Typhoon, Trap Hole.
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
