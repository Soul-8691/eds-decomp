# Effect testing scenarios (high-usage demo cards)

Manual play-test checklist for **demo-build** cards in [`demo_cards.txt`](demo_cards.txt), **excluding** any name that also appears under the `******` line in [`new_cards.txt`](new_cards.txt) (those duplicates are tracked for the full list, not for this pass). Cards must appear in at least **10%** of decks in some Campaign format's usage statistics. Inclusion is read only from each file's top-level `total` and `extra` arrays in [`usage/`](usage/) (fraction of `decks_processed` that run the card, main plus extra), then multiplied by **3** for limited cards and **1.5** for semi-limited in that format's banlist (capped at **100%**). Each card appears **once**, under the **earliest** format (in [`tools/banlist.py`](tools/banlist.py) `BANLISTS` order from **Yugi-Kaiba** through **Reaper**) where it meets that threshold.

For each card: **Implementation** and **Coded behavior** come from this repo (`src/added_effects.c`, engine effect keys, [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md), and related files). **YGOPRODeck text** is loaded from [YGOPRODeck API v7](https://db.ygoprodeck.com/api/v7/cardinfo.php) (`name` query). If the build differs from TCG/OCG, scenarios still follow the code—differences are noted, not treated as bugs to fix here.

**Scenarios** are exact, so an mGBA Lua script can reproduce each one ([`tools/testing_scenarios/`](tools/testing_scenarios/README.md)): the whole starting state, every input, and the state to check after. They are written in `tools/testing_scenarios/exact_cards_*.py` (the format: `tools/testing_scenarios/exact.py`); every card they name is legal in that format (in its [`card_pools/`](card_pools/) file, not **Forbidden** in [`banlists/<format>_ban.json`](banlists/)), which the generator checks. Each scenario has:

- **A header line:** the recording name (`<Card>_<Effect>_<Player|CPU>`), where the setup is applied (the start of that turn's Main Phase 1, the Draw Phase done; `turnCount` 0 is the duel's first turn, which has no Battle Phase), both LP, and whether the opponent is frozen (`gModTestingFreezeOpponent`: it plays nothing and answers nothing). **Player** scenarios test the human's use of the card; **CPU** scenarios test the CPU's. On the opponent's turn the CPU acts by itself.
- **A setup table:** every card placed, by side and place. Monster Zones `m0`-`m4` and Spell/Trap Zones `st0`-`st4` are `gDuelPlayers[p]` zones 0-4 and 5-9; the hand is listed from slot 0; the Deck rows are its top, top card first (`deck[0]` is the next draw), the rest being the format's usage deck. Both hands are exactly as listed; anything else not listed is empty, except in a **CPU** scenario: there the CPU's board and piles from its first turn stay (only its hand and the listed places are replaced), and the CPU Sets at most 4 Spell/Traps, so a zone stays free for its Normal Spell.
- **Steps:** each input as code (the script's step) and in words. `you.hand[Card]` is that card in your hand, `you.m0` your Monster Zone 0, `opp.m[Card]` the opponent's monster of that name; `when <event>: <step>` waits for the event first; `check` checks the state mid-way.
- **Expected:** the state to check after the last step, as code and in words (`=` is the exact pile or zone, `has` / `lacks` one card, `count` the number of cards).

**AI:** each card lists **AI coverage** traced from `src/added_effects.c` (`AiPlayAddedCards`, `AiWantsMonsterEffect`, `AiWantsSpell`, `AiShouldActivateAddedCard`, `AiChain*`, and `*Cpu*` effect steps). **Yes — generalized routine** means the default added-card fallbacks apply; **dedicated** means a named hook. **Automatic** means the CPU does not activate an effect from the card menu, but battle/engine code still runs.

**103** cards qualify (of 295 in `demo_cards.txt` minus `new_cards.txt` below `******`; 13 names excluded). Regenerate with `python3 tools/gen_testing_md.py`.

## Yugi-Kaiba

### Card Destruction

- **Usage:** 100.0% weighted inclusion in `usage/yugi-kaiba.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CARD_DESTRUCTION`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Both players discard as many cards as possible from their hands, then each player draws the same number of cards they discarded." No prepare: it may be activated like Pot of Greed. Each hand is discarded through DiscardHandCard by index, with the hand compacted only after its last card (the indices stay valid until then), so the discard triggers run (Night Assailant; Electric Snake and Minar for the opponent's hand, discarded by an opponent's effect). Then the draws: a player whose deck runs out decks out as usual (player 0 cannot).
- **YGOPRODeck (TCG) text:** Both players discard as many cards as possible from their hands, then each player draws the same number of cards they discarded.
- **AI coverage:** **Yes — generalized routine** — Monster menu: when the human holds more cards than the CPU would discard Main Phase Spell: when the human holds more cards than the CPU would discard Set/chain: when the human holds more cards than the CPU would discard

#### Player 1: Discard and draw

`Card_Destruction_Discard_And_Draw_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Card Destruction |  |
| You | Hand 1 | Dark Hole |  |
| You | Hand 2 | Battle Ox |  |
| You | Deck 0 | Summoned Skull |  |
| You | Deck 1 | Celtic Guardian |  |
| You | Deck 2 | Fissure |  |
| Opponent | Hand 0 | Trap Hole |  |
| Opponent | Hand 1 | Fissure |  |
| Opponent | Hand 2 | Battle Ox |  |
| Opponent | Hand 3 | La Jinn the Mystical Genie of the Lamp |  |
| Opponent | Deck 0 | Dark Magician |  |
| Opponent | Deck 1 | Celtic Guardian |  |
| Opponent | Deck 2 | Man-Eater Bug |  |
| Opponent | Deck 3 | Swords of Revealing Light |  |

**Steps**

1. `activate you.hand[Card Destruction]`: Open the card menu of **Card Destruction** in your hand and choose Activate.

**Expected**

- `you.hand = Summoned Skull, Celtic Guardian`: your hand is exactly Summoned Skull, Celtic Guardian.
- `you.gy = Card Destruction, Dark Hole, Battle Ox`: your Graveyard is exactly Card Destruction, Dark Hole, Battle Ox.
- `opp.hand = Dark Magician, Celtic Guardian, Man-Eater Bug, Swords of Revealing Light`: the opponent's hand is exactly Dark Magician, Celtic Guardian, Man-Eater Bug, Swords of Revealing Light.
- `opp.gy = Trap Hole, Fissure, Battle Ox, La Jinn the Mystical Genie of the Lamp`: the opponent's Graveyard is exactly Trap Hole, Fissure, Battle Ox, La Jinn the Mystical Genie of the Lamp.

#### CPU 1: Discard and draw

`Card_Destruction_Cpu_Discard_And_Draw_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

The CPU plays it when the human holds more cards than it would discard.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Card Destruction |  |
| Opponent | Hand 1 | Battle Ox |  |
| You | Hand 0 | Summoned Skull |  |
| You | Hand 1 | Dark Hole |  |
| You | Hand 2 | Fissure |  |
| You | Hand 3 | Trap Hole |  |
| You | Hand 4 | Celtic Guardian |  |
| You | Deck 0 | Battle Ox |  |
| You | Deck 1 | Celtic Guardian |  |
| You | Deck 2 | Man-Eater Bug |  |
| You | Deck 3 | Fissure |  |
| You | Deck 4 | Swords of Revealing Light |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Card Destruction`: the opponent's Graveyard includes Card Destruction.
- `you.gy = Summoned Skull, Dark Hole, Fissure, Trap Hole, Celtic Guardian`: your Graveyard is exactly Summoned Skull, Dark Hole, Fissure, Trap Hole, Celtic Guardian.
- `you.hand = Battle Ox, Celtic Guardian, Man-Eater Bug, Fissure, Swords of Revealing Light`: your hand is exactly Battle Ox, Celtic Guardian, Man-Eater Bug, Fissure, Swords of Revealing Light.

### Soul Exchange

- **Usage:** 91.1% weighted inclusion in `usage/yugi-kaiba.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SOUL_EXCHANGE`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Target 1 monster your opponent controls; this turn, if you Tribute a monster, you must Tribute that monster, as if you controlled it. You cannot conduct your Battle Phase the turn you activate this card." The engine's effect key 1220 (tools/newcards.py ENGINE_NUMBERS gives the card that number): prepare EffectCanTributeOpponentMonsterPrepare (Main Phase 1, a level 5+ monster to Tribute Summon or a Catapult Turtle-like monster), chainA EffectSkipBattlePhaseChainA, chainB EffectTributeOpponentMonsterChainB, resolve EffectTributeOpponentMonsterResolve (a Tribute Summon from the hand, or the Tribute of a Catapult Turtle, The Little Swordsman of Aile or Cannon Soldier). Two fixes in effect_resolve8.c: the summon paths Tributed the opponent's zone 0 (the packed target was passed as the player), and the steps are cursor UI only, so the CPU gets these:
- **YGOPRODeck (TCG) text:** Target 1 monster your opponent controls; this turn, if you Tribute a monster, you must Tribute that target, as if you controlled it. You cannot conduct your Battle Phase the turn you activate this card.
- **TCG vs this ROM:** Documented implementation quirk in this build (see **Coded behavior**); do not expect full modern TCG timing.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `SoulExchangeCpuChainB`, `SoulExchangeCpuResolve` Dedicated `*Cpu*` handlers in the card's effect steps.

#### Player 1: Tribute opponent monster

`Soul_Exchange_Tribute_Opponent_Monster_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Soul Exchange |  |
| You | Hand 1 | Summoned Skull |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |

**Steps**

1. `activate you.hand[Soul Exchange]`: Open the card menu of **Soul Exchange** in your hand and choose Activate.
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Battle Ox**) as the target.
3. `tribute_summon you.hand[Summoned Skull] -> m0 atk tributes opp.m0`: Tribute Summon **Summoned Skull** in your hand into your Monster Zone 0 (face-up Attack Position), Tributing the opponent's Monster Zone 0 (**Battle Ox**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Battle Ox**) is empty.
- `opp.gy = Battle Ox`: the opponent's Graveyard is exactly Battle Ox.
- `you.m0 = Summoned Skull atk`: your Monster Zone 0 holds Summoned Skull, face-up Attack Position.
- `you.gy = Soul Exchange`: your Graveyard is exactly Soul Exchange.
- `battle_phase blocked`: The Battle Phase cannot be entered.

#### CPU 1: Tribute opponent monster

`Soul_Exchange_Cpu_Tribute_Opponent_Monster_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Soul Exchange |  |
| Opponent | Hand 1 | Summoned Skull |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Battle Ox**) is empty.
- `you.gy = Battle Ox`: your Graveyard is exactly Battle Ox.
- `opp.monsters = Summoned Skull`: the opponent's monsters is exactly Summoned Skull.
- `opp.gy = Soul Exchange`: the opponent's Graveyard is exactly Soul Exchange.
- `you.lp = 8000`: LP of you = 8000.

## Imperial

### Thousand-Eyes Restrict

- **Usage:** 47.6% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THOUSAND_EYES_RESTRICT`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** "Relinquished" + "Thousand-Eyes Idol"
Other monsters on the field cannot change their battle positions or attack. Once per turn: You can target 1 monster your opponent controls; equip that target to this card (max. 1). This card's ATK/DEF become equal to that equipped monster's. If this card would be destroyed by battle, destroy that equipped monster instead.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — dedicated logic** — Monster menu: Conditional AiWants* / AiShouldActivateAddedCard Set/chain: Conditional AiWants* / AiShouldActivateAddedCard

#### Player 1: Absorb and attack

`Thousand_Eyes_Restrict_Absorb_And_Attack_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Thousand-Eyes Restrict | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 1 | Battle Ox | face-up Attack Position |

**Steps**

1. `activate you.m0`: Open the card menu of your Monster Zone 0 (**Thousand-Eyes Restrict**) and activate its effect.
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.
3. `check you.m0.atk = 2500`: Check: The face-up Attack Position of your Monster Zone 0 (**Thousand-Eyes Restrict**) is 2500.
4. `check you.m0.def = 1200`: Check: The face-up Defense Position of your Monster Zone 0 (**Thousand-Eyes Restrict**) is 1200.
5. `battle`: Enter the Battle Phase.
6. `attack you.m0 -> opp.m1`: Attack with your Monster Zone 0 (**Thousand-Eyes Restrict**): target the opponent's Monster Zone 1 (**Battle Ox**).

**Expected**

- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `opp.gy = Battle Ox`: the opponent's Graveyard is exactly Battle Ox.
- `opp.lp = 7200`: LP of opp = 7200.
- `menu you.m0 lacks Activate`: The card menu of your Monster Zone 0 (**Thousand-Eyes Restrict**) does not offer Activate.

#### CPU 1: Absorb

`Thousand_Eyes_Restrict_Cpu_Absorb_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

It absorbs Summoned Skull (2500 ATK), then attacks directly.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Thousand-Eyes Restrict | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.lp = 5500`: LP of you = 5500.

### The Fiend Megacyber

- **Usage:** 45.2% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THE_FIEND_MEGACYBER`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** If your opponent controls at least 2 more monsters than you do, you can Special Summon this card (from your hand).
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Special summon from hand

`The_Fiend_Megacyber_Special_Summon_From_Hand_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | The Fiend Megacyber |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 1 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |

**Steps**

1. `special_summon you.hand[The Fiend Megacyber] -> m0 atk`: Special Summon **The Fiend Megacyber** in your hand from the hand by its own rule into your Monster Zone 0 (face-up Attack Position).

**Expected**

- `you.m0 = The Fiend Megacyber atk`: your Monster Zone 0 holds The Fiend Megacyber, face-up Attack Position.

#### Player 2: Not with one monster difference

`The_Fiend_Megacyber_Not_With_One_Monster_Difference_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | The Fiend Megacyber |  |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 1 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |

**Steps**

1. `check menu you.hand[The Fiend Megacyber] lacks Special Summon`: Check: The card menu of **The Fiend Megacyber** in your hand does not offer Special Summon.

**Expected**

- `you.hand = The Fiend Megacyber`: your hand is exactly The Fiend Megacyber.

#### CPU 1: Special summon

`The_Fiend_Megacyber_Cpu_Special_Summon_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | The Fiend Megacyber |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Monster Zone 1 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.monsters has The Fiend Megacyber`: the opponent's monsters includes The Fiend Megacyber.

### Goblin Attack Force

- **Usage:** 40.5% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1344. key 1344: attacking uses up effectUnused (BattleStage_DamageCalc), at the end of the Battle Phase a face-up Attack Position one turns to Defense Position (BattleStage_EndBattlePhase), and its controller's next turn start locks its position for that turn and re-arms it (DuelCmd_TurnStart)
- **YGOPRODeck (TCG) text:** If this card attacks, it is changed to Defense Position at the end of the Battle Phase, and its battle position cannot be changed until the End Phase of your next turn.
- **AI coverage:** **Automatic** (no activation logic needed) — Effect runs in engine/battle code when the CPU summons and attacks normally (`ai_deck.c`, `ai_summon.c`, `ai_picks.c`). Confirm the opponent still sets/attacks with the card; the passive or mandatory follow-up should apply without a card-menu prompt.

#### Player 1: Mandatory follow up

`Goblin_Attack_Force_Mandatory_Follow_Up_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Goblin Attack Force | face-up Attack Position |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Goblin Attack Force**): target the opponent's Monster Zone 0 (**Celtic Guardian**).
3. `main2`: Go to Main Phase 2.
4. `check you.m0 = Goblin Attack Force def`: Check: your Monster Zone 0 (**Goblin Attack Force**) holds Goblin Attack Force, face-up Defense Position.
5. `pass`: End your turn (its End Phase effects resolve).
6. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).
7. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.
8. `check menu you.m0 lacks Change Position`: Check: The card menu of your Monster Zone 0 (**Goblin Attack Force**) does not offer Change Position.

**Expected**

- `opp.lp = 7100`: LP of opp = 7100.
- `opp.gy = Celtic Guardian`: the opponent's Graveyard is exactly Celtic Guardian.
- `you.m0 = Goblin Attack Force def`: your Monster Zone 0 (**Goblin Attack Force**) holds Goblin Attack Force, face-up Defense Position.

#### CPU 1: Attack then defense

`Goblin_Attack_Force_Cpu_Attack_Then_Defense_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Goblin Attack Force | face-up Attack Position |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Celtic Guardian**) is empty.
- `you.lp = 7100`: LP of you = 7100.
- `opp.m0 = Goblin Attack Force def`: the opponent's Monster Zone 0 (**Goblin Attack Force**) holds Goblin Attack Force, face-up Defense Position.

### Solemn Wishes

- **Usage:** 28.6% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SOLEMN_WISHES`, `gAddedCardEffects`)
- **Coded behavior:** Continuous Trap (YGOPRODeck text): "Increase your Life Points by 500 points each time you draw a card (or cards)." Same behaviour as the vanilla effect key CARD_1305 (DrawCards in duel_card_actions.c): while a face-up copy is on your field, each DrawCards call shows the card and gains 500 LP per copy (one gain per draw action, not per card in the batch). Activation resolve is a no-op; the trap stays face-up. CPU: AiPlayAddedCards / default Set flip.
- **YGOPRODeck (TCG) text:** Increase your Life Points by 500 points each time you draw a card (or cards).
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Gain per draw

`Solemn_Wishes_Gain_Per_Draw_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

One gain per draw action: Pot of Greed draws 2 cards for 500 LP.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Solemn Wishes | face-down (Set) |
| You | Hand 0 | Pot of Greed |  |
| You | Deck 0 | Battle Ox |  |
| You | Deck 1 | Celtic Guardian |  |
| You | Deck 2 | Fissure |  |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Solemn Wishes**) and choose Activate.
2. `activate you.hand[Pot of Greed]`: Open the card menu of **Pot of Greed** in your hand and choose Activate.
3. `check you.lp = 8500`: Check: LP of you = 8500.
4. `check you.hand = Battle Ox, Celtic Guardian`: Check: your hand is exactly Battle Ox, Celtic Guardian.
5. `pass`: End your turn (its End Phase effects resolve).
6. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).
7. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.

**Expected**

- `you.lp = 9000`: LP of you = 9000.
- `you.hand = Battle Ox, Celtic Guardian, Fissure`: your hand is exactly Battle Ox, Celtic Guardian, Fissure.
- `you.st0 = Solemn Wishes up`: your Spell/Trap Zone 0 (**Solemn Wishes**) holds Solemn Wishes, face up.

#### CPU 1: Activate and gain

`Solemn_Wishes_Cpu_Activate_And_Gain_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Solemn Wishes | face-down (Set) |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).
2. `check opp.st0 = Solemn Wishes up`: Check: the opponent's Spell/Trap Zone 0 (**Solemn Wishes**) holds Solemn Wishes, face up.
3. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.
4. `pass`: End your turn (its End Phase effects resolve).
5. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.lp >= 8500`: LP of opp >= 8500.

### Limiter Removal

- **Usage:** 21.4% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_LIMITER_REMOVAL`, `gAddedCardEffects`)
- **Coded behavior:** "Double the ATK of all Machine monsters you currently control, until the end of this turn. During the End Phase of this turn, destroy those monsters." Same engine pattern as key CARD_1314 (EffectDoubleMachineAtkResolve): a ZONE_LINK_CARD_EFFECT from this card's id on each qualifying monster; GetZoneCardStats doubles ATK for that link; EndPhase_LimiterRemoval destroys every monster that still holds the link on the activation turn. Machines you summon later this turn are not affected.
- **YGOPRODeck (TCG) text:** Double the ATK of all Machine monsters you currently control, until the end of this turn. During the End Phase of this turn, destroy those monsters.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsLimiterRemoval` Monster menu: Conditional AiWants* / AiShouldActivateAddedCard Main Phase Spell: Conditional AiWants* / AiShouldActivateAddedCard Set/chain: Conditional AiWants* / AiShouldActivateAddedCard

#### Player 1: Double machine atk

`Limiter_Removal_Double_Machine_Atk_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Limiter Removal |  |
| You | Monster Zone 0 | Jinzo | face-up Attack Position |
| You | Monster Zone 1 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.hand[Limiter Removal]`: Open the card menu of **Limiter Removal** in your hand and choose Activate.
2. `check you.m0.atk = 4800`: Check: The face-up Attack Position of your Monster Zone 0 (**Jinzo**) is 4800.
3. `check you.m1.atk = 1700`: Check: The face-up Attack Position of your Monster Zone 1 (**Battle Ox**) is 1700.
4. `battle`: Enter the Battle Phase.
5. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Jinzo**): target the opponent's Monster Zone 0 (**Summoned Skull**).
6. `check opp.lp = 5700`: Check: LP of opp = 5700.
7. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Jinzo**) is empty.
- `you.gy = Limiter Removal, Jinzo`: your Graveyard is exactly Limiter Removal, Jinzo.
- `you.m1 = Battle Ox atk`: your Monster Zone 1 (**Battle Ox**) holds Battle Ox, face-up Attack Position.
- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.

#### CPU 1: Double machine atk

`Limiter_Removal_Cpu_Double_Machine_Atk_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Limiter Removal |  |
| Opponent | Monster Zone 0 | Jinzo | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Limiter Removal`: the opponent's Graveyard includes Limiter Removal.
- `opp.gy has Jinzo`: the opponent's Graveyard includes Jinzo.
- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.lp = 5700`: LP of you = 5700.

### 4-Starred Ladybug of Doom

- **Usage:** 16.7% weighted inclusion in `usage/imperial.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_4_STARRED_LADYBUG_OF_DOOM`, `gAddedCardEffects`)
- **Coded behavior:** FLIP: "Destroy all Level 4 monsters your opponent controls." A resolve-only row, as Fiber Jar (card_detail.c HasFlipEffect lists it). Every face-up monster of the opponent (of Ladybug's controller) whose Level is 4 is destroyed, as Tribe-Infecting Virus destroys (DestroyFieldCard, OnCardDestroyedByEffect); a face-down monster has no Level, so it stays.
- **YGOPRODeck (TCG) text:** FLIP: Destroy all Level 4 monsters your opponent controls.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Flip destroy level 4

`4_Starred_Ladybug_of_Doom_Flip_Destroy_Level_4_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

A face-down monster has no Level, so Man-Eater Bug stays.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | 4-Starred Ladybug of Doom | face-down (Set) |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 1 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| Opponent | Monster Zone 2 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 3 | Man-Eater Bug | face-down (Set) |

**Steps**

1. `flip_summon you.m0`: Flip Summon your Monster Zone 0 (**4-Starred Ladybug of Doom**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Battle Ox**) is empty.
- `opp.m1 = empty`: the opponent's Monster Zone 1 (**La Jinn the Mystical Genie of the Lamp**) is empty.
- `opp.m2 = Summoned Skull atk`: the opponent's Monster Zone 2 (**Summoned Skull**) holds Summoned Skull, face-up Attack Position.
- `opp.m3 = Man-Eater Bug set`: the opponent's Monster Zone 3 (**Man-Eater Bug**) holds Man-Eater Bug, face-down (Set).
- `opp.gy = Battle Ox, La Jinn the Mystical Genie of the Lamp`: the opponent's Graveyard is exactly Battle Ox, La Jinn the Mystical Genie of the Lamp.
- `you.m0 = 4-Starred Ladybug of Doom atk`: your Monster Zone 0 (**4-Starred Ladybug of Doom**) holds 4-Starred Ladybug of Doom, face-up Attack Position.

#### CPU 1: Flip destroy level 4

`4_Starred_Ladybug_of_Doom_Cpu_Flip_Destroy_Level_4_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | 4-Starred Ladybug of Doom | face-down (Set) |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Monster Zone 1 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `you.gy = Battle Ox, Celtic Guardian`: your Graveyard is exactly Battle Ox, Celtic Guardian.

## Android

### Kycoo the Ghost Destroyer

- **Usage:** 100.0% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_KYCOO_THE_GHOST_DESTROYER`, `gAddedCardEffects`)
- **Coded behavior:** Reuses effect key 1511's handlers (EffectBanishOpponentGraveMonstersResolve, battle damage triggers in battle_phase2.c). While face up, it blocks graveyard banish effects the same way CARD_1511 does.
- **YGOPRODeck (TCG) text:** When this card inflicts battle damage to your opponent: You can target up to 2 monsters in their Graveyard; banish those targets. Your opponent cannot banish cards from either player's Graveyard.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Banish on battle damage

`Kycoo_the_Ghost_Destroyer_Banish_On_Battle_Damage_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Kycoo the Ghost Destroyer | face-up Attack Position |
| Opponent | Graveyard | Summoned Skull |  |
| Opponent | Graveyard | Battle Ox |  |
| Opponent | Graveyard | Sangan |  |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> direct`: Attack with your Monster Zone 0 (**Kycoo the Ghost Destroyer**): target the opponent directly.
3. `yes`: Answer Yes.
4. `pick opp.gy[Summoned Skull]`: Pick **Summoned Skull** in the opponent's Graveyard in the list or on the field.
5. `pick opp.gy[Sangan]`: Pick **Sangan** in the opponent's Graveyard in the list or on the field.

**Expected**

- `opp.lp = 6200`: LP of opp = 6200.
- `opp.banished = Summoned Skull, Sangan`: the opponent's banished cards is exactly Summoned Skull, Sangan.
- `opp.gy = Battle Ox`: the opponent's Graveyard is exactly Battle Ox.

#### CPU 1: Banish on battle damage

`Kycoo_the_Ghost_Destroyer_Cpu_Banish_On_Battle_Damage_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Kycoo the Ghost Destroyer | face-up Attack Position |
| You | Graveyard | Summoned Skull |  |
| You | Graveyard | Battle Ox |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.lp = 6200`: LP of you = 6200.
- `you.banished count 2`: your banished cards holds 2 card(s).
- `you.gy count 0`: your Graveyard holds 0 card(s).

### Bazoo the Soul-Eater

- **Usage:** 97.4% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BAZOO_THE_SOUL_EATER`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn: You can banish up to 3 monsters from your Graveyard; this card gains 300 ATK for each, until the end of your opponent's turn." The engine's effect key 1513 (ENGINE_NUMBERS gives the card that number): the card menu's ACTIVATE once per turn (effectUnused) with a monster in the player's Graveyard, EffectBanishGraveForAtkResolve (asks before each of up to 3 picks, then a ZONE_LINK_ATK_300_PER_VALUE link of the count), and DuelCmd_TurnStart re-arms it and drops the link at its controller's next turn. That resolve is UI only, so the CPU gets BazooCpuResolve (effect_resolve11.c calls it): its 3 weakest Graveyard monsters (fewer if it has fewer), then the same link and once-per-turn mark.
- **YGOPRODeck (TCG) text:** Once per turn: You can banish up to 3 monsters from your Graveyard; this card gains 300 ATK for each, until the end of your opponent's turn.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `BazooCpuResolve` Dedicated `*Cpu*` handlers in the card's effect steps.

#### Player 1: Banish three for atk

`Bazoo_the_Soul_Eater_Banish_Three_For_Atk_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Bazoo the Soul-Eater | face-up Attack Position |
| You | Graveyard | Battle Ox |  |
| You | Graveyard | La Jinn the Mystical Genie of the Lamp |  |
| You | Graveyard | Sangan |  |
| You | Graveyard | Summoned Skull |  |

**Steps**

1. `activate you.m0`: Open the card menu of your Monster Zone 0 (**Bazoo the Soul-Eater**) and activate its effect.
2. `yes`: Answer Yes.
3. `pick you.gy[Battle Ox]`: Pick **Battle Ox** in your Graveyard in the list or on the field.
4. `yes`: Answer Yes.
5. `pick you.gy[La Jinn the Mystical Genie of the Lamp]`: Pick **La Jinn the Mystical Genie of the Lamp** in your Graveyard in the list or on the field.
6. `yes`: Answer Yes.
7. `pick you.gy[Sangan]`: Pick **Sangan** in your Graveyard in the list or on the field.

**Expected**

- `you.m0.atk = 2500`: The face-up Attack Position of your Monster Zone 0 (**Bazoo the Soul-Eater**) is 2500.
- `you.banished = Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan`: your banished cards is exactly Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan.
- `you.gy = Summoned Skull`: your Graveyard is exactly Summoned Skull.
- `menu you.m0 lacks Activate`: The card menu of your Monster Zone 0 (**Bazoo the Soul-Eater**) does not offer Activate.

#### CPU 1: Banish weakest

`Bazoo_the_Soul_Eater_Cpu_Banish_Weakest_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

BazooCpuResolve banishes its 3 weakest Graveyard monsters; 2500 ATK then beats Jinzo.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Bazoo the Soul-Eater | face-up Attack Position |
| Opponent | Graveyard | Battle Ox |  |
| Opponent | Graveyard | La Jinn the Mystical Genie of the Lamp |  |
| Opponent | Graveyard | Sangan |  |
| Opponent | Graveyard | Kuriboh |  |
| You | Monster Zone 0 | Jinzo | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.banished = Kuriboh, Sangan, Battle Ox`: the opponent's banished cards is exactly Kuriboh, Sangan, Battle Ox.
- `opp.gy = La Jinn the Mystical Genie of the Lamp`: the opponent's Graveyard is exactly La Jinn the Mystical Genie of the Lamp.
- `opp.m0.atk = 2500`: The face-up Attack Position of the opponent's Monster Zone 0 (**Bazoo the Soul-Eater**) is 2500.
- `you.m0 = empty`: your Monster Zone 0 (**Jinzo**) is empty.
- `you.lp = 7900`: LP of you = 7900.

### Torrential Tribute

