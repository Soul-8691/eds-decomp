# Leftover card effects

*Yu-Gi-Oh! The Eternal Duelist Soul* (GBA, USA) contains duel code for 157 card numbers that have no card in the game. Each number below is one of them: the engine knows how to play it, but no card ID maps to it, so it can never be drawn, built into a deck or obtained. This document lists what each one does and how complete its code is.

Almost all of them are card numbers 1210 and above. That range seems to be left over from the larger card database shared with Konami's other Duel Monsters games (an assumption; nothing in the ROM names those games). Most of the effects match real OCG cards from around 2001-2002; where the source code names a likely OCG card, or the game's own Fusion recipe identifies one, it is given in the notes as a hypothesis.

## How effects are attached to cards

- **Effects are found by card number, not card ID.** The effect table at `0x0819A9D4` (426 rows: number, then resolve, check, prepare, chainA and chainB handlers) is searched by number. Many other rules (battle, stats, summoning, phases) test the number directly in the code.
- **136 of the table's rows** belong to numbers with no EDS card. A further 21 such numbers have rules elsewhere in the code, or appear only in the Fusion/ritual recipe tables.
- **Making one playable.** Give a new card ID that number (`tools/card_expand/write_tables.py`, see `wiki/tools/card-expand.md`). The effect then works as described here; nothing else needs to change. Card type, level, ATK/DEF and attribute come from the new card's stats, so they must suit the effect (for example a Trap for a trap effect, a Machine for 1314's targets).

## Status legend

| Status | Meaning | Count |
|---|---|---|
| Full | Everything the effect needs is coded: activation conditions, targeting, costs and the result. | 96 |
| Full (passive) | Fully coded, but as continuous rules in the duel code (battle, stats, phases, summon checks) rather than through an activated effect. Many have an empty effect-table row or a no-op resolve handler. | 44 |
| Full (human only) | Fully coded for the human player; the CPU's path is missing or does nothing. | 4 |
| Full, buggy | Fully coded, but a bug in the original code changes the result. | 2 |
| Partial | Part of the effect is missing. | 1 |
| No effect code | The number is only named (as a Fusion material, ritual monster, Tribute, equip target or AI hint); there is no effect for it. | 10 |
| **Total** | | **157** |

"Kind" is the card kind the code implies: Trap for attack, summon or chain responses; Equip for equip handlers; Monster for flip, summon, battle and tribute effects; "Magic/Trap" for continuous cards whose kind the code does not reveal. It is inferred, not stored anywhere for these numbers.

## Effects

### Numbers below 1300

