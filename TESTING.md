# Effect testing scenarios (high-usage demo cards)

Manual play-test checklist for **demo-build** cards in [`demo_cards.txt`](demo_cards.txt), **excluding** any name that also appears under the `******` line in [`new_cards.txt`](new_cards.txt) (those duplicates are tracked for the full list, not for this pass). Cards must appear in at least **10%** of decks in some Campaign format's usage statistics. Inclusion is read only from each file's top-level `total` and `extra` arrays in [`usage/`](usage/) (fraction of `decks_processed` that run the card, main plus extra), then multiplied by **3** for limited cards and **1.5** for semi-limited in that format's banlist (capped at **100%**). Each card appears **once**, under the **earliest** format (in [`tools/banlist.py`](tools/banlist.py) `BANLISTS` order from **Yugi-Kaiba** through **Reaper**) where it meets that threshold.

For each card: **Implementation** and **Coded behavior** come from this repo (`src/added_effects.c`, engine effect keys, [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md), and related files). **YGOPRODeck text** is loaded from [YGOPRODeck API v7](https://db.ygoprodeck.com/api/v7/cardinfo.php) (`name` query). If the build differs from TCG/OCG, scenarios still follow the code—differences are noted, not treated as bugs to fix here.

**103** cards qualify (of 295 in `demo_cards.txt` minus `new_cards.txt` below `******`; 13 names excluded). Regenerate with `python3 tools/gen_testing_md.py`.

## Yugi-Kaiba

### Card Destruction

- **Usage:** 100.0% weighted inclusion in `usage/yugi-kaiba.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CARD_DESTRUCTION`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Both players discard as many cards as possible from their hands, then each player draws the same number of cards they discarded." No prepare: it may be activated like Pot of Greed. Each hand is discarded through DiscardHandCard by index, with the hand compacted only after its last card (the indices stay valid until then), so the discard triggers run (Night Assailant; Electric Snake and Minar for the opponent's hand, discarded by an opponent's effect). Then the draws: a player whose deck runs out decks out as usual (player 0 cannot).
- **YGOPRODeck (TCG) text:** Both players discard as many cards as possible from their hands, then each player draws the same number of cards they discarded.
- **Scenarios:**
  1. Both players with uneven hands: activate **Card Destruction** and verify discard-then-draw counts for each side.

### Soul Exchange

- **Usage:** 91.1% weighted inclusion in `usage/yugi-kaiba.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SOUL_EXCHANGE`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Target 1 monster your opponent controls; this turn, if you Tribute a monster, you must Tribute that monster, as if you controlled it. You cannot conduct your Battle Phase the turn you activate this card." The engine's effect key 1220 (tools/newcards.py ENGINE_NUMBERS gives the card that number): prepare EffectCanTributeOpponentMonsterPrepare (Main Phase 1, a level 5+ monster to Tribute Summon or a Catapult Turtle-like monster), chainA EffectSkipBattlePhaseChainA, chainB EffectTributeOpponentMonsterChainB, resolve EffectTributeOpponentMonsterResolve (a Tribute Summon from the hand, or the Tribute of a Catapult Turtle, The Little Swordsman of Aile or Cannon Soldier). Two fixes in effect_resolve8.c: the summon paths Tributed the opponent's zone 0 (the packed target was passed as the player), and the steps are cursor UI only, so the CPU gets these:
- **YGOPRODeck (TCG) text:** Target 1 monster your opponent controls; this turn, if you Tribute a monster, you must Tribute that target, as if you controlled it. You cannot conduct your Battle Phase the turn you activate this card.
- **TCG vs this ROM:** Documented implementation quirk in this build (see **Coded behavior**); do not expect full modern TCG timing.
- **Scenarios:**
  1. **Yugi-Kaiba** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Soul Exchange**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.

## Imperial

### Thousand-Eyes Restrict

- **Usage:** 47.6% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THOUSAND_EYES_RESTRICT`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** "Relinquished" + "Thousand-Eyes Idol"
Other monsters on the field cannot change their battle positions or attack. Once per turn: You can target 1 monster your opponent controls; equip that target to this card (max. 1). This card's ATK/DEF become equal to that equipped monster's. If this card would be destroyed by battle, destroy that equipped monster instead.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Thousand-Eyes Restrict** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Equip **Thousand-Eyes Restrict** to a legal monster, change the number of face-up monsters or Spells/Traps on your field, and watch ATK/DEF updates.

### The Fiend Megacyber

- **Usage:** 45.2% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THE_FIEND_MEGACYBER`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** If your opponent controls at least 2 more monsters than you do, you can Special Summon this card (from your hand).
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Imperial**-rules duel, add **The Fiend Megacyber** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Goblin Attack Force

- **Usage:** 40.5% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1344. key 1344: attacking uses up effectUnused (BattleStage_DamageCalc), at the end of the Battle Phase a face-up Attack Position one turns to Defense Position (BattleStage_EndBattlePhase), and its controller's next turn start locks its position for that turn and re-arms it (DuelCmd_TurnStart)
- **YGOPRODeck (TCG) text:** If this card attacks, it is changed to Defense Position at the end of the Battle Phase, and its battle position cannot be changed until the End Phase of your next turn.
- **Scenarios:**
  1. In a **Imperial**-rules duel, add **Goblin Attack Force** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Solemn Wishes

- **Usage:** 28.6% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SOLEMN_WISHES`, `gAddedCardEffects`)
- **Coded behavior:** Continuous Trap (YGOPRODeck text): "Increase your Life Points by 500 points each time you draw a card (or cards)." Same behaviour as the vanilla effect key CARD_1305 (DrawCards in duel_card_actions.c): while a face-up copy is on your field, each DrawCards call shows the card and gains 500 LP per copy (one gain per draw action, not per card in the batch). Activation resolve is a no-op; the trap stays face-up. CPU: AiPlayAddedCards / default Set flip.
- **YGOPRODeck (TCG) text:** Increase your Life Points by 500 points each time you draw a card (or cards).
- **Scenarios:**
  1. Activate or trigger **Solemn Wishes** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.
  2. Set **Solemn Wishes** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Limiter Removal

- **Usage:** 21.4% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_LIMITER_REMOVAL`, `gAddedCardEffects`)
- **Coded behavior:** "Double the ATK of all Machine monsters you currently control, until the end of this turn. During the End Phase of this turn, destroy those monsters." Same engine pattern as key CARD_1314 (EffectDoubleMachineAtkResolve): a ZONE_LINK_CARD_EFFECT from this card's id on each qualifying monster; GetZoneCardStats doubles ATK for that link; EndPhase_LimiterRemoval destroys every monster that still holds the link on the activation turn. Machines you summon later this turn are not affected.
- **YGOPRODeck (TCG) text:** Double the ATK of all Machine monsters you currently control, until the end of this turn. During the End Phase of this turn, destroy those monsters.
- **Scenarios:**
  1. Summon monsters on both sides, activate **Limiter Removal** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.

### 4-Starred Ladybug of Doom

- **Usage:** 16.7% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_4_STARRED_LADYBUG_OF_DOOM`, `gAddedCardEffects`)
- **Coded behavior:** FLIP: "Destroy all Level 4 monsters your opponent controls." A resolve-only row, as Fiber Jar (card_detail.c HasFlipEffect lists it). Every face-up monster of the opponent (of Ladybug's controller) whose Level is 4 is destroyed, as Tribe-Infecting Virus destroys (DestroyFieldCard, OnCardDestroyedByEffect); a face-down monster has no Level, so it stays.
- **YGOPRODeck (TCG) text:** FLIP: Destroy all Level 4 monsters your opponent controls.
- **Scenarios:**
  1. Summon monsters on both sides, activate **4-Starred Ladybug of Doom** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.
  2. Set **4-Starred Ladybug of Doom** face-down, have the opponent attack or flip it, and test the FLIP effect once.

## Android

### Kycoo the Ghost Destroyer

- **Usage:** 100.0% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_KYCOO_THE_GHOST_DESTROYER`, `gAddedCardEffects`)
- **Coded behavior:** Reuses effect key 1511's handlers (EffectBanishOpponentGraveMonstersResolve, battle damage triggers in battle_phase2.c). While face up, it blocks graveyard banish effects the same way CARD_1511 does.
- **YGOPRODeck (TCG) text:** When this card inflicts battle damage to your opponent: You can target up to 2 monsters in their Graveyard; banish those targets. Your opponent cannot banish cards from either player's Graveyard.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Kycoo the Ghost Destroyer**, and confirm whether/how many opponent Graveyard cards are banished or removed.

### Bazoo the Soul-Eater

