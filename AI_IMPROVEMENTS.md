# CPU AI improvements

Campaign duelists in Yak's YGO Hack use a much stronger opponent AI than vanilla *The Eternal Duelist Soul*. Most of the behavior is implemented in `src/ai_*.c` and `src/added_effects.c`.

## Battle and positions

- It switches a monster that can't beat any of yours to Defense Position when you have a face-up monster with higher ATK, instead of leaving it to be run over. A monster with 1700 ATK or more stays in Attack Position unless your strongest monster has at least 1000 more ATK than it.
- When it chooses Attack or Defense Position, it counts the ATK a monster will gain from an Equip Spell in its hand, Breaker's Spell Counter, and Blade Knight's bonus once its hand shrinks.
- It summons a monster with 1400 or more ATK in Attack Position instead of Setting it (unless the monster has a flip effect, or it would be run over: below 1700 ATK that is any face-up monster with higher ATK; at 1700+ ATK only when your strongest face-up monster has at least 1000 more ATK), even on the first turn.
- It attacks your face-down Defense Position monsters with any monster of 1400 ATK or more, even though it can't see their DEF (it guesses 1400).
- It crashes into your monsters with equal ATK when it has no better attack, unless another of its monsters can beat yours outright.
- It Tribute Summons a stronger monster over weaker ones even when the swap doesn't change its attacks, such as Gyakutenno Megami (1800/2000) over Aqua Madoor (1200/2000), as long as the summon has more ATK and at least as much DEF as each monster it Tributes.
- When it plans to attack, it compares no-tribute and tribute summons and won't pick a weaker printed ATK beatstick over a stronger one unless the weaker card has a Normal Summon trigger and still beats your monsters; on a tie it prefers that trigger over a higher-ATK beatstick (flip monsters are Set, not in this rule).
- While your Swords of Revealing Light is up, it skips Change of Heart (it cannot attack this turn anyway). If you control no monsters, it still Normal Summons for next turn, but not beatsticks under 1000 ATK meant for a direct attack; Tribute Summons and tribute upgrades still go through. If you control any monsters, it skips attack summons and goes straight to Spells.
- It never Tributes a copy of the monster it is summoning, for a Tribute Summon or a Toon's Special Summon, unless that monster has an effect that triggers on its summon (Zaborg, Mobius, Breaker, ...).
- It does not Normal Summon or Set **Injection Fairy Lily** from its hand when it has 2000 LP or less (it cannot pay the battle effect cost).
- When it can **Tribute Summon** (level 5+ in hand, enough tributes, normal summon not blocked), it **Flip Summons** its face-down **flip-effect** monsters before tributing them, one per step, except flips that would leave the field or wipe the board (**Parasite Paracide**, **Cyber Jar**, **Morphing Jar #2**, **Jigen Bakudan**, **Fiber Jar** when coded).
- In its position-change step, it **Flip Summons** face-down **Hane-Hane** or **Penguin Soldier** when a simulated Battle Phase would deal more damage after their effects return your strongest monster(s) to the hand (the same targets it would pick in play).
- In that same step, it **Flip Summons** face-down **Reaper of the Cards** when any face-up **Trap** is on the field (even if your strongest face-up monster has more than **1380** ATK), or when your strongest face-up monster has at most **1380** ATK and it has a Set card in its Spell/Trap zones.
- It still **Flip Summons** **Man-Eater Bug** in that step when you have monsters and it has its Normal Summon available (vanilla behavior).

## Card use

- At the start of each Main Phase (before heal/burn Spells, Thunder Dragon, summons, and its other spell logic), it activates **Pot of Greed** and then **Graceful Charity** from its hand when it can.
- It plays its Spells after its Normal Summon, in both Main Phases. The original CPU skipped its Spells on any turn it summoned to attack (always, with your field empty), after a Tribute Summon, and in Main Phase 2.
- It uses Thunder Dragon at the start of its Main Phase. It discards one to add a single copy, then discards the new copy for the last one, then discards that one too, finding nothing. All three end up in its Graveyard. A lone copy, with none left in its Deck and not just added, stays in its hand.
- It plays Nobleman of Crossout on your face-down monsters (the original CPU never used it).
- It uses Solemn Judgment (the original CPU never did) while it has 4000 LP or less. It negates your Normal and Special Summons, your Flip Summons of flip-effect monsters, and your Spells and Traps when it has no cheaper answer.
- It chains Trap Hole and its other summon Traps to your monster's summon effect (Mobius, Zaborg, ...), so it can destroy the monster before the effect resolves.
- It plays Dark Hole only when you control at least as many monsters as it does (not when you have fewer), and tries to do so before it Normal or Special Summons a monster (same conditions as its later spell pass).
- It plays Snatch Steal in Main Phase 1 for lethal (even with a level 1-4 monster in hand), to set up a tribute summon (same hand checks as Change of Heart), or when the stolen ATK matches or beats its strongest attacker (or its highest DEF if it has no Attack Position monsters). It targets your strongest legal face-up monster (not Gearfried).
- It activates set Waboku on your turn when it still has monsters, when your attack would be lethal (even with no monsters left), or when chaining to Heavy Storm, Harpie's Feather Duster, or Giant Trunade.
- Its recruiters (Mystic Tomato, Giant Rat, UFO Turtle, Shining Fairy, Mother Grizzly, Flying Kamakiri #1, Pyramid Turtle, Apprentice Magician) summon another copy of themselves from the Deck when they can, so the next one can recruit again.
- Its Mystical Space Typhoon (and Gust, Driving Snow) also targets your face-up Traps, and never its own cards.
- It uses Magical Scientist with Catapult Turtle, Cannon Soldier, or Toon Cannon Soldier: it summons a Fusion Monster, then Tributes it for damage, and repeats while it has more than 1000 LP. It skips that tribute burn while you control a face-up **Des Wombat** (effect damage would be 0). Without any of those, it summons one Fusion Monster a turn in Main Phase 1 (with 4000 LP or more) when it could destroy one of your face-up monsters. Its burn monsters Tribute a Fusion Monster from Magical Scientist first, since it leaves at the End Phase anyway. It never fires Scientist's effect from the generic monster scan; only its dedicated Main Phase logic does.
- In its Toon World strategy it Normal Summons Toon Cannon Soldier from the hand (other Toons still Special Summon).
- It attacks your face-down Defense Position monsters with Sasuke Samurai and Mystic Swordsman LV2 (and any Defense Position monster with Ninja Grandmaster Sasuke), since their effects destroy the target whatever its DEF.
- It uses Chaos Emperor Dragon's effect when the damage would win, or when you have at least 3 more cards in hand and on the field than it does.
- Its Kycoo the Ghost Destroyer banishes the two strongest monsters in your Graveyard by itself (before the fix, you were asked to pick them).
- It doesn't summon Thunder Nyan Nyan next to its own face-up non-LIGHT monsters, or a non-LIGHT monster next to it, when that would just destroy Nyan Nyan.
- It plays Megamorph in Main Phase 1 only when its LP are lower than yours and equipping it makes a simulated Battle Phase lethal (not when it would already win without Megamorph).
- It plays **Scapegoat** in Main Phase 1 when the tribute-burn setup is legal: four free monster zones and it has not summoned yet—for a face-up **Cannon Soldier** alone on the field, or when it has no monster it would Normal or Special Summon that turn and either you have monsters while it has none or **Metamorphosis** could use Sheep Tokens (see below). A Set Scapegoat chains only when you declare an attack, not during its Main Phase on an empty field.
- It plays **Metamorphosis** only when it can Tribute a Level 1 monster (for example a Sheep Token), its Extra Deck has **Thousand-Eyes Restrict** at that Level, and you have a face-up monster with 1500 or more ATK for Restrict to absorb. It summons Restrict from the Metamorphosis list (not another Fusion) and activates Restrict’s effect when that target exists. It never Tributes higher-Level monsters for Metamorphosis.
- It plays Enemy Controller in Main Phase 1 to Tribute its weakest monster and take your strongest face-up one, when that has at least 500 more ATK.
- Its Poison of the Old Man deals 800 damage when that wins the duel, gains 1200 LP while it has 3000 or less, and otherwise deals the damage; while you control a face-up **Des Wombat**, it always picks the LP gain.
- It chains Emergency Provisions to your Heavy Storm, Harpie's Feather Duster or a card targeting its Spells and Traps, turning the cards about to go into LP; from its hand it plays it only at 1500 LP or less.
- It chains Pikeru's Circle of Enchantment to your burn Spells and Traps that would deal it 500 or more damage (or finish it), before using Hallowed Life Barrier for that, unless you already take no effect damage (for example a face-up **Des Wombat** on your field).
- It does not play its own effect-damage Spells and Traps (Sparks, Hinotama, Tremendous Fire, Secret Barrel, Wave-Motion Cannon ignition, and similar) while you control a face-up **Des Wombat**—only that continuous effect, not Pikeru's Circle or Hallowed Life Barrier on their own.
- It chains **Divine Wrath** to your activated monster effects (not plain summons), discarding a card to negate and destroy the monster; it answers Mobius, Breaker, Exiled Force, Magical Scientist, Tribe-Infecting Virus, Spirit Reaper, and similar high-impact effects, or any effect when it has three or more cards to discard. It Sets Divine Wrath from the hand when you have a wide board (five or more hand cards or any monster on your field).
- It plays **Mask of Restrict** in Main Phase when you have tribute plays it wants to stop, or more tributable monsters than it does; it Sets the trap whenever it can.
- It plays **Limiter Removal** when it controls at least one Machine monster (doubles their ATK until End Phase, then destroys those Machines).
- It activates Hallowed Life Barrier against an attack that would deal it 1500 or more damage, and in answer to your burn Spells and Traps (Ookazi, Tremendous Fire, ...) that would deal 800 or more; either way also when the damage would finish it.
- It Special Summons Gilasaurus when your best monster to revive has less than 1500 ATK (or you have none), or as a Tribute for a Level 5-6 monster it couldn't summon otherwise. When you Special Summon Gilasaurus, it always revives a monster if it can.
- It counts Cipher Soldier's +2000 against Warriors when it predicts a battle, for its own copy and for yours.
- Its Set Chain Disappearance goes off on any summon of yours that it can answer.
- It flips Threatening Roar as your Battle Phase begins when you have a face-up monster with 2000 or more ATK, and otherwise uses it against an attack when you have at least 2 monsters.
- When your Spell or Trap would destroy its Set Blast with Chain (a targeted removal, Heavy Storm or Harpie's Feather Duster), it chains Blast with Chain onto one of its face-up monsters first, so the destruction pops it. Against an attack it equips Blast with Chain to one of its face-up monsters.
- It answers your Spells with Magic Drain before Solemn Judgment, since Magic Drain costs nothing.
- It activates Ojama Trio only when you have three empty monster zones.
- It activates Secret Barrel in the Main Phase (or in a chain) when the damage would win, or when it would deal 600 or more (200 per card in your hand and on your field), unless you control a face-up **Des Wombat**.
- It plays Mystik Wok at 6000 LP or less, when the weakest monster it can Tribute has 1500 or more ATK or DEF, and gains whichever is higher.
- It plays Monster Reincarnation only with an empty monster field and a monster in its Graveyard to add; the graveyard pick favors a monster that can beat your strongest field monster.
- It plays Toon Table of Contents only when its Deck still has a "Toon" card to search.
- It plays Token Thanksgiving when any token is on the field (yours or its).
- It plays Card Destruction when you hold more cards in hand than it would discard after paying the cost.
- It plays Creature Swap when it can trade its weakest monster for one of yours with higher ATK.
- It plays Monster Gate when it can Tribute a monster with less than 1500 ATK.
- It plays Book of Moon when you have a face-up monster and it has a monster on the field.
- It never activates Hysteric Fairy's effect (two Tributes for 1000 LP).
- It plays Cold Wave in its Main Phase and does not keep playing Spells or Traps while either player's Cold Wave lock is active.
- While **Imperial Order** is face-up on either field, it does not activate Magic cards from its hand (including in its early heal-and-burn Spell step); it Sets them instead, like under Anti-Magic Fragrance.
- On its Standby Phase it activates face-up monsters with Standby effects on its own turn (Dark Mimic LV1's level-up, and the original game's Jigen Bakudan and similar cards), without waiting for you to move the cursor.
- Its Solemn Wishes is Set like other continuous Traps; while face-up, each time it draws it gains 500 LP per copy on its field (same rule as vanilla effect key 1305).
- Its Chiron the Mage discards a Spell to destroy one of your Spells or Traps when it can, once per turn.
- Its Bazoo the Soul-Eater activates once per turn when it has monsters in its Graveyard, banishing its three weakest (by ATK) for +300 ATK each until the end of your turn.
- Its Golem Sentry and Guardian Sphinx use their once-per-turn face-down effect in the Main Phase when you control a monster, so their next Flip Summon can bounce yours.
- Its Stealth Bird uses its once-per-turn face-down effect in the Main Phase whenever the menu allows (including while manual position changes are locked); its FLIP burn still triggers on Flip Summon and battle flips.
- At the End Phase, its Manticore of Darkness always Special Summons itself from the Graveyard when it was sent there this turn and it can pay the cost: it sends its weakest Beast, Beast-Warrior, or Winged Beast from its field or hand to the Graveyard.
- D.D. Survivor is mandatory: if a face-up copy is banished from the field or the Graveyard (including as Bazoo or a Chaos summon cost), it Special Summons itself in that End Phase when there is a free zone; the CPU picks Attack or Defense like any other Special Summon.
- It Sets Wave-Motion Cannon from its hand and later ignites it after at least two of its Standby Phases have passed, or sooner when the burn would win the duel, but not while you control a face-up **Des Wombat**.
- It pays Exarion Universe's ATK cost for piercing damage when its ATK minus 400 still beats your Defense Position monster's DEF.
- Its Roulette Barrel keeps the die that matches the Level of your strongest face-up monster, so it destroys the best target it can.
- It plays Level Limit - Area B, Gravity Bind, Final Attack Orders and Royal Oppression in its Main Phase. It uses Royal Oppression against your Special Summons while it has more than 800 LP.
- Its Dark Scorpion - Cliff the Trap Remover picks its Spell or Trap target like Mystical Space Typhoon does.
- When you attack into its flip monsters that remove or stop a monster (Man-Eater Bug, Hane-Hane, Penguin Soldier, Dark-Eyes Illusionist, Night Assailant, Tsukuyomi), it usually targets your strongest monster that hasn't attacked yet, so it can't attack this turn. In the **Yugi-Kaiba** banlist, **Man-Eater Bug** always picks **Summoned Skull** when you have one on the field, even if Skull already attacked (for example it battled into Man-Eater Bug), unless another Attack Position monster that has not attacked yet can inflict lethal battle damage.
- It never activates a card whose LP cost or self-damage would drop its own LP to 0 (Premature Burial, Cyber Stein, Tremendous Fire, a lost Time Wizard toss, ...).

## What it can see

It no longer knows the ATK or DEF of your face-down monsters. It assumes 1400 for both when deciding whether to attack one, judging how threatening your field is, and choosing targets for its effects.