- **Usage:** 97.4% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TORRENTIAL_TRIBUTE`, `gAddedCardEffects`)
- **Coded behavior:** "When a monster(s) is Summoned: Destroy all monsters on the field." A Normal Trap answering the response window that a summon opens (SummonAction_Update, EventResponse_Run): event RESPONSE_SUMMONED, RESPONSE_FLIP_SUMMONED or RESPONSE_SPECIAL_SUMMONED (not RESPONSE_SET), with the summoned monster at loc0, like Trap Hole (EffectTrapHolePrepare) but for any monster and either player's summon. The effect is Dark Hole's (EffectDarkHoleResolve: every monster, the opponent's first).
- **YGOPRODeck (TCG) text:** When a monster(s) is Summoned: Destroy all monsters on the field.
- **AI coverage:** **Yes — generalized routine** — Monster menu: it destroys the CPU's monsters too Main Phase Spell: it destroys the CPU's monsters too Set/chain: it destroys the CPU's monsters too

#### Player 1: Destroy all on summon

`Torrential_Tribute_Destroy_All_On_Summon_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

Destroyed by an effect: Mystic Tomato does not trigger, and the Set Man-Eater Bug is not flipped.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Mystic Tomato | face-up Attack Position |
| You | Spell/Trap Zone 0 | Torrential Tribute | face-down (Set) |
| You | Hand 0 | Battle Ox |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 1 | Man-Eater Bug | face-down (Set) |

**Steps**

1. `summon you.hand[Battle Ox] -> m1 atk`: Normal Summon **Battle Ox** in your hand into your Monster Zone 1 (face-up Attack Position).
2. `when prompt response: activate you.st0`: When the response prompt opens: open the card menu of your Spell/Trap Zone 0 (**Torrential Tribute**) and choose Activate.

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `you.gy = Mystic Tomato, Battle Ox, Torrential Tribute`: your Graveyard is exactly Mystic Tomato, Battle Ox, Torrential Tribute.
- `opp.gy = Summoned Skull, Man-Eater Bug`: the opponent's Graveyard is exactly Summoned Skull, Man-Eater Bug.

#### CPU 1: On human summon

`Torrential_Tribute_Cpu_On_Human_Summon_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Torrential Tribute | face-down (Set) |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| You | Monster Zone 1 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| You | Hand 0 | Battle Ox |  |

**Steps**

1. `summon you.hand[Battle Ox] -> m2 atk`: Normal Summon **Battle Ox** in your hand into your Monster Zone 2 (face-up Attack Position).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `you.gy = Summoned Skull, La Jinn the Mystical Genie of the Lamp, Battle Ox`: your Graveyard is exactly Summoned Skull, La Jinn the Mystical Genie of the Lamp, Battle Ox.
- `opp.gy = Torrential Tribute`: the opponent's Graveyard is exactly Torrential Tribute.

### Jar of Greed

- **Usage:** 92.1% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_JAR_OF_GREED`, `gAddedCardEffects`)
- **Coded behavior:** Normal Trap: "Draw 1 card." A row with no prepare or check, like Gift of the Mystical Elf's: CanActivateEffect lets a Set Jar of Greed be activated in any response window, at Trap speed.
- **YGOPRODeck (TCG) text:** Draw 1 card.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Draw one

`Jar_of_Greed_Draw_One_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Jar of Greed | face-down (Set) |
| You | Deck 0 | Battle Ox |  |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Jar of Greed**) and choose Activate.

**Expected**

- `you.hand = Battle Ox`: your hand is exactly Battle Ox.
- `you.gy = Jar of Greed`: your Graveyard is exactly Jar of Greed.

#### CPU 1: Draw one

`Jar_of_Greed_Cpu_Draw_One_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Jar of Greed | face-down (Set) |
| Opponent | Deck 0 | Battle Ox |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Jar of Greed`: the opponent's Graveyard includes Jar of Greed.

### Skull Lair

- **Usage:** 89.5% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SKULL_LAIR`, `gAddedCardEffects`)
- **Coded behavior:** Continuous Trap: "Remove from play any number of Monster Cards in your Graveyard to destroy 1 face-up monster on the field whose Level is equal to the number of the cards you removed from play." Like Ultimate Offering it can be activated again while face up (card_command_menu.c, effect_activation.c, effect_targets4.c list it), each time: check    a face-up monster (either side) the card may target, of Level 1 to the monsters in the player's Graveyard (it is the target filter of the pick too) prepare  such a monster exists chainB   the target: EffectTargetableFaceUpMonsterChainB; the CPU's pick (the human's strongest) is replaced by its best legal one (SkullLairBestTarget: the human's strongest such monster) resolve  the target still face up and still legal: as many Graveyard monsters as its Level are banished (the human picks each one in the Graveyard list, as Bazoo's; the CPU banishes its weakest), then it is destroyed.
- **YGOPRODeck (TCG) text:** Remove from play any number of Monster Cards in your Graveyard to destroy 1 face-up monster on the field whose Level is equal to the number of the cards you removed from play.
- **AI coverage:** **Yes — generalized routine** — Monster menu: destroy one of the human's monsters worth 1000+ ATK Main Phase Spell: destroy one of the human's monsters worth 1000+ ATK Set/chain: destroy one of the human's monsters worth 1000+ ATK

#### Player 1: Banish level count

`Skull_Lair_Banish_Level_Count_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Sangan is Level 3, so 3 Graveyard monsters are banished.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Skull Lair | face-down (Set) |
| You | Graveyard | Battle Ox |  |
| You | Graveyard | La Jinn the Mystical Genie of the Lamp |  |
| You | Graveyard | Sangan |  |
| You | Graveyard | Mystic Tomato |  |
| Opponent | Monster Zone 0 | Sangan | face-up Attack Position |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Skull Lair**) and choose Activate.
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Sangan**) as the target.
3. `pick you.gy[Battle Ox]`: Pick **Battle Ox** in your Graveyard in the list or on the field.
4. `pick you.gy[La Jinn the Mystical Genie of the Lamp]`: Pick **La Jinn the Mystical Genie of the Lamp** in your Graveyard in the list or on the field.
5. `pick you.gy[Sangan]`: Pick **Sangan** in your Graveyard in the list or on the field.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Sangan**) is empty.
- `opp.gy = Sangan`: the opponent's Graveyard is exactly Sangan.
- `you.banished = Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan`: your banished cards is exactly Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan.
- `you.gy = Mystic Tomato`: your Graveyard is exactly Mystic Tomato.
- `you.st0 = Skull Lair up`: your Spell/Trap Zone 0 (**Skull Lair**) holds Skull Lair, face up.

#### CPU 1: Destroy strong monster

`Skull_Lair_Cpu_Destroy_Strong_Monster_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Skull Lair | face-down (Set) |
| Opponent | Graveyard | Battle Ox |  |
| Opponent | Graveyard | La Jinn the Mystical Genie of the Lamp |  |
| Opponent | Graveyard | Sangan |  |
| Opponent | Graveyard | Kuriboh |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Battle Ox**) is empty.
- `opp.banished count 4`: the opponent's banished cards holds 4 card(s).
- `opp.st0 = Skull Lair up`: the opponent's Spell/Trap Zone 0 (**Skull Lair**) holds Skull Lair, face up.

### Magic Cylinder

- **Usage:** 78.9% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1214. key 1214: EffectAttackResponsePrepare / Resolve (negate the attack, the attacker's player loses its ATK); the CPU's trap logic knows it (ai_summon.c)
- **YGOPRODeck (TCG) text:** When an opponent's monster declares an attack: Target the attacking monster; negate the attack, and if you do, inflict damage to your opponent equal to its ATK.
- **AI coverage:** **Generic engine** — Uses the vanilla effect key tables; the CPU plays it like other cards in its deck lists. No dedicated `AiWants*` row in `src/added_effects.c`—watch whether the opponent activates the effect when rules allow.

#### Player 1: Reflect attack

`Magic_Cylinder_Reflect_Attack_Player` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Magic Cylinder | face-down (Set) |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `when attack opp.m0 -> direct: activate you.st0`: When the opponent's Monster Zone 0 (**Summoned Skull**) declares an attack on you directly: open the card menu of your Spell/Trap Zone 0 (**Magic Cylinder**) and choose Activate.

**Expected**

- `opp.lp = 5500`: LP of opp = 5500.
- `you.lp = 8000`: LP of you = 8000.
- `you.gy = Magic Cylinder`: your Graveyard is exactly Magic Cylinder.
- `opp.m0 = Summoned Skull atk`: the opponent's Monster Zone 0 (**Summoned Skull**) holds Summoned Skull, face-up Attack Position.

#### CPU 1: Reflect attack

`Magic_Cylinder_Cpu_Reflect_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Magic Cylinder | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> direct`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent directly.

**Expected**

- `you.lp = 5500`: LP of you = 5500.
- `opp.lp = 8000`: LP of opp = 8000.
- `opp.gy = Magic Cylinder`: the opponent's Graveyard is exactly Magic Cylinder.

### St. Joan

- **Usage:** 31.6% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1445**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** Fusion of 1444 + 1446. No effect code. Status in ROM: No effect code.
- **YGOPRODeck (TCG) text:** "The Forgiving Maiden" + "Darklord Marie"
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Unclear / likely missing** — No `CPU:` notes and no switch case found; treat as untested for CPU until you confirm in a duel.

#### Player 1: Fusion summon

`St_Joan_Fusion_Summon_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Polymerization |  |
| You | Hand 1 | The Forgiving Maiden |  |
| You | Hand 2 | Darklord Marie |  |
| You | Fusion Deck | St. Joan |  |

**Steps**

1. `fusion_summon St. Joan from you.hand[The Forgiving Maiden], you.hand[Darklord Marie] -> m0 atk`: Activate Polymerization from your hand: Fusion Summon **St. Joan** from **The Forgiving Maiden** in your hand and **Darklord Marie** in your hand into your Monster Zone 0 (face-up Attack Position).

**Expected**

- `you.m0 = St. Joan atk`: your Monster Zone 0 holds St. Joan, face-up Attack Position.
- `you.m0.atk = 2800`: The face-up Attack Position of your Monster Zone 0 is 2800.
- `you.gy = Polymerization, The Forgiving Maiden, Darklord Marie`: your Graveyard is exactly Polymerization, The Forgiving Maiden, Darklord Marie.

#### CPU 1: Attack

`St_Joan_Cpu_Attack_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | St. Joan | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.lp = 7700`: LP of you = 7700.

### United We Stand

- **Usage:** 23.7% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1449. key 1449: an Equip (EffectEquipResolve and the equip target rules), +800 ATK/DEF per face-up monster of the equip card's controller (CalcZoneCardStats)
- **YGOPRODeck (TCG) text:** The equipped monster gains 800 ATK/DEF for each face-up monster you control.
- **AI coverage:** **Generic engine** — Uses the vanilla effect key tables; the CPU plays it like other cards in its deck lists. No dedicated `AiWants*` row in `src/added_effects.c`—watch whether the opponent activates the effect when rules allow.

#### Player 1: Equip per face up monster

`United_We_Stand_Equip_Per_Face_Up_Monster_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

+800 ATK/DEF for each face-up monster you control: 2, then 3 after the Flip Summon.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | United We Stand |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Monster Zone 1 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| You | Monster Zone 2 | Mystic Tomato | face-down (Set) |

**Steps**

1. `activate you.hand[United We Stand]`: Open the card menu of **United We Stand** in your hand and choose Activate.
2. `target you.m0`: Pick your Monster Zone 0 (**Battle Ox**) as the target.
3. `check you.m0.atk = 3300`: Check: The face-up Attack Position of your Monster Zone 0 (**Battle Ox**) is 3300.
4. `check you.m0.def = 2600`: Check: The face-up Defense Position of your Monster Zone 0 (**Battle Ox**) is 2600.
5. `flip_summon you.m2`: Flip Summon your Monster Zone 2 (**Mystic Tomato**).

**Expected**

- `you.m0.atk = 4100`: The face-up Attack Position of your Monster Zone 0 (**Battle Ox**) is 4100.
- `you.m0.def = 3400`: The face-up Defense Position of your Monster Zone 0 (**Battle Ox**) is 3400.

#### CPU 1: Equip

`United_We_Stand_Cpu_Equip_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | United We Stand |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 1 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.spells has United We Stand`: the opponent's Spells/Traps includes United We Stand.
- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.

### Mage Power

- **Usage:** 15.8% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1450. key 1450: an Equip, +500 ATK/DEF per Spell/Trap of the equip card's controller, itself included (CalcZoneCardStats; changed to count the Field Spell too)
- **YGOPRODeck (TCG) text:** The equipped monster gains 500 ATK and DEF for each Spell/Trap Card you control.
- **AI coverage:** **Generic engine** — Uses the vanilla effect key tables; the CPU plays it like other cards in its deck lists. No dedicated `AiWants*` row in `src/added_effects.c`—watch whether the opponent activates the effect when rules allow.

#### Player 1: Equip per spell trap

`Mage_Power_Equip_Per_Spell_Trap_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

+500 for each of your Spells/Traps, Mage Power included: 3.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Mage Power |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| You | Spell/Trap Zone 1 | Call of the Haunted | face-down (Set) |

**Steps**

1. `activate you.hand[Mage Power]`: Open the card menu of **Mage Power** in your hand and choose Activate.
2. `target you.m0`: Pick your Monster Zone 0 (**Battle Ox**) as the target.

**Expected**

- `you.m0.atk = 3200`: The face-up Attack Position of your Monster Zone 0 (**Battle Ox**) is 3200.
- `you.m0.def = 2500`: The face-up Defense Position of your Monster Zone 0 (**Battle Ox**) is 2500.

#### CPU 1: Equip

`Mage_Power_Cpu_Equip_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Mage Power |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.m0.atk = 2700`: The face-up Attack Position of the opponent's Monster Zone 0 (**Battle Ox**) is 2700.
- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.lp = 7800`: LP of you = 7800.

### Zombyra the Dark

- **Usage:** 15.8% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_ZOMBYRA_THE_DARK`, `gAddedCardEffects`)
- **Coded behavior:** "This card cannot attack directly. Each time this card destroys a monster by battle, it loses 200 ATK." CanMonsterAttack (card_menu_input.c; the CPU's attacker list uses it too)   it may not attack while the opponent has no monster (that is a direct attack) BattleStage_DestroyMonsters (battle_phase2.c, AddedCardsAfterBattle)        when it survives a battle that destroyed the other monster, a self link ZONE_LINK_ATK_DOWN_200 (from its card ID; AddZoneLink raises the count of an existing one) takes 200 ATK more The link goes with the card when it leaves the field.
- **YGOPRODeck (TCG) text:** This card cannot attack your opponent directly. If this card destroys a monster by battle: This card loses 200 ATK.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Lose atk after destroying

`Zombyra_the_Dark_Lose_Atk_After_Destroying_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Zombyra the Dark | face-up Attack Position |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Zombyra the Dark**): target the opponent's Monster Zone 0 (**Battle Ox**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Battle Ox**) is empty.
- `opp.lp = 7600`: LP of opp = 7600.
- `you.m0.atk = 1900`: The face-up Attack Position of your Monster Zone 0 (**Zombyra the Dark**) is 1900.

#### Player 2: No direct attack

`Zombyra_the_Dark_No_Direct_Attack_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Zombyra the Dark | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.

**Expected**

- `cannot attack you.m0 -> direct`: your Monster Zone 0 (**Zombyra the Dark**) cannot attack the opponent directly.
- `opp.lp = 8000`: LP of opp = 8000.

#### CPU 1: Attack

`Zombyra_the_Dark_Cpu_Attack_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Zombyra the Dark | face-up Attack Position |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Celtic Guardian**) is empty.
- `you.lp = 7300`: LP of you = 7300.
- `opp.m0.atk = 1900`: The face-up Attack Position of the opponent's Monster Zone 0 (**Zombyra the Dark**) is 1900.

### Humanoid Worm Drake

- **Usage:** 10.5% weighted inclusion in `usage/android.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1404**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** Fusion of 1403 + 1402. No effect code. Status in ROM: No effect code.
- **YGOPRODeck (TCG) text:** "Worm Drake" + "Humanoid Slime"
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Unclear / likely missing** — No `CPU:` notes and no switch case found; treat as untested for CPU until you confirm in a duel.

#### Player 1: Fusion summon

`Humanoid_Worm_Drake_Fusion_Summon_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Polymerization |  |
| You | Hand 1 | Worm Drake |  |
| You | Hand 2 | Humanoid Slime |  |
| You | Fusion Deck | Humanoid Worm Drake |  |

**Steps**

1. `fusion_summon Humanoid Worm Drake from you.hand[Worm Drake], you.hand[Humanoid Slime] -> m0 atk`: Activate Polymerization from your hand: Fusion Summon **Humanoid Worm Drake** from **Worm Drake** in your hand and **Humanoid Slime** in your hand into your Monster Zone 0 (face-up Attack Position).

**Expected**

- `you.m0 = Humanoid Worm Drake atk`: your Monster Zone 0 holds Humanoid Worm Drake, face-up Attack Position.
- `you.gy = Polymerization, Worm Drake, Humanoid Slime`: your Graveyard is exactly Polymerization, Worm Drake, Humanoid Slime.

#### CPU 1: Attack

`Humanoid_Worm_Drake_Cpu_Attack_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Humanoid Worm Drake | face-up Attack Position |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Battle Ox**) is empty.
- `you.lp = 7500`: LP of you = 7500.

## Joey-Pegasus

### Scapegoat

- **Usage:** 36.8% weighted inclusion in `usage/joey-pegasus.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SCAPEGOAT`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** Special Summon 4 "Sheep Tokens" (Beast/EARTH/Level 1/ATK 0/DEF 0) in Defense Position. They cannot be Tributed for a Tribute Summon. You cannot Summon other monsters the turn you activate this card (but you can Normal Set).
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — dedicated logic** — Monster menu: Conditional AiWants* / AiShouldActivateAddedCard Main Phase Spell: Conditional AiWants* / AiShouldActivateAddedCard Set/chain: Conditional AiWants* / AiShouldActivateAddedCard

#### Player 1: Four sheep no summon

`Scapegoat_Four_Sheep_No_Summon_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The 4 Sheep Tokens are in Defense Position.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Scapegoat |  |
| You | Hand 1 | Battle Ox |  |

**Steps**

1. `activate you.hand[Scapegoat]`: Open the card menu of **Scapegoat** in your hand and choose Activate.

**Expected**

- `you.monsters = Sheep Token, Sheep Token, Sheep Token, Sheep Token`: your monsters is exactly Sheep Token, Sheep Token, Sheep Token, Sheep Token.
- `menu you.hand[Battle Ox] lacks Summon`: The card menu of **Battle Ox** in your hand does not offer Summon.
- `menu you.hand[Battle Ox] has Set`: The card menu of **Battle Ox** in your hand offers Set.

#### CPU 1: Block direct attack

`Scapegoat_Cpu_Block_Direct_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Scapegoat | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> direct`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent directly.
3. `when replay: cancel`: When the attack is replayed (its target left the field): cancel (B): do not attack again / make no choice.

**Expected**

- `opp.monsters = Sheep Token, Sheep Token, Sheep Token, Sheep Token`: the opponent's monsters is exactly Sheep Token, Sheep Token, Sheep Token, Sheep Token.
- `opp.lp = 8000`: LP of opp = 8000.
- `opp.gy = Scapegoat`: the opponent's Graveyard is exactly Scapegoat.

### Offerings to the Doomed

- **Usage:** 10.5% weighted inclusion in `usage/joey-pegasus.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_OFFERINGS_TO_THE_DOOMED`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** Target 1 face-up monster on the field; destroy that target, also skip your next Draw Phase.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroy and skip draw

`Offerings_to_the_Doomed_Destroy_And_Skip_Draw_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Your next Draw Phase is skipped: no Draw is offered, and the hand stays empty.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Offerings to the Doomed |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.hand[Offerings to the Doomed]`: Open the card menu of **Offerings to the Doomed** in your hand and choose Activate.
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.
3. `check opp.m0 = empty`: Check: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
4. `pass`: End your turn (its End Phase effects resolve).
5. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).

**Expected**

- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.
- `you.gy = Offerings to the Doomed`: your Graveyard is exactly Offerings to the Doomed.
- `you.hand count 0`: your hand holds 0 card(s).

#### CPU 1: Destroy

`Offerings_to_the_Doomed_Cpu_Destroy_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Offerings to the Doomed |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy has Offerings to the Doomed`: the opponent's Graveyard includes Offerings to the Doomed.

## Fiber

### Exiled Force

- **Usage:** 100.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_EXILED_FORCE`, `gAddedCardEffects`)
- **Coded behavior:** "You can Tribute this card, then target 1 monster on the field; destroy that target." The card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no once-per-turn limit: the card is gone once used. The target is any other monster (Man-Eater Bug's EffectTargetableMonsterChainB, with a wrapper that refuses Exiled Force itself). The Tribute is paid when the target is chosen rather than in chainA: the target step calls CanActivateEffect, and the prepare handler fails once the card has left its zone. The destruction is EffectDestroyTargetResolve's.
- **YGOPRODeck (TCG) text:** You can Tribute this card to target 1 monster on the field; destroy that target.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — Monster menu: only for a human monster stronger than itself Set/chain: only for a human monster stronger than itself

#### Player 1: Tribute destroy target

`Exiled_Force_Tribute_Destroy_Target_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Exiled Force | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.m0`: Open the card menu of your Monster Zone 0 (**Exiled Force**) and activate its effect.
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Exiled Force**) is empty.
- `you.gy = Exiled Force`: your Graveyard is exactly Exiled Force.
- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.

#### CPU 1: Tribute destroy stronger

`Exiled_Force_Cpu_Tribute_Destroy_Stronger_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Exiled Force | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.gy = Summoned Skull`: your Graveyard is exactly Summoned Skull.
- `opp.gy has Exiled Force`: the opponent's Graveyard includes Exiled Force.

### Fiber Jar

- **Usage:** 100.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_FIBER_JAR`, `gAddedCardEffects`)
- **Coded behavior:** FLIP: "Each player shuffles all cards from their hand, field, and Graveyard into the Deck, then draws 5 cards." (card_detail.c HasFlipEffect lists it.) For each player, all queued (deferred) commands: field     ReturnFieldCardToDeck on every zone, the Spell/Trap and Field zones first (an equip goes back before its monster's DestroyLinkedCards could send it to the Graveyard; DuelCmd_ReturnToDeck skips an emptied zone). Tokens are destroyed, as ReturnFieldCardToDeck does. hand      DUEL_CMD_RETURN_HAND_CARD_TO_DECK, last card first (the indices stay valid) Graveyard its Fusion Monsters to the Fusion Deck at once (TakeGraveyardCardAt, AddCardToFusionDeck), then DUEL_CMD_RETURN_GRAVEYARD_TO_DECK (the rest, with anything the field sent there; it would put a Fusion Monster in the Deck). A field Fusion Monster goes to the Fusion Deck (DuelCmd_ReturnToDeck). then each Deck is shuffled and each player draws 5 (DrawCards). Fiber Jar itself goes back with the field.
- **YGOPRODeck (TCG) text:** FLIP: Each player shuffles all cards from their hand, field, and Graveyard into the Deck, then draws 5 cards.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Flip reset both players

`Fiber_Jar_Flip_Reset_Both_Players_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Fiber Jar | face-down (Set) |
| You | Monster Zone 1 | Battle Ox | face-up Attack Position |
| You | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| You | Hand 0 | Summoned Skull |  |
| You | Hand 1 | Pot of Greed |  |
| You | Graveyard | La Jinn the Mystical Genie of the Lamp |  |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| Opponent | Hand 0 | Fissure |  |
| Opponent | Hand 1 | Dark Hole |  |
| Opponent | Hand 2 | Man-Eater Bug |  |
| Opponent | Graveyard | Sangan |  |

**Steps**

1. `flip_summon you.m0`: Flip Summon your Monster Zone 0 (**Fiber Jar**).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `you.spells count 0`: your Spells/Traps holds 0 card(s).
- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `you.gy count 0`: your Graveyard holds 0 card(s).
- `opp.gy count 0`: the opponent's Graveyard holds 0 card(s).
- `you.hand count 5`: your hand holds 5 card(s).
- `opp.hand count 5`: the opponent's hand holds 5 card(s).

#### CPU 1: Flipped by attack

`Fiber_Jar_Cpu_Flipped_By_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

Fiber Jar is flipped by the attack (DEF 500) and destroyed; its FLIP effect still resolves.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| You | Hand 0 | Fissure |  |
| Opponent | Monster Zone 0 | Fiber Jar | face-down (Set) |
| Opponent | Hand 0 | Dark Hole |  |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent's Monster Zone 0 (**Fiber Jar**).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `you.hand count 5`: your hand holds 5 card(s).
- `opp.hand count 5`: the opponent's hand holds 5 card(s).

### Injection Fairy Lily

- **Usage:** 100.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_INJECTION_FAIRY_LILY`, `gAddedCardEffects`)
- **Coded behavior:** "During either player's Damage Step, when this card attacks or is attacked: You can pay 2000 LP; this card gains 3000 ATK during the Damage Step only." EDS offers no face-up monster in a response window (only Set Spell/Trap cards and the hand), so, like D.D. Warrior Lady, the effect lives in the battle code: BattleStage_DamageCalc, DAMAGE_STEP_MARK_ATTACKER   InjectionFairyLilyResetBattle BattleStage_DamageCalc, DAMAGE_STEP_CALC            InjectionFairyLilyDamageStep, before CalcBattle: the attacker, then the target, when it is a face-up Lily whose controller has more than 2000 LP. The human is asked (PROMPT_CONFIRM_CARD_EFFECT); the CPU pays when the 3000 ATK turn the battle (or for a direct attack, with more than 4000 LP). Paying queues DUEL_CMD_LOSE_LP (a cost) and a ZONE_LINK_ATK_BONUS link of 3000; the stage waits a frame so that CalcBattle sees it. BattleStage_EndAttack                               InjectionFairyLilyEndAttack: the 3000 link goes again. The state is gInjectionFairyLily (EWRAM 0x0202FF00, symbols.ld).
- **YGOPRODeck (TCG) text:** If this card attacks or is attacked, during damage calculation (in either player's turn): You can pay 2000 LP once per battle; this card gains 3000 ATK during that damage calculation only.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `CpuWantsLily`

#### Player 1: Pay 2000 for 3000

`Injection_Fairy_Lily_Pay_2000_For_3000_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Injection Fairy Lily | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Injection Fairy Lily**): target the opponent's Monster Zone 0 (**Summoned Skull**).
3. `yes`: Answer Yes.

**Expected**

- `you.lp = 6000`: LP of you = 6000.
- `opp.lp = 7100`: LP of opp = 7100.
- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `you.m0.atk = 400`: The face-up Attack Position of your Monster Zone 0 (**Injection Fairy Lily**) is 400.

#### CPU 1: Pay when attacked

`Injection_Fairy_Lily_Cpu_Pay_When_Attacked_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 0 | Injection Fairy Lily | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent's Monster Zone 0 (**Injection Fairy Lily**).

**Expected**

- `opp.lp = 6000`: LP of opp = 6000.
- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.lp = 7100`: LP of you = 7100.
- `opp.m0 = Injection Fairy Lily atk`: the opponent's Monster Zone 0 (**Injection Fairy Lily**) holds Injection Fairy Lily, face-up Attack Position.

### Yata-Garasu

- **Usage:** 100.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_YATA_GARASU`, `gAddedCardEffects`)
- **Coded behavior:** "This card cannot be Special Summoned. This card returns to its owner's hand during the End Phase of the turn it is Normal Summoned or flipped face-up. When this card inflicts Battle Damage to your opponent, they skip their next Draw Phase." IsCannotSpecialSummon lists it (QueueSpecialSummon* refuse it; CanReviveGraveyardCard's IsSpecialSummonOnly path does not apply, but the summon itself is refused). EndPhase_ReturnSpiritMonsters (DuelPhase_End, battle_phase3.c; above): every face-up Yata-Garasu on the field goes back to its owner's hand. A face-up one at an End Phase was Normal Summoned or flipped face up that turn (it always leaves at the End Phase, and it cannot be Special Summoned), so no record of the turn is needed. Battle damage (BattleStage_InflictDamage steps 0 and 2, battle_phase2.c): DUEL_CMD_SKIP_NEXT_DRAW_PHASE (Time Seal's command) for the damaged player.
- **YGOPRODeck (TCG) text:** This card cannot be Special Summoned. This card returns to its owner's hand during the End Phase of the turn it is Normal Summoned or flipped face-up. When this card inflicts Battle Damage to your opponent, they skip their next Draw Phase.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Skip opponent draw

`Yata_Garasu_Skip_Opponent_Draw_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The opponent skips its next Draw Phase, so its hand stays at 1 card.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Yata-Garasu |  |
| Opponent | Hand 0 | Fissure |  |

**Steps**

1. `summon you.hand[Yata-Garasu] -> m0 atk`: Normal Summon **Yata-Garasu** in your hand into your Monster Zone 0 (face-up Attack Position).
2. `battle`: Enter the Battle Phase.
3. `attack you.m0 -> direct`: Attack with your Monster Zone 0: target the opponent directly.
4. `check opp.lp = 7800`: Check: LP of opp = 7800.
5. `pass`: End your turn (its End Phase effects resolve).
6. `check you.hand = Yata-Garasu`: Check: your hand is exactly Yata-Garasu.
7. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `opp.hand = Fissure`: the opponent's hand is exactly Fissure.

#### CPU 1: Skip human draw

`Yata_Garasu_Cpu_Skip_Human_Draw_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

Your next Draw Phase is skipped: no Draw is offered at the start of your turn.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Yata-Garasu |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.lp = 7800`: LP of you = 7800.
- `opp.hand = Yata-Garasu`: the opponent's hand is exactly Yata-Garasu.
- `you.hand count 0`: your hand holds 0 card(s).