- **Usage:** 97.4% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BAZOO_THE_SOUL_EATER`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn: You can banish up to 3 monsters from your Graveyard; this card gains 300 ATK for each, until the end of your opponent's turn." The engine's effect key 1513 (ENGINE_NUMBERS gives the card that number): the card menu's ACTIVATE once per turn (effectUnused) with a monster in the player's Graveyard, EffectBanishGraveForAtkResolve (asks before each of up to 3 picks, then a ZONE_LINK_ATK_300_PER_VALUE link of the count), and DuelCmd_TurnStart re-arms it and drops the link at its controller's next turn. That resolve is UI only, so the CPU gets BazooCpuResolve (effect_resolve11.c calls it): its 3 weakest Graveyard monsters (fewer if it has fewer), then the same link and once-per-turn mark.
- **YGOPRODeck (TCG) text:** Once per turn: You can banish up to 3 monsters from your Graveyard; this card gains 300 ATK for each, until the end of your opponent's turn.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Bazoo the Soul-Eater**, and confirm whether/how many opponent Graveyard cards are banished or removed.

### Torrential Tribute

- **Usage:** 97.4% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TORRENTIAL_TRIBUTE`, `gAddedCardEffects`)
- **Coded behavior:** "When a monster(s) is Summoned: Destroy all monsters on the field." A Normal Trap answering the response window that a summon opens (SummonAction_Update, EventResponse_Run): event RESPONSE_SUMMONED, RESPONSE_FLIP_SUMMONED or RESPONSE_SPECIAL_SUMMONED (not RESPONSE_SET), with the summoned monster at loc0, like Trap Hole (EffectTrapHolePrepare) but for any monster and either player's summon. The effect is Dark Hole's (EffectDarkHoleResolve: every monster, the opponent's first).
- **YGOPRODeck (TCG) text:** When a monster(s) is Summoned: Destroy all monsters on the field.
- **Scenarios:**
  1. Summon monsters on both sides, activate **Torrential Tribute** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.
  2. Set **Torrential Tribute** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Jar of Greed

- **Usage:** 92.1% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_JAR_OF_GREED`, `gAddedCardEffects`)
- **Coded behavior:** Normal Trap: "Draw 1 card." A row with no prepare or check, like Gift of the Mystical Elf's: CanActivateEffect lets a Set Jar of Greed be activated in any response window, at Trap speed.
- **YGOPRODeck (TCG) text:** Draw 1 card.
- **Scenarios:**
  1. Activate or trigger **Jar of Greed** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.

### Skull Lair

- **Usage:** 89.5% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SKULL_LAIR`, `gAddedCardEffects`)
- **Coded behavior:** Continuous Trap: "Remove from play any number of Monster Cards in your Graveyard to destroy 1 face-up monster on the field whose Level is equal to the number of the cards you removed from play." Like Ultimate Offering it can be activated again while face up (card_command_menu.c, effect_activation.c, effect_targets4.c list it), each time: check    a face-up monster (either side) the card may target, of Level 1 to the monsters in the player's Graveyard (it is the target filter of the pick too) prepare  such a monster exists chainB   the target: EffectTargetableFaceUpMonsterChainB; the CPU's pick (the human's strongest) is replaced by its best legal one (SkullLairBestTarget: the human's strongest such monster) resolve  the target still face up and still legal: as many Graveyard monsters as its Level are banished (the human picks each one in the Graveyard list, as Bazoo's; the CPU banishes its weakest), then it is destroyed.
- **YGOPRODeck (TCG) text:** Remove from play any number of Monster Cards in your Graveyard to destroy 1 face-up monster on the field whose Level is equal to the number of the cards you removed from play.
- **Scenarios:**
  1. Give **Skull Lair** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Magic Cylinder

- **Usage:** 78.9% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1214. key 1214: EffectAttackResponsePrepare / Resolve (negate the attack, the attacker's player loses its ATK); the CPU's trap logic knows it (ai_summon.c)
- **YGOPRODeck (TCG) text:** When an opponent's monster declares an attack: Target the attacking monster; negate the attack, and if you do, inflict damage to your opponent equal to its ATK.
- **Scenarios:**
  1. Declare an attack against you with a high-ATK monster, chain **Magic Cylinder** or equivalent, and confirm the attack stops and LP changes match the attacker's ATK.

### St. Joan

- **Usage:** 31.6% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1445**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** Fusion of 1444 + 1446. No effect code. Status in ROM: No effect code.
- **YGOPRODeck (TCG) text:** "The Forgiving Maiden" + "Darklord Marie"
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Android**-rules duel, add **St. Joan** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### United We Stand

- **Usage:** 23.7% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1449. key 1449: an Equip (EffectEquipResolve and the equip target rules), +800 ATK/DEF per face-up monster of the equip card's controller (CalcZoneCardStats)
- **YGOPRODeck (TCG) text:** The equipped monster gains 800 ATK/DEF for each face-up monster you control.
- **Scenarios:**
  1. Equip **United We Stand** to a legal monster, change the number of face-up monsters or Spells/Traps on your field, and watch ATK/DEF updates.

### Mage Power

- **Usage:** 15.8% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1450. key 1450: an Equip, +500 ATK/DEF per Spell/Trap of the equip card's controller, itself included (CalcZoneCardStats; changed to count the Field Spell too)
- **YGOPRODeck (TCG) text:** The equipped monster gains 500 ATK and DEF for each Spell/Trap Card you control.
- **Scenarios:**
  1. Equip **Mage Power** to a legal monster, change the number of face-up monsters or Spells/Traps on your field, and watch ATK/DEF updates.

### Zombyra the Dark

- **Usage:** 15.8% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_ZOMBYRA_THE_DARK`, `gAddedCardEffects`)
- **Coded behavior:** "This card cannot attack directly. Each time this card destroys a monster by battle, it loses 200 ATK." CanMonsterAttack (card_menu_input.c; the CPU's attacker list uses it too)   it may not attack while the opponent has no monster (that is a direct attack) BattleStage_DestroyMonsters (battle_phase2.c, AddedCardsAfterBattle)        when it survives a battle that destroyed the other monster, a self link ZONE_LINK_ATK_DOWN_200 (from its card ID; AddZoneLink raises the count of an existing one) takes 200 ATK more The link goes with the card when it leaves the field.
- **YGOPRODeck (TCG) text:** This card cannot attack your opponent directly. If this card destroys a monster by battle: This card loses 200 ATK.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Zombyra the Dark** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Humanoid Worm Drake

- **Usage:** 10.5% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1404**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** Fusion of 1403 + 1402. No effect code. Status in ROM: No effect code.
- **YGOPRODeck (TCG) text:** "Worm Drake" + "Humanoid Slime"
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Android**-rules duel, add **Humanoid Worm Drake** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

## Joey-Pegasus

### Scapegoat

- **Usage:** 36.8% weighted inclusion in `usage/joey-pegasus.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SCAPEGOAT`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** Special Summon 4 "Sheep Tokens" (Beast/EARTH/Level 1/ATK 0/DEF 0) in Defense Position. They cannot be Tributed for a Tribute Summon. You cannot Summon other monsters the turn you activate this card (but you can Normal Set).
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. **Joey-Pegasus** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Scapegoat**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. With four open zones and no prior summon, activate **Scapegoat** (or similar), confirm four tokens and that further summons are blocked for the turn.

### Offerings to the Doomed

- **Usage:** 10.5% weighted inclusion in `usage/joey-pegasus.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_OFFERINGS_TO_THE_DOOMED`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** Target 1 face-up monster on the field; destroy that target, also skip your next Draw Phase.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Offerings to the Doomed** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Activate or trigger **Offerings to the Doomed** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.

## Fiber

### Exiled Force

- **Usage:** 100.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_EXILED_FORCE`, `gAddedCardEffects`)
- **Coded behavior:** "You can Tribute this card, then target 1 monster on the field; destroy that target." The card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no once-per-turn limit: the card is gone once used. The target is any other monster (Man-Eater Bug's EffectTargetableMonsterChainB, with a wrapper that refuses Exiled Force itself). The Tribute is paid when the target is chosen rather than in chainA: the target step calls CanActivateEffect, and the prepare handler fails once the card has left its zone. The destruction is EffectDestroyTargetResolve's.
- **YGOPRODeck (TCG) text:** You can Tribute this card to target 1 monster on the field; destroy that target.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. **Fiber** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Exiled Force**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. Summon monsters on both sides, activate **Exiled Force** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.

### Fiber Jar

- **Usage:** 100.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_FIBER_JAR`, `gAddedCardEffects`)
- **Coded behavior:** FLIP: "Each player shuffles all cards from their hand, field, and Graveyard into the Deck, then draws 5 cards." (card_detail.c HasFlipEffect lists it.) For each player, all queued (deferred) commands: field     ReturnFieldCardToDeck on every zone, the Spell/Trap and Field zones first (an equip goes back before its monster's DestroyLinkedCards could send it to the Graveyard; DuelCmd_ReturnToDeck skips an emptied zone). Tokens are destroyed, as ReturnFieldCardToDeck does. hand      DUEL_CMD_RETURN_HAND_CARD_TO_DECK, last card first (the indices stay valid) Graveyard its Fusion Monsters to the Fusion Deck at once (TakeGraveyardCardAt, AddCardToFusionDeck), then DUEL_CMD_RETURN_GRAVEYARD_TO_DECK (the rest, with anything the field sent there; it would put a Fusion Monster in the Deck). A field Fusion Monster goes to the Fusion Deck (DuelCmd_ReturnToDeck). then each Deck is shuffled and each player draws 5 (DrawCards). Fiber Jar itself goes back with the field.
- **YGOPRODeck (TCG) text:** FLIP: Each player shuffles all cards from their hand, field, and Graveyard into the Deck, then draws 5 cards.
- **Scenarios:**
  1. Summon monsters on both sides, activate **Fiber Jar** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.
  2. Activate or trigger **Fiber Jar** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.
  3. With four open zones and no prior summon, activate **Scapegoat** (or similar), confirm four tokens and that further summons are blocked for the turn.

### Injection Fairy Lily

- **Usage:** 100.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_INJECTION_FAIRY_LILY`, `gAddedCardEffects`)
- **Coded behavior:** "During either player's Damage Step, when this card attacks or is attacked: You can pay 2000 LP; this card gains 3000 ATK during the Damage Step only." EDS offers no face-up monster in a response window (only Set Spell/Trap cards and the hand), so, like D.D. Warrior Lady, the effect lives in the battle code: BattleStage_DamageCalc, DAMAGE_STEP_MARK_ATTACKER   InjectionFairyLilyResetBattle BattleStage_DamageCalc, DAMAGE_STEP_CALC            InjectionFairyLilyDamageStep, before CalcBattle: the attacker, then the target, when it is a face-up Lily whose controller has more than 2000 LP. The human is asked (PROMPT_CONFIRM_CARD_EFFECT); the CPU pays when the 3000 ATK turn the battle (or for a direct attack, with more than 4000 LP). Paying queues DUEL_CMD_LOSE_LP (a cost) and a ZONE_LINK_ATK_BONUS link of 3000; the stage waits a frame so that CalcBattle sees it. BattleStage_EndAttack                               InjectionFairyLilyEndAttack: the 3000 link goes again. The state is gInjectionFairyLily (EWRAM 0x0202FF00, symbols.ld).
- **YGOPRODeck (TCG) text:** If this card attacks or is attacked, during damage calculation (in either player's turn): You can pay 2000 LP once per battle; this card gains 3000 ATK during that damage calculation only.
- **Scenarios:**
  1. In a **Fiber**-rules duel, add **Injection Fairy Lily** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Yata-Garasu

- **Usage:** 100.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_YATA_GARASU`, `gAddedCardEffects`)
- **Coded behavior:** "This card cannot be Special Summoned. This card returns to its owner's hand during the End Phase of the turn it is Normal Summoned or flipped face-up. When this card inflicts Battle Damage to your opponent, they skip their next Draw Phase." IsCannotSpecialSummon lists it (QueueSpecialSummon* refuse it; CanReviveGraveyardCard's IsSpecialSummonOnly path does not apply, but the summon itself is refused). EndPhase_ReturnSpiritMonsters (DuelPhase_End, battle_phase3.c; above): every face-up Yata-Garasu on the field goes back to its owner's hand. A face-up one at an End Phase was Normal Summoned or flipped face up that turn (it always leaves at the End Phase, and it cannot be Special Summoned), so no record of the turn is needed. Battle damage (BattleStage_InflictDamage steps 0 and 2, battle_phase2.c): DUEL_CMD_SKIP_NEXT_DRAW_PHASE (Time Seal's command) for the damaged player.
- **YGOPRODeck (TCG) text:** This card cannot be Special Summoned. This card returns to its owner's hand during the End Phase of the turn it is Normal Summoned or flipped face-up. When this card inflicts Battle Damage to your opponent, they skip their next Draw Phase.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Yata-Garasu**, and confirm whether/how many opponent Graveyard cards are banished or removed.
  2. Activate or trigger **Yata-Garasu** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.
  3. Set **Yata-Garasu** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Creature Swap

- **Usage:** 73.4% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CREATURE_SWAP`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Each player chooses 1 monster they control and switches control of those monsters with each other. Those monsters cannot change their battle positions for the rest of this turn." Nothing is targeted: the choices are made at resolution. The human picks one of their monsters with the cursor (as in EffectGainOpponentMonsterStatsResolve), the CPU gives its weakest (AiFindWeakestMonster). The swap is Invader of the Throne's (SwapFieldCards: DUEL_CMD_SWAP_ZONES and the control-change rules of the swapped cards), then both monsters are locked in their position (DUEL_CMD_SET_POSITION_LOCKED; DuelCmd_TurnStart clears the flag at the start of their controller's next turn).
- **YGOPRODeck (TCG) text:** Each player chooses 1 monster they control and switches control of those monsters with each other. Those monsters cannot change their battle positions for the rest of this turn.
- **Scenarios:**
  1. In a **Fiber**-rules duel, add **Creature Swap** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Asura Priest

- **Usage:** 72.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_ASURA_PRIEST`, `gAddedCardEffects`)
- **Coded behavior:** "This card can attack all monsters your opponent controls once each." (Its Spirit rule and "Cannot be Special Summoned" are with Yata-Garasu's: EndPhase_ReturnSpiritMonsters, IsCannotSpecialSummon.) record   for the Battle Phase of turn gModDuel.asuraTurn - 1 (set, and the record emptied, as each Battle Phase starts: ModBattlePhaseStart): per zone of the attacking player, the monsters it attacked, as the opponent's zone (asuraTargets, a bit per zone) and that monster's DuelZone.serial there (asuraSerials; gBattle.zoneSerialSnapshot holds the target's at the declaration). The serial is stamped when a card is placed and kept when it is flipped, so a monster that came into the zone later (recruited by Mystic Tomato, Special Summoned, ...) is a new one it may attack again    ModEndAttack: after an attack on a monster (not a direct one) by a face-up Asura Priest still in its zone, the target is recorded, and while the opponent controls a monster it has not attacked, its attack mark (attackedMask) is cleared so it may attack again (as Black Luster Soldier's second attack does) targets  ModAttackTargetBlocked: the human's target pick (BattleStage_SelectTarget, card_menu_input.c) and the CPU's attack plans (AiFindAttackTarget, ai_picks.c) refuse a monster it has attacked; with the mark still set after its last monster, it cannot follow with a direct attack, and after a direct attack it is done
- **YGOPRODeck (TCG) text:** Cannot be Special Summoned. During the End Phase of the turn this card is Normal Summoned or flipped face-up: Return it to the hand. This card can attack all monsters your opponent controls once each.
- **Scenarios:**
  1. Set **Asura Priest** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Spear Dragon

- **Usage:** 25.3% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SPEAR_DRAGON`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** During battle between this attacking card and a Defense Position monster whose DEF is lower than the ATK of this card, inflict the difference as Battle Damage to your opponent. If this card attacks, it is changed to Defense Position at the end of the Damage Step.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Fiber**-rules duel, add **Spear Dragon** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Dark Ruler Ha Des

- **Usage:** 15.6% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_DARK_RULER_HA_DES`, `gAddedCardEffects`)
- **Coded behavior:** "Cannot be Special Summoned from the GY. Negate the effects of monsters destroyed by battle with Fiend monsters you control." CanReviveGraveyardCard (duel_ritual.c) refuses it: the revival cards cannot pick it. A monster destroyed in battle by a Fiend whose controller has a face-up Ha Des (HaDesNegatesBattleDestroyed): SendBattleDestroyedCardToGraveyard (duel_field_moves.c) queues none of its "destroyed in battle" effects (Sangan, Witch of the Black Forest, Mystic Tomato, ...), and a face-down one's Flip effect is not activated (HaDesNegatesBattleFlipEffect, beside Blade Knight's in battle_phase1.c / battle_phase2.c).
- **YGOPRODeck (TCG) text:** Cannot be Special Summoned from the GY. Negate the effects of monsters destroyed by battle with Fiend monsters you control.
- **Scenarios:**
  1. Give **Dark Ruler Ha Des** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Set **Dark Ruler Ha Des** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Airknight Parshath

- **Usage:** 14.5% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_AIRKNIGHT_PARSHATH`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** If this card attacks a Defense Position monster, inflict piercing battle damage. If this card inflicts battle damage to your opponent: Draw 1 card.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Activate or trigger **Airknight Parshath** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.

## Yata

### Mirage of Nightmare

- **Usage:** 100.0% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MIRAGE_OF_NIGHTMARE`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn, during your opponent's Standby Phase: Draw cards until you have 4 cards in your hand. If you do, during your next Standby Phase after that: Randomly discard the same number of cards you drew (or your entire hand, if you do not have enough cards)." (Continuous Spell) Its activation does nothing (EffectMirageOfNightmareResolve; a Continuous Spell stays on the field). The rest runs in ModStandbyStep, in every Standby Phase (`player` is the turn player), on gDuel.phaseSubStep: discard  the turn player's draws from their last Mirage draw (gModDuel.mirageDrawn) are discarded at random, one per step, while a face-up Mirage of Nightmare is still on their field (destroyed in between, there is nothing to discard); the count is cleared either way draw     the other player, with a face-up Mirage of Nightmare and fewer than 4 cards in hand, draws up to 4 (mandatory, nobody is asked); the number drawn is kept for their next Standby Phase
- **YGOPRODeck (TCG) text:** Once per turn, during your opponent's Standby Phase: Draw cards until you have 4 cards in your hand. If you do, during your next Standby Phase after that: Randomly discard the same number of cards you drew (or your entire hand, if you do not have enough cards).
- **Scenarios:**
  1. Summon monsters on both sides, activate **Mirage of Nightmare** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.
  2. Both players with uneven hands: activate **Mirage of Nightmare** and verify discard-then-draw counts for each side.

### Spirit Reaper

- **Usage:** 87.2% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SPIRIT_REAPER`, `gAddedCardEffects`)
- **Coded behavior:** "Cannot be destroyed by battle. After this card is targeted by an effect, destroy this card. When this card inflicts battle damage to your opponent: Discard 1 random card from their hand." CalcBattle (campaign.c)                   its destroyed flag is cleared, as attacker or target TryAddEffectTarget (effect_targets1.c)     OnFieldCardTargeted: a face-up Spirit Reaper that an effect targets is destroyed at once (its own effect, so the targeting effect finds it gone) BattleStage_InflictDamage (battle_phase2.c) White Magical Hat's case: the opponent discards 1 random card TryAddEffectTarget also records the picks of a few effects that Tribute rather than target (Catapult Turtle's EffectTributeTargetChainB and Metamorphosis); those do not count.
- **YGOPRODeck (TCG) text:** Cannot be destroyed by battle. After resolving a card effect that targets this face-up card, destroy this card. When this card inflicts battle damage to your opponent by a direct attack: Discard 1 random card from their hand.
- **Scenarios:**
  1. **Yata** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Spirit Reaper**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. Give **Spirit Reaper** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Don Zaloog

