# Leftover card effects

*Yu-Gi-Oh! The Eternal Duelist Soul* (GBA, USA) contains duel code for 157 card numbers that have no card in the game. Each number below is one of them: the engine knows how to play it, but no card ID maps to it, so it can never be drawn, built into a deck or obtained. This document lists what each one does and how complete its code is.

Almost all of them are card numbers 1210 and above. That range seems to be left over from the larger card database shared with Konami's other Duel Monsters games (an assumption; nothing in the ROM names those games). Most of the effects match real OCG cards from around 2001-2002; where the source code names a likely OCG card, or the game's own Fusion recipe identifies one, it is given in the notes as a hypothesis.

## How effects are attached to cards

- **Effects are found by card number, not card ID.** The effect table at `0x0819A9D4` (426 rows: number, then resolve, check, prepare, chainA and chainB handlers) is searched by number. Many other rules (battle, stats, summoning, phases) test the number directly in the code.
- **136 of the table's rows** belong to numbers with no EDS card. A further 21 such numbers have rules elsewhere in the code, or appear only in the Fusion/ritual recipe tables.
- **Making them playable.** `tools/card_expand/expand_rom.py` adds every card in the "Card" column to a ROM at its own number, with its stats, text, password and artwork, so all of these effects can be used (see `wiki/tools/card-expand.md`). To add a single one by hand, give a new card ID that number (`tools/card_expand/write_tables.py`). The effect then works as described here; nothing else needs to change. Card type, level, ATK/DEF and attribute come from the new card's stats, so they must suit the effect (for example a Trap for a trap effect, a Machine for 1314's targets).

## Status legend

| Status | Meaning | Count |
|---|---|---|
| Full | Everything the effect needs is coded: activation conditions, targeting, costs and the result. | 95 |
| Full (passive) | Fully coded, but as continuous rules in the duel code (battle, stats, phases, summon checks) rather than through an activated effect. Many have an empty effect-table row or a no-op resolve handler. | 44 |
| Full (human only) | Fully coded for the human player; the CPU's path is missing or does nothing. | 4 |
| Full, buggy | Fully coded, but a bug in the original code changes the result. | 2 |
| Partial | Part of the effect is missing. | 2 |
| No effect code | The number is only named (as a Fusion material, ritual monster, Tribute, equip target or AI hint); there is no effect for it. | 10 |
| **Total** | | **157** |

"Kind" is the identified card's type as printed. For the three numbers without a card it is the kind the code implies.

## Effects

### Numbers below 1300