### Creature Swap

- **Usage:** 73.4% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CREATURE_SWAP`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Each player chooses 1 monster they control and switches control of those monsters with each other. Those monsters cannot change their battle positions for the rest of this turn." Nothing is targeted: the choices are made at resolution. The human picks one of their monsters with the cursor (as in EffectGainOpponentMonsterStatsResolve), the CPU gives its weakest (AiFindWeakestMonster). The swap is Invader of the Throne's (SwapFieldCards: DUEL_CMD_SWAP_ZONES and the control-change rules of the swapped cards), then both monsters are locked in their position (DUEL_CMD_SET_POSITION_LOCKED; DuelCmd_TurnStart clears the flag at the start of their controller's next turn).
- **YGOPRODeck (TCG) text:** Each player chooses 1 monster they control and switches control of those monsters with each other. Those monsters cannot change their battle positions for the rest of this turn.
- **AI coverage:** **Yes — generalized routine** — Monster menu: give the weakest monster for a stronger one Main Phase Spell: give the weakest monster for a stronger one Set/chain: give the weakest monster for a stronger one

#### Player 1: Swap control

`Creature_Swap_Swap_Control_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The CPU gives its weakest monster (Battle Ox).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Creature Swap |  |
| You | Monster Zone 0 | Mystic Tomato | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 1 | Battle Ox | face-up Attack Position |

**Steps**

1. `activate you.hand[Creature Swap]`: Open the card menu of **Creature Swap** in your hand and choose Activate.
2. `pick you.m0`: Pick your Monster Zone 0 (**Mystic Tomato**) in the list or on the field.

**Expected**

- `you.monsters = Battle Ox`: your monsters is exactly Battle Ox.
- `opp.monsters = Summoned Skull, Mystic Tomato`: the opponent's monsters is exactly Summoned Skull, Mystic Tomato.
- `menu you.m[Battle Ox] lacks Change Position`: The card menu of your **Battle Ox** does not offer Change Position.

#### CPU 1: Swap weakest

`Creature_Swap_Cpu_Swap_Weakest_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Creature Swap |  |
| Opponent | Monster Zone 0 | Mystic Tomato | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.monsters has Summoned Skull`: the opponent's monsters includes Summoned Skull.
- `opp.gy has Creature Swap`: the opponent's Graveyard includes Creature Swap.

### Asura Priest

- **Usage:** 72.0% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_ASURA_PRIEST`, `gAddedCardEffects`)
- **Coded behavior:** "This card can attack all monsters your opponent controls once each." (Its Spirit rule and "Cannot be Special Summoned" are with Yata-Garasu's: EndPhase_ReturnSpiritMonsters, IsCannotSpecialSummon.) record   for the Battle Phase of turn gModDuel.asuraTurn - 1 (set, and the record emptied, as each Battle Phase starts: ModBattlePhaseStart): per zone of the attacking player, the monsters it attacked, as the opponent's zone (asuraTargets, a bit per zone) and that monster's DuelZone.serial there (asuraSerials; gBattle.zoneSerialSnapshot holds the target's at the declaration). The serial is stamped when a card is placed and kept when it is flipped, so a monster that came into the zone later (recruited by Mystic Tomato, Special Summoned, ...) is a new one it may attack again    ModEndAttack: after an attack on a monster (not a direct one) by a face-up Asura Priest still in its zone, the target is recorded, and while the opponent controls a monster it has not attacked, its attack mark (attackedMask) is cleared so it may attack again (as Black Luster Soldier's second attack does) targets  ModAttackTargetBlocked: the human's target pick (BattleStage_SelectTarget, card_menu_input.c) and the CPU's attack plans (AiFindAttackTarget, ai_picks.c) refuse a monster it has attacked; with the mark still set after its last monster, it cannot follow with a direct attack, and after a direct attack it is done
- **YGOPRODeck (TCG) text:** Cannot be Special Summoned. During the End Phase of the turn this card is Normal Summoned or flipped face-up: Return it to the hand. This card can attack all monsters your opponent controls once each.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Attack every monster

`Asura_Priest_Attack_Every_Monster_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Only the attack on Kuriboh (300 ATK) deals damage: 1400. Asura Priest returns to the hand at the End Phase.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Asura Priest | face-up Attack Position |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Defense Position |
| Opponent | Monster Zone 1 | Battle Ox | face-up Defense Position |
| Opponent | Monster Zone 2 | Kuriboh | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Asura Priest**): target the opponent's Monster Zone 0 (**Celtic Guardian**).
3. `attack you.m0 -> opp.m1`: Attack with your Monster Zone 0 (**Asura Priest**): target the opponent's Monster Zone 1 (**Battle Ox**).
4. `attack you.m0 -> opp.m2`: Attack with your Monster Zone 0 (**Asura Priest**): target the opponent's Monster Zone 2 (**Kuriboh**).
5. `check cannot attack you.m0 -> direct`: Check: your Monster Zone 0 (**Asura Priest**) cannot attack the opponent directly.
6. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `opp.lp = 6600`: LP of opp = 6600.
- `you.hand = Asura Priest`: your hand is exactly Asura Priest.

#### CPU 1: Attack every monster

`Asura_Priest_Cpu_Attack_Every_Monster_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Asura Priest | face-up Attack Position |
| You | Monster Zone 0 | Celtic Guardian | face-up Defense Position |
| You | Monster Zone 1 | Battle Ox | face-up Defense Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `opp.hand has Asura Priest`: the opponent's hand includes Asura Priest.

### Spear Dragon

- **Usage:** 25.3% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SPEAR_DRAGON`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** During battle between this attacking card and a Defense Position monster whose DEF is lower than the ATK of this card, inflict the difference as Battle Damage to your opponent. If this card attacks, it is changed to Defense Position at the end of the Damage Step.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Pierce then defense

`Spear_Dragon_Pierce_Then_Defense_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Spear Dragon | face-up Attack Position |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Defense Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Spear Dragon**): target the opponent's Monster Zone 0 (**Celtic Guardian**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Celtic Guardian**) is empty.
- `opp.lp = 7300`: LP of opp = 7300.
- `you.m0 = Spear Dragon def`: your Monster Zone 0 (**Spear Dragon**) holds Spear Dragon, face-up Defense Position.

#### CPU 1: Pierce

`Spear_Dragon_Cpu_Pierce_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Spear Dragon | face-up Attack Position |
| You | Monster Zone 0 | Celtic Guardian | face-up Defense Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Celtic Guardian**) is empty.
- `you.lp = 7300`: LP of you = 7300.
- `opp.m0 = Spear Dragon def`: the opponent's Monster Zone 0 (**Spear Dragon**) holds Spear Dragon, face-up Defense Position.

### Dark Ruler Ha Des

- **Usage:** 15.6% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_DARK_RULER_HA_DES`, `gAddedCardEffects`)
- **Coded behavior:** "Cannot be Special Summoned from the GY. Negate the effects of monsters destroyed by battle with Fiend monsters you control." CanReviveGraveyardCard (duel_ritual.c) refuses it: the revival cards cannot pick it. A monster destroyed in battle by a Fiend whose controller has a face-up Ha Des (HaDesNegatesBattleDestroyed): SendBattleDestroyedCardToGraveyard (duel_field_moves.c) queues none of its "destroyed in battle" effects (Sangan, Witch of the Black Forest, Mystic Tomato, ...), and a face-down one's Flip effect is not activated (HaDesNegatesBattleFlipEffect, beside Blade Knight's in battle_phase1.c / battle_phase2.c).
- **YGOPRODeck (TCG) text:** Cannot be Special Summoned from the GY. Negate the effects of monsters destroyed by battle with Fiend monsters you control.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Negate battle destroyed effects

`Dark_Ruler_Ha_Des_Negate_Battle_Destroyed_Effects_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Sangan's search and Man-Eater Bug's FLIP effect do not activate (both destroyed by Fiends).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Dark Ruler Ha Des | face-up Attack Position |
| You | Monster Zone 1 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| Opponent | Monster Zone 0 | Sangan | face-up Attack Position |
| Opponent | Monster Zone 1 | Man-Eater Bug | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m1 -> opp.m0`: Attack with your Monster Zone 1 (**La Jinn the Mystical Genie of the Lamp**): target the opponent's Monster Zone 0 (**Sangan**).
3. `attack you.m0 -> opp.m1`: Attack with your Monster Zone 0 (**Dark Ruler Ha Des**): target the opponent's Monster Zone 1 (**Man-Eater Bug**).

**Expected**

- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `you.m0 = Dark Ruler Ha Des atk`: your Monster Zone 0 (**Dark Ruler Ha Des**) holds Dark Ruler Ha Des, face-up Attack Position.
- `you.m1 = La Jinn the Mystical Genie of the Lamp atk`: your Monster Zone 1 (**La Jinn the Mystical Genie of the Lamp**) holds La Jinn the Mystical Genie of the Lamp, face-up Attack Position.
- `opp.lp = 7200`: LP of opp = 7200.
- `opp.hand count 0`: the opponent's hand holds 0 card(s).

#### CPU 1: Negate sangan

`Dark_Ruler_Ha_Des_Cpu_Negate_Sangan_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Dark Ruler Ha Des | face-up Attack Position |
| You | Monster Zone 0 | Sangan | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Sangan**) is empty.
- `you.lp = 6550`: LP of you = 6550.
- `you.hand count 0`: your hand holds 0 card(s).

### Airknight Parshath

- **Usage:** 14.5% weighted inclusion in `usage/fiber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_AIRKNIGHT_PARSHATH`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** If this card attacks a Defense Position monster, inflict piercing battle damage. If this card inflicts battle damage to your opponent: Draw 1 card.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Pierce and draw

`Airknight_Parshath_Pierce_And_Draw_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Airknight Parshath | face-up Attack Position |
| You | Deck 0 | Battle Ox |  |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Defense Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Airknight Parshath**): target the opponent's Monster Zone 0 (**Celtic Guardian**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Celtic Guardian**) is empty.
- `opp.lp = 7300`: LP of opp = 7300.
- `you.hand = Battle Ox`: your hand is exactly Battle Ox.

#### CPU 1: Pierce and draw

`Airknight_Parshath_Cpu_Pierce_And_Draw_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Airknight Parshath | face-up Attack Position |
| Opponent | Deck 0 | Battle Ox |  |
| You | Monster Zone 0 | Celtic Guardian | face-up Defense Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.lp = 7300`: LP of you = 7300.
- `you.m0 = empty`: your Monster Zone 0 (**Celtic Guardian**) is empty.

## Yata

### Mirage of Nightmare

- **Usage:** 100.0% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MIRAGE_OF_NIGHTMARE`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn, during your opponent's Standby Phase: Draw cards until you have 4 cards in your hand. If you do, during your next Standby Phase after that: Randomly discard the same number of cards you drew (or your entire hand, if you do not have enough cards)." (Continuous Spell) Its activation does nothing (EffectMirageOfNightmareResolve; a Continuous Spell stays on the field). The rest runs in ModStandbyStep, in every Standby Phase (`player` is the turn player), on gDuel.phaseSubStep: discard  the turn player's draws from their last Mirage draw (gModDuel.mirageDrawn) are discarded at random, one per step, while a face-up Mirage of Nightmare is still on their field (destroyed in between, there is nothing to discard); the count is cleared either way draw     the other player, with a face-up Mirage of Nightmare and fewer than 4 cards in hand, draws up to 4 (mandatory, nobody is asked); the number drawn is kept for their next Standby Phase
- **YGOPRODeck (TCG) text:** Once per turn, during your opponent's Standby Phase: Draw cards until you have 4 cards in your hand. If you do, during your next Standby Phase after that: Randomly discard the same number of cards you drew (or your entire hand, if you do not have enough cards).
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Draw to four then discard

`Mirage_of_Nightmare_Draw_To_Four_Then_Discard_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

In the opponent's Standby Phase you draw up to 4 (3 cards); after your own draw (Fissure, 5 cards), your Standby Phase discards 3 at random.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Mirage of Nightmare |  |
| You | Hand 1 | Battle Ox |  |
| You | Deck 0 | Celtic Guardian |  |
| You | Deck 1 | La Jinn the Mystical Genie of the Lamp |  |
| You | Deck 2 | Sangan |  |
| You | Deck 3 | Fissure |  |

**Steps**

1. `activate you.hand[Mirage of Nightmare]`: Open the card menu of **Mirage of Nightmare** in your hand and choose Activate.
2. `pass`: End your turn (its End Phase effects resolve).
3. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).
4. `check you.hand = Battle Ox, Celtic Guardian, La Jinn the Mystical Genie of the Lamp, Sangan`: Check: your hand is exactly Battle Ox, Celtic Guardian, La Jinn the Mystical Genie of the Lamp, Sangan.
5. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.

**Expected**

- `you.hand count 2`: your hand holds 2 card(s).
- `you.gy count 3`: your Graveyard holds 3 card(s).
- `you.spells has Mirage of Nightmare`: your Spells/Traps includes Mirage of Nightmare.

#### CPU 1: Draw to four

`Mirage_of_Nightmare_Cpu_Draw_To_Four_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

The CPU draws up to 4 in your Standby Phase (after your own draw).

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Mirage of Nightmare | face up |
| Opponent | Hand 0 | Fissure |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).
2. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.

**Expected**

- `opp.hand count 4`: the opponent's hand holds 4 card(s).

### Spirit Reaper

- **Usage:** 87.2% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SPIRIT_REAPER`, `gAddedCardEffects`)
- **Coded behavior:** "Cannot be destroyed by battle. After this card is targeted by an effect, destroy this card. When this card inflicts battle damage to your opponent: Discard 1 random card from their hand." CalcBattle (campaign.c)                   its destroyed flag is cleared, as attacker or target TryAddEffectTarget (effect_targets1.c)     OnFieldCardTargeted: a face-up Spirit Reaper that an effect targets is destroyed at once (its own effect, so the targeting effect finds it gone) BattleStage_InflictDamage (battle_phase2.c) White Magical Hat's case: the opponent discards 1 random card TryAddEffectTarget also records the picks of a few effects that Tribute rather than target (Catapult Turtle's EffectTributeTargetChainB and Metamorphosis); those do not count.
- **YGOPRODeck (TCG) text:** Cannot be destroyed by battle. After resolving a card effect that targets this face-up card, destroy this card. When this card inflicts battle damage to your opponent by a direct attack: Discard 1 random card from their hand.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Discard on direct attack

`Spirit_Reaper_Discard_On_Direct_Attack_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Spirit Reaper | face-up Attack Position |
| Opponent | Hand 0 | Fissure |  |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> direct`: Attack with your Monster Zone 0 (**Spirit Reaper**): target the opponent directly.

**Expected**

- `opp.lp = 7700`: LP of opp = 7700.
- `opp.hand count 0`: the opponent's hand holds 0 card(s).
- `opp.gy = Fissure`: the opponent's Graveyard is exactly Fissure.

#### Player 2: Survive battle

`Spirit_Reaper_Survive_Battle_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Spirit Reaper | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Spirit Reaper**): target the opponent's Monster Zone 0 (**Summoned Skull**).

**Expected**

- `you.m0 = Spirit Reaper atk`: your Monster Zone 0 (**Spirit Reaper**) holds Spirit Reaper, face-up Attack Position.
- `you.lp = 5800`: LP of you = 5800.

#### CPU 1: Discard human card

`Spirit_Reaper_Cpu_Discard_Human_Card_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Spirit Reaper | face-up Attack Position |
| You | Hand 0 | Fissure |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.lp = 7700`: LP of you = 7700.
- `you.hand count 0`: your hand holds 0 card(s).
- `you.gy = Fissure`: your Graveyard is exactly Fissure.

### Don Zaloog

- **Usage:** 74.4% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_DON_ZALOOG`, `gAddedCardEffects`)
- **Coded behavior:** "When this card inflicts battle damage to your opponent: You can activate 1 of these effects. - Discard 1 random card from your opponent's hand. - Send the top 2 cards of your opponent's Deck to the GY." BattleStage_InflictDamage (battle_phase2.c) queues its battle-damage trigger like Kycoo's: as the attacker (RESPONSE_BATTLE_DAMAGE) and as the target when the attacker's player takes the damage (RESPONSE_BATTLE_DEFLECTED_DAMAGE). The low nibble of loc1 is the damaged player. Pending triggers are not checked by a prepare handler, so the resolve checks everything and asks: the human gets Yes/No for the discard, then for the Deck; the CPU discards if it can, else sends the Deck cards. The discard is White Magical Hat's (DuelPrompt_PostRandomDiscard, by an opponent's effect); the Deck cards go one by one (DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD, the top card is deck[0]).
- **YGOPRODeck (TCG) text:** When this card inflicts battle damage to your opponent: You can activate 1 of these effects;
● Discard 1 random card from their hand.
● Send the top 2 cards of their Deck to the GY.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Discard random

`Don_Zaloog_Discard_Random_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Don Zaloog | face-up Attack Position |
| Opponent | Hand 0 | Fissure |  |
| Opponent | Hand 1 | Trap Hole |  |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> direct`: Attack with your Monster Zone 0 (**Don Zaloog**): target the opponent directly.
3. `yes`: Answer Yes.

**Expected**

- `opp.lp = 6600`: LP of opp = 6600.
- `opp.hand count 1`: the opponent's hand holds 1 card(s).
- `opp.gy count 1`: the opponent's Graveyard holds 1 card(s).

#### Player 2: Send top two

`Don_Zaloog_Send_Top_Two_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Don Zaloog | face-up Attack Position |
| Opponent | Hand 0 | Fissure |  |
| Opponent | Hand 1 | Trap Hole |  |
| Opponent | Deck 0 | Summoned Skull |  |
| Opponent | Deck 1 | Battle Ox |  |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> direct`: Attack with your Monster Zone 0 (**Don Zaloog**): target the opponent directly.
3. `no`: Answer No.
4. `yes`: Answer Yes.

**Expected**

- `opp.lp = 6600`: LP of opp = 6600.
- `opp.hand count 2`: the opponent's hand holds 2 card(s).
- `opp.gy = Summoned Skull, Battle Ox`: the opponent's Graveyard is exactly Summoned Skull, Battle Ox.

#### CPU 1: Discard random

`Don_Zaloog_Cpu_Discard_Random_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Don Zaloog | face-up Attack Position |
| You | Hand 0 | Fissure |  |
| You | Hand 1 | Trap Hole |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.lp = 6600`: LP of you = 6600.
- `you.hand count 1`: your hand holds 1 card(s).

### Reinforcement of the Army

- **Usage:** 73.1% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_REINFORCEMENT_OF_THE_ARMY`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Add 1 Level 4 or lower Warrior monster from your Deck to your hand." Sangan's resolve (EffectSanganResolve) does the search: it lists CollectEffectTargets (effect_target_collect.c: IsReinforcementTarget) for the card number, lets the human pick (the CPU: AiPickCardListEntry), adds the card and shuffles. The prepare handler needs such a monster in the deck, so the Spell is not wasted.
- **YGOPRODeck (TCG) text:** Add 1 Level 4 or lower Warrior monster from your Deck to your hand.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Search warrior

`Reinforcement_of_the_Army_Search_Warrior_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Reinforcement of the Army |  |
| You | Deck 0 | Battle Ox |  |
| You | Deck 1 | Celtic Guardian |  |
| You | Deck 2 | Summoned Skull |  |

**Steps**

1. `activate you.hand[Reinforcement of the Army]`: Open the card menu of **Reinforcement of the Army** in your hand and choose Activate.
2. `pick you.deck[Celtic Guardian]`: Pick **Celtic Guardian** in your Deck in the list or on the field.

**Expected**

- `you.hand = Celtic Guardian`: your hand is exactly Celtic Guardian.
- `you.gy = Reinforcement of the Army`: your Graveyard is exactly Reinforcement of the Army.

#### CPU 1: Search warrior

`Reinforcement_of_the_Army_Cpu_Search_Warrior_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Reinforcement of the Army |  |
| Opponent | Deck 0 | Celtic Guardian |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Reinforcement of the Army`: the opponent's Graveyard includes Reinforcement of the Army.

### Gravekeeper's Spy

- **Usage:** 60.3% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_GRAVEKEEPERS_SPY`, `gAddedCardEffects`)
- **Coded behavior:** "FLIP: Special Summon 1 'Gravekeeper's' monster with 1500 or less ATK from your Deck." A flip effect (HasFlipEffect, card_detail.c), resolved by the floaters' handler EffectSpecialSummonFromDeckResolve: it asks the human, lists the candidates (CollectEffectTargets, effect_target_collect.c: deck "Gravekeeper's" monsters with ATK 1500 or less, not Fusion or Ritual), Special Summons the pick in Attack Position and shuffles.
- **YGOPRODeck (TCG) text:** FLIP: Special Summon 1 "Gravekeeper's" monster with 1500 or less ATK from your Deck.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Flip summon gravekeeper

`Gravekeepers_Spy_Flip_Summon_Gravekeeper_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Gravekeeper's Spy is the only Gravekeeper's monster in the ROM.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Gravekeeper's Spy | face-down (Set) |
| You | Deck 0 | Gravekeeper's Spy |  |
| You | Deck 1 | Battle Ox |  |

**Steps**

1. `flip_summon you.m0`: Flip Summon your Monster Zone 0 (**Gravekeeper's Spy**).
2. `yes`: Answer Yes.
3. `pick you.deck[Gravekeeper's Spy]`: Pick **Gravekeeper's Spy** in your Deck in the list or on the field.
4. `zone m1`: Choose your Monster Zone 1 for the summoned monster.

**Expected**

- `you.m0 = Gravekeeper's Spy atk`: your Monster Zone 0 (**Gravekeeper's Spy**) holds Gravekeeper's Spy, face-up Attack Position.
- `you.m1 = Gravekeeper's Spy atk`: your Monster Zone 1 holds Gravekeeper's Spy, face-up Attack Position.

#### CPU 1: Flipped by attack

`Gravekeepers_Spy_Cpu_Flipped_By_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Gravekeeper's Spy | face-down (Set) |
| Opponent | Deck 0 | Gravekeeper's Spy |  |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Battle Ox**): target the opponent's Monster Zone 0 (**Gravekeeper's Spy**).

**Expected**

- `you.lp = 7700`: LP of you = 7700.
- `opp.monsters = Gravekeeper's Spy, Gravekeeper's Spy`: the opponent's monsters is exactly Gravekeeper's Spy, Gravekeeper's Spy.

### Book of Moon

- **Usage:** 34.6% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BOOK_OF_MOON`, `gAddedCardEffects`)
- **Coded behavior:** Book of Moon (Quick-Play Spell) and Tsukuyomi (FLIP) share the same effect: target 1 face-up monster on the field; change it to face-down Defense Position. Targeting uses EffectTargetableFaceUpMonsterCheck / ChainB (Chaos Sorcerer's pickers). The resolve first destroys the target's Equip Cards (they cannot equip a face-down monster; a destroyed Premature Burial takes the monster with it). Then DUEL_CMD_CHANGE_POSITION puts a monster in Attack Position into Defense Position, and DUEL_CMD_FLIP_CARD turns it face down, which removes its counters (OnMonsterTurnedFaceDown). Tsukuyomi is listed in HasFlipEffect (card_detail.c).
- **YGOPRODeck (TCG) text:** Target 1 face-up monster on the field; change that target to face-down Defense Position.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — Monster menu: clears the way for the CPU's attack Main Phase Spell: clears the way for the CPU's attack Set/chain: clears the way for the CPU's attack

#### Player 1: Face down target

`Book_of_Moon_Face_Down_Target_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Book of Moon |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.hand[Book of Moon]`: Open the card menu of **Book of Moon** in your hand and choose Activate.
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `opp.m0 = Summoned Skull set`: the opponent's Monster Zone 0 (**Summoned Skull**) holds Summoned Skull, face-down (Set).
- `you.gy = Book of Moon`: your Graveyard is exactly Book of Moon.

#### CPU 1: Clear the way

`Book_of_Moon_Cpu_Clear_The_Way_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

Book of Moon turns Summoned Skull face down (DEF 1200), then La Jinn (1800) destroys it.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Book of Moon |  |
| Opponent | Monster Zone 0 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Book of Moon`: the opponent's Graveyard includes Book of Moon.
- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.

### King Tiger Wanghu

- **Usage:** 34.6% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_KING_TIGER_WANGHU`, `gAddedCardEffects`)
- **Coded behavior:** "When a monster with 1400 or less ATK is Normal or Special Summoned: Destroy it." ExecuteSummonAction (summon_action.c) asks KingTigerWanghuOnSummon after a Normal Summon or a Special Summon, before the opponent's response window: a face-up summoned monster (a Set or face-down one shows no ATK) with 1400 or less ATK is destroyed while a face-up, non-negated King Tiger Wanghu is on either side of the field. Flip Summons do not count.
- **YGOPRODeck (TCG) text:** When a monster(s) with 1400 or less ATK is Normal Summoned or Special Summoned: Destroy those monsters with 1400 or less ATK. This card must be face-up on the field to activate and to resolve this effect.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroy weak summon

`King_Tiger_Wanghu_Destroy_Weak_Summon_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | King Tiger Wanghu | face-up Attack Position |
| You | Hand 0 | Mystic Tomato |  |

**Steps**

1. `summon you.hand[Mystic Tomato] -> m1 atk`: Normal Summon **Mystic Tomato** in your hand into your Monster Zone 1 (face-up Attack Position).

**Expected**

- `you.m1 = empty`: your Monster Zone 1 is empty.
- `you.gy = Mystic Tomato`: your Graveyard is exactly Mystic Tomato.
- `you.m0 = King Tiger Wanghu atk`: your Monster Zone 0 (**King Tiger Wanghu**) holds King Tiger Wanghu, face-up Attack Position.

#### CPU 1: Wanghu on human summon

`King_Tiger_Wanghu_Cpu_Wanghu_On_Human_Summon_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | King Tiger Wanghu | face-up Attack Position |
| You | Hand 0 | Mystic Tomato |  |

**Steps**

1. `summon you.hand[Mystic Tomato] -> m0 atk`: Normal Summon **Mystic Tomato** in your hand into your Monster Zone 0 (face-up Attack Position).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 is empty.
- `you.gy = Mystic Tomato`: your Graveyard is exactly Mystic Tomato.

### Newdoria

- **Usage:** 26.9% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_NEWDORIA`, `gAddedCardEffects`)
- **Coded behavior:** "If this card is destroyed by battle and sent to the GY: Target 1 monster on the field; destroy that target." Queued with Sangan's and Gigantes' triggers (SendBattleDestroyedCardToGraveyard, duel_field_moves.c: destroyed by battle and sent to the Graveyard, not banished). The target is any monster on the field (Man-Eater Bug's EffectTargetableMonsterCheck / ChainB; the CPU picks the human's strongest), destroyed by EffectDestroyTargetResolve.
- **YGOPRODeck (TCG) text:** If this card is destroyed by battle and sent to the GY: Target 1 monster on the field; destroy that target.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroy on battle destruction

`Newdoria_Destroy_On_Battle_Destruction_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Newdoria | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Newdoria**): target the opponent's Monster Zone 0 (**Summoned Skull**).
3. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `you.lp = 6700`: LP of you = 6700.
- `you.gy = Newdoria`: your Graveyard is exactly Newdoria.
- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.

#### CPU 1: Destroy strongest

`Newdoria_Cpu_Destroy_Strongest_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| You | Monster Zone 1 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Newdoria | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent's Monster Zone 0 (**Newdoria**).

**Expected**

- `opp.lp = 6700`: LP of opp = 6700.
- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.m1 = Battle Ox atk`: your Monster Zone 1 (**Battle Ox**) holds Battle Ox, face-up Attack Position.

### Metamorphosis

- **Usage:** 15.4% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_METAMORPHOSIS`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Tribute 1 monster. Special Summon 1 Fusion Monster from your Extra Deck with the same Level as the Tributed monster." Prepare: one of the player's monsters has the Level of a Fusion Monster in the Fusion Deck. ChainB picks it with Catapult Turtle's EffectTributeTargetChainB (the CPU: its highest-Level such monster), refuses a monster with no such Fusion, then Tributes it (the cost, paid once the pick is made, as Exiled Force's) and keeps its Level in targets[1]. Resolve: Cyber-Stein's steps with a list of the Fusion Monsters of that Level (CollectEffectTargets, effect_target_collect.c: IsMetamorphosisFusion with param = the Level); the CPU takes the highest ATK.
- **YGOPRODeck (TCG) text:** Tribute 1 monster. Special Summon 1 Fusion Monster from your Extra Deck with the same Level as the Tributed monster.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiCpuMetamorphosisAllowed` Dedicated `*Cpu*` handlers in the card's effect steps. Monster menu: Conditional AiWants* / AiShouldActivateAddedCard Main Phase Spell: Conditional AiWants* / AiShouldActivateAddedCard Set/chain: Conditional AiWants* / AiShouldActivateAddedCard

#### Player 1: Tribute for same level fusion

`Metamorphosis_Tribute_For_Same_Level_Fusion_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Metamorphosis |  |
| You | Monster Zone 0 | Dark Magician | face-up Attack Position |
| You | Fusion Deck | Thousand Dragon |  |
| You | Fusion Deck | Flame Swordsman |  |

**Steps**

1. `activate you.hand[Metamorphosis]`: Open the card menu of **Metamorphosis** in your hand and choose Activate.
2. `target you.m0`: Pick your Monster Zone 0 (**Dark Magician**) as the target.
3. `pick you.extra[Thousand Dragon]`: Pick **Thousand Dragon** in your Fusion Deck in the list or on the field.
4. `zone m0`: Choose your Monster Zone 0 (**Dark Magician**) for the summoned monster.
5. `position atk`: Choose face-up Attack Position for the summoned monster.

**Expected**

- `you.m0 = Thousand Dragon atk`: your Monster Zone 0 (**Dark Magician**) holds Thousand Dragon, face-up Attack Position.
- `you.gy = Metamorphosis, Dark Magician`: your Graveyard is exactly Metamorphosis, Dark Magician.
- `you.extra = Flame Swordsman`: your Fusion Deck is exactly Flame Swordsman.

#### CPU 1: Highest atk fusion

`Metamorphosis_Cpu_Highest_Atk_Fusion_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Metamorphosis |  |
| Opponent | Monster Zone 0 | Dark Magician | face-up Attack Position |
| Opponent | Fusion Deck | Thousand Dragon |  |
| Opponent | Fusion Deck | Gaia the Dragon Champion |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.monsters has Gaia the Dragon Champion`: the opponent's monsters includes Gaia the Dragon Champion.
- `opp.gy has Dark Magician`: the opponent's Graveyard includes Dark Magician.

### Cure Mermaid

- **Usage:** 12.8% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1441**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** +800 LP in each of your Standby Phases. Status in ROM: Full (passive).
- **YGOPRODeck (TCG) text:** As long as this card remains face-up on your side of the field, increase your Life Points by 800 points during each of your Standby Phases.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Unclear / likely missing** — No `CPU:` notes and no switch case found; treat as untested for CPU until you confirm in a duel.

#### Player 1: Gain 800 standby

`Cure_Mermaid_Gain_800_Standby_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Cure Mermaid | face-up Attack Position |

**Steps**

1. `pass`: End your turn (its End Phase effects resolve).
2. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).
3. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.

**Expected**

- `you.lp = 8800`: LP of you = 8800.

#### CPU 1: Gain 800 standby

`Cure_Mermaid_Cpu_Gain_800_Standby_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Cure Mermaid | face-up Attack Position |

**Steps**

1. `pass`: End your turn (its End Phase effects resolve).
2. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.lp >= 8800`: LP of opp >= 8800.

### Raigeki Break

- **Usage:** 11.5% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_RAIGEKI_BREAK`, `gAddedCardEffects`)
- **Coded behavior:** "Discard 1 card, then target 1 card on the field; destroy it." Normal Trap from the field (or hand): discard cost like Tribe-Infecting Virus / Magic Jammer (DuelPrompt_PostDiscardCost), target any card on either side (monsters and Spell/Trap zones, CanCardTargetZone), destroy with EffectDestroyTargetResolve.
- **YGOPRODeck (TCG) text:** Discard 1 card, then target 1 card on the field; destroy it.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Discard destroy card