- **Usage:** 74.4% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_DON_ZALOOG`, `gAddedCardEffects`)
- **Coded behavior:** "When this card inflicts battle damage to your opponent: You can activate 1 of these effects. - Discard 1 random card from your opponent's hand. - Send the top 2 cards of your opponent's Deck to the GY." BattleStage_InflictDamage (battle_phase2.c) queues its battle-damage trigger like Kycoo's: as the attacker (RESPONSE_BATTLE_DAMAGE) and as the target when the attacker's player takes the damage (RESPONSE_BATTLE_DEFLECTED_DAMAGE). The low nibble of loc1 is the damaged player. Pending triggers are not checked by a prepare handler, so the resolve checks everything and asks: the human gets Yes/No for the discard, then for the Deck; the CPU discards if it can, else sends the Deck cards. The discard is White Magical Hat's (DuelPrompt_PostRandomDiscard, by an opponent's effect); the Deck cards go one by one (DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD, the top card is deck[0]).
- **YGOPRODeck (TCG) text:** When this card inflicts battle damage to your opponent: You can activate 1 of these effects;
● Discard 1 random card from their hand.
● Send the top 2 cards of their Deck to the GY.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Don Zaloog**, and confirm whether/how many opponent Graveyard cards are banished or removed.

### Reinforcement of the Army

- **Usage:** 73.1% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_REINFORCEMENT_OF_THE_ARMY`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Add 1 Level 4 or lower Warrior monster from your Deck to your hand." Sangan's resolve (EffectSanganResolve) does the search: it lists CollectEffectTargets (effect_target_collect.c: IsReinforcementTarget) for the card number, lets the human pick (the CPU: AiPickCardListEntry), adds the card and shuffles. The prepare handler needs such a monster in the deck, so the Spell is not wasted.
- **YGOPRODeck (TCG) text:** Add 1 Level 4 or lower Warrior monster from your Deck to your hand.
- **Scenarios:**
  1. In a **Yata**-rules duel, add **Reinforcement of the Army** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Gravekeeper's Spy

- **Usage:** 60.3% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_GRAVEKEEPERS_SPY`, `gAddedCardEffects`)
- **Coded behavior:** "FLIP: Special Summon 1 'Gravekeeper's' monster with 1500 or less ATK from your Deck." A flip effect (HasFlipEffect, card_detail.c), resolved by the floaters' handler EffectSpecialSummonFromDeckResolve: it asks the human, lists the candidates (CollectEffectTargets, effect_target_collect.c: deck "Gravekeeper's" monsters with ATK 1500 or less, not Fusion or Ritual), Special Summons the pick in Attack Position and shuffles.
- **YGOPRODeck (TCG) text:** FLIP: Special Summon 1 "Gravekeeper's" monster with 1500 or less ATK from your Deck.
- **Scenarios:**
  1. Set **Gravekeeper's Spy** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Book of Moon

- **Usage:** 34.6% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BOOK_OF_MOON`, `gAddedCardEffects`)
- **Coded behavior:** Book of Moon (Quick-Play Spell) and Tsukuyomi (FLIP) share the same effect: target 1 face-up monster on the field; change it to face-down Defense Position. Targeting uses EffectTargetableFaceUpMonsterCheck / ChainB (Chaos Sorcerer's pickers). The resolve first destroys the target's Equip Cards (they cannot equip a face-down monster; a destroyed Premature Burial takes the monster with it). Then DUEL_CMD_CHANGE_POSITION puts a monster in Attack Position into Defense Position, and DUEL_CMD_FLIP_CARD turns it face down, which removes its counters (OnMonsterTurnedFaceDown). Tsukuyomi is listed in HasFlipEffect (card_detail.c).
- **YGOPRODeck (TCG) text:** Target 1 face-up monster on the field; change that target to face-down Defense Position.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Book of Moon** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Set **Book of Moon** face-down, have the opponent attack or flip it, and test the FLIP effect once.
  3. Equip **Book of Moon** to a legal monster, change the number of face-up monsters or Spells/Traps on your field, and watch ATK/DEF updates.

### King Tiger Wanghu

- **Usage:** 34.6% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_KING_TIGER_WANGHU`, `gAddedCardEffects`)
- **Coded behavior:** "When a monster with 1400 or less ATK is Normal or Special Summoned: Destroy it." ExecuteSummonAction (summon_action.c) asks KingTigerWanghuOnSummon after a Normal Summon or a Special Summon, before the opponent's response window: a face-up summoned monster (a Set or face-down one shows no ATK) with 1400 or less ATK is destroyed while a face-up, non-negated King Tiger Wanghu is on either side of the field. Flip Summons do not count.
- **YGOPRODeck (TCG) text:** When a monster(s) with 1400 or less ATK is Normal Summoned or Special Summoned: Destroy those monsters with 1400 or less ATK. This card must be face-up on the field to activate and to resolve this effect.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **King Tiger Wanghu** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Set **King Tiger Wanghu** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Newdoria

- **Usage:** 26.9% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_NEWDORIA`, `gAddedCardEffects`)
- **Coded behavior:** "If this card is destroyed by battle and sent to the GY: Target 1 monster on the field; destroy that target." Queued with Sangan's and Gigantes' triggers (SendBattleDestroyedCardToGraveyard, duel_field_moves.c: destroyed by battle and sent to the Graveyard, not banished). The target is any monster on the field (Man-Eater Bug's EffectTargetableMonsterCheck / ChainB; the CPU picks the human's strongest), destroyed by EffectDestroyTargetResolve.
- **YGOPRODeck (TCG) text:** If this card is destroyed by battle and sent to the GY: Target 1 monster on the field; destroy that target.
- **Scenarios:**
  1. Give **Newdoria** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Metamorphosis

- **Usage:** 15.4% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_METAMORPHOSIS`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Tribute 1 monster. Special Summon 1 Fusion Monster from your Extra Deck with the same Level as the Tributed monster." Prepare: one of the player's monsters has the Level of a Fusion Monster in the Fusion Deck. ChainB picks it with Catapult Turtle's EffectTributeTargetChainB (the CPU: its highest-Level such monster), refuses a monster with no such Fusion, then Tributes it (the cost, paid once the pick is made, as Exiled Force's) and keeps its Level in targets[1]. Resolve: Cyber-Stein's steps with a list of the Fusion Monsters of that Level (CollectEffectTargets, effect_target_collect.c: IsMetamorphosisFusion with param = the Level); the CPU takes the highest ATK.
- **YGOPRODeck (TCG) text:** Tribute 1 monster. Special Summon 1 Fusion Monster from your Extra Deck with the same Level as the Tributed monster.
- **Scenarios:**
  1. **Yata** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Metamorphosis**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.

### Cure Mermaid

- **Usage:** 12.8% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1441**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** +800 LP in each of your Standby Phases. Status in ROM: Full (passive).
- **YGOPRODeck (TCG) text:** As long as this card remains face-up on your side of the field, increase your Life Points by 800 points during each of your Standby Phases.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Yata**-rules duel, add **Cure Mermaid** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Raigeki Break

- **Usage:** 11.5% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_RAIGEKI_BREAK`, `gAddedCardEffects`)
- **Coded behavior:** "Discard 1 card, then target 1 card on the field; destroy it." Normal Trap from the field (or hand): discard cost like Tribe-Infecting Virus / Magic Jammer (DuelPrompt_PostDiscardCost), target any card on either side (monsters and Spell/Trap zones, CanCardTargetZone), destroy with EffectDestroyTargetResolve.
- **YGOPRODeck (TCG) text:** Discard 1 card, then target 1 card on the field; destroy it.
- **Scenarios:**
  1. Give **Raigeki Break** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Trap Dustshoot

