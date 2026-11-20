# Adding New Cards

How a card gets from a name in a text file to a working card in the game, from the simplest case (a vanilla monster)
to the hardest (a new mechanic). Every function named here exists in the source; `src/added_effects.c` is where
almost all card code lives, and the cards already in it are the best examples to copy.

## Contents

1. [The card list](#1-the-card-list)
2. [Level 0: cards that need no code](#2-level-0-cards-that-need-no-code)
3. [Level 1: one effect row](#3-level-1-one-effect-row)
4. [Level 2: triggers](#4-level-2-triggers)
5. [Level 3: rules that change the game](#5-level-3-rules-that-change-the-game)
6. [Level 4: new mechanics](#6-level-4-new-mechanics)
7. [The CPU](#7-the-cpu)
8. [Toolbox: functions for common effects](#8-toolbox-functions-for-common-effects)
9. [Testing in the emulator](#9-testing-in-the-emulator)
10. [Checklist](#10-checklist)

---

## 1. The card list

| File | What it is |
|---|---|
| `config/new_cards.txt` | The full list (`make`, `make release`). One English card name per line (`name \| shorter in-game name` for a name over 41 characters). |
| `new_cards.txt` | The demo list (`make demo`, `make release-demo`): only the lines **above `------`** are built. |
| `config/leftover_cards.txt` | The cards of the engine's **leftover effects** (`number \| name`), added to **every** build after the list's cards, each at its own card number (see Level 0). Don't put these in the lists. |
| `overlaps.txt` | Leftover cards that stay in the lists at a free number because `src/added_effects.c` has code of its own for them (to be merged later). |
| `demo_cards.txt` | Generated: every card the demo build adds, alphabetical (`tools/gen_new_cards_md.py`). Don't edit it. |

Add a new card's name to both lists. The added card IDs are, in order: the list's cards (821, 822, ...), then the
leftover cards, then, in `make demo` only, **blank slots** up to 615 added cards (`NEWCARDS_PAD_TO`: no name, stats
or art, left out of every card list, for filling in later in Hex Maniac Advance). Inserting or removing a line
changes the IDs after it, so a save made with another list shows other cards. The capacity is 615 added cards
(`python3 tools/newcards.py capacity`).

On the next build, `tools/newcards.py`:

- fetches the card from YGOPRODeck (name, type, race, attribute, Level, ATK/DEF, text, passcode, art) into
  `assets/new_cards/cards.json` and `assets/new_cards/art/<passcode>.png`;
- packs its stats word, name, text and art into the card tables;
- gives it a card number and writes `CARD_<NAME>` into `include/constants/cards.h` and `HAS_CARD_<NAME>` into
  `build/include/card_count.h`.

Wrap every piece of code for a card in `#ifdef HAS_CARD_<NAME>`, so a list without that card still builds. Code
that walks every card ID stops at `CARD_ID_LAST_REAL` (`include/card_data.h`), not `CARD_ID_LAST`, so it skips the
blank slots; the debug builds' starting trunk (`GiveStartingTrunk`) gives 8 copies of every real card.

Things `newcards.py` lets you control:

| Table in `tools/newcards.py` | Use |
|---|---|
| `ENGINE_NUMBERS` | Give the card an **engine key**: a number the original engine already has code for (see Level 0). |
| `TOKEN_NUMBERS` | Tokens take the fixed number of their token kind. |
| `SUBTYPE_OVERRIDES` | Change a Spell/Trap's subtype (Blast with Chain is an Equip Trap). |
| `EXTRA_RACES` | Races EDS lacks (Psychic, Wyrm, ...): stored as the closest EDS race plus `gCardTypeEx`. |

The card's type comes from YGOPRODeck: Normal/Effect/Fusion/Ritual monster, Spell/Trap with its subtype, and the
Extra Deck kinds the engine lacks, kept in `gCardTypeEx` (the stats word stores them as Fusion):

| Kind | Read from the card's text | Docs |
|---|---|---|
| Synchro (and Tuners) | the Tuner rule -> `build/include/synchro_rules.h` | [wiki/game/synchro.md](wiki/game/synchro.md) |
| Xyz (Rank in the Level field) | the materials (count, Level, Type/Attribute) -> `build/include/xyz_rules.h` | [wiki/game/xyz.md](wiki/game/xyz.md) |
| Link (Link Rating in the Level field, DEF 0) | the materials -> `build/include/link_rules.h`; the Link Arrows -> `gCardLinkMarkers` | [wiki/game/link.md](wiki/game/link.md) |

Pendulum cards, and monsters with a Type or Attribute EDS can't show, stop the build.

---

## 2. Level 0: cards that need no code

- **Normal monsters**, and effect monsters whose effect you leave out, work as soon as they are in the list.
- **Leftover effects (engine keys).** The original game has code for about 150 cards that are not in it ("no EDS
  card" in `include/constants/cards.h`, `CARD_1xxx`; the inventory is `LEFTOVER_EFFECTS.md` on the `binary` branch).
  - If the leftover code is all your card needs, add `number | name` to **`config/leftover_cards.txt`** and nothing
    else: every build adds the card at that number, so the original code plays it (Magic Cylinder 1214, United We
    Stand 1449, Mage Power 1450, Nightmare's Steelcage 1230, Fire Princess 1434, ...). The file lists every working one
    already; the ones left out are noted at its top.
  - If the card needs added code on top of the leftover code, put it in the lists and give it the key in
    `ENGINE_NUMBERS` (`tools/newcards.py`); then `config/leftover_cards.txt` skips it (Big Bang Shot is listed at 1313,
    so Fairy Meteor Crush's 1313 is skipped).
  - A listed card that has its own code at a free number although the engine has a key for it is an overlap
    (`overlaps.txt`): at the key both would run, so don't move it there without merging the two.
  To check a key: `grep -rn "CARD_1xxx" src/` and read what each place does with it. Check every use: some keys look
  right and are not (1244 is Shadow Spell, not Nightmare Wheel).
- **Copying an original card.** If your card is an existing card with a small difference, look for where the original
  card's number is handled (`grep -rn CARD_<ORIGINAL> src/`) and add yours next to it.

---

## 3. Level 1: one effect row

Most cards are one row of **`gAddedCardEffects`** (end of `src/added_effects.c`). It has the same layout as the
engine's table:

```c
#ifdef HAS_CARD_MY_CARD
    {
        CARD_MY_CARD, 0,
        EffectMyCardResolve,    /* resolve:  what the effect does */
        EffectMyCardCheck,      /* check:    may it touch the card at (player, zone)?  (targets) */
        EffectMyCardPrepare,    /* prepare:  may it be activated now? */
        EffectMyCardChainA,     /* chainA:   the cost, paid when it is activated */
        EffectMyCardChainB,     /* chainB:   choose its targets */
    },
#endif
```

Any slot can be `NULL`. `FindCardEffect` finds the row by card number; the engine calls the handlers.

### The handlers

| Handler | Signature | Returns |
|---|---|---|
| prepare | `int (struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)` | 1 = may activate. `chainLink` is the link it answers (NULL = not answering), `fromHand` 1 from the hand. |
| check | `u16 (struct ChainEntry *card, u16 pos)` | 1 = the card at `pos` (`player \| zone << 8`) is a legal target. |
| chainA | `int (struct ChainEntry *link, struct ChainEntry *prevLink)` | 1 = done. Called every frame until it returns 1 (step in `gChain.costStep`). |
| chainB | `int (struct ChainEntry *link, struct ChainEntry *prevLink)` | 1 = done. Called every frame until it returns 1 (step in `gChain.targetStep`). |
| resolve | `int (struct ChainEntry *link, struct ChainEntry *chainedTo)` | The next step, or `EFFECT_STEP_DONE`. |

**Resolve steps.** Chain_Resolve calls the resolve with `gChain.effectStep` set to `EFFECT_STEP_START`, then again with
whatever it returned, after any queued commands, text boxes or card lists have finished. Return `EFFECT_STEP_2`,
`EFFECT_STEP_3`, ... to continue, `EFFECT_STEP_END` or `EFFECT_STEP_DONE` to finish (`include/effect.h`). Always
`if (link->negated) return EFFECT_STEP_DONE;` first.

**The link** (`struct ChainEntry`, `include/chain.h`): `card` (card ID), `player`, `zone`, `kind` (`CHAIN_KIND_SPELL_TRAP`,
`CHAIN_KIND_MONSTER`, `CHAIN_KIND_OFF_FIELD`), `event` (the window it answers: `RESPONSE_SUMMONED`,
`RESPONSE_ATTACK_DECLARED`, ...), `loc0`/`loc1` (the event's cards: `DUEL_LOC_PLAYER(loc)`, `DUEL_LOC_ZONE(loc)`),
`numTargets` / `targets[]`, `negated`.

### Reuse before writing

Many rows need no new handler at all. Ready-made handlers (cast them to the row's types as the existing rows do):

| Effect | Check | ChainB | Resolve |
|---|---|---|---|
| Destroy 1 monster (any) | `EffectTargetableMonsterCheck` | `EffectTargetableMonsterChainB` | `EffectDestroyTargetResolve` |
| Destroy 1 opponent's monster | `EffectOpponentMonsterCheck` | `EffectOpponentMonsterChainB` | `EffectDestroyTargetResolve` |
| Destroy 1 Spell/Trap (any) | `EffectAnySpellTrapCheck` | `EffectSpellTrapTargetChainB` | `EffectDestroyTargetResolve` |
| Destroy 1 opponent's Spell/Trap | `EffectDustTornadoCheck` | `EffectSpellTrapTargetChainB` | `EffectDestroyTargetResolve` |
| Return 1 opponent's monster to hand | `EffectOpponentMonsterCheck` | `EffectOpponentMonsterChainB` | `EffectReturnTargetToHandResolve` |
| Turn 1 monster face-down | `EffectTargetableFaceUpMonsterCheck` | `EffectTargetableFaceUpMonsterChainB` | `EffectBookOfMoonResolve` |
| Lock 1 opponent's monster | `EffectOpponentMonsterCheck` | `EffectOpponentMonsterChainB` | `EffectSpellbindingCircleResolve` |

Examples: Old Vindictive Magician and Zaborg are rows made only of these; Nightmare Wheel reuses Spellbinding Circle's.
When a ready-made row is almost right, keep its ChainB and resolve and write only a check: Gravekeeper's Guard is
Hane-Hane's flip (`EffectTargetableMonsterChainB`, `EffectReturnTargetToHandResolve`) with
`EffectGravekeepersGuardCheck`, which allows the opponent's monsters only. The CPU's ChainB picks through
`TryAddEffectTarget`, which asks the check, so it follows the new rule too.

### Typical shapes (copy the named card)

| Card shape | Copy |
|---|---|
| Normal Spell/Trap, no target | Jar of Greed (`EffectJarOfGreedResolve`) |
| "Discard 1 card, then target ..." Trap | Raigeki Break, Karma Cut (`DuelPrompt_PostDiscardCost` in chainA) |
| Pay LP cost | Royal Oppression (`DUEL_CMD_LOSE_LP` in chainA) |
| Counter Trap negating a Spell/monster effect | Divine Wrath, Magic Drain, Cursed Seal of the Forbidden Spell |
| Negate a card that would destroy | My Body as a Shield (`sMyBodyDestroyers`), Stardust Dragon |
| Attack-declaration Trap | Draining Shield, Dimension Wall (`event == RESPONSE_ATTACK_DECLARED`, `loc0` = attacker) |
| Equip | Blast with Chain, Big Bang Shot |
| Monster with a card-menu ACTIVATE (ignition) | Magical Scientist, Stealth Bird, Roulette Barrel |
| Pick cards from a pile | Magical Scientist, Marauding Captain (`CollectEffectTargets` + `CardListView_Open`) |
| Continuous Trap / Field Spell that stays on the field | Skill Drain, Necrovalley (a resolve that only returns `EFFECT_STEP_DONE`; the effect is a check elsewhere, Level 3) |
| Face-up monster that answers a chain link (Quick Effect) | Stardust Dragon, Dark Balter the Terrible, Ryu Senshi (`ModMonsterCanRespond`) |
| Summon trigger that picks something | Ojama King (picks the opponent's empty zones in ChainB with `DuelCursor_PickAny`) |

**Flip effects:** add the card to `HasFlipEffect` (`src/card_detail.c`); the engine then queues the link on every flip.
Old Vindictive Magician is the simplest example.

**Pitfalls**

- **A Spell played from the hand is a link of kind 0**, not `CHAIN_KIND_SPELL_TRAP`: test the card's type
  (`CARD_STATS_TYPE`, `CARD_STATS_SUBTYPE`), never `link->kind`, to recognise "a Spell" or "a Normal Spell" (Dark
  Deal, Dark Balter). Use `link->kind != CHAIN_KIND_OFF_FIELD` to leave out Graveyard effects.
- **A Continuous card's prepare must refuse its own face-up copy.** The engine asks the prepare of face-up cards in
  response windows too, so a prepare that only checks the cost offers to activate the card again (Skill Drain:
  `card->zone` already holds it face-up -> return 0).
- **C89:** the compiler is agbcc, so declarations come before statements in a block. To run something first in a
  function that declares variables, move the old body into a helper (`ModDuelStateCheck` -> `ModDuelStateRules`).

---

## 4. Level 2: triggers

A trigger is a link the game queues itself, with `Chain_AddPending`. Build the packed word with the macros at the top
of `src/added_effects.c`:

```c
Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                     | TRIGGER_EVENT(RESPONSE_SUMMONED) | (id & CARD_ID_MASK),
                 DUEL_LOC(player, zone));
```

The prepare then checks `card->event` (and `loc0`), so one row can serve both a trigger and a menu ACTIVATE (Golem
Sentry, Night Assailant).

Where to queue it (most likely first):

| When | Where |
|---|---|
| Summoned (Normal / Flip / Special) | `AddedCardsSummoned` (`src/added_effects.c`): `kind` is the summon kind (`kind >= SUMMON_ACTION_SPECIAL` for a Special Summon), `faceUp` its position. Junk Synchron, Marauding Captain, Golem Sentry, Ojama King, The Last Warrior from Another Planet. |
| Tribute / Normal Summoned (engine list) | The two Normal Summon paths in `src/summon_checks.c` (Zaborg, Mobius, Granmarg). |
| Synchro / Xyz / Link Summoned | `AddedCardsSummoned`: `ModSynchroArrived` returns whether it was a Synchro Summon (Junk Warrior); `ModXyzArrived` attaches an Xyz Summon's materials and `ModLinkArrived` stands a monster in its Extra Monster Zone there, so a trigger for those goes after them. |
| Your Standby Phase | `ModStandbyStep` (a part of its switch, `gModDuel.standbyPart`): Vampire Lord, Treeborn Frog, Nightmare Wheel; Lava Golem's 1000 damage shares Nightmare Wheel's part. |
| End Phase | The `EndPhase_*` calls in `DuelPhase_End` (`src/battle_phase3.c`): Stardust Dragon, Limiter Removal. |
| Destroyed by battle | `SendBattleDestroyedCardToGraveyard` (`src/duel_field_moves.c`), `src/battle_phase2.c`. |
| Destroyed by an effect | `DestroyFieldCard` (`src/duel_field_moves.c`) and `OnCardDestroyedByEffect` (`src/effect_hooks.c`): Vampire Lord. |
| Sent to the Graveyard / discarded | `SendFieldCardToGrave` (`src/duel_send_to_grave.c`), `DiscardHandCard` (`src/duel_field_moves.c`). |
| An attack ends | `ModEndAttack` (`src/added_effects.c`). |

A Graveyard trigger is `CHAIN_KIND_OFF_FIELD` (Sangan, Night Assailant's discard).

---

## 5. Level 3: rules that change the game

Continuous effects and replacements are not rows: they are checks in the engine. The engine asks a `Mod...` function
in `src/added_effects.c`, so the original files only get one call each.

| Rule | Ask in / existing hook |
|---|---|
| ATK/DEF of a face-up monster | `ModStatBonus` (`GetZoneCardStats`): returns ATK in the low half, DEF in the high half. Gyaku-Gire Panda, Colossal Fighter, Necrovalley (+500 for "Gravekeeper's"), Fusilier Dragon (half its printed stats). Returns 0 for a negated monster (`isDisabled`). |
| A monster's effects negated | Set `isDisabled` on its zone: `CountActiveCardsOnField` and most checks skip it. A monster effect that resolves on the field: `ModMonsterLinkNegated` (`Chain_Resolve`). Skill Drain, Junk Synchron. |
| Flip effects do not start | `FlipEffectsLockedForTurn` (asked wherever a Flip effect starts): Fiend Skull Dragon, Skill Drain. |
| A Spell/Trap that targets a card is negated | `ModTargetedLinkNegated` (`Chain_Resolve`): Ryu Senshi (Spells), Fiend Skull Dragon (Traps). |
| A monster destroyed by battle has its effects negated | `HaDesNegatesBattleDestroyed`: Dark Ruler Ha Des, Dark Balter the Terrible. |
| Cannot Summon | `CanNormalSummon` / `CanSpecialSummon` (`src/effect_hooks.c`): The Last Warrior from Another Planet. The card menu only takes Summon and Flip away for `CanNormalSummon`, so Set stays. |
| A Monster Zone cannot be used | `IsMonsterZoneFree` asks `ModZoneBlockerAt` for a `ZONE_LINK_CONTINUOUS` link on the empty zone: Ojama King (the engine's key 1320 works the same way). |
| Cards cannot leave the Graveyard | `ModFilterEffectTargets` (end of `CollectEffectTargets`) drops the Graveyard entries of every effect's list: Necrovalley. |
| Summon or Set without Tributes | `CanSummonFromHand` (`src/summon_checks.c`) and `CardMenu_SummonMonster` (`src/effect_hooks.c`): Fusilier Dragon. |
| Special Summon procedure from the hand | `IsSpecialSummonOnly` (`src/card_detail.c`), `CanSummonFromHand`, and a step of `CardMenu_SummonMonster` that calls a step function in `src/added_effects.c`: Lava Golem (`SUMMON_STEP_LAVA_GOLEM`), Black Luster Soldier. |
| Cannot attack / change position | `CountZoneLinksFromCard` (`src/duel_piles.c`): Nightmare Wheel counts as Spellbinding Circle. `ModAttacksBlocked`, `ModCannotAttackDirectly`, `ModAttackTargetLocked`. |
| Cannot activate | `CanActivateEffect` (`src/effect_activation.c`): `ModCardNameSealed` (Cursed Seal), Spell Canceller, Imperial Order. |
| Battle damage changed | `InflictBattleDamage` (`src/duel_card_actions.c`): `ModTakesNoDamage`, `ModRedirectBattleDamage` (Dimension Wall). |
| Effect damage changed | `LoseLifePoints`: `ModTakesNoEffectDamage`; redirected: `ModEffectDamageTarget` (Barrel Behind the Door). LP paid as a cost does not go through `LoseLifePoints`. |
| Piercing | `ModHasPiercing` (`CalcBattle`, `src/campaign.c`). |
| ATK/DEF changes | `GetZoneCardStats` (`src/card_stats.c`: the Equip and link cases), or a `ZONE_LINK_ATK_BONUS` link (see Toolbox). |
| Banished instead of leaving | `IsFaceUpDarkMagicianOfChaos` (asked by every way off the field): Quillbolt Hedgehog. |
| Leaving the field does something | `BigBangShotLeaves` / `BigBangShotMonsterLeaving` pattern (Big Bang Shot). |
| Destroyed when its target leaves | `DestroyLinkedCards` (`src/duel_send_to_grave.c`). |
| Checked every idle frame | `ModDuelStateCheck` (Berserk Gorilla, Level Limit - Area B, Skill Drain's `SkillDrainUpdate`). It runs only when the duel is idle (no chain, summon or prompt), before response windows open. |
| Turn-start reset of "once per turn" | `DuelCmd_TurnStart` (`src/duel_cmd_turn.c`) sets `effectUnused`. |
| After each chain link | `ModChainLinkResolved` (Barrel Behind the Door ends its redirect; Skilled Dark Magician's counters). |

**State.** Anything a card must remember during a duel goes in `struct ModDuelState gModDuel` (top of
`src/added_effects.c`), cleared in `ModDuelInit`. It is small (it must end before 0x02030000; the build fails with
`ModDuelStateFits` if not), so use bytes and bit masks. `struct ModDuelExtra gModDuelExtra` (80 bytes, nearly full)
holds the overflow.

A mark that belongs to one monster can go in a spare bit of its `struct DuelZone` (`include/duel.h`): `ClearZone`
wipes the whole zone when the card leaves, and a control change copies it with the card. `modHalvedStats` (Fusilier
Dragon) and `modSkillDrained` (Skill Drain) are taken; `unk8_3:5` is free.

**Two rules that undo each other.** If your check reads a state that the engine recomputes from the card your rule
changes, the two can flip each other every frame and hang the duel (Skill Drain first read Jinzo's `trapsNegated`).
Decide from the cards on the field, not from the engine's derived flags.

---

## 6. Level 4: new mechanics

Something the engine has no idea of (Synchro, Xyz and Link Summoning, the Extra Monster Zones, new races) needs:

- **Data:** new card types go in `gCardTypeEx` (`tools/newcards.py`, `include/card_data.h`); the packed stats word has
  no free bits.
- **A new unit:** `src/mod_<name>.c`, listed in `units.txt` with `@farcode` and `@rodata` (keeps it out of the
  size-limited code), state at a fixed EWRAM address in `symbols.ld`.
- **UI:** reuse the card menu (`CardMenu_GetAvailableCommands` in `src/card_menu_input.c`, run in `CardMenu_Execute`,
  `src/duel_cmd_queue.c`), the card list (`CardListView_Open`) and the text box (`TextBoxOpen`, `TextBoxSetMenu`) rather
  than drawing new screens.

Full examples: Synchro ([wiki/game/synchro.md](wiki/game/synchro.md), `src/mod_synchro.c`), Xyz with its materials
(`src/mod_xyz.c`, [wiki/game/xyz.md](wiki/game/xyz.md)), Link with the Extra Monster Zones, the Link Arrows and the
zone pick for every summon (`src/mod_link.c`, [wiki/game/link.md](wiki/game/link.md)).

**Data for Hex Maniac Advance.** `make demo` and `make release-demo` write a TOML next to the ROM
(`tools/hma_toml.py`, [wiki/tools/hma-toml.md](wiki/tools/hma-toml.md)). A new data table that people may want to edit
goes in its `mod_anchors` (found by its symbol, so it follows every build) with a line in its header guide; keep
the data in a layout HMA can show (one array per kind of data, images as plain or tiled pixels).

**Size limit.** The release build's `.text` must end at or before `0x08080A20`
(`grep '^\.text ' build/eds.map`; the demo build's ends at 0x0807F7F8 as of 2026-11-20). Code in `src/added_effects.c` and other `@farcode` units does not count; every line
added to an **original** unit does. Keep hooks in original files to one call or one `case` label.

---

## 7. The CPU

The CPU only plays added cards through these, all in `src/added_effects.c`:

| Function | When it runs | What to add |
|---|---|---|
| `AiPlayAddedCards` | Its Main Phase | Special Summons, monster ACTIVATEs, and Spells (through `AiWantsSpell`). |
| `AiWantsSpell` | For each added Spell it holds | A `case` saying when to play it (Nightmare's Steelcage, Enemy Controller). |
| `AiShouldActivateAddedCard` | A Set card or face-up card in a window | A `case`: return 0 to never flip it blindly (cards it chains itself), or a condition. Default: 1 (activate). |
| `AiChainSummonTriggerResponse` | Before the CPU answers any chain link | Call your `AiChain<Card>(entry)` here: Counter Traps, quick effects (My Body as a Shield, Stardust Dragon, Cursed Seal). |
| `AiWantsMonsterEffect` | A face-up added monster's ACTIVATE | A `case` with the condition. |
| `AiHasSummonTrigger` | Summon choice | Add monsters with a summon trigger, so the CPU values them. |
| `IsBattleFlipRemoval` | Flip removal planning | Flip monsters that destroy an attacker. |
| `AiFlipRemovalMonsterNumber`, `AiSimFlipRemovalEffect`, the `CARD_HANE_HANE` case in `src/ai_steps.c` | Flip Summoning a Flip monster that removes a blocker | Flip monsters that remove one of the human's monsters (Gravekeeper's Guard is Hane-Hane here). |
| `AiSpecialSummonCyberDragon` | Its Main Phase, **before** it picks a Normal Summon | Special Summons from the hand that a Normal Summon would block (Cyber Dragon, Gilasaurus, Lava Golem). |
| `AiMetamorphosisFusionFor` | Metamorphosis | Which Fusion Monster with a coded effect it takes for a Tribute of each Level. |

Chain a Set card with `AiChainSetCard(entry, CARD_X)`, play a card with `AiTryPlaySpellTrap(CARD_X)`. In a handler, the
CPU is `link->player == 1` (`AI_PLAYER`); it does not answer prompts, so pick its choice in code (set
`gCardListView.top` to the entry and skip the list, as Magical Scientist does).

Traps and Quick-Play Spells are Set by `AiStepSetSpellTraps` without any code.

---

## 8. Toolbox: functions for common effects

Players are 0 (the human) and 1 (the CPU); zones 0-4 monsters, 5-9 Spell/Trap, 10 Field. Commands are queued
(`DuelCmd_Push`) and run after the handler returns.

**Reading the field**

| Function | Use |
|---|---|
| `ZoneOf(player, zone)` | The zone (`card`, `isFaceUp`, `isDefense`, `isDisabled`, `effectUnused`, `links[]`). |
| `DUEL_CARD_ID(&z->card)`, `CardNumberOfId(id)` | Card ID, card number (compare with `CARD_<NAME>`). |
| `gCardStats[id & CARD_ID_MASK]` + `CARD_STATS_ATK / _DEF / _LEVEL / _TYPE / _ATTR / _KIND` | Printed stats. |
| `GetZoneCardAtk(player, zone)`, `GetZoneCardStats(player, zone, &out)` | Current ATK / stats. |
| `CountMonsters`, `CountFreeMonsterZones`, `FindFreeMonsterZone`, `CountActiveCardsOnField(player, number)` | Counting. |
| `FindHandCardByNumber(player, number)`, `gDuelPlayers[p].hand / deck / graveyard / fusionDeck` | Hand and piles. |
| `CanCardTargetZone(cardId, player, zone)` | Respects untargetable cards. |
| `IsSameCardName(id1, id2)` | Same name. |

**Doing things**

| Effect | Function |
|---|---|
| Destroy | `DestroyFieldCard(player, zone, 1)` (call `OnCardDestroyedByEffect(src, player, zone)` for "destroyed by an effect" triggers); `DestroyFieldCardByEffect(player, zone)` also flags Sangan and Witch of the Black Forest so their Graveyard triggers run |
| Banish from the field | `BanishFieldCard(player, zone, 0)` |
| Banish from the Graveyard | `DuelCmd_Push(DUEL_CMD_BANISH_GRAVEYARD_CARD, word halves)` |
| Send to the Graveyard | `SendFieldCardToGrave(player, zone, destroyed, runTriggers)` |
| Return to hand / Deck | `ReturnFieldCardToHand(player, zone, 0)`, `ReturnFieldCardToDeck(player, zone)` |
| Tribute | `TributeMonster(player, zone)` |
| Discard | `DiscardHandCard(player, handIndex, 0, 1)`; as a cost: `DuelPrompt_PostDiscardCost(player, count, 0, 0)` |
| Draw | `DrawCards(player, count)` |
| Search | `AddDeckCardToHand(player, number)` |
| Damage / heal | `LoseLifePoints(player, n)`, `GainLifePoints(player, n)`; LP cost: `DuelCmd_Push(DUEL_CMD_LOSE_LP, n, 1, 0)` |
| Special Summon | `QueueSpecialSummon(player, &card, faceUp, isDefense, 0)`, `QueueSpecialSummonChoosePosition(player, &card, 1, 0)`. It takes its zone when queued, so after Tributes wait until their zones are empty. To the opponent's field: `RemoveCardFromHand` then `QueueSpecialSummon(opponent, ...)` (Lava Golem; the owner stays). |
| Take a card out of a pile first | `DUEL_CMD_REMOVE_CARD_FROM_GRAVEYARD / _DECK / _FUSION_DECK / _HAND` with the card word |
| Negate a chain link | `DuelCmd_Push(DUEL_CMD_NEGATE_ACTIVATION, 1, 0, 0)`; destroy its Spell/Trap: `DUEL_CMD_SET_SPELL_TRAP_DISABLED` |
| Negate an attack | `DuelCmd_Push(DUEL_CMD_NEGATE_ATTACK, attackerZone, 0, 0)` |
| ATK up while on the field | `QueueAddZoneLink(player, amount, DUEL_LOC(player, zone), ZONE_LINK_ATK_BONUS)` |
| Show the card | `ShowCardEffect(player, id)` |

For the opponent's side, OR `DUEL_CMD_PLAYER` into the command (`player ? DUEL_CMD_PLAYER | cmd : cmd`).

**Asking the human**

| What | How |
|---|---|
| Yes / No | `TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 7), TEXTBOX_FLAGS_DEFAULT, text)`, `TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL)`; next step reads `gTextBox.result` |
| Pick a card on the field | `DuelCursor_PickTarget(mask)` (monsters need a face bit and a position bit; `PICK_PLAYER1(mask)` is player 1's side, `PICK_BOTH_SIDES(mask)` both), then `TryAddEffectTarget(link, player, zone)`. An empty zone: `DuelCursor_PickAny` (Ojama King). |
| Pick from a pile | Add a `case` to `ModCollectEffectTargets` (`ModAddPileTargets(pile, count, source, accept, param)`), then `CollectEffectTargets(player, CARD_X, 0)` and `CardListView_Open(player, CARDLIST_AREA_EFFECT_TARGETS, CARD_X, 0)`; the pick is `gCardListView.cards[gCardListView.cursorRow + gCardListView.top]` |
| Custom prompt | `TextBoxSetMenu(TEXTBOX_MENU_CUSTOM, draw, input)` (the Synchro picks) |

---

## 9. Testing in the emulator

A quick way to see a card work is a **probe**: a temporary scenario that sets up the board, plays it and records a
video.

1. Add an entry to the `MEDIA` list in `tools/media/gen_duel_media.py` (copy one), with the board (`you.hand: ...`,
   `opp.m0: Sangan def`, `you.st0: Mirror Force set`, `you.gy: ...`, `you.extra: ...`) and the steps.
2. `python tools/media/gen_duel_media.py` writes `tools/media/<Name>_Media_Player.lua`.
3. Run it: `tools/testing_scenarios/run_tests.ps1 -Media -Names @('<Name>_Media_Player')`. The step log is
   `tools/testing_scenarios/recordings/_console.log`; the video is `screenshots/videos/<clip>.mp4` (ffmpeg must be on
   PATH, or no video is written).
4. Look at frames (`ffmpeg -i clip.mp4 -vf "fps=1,scale=400:-1,tile=6x4" -frames:v 1 sheet.png`), then delete the
   entry, its `.lua` and the video.

Steps worth knowing:

| Situation | Step |
|---|---|
| A Yes/No the card asks (Gravekeeper's Spy's "Special Summon a monster?") | `yes` / `no` right after the step that opens it |
| "Do you wish to activate a Quick-play Magic or Trap card?" after an attack (you have a Set Trap) | `no` once after the declaration and once after the battle, or the harness picks Yes and waits |
| A card list (Deck, Graveyard, Extra Deck) | `pick you.deck[Name]` |
| The summon position menu | `position atk` / `position def` |
| A hand card's Special Summon procedure | `open_menu you.hand[Name]`, then `menu sp_summon` (`special_summon` adds optional zone and position steps, which can get in the way of a procedure that picks on the field) |
| A slow chain | a second `settle` (each waits up to 15 seconds) |
| The CPU's turn | `unfreeze`, `pass`, `cpu_turn` |

A Fusion Monster has to be properly summoned before Monster Reborn can revive it, so in a probe bring one out with
Metamorphosis (Tribute a monster of its Level) or Polymerization. Card names with a comma ("Fusilier Dragon, the
Dual-Mode Beast") work in the board lists.

---

## 10. Checklist

1. Name in `config/new_cards.txt` and above `------` in `new_cards.txt` (or, for a working leftover effect, only
   `number | name` in `config/leftover_cards.txt`).
2. Code under `#ifdef HAS_CARD_<NAME>`: the row in `gAddedCardEffects`, handlers, hooks, CPU policy.
3. `make -j8 demo` (and `make release` now and then for the size limit). Try it in the emulator (Testing).
4. Remove the name from every `card_pools/missing*.txt`.
5. Docs: a section in `wiki/game/added-cards.md`, `wiki/log.md`, the card count in `README.md`, CPU behaviour in
   `AI_IMPROVEMENTS.md`; run `PYTHONUTF8=1 python3 scripts/wiki_lint.py`.
6. `NEW_CARDS.md`: after `make demo`, `python3 tools/gen_new_cards_md.py` rewrites `demo_cards.txt` and
   `NEW_CARDS.md` and lists the cards without a screenshot. Put those names in `cards/_capture_only.txt`, run
   `tools/card_screenshots.lua` in mGBA on `eds_demo.gba` (it reads the cards and their IDs from
   `build/new_cards.tsv`), delete the capture file and run the generator again.