`Raigeki_Break_Discard_Destroy_Card_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Raigeki Break | face-down (Set) |
| You | Hand 0 | Battle Ox |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Raigeki Break**) and choose Activate.
2. `discard you.hand[Battle Ox]`: Discard **Battle Ox** in your hand for the cost.
3. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.
- `you.gy = Raigeki Break, Battle Ox`: your Graveyard is exactly Raigeki Break, Battle Ox.

#### CPU 1: Discard destroy

`Raigeki_Break_Cpu_Discard_Destroy_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Raigeki Break | face-down (Set) |
| Opponent | Hand 0 | Fissure |  |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Raigeki Break`: the opponent's Graveyard includes Raigeki Break.
- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.

### Trap Dustshoot

- **Usage:** 11.5% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TRAP_DUSTSHOOT`, `gAddedCardEffects`)
- **Coded behavior:** "Activate only if your opponent has 4 or more cards in their hand. Look at your opponent's hand, select 1 Monster Card in it, and return that card to its owner's Deck." The pick is Confiscation's (PROMPT_PICK_OPPONENT_HAND_CARD: the opponent's hand is revealed and the cursor picks a card); a card that is not a monster plays the error sound and the prompt runs again. The CPU takes the first monster in the human's hand. The card goes to the bottom of the Deck (ReturnHandCardToDeck), which is then shuffled. With no monster in the opponent's hand nothing happens.
- **YGOPRODeck (TCG) text:** Activate only if your opponent has 4 or more cards in their hand. Look at your opponent's hand, select 1 Monster Card in it, and return that card to its owner's Deck.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Return hand monster

`Trap_Dustshoot_Return_Hand_Monster_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Trap Dustshoot | face-down (Set) |
| Opponent | Hand 0 | Summoned Skull |  |
| Opponent | Hand 1 | Fissure |  |
| Opponent | Hand 2 | Trap Hole |  |
| Opponent | Hand 3 | Dark Hole |  |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Trap Dustshoot**) and choose Activate.
2. `pick opp.hand[Summoned Skull]`: Pick **Summoned Skull** in the opponent's hand in the list or on the field.

**Expected**

- `opp.hand = Fissure, Trap Hole, Dark Hole`: the opponent's hand is exactly Fissure, Trap Hole, Dark Hole.
- `you.gy = Trap Dustshoot`: your Graveyard is exactly Trap Dustshoot.

#### CPU 1: Return first monster

`Trap_Dustshoot_Cpu_Return_First_Monster_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Trap Dustshoot | face-down (Set) |
| You | Hand 0 | Summoned Skull |  |
| You | Hand 1 | Fissure |  |
| You | Hand 2 | Trap Hole |  |
| You | Hand 3 | Dark Hole |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.hand = Fissure, Trap Hole, Dark Hole`: your hand is exactly Fissure, Trap Hole, Dark Hole.
- `opp.gy has Trap Dustshoot`: the opponent's Graveyard includes Trap Dustshoot.

### Blast with Chain

- **Usage:** 10.3% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BLAST_WITH_CHAIN`, `gAddedCardEffects`)
- **Coded behavior:** "Target 1 face-up monster you control; equip this card to that target. It gains 500 ATK. If this card is destroyed by a card effect while equipped: Target 1 card on the field; destroy that target." An Equip Trap, as Kunai with Chain (tools/newcards.py's SUBTYPE_OVERRIDES: subtype Equip, so it stays on the field, Chain_CardGoesToGrave). One row, told apart by the event: activation   the target is a face-up monster of its controller (the human's cursor; the CPU takes the attacked monster when it answers an attack on one of its own, else its strongest face-up one); the resolve equips it (EquipCard) as Kunai's does, or destroys the card when the target is gone. +500 ATK: CalcZoneCardStats' Equip case (card_stats.c), beside Kunai's. destroyed    OnCardDestroyedByEffect (effect_hooks.c) calls BlastWithChainDestroyed for every card an effect destroys: a face-up Blast with Chain that equips a monster (FindMonsterWithLinkTo) queues its trigger from the Graveyard (RESPONSE_TRAP_TO_GRAVE). An Equip lost with its monster is not destroyed by an effect, so it does not go off. The target is any card on the field (the CPU: the human's strongest monster, else a Spell/Trap of the human's); it is destroyed. The CPU activates it in answer to an attack while it has a face-up monster (AiShouldActivateAddedCard).
- **YGOPRODeck (TCG) text:** Target 1 face-up monster you control; equip this card to that target. It gains 500 ATK. If this card is destroyed by a card effect while equipped: Target 1 card on the field; destroy that target.
- **AI coverage:** **Yes — generalized routine** — Monster menu: against an attack, on a face-up monster of its own Main Phase Spell: against an attack, on a face-up monster of its own Set/chain: against an attack, on a face-up monster of its own

#### Player 1: Equip then destroy when destroyed

`Blast_with_Chain_Equip_Then_Destroy_When_Destroyed_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Blast with Chain | face-down (Set) |
| You | Hand 0 | Mystical Space Typhoon |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Blast with Chain**) and choose Activate.
2. `target you.m0`: Pick your Monster Zone 0 (**Battle Ox**) as the target.
3. `check you.m0.atk = 2200`: Check: The face-up Attack Position of your Monster Zone 0 (**Battle Ox**) is 2200.
4. `activate you.hand[Mystical Space Typhoon]`: Open the card menu of **Mystical Space Typhoon** in your hand and choose Activate.
5. `target you.st0`: Pick your Spell/Trap Zone 0 (**Blast with Chain**) as the target.
6. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.
- `you.gy = Blast with Chain, Mystical Space Typhoon`: your Graveyard is exactly Blast with Chain, Mystical Space Typhoon.
- `you.m0.atk = 1700`: The face-up Attack Position of your Monster Zone 0 (**Battle Ox**) is 1700.

#### CPU 1: Equip against attack

`Blast_with_Chain_Cpu_Equip_Against_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Blast with Chain | face-down (Set) |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**La Jinn the Mystical Genie of the Lamp**): target the opponent's Monster Zone 0 (**Battle Ox**).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**La Jinn the Mystical Genie of the Lamp**) is empty.
- `you.lp = 7600`: LP of you = 7600.
- `opp.m0 = Battle Ox atk`: the opponent's Monster Zone 0 (**Battle Ox**) holds Battle Ox, face-up Attack Position.
- `opp.m0.atk = 2200`: The face-up Attack Position of the opponent's Monster Zone 0 (**Battle Ox**) is 2200.

### Gearfried the Iron Knight

- **Usage:** 10.3% weighted inclusion in `usage/yata.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (`tools/newcards.py` `ENGINE_NUMBERS`)
- **Coded behavior:** Engine effect key 1351. key 1351: EquipCard destroys an Equip put on it (and the monster too for Premature Burial); Snatch Steal cannot target it
- **YGOPRODeck (TCG) text:** If either player equips an Equip Card(s) to this card: Destroy that Equip Card(s).
- **AI coverage:** **Generic engine** — Uses the vanilla effect key tables; the CPU plays it like other cards in its deck lists. No dedicated `AiWants*` row in `src/added_effects.c`—watch whether the opponent activates the effect when rules allow.

#### Player 1: Destroy equip

`Gearfried_the_Iron_Knight_Destroy_Equip_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Gearfried the Iron Knight | face-up Attack Position |
| You | Hand 0 | Mage Power |  |

**Steps**

1. `activate you.hand[Mage Power]`: Open the card menu of **Mage Power** in your hand and choose Activate.
2. `target you.m0`: Pick your Monster Zone 0 (**Gearfried the Iron Knight**) as the target.

**Expected**

- `you.gy = Mage Power`: your Graveyard is exactly Mage Power.
- `you.m0 = Gearfried the Iron Knight atk`: your Monster Zone 0 (**Gearfried the Iron Knight**) holds Gearfried the Iron Knight, face-up Attack Position.
- `you.m0.atk = 1800`: The face-up Attack Position of your Monster Zone 0 (**Gearfried the Iron Knight**) is 1800.

#### CPU 1: Attack

`Gearfried_the_Iron_Knight_Cpu_Attack_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Gearfried the Iron Knight | face-up Attack Position |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Celtic Guardian**) is empty.
- `you.lp = 7600`: LP of you = 7600.

## Scientist

### Breaker the Magical Warrior

- **Usage:** 100.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR`, `gAddedCardEffects`)
- **Coded behavior:** "When this card is Normal Summoned: Place 1 Spell Counter on it (max. 1). Gains 300 ATK for each Spell Counter on it. You can remove 1 Spell Counter from this card; target 1 Spell/Trap on the field; destroy that target." One row serves both effects; the link's event tells them apart: RESPONSE_SUMMONED   the trigger that ExecuteSummonAction queues when Breaker is Normal Summoned (loc0 is its own zone): place the counter RESPONSE_NONE       the activated effect from the card menu (Main Phase, CanActivateMonsterEffect): needs a counter; the cost removes it, then it destroys 1 Spell/Trap like Mystical Space Typhoon (EffectAnySpellTrapCheck, EffectSpellTrapTargetChainB, EffectDestroyTargetResolve)
- **YGOPRODeck (TCG) text:** If this card is Normal Summoned: Place 1 Spell Counter on it (max. 1). Gains 300 ATK for each Spell Counter on it. You can remove 1 Spell Counter from this card, then target 1 Spell/Trap on the field; destroy that target.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — Monster menu: the target step takes the human's cards first Set/chain: the target step takes the human's cards first

#### Player 1: Counter then destroy spell trap

`Breaker_the_Magical_Warrior_Counter_Then_Destroy_Spell_Trap_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Breaker the Magical Warrior |  |
| Opponent | Spell/Trap Zone 0 | Call of the Haunted | face-down (Set) |

**Steps**

1. `summon you.hand[Breaker the Magical Warrior] -> m0 atk`: Normal Summon **Breaker the Magical Warrior** in your hand into your Monster Zone 0 (face-up Attack Position).
2. `check you.m0.atk = 1900`: Check: The face-up Attack Position of your Monster Zone 0 is 1900.
3. `activate you.m0`: Open the card menu of your Monster Zone 0 and activate its effect.
4. `target opp.st0`: Pick the opponent's Spell/Trap Zone 0 (**Call of the Haunted**) as the target.

**Expected**

- `opp.st0 = empty`: the opponent's Spell/Trap Zone 0 (**Call of the Haunted**) is empty.
- `opp.gy = Call of the Haunted`: the opponent's Graveyard is exactly Call of the Haunted.
- `you.m0.atk = 1600`: The face-up Attack Position of your Monster Zone 0 is 1600.
- `menu you.m0 lacks Activate`: The card menu of your Monster Zone 0 does not offer Activate.

#### CPU 1: Destroy human spell trap

`Breaker_the_Magical_Warrior_Cpu_Destroy_Human_Spell_Trap_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Breaker the Magical Warrior |  |
| You | Spell/Trap Zone 0 | Call of the Haunted | face-down (Set) |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.st0 = empty`: your Spell/Trap Zone 0 (**Call of the Haunted**) is empty.
- `you.gy = Call of the Haunted`: your Graveyard is exactly Call of the Haunted.
- `opp.monsters has Breaker the Magical Warrior`: the opponent's monsters includes Breaker the Magical Warrior.

### Magical Scientist

- **Usage:** 100.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MAGICAL_SCIENTIST`, `gAddedCardEffects`)
- **Coded behavior:** "You can pay 1000 Life Points; Special Summon 1 Level 6 or lower Fusion Monster from your Extra Deck in face-up Attack or Defense Position. That Fusion Monster cannot attack your opponent directly, and is returned to the Extra Deck at the end of this turn." The card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no limit per turn, as Cyber-Stein's: prepare  face up on the field, 1000 LP, a free monster zone and such a Fusion Monster in the Fusion Deck chainA   1000 LP (DUEL_CMD_LOSE_LP, a cost) resolve  Metamorphosis' steps with a list of the Fusion Deck's Level 6 or lower Fusion Monsters (ModCollectEffectTargets: IsScientistFusion; the CPU takes the highest ATK), Special Summoned face up in the position the player picks (QueueSpecialSummonChoosePosition, which takes FindFreeMonsterZone's zone). The zone and card ID go to gModDuel.scientistReturn, as Return from the Different Dimension marks its own. DuelPhase_End (battle_phase3.c)  EndPhase_ReturnMagicalScientist returns each marked monster still in its zone (ReturnFieldCardToHand: a Fusion Monster goes to the Fusion Deck), then clears the marks. No direct attack  CanMonsterAttack (card_menu_input.c) asks ModCannotAttackDirectly, which refuses a marked monster's attack while the opponent has no monster, as for Zombyra the Dark. The CPU: AiUseMagicalScientist, below AiSpecialSummonCyberDragon.
- **YGOPRODeck (TCG) text:** Pay 1000 Life Points to Special Summon 1 level 6 or lower Fusion Monster from your Extra Deck in face-up Attack or Defense Position. That Fusion Monster cannot attack your opponent's Life Points directly, and is returned to your Extra Deck at the end of the turn.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiUseMagicalScientist` Monster menu: AiUseMagicalScientist decides Set/chain: AiUseMagicalScientist decides

#### Player 1: Pay 1000 for fusion

`Magical_Scientist_Pay_1000_For_Fusion_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Thousand Dragon (Level 7) is not offered: only Level 6 or lower.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Magical Scientist | face-up Attack Position |
| You | Fusion Deck | Flame Swordsman |  |
| You | Fusion Deck | Thousand Dragon |  |

**Steps**

1. `activate you.m0`: Open the card menu of your Monster Zone 0 (**Magical Scientist**) and activate its effect.
2. `pick you.extra[Flame Swordsman]`: Pick **Flame Swordsman** in your Fusion Deck in the list or on the field.
3. `zone m1`: Choose your Monster Zone 1 for the summoned monster.
4. `position atk`: Choose face-up Attack Position for the summoned monster.
5. `check you.lp = 7000`: Check: LP of you = 7000.
6. `check you.m1 = Flame Swordsman atk`: Check: your Monster Zone 1 holds Flame Swordsman, face-up Attack Position.
7. `battle`: Enter the Battle Phase.
8. `check cannot attack you.m1 -> direct`: Check: your Monster Zone 1 cannot attack the opponent directly.
9. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `you.m1 = empty`: your Monster Zone 1 is empty.
- `you.extra = Flame Swordsman, Thousand Dragon`: your Fusion Deck is exactly Flame Swordsman, Thousand Dragon.
- `you.lp = 7000`: LP of you = 7000.

#### CPU 1: Summon fusion

`Magical_Scientist_Cpu_Summon_Fusion_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Magical Scientist | face-up Attack Position |
| Opponent | Fusion Deck | Flame Swordsman |  |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.lp <= 7000`: LP of opp <= 7000.
- `opp.extra has Flame Swordsman`: the opponent's Fusion Deck includes Flame Swordsman.

### Tribe-Infecting Virus

- **Usage:** 100.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TRIBE_INFECTING_VIRUS`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn: You can discard 1 card, then declare 1 Monster Type; destroy all face-up monsters of the declared Type on the field." An activated effect from the card menu in the Main Phase (CanActivateMonsterEffect), once per turn through the zone's effectUnused flag (DuelCmd_TurnStart sets it again). The pieces are the game's own: the discard cost of Magic Jammer (DuelPrompt_PostDiscardCost), the type prompt of DNA Surgery (EffectDeclareTypeChainB: targets[0] is the enum CardType) and the destruction of Eradicating Aerosol (DestroyFieldCard + OnCardDestroyedByEffect, the opponent's monsters first). The Virus does not target, so face-up is the only condition.
- **YGOPRODeck (TCG) text:** Discard 1 card from your hand and declare 1 Type of monster. Destroy all face-up monsters of the declared Type on the field.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — Monster menu: Default yes Set/chain: Default yes

#### Player 1: Declare type destroy

`Tribe_Infecting_Virus_Declare_Type_Destroy_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Tribe-Infecting Virus | face-up Attack Position |
| You | Hand 0 | Fissure |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 1 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| Opponent | Monster Zone 2 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.m0`: Open the card menu of your Monster Zone 0 (**Tribe-Infecting Virus**) and activate its effect.
2. `discard you.hand[Fissure]`: Discard **Fissure** in your hand for the cost.
3. `declare Fiend`: Declare Fiend.

**Expected**

- `opp.m0 = Battle Ox atk`: the opponent's Monster Zone 0 (**Battle Ox**) holds Battle Ox, face-up Attack Position.
- `opp.m1 = empty`: the opponent's Monster Zone 1 (**La Jinn the Mystical Genie of the Lamp**) is empty.
- `opp.m2 = empty`: the opponent's Monster Zone 2 (**Summoned Skull**) is empty.
- `you.gy = Fissure`: your Graveyard is exactly Fissure.
- `menu you.m0 lacks Activate`: The card menu of your Monster Zone 0 (**Tribe-Infecting Virus**) does not offer Activate.

#### CPU 1: Declare best type

`Tribe_Infecting_Virus_Cpu_Declare_Best_Type_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Tribe-Infecting Virus | face-up Attack Position |
| Opponent | Hand 0 | Fissure |  |
| You | Monster Zone 0 | La Jinn the Mystical Genie of the Lamp | face-up Attack Position |
| You | Monster Zone 1 | Summoned Skull | face-up Attack Position |
| You | Monster Zone 2 | Battle Ox | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.gy has Summoned Skull`: your Graveyard includes Summoned Skull.
- `you.gy has La Jinn the Mystical Genie of the Lamp`: your Graveyard includes La Jinn the Mystical Genie of the Lamp.
- `you.monsters lacks Summoned Skull`: your monsters does not include Summoned Skull.

### Gilasaurus

- **Usage:** 33.3% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_GILASAURUS`, `gAddedCardEffects`)
- **Coded behavior:** "You can Special Summon this card (from your hand). If Summoned this way: Activate this effect; your opponent can Special Summon 1 monster from their GY." Engine key 1520 (tools/newcards.py ENGINE_NUMBERS): the hand menu adds the Special Summon commands (card_command_menu.c), and the summon steps (effect_hooks.c) queue QueueSpecialSummonFromHand and, while the opponent has a monster to revive, a free zone and may Special Summon, a RESPONSE_SPECIAL_SUMMONED trigger. Its resolve, EffectOpponentGraveSummonResolve (effect_resolve11.c), asks the opponent (PROMPT_CONFIRM_GRAVEYARD_SUMMON; the CPU says Yes), who picks a monster from the Call of the Haunted list (the CPU: AiPickCardListEntry) and its position. The CPU's own Special Summon is in AiSpecialSummonCyberDragon below (AiGilasaurusSpecialSummon).
- **YGOPRODeck (TCG) text:** You can Special Summon this card (from your hand). If Summoned this way: Activate this effect; your opponent can Special Summon 1 monster from their GY.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsGilasaurus`

#### Player 1: Special summon opponent revives

`Gilasaurus_Special_Summon_Opponent_Revives_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The CPU always says Yes to its Graveyard summon.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Gilasaurus |  |
| Opponent | Graveyard | Summoned Skull |  |

**Steps**

1. `special_summon you.hand[Gilasaurus] -> m0 atk`: Special Summon **Gilasaurus** in your hand from the hand by its own rule into your Monster Zone 0 (face-up Attack Position).

**Expected**

- `you.m0 = Gilasaurus atk`: your Monster Zone 0 holds Gilasaurus, face-up Attack Position.
- `opp.monsters = Summoned Skull`: the opponent's monsters is exactly Summoned Skull.
- `opp.gy count 0`: the opponent's Graveyard holds 0 card(s).

#### CPU 1: Special summon human revives

`Gilasaurus_Cpu_Special_Summon_Human_Revives_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Gilasaurus |  |
| You | Graveyard | Summoned Skull |  |

**Steps**

1. `when prompt graveyard_summon: yes`: When the graveyard_summon prompt opens: answer Yes.
2. `pick you.gy[Summoned Skull]`: Pick **Summoned Skull** in your Graveyard in the list or on the field.
3. `zone m0`: Choose your Monster Zone 0 for the summoned monster.
4. `position atk`: Choose face-up Attack Position for the summoned monster.

**Expected**

- `opp.monsters has Gilasaurus`: the opponent's monsters includes Gilasaurus.
- `you.m0 = Summoned Skull atk`: your Monster Zone 0 holds Summoned Skull, face-up Attack Position.
- `you.gy count 0`: your Graveyard holds 0 card(s).

### Poison of the Old Man

- **Usage:** 25.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_POISON_OF_THE_OLD_MAN`, `gAddedCardEffects`)
- **Coded behavior:** Quick-Play Spell: "Activate 1 of these effects; ● Gain 1200 LP. ● Inflict 800 damage to your opponent." chainA   the choice, kept in the link's unused unk12 as Enemy Controller's: the human picks from a two-line menu (the handler waits while the box is open: Chain_Build calls it every frame); the CPU decides (AiPoisonOfTheOldManChoice) resolve  GainLifePoints (1200) for the player, or LoseLifePoints (800) for the opponent CPU      AiPoisonOfTheOldManChoice: the damage when it wins the duel, the LP while it has POTOM_AI_LOW_LP or less, else the damage; AiWantsSpell plays it whenever it may (AiPlayAddedCards, Main Phase)
- **YGOPRODeck (TCG) text:** Activate 1 of these effects;
● Gain 1200 LP.
● Inflict 800 damage to your opponent.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsSpell` Monster menu: Default yes Main Phase Spell: always: AiPoisonOfTheOldManChoice picks the effect Set/chain: Default yes

#### Player 1: Gain 1200

`Poison_of_the_Old_Man_Gain_1200_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Poison of the Old Man |  |

**Steps**

1. `activate you.hand[Poison of the Old Man]`: Open the card menu of **Poison of the Old Man** in your hand and choose Activate.
2. `choose "Gain 1200 LP"`: Choose "Gain 1200 LP" in the menu.

**Expected**

- `you.lp = 9200`: LP of you = 9200.
- `opp.lp = 8000`: LP of opp = 8000.

#### Player 2: Burn 800

`Poison_of_the_Old_Man_Burn_800_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Poison of the Old Man |  |

**Steps**

1. `activate you.hand[Poison of the Old Man]`: Open the card menu of **Poison of the Old Man** in your hand and choose Activate.
2. `choose "Inflict 800 damage"`: Choose "Inflict 800 damage" in the menu.

**Expected**

- `opp.lp = 7200`: LP of opp = 7200.
- `you.lp = 8000`: LP of you = 8000.

#### CPU 1: Burn

`Poison_of_the_Old_Man_Cpu_Burn_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

With 8000 LP the CPU picks the damage (AiPoisonOfTheOldManChoice).

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Poison of the Old Man |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.lp = 7200`: LP of you = 7200.
- `opp.gy = Poison of the Old Man`: the opponent's Graveyard is exactly Poison of the Old Man.

### Reasoning

- **Usage:** 25.0% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_REASONING`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Your opponent declares a monster Level from 1 to 12. Excavate cards from the top of your Deck until you excavate a monster that can be Normal Summoned/Set, then, if that monster is the same Level as the one declared by your opponent, send all excavated cards to the GY. If not, Special Summon the excavated monster, also send the remaining cards to the GY." The excavation is Monster Gate's (IsGateMonster, so this needs Monster Gate listed too). The declaration: the human declares (the CPU activated it): a text box with a Level menu (LevelMenu_Draw / LevelMenu_HandleInput, made like the Monster Type menu of DuelPrompt_SelectType: LEFT/RIGHT, A), starting at Level 4; the CPU declares (the human activated it): Level 4, the most common Level (it cannot see the Deck), shown in a text box. The Level is kept in gChain.effectSubStep. The monster leaves the Deck, and the next step Special Summons it face up in the position the player picks.
- **YGOPRODeck (TCG) text:** Your opponent declares a monster Level from 1 to 12. Excavate cards from the top of your Deck until you excavate a monster that can be Normal Summoned/Set, then, if that monster is the same Level as the one declared by your opponent, send all excavated cards to the GY. If not, Special Summon the excavated monster, also send the remaining cards to the GY.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `sStrReasoningCpuDeclares` Dedicated `*Cpu*` handlers in the card's effect steps.

#### Player 1: Excavate special summon

`Reasoning_Excavate_Special_Summon_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The CPU declares Level 4; Summoned Skull is Level 6, so it is Special Summoned.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Reasoning |  |
| You | Deck 0 | Fissure |  |
| You | Deck 1 | Trap Hole |  |
| You | Deck 2 | Summoned Skull |  |
| You | Deck 3 | Battle Ox |  |

**Steps**

1. `activate you.hand[Reasoning]`: Open the card menu of **Reasoning** in your hand and choose Activate.
2. `zone m0`: Choose your Monster Zone 0 for the summoned monster.
3. `position atk`: Choose face-up Attack Position for the summoned monster.

**Expected**

- `you.m0 = Summoned Skull atk`: your Monster Zone 0 holds Summoned Skull, face-up Attack Position.
- `you.gy = Reasoning, Fissure, Trap Hole`: your Graveyard is exactly Reasoning, Fissure, Trap Hole.

#### CPU 1: Human declares match

`Reasoning_Cpu_Human_Declares_Match_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Reasoning |  |
| Opponent | Deck 0 | Fissure |  |
| Opponent | Deck 1 | Battle Ox |  |

**Steps**

1. `when prompt declare_level: declare level 4`: When the declare_level prompt opens: declare level 4.

**Expected**

- `opp.gy has Reasoning`: the opponent's Graveyard includes Reasoning.
- `opp.gy has Fissure`: the opponent's Graveyard includes Fissure.
- `opp.gy has Battle Ox`: the opponent's Graveyard includes Battle Ox.
- `opp.monsters lacks Battle Ox`: the opponent's monsters does not include Battle Ox.

### Jowgen the Spiritualist

- **Usage:** 16.7% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_JOWGEN_THE_SPIRITUALIST`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** You can discard 1 random card from your hand to the Graveyard; destroy all Special Summoned monsters on the field. Neither player can Special Summon monsters.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroy special summoned

`Jowgen_the_Spiritualist_Destroy_Special_Summoned_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The discard is random (Fissure or Gilasaurus).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Jowgen the Spiritualist | face-up Attack Position |
| You | Hand 0 | Fissure |  |
| You | Hand 1 | Gilasaurus |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position, Special Summoned |
| Opponent | Monster Zone 1 | Battle Ox | face-up Attack Position |

**Steps**

1. `check menu you.hand[Gilasaurus] lacks Special Summon`: Check: The card menu of **Gilasaurus** in your hand does not offer Special Summon.
2. `activate you.m0`: Open the card menu of your Monster Zone 0 (**Jowgen the Spiritualist**) and activate its effect.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.m1 = Battle Ox atk`: the opponent's Monster Zone 1 (**Battle Ox**) holds Battle Ox, face-up Attack Position.
- `you.hand count 1`: your hand holds 1 card(s).
- `you.gy count 1`: your Graveyard holds 1 card(s).

#### CPU 1: Lock special summon