- **Usage:** 11.5% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TRAP_DUSTSHOOT`, `gAddedCardEffects`)
- **Coded behavior:** "Activate only if your opponent has 4 or more cards in their hand. Look at your opponent's hand, select 1 Monster Card in it, and return that card to its owner's Deck." The pick is Confiscation's (PROMPT_PICK_OPPONENT_HAND_CARD: the opponent's hand is revealed and the cursor picks a card); a card that is not a monster plays the error sound and the prompt runs again. The CPU takes the first monster in the human's hand. The card goes to the bottom of the Deck (ReturnHandCardToDeck), which is then shuffled. With no monster in the opponent's hand nothing happens.
- **YGOPRODeck (TCG) text:** Activate only if your opponent has 4 or more cards in their hand. Look at your opponent's hand, select 1 Monster Card in it, and return that card to its owner's Deck.
- **Scenarios:**
  1. In a **Yata**-rules duel, add **Trap Dustshoot** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Blast with Chain

- **Usage:** 10.3% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BLAST_WITH_CHAIN`, `gAddedCardEffects`)
- **Coded behavior:** "Target 1 face-up monster you control; equip this card to that target. It gains 500 ATK. If this card is destroyed by a card effect while equipped: Target 1 card on the field; destroy that target." An Equip Trap, as Kunai with Chain (tools/newcards.py's SUBTYPE_OVERRIDES: subtype Equip, so it stays on the field, Chain_CardGoesToGrave). One row, told apart by the event: activation   the target is a face-up monster of its controller (the human's cursor; the CPU takes the attacked monster when it answers an attack on one of its own, else its strongest face-up one); the resolve equips it (EquipCard) as Kunai's does, or destroys the card when the target is gone. +500 ATK: CalcZoneCardStats' Equip case (card_stats.c), beside Kunai's. destroyed    OnCardDestroyedByEffect (effect_hooks.c) calls BlastWithChainDestroyed for every card an effect destroys: a face-up Blast with Chain that equips a monster (FindMonsterWithLinkTo) queues its trigger from the Graveyard (RESPONSE_TRAP_TO_GRAVE). An Equip lost with its monster is not destroyed by an effect, so it does not go off. The target is any card on the field (the CPU: the human's strongest monster, else a Spell/Trap of the human's); it is destroyed. The CPU activates it in answer to an attack while it has a face-up monster (AiShouldActivateAddedCard).
- **YGOPRODeck (TCG) text:** Target 1 face-up monster you control; equip this card to that target. It gains 500 ATK. If this card is destroyed by a card effect while equipped: Target 1 card on the field; destroy that target.
- **Scenarios:**
  1. Summon monsters on both sides, activate **Blast with Chain** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.
  2. Equip **Blast with Chain** to a legal monster, change the number of face-up monsters or Spells/Traps on your field, and watch ATK/DEF updates.

### Gearfried the Iron Knight

- **Usage:** 10.3% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1351. key 1351: EquipCard destroys an Equip put on it (and the monster too for Premature Burial); Snatch Steal cannot target it
- **YGOPRODeck (TCG) text:** If either player equips an Equip Card(s) to this card: Destroy that Equip Card(s).
- **Scenarios:**
  1. Give **Gearfried the Iron Knight** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Equip **Gearfried the Iron Knight** to a legal monster, change the number of face-up monsters or Spells/Traps on your field, and watch ATK/DEF updates.

## Scientist

### Breaker the Magical Warrior

- **Usage:** 100.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR`, `gAddedCardEffects`)
- **Coded behavior:** "When this card is Normal Summoned: Place 1 Spell Counter on it (max. 1). Gains 300 ATK for each Spell Counter on it. You can remove 1 Spell Counter from this card; target 1 Spell/Trap on the field; destroy that target." One row serves both effects; the link's event tells them apart: RESPONSE_SUMMONED   the trigger that ExecuteSummonAction queues when Breaker is Normal Summoned (loc0 is its own zone): place the counter RESPONSE_NONE       the activated effect from the card menu (Main Phase, CanActivateMonsterEffect): needs a counter; the cost removes it, then it destroys 1 Spell/Trap like Mystical Space Typhoon (EffectAnySpellTrapCheck, EffectSpellTrapTargetChainB, EffectDestroyTargetResolve)
- **YGOPRODeck (TCG) text:** If this card is Normal Summoned: Place 1 Spell Counter on it (max. 1). Gains 300 ATK for each Spell Counter on it. You can remove 1 Spell Counter from this card, then target 1 Spell/Trap on the field; destroy that target.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Breaker the Magical Warrior** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Magical Scientist

- **Usage:** 100.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MAGICAL_SCIENTIST`, `gAddedCardEffects`)
- **Coded behavior:** "You can pay 1000 Life Points; Special Summon 1 Level 6 or lower Fusion Monster from your Extra Deck in face-up Attack or Defense Position. That Fusion Monster cannot attack your opponent directly, and is returned to the Extra Deck at the end of this turn." The card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no limit per turn, as Cyber-Stein's: prepare  face up on the field, 1000 LP, a free monster zone and such a Fusion Monster in the Fusion Deck chainA   1000 LP (DUEL_CMD_LOSE_LP, a cost) resolve  Metamorphosis' steps with a list of the Fusion Deck's Level 6 or lower Fusion Monsters (ModCollectEffectTargets: IsScientistFusion; the CPU takes the highest ATK), Special Summoned face up in the position the player picks (QueueSpecialSummonChoosePosition, which takes FindFreeMonsterZone's zone). The zone and card ID go to gModDuel.scientistReturn, as Return from the Different Dimension marks its own. DuelPhase_End (battle_phase3.c)  EndPhase_ReturnMagicalScientist returns each marked monster still in its zone (ReturnFieldCardToHand: a Fusion Monster goes to the Fusion Deck), then clears the marks. No direct attack  CanMonsterAttack (card_menu_input.c) asks ModCannotAttackDirectly, which refuses a marked monster's attack while the opponent has no monster, as for Zombyra the Dark. The CPU: AiUseMagicalScientist, below AiSpecialSummonCyberDragon.
- **YGOPRODeck (TCG) text:** Pay 1000 Life Points to Special Summon 1 level 6 or lower Fusion Monster from your Extra Deck in face-up Attack or Defense Position. That Fusion Monster cannot attack your opponent's Life Points directly, and is returned to your Extra Deck at the end of the turn.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Scientist**-rules duel, add **Magical Scientist** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Tribe-Infecting Virus

- **Usage:** 100.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TRIBE_INFECTING_VIRUS`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn: You can discard 1 card, then declare 1 Monster Type; destroy all face-up monsters of the declared Type on the field." An activated effect from the card menu in the Main Phase (CanActivateMonsterEffect), once per turn through the zone's effectUnused flag (DuelCmd_TurnStart sets it again). The pieces are the game's own: the discard cost of Magic Jammer (DuelPrompt_PostDiscardCost), the type prompt of DNA Surgery (EffectDeclareTypeChainB: targets[0] is the enum CardType) and the destruction of Eradicating Aerosol (DestroyFieldCard + OnCardDestroyedByEffect, the opponent's monsters first). The Virus does not target, so face-up is the only condition.
- **YGOPRODeck (TCG) text:** Discard 1 card from your hand and declare 1 Type of monster. Destroy all face-up monsters of the declared Type on the field.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Summon monsters on both sides, activate **Tribe-Infecting Virus** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.
  2. Declare an allowed type with **Tribe-Infecting Virus**, pay the cost, and confirm opponent monsters of that type are destroyed while yours remain.

### Gilasaurus

- **Usage:** 33.3% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_GILASAURUS`, `gAddedCardEffects`)
- **Coded behavior:** "You can Special Summon this card (from your hand). If Summoned this way: Activate this effect; your opponent can Special Summon 1 monster from their GY." Engine key 1520 (tools/newcards.py ENGINE_NUMBERS): the hand menu adds the Special Summon commands (card_command_menu.c), and the summon steps (effect_hooks.c) queue QueueSpecialSummonFromHand and, while the opponent has a monster to revive, a free zone and may Special Summon, a RESPONSE_SPECIAL_SUMMONED trigger. Its resolve, EffectOpponentGraveSummonResolve (effect_resolve11.c), asks the opponent (PROMPT_CONFIRM_GRAVEYARD_SUMMON; the CPU says Yes), who picks a monster from the Call of the Haunted list (the CPU: AiPickCardListEntry) and its position. The CPU's own Special Summon is in AiSpecialSummonCyberDragon below (AiGilasaurusSpecialSummon).
- **YGOPRODeck (TCG) text:** You can Special Summon this card (from your hand). If Summoned this way: Activate this effect; your opponent can Special Summon 1 monster from their GY.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Gilasaurus**, and confirm whether/how many opponent Graveyard cards are banished or removed.

### Poison of the Old Man

- **Usage:** 25.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_POISON_OF_THE_OLD_MAN`, `gAddedCardEffects`)
- **Coded behavior:** Quick-Play Spell: "Activate 1 of these effects; ● Gain 1200 LP. ● Inflict 800 damage to your opponent." chainA   the choice, kept in the link's unused unk12 as Enemy Controller's: the human picks from a two-line menu (the handler waits while the box is open: Chain_Build calls it every frame); the CPU decides (AiPoisonOfTheOldManChoice) resolve  GainLifePoints (1200) for the player, or LoseLifePoints (800) for the opponent CPU      AiPoisonOfTheOldManChoice: the damage when it wins the duel, the LP while it has POTOM_AI_LOW_LP or less, else the damage; AiWantsSpell plays it whenever it may (AiPlayAddedCards, Main Phase)
- **YGOPRODeck (TCG) text:** Activate 1 of these effects;
● Gain 1200 LP.
● Inflict 800 damage to your opponent.
- **Scenarios:**
  1. In a **Scientist**-rules duel, add **Poison of the Old Man** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Reasoning

- **Usage:** 25.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_REASONING`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Your opponent declares a monster Level from 1 to 12. Excavate cards from the top of your Deck until you excavate a monster that can be Normal Summoned/Set, then, if that monster is the same Level as the one declared by your opponent, send all excavated cards to the GY. If not, Special Summon the excavated monster, also send the remaining cards to the GY." The excavation is Monster Gate's (IsGateMonster, so this needs Monster Gate listed too). The declaration: the human declares (the CPU activated it): a text box with a Level menu (LevelMenu_Draw / LevelMenu_HandleInput, made like the Monster Type menu of DuelPrompt_SelectType: LEFT/RIGHT, A), starting at Level 4; the CPU declares (the human activated it): Level 4, the most common Level (it cannot see the Deck), shown in a text box. The Level is kept in gChain.effectSubStep. The monster leaves the Deck, and the next step Special Summons it face up in the position the player picks.
- **YGOPRODeck (TCG) text:** Your opponent declares a monster Level from 1 to 12. Excavate cards from the top of your Deck until you excavate a monster that can be Normal Summoned/Set, then, if that monster is the same Level as the one declared by your opponent, send all excavated cards to the GY. If not, Special Summon the excavated monster, also send the remaining cards to the GY.
- **Scenarios:**
  1. Activate or trigger **Reasoning** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.

### Jowgen the Spiritualist

- **Usage:** 16.7% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_JOWGEN_THE_SPIRITUALIST`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** You can discard 1 random card from your hand to the Graveyard; destroy all Special Summoned monsters on the field. Neither player can Special Summon monsters.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Summon monsters on both sides, activate **Jowgen the Spiritualist** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.

### Spiritualism

- **Usage:** 16.7% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1539**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** Return a Magic or Trap card of the opponent to their hand. It can never be negated. Status in ROM: Full.
- **YGOPRODeck (TCG) text:** Return 1 Spell/Trap Card your opponent controls to the hand. This card's activation and effect cannot be negated.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Scientist**-rules duel, add **Spiritualism** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

## Vampire

### D.D. Warrior Lady

- **Usage:** 91.3% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_D_D_WARRIOR_LADY`, `gAddedCardEffects`)
- **Coded behavior:** "After damage calculation, if this card battled an opponent's monster: You can banish that monster, also banish this card." No effect row: the effect lives in BattleStage_DestroyMonsters (battle_phase2.c), which asks its controller before the destroyed monsters leave the field (the human through PROMPT_CONFIRM_CARD_EFFECT with this text; the CPU always says Yes). On Yes the battle goes on like a Dimensional Warrior battle: a destroyed monster is banished instead of sent to the Graveyard, and a monster that survived is banished too.
- **YGOPRODeck (TCG) text:** After damage calculation, when this card battles an opponent's monster: You can banish that monster, also banish this card.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **D.D. Warrior Lady**, and confirm whether/how many opponent Graveyard cards are banished or removed.
  2. Give **D.D. Warrior Lady** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Sakuretsu Armor

- **Usage:** 56.5% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SAKURETSU_ARMOR`, `gAddedCardEffects`)
- **Coded behavior:** "When an opponent's monster declares an attack: Target the attacking monster; destroy that target." Normal Trap during RESPONSE_ATTACK_DECLARED (loc0 = attacker, loc1 = attack target), like Negate Attack / Enchanted Javelin. Only loc0 is a legal target; chainB sets it; EffectDestroyTargetResolve destroys it.
- **YGOPRODeck (TCG) text:** When an opponent's monster declares an attack: Target the attacking monster; destroy that target.
- **Scenarios:**
  1. Give **Sakuretsu Armor** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Declare an attack against you with a high-ATK monster, chain **Magic Cylinder** or equivalent, and confirm the attack stops and LP changes match the attacker's ATK.

### Sasuke Samurai

- **Usage:** 30.4% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SASUKE_SAMURAI`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** At the start of the Damage Step, if this card attacks a face-down Defense Position monster: Destroy that face-down monster.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Sasuke Samurai** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Amazoness Archer