| Number | Card (identified) | Kind | Effect | Status | Notes |
|---|---|---|---|---|---|
| 230 | unknown | ? | Only the CPU knows it: `AiTryChainResponse` will not chain it while the CPU has more than 1000 LP. | No effect code | AI hint only. The number is below 1200, so it is an ordinary Konami card left out of EDS. |
| 1210 | none (a Dark Magician variant) | Monster | A second Dark Magician. It counts as Dark Magician everywhere: same-name checks, the copy limit, the +300 ATK per Dark Magician in the graveyards (presumably Dark Magician Girl's bonus), Time Wizard, the Dark Magician searches of 1211/1212. | Full | Alias only; it needs no effect of its own. |
| 1211 | Thousand Knives | Spell | If you control Dark Magician (either art, or 1210) and the opponent has a monster: destroy one targeted monster. | Full | Shares Man-Eater Bug's destroy handler. |
| 1212 | Dark Magic Curtain | Spell | Pay half your LP; Special Summon a Dark Magician from the deck. Needs an unused Normal Summon; afterwards you cannot Normal or Special Summon for the turn. | Full |  |
| 1213 | Mystic Box | Spell | Target one opponent monster and one of yours: destroy theirs, then give yours to the opponent in its zone. | Full | Quirk: the two target picks advance even if a pick is refused. |
| 1214 | Magic Cylinder | Trap | When the opponent declares an attack: negate it, and the attacker's controller loses LP equal to the attacker's ATK. | Full |  |
| 1220 | Soul Exchange | Spell | Main Phase 1, costs your Battle Phase: use one opponent monster as a Tribute, for a Tribute Summon of a level 5+ monster or for Catapult Turtle / The Little Swordsman of Aile / Cannon Soldier. | Full, buggy | ROM bug: the summon paths tribute the monster in the target side's zone 0, not the target. |
| 1221 | Card Destruction | Spell | Both players discard their whole hand and draw that many cards. | Full | The CPU uses it (AI routines check for it). |
| 1230 | Nightmare's Steelcage | Spell | While it is on either field, no monster can attack (the attack menu is empty). It gets a turn counter in the End Phase and is destroyed when the counter runs out. | Full (passive) | Handled next to Swords of Revealing Light; resolve is a no-op, the rules are continuous. |
| 1231 | Rocket Warrior | Effect Monster | When it attacks it is not destroyed and takes no battle damage, and the defender gets a -500 ATK link. | Full (passive) | Battle code (`CalcBattle`, `GetZoneCardStats`). |
| 1232 | Multiply | Quick-Play Spell | Tribute a face-up Kuriboh: fill every free monster zone of yours with Kuriboh tokens (card number 1921, Defense Position). | Full | Blocked while 1418 is active. |
| 1240 | Berfomet | Effect Monster | When summoned (Normal, Flip or Special): add Gazelle the King of Mythical Beasts from the deck to the hand (you are asked; the CPU always does it). | Full | Also a Fusion material of 1241. |
| 1241 | Chimera the Flying Mythical Beast | Fusion Monster | Fusion of Gazelle the King of Mythical Beasts + 1240. When it is destroyed (in battle or by an effect, after being summoned): Special Summon a monster from your graveyard (the Monster Reborn handler with key 1241's own candidate list). | Full | Shown as an effect monster on the card screen. |
| 1242 | Archfiend of Gilfer | Effect Monster | Equipped monster: -500 ATK. Whenever this card is sent to the graveyard (discard, mill, tribute, battle, Painful Choice...), you may equip it from the graveyard to a face-up monster on either side. | Full |  |
| 1243 | Ancient Lamp | Effect Monster | While it is face up, you can Special Summon La Jinn the Mystical Genie of the Lamp from your hand. When it is attacked while face down and its controller has another monster, the attack can be switched to a substitute target, and the attacked monster moves to the attacker's side afterwards. | Full (passive) | Battle code and the card menu; resolve is a no-op. The card text says the attack goes to another monster of the attacker; how the code's substitute and move map onto that was not traced in detail. |
| 1244 | Shadow Spell | Continuous Trap | Link to a face-up opponent monster: it cannot attack or change position, and has -700 ATK while the link lasts. | Full | A stronger Spellbinding Circle (shares its handler). |
| 1245 | Scapegoat | Quick-Play Spell | With 4 free monster zones and no summon this turn: summon four tokens (card number 1922, Defense Position); no more Normal or Special Summons this turn. | Full | The CPU has a strategy built around it (with Cannon Soldier). |
| 1246 | Copycat | Effect Monster | When summoned (any way): it gains the base ATK/DEF of a face-up opponent monster you pick. | Full |  |
| 1247 | Mystical Refpanel | Trap | When the opponent plays one of a list of one-player Magic cards (healing, burn, draw, hand cards): run that card for yourself instead, and negate the original. | Full |  |
| 1248 | Makiu, the Magical Mist | Spell | Main Phase 1, costs your Battle Phase: target your face-up Summoned Skull or Thunder monster; destroy every face-up opponent monster whose DEF is below its ATK. | Full |  |
| 1249 | none (a Harpie Lady variant) | Monster | A second Harpie Lady: counts as Harpie Lady for same-name checks, the copy limit, Cyber Shield, Harpie's Pet Dragon and Elegant Egotist. | Full | Alias only. |
| 1250 | Swift Gaia the Fierce Knight | Effect Monster | Summon rule: Normal Summon without Tributes when it is your only hand card, otherwise it needs two Tributes. | Full (passive) | `CanSummonFromHand`. |
| 1251 | Obnoxious Celtic Guard | Effect Monster | Not destroyed in battle by a monster with 1900 or more ATK. | Full (passive) | Empty effect-table row; the rule is in `CalcBattle`. |
| 1252 | Fairy King Truesdale | Effect Monster | Your Plant monsters gain 500 ATK/DEF for each face-up Defense Position copy of this card you control. | Full (passive) | `GetZoneCardStats`. |
| 1253 | Cipher Soldier | Effect Monster | +2000 ATK/DEF when it battles a Warrior (attacking or defending). | Full (passive) | Empty effect-table row; the rule is in `CalcBattle`. |
| 1254 | Slate Warrior | Flip Effect Monster | Once per turn: +500 ATK/DEF until the end of the turn (Karate Man's handler). In battle it also links a -500 stats effect to the monster it battles. | Full | The battle link is in `BattleStage_DestroyMonsters` (shared with 1255 and 1256); not traced in detail. |
| 1255 | Electromagnetic Bagworm | Flip Effect Monster | Flip: take control of a face-up opponent Machine until the end of the turn. | Full |  |
| 1256 | Rigorous Reaver | Flip Effect Monster | Flip: each player discards one card. | Full |  |
| 1257 | Masked Beast Des Gardius | Effect Monster | Cannot be Normal Summoned; summoned by tributing two monsters, one of them 1410 or 1412. When it is sent to the graveyard after being summoned: take 1258 from your deck, equip it to a face-up monster, and that monster changes sides. | Full |  |
| 1258 | The Mask of Remnants | Spell | Equip card for 1257's effect (any face-up monster). Its own effect returns it to the top of the deck and shuffles; it stays on the field while it resolves. | Full |  |

### 1301-1352

| Number | Card (identified) | Kind | Effect | Status | Notes |
|---|---|---|---|---|---|
| 1301 | Michizure | Trap | When one of your monsters is sent to the graveyard, or a battle involves your side: destroy one targeted monster. | Full |  |
| 1302 | Minor Goblin Official | Continuous Trap | Can be activated only while the opponent has 3000 LP or less. While it is face up, the opponent loses 500 LP in each of their Standby Phases. | Full (passive) | Resolve is a no-op; the drain is in `ApplyStandbyPhaseEffects`. |
| 1303 | Gamble | Trap | Only when the opponent holds 6+ cards and you 2 or fewer: call a coin toss. Right: draw until you hold 5 cards. Wrong: skip your next turn. | Full |  |
| 1304 | Attack and Receive | Trap | When you take damage: the opponent loses 700 LP plus 300 for each copy in your graveyard. | Full | Uses Numinous Healer's handler in reverse. |
| 1305 | Solemn Wishes | Continuous Trap | While it is face up, you gain 500 LP each time you draw (per copy). | Full (passive) | `DrawCards`; resolve is a no-op. |
| 1306 | Skull Invitation | Continuous Trap | While it is on either field, a player loses 300 LP for each card sent to their graveyard. | Full (passive) | `LoseLpOnSendToGraveyard`; resolve is a no-op. |
| 1307 | Bubonic Vermin | Flip Effect Monster | Flip: you may Special Summon one more copy of this card from the deck, face down in Defense Position. | Full | Shares Giant Germ / Nimble Momonga's handler; key 1307 stops after one copy by design. |
| 1311 | Burning Land | Continuous Spell | Destroy the card in each player's Field Magic zone. While it stays face up, you lose 500 LP in each of your Standby Phases. | Full |  |
| 1312 | Cold Wave | Spell | Only at the start of Main Phase 1 (no Magic/Trap played and no Normal Summon yet): neither player may play Magic or Trap cards for 2 turns. | Full |  |
| 1313 | Fairy Meteor Crush | Equip Spell | Equip to any face-up monster: it inflicts piercing battle damage. | Full |  |
| 1314 | Limiter Removal | Quick-Play Spell | Double the ATK of every face-up Machine you control; those Machines are destroyed at the end of the turn. | Full | The CPU has a strategy for it. |
| 1315 | Rain of Mercy | Spell | Both players gain 1000 LP. | Full |  |
| 1316 | Monster Recovery | Quick-Play Spell | Return one of your monsters and your whole hand to the deck, shuffle, and draw as many cards as the hand held. | Full (human only) | Its target picker has no CPU branch. |
| 1317 | Shift | Trap | Move the opponent's attack, or a single-target effect, onto another of your monsters. | Full |  |
| 1318 | Insect Imitation | Spell | Tribute one of your monsters: Special Summon an Insect from the deck whose level is one higher. | Full, buggy | ROM bug: the packed position is passed as the zone, so the Tribute and level lookup only work for one zone. |
| 1319 | Interdimensional Matter Transporter | Trap | Banish one of your monsters; it comes back later and its zone stays reserved for it. | Full (human only) | The return is in the Standby Phase code. Its target picker has no CPU branch. |
| 1320 | Ground Collapse | Continuous Spell | Choose two empty monster zones (either side): they cannot be used while this card stays on the field. | Full |  |
| 1321 | Magic Drain | Counter Trap | When the opponent plays a Magic card: negate it unless they discard a Magic card. | Full |  |
| 1322 | Infinite Dismissal | Continuous Trap | While it is on either field, each End Phase destroys the turn player's face-up monsters of level 3 or less. | Full (passive) | `EndPhase_DestroyLowLevelMonsters`; resolve is a no-op. |
| 1323 | Gravity Bind | Continuous Trap | While it is on either field, monsters above level 3 cannot attack. | Full (passive) | `CanMonsterAttack`; resolve is a no-op. |
| 1324 | Type Zero Magic Crusher | Continuous Trap | Activated from a Set position only. Cost: discard a Magic card. The opponent loses 500 LP. | Full |  |
| 1325 | Shadow of Eyes | Trap | When the opponent Sets a monster: turn it face-up in Attack Position, without its flip effect. | Full |  |
| 1326 | The Legendary Fisherman | Effect Monster | While Umi is the face-up Field Magic, Magic cards (other than equips) cannot target it or affect it, and it cannot be picked as an attack target in some cases. | Full (passive) | Targeting and stat code; empty effect-table row. |
| 1327 | Sword Hunter | Effect Monster | At the end of the Battle Phase, the monsters it destroyed in battle are equipped to it from the graveyard (the card text gives +200 ATK for each). | Full (passive) | Battle code; empty effect-table row. |
| 1328 | Drill Bug | Effect Monster | When it inflicts battle damage: you may put Parasite Paracide from your deck on top of the deck. | Full |  |
| 1329 | Deepsea Warrior | Effect Monster | Same as 1326: Magic cannot target it while Umi is the face-up Field Magic. | Full (passive) | No constant name in the source; tested as the literal 1329. |
| 1330 | Bite Shoes | Flip Effect Monster | Flip: change the battle position of a targeted face-up monster. | Full |  |
| 1332 | Invitation to a Dark Sleep | Effect Monster | Flip / when summoned: the targeted opponent monster cannot attack while this card is face up. | Full | Shares Dark-Eyes Illusionist's handler. |
| 1333 | Thousand-Eyes Idol | Normal Monster | Fusion material of 1334 (with Relinquished). No effect code. | No effect code | In the recipe table only; no constant in `cards.h`. |
| 1334 | Thousand-Eyes Restrict | Fusion Monster | Fusion of Relinquished + 1333. Once per turn: absorb an opponent monster as an equip (like Relinquished; its ATK/DEF become the absorbed monster's). While it is on either field, no other monster can attack or change its battle position. | Full | Shown as Fusion/Effect. |
| 1336 | Hayabusa Knight | Effect Monster | Can attack twice per Battle Phase. | Full (passive) | Battle code; empty effect-table row. |
| 1337 | Bombardment Beetle | Flip Effect Monster | Flip: turn one of the opponent's face-down Defense Position cards face up without its flip effect; if it is an effect monster, destroy it. | Full |  |
| 1338 | 4-Starred Ladybug of Doom | Flip Effect Monster | Flip: destroy each of the opponent's face-up level 4 monsters. | Full |  |
| 1339 | Gradius | Normal Monster | Only named as the monster 1540 may equip (+300 ATK). No effect of its own. | No effect code | Reference only. |
| 1340 | Vampire Baby | Effect Monster | At the end of the Battle Phase, a monster it destroyed in battle is Special Summoned from the graveyard to its controller's side. | Full (passive) | Battle code; empty effect-table row. |
| 1341 | Mad Sword Beast | Effect Monster | Inflicts piercing battle damage. | Full (passive) | Empty effect-table row; the rule is in `CalcBattle`. |
| 1344 | Goblin Attack Force | Effect Monster | After it attacks it changes to Defense Position at the end of the Battle Phase, and it cannot change position at the start of the next turn. | Full (passive) | Battle and turn-start code; empty effect-table row. |
| 1350 | The Fiend Megacyber | Effect Monster | Special Summon from the hand if the opponent controls at least 2 more monsters than you; otherwise it needs a Tribute. | Full (passive) | Summon rule in `CanSummonFromHand` and the card menu. |
| 1351 | Gearfried the Iron Knight | Effect Monster | Cannot be equipped: an equip put on it is destroyed (and a Premature Burial on it destroys it). Snatch Steal cannot take it. | Full (passive) | No constant name for the Snatch Steal test (literal 1351). |
| 1352 | Insect Barrier | Continuous Spell | While it is on the opponent's field, your Insect monsters cannot attack. | Full (passive) | `CanMonsterAttack`; resolve is a no-op. |

### 1402-1451

| Number | Card (identified) | Kind | Effect | Status | Notes |
|---|---|---|---|---|---|
| 1402 | Humanoid Slime | Normal Monster | Fusion material of 1404. No effect code. | No effect code | In the recipe table only; no constant in `cards.h`. |
| 1403 | Worm Drake | Normal Monster | Fusion material of 1404. No effect code. | No effect code | In the recipe table only; no constant in `cards.h`. |
| 1404 | Humanoid Worm Drake | Fusion Monster | Fusion of 1403 + 1402. No effect code. | No effect code | Recipe only; possibly a vanilla Fusion. |
| 1405 | Revival Jam | Effect Monster | When it is destroyed in battle: you may pay 1000 LP to Special Summon it in your next Standby Phase. 1428 can redirect attacks to it. | Full | Cosmetic bug: 1428's prompt looks up its name through the number→ID table, which gives card 0 in EDS. |
| 1410 | Grand Tiki Elder | Normal Monster | Only named as one of the Tributes 1257 needs. No effect of its own. | No effect code | Reference only. |
| 1411 | The Masked Beast | Ritual Monster | Ritual monster of the ritual spell 1416 (level-8 Tributes). No effect code. | No effect code | In the ritual table only; no constant in `cards.h`. |
| 1412 | Melchid the Four-Face Beast | Normal Monster | Only named as one of the Tributes 1257 needs. No effect of its own. | No effect code | Reference only. |
| 1413 | Nuvia the Wicked | Effect Monster | Destroyed when Normal Summoned; -200 ATK for each monster the opponent controls. | Full (passive) | Summon and stat code (like Boar Soldier). |
| 1414 | Chosen One | Spell | Hand roulette: pick a monster and two Magic/Trap cards in your hand; one of the three is chosen at random. The other two are discarded; a chosen monster is Special Summoned, a chosen Magic/Trap is discarded too. | Full |  |
| 1415 | Mask of Weakness | Trap | When the opponent attacks: link this card to the attacker, which loses 700 ATK per link. | Full |  |
| 1416 | Curse of the Masked Beast | Ritual Spell | Ritual spell: summon ritual monster 1411 by tributing level 8 worth of monsters. | Full | Uses the shared ritual code and recipe row 15; only useful if 1411 exists. |
| 1417 | Mask of Dispel | Continuous Spell | Put a "mask" on a face-up Magic card: its controller loses 500 LP in each of their Standby Phases while the link lasts. | Full |  |
| 1418 | Mask of Restrict | Continuous Trap | While it is on either field, no player can Tribute: no Tribute Summons, rituals, Tribute costs or Tribute-based effects. | Full (passive) | Resolve is a no-op; about 30 checks across the duel code test for it. |
| 1419 | Mask of the Accursed | Equip Spell | Equipped monster cannot attack, and its controller loses 500 LP in each of their Standby Phases. | Full | The CPU puts it on the opponent's monsters. |
| 1420 | Mask of Brutality | Equip Spell | +1000 ATK and -1000 DEF. Costs 1000 LP in each of your Standby Phases (destroyed if you cannot pay). | Full |  |
| 1421 | Return of the Doomed | Spell | Cost: discard a monster from the hand. Pick one of your monsters destroyed in battle this turn and mark it; the card text returns it to your hand at the end of the turn. | Partial | The mark sets graveyard flag bit 23, which nothing reads, so the monster never returns. |
| 1422 | Lightning Blade | Equip Spell | Warriors only: +800 ATK. | Partial | The card text also makes all WATER monsters lose 500 ATK; that part is not coded. |
| 1423 | Tornado Wall | Continuous Trap | Only activatable while Umi is the face-up Field Magic. Its controller takes no battle damage from attacks; it is destroyed when Umi leaves the field. | Full (passive) | Battle and field-change code. |
| 1424 | Fairy Box | Continuous Trap | Costs 500 LP in each of your Standby Phases. When the opponent attacks: call a coin toss; a right call makes the attacker's ATK 0 for that battle. | Full |  |
| 1425 | Torrential Tribute | Trap | When a monster is summoned (any way) and there is a monster on the field: destroy every monster. | Full | A Dark Hole trap (shares Dark Hole's handler). |
| 1426 | Jam Breeding Machine | Continuous Spell | In each of your Standby Phases, summon a token into a free zone. While it is face up, you cannot Normal or Special Summon. | Full (passive) | Standby code and the summon checks; resolve is a no-op. |
| 1427 | Infinite Cards | Continuous Spell | While it is on either field, there is no hand-size limit in the End Phase. | Full (passive) | Resolve is a no-op. |
| 1428 | Jam Defender | Continuous Trap | Activated from a Set position only. When an opponent monster attacks one of yours: make your face-up 1405 the attack target instead. | Full |  |
| 1429 | Card of Safe Return | Continuous Spell | While it is face up, you draw one card (per copy) whenever you Special Summon a monster from the graveyard. | Full (passive) | Summon code; resolve is a no-op. |
| 1430 | Lady Panther | Effect Monster | Tribute this card: return a monster destroyed in battle from your graveyard to the top of the deck. | Full | Blocked while 1418 is active. |
| 1431 | The Unfriendly Amazon | Effect Monster | In each of your Standby Phases, Tribute another monster to keep it, otherwise it is destroyed. | Full (passive) | Empty effect-table row; the upkeep is in the Standby code (like The Regulation of Tribe). |
| 1432 | Amazoness Archer | Effect Monster | Tribute two of your monsters: the opponent loses 1200 LP. | Full |  |
| 1433 | Crimson Sentry | Effect Monster | Tribute this card: return a monster destroyed in battle from your graveyard to the bottom of the deck. | Full |  |
| 1434 | Fire Princess | Effect Monster | Whenever you gain LP, the opponent loses 500 LP for each face-up copy. | Full (passive) | `GainLifePoints`; no constant name (literal 1434). |
| 1435 | Lady Assailant of Flames | Flip Effect Monster | Flip: banish the top 3 cards of your deck; the opponent loses 800 LP. | Full |  |
| 1436 | Fire Sorcerer | Flip Effect Monster | Flip: banish 2 random cards from your hand; the opponent loses 800 LP. | Full |  |
| 1437 | Spirit of the Breeze | Effect Monster | +1000 LP in each of your Standby Phases while it is in Attack Position. | Full (passive) | Empty effect-table row; Standby code. |
| 1438 | Dancing Fairy | Effect Monster | +1000 LP in each of your Standby Phases while it is in Defense Position. | Full (passive) | Empty effect-table row; Standby code. |
| 1439 | Fairy Guardian | Effect Monster | Tribute this card: put a Magic card from your graveyard that the opponent destroyed on the bottom of your deck. | Full |  |
| 1441 | Cure Mermaid | Effect Monster | +800 LP in each of your Standby Phases. | Full (passive) | Empty effect-table row; Standby code. |
| 1442 | Hysteric Fairy | Effect Monster | Tribute two of your monsters: gain 1000 LP. | Full |  |
| 1444 | The Forgiving Maiden | Effect Monster | Tribute this card: return a monster destroyed in battle from your graveyard to the hand. Fusion material of 1445. | Full |  |
| 1445 | St. Joan | Fusion Monster | Fusion of 1444 + 1446. No effect code. | No effect code | Recipe only; possibly a vanilla Fusion. |
| 1446 | Darklord Marie | Effect Monster | In its owner's Standby Phase, +200 LP for each copy in the graveyard. Fusion material of 1445. | Full (passive) | Empty effect-table row; Standby code. |
| 1447 | Jar of Greed | Trap | Draw 1 card. | Full | Shares Pot of Greed's handler. The CPU uses it. |
| 1448 | Scroll of Bewitchment | Equip Spell | Declare an Attribute: the equipped monster becomes that Attribute (shown with its icon on the duel screen). | Full |  |
| 1449 | United We Stand | Equip Spell | +800 ATK/DEF for each face-up monster of the equipping player. | Full |  |
| 1450 | Mage Power | Equip Spell | +500 ATK/DEF for each Magic/Trap card of the equipping player. | Full |  |
| 1451 | Offerings to the Doomed | Quick-Play Spell | Destroy a targeted face-up card; you skip your next Draw Phase. | Full | Shares Bell of Destruction's handler. |

### 1510-1552

| Number | Card (identified) | Kind | Effect | Status | Notes |
|---|---|---|---|---|---|
| 1510 | Jowgen the Spiritualist | Effect Monster | Cost: discard a random card. Destroy every Special Summoned monster on the field; neither player may Special Summon this turn. While it is on either field, nobody can Special Summon. | Full |  |
| 1511 | Kycoo the Ghost Destroyer | Effect Monster | When it inflicts battle damage: banish up to 2 monsters from the opponent's graveyard. While it is on your field, the opponent cannot banish from graveyards (Gravedigger Ghoul, Soul Release, banish costs, 1532). | Full |  |
| 1512 | Summoner of Illusions | Flip Effect Monster | Flip: tribute another of your monsters and Special Summon a monster from the Fusion Deck; it is destroyed at the end of the turn. | Full (human only) | For the CPU (player 1) the effect does nothing. |
| 1513 | Bazoo the Soul-Eater | Effect Monster | Once per turn: banish up to 3 monsters from your graveyard; +300 ATK each until your next turn. | Full |  |
| 1514 | Dark Necrofear | Effect Monster | Special Summon only, by banishing 3 Fiends from your graveyard. When the opponent's effect destroys it: at the End Phase it is equipped from the graveyard to a face-up opponent monster, and you take control of that monster. | Full | The CPU has a strategy for it. |
| 1515 | Soul of Purity and Light | Effect Monster | Special Summon only, by banishing 2 LIGHT monsters from your graveyard. While it is face up, the opponent's monsters lose 300 ATK during their Battle Phase. | Full (passive) | Empty effect-table row; summon and stat code. |
| 1516 | Spirit of Flames | Effect Monster | Special Summon only, by banishing a FIRE monster from your graveyard. +300 ATK during your Battle Phase. | Full (passive) | Empty effect-table row; summon and stat code. |
| 1517 | Aqua Spirit | Effect Monster | Special Summon only, by banishing a WATER monster from your graveyard. In the opponent's Standby Phase you may change the position of one of their face-up monsters and lock it. | Full |  |
| 1518 | The Rock Spirit | Effect Monster | Special Summon only, by banishing an EARTH monster from your graveyard. +300 ATK during the opponent's Battle Phase. | Full (passive) | No effect-table row; summon and stat code. |
| 1519 | Garuda the Wind Spirit | Effect Monster | Special Summon only, by banishing a WIND monster from your graveyard. In the opponent's End Phase you may change the position of one of their face-up monsters and lock it. | Full |  |
| 1520 | Gilasaurus | Effect Monster | Can be Special Summoned from the hand; when it is, the opponent may Special Summon a monster from their own graveyard. | Full |  |
| 1521 | Tornado Bird | Flip Effect Monster | Flip: return two Magic/Trap cards on the field to their owners' hands. | Full |  |
| 1522 | Dreamsprite | Effect Monster | When it is attacked and you have another monster, you may redirect the attack to another of your monsters. | Full (passive) | Battle code; empty effect-table row. |
| 1523 | Zombyra the Dark | Effect Monster | Cannot attack while the opponent has no monsters. After it destroys a monster in battle it loses 200 ATK (once). Fusion material of 1526. | Full (passive) | Battle code; empty effect-table row. |
| 1524 | Supply | Flip Effect Monster | Flip: add two monsters from your graveyard that were used as Fusion materials to your hand. | Full |  |
| 1525 | Maryokutai | Effect Monster | While face up, when the opponent plays a Magic card: tribute this card to negate and destroy it. Fusion material of 1526. | Full | Blocked while 1418 is active. |
| 1526 | The Last Warrior from Another Planet | Fusion Monster | Fusion of 1523 + 1525. When Special Summoned, your other monsters are destroyed. While it is on either field, nobody can Normal or Special Summon. | Full (passive) | No effect-table row; summon code. Shown as Fusion/Effect. |
| 1527 | Collected Power | Trap | Move every face-up Equip card on the field to one targeted monster; equips that cannot go there are destroyed. | Full |  |
| 1528 | Destiny Board | Continuous Trap | Destiny Board: in each of the opponent's End Phases, the next Spirit Message letter (1541-1544) is placed in your Magic/Trap zone. With Destiny Board and all four letters on your field, you win the duel (the Destiny Board win scene). | Full | Fully wired: `PlaceNextSpiritMessage`, `HasDestinyBoardComplete` and the win scene all exist, but no EDS card has the numbers. |
| 1529 | Dark Spirit of the Silent | Trap | When the opponent attacks: choose another of their face-up monsters to attack instead (it is switched to Attack Position). | Full |  |
| 1530 | Royal Command | Continuous Trap | Counter Trap: negate a flip-effect monster's effect. While it is on either field, flip effects do not trigger at all. | Full |  |
| 1531 | Riryoku Field | Counter Trap | Counter Trap: negate a Magic card that targets a monster (equips, Change of Heart, Snatch Steal, ...). | Full |  |
| 1532 | Skull Lair | Continuous Trap | Activated from a Set position only. Banish monsters from your graveyard equal to a face-up monster's level to destroy it. | Full |  |
| 1533 | Graverobber's Retribution | Continuous Trap | While it is face up, in each of your Standby Phases the opponent loses 100 LP for each of their banished monsters. | Full (passive) | Standby code; resolve is a no-op. |
| 1534 | Deal of Phantom | Trap | A face-up monster gains 100 ATK for each monster in your graveyard until the end of the turn. | Full | Quirk (ROM): the graveyard scan reuses the link loop index. |
| 1535 | Destruction Punch | Trap | When an opponent monster attacks your Defense Position monster and its controller takes battle damage: destroy the attacker. | Full |  |
| 1536 | Blind Destruction | Continuous Trap | In each of your Standby Phases, roll a die: destroy every face-up monster of that level (a 6 also destroys levels above 6). | Full (passive) | Standby code; resolve is a no-op. An unused copy of the routine also exists (`RollDieDestroyMonstersByLevel`). |
| 1537 | The Emperor's Holiday | Continuous Trap | While it is on either field, every other Equip Magic is negated. | Full | Shares Royal Decree / Imperial Order's handler. |
| 1538 | The Dark Door | Continuous Spell | While it is on either field, the Battle Phase ends after the first attack. | Full (passive) | `BattleStage_EndAttack`; resolve is a no-op. |
| 1539 | Spiritualism | Spell | Return a Magic or Trap card of the opponent to their hand. It can never be negated. | Full |  |
| 1540 | Cyclon Laser | Equip Spell | Equips only to 1339: +300 ATK and piercing battle damage. | Full | Usable only if 1339 exists. |
| 1541 | Spirit Message "I" | Continuous Spell | Spirit Message letter 1. Never activated or Set from the hand: Destiny Board (1528) places it. | Full | Part of Destiny Board. Its prepare handler always returns 0 by design. |
| 1542 | Spirit Message "N" | Continuous Spell | Spirit Message letter 2 (see 1541). | Full | Part of Destiny Board. |
| 1543 | Spirit Message "A" | Continuous Spell | Spirit Message letter 3 (see 1541). | Full | Part of Destiny Board. |
| 1544 | Spirit Message "L" | Continuous Spell | Spirit Message letter 4 (see 1541). | Full | Part of Destiny Board. |
| 1545 | Bait Doll | Spell | Flip a targeted face-down Magic/Trap card. A non-Trap is set again; a Trap that cannot be activated now is destroyed, any other Trap is activated. This card then goes back into the deck. | Full |  |
| 1546 | De-Fusion | Quick-Play Spell | Return a face-up Fusion monster to the Fusion Deck; you may then Special Summon its Fusion materials from your graveyard. | Full (human only) | For the CPU the effect ends after the return. |
| 1547 | Fusion Gate | Field Spell | Field Magic with its own background (index 14). While it is the face-up Field Magic, you can Fusion Summon from the Fusion Deck through the card menu. | Full | The background image exists (`gFieldBackgroundImages[14]`). |
| 1548 | Ekibyo Drakmord | Equip Spell | Equipped monster cannot attack. At the End Phase the equipped monster is destroyed and this card returns to its owner's hand. | Full | The CPU puts it on the opponent's monsters. The End Phase conditions were not fully traced. |
| 1549 | Miracle Dig | Spell | With more than 4 of your monsters banished: return up to three of them to the graveyard. | Full | Blocked by a face-up Banisher of the Light. |
| 1550 | Dragonic Attack | Equip Spell | Warriors only: the monster becomes a Dragon and gains 500 ATK/DEF. | Full |  |
| 1551 | Spirit Elimination | Spell | Sets a flag for the turn under which the banish costs of 1514-1519 come from your own field instead of the graveyard (hypothesis from the readers of the flag). | Full | The card behind it is unknown. |
| 1552 | Vengeful Bog Spirit | Continuous Spell | While it is on either field, a monster summoned face up cannot attack that turn. | Full (passive) | Summon code; resolve is a no-op. |

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
- Identification: the behaviour traced in the code was matched against every card text in the YGOPRODeck database (the OCG cards released before mid-2003), using the release-set order of the numbers to separate close candidates. The game's own recipe tables confirm the Fusions (for example 1334 = Relinquished + 1333, the recipe of Thousand-Eyes Restrict) and the ritual (1416 summons 1411 with level-8 Tributes, as Curse of the Masked Beast does for The Masked Beast). Where two numbers behave alike, the code decides: only 1326 cannot be chosen as an attack target, which is The Legendary Fisherman's text, so 1329 is Deepsea Warrior. 1410 and 1412 (the two Tributes Masked Beast Des Gardius names) and the order of the four Spirit Message letters are interchangeable in the code and were assigned in list order. 230 has no effect code to match.
- Where a card text says more than the code does, the extra part is either noted as missing (1421, 1422) or the effect is described as coded.
- Numbers tested only through computed values (not a constant or a literal in the source) would not have been found. None are known.
