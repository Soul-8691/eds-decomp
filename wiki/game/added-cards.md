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

### Book of Moon (2026-10-25)
- Quick-Play Spell: "Target 1 face-up monster on the field; change that target to face-down Defense Position." It
  targets like Chaos Sorcerer (`EffectTargetableFaceUpMonsterCheck` / `ChainB`). `EffectBookOfMoonResolve` first
  destroys the target's Equip Cards, because they cannot equip a face-down monster; a destroyed Premature Burial
  takes its monster with it. A monster in Attack Position then gets `DUEL_CMD_CHANGE_POSITION`, and every target gets
  `DUEL_CMD_FLIP_CARD` (it toggles face up / face down). The flip removes its counters (`OnMonsterTurnedFaceDown`).
- Continuous links (Spellbinding Circle, Call of the Haunted) stay, as in the real rules.
- Checked in the source and the build; not tested in an emulator.

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