- **Usage:** 17.4% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1432**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** Tribute two of your monsters: the opponent loses 1200 LP. Status in ROM: Full.
- **YGOPRODeck (TCG) text:** You can Tribute 2 monsters; inflict 1200 damage to your opponent.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. **Vampire** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Amazoness Archer**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.

### Vampire Lord

- **Usage:** 13.0% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_VAMPIRE_LORD`, `gAddedCardEffects`)
- **Coded behavior:** "If this card inflicts battle damage to your opponent: Declare 1 type of card (Monster, Spell, or Trap); your opponent sends 1 card of that type from their Deck to the GY. Once per turn, during your next Standby Phase after this card in your possession was destroyed and sent to your GY by an opponent's card effect: Special Summon this card from the GY." damage   queued from BattleStage_InflictDamage (battle_phase2.c) like Don Zaloog. The human declares in a menu (LabelMenu); the CPU declares one of the three at random (it cannot see the Deck). The opponent of the card's controller picks: the human in the card list, the CPU its weakest monster or its first Spell/Trap. The Deck is shuffled. A declared type the CPU's Deck lacks is only reported, and one the human's Deck lacks does nothing (the real card has the Deck revealed). revival  Many of the engine's effects destroy without OnCardDestroyedByEffect (Trap Hole, for one), so DestroyFieldCard (duel_field_moves.c) asks VampireLordFieldDestroyed: while a chain resolves, the source is the resolving link's player. Effects outside a chain (Sasuke Samurai's) still report through OnCardDestroyedByEffect (effect_hooks.c). One its owner controlled, destroyed by the other player's effect, adds to gModDuel.vampireLordPending. In that player's next Standby Phase, VampireLordStandbyStep (ModStandbyStep) Special Summons it from the Graveyard in the position they pick, not as a chain link (as Treeborn Frog); a copy no longer there, or no free zone, and nothing happens.
- **YGOPRODeck (TCG) text:** If this card inflicts battle damage to your opponent: Declare 1 type of card (Monster, Spell, or Trap); your opponent sends 1 card of that type from their Deck to the GY. Once per turn, during your next Standby Phase after this card in your possession was destroyed and sent to your GY by an opponent's card effect: Special Summon this card from the GY.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Vampire Lord**, and confirm whether/how many opponent Graveyard cards are banished or removed.
  2. Give **Vampire Lord** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

## Chaos

### Chaos Emperor Dragon - Envoy of the End

- **Usage:** 100.0% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END`, `gAddedCardEffects`)
- **Coded behavior:** Its first-edition text, as the user asked (the fetched one is the later errata, with "Once per turn", damage only for the opponent's cards, and "You cannot activate other cards or effects during the turn you activate this card's effect"): "This card can only be Special Summoned by removing 1 LIGHT and 1 DARK monster in your Graveyard from play. Pay 1000 of your Life Points to send all cards in both players' hands and on the field to the Graveyard. Inflict 300 points of damage to your opponent's Life Points for each card that is sent to the Graveyard by this effect." The summon is Black Luster Soldier - Envoy of the Beginning's (the same hooks, HAS_LIGHT_DARK_BANISH_SUMMON). The effect is the card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no limit per turn: prepare  face up on the field and 1000 LP chainA   1000 LP (DUEL_CMD_LOSE_LP, a cost) resolve  every card on both fields (zones 0-10, as Final Destiny walks them) goes to the Graveyard (SendFieldCardToGrave, with its leave-the-field effects), then every card in both hands (DUEL_CMD_SEND_HAND_CARD_TO_GRAVEYARD: sent, not discarded, so no discard effect); then the opponent takes 300 for each card sent, from either player (counted before: a token goes nowhere and does not count; the Dragon itself does)
- **YGOPRODeck (TCG) text:** Cannot be Normal Summoned/Set. Must be Special Summoned (from your hand) by banishing 1 LIGHT and 1 DARK monster from your GY. Once per turn: You can pay 1000 LP; send as many cards in both players' hands and on the field as possible to the GY, then inflict 300 damage to your opponent for each card sent to the opponent's GY by this effect. You cannot activate other cards or effects during the turn you activate this card's effect.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Chaos Emperor Dragon - Envoy of the End**, and confirm whether/how many opponent Graveyard cards are banished or removed.
  2. With four open zones and no prior summon, activate **Scapegoat** (or similar), confirm four tokens and that further summons are blocked for the turn.
  3. Reach 7+ LIGHT/DARK in hand or field, Special Summon **Chaos Emperor Dragon**, pay costs, and confirm mass hand/field return and damage.

### Dark Magician of Chaos

- **Usage:** 17.2% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_DARK_MAGICIAN_OF_CHAOS`, `gAddedCardEffects`)
- **Coded behavior:** "When this card is Normal or Special Summoned, select 1 Spell Card from your Graveyard during the End Phase, and add it to your hand. If this card destroys a monster by battle, remove that monster from play. If this face-up card is removed from the field, remove it from play." SummonAction_Update (summon_action.c)  DarkMagicianOfChaosSummoned: a face-up Normal Summon or any Special Summon sets its bit in gModDuel.chaosSummoned (a Set or a Flip Summon does not) DuelPhase_End (battle_phase3.c)        EndPhase_DarkMagicianOfChaos: a trigger for each marked zone whose player has a Spell in the Graveyard (the summon was enough: the card need not be on the field any more); the marks are cleared. Its row is Magician of Faith's: the Graveyard Spells in the card-list viewer (EffectCardListTargetChainB, CollectEffectTargets case shared with it, AiPickCardListEntry's case too), added by EffectMaskOfDarknessResolve BattleStage_DestroyMonsters            with it in the battle, the destroyed monsters are banished instead of sent to the Graveyard (gBattle.ddWarriorLady = DD_BANISH_DESTROYED; survivors stay) SendFieldCardToGrave, TributeMonster, ReturnFieldCardToHand, ReturnFieldCardToDeck
- **YGOPRODeck (TCG) text:** During the End Phase, if this card was Normal or Special Summoned this turn: You can target 1 Spell in your GY; add it to your hand. You can only use this effect of "Dark Magician of Chaos" once per turn. If this card destroys an opponent's monster by battle, after damage calculation: Banish that opponent's monster. If this face-up card would leave the field, banish it instead.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. **Chaos** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Dark Magician of Chaos**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. Fill the opponent's Graveyard, attack or trigger **Dark Magician of Chaos**, and confirm whether/how many opponent Graveyard cards are banished or removed.
  3. Give **Dark Magician of Chaos** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Chaos Sorcerer

- **Usage:** 16.1% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CHAOS_SORCERER`, `gAddedCardEffects`)
- **Coded behavior:** Their summon (banish 1 LIGHT and 1 DARK monster from the Graveyard) and their "banish the target; this card cannot attack this turn" effect, once per turn through effectUnused.
- **YGOPRODeck (TCG) text:** Cannot be Normal Summoned/Set. Must first be Special Summoned (from your hand) by banishing 1 LIGHT and 1 DARK monster from your GY. Once per turn: You can target 1 face-up monster on the field; banish that target. This card cannot attack the turn you activate this effect.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Chaos**-rules duel, add **Chaos Sorcerer** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Tsukuyomi

- **Usage:** 13.8% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TSUKUYOMI`, `gAddedCardEffects`)
- **Coded behavior:** Book of Moon (Quick-Play Spell) and Tsukuyomi (FLIP) share the same effect: target 1 face-up monster on the field; change it to face-down Defense Position. Targeting uses EffectTargetableFaceUpMonsterCheck / ChainB (Chaos Sorcerer's pickers). The resolve first destroys the target's Equip Cards (they cannot equip a face-down monster; a destroyed Premature Burial takes the monster with it). Then DUEL_CMD_CHANGE_POSITION puts a monster in Attack Position into Defense Position, and DUEL_CMD_FLIP_CARD turns it face down, which removes its counters (OnMonsterTurnedFaceDown). Tsukuyomi is listed in HasFlipEffect (card_detail.c).
- **YGOPRODeck (TCG) text:** Cannot be Special Summoned. If this card is Normal Summoned or flipped face-up: Target 1 face-up monster on the field; change that target to face-down Defense Position. Once per turn, during the End Phase, if this card was Normal Summoned or flipped face-up this turn: Return it to the hand.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Tsukuyomi** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Set **Tsukuyomi** face-down, have the opponent attack or flip it, and test the FLIP effect once.
  3. Equip **Tsukuyomi** to a legal monster, change the number of face-up monsters or Spells/Traps on your field, and watch ATK/DEF updates.

### Cold Wave

- **Usage:** 12.6% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_COLD_WAVE`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "This turn and the next turn, no player can play any Spell or Trap Cards." Same as engine key 1312 (EffectStartOfMainPhase1Prepare, EffectLockMagicTrapResolve): Main Phase 1 before any Spell/Trap activation or Normal Summon; then both players get magicTrapLockTurns = 2 (DUEL_CMD_SET_MAGIC_TRAP_LOCK_TURNS). The hand menu drops ACTIVATE after either flag is set (card_command_menu.c, as CARD_1312). CPU: AiWantsSpell whenever it may; AiTryPlaySpellTrap also refuses while magicTrapLockTurns is set (ai_deck.c).
- **YGOPRODeck (TCG) text:** This card can only be activated at the start of Main Phase 1. Until your next turn, you and your opponent cannot play or Set any Spell or Trap Cards.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Open Main Phase 1 with no prior plays: activate **Cold Wave** and confirm both players cannot activate Spells/Traps for the coded duration.

### Thunder Nyan Nyan

- **Usage:** 10.3% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THUNDER_NYAN_NYAN`, `gAddedCardEffects`)
- **Coded behavior:** "If there is a non-LIGHT monster on your side of the field, destroy this face-up card." rule  ModDuelStateCheck (DuelMainStep, while nothing else is busy; not during an attack, as for Berserk Gorilla) destroys a face-up Thunder Nyan Nyan whose controller has a face-up monster that is not LIGHT (its effective attribute, GetZoneCardStats). A face-down monster has no attribute and does not count; a face-down Nyan Nyan is not destroyed. CPU   AiSimSummonBoosts (the summon simulation's sandbox) removes the CPU's Nyan Nyans the rule would destroy, so the CPU does not summon it next to a non-LIGHT monster, nor a non-LIGHT monster next to it, unless the simulation still comes out ahead.
- **YGOPRODeck (TCG) text:** If there is a non-LIGHT monster on your side of the field, destroy this face-up card.
- **Scenarios:**
  1. **Chaos** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Thunder Nyan Nyan**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. Give **Thunder Nyan Nyan** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

## Warrior

### Cipher Soldier

- **Usage:** 63.2% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CIPHER_SOLDIER`, `gAddedCardEffects`)
- **Coded behavior:** "If this card battles a Warrior-Type monster, during damage calculation: This card gains 2000 ATK and DEF during that damage calculation only." stats    GetZoneCardStats (card_stats.c) adds ModStatBonus, which has ModBattleStatBonus, to a face-up monster's ATK and DEF: while the battle is in BATTLE_STAGE_DAMAGE_CALC (CalcBattle runs there and keeps its results in gBattle, so the later stages need nothing), a face-up Cipher Soldier that is the attacker or the attack target gains 2000 when the other monster is a Warrior (its printed Type: reading the other monster's stats from here would loop between two Cipher Soldiers) CPU      AiEvalAttack (ai_picks.c) adds ModCipherBonus to both sides of the battles it predicts
- **YGOPRODeck (TCG) text:** If this card battles a Warrior-Type monster, during damage calculation: This card gains 2000 ATK and DEF during that damage calculation only.
- **Scenarios:**
  1. In a **Warrior**-rules duel, add **Cipher Soldier** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Mobius the Frost Monarch