| Number | Kind | Effect | Status | Notes |
|---|---|---|---|---|
| 230 | ? | Only the CPU knows it: `AiTryChainResponse` will not chain it while the CPU has more than 1000 LP. | No effect code | AI hint only. The number is below 1200, so it is an ordinary Konami card left out of EDS. |
| 1210 | Monster | A second Dark Magician. It counts as Dark Magician everywhere: same-name checks, the copy limit, the +300 ATK per Dark Magician in the graveyards (presumably Dark Magician Girl's bonus), Time Wizard, the Dark Magician searches of 1211/1212. | Full | Alias only; it needs no effect of its own. |
| 1211 | Magic | If you control Dark Magician (either art, or 1210) and the opponent has a monster: destroy one targeted monster. | Full | OCG *Thousand Knives* (hypothesis). Shares Man-Eater Bug's destroy handler. |
| 1212 | Magic | Pay half your LP; Special Summon a Dark Magician from the deck. Needs an unused Normal Summon; afterwards you cannot Normal or Special Summon for the turn. | Full | OCG *Dark Magic Curtain* (hypothesis). |
| 1213 | Magic | Target one opponent monster and one of yours: destroy theirs, then give yours to the opponent in its zone. | Full | OCG *Mystic Box* (hypothesis, from the code comments). Quirk: the two target picks advance even if a pick is refused. |
| 1214 | Trap | When the opponent declares an attack: negate it, and the attacker's controller loses LP equal to the attacker's ATK. | Full |  |
| 1220 | Magic | Main Phase 1, costs your Battle Phase: use one opponent monster as a Tribute, for a Tribute Summon of a level 5+ monster or for Catapult Turtle / The Little Swordsman of Aile / Cannon Soldier. | Full, buggy | OCG *Soul Exchange* (hypothesis). ROM bug: the summon paths tribute the monster in the target side's zone 0, not the target. |
| 1221 | Magic | Both players discard their whole hand and draw that many cards. | Full | OCG *Card Destruction* (hypothesis). The CPU uses it (AI routines check for it). |
| 1230 | Magic/Trap | While it is on either field, no monster can attack (the attack menu is empty). It gets a turn counter in the End Phase and is destroyed when the counter runs out. | Full (passive) | Handled next to Swords of Revealing Light; resolve is a no-op, the rules are continuous. |
| 1231 | Monster | When it attacks it is not destroyed and takes no battle damage, and the defender gets a -500 ATK link. | Full (passive) | Battle code (`CalcBattle`, `GetZoneCardStats`). |
| 1232 | Magic | Tribute a face-up Kuriboh: fill every free monster zone of yours with Kuriboh tokens (card number 1921, Defense Position). | Full | OCG *Multiply* (hypothesis). Blocked while 1418 is active. |
| 1240 | Monster | When summoned (Normal, Flip or Special): add Gazelle the King of Mythical Beasts from the deck to the hand (you are asked; the CPU always does it). | Full | OCG *Berfomet* (hypothesis, by behaviour). Also a Fusion material of 1241. |
| 1241 | Fusion | Fusion of Gazelle the King of Mythical Beasts + 1240. When it is destroyed (in battle or by an effect, after being summoned): Special Summon a monster from your graveyard (the Monster Reborn handler with key 1241's own candidate list). | Full | OCG *Chimera the Flying Mythical Beast* (hypothesis: the recipe matches, and Chimera revives Berfomet or Gazelle). Shown as an effect monster on the card screen. |
| 1242 | Equip / Monster | Equipped monster: -500 ATK. Whenever this card is sent to the graveyard (discard, mill, tribute, battle, Painful Choice...), you may equip it from the graveyard to a face-up monster on either side. | Full |  |
| 1243 | Monster | When it is attacked while face down and flipped, and you have another monster and the attacker's side has a free zone: you may substitute another monster as the target; after the attack, the attacked monster moves to the attacker's side. | Full (passive) | Battle code (`BattleStage_DamageCalc`, `BattleStage_EndAttack`); resolve is a no-op. Behaviour read from the code; the exact rules text is a guess. |
| 1244 | Trap | Link to a face-up opponent monster: it cannot attack or change position, and has -700 ATK while the link lasts. | Full | A stronger Spellbinding Circle (shares its handler). |
| 1245 | Magic | With 4 free monster zones and no summon this turn: summon four tokens (card number 1922, Defense Position); no more Normal or Special Summons this turn. | Full | OCG *Scapegoat* (hypothesis). The CPU has a strategy built around it (with Cannon Soldier). |
| 1246 | Monster | When summoned (any way): it gains the base ATK/DEF of a face-up opponent monster you pick. | Full |  |
| 1247 | Trap | When the opponent plays one of a list of one-player Magic cards (healing, burn, draw, hand cards): run that card for yourself instead, and negate the original. | Full | OCG *Mystical Refpanel* (hypothesis). |
| 1248 | Magic | Main Phase 1, costs your Battle Phase: target your face-up Summoned Skull or Thunder monster; destroy every face-up opponent monster whose DEF is below its ATK. | Full |  |
| 1249 | Monster | A second Harpie Lady: counts as Harpie Lady for same-name checks, the copy limit, Cyber Shield, Harpie's Pet Dragon and Elegant Egotist. | Full | Alias only. |
| 1250 | Monster | Summon rule: Normal Summon without Tributes when it is your only hand card, otherwise it needs two Tributes. | Full (passive) | `CanSummonFromHand`. |
| 1251 | Monster | Not destroyed in battle by a monster with 1900 or more ATK. | Full (passive) | Empty effect-table row; the rule is in `CalcBattle`. |
| 1252 | Monster | Your Plant monsters gain 500 ATK/DEF for each face-up Defense Position copy of this card you control. | Full (passive) | `GetZoneCardStats`. |
| 1253 | Monster | +2000 ATK/DEF when it battles a Warrior (attacking or defending). | Full (passive) | Empty effect-table row; the rule is in `CalcBattle`. |
| 1254 | Monster | Once per turn: +500 ATK/DEF until the end of the turn (Karate Man's handler). In battle it also links a -500 stats effect to the monster it battles. | Full | The battle link is in `BattleStage_DestroyMonsters` (shared with 1255 and 1256); not traced in detail. |
| 1255 | Monster | Flip: take control of a face-up opponent Machine until the end of the turn. | Full |  |
| 1256 | Monster | Flip: each player discards one card. | Full |  |
| 1257 | Monster | Cannot be Normal Summoned; summoned by tributing two monsters, one of them 1410 or 1412. When it is sent to the graveyard after being summoned: take 1258 from your deck, equip it to a face-up monster, and that monster changes sides. | Full |  |
| 1258 | Equip | Equip card for 1257's effect (any face-up monster). Its own effect returns it to the top of the deck and shuffles; it stays on the field while it resolves. | Full |  |

### 1301-1352

| Number | Kind | Effect | Status | Notes |
|---|---|---|---|---|
| 1301 | Trap | When one of your monsters is sent to the graveyard, or a battle involves your side: destroy one targeted monster. | Full |  |
| 1302 | Magic/Trap | Can be activated only while the opponent has 3000 LP or less. While it is face up, the opponent loses 500 LP in each of their Standby Phases. | Full (passive) | Resolve is a no-op; the drain is in `ApplyStandbyPhaseEffects`. |
| 1303 | Trap | Only when the opponent holds 6+ cards and you 2 or fewer: call a coin toss. Right: draw until you hold 5 cards. Wrong: skip your next turn. | Full | OCG *Gamble* (hypothesis). |
| 1304 | Trap | When you take damage: the opponent loses 700 LP plus 300 for each copy in your graveyard. | Full | Uses Numinous Healer's handler in reverse. |
| 1305 | Magic/Trap | While it is face up, you gain 500 LP each time you draw (per copy). | Full (passive) | `DrawCards`; resolve is a no-op. |
| 1306 | Magic/Trap | While it is on either field, a player loses 300 LP for each card sent to their graveyard. | Full (passive) | `LoseLpOnSendToGraveyard`; resolve is a no-op. |
| 1307 | Monster | Flip: you may Special Summon one more copy of this card from the deck, face down in Defense Position. | Full | Shares Giant Germ / Nimble Momonga's handler; key 1307 stops after one copy by design. |
| 1311 | Magic | Destroy the card in each player's Field Magic zone. While it stays face up, you lose 500 LP in each of your Standby Phases. | Full | OCG *Burning Land* (hypothesis). |
| 1312 | Magic | Only at the start of Main Phase 1 (no Magic/Trap played and no Normal Summon yet): neither player may play Magic or Trap cards for 2 turns. | Full | OCG *Cold Wave* (hypothesis). |
| 1313 | Equip | Equip to any face-up monster: it inflicts piercing battle damage. | Full |  |
| 1314 | Magic | Double the ATK of every face-up Machine you control; those Machines are destroyed at the end of the turn. | Full | OCG *Limiter Removal* (hypothesis). The CPU has a strategy for it. |
| 1315 | Magic | Both players gain 1000 LP. | Full |  |
| 1316 | Magic | Return one of your monsters and your whole hand to the deck, shuffle, and draw as many cards as the hand held. | Full (human only) | Its target picker has no CPU branch. |
| 1317 | Trap | Move the opponent's attack, or a single-target effect, onto another of your monsters. | Full | OCG *Shift* (hypothesis). |
| 1318 | Trap | Tribute one of your monsters: Special Summon an Insect from the deck whose level is one higher. | Full, buggy | OCG *Insect Imitation* (hypothesis). ROM bug: the packed position is passed as the zone, so the Tribute and level lookup only work for one zone. |
| 1319 | Trap | Banish one of your monsters; it comes back later and its zone stays reserved for it. | Full (human only) | OCG *Interdimensional Matter Transporter* (hypothesis). The return is in the Standby Phase code. Its target picker has no CPU branch. |
| 1320 | Trap | Choose two empty monster zones (either side): they cannot be used while this card stays on the field. | Full | OCG *Ground Collapse* (hypothesis). |
| 1321 | Trap | When the opponent plays a Magic card: negate it unless they discard a Magic card. | Full | OCG *Magic Drain* (hypothesis). |
| 1322 | Magic/Trap | While it is on either field, each End Phase destroys the turn player's face-up monsters of level 3 or less. | Full (passive) | `EndPhase_DestroyLowLevelMonsters`; resolve is a no-op. |
| 1323 | Magic/Trap | While it is on either field, monsters above level 3 cannot attack. | Full (passive) | `CanMonsterAttack`; resolve is a no-op. |
| 1324 | Trap | Activated from a Set position only. Cost: discard a Magic card. The opponent loses 500 LP. | Full |  |
| 1325 | Trap | When the opponent Sets a monster: turn it face-up in Attack Position, without its flip effect. | Full | OCG *Shadow of Eyes* (hypothesis). |
| 1326 | Monster | While Umi is the face-up Field Magic, Magic cards (other than equips) cannot target it or affect it, and it cannot be picked as an attack target in some cases. | Full (passive) | Targeting and stat code; empty effect-table row. |
| 1327 | Monster | When it is destroyed in battle, it is equipped from the graveyard to the monster that destroyed it at the end of the Battle Phase. | Full (passive) | Battle code; empty effect-table row. What the equip then does was not traced. |
| 1328 | Monster | When it inflicts battle damage: you may put Parasite Paracide from your deck on top of the deck. | Full |  |
| 1329 | Monster | Same as 1326: Magic cannot target it while Umi is the face-up Field Magic. | Full (passive) | No constant name in the source; tested as the literal 1329. |
| 1330 | Monster | Flip: change the battle position of a targeted face-up monster. | Full |  |
| 1332 | Monster | Flip / when summoned: the targeted opponent monster cannot attack while this card is face up. | Full | Shares Dark-Eyes Illusionist's handler. |
| 1333 | Monster | Fusion material of 1334 (with Relinquished). No effect code. | No effect code | In the recipe table only; no constant in `cards.h`. OCG *Thousand-Eyes Idol* (hypothesis, from the recipe). |
| 1334 | Fusion | Fusion of Relinquished + 1333. Once per turn: absorb an opponent monster as an equip (like Relinquished; its ATK/DEF become the absorbed monster's). While it is on either field, no other monster can attack or change its battle position. | Full | OCG *Thousand-Eyes Restrict* (hypothesis: the recipe and behaviour match). Shown as Fusion/Effect. |
| 1336 | Monster | Can attack twice per Battle Phase. | Full (passive) | Battle code; empty effect-table row. |
| 1337 | Monster | Flip: turn one of the opponent's face-down Defense Position cards face up without its flip effect; if it is an effect monster, destroy it. | Full |  |
| 1338 | Monster | Flip: destroy each of the opponent's face-up level 4 monsters. | Full |  |
| 1339 | Monster | Only named as the monster 1540 may equip (+300 ATK). No effect of its own. | No effect code | Reference only. |
| 1340 | Monster | When it is destroyed in battle, a Special Summon for the other player is queued for the end of the Battle Phase. | Full (passive) | Battle code; empty effect-table row. The summon's details were not traced. |
| 1341 | Monster | Inflicts piercing battle damage. | Full (passive) | Empty effect-table row; the rule is in `CalcBattle`. |
| 1344 | Monster | After it attacks it changes to Defense Position at the end of the Battle Phase, and it cannot change position at the start of the next turn. | Full (passive) | Battle and turn-start code; empty effect-table row. |
| 1350 | Monster | Special Summon from the hand if the opponent controls at least 2 more monsters than you; otherwise it needs a Tribute. | Full (passive) | Summon rule in `CanSummonFromHand` and the card menu. |
| 1351 | Monster | Cannot be equipped: an equip put on it is destroyed (and a Premature Burial on it destroys it). Snatch Steal cannot take it. | Full (passive) | No constant name for the Snatch Steal test (literal 1351). |
| 1352 | Magic/Trap | While it is on the opponent's field, your Insect monsters cannot attack. | Full (passive) | `CanMonsterAttack`; resolve is a no-op. |

### 1402-1451

| Number | Kind | Effect | Status | Notes |
|---|---|---|---|---|
| 1402 | Monster | Fusion material of 1404. No effect code. | No effect code | In the recipe table only; no constant in `cards.h`. |
| 1403 | Monster | Fusion material of 1404. No effect code. | No effect code | In the recipe table only; no constant in `cards.h`. |
| 1404 | Fusion | Fusion of 1403 + 1402. No effect code. | No effect code | Recipe only; possibly a vanilla Fusion. |
| 1405 | Monster | When it is destroyed in battle: you may pay 1000 LP to Special Summon it in your next Standby Phase. 1428 can redirect attacks to it. | Full | OCG *Revival Jam* (hypothesis). Cosmetic bug: 1428's prompt looks up its name through the number→ID table, which gives card 0 in EDS. |
| 1410 | Monster | Only named as one of the Tributes 1257 needs. No effect of its own. | No effect code | Reference only. |
| 1411 | Ritual | Ritual monster of the ritual spell 1416 (level-8 Tributes). No effect code. | No effect code | In the ritual table only; no constant in `cards.h`. |
| 1412 | Monster | Only named as one of the Tributes 1257 needs. No effect of its own. | No effect code | Reference only. |
| 1413 | Monster | Destroyed when Normal Summoned; -200 ATK for each monster the opponent controls. | Full (passive) | Summon and stat code (like Boar Soldier). |
| 1414 | Magic | Hand roulette: pick a monster and two Magic/Trap cards in your hand; one of the three is chosen at random. The other two are discarded; a chosen monster is Special Summoned, a chosen Magic/Trap is discarded too. | Full |  |
| 1415 | Trap | When the opponent attacks: link this card to the attacker, which loses 700 ATK per link. | Full |  |
| 1416 | Ritual Magic | Ritual spell: summon ritual monster 1411 by tributing level 8 worth of monsters. | Full | Uses the shared ritual code and recipe row 15; only useful if 1411 exists. |
| 1417 | Trap | Put a "mask" on a face-up Magic card: its controller loses 500 LP in each of their Standby Phases while the link lasts. | Full | OCG *Mask of Dispel* (hypothesis). |
| 1418 | Trap | While it is on either field, no player can Tribute: no Tribute Summons, rituals, Tribute costs or Tribute-based effects. | Full (passive) | OCG *Mask of Restrict* (hypothesis). Resolve is a no-op; about 30 checks across the duel code test for it. |
| 1419 | Equip | Equipped monster cannot attack, and its controller loses 500 LP in each of their Standby Phases. | Full | The CPU puts it on the opponent's monsters. |
| 1420 | Equip | +1000 ATK and -1000 DEF. Costs 1000 LP in each of your Standby Phases (destroyed if you cannot pay). | Full |  |
| 1421 | Monster | Cost: discard a monster from the hand. Pick a graveyard monster destroyed in battle this turn and mark it. | Partial | The mark sets graveyard flag bit 23, which nothing in the code reads, so the effect has no result. |
| 1422 | Equip | Warriors only: +800 ATK. | Full |  |
| 1423 | Monster | Only activatable while Umi is the face-up Field Magic. Its controller takes no battle damage; it is destroyed when Umi leaves the field. | Full (passive) | Battle and field-change code. |
| 1424 | Trap | Costs 500 LP in each of your Standby Phases. When the opponent attacks: call a coin toss; a right call makes the attacker's ATK 0 for that battle. | Full | OCG *Fairy Box* (hypothesis). |
| 1425 | Trap | When a monster is summoned (any way) and there is a monster on the field: destroy every monster. | Full | A Dark Hole trap (shares Dark Hole's handler). |
| 1426 | Magic/Trap | In each of your Standby Phases, summon a token into a free zone. While it is face up, you cannot Normal or Special Summon. | Full (passive) | Standby code and the summon checks; resolve is a no-op. |
| 1427 | Magic/Trap | While it is on either field, there is no hand-size limit in the End Phase. | Full (passive) | Resolve is a no-op. |
| 1428 | Trap | Activated from a Set position only. When an opponent monster attacks one of yours: make your face-up 1405 the attack target instead. | Full | OCG *Jam Defender* (hypothesis). |
| 1429 | Magic/Trap | While it is face up, you draw one card (per copy) whenever you Special Summon a monster from the graveyard. | Full (passive) | Summon code; resolve is a no-op. |
| 1430 | Monster | Tribute this card: return a monster destroyed in battle from your graveyard to the top of the deck. | Full | Blocked while 1418 is active. |
| 1431 | Monster | In each of your Standby Phases, Tribute another monster to keep it, otherwise it is destroyed. | Full (passive) | Empty effect-table row; the upkeep is in the Standby code (like The Regulation of Tribe). |
| 1432 | Monster | Tribute two of your monsters: the opponent loses 1200 LP. | Full |  |
| 1433 | Monster | Tribute this card: return a monster destroyed in battle from your graveyard to the bottom of the deck. | Full |  |
| 1434 | Monster | Whenever you gain LP, the opponent loses 500 LP for each face-up copy. | Full (passive) | `GainLifePoints`; no constant name (literal 1434). |
| 1435 | Monster | Flip: banish the top 3 cards of your deck; the opponent loses 800 LP. | Full |  |
| 1436 | Monster | Flip: banish 2 random cards from your hand; the opponent loses 800 LP. | Full | OCG *Fire Sorcerer* (hypothesis). |
| 1437 | Monster | +1000 LP in each of your Standby Phases while it is in Attack Position. | Full (passive) | Empty effect-table row; Standby code. |
| 1438 | Monster | +1000 LP in each of your Standby Phases while it is in Defense Position. | Full (passive) | Empty effect-table row; Standby code. |
| 1439 | Monster | Tribute this card: put a Magic card from your graveyard that the opponent destroyed on the bottom of your deck. | Full |  |
| 1441 | Monster | +800 LP in each of your Standby Phases. | Full (passive) | Empty effect-table row; Standby code. |
| 1442 | Monster | Tribute two of your monsters: gain 1000 LP. | Full |  |
| 1444 | Monster | Tribute this card: return a monster destroyed in battle from your graveyard to the hand. Fusion material of 1445. | Full |  |
| 1445 | Fusion | Fusion of 1444 + 1446. No effect code. | No effect code | Recipe only; possibly a vanilla Fusion. |
| 1446 | Monster | In its owner's Standby Phase, +200 LP for each copy in the graveyard. Fusion material of 1445. | Full (passive) | Empty effect-table row; Standby code. |
| 1447 | Trap | Draw 1 card. | Full | Shares Pot of Greed's handler. The CPU uses it. |
| 1448 | Equip | Declare an Attribute: the equipped monster becomes that Attribute (shown with its icon on the duel screen). | Full |  |
| 1449 | Equip | +800 ATK/DEF for each face-up monster of the equipping player. | Full |  |
| 1450 | Equip | +500 ATK/DEF for each Magic/Trap card of the equipping player. | Full |  |
| 1451 | Magic/Trap | Destroy a targeted face-up card; you skip your next Draw Phase. | Full | Shares Bell of Destruction's handler. |

### 1510-1552

| Number | Kind | Effect | Status | Notes |
|---|---|---|---|---|
| 1510 | Monster | Cost: discard a random card. Destroy every Special Summoned monster on the field; neither player may Special Summon this turn. While it is on either field, nobody can Special Summon. | Full |  |
| 1511 | Monster | When it inflicts battle damage: banish up to 2 monsters from the opponent's graveyard. While it is on your field, the opponent cannot banish from graveyards (Gravedigger Ghoul, Soul Release, banish costs, 1532). | Full | OCG *Kycoo the Ghost Destroyer* (hypothesis). |
| 1512 | Monster | Flip: tribute another of your monsters and Special Summon a monster from the Fusion Deck; it is destroyed at the end of the turn. | Full (human only) | For the CPU (player 1) the effect does nothing. |
| 1513 | Monster | Once per turn: banish up to 3 monsters from your graveyard; +300 ATK each until your next turn. | Full | OCG *Bazoo the Soul-Eater* (hypothesis). |
| 1514 | Monster | Special Summon only, by banishing 3 Fiends from your graveyard. When the opponent's effect destroys it: at the End Phase it is equipped from the graveyard to a face-up opponent monster, and you take control of that monster. | Full | OCG *Dark Necrofear* (hypothesis). The CPU has a strategy for it. |
| 1515 | Monster | Special Summon only, by banishing 2 LIGHT monsters from your graveyard. While it is face up, the opponent's monsters lose 300 ATK during their Battle Phase. | Full (passive) | Empty effect-table row; summon and stat code. |
| 1516 | Monster | Special Summon only, by banishing a FIRE monster from your graveyard. +300 ATK during your Battle Phase. | Full (passive) | Empty effect-table row; summon and stat code. |
| 1517 | Monster | Special Summon only, by banishing a WATER monster from your graveyard. In the opponent's Standby Phase you may change the position of one of their face-up monsters and lock it. | Full |  |
| 1518 | Monster | Special Summon only, by banishing an EARTH monster from your graveyard. +300 ATK during the opponent's Battle Phase. | Full (passive) | No effect-table row; summon and stat code. |
| 1519 | Monster | Special Summon only, by banishing a WIND monster from your graveyard. In the opponent's End Phase you may change the position of one of their face-up monsters and lock it. | Full |  |
| 1520 | Monster | Can be Special Summoned from the hand; when it is, the opponent may Special Summon a monster from their own graveyard. | Full |  |
| 1521 | Monster | Flip: return two Magic/Trap cards on the field to their owners' hands. | Full |  |
| 1522 | Monster | When it is attacked and you have another monster, you may redirect the attack to another of your monsters. | Full (passive) | Battle code; empty effect-table row. |
| 1523 | Monster | Cannot attack while the opponent has no monsters. After it destroys a monster in battle it loses 200 ATK (once). Fusion material of 1526. | Full (passive) | Battle code; empty effect-table row. |
| 1524 | Monster | Flip: add two monsters from your graveyard that were used as Fusion materials to your hand. | Full |  |
| 1525 | Monster | While face up, when the opponent plays a Magic card: tribute this card to negate and destroy it. Fusion material of 1526. | Full | Blocked while 1418 is active. |
| 1526 | Fusion | Fusion of 1523 + 1525. When Special Summoned, your other monsters are destroyed. While it is on either field, nobody can Normal or Special Summon. | Full (passive) | No effect-table row; summon code. Shown as Fusion/Effect. |
| 1527 | Magic | Move every face-up Equip card on the field to one targeted monster; equips that cannot go there are destroyed. | Full |  |
| 1528 | Trap | Destiny Board: in each of the opponent's End Phases, the next Spirit Message letter (1541-1544) is placed in your Magic/Trap zone. With Destiny Board and all four letters on your field, you win the duel (the Destiny Board win scene). | Full | Fully wired: `PlaceNextSpiritMessage`, `HasDestinyBoardComplete` and the win scene all exist, but no EDS card has the numbers. |
| 1529 | Trap | When the opponent attacks: choose another of their face-up monsters to attack instead (it is switched to Attack Position). | Full |  |
| 1530 | Trap | Counter Trap: negate a flip-effect monster's effect. While it is on either field, flip effects do not trigger at all. | Full |  |
| 1531 | Trap | Counter Trap: negate a Magic card that targets a monster (equips, Change of Heart, Snatch Steal, ...). | Full |  |
| 1532 | Trap | Activated from a Set position only. Banish monsters from your graveyard equal to a face-up monster's level to destroy it. | Full |  |
| 1533 | Magic/Trap | While it is face up, in each of your Standby Phases the opponent loses 100 LP for each of their banished monsters. | Full (passive) | Standby code; resolve is a no-op. |
| 1534 | Magic | A face-up monster gains 100 ATK for each monster in your graveyard until the end of the turn. | Full | Quirk (ROM): the graveyard scan reuses the link loop index. |
| 1535 | Trap | When an opponent monster attacks your Defense Position monster and its controller takes battle damage: destroy the attacker. | Full |  |
| 1536 | Magic/Trap | In each of your Standby Phases, roll a die: destroy every face-up monster of that level (a 6 also destroys levels above 6). | Full (passive) | Standby code; resolve is a no-op. An unused copy of the routine also exists (`RollDieDestroyMonstersByLevel`). |
| 1537 | Equip | While it is on either field, every other Equip Magic is negated. | Full | Shares Royal Decree / Imperial Order's handler. |
| 1538 | Magic/Trap | While it is on either field, the Battle Phase ends after the first attack. | Full (passive) | `BattleStage_EndAttack`; resolve is a no-op. |
| 1539 | Magic | Return a Magic or Trap card of the opponent to their hand. It can never be negated. | Full |  |
| 1540 | Equip | Equips only to 1339: +300 ATK and piercing battle damage. | Full | Usable only if 1339 exists. |
| 1541 | Magic | Spirit Message letter 1. Never activated or Set from the hand: Destiny Board (1528) places it. | Full | Part of Destiny Board. Its prepare handler always returns 0 by design. |
| 1542 | Magic | Spirit Message letter 2 (see 1541). | Full | Part of Destiny Board. |
| 1543 | Magic | Spirit Message letter 3 (see 1541). | Full | Part of Destiny Board. |
| 1544 | Magic | Spirit Message letter 4 (see 1541). | Full | Part of Destiny Board. |
| 1545 | Trap | Flip a targeted face-down Magic/Trap card. A non-Trap is set again; a Trap that cannot be activated now is destroyed, any other Trap is activated. This card then goes back into the deck. | Full | OCG *Bait Doll* (hypothesis). |
| 1546 | Magic | Return a face-up Fusion monster to the Fusion Deck; you may then Special Summon its Fusion materials from your graveyard. | Full (human only) | OCG *De-Fusion* (hypothesis). For the CPU the effect ends after the return. |
| 1547 | Field Magic | Field Magic with its own background (index 14). While it is the face-up Field Magic, you can Fusion Summon from the Fusion Deck through the card menu. | Full | The background image exists (`gFieldBackgroundImages[14]`). |
| 1548 | Equip | Equipped monster cannot attack. At the End Phase the equipped monster is destroyed and this card returns to its owner's hand. | Full | The CPU puts it on the opponent's monsters. The End Phase conditions were not fully traced. |
| 1549 | Magic | With more than 4 of your monsters banished: return up to three of them to the graveyard. | Full | Blocked by a face-up Banisher of the Light. |
| 1550 | Equip | Warriors only: the monster becomes a Dragon and gains 500 ATK/DEF. | Full |  |
| 1551 | ? | Sets a flag for the turn under which the banish costs of 1514-1519 come from your own field instead of the graveyard (hypothesis from the readers of the flag). | Full | The card behind it is unknown. |
| 1552 | Magic/Trap | While it is on either field, a monster summoned face up cannot attack that turn. | Full (passive) | Summon code; resolve is a no-op. |

## Cards in EDS that cannot be played

Seven cards do exist in EDS (they have IDs, names and art) but can never be used in a duel:

| Number | ID | Card | Why it cannot be played | Effect code |
|---|---|---|---|---|
| 1901-1903 | 814-816 | The Monarchy, Set Sail for the Kingdom, Glory of the King's Hand | Type Ticket. They are the prizes for the rounds of the November tournament. Their copy limit on the Forbidden/Limited list is 0, so no deck may contain them. | None. |
| 1910-1912 | 817-819 | Obelisk the Tormentor, Slifer the Sky Dragon, The Winged Dragon of Ra | Type Divine, 0/0 in the stats table, no password, copy limit 0. | No effect-table rows. Helper code treats them as level 10 with 4000 ATK (AI, level checks) and draws Obelisk with a Ritual frame and Slifer/Ra with Effect frames. |
| 1920 | 820 | Insect Monster Token | A token: created by Insect Queen, hidden from Deck Edit, packs and trading, copy limit 0. | Its stats only. |

## Method and limits

- Sources: the decompiled C (every function in the ROM matches byte for byte), the effect table (`assets/tables/card_effect_handlers.json`), the comments in `include/constants/cards.h`, and the Fusion (`0x0819A7C8`) and ritual (`0x0819A990`) recipe tables read from the ROM. For every number, every source line that tests it was collected and read along with the comment of each handler it uses.
- "Full" means every piece the code needs is present. Nothing here was run in an emulator. The behaviour of some continuous rules (1243, 1327, 1340, 1548) was read from the code, and their exact rules text is a best guess.
- The OCG names are hypotheses from behaviour, from the source comments and, for 1241 and 1334, from the Fusion recipes. The ROM contains no names for these numbers.
- Numbers tested only through computed values (not a constant or a literal in the source) would not have been found. None are known.