`Jowgen_the_Spiritualist_Cpu_Lock_Special_Summon_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Jowgen the Spiritualist | face-up Attack Position |
| You | Hand 0 | Gilasaurus |  |

**Steps**

1. `check menu you.hand[Gilasaurus] lacks Special Summon`: Check: The card menu of **Gilasaurus** in your hand does not offer Special Summon.

**Expected**

- `you.hand = Gilasaurus`: your hand is exactly Gilasaurus.

### Spiritualism

- **Usage:** 16.7% weighted inclusion in `usage/scientist.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1539**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** Return a Magic or Trap card of the opponent to their hand. It can never be negated. Status in ROM: Full.
- **YGOPRODeck (TCG) text:** Return 1 Spell/Trap Card your opponent controls to the hand. This card's activation and effect cannot be negated.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Unclear / likely missing** — No `CPU:` notes and no switch case found; treat as untested for CPU until you confirm in a duel.

#### Player 1: Return spell trap

`Spiritualism_Return_Spell_Trap_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Spiritualism |  |
| Opponent | Spell/Trap Zone 0 | Call of the Haunted | face-down (Set) |

**Steps**

1. `activate you.hand[Spiritualism]`: Open the card menu of **Spiritualism** in your hand and choose Activate.
2. `target opp.st0`: Pick the opponent's Spell/Trap Zone 0 (**Call of the Haunted**) as the target.

**Expected**

- `opp.st0 = empty`: the opponent's Spell/Trap Zone 0 (**Call of the Haunted**) is empty.
- `opp.hand = Call of the Haunted`: the opponent's hand is exactly Call of the Haunted.
- `you.gy = Spiritualism`: your Graveyard is exactly Spiritualism.

#### CPU 1: Return human spell trap

`Spiritualism_Cpu_Return_Human_Spell_Trap_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Spiritualism |  |
| You | Spell/Trap Zone 0 | Call of the Haunted | face-down (Set) |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.st0 = empty`: your Spell/Trap Zone 0 (**Call of the Haunted**) is empty.
- `you.hand = Call of the Haunted`: your hand is exactly Call of the Haunted.

## Vampire

### D.D. Warrior Lady

- **Usage:** 91.3% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_D_D_WARRIOR_LADY`, `gAddedCardEffects`)
- **Coded behavior:** "After damage calculation, if this card battled an opponent's monster: You can banish that monster, also banish this card." No effect row: the effect lives in BattleStage_DestroyMonsters (battle_phase2.c), which asks its controller before the destroyed monsters leave the field (the human through PROMPT_CONFIRM_CARD_EFFECT with this text; the CPU always says Yes). On Yes the battle goes on like a Dimensional Warrior battle: a destroyed monster is banished instead of sent to the Graveyard, and a monster that survived is banished too.
- **YGOPRODeck (TCG) text:** After damage calculation, when this card battles an opponent's monster: You can banish that monster, also banish this card.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Banish both

`DD_Warrior_Lady_Banish_Both_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | D.D. Warrior Lady | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**D.D. Warrior Lady**): target the opponent's Monster Zone 0 (**Summoned Skull**).
3. `yes`: Answer Yes.

**Expected**

- `you.lp = 7000`: LP of you = 7000.
- `opp.banished = Summoned Skull`: the opponent's banished cards is exactly Summoned Skull.
- `you.banished = D.D. Warrior Lady`: your banished cards is exactly D.D. Warrior Lady.
- `you.monsters count 0`: your monsters holds 0 card(s).
- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).

#### CPU 1: Banish both

`DD_Warrior_Lady_Cpu_Banish_Both_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 0 | D.D. Warrior Lady | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent's Monster Zone 0 (**D.D. Warrior Lady**).

**Expected**

- `opp.lp = 7000`: LP of opp = 7000.
- `you.banished = Summoned Skull`: your banished cards is exactly Summoned Skull.
- `opp.banished = D.D. Warrior Lady`: the opponent's banished cards is exactly D.D. Warrior Lady.

### Sakuretsu Armor

- **Usage:** 56.5% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SAKURETSU_ARMOR`, `gAddedCardEffects`)
- **Coded behavior:** "When an opponent's monster declares an attack: Target the attacking monster; destroy that target." Normal Trap during RESPONSE_ATTACK_DECLARED (loc0 = attacker, loc1 = attack target), like Negate Attack / Enchanted Javelin. Only loc0 is a legal target; chainB sets it; EffectDestroyTargetResolve destroys it.
- **YGOPRODeck (TCG) text:** When an opponent's monster declares an attack: Target the attacking monster; destroy that target.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Destroy attacker

`Sakuretsu_Armor_Destroy_Attacker_Player` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Sakuretsu Armor | face-down (Set) |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `when attack opp.m0 -> you.m0: activate you.st0`: When the opponent's Monster Zone 0 (**Summoned Skull**) declares an attack on your Monster Zone 0 (**Celtic Guardian**): open the card menu of your Spell/Trap Zone 0 (**Sakuretsu Armor**) and choose Activate.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.
- `you.m0 = Celtic Guardian atk`: your Monster Zone 0 (**Celtic Guardian**) holds Celtic Guardian, face-up Attack Position.
- `you.lp = 8000`: LP of you = 8000.

#### CPU 1: Destroy attacker

`Sakuretsu_Armor_Cpu_Destroy_Attacker_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Sakuretsu Armor | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> direct`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent directly.

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.gy = Summoned Skull`: your Graveyard is exactly Summoned Skull.
- `opp.lp = 8000`: LP of opp = 8000.

### Sasuke Samurai

- **Usage:** 30.4% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SASUKE_SAMURAI`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** At the start of the Damage Step, if this card attacks a face-down Defense Position monster: Destroy that face-down monster.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroy face down

`Sasuke_Samurai_Destroy_Face_Down_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Sasuke Samurai | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Sasuke Samurai**): target the opponent's Monster Zone 0 (**Summoned Skull**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.
- `you.lp = 8000`: LP of you = 8000.

#### CPU 1: Destroy face down

`Sasuke_Samurai_Cpu_Destroy_Face_Down_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Sasuke Samurai | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-down (Set) |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.gy = Summoned Skull`: your Graveyard is exactly Summoned Skull.

### Amazoness Archer

- **Usage:** 17.4% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** Vanilla engine (effect key **1432**; [`LEFTOVER_EFFECTS.md`](LEFTOVER_EFFECTS.md))
- **Coded behavior:** Tribute two of your monsters: the opponent loses 1200 LP. Status in ROM: Full.
- **YGOPRODeck (TCG) text:** You can Tribute 2 monsters; inflict 1200 damage to your opponent.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Unclear / likely missing** — No `CPU:` notes and no switch case found; treat as untested for CPU until you confirm in a duel.

#### Player 1: Tribute two burn

`Amazoness_Archer_Tribute_Two_Burn_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Amazoness Archer | face-up Attack Position |
| You | Monster Zone 1 | Battle Ox | face-up Attack Position |
| You | Monster Zone 2 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `activate you.m0`: Open the card menu of your Monster Zone 0 (**Amazoness Archer**) and activate its effect.
2. `target you.m1`: Pick your Monster Zone 1 (**Battle Ox**) as the target.
3. `target you.m2`: Pick your Monster Zone 2 (**Celtic Guardian**) as the target.

**Expected**

- `opp.lp = 6800`: LP of opp = 6800.
- `you.gy = Battle Ox, Celtic Guardian`: your Graveyard is exactly Battle Ox, Celtic Guardian.
- `you.m0 = Amazoness Archer atk`: your Monster Zone 0 (**Amazoness Archer**) holds Amazoness Archer, face-up Attack Position.

#### CPU 1: Tribute two burn

`Amazoness_Archer_Cpu_Tribute_Two_Burn_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Amazoness Archer | face-up Attack Position |
| Opponent | Monster Zone 1 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 2 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.lp <= 6800`: LP of you <= 6800.

### Vampire Lord

- **Usage:** 13.0% weighted inclusion in `usage/vampire.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_VAMPIRE_LORD`, `gAddedCardEffects`)
- **Coded behavior:** "If this card inflicts battle damage to your opponent: Declare 1 type of card (Monster, Spell, or Trap); your opponent sends 1 card of that type from their Deck to the GY. Once per turn, during your next Standby Phase after this card in your possession was destroyed and sent to your GY by an opponent's card effect: Special Summon this card from the GY." damage   queued from BattleStage_InflictDamage (battle_phase2.c) like Don Zaloog. The human declares in a menu (LabelMenu); the CPU declares one of the three at random (it cannot see the Deck). The opponent of the card's controller picks: the human in the card list, the CPU its weakest monster or its first Spell/Trap. The Deck is shuffled. A declared type the CPU's Deck lacks is only reported, and one the human's Deck lacks does nothing (the real card has the Deck revealed). revival  Many of the engine's effects destroy without OnCardDestroyedByEffect (Trap Hole, for one), so DestroyFieldCard (duel_field_moves.c) asks VampireLordFieldDestroyed: while a chain resolves, the source is the resolving link's player. Effects outside a chain (Sasuke Samurai's) still report through OnCardDestroyedByEffect (effect_hooks.c). One its owner controlled, destroyed by the other player's effect, adds to gModDuel.vampireLordPending. In that player's next Standby Phase, VampireLordStandbyStep (ModStandbyStep) Special Summons it from the Graveyard in the position they pick, not as a chain link (as Treeborn Frog); a copy no longer there, or no free zone, and nothing happens.
- **YGOPRODeck (TCG) text:** If this card inflicts battle damage to your opponent: Declare 1 type of card (Monster, Spell, or Trap); your opponent sends 1 card of that type from their Deck to the GY. Once per turn, during your next Standby Phase after this card in your possession was destroyed and sent to your GY by an opponent's card effect: Special Summon this card from the GY.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `sStrVampireCpuMonster`, `sStrVampireCpuSpell`, `sStrVampireCpuTrap` Dedicated `*Cpu*` handlers in the card's effect steps.

#### Player 1: Declare spell on damage

`Vampire_Lord_Declare_Spell_On_Damage_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The CPU sends its first Spell/Trap of the declared type (Fissure is on top).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Vampire Lord | face-up Attack Position |
| Opponent | Deck 0 | Fissure |  |
| Opponent | Deck 1 | Battle Ox |  |
| Opponent | Deck 2 | Trap Hole |  |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> direct`: Attack with your Monster Zone 0 (**Vampire Lord**): target the opponent directly.
3. `choose "Spell"`: Choose "Spell" in the menu.

**Expected**

- `opp.lp = 6000`: LP of opp = 6000.
- `opp.gy count 1`: the opponent's Graveyard holds 1 card(s).

#### Player 2: Revive after destroyed by effect

`Vampire_Lord_Revive_After_Destroyed_By_Effect_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

The CPU answers the summon with Trap Hole; Vampire Lord comes back in your next Standby Phase.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Hand 0 | Vampire Lord |  |
| Opponent | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |

**Steps**

1. `tribute_summon you.hand[Vampire Lord] -> m0 atk tributes you.m0`: Tribute Summon **Vampire Lord** in your hand into your Monster Zone 0 (**Battle Ox**) (face-up Attack Position), Tributing your Monster Zone 0 (**Battle Ox**).
2. `check you.gy has Vampire Lord`: Check: your Graveyard includes Vampire Lord.
3. `pass`: End your turn (its End Phase effects resolve).
4. `cpu_turn`: The CPU plays its whole turn (not frozen).
5. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.
6. `position atk`: Choose face-up Attack Position for the summoned monster.

**Expected**

- `you.monsters has Vampire Lord`: your monsters includes Vampire Lord.
- `you.gy lacks Vampire Lord`: your Graveyard does not include Vampire Lord.
- `opp.gy has Trap Hole`: the opponent's Graveyard includes Trap Hole.

#### CPU 1: Declare on damage

`Vampire_Lord_Cpu_Declare_On_Damage_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

The CPU declares a type at random; pick the first card the list offers.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Vampire Lord | face-up Attack Position |

**Steps**

1. `when prompt deck_pick: pick you.deck[0]`: When the deck_pick prompt opens: pick your Deck slot 0 in the list or on the field.

**Expected**

- `you.lp = 6000`: LP of you = 6000.
- `you.gy count 1`: your Graveyard holds 1 card(s).

## Chaos

### Chaos Emperor Dragon - Envoy of the End

- **Usage:** 100.0% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END`, `gAddedCardEffects`)
- **Coded behavior:** Its first-edition text, as the user asked (the fetched one is the later errata, with "Once per turn", damage only for the opponent's cards, and "You cannot activate other cards or effects during the turn you activate this card's effect"): "This card can only be Special Summoned by removing 1 LIGHT and 1 DARK monster in your Graveyard from play. Pay 1000 of your Life Points to send all cards in both players' hands and on the field to the Graveyard. Inflict 300 points of damage to your opponent's Life Points for each card that is sent to the Graveyard by this effect." The summon is Black Luster Soldier - Envoy of the Beginning's (the same hooks, HAS_LIGHT_DARK_BANISH_SUMMON). The effect is the card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no limit per turn: prepare  face up on the field and 1000 LP chainA   1000 LP (DUEL_CMD_LOSE_LP, a cost) resolve  every card on both fields (zones 0-10, as Final Destiny walks them) goes to the Graveyard (SendFieldCardToGrave, with its leave-the-field effects), then every card in both hands (DUEL_CMD_SEND_HAND_CARD_TO_GRAVEYARD: sent, not discarded, so no discard effect); then the opponent takes 300 for each card sent, from either player (counted before: a token goes nowhere and does not count; the Dragon itself does)
- **YGOPRODeck (TCG) text:** Cannot be Normal Summoned/Set. Must be Special Summoned (from your hand) by banishing 1 LIGHT and 1 DARK monster from your GY. Once per turn: You can pay 1000 LP; send as many cards in both players' hands and on the field as possible to the GY, then inflict 300 damage to your opponent for each card sent to the opponent's GY by this effect. You cannot activate other cards or effects during the turn you activate this card's effect.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsChaosEmperorDragon`, `AiWantsMonsterEffect` Monster menu: Conditional AiWants* / AiShouldActivateAddedCard Set/chain: Conditional AiWants* / AiShouldActivateAddedCard

#### Player 1: Summon and wipe

`Chaos_Emperor_Dragon_Envoy_of_the_End_Summon_And_Wipe_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

7 cards are sent (the Dragon, 3 more on the field, 3 in the hands): 2100 damage.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Chaos Emperor Dragon - Envoy of the End |  |
| You | Hand 1 | Battle Ox |  |
| You | Graveyard | Summoned Skull |  |
| You | Graveyard | Blue-Eyes White Dragon |  |
| You | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| Opponent | Hand 0 | Fissure |  |
| Opponent | Hand 1 | Dark Hole |  |

**Steps**

1. `special_summon you.hand[Chaos Emperor Dragon - Envoy of the End] -> m0 atk banish you.gy[Blue-Eyes White Dragon], you.gy[Summoned Skull]`: Special Summon **Chaos Emperor Dragon - Envoy of the End** in your hand from the hand by its own rule into your Monster Zone 0 (face-up Attack Position), banishing **Blue-Eyes White Dragon** in your Graveyard and **Summoned Skull** in your Graveyard.
2. `activate you.m0`: Open the card menu of your Monster Zone 0 and activate its effect.

**Expected**

- `you.lp = 7000`: LP of you = 7000.
- `opp.lp = 5900`: LP of opp = 5900.
- `you.monsters count 0`: your monsters holds 0 card(s).
- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `you.hand count 0`: your hand holds 0 card(s).
- `opp.hand count 0`: the opponent's hand holds 0 card(s).
- `you.gy = Chaos Emperor Dragon - Envoy of the End, Trap Hole, Battle Ox`: your Graveyard is exactly Chaos Emperor Dragon - Envoy of the End, Trap Hole, Battle Ox.
- `opp.gy = Battle Ox, Trap Hole, Fissure, Dark Hole`: the opponent's Graveyard is exactly Battle Ox, Trap Hole, Fissure, Dark Hole.
- `you.banished = Blue-Eyes White Dragon, Summoned Skull`: your banished cards is exactly Blue-Eyes White Dragon, Summoned Skull.

#### CPU 1: Summon and wipe

`Chaos_Emperor_Dragon_Envoy_of_the_End_Cpu_Summon_And_Wipe_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

6 cards are sent (the Dragon, your 3 field cards and 2 hand cards): 1800 damage to you.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Chaos Emperor Dragon - Envoy of the End |  |
| Opponent | Graveyard | Summoned Skull |  |
| Opponent | Graveyard | Blue-Eyes White Dragon |  |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| You | Monster Zone 1 | Battle Ox | face-up Attack Position |
| You | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| You | Hand 0 | Fissure |  |
| You | Hand 1 | Dark Hole |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.lp = 7000`: LP of opp = 7000.
- `you.lp = 6200`: LP of you = 6200.
- `you.monsters count 0`: your monsters holds 0 card(s).
- `you.hand count 0`: your hand holds 0 card(s).

### Dark Magician of Chaos

- **Usage:** 17.2% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_DARK_MAGICIAN_OF_CHAOS`, `gAddedCardEffects`)
- **Coded behavior:** "When this card is Normal or Special Summoned, select 1 Spell Card from your Graveyard during the End Phase, and add it to your hand. If this card destroys a monster by battle, remove that monster from play. If this face-up card is removed from the field, remove it from play." SummonAction_Update (summon_action.c)  DarkMagicianOfChaosSummoned: a face-up Normal Summon or any Special Summon sets its bit in gModDuel.chaosSummoned (a Set or a Flip Summon does not) DuelPhase_End (battle_phase3.c)        EndPhase_DarkMagicianOfChaos: a trigger for each marked zone whose player has a Spell in the Graveyard (the summon was enough: the card need not be on the field any more); the marks are cleared. Its row is Magician of Faith's: the Graveyard Spells in the card-list viewer (EffectCardListTargetChainB, CollectEffectTargets case shared with it, AiPickCardListEntry's case too), added by EffectMaskOfDarknessResolve BattleStage_DestroyMonsters            with it in the battle, the destroyed monsters are banished instead of sent to the Graveyard (gBattle.ddWarriorLady = DD_BANISH_DESTROYED; survivors stay) SendFieldCardToGrave, TributeMonster, ReturnFieldCardToHand, ReturnFieldCardToDeck
- **YGOPRODeck (TCG) text:** During the End Phase, if this card was Normal or Special Summoned this turn: You can target 1 Spell in your GY; add it to your hand. You can only use this effect of "Dark Magician of Chaos" once per turn. If this card destroys an opponent's monster by battle, after damage calculation: Banish that opponent's monster. If this face-up card would leave the field, banish it instead.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Banish and recover spell

`Dark_Magician_of_Chaos_Banish_And_Recover_Spell_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Dark Magician of Chaos |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Monster Zone 1 | Celtic Guardian | face-up Attack Position |
| You | Graveyard | Monster Reborn |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `tribute_summon you.hand[Dark Magician of Chaos] -> m0 atk tributes you.m0, you.m1`: Tribute Summon **Dark Magician of Chaos** in your hand into your Monster Zone 0 (**Battle Ox**) (face-up Attack Position), Tributing your Monster Zone 0 (**Battle Ox**) and your Monster Zone 1 (**Celtic Guardian**).
2. `battle`: Enter the Battle Phase.
3. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Battle Ox**): target the opponent's Monster Zone 0 (**Summoned Skull**).
4. `pass`: End your turn (its End Phase effects resolve).
5. `yes`: Answer Yes.
6. `pick you.gy[Monster Reborn]`: Pick **Monster Reborn** in your Graveyard in the list or on the field.

**Expected**

- `opp.banished = Summoned Skull`: the opponent's banished cards is exactly Summoned Skull.
- `opp.lp = 7700`: LP of opp = 7700.
- `you.hand = Monster Reborn`: your hand is exactly Monster Reborn.

#### CPU 1: Banish by battle

`Dark_Magician_of_Chaos_Cpu_Banish_By_Battle_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Dark Magician of Chaos | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.banished = Summoned Skull`: your banished cards is exactly Summoned Skull.
- `you.lp = 7700`: LP of you = 7700.

### Chaos Sorcerer

- **Usage:** 16.1% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CHAOS_SORCERER`, `gAddedCardEffects`)
- **Coded behavior:** Their summon (banish 1 LIGHT and 1 DARK monster from the Graveyard) and their "banish the target; this card cannot attack this turn" effect, once per turn through effectUnused.
- **YGOPRODeck (TCG) text:** Cannot be Normal Summoned/Set. Must first be Special Summoned (from your hand) by banishing 1 LIGHT and 1 DARK monster from your GY. Once per turn: You can target 1 face-up monster on the field; banish that target. This card cannot attack the turn you activate this effect.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — Monster menu: Default yes Set/chain: Default yes

#### Player 1: Summon and banish

`Chaos_Sorcerer_Summon_And_Banish_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Chaos Sorcerer |  |
| You | Graveyard | Summoned Skull |  |
| You | Graveyard | Blue-Eyes White Dragon |  |
| Opponent | Monster Zone 0 | Jinzo | face-up Attack Position |

**Steps**

1. `special_summon you.hand[Chaos Sorcerer] -> m0 atk banish you.gy[Blue-Eyes White Dragon], you.gy[Summoned Skull]`: Special Summon **Chaos Sorcerer** in your hand from the hand by its own rule into your Monster Zone 0 (face-up Attack Position), banishing **Blue-Eyes White Dragon** in your Graveyard and **Summoned Skull** in your Graveyard.
2. `activate you.m0`: Open the card menu of your Monster Zone 0 and activate its effect.
3. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Jinzo**) as the target.
4. `battle`: Enter the Battle Phase.
5. `check cannot attack you.m0 -> direct`: Check: your Monster Zone 0 cannot attack the opponent directly.

**Expected**

- `opp.banished = Jinzo`: the opponent's banished cards is exactly Jinzo.
- `you.banished = Blue-Eyes White Dragon, Summoned Skull`: your banished cards is exactly Blue-Eyes White Dragon, Summoned Skull.
- `opp.lp = 8000`: LP of opp = 8000.

#### CPU 1: Banish human monster

`Chaos_Sorcerer_Cpu_Banish_Human_Monster_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Chaos Sorcerer | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.banished = Summoned Skull`: your banished cards is exactly Summoned Skull.

### Tsukuyomi

- **Usage:** 13.8% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TSUKUYOMI`, `gAddedCardEffects`)
- **Coded behavior:** Book of Moon (Quick-Play Spell) and Tsukuyomi (FLIP) share the same effect: target 1 face-up monster on the field; change it to face-down Defense Position. Targeting uses EffectTargetableFaceUpMonsterCheck / ChainB (Chaos Sorcerer's pickers). The resolve first destroys the target's Equip Cards (they cannot equip a face-down monster; a destroyed Premature Burial takes the monster with it). Then DUEL_CMD_CHANGE_POSITION puts a monster in Attack Position into Defense Position, and DUEL_CMD_FLIP_CARD turns it face down, which removes its counters (OnMonsterTurnedFaceDown). Tsukuyomi is listed in HasFlipEffect (card_detail.c).
- **YGOPRODeck (TCG) text:** Cannot be Special Summoned. If this card is Normal Summoned or flipped face-up: Target 1 face-up monster on the field; change that target to face-down Defense Position. Once per turn, during the End Phase, if this card was Normal Summoned or flipped face-up this turn: Return it to the hand.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Flip set target

`Tsukuyomi_Flip_Set_Target_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Tsukuyomi returns to the hand at the End Phase (Spirit).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Tsukuyomi | face-down (Set) |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `flip_summon you.m0`: Flip Summon your Monster Zone 0 (**Tsukuyomi**).
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.
3. `check opp.m0 = Summoned Skull set`: Check: the opponent's Monster Zone 0 (**Summoned Skull**) holds Summoned Skull, face-down (Set).
4. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `you.hand = Tsukuyomi`: your hand is exactly Tsukuyomi.
- `you.monsters count 0`: your monsters holds 0 card(s).

#### CPU 1: Flip set target

`Tsukuyomi_Cpu_Flip_Set_Target_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Tsukuyomi | face-down (Set) |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = Summoned Skull set`: your Monster Zone 0 (**Summoned Skull**) holds Summoned Skull, face-down (Set).

### Cold Wave

- **Usage:** 12.6% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_COLD_WAVE`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "This turn and the next turn, no player can play any Spell or Trap Cards." Same as engine key 1312 (EffectStartOfMainPhase1Prepare, EffectLockMagicTrapResolve): Main Phase 1 before any Spell/Trap activation or Normal Summon; then both players get magicTrapLockTurns = 2 (DUEL_CMD_SET_MAGIC_TRAP_LOCK_TURNS). The hand menu drops ACTIVATE after either flag is set (card_command_menu.c, as CARD_1312). CPU: AiWantsSpell whenever it may; AiTryPlaySpellTrap also refuses while magicTrapLockTurns is set (ai_deck.c).
- **YGOPRODeck (TCG) text:** This card can only be activated at the start of Main Phase 1. Until your next turn, you and your opponent cannot play or Set any Spell or Trap Cards.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsSpell`

#### Player 1: Lock spells traps

`Cold_Wave_Lock_Spells_Traps_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Activate it first thing in Main Phase 1. The lock lasts this turn and the next one.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Cold Wave |  |
| You | Hand 1 | Pot of Greed |  |

**Steps**

1. `activate you.hand[Cold Wave]`: Open the card menu of **Cold Wave** in your hand and choose Activate.
2. `check menu you.hand[Pot of Greed] lacks Activate`: Check: The card menu of **Pot of Greed** in your hand does not offer Activate.
3. `pass`: End your turn (its End Phase effects resolve).
4. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).
5. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.

**Expected**

- `menu you.hand[Pot of Greed] has Activate`: The card menu of **Pot of Greed** in your hand offers Activate.
- `you.gy = Cold Wave`: your Graveyard is exactly Cold Wave.

#### CPU 1: Lock spells traps

`Cold_Wave_Cpu_Lock_Spells_Traps_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Cold Wave |  |
| You | Hand 0 | Pot of Greed |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).
2. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.

**Expected**

- `opp.gy has Cold Wave`: the opponent's Graveyard includes Cold Wave.
- `menu you.hand[Pot of Greed] lacks Activate`: The card menu of **Pot of Greed** in your hand does not offer Activate.

### Thunder Nyan Nyan

- **Usage:** 10.3% weighted inclusion in `usage/chaos.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THUNDER_NYAN_NYAN`, `gAddedCardEffects`)
- **Coded behavior:** "If there is a non-LIGHT monster on your side of the field, destroy this face-up card." rule  ModDuelStateCheck (DuelMainStep, while nothing else is busy; not during an attack, as for Berserk Gorilla) destroys a face-up Thunder Nyan Nyan whose controller has a face-up monster that is not LIGHT (its effective attribute, GetZoneCardStats). A face-down monster has no attribute and does not count; a face-down Nyan Nyan is not destroyed. CPU   AiSimSummonBoosts (the summon simulation's sandbox) removes the CPU's Nyan Nyans the rule would destroy, so the CPU does not summon it next to a non-LIGHT monster, nor a non-LIGHT monster next to it, unless the simulation still comes out ahead.
- **YGOPRODeck (TCG) text:** If there is a non-LIGHT monster on your side of the field, destroy this face-up card.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroyed by non light

`Thunder_Nyan_Nyan_Destroyed_By_Non_Light_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Thunder Nyan Nyan | face-up Attack Position |
| You | Hand 0 | Battle Ox |  |

**Steps**

1. `summon you.hand[Battle Ox] -> m1 atk`: Normal Summon **Battle Ox** in your hand into your Monster Zone 1 (face-up Attack Position).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Thunder Nyan Nyan**) is empty.
- `you.gy = Thunder Nyan Nyan`: your Graveyard is exactly Thunder Nyan Nyan.
- `you.m1 = Battle Ox atk`: your Monster Zone 1 holds Battle Ox, face-up Attack Position.

#### Player 2: Face down does not count

`Thunder_Nyan_Nyan_Face_Down_Does_Not_Count_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Thunder Nyan Nyan | face-up Attack Position |
| You | Hand 0 | Battle Ox |  |

**Steps**

1. `set you.hand[Battle Ox] -> m1`: Set **Battle Ox** in your hand into your Monster Zone 1.

**Expected**

- `you.m0 = Thunder Nyan Nyan atk`: your Monster Zone 0 (**Thunder Nyan Nyan**) holds Thunder Nyan Nyan, face-up Attack Position.
- `you.m1 = Battle Ox set`: your Monster Zone 1 holds Battle Ox, face-down (Set).

#### CPU 1: Avoid non light

`Thunder_Nyan_Nyan_Cpu_Avoid_Non_Light_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

The CPU does not summon Battle Ox face up beside it (it may Set it).

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Thunder Nyan Nyan | face-up Attack Position |
| Opponent | Hand 0 | Battle Ox |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.monsters has Thunder Nyan Nyan`: the opponent's monsters includes Thunder Nyan Nyan.

## Warrior

### Cipher Soldier

- **Usage:** 63.2% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CIPHER_SOLDIER`, `gAddedCardEffects`)
- **Coded behavior:** "If this card battles a Warrior-Type monster, during damage calculation: This card gains 2000 ATK and DEF during that damage calculation only." stats    GetZoneCardStats (card_stats.c) adds ModStatBonus, which has ModBattleStatBonus, to a face-up monster's ATK and DEF: while the battle is in BATTLE_STAGE_DAMAGE_CALC (CalcBattle runs there and keeps its results in gBattle, so the later stages need nothing), a face-up Cipher Soldier that is the attacker or the attack target gains 2000 when the other monster is a Warrior (its printed Type: reading the other monster's stats from here would loop between two Cipher Soldiers) CPU      AiEvalAttack (ai_picks.c) adds ModCipherBonus to both sides of the battles it predicts
- **YGOPRODeck (TCG) text:** If this card battles a Warrior-Type monster, during damage calculation: This card gains 2000 ATK and DEF during that damage calculation only.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Bonus against warrior