- **Usage:** 57.9% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MOBIUS_THE_FROST_MONARCH`, `gAddedCardEffects`)
- **Coded behavior:** "If this card is Tribute Summoned: You can target up to 2 Spells/Traps on the field; destroy them." The two Normal Summon paths (summon_checks.c) queue its RESPONSE_SUMMONED trigger in Zaborg's case; it is Level 6, so that summon always has a Tribute. Check: a Spell/Trap is on the field (EffectAnySpellTrapCheck, as Mystical Space Typhoon). Targets (EffectMobiusChainB): human  the first card (B: none, the effect is not used), then a second one (B: only the first; skipped when no other Spell/Trap is on the field), Greenkappa's prompts CPU    up to 2 of the human's Spells/Traps, face-up ones first; never its own EffectMobiusResolve destroys each target still on the field, as EffectDestroyTargetResolve does one.
- **YGOPRODeck (TCG) text:** When this card is Tribute Summoned: You can target up to 2 Spell/Trap Cards on the field; destroy those targets.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. **Warrior** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Mobius the Frost Monarch**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. Give **Mobius the Frost Monarch** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  3. Tribute Summon **Mobius the Frost Monarch** (or Special Summon if coded), trigger its on-summon effect, and confirm hand/triggered effects.

### Smashing Ground

- **Usage:** 57.9% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SMASHING_GROUND`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Destroy the 1 face-up monster your opponent controls that has the highest DEF (your choice, if tied)." No target. Prepare: the opponent has a face-up monster. Resolve, like Widespread Ruin's highest-ATK search: the highest DEF among the opponent's face-up monsters (GetZoneCardDef); on a tie the human picks one of them with the cursor (another pick plays the error sound), the CPU takes the first. Then DestroyFieldCard + OnCardDestroyedByEffect.
- **YGOPRODeck (TCG) text:** Destroy the 1 face-up monster your opponent controls that has the highest DEF (your choice, if tied).
- **Scenarios:**
  1. Give **Smashing Ground** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Blade Knight

- **Usage:** 51.3% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BLADE_KNIGHT`, `gAddedCardEffects`)
- **Coded behavior:** +400 ATK at 0–1 cards in hand is in card_stats.c. When its controller has no other monsters, Flip Effects of monsters destroyed by battle with it do not activate (BattleStage_TriggerFlipEffect).
- **YGOPRODeck (TCG) text:** Gains 400 ATK while you have 1 or less cards in your hand. If you control no other monsters, the effects of Flip monsters destroyed by battle with this card are negated.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Blade Knight** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Set **Blade Knight** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Mystic Swordsman LV2

- **Usage:** 40.8% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MYSTIC_SWORDSMAN_LV2`, `gAddedCardEffects`)
- **Coded behavior:** "At the End Phase of a turn in which this card destroyed a monster by battle: You can send this card to the GY; Special Summon 1 'Mystic Swordsman LV4' from your hand or Deck." BattleStage_DestroyMonsters (battle_phase2.c)  MysticSwordsmanAfterBattle: an LV2 (attacking or attacked) whose opponent's monster is destroyed by the battle gets its bit in gModDuel.battleKills. DuelPhase_End (battle_phase3.c)                EndPhase_MysticSwordsmanLevelUp: each marked zone that still holds a face-up LV2 queues a trigger (RESPONSE_NONE, its zone), then the marks are cleared. the trigger's resolve                          the human is asked (the CPU always says Yes), the LV2 is sent to the Graveyard and an LV4 is taken from the Deck (else the hand); the next step, once those commands ran (so the LV2's zone is free), Special Summons it (QueueSpecialSummonChoosePosition).
- **YGOPRODeck (TCG) text:** At the start of the Damage Step, if this card attacked a face-down Defense Position monster: Destroy that monster. During the End Phase, if this card destroyed a monster by battle this turn: You can send this face-up card to the Graveyard; Special Summon 1 "Mystic Swordsman LV4" from your hand or Deck.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Mystic Swordsman LV2**, and confirm whether/how many opponent Graveyard cards are banished or removed.
  2. Give **Mystic Swordsman LV2** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Bottomless Trap Hole

- **Usage:** 36.8% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BOTTOMLESS_TRAP_HOLE`, `gAddedCardEffects`)
- **Coded behavior:** Normal Trap: "When your opponent Summons a monster(s) with 1500 or more ATK: Destroy that monster(s) with 1500 or more ATK, and if you do, banish it." Torrential Tribute's summon window (RESPONSE_SUMMONED, RESPONSE_FLIP_SUMMONED or RESPONSE_SPECIAL_SUMMONED, the summoned monster at loc0); Trap Hole's own handlers test their card numbers and only Normal / Flip Summons. The monster must be the opponent's, face up, with 1500 or more ATK, at activation and at resolution. It is banished (BanishFieldCard: no Graveyard, so no Graveyard effects).
- **YGOPRODeck (TCG) text:** When your opponent Summons a monster(s) with 1500 or more ATK: Destroy that monster(s) with 1500 or more ATK, and if you do, banish it.
- **Scenarios:**
  1. **Warrior** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Bottomless Trap Hole**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. Fill the opponent's Graveyard, attack or trigger **Bottomless Trap Hole**, and confirm whether/how many opponent Graveyard cards are banished or removed.
  3. Give **Bottomless Trap Hole** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Emergency Provisions

- **Usage:** 32.9% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_EMERGENCY_PROVISIONS`, `gAddedCardEffects`)
- **Coded behavior:** Quick-Play Spell: "Send any number of other Spell/Trap Cards you control to the Graveyard; gain 1000 LP for each card sent to the Graveyard this way." prepare  another Spell/Trap of the player's on the field (zones 5-10, the Field Spell too) chainA   the cost, counted in the link's unused unk12. The human: a prompt, then cursor picks of their other Spells/Traps, each sent at once (SendFieldCardToGrave), until B (after at least one) or none is left. The CPU: the ones the link it answers targets, when that targets its Spells/Traps (Mystical Space Typhoon, Dust Tornado, ...), else all of its others (against Heavy Storm, Harpie's Feather Duster, ...) resolve  GainLifePoints, 1000 a card CPU      a Set copy is chained (AiChainSolemnJudgment) to the human's Heavy Storm or Harpie's Feather Duster, or to a card that targets its Spells/Traps (AiChainEmergencyProvisions); from the hand in its Main Phase (AiWantsSpell) only with EP_AI_LOW_LP or less and 2 or more other Spells/Traps
- **YGOPRODeck (TCG) text:** Send any number of other Spell/Trap Cards you control to the Graveyard; gain 1000 LP for each card sent to the Graveyard this way.
- **Scenarios:**
  1. In a **Warrior**-rules duel, add **Emergency Provisions** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Enemy Controller

- **Usage:** 25.0% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_ENEMY_CONTROLLER`, `gAddedCardEffects`)
- **Coded behavior:** Quick-Play Spell: "Activate 1 of these effects. - Target 1 face-up monster your opponent controls; change that target's battle position. - Tribute 1 monster, then target 1 face-up monster your opponent controls; take control of that target until the End Phase." prepare  the opponent has a face-up monster the card may target chainA   the choice (kept in the link's unused unk12): the human picks from a two-line menu, offered only with a monster to Tribute; the CPU takes control when it can, else changes the position chainB   control: first the Tribute (the cost), Catapult Turtle's EffectTributeTargetChainB pick, a monster that cannot be Tributed refused, Tributed at once (as Metamorphosis does); the CPU Tributes its weakest. Then the target: a face-up monster of the opponent's (the human's cursor; the CPU, for control the highest ATK, for the position its first one) resolve  the target must still be face up there: the position changes (ChangeBattlePosition), or Change of Heart's resolve moves it to the player and links it (ZONE_LINK_CARD_EFFECT from Enemy Controller), and DuelCmd_TurnEnd gives it back (ReturnControlAtTurnEnd, duel_cmd_turn.c). Unlike Mind Control's, the monster may attack and be Tributed.
- **YGOPRODeck (TCG) text:** Activate 1 of these effects.
● Target 1 face-up monster your opponent controls; change that target's battle position.
● Tribute 1 monster, then target 1 face-up monster your opponent controls; take control of that target until the End Phase.
- **Scenarios:**
  1. **Warrior** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Enemy Controller**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.

### Mystik Wok

- **Usage:** 23.7% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MYSTIK_WOK`, `gAddedCardEffects`)
- **Coded behavior:** Quick-Play Spell: "Tribute 1 monster. Select either its ATK or DEF and gain that many Life Points." prepare  at least one monster may be Tributed chainB   EffectTributeTargetChainB (the CPU: its weakest by ATK); ATK/DEF are read before TributeMonster resolve  the human picks ATK or DEF from a two-line menu; the CPU takes whichever is higher CPU      AiWantsMystikWok when LP is low and the best stat on its weakest Tribute is worth it
- **YGOPRODeck (TCG) text:** Tribute 1 monster. Select either its ATK or DEF and gain that many Life Points.
- **Scenarios:**
  1. **Warrior** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Mystik Wok**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.

### Berserk Gorilla

- **Usage:** 22.4% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BERSERK_GORILLA`, `gAddedCardEffects`)
- **Coded behavior:** "If this card is in Defense Position: Destroy it. This card must attack, if able." Defense Position  ModDuelStateCheck (DuelMainStep, while nothing else is busy) destroys a face-up one in Defense Position (a face-down one's effect does not apply), except during an attack (battle stages 2-11: a Set one flipped by the attack is destroyed once the attack is over). Must attack       the human cannot end the Battle Phase (B in the attacker selection, card_menu_input.c) while one may still attack (BerserkGorillaBlocksBattleEnd), nor pick "Complete turn" in the Main Phase menu instead of entering it (BerserkGorillaBlocksTurnEnd, duel_phases.c); the CPU attacks with it when it has no better attack (AiPlanForcedAttack). AiShouldRetreatToDefense never turns it to Defense.
- **YGOPRODeck (TCG) text:** If this card is in face-up Defense Position, destroy this card. This card must attack if able.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Berserk Gorilla** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. Set **Berserk Gorilla** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Ninja Grandmaster Sasuke

- **Usage:** 21.1% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_NINJA_GRANDMASTER_SASUKE`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** At the start of the Damage Step, if this card attacks a face-up Defense Position monster: Destroy that monster.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Ninja Grandmaster Sasuke** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### D.D. Assailant

- **Usage:** 19.7% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** Unknown / stats only
- **Coded behavior:** No `gAddedCardEffects` row or engine key found for this demo card; verify in Deck Edit and trunk.
- **YGOPRODeck (TCG) text:** After damage calculation, when this card is destroyed by battle with an opponent's monster: Banish that monster, also banish this card.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **D.D. Assailant** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Return from the Different Dimension

- **Usage:** 15.8% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION`, `gAddedCardEffects`)
- **Coded behavior:** Normal Trap: "Pay half your Life Points; Special Summon as many of your banished monsters as possible. During the End Phase, banish all monsters that were Special Summoned by this effect." prepare  a free monster zone and a monster in the player's banished pile (banished normally: not face down or until the End Phase; not a token) chainA   half the LP (DUEL_CMD_LOSE_LP, a cost) resolve  one monster per step while zones and monsters remain, the strongest ATK first: out of the banished pile (RemoveBanishedCardAt) and Special Summoned face up in the position the player picks (QueueSpecialSummonChoosePosition; the chain waits for each summon). Each one's zone and card ID go to gModDuel.dimensionReturn. DuelPhase_End (battle_phase3.c)  EndPhase_ReturnFromTheDifferentDimension banishes the marked monsters still in their zones, then clears the marks. A monster that cannot be Special Summoned (IsCannotSpecialSummon) stays banished: QueueSpecialSummonChoosePosition would drop it after it left the pile. Dimension Fusion shares the helpers.
- **YGOPRODeck (TCG) text:** Pay half your Life Points; Special Summon as many of your banished monsters as possible. During the End Phase, banish all monsters that were Special Summoned by this effect.
- **Scenarios:**
  1. With four open zones and no prior summon, activate **Scapegoat** (or similar), confirm four tokens and that further summons are blocked for the turn.

### Monster Gate

- **Usage:** 13.2% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MONSTER_GATE`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Tribute 1 monster; excavate cards from the top of your Deck until you excavate a monster that can be Normal Summoned/Set. Special Summon it, also send the other excavated cards to the GY." prepare  a monster the player may Tribute, and a Deck monster that can be Normal Summoned/Set chainB   the cost: Catapult Turtle's EffectTributeTargetChainB pick (a monster that cannot be Tributed is refused), Tributed at once as Metamorphosis does; the CPU Tributes its weakest monster resolve  from the top of the Deck (deck[0]): the cards before the first such monster go to the Graveyard, the monster leaves the Deck; the next step (the commands done) Special Summons it face up in the position the player picks (QueueSpecialSummonChoosePosition). With none, every card goes.
- **YGOPRODeck (TCG) text:** Tribute 1 monster; excavate cards from the top of your Deck until you excavate a monster that can be Normal Summoned/Set. Special Summon it, also send the other excavated cards to the GY.
- **Scenarios:**
  1. **Warrior** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Monster Gate**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.

