# Vanilla CPU AI quirks (fixed)

*The Eternal Duelist Soul* shipped with a very limited duel CPU. This hack replaces much of that behavior in `src/ai_*.c` and related logic. Below is what was wrong in the **original game’s** AI—not how it was fixed.

Only **original-roster** cards are named (the base 820-card USA set). Cards added by this project are omitted even when their AI was also improved.

## Battle, summons, and positions

- Left monsters in **Attack Position** when they could not beat any of your face-up monsters and your strongest face-up monster had higher ATK.
- Did not count **Equip Spells** in hand, **Breaker the Dark Destruction**’s Spell Counter, or other hand- and field-based ATK changes when choosing Attack vs Defense Position.
- **Set** or placed in **Defense Position** monsters with **1400+ ATK** that should have been summoned in Attack (including on the first turn), except where flip effects or a clear run-over threat applied.
- Would not **attack face-down Defense Position** monsters with **1400+ ATK**, even though it could not see their DEF.
- Would not **crash into equal-ATK** Attack Position monsters when it had no better attack, even when no other of its monsters could beat yours outright.
- Would not **Tribute Summon** a stronger monster over weaker ones when the swap did not change its attack plan (same DEF was enough to skip the play).
- When choosing between a no-tribute and a tribute summon, could pick a **weaker beatstick** over a stronger one despite a weaker attack plan (except where a Normal Summon trigger still won).
- While your **Swords of Revealing Light** was active and you controlled **no monsters**, it often **skipped Normal Summons** (including tribute lines) and went straight to other actions.
- Could **Tribute a copy of the same card** it was Tribute Summoning (or tributing for a Toon Special Summon), including when that card had no on-summon effect.
- Would **Normal Summon or Set** monsters whose activated effects would **reduce its LP to 0** (for example **Premature Burial** upkeep, **Cyber Stein**’s cost, **Tremendous Fire**, or a lost **Time Wizard** gamble).
- In the position-change step, did not **Flip Summon** face-down flip monsters before tributing them when that was safe (it treated dangerous flips like **Parasite Paracide**, **Cyber Jar**, **Morphing Jar #2**, and **Jigen Bakudan** correctly as exceptions).
- Did not **Flip Summon** **Hane-Hane** or **Penguin Soldier** when a simulated Battle Phase would gain more damage by bouncing your strongest monster(s).
- Did not **Flip Summon** **Reaper of the Cards** when a face-up Trap was on the field, or when your strongest face-up monster was weak enough and you had Set back-row cards.
- When you attacked its flip monsters that remove or stop a monster (**Man-Eater Bug**, **Hane-Hane**, **Penguin Soldier**, **Dark-Eyes Illusionist**), it picked a **weak target** instead of your **strongest monster that had not attacked yet** (so that monster could still attack).

## What the CPU could see

- Treated your **face-down** monsters as if it knew their **true ATK and DEF** when deciding attacks, threat level, and many effect targets. It now assumes **1400** for both.

## Main Phase order and Spells

- Did not open the Main Phase by activating **Pot of Greed** and **Graceful Charity** when legal.
- **Skipped its Spell/Trap plays** on turns it summoned to attack (including with your field empty), **after Tribute Summons**, and in **Main Phase 2**.
- Never used **Thunder Dragon**’s discard-to-search effect at the start of the Main Phase (including emptying duplicate searches into the Graveyard when it held multiple copies).
- Never played **Nobleman of Crossout** on your face-down monsters.
- Played **Dark Hole** when you controlled **fewer monsters** than it did (instead of only when counts were at least equal).
- **Mystical Space Typhoon**, **Gust**, and **Driving Snow** did not reliably **target your face-up Traps** and could **hit its own** cards.
- **Recruitment** monsters (**Mystic Tomato**, **Giant Rat**, **UFO Turtle**, **Shining Fairy**, **Mother Grizzly**, **Flying Kamakiri #1**, and similar) did not **Special Summon another copy of themselves** from the Deck to chain further recruits.

## Traps, chains, and set cards

- Never activated **Solemn Judgment** (neither from the hand nor from Set cards), so it did not negate summons, flips, or Spells/Traps when that was correct.
- Did not **chain Trap Hole** (and similar summon traps) to **on-summon monster effects** so the monster could be destroyed before the effect resolved.
- **Snatch Steal** was never played from the hand despite strong situations (lethal, tribute setup, or stealing your best attacker).
- **Waboku** was not activated on your turn when an attack would be lethal, when it still had monsters, or when chaining to **Heavy Storm**, **Harpie’s Feather Duster**, or **Giant Trunade**.
- Did not **chain Magic Drain** ahead of **Solemn Judgment** when Drain was free.

## Other behavior (often involving added cards in this hack)

Many fixes target cards that are **not** in the original 820 (goat-era and later imports). Those improvements are real in this ROM but are intentionally **not** catalogued here. Examples of the *kind* of vanilla gap they fill:

- Never playing strong Spells and Traps that were absent from its hard-coded play lists.
- Wrong timing for board wipes, burn, equip, flip, and End Phase effects.
- Bad tribute, fusion, and Special Summon lines for combo decks.
- Making the human pick targets or modes the CPU should choose itself.
- Ignoring locks (**Cold Wave**, **Imperial Order**) and continuous effects that change what it may activate.

**Imperial Order** (original roster): while it was face-up, the CPU still **activated Magic cards from its hand** instead of Setting them.

**Standby effects** on the CPU’s own turn: it waited for **your cursor** instead of resolving effects such as **Dark Mimic LV1**’s level-up and **Jigen Bakudan** on its Standby Phase.

## See also

[AI_IMPROVEMENTS.md](AI_IMPROVEMENTS.md) describes what the Campaign CPU does **now** (behavior after these fixes and other additions).