`Cipher_Soldier_Bonus_Against_Warrior_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

3350 ATK during damage calculation only.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Cipher Soldier | face-up Attack Position |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Cipher Soldier**): target the opponent's Monster Zone 0 (**Celtic Guardian**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Celtic Guardian**) is empty.
- `opp.lp = 6050`: LP of opp = 6050.
- `you.m0.atk = 1350`: The face-up Attack Position of your Monster Zone 0 (**Cipher Soldier**) is 1350.

#### CPU 1: Bonus when attacked

`Cipher_Soldier_Cpu_Bonus_When_Attacked_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| Opponent | Monster Zone 0 | Cipher Soldier | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Celtic Guardian**): target the opponent's Monster Zone 0 (**Cipher Soldier**).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Celtic Guardian**) is empty.
- `you.lp = 6050`: LP of you = 6050.
- `opp.m0 = Cipher Soldier atk`: the opponent's Monster Zone 0 (**Cipher Soldier**) holds Cipher Soldier, face-up Attack Position.

### Mobius the Frost Monarch

- **Usage:** 57.9% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MOBIUS_THE_FROST_MONARCH`, `gAddedCardEffects`)
- **Coded behavior:** "If this card is Tribute Summoned: You can target up to 2 Spells/Traps on the field; destroy them." The two Normal Summon paths (summon_checks.c) queue its RESPONSE_SUMMONED trigger in Zaborg's case; it is Level 6, so that summon always has a Tribute. Check: a Spell/Trap is on the field (EffectAnySpellTrapCheck, as Mystical Space Typhoon). Targets (EffectMobiusChainB): human  the first card (B: none, the effect is not used), then a second one (B: only the first; skipped when no other Spell/Trap is on the field), Greenkappa's prompts CPU    up to 2 of the human's Spells/Traps, face-up ones first; never its own EffectMobiusResolve destroys each target still on the field, as EffectDestroyTargetResolve does one.
- **YGOPRODeck (TCG) text:** When this card is Tribute Summoned: You can target up to 2 Spell/Trap Cards on the field; destroy those targets.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Destroy two spell traps

`Mobius_the_Frost_Monarch_Destroy_Two_Spell_Traps_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Mobius the Frost Monarch |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Call of the Haunted | face-down (Set) |
| Opponent | Spell/Trap Zone 1 | Magic Jammer | face-down (Set) |

**Steps**

1. `tribute_summon you.hand[Mobius the Frost Monarch] -> m0 atk tributes you.m0`: Tribute Summon **Mobius the Frost Monarch** in your hand into your Monster Zone 0 (**Battle Ox**) (face-up Attack Position), Tributing your Monster Zone 0 (**Battle Ox**).
2. `yes`: Answer Yes.
3. `target opp.st0`: Pick the opponent's Spell/Trap Zone 0 (**Call of the Haunted**) as the target.
4. `target opp.st1`: Pick the opponent's Spell/Trap Zone 1 (**Magic Jammer**) as the target.

**Expected**

- `opp.spells count 0`: the opponent's Spells/Traps holds 0 card(s).
- `opp.gy = Call of the Haunted, Magic Jammer`: the opponent's Graveyard is exactly Call of the Haunted, Magic Jammer.
- `you.m0 = Mobius the Frost Monarch atk`: your Monster Zone 0 (**Battle Ox**) holds Mobius the Frost Monarch, face-up Attack Position.

#### CPU 1: Destroy human spell traps

`Mobius_the_Frost_Monarch_Cpu_Destroy_Human_Spell_Traps_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Mobius the Frost Monarch |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Spell/Trap Zone 0 | Call of the Haunted | face-down (Set) |
| You | Spell/Trap Zone 1 | Magic Jammer | face-down (Set) |

**Steps**

1. `when prompt response: no`: When the response prompt opens: answer No.
2. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.spells count 0`: your Spells/Traps holds 0 card(s).
- `opp.monsters has Mobius the Frost Monarch`: the opponent's monsters includes Mobius the Frost Monarch.

### Smashing Ground

- **Usage:** 57.9% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_SMASHING_GROUND`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Destroy the 1 face-up monster your opponent controls that has the highest DEF (your choice, if tied)." No target. Prepare: the opponent has a face-up monster. Resolve, like Widespread Ruin's highest-ATK search: the highest DEF among the opponent's face-up monsters (GetZoneCardDef); on a tie the human picks one of them with the cursor (another pick plays the error sound), the CPU takes the first. Then DestroyFieldCard + OnCardDestroyedByEffect.
- **YGOPRODeck (TCG) text:** Destroy the 1 face-up monster your opponent controls that has the highest DEF (your choice, if tied).
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Destroy highest def

`Smashing_Ground_Destroy_Highest_Def_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Giant Rat has DEF 1450, Celtic Guardian 1200.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Smashing Ground |  |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| Opponent | Monster Zone 1 | Giant Rat | face-up Attack Position |

**Steps**

1. `activate you.hand[Smashing Ground]`: Open the card menu of **Smashing Ground** in your hand and choose Activate.

**Expected**

- `opp.m0 = Celtic Guardian atk`: the opponent's Monster Zone 0 (**Celtic Guardian**) holds Celtic Guardian, face-up Attack Position.
- `opp.m1 = empty`: the opponent's Monster Zone 1 (**Giant Rat**) is empty.
- `opp.gy = Giant Rat`: the opponent's Graveyard is exactly Giant Rat.

#### CPU 1: Destroy highest def

`Smashing_Ground_Cpu_Destroy_Highest_Def_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Smashing Ground |  |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| You | Monster Zone 1 | Giant Rat | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m1 = empty`: your Monster Zone 1 (**Giant Rat**) is empty.
- `you.gy has Giant Rat`: your Graveyard includes Giant Rat.

### Blade Knight

- **Usage:** 51.3% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BLADE_KNIGHT`, `gAddedCardEffects`)
- **Coded behavior:** +400 ATK at 0–1 cards in hand is in card_stats.c. When its controller has no other monsters, Flip Effects of monsters destroyed by battle with it do not activate (BattleStage_TriggerFlipEffect).
- **YGOPRODeck (TCG) text:** Gains 400 ATK while you have 1 or less cards in your hand. If you control no other monsters, the effects of Flip monsters destroyed by battle with this card are negated.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Boost and negate flip

`Blade_Knight_Boost_And_Negate_Flip_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

With no other monster of yours, Man-Eater Bug's FLIP effect does not activate.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Blade Knight | face-up Attack Position |
| You | Hand 0 | Fissure |  |
| Opponent | Monster Zone 0 | Man-Eater Bug | face-down (Set) |

**Steps**

1. `check you.m0.atk = 2000`: Check: The face-up Attack Position of your Monster Zone 0 (**Blade Knight**) is 2000.
2. `battle`: Enter the Battle Phase.
3. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Blade Knight**): target the opponent's Monster Zone 0 (**Man-Eater Bug**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Man-Eater Bug**) is empty.
- `opp.gy = Man-Eater Bug`: the opponent's Graveyard is exactly Man-Eater Bug.
- `you.m0 = Blade Knight atk`: your Monster Zone 0 (**Blade Knight**) holds Blade Knight, face-up Attack Position.

#### CPU 1: Negate flip

`Blade_Knight_Cpu_Negate_Flip_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Blade Knight | face-up Attack Position |
| You | Monster Zone 0 | Man-Eater Bug | face-down (Set) |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Man-Eater Bug**) is empty.
- `opp.m0 = Blade Knight atk`: the opponent's Monster Zone 0 (**Blade Knight**) holds Blade Knight, face-up Attack Position.

### Mystic Swordsman LV2

- **Usage:** 40.8% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MYSTIC_SWORDSMAN_LV2`, `gAddedCardEffects`)
- **Coded behavior:** "At the End Phase of a turn in which this card destroyed a monster by battle: You can send this card to the GY; Special Summon 1 'Mystic Swordsman LV4' from your hand or Deck." BattleStage_DestroyMonsters (battle_phase2.c)  MysticSwordsmanAfterBattle: an LV2 (attacking or attacked) whose opponent's monster is destroyed by the battle gets its bit in gModDuel.battleKills. DuelPhase_End (battle_phase3.c)                EndPhase_MysticSwordsmanLevelUp: each marked zone that still holds a face-up LV2 queues a trigger (RESPONSE_NONE, its zone), then the marks are cleared. the trigger's resolve                          the human is asked (the CPU always says Yes), the LV2 is sent to the Graveyard and an LV4 is taken from the Deck (else the hand); the next step, once those commands ran (so the LV2's zone is free), Special Summons it (QueueSpecialSummonChoosePosition).
- **YGOPRODeck (TCG) text:** At the start of the Damage Step, if this card attacked a face-down Defense Position monster: Destroy that monster. During the End Phase, if this card destroyed a monster by battle this turn: You can send this face-up card to the Graveyard; Special Summon 1 "Mystic Swordsman LV4" from your hand or Deck.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroy face down

`Mystic_Swordsman_LV2_Destroy_Face_Down_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The card text's first effect (YGOPRODeck): it destroys a face-down Defense monster it attacks.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Mystic Swordsman LV2 | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Mystic Swordsman LV2**): target the opponent's Monster Zone 0 (**Summoned Skull**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.
- `you.lp = 8000`: LP of you = 8000.

#### Player 2: Level up declined

`Mystic_Swordsman_LV2_Level_Up_Declined_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Mystic Swordsman LV4 is not in the ROM, so answer No to the End Phase level-up.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Mystic Swordsman LV2 | face-up Attack Position |
| Opponent | Monster Zone 0 | Kuriboh | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Mystic Swordsman LV2**): target the opponent's Monster Zone 0 (**Kuriboh**).
3. `pass`: End your turn (its End Phase effects resolve).
4. `no`: Answer No.

**Expected**

- `opp.lp = 7400`: LP of opp = 7400.
- `you.m0 = Mystic Swordsman LV2 atk`: your Monster Zone 0 (**Mystic Swordsman LV2**) holds Mystic Swordsman LV2, face-up Attack Position.

#### CPU 1: Destroy face down

`Mystic_Swordsman_LV2_Cpu_Destroy_Face_Down_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Mystic Swordsman LV2 | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-down (Set) |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.gy = Summoned Skull`: your Graveyard is exactly Summoned Skull.

### Bottomless Trap Hole

- **Usage:** 36.8% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BOTTOMLESS_TRAP_HOLE`, `gAddedCardEffects`)
- **Coded behavior:** Normal Trap: "When your opponent Summons a monster(s) with 1500 or more ATK: Destroy that monster(s) with 1500 or more ATK, and if you do, banish it." Torrential Tribute's summon window (RESPONSE_SUMMONED, RESPONSE_FLIP_SUMMONED or RESPONSE_SPECIAL_SUMMONED, the summoned monster at loc0); Trap Hole's own handlers test their card numbers and only Normal / Flip Summons. The monster must be the opponent's, face up, with 1500 or more ATK, at activation and at resolution. It is banished (BanishFieldCard: no Graveyard, so no Graveyard effects).
- **YGOPRODeck (TCG) text:** When your opponent Summons a monster(s) with 1500 or more ATK: Destroy that monster(s) with 1500 or more ATK, and if you do, banish it.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Banish summoned monster

`Bottomless_Trap_Hole_Banish_Summoned_Monster_Player` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Bottomless Trap Hole | face-down (Set) |
| Opponent | Hand 0 | Gemini Elf |  |

**Steps**

1. `when summon opp.hand[Gemini Elf]: activate you.st0`: When **Gemini Elf** in the opponent's hand is summoned: open the card menu of your Spell/Trap Zone 0 (**Bottomless Trap Hole**) and choose Activate.

**Expected**

- `opp.banished = Gemini Elf`: the opponent's banished cards is exactly Gemini Elf.
- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `you.gy = Bottomless Trap Hole`: your Graveyard is exactly Bottomless Trap Hole.

#### CPU 1: Banish human summon

`Bottomless_Trap_Hole_Cpu_Banish_Human_Summon_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Bottomless Trap Hole | face-down (Set) |
| You | Hand 0 | Gemini Elf |  |

**Steps**

1. `summon you.hand[Gemini Elf] -> m0 atk`: Normal Summon **Gemini Elf** in your hand into your Monster Zone 0 (face-up Attack Position).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 is empty.
- `you.banished = Gemini Elf`: your banished cards is exactly Gemini Elf.
- `opp.gy = Bottomless Trap Hole`: the opponent's Graveyard is exactly Bottomless Trap Hole.

### Emergency Provisions

- **Usage:** 32.9% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_EMERGENCY_PROVISIONS`, `gAddedCardEffects`)
- **Coded behavior:** Quick-Play Spell: "Send any number of other Spell/Trap Cards you control to the Graveyard; gain 1000 LP for each card sent to the Graveyard this way." prepare  another Spell/Trap of the player's on the field (zones 5-10, the Field Spell too) chainA   the cost, counted in the link's unused unk12. The human: a prompt, then cursor picks of their other Spells/Traps, each sent at once (SendFieldCardToGrave), until B (after at least one) or none is left. The CPU: the ones the link it answers targets, when that targets its Spells/Traps (Mystical Space Typhoon, Dust Tornado, ...), else all of its others (against Heavy Storm, Harpie's Feather Duster, ...) resolve  GainLifePoints, 1000 a card CPU      a Set copy is chained (AiChainSolemnJudgment) to the human's Heavy Storm or Harpie's Feather Duster, or to a card that targets its Spells/Traps (AiChainEmergencyProvisions); from the hand in its Main Phase (AiWantsSpell) only with EP_AI_LOW_LP or less and 2 or more other Spells/Traps
- **YGOPRODeck (TCG) text:** Send any number of other Spell/Trap Cards you control to the Graveyard; gain 1000 LP for each card sent to the Graveyard this way.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsEmergencyProvisions`, `AiWantsSpell` Monster menu: only when low on LP Main Phase Spell: only when low on LP Set/chain: only when low on LP

#### Player 1: Send two gain 2000

`Emergency_Provisions_Send_Two_Gain_2000_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Emergency Provisions |  |
| You | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| You | Spell/Trap Zone 1 | Call of the Haunted | face-down (Set) |

**Steps**

1. `activate you.hand[Emergency Provisions]`: Open the card menu of **Emergency Provisions** in your hand and choose Activate.
2. `pick you.st0`: Pick your Spell/Trap Zone 0 (**Trap Hole**) in the list or on the field.
3. `pick you.st1`: Pick your Spell/Trap Zone 1 (**Call of the Haunted**) in the list or on the field.

**Expected**

- `you.lp = 10000`: LP of you = 10000.
- `you.gy = Trap Hole, Call of the Haunted, Emergency Provisions`: your Graveyard is exactly Trap Hole, Call of the Haunted, Emergency Provisions.
- `you.spells count 0`: your Spells/Traps holds 0 card(s).

#### CPU 1: Answer spell trap removal

`Emergency_Provisions_Cpu_Answer_Spell_Trap_Removal_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 1500 · opponent not frozen

With low LP the CPU chains it, sending the targeted Trap Hole for 1000 LP.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Mystical Space Typhoon |  |
| Opponent | Spell/Trap Zone 0 | Emergency Provisions | face-down (Set) |
| Opponent | Spell/Trap Zone 1 | Trap Hole | face-down (Set) |

**Steps**

1. `activate you.hand[Mystical Space Typhoon]`: Open the card menu of **Mystical Space Typhoon** in your hand and choose Activate.
2. `target opp.st1`: Pick the opponent's Spell/Trap Zone 1 (**Trap Hole**) as the target.

**Expected**

- `opp.lp = 2500`: LP of opp = 2500.
- `opp.gy has Emergency Provisions`: the opponent's Graveyard includes Emergency Provisions.
- `opp.gy has Trap Hole`: the opponent's Graveyard includes Trap Hole.

### Enemy Controller

- **Usage:** 25.0% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_ENEMY_CONTROLLER`, `gAddedCardEffects`)
- **Coded behavior:** Quick-Play Spell: "Activate 1 of these effects. - Target 1 face-up monster your opponent controls; change that target's battle position. - Tribute 1 monster, then target 1 face-up monster your opponent controls; take control of that target until the End Phase." prepare  the opponent has a face-up monster the card may target chainA   the choice (kept in the link's unused unk12): the human picks from a two-line menu, offered only with a monster to Tribute; the CPU takes control when it can, else changes the position chainB   control: first the Tribute (the cost), Catapult Turtle's EffectTributeTargetChainB pick, a monster that cannot be Tributed refused, Tributed at once (as Metamorphosis does); the CPU Tributes its weakest. Then the target: a face-up monster of the opponent's (the human's cursor; the CPU, for control the highest ATK, for the position its first one) resolve  the target must still be face up there: the position changes (ChangeBattlePosition), or Change of Heart's resolve moves it to the player and links it (ZONE_LINK_CARD_EFFECT from Enemy Controller), and DuelCmd_TurnEnd gives it back (ReturnControlAtTurnEnd, duel_cmd_turn.c). Unlike Mind Control's, the monster may attack and be Tributed.
- **YGOPRODeck (TCG) text:** Activate 1 of these effects.
● Target 1 face-up monster your opponent controls; change that target's battle position.
● Tribute 1 monster, then target 1 face-up monster your opponent controls; take control of that target until the End Phase.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsEnemyController`, `AiWantsSpell` Monster menu: to take a much stronger monster Main Phase Spell: to take a much stronger monster Set/chain: to take a much stronger monster

#### Player 1: Change position

`Enemy_Controller_Change_Position_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Enemy Controller |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.hand[Enemy Controller]`: Open the card menu of **Enemy Controller** in your hand and choose Activate.
2. `choose "Change battle position"`: Choose "Change battle position" in the menu.
3. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `opp.m0 = Summoned Skull def`: the opponent's Monster Zone 0 (**Summoned Skull**) holds Summoned Skull, face-up Defense Position.
- `you.gy = Enemy Controller`: your Graveyard is exactly Enemy Controller.

#### Player 2: Take control

`Enemy_Controller_Take_Control_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Enemy Controller |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.hand[Enemy Controller]`: Open the card menu of **Enemy Controller** in your hand and choose Activate.
2. `choose "Take control"`: Choose "Take control" in the menu.
3. `target you.m0`: Pick your Monster Zone 0 (**Battle Ox**) as the target.
4. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.
5. `check you.monsters = Summoned Skull`: Check: your monsters is exactly Summoned Skull.
6. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `opp.monsters = Summoned Skull`: the opponent's monsters is exactly Summoned Skull.
- `you.gy = Battle Ox, Enemy Controller`: your Graveyard is exactly Battle Ox, Enemy Controller.

#### CPU 1: Take stronger monster

`Enemy_Controller_Cpu_Take_Stronger_Monster_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

Control comes back at the End Phase.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Enemy Controller | face-down (Set) |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Enemy Controller`: the opponent's Graveyard includes Enemy Controller.
- `opp.gy has Battle Ox`: the opponent's Graveyard includes Battle Ox.
- `you.monsters = Summoned Skull`: your monsters is exactly Summoned Skull.

### Mystik Wok

- **Usage:** 23.7% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MYSTIK_WOK`, `gAddedCardEffects`)
- **Coded behavior:** Quick-Play Spell: "Tribute 1 monster. Select either its ATK or DEF and gain that many Life Points." prepare  at least one monster may be Tributed chainB   EffectTributeTargetChainB (the CPU: its weakest by ATK); ATK/DEF are read before TributeMonster resolve  the human picks ATK or DEF from a two-line menu; the CPU takes whichever is higher CPU      AiWantsMystikWok when LP is low and the best stat on its weakest Tribute is worth it
- **YGOPRODeck (TCG) text:** Tribute 1 monster. Select either its ATK or DEF and gain that many Life Points.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsMystikWok` Monster menu: Conditional AiWants* / AiShouldActivateAddedCard Main Phase Spell: Conditional AiWants* / AiShouldActivateAddedCard Set/chain: Conditional AiWants* / AiShouldActivateAddedCard

#### Player 1: Tribute gain def

`Mystik_Wok_Tribute_Gain_Def_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Mystik Wok |  |
| You | Monster Zone 0 | Giant Rat | face-up Attack Position |

**Steps**

1. `activate you.hand[Mystik Wok]`: Open the card menu of **Mystik Wok** in your hand and choose Activate.
2. `target you.m0`: Pick your Monster Zone 0 (**Giant Rat**) as the target.
3. `choose "DEF"`: Choose "DEF" in the menu.

**Expected**

- `you.lp = 9450`: LP of you = 9450.
- `you.gy = Giant Rat, Mystik Wok`: your Graveyard is exactly Giant Rat, Mystik Wok.

#### CPU 1: Low lp answer attack

`Mystik_Wok_Cpu_Low_Lp_Answer_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 1000 · opponent not frozen

It Tributes the attacked Celtic Guardian and takes its ATK (1400, higher than its DEF).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Mystik Wok | face-down (Set) |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent's Monster Zone 0 (**Celtic Guardian**).
3. `when replay: cancel`: When the attack is replayed (its target left the field): cancel (B): do not attack again / make no choice.

**Expected**

- `opp.lp = 2400`: LP of opp = 2400.
- `opp.gy has Celtic Guardian`: the opponent's Graveyard includes Celtic Guardian.
- `opp.gy has Mystik Wok`: the opponent's Graveyard includes Mystik Wok.

### Berserk Gorilla

- **Usage:** 22.4% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BERSERK_GORILLA`, `gAddedCardEffects`)
- **Coded behavior:** "If this card is in Defense Position: Destroy it. This card must attack, if able." Defense Position  ModDuelStateCheck (DuelMainStep, while nothing else is busy) destroys a face-up one in Defense Position (a face-down one's effect does not apply), except during an attack (battle stages 2-11: a Set one flipped by the attack is destroyed once the attack is over). Must attack       the human cannot end the Battle Phase (B in the attacker selection, card_menu_input.c) while one may still attack (BerserkGorillaBlocksBattleEnd), nor pick "Complete turn" in the Main Phase menu instead of entering it (BerserkGorillaBlocksTurnEnd, duel_phases.c); the CPU attacks with it when it has no better attack (AiPlanForcedAttack). AiShouldRetreatToDefense never turns it to Defense.
- **YGOPRODeck (TCG) text:** If this card is in face-up Defense Position, destroy this card. This card must attack if able.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroyed in defense

`Berserk_Gorilla_Destroyed_In_Defense_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Berserk Gorilla | face-up Attack Position |

**Steps**

1. `change_position you.m0`: Change the battle position of your Monster Zone 0 (**Berserk Gorilla**) (card menu).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Berserk Gorilla**) is empty.
- `you.gy = Berserk Gorilla`: your Graveyard is exactly Berserk Gorilla.

#### Player 2: Must attack

`Berserk_Gorilla_Must_Attack_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

B (cancel) cannot end the Battle Phase while the Gorilla can attack.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Berserk Gorilla | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `check menu you.m0 has Change Position`: Check: The card menu of your Monster Zone 0 (**Berserk Gorilla**) offers Change Position.
2. `battle`: Enter the Battle Phase.
3. `cancel`: Cancel (B): do not attack again / make no choice.
4. `check you.m0 = Berserk Gorilla atk`: Check: your Monster Zone 0 (**Berserk Gorilla**) holds Berserk Gorilla, face-up Attack Position.
5. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Berserk Gorilla**): target the opponent's Monster Zone 0 (**Summoned Skull**).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Berserk Gorilla**) is empty.
- `you.lp = 7500`: LP of you = 7500.

#### CPU 1: Must attack

`Berserk_Gorilla_Cpu_Must_Attack_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Berserk Gorilla | face-up Attack Position |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Celtic Guardian**) is empty.
- `you.lp = 7400`: LP of you = 7400.

### Ninja Grandmaster Sasuke

- **Usage:** 21.1% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_NINJA_GRANDMASTER_SASUKE`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** At the start of the Damage Step, if this card attacks a face-up Defense Position monster: Destroy that monster.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Destroy face up defense

`Ninja_Grandmaster_Sasuke_Destroy_Face_Up_Defense_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Ninja Grandmaster Sasuke | face-up Attack Position |
| Opponent | Monster Zone 0 | Big Shield Gardna | face-up Defense Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Ninja Grandmaster Sasuke**): target the opponent's Monster Zone 0 (**Big Shield Gardna**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Big Shield Gardna**) is empty.
- `opp.gy = Big Shield Gardna`: the opponent's Graveyard is exactly Big Shield Gardna.
- `you.lp = 8000`: LP of you = 8000.

#### CPU 1: Destroy face up defense

`Ninja_Grandmaster_Sasuke_Cpu_Destroy_Face_Up_Defense_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Ninja Grandmaster Sasuke | face-up Attack Position |
| You | Monster Zone 0 | Big Shield Gardna | face-up Defense Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Big Shield Gardna**) is empty.
- `you.gy = Big Shield Gardna`: your Graveyard is exactly Big Shield Gardna.

### D.D. Assailant

- **Usage:** 19.7% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** Unknown / stats only
- **Coded behavior:** No `gAddedCardEffects` row or engine key found for this demo card; verify in Deck Edit and trunk.
- **YGOPRODeck (TCG) text:** After damage calculation, when this card is destroyed by battle with an opponent's monster: Banish that monster, also banish this card.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Unclear / likely missing** — No `CPU:` notes and no switch case found; treat as untested for CPU until you confirm in a duel.

#### Player 1: Banish both when destroyed

`DD_Assailant_Banish_Both_When_Destroyed_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

No effect row was found for this card: this scenario checks the card text.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | D.D. Assailant | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**D.D. Assailant**): target the opponent's Monster Zone 0 (**Summoned Skull**).

**Expected**

- `you.lp = 7200`: LP of you = 7200.
- `you.banished = D.D. Assailant`: your banished cards is exactly D.D. Assailant.
- `opp.banished = Summoned Skull`: the opponent's banished cards is exactly Summoned Skull.

#### CPU 1: Banish both when destroyed

`DD_Assailant_Cpu_Banish_Both_When_Destroyed_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 0 | D.D. Assailant | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent's Monster Zone 0 (**D.D. Assailant**).

**Expected**

- `opp.lp = 7200`: LP of opp = 7200.
- `you.banished = Summoned Skull`: your banished cards is exactly Summoned Skull.
- `opp.banished = D.D. Assailant`: the opponent's banished cards is exactly D.D. Assailant.

### Return from the Different Dimension

- **Usage:** 15.8% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION`, `gAddedCardEffects`)
- **Coded behavior:** Normal Trap: "Pay half your Life Points; Special Summon as many of your banished monsters as possible. During the End Phase, banish all monsters that were Special Summoned by this effect." prepare  a free monster zone and a monster in the player's banished pile (banished normally: not face down or until the End Phase; not a token) chainA   half the LP (DUEL_CMD_LOSE_LP, a cost) resolve  one monster per step while zones and monsters remain, the strongest ATK first: out of the banished pile (RemoveBanishedCardAt) and Special Summoned face up in the position the player picks (QueueSpecialSummonChoosePosition; the chain waits for each summon). Each one's zone and card ID go to gModDuel.dimensionReturn. DuelPhase_End (battle_phase3.c)  EndPhase_ReturnFromTheDifferentDimension banishes the marked monsters still in their zones, then clears the marks. A monster that cannot be Special Summoned (IsCannotSpecialSummon) stays banished: QueueSpecialSummonChoosePosition would drop it after it left the pile. Dimension Fusion shares the helpers.
- **YGOPRODeck (TCG) text:** Pay half your Life Points; Special Summon as many of your banished monsters as possible. During the End Phase, banish all monsters that were Special Summoned by this effect.
- **AI coverage:** **Yes — generalized routine** — Monster menu: two or more monsters back, with LP to spare Main Phase Spell: two or more monsters back, with LP to spare Set/chain: two or more monsters back, with LP to spare

#### Player 1: Return banished then banish

`Return_from_the_Different_Dimension_Return_Banished_Then_Banish_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

The strongest ATK comes back first: Summoned Skull, Battle Ox, Celtic Guardian.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Return from the Different Dimension | face-down (Set) |
| You | Banished | Summoned Skull |  |
| You | Banished | Battle Ox |  |
| You | Banished | Celtic Guardian |  |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Return from the Different Dimension**) and choose Activate.
2. `zone m0`: Choose your Monster Zone 0 for the summoned monster.
3. `position atk`: Choose face-up Attack Position for the summoned monster.
4. `zone m1`: Choose your Monster Zone 1 for the summoned monster.
5. `position atk`: Choose face-up Attack Position for the summoned monster.
6. `zone m2`: Choose your Monster Zone 2 for the summoned monster.
7. `position atk`: Choose face-up Attack Position for the summoned monster.
8. `check you.lp = 4000`: Check: LP of you = 4000.
9. `check you.monsters = Summoned Skull, Battle Ox, Celtic Guardian`: Check: your monsters is exactly Summoned Skull, Battle Ox, Celtic Guardian.
10. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `you.banished = Summoned Skull, Battle Ox, Celtic Guardian`: your banished cards is exactly Summoned Skull, Battle Ox, Celtic Guardian.

#### CPU 1: Return banished

`Return_from_the_Different_Dimension_Cpu_Return_Banished_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Return from the Different Dimension | face-down (Set) |
| Opponent | Banished | Summoned Skull |  |
| Opponent | Banished | Battle Ox |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.lp = 4000`: LP of opp = 4000.
- `opp.banished = Summoned Skull, Battle Ox`: the opponent's banished cards is exactly Summoned Skull, Battle Ox.
- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).

### Monster Gate

- **Usage:** 13.2% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MONSTER_GATE`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Tribute 1 monster; excavate cards from the top of your Deck until you excavate a monster that can be Normal Summoned/Set. Special Summon it, also send the other excavated cards to the GY." prepare  a monster the player may Tribute, and a Deck monster that can be Normal Summoned/Set chainB   the cost: Catapult Turtle's EffectTributeTargetChainB pick (a monster that cannot be Tributed is refused), Tributed at once as Metamorphosis does; the CPU Tributes its weakest monster resolve  from the top of the Deck (deck[0]): the cards before the first such monster go to the Graveyard, the monster leaves the Deck; the next step (the commands done) Special Summons it face up in the position the player picks (QueueSpecialSummonChoosePosition). With none, every card goes.
- **YGOPRODeck (TCG) text:** Tribute 1 monster; excavate cards from the top of your Deck until you excavate a monster that can be Normal Summoned/Set. Special Summon it, also send the other excavated cards to the GY.
- **AI coverage:** **Yes — generalized routine** — Monster menu: Tribute a weak monster for whatever the Deck gives Main Phase Spell: Tribute a weak monster for whatever the Deck gives Set/chain: Tribute a weak monster for whatever the Deck gives