### Monster Reincarnation

- **Usage:** 13.2% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MONSTER_REINCARNATION`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Discard 1 card, then target 1 monster in your Graveyard; add that target to your hand." prepare  at least one card in the hand to discard and a monster in the Graveyard (ModCollectEffectTargets) chainA   DuelPrompt_PostDiscardCost (any card; the CPU picks its own) chainB   EffectCardListTargetChainB (graveyard monsters) resolve  EffectMaskOfDarknessResolve (DUEL_CMD_RETURN_GRAVEYARD_CARD_TO_HAND) CPU      AiWantsMonsterReincarnation (empty monster field only); AiPickCardListEntry prefers a monster that beats the human's strongest field monster when they control one
- **YGOPRODeck (TCG) text:** Discard 1 card, then target 1 monster in your GY; add it to your hand.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Warrior**-rules duel, add **Monster Reincarnation** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Pikeru's Circle of Enchantment

- **Usage:** 13.2% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT`, `gAddedCardEffects`)
- **Coded behavior:** Normal Trap: "Damage to you from card effects becomes 0 until the end of this turn." resolve  gModDuel.noEffectDamageTurn[player] = this turn: ModTakesNoEffectDamage stops the damage of LoseLifePoints (every effect damage) for the player until the turn ends. Battle damage (InflictBattleDamage) and LP paid as a cost are not effect damage. CPU      only chained to the human's burn Spell or Trap (AiChainPikerusCircle, in AiChainSolemnJudgment before Hallowed Life Barrier, which costs a card): PIKERU_AI_MIN_BURN or more, or all of its LP (AiIncomingBurn); AiShouldActivateAddedCard keeps it Set otherwise
- **YGOPRODeck (TCG) text:** Damage to you from card effects becomes 0 until the end of this turn.
- **Scenarios:**
  1. In a **Warrior**-rules duel, add **Pikeru's Circle of Enchantment** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

## Goat

### Mind Control

- **Usage:** 66.5% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MIND_CONTROL`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Target 1 monster your opponent controls; take control of it until the End Phase, but it cannot declare an attack or be Tributed." The row is Change of Heart's: EffectChangeOfHeartPrepare, EffectOpponentMonsterCheck, EffectTakeControlChainB and EffectChangeOfHeartResolve, which moves the monster to a free zone of the player and gives it a ZONE_LINK_CARD_EFFECT link from the activating card, here Mind Control. That link is what the rest keys on: DuelCmd_TurnEnd (duel_cmd_turn.c)   ReturnControlAtTurnEnd(player, zone, CARD_MIND_CONTROL), as for Change of Heart CanMonsterAttack (card_menu_input.c) and IsTributableMonster (duel_zones.c)   IsMindControlled refuses it.
- **YGOPRODeck (TCG) text:** Target 1 monster your opponent controls; until the End Phase, take control of that target, but it cannot declare an attack or be Tributed.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. **Goat** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Mind Control**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.

### Night Assailant

- **Usage:** 52.8% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_NIGHT_ASSAILANT`, `gAddedCardEffects`)
- **Coded behavior:** "FLIP: Target 1 monster your opponent controls; destroy that target. When this card is sent from your hand to the GY: Target 1 Flip monster in your GY, except 'Night Assailant'; add it to your hand." One row, told apart by the link's event: flip                 a flip effect (HasFlipEffect, card_detail.c): Crass Clown's opponent-monster target (EffectOpponentMonsterCheck / ChainB), Man-Eater Bug's EffectDestroyTargetResolve RESPONSE_DISCARDED   the off-field trigger that DiscardHandCard (duel_field_moves.c) queues through QueueNightAssailantTrigger, only when the Graveyard holds a target; the list (CollectEffectTargets, effect_target_collect.c: IsNightAssailantTarget) goes through Mask of Darkness's card-list viewer and EffectMaskOfDarknessResolve. A "Flip monster" is one that HasFlipEffect lists (the coded flip effects).
- **YGOPRODeck (TCG) text:** FLIP: Target 1 monster your opponent controls; destroy that target.
When this card is sent from the hand to the Graveyard: Target 1 Flip Effect Monster in your Graveyard, except this card; return that target to the hand.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Fill the opponent's Graveyard, attack or trigger **Night Assailant**, and confirm whether/how many opponent Graveyard cards are banished or removed.
  2. Give **Night Assailant** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  3. Set **Night Assailant** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Dekoichi the Battlechanted Locomotive

- **Usage:** 52.4% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE`, `gAddedCardEffects`)
- **Coded behavior:** "FLIP: Draw 1 card, then draw 1 more card for each 'Bokoichi the Freightening Car' you control." A flip effect (HasFlipEffect, card_detail.c). Bokoichi counts only when it is one of the added cards.
- **YGOPRODeck (TCG) text:** FLIP: Draw 1 card, then draw 1 additional card for each face-up "Bokoichi the Freightening Car" you control.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Activate or trigger **Dekoichi the Battlechanted Locomotive** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.
  2. Set **Dekoichi the Battlechanted Locomotive** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Lightning Vortex

- **Usage:** 32.5% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_LIGHTNING_VORTEX`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Discard 1 card; destroy all face-up monsters your opponent controls." Prepare: a card to discard (2 in hand when played from the hand) and a face-up monster on the opponent's side. ChainA: the discard cost of Magic Jammer / Tribe-Infecting Virus (DuelPrompt_PostDiscardCost). Resolve: no target; every face-up monster the opponent controls then is destroyed (DestroyFieldCard + OnCardDestroyedByEffect).
- **YGOPRODeck (TCG) text:** Discard 1 card; destroy all face-up monsters your opponent controls.
- **Scenarios:**
  1. Summon monsters on both sides, activate **Lightning Vortex** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.

### Phoenix Wing Wind Blast

