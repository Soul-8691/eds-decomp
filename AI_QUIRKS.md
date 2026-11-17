# Vanilla CPU AI quirks, and suggested behaviors

*The Eternal Duelist Soul* shipped with a very limited duel CPU. Each table below sets what the **original game's** CPU
did beside a **suggested behavior** for it. This hack's Campaign CPU follows these suggestions (in `src/ai_*.c` and
`src/added_effects.c`); [AI_IMPROVEMENTS.md](AI_IMPROVEMENTS.md) describes its behavior in full.

Only **original-roster** cards are named (the base 820-card USA set). Cards added by this project are left out even when
their AI was also improved; [AI_IMPROVEMENTS.md](AI_IMPROVEMENTS.md) covers them.

## Battle, summons, and positions

| The original CPU | Suggested behavior |
|---|---|
| Left monsters in **Attack Position** when they could not beat any of your face-up monsters and your strongest face-up monster had higher ATK. | Switch such a monster to Defense Position; keep one with 1700+ ATK in Attack unless the opponent's strongest has at least 1000 more ATK. |
| Did not count **Equip Spells** in its hand, or other hand- and field-based ATK changes, when choosing Attack or Defense Position. | Count the ATK an Equip Spell in hand (and similar boosts) will add. |
| **Set** or placed in **Defense Position** monsters with **1400+ ATK** that should have been summoned in Attack, even on the first turn. | Summon them in Attack Position, unless the monster has a flip effect or would be run over. |
| Would not **attack face-down Defense Position** monsters with **1400+ ATK**, though it could not see their DEF. | Attack them with any monster of 1400 ATK or more (guessing 1400 DEF). |
| Would not **crash into equal-ATK** Attack Position monsters when it had no better attack. | Crash into them, unless another monster can beat the opponent's outright. |
| Would not **Tribute Summon** a stronger monster over weaker ones when the swap did not change its attack plan. | Make the swap when the summon has more ATK and at least as much DEF as each Tribute. |
| Between a no-tribute and a tribute summon, could pick a **weaker beatstick** over a stronger one. | Pick the stronger printed ATK, unless the weaker card has a Normal Summon trigger and still beats the opponent's monsters. |
| While your **Swords of Revealing Light** was up and you had **no monsters**, often **skipped its Normal Summon** (tribute lines included). | Still summon for next turn (not low-ATK beatsticks meant for a direct attack), and skip Change of Heart under Swords. |
| Could **Tribute a copy of the card** it was Tribute Summoning (or a Toon's Special Summon), even with no on-summon effect. | Never Tribute a copy, unless that monster has a summon trigger. |
| Would **Normal Summon or Set** monsters whose effects would **drop its LP to 0** (**Premature Burial**, **Cyber Stein**, **Tremendous Fire**, a lost **Time Wizard** toss). | Never activate a card whose LP cost or self-damage would drop its own LP to 0. |
| With **Change of Heart** in hand, Tribute Summoned with its own monsters, or with none summoned a weaker monster first; the steal came later, in its Spell step. | Take one of the opponent's monsters first (Change of Heart, Snatch Steal, ...) and Tribute it for the summon. |
| Did not **Flip Summon** its face-down flip monsters before Tributing them, even when safe (it did treat **Parasite Paracide**, **Cyber Jar**, **Morphing Jar #2** and **Jigen Bakudan** as dangerous). | Flip the safe ones first, one per step, before a Tribute Summon. |
| Did not **Flip Summon** **Hane-Hane** or **Penguin Soldier** when bouncing your strongest monster(s) would gain battle damage. | Flip them when a simulated Battle Phase deals more damage after the bounce. |
| Did not **Flip Summon** **Reaper of the Cards** when a face-up Trap was on the field, or when your strongest face-up monster was weak and you had Set back-row cards. | Flip it in both cases. |
| When you attacked its **Man-Eater Bug**, **Hane-Hane**, **Penguin Soldier** or **Dark-Eyes Illusionist**, picked a **weak target**. | Target the opponent's strongest monster that has not attacked yet (Man-Eater Bug: Summoned Skull first in the Yugi-Kaiba banlist). |
| **Relinquished** could absorb one of your **face-down** monsters. | Absorb only a face-up monster, the one with the highest printed ATK. |

## What the CPU could see

| The original CPU | Suggested behavior |
|---|---|
| Treated your **face-down** monsters as if it knew their **true ATK and DEF** when deciding attacks, threat level, and many effect targets. | Assume **1400** for both. |

## Main Phase order and Spells

| The original CPU | Suggested behavior |
|---|---|
| Did not open the Main Phase with **Pot of Greed**, **Graceful Charity** or **Upstart Goblin**; it could play them after its other cards. | Play them first, before any summon or other card, then try what they drew. |
| **Skipped its Spell/Trap plays** on turns it summoned to attack (including with your field empty), **after Tribute Summons**, and in **Main Phase 2**. | Play Spells after the Normal Summon, in both Main Phases. |
| Never used **Thunder Dragon**'s discard-to-search effect. | Use it at the start of the Main Phase, emptying duplicate searches into the Graveyard. |
| Never played **Nobleman of Crossout** on your face-down monsters. | Play it on them. |
| Played **Dark Hole** when you controlled **fewer monsters** than it did. | Play it only when the opponent controls at least as many monsters, and before summoning. |
| Played a **second Swords of Revealing Light** while its first was still up. | Wait until the first runs out. |
| Played **Megamorph** only in its Cyber Stein strategy, on fixed LP checks. | Play it in Main Phase 1 when its LP are lower than the opponent's and Megamorph makes a simulated Battle Phase lethal. |
| **Mystical Space Typhoon**, **Gust** and **Driving Snow** did not reliably **target your face-up Traps** and could **hit its own** cards. | Target the opponent's face-up Traps too, and never its own cards. |
| Its **recruiters** (**Mystic Tomato**, **Giant Rat**, **UFO Turtle**, **Shining Angel**, **Mother Grizzly**, **Flying Kamakiri #1**, ...) did not **summon another copy of themselves**. | Summon another copy from the Deck when possible, so the next one recruits again. |
| While **Imperial Order** was face-up, still **activated Magic cards from its hand**. | Set them instead, as under Anti-Magic Fragrance. |
| On its own **Standby Phase**, waited for **your cursor** before effects such as **Jigen Bakudan**'s. | Activate them without waiting for the human. |
| **Sonic Bird** and **Senju of the Thousand Hands** showed **you** its Yes/No search question. | Search without asking the human: the Ritual card that matches one in hand, else the first. |

## Traps, chains, and Set cards

| The original CPU | Suggested behavior |
|---|---|
| Never activated **Solemn Judgment**. | Use it at 4000 LP or less on summons, flips of flip-effect monsters, and Spells/Traps with no cheaper answer. |
| Did not **chain Trap Hole** (and similar summon Traps) to **on-summon monster effects**. | Chain them, destroying the monster before its effect resolves. |
| Never played **Snatch Steal** from the hand. | Play it for lethal, to set up a Tribute Summon, or to take a monster as strong as its best attacker. |
| Did not activate **Waboku** on your turn when an attack would hurt, while it still had monsters, or in answer to **Heavy Storm**, **Harpie's Feather Duster** or **Giant Trunade**. | Activate it in all of those. |
| Did not **chain Magic Drain** ahead of **Solemn Judgment**, though Drain is free. | Answer Spells with Magic Drain first. |

## Other behavior (often involving added cards in this hack)

Many fixes target cards that are **not** in the original 820 (Goat-era and later imports). Those improvements are real in
this ROM and listed in [AI_IMPROVEMENTS.md](AI_IMPROVEMENTS.md), but are intentionally **not** catalogued here. Examples
of the *kind* of vanilla gap they fill:

- Never playing strong Spells and Traps that were absent from its hard-coded play lists.
- Wrong timing for board wipes, burn, equip, flip, and End Phase effects.
- Bad tribute, fusion, and Special Summon lines for combo decks.
- Making the human pick targets or modes the CPU should choose itself.
- Ignoring locks and continuous effects that change what it may activate.