#### Player 1: Tribute excavate summon

`Monster_Gate_Tribute_Excavate_Summon_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Monster Gate |  |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| You | Deck 0 | Fissure |  |
| You | Deck 1 | Trap Hole |  |
| You | Deck 2 | Summoned Skull |  |

**Steps**

1. `activate you.hand[Monster Gate]`: Open the card menu of **Monster Gate** in your hand and choose Activate.
2. `target you.m0`: Pick your Monster Zone 0 (**Celtic Guardian**) as the target.
3. `zone m0`: Choose your Monster Zone 0 (**Celtic Guardian**) for the summoned monster.
4. `position atk`: Choose face-up Attack Position for the summoned monster.

**Expected**

- `you.m0 = Summoned Skull atk`: your Monster Zone 0 (**Celtic Guardian**) holds Summoned Skull, face-up Attack Position.
- `you.gy = Monster Gate, Celtic Guardian, Fissure, Trap Hole`: your Graveyard is exactly Monster Gate, Celtic Guardian, Fissure, Trap Hole.

#### CPU 1: Tribute excavate summon

`Monster_Gate_Cpu_Tribute_Excavate_Summon_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Monster Gate |  |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| Opponent | Deck 0 | Fissure |  |
| Opponent | Deck 1 | Summoned Skull |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.monsters has Summoned Skull`: the opponent's monsters includes Summoned Skull.
- `opp.gy has Fissure`: the opponent's Graveyard includes Fissure.
- `opp.gy has Monster Gate`: the opponent's Graveyard includes Monster Gate.

### Monster Reincarnation

- **Usage:** 13.2% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MONSTER_REINCARNATION`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Discard 1 card, then target 1 monster in your Graveyard; add that target to your hand." prepare  at least one card in the hand to discard and a monster in the Graveyard (ModCollectEffectTargets) chainA   DuelPrompt_PostDiscardCost (any card; the CPU picks its own) chainB   EffectCardListTargetChainB (graveyard monsters) resolve  EffectMaskOfDarknessResolve (DUEL_CMD_RETURN_GRAVEYARD_CARD_TO_HAND) CPU      AiWantsMonsterReincarnation (empty monster field only); AiPickCardListEntry prefers a monster that beats the human's strongest field monster when they control one
- **YGOPRODeck (TCG) text:** Discard 1 card, then target 1 monster in your GY; add it to your hand.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsMonsterReincarnation` Monster menu: Conditional AiWants* / AiShouldActivateAddedCard Main Phase Spell: Conditional AiWants* / AiShouldActivateAddedCard Set/chain: Conditional AiWants* / AiShouldActivateAddedCard

#### Player 1: Discard add monster

`Monster_Reincarnation_Discard_Add_Monster_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Monster Reincarnation |  |
| You | Hand 1 | Fissure |  |
| You | Graveyard | Summoned Skull |  |

**Steps**

1. `activate you.hand[Monster Reincarnation]`: Open the card menu of **Monster Reincarnation** in your hand and choose Activate.
2. `discard you.hand[Fissure]`: Discard **Fissure** in your hand for the cost.
3. `pick you.gy[Summoned Skull]`: Pick **Summoned Skull** in your Graveyard in the list or on the field.

**Expected**

- `you.hand = Summoned Skull`: your hand is exactly Summoned Skull.
- `you.gy = Monster Reincarnation, Fissure`: your Graveyard is exactly Monster Reincarnation, Fissure.

#### CPU 1: Add monster empty field

`Monster_Reincarnation_Cpu_Add_Monster_Empty_Field_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Monster Reincarnation |  |
| Opponent | Hand 1 | Fissure |  |
| Opponent | Graveyard | Summoned Skull |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Monster Reincarnation`: the opponent's Graveyard includes Monster Reincarnation.
- `opp.gy lacks Summoned Skull`: the opponent's Graveyard does not include Summoned Skull.

### Pikeru's Circle of Enchantment

- **Usage:** 13.2% weighted inclusion in `usage/warrior.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT`, `gAddedCardEffects`)
- **Coded behavior:** Normal Trap: "Damage to you from card effects becomes 0 until the end of this turn." resolve  gModDuel.noEffectDamageTurn[player] = this turn: ModTakesNoEffectDamage stops the damage of LoseLifePoints (every effect damage) for the player until the turn ends. Battle damage (InflictBattleDamage) and LP paid as a cost are not effect damage. CPU      only chained to the human's burn Spell or Trap (AiChainPikerusCircle, in AiChainSolemnJudgment before Hallowed Life Barrier, which costs a card): PIKERU_AI_MIN_BURN or more, or all of its LP (AiIncomingBurn); AiShouldActivateAddedCard keeps it Set otherwise
- **YGOPRODeck (TCG) text:** Damage to you from card effects becomes 0 until the end of this turn.
- **AI coverage:** **Yes — dedicated logic** — Monster menu: only chained to burn (AiChainPikerusCircle) Main Phase Spell: only chained to burn (AiChainPikerusCircle) Set/chain: only chained to burn (AiChainPikerusCircle)

#### Player 1: No effect damage

`Pikerus_Circle_of_Enchantment_No_Effect_Damage_Player` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Pikeru's Circle of Enchantment | face-down (Set) |
| Opponent | Hand 0 | Poison of the Old Man |  |

**Steps**

1. `when activate opp.hand[Poison of the Old Man]: activate you.st0`: When **Poison of the Old Man** in the opponent's hand is activated: open the card menu of your Spell/Trap Zone 0 (**Pikeru's Circle of Enchantment**) and choose Activate.

**Expected**

- `you.lp = 8000`: LP of you = 8000.
- `you.gy = Pikeru's Circle of Enchantment`: your Graveyard is exactly Pikeru's Circle of Enchantment.
- `opp.gy = Poison of the Old Man`: the opponent's Graveyard is exactly Poison of the Old Man.

#### CPU 1: Chain to lethal burn

`Pikerus_Circle_of_Enchantment_Cpu_Chain_To_Lethal_Burn_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 800 · opponent not frozen

The 800 damage is all of its LP, so the CPU chains Pikeru.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Poison of the Old Man |  |
| Opponent | Spell/Trap Zone 0 | Pikeru's Circle of Enchantment | face-down (Set) |

**Steps**

1. `activate you.hand[Poison of the Old Man]`: Open the card menu of **Poison of the Old Man** in your hand and choose Activate.
2. `choose "Inflict 800 damage"`: Choose "Inflict 800 damage" in the menu.

**Expected**

- `opp.lp = 800`: LP of opp = 800.
- `opp.gy = Pikeru's Circle of Enchantment`: the opponent's Graveyard is exactly Pikeru's Circle of Enchantment.

## Goat

### Mind Control

- **Usage:** 66.5% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MIND_CONTROL`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Target 1 monster your opponent controls; take control of it until the End Phase, but it cannot declare an attack or be Tributed." The row is Change of Heart's: EffectChangeOfHeartPrepare, EffectOpponentMonsterCheck, EffectTakeControlChainB and EffectChangeOfHeartResolve, which moves the monster to a free zone of the player and gives it a ZONE_LINK_CARD_EFFECT link from the activating card, here Mind Control. That link is what the rest keys on: DuelCmd_TurnEnd (duel_cmd_turn.c)   ReturnControlAtTurnEnd(player, zone, CARD_MIND_CONTROL), as for Change of Heart CanMonsterAttack (card_menu_input.c) and IsTributableMonster (duel_zones.c)   IsMindControlled refuses it.
- **YGOPRODeck (TCG) text:** Target 1 monster your opponent controls; until the End Phase, take control of that target, but it cannot declare an attack or be Tributed.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Take control no attack

`Mind_Control_Take_Control_No_Attack_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Mind Control |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.hand[Mind Control]`: Open the card menu of **Mind Control** in your hand and choose Activate.
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.
3. `check you.monsters = Summoned Skull`: Check: your monsters is exactly Summoned Skull.
4. `battle`: Enter the Battle Phase.
5. `check cannot attack you.m[Summoned Skull] -> direct`: Check: your **Summoned Skull** cannot attack the opponent directly.
6. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `opp.monsters = Summoned Skull`: the opponent's monsters is exactly Summoned Skull.
- `opp.lp = 8000`: LP of opp = 8000.

#### CPU 1: Take control

`Mind_Control_Cpu_Take_Control_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

The CPU takes Summoned Skull for its turn but cannot attack with it; it comes back at the End Phase.

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Mind Control |  |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Mind Control`: the opponent's Graveyard includes Mind Control.
- `you.monsters = Summoned Skull`: your monsters is exactly Summoned Skull.
- `you.lp = 8000`: LP of you = 8000.

### Night Assailant

- **Usage:** 52.8% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_NIGHT_ASSAILANT`, `gAddedCardEffects`)
- **Coded behavior:** "FLIP: Target 1 monster your opponent controls; destroy that target. When this card is sent from your hand to the GY: Target 1 Flip monster in your GY, except 'Night Assailant'; add it to your hand." One row, told apart by the link's event: flip                 a flip effect (HasFlipEffect, card_detail.c): Crass Clown's opponent-monster target (EffectOpponentMonsterCheck / ChainB), Man-Eater Bug's EffectDestroyTargetResolve RESPONSE_DISCARDED   the off-field trigger that DiscardHandCard (duel_field_moves.c) queues through QueueNightAssailantTrigger, only when the Graveyard holds a target; the list (CollectEffectTargets, effect_target_collect.c: IsNightAssailantTarget) goes through Mask of Darkness's card-list viewer and EffectMaskOfDarknessResolve. A "Flip monster" is one that HasFlipEffect lists (the coded flip effects).
- **YGOPRODeck (TCG) text:** FLIP: Target 1 monster your opponent controls; destroy that target.
When this card is sent from the hand to the Graveyard: Target 1 Flip Effect Monster in your Graveyard, except this card; return that target to the hand.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Flip destroy

`Night_Assailant_Flip_Destroy_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Night Assailant | face-down (Set) |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `flip_summon you.m0`: Flip Summon your Monster Zone 0 (**Night Assailant**).
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.

#### Player 2: Discarded add flip monster

`Night_Assailant_Discarded_Add_Flip_Monster_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Lightning Vortex |  |
| You | Hand 1 | Night Assailant |  |
| You | Graveyard | Man-Eater Bug |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.hand[Lightning Vortex]`: Open the card menu of **Lightning Vortex** in your hand and choose Activate.
2. `discard you.hand[Night Assailant]`: Discard **Night Assailant** in your hand for the cost.
3. `pick you.gy[Man-Eater Bug]`: Pick **Man-Eater Bug** in your Graveyard in the list or on the field.

**Expected**

- `you.hand = Man-Eater Bug`: your hand is exactly Man-Eater Bug.
- `you.gy = Night Assailant, Lightning Vortex`: your Graveyard is exactly Night Assailant, Lightning Vortex.
- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.

#### CPU 1: Flipped by attack

`Night_Assailant_Cpu_Flipped_By_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

Flipped by the attack, its FLIP effect destroys one of your monsters.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| You | Monster Zone 1 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Night Assailant | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m1 -> opp.m0`: Attack with your Monster Zone 1 (**Battle Ox**): target the opponent's Monster Zone 0 (**Night Assailant**).

**Expected**

- `you.monsters count 1`: your monsters holds 1 card(s).
- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Night Assailant**) is empty.

### Dekoichi the Battlechanted Locomotive

- **Usage:** 52.4% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE`, `gAddedCardEffects`)
- **Coded behavior:** "FLIP: Draw 1 card, then draw 1 more card for each 'Bokoichi the Freightening Car' you control." A flip effect (HasFlipEffect, card_detail.c). Bokoichi counts only when it is one of the added cards.
- **YGOPRODeck (TCG) text:** FLIP: Draw 1 card, then draw 1 additional card for each face-up "Bokoichi the Freightening Car" you control.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Flip draw one

`Dekoichi_the_Battlechanted_Locomotive_Flip_Draw_One_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Bokoichi the Freightening Car is not in the ROM, so it is always 1 card.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Dekoichi the Battlechanted Locomotive | face-down (Set) |
| You | Deck 0 | Battle Ox |  |

**Steps**

1. `flip_summon you.m0`: Flip Summon your Monster Zone 0 (**Dekoichi the Battlechanted Locomotive**).

**Expected**

- `you.hand = Battle Ox`: your hand is exactly Battle Ox.
- `you.m0 = Dekoichi the Battlechanted Locomotive atk`: your Monster Zone 0 (**Dekoichi the Battlechanted Locomotive**) holds Dekoichi the Battlechanted Locomotive, face-up Attack Position.

#### CPU 1: Flipped by attack

`Dekoichi_the_Battlechanted_Locomotive_Cpu_Flipped_By_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 0 | Dekoichi the Battlechanted Locomotive | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent's Monster Zone 0 (**Dekoichi the Battlechanted Locomotive**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Dekoichi the Battlechanted Locomotive**) is empty.
- `opp.hand count 1`: the opponent's hand holds 1 card(s).

### Lightning Vortex

- **Usage:** 32.5% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_LIGHTNING_VORTEX`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Discard 1 card; destroy all face-up monsters your opponent controls." Prepare: a card to discard (2 in hand when played from the hand) and a face-up monster on the opponent's side. ChainA: the discard cost of Magic Jammer / Tribe-Infecting Virus (DuelPrompt_PostDiscardCost). Resolve: no target; every face-up monster the opponent controls then is destroyed (DestroyFieldCard + OnCardDestroyedByEffect).
- **YGOPRODeck (TCG) text:** Discard 1 card; destroy all face-up monsters your opponent controls.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Destroy face up

`Lightning_Vortex_Destroy_Face_Up_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Lightning Vortex |  |
| You | Hand 1 | Fissure |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 1 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 2 | Man-Eater Bug | face-down (Set) |

**Steps**

1. `activate you.hand[Lightning Vortex]`: Open the card menu of **Lightning Vortex** in your hand and choose Activate.
2. `discard you.hand[Fissure]`: Discard **Fissure** in your hand for the cost.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.m1 = empty`: the opponent's Monster Zone 1 (**Battle Ox**) is empty.
- `opp.m2 = Man-Eater Bug set`: the opponent's Monster Zone 2 (**Man-Eater Bug**) holds Man-Eater Bug, face-down (Set).
- `you.gy = Fissure, Lightning Vortex`: your Graveyard is exactly Fissure, Lightning Vortex.

#### CPU 1: Destroy face up

`Lightning_Vortex_Cpu_Destroy_Face_Up_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Lightning Vortex |  |
| Opponent | Hand 1 | Fissure |  |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| You | Monster Zone 1 | Battle Ox | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `opp.gy has Lightning Vortex`: the opponent's Graveyard includes Lightning Vortex.

### Phoenix Wing Wind Blast

- **Usage:** 15.5% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_PHOENIX_WING_WIND_BLAST`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Discard 1 card, then target 1 card your opponent controls; place that target on the top of the Deck." Cost: the discard of Magic Jammer / Tribe-Infecting Virus (DuelPrompt_PostDiscardCost). Target: any card in the opponent's monster, Spell/Trap or Field zones that the card may target (CanCardTargetZone), as Raigeki Break but on one side. The target step does not ask CanActivateEffect again: the discard is paid by then, and the prepare handler counts the hand. Resolve: ReturnFieldCardToDeck (to the top of its owner's Deck; a Fusion Monster goes to the Fusion Deck and a token is destroyed, the game's rules for DUEL_CMD_RETURN_TO_DECK).
- **YGOPRODeck (TCG) text:** Discard 1 card, then target 1 card your opponent controls; place that target on the top of the Deck.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Return to deck top

`Phoenix_Wing_Wind_Blast_Return_To_Deck_Top_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Phoenix Wing Wind Blast | face-down (Set) |
| You | Hand 0 | Fissure |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Phoenix Wing Wind Blast**) and choose Activate.
2. `discard you.hand[Fissure]`: Discard **Fissure** in your hand for the cost.
3. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.deck[0] = Summoned Skull`: the opponent's Deck slot 0 is exactly Summoned Skull.
- `you.gy = Fissure, Phoenix Wing Wind Blast`: your Graveyard is exactly Fissure, Phoenix Wing Wind Blast.

#### CPU 1: Return to deck top

`Phoenix_Wing_Wind_Blast_Cpu_Return_To_Deck_Top_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Phoenix Wing Wind Blast | face-down (Set) |
| Opponent | Hand 0 | Fissure |  |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `you.deck[0] = Summoned Skull`: your Deck slot 0 is exactly Summoned Skull.

### Xing Zhen Hu

- **Usage:** 15.2% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_XING_ZHEN_HU`, `gAddedCardEffects`)
- **Coded behavior:** Continuous Trap: "Activate by selecting 2 Set Spell or Trap Cards on the field. The selected Spell or Trap Cards cannot be activated." (the card data's text) Like Spellbinding Circle on a monster: the resolve gives each target still Set a ZONE_LINK_CONTINUOUS link whose value is Xing Zhen Hu's DUEL_LOC. CanActivateEffect (effect_activation.c) asks IsLockedByXingZhenHu first, so a locked Set card is refused everywhere (card menu, response windows, CPU). The lock holds while that zone holds a face-up, enabled Xing Zhen Hu; a target that leaves takes its links with it (ClearZone). prepare  two Set Spell/Trap cards on the field besides this one (the Set Xing Zhen Hu counts as Set) chainB   two different Set cards with the cursor (B starts again); the CPU takes the human's
- **YGOPRODeck (TCG) text:** Activate by selecting 2 Set Spell or Trap Cards on the field. The selected Spell or Trap Cards cannot be activated.
- **AI coverage:** **Yes — generalized routine** — Monster menu: only on two of the human's Set cards Main Phase Spell: only on two of the human's Set cards Set/chain: only on two of the human's Set cards

#### Player 1: Lock two set cards

`Xing_Zhen_Hu_Lock_Two_Set_Cards_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

The locked Trap Hole cannot answer your summon (the opponent is not frozen).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Xing Zhen Hu | face-down (Set) |
| You | Hand 0 | Gemini Elf |  |
| Opponent | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| Opponent | Spell/Trap Zone 1 | Call of the Haunted | face-down (Set) |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Xing Zhen Hu**) and choose Activate.
2. `target opp.st0`: Pick the opponent's Spell/Trap Zone 0 (**Trap Hole**) as the target.
3. `target opp.st1`: Pick the opponent's Spell/Trap Zone 1 (**Call of the Haunted**) as the target.
4. `pass`: End your turn (its End Phase effects resolve).
5. `cpu_turn`: The CPU plays its whole turn (not frozen).
6. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.
7. `summon you.hand[Gemini Elf] -> m0 atk`: Normal Summon **Gemini Elf** in your hand into your Monster Zone 0 (face-up Attack Position).

**Expected**

- `you.m0 = Gemini Elf atk`: your Monster Zone 0 holds Gemini Elf, face-up Attack Position.
- `opp.st0 = Trap Hole set`: the opponent's Spell/Trap Zone 0 (**Trap Hole**) holds Trap Hole, face-down (Set).
- `you.st0 = Xing Zhen Hu up`: your Spell/Trap Zone 0 (**Xing Zhen Hu**) holds Xing Zhen Hu, face up.

#### CPU 1: Lock human set cards

`Xing_Zhen_Hu_Cpu_Lock_Human_Set_Cards_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Xing Zhen Hu | face-down (Set) |
| You | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |
| You | Spell/Trap Zone 1 | Call of the Haunted | face-down (Set) |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).
2. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.

**Expected**

- `opp.st0 = Xing Zhen Hu up`: the opponent's Spell/Trap Zone 0 (**Xing Zhen Hu**) holds Xing Zhen Hu, face up.
- `menu you.st0 lacks Activate`: The card menu of your Spell/Trap Zone 0 (**Trap Hole**) does not offer Activate.
- `menu you.st1 lacks Activate`: The card menu of your Spell/Trap Zone 1 (**Call of the Haunted**) does not offer Activate.

### Brain Control

- **Usage:** 12.8% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_BRAIN_CONTROL`, `gAddedCardEffects`)
- **Coded behavior:** Normal Spell: "Pay 800 LP, then target 1 face-up monster your opponent controls that can be Normal Summoned/Set; take control of it until the End Phase." Change of Heart's way, like Mind Control: EffectChangeOfHeartResolve moves the target and links it to this card (ZONE_LINK_CARD_EFFECT), and DuelCmd_TurnEnd returns it (ReturnControlAtTurnEnd with CARD_BRAIN_CONTROL). prepare  more than 800 LP, a free monster zone, and a monster IsBrainControlTarget accepts chainA   the 800 LP (DUEL_CMD_LOSE_LP as a cost) chainB   EffectTakeControlChainB with Snatch Steal's face-up cursor (effect_targets2.c), wrapped: a monster that cannot be Normal Summoned/Set (a Fusion, Ritual or Special-Summon-only monster, a token) is refused, and the CPU, whose pick is the human's strongest monster, takes the strongest valid one instead
- **YGOPRODeck (TCG) text:** Pay 800 LP, then target 1 face-up monster your opponent controls that can be Normal Summoned/Set; take control of that target until the End Phase.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Pay 800 take control

`Brain_Control_Pay_800_Take_Control_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Brain Control |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.hand[Brain Control]`: Open the card menu of **Brain Control** in your hand and choose Activate.
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.
3. `check you.lp = 7200`: Check: LP of you = 7200.
4. `battle`: Enter the Battle Phase.
5. `attack you.m[Summoned Skull] -> direct`: Attack with your **Summoned Skull**: target the opponent directly.
6. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `opp.lp = 5500`: LP of opp = 5500.
- `opp.monsters = Summoned Skull`: the opponent's monsters is exactly Summoned Skull.

#### CPU 1: Pay 800 take control

`Brain_Control_Cpu_Pay_800_Take_Control_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Brain Control |  |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.lp = 7200`: LP of opp = 7200.
- `you.lp = 5500`: LP of you = 5500.
- `you.monsters = Summoned Skull`: your monsters is exactly Summoned Skull.

### Zaborg the Thunder Monarch

- **Usage:** 10.6% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_ZABORG_THE_THUNDER_MONARCH`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** If this card is Tribute Summoned: Target 1 monster on the field; destroy that target.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Tribute summon destroy

`Zaborg_the_Thunder_Monarch_Tribute_Summon_Destroy_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Zaborg the Thunder Monarch |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `tribute_summon you.hand[Zaborg the Thunder Monarch] -> m0 atk tributes you.m0`: Tribute Summon **Zaborg the Thunder Monarch** in your hand into your Monster Zone 0 (**Battle Ox**) (face-up Attack Position), Tributing your Monster Zone 0 (**Battle Ox**).
2. `target opp.m0`: Pick the opponent's Monster Zone 0 (**Summoned Skull**) as the target.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.
- `you.m0 = Zaborg the Thunder Monarch atk`: your Monster Zone 0 (**Battle Ox**) holds Zaborg the Thunder Monarch, face-up Attack Position.

#### CPU 1: Tribute summon destroy

`Zaborg_the_Thunder_Monarch_Cpu_Tribute_Summon_Destroy_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Zaborg the Thunder Monarch |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Summoned Skull**) is empty.
- `opp.monsters has Zaborg the Thunder Monarch`: the opponent's monsters includes Zaborg the Thunder Monarch.

### Level Limit - Area B

- **Usage:** 10.3% weighted inclusion in `usage/goat.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_LEVEL_LIMIT_AREA_B`, `gAddedCardEffects`)
- **Coded behavior:** Effect text (YGOPRODeck API): "Change all face-up Level 4 or higher monsters to Defense Position." Continuous Spell in the card pool; stays on the field when activated. Like Dragon Capture Jar (328): activation and ApplyLevelLimitAreaB after a face-up summon; ModDuelStateCheck keeps attack-position Level 4+ down while it is active; the human cannot manually switch a face-up Level 4+ monster to Attack (card_command_menu.c).
- **YGOPRODeck (TCG) text:** Change all face-up Level 4 or higher monsters to Defense Position.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Level 4 to defense

`Level_Limit_Area_B_Level_4_To_Defense_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Level Limit - Area B |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 1 | Man-Eater Bug | face-up Attack Position |

**Steps**

1. `activate you.hand[Level Limit - Area B]`: Open the card menu of **Level Limit - Area B** in your hand and choose Activate.

**Expected**

- `you.m0 = Battle Ox def`: your Monster Zone 0 (**Battle Ox**) holds Battle Ox, face-up Defense Position.
- `opp.m0 = Summoned Skull def`: the opponent's Monster Zone 0 (**Summoned Skull**) holds Summoned Skull, face-up Defense Position.
- `opp.m1 = Man-Eater Bug atk`: the opponent's Monster Zone 1 (**Man-Eater Bug**) holds Man-Eater Bug, face-up Attack Position.
- `menu you.m0 lacks Change Position`: The card menu of your Monster Zone 0 (**Battle Ox**) does not offer Change Position.

#### CPU 1: Level 4 to defense

`Level_Limit_Area_B_Cpu_Level_4_To_Defense_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Level Limit - Area B |  |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = Summoned Skull def`: your Monster Zone 0 (**Summoned Skull**) holds Summoned Skull, face-up Defense Position.

## Cyber

### Cyber Twin Dragon

- **Usage:** 95.6% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CYBER_TWIN_DRAGON`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** "Cyber Dragon" + "Cyber Dragon"
A Fusion Summon of this card can only be done with the above Fusion Materials. This card can make a second attack during each Battle Phase.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Second attack

`Cyber_Twin_Dragon_Second_Attack_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Cyber Twin Dragon | face-up Attack Position |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| Opponent | Monster Zone 1 | Battle Ox | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Cyber Twin Dragon**): target the opponent's Monster Zone 0 (**Celtic Guardian**).
3. `attack you.m0 -> opp.m1`: Attack with your Monster Zone 0 (**Cyber Twin Dragon**): target the opponent's Monster Zone 1 (**Battle Ox**).

**Expected**

- `opp.monsters count 0`: the opponent's monsters holds 0 card(s).
- `opp.lp = 5500`: LP of opp = 5500.

#### CPU 1: Second attack

`Cyber_Twin_Dragon_Cpu_Second_Attack_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Cyber Twin Dragon | face-up Attack Position |
| You | Monster Zone 0 | Celtic Guardian | face-up Attack Position |
| You | Monster Zone 1 | Battle Ox | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.monsters count 0`: your monsters holds 0 card(s).
- `you.lp = 5500`: LP of you = 5500.

### Cyber Dragon

- **Usage:** 84.4% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CYBER_DRAGON`, `gAddedCardEffects`)
- **Coded behavior:** "If only your opponent controls a monster, you can Special Summon this card (from your hand)." Like key 1350 (the original's Fiend Megacyber rule): CanSummonFromHand (summon_checks.c) allows it, the hand card menu (card_command_menu.c) adds the Special Summon commands, and the summon steps (effect_hooks.c) queue QueueSpecialSummonFromHand for them (no Tribute). It is still a Level 5 Normal Summon with one Tribute. The CPU Special Summons it first thing in its Main Phase (AiStepMainPhase calls AiSpecialSummonCyberDragon before it picks a Normal Summon), in Attack Position.
- **YGOPRODeck (TCG) text:** If only your opponent controls a monster, you can Special Summon this card (from your hand).
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Special summon from hand

`Cyber_Dragon_Special_Summon_From_Hand_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Cyber Dragon |  |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `special_summon you.hand[Cyber Dragon] -> m0 atk`: Special Summon **Cyber Dragon** in your hand from the hand by its own rule into your Monster Zone 0 (face-up Attack Position).

**Expected**

- `you.m0 = Cyber Dragon atk`: your Monster Zone 0 holds Cyber Dragon, face-up Attack Position.

#### Player 2: Not while you control a monster

`Cyber_Dragon_Not_While_You_Control_A_Monster_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Cyber Dragon |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `check menu you.hand[Cyber Dragon] lacks Special Summon`: Check: The card menu of **Cyber Dragon** in your hand does not offer Special Summon.

**Expected**

- `you.hand = Cyber Dragon`: your hand is exactly Cyber Dragon.

#### CPU 1: Special summon first

`Cyber_Dragon_Cpu_Special_Summon_First_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Cyber Dragon |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.monsters has Cyber Dragon`: the opponent's monsters includes Cyber Dragon.
- `you.m0 = empty`: your Monster Zone 0 (**Battle Ox**) is empty.
- `you.lp = 7600`: LP of you = 7600.

### Cyber End Dragon

- **Usage:** 82.2% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CYBER_END_DRAGON`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** "Cyber Dragon" + "Cyber Dragon" + "Cyber Dragon"
A Fusion Summon of this card can only be done with the above Fusion Materials. If this card attacks a Defense Position monster, inflict piercing battle damage.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Pierce