- **Usage:** 15.5% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_PHOENIX_WING_WIND_BLAST`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Discard 1 card, then target 1 card your opponent controls; place that target on the top of the Deck." Cost: the discard of Magic Jammer / Tribe-Infecting Virus (DuelPrompt_PostDiscardCost). Target: any card in the opponent's monster, Spell/Trap or Field zones that the card may target (CanCardTargetZone), as Raigeki Break but on one side. The target step does not ask CanActivateEffect again: the discard is paid by then, and the prepare handler counts the hand. Resolve: ReturnFieldCardToDeck (to the top of its owner's Deck; a Fusion Monster goes to the Fusion Deck and a token is destroyed, the game's rules for DUEL_CMD_RETURN_TO_DECK).
- **YGOPRODeck (TCG) text:** Discard 1 card, then target 1 card your opponent controls; place that target on the top of the Deck.
- **Scenarios:**
  1. Give **Phoenix Wing Wind Blast** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  2. With four open zones and no prior summon, activate **Scapegoat** (or similar), confirm four tokens and that further summons are blocked for the turn.

### Xing Zhen Hu

- **Usage:** 15.2% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_XING_ZHEN_HU`, `gAddedCardEffects`)
- **Coded behavior:** Continuous Trap: "Activate by selecting 2 Set Spell or Trap Cards on the field. The selected Spell or Trap Cards cannot be activated." (the card data's text) Like Spellbinding Circle on a monster: the resolve gives each target still Set a ZONE_LINK_CONTINUOUS link whose value is Xing Zhen Hu's DUEL_LOC. CanActivateEffect (effect_activation.c) asks IsLockedByXingZhenHu first, so a locked Set card is refused everywhere (card menu, response windows, CPU). The lock holds while that zone holds a face-up, enabled Xing Zhen Hu; a target that leaves takes its links with it (ClearZone). prepare  two Set Spell/Trap cards on the field besides this one (the Set Xing Zhen Hu counts as Set) chainB   two different Set cards with the cursor (B starts again); the CPU takes the human's
- **YGOPRODeck (TCG) text:** Activate by selecting 2 Set Spell or Trap Cards on the field. The selected Spell or Trap Cards cannot be activated.
- **Scenarios:**
  1. In a **Goat**-rules duel, add **Xing Zhen Hu** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Brain Control

- **Usage:** 12.8% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BRAIN_CONTROL`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Pay 800 LP, then target 1 face-up monster your opponent controls that can be Normal Summoned/Set; take control of it until the End Phase." Change of Heart's way, like Mind Control: EffectChangeOfHeartResolve moves the target and links it to this card (ZONE_LINK_CARD_EFFECT), and DuelCmd_TurnEnd returns it (ReturnControlAtTurnEnd with CARD_BRAIN_CONTROL). prepare  more than 800 LP, a free monster zone, and a monster IsBrainControlTarget accepts chainA   the 800 LP (DUEL_CMD_LOSE_LP as a cost) chainB   EffectTakeControlChainB with Snatch Steal's face-up cursor (effect_targets2.c), wrapped: a monster that cannot be Normal Summoned/Set (a Fusion, Ritual or Special-Summon-only monster, a token) is refused, and the CPU, whose pick is the human's strongest monster, takes the strongest valid one instead
- **YGOPRODeck (TCG) text:** Pay 800 LP, then target 1 face-up monster your opponent controls that can be Normal Summoned/Set; take control of that target until the End Phase.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. With four open zones and no prior summon, activate **Scapegoat** (or similar), confirm four tokens and that further summons are blocked for the turn.

### Zaborg the Thunder Monarch

- **Usage:** 10.6% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_ZABORG_THE_THUNDER_MONARCH`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** If this card is Tribute Summoned: Target 1 monster on the field; destroy that target.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. **Goat** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Zaborg the Thunder Monarch**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. Give **Zaborg the Thunder Monarch** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.
  3. Tribute Summon **Zaborg the Thunder Monarch** (or Special Summon if coded), trigger its on-summon effect, and confirm hand/triggered effects.

### Level Limit - Area B

- **Usage:** 10.3% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_LEVEL_LIMIT_AREA_B`, `gAddedCardEffects`)
- **Coded behavior:** Effect text (YGOPRODeck API): "Change all face-up Level 4 or higher monsters to Defense Position." Continuous Spell in the card pool; stays on the field when activated. Like Dragon Capture Jar (328): activation and ApplyLevelLimitAreaB after a face-up summon; ModDuelStateCheck keeps attack-position Level 4+ down while it is active; the human cannot manually switch a face-up Level 4+ monster to Attack (card_command_menu.c).
- **YGOPRODeck (TCG) text:** Change all face-up Level 4 or higher monsters to Defense Position.
- **Scenarios:**
  1. In a **Goat**-rules duel, add **Level Limit - Area B** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

## Cyber

### Cyber Twin Dragon

- **Usage:** 95.6% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CYBER_TWIN_DRAGON`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** "Cyber Dragon" + "Cyber Dragon"
A Fusion Summon of this card can only be done with the above Fusion Materials. This card can make a second attack during each Battle Phase.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Cyber**-rules duel, add **Cyber Twin Dragon** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Cyber Dragon

- **Usage:** 84.4% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CYBER_DRAGON`, `gAddedCardEffects`)
- **Coded behavior:** "If only your opponent controls a monster, you can Special Summon this card (from your hand)." Like key 1350 (the original's Fiend Megacyber rule): CanSummonFromHand (summon_checks.c) allows it, the hand card menu (card_command_menu.c) adds the Special Summon commands, and the summon steps (effect_hooks.c) queue QueueSpecialSummonFromHand for them (no Tribute). It is still a Level 5 Normal Summon with one Tribute. The CPU Special Summons it first thing in its Main Phase (AiStepMainPhase calls AiSpecialSummonCyberDragon before it picks a Normal Summon), in Attack Position.
- **YGOPRODeck (TCG) text:** If only your opponent controls a monster, you can Special Summon this card (from your hand).
- **Scenarios:**
  1. **Cyber** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Cyber Dragon**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.

### Cyber End Dragon

- **Usage:** 82.2% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CYBER_END_DRAGON`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** "Cyber Dragon" + "Cyber Dragon" + "Cyber Dragon"
A Fusion Summon of this card can only be done with the above Fusion Materials. If this card attacks a Defense Position monster, inflict piercing battle damage.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. In a **Cyber**-rules duel, add **Cyber End Dragon** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Threatening Roar

- **Usage:** 60.0% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THREATENING_ROAR`, `gAddedCardEffects`)
- **Coded behavior:** "Your opponent cannot declare an attack this turn." (Normal Trap) The resolve stores the turn in gModDuel.roarTurn of the activating player's opponent (gDuel.turnCount + 1, 0 none). ModAttacksBlocked then holds for that player for the rest of the turn: BuildAttackableMask (card_menu_input.c) leaves their attackable mask empty, as Swords of Revealing Light does (no Battle Phase, no attacker to pick), and the CPU's attacker choice (BattleStage_SelectAttacker) is skipped. An attack already declared goes on: the replay check (Battle_CheckReplay) asks CanMonsterAttack, which is unchanged. The CPU activates it: before the human's Battle Phase   ModBattlePhaseStart (BattleStage_Start, card_menu_input.c, as the Battle Phase begins): while the human has a face-up monster with 2000 or more ATK, it flips its Set Threatening Roar (CanActivateFieldCard) and queues it, as its Standby activations do; it resolves before the attackable mask is built against an attack                 while the human has another monster that could attack after it (AiShouldActivateAddedCard)
- **YGOPRODeck (TCG) text:** Your opponent cannot declare an attack this turn.
- **Scenarios:**
  1. Set **Threatening Roar** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Exarion Universe

- **Usage:** 40.0% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_EXARION_UNIVERSE`, `gAddedCardEffects`)
- **Coded behavior:** (YGOPRODeck) "During your Battle Step, if this card attacks a Defense Position monster: You can make this card lose exactly 400 ATK, and if it does, it will inflict piercing battle damage. These effects last until the end of this turn." Like Injection Fairy Lily, the optional effect is handled in the battle code (no face-up monster response window): BattleStage_DamageCalc, DAMAGE_STEP_MARK_ATTACKER   ExarionUniverseResetBattle BattleStage_DamageCalc, DAMAGE_STEP_CALC            ExarionUniverseDamageStep before CalcBattle: the attacking Exarion vs a Defense Position monster, ATK at least 400. The human is asked (PROMPT_CONFIRM_CARD_EFFECT); the CPU pays when its ATK exceeds the DEF (piercing LP damage). Paying queues a gModDuel.exarionPenaltyStacks (ModStatBonus: -400 ATK each, cleared when that player's turn ends) and sets exarionPierce for ModHasPiercing. DuelCmd_TurnEnd                                       ExarionUniverseClearTurnPenalty for the turn player. ModEndAttack                                        clears exarionPierce for the next attack.
- **YGOPRODeck (TCG) text:** During your Battle Step, if this card attacks a Defense Position monster: You can make this card lose exactly 400 ATK, and if it does, it will inflict piercing battle damage. These effects last until the end of this turn.
- **Scenarios:**
  1. In a **Cyber**-rules duel, add **Exarion Universe** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Gravity Bind

- **Usage:** 16.7% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_GRAVITY_BIND`, `gAddedCardEffects`)
- **Coded behavior:** Effect text (YGOPRODeck API): "Level 4 or higher monsters cannot attack." Continuous Trap; stays face-up when activated. The attack rule is in CanMonsterAttack (card_menu_input.c), with vanilla effect key CARD_1323. CPU: AiPlayAddedCards; AiShouldActivateAddedCard default yes (CARD_1323 in AiShouldActivateSetCard).
- **YGOPRODeck (TCG) text:** Level 4 or higher monsters cannot attack.
- **Scenarios:**
  1. In a **Cyber**-rules duel, add **Gravity Bind** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Chiron the Mage

- **Usage:** 11.1% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CHIRON_THE_MAGE`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn: You can discard 1 Spell, then target 1 Spell/Trap your opponent controls; destroy that target." Passcode 16956455 ([YGOPRODeck](https://ygoprodeck.com/card/chiron-the-mage-5386)). Main Phase ignition (CanActivateMonsterEffect), once per turn (effectUnused). ChainA discards a Magic card (DISCARD_FLAG_SPELL for the human; the CPU uses FindMagicInHand). ChainB targets the opponent's Spell/Trap zones (EffectOpponentSpellTrapCheck; the human uses EffectDustTornadoChainB). Resolve: EffectDestroyTargetResolve.
- **YGOPRODeck (TCG) text:** Once per turn: You can discard 1 Spell Card, then target 1 Spell/Trap Card your opponent controls; destroy that target.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Chiron the Mage** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Cursed Seal of the Forbidden Spell

- **Usage:** 11.1% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CURSED_SEAL_OF_THE_FORBIDDEN_SPELL`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** When a Spell Card is activated: Discard 1 Spell; negate the activation, and if you do, destroy it, and if you do that, your opponent cannot activate Spell Cards with that name for the rest of this Duel.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Give **Cursed Seal of the Forbidden Spell** a legal target (face-up monster, Spell/Trap zone, etc.), activate it, and confirm the target leaves the field and chained effects resolve in the right order.

### Wave-Motion Cannon

- **Usage:** 11.1% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_WAVE_MOTION_CANNON`, `gAddedCardEffects`)
- **Coded behavior:** Continuous Spell: "During your Main Phase: You can send this face-up card to the GY; inflict 1000 damage to your opponent for each of your Standby Phases that have passed since this card was activated." activation from hand   resolve does nothing (the card stays face-up); turnCounter stays 0 until Standby each Standby Phase   once per phase (gModDuel.wmcStandbyTicked), turnCounter on the turn player's face-up copies ignition from field  prepare requires Main Phase and !fromHand; resolve sends the card to the GY and burns 1000 * turnCounter (link kind CHAIN_KIND_SPELL_TRAP, as field activations in duel_cmd_queue.c). Card menu and CanChainFieldCard allow face-up activation in Main Phase.
- **YGOPRODeck (TCG) text:** During your Main Phase: You can send this face-up card to the GY; inflict 1000 damage to your opponent for each of your Standby Phases that have passed since this card was activated.
- **Scenarios:**
  1. In a **Cyber**-rules duel, add **Wave-Motion Cannon** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

## Reaper

### Treeborn Frog

- **Usage:** 28.4% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TREEBORN_FROG`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn, during your Standby Phase, if this card is in your Graveyard and you do not control "Treeborn Frog": You can Special Summon this card. You must control no Spell/Trap Cards to activate and to resolve this effect." DuelPhase_Standby (duel_phases.c) runs TreebornFrogStandbyStep right after Sinister Serpent's step, in the same way (on gDuel.phaseSubStep, not a chain link; once per turn, as the Standby Phase runs once): step 0  done unless the player has one in the Graveyard, no face-up Treeborn Frog, no Spell/Trap card (the Field Spell included), a free monster zone and CanSpecialSummon. The CPU says Yes; the human is asked. step 1  on Yes, with still no Spell/Trap card: it leaves the Graveyard (RemoveGraveyardCardByNumber) and is Special Summoned face up, the player choosing its position (QueueSpecialSummonChoosePosition).
- **YGOPRODeck (TCG) text:** Once per turn, during your Standby Phase, if this card is in your Graveyard and you do not control "Treeborn Frog": You can Special Summon this card. You must control no Spell/Trap Cards to activate and to resolve this effect.
- **Scenarios:**
  1. In a **Reaper**-rules duel, add **Treeborn Frog** to the deck, draw it in a realistic line, and run the effect once as human and once watching CPU handling.
  2. Repeat after a relevant negation (Magic Jammer, Solemn Judgment, etc.) if the card is a Spell/Trap with an activation.

### Thestalos the Firestorm Monarch

- **Usage:** 21.3% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THESTALOS_THE_FIRESTORM_MONARCH`, `gAddedCardEffects`)
- **Coded behavior:** "If this card is Tribute Summoned: Discard 1 random card from your opponent's hand, then, if it was a Monster Card, inflict damage to your opponent equal to its original Level x 100." The two Normal Summon paths (summon_checks.c) queue its RESPONSE_SUMMONED trigger in Zaborg's case; it is Level 6, so that summon always has a Tribute. Mandatory, no target. The resolve discards a random card of the opponent's hand (DiscardHandCard, by the opponent's effect) and, for a monster, takes its printed Level x 100 from the opponent's LP (LoseLifePoints).
- **YGOPRODeck (TCG) text:** If this card is Tribute Summoned: Discard 1 random card from your opponent's hand, then, if it was a Monster Card, inflict damage to your opponent equal to its original Level x 100.
- **Scenarios:**
  1. **Reaper** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Thestalos the Firestorm Monarch**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
  2. Tribute Summon **Thestalos the Firestorm Monarch** (or Special Summon if coded), trigger its on-summon effect, and confirm hand/triggered effects.

### D.D. Survivor

- **Usage:** 19.4% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_D_D_SURVIVOR`, `gAddedCardEffects`)
- **Coded behavior:** "If this face-up card is banished: Special Summon it during the End Phase." DuelCmd_Banish (duel_cmd_field.c), BanishBattleDestroyedCard   DDSurvivorBanished DuelCmd_BanishGraveyardCard (duel_cmd_piles.c: Bazoo, Chaos costs; Graveyard cards are face up) DuelPhase_End (battle_phase3.c)                                EndPhase_DDSurvivor Banished before the End Phase, the owner's count goes up and EndPhase_DDSurvivor queues one trigger per copy; banished during the End Phase (Return from the Different Dimension's banish), the trigger is queued at once, so it still returns that End Phase. Mandatory: the resolve Special Summons a D.D. Survivor from the owner's banished pile (banished normally, not face down) face up in the position the owner picks.
- **YGOPRODeck (TCG) text:** During the End Phase, if this card was banished while face-up, on your side of the field, this turn: Special Summon this banished card. This effect can only activate once per turn.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Summon monsters on both sides, activate **D.D. Survivor** at legal timing, and confirm board-wide destruction and whether FLIP or floating effects still queue.

### Pot of Avarice

- **Usage:** 17.5% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_POT_OF_AVARICE`, `gAddedCardEffects`)
- **Coded behavior:** "Target 5 monsters in your GY; shuffle all 5 into the Deck, then draw 2 cards." (Normal Spell)
- **YGOPRODeck (TCG) text:** Target 5 monsters in your GY; shuffle all 5 into the Deck, then draw 2 cards.
- **Scenarios:**
  1. Activate or trigger **Pot of Avarice** when its draw effect should run (draw phase, chain, etc.) and count cards in hand before and after.

### Magical Merchant

- **Usage:** 17.1% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MAGICAL_MERCHANT`, `gAddedCardEffects`)
- **Coded behavior:** "FLIP: Excavate cards from the top of your Deck until you excavate a Spell/Trap. Add that card to your hand, also send the remaining cards to the GY." A flip effect (HasFlipEffect, card_detail.c). The top of the Deck is deck[0]. The monsters above the first Spell/Trap go to the Graveyard (DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD, one by one); with no Spell/Trap at all the whole Deck goes. The Spell/Trap is shown and added with Sangan's AddDeckCardToHand, which takes the first card of that number in the Deck: that one, as every card above it is a monster.
- **YGOPRODeck (TCG) text:** FLIP: Excavate cards from the top of your Deck until you excavate a Spell/Trap, then add that card to your hand, also send the remaining cards to the GY.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **Scenarios:**
  1. Set **Magical Merchant** face-down, have the opponent attack or flip it, and test the FLIP effect once.

### Mask of Restrict

- **Usage:** 14.2% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MASK_OF_RESTRICT`, `gAddedCardEffects`)
- **Coded behavior:** "Neither player can Tribute cards." Continuous Trap; the tribute lock is vanilla effect key CARD_1418 in the original engine — face-up copies count through IsTributeLockActive (every CARD_1418 check in the duel code). CPU: AiWantsMaskOfRestrict, AiPlayAddedCards (Main Phase); Set copies use AiShouldActivateAddedCard (yes).
- **YGOPRODeck (TCG) text:** Neither player can Tribute cards.
- **Scenarios:**
  1. **Reaper** duel, Main Phase 1: with an opponent monster you can use as Tribute, activate **Mask of Restrict**, Tribute Summon a high-Level monster, and confirm battle skipping / tribute timing.