`Cyber_End_Dragon_Pierce_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Cyber End Dragon | face-up Attack Position |
| Opponent | Monster Zone 0 | Big Shield Gardna | face-up Defense Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Cyber End Dragon**): target the opponent's Monster Zone 0 (**Big Shield Gardna**).

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Big Shield Gardna**) is empty.
- `opp.lp = 6600`: LP of opp = 6600.

#### CPU 1: Pierce

`Cyber_End_Dragon_Cpu_Pierce_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Cyber End Dragon | face-up Attack Position |
| You | Monster Zone 0 | Big Shield Gardna | face-up Defense Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Big Shield Gardna**) is empty.
- `you.lp = 6600`: LP of you = 6600.

### Threatening Roar

- **Usage:** 60.0% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THREATENING_ROAR`, `gAddedCardEffects`)
- **Coded behavior:** "Your opponent cannot declare an attack this turn." (Normal Trap) The resolve stores the turn in gModDuel.roarTurn of the activating player's opponent (gDuel.turnCount + 1, 0 none). ModAttacksBlocked then holds for that player for the rest of the turn: BuildAttackableMask (card_menu_input.c) leaves their attackable mask empty, as Swords of Revealing Light does (no Battle Phase, no attacker to pick), and the CPU's attacker choice (BattleStage_SelectAttacker) is skipped. An attack already declared goes on: the replay check (Battle_CheckReplay) asks CanMonsterAttack, which is unchanged. The CPU activates it: before the human's Battle Phase   ModBattlePhaseStart (BattleStage_Start, card_menu_input.c, as the Battle Phase begins): while the human has a face-up monster with 2000 or more ATK, it flips its Set Threatening Roar (CanActivateFieldCard) and queues it, as its Standby activations do; it resolves before the attackable mask is built against an attack                 while the human has another monster that could attack after it (AiShouldActivateAddedCard)
- **YGOPRODeck (TCG) text:** Your opponent cannot declare an attack this turn.
- **AI coverage:** **Yes — generalized routine** — Monster menu: against an attack, when another of the human's could attack after Main Phase Spell: against an attack, when another of the human's could attack after Set/chain: against an attack, when another of the human's could attack after

#### Player 1: Stop further attacks

`Threatening_Roar_Stop_Further_Attacks_Player` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

The declared attack goes on (2500); Battle Ox cannot attack after it. If Battle Ox attacks first, answer that attack instead (then you.lp is 6300).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Threatening Roar | face-down (Set) |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 1 | Battle Ox | face-up Attack Position |

**Steps**

1. `when attack opp.m0 -> direct: activate you.st0`: When the opponent's Monster Zone 0 (**Summoned Skull**) declares an attack on you directly: open the card menu of your Spell/Trap Zone 0 (**Threatening Roar**) and choose Activate.

**Expected**

- `you.lp in 5500|6300`: LP of you is one of 5500|6300.
- `you.gy = Threatening Roar`: your Graveyard is exactly Threatening Roar.

#### CPU 1: Before battle phase

`Threatening_Roar_Cpu_Before_Battle_Phase_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

The CPU flips it as your Battle Phase starts, since you have a monster with 2000+ ATK.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Spell/Trap Zone 0 | Threatening Roar | face-down (Set) |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `check cannot attack you.m0 -> direct`: Check: your Monster Zone 0 (**Summoned Skull**) cannot attack the opponent directly.

**Expected**

- `opp.lp = 8000`: LP of opp = 8000.
- `opp.gy = Threatening Roar`: the opponent's Graveyard is exactly Threatening Roar.

### Exarion Universe

- **Usage:** 40.0% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_EXARION_UNIVERSE`, `gAddedCardEffects`)
- **Coded behavior:** (YGOPRODeck) "During your Battle Step, if this card attacks a Defense Position monster: You can make this card lose exactly 400 ATK, and if it does, it will inflict piercing battle damage. These effects last until the end of this turn." Like Injection Fairy Lily, the optional effect is handled in the battle code (no face-up monster response window): BattleStage_DamageCalc, DAMAGE_STEP_MARK_ATTACKER   ExarionUniverseResetBattle BattleStage_DamageCalc, DAMAGE_STEP_CALC            ExarionUniverseDamageStep before CalcBattle: the attacking Exarion vs a Defense Position monster, ATK at least 400. The human is asked (PROMPT_CONFIRM_CARD_EFFECT); the CPU pays when its ATK exceeds the DEF (piercing LP damage). Paying queues a gModDuel.exarionPenaltyStacks (ModStatBonus: -400 ATK each, cleared when that player's turn ends) and sets exarionPierce for ModHasPiercing. DuelCmd_TurnEnd                                       ExarionUniverseClearTurnPenalty for the turn player. ModEndAttack                                        clears exarionPierce for the next attack.
- **YGOPRODeck (TCG) text:** During your Battle Step, if this card attacks a Defense Position monster: You can make this card lose exactly 400 ATK, and if it does, it will inflict piercing battle damage. These effects last until the end of this turn.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `CpuWantsExarion`

#### Player 1: Lose 400 to pierce

`Exarion_Universe_Lose_400_To_Pierce_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Exarion Universe | face-up Attack Position |
| Opponent | Monster Zone 0 | Celtic Guardian | face-up Defense Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Exarion Universe**): target the opponent's Monster Zone 0 (**Celtic Guardian**).
3. `yes`: Answer Yes.

**Expected**

- `opp.m0 = empty`: the opponent's Monster Zone 0 (**Celtic Guardian**) is empty.
- `opp.lp = 7800`: LP of opp = 7800.
- `you.m0.atk = 1400`: The face-up Attack Position of your Monster Zone 0 (**Exarion Universe**) is 1400.

#### CPU 1: Lose 400 to pierce

`Exarion_Universe_Cpu_Lose_400_To_Pierce_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Exarion Universe | face-up Attack Position |
| You | Monster Zone 0 | Celtic Guardian | face-up Defense Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.m0 = empty`: your Monster Zone 0 (**Celtic Guardian**) is empty.
- `you.lp = 7800`: LP of you = 7800.

### Gravity Bind

- **Usage:** 16.7% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_GRAVITY_BIND`, `gAddedCardEffects`)
- **Coded behavior:** Effect text (YGOPRODeck API): "Level 4 or higher monsters cannot attack." Continuous Trap; stays face-up when activated. The attack rule is in CanMonsterAttack (card_menu_input.c), with vanilla effect key CARD_1323. CPU: AiPlayAddedCards; AiShouldActivateAddedCard default yes (CARD_1323 in AiShouldActivateSetCard).
- **YGOPRODeck (TCG) text:** Level 4 or higher monsters cannot attack.
- **AI coverage:** **Yes — dedicated logic** — Block comment: CPU: AiPlayAddedCards; AiShouldActivateAddedCard default yes (CARD_1323 in AiShouldActivateSetCard).

#### Player 1: Stop level 4 attacks

`Gravity_Bind_Stop_Level_4_Attacks_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Gravity Bind | face-down (Set) |
| Opponent | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Gravity Bind**) and choose Activate.
2. `pass`: End your turn (its End Phase effects resolve).
3. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.lp = 8000`: LP of you = 8000.
- `you.st0 = Gravity Bind up`: your Spell/Trap Zone 0 (**Gravity Bind**) holds Gravity Bind, face up.

#### CPU 1: Stop human attacks

`Gravity_Bind_Cpu_Stop_Human_Attacks_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Gravity Bind | face-down (Set) |
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).
2. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.
3. `battle`: Enter the Battle Phase.

**Expected**

- `opp.st0 = Gravity Bind up`: the opponent's Spell/Trap Zone 0 (**Gravity Bind**) holds Gravity Bind, face up.
- `cannot attack you.m0 -> direct`: your Monster Zone 0 (**Summoned Skull**) cannot attack the opponent directly.
- `opp.lp = 8000`: LP of opp = 8000.

### Chiron the Mage

- **Usage:** 11.1% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CHIRON_THE_MAGE`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn: You can discard 1 Spell, then target 1 Spell/Trap your opponent controls; destroy that target." Passcode 16956455 ([YGOPRODeck](https://ygoprodeck.com/card/chiron-the-mage-5386)). Main Phase ignition (CanActivateMonsterEffect), once per turn (effectUnused). ChainA discards a Magic card (DISCARD_FLAG_SPELL for the human; the CPU uses FindMagicInHand). ChainB targets the opponent's Spell/Trap zones (EffectOpponentSpellTrapCheck; the human uses EffectDustTornadoChainB). Resolve: EffectDestroyTargetResolve.
- **YGOPRODeck (TCG) text:** Once per turn: You can discard 1 Spell Card, then target 1 Spell/Trap Card your opponent controls; destroy that target.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — Monster menu: Default yes Set/chain: Default yes

#### Player 1: Discard spell destroy

`Chiron_the_Mage_Discard_Spell_Destroy_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Chiron the Mage | face-up Attack Position |
| You | Hand 0 | Fissure |  |
| Opponent | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |

**Steps**

1. `activate you.m0`: Open the card menu of your Monster Zone 0 (**Chiron the Mage**) and activate its effect.
2. `discard you.hand[Fissure]`: Discard **Fissure** in your hand for the cost.
3. `target opp.st0`: Pick the opponent's Spell/Trap Zone 0 (**Trap Hole**) as the target.

**Expected**

- `opp.st0 = empty`: the opponent's Spell/Trap Zone 0 (**Trap Hole**) is empty.
- `opp.gy = Trap Hole`: the opponent's Graveyard is exactly Trap Hole.
- `you.gy = Fissure`: your Graveyard is exactly Fissure.
- `menu you.m0 lacks Activate`: The card menu of your Monster Zone 0 (**Chiron the Mage**) does not offer Activate.

#### CPU 1: Discard spell destroy

`Chiron_the_Mage_Cpu_Discard_Spell_Destroy_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Monster Zone 0 | Chiron the Mage | face-up Attack Position |
| Opponent | Hand 0 | Fissure |  |
| You | Spell/Trap Zone 0 | Trap Hole | face-down (Set) |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.st0 = empty`: your Spell/Trap Zone 0 (**Trap Hole**) is empty.
- `you.gy = Trap Hole`: your Graveyard is exactly Trap Hole.

### Cursed Seal of the Forbidden Spell

- **Usage:** 11.1% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_CURSED_SEAL_OF_THE_FORBIDDEN_SPELL`, `gAddedCardEffects`)
- **Coded behavior:** (See handlers in `src/added_effects.c`; no block comment was matched.)
- **YGOPRODeck (TCG) text:** When a Spell Card is activated: Discard 1 Spell; negate the activation, and if you do, destroy it, and if you do that, your opponent cannot activate Spell Cards with that name for the rest of this Duel.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — dedicated logic** — Monster menu: only chained (AiChainCursedSeal), not flipped blindly Main Phase Spell: only chained (AiChainCursedSeal), not flipped blindly Set/chain: only chained (AiChainCursedSeal), not flipped blindly

#### Player 1: Negate spell

`Cursed_Seal_of_the_Forbidden_Spell_Negate_Spell_Player` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Cursed Seal of the Forbidden Spell | face-down (Set) |
| You | Hand 0 | Fissure |  |
| Opponent | Hand 0 | Pot of Greed |  |

**Steps**

1. `when activate opp.hand[Pot of Greed]: activate you.st0`: When **Pot of Greed** in the opponent's hand is activated: open the card menu of your Spell/Trap Zone 0 (**Cursed Seal of the Forbidden Spell**) and choose Activate.
2. `discard you.hand[Fissure]`: Discard **Fissure** in your hand for the cost.

**Expected**

- `opp.gy = Pot of Greed`: the opponent's Graveyard is exactly Pot of Greed.
- `opp.hand count 0`: the opponent's hand holds 0 card(s).
- `you.gy = Cursed Seal of the Forbidden Spell, Fissure`: your Graveyard is exactly Cursed Seal of the Forbidden Spell, Fissure.

#### CPU 1: Negate human spell

`Cursed_Seal_of_the_Forbidden_Spell_Cpu_Negate_Human_Spell_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Pot of Greed |  |
| Opponent | Spell/Trap Zone 0 | Cursed Seal of the Forbidden Spell | face-down (Set) |
| Opponent | Hand 0 | Fissure |  |

**Steps**

1. `activate you.hand[Pot of Greed]`: Open the card menu of **Pot of Greed** in your hand and choose Activate.

**Expected**

- `you.hand count 0`: your hand holds 0 card(s).
- `you.gy = Pot of Greed`: your Graveyard is exactly Pot of Greed.
- `opp.gy = Cursed Seal of the Forbidden Spell, Fissure`: the opponent's Graveyard is exactly Cursed Seal of the Forbidden Spell, Fissure.

### Wave-Motion Cannon

- **Usage:** 11.1% weighted inclusion in `usage/cyber.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_WAVE_MOTION_CANNON`, `gAddedCardEffects`)
- **Coded behavior:** Continuous Spell: "During your Main Phase: You can send this face-up card to the GY; inflict 1000 damage to your opponent for each of your Standby Phases that have passed since this card was activated." activation from hand   resolve does nothing (the card stays face-up); turnCounter stays 0 until Standby each Standby Phase   once per phase (gModDuel.wmcStandbyTicked), turnCounter on the turn player's face-up copies ignition from field  prepare requires Main Phase and !fromHand; resolve sends the card to the GY and burns 1000 * turnCounter (link kind CHAIN_KIND_SPELL_TRAP, as field activations in duel_cmd_queue.c). Card menu and CanChainFieldCard allow face-up activation in Main Phase.
- **YGOPRODeck (TCG) text:** During your Main Phase: You can send this face-up card to the GY; inflict 1000 damage to your opponent for each of your Standby Phases that have passed since this card was activated.
- **AI coverage:** **Yes — dedicated logic** — Hooks: `AiWantsWaveMotionIgnition` Monster menu: HAS_CARD_DES_WOMBAT Main Phase Spell: Conditional AiWants* / AiShouldActivateAddedCard Set/chain: HAS_CARD_DES_WOMBAT

#### Player 1: Burn after one standby

`Wave_Motion_Cannon_Burn_After_One_Standby_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

One of your Standby Phases has passed since it was activated: 1000 damage.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Wave-Motion Cannon |  |

**Steps**

1. `activate you.hand[Wave-Motion Cannon]`: Open the card menu of **Wave-Motion Cannon** in your hand and choose Activate.
2. `pass`: End your turn (its End Phase effects resolve).
3. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).
4. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.
5. `activate you.st[Wave-Motion Cannon]`: Open the card menu of your **Wave-Motion Cannon** and choose Activate.

**Expected**

- `opp.lp = 7000`: LP of opp = 7000.
- `you.gy = Wave-Motion Cannon`: your Graveyard is exactly Wave-Motion Cannon.

#### CPU 1: Activate and keep

`Wave_Motion_Cannon_Cpu_Activate_And_Keep_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Wave-Motion Cannon |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.spells has Wave-Motion Cannon`: the opponent's Spells/Traps includes Wave-Motion Cannon.

## Reaper

### Treeborn Frog

- **Usage:** 28.4% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_TREEBORN_FROG`, `gAddedCardEffects`)
- **Coded behavior:** "Once per turn, during your Standby Phase, if this card is in your Graveyard and you do not control "Treeborn Frog": You can Special Summon this card. You must control no Spell/Trap Cards to activate and to resolve this effect." DuelPhase_Standby (duel_phases.c) runs TreebornFrogStandbyStep right after Sinister Serpent's step, in the same way (on gDuel.phaseSubStep, not a chain link; once per turn, as the Standby Phase runs once): step 0  done unless the player has one in the Graveyard, no face-up Treeborn Frog, no Spell/Trap card (the Field Spell included), a free monster zone and CanSpecialSummon. The CPU says Yes; the human is asked. step 1  on Yes, with still no Spell/Trap card: it leaves the Graveyard (RemoveGraveyardCardByNumber) and is Special Summoned face up, the player choosing its position (QueueSpecialSummonChoosePosition).
- **YGOPRODeck (TCG) text:** Once per turn, during your Standby Phase, if this card is in your Graveyard and you do not control "Treeborn Frog": You can Special Summon this card. You must control no Spell/Trap Cards to activate and to resolve this effect.
- **AI coverage:** **Unclear / likely missing** — No `CPU:` notes and no switch case found; treat as untested for CPU until you confirm in a duel.

#### Player 1: Revive in standby

`Treeborn_Frog_Revive_In_Standby_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Graveyard | Treeborn Frog |  |

**Steps**

1. `pass`: End your turn (its End Phase effects resolve).
2. `opp_turn`: The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects still run).
3. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.
4. `yes`: Answer Yes.
5. `position atk`: Choose face-up Attack Position for the summoned monster.

**Expected**

- `you.monsters = Treeborn Frog`: your monsters is exactly Treeborn Frog.
- `you.gy count 0`: your Graveyard holds 0 card(s).

#### CPU 1: Revive in standby

`Treeborn_Frog_Cpu_Revive_In_Standby_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Graveyard | Treeborn Frog |  |

**Steps**

1. `pass`: End your turn (its End Phase effects resolve).
2. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy lacks Treeborn Frog`: the opponent's Graveyard does not include Treeborn Frog.

### Thestalos the Firestorm Monarch

- **Usage:** 21.3% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_THESTALOS_THE_FIRESTORM_MONARCH`, `gAddedCardEffects`)
- **Coded behavior:** "If this card is Tribute Summoned: Discard 1 random card from your opponent's hand, then, if it was a Monster Card, inflict damage to your opponent equal to its original Level x 100." The two Normal Summon paths (summon_checks.c) queue its RESPONSE_SUMMONED trigger in Zaborg's case; it is Level 6, so that summon always has a Tribute. Mandatory, no target. The resolve discards a random card of the opponent's hand (DiscardHandCard, by the opponent's effect) and, for a monster, takes its printed Level x 100 from the opponent's LP (LoseLifePoints).
- **YGOPRODeck (TCG) text:** If this card is Tribute Summoned: Discard 1 random card from your opponent's hand, then, if it was a Monster Card, inflict damage to your opponent equal to its original Level x 100.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Discard monster burn

`Thestalos_the_Firestorm_Monarch_Discard_Monster_Burn_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

Summoned Skull is the only card in hand, so the random discard is known: Level 6, 600 damage.

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Thestalos the Firestorm Monarch |  |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| Opponent | Hand 0 | Summoned Skull |  |

**Steps**

1. `tribute_summon you.hand[Thestalos the Firestorm Monarch] -> m0 atk tributes you.m0`: Tribute Summon **Thestalos the Firestorm Monarch** in your hand into your Monster Zone 0 (**Battle Ox**) (face-up Attack Position), Tributing your Monster Zone 0 (**Battle Ox**).

**Expected**

- `opp.hand count 0`: the opponent's hand holds 0 card(s).
- `opp.gy = Summoned Skull`: the opponent's Graveyard is exactly Summoned Skull.
- `opp.lp = 7400`: LP of opp = 7400.

#### CPU 1: Discard monster burn

`Thestalos_the_Firestorm_Monarch_Cpu_Discard_Monster_Burn_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Thestalos the Firestorm Monarch |  |
| Opponent | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Hand 0 | Summoned Skull |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `you.hand count 0`: your hand holds 0 card(s).
- `you.gy = Summoned Skull`: your Graveyard is exactly Summoned Skull.
- `you.lp <= 7400`: LP of you <= 7400.

### D.D. Survivor

- **Usage:** 19.4% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_D_D_SURVIVOR`, `gAddedCardEffects`)
- **Coded behavior:** "If this face-up card is banished: Special Summon it during the End Phase." DuelCmd_Banish (duel_cmd_field.c), BanishBattleDestroyedCard   DDSurvivorBanished DuelCmd_BanishGraveyardCard (duel_cmd_piles.c: Bazoo, Chaos costs; Graveyard cards are face up) DuelPhase_End (battle_phase3.c)                                EndPhase_DDSurvivor Banished before the End Phase, the owner's count goes up and EndPhase_DDSurvivor queues one trigger per copy; banished during the End Phase (Return from the Different Dimension's banish), the trigger is queued at once, so it still returns that End Phase. Mandatory: the resolve Special Summons a D.D. Survivor from the owner's banished pile (banished normally, not face down) face up in the position the owner picks.
- **YGOPRODeck (TCG) text:** During the End Phase, if this card was banished while face-up, on your side of the field, this turn: Special Summon this banished card. This effect can only activate once per turn.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Automatic** (no card-menu activation) — No `CanActivateMonsterEffect` path; CPU uses standard lines.

#### Player 1: Return at end phase

`DD_Survivor_Return_At_End_Phase_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

D.D. Warrior Lady's controller always banishes both (the CPU says Yes).

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | D.D. Survivor | face-up Attack Position |
| Opponent | Monster Zone 0 | D.D. Warrior Lady | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**D.D. Survivor**): target the opponent's Monster Zone 0 (**D.D. Warrior Lady**).
3. `check you.banished = D.D. Survivor`: Check: your banished cards is exactly D.D. Survivor.
4. `pass`: End your turn (its End Phase effects resolve).
5. `position atk`: Choose face-up Attack Position for the summoned monster.

**Expected**

- `you.monsters = D.D. Survivor`: your monsters is exactly D.D. Survivor.
- `you.banished count 0`: your banished cards holds 0 card(s).
- `opp.banished = D.D. Warrior Lady`: the opponent's banished cards is exactly D.D. Warrior Lady.

#### CPU 1: Return at end phase

`DD_Survivor_Cpu_Return_At_End_Phase_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | D.D. Warrior Lady | face-up Attack Position |
| Opponent | Monster Zone 0 | D.D. Survivor | face-up Attack Position |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**D.D. Warrior Lady**): target the opponent's Monster Zone 0 (**D.D. Survivor**).
3. `yes`: Answer Yes.
4. `pass`: End your turn (its End Phase effects resolve).

**Expected**

- `opp.monsters = D.D. Survivor`: the opponent's monsters is exactly D.D. Survivor.
- `you.banished = D.D. Warrior Lady`: your banished cards is exactly D.D. Warrior Lady.

### Pot of Avarice

- **Usage:** 17.5% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_POT_OF_AVARICE`, `gAddedCardEffects`)
- **Coded behavior:** "Target 5 monsters in your GY; shuffle all 5 into the Deck, then draw 2 cards." (Normal Spell)
- **YGOPRODeck (TCG) text:** Target 5 monsters in your GY; shuffle all 5 into the Deck, then draw 2 cards.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Shuffle five draw two

`Pot_of_Avarice_Shuffle_Five_Draw_Two_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Hand 0 | Pot of Avarice |  |
| You | Graveyard | Battle Ox |  |
| You | Graveyard | Celtic Guardian |  |
| You | Graveyard | La Jinn the Mystical Genie of the Lamp |  |
| You | Graveyard | Summoned Skull |  |
| You | Graveyard | Man-Eater Bug |  |

**Steps**

1. `activate you.hand[Pot of Avarice]`: Open the card menu of **Pot of Avarice** in your hand and choose Activate.
2. `pick you.gy[Battle Ox]`: Pick **Battle Ox** in your Graveyard in the list or on the field.
3. `pick you.gy[Celtic Guardian]`: Pick **Celtic Guardian** in your Graveyard in the list or on the field.
4. `pick you.gy[La Jinn the Mystical Genie of the Lamp]`: Pick **La Jinn the Mystical Genie of the Lamp** in your Graveyard in the list or on the field.
5. `pick you.gy[Summoned Skull]`: Pick **Summoned Skull** in your Graveyard in the list or on the field.
6. `pick you.gy[Man-Eater Bug]`: Pick **Man-Eater Bug** in your Graveyard in the list or on the field.

**Expected**

- `you.gy = Pot of Avarice`: your Graveyard is exactly Pot of Avarice.
- `you.hand count 2`: your hand holds 2 card(s).

#### CPU 1: Shuffle five draw two

`Pot_of_Avarice_Cpu_Shuffle_Five_Draw_Two_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Hand 0 | Pot of Avarice |  |
| Opponent | Graveyard | Battle Ox |  |
| Opponent | Graveyard | Celtic Guardian |  |
| Opponent | Graveyard | La Jinn the Mystical Genie of the Lamp |  |
| Opponent | Graveyard | Summoned Skull |  |
| Opponent | Graveyard | Man-Eater Bug |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).

**Expected**

- `opp.gy has Pot of Avarice`: the opponent's Graveyard includes Pot of Avarice.
- `opp.gy lacks Summoned Skull`: the opponent's Graveyard does not include Summoned Skull.

### Magical Merchant

- **Usage:** 17.1% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MAGICAL_MERCHANT`, `gAddedCardEffects`)
- **Coded behavior:** "FLIP: Excavate cards from the top of your Deck until you excavate a Spell/Trap. Add that card to your hand, also send the remaining cards to the GY." A flip effect (HasFlipEffect, card_detail.c). The top of the Deck is deck[0]. The monsters above the first Spell/Trap go to the Graveyard (DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD, one by one); with no Spell/Trap at all the whole Deck goes. The Spell/Trap is shown and added with Sangan's AddDeckCardToHand, which takes the first card of that number in the Deck: that one, as every card above it is a monster.
- **YGOPRODeck (TCG) text:** FLIP: Excavate cards from the top of your Deck until you excavate a Spell/Trap, then add that card to your hand, also send the remaining cards to the GY.
- **TCG vs this ROM:** Coded behavior may differ from current TCG/OCG text; test against the **Coded behavior** section, not the card database alone.
- **AI coverage:** **Yes — generalized routine** — `AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.

#### Player 1: Flip excavate spell trap

`Magical_Merchant_Flip_Excavate_Spell_Trap_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Magical Merchant | face-down (Set) |
| You | Deck 0 | Battle Ox |  |
| You | Deck 1 | Celtic Guardian |  |
| You | Deck 2 | Fissure |  |
| You | Deck 3 | Trap Hole |  |

**Steps**

1. `flip_summon you.m0`: Flip Summon your Monster Zone 0 (**Magical Merchant**).

**Expected**

- `you.hand = Fissure`: your hand is exactly Fissure.
- `you.gy = Battle Ox, Celtic Guardian`: your Graveyard is exactly Battle Ox, Celtic Guardian.
- `you.deck[0] = Trap Hole`: your Deck slot 0 (**Battle Ox**) is exactly Trap Hole.

#### CPU 1: Flipped by attack

`Magical_Merchant_Cpu_Flipped_By_Attack_CPU` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Monster Zone 0 | Summoned Skull | face-up Attack Position |
| Opponent | Monster Zone 0 | Magical Merchant | face-down (Set) |
| Opponent | Deck 0 | Battle Ox |  |
| Opponent | Deck 1 | Fissure |  |

**Steps**

1. `battle`: Enter the Battle Phase.
2. `attack you.m0 -> opp.m0`: Attack with your Monster Zone 0 (**Summoned Skull**): target the opponent's Monster Zone 0 (**Magical Merchant**).

**Expected**

- `opp.hand = Fissure`: the opponent's hand is exactly Fissure.
- `opp.gy has Battle Ox`: the opponent's Graveyard includes Battle Ox.
- `opp.gy has Magical Merchant`: the opponent's Graveyard includes Magical Merchant.

### Mask of Restrict

- **Usage:** 14.2% weighted inclusion in `usage/reaper.json` (earliest qualifying format).
- **Implementation:** `src/added_effects.c` (`HAS_CARD_MASK_OF_RESTRICT`, `gAddedCardEffects`)
- **Coded behavior:** "Neither player can Tribute cards." Continuous Trap; the tribute lock is vanilla effect key CARD_1418 in the original engine — face-up copies count through IsTributeLockActive (every CARD_1418 check in the duel code). CPU: AiWantsMaskOfRestrict, AiPlayAddedCards (Main Phase); Set copies use AiShouldActivateAddedCard (yes).
- **YGOPRODeck (TCG) text:** Neither player can Tribute cards.
- **AI coverage:** **Yes — dedicated logic** — Block comment: CPU: AiWantsMaskOfRestrict, AiPlayAddedCards (Main Phase); Set copies use AiShouldActivateAddedCard (yes). Hooks: `AiWantsMaskOfRestrict`

#### Player 1: No tributes

`Mask_of_Restrict_No_Tributes_Player` · start: your turn, Main Phase 1 (turnCount 2) · LP 8000 / 8000 · opponent frozen

| Side | Place | Card | Position |
|---|---|---|---|
| You | Spell/Trap Zone 0 | Mask of Restrict | face-down (Set) |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Hand 0 | Summoned Skull |  |

**Steps**

1. `activate you.st0`: Open the card menu of your Spell/Trap Zone 0 (**Mask of Restrict**) and choose Activate.

**Expected**

- `menu you.hand[Summoned Skull] lacks Summon`: The card menu of **Summoned Skull** in your hand does not offer Summon.
- `you.st0 = Mask of Restrict up`: your Spell/Trap Zone 0 (**Mask of Restrict**) holds Mask of Restrict, face up.

#### CPU 1: Lock tributes

`Mask_of_Restrict_Cpu_Lock_Tributes_CPU` · start: the opponent's turn, Main Phase 1 (turnCount 3) · LP 8000 / 8000 · opponent not frozen

| Side | Place | Card | Position |
|---|---|---|---|
| Opponent | Spell/Trap Zone 0 | Mask of Restrict | face-down (Set) |
| You | Monster Zone 0 | Battle Ox | face-up Attack Position |
| You | Hand 0 | Summoned Skull |  |

**Steps**

1. `cpu_turn`: The CPU plays its whole turn (not frozen).
2. `draw`: In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.

**Expected**

- `opp.st0 = Mask of Restrict up`: the opponent's Spell/Trap Zone 0 (**Mask of Restrict**) holds Mask of Restrict, face up.
- `menu you.hand[Summoned Skull] lacks Summon`: The card menu of **Summoned Skull** in your hand does not offer Summon.
