# Booster packs and card prices

TCG set packs and usage-weighted format packs for the main-menu **Booster Packs** shop (and the free pick after a Campaign win). Generated documentation; do not edit the marked sections by hand.

Regenerate when data changes:

- `python3 tools/pack_chances_md.py` — new cards, new `packs/*_tcg.json` entries, or rarity edits
- `python3 tools/pack_prices_md.py` — `usage/*.json` or `banlists/*_ban.json` changes
- `python3 tools/packs_md.py` — both sections

Build-time sources: `tools/format_packs.py` → `build/include/format_pack_data.h`; `tools/cardprices.py` → `build/include/card_prices.h`.

## How pulls work

Each opened pack gives **five cards**. For format packs (IDs 601+), `ModGenerateFormatPackCards` (`src/mod_format_packs.c`) picks five **independent** indices into a fixed **8192-entry roulette** (built at compile time with a per-pool seed). Each index maps to one card number; weights match the tables below. The same card is not duplicated in one pack (up to 64 re-rolls per slot). A final shuffle permutes the five slots.

**Single-card probability** (what the tables list): `weight / sum(weights)` for that pack's pool. With five draws without replacement, a card's chance to appear **at least once** in a pack is slightly higher than `1 - (1 - p)^5` would suggest for independent trials; for rare cards the table value is a good approximation.

Only card numbers present in the ROM are in a pool. Names resolve through `tools/banlist.py` (including aliases). TCG JSON names with no ROM match are skipped at build time.

Shop prices: **250 DP** per TCG set pack, **500 DP** per usage-weighted pack (`include/format_packs.h`). Vanilla campaign boosters stay **1000 DP**.

### TCG set packs (`packs/*_tcg.json`)

Each card name maps to a **rarity rank** (best token among its printings in that JSON). Ranks map to weights:

| Rank | Printings | Weight |
|---:|---|---:|
| 0 | Common | 10000 |
| 1 | Short Print / Super Short Print | 7000 |
| 2 | Rare | 2500 |
| 3 | Super Rare | 600 |
| 4 | Ultra Rare | 100 |
| 5 | Secret Rare | 25 |
| 6 | Ultimate Rare | 10 |

If one card number appears under several names or rarities, the build keeps the **lowest weight** (rarest). Pack cover art uses the card with the minimum weight in the pool (ties: lowest card number).

### Usage-weighted packs (one per Campaign format)

The pool is every card listed in `usage/<stem>.json` (main, side, or extra), not the full legal pool. For each card, share = decks running it / `decks_processed`. Score = `(share_main + share_extra/10) × 1000` × Limited (×3) or Semi-Limited (×1.5) from that format's banlist. Pull weight:

`weight = max(1, min(65535, round((1 - 120/10000) × 10000/score^0.4 + 120)))`

Forbidden cards (limit 0) are omitted. Lower usage ⇒ higher weight ⇒ more common in the pack.


<!-- pack-chances -->

## Pack pull rates

50 format packs (TCG sets interleaved with usage packs per Campaign format). Roulette size: 8192 entries per pool.

### Legend of Blue Eyes White Dragon

- **Type:** TCG · **Format:** Yugi-Kaiba · **Cards in pool:** 126 · **Weight sum:** 837550

| Card | Weight | Pull % |
|---|---:|---:|
| Armaill | 10000 | 1.1940 |
| Armored Starfish | 10000 | 1.1940 |
| Basic Insect | 10000 | 1.1940 |
| Beaver Warrior | 10000 | 1.1940 |
| Dark Gray | 10000 | 1.1940 |
| Dark King of the Abyss | 10000 | 1.1940 |
| Darkworld Thorns | 10000 | 1.1940 |
| Dissolverock | 10000 | 1.1940 |
| Drooling Lizard | 10000 | 1.1940 |
| Enchanting Mermaid | 10000 | 1.1940 |
| Fiend Reflection #2 | 10000 | 1.1940 |
| Firegrass | 10000 | 1.1940 |
| Fireyarou | 10000 | 1.1940 |
| Flame Manipulator | 10000 | 1.1940 |
| Forest | 10000 | 1.1940 |
| Frenzied Panda | 10000 | 1.1940 |
| Green Phantom King | 10000 | 1.1940 |
| Hard Armor | 10000 | 1.1940 |
| Hinotama | 10000 | 1.1940 |
| Hinotama Soul | 10000 | 1.1940 |
| Hitotsu-Me Giant | 10000 | 1.1940 |
| Kagemusha of the Blue Flame | 10000 | 1.1940 |
| King Fog | 10000 | 1.1940 |
| Kumootoko | 10000 | 1.1940 |
| Kurama | 10000 | 1.1940 |
| Larvas | 10000 | 1.1940 |
| Lesser Dragon | 10000 | 1.1940 |
| M-Warrior #1 | 10000 | 1.1940 |
| M-Warrior #2 | 10000 | 1.1940 |
| Mammoth Graveyard | 10000 | 1.1940 |
| Man Eater | 10000 | 1.1940 |
| Masaki the Legendary Swordsman | 10000 | 1.1940 |
| Meda Bat | 10000 | 1.1940 |
| Misairuzame | 10000 | 1.1940 |
| Monster Egg | 10000 | 1.1940 |
| Mountain | 10000 | 1.1940 |
| Mystical Sheep #2 | 10000 | 1.1940 |
| Nemuriko | 10000 | 1.1940 |
| One-Eyed Shield Dragon | 10000 | 1.1940 |
| Petit Angel | 10000 | 1.1940 |
| Petit Dragon | 10000 | 1.1940 |
| Ray & Temperature | 10000 | 1.1940 |
| Red Medicine | 10000 | 1.1940 |
| Remove Trap | 10000 | 1.1940 |
| Root Water | 10000 | 1.1940 |
| Sand Stone | 10000 | 1.1940 |
| Silver Fang | 10000 | 1.1940 |
| Skull Red Bird | 10000 | 1.1940 |
| Skull Servant | 10000 | 1.1940 |
| Sogen | 10000 | 1.1940 |
| Sparks | 10000 | 1.1940 |
| Spike Seadra | 10000 | 1.1940 |
| Steel Ogre Grotto #1 | 10000 | 1.1940 |
| Succubus Knight | 10000 | 1.1940 |
| Terra the Terrible | 10000 | 1.1940 |
| The 13th Grave | 10000 | 1.1940 |
| The Furious Sea King | 10000 | 1.1940 |
| Trial of Nightmare | 10000 | 1.1940 |
| Tripwire Beast | 10000 | 1.1940 |
| Turtle Tiger | 10000 | 1.1940 |
| Two-Mouth Darkruler | 10000 | 1.1940 |
| Tyhone | 10000 | 1.1940 |
| Umi  | 10000 | 1.1940 |
| Uraby | 10000 | 1.1940 |
| Wasteland | 10000 | 1.1940 |
| Witty Phantom | 10000 | 1.1940 |
| Yami | 10000 | 1.1940 |
| Beast Fangs | 7000 | 0.8358 |
| Book of Secret Arts | 7000 | 0.8358 |
| Dark Energy | 7000 | 0.8358 |
| Dragon Treasure | 7000 | 0.8358 |
| Electro-Whip | 7000 | 0.8358 |
| Follow Wind | 7000 | 0.8358 |
| Laser Cannon Armor | 7000 | 0.8358 |
| Legendary Sword | 7000 | 0.8358 |
| Machine Conversion Factory | 7000 | 0.8358 |
| Mystical Moon | 7000 | 0.8358 |
| Power of Kaishin | 7000 | 0.8358 |
| Raise Body Heat | 7000 | 0.8358 |
| Silver Bow and Arrow | 7000 | 0.8358 |
| Vile Germs | 7000 | 0.8358 |
| Violet Crystal | 7000 | 0.8358 |
| Aqua Madoor | 2500 | 0.2985 |
| Armed Ninja | 2500 | 0.2985 |
| Charubin the Fire Knight | 2500 | 0.2985 |
| Darkfire Dragon | 2500 | 0.2985 |
| Dragon Capture Jar | 2500 | 0.2985 |
| Dragoness the Wicked Knight | 2500 | 0.2985 |
| Final Flame | 2500 | 0.2985 |
| Fissure | 2500 | 0.2985 |
| Flame Ghost | 2500 | 0.2985 |
| Flower Wolf | 2500 | 0.2985 |
| Fusionist | 2500 | 0.2985 |
| Giant Soldier of Stone | 2500 | 0.2985 |
| Goblin's Secret Remedy | 2500 | 0.2985 |
| Gravedigger Ghoul | 2500 | 0.2985 |
| Hane-Hane | 2500 | 0.2985 |
| Karbonala Warrior | 2500 | 0.2985 |
| Metal Dragon | 2500 | 0.2985 |
| Pot of Greed | 2500 | 0.2985 |
| Reaper of the Cards | 2500 | 0.2985 |
| Spirit of the Harp | 2500 | 0.2985 |
| Stop Defense | 2500 | 0.2985 |
| Two-Pronged Attack | 2500 | 0.2985 |
| Celtic Guardian | 600 | 0.0716 |
| Curse of Dragon | 600 | 0.0716 |
| Dark Hole | 600 | 0.0716 |
| Flame Swordsman | 600 | 0.0716 |
| Man-Eater Bug | 600 | 0.0716 |
| Mystical Elf | 600 | 0.0716 |
| Polymerization | 600 | 0.0716 |
| Polymerization | 600 | 0.0716 |
| Raigeki | 600 | 0.0716 |
| Swords of Revealing Light | 600 | 0.0716 |
| Trap Hole | 600 | 0.0716 |
| Dark Magician | 100 | 0.0119 |
| Exodia the Forbidden One | 100 | 0.0119 |
| Gaia The Fierce Knight | 100 | 0.0119 |
| Left Arm of the Forbidden One | 100 | 0.0119 |
| Left Leg of the Forbidden One | 100 | 0.0119 |
| Monster Reborn | 100 | 0.0119 |
| Red-Eyes B. Dragon | 100 | 0.0119 |
| Right Arm of the Forbidden One | 100 | 0.0119 |
| Right Leg of the Forbidden One | 100 | 0.0119 |
| Gaia the Dragon Champion | 25 | 0.0030 |
| Tri-Horned Dragon | 25 | 0.0030 |

### Starter Deck- Yugi

- **Type:** TCG · **Format:** Yugi-Kaiba · **Cards in pool:** 47 · **Weight sum:** 441300

| Card | Weight | Pull % |
|---|---:|---:|
| Ancient Elf | 10000 | 2.2660 |
| Ansatsu | 10000 | 2.2660 |
| Beaver Warrior | 10000 | 2.2660 |
| Book of Secret Arts | 10000 | 2.2660 |
| Castle Walls | 10000 | 2.2660 |
| Celtic Guardian | 10000 | 2.2660 |
| Change of Heart | 10000 | 2.2660 |
| Claw Reacher | 10000 | 2.2660 |
| Curse of Dragon | 10000 | 2.2660 |
| Dark Hole | 10000 | 2.2660 |
| De-Spell | 10000 | 2.2660 |
| Dian Keto the Cure Master | 10000 | 2.2660 |
| Doma The Angel of Silence | 10000 | 2.2660 |
| Dragon Capture Jar | 10000 | 2.2660 |
| Dragon Zombie | 10000 | 2.2660 |
| Feral Imp | 10000 | 2.2660 |
| Fissure | 10000 | 2.2660 |
| Gaia The Fierce Knight | 10000 | 2.2660 |
| Giant Soldier of Stone | 10000 | 2.2660 |
| Great White | 10000 | 2.2660 |
| Last Will | 10000 | 2.2660 |
| Magical Ghost | 10000 | 2.2660 |
| Mammoth Graveyard | 10000 | 2.2660 |
| Man-Eater Bug | 10000 | 2.2660 |
| Man-Eating Treasure Chest | 10000 | 2.2660 |
| Monster Reborn | 10000 | 2.2660 |
| Mystical Elf | 10000 | 2.2660 |
| Neo the Magic Swordsman | 10000 | 2.2660 |
| Reinforcements | 10000 | 2.2660 |
| Remove Trap | 10000 | 2.2660 |
| Reverse Trap | 10000 | 2.2660 |
| Silver Fang | 10000 | 2.2660 |
| Summoned Skull | 10000 | 2.2660 |
| Sword of Dark Destruction | 10000 | 2.2660 |
| The Stern Mystic | 10000 | 2.2660 |
| Trap Hole | 10000 | 2.2660 |
| Trap Master | 10000 | 2.2660 |
| Two-Pronged Attack | 10000 | 2.2660 |
| Ultimate Offering | 10000 | 2.2660 |
| Waboku | 10000 | 2.2660 |
| Wall of Illusion | 10000 | 2.2660 |
| Winged Dragon,Guardian of the Fortress #1 | 10000 | 2.2660 |
| Witty Phantom | 10000 | 2.2660 |
| Yami | 10000 | 2.2660 |
| Card Destruction | 600 | 0.1360 |
| Soul Exchange | 600 | 0.1360 |
| Dark Magician | 100 | 0.0227 |

### Starter Deck- Kaiba

- **Type:** TCG · **Format:** Yugi-Kaiba · **Cards in pool:** 47 · **Weight sum:** 451200

| Card | Weight | Pull % |
|---|---:|---:|
| Ancient Telescope | 10000 | 2.2163 |
| Battle Ox | 10000 | 2.2163 |
| Castle Walls | 10000 | 2.2163 |
| D. Human | 10000 | 2.2163 |
| Dark Assailant | 10000 | 2.2163 |
| Dark Energy | 10000 | 2.2163 |
| Dark Hole | 10000 | 2.2163 |
| De-Spell | 10000 | 2.2163 |
| Destroyer Golem | 10000 | 2.2163 |
| Fissure | 10000 | 2.2163 |
| Gyakutenno Megami | 10000 | 2.2163 |
| Hane-Hane | 10000 | 2.2163 |
| Hitotsu-Me Giant | 10000 | 2.2163 |
| Invigoration | 10000 | 2.2163 |
| Judge Man | 10000 | 2.2163 |
| Just Desserts | 10000 | 2.2163 |
| Kojikocy | 10000 | 2.2163 |
| Koumori Dragon | 10000 | 2.2163 |
| La Jinn the Mystical Genie of the Lamp | 10000 | 2.2163 |
| Master & Expert | 10000 | 2.2163 |
| Monster Reborn | 10000 | 2.2163 |
| Mysterious Puppeteer | 10000 | 2.2163 |
| Mystic Horseman | 10000 | 2.2163 |
| Ogre of the Black Shadow | 10000 | 2.2163 |
| Ookazi | 10000 | 2.2163 |
| Pale Beast | 10000 | 2.2163 |
| Reinforcements | 10000 | 2.2163 |
| Remove Trap | 10000 | 2.2163 |
| Reverse Trap | 10000 | 2.2163 |
| Rogue Doll | 10000 | 2.2163 |
| Rude Kaiser | 10000 | 2.2163 |
| Ryu-Kishin | 10000 | 2.2163 |
| Ryu-Kishin Powered | 10000 | 2.2163 |
| Skull Red Bird | 10000 | 2.2163 |
| Sogen | 10000 | 2.2163 |
| Swordstalker | 10000 | 2.2163 |
| Terra the Terrible | 10000 | 2.2163 |
| The Inexperienced Spy | 10000 | 2.2163 |
| The Wicked Worm Beast | 10000 | 2.2163 |
| Trap Hole | 10000 | 2.2163 |
| Trap Master | 10000 | 2.2163 |
| Two-Pronged Attack | 10000 | 2.2163 |
| Ultimate Offering | 10000 | 2.2163 |
| Unknown Warrior of Fiend | 10000 | 2.2163 |
| Uraby | 10000 | 2.2163 |
| Lord of D. | 600 | 0.1330 |
| The Flute of Summoning Dragon | 600 | 0.1330 |

### Yugi-Kaiba

- **Type:** Usage · **Format:** Yugi-Kaiba · **Cards in pool:** 74 · **Weight sum:** 146491

| Card | Weight | Pull % |
|---|---:|---:|
| Destroyer Golem | 4393 | 2.9988 |
| Dragon Zombie | 4393 | 2.9988 |
| Koumori Dragon | 4393 | 2.9988 |
| Petit Dragon | 4393 | 2.9988 |
| Sparks | 4393 | 2.9988 |
| The Stern Mystic | 4393 | 2.9988 |
| Wasteland | 4393 | 2.9988 |
| Charubin the Fire Knight | 3967 | 2.7080 |
| Dian Keto the Cure Master | 3358 | 2.2923 |
| Forest | 3358 | 2.2923 |
| Invigoration | 3358 | 2.2923 |
| Ryu-Kishin Powered | 3358 | 2.2923 |
| Sword of Dark Destruction | 3358 | 2.2923 |
| Two-Pronged Attack | 3358 | 2.2923 |
| Exodia the Forbidden One | 2873 | 1.9612 |
| Left Arm of the Forbidden One | 2873 | 1.9612 |
| Left Leg of the Forbidden One | 2873 | 1.9612 |
| Right Arm of the Forbidden One | 2873 | 1.9612 |
| Right Leg of the Forbidden One | 2873 | 1.9612 |
| Flame Ghost | 2770 | 1.8909 |
| Karbonala Warrior | 2709 | 1.8493 |
| Castle Walls | 2574 | 1.7571 |
| Curse of Dragon | 2574 | 1.7571 |
| Darkfire Dragon | 2574 | 1.7571 |
| Lord of D. | 2574 | 1.7571 |
| The Flute of Summoning Dragon | 2574 | 1.7571 |
| Dragoness the Wicked Knight | 2527 | 1.7250 |
| Flame Swordsman | 2527 | 1.7250 |
| Fusionist | 2402 | 1.6397 |
| Gaia the Dragon Champion | 2402 | 1.6397 |
| Flower Wolf | 2297 | 1.5680 |
| Yami | 2207 | 1.5066 |
| Metal Dragon | 2193 | 1.4970 |
| Final Flame | 2082 | 1.4212 |
| Reverse Trap | 2082 | 1.4212 |
| Armed Ninja | 1894 | 1.2929 |
| Umi  | 1821 | 1.2431 |
| Reaper of the Cards | 1701 | 1.1612 |
| Dragon Capture Jar | 1607 | 1.0970 |
| Last Will | 1566 | 1.0690 |
| Great White | 1530 | 1.0444 |
| Spirit of the Harp | 1384 | 0.9448 |
| Gyakutenno Megami | 1361 | 0.9291 |
| Hane-Hane | 1361 | 0.9291 |
| Remove Trap | 1339 | 0.9140 |
| Mystical Elf | 1163 | 0.7939 |
| Stop Defense | 1151 | 0.7857 |
| Ookazi | 1052 | 0.7181 |
| The Wicked Worm Beast | 1028 | 0.7017 |
| Just Desserts | 1014 | 0.6922 |
| Aqua Madoor | 872 | 0.5953 |
| Judge Man | 872 | 0.5953 |
| Trap Master | 843 | 0.5755 |
| De-Spell | 826 | 0.5639 |
| Ultimate Offering | 803 | 0.5482 |
| Wall of Illusion | 777 | 0.5304 |
| Soul Exchange | 767 | 0.5236 |
| Summoned Skull | 758 | 0.5174 |
| Battle Ox | 754 | 0.5147 |
| Waboku | 750 | 0.5120 |
| Giant Soldier of Stone | 747 | 0.5099 |
| Neo the Magic Swordsman | 747 | 0.5099 |
| Reinforcements | 747 | 0.5099 |
| La Jinn the Mystical Genie of the Lamp | 745 | 0.5086 |
| Fissure | 743 | 0.5072 |
| Man-Eater Bug | 743 | 0.5072 |
| Trap Hole | 743 | 0.5072 |
| Card Destruction | 718 | 0.4901 |
| Swords of Revealing Light | 655 | 0.4471 |
| Change of Heart | 522 | 0.3563 |
| Dark Hole | 522 | 0.3563 |
| Monster Reborn | 522 | 0.3563 |
| Pot of Greed | 522 | 0.3563 |
| Raigeki | 522 | 0.3563 |

### Metal Raiders

- **Type:** TCG · **Format:** Critter · **Cards in pool:** 144 · **Weight sum:** 1020050

| Card | Weight | Pull % |
|---|---:|---:|
| 7 Colored Fish | 10000 | 0.9803 |
| Ancient Brain | 10000 | 0.9803 |
| Ancient Elf | 10000 | 0.9803 |
| Ancient Lizard Warrior | 10000 | 0.9803 |
| Armored Lizard | 10000 | 0.9803 |
| Armored Zombie | 10000 | 0.9803 |
| Battle Steer | 10000 | 0.9803 |
| Bickuribox | 10000 | 0.9803 |
| Big Eye | 10000 | 0.9803 |
| Blackland Fire Dragon | 10000 | 0.9803 |
| Blast Juggler | 10000 | 0.9803 |
| Block Attack | 10000 | 0.9803 |
| Blue-Winged Crown | 10000 | 0.9803 |
| Bottom Dweller | 10000 | 0.9803 |
| Castle of Dark Illusions | 10000 | 0.9803 |
| Crawling Dragon | 10000 | 0.9803 |
| Cyber Saurus | 10000 | 0.9803 |
| Deepsea Shark | 10000 | 0.9803 |
| Destroyer Golem | 10000 | 0.9803 |
| Disk Magician | 10000 | 0.9803 |
| Doma The Angel of Silence | 10000 | 0.9803 |
| Dragon Piper | 10000 | 0.9803 |
| Electric Lizard | 10000 | 0.9803 |
| Empress Judge | 10000 | 0.9803 |
| Feral Imp | 10000 | 0.9803 |
| Flame Cerebrus | 10000 | 0.9803 |
| Germ Infection | 10000 | 0.9803 |
| Giga-tech Wolf | 10000 | 0.9803 |
| Giltia the D. Knight | 10000 | 0.9803 |
| Ground Attacker Bugroth | 10000 | 0.9803 |
| Guardian of the Labyrinth | 10000 | 0.9803 |
| Harpie Lady | 10000 | 0.9803 |
| Hibikime | 10000 | 0.9803 |
| Hunter Spider | 10000 | 0.9803 |
| Hyosube | 10000 | 0.9803 |
| Illusionist Faceless Mage | 10000 | 0.9803 |
| Insect Soldiers of the Sky | 10000 | 0.9803 |
| Jellyfish | 10000 | 0.9803 |
| Jirai Gumo | 10000 | 0.9803 |
| Kaminari Attack | 10000 | 0.9803 |
| Killer Needle | 10000 | 0.9803 |
| King of Yamimakai | 10000 | 0.9803 |
| Kojikocy | 10000 | 0.9803 |
| Labyrinth Tank | 10000 | 0.9803 |
| Lady of Faith | 10000 | 0.9803 |
| Larvae Moth | 10000 | 0.9803 |
| Launcher Spider | 10000 | 0.9803 |
| Lava Battleguard | 10000 | 0.9803 |
| Leogun | 10000 | 0.9803 |
| Mega Thunderball | 10000 | 0.9803 |
| Morinphen | 10000 | 0.9803 |
| Mushroom Man #2 | 10000 | 0.9803 |
| Musician King | 10000 | 0.9803 |
| Mystic Horseman | 10000 | 0.9803 |
| Niwatori | 10000 | 0.9803 |
| Ocubeam | 10000 | 0.9803 |
| Pale Beast | 10000 | 0.9803 |
| Paralyzing Potion | 10000 | 0.9803 |
| Petit Moth | 10000 | 0.9803 |
| Prevent Rat | 10000 | 0.9803 |
| Protector of the Throne | 10000 | 0.9803 |
| Pumpking the King of Ghosts | 10000 | 0.9803 |
| Punished Eagle | 10000 | 0.9803 |
| Rabid Horseman | 10000 | 0.9803 |
| Ring of Magnetism | 10000 | 0.9803 |
| Roaring Ocean Snake | 10000 | 0.9803 |
| Rock Ogre Grotto #1 | 10000 | 0.9803 |
| Ryu-Kishin Powered | 10000 | 0.9803 |
| Saggi the Dark Clown | 10000 | 0.9803 |
| Share the Pain | 10000 | 0.9803 |
| Skull Knight | 10000 | 0.9803 |
| Soul Release | 10000 | 0.9803 |
| Steel Scorpion | 10000 | 0.9803 |
| Swamp Battleguard | 10000 | 0.9803 |
| Sword of Deep-Seated | 10000 | 0.9803 |
| Tainted Wisdom | 10000 | 0.9803 |
| The Bistro Butcher | 10000 | 0.9803 |
| The Cheerful Coffin | 10000 | 0.9803 |
| The Immortal of Thunder | 10000 | 0.9803 |
| The Little Swordsman of Aile | 10000 | 0.9803 |
| Tongyo | 10000 | 0.9803 |
| Tremendous Fire | 10000 | 0.9803 |
| Trent | 10000 | 0.9803 |
| Water Omotics | 10000 | 0.9803 |
| Winged Dragon,Guardian of the Fortress #1 | 10000 | 0.9803 |
| Yado Karu | 10000 | 0.9803 |
| Baby Dragon | 7000 | 0.6862 |
| Cocoon of Evolution | 7000 | 0.6862 |
| Crass Clown | 7000 | 0.6862 |
| Dream Clown | 7000 | 0.6862 |
| Gazelle the King of Mythical Beasts | 7000 | 0.6862 |
| Jinzo #7 | 7000 | 0.6862 |
| Leghul | 7000 | 0.6862 |
| Mystic Lamp | 7000 | 0.6862 |
| Ooguchi | 7000 | 0.6862 |
| Queen's Double | 7000 | 0.6862 |
| Rainbow Flower | 7000 | 0.6862 |
| Stim-Pack | 7000 | 0.6862 |
| The Unhappy Maiden | 7000 | 0.6862 |
| Thunder Dragon | 7000 | 0.6862 |
| Bladefly | 2500 | 0.2451 |
| Cannon Soldier | 2500 | 0.2451 |
| Dark Elf | 2500 | 0.2451 |
| Elegant Egotist | 2500 | 0.2451 |
| Fake Trap | 2500 | 0.2451 |
| Great Moth | 2500 | 0.2451 |
| Hoshiningen | 2500 | 0.2451 |
| Little Chimera | 2500 | 0.2451 |
| Magician of Faith | 2500 | 0.2451 |
| Mask of Darkness | 2500 | 0.2451 |
| Masked Sorcerer | 2500 | 0.2451 |
| Milus Radiant | 2500 | 0.2451 |
| Muka Muka | 2500 | 0.2451 |
| Princess of Tsurugi | 2500 | 0.2451 |
| Robbin' Goblin | 2500 | 0.2451 |
| Sangan | 2500 | 0.2451 |
| Shadow Ghoul | 2500 | 0.2451 |
| Shield & Sword | 2500 | 0.2451 |
| Star Boy | 2500 | 0.2451 |
| White Magical Hat | 2500 | 0.2451 |
| Witch of the Black Forest | 2500 | 0.2451 |
| Witch's Apprentice | 2500 | 0.2451 |
| Catapult Turtle | 600 | 0.0588 |
| Garnecia Elefantis | 600 | 0.0588 |
| Harpie Lady Sisters | 600 | 0.0588 |
| Heavy Storm | 600 | 0.0588 |
| Kazejin | 600 | 0.0588 |
| Kuriboh | 600 | 0.0588 |
| Sanga of the Thunder | 600 | 0.0588 |
| Suijin | 600 | 0.0588 |
| Tribute to The Doomed | 600 | 0.0588 |
| Twin-Headed Thunder Dragon | 600 | 0.0588 |
| B. Skull Dragon | 100 | 0.0098 |
| Barrel Dragon | 100 | 0.0098 |
| Change of Heart | 100 | 0.0098 |
| Horn of Heaven | 100 | 0.0098 |
| Magic Jammer | 100 | 0.0098 |
| Mirror Force | 100 | 0.0098 |
| Seven Tools of the Bandit | 100 | 0.0098 |
| Solemn Judgment | 100 | 0.0098 |
| Summoned Skull | 100 | 0.0098 |
| Time Wizard | 100 | 0.0098 |
| Gate Guardian | 25 | 0.0025 |
| Thousand Dragon | 25 | 0.0025 |

### Critter

- **Type:** Usage · **Format:** Critter · **Cards in pool:** 88 · **Weight sum:** 162886

| Card | Weight | Pull % |
|---|---:|---:|
| Flame Swordsman | 4568 | 2.8044 |
| Skull Knight | 4008 | 2.4606 |
| Flower Wolf | 3806 | 2.3366 |
| Metal Dragon | 3636 | 2.2322 |
| Catapult Turtle | 3491 | 2.1432 |
| Dream Clown | 3491 | 2.1432 |
| Ookazi | 3491 | 2.1432 |
| Remove Trap | 3491 | 2.1432 |
| Share the Pain | 3491 | 2.1432 |
| Soul Exchange | 3491 | 2.1432 |
| The Wicked Worm Beast | 3491 | 2.1432 |
| Giltia the D. Knight | 3365 | 2.0659 |
| Flame Ghost | 3254 | 1.9977 |
| Karbonala Warrior | 3254 | 1.9977 |
| Roaring Ocean Snake | 3155 | 1.9369 |
| Empress Judge | 3067 | 1.8829 |
| Deepsea Shark | 2913 | 1.7884 |
| Fusionist | 2913 | 1.7884 |
| Punished Eagle | 2913 | 1.7884 |
| Bickuribox | 2785 | 1.7098 |
| Cyber Saurus | 2785 | 1.7098 |
| Musician King | 2728 | 1.6748 |
| Darkfire Dragon | 2675 | 1.6423 |
| Dragoness the Wicked Knight | 2675 | 1.6423 |
| Shield & Sword | 2675 | 1.6423 |
| Stim-Pack | 2675 | 1.6423 |
| Stop Defense | 2675 | 1.6423 |
| Summoned Skull | 2675 | 1.6423 |
| Charubin the Fire Knight | 2625 | 1.6116 |
| Labyrinth Tank | 2457 | 1.5084 |
| Thousand Dragon | 2457 | 1.5084 |
| Giga-tech Wolf | 2292 | 1.4071 |
| Gaia the Dragon Champion | 2237 | 1.3734 |
| B. Skull Dragon | 2096 | 1.2868 |
| Jinzo #7 | 2056 | 1.2622 |
| Tribute to The Doomed | 2056 | 1.2622 |
| Twin-Headed Thunder Dragon | 2001 | 1.2285 |
| Crass Clown | 1891 | 1.1609 |
| Polymerization | 1891 | 1.1609 |
| Polymerization | 1891 | 1.1609 |
| Aqua Madoor | 1766 | 1.0842 |
| Dark Elf | 1766 | 1.0842 |
| Tremendous Fire | 1766 | 1.0842 |
| De-Spell | 1668 | 1.0240 |
| Princess of Tsurugi | 1668 | 1.0240 |
| Reinforcements | 1668 | 1.0240 |
| Mask of Darkness | 1587 | 0.9743 |
| Trap Master | 1587 | 0.9743 |
| Kuriboh | 1520 | 0.9332 |
| Horn of Heaven | 1412 | 0.8669 |
| Ultimate Offering | 1412 | 0.8669 |
| Magic Jammer | 1261 | 0.7742 |
| Fake Trap | 1158 | 0.7109 |
| Exodia the Forbidden One | 1022 | 0.6274 |
| Left Arm of the Forbidden One | 1022 | 0.6274 |
| Left Leg of the Forbidden One | 1022 | 0.6274 |
| Right Arm of the Forbidden One | 1022 | 0.6274 |
| Right Leg of the Forbidden One | 1022 | 0.6274 |
| Jirai Gumo | 985 | 0.6047 |
| 7 Colored Fish | 974 | 0.5980 |
| Wall of Illusion | 974 | 0.5980 |
| White Magical Hat | 952 | 0.5845 |
| Seven Tools of the Bandit | 855 | 0.5249 |
| Thunder Dragon | 814 | 0.4997 |
| Man-Eater Bug | 809 | 0.4967 |
| Last Will | 784 | 0.4813 |
| Robbin' Goblin | 775 | 0.4758 |
| Masked Sorcerer | 767 | 0.4709 |
| Muka Muka | 767 | 0.4709 |
| Solemn Judgment | 767 | 0.4709 |
| Giant Soldier of Stone | 763 | 0.4684 |
| La Jinn the Mystical Genie of the Lamp | 755 | 0.4635 |
| Magician of Faith | 751 | 0.4611 |
| Waboku | 751 | 0.4611 |
| Cannon Soldier | 747 | 0.4586 |
| Fissure | 743 | 0.4561 |
| Trap Hole | 743 | 0.4561 |
| Card Destruction | 729 | 0.4476 |
| Swords of Revealing Light | 663 | 0.4070 |
| Heavy Storm | 650 | 0.3991 |
| Sangan | 650 | 0.3991 |
| Witch of the Black Forest | 650 | 0.3991 |
| Change of Heart | 522 | 0.3205 |
| Dark Hole | 522 | 0.3205 |
| Mirror Force | 522 | 0.3205 |
| Monster Reborn | 522 | 0.3205 |
| Pot of Greed | 522 | 0.3205 |
| Raigeki | 522 | 0.3205 |

### Spell Ruler

- **Type:** TCG · **Format:** Treasure · **Cards in pool:** 103 · **Weight sum:** 655050

| Card | Weight | Pull % |
|---|---:|---:|
| Ancient One of the Deep Forest | 10000 | 1.5266 |
| Boar Soldier | 10000 | 1.5266 |
| Ceremonial Bell | 10000 | 1.5266 |
| Chain Energy | 10000 | 1.5266 |
| Chorus of Sanctuary | 10000 | 1.5266 |
| Commencement Dance | 10000 | 1.5266 |
| Crab Turtle | 10000 | 1.5266 |
| Dark Witch | 10000 | 1.5266 |
| Dark Zebra | 10000 | 1.5266 |
| Eatgaboon | 10000 | 1.5266 |
| Electric Snake | 10000 | 1.5266 |
| Final Destiny | 10000 | 1.5266 |
| Fire Kraken | 10000 | 1.5266 |
| Flash Assailant | 10000 | 1.5266 |
| Giant Turtle Who Feeds on Flames | 10000 | 1.5266 |
| Griggle | 10000 | 1.5266 |
| Guardian of the Throne Room | 10000 | 1.5266 |
| Hamburger Recipe | 10000 | 1.5266 |
| High Tide Gyojin | 10000 | 1.5266 |
| Horn of Light | 10000 | 1.5266 |
| House of Adhesive Tape | 10000 | 1.5266 |
| Hungry Burger | 10000 | 1.5266 |
| Kotodama | 10000 | 1.5266 |
| Labyrinth Wall | 10000 | 1.5266 |
| Liquid Beast | 10000 | 1.5266 |
| Magical Labyrinth | 10000 | 1.5266 |
| Malevolent Nuzzler | 10000 | 1.5266 |
| Mechanical Snail | 10000 | 1.5266 |
| Metal Fish | 10000 | 1.5266 |
| Minar | 10000 | 1.5266 |
| Mystic Plasma Zone | 10000 | 1.5266 |
| Octoberser | 10000 | 1.5266 |
| Peacock | 10000 | 1.5266 |
| Penguin Knight | 10000 | 1.5266 |
| Performance of Sword | 10000 | 1.5266 |
| Psychic Kappa | 10000 | 1.5266 |
| Queen Bird | 10000 | 1.5266 |
| Red Archery Girl | 10000 | 1.5266 |
| Ryu-Ran | 10000 | 1.5266 |
| Snake Fang | 10000 | 1.5266 |
| Sonic Bird | 10000 | 1.5266 |
| Spear Cretin | 10000 | 1.5266 |
| Stone Ogre Grotto | 10000 | 1.5266 |
| The Reliable Guardian | 10000 | 1.5266 |
| Turtle Oath | 10000 | 1.5266 |
| Twin Long Rods #2 | 10000 | 1.5266 |
| Tyhone #2 | 10000 | 1.5266 |
| Upstart Goblin | 10000 | 1.5266 |
| Wall Shadow | 10000 | 1.5266 |
| Weather Report | 10000 | 1.5266 |
| Whiptail Crow | 10000 | 1.5266 |
| Curse of Fiend | 7000 | 1.0686 |
| Darkness Approaches | 7000 | 1.0686 |
| Eternal Rest | 7000 | 1.0686 |
| Fairy's Hand Mirror | 7000 | 1.0686 |
| Gaia Power | 7000 | 1.0686 |
| Gravekeeper's Servant | 7000 | 1.0686 |
| Jigen Bakudan | 7000 | 1.0686 |
| Luminous Spark | 7000 | 1.0686 |
| Molten Destruction | 7000 | 1.0686 |
| Rising Air Current | 7000 | 1.0686 |
| Slot Machine | 7000 | 1.0686 |
| Tailor of the Fickle | 7000 | 1.0686 |
| Toll | 7000 | 1.0686 |
| Umiiruka | 7000 | 1.0686 |
| Ameba | 2500 | 0.3817 |
| Cyber Jar | 2500 | 0.3817 |
| Flying Kamakiri #1 | 2500 | 0.3817 |
| Giant Germ | 2500 | 0.3817 |
| Giant Rat | 2500 | 0.3817 |
| Hiro's Shadow Scout | 2500 | 0.3817 |
| Horn of the Unicorn | 2500 | 0.3817 |
| Hyozanryu | 2500 | 0.3817 |
| Karate Man | 2500 | 0.3817 |
| Manga Ryu-Ran | 2500 | 0.3817 |
| Mother Grizzly | 2500 | 0.3817 |
| Mystic Tomato | 2500 | 0.3817 |
| Nimble Momonga | 2500 | 0.3817 |
| Rush Recklessly | 2500 | 0.3817 |
| Senju of the Thousand Hands | 2500 | 0.3817 |
| UFO Turtle | 2500 | 0.3817 |
| Banisher of the Light | 600 | 0.0916 |
| Black Illusion Ritual | 600 | 0.0916 |
| Black Pendant | 600 | 0.0916 |
| Confiscation | 600 | 0.0916 |
| Giant Trunade | 600 | 0.0916 |
| Invader of the Throne | 600 | 0.0916 |
| Maha Vailo | 600 | 0.0916 |
| Messenger of Peace | 600 | 0.0916 |
| Painful Choice | 600 | 0.0916 |
| Toon World | 600 | 0.0916 |
| Axe of Despair | 100 | 0.0153 |
| Delinquent Duo | 100 | 0.0153 |
| Megamorph | 100 | 0.0153 |
| Mystical Space Typhoon | 100 | 0.0153 |
| Relinquished | 100 | 0.0153 |
| Snatch Steal | 100 | 0.0153 |
| Spellbinding Circle | 100 | 0.0153 |
| The Forceful Sentry | 100 | 0.0153 |
| Toon Mermaid | 100 | 0.0153 |
| Toon Summoned Skull | 100 | 0.0153 |
| Blue-Eyes Toon Dragon | 25 | 0.0038 |
| Serpent Night Dragon | 25 | 0.0038 |

### Treasure

- **Type:** Usage · **Format:** Treasure · **Cards in pool:** 84 · **Weight sum:** 140396

| Card | Weight | Pull % |
|---|---:|---:|
| Skull Knight | 5608 | 3.9944 |
| Cyber Saurus | 3657 | 2.6048 |
| Deepsea Shark | 3657 | 2.6048 |
| Empress Judge | 3657 | 2.6048 |
| Flower Wolf | 3657 | 2.6048 |
| Karbonala Warrior | 3657 | 2.6048 |
| Metal Dragon | 3657 | 2.6048 |
| Musician King | 3657 | 2.6048 |
| Fusionist | 3272 | 2.3306 |
| Labyrinth Tank | 3272 | 2.3306 |
| Thousand Dragon | 3272 | 2.3306 |
| Twin-Headed Thunder Dragon | 3272 | 2.3306 |
| Bickuribox | 2800 | 1.9944 |
| Gaia the Dragon Champion | 2800 | 1.9944 |
| B. Skull Dragon | 2640 | 1.8804 |
| Catapult Turtle | 2305 | 1.6418 |
| Crass Clown | 2305 | 1.6418 |
| Flash Assailant | 2305 | 1.6418 |
| Gaia Power | 2305 | 1.6418 |
| Hercules Beetle | 2305 | 1.6418 |
| Horn of Heaven | 2305 | 1.6418 |
| Kuriboh | 2305 | 1.6418 |
| Mother Grizzly | 2305 | 1.6418 |
| Share the Pain | 2305 | 1.6418 |
| Slot Machine | 2305 | 1.6418 |
| White Magical Hat | 2305 | 1.6418 |
| Charubin the Fire Knight | 2087 | 1.4865 |
| Flame Ghost | 2087 | 1.4865 |
| Flame Swordsman | 2087 | 1.4865 |
| Darkfire Dragon | 2030 | 1.4459 |
| Dragoness the Wicked Knight | 2030 | 1.4459 |
| Giant Rat | 1776 | 1.2650 |
| La Jinn the Mystical Genie of the Lamp | 1776 | 1.2650 |
| Wall of Illusion | 1776 | 1.2650 |
| Chain Energy | 1528 | 1.0884 |
| Giant Germ | 1528 | 1.0884 |
| Just Desserts | 1528 | 1.0884 |
| Minar | 1528 | 1.0884 |
| Princess of Tsurugi | 1528 | 1.0884 |
| Soul Exchange | 1528 | 1.0884 |
| Spear Cretin | 1528 | 1.0884 |
| Summoned Skull | 1528 | 1.0884 |
| Toll | 1528 | 1.0884 |
| Magic Jammer | 1375 | 0.9794 |
| Thunder Dragon | 1268 | 0.9032 |
| Giant Soldier of Stone | 1187 | 0.8455 |
| Swords of Revealing Light | 1187 | 0.8455 |
| Masked Sorcerer | 1123 | 0.7999 |
| Robbin' Goblin | 1123 | 0.7999 |
| Rush Recklessly | 1123 | 0.7999 |
| Card Destruction | 1096 | 0.7806 |
| Nimble Momonga | 990 | 0.7051 |
| Jirai Gumo | 957 | 0.6816 |
| Last Will | 929 | 0.6617 |
| Upstart Goblin | 929 | 0.6617 |
| Solemn Judgment | 880 | 0.6268 |
| Tribute to The Doomed | 860 | 0.6126 |
| Banisher of the Light | 841 | 0.5990 |
| Giant Trunade | 824 | 0.5869 |
| Seven Tools of the Bandit | 824 | 0.5869 |
| Cyber Jar | 808 | 0.5755 |
| Magician of Faith | 793 | 0.5648 |
| Fissure | 779 | 0.5549 |
| Man-Eater Bug | 779 | 0.5549 |
| Mechanicalchaser | 779 | 0.5549 |
| Mystical Space Typhoon | 779 | 0.5549 |
| Trap Hole | 779 | 0.5549 |
| Waboku | 779 | 0.5549 |
| Cannon Soldier | 743 | 0.5292 |
| Mystic Tomato | 743 | 0.5292 |
| Heavy Storm | 681 | 0.4851 |
| Sangan | 650 | 0.4630 |
| Witch of the Black Forest | 650 | 0.4630 |
| Painful Choice | 597 | 0.4252 |
| Snatch Steal | 545 | 0.3882 |
| Monster Reborn | 529 | 0.3768 |
| Change of Heart | 522 | 0.3718 |
| Confiscation | 522 | 0.3718 |
| Dark Hole | 522 | 0.3718 |
| Delinquent Duo | 522 | 0.3718 |
| Mirror Force | 522 | 0.3718 |
| Pot of Greed | 522 | 0.3718 |
| Raigeki | 522 | 0.3718 |
| The Forceful Sentry | 522 | 0.3718 |

### Pharaoh's Servant

- **Type:** TCG · **Format:** Imperial · **Cards in pool:** 62 · **Weight sum:** 365750

| Card | Weight | Pull % |
|---|---:|---:|
| 4-Starred Ladybug of Doom | 10000 | 2.7341 |
| 7 Completed | 10000 | 2.7341 |
| Armored Glass | 10000 | 2.7341 |
| Cold Wave | 10000 | 2.7341 |
| Cyber Falcon | 10000 | 2.7341 |
| DNA Surgery | 10000 | 2.7341 |
| Darkfire Soldier #1 | 10000 | 2.7341 |
| Darkfire Soldier #2 | 10000 | 2.7341 |
| Driving Snow | 10000 | 2.7341 |
| Earthshaker | 10000 | 2.7341 |
| Enchanted Javelin | 10000 | 2.7341 |
| Flame Champion | 10000 | 2.7341 |
| Flying Kamakiri #2 | 10000 | 2.7341 |
| Gift of The Mystical Elf | 10000 | 2.7341 |
| Gust | 10000 | 2.7341 |
| Inspection | 10000 | 2.7341 |
| Kiseitai | 10000 | 2.7341 |
| Light of Intervention | 10000 | 2.7341 |
| Major Riot | 10000 | 2.7341 |
| Metal Detector | 10000 | 2.7341 |
| Mr. Volcano | 10000 | 2.7341 |
| Mystic Probe | 10000 | 2.7341 |
| Respect Play | 10000 | 2.7341 |
| Solemn Wishes | 10000 | 2.7341 |
| Solomon's Lawbook | 10000 | 2.7341 |
| Steel Ogre Grotto #2 | 10000 | 2.7341 |
| The Eye of Truth | 10000 | 2.7341 |
| The Regulation of Tribe | 10000 | 2.7341 |
| Thousand-Eyes Idol | 10000 | 2.7341 |
| Three-Headed Geedo | 10000 | 2.7341 |
| Time Seal | 10000 | 2.7341 |
| Twin-Headed Fire Dragon | 10000 | 2.7341 |
| World Suppression | 10000 | 2.7341 |
| Numinous Healer | 7000 | 1.9139 |
| Appropriate | 2500 | 0.6835 |
| Forced Requisition | 2500 | 0.6835 |
| Gravity Bind | 2500 | 0.6835 |
| Lightforce Sword | 2500 | 0.6835 |
| Magic Drain | 2500 | 0.6835 |
| Morphing Jar #2 | 2500 | 0.6835 |
| Nobleman of Extermination | 2500 | 0.6835 |
| Prohibition | 2500 | 0.6835 |
| The Shallow Grave | 2500 | 0.6835 |
| Backup Soldier | 600 | 0.1640 |
| Dust Tornado | 600 | 0.1640 |
| Gearfried the Iron Knight | 600 | 0.1640 |
| Graverobber | 600 | 0.1640 |
| Limiter Removal | 600 | 0.1640 |
| Magical Hats | 600 | 0.1640 |
| Mirror Wall | 600 | 0.1640 |
| Nobleman of Crossout | 600 | 0.1640 |
| Parasite Paracide | 600 | 0.1640 |
| Buster Blader | 100 | 0.0273 |
| Call Of The Haunted | 100 | 0.0273 |
| Ceasefire | 100 | 0.0273 |
| Chain Destruction | 100 | 0.0273 |
| Goblin Attack Force | 100 | 0.0273 |
| Premature Burial | 100 | 0.0273 |
| The Fiend Megacyber | 100 | 0.0273 |
| Thousand-Eyes Restrict | 100 | 0.0273 |
| Imperial Order | 25 | 0.0068 |
| Jinzo | 25 | 0.0068 |

### Imperial

- **Type:** Usage · **Format:** Imperial · **Cards in pool:** 105 · **Weight sum:** 207032

| Card | Weight | Pull % |
|---|---:|---:|
| Empress Judge | 7103 | 3.4309 |
| Fusionist | 7103 | 3.4309 |
| Giltia the D. Knight | 7103 | 3.4309 |
| Rabid Horseman | 7103 | 3.4309 |
| Gaia the Dragon Champion | 4620 | 2.2315 |
| Bickuribox | 4131 | 1.9953 |
| Skull Knight | 4131 | 1.9953 |
| Charubin the Fire Knight | 3788 | 1.8297 |
| Flame Ghost | 3788 | 1.8297 |
| Cyber Saurus | 3530 | 1.7051 |
| Flame Swordsman | 3326 | 1.6065 |
| Aqua Madoor | 2900 | 1.4007 |
| Exchange | 2900 | 1.4007 |
| Gravity Bind | 2900 | 1.4007 |
| Kuriboh | 2900 | 1.4007 |
| Light of Intervention | 2900 | 1.4007 |
| Mask of Darkness | 2900 | 1.4007 |
| Megamorph | 2900 | 1.4007 |
| Nimble Momonga | 2900 | 1.4007 |
| Prohibition | 2900 | 1.4007 |
| Reinforcements | 2900 | 1.4007 |
| Robbin' Goblin | 2900 | 1.4007 |
| Seven Tools of the Bandit | 2900 | 1.4007 |
| Soul Exchange | 2900 | 1.4007 |
| Thunder Dragon | 2900 | 1.4007 |
| Kwagar Hercules | 2796 | 1.3505 |
| Metal Dragon | 2796 | 1.3505 |
| Deepsea Shark | 2705 | 1.3066 |
| Punished Eagle | 2705 | 1.3066 |
| Roaring Ocean Snake | 2705 | 1.3066 |
| Dragoness the Wicked Knight | 2623 | 1.2670 |
| Thousand Dragon | 2623 | 1.2670 |
| Musician King | 2550 | 1.2317 |
| Backup Soldier | 2484 | 1.1998 |
| Warrior of Tradition | 2484 | 1.1998 |
| Twin-Headed Thunder Dragon | 2368 | 1.1438 |
| Darkfire Dragon | 2271 | 1.0969 |
| Labyrinth Tank | 2271 | 1.0969 |
| Acid Trap Hole | 2227 | 1.0757 |
| B. Skull Dragon | 2227 | 1.0757 |
| Magic Drain | 2227 | 1.0757 |
| Thousand-Eyes Restrict | 2227 | 1.0757 |
| Tribute to The Doomed | 2227 | 1.0757 |
| White Magical Hat | 2227 | 1.0757 |
| Exodia the Forbidden One | 1911 | 0.9230 |
| Giant Germ | 1911 | 0.9230 |
| Graverobber | 1911 | 0.9230 |
| Last Will | 1911 | 0.9230 |
| Left Arm of the Forbidden One | 1911 | 0.9230 |
| Left Leg of the Forbidden One | 1911 | 0.9230 |
| Magic Jammer | 1911 | 0.9230 |
| Morphing Jar #2 | 1911 | 0.9230 |
| Right Arm of the Forbidden One | 1911 | 0.9230 |
| Right Leg of the Forbidden One | 1911 | 0.9230 |
| Black Illusion Ritual | 1717 | 0.8293 |
| Masked Sorcerer | 1717 | 0.8293 |
| Relinquished | 1717 | 0.8293 |
| Rush Recklessly | 1717 | 0.8293 |
| Senju of the Thousand Hands | 1717 | 0.8293 |
| Sonic Bird | 1717 | 0.8293 |
| Upstart Goblin | 1478 | 0.7139 |
| 4-Starred Ladybug of Doom | 1396 | 0.6743 |
| Waboku | 1330 | 0.6424 |
| Giant Soldier of Stone | 1274 | 0.6154 |
| Limiter Removal | 1274 | 0.6154 |
| Card Destruction | 1149 | 0.5550 |
| Solemn Wishes | 1149 | 0.5550 |
| Time Seal | 1037 | 0.5009 |
| Goblin Attack Force | 1015 | 0.4903 |
| Giant Trunade | 995 | 0.4806 |
| The Fiend Megacyber | 976 | 0.4714 |
| Banisher of the Light | 887 | 0.4284 |
| Solemn Judgment | 833 | 0.4024 |
| Cannon Soldier | 791 | 0.3821 |
| Fissure | 769 | 0.3714 |
| Trap Hole | 769 | 0.3714 |
| Dust Tornado | 749 | 0.3618 |
| Magician of Faith | 749 | 0.3618 |
| Man-Eater Bug | 749 | 0.3618 |
| Mechanicalchaser | 749 | 0.3618 |
| Mystic Tomato | 743 | 0.3589 |
| Mystical Space Typhoon | 743 | 0.3589 |
| Painful Choice | 726 | 0.3507 |
| Heavy Storm | 666 | 0.3217 |
| Nobleman of Crossout | 650 | 0.3140 |
| Sangan | 650 | 0.3140 |
| Swords of Revealing Light | 640 | 0.3091 |
| Cyber Jar | 599 | 0.2893 |
| Ceasefire | 574 | 0.2773 |
| Morphing Jar | 568 | 0.2744 |
| Premature Burial | 562 | 0.2715 |
| Call Of The Haunted | 526 | 0.2541 |
| Change of Heart | 522 | 0.2521 |
| Confiscation | 522 | 0.2521 |
| Dark Hole | 522 | 0.2521 |
| Delinquent Duo | 522 | 0.2521 |
| Imperial Order | 522 | 0.2521 |
| Jinzo | 522 | 0.2521 |
| Mirror Force | 522 | 0.2521 |
| Monster Reborn | 522 | 0.2521 |
| Pot of Greed | 522 | 0.2521 |
| Raigeki | 522 | 0.2521 |
| Snatch Steal | 522 | 0.2521 |
| The Forceful Sentry | 522 | 0.2521 |
| Witch of the Black Forest | 522 | 0.2521 |

### Labyrinth of Nightmare

- **Type:** TCG · **Format:** Android · **Cards in pool:** 15 · **Weight sum:** 44850

| Card | Weight | Pull % |
|---|---:|---:|
| Fairy Box | 10000 | 22.2965 |
| Hysteric Fairy | 10000 | 22.2965 |
| Skull Lair | 10000 | 22.2965 |
| Zombyra the Dark | 10000 | 22.2965 |
| Gilasaurus | 2500 | 5.5741 |
| Bazoo the Soul-Eater | 600 | 1.3378 |
| Jar of Greed | 600 | 1.3378 |
| Kycoo the Ghost Destroyer | 600 | 1.3378 |
| Mage Power | 100 | 0.2230 |
| Mask of Restrict | 100 | 0.2230 |
| The Last Warrior from Another Planet | 100 | 0.2230 |
| Torrential Tribute | 100 | 0.2230 |
| United We Stand | 100 | 0.2230 |
| Gemini Elf | 25 | 0.0557 |
| Magic Cylinder | 25 | 0.0557 |

### Android

- **Type:** Usage · **Format:** Android · **Cards in pool:** 102 · **Weight sum:** 208095

| Card | Weight | Pull % |
|---|---:|---:|
| Flame Ghost | 6829 | 3.2817 |
| Flower Wolf | 6829 | 3.2817 |
| Giltia the D. Knight | 6829 | 3.2817 |
| Karbonala Warrior | 6829 | 3.2817 |
| Punished Eagle | 6829 | 3.2817 |
| Kwagar Hercules | 5205 | 2.5013 |
| Cyber Saurus | 4443 | 2.1351 |
| Kaminari Attack | 4443 | 2.1351 |
| Labyrinth Tank | 4443 | 2.1351 |
| Skull Knight | 4443 | 2.1351 |
| Charubin the Fire Knight | 3973 | 1.9092 |
| Metal Dragon | 3973 | 1.9092 |
| Thousand Dragon | 3973 | 1.9092 |
| Bickuribox | 3644 | 1.7511 |
| Musician King | 3396 | 1.6319 |
| Fusionist | 3201 | 1.5382 |
| Warrior of Tradition | 3201 | 1.5382 |
| Dragoness the Wicked Knight | 3040 | 1.4609 |
| Twin-Headed Thunder Dragon | 3040 | 1.4609 |
| Darkfire Dragon | 2906 | 1.3965 |
| Axe of Despair | 2791 | 1.3412 |
| Castle of Dark Illusions | 2791 | 1.3412 |
| Dark Elf | 2791 | 1.3412 |
| Fake Trap | 2791 | 1.3412 |
| Firegrass | 2791 | 1.3412 |
| Giant Germ | 2791 | 1.3412 |
| Giant Soldier of Stone | 2791 | 1.3412 |
| Giant Trunade | 2791 | 1.3412 |
| La Jinn the Mystical Genie of the Lamp | 2791 | 1.3412 |
| Megamorph | 2791 | 1.3412 |
| Mirror Wall | 2791 | 1.3412 |
| Soul Release | 2791 | 1.3412 |
| Summoned Skull | 2791 | 1.3412 |
| The Fiend Megacyber | 2791 | 1.3412 |
| Thunder Dragon | 2791 | 1.3412 |
| Flame Swordsman | 2603 | 1.2509 |
| Gaia the Dragon Champion | 2525 | 1.2134 |
| Thousand-Eyes Restrict | 2455 | 1.1797 |
| B. Skull Dragon | 2391 | 1.1490 |
| The Last Warrior from Another Planet | 2231 | 1.0721 |
| Banisher of the Light | 2144 | 1.0303 |
| Kuriboh | 2144 | 1.0303 |
| Nobleman of Extermination | 2144 | 1.0303 |
| Fairy Box | 1841 | 0.8847 |
| Goblin Attack Force | 1841 | 0.8847 |
| Jirai Gumo | 1841 | 0.8847 |
| Magic Drain | 1841 | 0.8847 |
| Nimble Momonga | 1841 | 0.8847 |
| Waboku | 1841 | 0.8847 |
| White Magical Hat | 1841 | 0.8847 |
| Man-Eater Bug | 1523 | 0.7319 |
| Mask of Darkness | 1523 | 0.7319 |
| Seven Tools of the Bandit | 1523 | 0.7319 |
| Mage Power | 1424 | 0.6843 |
| Upstart Goblin | 1424 | 0.6843 |
| Zombyra the Dark | 1424 | 0.6843 |
| Exchange | 1346 | 0.6468 |
| Solemn Wishes | 1346 | 0.6468 |
| United We Stand | 1229 | 0.5906 |
| Masked Sorcerer | 1144 | 0.5497 |
| Penguin Knight | 1144 | 0.5497 |
| 4-Starred Ladybug of Doom | 1109 | 0.5329 |
| Time Seal | 1049 | 0.5041 |
| Card Destruction | 1024 | 0.4921 |
| Solemn Judgment | 980 | 0.4709 |
| Cannon Soldier | 943 | 0.4532 |
| Dust Tornado | 805 | 0.3868 |
| Magic Cylinder | 805 | 0.3868 |
| Magician of Faith | 780 | 0.3748 |
| Swords of Revealing Light | 780 | 0.3748 |
| Trap Hole | 780 | 0.3748 |
| Skull Lair | 772 | 0.3710 |
| Jar of Greed | 764 | 0.3671 |
| Fissure | 757 | 0.3638 |
| Mystic Tomato | 757 | 0.3638 |
| Bazoo the Soul-Eater | 750 | 0.3604 |
| Gemini Elf | 750 | 0.3604 |
| Torrential Tribute | 750 | 0.3604 |
| Kycoo the Ghost Destroyer | 743 | 0.3570 |
| Mystical Space Typhoon | 743 | 0.3570 |
| Ceasefire | 737 | 0.3542 |
| Cyber Jar | 662 | 0.3181 |
| Heavy Storm | 650 | 0.3124 |
| Nobleman of Crossout | 650 | 0.3124 |
| Premature Burial | 650 | 0.3124 |
| Morphing Jar | 629 | 0.3023 |
| Painful Choice | 530 | 0.2547 |
| Imperial Order | 526 | 0.2528 |
| Snatch Steal | 526 | 0.2528 |
| Call Of The Haunted | 522 | 0.2508 |
| Change of Heart | 522 | 0.2508 |
| Confiscation | 522 | 0.2508 |
| Dark Hole | 522 | 0.2508 |
| Delinquent Duo | 522 | 0.2508 |
| Jinzo | 522 | 0.2508 |
| Mirror Force | 522 | 0.2508 |
| Monster Reborn | 522 | 0.2508 |
| Pot of Greed | 522 | 0.2508 |
| Raigeki | 522 | 0.2508 |
| Sangan | 522 | 0.2508 |
| The Forceful Sentry | 522 | 0.2508 |
| Witch of the Black Forest | 522 | 0.2508 |

### Starter Deck- Joey

- **Type:** TCG · **Format:** Joey-Pegasus · **Cards in pool:** 48 · **Weight sum:** 451300

| Card | Weight | Pull % |
|---|---:|---:|
| 7 Colored Fish | 10000 | 2.2158 |
| Armored Lizard | 10000 | 2.2158 |
| Baby Dragon | 10000 | 2.2158 |
| Big Eye | 10000 | 2.2158 |
| Block Attack | 10000 | 2.2158 |
| Castle Walls | 10000 | 2.2158 |
| Change of Heart | 10000 | 2.2158 |
| Dark Hole | 10000 | 2.2158 |
| Darkfire Soldier #1 | 10000 | 2.2158 |
| De-Spell | 10000 | 2.2158 |
| Dian Keto the Cure Master | 10000 | 2.2158 |
| Dragon Treasure | 10000 | 2.2158 |
| Eternal Rest | 10000 | 2.2158 |
| Fake Trap | 10000 | 2.2158 |
| Fissure | 10000 | 2.2158 |
| Flame Manipulator | 10000 | 2.2158 |
| Flame Swordsman | 10000 | 2.2158 |
| Gearfried the Iron Knight | 10000 | 2.2158 |
| Giant Trunade | 10000 | 2.2158 |
| Just Desserts | 10000 | 2.2158 |
| Karate Man | 10000 | 2.2158 |
| Magician of Faith | 10000 | 2.2158 |
| Maha Vailo | 10000 | 2.2158 |
| Malevolent Nuzzler | 10000 | 2.2158 |
| Masaki the Legendary Swordsman | 10000 | 2.2158 |
| Milus Radiant | 10000 | 2.2158 |
| Monster Reborn | 10000 | 2.2158 |
| Mountain | 10000 | 2.2158 |
| Polymerization | 10000 | 2.2158 |
| Polymerization | 10000 | 2.2158 |
| Princess of Tsurugi | 10000 | 2.2158 |
| Reinforcements | 10000 | 2.2158 |
| Remove Trap | 10000 | 2.2158 |
| Reverse Trap | 10000 | 2.2158 |
| Sangan | 10000 | 2.2158 |
| Seven Tools of the Bandit | 10000 | 2.2158 |
| Shield & Sword | 10000 | 2.2158 |
| Spirit of the Harp | 10000 | 2.2158 |
| The Reliable Guardian | 10000 | 2.2158 |
| Thousand Dragon | 10000 | 2.2158 |
| Time Wizard | 10000 | 2.2158 |
| Trap Hole | 10000 | 2.2158 |
| Ultimate Offering | 10000 | 2.2158 |
| Waboku | 10000 | 2.2158 |
| White Magical Hat | 10000 | 2.2158 |
| Penguin Soldier | 600 | 0.1329 |
| Scapegoat | 600 | 0.1329 |
| Red-Eyes B. Dragon | 100 | 0.0222 |

### Starter Deck- Pegasus

- **Type:** TCG · **Format:** Joey-Pegasus · **Cards in pool:** 50 · **Weight sum:** 471300

| Card | Weight | Pull % |
|---|---:|---:|
| Aqua Madoor | 10000 | 2.1218 |
| Armed Ninja | 10000 | 2.1218 |
| Black Illusion Ritual | 10000 | 2.1218 |
| Black Pendant | 10000 | 2.1218 |
| Blue-Eyes Toon Dragon | 10000 | 2.1218 |
| Castle Walls | 10000 | 2.1218 |
| Change of Heart | 10000 | 2.1218 |
| Dark Hole | 10000 | 2.1218 |
| De-Spell | 10000 | 2.1218 |
| Dian Keto the Cure Master | 10000 | 2.1218 |
| Dream Clown | 10000 | 2.1218 |
| Enchanted Javelin | 10000 | 2.1218 |
| Fissure | 10000 | 2.1218 |
| Giant Soldier of Stone | 10000 | 2.1218 |
| Hane-Hane | 10000 | 2.1218 |
| Hiro's Shadow Scout | 10000 | 2.1218 |
| Illusionist Faceless Mage | 10000 | 2.1218 |
| Jigen Bakudan | 10000 | 2.1218 |
| Magic Jammer | 10000 | 2.1218 |
| Man-Eater Bug | 10000 | 2.1218 |
| Manga Ryu-Ran | 10000 | 2.1218 |
| Mask of Darkness | 10000 | 2.1218 |
| Monster Reborn | 10000 | 2.1218 |
| Muka Muka | 10000 | 2.1218 |
| Mystical Space Typhoon | 10000 | 2.1218 |
| Red Archery Girl | 10000 | 2.1218 |
| Reinforcements | 10000 | 2.1218 |
| Remove Trap | 10000 | 2.1218 |
| Ring of Magnetism | 10000 | 2.1218 |
| Robbin' Goblin | 10000 | 2.1218 |
| Rogue Doll | 10000 | 2.1218 |
| Rush Recklessly | 10000 | 2.1218 |
| Ryu-Ran | 10000 | 2.1218 |
| Seven Tools of the Bandit | 10000 | 2.1218 |
| Sonic Bird | 10000 | 2.1218 |
| Soul Release | 10000 | 2.1218 |
| Stop Defense | 10000 | 2.1218 |
| Toon Alligator | 10000 | 2.1218 |
| Toon Mermaid | 10000 | 2.1218 |
| Toon Summoned Skull | 10000 | 2.1218 |
| Toon World | 10000 | 2.1218 |
| Trap Hole | 10000 | 2.1218 |
| Ultimate Offering | 10000 | 2.1218 |
| Uraby | 10000 | 2.1218 |
| Waboku | 10000 | 2.1218 |
| Witch of the Black Forest | 10000 | 2.1218 |
| Yami | 10000 | 2.1218 |
| Graceful Charity | 600 | 0.1273 |
| Gryphon Wing | 600 | 0.1273 |
| Relinquished | 100 | 0.0212 |

### Joey-Pegasus

- **Type:** Usage · **Format:** Joey-Pegasus · **Cards in pool:** 105 · **Weight sum:** 202512

| Card | Weight | Pull % |
|---|---:|---:|
| Bickuribox | 5205 | 2.5702 |
| Charubin the Fire Knight | 5205 | 2.5702 |
| Dark Balter the Terrible | 5205 | 2.5702 |
| Empress Judge | 5205 | 2.5702 |
| Fiend Skull Dragon | 5205 | 2.5702 |
| Flame Ghost | 5205 | 2.5702 |
| Flower Wolf | 5205 | 2.5702 |
| Giltia the D. Knight | 5205 | 2.5702 |
| Kaminari Attack | 5205 | 2.5702 |
| Roaring Ocean Snake | 5205 | 2.5702 |
| Ryu Senshi | 5205 | 2.5702 |
| Twin-Headed Thunder Dragon | 5205 | 2.5702 |
| Warrior of Tradition | 5205 | 2.5702 |
| Cyber Saurus | 3973 | 1.9619 |
| Dragoness the Wicked Knight | 3973 | 1.9619 |
| Metal Dragon | 3973 | 1.9619 |
| Musician King | 3973 | 1.9619 |
| Skull Knight | 3973 | 1.9619 |
| Darkfire Dragon | 3396 | 1.6769 |
| Fusionist | 3396 | 1.6769 |
| Labyrinth Tank | 3396 | 1.6769 |
| Thousand-Eyes Restrict | 2791 | 1.3782 |
| Flame Swordsman | 2455 | 1.2123 |
| Gaia the Dragon Champion | 2455 | 1.2123 |
| B. Skull Dragon | 2231 | 1.1017 |
| Armed Ninja | 2144 | 1.0587 |
| Axe of Despair | 2144 | 1.0587 |
| Cold Wave | 2144 | 1.0587 |
| Giant Germ | 2144 | 1.0587 |
| Giant Rat | 2144 | 1.0587 |
| Giant Trunade | 2144 | 1.0587 |
| Guardian Sphinx | 2144 | 1.0587 |
| Kotodama | 2144 | 1.0587 |
| Last Will | 2144 | 1.0587 |
| Magic Drain | 2144 | 1.0587 |
| Penguin Knight | 2144 | 1.0587 |
| Reinforcement of the Army | 2144 | 1.0587 |
| Rush Recklessly | 2144 | 1.0587 |
| Solemn Wishes | 2144 | 1.0587 |
| Summoned Skull | 2144 | 1.0587 |
| White Magical Hat | 2144 | 1.0587 |
| The Last Warrior from Another Planet | 2069 | 1.0217 |
| Backup Soldier | 1841 | 0.9091 |
| Exchange | 1654 | 0.8167 |
| Kuriboh | 1654 | 0.8167 |
| Masked Sorcerer | 1654 | 0.8167 |
| Mirror Wall | 1654 | 0.8167 |
| Nimble Momonga | 1654 | 0.8167 |
| Seven Tools of the Bandit | 1654 | 0.8167 |
| Thunder Dragon | 1654 | 0.8167 |
| Card Destruction | 1424 | 0.7032 |
| Exodia the Forbidden One | 1424 | 0.7032 |
| Left Arm of the Forbidden One | 1424 | 0.7032 |
| Left Leg of the Forbidden One | 1424 | 0.7032 |
| Limiter Removal | 1424 | 0.7032 |
| Right Arm of the Forbidden One | 1424 | 0.7032 |
| Right Leg of the Forbidden One | 1424 | 0.7032 |
| Swords of Revealing Light | 1424 | 0.7032 |
| Time Seal | 1183 | 0.5842 |
| Man-Eater Bug | 1109 | 0.5476 |
| United We Stand | 1109 | 0.5476 |
| Scapegoat | 1049 | 0.5180 |
| Waboku | 1001 | 0.4943 |
| Cannon Soldier | 961 | 0.4745 |
| Mage Power | 961 | 0.4745 |
| Trap Hole | 961 | 0.4745 |
| Upstart Goblin | 961 | 0.4745 |
| Dust Tornado | 926 | 0.4573 |
| Solemn Judgment | 926 | 0.4573 |
| Fissure | 824 | 0.4069 |
| Skull Lair | 824 | 0.4069 |
| Ceasefire | 805 | 0.3975 |
| Jar of Greed | 805 | 0.3975 |
| Magic Cylinder | 805 | 0.3975 |
| Cyber Jar | 757 | 0.3738 |
| Torrential Tribute | 757 | 0.3738 |
| Bazoo the Soul-Eater | 743 | 0.3669 |
| Gemini Elf | 743 | 0.3669 |
| Kycoo the Ghost Destroyer | 743 | 0.3669 |
| Magician of Faith | 743 | 0.3669 |
| Mystic Tomato | 743 | 0.3669 |
| Mystical Space Typhoon | 743 | 0.3669 |
| Morphing Jar | 688 | 0.3397 |
| Nobleman of Crossout | 662 | 0.3269 |
| Graceful Charity | 650 | 0.3210 |
| Heavy Storm | 562 | 0.2775 |
| Painful Choice | 530 | 0.2617 |
| Premature Burial | 530 | 0.2617 |
| Call Of The Haunted | 522 | 0.2578 |
| Change of Heart | 522 | 0.2578 |
| Confiscation | 522 | 0.2578 |
| Dark Hole | 522 | 0.2578 |
| Delinquent Duo | 522 | 0.2578 |
| Harpie's Feather Duster | 522 | 0.2578 |
| Imperial Order | 522 | 0.2578 |
| Jinzo | 522 | 0.2578 |
| Mirror Force | 522 | 0.2578 |
| Monster Reborn | 522 | 0.2578 |
| Pot of Greed | 522 | 0.2578 |
| Raigeki | 522 | 0.2578 |
| Sangan | 522 | 0.2578 |
| Sinister Serpent | 522 | 0.2578 |
| Snatch Steal | 522 | 0.2578 |
| The Forceful Sentry | 522 | 0.2578 |
| Witch of the Black Forest | 522 | 0.2578 |

### Legacy of Darkness

- **Type:** TCG · **Format:** Fiber · **Cards in pool:** 19 · **Weight sum:** 24750

| Card | Weight | Pull % |
|---|---:|---:|
| Emergency Provisions | 10000 | 40.4040 |
| Blast with Chain | 2500 | 10.1010 |
| Bottomless Trap Hole | 2500 | 10.1010 |
| Royal Oppression | 2500 | 10.1010 |
| Thunder Nyan Nyan | 2500 | 10.1010 |
| Asura Priest | 600 | 2.4242 |
| Dark Balter the Terrible | 600 | 2.4242 |
| Exiled Force | 600 | 2.4242 |
| Fiend Skull Dragon | 600 | 2.4242 |
| Reinforcement of the Army | 600 | 2.4242 |
| Ryu Senshi | 600 | 2.4242 |
| Spear Dragon | 600 | 2.4242 |
| Airknight Parshath | 100 | 0.4040 |
| Creature Swap | 100 | 0.4040 |
| Dark Ruler Ha Des | 100 | 0.4040 |
| Fiber Jar | 100 | 0.4040 |
| Marauding Captain | 100 | 0.4040 |
| Injection Fairy Lily | 25 | 0.1010 |
| Yata-Garasu | 25 | 0.1010 |

### Fiber

- **Type:** Usage · **Format:** Fiber · **Cards in pool:** 132 · **Weight sum:** 350776

| Card | Weight | Pull % |
|---|---:|---:|
| Giltia the D. Knight | 12784 | 3.6445 |
| Punished Eagle | 9717 | 2.7701 |
| Empress Judge | 8280 | 2.3605 |
| Flower Wolf | 8280 | 2.3605 |
| Kwagar Hercules | 8280 | 2.3605 |
| Metal Dragon | 6304 | 1.7972 |
| Charubin the Fire Knight | 5632 | 1.6056 |
| Cyber Saurus | 5632 | 1.6056 |
| Warrior of Tradition | 5632 | 1.6056 |
| Bickuribox | 5379 | 1.5335 |
| Skull Knight | 5379 | 1.5335 |
| Black Illusion Ritual | 5162 | 1.4716 |
| Bottomless Trap Hole | 5162 | 1.4716 |
| Don Zaloog | 5162 | 1.4716 |
| Flash Assailant | 5162 | 1.4716 |
| Flying Kamakiri #1 | 5162 | 1.4716 |
| Graceful Dice | 5162 | 1.4716 |
| Gravity Bind | 5162 | 1.4716 |
| Jirai Gumo | 5162 | 1.4716 |
| Man-Eater Bug | 5162 | 1.4716 |
| Megamorph | 5162 | 1.4716 |
| Metalmorph | 5162 | 1.4716 |
| Mirror Wall | 5162 | 1.4716 |
| Nobleman of Extermination | 5162 | 1.4716 |
| Relinquished | 5162 | 1.4716 |
| Rush Recklessly | 5162 | 1.4716 |
| Senju of the Thousand Hands | 5162 | 1.4716 |
| Sonic Bird | 5162 | 1.4716 |
| Summoned Skull | 5162 | 1.4716 |
| Flame Swordsman | 4973 | 1.4177 |
| Flame Ghost | 4807 | 1.3704 |
| Fusionist | 4105 | 1.1703 |
| Giant Trunade | 3941 | 1.1235 |
| Graverobber | 3941 | 1.1235 |
| Kuriboh | 3941 | 1.1235 |
| Mask of Darkness | 3941 | 1.1235 |
| Princess of Tsurugi | 3941 | 1.1235 |
| White Magical Hat | 3941 | 1.1235 |
| Thousand Dragon | 3867 | 1.1024 |
| Gaia the Dragon Champion | 3560 | 1.0149 |
| Roaring Ocean Snake | 3560 | 1.0149 |
| Acid Trap Hole | 3369 | 0.9604 |
| Last Will | 3369 | 0.9604 |
| Nimble Momonga | 3369 | 0.9604 |
| Solemn Wishes | 3369 | 0.9604 |
| Soul Exchange | 3369 | 0.9604 |
| Musician King | 3210 | 0.9151 |
| Labyrinth Tank | 3107 | 0.8858 |
| Magic Drain | 3016 | 0.8598 |
| Darkfire Dragon | 2933 | 0.8361 |
| Exchange | 2768 | 0.7891 |
| Fiend Skull Dragon | 2566 | 0.7315 |
| Dark Balter the Terrible | 2504 | 0.7138 |
| Dragoness the Wicked Knight | 2504 | 0.7138 |
| B. Skull Dragon | 2490 | 0.7099 |
| Ryu Senshi | 2348 | 0.6694 |
| Twin-Headed Thunder Dragon | 2348 | 0.6694 |
| Blast with Chain | 2314 | 0.6597 |
| Gearfried the Iron Knight | 2314 | 0.6597 |
| Tribute to The Doomed | 2314 | 0.6597 |
| Chain Energy | 2213 | 0.6309 |
| Electric Snake | 2213 | 0.6309 |
| The Fiend Megacyber | 2213 | 0.6309 |
| Waboku | 2213 | 0.6309 |
| Giant Germ | 2127 | 0.6064 |
| The Last Warrior from Another Planet | 2103 | 0.5995 |
| Thousand-Eyes Restrict | 2073 | 0.5910 |
| Magic Jammer | 1927 | 0.5494 |
| Seven Tools of the Bandit | 1874 | 0.5342 |
| Swords of Revealing Light | 1827 | 0.5208 |
| Thunder Dragon | 1827 | 0.5208 |
| Zombyra the Dark | 1827 | 0.5208 |
| Reinforcement of the Army | 1763 | 0.5026 |
| Goblin Attack Force | 1743 | 0.4969 |
| 4-Starred Ladybug of Doom | 1612 | 0.4596 |
| Solemn Judgment | 1612 | 0.4596 |
| Airknight Parshath | 1469 | 0.4188 |
| Dark Ruler Ha Des | 1431 | 0.4080 |
| Limiter Removal | 1413 | 0.4028 |
| Mage Power | 1365 | 0.3891 |
| Giant Rat | 1284 | 0.3660 |
| Time Seal | 1273 | 0.3629 |
| Cannon Soldier | 1220 | 0.3478 |
| Spear Dragon | 1201 | 0.3424 |
| United We Stand | 1192 | 0.3398 |
| Upstart Goblin | 1142 | 0.3256 |
| Skull Lair | 1100 | 0.3136 |
| Card Destruction | 1064 | 0.3033 |
| Jar of Greed | 904 | 0.2577 |
| Fissure | 839 | 0.2392 |
| Asura Priest | 831 | 0.2369 |
| Ceasefire | 829 | 0.2363 |
| Trap Hole | 829 | 0.2363 |
| Creature Swap | 826 | 0.2355 |
| Dust Tornado | 822 | 0.2343 |
| Scapegoat | 822 | 0.2343 |
| Gemini Elf | 792 | 0.2258 |
| Magician of Faith | 792 | 0.2258 |
| Kycoo the Ghost Destroyer | 784 | 0.2235 |
| Bazoo the Soul-Eater | 779 | 0.2221 |
| Mystic Tomato | 762 | 0.2172 |
| Mystical Space Typhoon | 745 | 0.2124 |
| Torrential Tribute | 743 | 0.2118 |
| Magic Cylinder | 683 | 0.1947 |
| Cyber Jar | 672 | 0.1916 |
| Morphing Jar | 669 | 0.1907 |
| Nobleman of Crossout | 663 | 0.1890 |
| Graceful Charity | 650 | 0.1853 |
| Premature Burial | 586 | 0.1671 |
| Heavy Storm | 571 | 0.1628 |
| Yata-Garasu | 543 | 0.1548 |
| Painful Choice | 534 | 0.1522 |
| Jinzo | 529 | 0.1508 |
| Sinister Serpent | 529 | 0.1508 |
| Confiscation | 526 | 0.1500 |
| Delinquent Duo | 525 | 0.1497 |
| Injection Fairy Lily | 525 | 0.1497 |
| Call Of The Haunted | 523 | 0.1491 |
| Change of Heart | 523 | 0.1491 |
| Exiled Force | 523 | 0.1491 |
| Fiber Jar | 523 | 0.1491 |
| Harpie's Feather Duster | 523 | 0.1491 |
| Monster Reborn | 523 | 0.1491 |
| Snatch Steal | 523 | 0.1491 |
| Dark Hole | 522 | 0.1488 |
| Imperial Order | 522 | 0.1488 |
| Mirror Force | 522 | 0.1488 |
| Pot of Greed | 522 | 0.1488 |
| Raigeki | 522 | 0.1488 |
| Sangan | 522 | 0.1488 |
| The Forceful Sentry | 522 | 0.1488 |
| Witch of the Black Forest | 522 | 0.1488 |

### Pharaonic Guardian

- **Type:** TCG · **Format:** Yata · **Cards in pool:** 18 · **Weight sum:** 75125

| Card | Weight | Pull % |
|---|---:|---:|
| Different Dimension Capsule | 10000 | 13.3111 |
| Gravekeeper's Spy | 10000 | 13.3111 |
| Metamorphosis | 10000 | 13.3111 |
| Raigeki Break | 10000 | 13.3111 |
| Reasoning | 10000 | 13.3111 |
| Trap Dustshoot | 10000 | 13.3111 |
| Book of Moon | 2500 | 3.3278 |
| King Tiger Wanghu | 2500 | 3.3278 |
| Newdoria | 2500 | 3.3278 |
| Pyramid Turtle | 2500 | 3.3278 |
| Spirit Reaper | 2500 | 3.3278 |
| Book of Life | 600 | 0.7987 |
| Mirage of Nightmare | 600 | 0.7987 |
| Reaper on the Nightmare | 600 | 0.7987 |
| Sasuke Samurai | 600 | 0.7987 |
| Don Zaloog | 100 | 0.1331 |
| Guardian Sphinx | 100 | 0.1331 |
| Bell of Destruction | 25 | 0.0333 |

### Yata

- **Type:** Usage · **Format:** Yata · **Cards in pool:** 132 · **Weight sum:** 265361

| Card | Weight | Pull % |
|---|---:|---:|
| Warrior of Tradition | 9065 | 3.4161 |
| Punished Eagle | 6899 | 2.5999 |
| Fusionist | 5884 | 2.2174 |
| Skull Knight | 5884 | 2.2174 |
| Metal Dragon | 5258 | 1.9815 |
| Thousand Dragon | 5258 | 1.9815 |
| Flower Wolf | 4819 | 1.8160 |
| Giltia the D. Knight | 4819 | 1.8160 |
| Charubin the Fire Knight | 4489 | 1.6917 |
| Flame Ghost | 4227 | 1.5929 |
| Labyrinth Tank | 4227 | 1.5929 |
| Karbonala Warrior | 4014 | 1.5127 |
| Cyber Saurus | 3834 | 1.4448 |
| Cold Wave | 3681 | 1.3872 |
| Forced Requisition | 3681 | 1.3872 |
| Graverobber | 3681 | 1.3872 |
| Just Desserts | 3681 | 1.3872 |
| Last Will | 3681 | 1.3872 |
| Magic Drain | 3681 | 1.3872 |
| Mask of Darkness | 3681 | 1.3872 |
| Masked Sorcerer | 3681 | 1.3872 |
| Mirror Wall | 3681 | 1.3872 |
| Musician King | 3681 | 1.3872 |
| Empress Judge | 3548 | 1.3370 |
| Flame Swordsman | 3548 | 1.3370 |
| Roaring Ocean Snake | 3548 | 1.3370 |
| Bickuribox | 3431 | 1.2930 |
| Morphing Jar #2 | 3148 | 1.1863 |
| Book of Life | 2819 | 1.0623 |
| Giant Trunade | 2819 | 1.0623 |
| Magic Jammer | 2819 | 1.0623 |
| Princess of Tsurugi | 2819 | 1.0623 |
| Twin-Headed Thunder Dragon | 2819 | 1.0623 |
| Waboku | 2819 | 1.0623 |
| Gaia the Dragon Champion | 2718 | 1.0243 |
| Darkfire Dragon | 2588 | 0.9753 |
| Dragoness the Wicked Knight | 2446 | 0.9218 |
| Barrel Dragon | 2415 | 0.9101 |
| Giant Rat | 2415 | 0.9101 |
| Kuriboh | 2415 | 0.9101 |
| Limiter Removal | 2415 | 0.9101 |
| Marauding Captain | 2415 | 0.9101 |
| Mechanicalchaser | 2415 | 0.9101 |
| Pyramid Turtle | 2415 | 0.9101 |
| Reasoning | 2415 | 0.9101 |
| Spear Dragon | 2415 | 0.9101 |
| United We Stand | 2415 | 0.9101 |
| White Magical Hat | 2415 | 0.9101 |
| B. Skull Dragon | 2278 | 0.8585 |
| Giant Germ | 2165 | 0.8159 |
| Nimble Momonga | 2165 | 0.8159 |
| The Last Warrior from Another Planet | 2126 | 0.8012 |
| Reaper on the Nightmare | 2089 | 0.7872 |
| Guardian Sphinx | 1991 | 0.7503 |
| Fiend Skull Dragon | 1921 | 0.7239 |
| Dark Balter the Terrible | 1883 | 0.7096 |
| Card Destruction | 1859 | 0.7006 |
| Chain Energy | 1859 | 0.7006 |
| Ryu Senshi | 1836 | 0.6919 |
| Thousand-Eyes Restrict | 1794 | 0.6761 |
| Goblin Attack Force | 1755 | 0.6614 |
| Sasuke Samurai | 1755 | 0.6614 |
| Seven Tools of the Bandit | 1755 | 0.6614 |
| Blast with Chain | 1670 | 0.6293 |
| Gearfried the Iron Knight | 1670 | 0.6293 |
| Raigeki Break | 1599 | 0.6026 |
| Solemn Judgment | 1599 | 0.6026 |
| Time Seal | 1599 | 0.6026 |
| Trap Dustshoot | 1599 | 0.6026 |
| 4-Starred Ladybug of Doom | 1538 | 0.5796 |
| Solemn Wishes | 1538 | 0.5796 |
| Cannon Soldier | 1485 | 0.5596 |
| Trap Hole | 1485 | 0.5596 |
| Metamorphosis | 1438 | 0.5419 |
| Upstart Goblin | 1438 | 0.5419 |
| Airknight Parshath | 1396 | 0.5261 |
| Electric Snake | 1359 | 0.5121 |
| Swords of Revealing Light | 1325 | 0.4993 |
| Jar of Greed | 1217 | 0.4586 |
| Dark Ruler Ha Des | 1174 | 0.4424 |
| Newdoria | 1174 | 0.4424 |
| Asura Priest | 1136 | 0.4281 |
| Book of Moon | 1073 | 0.4044 |
| King Tiger Wanghu | 1073 | 0.4044 |
| Bazoo the Soul-Eater | 1059 | 0.3991 |
| Kycoo the Ghost Destroyer | 1059 | 0.3991 |
| Creature Swap | 1016 | 0.3829 |
| Fissure | 999 | 0.3765 |
| Cyber Jar | 943 | 0.3554 |
| Scapegoat | 943 | 0.3554 |
| Dust Tornado | 897 | 0.3380 |
| Gravekeeper's Spy | 883 | 0.3328 |
| Zombyra the Dark | 859 | 0.3237 |
| Magician of Faith | 848 | 0.3196 |
| Magic Cylinder | 842 | 0.3173 |
| Reinforcement of the Army | 827 | 0.3117 |
| Don Zaloog | 822 | 0.3098 |
| Gemini Elf | 822 | 0.3098 |
| Morphing Jar | 812 | 0.3060 |
| Mystic Tomato | 808 | 0.3045 |
| Spirit Reaper | 779 | 0.2936 |
| Torrential Tribute | 750 | 0.2826 |
| Mystical Space Typhoon | 743 | 0.2800 |
| Ceasefire | 701 | 0.2642 |
| Premature Burial | 701 | 0.2642 |
| Nobleman of Crossout | 653 | 0.2461 |
| Painful Choice | 575 | 0.2167 |
| Yata-Garasu | 575 | 0.2167 |
| Sinister Serpent | 560 | 0.2110 |
| Fiber Jar | 549 | 0.2069 |
| Injection Fairy Lily | 547 | 0.2061 |
| Heavy Storm | 539 | 0.2031 |
| Call Of The Haunted | 537 | 0.2024 |
| Confiscation | 530 | 0.1997 |
| Delinquent Duo | 528 | 0.1990 |
| Exiled Force | 528 | 0.1990 |
| Sangan | 528 | 0.1990 |
| Jinzo | 526 | 0.1982 |
| Monster Reborn | 526 | 0.1982 |
| Imperial Order | 524 | 0.1975 |
| Mirage of Nightmare | 524 | 0.1975 |
| Bell of Destruction | 522 | 0.1967 |
| Change of Heart | 522 | 0.1967 |
| Dark Hole | 522 | 0.1967 |
| Graceful Charity | 522 | 0.1967 |
| Harpie's Feather Duster | 522 | 0.1967 |
| Mirror Force | 522 | 0.1967 |
| Pot of Greed | 522 | 0.1967 |
| Raigeki | 522 | 0.1967 |
| Snatch Steal | 522 | 0.1967 |
| The Forceful Sentry | 522 | 0.1967 |
| Witch of the Black Forest | 522 | 0.1967 |

### Magician's Force

- **Type:** TCG · **Format:** Scientist · **Cards in pool:** 15 · **Weight sum:** 73525

| Card | Weight | Pull % |
|---|---:|---:|
| Dark Scorpion - Cliff the Trap Remover | 10000 | 13.6008 |
| Magical Merchant | 10000 | 13.6008 |
| Magical Scientist | 10000 | 13.6008 |
| Poison of the Old Man | 10000 | 13.6008 |
| Roulette Barrel | 10000 | 13.6008 |
| Wave-Motion Cannon | 10000 | 13.6008 |
| Secret Barrel | 7000 | 9.5206 |
| Apprentice Magician | 2500 | 3.4002 |
| Des Koala | 2500 | 3.4002 |
| Skilled Dark Magician | 600 | 0.8160 |
| Tribe-Infecting Virus | 600 | 0.8160 |
| Breaker the Magical Warrior | 100 | 0.1360 |
| Dark Paladin | 100 | 0.1360 |
| Spell Canceller | 100 | 0.1360 |
| Dark Magician Girl | 25 | 0.0340 |

### Scientist

- **Type:** Usage · **Format:** Scientist · **Cards in pool:** 88 · **Weight sum:** 104629

| Card | Weight | Pull % |
|---|---:|---:|
| Dark Paladin | 4351 | 4.1585 |
| Reaper on the Nightmare | 4351 | 4.1585 |
| The Last Warrior from Another Planet | 4351 | 4.1585 |
| Darkfire Dragon | 2186 | 2.0893 |
| Dragoness the Wicked Knight | 1962 | 1.8752 |
| Apprentice Magician | 1804 | 1.7242 |
| Asura Priest | 1804 | 1.7242 |
| Dark Magician | 1804 | 1.7242 |
| Dark Ruler Ha Des | 1804 | 1.7242 |
| Don Zaloog | 1804 | 1.7242 |
| Kycoo the Ghost Destroyer | 1804 | 1.7242 |
| Mask of Restrict | 1804 | 1.7242 |
| Nimble Momonga | 1804 | 1.7242 |
| Sasuke Samurai | 1804 | 1.7242 |
| Skilled Dark Magician | 1804 | 1.7242 |
| Spell Canceller | 1804 | 1.7242 |
| Waboku | 1804 | 1.7242 |
| Empress Judge | 1741 | 1.6640 |
| Fiend Skull Dragon | 1741 | 1.6640 |
| Punished Eagle | 1741 | 1.6640 |
| Roaring Ocean Snake | 1741 | 1.6640 |
| Dark Balter the Terrible | 1686 | 1.6114 |
| Ryu Senshi | 1686 | 1.6114 |
| Thousand-Eyes Restrict | 1686 | 1.6114 |
| Creature Swap | 1552 | 1.4833 |
| Reinforcement of the Army | 1552 | 1.4833 |
| Airknight Parshath | 1396 | 1.3342 |
| Goblin Attack Force | 1396 | 1.3342 |
| Jar of Greed | 1396 | 1.3342 |
| Royal Decree | 1396 | 1.3342 |
| Scapegoat | 1396 | 1.3342 |
| Cannon Soldier | 1205 | 1.1517 |
| Fissure | 1205 | 1.1517 |
| Giant Trunade | 1205 | 1.1517 |
| Poison of the Old Man | 1205 | 1.1517 |
| Raigeki Break | 1205 | 1.1517 |
| Reasoning | 1205 | 1.1517 |
| Swords of Revealing Light | 1205 | 1.1517 |
| Upstart Goblin | 1205 | 1.1517 |
| Gilasaurus | 1087 | 1.0389 |
| Magician of Faith | 1087 | 1.0389 |
| Newdoria | 1087 | 1.0389 |
| Gravekeeper's Spy | 1005 | 0.9605 |
| Metamorphosis | 1005 | 0.9605 |
| Trap Dustshoot | 1005 | 0.9605 |
| Cyber Jar | 943 | 0.9013 |
| Dust Tornado | 943 | 0.9013 |
| Last Will | 943 | 0.9013 |
| Spirit Reaper | 943 | 0.9013 |
| Book of Moon | 893 | 0.8535 |
| Catapult Turtle | 893 | 0.8535 |
| King Tiger Wanghu | 893 | 0.8535 |
| Mystic Tomato | 819 | 0.7828 |
| Nobleman of Crossout | 778 | 0.7436 |
| Torrential Tribute | 765 | 0.7312 |
| Card Destruction | 743 | 0.7101 |
| Ceasefire | 743 | 0.7101 |
| Magical Scientist | 743 | 0.7101 |
| Mystical Space Typhoon | 743 | 0.7101 |
| Premature Burial | 650 | 0.6212 |
| Heavy Storm | 618 | 0.5907 |
| Yata-Garasu | 618 | 0.5907 |
| Injection Fairy Lily | 592 | 0.5658 |
| The Forceful Sentry | 592 | 0.5658 |
| Confiscation | 571 | 0.5457 |
| Delinquent Duo | 571 | 0.5457 |
| Exiled Force | 571 | 0.5457 |
| Jinzo | 571 | 0.5457 |
| Sinister Serpent | 571 | 0.5457 |
| Breaker the Magical Warrior | 552 | 0.5276 |
| Mirage of Nightmare | 552 | 0.5276 |
| Call Of The Haunted | 536 | 0.5123 |
| Fiber Jar | 536 | 0.5123 |
| Mirror Force | 536 | 0.5123 |
| Monster Reborn | 536 | 0.5123 |
| Tribe-Infecting Virus | 536 | 0.5123 |
| Bell of Destruction | 522 | 0.4989 |
| Change of Heart | 522 | 0.4989 |
| Dark Hole | 522 | 0.4989 |
| Graceful Charity | 522 | 0.4989 |
| Harpie's Feather Duster | 522 | 0.4989 |
| Imperial Order | 522 | 0.4989 |
| Painful Choice | 522 | 0.4989 |
| Pot of Greed | 522 | 0.4989 |
| Raigeki | 522 | 0.4989 |
| Sangan | 522 | 0.4989 |
| Snatch Steal | 522 | 0.4989 |
| Witch of the Black Forest | 522 | 0.4989 |

### Dark Crisis

- **Type:** TCG · **Format:** Vampire · **Cards in pool:** 8 · **Weight sum:** 53700

| Card | Weight | Pull % |
|---|---:|---:|
| Final Attack Orders | 10000 | 18.6220 |
| Gyaku-Gire Panda | 10000 | 18.6220 |
| Ojama Trio | 10000 | 18.6220 |
| Sakuretsu Armor | 10000 | 18.6220 |
| Token Thanksgiving | 10000 | 18.6220 |
| Tsukuyomi | 2500 | 4.6555 |
| D.D. Warrior Lady | 600 | 1.1173 |
| Dark Flare Knight | 600 | 1.1173 |

### Vampire

- **Type:** Usage · **Format:** Vampire · **Cards in pool:** 110 · **Weight sum:** 176136

| Card | Weight | Pull % |
|---|---:|---:|
| Empress Judge | 5608 | 3.1839 |
| Giltia the D. Knight | 5608 | 3.1839 |
| Skull Knight | 5608 | 3.1839 |
| B. Skull Dragon | 4279 | 2.4294 |
| Flame Ghost | 4279 | 2.4294 |
| Twin-Headed Thunder Dragon | 4279 | 2.4294 |
| Punished Eagle | 3657 | 2.0762 |
| Roaring Ocean Snake | 2509 | 1.4245 |
| The Last Warrior from Another Planet | 2509 | 1.4245 |
| Darkfire Dragon | 2399 | 1.3620 |
| Dragoness the Wicked Knight | 2399 | 1.3620 |
| Book of Life | 2305 | 1.3086 |
| Bottomless Trap Hole | 2305 | 1.3086 |
| Cannon Soldier | 2305 | 1.3086 |
| Catapult Turtle | 2305 | 1.3086 |
| Cipher Soldier | 2305 | 1.3086 |
| Cold Wave | 2305 | 1.3086 |
| Dark Scorpion - Cliff the Trap Remover | 2305 | 1.3086 |
| Des Koala | 2305 | 1.3086 |
| Electric Snake | 2305 | 1.3086 |
| Gemini Elf | 2305 | 1.3086 |
| Goblin Attack Force | 2305 | 1.3086 |
| Guardian Sphinx | 2305 | 1.3086 |
| Gyaku-Gire Panda | 2305 | 1.3086 |
| Last Will | 2305 | 1.3086 |
| Masked Sorcerer | 2305 | 1.3086 |
| Mother Grizzly | 2305 | 1.3086 |
| Ojama Trio | 2305 | 1.3086 |
| Pyramid Turtle | 2305 | 1.3086 |
| Raigeki Break | 2305 | 1.3086 |
| Secret Barrel | 2305 | 1.3086 |
| Spirit Reaper | 2305 | 1.3086 |
| Summoned Skull | 2305 | 1.3086 |
| Time Seal | 2305 | 1.3086 |
| Waboku | 2305 | 1.3086 |
| White Magical Hat | 2305 | 1.3086 |
| Reaper on the Nightmare | 2087 | 1.1849 |
| Creature Swap | 1978 | 1.1230 |
| Marauding Captain | 1978 | 1.1230 |
| Morphing Jar | 1978 | 1.1230 |
| Dark Flare Knight | 1776 | 1.0083 |
| Giant Trunade | 1776 | 1.0083 |
| Jar of Greed | 1776 | 1.0083 |
| Just Desserts | 1776 | 1.0083 |
| Kycoo the Ghost Destroyer | 1776 | 1.0083 |
| Solemn Judgment | 1776 | 1.0083 |
| Trap Hole | 1776 | 1.0083 |
| Fiend Skull Dragon | 1714 | 0.9731 |
| Dark Balter the Terrible | 1686 | 0.9572 |
| Ryu Senshi | 1686 | 0.9572 |
| Thousand-Eyes Restrict | 1686 | 0.9572 |
| Dark Ruler Ha Des | 1528 | 0.8675 |
| Mage Power | 1528 | 0.8675 |
| Poison of the Old Man | 1528 | 0.8675 |
| Swords of Revealing Light | 1528 | 0.8675 |
| Trap Dustshoot | 1528 | 0.8675 |
| Upstart Goblin | 1528 | 0.8675 |
| Mystic Tomato | 1375 | 0.7806 |
| The Little Swordsman of Aile | 1375 | 0.7806 |
| Metamorphosis | 1268 | 0.7199 |
| Scapegoat | 1268 | 0.7199 |
| Card Destruction | 1187 | 0.6739 |
| Magic Cylinder | 1187 | 0.6739 |
| Dust Tornado | 1123 | 0.6376 |
| Sasuke Samurai | 1123 | 0.6376 |
| Cyber Jar | 1027 | 0.5831 |
| King Tiger Wanghu | 1027 | 0.5831 |
| Magician of Faith | 990 | 0.5621 |
| Book of Moon | 957 | 0.5433 |
| Royal Decree | 929 | 0.5274 |
| Sakuretsu Armor | 903 | 0.5127 |
| Ceasefire | 860 | 0.4883 |
| Don Zaloog | 860 | 0.4883 |
| Gravekeeper's Spy | 860 | 0.4883 |
| Zombyra the Dark | 841 | 0.4775 |
| D.D. Warrior Lady | 766 | 0.4349 |
| Mystical Space Typhoon | 743 | 0.4218 |
| Torrential Tribute | 743 | 0.4218 |
| Premature Burial | 681 | 0.3866 |
| Reinforcement of the Army | 681 | 0.3866 |
| Nobleman of Crossout | 670 | 0.3804 |
| Painful Choice | 610 | 0.3463 |
| Yata-Garasu | 584 | 0.3316 |
| Injection Fairy Lily | 563 | 0.3196 |
| Heavy Storm | 554 | 0.3145 |
| Delinquent Duo | 545 | 0.3094 |
| Fiber Jar | 545 | 0.3094 |
| Jinzo | 545 | 0.3094 |
| Call Of The Haunted | 537 | 0.3049 |
| Magical Scientist | 537 | 0.3049 |
| Mirage of Nightmare | 537 | 0.3049 |
| Sinister Serpent | 537 | 0.3049 |
| Tribe-Infecting Virus | 537 | 0.3049 |
| Bell of Destruction | 529 | 0.3003 |
| Confiscation | 529 | 0.3003 |
| Exiled Force | 529 | 0.3003 |
| Imperial Order | 529 | 0.3003 |
| Snatch Steal | 529 | 0.3003 |
| Breaker the Magical Warrior | 522 | 0.2964 |
| Change of Heart | 522 | 0.2964 |
| Dark Hole | 522 | 0.2964 |
| Graceful Charity | 522 | 0.2964 |
| Harpie's Feather Duster | 522 | 0.2964 |
| Mirror Force | 522 | 0.2964 |
| Monster Reborn | 522 | 0.2964 |
| Pot of Greed | 522 | 0.2964 |
| Raigeki | 522 | 0.2964 |
| Sangan | 522 | 0.2964 |
| The Forceful Sentry | 522 | 0.2964 |
| Witch of the Black Forest | 522 | 0.2964 |

### Starter Deck- Yugi Evolution

- **Type:** TCG · **Format:** Chaos · **Cards in pool:** 44 · **Weight sum:** 411300

| Card | Weight | Pull % |
|---|---:|---:|
| Axe of Despair | 10000 | 2.4313 |
| Backup Soldier | 10000 | 2.4313 |
| Beaver Warrior | 10000 | 2.4313 |
| Card Destruction | 10000 | 2.4313 |
| Celtic Guardian | 10000 | 2.4313 |
| Change of Heart | 10000 | 2.4313 |
| Dark Hole | 10000 | 2.4313 |
| Dian Keto the Cure Master | 10000 | 2.4313 |
| Feral Imp | 10000 | 2.4313 |
| Fissure | 10000 | 2.4313 |
| Gaia The Fierce Knight | 10000 | 2.4313 |
| Gazelle the King of Mythical Beasts | 10000 | 2.4313 |
| Giant Rat | 10000 | 2.4313 |
| Giant Soldier of Stone | 10000 | 2.4313 |
| Great White | 10000 | 2.4313 |
| Kuriboh | 10000 | 2.4313 |
| Last Will | 10000 | 2.4313 |
| Magic Jammer | 10000 | 2.4313 |
| Malevolent Nuzzler | 10000 | 2.4313 |
| Man-Eater Bug | 10000 | 2.4313 |
| Monster Reborn | 10000 | 2.4313 |
| Mystic Plasma Zone | 10000 | 2.4313 |
| Mystical Elf | 10000 | 2.4313 |
| Mystical Space Typhoon | 10000 | 2.4313 |
| Neo the Magic Swordsman | 10000 | 2.4313 |
| Pot of Greed | 10000 | 2.4313 |
| Raigeki Break | 10000 | 2.4313 |
| Remove Trap | 10000 | 2.4313 |
| Sangan | 10000 | 2.4313 |
| Seven Tools of the Bandit | 10000 | 2.4313 |
| Sonic Bird | 10000 | 2.4313 |
| Spellbinding Circle | 10000 | 2.4313 |
| Summoned Skull | 10000 | 2.4313 |
| Swords of Revealing Light | 10000 | 2.4313 |
| The Eye of Truth | 10000 | 2.4313 |
| The Reliable Guardian | 10000 | 2.4313 |
| Trap Hole | 10000 | 2.4313 |
| Waboku | 10000 | 2.4313 |
| Wall of Illusion | 10000 | 2.4313 |
| Winged Dragon,Guardian of the Fortress #1 | 10000 | 2.4313 |
| Zombyra the Dark | 10000 | 2.4313 |
| Black Luster Ritual | 600 | 0.1459 |
| Dark Magician | 600 | 0.1459 |
| Black Luster Soldier | 100 | 0.0243 |

### Starter Deck- Kaiba Evolution

- **Type:** TCG · **Format:** Chaos · **Cards in pool:** 36 · **Weight sum:** 360000

| Card | Weight | Pull % |
|---|---:|---:|
| Battle Ox | 10000 | 2.7778 |
| Change of Heart | 10000 | 2.7778 |
| De-Spell | 10000 | 2.7778 |
| Dust Tornado | 10000 | 2.7778 |
| Fissure | 10000 | 2.7778 |
| Gift of The Mystical Elf | 10000 | 2.7778 |
| Hane-Hane | 10000 | 2.7778 |
| Hyozanryu | 10000 | 2.7778 |
| Jar of Greed | 10000 | 2.7778 |
| Just Desserts | 10000 | 2.7778 |
| Kojikocy | 10000 | 2.7778 |
| Koumori Dragon | 10000 | 2.7778 |
| La Jinn the Mystical Genie of the Lamp | 10000 | 2.7778 |
| Light of Intervention | 10000 | 2.7778 |
| Lord of D. | 10000 | 2.7778 |
| Megamorph | 10000 | 2.7778 |
| Monster Reborn | 10000 | 2.7778 |
| Mountain | 10000 | 2.7778 |
| Mysterious Puppeteer | 10000 | 2.7778 |
| Mystic Horseman | 10000 | 2.7778 |
| Mystic Tomato | 10000 | 2.7778 |
| Nobleman of Crossout | 10000 | 2.7778 |
| Ookazi | 10000 | 2.7778 |
| Premature Burial | 10000 | 2.7778 |
| Rogue Doll | 10000 | 2.7778 |
| Rush Recklessly | 10000 | 2.7778 |
| Ryu-Kishin Powered | 10000 | 2.7778 |
| Skull Red Bird | 10000 | 2.7778 |
| Soul Exchange | 10000 | 2.7778 |
| The Flute of Summoning Dragon | 10000 | 2.7778 |
| The Inexperienced Spy | 10000 | 2.7778 |
| Trap Hole | 10000 | 2.7778 |
| Trap Master | 10000 | 2.7778 |
| Tribute to The Doomed | 10000 | 2.7778 |
| Uraby | 10000 | 2.7778 |
| Witch of the Black Forest | 10000 | 2.7778 |

### Invasion of Chaos

- **Type:** TCG · **Format:** Chaos · **Cards in pool:** 14 · **Weight sum:** 67925

| Card | Weight | Pull % |
|---|---:|---:|
| Chaos Sorcerer | 10000 | 14.7221 |
| Gigantes | 10000 | 14.7221 |
| Ryu Kokki | 10000 | 14.7221 |
| Sacred Crane | 10000 | 14.7221 |
| Smashing Ground | 10000 | 14.7221 |
| Stealth Bird | 10000 | 14.7221 |
| Berserk Gorilla | 2500 | 3.6805 |
| Chain Disappearance | 2500 | 3.6805 |
| Compulsory Evacuation Device | 2500 | 3.6805 |
| Black Luster Soldier - Envoy of Beginning | 100 | 0.1472 |
| Dark Magician of Chaos | 100 | 0.1472 |
| Dimension Fusion | 100 | 0.1472 |
| Manticore of Darkness | 100 | 0.1472 |
| Chaos Emperor Dragon - Envoy of the End | 25 | 0.0368 |

### Chaos

- **Type:** Usage · **Format:** Chaos · **Cards in pool:** 137 · **Weight sum:** 297843

| Card | Weight | Pull % |
|---|---:|---:|
| Deepsea Shark | 9465 | 3.1778 |
| Bickuribox | 7202 | 2.4181 |
| Labyrinth Tank | 7202 | 2.4181 |
| Metal Dragon | 7202 | 2.4181 |
| Warrior of Tradition | 7202 | 2.4181 |
| Flame Swordsman | 6142 | 2.0622 |
| Skull Knight | 6142 | 2.0622 |
| Punished Eagle | 5487 | 1.8422 |
| Dark Paladin | 5029 | 1.6885 |
| Empress Judge | 5029 | 1.6885 |
| Karbonala Warrior | 5029 | 1.6885 |
| B. Skull Dragon | 4000 | 1.3430 |
| Fusionist | 4000 | 1.3430 |
| Twin-Headed Thunder Dragon | 4000 | 1.3430 |
| Catapult Turtle | 3840 | 1.2893 |
| Dark Ruler Ha Des | 3840 | 1.2893 |
| Electric Snake | 3840 | 1.2893 |
| Gaia the Dragon Champion | 3840 | 1.2893 |
| Gigantes | 3840 | 1.2893 |
| Goblin Attack Force | 3840 | 1.2893 |
| Hysteric Fairy | 3840 | 1.2893 |
| Kuriboh | 3840 | 1.2893 |
| Magical Merchant | 3840 | 1.2893 |
| Musician King | 3840 | 1.2893 |
| Pyramid Turtle | 3840 | 1.2893 |
| Return from the Different Dimension | 3840 | 1.2893 |
| Spell Canceller | 3840 | 1.2893 |
| The Last Warrior from Another Planet | 3840 | 1.2893 |
| White Magical Hat | 3840 | 1.2893 |
| Zaborg the Thunder Monarch | 3840 | 1.2893 |
| Dragoness the Wicked Knight | 3579 | 1.2016 |
| Flame Ghost | 3579 | 1.2016 |
| Darkfire Dragon | 3203 | 1.0754 |
| Axe of Despair | 2939 | 0.9868 |
| Black Illusion Ritual | 2939 | 0.9868 |
| Enemy Controller | 2939 | 0.9868 |
| Jar of Greed | 2939 | 0.9868 |
| Magic Jammer | 2939 | 0.9868 |
| Masked Sorcerer | 2939 | 0.9868 |
| Relinquished | 2939 | 0.9868 |
| Reaper on the Nightmare | 2786 | 0.9354 |
| Giltia the D. Knight | 2741 | 0.9203 |
| Emergency Provisions | 2517 | 0.8451 |
| Giant Trunade | 2517 | 0.8451 |
| Roulette Barrel | 2517 | 0.8451 |
| Soul Release | 2517 | 0.8451 |
| Spear Dragon | 2517 | 0.8451 |
| The Shallow Grave | 2517 | 0.8451 |
| Prohibition | 2257 | 0.7578 |
| Roaring Ocean Snake | 2257 | 0.7578 |
| Royal Oppression | 2257 | 0.7578 |
| Time Seal | 2257 | 0.7578 |
| 4-Starred Ladybug of Doom | 2074 | 0.6963 |
| Dimension Fusion | 2074 | 0.6963 |
| Mystic Tomato | 2074 | 0.6963 |
| Berserk Gorilla | 1937 | 0.6503 |
| Cipher Soldier | 1937 | 0.6503 |
| Sasuke Samurai | 1937 | 0.6503 |
| Skilled Dark Magician | 1937 | 0.6503 |
| Dark Flare Knight | 1880 | 0.6312 |
| Airknight Parshath | 1828 | 0.6137 |
| Exchange | 1828 | 0.6137 |
| Fiend Skull Dragon | 1782 | 0.5983 |
| Ryu Senshi | 1747 | 0.5866 |
| Bottomless Trap Hole | 1739 | 0.5839 |
| Dark Balter the Terrible | 1739 | 0.5839 |
| Don Zaloog | 1739 | 0.5839 |
| King Tiger Wanghu | 1739 | 0.5839 |
| Thousand-Eyes Restrict | 1739 | 0.5839 |
| Book of Moon | 1665 | 0.5590 |
| Creature Swap | 1665 | 0.5590 |
| Thunder Nyan Nyan | 1665 | 0.5590 |
| Metamorphosis | 1601 | 0.5375 |
| Cold Wave | 1546 | 0.5191 |
| Tsukuyomi | 1497 | 0.5026 |
| Dust Tornado | 1453 | 0.4878 |
| Poison of the Old Man | 1453 | 0.4878 |
| Spirit Reaper | 1453 | 0.4878 |
| Chaos Sorcerer | 1415 | 0.4751 |
| Solemn Judgment | 1415 | 0.4751 |
| Dark Magician of Chaos | 1379 | 0.4630 |
| Swords of Revealing Light | 1379 | 0.4630 |
| Waboku | 1379 | 0.4630 |
| Sakuretsu Armor | 1318 | 0.4425 |
| Royal Decree | 1291 | 0.4334 |
| Scapegoat | 1291 | 0.4334 |
| Asura Priest | 1266 | 0.4251 |
| Ceasefire | 1221 | 0.4099 |
| Zombyra the Dark | 1221 | 0.4099 |
| Magic Cylinder | 1115 | 0.3744 |
| Morphing Jar | 1115 | 0.3744 |
| Raigeki Break | 1074 | 0.3606 |
| Reinforcement of the Army | 1056 | 0.3545 |
| Thunder Dragon | 1039 | 0.3488 |
| Upstart Goblin | 979 | 0.3287 |
| Trap Dustshoot | 971 | 0.3260 |
| Cyber Jar | 874 | 0.2934 |
| Gravekeeper's Spy | 864 | 0.2901 |
| Torrential Tribute | 789 | 0.2649 |
| D.D. Warrior Lady | 778 | 0.2612 |
| Magician of Faith | 778 | 0.2612 |
| Kycoo the Ghost Destroyer | 771 | 0.2589 |
| Exiled Force | 761 | 0.2555 |
| Card Destruction | 752 | 0.2525 |
| Mystical Space Typhoon | 743 | 0.2495 |
| Yata-Garasu | 727 | 0.2441 |
| Injection Fairy Lily | 698 | 0.2344 |
| Nobleman of Crossout | 698 | 0.2344 |
| Magical Scientist | 634 | 0.2129 |
| Jinzo | 630 | 0.2115 |
| Premature Burial | 610 | 0.2048 |
| Heavy Storm | 599 | 0.2011 |
| Fiber Jar | 580 | 0.1947 |
| Call Of The Haunted | 558 | 0.1873 |
| Sinister Serpent | 553 | 0.1857 |
| Mirage of Nightmare | 542 | 0.1820 |
| Confiscation | 540 | 0.1813 |
| Sangan | 540 | 0.1813 |
| Breaker the Magical Warrior | 538 | 0.1806 |
| Delinquent Duo | 538 | 0.1806 |
| Mirror Force | 533 | 0.1790 |
| Snatch Steal | 531 | 0.1783 |
| Dark Hole | 529 | 0.1776 |
| Painful Choice | 529 | 0.1776 |
| Tribe-Infecting Virus | 529 | 0.1776 |
| The Forceful Sentry | 527 | 0.1769 |
| Harpie's Feather Duster | 525 | 0.1763 |
| Witch of the Black Forest | 525 | 0.1763 |
| Bell of Destruction | 524 | 0.1759 |
| Black Luster Soldier - Envoy of Beginning | 524 | 0.1759 |
| Change of Heart | 524 | 0.1759 |
| Chaos Emperor Dragon - Envoy of the End | 524 | 0.1759 |
| Graceful Charity | 522 | 0.1753 |
| Imperial Order | 522 | 0.1753 |
| Monster Reborn | 522 | 0.1753 |
| Pot of Greed | 522 | 0.1753 |
| Raigeki | 522 | 0.1753 |

### Ancient Sanctuary

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 6 · **Weight sum:** 37700

| Card | Weight | Pull % |
|---|---:|---:|
| Monster Gate | 10000 | 26.5252 |
| Mystik Wok | 10000 | 26.5252 |
| Night Assailant | 10000 | 26.5252 |
| Level Limit - Area B | 7000 | 18.5676 |
| Zaborg the Thunder Monarch | 600 | 1.5915 |
| Enemy Controller | 100 | 0.2653 |

### Soul of the Duelist

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 8 · **Weight sum:** 20060

| Card | Weight | Pull % |
|---|---:|---:|
| Dark Mimic LV1 | 10000 | 49.8504 |
| Masked Dragon | 10000 | 49.8504 |
| Hallowed Life Barrier | 10 | 0.0499 |
| Master of Oz | 10 | 0.0499 |
| Mobius the Frost Monarch | 10 | 0.0499 |
| Mystic Swordsman LV2 | 10 | 0.0499 |
| Ninja Grandmaster Sasuke | 10 | 0.0499 |
| Ojama King | 10 | 0.0499 |

### Dark Beginning 1

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 184 · **Weight sum:** 1337200

| Card | Weight | Pull % |
|---|---:|---:|
| 4-Starred Ladybug of Doom | 10000 | 0.7478 |
| Ameba | 10000 | 0.7478 |
| Aqua Madoor | 10000 | 0.7478 |
| Armed Ninja | 10000 | 0.7478 |
| Armored Lizard | 10000 | 0.7478 |
| Armored Zombie | 10000 | 0.7478 |
| Backup Soldier | 10000 | 0.7478 |
| Banisher of the Light | 10000 | 0.7478 |
| Beaver Warrior | 10000 | 0.7478 |
| Big Eye | 10000 | 0.7478 |
| Blast Juggler | 10000 | 0.7478 |
| Celtic Guardian | 10000 | 0.7478 |
| Ceremonial Bell | 10000 | 0.7478 |
| Chain Destruction | 10000 | 0.7478 |
| Chain Energy | 10000 | 0.7478 |
| Chorus of Sanctuary | 10000 | 0.7478 |
| Cocoon of Evolution | 10000 | 0.7478 |
| Confiscation | 10000 | 0.7478 |
| Curse of Dragon | 10000 | 0.7478 |
| DNA Surgery | 10000 | 0.7478 |
| Dark King of the Abyss | 10000 | 0.7478 |
| Dragon Capture Jar | 10000 | 0.7478 |
| Electric Snake | 10000 | 0.7478 |
| Elegant Egotist | 10000 | 0.7478 |
| Enchanted Javelin | 10000 | 0.7478 |
| Eternal Rest | 10000 | 0.7478 |
| Fairy Box | 10000 | 0.7478 |
| Fairy's Hand Mirror | 10000 | 0.7478 |
| Fake Trap | 10000 | 0.7478 |
| Feral Imp | 10000 | 0.7478 |
| Final Destiny | 10000 | 0.7478 |
| Flame Manipulator | 10000 | 0.7478 |
| Flash Assailant | 10000 | 0.7478 |
| Flying Kamakiri #1 | 10000 | 0.7478 |
| Forced Requisition | 10000 | 0.7478 |
| Gaia Power | 10000 | 0.7478 |
| Giant Germ | 10000 | 0.7478 |
| Giant Soldier of Stone | 10000 | 0.7478 |
| Gift of The Mystical Elf | 10000 | 0.7478 |
| Gravekeeper's Servant | 10000 | 0.7478 |
| Graverobber | 10000 | 0.7478 |
| Gravity Bind | 10000 | 0.7478 |
| Griggle | 10000 | 0.7478 |
| Hane-Hane | 10000 | 0.7478 |
| Harpie Lady | 10000 | 0.7478 |
| Harpie Lady Sisters | 10000 | 0.7478 |
| Hinotama | 10000 | 0.7478 |
| Hiro's Shadow Scout | 10000 | 0.7478 |
| Hitotsu-Me Giant | 10000 | 0.7478 |
| Horn of Light | 10000 | 0.7478 |
| Horn of the Unicorn | 10000 | 0.7478 |
| Hyozanryu | 10000 | 0.7478 |
| Hysteric Fairy | 10000 | 0.7478 |
| Invader of the Throne | 10000 | 0.7478 |
| Jinzo #7 | 10000 | 0.7478 |
| Karate Man | 10000 | 0.7478 |
| Kazejin | 10000 | 0.7478 |
| Kiseitai | 10000 | 0.7478 |
| Kojikocy | 10000 | 0.7478 |
| Kotodama | 10000 | 0.7478 |
| Labyrinth Wall | 10000 | 0.7478 |
| Larvae Moth | 10000 | 0.7478 |
| Left Arm of the Forbidden One | 10000 | 0.7478 |
| Left Leg of the Forbidden One | 10000 | 0.7478 |
| Light of Intervention | 10000 | 0.7478 |
| Lightforce Sword | 10000 | 0.7478 |
| Luminous Spark | 10000 | 0.7478 |
| Maha Vailo | 10000 | 0.7478 |
| Malevolent Nuzzler | 10000 | 0.7478 |
| Mammoth Graveyard | 10000 | 0.7478 |
| Manga Ryu-Ran | 10000 | 0.7478 |
| Masaki the Legendary Swordsman | 10000 | 0.7478 |
| Mask of Darkness | 10000 | 0.7478 |
| Mask of Restrict | 10000 | 0.7478 |
| Masked Sorcerer | 10000 | 0.7478 |
| Messenger of Peace | 10000 | 0.7478 |
| Minar | 10000 | 0.7478 |
| Mirror Wall | 10000 | 0.7478 |
| Molten Destruction | 10000 | 0.7478 |
| Mother Grizzly | 10000 | 0.7478 |
| Mystic Lamp | 10000 | 0.7478 |
| Mystic Plasma Zone | 10000 | 0.7478 |
| Mystical Elf | 10000 | 0.7478 |
| Nobleman of Extermination | 10000 | 0.7478 |
| Numinous Healer | 10000 | 0.7478 |
| Painful Choice | 10000 | 0.7478 |
| Parasite Paracide | 10000 | 0.7478 |
| Penguin Knight | 10000 | 0.7478 |
| Petit Moth | 10000 | 0.7478 |
| Polymerization | 10000 | 0.7478 |
| Polymerization | 10000 | 0.7478 |
| Reaper of the Cards | 10000 | 0.7478 |
| Red Archery Girl | 10000 | 0.7478 |
| Red Medicine | 10000 | 0.7478 |
| Respect Play | 10000 | 0.7478 |
| Right Arm of the Forbidden One | 10000 | 0.7478 |
| Right Leg of the Forbidden One | 10000 | 0.7478 |
| Rising Air Current | 10000 | 0.7478 |
| Rush Recklessly | 10000 | 0.7478 |
| Sanga of the Thunder | 10000 | 0.7478 |
| Senju of the Thousand Hands | 10000 | 0.7478 |
| Silver Fang | 10000 | 0.7478 |
| Skull Servant | 10000 | 0.7478 |
| Slot Machine | 10000 | 0.7478 |
| Solemn Wishes | 10000 | 0.7478 |
| Sonic Bird | 10000 | 0.7478 |
| Soul Release | 10000 | 0.7478 |
| Stop Defense | 10000 | 0.7478 |
| Suijin | 10000 | 0.7478 |
| Tailor of the Fickle | 10000 | 0.7478 |
| The Cheerful Coffin | 10000 | 0.7478 |
| The Eye of Truth | 10000 | 0.7478 |
| The Reliable Guardian | 10000 | 0.7478 |
| The Shallow Grave | 10000 | 0.7478 |
| Thousand-Eyes Idol | 10000 | 0.7478 |
| Time Seal | 10000 | 0.7478 |
| Toll | 10000 | 0.7478 |
| Toon Mermaid | 10000 | 0.7478 |
| Toon World | 10000 | 0.7478 |
| UFO Turtle | 10000 | 0.7478 |
| Umi  | 10000 | 0.7478 |
| Umiiruka | 10000 | 0.7478 |
| Upstart Goblin | 10000 | 0.7478 |
| Uraby | 10000 | 0.7478 |
| White Magical Hat | 10000 | 0.7478 |
| Winged Dragon,Guardian of the Fortress #1 | 10000 | 0.7478 |
| Axe of Despair | 2500 | 0.1870 |
| Black Pendant | 2500 | 0.1870 |
| Cyber Jar | 2500 | 0.1870 |
| Dust Tornado | 2500 | 0.1870 |
| Fissure | 2500 | 0.1870 |
| Flame Swordsman | 2500 | 0.1870 |
| Gaia The Fierce Knight | 2500 | 0.1870 |
| Gaia the Dragon Champion | 2500 | 0.1870 |
| Gearfried the Iron Knight | 2500 | 0.1870 |
| Giant Rat | 2500 | 0.1870 |
| Giant Trunade | 2500 | 0.1870 |
| Jar of Greed | 2500 | 0.1870 |
| Limiter Removal | 2500 | 0.1870 |
| Magic Drain | 2500 | 0.1870 |
| Magical Hats | 2500 | 0.1870 |
| Magician of Faith | 2500 | 0.1870 |
| Man-Eater Bug | 2500 | 0.1870 |
| Morphing Jar #2 | 2500 | 0.1870 |
| Mystic Tomato | 2500 | 0.1870 |
| Nimble Momonga | 2500 | 0.1870 |
| Spellbinding Circle | 2500 | 0.1870 |
| The Fiend Megacyber | 2500 | 0.1870 |
| The Forceful Sentry | 2500 | 0.1870 |
| Toon Summoned Skull | 2500 | 0.1870 |
| Trap Hole | 2500 | 0.1870 |
| Tribute to The Doomed | 2500 | 0.1870 |
| B. Skull Dragon | 600 | 0.0449 |
| Blue-Eyes Toon Dragon | 600 | 0.0449 |
| Buster Blader | 600 | 0.0449 |
| Call Of The Haunted | 600 | 0.0449 |
| Ceasefire | 600 | 0.0449 |
| Dark Hole | 600 | 0.0449 |
| Delinquent Duo | 600 | 0.0449 |
| Exchange | 600 | 0.0449 |
| Goblin Attack Force | 600 | 0.0449 |
| Megamorph | 600 | 0.0449 |
| Mystical Space Typhoon | 600 | 0.0449 |
| Nobleman of Crossout | 600 | 0.0449 |
| Pot of Greed | 600 | 0.0449 |
| Premature Burial | 600 | 0.0449 |
| Red-Eyes B. Dragon | 600 | 0.0449 |
| Relinquished | 600 | 0.0449 |
| Summoned Skull | 600 | 0.0449 |
| Swords of Revealing Light | 600 | 0.0449 |
| Big Shield Gardna | 100 | 0.0075 |
| Change of Heart | 100 | 0.0075 |
| Dark Magician | 100 | 0.0075 |
| Dark Sage | 100 | 0.0075 |
| Exodia the Forbidden One | 100 | 0.0075 |
| Imperial Order | 100 | 0.0075 |
| Jinzo | 100 | 0.0075 |
| Mage Power | 100 | 0.0075 |
| Monster Reborn | 100 | 0.0075 |
| Raigeki | 100 | 0.0075 |
| Snatch Steal | 100 | 0.0075 |
| Thousand-Eyes Restrict | 100 | 0.0075 |
| Torrential Tribute | 100 | 0.0075 |
| United We Stand | 100 | 0.0075 |

### Rise of Destiny

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 7 · **Weight sum:** 10060

| Card | Weight | Pull % |
|---|---:|---:|
| Xing Zhen Hu | 10000 | 99.4036 |
| Dark Blade the Dragon Knight | 10 | 0.0994 |
| Dekoichi the Battlechanted Locomotive | 10 | 0.0994 |
| Divine Wrath | 10 | 0.0994 |
| Monster Reincarnation | 10 | 0.0994 |
| Pikeru's Circle of Enchantment | 10 | 0.0994 |
| Thestalos the Firestorm Monarch | 10 | 0.0994 |

### Structure Deck- Dragon's Roar

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 12 · **Weight sum:** 120000

| Card | Weight | Pull % |
|---|---:|---:|
| Call Of The Haunted | 10000 | 8.3333 |
| Ceasefire | 10000 | 8.3333 |
| Creature Swap | 10000 | 8.3333 |
| Heavy Storm | 10000 | 8.3333 |
| Masked Dragon | 10000 | 8.3333 |
| Mystical Space Typhoon | 10000 | 8.3333 |
| Nobleman of Crossout | 10000 | 8.3333 |
| Pot of Greed | 10000 | 8.3333 |
| Premature Burial | 10000 | 8.3333 |
| Red-Eyes B. Dragon | 10000 | 8.3333 |
| Snatch Steal | 10000 | 8.3333 |
| Swords of Revealing Light | 10000 | 8.3333 |

### Structure Deck- Zombie Madness

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 15 · **Weight sum:** 150000

| Card | Weight | Pull % |
|---|---:|---:|
| Book of Life | 10000 | 6.6667 |
| Compulsory Evacuation Device | 10000 | 6.6667 |
| Creature Swap | 10000 | 6.6667 |
| Dust Tornado | 10000 | 6.6667 |
| Giant Trunade | 10000 | 6.6667 |
| Heavy Storm | 10000 | 6.6667 |
| Magic Jammer | 10000 | 6.6667 |
| Mystical Space Typhoon | 10000 | 6.6667 |
| Nobleman of Crossout | 10000 | 6.6667 |
| Pot of Greed | 10000 | 6.6667 |
| Pyramid Turtle | 10000 | 6.6667 |
| Ryu Kokki | 10000 | 6.6667 |
| Snatch Steal | 10000 | 6.6667 |
| Spirit Reaper | 10000 | 6.6667 |
| Torrential Tribute | 10000 | 6.6667 |

### Warrior

- **Type:** Usage · **Format:** Warrior · **Cards in pool:** 180 · **Weight sum:** 399512

| Card | Weight | Pull % |
|---|---:|---:|
| Cyber Saurus | 8973 | 2.2460 |
| Fusionist | 8973 | 2.2460 |
| Gaia the Dragon Champion | 8973 | 2.2460 |
| Labyrinth Tank | 8973 | 2.2460 |
| Rabid Horseman | 8973 | 2.2460 |
| Dark Paladin | 6829 | 1.7093 |
| Charubin the Fire Knight | 5825 | 1.4580 |
| Flame Swordsman | 5205 | 1.3028 |
| Karbonala Warrior | 5205 | 1.3028 |
| Empress Judge | 4770 | 1.1940 |
| Musician King | 4185 | 1.0475 |
| Punished Eagle | 4185 | 1.0475 |
| Twin-Headed Thunder Dragon | 3973 | 0.9945 |
| Axe of Despair | 3644 | 0.9121 |
| Buster Blader | 3644 | 0.9121 |
| Chain Energy | 3644 | 0.9121 |
| Cyber-Stein | 3644 | 0.9121 |
| Different Dimension Capsule | 3644 | 0.9121 |
| Exile of the Wicked | 3644 | 0.9121 |
| Fairy Box | 3644 | 0.9121 |
| Gaia Power | 3644 | 0.9121 |
| Greenkappa | 3644 | 0.9121 |
| Light of Intervention | 3644 | 0.9121 |
| Mask of Restrict | 3644 | 0.9121 |
| Mirror Wall | 3644 | 0.9121 |
| Mother Grizzly | 3644 | 0.9121 |
| Night Assailant | 3644 | 0.9121 |
| Princess of Tsurugi | 3644 | 0.9121 |
| Prohibition | 3644 | 0.9121 |
| Pyramid Turtle | 3644 | 0.9121 |
| Robbin' Goblin | 3644 | 0.9121 |
| Seven Tools of the Bandit | 3644 | 0.9121 |
| Soul Exchange | 3644 | 0.9121 |
| Stealth Bird | 3644 | 0.9121 |
| The Fiend Megacyber | 3644 | 0.9121 |
| Windstorm of Etaqua | 3644 | 0.9121 |
| Flame Ghost | 3117 | 0.7802 |
| Marauding Captain | 3117 | 0.7802 |
| Big Shield Gardna | 2791 | 0.6986 |
| Blast with Chain | 2791 | 0.6986 |
| Chain Disappearance | 2791 | 0.6986 |
| Dark Magician | 2791 | 0.6986 |
| Dark Scorpion - Cliff the Trap Remover | 2791 | 0.6986 |
| Des Koala | 2791 | 0.6986 |
| Exchange | 2791 | 0.6986 |
| Final Attack Orders | 2791 | 0.6986 |
| Gearfried the Iron Knight | 2791 | 0.6986 |
| Magical Merchant | 2791 | 0.6986 |
| Newdoria | 2791 | 0.6986 |
| Nimble Momonga | 2791 | 0.6986 |
| Nobleman of Extermination | 2791 | 0.6986 |
| Poison of the Old Man | 2791 | 0.6986 |
| Royal Oppression | 2791 | 0.6986 |
| Trap Hole | 2791 | 0.6986 |
| Master of Oz | 2739 | 0.6856 |
| Roaring Ocean Snake | 2691 | 0.6736 |
| Giltia the D. Knight | 2603 | 0.6515 |
| Apprentice Magician | 2391 | 0.5985 |
| Gravity Bind | 2391 | 0.5985 |
| Magic Jammer | 2391 | 0.5985 |
| Messenger of Peace | 2391 | 0.5985 |
| Mystic Tomato | 2391 | 0.5985 |
| Sasuke Samurai | 2391 | 0.5985 |
| Spear Dragon | 2391 | 0.5985 |
| Spell Canceller | 2391 | 0.5985 |
| Wave-Motion Cannon | 2391 | 0.5985 |
| The Last Warrior from Another Planet | 2361 | 0.5910 |
| Dragoness the Wicked Knight | 2280 | 0.5707 |
| Cannon Soldier | 2144 | 0.5367 |
| Gilasaurus | 2144 | 0.5367 |
| Goblin Attack Force | 2144 | 0.5367 |
| Just Desserts | 2144 | 0.5367 |
| Kuriboh | 2144 | 0.5367 |
| Last Will | 2144 | 0.5367 |
| Level Limit - Area B | 2144 | 0.5367 |
| Mask of Darkness | 2144 | 0.5367 |
| Masked Sorcerer | 2144 | 0.5367 |
| Mystic Plasma Zone | 2144 | 0.5367 |
| Ojama Trio | 2144 | 0.5367 |
| Secret Barrel | 2144 | 0.5367 |
| Waboku | 2144 | 0.5367 |
| Ojama King | 2124 | 0.5316 |
| Darkfire Dragon | 2105 | 0.5269 |
| Dark Flare Knight | 2018 | 0.5051 |
| Abyss Soldier | 1971 | 0.4934 |
| Catapult Turtle | 1971 | 0.4934 |
| Compulsory Evacuation Device | 1971 | 0.4934 |
| Gigantes | 1971 | 0.4934 |
| Reaper on the Nightmare | 1971 | 0.4934 |
| Sacred Crane | 1971 | 0.4934 |
| Skilled Dark Magician | 1971 | 0.4934 |
| Token Thanksgiving | 1971 | 0.4934 |
| Xing Zhen Hu | 1971 | 0.4934 |
| Fiend Skull Dragon | 1877 | 0.4698 |
| Dekoichi the Battlechanted Locomotive | 1841 | 0.4608 |
| Dimension Fusion | 1841 | 0.4608 |
| Divine Wrath | 1841 | 0.4608 |
| Gyaku-Gire Panda | 1841 | 0.4608 |
| Magic Drain | 1841 | 0.4608 |
| Widespread Ruin | 1841 | 0.4608 |
| Dark Blade the Dragon Knight | 1797 | 0.4498 |
| Thousand-Eyes Restrict | 1797 | 0.4498 |
| Ryu Senshi | 1777 | 0.4448 |
| Dark Balter the Terrible | 1767 | 0.4423 |
| Giant Rat | 1738 | 0.4350 |
| Hallowed Life Barrier | 1738 | 0.4350 |
| Zaborg the Thunder Monarch | 1738 | 0.4350 |
| Reasoning | 1583 | 0.3962 |
| Monster Gate | 1523 | 0.3812 |
| Monster Reincarnation | 1523 | 0.3812 |
| Pikeru's Circle of Enchantment | 1523 | 0.3812 |
| Return from the Different Dimension | 1424 | 0.3564 |
| Tsukuyomi | 1424 | 0.3564 |
| Jar of Greed | 1383 | 0.3462 |
| Chaos Sorcerer | 1346 | 0.3369 |
| Spirit Reaper | 1346 | 0.3369 |
| D.D. Assailant | 1313 | 0.3287 |
| Asura Priest | 1283 | 0.3211 |
| Ninja Grandmaster Sasuke | 1283 | 0.3211 |
| Zombyra the Dark | 1283 | 0.3211 |
| Berserk Gorilla | 1255 | 0.3141 |
| Mystik Wok | 1229 | 0.3076 |
| Enemy Controller | 1205 | 0.3016 |
| Giant Trunade | 1205 | 0.3016 |
| Metamorphosis | 1205 | 0.3016 |
| Raigeki Break | 1163 | 0.2911 |
| Gravekeeper's Spy | 1144 | 0.2863 |
| Royal Decree | 1144 | 0.2863 |
| Solemn Judgment | 1144 | 0.2863 |
| Trap Dustshoot | 1144 | 0.2863 |
| Cyber Jar | 1109 | 0.2776 |
| Emergency Provisions | 1093 | 0.2736 |
| King Tiger Wanghu | 1063 | 0.2661 |
| Airknight Parshath | 1049 | 0.2626 |
| Bottomless Trap Hole | 1049 | 0.2626 |
| Don Zaloog | 1049 | 0.2626 |
| Mystic Swordsman LV2 | 1012 | 0.2533 |
| Morphing Jar | 990 | 0.2478 |
| Magician of Faith | 980 | 0.2453 |
| Upstart Goblin | 961 | 0.2405 |
| Creature Swap | 947 | 0.2370 |
| Blade Knight | 934 | 0.2338 |
| Card Destruction | 934 | 0.2338 |
| Kycoo the Ghost Destroyer | 903 | 0.2260 |
| Mobius the Frost Monarch | 896 | 0.2243 |
| Scapegoat | 896 | 0.2243 |
| Smashing Ground | 896 | 0.2243 |
| Cipher Soldier | 869 | 0.2175 |
| Book of Moon | 863 | 0.2160 |
| Dust Tornado | 857 | 0.2145 |
| Sakuretsu Armor | 857 | 0.2145 |
| Ceasefire | 851 | 0.2130 |
| Injection Fairy Lily | 835 | 0.2090 |
| Magic Cylinder | 819 | 0.2050 |
| D.D. Warrior Lady | 815 | 0.2040 |
| Dark Magician of Chaos | 805 | 0.2015 |
| Reinforcement of the Army | 805 | 0.2015 |
| Nobleman of Crossout | 681 | 0.1705 |
| Mirage of Nightmare | 674 | 0.1687 |
| Swords of Revealing Light | 662 | 0.1657 |
| Fiber Jar | 639 | 0.1599 |
| Sinister Serpent | 634 | 0.1587 |
| Exiled Force | 624 | 0.1562 |
| Jinzo | 603 | 0.1509 |
| Magical Scientist | 595 | 0.1489 |
| Black Luster Soldier - Envoy of Beginning | 565 | 0.1414 |
| Painful Choice | 556 | 0.1392 |
| Call Of The Haunted | 553 | 0.1384 |
| Confiscation | 550 | 0.1377 |
| Tribe-Infecting Virus | 548 | 0.1372 |
| The Forceful Sentry | 545 | 0.1364 |
| Breaker the Magical Warrior | 542 | 0.1357 |
| Torrential Tribute | 542 | 0.1357 |
| Premature Burial | 540 | 0.1352 |
| Snatch Steal | 535 | 0.1339 |
| Change of Heart | 533 | 0.1334 |
| Mystical Space Typhoon | 533 | 0.1334 |
| Bell of Destruction | 528 | 0.1322 |
| Heavy Storm | 526 | 0.1317 |
| Pot of Greed | 522 | 0.1307 |

### Flaming Eternity

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 7 · **Weight sum:** 20050

| Card | Weight | Pull % |
|---|---:|---:|
| Golem Sentry | 10000 | 49.8753 |
| Threatening Roar | 10000 | 49.8753 |
| Chiron the Mage | 10 | 0.0499 |
| Gatling Dragon | 10 | 0.0499 |
| King Dragun | 10 | 0.0499 |
| Lightning Vortex | 10 | 0.0499 |
| Phoenix Wing Wind Blast | 10 | 0.0499 |

### Dark Revelation Volume 1

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 29 · **Weight sum:** 189000

| Card | Weight | Pull % |
|---|---:|---:|
| Apprentice Magician | 10000 | 5.2910 |
| Different Dimension Capsule | 10000 | 5.2910 |
| Final Attack Orders | 10000 | 5.2910 |
| Gravekeeper's Spy | 10000 | 5.2910 |
| Gyaku-Gire Panda | 10000 | 5.2910 |
| Magical Merchant | 10000 | 5.2910 |
| Metamorphosis | 10000 | 5.2910 |
| Ojama Trio | 10000 | 5.2910 |
| Poison of the Old Man | 10000 | 5.2910 |
| Raigeki Break | 10000 | 5.2910 |
| Reasoning | 10000 | 5.2910 |
| Roulette Barrel | 10000 | 5.2910 |
| Sakuretsu Armor | 10000 | 5.2910 |
| Secret Barrel | 10000 | 5.2910 |
| Token Thanksgiving | 10000 | 5.2910 |
| Tsukuyomi | 10000 | 5.2910 |
| Wave-Motion Cannon | 10000 | 5.2910 |
| Des Koala | 2500 | 1.3228 |
| Magical Scientist | 2500 | 1.3228 |
| Newdoria | 2500 | 1.3228 |
| Reaper on the Nightmare | 2500 | 1.3228 |
| Skilled Dark Magician | 2500 | 1.3228 |
| Spirit Reaper | 2500 | 1.3228 |
| Tribe-Infecting Virus | 2500 | 1.3228 |
| D.D. Warrior Lady | 600 | 0.3175 |
| Dark Flare Knight | 600 | 0.3175 |
| Breaker the Magical Warrior | 100 | 0.0529 |
| Dark Paladin | 100 | 0.0529 |
| Spell Canceller | 100 | 0.0529 |

### Structure Deck- Blaze of Destruction

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 15 · **Weight sum:** 150000

| Card | Weight | Pull % |
|---|---:|---:|
| Call Of The Haunted | 10000 | 6.6667 |
| Dust Tornado | 10000 | 6.6667 |
| Heavy Storm | 10000 | 6.6667 |
| Jar of Greed | 10000 | 6.6667 |
| Level Limit - Area B | 10000 | 6.6667 |
| Little Chimera | 10000 | 6.6667 |
| Molten Destruction | 10000 | 6.6667 |
| Mystical Space Typhoon | 10000 | 6.6667 |
| Nobleman of Crossout | 10000 | 6.6667 |
| Pot of Greed | 10000 | 6.6667 |
| Premature Burial | 10000 | 6.6667 |
| Snatch Steal | 10000 | 6.6667 |
| Thestalos the Firestorm Monarch | 10000 | 6.6667 |
| Tribute to The Doomed | 10000 | 6.6667 |
| UFO Turtle | 10000 | 6.6667 |

### Structure Deck- Fury from the Deep

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 16 · **Weight sum:** 160000

| Card | Weight | Pull % |
|---|---:|---:|
| 7 Colored Fish | 10000 | 6.2500 |
| Call Of The Haunted | 10000 | 6.2500 |
| Creature Swap | 10000 | 6.2500 |
| Dust Tornado | 10000 | 6.2500 |
| Gravity Bind | 10000 | 6.2500 |
| Heavy Storm | 10000 | 6.2500 |
| Mobius the Frost Monarch | 10000 | 6.2500 |
| Mother Grizzly | 10000 | 6.2500 |
| Mystical Space Typhoon | 10000 | 6.2500 |
| Pot of Greed | 10000 | 6.2500 |
| Premature Burial | 10000 | 6.2500 |
| Snatch Steal | 10000 | 6.2500 |
| Star Boy | 10000 | 6.2500 |
| Torrential Tribute | 10000 | 6.2500 |
| Tribe-Infecting Virus | 10000 | 6.2500 |
| Xing Zhen Hu | 10000 | 6.2500 |

### The Lost Millennium

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 3 · **Weight sum:** 10020

| Card | Weight | Pull % |
|---|---:|---:|
| Des Wombat | 10000 | 99.8004 |
| Brain Control | 10 | 0.0998 |
| D.D. Survivor | 10 | 0.0998 |

### Dark Beginning 2

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 112 · **Weight sum:** 727800

| Card | Weight | Pull % |
|---|---:|---:|
| Asura Priest | 10000 | 1.3740 |
| Axe Raider | 10000 | 1.3740 |
| Baby Dragon | 10000 | 1.3740 |
| Battle Steer | 10000 | 1.3740 |
| Black Illusion Ritual | 10000 | 1.3740 |
| Blackland Fire Dragon | 10000 | 1.3740 |
| Bladefly | 10000 | 1.3740 |
| Block Attack | 10000 | 1.3740 |
| Book of Moon | 10000 | 1.3740 |
| Bottomless Trap Hole | 10000 | 1.3740 |
| Crass Clown | 10000 | 1.3740 |
| Dark-Piercing Light | 10000 | 1.3740 |
| Dragon Piper | 10000 | 1.3740 |
| Dragon Seeker | 10000 | 1.3740 |
| Dream Clown | 10000 | 1.3740 |
| Exiled Force | 10000 | 1.3740 |
| Fusion Sage | 10000 | 1.3740 |
| Gazelle the King of Mythical Beasts | 10000 | 1.3740 |
| Gemini Elf | 10000 | 1.3740 |
| Gilasaurus | 10000 | 1.3740 |
| Goddess with the Third Eye | 10000 | 1.3740 |
| Great Moth | 10000 | 1.3740 |
| Greenkappa | 10000 | 1.3740 |
| Hoshiningen | 10000 | 1.3740 |
| Illusionist Faceless Mage | 10000 | 1.3740 |
| Jirai Gumo | 10000 | 1.3740 |
| King Tiger Wanghu | 10000 | 1.3740 |
| King of Yamimakai | 10000 | 1.3740 |
| Launcher Spider | 10000 | 1.3740 |
| Little Chimera | 10000 | 1.3740 |
| Lord of the Lamp | 10000 | 1.3740 |
| Machine King | 10000 | 1.3740 |
| Mechanicalchaser | 10000 | 1.3740 |
| Milus Radiant | 10000 | 1.3740 |
| Mirage of Nightmare | 10000 | 1.3740 |
| Muka Muka | 10000 | 1.3740 |
| Mystic Horseman | 10000 | 1.3740 |
| Needle Worm | 10000 | 1.3740 |
| Princess of Tsurugi | 10000 | 1.3740 |
| Rabid Horseman | 10000 | 1.3740 |
| Restructer Revolution | 10000 | 1.3740 |
| Robbin' Goblin | 10000 | 1.3740 |
| Ryu-Kishin Powered | 10000 | 1.3740 |
| Saggi the Dark Clown | 10000 | 1.3740 |
| Shadow Ghoul | 10000 | 1.3740 |
| Share the Pain | 10000 | 1.3740 |
| Shield & Sword | 10000 | 1.3740 |
| Skull Lair | 10000 | 1.3740 |
| Soul of the Pure | 10000 | 1.3740 |
| Star Boy | 10000 | 1.3740 |
| The Bistro Butcher | 10000 | 1.3740 |
| The Immortal of Thunder | 10000 | 1.3740 |
| The Unhappy Maiden | 10000 | 1.3740 |
| The Wicked Worm Beast | 10000 | 1.3740 |
| Thousand Dragon | 10000 | 1.3740 |
| Thunder Dragon | 10000 | 1.3740 |
| Tiger Axe | 10000 | 1.3740 |
| Toon Cannon Soldier | 10000 | 1.3740 |
| Toon Table of Contents | 10000 | 1.3740 |
| Trap Dustshoot | 10000 | 1.3740 |
| Tremendous Fire | 10000 | 1.3740 |
| Twin-Headed Thunder Dragon | 10000 | 1.3740 |
| Two-Headed King Rex | 10000 | 1.3740 |
| Witch's Apprentice | 10000 | 1.3740 |
| Yata-Garasu | 10000 | 1.3740 |
| Zombyra the Dark | 10000 | 1.3740 |
| Bazoo the Soul-Eater | 2500 | 0.3435 |
| Blast with Chain | 2500 | 0.3435 |
| Cyber-Stein | 2500 | 0.3435 |
| Dark Balter the Terrible | 2500 | 0.3435 |
| Emergency Provisions | 2500 | 0.3435 |
| Fiber Jar | 2500 | 0.3435 |
| Fiend Skull Dragon | 2500 | 0.3435 |
| Graceful Charity | 2500 | 0.3435 |
| Horn of Heaven | 2500 | 0.3435 |
| Kuriboh | 2500 | 0.3435 |
| Kycoo the Ghost Destroyer | 2500 | 0.3435 |
| Marauding Captain | 2500 | 0.3435 |
| Morphing Jar | 2500 | 0.3435 |
| Penguin Soldier | 2500 | 0.3435 |
| Pyramid Turtle | 2500 | 0.3435 |
| Reinforcement of the Army | 2500 | 0.3435 |
| Ring of Magnetism | 2500 | 0.3435 |
| Royal Oppression | 2500 | 0.3435 |
| Ryu Senshi | 2500 | 0.3435 |
| Sangan | 2500 | 0.3435 |
| Sasuke Samurai | 2500 | 0.3435 |
| Thunder Nyan Nyan | 2500 | 0.3435 |
| Witch of the Black Forest | 2500 | 0.3435 |
| Airknight Parshath | 600 | 0.0824 |
| Book of Life | 600 | 0.0824 |
| Cannon Soldier | 600 | 0.0824 |
| Catapult Turtle | 600 | 0.0824 |
| Creature Swap | 600 | 0.0824 |
| Don Zaloog | 600 | 0.0824 |
| Heavy Storm | 600 | 0.0824 |
| Magic Jammer | 600 | 0.0824 |
| Mirror Force | 600 | 0.0824 |
| Royal Decree | 600 | 0.0824 |
| Seven Tools of the Bandit | 600 | 0.0824 |
| Solemn Judgment | 600 | 0.0824 |
| Spear Dragon | 600 | 0.0824 |
| The Last Warrior from Another Planet | 600 | 0.0824 |
| Time Wizard | 600 | 0.0824 |
| Total Defense Shogun | 600 | 0.0824 |
| Barrel Dragon | 100 | 0.0137 |
| Blade Knight | 100 | 0.0137 |
| Dark Ruler Ha Des | 100 | 0.0137 |
| Guardian Sphinx | 100 | 0.0137 |
| Injection Fairy Lily | 100 | 0.0137 |
| Insect Queen | 100 | 0.0137 |
| Perfectly Ultimate Great Moth | 100 | 0.0137 |

### Dark Revelation Volume 2

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 20 · **Weight sum:** 78300

| Card | Weight | Pull % |
|---|---:|---:|
| Chaos Sorcerer | 10000 | 12.7714 |
| Gigantes | 10000 | 12.7714 |
| Monster Gate | 10000 | 12.7714 |
| Mystik Wok | 10000 | 12.7714 |
| Sacred Crane | 10000 | 12.7714 |
| Smashing Ground | 10000 | 12.7714 |
| Berserk Gorilla | 2500 | 3.1928 |
| Chain Disappearance | 2500 | 3.1928 |
| Compulsory Evacuation Device | 2500 | 3.1928 |
| Level Limit - Area B | 2500 | 3.1928 |
| Night Assailant | 2500 | 3.1928 |
| Stealth Bird | 2500 | 3.1928 |
| Dimension Fusion | 600 | 0.7663 |
| Enemy Controller | 600 | 0.7663 |
| Manticore of Darkness | 600 | 0.7663 |
| Ryu Kokki | 600 | 0.7663 |
| Zaborg the Thunder Monarch | 600 | 0.7663 |
| Black Luster Soldier - Envoy of Beginning | 100 | 0.1277 |
| Chaos Emperor Dragon - Envoy of the End | 100 | 0.1277 |
| Dark Magician of Chaos | 100 | 0.1277 |

### Structure Deck- Warrior's Triumph

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 17 · **Weight sum:** 170000

| Card | Weight | Pull % |
|---|---:|---:|
| Blast with Chain | 10000 | 5.8824 |
| Call Of The Haunted | 10000 | 5.8824 |
| D.D. Warrior Lady | 10000 | 5.8824 |
| Exiled Force | 10000 | 5.8824 |
| Gearfried the Iron Knight | 10000 | 5.8824 |
| Giant Trunade | 10000 | 5.8824 |
| Goblin Attack Force | 10000 | 5.8824 |
| Heavy Storm | 10000 | 5.8824 |
| Lightning Vortex | 10000 | 5.8824 |
| Magic Jammer | 10000 | 5.8824 |
| Marauding Captain | 10000 | 5.8824 |
| Mystic Swordsman LV2 | 10000 | 5.8824 |
| Mystical Space Typhoon | 10000 | 5.8824 |
| Ninja Grandmaster Sasuke | 10000 | 5.8824 |
| Reinforcement of the Army | 10000 | 5.8824 |
| Royal Decree | 10000 | 5.8824 |
| Snatch Steal | 10000 | 5.8824 |

### Goat

- **Type:** Usage · **Format:** Goat · **Cards in pool:** 252 · **Weight sum:** 1182096

| Card | Weight | Pull % |
|---|---:|---:|
| Flower Wolf | 33082 | 2.7986 |
| Kaminari Attack | 25100 | 2.1233 |
| Rabid Horseman | 21360 | 1.8070 |
| Kwagar Hercules | 17435 | 1.4749 |
| Empress Judge | 14467 | 1.2238 |
| Backup Soldier | 13242 | 1.1202 |
| Catapult Turtle | 13242 | 1.1202 |
| Dark Magician | 13242 | 1.1202 |
| Dark Ruler Ha Des | 13242 | 1.1202 |
| Dragon Zombie | 13242 | 1.1202 |
| Gaia Power | 13242 | 1.1202 |
| Giant Germ | 13242 | 1.1202 |
| Graverobber | 13242 | 1.1202 |
| Hiro's Shadow Scout | 13242 | 1.1202 |
| Kuwagata a | 13242 | 1.1202 |
| Lightforce Sword | 13242 | 1.1202 |
| Minar | 13242 | 1.1202 |
| Restructer Revolution | 13242 | 1.1202 |
| Sonic Bird | 13242 | 1.1202 |
| Thunder Nyan Nyan | 13242 | 1.1202 |
| Two-Pronged Attack | 13242 | 1.1202 |
| Ultimate Offering | 13242 | 1.1202 |
| Skull Knight | 11935 | 1.0096 |
| Warrior of Tradition | 11935 | 1.0096 |
| Deepsea Shark | 11590 | 0.9805 |
| Metal Dragon | 11590 | 0.9805 |
| Gaia the Dragon Champion | 10993 | 0.9300 |
| Labyrinth Tank | 10733 | 0.9080 |
| Giant Soldier of Stone | 10065 | 0.8515 |
| Gilasaurus | 10065 | 0.8515 |
| Guardian Sphinx | 10065 | 0.8515 |
| Mother Grizzly | 10065 | 0.8515 |
| Pikeru's Circle of Enchantment | 10065 | 0.8515 |
| Rush Recklessly | 10065 | 0.8515 |
| The Fiend Megacyber | 10065 | 0.8515 |
| Different Dimension Capsule | 8576 | 0.7255 |
| Dream Clown | 8576 | 0.7255 |
| Exodia the Forbidden One | 8576 | 0.7255 |
| Left Arm of the Forbidden One | 8576 | 0.7255 |
| Left Leg of the Forbidden One | 8576 | 0.7255 |
| Right Arm of the Forbidden One | 8576 | 0.7255 |
| Right Leg of the Forbidden One | 8576 | 0.7255 |
| The Little Swordsman of Aile | 8576 | 0.7255 |
| The Wicked Worm Beast | 8576 | 0.7255 |
| Windstorm of Etaqua | 8576 | 0.7255 |
| Flame Swordsman | 8360 | 0.7072 |
| Musician King | 8360 | 0.7072 |
| Fusionist | 7896 | 0.6680 |
| D.D. Survivor | 7657 | 0.6477 |
| Mask of Restrict | 7657 | 0.6477 |
| Mirror Wall | 7657 | 0.6477 |
| Spear Dragon | 7657 | 0.6477 |
| Time Wizard | 7657 | 0.6477 |
| Bickuribox | 7442 | 0.6296 |
| Cyber Saurus | 7375 | 0.6239 |
| Marauding Captain | 7310 | 0.6184 |
| Axe of Despair | 7013 | 0.5933 |
| Black Pendant | 7013 | 0.5933 |
| Morphing Jar #2 | 7013 | 0.5933 |
| Robbin' Goblin | 7013 | 0.5933 |
| Solemn Wishes | 7013 | 0.5933 |
| The Shallow Grave | 7013 | 0.5933 |
| Trap Hole | 7013 | 0.5933 |
| Dark Paladin | 6906 | 0.5842 |
| Roaring Ocean Snake | 6804 | 0.5756 |
| Charubin the Fire Knight | 6572 | 0.5560 |
| Acid Trap Hole | 6528 | 0.5522 |
| Exchange | 6528 | 0.5522 |
| Nimble Momonga | 6528 | 0.5522 |
| Toon World | 6528 | 0.5522 |
| Trap Master | 6528 | 0.5522 |
| Tribute to The Doomed | 6528 | 0.5522 |
| Thousand Dragon | 6405 | 0.5418 |
| B. Skull Dragon | 6365 | 0.5385 |
| Karbonala Warrior | 6289 | 0.5320 |
| Electric Snake | 6145 | 0.5198 |
| Final Attack Orders | 6145 | 0.5198 |
| Limiter Removal | 6145 | 0.5198 |
| Penguin Soldier | 6145 | 0.5198 |
| Shield & Sword | 6145 | 0.5198 |
| Stealth Bird | 6145 | 0.5198 |
| Banisher of the Light | 5832 | 0.4934 |
| Prohibition | 5832 | 0.4934 |
| Fake Trap | 5569 | 0.4711 |
| Light of Intervention | 5569 | 0.4711 |
| Thestalos the Firestorm Monarch | 5569 | 0.4711 |
| Blast with Chain | 5344 | 0.4521 |
| Divine Wrath | 5344 | 0.4521 |
| Gearfried the Iron Knight | 5344 | 0.4521 |
| Skull Lair | 5344 | 0.4521 |
| Senju of the Thousand Hands | 5149 | 0.4356 |
| Emergency Provisions | 4753 | 0.4021 |
| Book of Life | 4345 | 0.3676 |
| Greenkappa | 4345 | 0.3676 |
| Jirai Gumo | 4345 | 0.3676 |
| Black Illusion Ritual | 4250 | 0.3595 |
| Magic Drain | 4250 | 0.3595 |
| Relinquished | 4250 | 0.3595 |
| Ryu Kokki | 4250 | 0.3595 |
| Dark Flare Knight | 4240 | 0.3587 |
| Giltia the D. Knight | 4196 | 0.3550 |
| Chain Disappearance | 4079 | 0.3451 |
| Cold Wave | 4003 | 0.3386 |
| Megamorph | 3864 | 0.3269 |
| Golem Sentry | 3801 | 0.3215 |
| Pyramid Turtle | 3741 | 0.3165 |
| Toon Cannon Soldier | 3741 | 0.3165 |
| Soul Exchange | 3685 | 0.3117 |
| Cannon Soldier | 3631 | 0.3072 |
| Royal Oppression | 3581 | 0.3029 |
| Milus Radiant | 3532 | 0.2988 |
| Time Seal | 3532 | 0.2988 |
| Poison of the Old Man | 3443 | 0.2913 |
| Toon Table of Contents | 3401 | 0.2877 |
| Bazoo the Soul-Eater | 3360 | 0.2842 |
| Twin-Headed Thunder Dragon | 3300 | 0.2792 |
| Chain Energy | 3285 | 0.2779 |
| Nobleman of Extermination | 3216 | 0.2721 |
| Apprentice Magician | 3091 | 0.2615 |
| Chiron the Mage | 3091 | 0.2615 |
| Soul Release | 3091 | 0.2615 |
| Mage Power | 3062 | 0.2590 |
| Token Thanksgiving | 3008 | 0.2545 |
| Roulette Barrel | 2887 | 0.2442 |
| Newdoria | 2864 | 0.2423 |
| Waboku | 2864 | 0.2423 |
| Hallowed Life Barrier | 2843 | 0.2405 |
| Big Shield Gardna | 2822 | 0.2387 |
| Gigantes | 2822 | 0.2387 |
| Monster Reincarnation | 2801 | 0.2370 |
| Flame Ghost | 2769 | 0.2342 |
| Hysteric Fairy | 2724 | 0.2304 |
| Des Koala | 2706 | 0.2289 |
| Messenger of Peace | 2706 | 0.2289 |
| Goblin Attack Force | 2688 | 0.2274 |
| Mask of Darkness | 2688 | 0.2274 |
| Dark Mimic LV1 | 2671 | 0.2260 |
| Sacred Crane | 2671 | 0.2260 |
| Dimension Fusion | 2638 | 0.2232 |
| Last Will | 2638 | 0.2232 |
| Magic Jammer | 2591 | 0.2192 |
| Wave-Motion Cannon | 2576 | 0.2179 |
| Monster Gate | 2547 | 0.2155 |
| Reasoning | 2547 | 0.2155 |
| Enemy Controller | 2417 | 0.2045 |
| Seven Tools of the Bandit | 2372 | 0.2007 |
| Spell Canceller | 2217 | 0.1875 |
| Just Desserts | 2101 | 0.1777 |
| Return from the Different Dimension | 2101 | 0.1777 |
| Ojama Trio | 2053 | 0.1737 |
| Master of Oz | 2039 | 0.1725 |
| Dragoness the Wicked Knight | 2021 | 0.1710 |
| Magical Merchant | 2010 | 0.1700 |
| Secret Barrel | 1993 | 0.1686 |
| Skilled Dark Magician | 1987 | 0.1681 |
| Giant Rat | 1981 | 0.1676 |
| Ojama King | 1943 | 0.1644 |
| Bottomless Trap Hole | 1933 | 0.1635 |
| Cyber-Stein | 1933 | 0.1635 |
| King Dragun | 1919 | 0.1623 |
| The Last Warrior from Another Planet | 1916 | 0.1621 |
| Gyaku-Gire Panda | 1888 | 0.1597 |
| Darkfire Dragon | 1874 | 0.1585 |
| Airknight Parshath | 1870 | 0.1582 |
| Compulsory Evacuation Device | 1865 | 0.1578 |
| Des Wombat | 1861 | 0.1574 |
| Fiend Skull Dragon | 1854 | 0.1568 |
| Reaper on the Nightmare | 1849 | 0.1564 |
| Gravity Bind | 1841 | 0.1557 |
| Dark Blade the Dragon Knight | 1836 | 0.1553 |
| Abyss Soldier | 1810 | 0.1531 |
| D.D. Assailant | 1810 | 0.1531 |
| Mystic Tomato | 1798 | 0.1521 |
| Gatling Dragon | 1790 | 0.1514 |
| Ryu Senshi | 1781 | 0.1507 |
| Threatening Roar | 1775 | 0.1502 |
| Dark Balter the Terrible | 1755 | 0.1485 |
| Thousand-Eyes Restrict | 1735 | 0.1468 |
| Dark Scorpion - Cliff the Trap Remover | 1706 | 0.1443 |
| Kuriboh | 1706 | 0.1443 |
| Level Limit - Area B | 1670 | 0.1413 |
| Zaborg the Thunder Monarch | 1648 | 0.1394 |
| United We Stand | 1600 | 0.1354 |
| Widespread Ruin | 1580 | 0.1337 |
| Brain Control | 1537 | 0.1300 |
| Giant Trunade | 1494 | 0.1264 |
| Berserk Gorilla | 1473 | 0.1246 |
| Xing Zhen Hu | 1443 | 0.1221 |
| Phoenix Wing Wind Blast | 1434 | 0.1213 |
| Magic Cylinder | 1379 | 0.1167 |
| Spirit Reaper | 1355 | 0.1146 |
| King Tiger Wanghu | 1334 | 0.1129 |
| Smashing Ground | 1327 | 0.1123 |
| Dark Magician of Chaos | 1316 | 0.1113 |
| Royal Decree | 1287 | 0.1089 |
| Creature Swap | 1264 | 0.1069 |
| Don Zaloog | 1230 | 0.1041 |
| Ninja Grandmaster Sasuke | 1220 | 0.1032 |
| Ceasefire | 1161 | 0.0982 |
| Cipher Soldier | 1154 | 0.0976 |
| Metamorphosis | 1151 | 0.0974 |
| Mystic Swordsman LV2 | 1115 | 0.0943 |
| Scapegoat | 1100 | 0.0931 |
| Lightning Vortex | 1098 | 0.0929 |
| Injection Fairy Lily | 1089 | 0.0921 |
| Blade Knight | 1068 | 0.0903 |
| Cyber Jar | 1056 | 0.0893 |
| Asura Priest | 1051 | 0.0889 |
| Raigeki Break | 1032 | 0.0873 |
| Jar of Greed | 997 | 0.0843 |
| Zombyra the Dark | 996 | 0.0843 |
| Mobius the Frost Monarch | 964 | 0.0816 |
| Mystik Wok | 964 | 0.0816 |
| Jinzo | 961 | 0.0813 |
| Thunder Dragon | 940 | 0.0795 |
| Reinforcement of the Army | 938 | 0.0794 |
| Dekoichi the Battlechanted Locomotive | 927 | 0.0784 |
| Night Assailant | 925 | 0.0783 |
| Morphing Jar | 914 | 0.0773 |
| Solemn Judgment | 910 | 0.0770 |
| Dust Tornado | 903 | 0.0764 |
| Kycoo the Ghost Destroyer | 888 | 0.0751 |
| Tsukuyomi | 884 | 0.0748 |
| Chaos Sorcerer | 883 | 0.0747 |
| Gravekeeper's Spy | 880 | 0.0744 |
| Mind Control | 854 | 0.0722 |
| Upstart Goblin | 847 | 0.0717 |
| Book of Moon | 833 | 0.0705 |
| Magician of Faith | 832 | 0.0704 |
| Trap Dustshoot | 829 | 0.0701 |
| Sakuretsu Armor | 823 | 0.0696 |
| Card Destruction | 738 | 0.0624 |
| Swords of Revealing Light | 713 | 0.0603 |
| Exiled Force | 704 | 0.0596 |
| Call Of The Haunted | 702 | 0.0594 |
| D.D. Warrior Lady | 679 | 0.0574 |
| Premature Burial | 663 | 0.0561 |
| Nobleman of Crossout | 662 | 0.0560 |
| Sinister Serpent | 603 | 0.0510 |
| Sangan | 572 | 0.0484 |
| Torrential Tribute | 557 | 0.0471 |
| Tribe-Infecting Virus | 544 | 0.0460 |
| Black Luster Soldier - Envoy of Beginning | 543 | 0.0459 |
| Delinquent Duo | 537 | 0.0454 |
| Breaker the Magical Warrior | 532 | 0.0450 |
| Mystical Space Typhoon | 532 | 0.0450 |
| Mirror Force | 529 | 0.0448 |
| Bell of Destruction | 526 | 0.0445 |
| Snatch Steal | 526 | 0.0445 |
| Heavy Storm | 524 | 0.0443 |
| Graceful Charity | 522 | 0.0442 |
| Pot of Greed | 522 | 0.0442 |

### Cybernetic Revolution

- **Type:** TCG · **Format:** Cyber · **Cards in pool:** 5 · **Weight sum:** 10040

| Card | Weight | Pull % |
|---|---:|---:|
| Drillroid | 10000 | 99.6016 |
| Cyber Dragon | 10 | 0.0996 |
| Cyber End Dragon | 10 | 0.0996 |
| Cyber Twin Dragon | 10 | 0.0996 |
| Goblin Elite Attack Force | 10 | 0.0996 |

### Cyber

- **Type:** Usage · **Format:** Cyber · **Cards in pool:** 139 · **Weight sum:** 238406

| Card | Weight | Pull % |
|---|---:|---:|
| Karbonala Warrior | 7299 | 3.0616 |
| Roaring Ocean Snake | 7299 | 3.0616 |
| Twin-Headed Thunder Dragon | 7299 | 3.0616 |
| Flame Ghost | 5560 | 2.3322 |
| Fusionist | 5560 | 2.3322 |
| Giltia the D. Knight | 5560 | 2.3322 |
| Gaia the Dragon Champion | 3416 | 1.4328 |
| Apprentice Magician | 2978 | 1.2491 |
| Cannon Soldier | 2978 | 1.2491 |
| Dark Flare Knight | 2978 | 1.2491 |
| Dark Scorpion - Cliff the Trap Remover | 2978 | 1.2491 |
| Different Dimension Capsule | 2978 | 1.2491 |
| Greenkappa | 2978 | 1.2491 |
| Return from the Different Dimension | 2978 | 1.2491 |
| Sasuke Samurai | 2978 | 1.2491 |
| Soul Exchange | 2978 | 1.2491 |
| Stealth Bird | 2978 | 1.2491 |
| Thestalos the Firestorm Monarch | 2978 | 1.2491 |
| Thunder Dragon | 2978 | 1.2491 |
| Xing Zhen Hu | 2978 | 1.2491 |
| Upstart Goblin | 2550 | 1.0696 |
| Des Wombat | 2286 | 0.9589 |
| Dragoness the Wicked Knight | 2286 | 0.9589 |
| Electric Snake | 2286 | 0.9589 |
| Jar of Greed | 2286 | 0.9589 |
| Monster Gate | 2286 | 0.9589 |
| Monster Reincarnation | 2286 | 0.9589 |
| Poison of the Old Man | 2286 | 0.9589 |
| Raigeki Break | 2286 | 0.9589 |
| Reasoning | 2286 | 0.9589 |
| Sacred Crane | 2286 | 0.9589 |
| Darkfire Dragon | 2070 | 0.8683 |
| Ojama King | 2041 | 0.8561 |
| Master of Oz | 2013 | 0.8444 |
| Abyss Soldier | 1962 | 0.8230 |
| Card Destruction | 1962 | 0.8230 |
| Chain Disappearance | 1962 | 0.8230 |
| Giant Rat | 1962 | 0.8230 |
| Gyaku-Gire Panda | 1962 | 0.8230 |
| Spell Canceller | 1962 | 0.8230 |
| Token Thanksgiving | 1962 | 0.8230 |
| Widespread Ruin | 1962 | 0.8230 |
| Cyber End Dragon | 1813 | 0.7605 |
| King Dragun | 1778 | 0.7458 |
| Ryu Senshi | 1778 | 0.7458 |
| The Last Warrior from Another Planet | 1778 | 0.7458 |
| Blade Knight | 1761 | 0.7387 |
| Dark Blade the Dragon Knight | 1761 | 0.7387 |
| Des Koala | 1761 | 0.7387 |
| Just Desserts | 1761 | 0.7387 |
| Magical Merchant | 1761 | 0.7387 |
| Nimble Momonga | 1761 | 0.7387 |
| Ojama Trio | 1761 | 0.7387 |
| Secret Barrel | 1761 | 0.7387 |
| Reaper on the Nightmare | 1745 | 0.7319 |
| Cyber Twin Dragon | 1715 | 0.7194 |
| Fiend Skull Dragon | 1715 | 0.7194 |
| Gatling Dragon | 1715 | 0.7194 |
| Dark Balter the Terrible | 1700 | 0.7131 |
| Thousand-Eyes Restrict | 1700 | 0.7131 |
| Big Shield Gardna | 1621 | 0.6799 |
| Chiron the Mage | 1621 | 0.6799 |
| Megamorph | 1621 | 0.6799 |
| Ninja Grandmaster Sasuke | 1621 | 0.6799 |
| Wave-Motion Cannon | 1621 | 0.6799 |
| Dark Magician of Chaos | 1516 | 0.6359 |
| Don Zaloog | 1516 | 0.6359 |
| Enemy Controller | 1516 | 0.6359 |
| Level Limit - Area B | 1516 | 0.6359 |
| United We Stand | 1516 | 0.6359 |
| Gravity Bind | 1396 | 0.5856 |
| Airknight Parshath | 1364 | 0.5721 |
| D.D. Assailant | 1364 | 0.5721 |
| Dekoichi the Battlechanted Locomotive | 1364 | 0.5721 |
| Kycoo the Ghost Destroyer | 1364 | 0.5721 |
| Solemn Judgment | 1364 | 0.5721 |
| Waboku | 1364 | 0.5721 |
| Cipher Soldier | 1307 | 0.5482 |
| Reinforcement of the Army | 1307 | 0.5482 |
| Smashing Ground | 1307 | 0.5482 |
| Cold Wave | 1258 | 0.5277 |
| Creature Swap | 1236 | 0.5184 |
| Giant Trunade | 1215 | 0.5096 |
| Mystic Swordsman LV2 | 1215 | 0.5096 |
| Limiter Removal | 1178 | 0.4941 |
| Asura Priest | 1114 | 0.4673 |
| Gravekeeper's Spy | 1114 | 0.4673 |
| Mind Control | 1114 | 0.4673 |
| Chaos Sorcerer | 1087 | 0.4559 |
| Injection Fairy Lily | 1087 | 0.4559 |
| Magic Cylinder | 1087 | 0.4559 |
| Mystic Tomato | 1063 | 0.4459 |
| Bottomless Trap Hole | 1040 | 0.4362 |
| Mystik Wok | 1040 | 0.4362 |
| Zaborg the Thunder Monarch | 1040 | 0.4362 |
| Cyber Jar | 1019 | 0.4274 |
| Exarion Universe | 1019 | 0.4274 |
| Tsukuyomi | 1019 | 0.4274 |
| Dust Tornado | 1000 | 0.4195 |
| Last Will | 1000 | 0.4195 |
| Scapegoat | 1000 | 0.4195 |
| King Tiger Wanghu | 966 | 0.4052 |
| Kuriboh | 966 | 0.4052 |
| Metamorphosis | 950 | 0.3985 |
| Royal Decree | 935 | 0.3922 |
| Brain Control | 909 | 0.3813 |
| Threatening Roar | 885 | 0.3712 |
| Sakuretsu Armor | 874 | 0.3666 |
| Cyber-Stein | 863 | 0.3620 |
| Spirit Reaper | 863 | 0.3620 |
| Ceasefire | 853 | 0.3578 |
| Trap Dustshoot | 853 | 0.3578 |
| Mobius the Frost Monarch | 844 | 0.3540 |
| Morphing Jar | 826 | 0.3465 |
| Magician of Faith | 809 | 0.3393 |
| Book of Moon | 802 | 0.3364 |
| Cyber Dragon | 787 | 0.3301 |
| Swords of Revealing Light | 761 | 0.3192 |
| Lightning Vortex | 713 | 0.2991 |
| Nobleman of Crossout | 665 | 0.2789 |
| Sinister Serpent | 665 | 0.2789 |
| Jinzo | 655 | 0.2747 |
| Exiled Force | 613 | 0.2571 |
| Call Of The Haunted | 599 | 0.2513 |
| D.D. Warrior Lady | 592 | 0.2483 |
| Sangan | 580 | 0.2433 |
| Tribe-Infecting Virus | 580 | 0.2433 |
| Breaker the Magical Warrior | 554 | 0.2324 |
| Premature Burial | 550 | 0.2307 |
| Black Luster Soldier - Envoy of Beginning | 545 | 0.2286 |
| Delinquent Duo | 541 | 0.2269 |
| Mystical Space Typhoon | 537 | 0.2252 |
| Bell of Destruction | 529 | 0.2219 |
| Snatch Steal | 529 | 0.2219 |
| Torrential Tribute | 529 | 0.2219 |
| Mirror Force | 525 | 0.2202 |
| Graceful Charity | 522 | 0.2190 |
| Heavy Storm | 522 | 0.2190 |
| Pot of Greed | 522 | 0.2190 |

### Elemental Energy

- **Type:** TCG · **Format:** Reaper · **Cards in pool:** 3 · **Weight sum:** 10020

| Card | Weight | Pull % |
|---|---:|---:|
| Hydrogeddon | 10000 | 99.8004 |
| Cyber Blader | 10 | 0.0998 |
| Pot of Avarice | 10 | 0.0998 |

### Structure Deck- Spellcaster's Judgment

- **Type:** TCG · **Format:** Reaper · **Cards in pool:** 18 · **Weight sum:** 180000

| Card | Weight | Pull % |
|---|---:|---:|
| Apprentice Magician | 10000 | 5.5556 |
| Breaker the Magical Warrior | 10000 | 5.5556 |
| Call Of The Haunted | 10000 | 5.5556 |
| Chaos Sorcerer | 10000 | 5.5556 |
| Dark Magician | 10000 | 5.5556 |
| Divine Wrath | 10000 | 5.5556 |
| Gemini Elf | 10000 | 5.5556 |
| Heavy Storm | 10000 | 5.5556 |
| Lightning Vortex | 10000 | 5.5556 |
| Mage Power | 10000 | 5.5556 |
| Magic Cylinder | 10000 | 5.5556 |
| Magician of Faith | 10000 | 5.5556 |
| Mystical Space Typhoon | 10000 | 5.5556 |
| Nobleman of Crossout | 10000 | 5.5556 |
| Premature Burial | 10000 | 5.5556 |
| Skilled Dark Magician | 10000 | 5.5556 |
| Swords of Revealing Light | 10000 | 5.5556 |
| Tsukuyomi | 10000 | 5.5556 |

### Shadow of Infinity

- **Type:** TCG · **Format:** Reaper · **Cards in pool:** 2 · **Weight sum:** 10010

| Card | Weight | Pull % |
|---|---:|---:|
| Sand Moth | 10000 | 99.9001 |
| Treeborn Frog | 10 | 0.0999 |

### Starter Deck 2006

- **Type:** TCG · **Format:** Reaper · **Cards in pool:** 23 · **Weight sum:** 230000

| Card | Weight | Pull % |
|---|---:|---:|
| Black Pendant | 10000 | 4.3478 |
| Brain Control | 10000 | 4.3478 |
| Castle Walls | 10000 | 4.3478 |
| Compulsory Evacuation Device | 10000 | 4.3478 |
| Dust Tornado | 10000 | 4.3478 |
| Fissure | 10000 | 4.3478 |
| Gaia Power | 10000 | 4.3478 |
| Gazelle the King of Mythical Beasts | 10000 | 4.3478 |
| Heavy Storm | 10000 | 4.3478 |
| Jar of Greed | 10000 | 4.3478 |
| Kuriboh | 10000 | 4.3478 |
| Magic Cylinder | 10000 | 4.3478 |
| Magician of Faith | 10000 | 4.3478 |
| Monster Reincarnation | 10000 | 4.3478 |
| Ookazi | 10000 | 4.3478 |
| Premature Burial | 10000 | 4.3478 |
| Princess of Tsurugi | 10000 | 4.3478 |
| Red Medicine | 10000 | 4.3478 |
| Reinforcements | 10000 | 4.3478 |
| Sakuretsu Armor | 10000 | 4.3478 |
| Skelengel | 10000 | 4.3478 |
| Spear Dragon | 10000 | 4.3478 |
| Tribute to The Doomed | 10000 | 4.3478 |

### Reaper

- **Type:** Usage · **Format:** Reaper · **Cards in pool:** 207 · **Weight sum:** 607663

| Card | Weight | Pull % |
|---|---:|---:|
| B. Skull Dragon | 13439 | 2.2116 |
| Cyber Saurus | 13439 | 2.2116 |
| Deepsea Shark | 13439 | 2.2116 |
| Labyrinth Tank | 13439 | 2.2116 |
| Musician King | 13439 | 2.2116 |
| Bickuribox | 10214 | 1.6809 |
| Charubin the Fire Knight | 10214 | 1.6809 |
| Roaring Ocean Snake | 10214 | 1.6809 |
| Dark Paladin | 8703 | 1.4322 |
| Gaia the Dragon Champion | 8703 | 1.4322 |
| Flame Swordsman | 7770 | 1.2787 |
| Kaminari Attack | 7770 | 1.2787 |
| Karbonala Warrior | 7770 | 1.2787 |
| Empress Judge | 7117 | 1.1712 |
| Giltia the D. Knight | 6624 | 1.0901 |
| Flame Ghost | 5651 | 0.9300 |
| Acid Trap Hole | 5422 | 0.8923 |
| Asura Priest | 5422 | 0.8923 |
| De-Spell | 5422 | 0.8923 |
| Giant Germ | 5422 | 0.8923 |
| Light of Intervention | 5422 | 0.8923 |
| Magic Drain | 5422 | 0.8923 |
| Marauding Captain | 5422 | 0.8923 |
| Masked Dragon | 5422 | 0.8923 |
| Mirror Wall | 5422 | 0.8923 |
| Reasoning | 5422 | 0.8923 |
| Remove Trap | 5422 | 0.8923 |
| Rising Air Current | 5422 | 0.8923 |
| Roulette Barrel | 5422 | 0.8923 |
| Royal Oppression | 5422 | 0.8923 |
| Sacred Crane | 5422 | 0.8923 |
| Skull Lair | 5422 | 0.8923 |
| Time Wizard | 5422 | 0.8923 |
| UFO Turtle | 5422 | 0.8923 |
| Warrior of Tradition | 5422 | 0.8923 |
| Fusionist | 5049 | 0.8309 |
| Abyss Soldier | 4629 | 0.7618 |
| Emergency Provisions | 4629 | 0.7618 |
| Twin-Headed Thunder Dragon | 4311 | 0.7094 |
| Black Illusion Ritual | 4138 | 0.6810 |
| Blast with Chain | 4138 | 0.6810 |
| Chain Energy | 4138 | 0.6810 |
| Dimension Fusion | 4138 | 0.6810 |
| Divine Wrath | 4138 | 0.6810 |
| Electric Snake | 4138 | 0.6810 |
| Fissure | 4138 | 0.6810 |
| Princess of Tsurugi | 4138 | 0.6810 |
| Relinquished | 4138 | 0.6810 |
| Robbin' Goblin | 4138 | 0.6810 |
| Rush Recklessly | 4138 | 0.6810 |
| Time Seal | 4138 | 0.6810 |
| Warrior Elimination | 4138 | 0.6810 |
| Big Shield Gardna | 3537 | 0.5821 |
| Mage Power | 3537 | 0.5821 |
| Magic Jammer | 3537 | 0.5821 |
| Mind Control | 3537 | 0.5821 |
| Nimble Momonga | 3537 | 0.5821 |
| Poison of the Old Man | 3537 | 0.5821 |
| Skelengel | 3537 | 0.5821 |
| The Wicked Worm Beast | 3537 | 0.5821 |
| Tribute to The Doomed | 3537 | 0.5821 |
| Flying Kamakiri #1 | 3165 | 0.5208 |
| Goblin Elite Attack Force | 3165 | 0.5208 |
| Dark Flare Knight | 3135 | 0.5159 |
| Dragoness the Wicked Knight | 2975 | 0.4896 |
| Cyber Blader | 2928 | 0.4818 |
| Cannon Soldier | 2905 | 0.4781 |
| Mask of Darkness | 2905 | 0.4781 |
| Sand Moth | 2905 | 0.4781 |
| Sasuke Samurai | 2905 | 0.4781 |
| Secret Barrel | 2905 | 0.4781 |
| Seven Tools of the Bandit | 2905 | 0.4781 |
| Darkfire Dragon | 2883 | 0.4744 |
| Master of Oz | 2763 | 0.4547 |
| Apprentice Magician | 2709 | 0.4458 |
| Dark Magician of Chaos | 2709 | 0.4458 |
| Goblin Attack Force | 2709 | 0.4458 |
| Prohibition | 2709 | 0.4458 |
| Thunder Dragon | 2709 | 0.4458 |
| Berserk Gorilla | 2555 | 0.4205 |
| Book of Life | 2555 | 0.4205 |
| Exarion Universe | 2555 | 0.4205 |
| Hallowed Life Barrier | 2555 | 0.4205 |
| Just Desserts | 2555 | 0.4205 |
| Megamorph | 2555 | 0.4205 |
| Milus Radiant | 2555 | 0.4205 |
| Nobleman of Extermination | 2555 | 0.4205 |
| Ojama Trio | 2555 | 0.4205 |
| Upstart Goblin | 2488 | 0.4094 |
| Gigantes | 2428 | 0.3996 |
| Jar of Greed | 2428 | 0.3996 |
| Pyramid Turtle | 2428 | 0.3996 |
| Ryu Kokki | 2428 | 0.3996 |
| Skilled Dark Magician | 2428 | 0.3996 |
| Compulsory Evacuation Device | 2322 | 0.3821 |
| Gyaku-Gire Panda | 2322 | 0.3821 |
| Mystik Wok | 2322 | 0.3821 |
| Ojama King | 2248 | 0.3699 |
| Airknight Parshath | 2231 | 0.3671 |
| Chiron the Mage | 2231 | 0.3671 |
| Dark Mimic LV1 | 2231 | 0.3671 |
| Blue-Eyes Ultimate Dragon | 2223 | 0.3658 |
| Dark Blade the Dragon Knight | 2223 | 0.3658 |
| Cold Wave | 2152 | 0.3541 |
| Des Koala | 2152 | 0.3541 |
| Spell Canceller | 2152 | 0.3541 |
| King Dragun | 2145 | 0.3530 |
| Reaper on the Nightmare | 2137 | 0.3517 |
| Fiend Skull Dragon | 2096 | 0.3449 |
| Zombyra the Dark | 2082 | 0.3426 |
| The Last Warrior from Another Planet | 2063 | 0.3395 |
| Blade Knight | 2021 | 0.3326 |
| Messenger of Peace | 2021 | 0.3326 |
| Gatling Dragon | 1998 | 0.3288 |
| Ryu Senshi | 1998 | 0.3288 |
| Dark Balter the Terrible | 1981 | 0.3260 |
| Mother Grizzly | 1965 | 0.3234 |
| Phoenix Wing Wind Blast | 1965 | 0.3234 |
| Chain Disappearance | 1915 | 0.3151 |
| Cyber End Dragon | 1915 | 0.3151 |
| Cyber Twin Dragon | 1915 | 0.3151 |
| Hydrogeddon | 1915 | 0.3151 |
| Dark Scorpion - Cliff the Trap Remover | 1869 | 0.3076 |
| Giant Rat | 1827 | 0.3007 |
| Trap Hole | 1827 | 0.3007 |
| Drillroid | 1789 | 0.2944 |
| Stealth Bird | 1789 | 0.2944 |
| Des Wombat | 1720 | 0.2831 |
| Newdoria | 1689 | 0.2780 |
| Gravity Bind | 1646 | 0.2709 |
| Limiter Removal | 1607 | 0.2645 |
| Night Assailant | 1607 | 0.2645 |
| Ninja Grandmaster Sasuke | 1607 | 0.2645 |
| United We Stand | 1607 | 0.2645 |
| Bazoo the Soul-Eater | 1560 | 0.2567 |
| Soul Exchange | 1539 | 0.2533 |
| Card Destruction | 1480 | 0.2436 |
| Kuriboh | 1480 | 0.2436 |
| Mask of Restrict | 1480 | 0.2436 |
| Wave-Motion Cannon | 1463 | 0.2408 |
| Level Limit - Area B | 1454 | 0.2393 |
| Gravekeeper's Spy | 1429 | 0.2352 |
| Waboku | 1429 | 0.2352 |
| Magical Merchant | 1385 | 0.2279 |
| Raigeki Break | 1385 | 0.2279 |
| Pot of Avarice | 1371 | 0.2256 |
| Dark Ruler Ha Des | 1345 | 0.2213 |
| D.D. Survivor | 1320 | 0.2172 |
| Return from the Different Dimension | 1309 | 0.2154 |
| Thestalos the Firestorm Monarch | 1277 | 0.2101 |
| Thousand-Eyes Restrict | 1274 | 0.2097 |
| Cipher Soldier | 1229 | 0.2023 |
| King Tiger Wanghu | 1229 | 0.2023 |
| Injection Fairy Lily | 1220 | 0.2008 |
| Magic Cylinder | 1220 | 0.2008 |
| Last Will | 1195 | 0.1967 |
| Giant Trunade | 1165 | 0.1917 |
| Dekoichi the Battlechanted Locomotive | 1158 | 0.1906 |
| Treeborn Frog | 1151 | 0.1894 |
| Cyber-Stein | 1131 | 0.1861 |
| Lightning Vortex | 1131 | 0.1861 |
| Chaos Sorcerer | 1118 | 0.1840 |
| Royal Decree | 1101 | 0.1812 |
| Solemn Judgment | 1095 | 0.1802 |
| Widespread Ruin | 1084 | 0.1784 |
| Trap Dustshoot | 1068 | 0.1758 |
| Brain Control | 1063 | 0.1749 |
| Zaborg the Thunder Monarch | 1063 | 0.1749 |
| D.D. Assailant | 1058 | 0.1741 |
| Don Zaloog | 1058 | 0.1741 |
| Bottomless Trap Hole | 1039 | 0.1710 |
| Enemy Controller | 1009 | 0.1660 |
| Mystic Tomato | 989 | 0.1628 |
| Creature Swap | 980 | 0.1613 |
| Threatening Roar | 978 | 0.1609 |
| Ceasefire | 944 | 0.1553 |
| Mystic Swordsman LV2 | 932 | 0.1534 |
| Kycoo the Ghost Destroyer | 920 | 0.1514 |
| Cyber Jar | 917 | 0.1509 |
| Metamorphosis | 917 | 0.1509 |
| Morphing Jar | 909 | 0.1496 |
| Smashing Ground | 896 | 0.1475 |
| Sakuretsu Armor | 840 | 0.1382 |
| Mobius the Frost Monarch | 831 | 0.1368 |
| Dust Tornado | 820 | 0.1349 |
| Swords of Revealing Light | 813 | 0.1338 |
| Reinforcement of the Army | 791 | 0.1302 |
| Cyber Dragon | 789 | 0.1298 |
| Spirit Reaper | 780 | 0.1284 |
| Tsukuyomi | 780 | 0.1284 |
| Jinzo | 734 | 0.1208 |
| Magician of Faith | 731 | 0.1203 |
| Scapegoat | 715 | 0.1177 |
| Call Of The Haunted | 620 | 0.1020 |
| Book of Moon | 603 | 0.0992 |
| Exiled Force | 580 | 0.0954 |
| Confiscation | 568 | 0.0935 |
| D.D. Warrior Lady | 568 | 0.0935 |
| Premature Burial | 553 | 0.0910 |
| Nobleman of Crossout | 546 | 0.0899 |
| Breaker the Magical Warrior | 535 | 0.0880 |
| Sangan | 535 | 0.0880 |
| Torrential Tribute | 531 | 0.0874 |
| Snatch Steal | 530 | 0.0872 |
| Dark Hole | 526 | 0.0866 |
| Heavy Storm | 526 | 0.0866 |
| Mystical Space Typhoon | 526 | 0.0866 |

_1223 card names in JSON/usage had no ROM match (omitted from pools)._

<!-- /pack-chances -->

<!-- pack-prices -->

## Password machine prices (DP)

Same formula as `tools/cardprices.py` / `src/mod_dp.c`. Each table lists only cards that format's usage file mentions. At runtime, **GetCardCost** uses the newest unlocked format whose decks use the card.

- **Usage score:** main+side share × 1000 + extra share × 1000/10; ×3 if Limited, ×1.5 if Semi-Limited.
- **Scaled price:** linear map per format so the cheapest listed card is **200 DP** and the priciest is **3000 DP**.
- **Not in any usage table:** **100 DP** (`POOL_CARD_COST` / `DEFAULT_CARD_COST`).

| Format | Cards priced |
|---|---:|
| [Yugi-Kaiba](#yugi-kaiba) | 75 |
| [Critter](#critter) | 88 |
| [Treasure](#treasure) | 84 |
| [Imperial](#imperial) | 105 |
| [Android](#android) | 102 |
| [Joey-Pegasus](#joey-pegasus) | 105 |
| [Fiber](#fiber) | 132 |
| [Yata](#yata) | 132 |
| [Scientist](#scientist) | 88 |
| [Vampire](#vampire) | 110 |
| [Chaos](#chaos) | 137 |
| [Warrior](#warrior) | 180 |
| [Goat](#goat) | 252 |
| [Cyber](#cyber) | 139 |
| [Reaper](#reaper) | 207 |

### Yugi-Kaiba

| Card | DP | Limit |
|---|---:|---|
| Change of Heart | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Swords of Revealing Light | 1562 | Semi-Limited |
| Card Destruction | 1231 | Semi-Limited |
| Fissure | 1128 |  |
| Man-Eater Bug | 1128 |  |
| Trap Hole | 1128 |  |
| La Jinn the Mystical Genie of the Lamp | 1121 |  |
| Giant Soldier of Stone | 1113 |  |
| Neo the Magic Swordsman | 1113 |  |
| Reinforcements | 1113 |  |
| Waboku | 1105 |  |
| Battle Ox | 1090 |  |
| Summoned Skull | 1075 |  |
| Soul Exchange | 1045 |  |
| Wall of Illusion | 1014 |  |
| Ultimate Offering | 938 |  |
| De-Spell | 877 |  |
| Trap Master | 839 |  |
| Aqua Madoor | 778 |  |
| Judge Man | 778 |  |
| Just Desserts | 573 |  |
| The Wicked Worm Beast | 558 |  |
| Ookazi | 535 |  |
| Stop Defense | 459 |  |
| Mystical Elf | 451 |  |
| Remove Trap | 367 |  |
| Gyakutenno Megami | 360 |  |
| Hane-Hane | 360 |  |
| Spirit of the Harp | 352 |  |
| Great White | 314 |  |
| Last Will | 307 |  |
| Dragon Capture Jar | 299 |  |
| Reaper of the Cards | 284 |  |
| Umi  | 268 |  |
| Armed Ninja | 261 |  |
| Final Flame | 246 |  |
| Reverse Trap | 246 |  |
| Metal Dragon | 239 |  |
| Yami | 238 |  |
| Flower Wolf | 233 |  |
| Fusionist | 229 |  |
| Gaia the Dragon Champion | 229 |  |
| Dragoness the Wicked Knight | 224 |  |
| Flame Swordsman | 224 |  |
| Blue-Eyes White Dragon | 223 |  |
| Castle Walls | 223 |  |
| Curse of Dragon | 223 |  |
| Darkfire Dragon | 223 |  |
| Lord of D. | 223 |  |
| The Flute of Summoning Dragon | 223 |  |
| Karbonala Warrior | 219 |  |
| Flame Ghost | 218 |  |
| Exodia the Forbidden One | 215 | Limited |
| Left Arm of the Forbidden One | 215 | Limited |
| Left Leg of the Forbidden One | 215 | Limited |
| Right Arm of the Forbidden One | 215 | Limited |
| Right Leg of the Forbidden One | 215 | Limited |
| Dian Keto the Cure Master | 208 |  |
| Forest | 208 |  |
| Invigoration | 208 |  |
| Ryu-Kishin Powered | 208 |  |
| Sword of Dark Destruction | 208 |  |
| Two-Pronged Attack | 208 |  |
| Charubin the Fire Knight | 202 |  |
| Destroyer Golem | 200 |  |
| Dragon Zombie | 200 |  |
| Koumori Dragon | 200 |  |
| Petit Dragon | 200 |  |
| Sparks | 200 |  |
| The Stern Mystic | 200 |  |
| Wasteland | 200 |  |

### Critter

| Card | DP | Limit |
|---|---:|---|
| Change of Heart | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Heavy Storm | 1597 | Semi-Limited |
| Sangan | 1597 | Semi-Limited |
| Witch of the Black Forest | 1597 | Semi-Limited |
| Swords of Revealing Light | 1514 | Semi-Limited |
| Card Destruction | 1184 | Semi-Limited |
| Fissure | 1129 |  |
| Trap Hole | 1129 |  |
| Cannon Soldier | 1115 |  |
| Magician of Faith | 1101 |  |
| Waboku | 1101 |  |
| La Jinn the Mystical Genie of the Lamp | 1087 |  |
| Giant Soldier of Stone | 1060 |  |
| Masked Sorcerer | 1046 |  |
| Muka Muka | 1046 |  |
| Solemn Judgment | 1046 |  |
| Robbin' Goblin | 1019 |  |
| Last Will | 991 |  |
| Man-Eater Bug | 922 |  |
| Thunder Dragon | 909 |  |
| Seven Tools of the Bandit | 812 |  |
| White Magical Hat | 647 |  |
| 7 Colored Fish | 620 |  |
| Wall of Illusion | 620 |  |
| Jirai Gumo | 606 |  |
| Exodia the Forbidden One | 565 | Limited |
| Left Arm of the Forbidden One | 565 | Limited |
| Left Leg of the Forbidden One | 565 | Limited |
| Right Arm of the Forbidden One | 565 | Limited |
| Right Leg of the Forbidden One | 565 | Limited |
| Fake Trap | 455 |  |
| Magic Jammer | 400 |  |
| Horn of Heaven | 344 |  |
| Ultimate Offering | 344 |  |
| Kuriboh | 317 |  |
| Mask of Darkness | 303 |  |
| Trap Master | 303 |  |
| De-Spell | 289 |  |
| Princess of Tsurugi | 289 |  |
| Reinforcements | 289 |  |
| Aqua Madoor | 276 |  |
| Dark Elf | 276 |  |
| Tremendous Fire | 276 |  |
| Crass Clown | 262 |  |
| Polymerization | 262 |  |
| Twin-Headed Thunder Dragon | 252 |  |
| Jinzo #7 | 248 |  |
| Tribute to The Doomed | 248 |  |
| B. Skull Dragon | 245 |  |
| Gaia the Dragon Champion | 237 |  |
| Giga-tech Wolf | 234 |  |
| Labyrinth Tank | 228 |  |
| Thousand Dragon | 228 |  |
| Charubin the Fire Knight | 222 |  |
| Darkfire Dragon | 221 |  |
| Dragoness the Wicked Knight | 221 |  |
| Shield & Sword | 221 |  |
| Stim-Pack | 221 |  |
| Stop Defense | 221 |  |
| Summoned Skull | 221 |  |
| Musician King | 219 |  |
| Bickuribox | 218 |  |
| Cyber Saurus | 218 |  |
| Deepsea Shark | 215 |  |
| Fusionist | 215 |  |
| Punished Eagle | 215 |  |
| Empress Judge | 212 |  |
| Roaring Ocean Snake | 211 |  |
| Flame Ghost | 210 |  |
| Karbonala Warrior | 210 |  |
| Giltia the D. Knight | 208 |  |
| Catapult Turtle | 207 |  |
| Dream Clown | 207 |  |
| Ookazi | 207 |  |
| Remove Trap | 207 |  |
| Share the Pain | 207 |  |
| Soul Exchange | 207 |  |
| The Wicked Worm Beast | 207 |  |
| Metal Dragon | 206 |  |
| Flower Wolf | 204 |  |
| Skull Knight | 203 |  |
| Flame Swordsman | 200 |  |

### Treasure

| Card | DP | Limit |
|---|---:|---|
| Change of Heart | 3000 | Limited |
| Confiscation | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Delinquent Duo | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Monster Reborn | 2878 | Limited |
| Snatch Steal | 2634 | Limited |
| Painful Choice | 2025 | Limited |
| Sangan | 1598 | Semi-Limited |
| Witch of the Black Forest | 1598 | Semi-Limited |
| Heavy Storm | 1415 | Semi-Limited |
| Cannon Soldier | 1131 |  |
| Mystic Tomato | 1131 |  |
| Fissure | 1009 |  |
| Man-Eater Bug | 1009 |  |
| Mechanicalchaser | 1009 |  |
| Mystical Space Typhoon | 1009 |  |
| Trap Hole | 1009 |  |
| Waboku | 1009 |  |
| Magician of Faith | 968 |  |
| Cyber Jar | 927 | Limited |
| Giant Trunade | 887 |  |
| Seven Tools of the Bandit | 887 |  |
| Banisher of the Light | 846 |  |
| Tribute to The Doomed | 806 |  |
| Solemn Judgment | 765 |  |
| Last Will | 684 |  |
| Upstart Goblin | 684 | Limited |
| Jirai Gumo | 643 |  |
| Nimble Momonga | 602 |  |
| Card Destruction | 501 | Semi-Limited |
| Masked Sorcerer | 480 |  |
| Robbin' Goblin | 480 |  |
| Rush Recklessly | 480 |  |
| Giant Soldier of Stone | 440 |  |
| Swords of Revealing Light | 440 | Semi-Limited |
| Thunder Dragon | 399 |  |
| Magic Jammer | 358 |  |
| Chain Energy | 318 |  |
| Giant Germ | 318 |  |
| Just Desserts | 318 |  |
| Minar | 318 |  |
| Princess of Tsurugi | 318 |  |
| Soul Exchange | 318 |  |
| Spear Cretin | 318 |  |
| Summoned Skull | 318 |  |
| Toll | 318 |  |
| Giant Rat | 277 |  |
| La Jinn the Mystical Genie of the Lamp | 277 |  |
| Wall of Illusion | 277 |  |
| Darkfire Dragon | 253 |  |
| Dragoness the Wicked Knight | 253 |  |
| Charubin the Fire Knight | 249 |  |
| Flame Ghost | 249 |  |
| Flame Swordsman | 249 |  |
| Catapult Turtle | 237 |  |
| Crass Clown | 237 |  |
| Flash Assailant | 237 |  |
| Gaia Power | 237 |  |
| Hercules Beetle | 237 |  |
| Horn of Heaven | 237 |  |
| Kuriboh | 237 |  |
| Mother Grizzly | 237 |  |
| Share the Pain | 237 |  |
| Slot Machine | 237 |  |
| White Magical Hat | 237 |  |
| B. Skull Dragon | 224 |  |
| Bickuribox | 220 |  |
| Gaia the Dragon Champion | 220 |  |
| Fusionist | 212 |  |
| Labyrinth Tank | 212 |  |
| Thousand Dragon | 212 |  |
| Twin-Headed Thunder Dragon | 212 |  |
| Cyber Saurus | 208 |  |
| Deepsea Shark | 208 |  |
| Empress Judge | 208 |  |
| Flower Wolf | 208 |  |
| Karbonala Warrior | 208 |  |
| Metal Dragon | 208 |  |
| Musician King | 208 |  |
| Skull Knight | 200 |  |

### Imperial

| Card | DP | Limit |
|---|---:|---|
| Change of Heart | 3000 | Limited |
| Confiscation | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Delinquent Duo | 3000 | Limited |
| Imperial Order | 3000 | Limited |
| Jinzo | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Snatch Steal | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Call Of The Haunted | 2933 | Limited |
| Premature Burial | 2400 | Limited |
| Morphing Jar | 2333 | Limited |
| Ceasefire | 2266 | Limited |
| Cyber Jar | 1999 | Limited |
| Swords of Revealing Light | 1666 | Limited |
| Nobleman of Crossout | 1599 | Semi-Limited |
| Sangan | 1599 | Semi-Limited |
| Heavy Storm | 1499 | Semi-Limited |
| Painful Choice | 1199 | Limited |
| Mystic Tomato | 1132 |  |
| Mystical Space Typhoon | 1132 |  |
| Dust Tornado | 1110 |  |
| Magician of Faith | 1110 |  |
| Man-Eater Bug | 1110 |  |
| Mechanicalchaser | 1110 |  |
| Fissure | 1043 |  |
| Trap Hole | 1043 |  |
| Cannon Soldier | 976 |  |
| Solemn Judgment | 865 |  |
| Banisher of the Light | 754 |  |
| The Fiend Megacyber | 620 |  |
| Giant Trunade | 598 |  |
| Goblin Attack Force | 576 |  |
| Time Seal | 554 |  |
| Card Destruction | 465 | Limited |
| Solemn Wishes | 465 |  |
| Giant Soldier of Stone | 398 |  |
| Limiter Removal | 398 | Limited |
| Waboku | 376 |  |
| 4-Starred Ladybug of Doom | 353 |  |
| Upstart Goblin | 331 | Limited |
| Black Illusion Ritual | 287 |  |
| Masked Sorcerer | 287 |  |
| Relinquished | 287 |  |
| Rush Recklessly | 287 |  |
| Senju of the Thousand Hands | 287 |  |
| Sonic Bird | 287 |  |
| Exodia the Forbidden One | 264 | Limited |
| Giant Germ | 264 |  |
| Graverobber | 264 |  |
| Last Will | 264 |  |
| Left Arm of the Forbidden One | 264 | Limited |
| Left Leg of the Forbidden One | 264 | Limited |
| Magic Jammer | 264 |  |
| Morphing Jar #2 | 264 | Semi-Limited |
| Right Arm of the Forbidden One | 264 | Limited |
| Right Leg of the Forbidden One | 264 | Limited |
| Acid Trap Hole | 242 |  |
| B. Skull Dragon | 242 |  |
| Magic Drain | 242 |  |
| Thousand-Eyes Restrict | 242 |  |
| Tribute to The Doomed | 242 |  |
| White Magical Hat | 242 |  |
| Darkfire Dragon | 240 |  |
| Labyrinth Tank | 240 |  |
| Twin-Headed Thunder Dragon | 236 |  |
| Backup Soldier | 231 | Semi-Limited |
| Warrior of Tradition | 231 |  |
| Musician King | 229 |  |
| Dragoness the Wicked Knight | 227 |  |
| Thousand Dragon | 227 |  |
| Deepsea Shark | 224 |  |
| Punished Eagle | 224 |  |
| Roaring Ocean Snake | 224 |  |
| Kwagar Hercules | 222 |  |
| Metal Dragon | 222 |  |
| Aqua Madoor | 220 |  |
| Exchange | 220 |  |
| Gravity Bind | 220 |  |
| Kuriboh | 220 |  |
| Light of Intervention | 220 |  |
| Mask of Darkness | 220 |  |
| Megamorph | 220 |  |
| Nimble Momonga | 220 |  |
| Prohibition | 220 |  |
| Reinforcements | 220 |  |
| Robbin' Goblin | 220 |  |
| Seven Tools of the Bandit | 220 |  |
| Soul Exchange | 220 |  |
| Thunder Dragon | 220 |  |
| Flame Swordsman | 213 |  |
| Cyber Saurus | 211 |  |
| Charubin the Fire Knight | 209 |  |
| Flame Ghost | 209 |  |
| Bickuribox | 207 |  |
| Skull Knight | 207 |  |
| Gaia the Dragon Champion | 204 |  |
| Empress Judge | 200 |  |
| Fusionist | 200 |  |
| Giltia the D. Knight | 200 |  |
| Rabid Horseman | 200 |  |

### Android

| Card | DP | Limit |
|---|---:|---|
| Call Of The Haunted | 3000 | Limited |
| Change of Heart | 3000 | Limited |
| Confiscation | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Delinquent Duo | 3000 | Limited |
| Jinzo | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Imperial Order | 2926 | Limited |
| Snatch Steal | 2926 | Limited |
| Painful Choice | 2853 | Limited |
| Morphing Jar | 1746 | Limited |
| Heavy Storm | 1599 | Semi-Limited |
| Nobleman of Crossout | 1599 | Semi-Limited |
| Premature Burial | 1599 | Limited |
| Cyber Jar | 1525 | Limited |
| Ceasefire | 1156 | Limited |
| Kycoo the Ghost Destroyer | 1132 |  |
| Mystical Space Typhoon | 1132 |  |
| Bazoo the Soul-Eater | 1107 |  |
| Gemini Elf | 1107 |  |
| Torrential Tribute | 1107 |  |
| Fissure | 1083 |  |
| Mystic Tomato | 1083 |  |
| Jar of Greed | 1058 |  |
| Skull Lair | 1033 |  |
| Magician of Faith | 1009 |  |
| Swords of Revealing Light | 1009 | Limited |
| Trap Hole | 1009 |  |
| Dust Tornado | 935 |  |
| Magic Cylinder | 935 | Limited |
| Cannon Soldier | 665 |  |
| Solemn Judgment | 615 |  |
| Card Destruction | 566 | Limited |
| Time Seal | 542 |  |
| 4-Starred Ladybug of Doom | 493 |  |
| Masked Sorcerer | 468 |  |
| Penguin Knight | 468 |  |
| United We Stand | 419 | Limited |
| Exchange | 370 |  |
| Solemn Wishes | 370 |  |
| Mage Power | 345 | Limited |
| Upstart Goblin | 345 | Limited |
| Zombyra the Dark | 345 |  |
| Man-Eater Bug | 320 |  |
| Mask of Darkness | 320 |  |
| Seven Tools of the Bandit | 320 |  |
| Fairy Box | 271 |  |
| Goblin Attack Force | 271 |  |
| Jirai Gumo | 271 |  |
| Magic Drain | 271 |  |
| Nimble Momonga | 271 |  |
| Waboku | 271 |  |
| White Magical Hat | 271 |  |
| Banisher of the Light | 247 |  |
| Kuriboh | 247 |  |
| Nobleman of Extermination | 247 |  |
| The Last Warrior from Another Planet | 242 |  |
| B. Skull Dragon | 234 |  |
| Thousand-Eyes Restrict | 232 |  |
| Gaia the Dragon Champion | 229 |  |
| Flame Swordsman | 227 |  |
| Axe of Despair | 222 |  |
| Castle of Dark Illusions | 222 |  |
| Dark Elf | 222 |  |
| Fake Trap | 222 |  |
| Firegrass | 222 |  |
| Giant Germ | 222 |  |
| Giant Soldier of Stone | 222 |  |
| Giant Trunade | 222 |  |
| La Jinn the Mystical Genie of the Lamp | 222 |  |
| Megamorph | 222 |  |
| Mirror Wall | 222 |  |
| Soul Release | 222 |  |
| Summoned Skull | 222 |  |
| The Fiend Megacyber | 222 |  |
| Thunder Dragon | 222 |  |
| Darkfire Dragon | 220 |  |
| Dragoness the Wicked Knight | 217 |  |
| Twin-Headed Thunder Dragon | 217 |  |
| Fusionist | 215 |  |
| Warrior of Tradition | 215 |  |
| Musician King | 212 |  |
| Bickuribox | 210 |  |
| Charubin the Fire Knight | 207 |  |
| Metal Dragon | 207 |  |
| Thousand Dragon | 207 |  |
| Cyber Saurus | 205 |  |
| Kaminari Attack | 205 |  |
| Labyrinth Tank | 205 |  |
| Skull Knight | 205 |  |
| Kwagar Hercules | 202 |  |
| Flame Ghost | 200 |  |
| Flower Wolf | 200 |  |
| Giltia the D. Knight | 200 |  |
| Karbonala Warrior | 200 |  |
| Punished Eagle | 200 |  |

### Joey-Pegasus

| Card | DP | Limit |
|---|---:|---|
| Call Of The Haunted | 3000 | Limited |
| Change of Heart | 3000 | Limited |
| Confiscation | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Delinquent Duo | 3000 | Limited |
| Harpie's Feather Duster | 3000 | Limited |
| Imperial Order | 3000 | Limited |
| Jinzo | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Limited |
| Sinister Serpent | 3000 | Limited |
| Snatch Steal | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Painful Choice | 2852 | Limited |
| Premature Burial | 2852 | Limited |
| Heavy Storm | 2409 | Limited |
| Graceful Charity | 1598 | Semi-Limited |
| Nobleman of Crossout | 1524 | Semi-Limited |
| Morphing Jar | 1376 | Limited |
| Bazoo the Soul-Eater | 1130 |  |
| Gemini Elf | 1130 |  |
| Kycoo the Ghost Destroyer | 1130 |  |
| Magician of Faith | 1130 |  |
| Mystic Tomato | 1130 |  |
| Mystical Space Typhoon | 1130 |  |
| Cyber Jar | 1081 | Limited |
| Torrential Tribute | 1081 |  |
| Ceasefire | 933 | Limited |
| Jar of Greed | 933 |  |
| Magic Cylinder | 933 | Limited |
| Fissure | 884 |  |
| Skull Lair | 884 |  |
| Dust Tornado | 687 |  |
| Solemn Judgment | 687 |  |
| Cannon Soldier | 638 |  |
| Mage Power | 638 | Limited |
| Trap Hole | 638 |  |
| Upstart Goblin | 638 | Limited |
| Waboku | 589 |  |
| Scapegoat | 540 |  |
| Man-Eater Bug | 490 |  |
| United We Stand | 490 | Limited |
| Time Seal | 441 |  |
| Card Destruction | 343 | Limited |
| Exodia the Forbidden One | 343 | Limited |
| Left Arm of the Forbidden One | 343 | Limited |
| Left Leg of the Forbidden One | 343 | Limited |
| Limiter Removal | 343 | Limited |
| Right Arm of the Forbidden One | 343 | Limited |
| Right Leg of the Forbidden One | 343 | Limited |
| Swords of Revealing Light | 343 | Limited |
| Exchange | 293 |  |
| Kuriboh | 293 |  |
| Masked Sorcerer | 293 |  |
| Mirror Wall | 293 |  |
| Nimble Momonga | 293 |  |
| Seven Tools of the Bandit | 293 |  |
| Thunder Dragon | 293 |  |
| Backup Soldier | 269 | Semi-Limited |
| The Last Warrior from Another Planet | 249 |  |
| Armed Ninja | 244 |  |
| Axe of Despair | 244 |  |
| Cold Wave | 244 |  |
| Giant Germ | 244 |  |
| Giant Rat | 244 |  |
| Giant Trunade | 244 |  |
| Guardian Sphinx | 244 |  |
| Kotodama | 244 |  |
| Last Will | 244 |  |
| Magic Drain | 244 |  |
| Penguin Knight | 244 |  |
| Reinforcement of the Army | 244 |  |
| Rush Recklessly | 244 |  |
| Solemn Wishes | 244 |  |
| Summoned Skull | 244 |  |
| White Magical Hat | 244 |  |
| B. Skull Dragon | 239 |  |
| Flame Swordsman | 230 |  |
| Gaia the Dragon Champion | 230 |  |
| Thousand-Eyes Restrict | 220 |  |
| Darkfire Dragon | 210 |  |
| Fusionist | 210 |  |
| Labyrinth Tank | 210 |  |
| Cyber Saurus | 205 |  |
| Dragoness the Wicked Knight | 205 |  |
| Metal Dragon | 205 |  |
| Musician King | 205 |  |
| Skull Knight | 205 |  |
| Bickuribox | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Empress Judge | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Flame Ghost | 200 |  |
| Flower Wolf | 200 |  |
| Giltia the D. Knight | 200 |  |
| Kaminari Attack | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Ryu Senshi | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Warrior of Tradition | 200 |  |

### Fiber

| Card | DP | Limit |
|---|---:|---|
| Dark Hole | 3000 | Limited |
| Imperial Order | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Harpie's Feather Duster | 2985 | Limited |
| Snatch Steal | 2985 | Limited |
| Call Of The Haunted | 2970 | Limited |
| Change of Heart | 2970 | Limited |
| Exiled Force | 2970 | Limited |
| Fiber Jar | 2970 | Limited |
| Monster Reborn | 2970 | Limited |
| Delinquent Duo | 2940 | Limited |
| Injection Fairy Lily | 2940 | Limited |
| Confiscation | 2925 | Limited |
| Jinzo | 2880 | Limited |
| Sinister Serpent | 2880 | Limited |
| Painful Choice | 2789 | Limited |
| Yata-Garasu | 2654 | Limited |
| Heavy Storm | 2292 | Limited |
| Premature Burial | 2127 | Limited |
| Graceful Charity | 1600 | Semi-Limited |
| Nobleman of Crossout | 1517 | Semi-Limited |
| Morphing Jar | 1479 | Limited |
| Cyber Jar | 1464 | Limited |
| Magic Cylinder | 1404 | Limited |
| Torrential Tribute | 1133 |  |
| Mystical Space Typhoon | 1128 |  |
| Mystic Tomato | 1068 |  |
| Bazoo the Soul-Eater | 1013 |  |
| Kycoo the Ghost Destroyer | 997 |  |
| Gemini Elf | 972 |  |
| Magician of Faith | 972 |  |
| Dust Tornado | 892 |  |
| Scapegoat | 892 |  |
| Creature Swap | 885 | Semi-Limited |
| Ceasefire | 877 | Limited |
| Trap Hole | 877 |  |
| Asura Priest | 872 |  |
| Fissure | 852 |  |
| Jar of Greed | 726 |  |
| Card Destruction | 531 | Limited |
| Skull Lair | 501 |  |
| Upstart Goblin | 471 | Limited |
| United We Stand | 440 | Limited |
| Spear Dragon | 435 |  |
| Cannon Soldier | 425 |  |
| Time Seal | 400 |  |
| Giant Rat | 395 |  |
| Mage Power | 365 | Limited |
| Limiter Removal | 350 | Limited |
| Dark Ruler Ha Des | 345 |  |
| Airknight Parshath | 335 |  |
| 4-Starred Ladybug of Doom | 305 |  |
| Solemn Judgment | 305 |  |
| Goblin Attack Force | 285 |  |
| Reinforcement of the Army | 282 | Semi-Limited |
| Swords of Revealing Light | 275 | Limited |
| Thunder Dragon | 275 |  |
| Zombyra the Dark | 275 |  |
| Seven Tools of the Bandit | 270 |  |
| Magic Jammer | 265 |  |
| Thousand-Eyes Restrict | 253 |  |
| The Last Warrior from Another Planet | 251 |  |
| Giant Germ | 250 |  |
| Chain Energy | 245 |  |
| Electric Snake | 245 |  |
| The Fiend Megacyber | 245 |  |
| Waboku | 245 |  |
| Blast with Chain | 240 |  |
| Gearfried the Iron Knight | 240 |  |
| Tribute to The Doomed | 240 |  |
| Ryu Senshi | 238 |  |
| Twin-Headed Thunder Dragon | 238 |  |
| B. Skull Dragon | 233 |  |
| Dark Balter the Terrible | 232 |  |
| Dragoness the Wicked Knight | 232 |  |
| Fiend Skull Dragon | 230 |  |
| Exchange | 225 |  |
| Darkfire Dragon | 221 |  |
| Magic Drain | 220 |  |
| Labyrinth Tank | 218 |  |
| Musician King | 217 |  |
| Acid Trap Hole | 215 |  |
| Last Will | 215 |  |
| Nimble Momonga | 215 |  |
| Solemn Wishes | 215 |  |
| Soul Exchange | 215 |  |
| Gaia the Dragon Champion | 213 |  |
| Roaring Ocean Snake | 213 |  |
| Giant Trunade | 210 |  |
| Graverobber | 210 |  |
| Kuriboh | 210 |  |
| Mask of Darkness | 210 |  |
| Princess of Tsurugi | 210 |  |
| Thousand Dragon | 210 |  |
| White Magical Hat | 210 |  |
| Fusionist | 209 |  |
| Flame Ghost | 206 |  |
| Black Illusion Ritual | 205 |  |
| Bottomless Trap Hole | 205 |  |
| Don Zaloog | 205 |  |
| Flame Swordsman | 205 |  |
| Flash Assailant | 205 |  |
| Flying Kamakiri #1 | 205 |  |
| Graceful Dice | 205 |  |
| Gravity Bind | 205 |  |
| Jirai Gumo | 205 |  |
| Man-Eater Bug | 205 |  |
| Megamorph | 205 |  |
| Metalmorph | 205 |  |
| Mirror Wall | 205 |  |
| Nobleman of Extermination | 205 |  |
| Relinquished | 205 |  |
| Rush Recklessly | 205 |  |
| Senju of the Thousand Hands | 205 |  |
| Sonic Bird | 205 |  |
| Summoned Skull | 205 |  |
| Bickuribox | 204 |  |
| Charubin the Fire Knight | 204 |  |
| Cyber Saurus | 204 |  |
| Skull Knight | 204 |  |
| Warrior of Tradition | 204 |  |
| Metal Dragon | 203 |  |
| Empress Judge | 201 |  |
| Flower Wolf | 201 |  |
| Kwagar Hercules | 201 |  |
| Punished Eagle | 201 |  |
| Giltia the D. Knight | 200 |  |

### Yata

| Card | DP | Limit |
|---|---:|---|
| Bell of Destruction | 3000 | Limited |
| Change of Heart | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Graceful Charity | 3000 | Limited |
| Harpie's Feather Duster | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Snatch Steal | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Imperial Order | 2964 | Limited |
| Mirage of Nightmare | 2964 | Limited |
| Jinzo | 2928 | Limited |
| Monster Reborn | 2928 | Limited |
| Delinquent Duo | 2892 | Limited |
| Exiled Force | 2892 | Limited |
| Sangan | 2892 | Limited |
| Confiscation | 2856 | Limited |
| Call Of The Haunted | 2749 | Limited |
| Heavy Storm | 2713 | Limited |
| Injection Fairy Lily | 2605 | Limited |
| Fiber Jar | 2569 | Limited |
| Sinister Serpent | 2425 | Limited |
| Painful Choice | 2246 | Limited |
| Yata-Garasu | 2246 | Limited |
| Nobleman of Crossout | 1581 | Semi-Limited |
| Ceasefire | 1312 | Limited |
| Premature Burial | 1312 | Limited |
| Mystical Space Typhoon | 1133 |  |
| Torrential Tribute | 1109 |  |
| Spirit Reaper | 1013 |  |
| Mystic Tomato | 929 |  |
| Morphing Jar | 917 | Limited |
| Don Zaloog | 893 |  |
| Gemini Elf | 893 |  |
| Reinforcement of the Army | 881 | Semi-Limited |
| Magic Cylinder | 845 | Limited |
| Magician of Faith | 833 |  |
| Zombyra the Dark | 809 |  |
| Gravekeeper's Spy | 761 |  |
| Dust Tornado | 737 |  |
| Cyber Jar | 666 | Limited |
| Scapegoat | 666 |  |
| Fissure | 594 |  |
| Creature Swap | 576 | Semi-Limited |
| Bazoo the Soul-Eater | 534 |  |
| Kycoo the Ghost Destroyer | 534 |  |
| Book of Moon | 522 |  |
| King Tiger Wanghu | 522 |  |
| Asura Priest | 474 |  |
| Dark Ruler Ha Des | 450 |  |
| Newdoria | 450 |  |
| Jar of Greed | 426 |  |
| Swords of Revealing Light | 378 | Limited |
| Electric Snake | 366 |  |
| Airknight Parshath | 354 |  |
| Metamorphosis | 342 |  |
| Upstart Goblin | 342 | Limited |
| Cannon Soldier | 330 |  |
| Trap Hole | 330 |  |
| 4-Starred Ladybug of Doom | 319 |  |
| Solemn Wishes | 319 |  |
| Raigeki Break | 307 |  |
| Solemn Judgment | 307 |  |
| Time Seal | 307 |  |
| Trap Dustshoot | 307 |  |
| Blast with Chain | 295 |  |
| Gearfried the Iron Knight | 295 |  |
| Goblin Attack Force | 283 |  |
| Sasuke Samurai | 283 |  |
| Seven Tools of the Bandit | 283 |  |
| Thousand-Eyes Restrict | 278 |  |
| Ryu Senshi | 273 |  |
| Card Destruction | 271 | Limited |
| Chain Energy | 271 |  |
| Dark Balter the Terrible | 268 |  |
| Fiend Skull Dragon | 265 |  |
| Guardian Sphinx | 259 |  |
| Reaper on the Nightmare | 251 |  |
| The Last Warrior from Another Planet | 249 |  |
| Giant Germ | 247 |  |
| Nimble Momonga | 247 |  |
| B. Skull Dragon | 241 |  |
| Barrel Dragon | 235 |  |
| Giant Rat | 235 |  |
| Kuriboh | 235 |  |
| Limiter Removal | 235 |  |
| Marauding Captain | 235 | Semi-Limited |
| Mechanicalchaser | 235 |  |
| Pyramid Turtle | 235 |  |
| Reasoning | 235 |  |
| Spear Dragon | 235 |  |
| United We Stand | 235 | Limited |
| White Magical Hat | 235 |  |
| Dragoness the Wicked Knight | 234 |  |
| Darkfire Dragon | 229 |  |
| Gaia the Dragon Champion | 225 |  |
| Book of Life | 223 |  |
| Giant Trunade | 223 |  |
| Magic Jammer | 223 |  |
| Princess of Tsurugi | 223 |  |
| Twin-Headed Thunder Dragon | 223 |  |
| Waboku | 223 |  |
| Morphing Jar #2 | 217 | Semi-Limited |
| Bickuribox | 213 |  |
| Empress Judge | 212 |  |
| Flame Swordsman | 212 |  |
| Roaring Ocean Snake | 212 |  |
| Cold Wave | 211 |  |
| Forced Requisition | 211 |  |
| Graverobber | 211 |  |
| Just Desserts | 211 |  |
| Last Will | 211 |  |
| Magic Drain | 211 |  |
| Mask of Darkness | 211 |  |
| Masked Sorcerer | 211 |  |
| Mirror Wall | 211 |  |
| Musician King | 211 |  |
| Cyber Saurus | 210 |  |
| Karbonala Warrior | 208 |  |
| Flame Ghost | 207 |  |
| Labyrinth Tank | 207 |  |
| Charubin the Fire Knight | 206 |  |
| Flower Wolf | 205 |  |
| Giltia the D. Knight | 205 |  |
| Metal Dragon | 204 |  |
| Thousand Dragon | 204 |  |
| Fusionist | 202 |  |
| Skull Knight | 202 |  |
| Punished Eagle | 201 |  |
| Warrior of Tradition | 200 |  |

### Scientist

| Card | DP | Limit |
|---|---:|---|
| Bell of Destruction | 3000 | Limited |
| Change of Heart | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Graceful Charity | 3000 | Limited |
| Harpie's Feather Duster | 3000 | Limited |
| Imperial Order | 3000 | Limited |
| Painful Choice | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Limited |
| Snatch Steal | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Call Of The Haunted | 2766 | Limited |
| Fiber Jar | 2766 | Limited |
| Mirror Force | 2766 | Limited |
| Monster Reborn | 2766 | Limited |
| Tribe-Infecting Virus | 2766 | Limited |
| Breaker the Magical Warrior | 2532 | Limited |
| Mirage of Nightmare | 2532 | Limited |
| Confiscation | 2298 | Limited |
| Delinquent Duo | 2298 | Limited |
| Exiled Force | 2298 | Limited |
| Jinzo | 2298 | Limited |
| Sinister Serpent | 2298 | Limited |
| Injection Fairy Lily | 2064 | Limited |
| The Forceful Sentry | 2064 | Limited |
| Heavy Storm | 1830 | Limited |
| Yata-Garasu | 1830 | Limited |
| Premature Burial | 1596 | Limited |
| Card Destruction | 1128 | Limited |
| Ceasefire | 1128 | Limited |
| Magical Scientist | 1128 |  |
| Mystical Space Typhoon | 1128 |  |
| Torrential Tribute | 1050 |  |
| Nobleman of Crossout | 1011 | Semi-Limited |
| Mystic Tomato | 894 |  |
| Book of Moon | 738 |  |
| Catapult Turtle | 738 |  |
| King Tiger Wanghu | 738 |  |
| Cyber Jar | 660 | Limited |
| Dust Tornado | 660 |  |
| Last Will | 660 |  |
| Spirit Reaper | 660 |  |
| Gravekeeper's Spy | 582 |  |
| Metamorphosis | 582 |  |
| Trap Dustshoot | 582 |  |
| Gilasaurus | 504 |  |
| Magician of Faith | 504 |  |
| Newdoria | 504 |  |
| Cannon Soldier | 426 |  |
| Fissure | 426 |  |
| Giant Trunade | 426 |  |
| Poison of the Old Man | 426 |  |
| Raigeki Break | 426 |  |
| Reasoning | 426 |  |
| Swords of Revealing Light | 426 | Limited |
| Upstart Goblin | 426 | Limited |
| Airknight Parshath | 348 |  |
| Goblin Attack Force | 348 |  |
| Jar of Greed | 348 |  |
| Royal Decree | 348 |  |
| Scapegoat | 348 |  |
| Creature Swap | 309 | Semi-Limited |
| Reinforcement of the Army | 309 | Semi-Limited |
| Dark Balter the Terrible | 286 |  |
| Ryu Senshi | 286 |  |
| Thousand-Eyes Restrict | 286 |  |
| Empress Judge | 278 |  |
| Fiend Skull Dragon | 278 |  |
| Punished Eagle | 278 |  |
| Roaring Ocean Snake | 278 |  |
| Apprentice Magician | 270 |  |
| Asura Priest | 270 |  |
| Dark Magician | 270 |  |
| Dark Ruler Ha Des | 270 |  |
| Don Zaloog | 270 |  |
| Kycoo the Ghost Destroyer | 270 |  |
| Mask of Restrict | 270 |  |
| Nimble Momonga | 270 |  |
| Sasuke Samurai | 270 |  |
| Skilled Dark Magician | 270 |  |
| Spell Canceller | 270 |  |
| Waboku | 270 |  |
| Dragoness the Wicked Knight | 255 |  |
| Darkfire Dragon | 239 |  |
| Dark Paladin | 200 |  |
| Reaper on the Nightmare | 200 |  |
| The Last Warrior from Another Planet | 200 |  |

### Vampire

| Card | DP | Limit |
|---|---:|---|
| Breaker the Magical Warrior | 3000 | Limited |
| Change of Heart | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Graceful Charity | 3000 | Limited |
| Harpie's Feather Duster | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Bell of Destruction | 2878 | Limited |
| Confiscation | 2878 | Limited |
| Exiled Force | 2878 | Limited |
| Imperial Order | 2878 | Limited |
| Snatch Steal | 2878 | Limited |
| Call Of The Haunted | 2756 | Limited |
| Magical Scientist | 2756 | Limited |
| Mirage of Nightmare | 2756 | Limited |
| Sinister Serpent | 2756 | Limited |
| Tribe-Infecting Virus | 2756 | Limited |
| Delinquent Duo | 2634 | Limited |
| Fiber Jar | 2634 | Limited |
| Jinzo | 2634 | Limited |
| Heavy Storm | 2512 | Limited |
| Injection Fairy Lily | 2390 | Limited |
| Yata-Garasu | 2147 | Limited |
| Painful Choice | 1903 | Limited |
| Nobleman of Crossout | 1476 | Semi-Limited |
| Premature Burial | 1415 | Limited |
| Reinforcement of the Army | 1415 | Semi-Limited |
| Mystical Space Typhoon | 1131 |  |
| Torrential Tribute | 1131 |  |
| D.D. Warrior Lady | 1049 |  |
| Zombyra the Dark | 846 |  |
| Ceasefire | 806 | Limited |
| Don Zaloog | 806 |  |
| Gravekeeper's Spy | 806 |  |
| Sakuretsu Armor | 724 |  |
| Royal Decree | 684 |  |
| Book of Moon | 643 |  |
| Magician of Faith | 602 |  |
| Cyber Jar | 562 | Limited |
| King Tiger Wanghu | 562 |  |
| Dust Tornado | 480 |  |
| Sasuke Samurai | 480 |  |
| Card Destruction | 440 | Limited |
| Magic Cylinder | 440 | Limited |
| Metamorphosis | 399 |  |
| Scapegoat | 399 |  |
| Mystic Tomato | 358 |  |
| The Little Swordsman of Aile | 358 |  |
| Dark Ruler Ha Des | 318 |  |
| Mage Power | 318 | Limited |
| Poison of the Old Man | 318 |  |
| Swords of Revealing Light | 318 | Limited |
| Trap Dustshoot | 318 |  |
| Upstart Goblin | 318 | Limited |
| Dark Balter the Terrible | 289 |  |
| Ryu Senshi | 289 |  |
| Thousand-Eyes Restrict | 289 |  |
| Fiend Skull Dragon | 285 |  |
| Dark Flare Knight | 277 |  |
| Giant Trunade | 277 |  |
| Jar of Greed | 277 |  |
| Just Desserts | 277 |  |
| Kycoo the Ghost Destroyer | 277 |  |
| Solemn Judgment | 277 |  |
| Trap Hole | 277 |  |
| Creature Swap | 257 | Semi-Limited |
| Marauding Captain | 257 | Semi-Limited |
| Morphing Jar | 257 | Semi-Limited |
| Reaper on the Nightmare | 249 |  |
| Book of Life | 237 |  |
| Bottomless Trap Hole | 237 |  |
| Cannon Soldier | 237 |  |
| Catapult Turtle | 237 |  |
| Cipher Soldier | 237 |  |
| Cold Wave | 237 |  |
| Dark Scorpion - Cliff the Trap Remover | 237 |  |
| Des Koala | 237 |  |
| Electric Snake | 237 |  |
| Gemini Elf | 237 |  |
| Goblin Attack Force | 237 |  |
| Guardian Sphinx | 237 |  |
| Gyaku-Gire Panda | 237 |  |
| Last Will | 237 |  |
| Masked Sorcerer | 237 |  |
| Mother Grizzly | 237 |  |
| Ojama Trio | 237 |  |
| Pyramid Turtle | 237 |  |
| Raigeki Break | 237 |  |
| Secret Barrel | 237 |  |
| Spirit Reaper | 237 |  |
| Summoned Skull | 237 |  |
| Time Seal | 237 |  |
| Waboku | 237 |  |
| White Magical Hat | 237 |  |
| Darkfire Dragon | 233 |  |
| Dragoness the Wicked Knight | 233 |  |
| Roaring Ocean Snake | 228 |  |
| The Last Warrior from Another Planet | 228 |  |
| Punished Eagle | 208 |  |
| B. Skull Dragon | 204 |  |
| Flame Ghost | 204 |  |
| Twin-Headed Thunder Dragon | 204 |  |
| Empress Judge | 200 |  |
| Giltia the D. Knight | 200 |  |
| Skull Knight | 200 |  |

### Chaos

| Card | DP | Limit |
|---|---:|---|
| Graceful Charity | 3000 | Limited |
| Imperial Order | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Bell of Destruction | 2968 | Limited |
| Black Luster Soldier - Envoy of Beginning | 2968 | Limited |
| Change of Heart | 2968 | Limited |
| Chaos Emperor Dragon - Envoy of the End | 2968 | Limited |
| Harpie's Feather Duster | 2936 | Limited |
| Witch of the Black Forest | 2936 | Limited |
| The Forceful Sentry | 2903 | Limited |
| Dark Hole | 2871 | Limited |
| Painful Choice | 2871 | Limited |
| Tribe-Infecting Virus | 2871 | Limited |
| Snatch Steal | 2839 | Limited |
| Mirror Force | 2807 | Limited |
| Breaker the Magical Warrior | 2742 | Limited |
| Delinquent Duo | 2742 | Limited |
| Confiscation | 2710 | Limited |
| Sangan | 2710 | Limited |
| Mirage of Nightmare | 2678 | Limited |
| Sinister Serpent | 2517 | Limited |
| Call Of The Haunted | 2453 | Limited |
| Fiber Jar | 2195 | Limited |
| Heavy Storm | 2002 | Limited |
| Premature Burial | 1905 | Limited |
| Jinzo | 1744 | Limited |
| Magical Scientist | 1712 | Limited |
| Injection Fairy Lily | 1326 | Limited |
| Nobleman of Crossout | 1326 | Semi-Limited |
| Yata-Garasu | 1197 | Limited |
| Mystical Space Typhoon | 1133 |  |
| Card Destruction | 1100 | Limited |
| Exiled Force | 1068 | Limited |
| Kycoo the Ghost Destroyer | 1036 |  |
| D.D. Warrior Lady | 1015 |  |
| Magician of Faith | 1015 |  |
| Torrential Tribute | 982 |  |
| Gravekeeper's Spy | 800 |  |
| Cyber Jar | 778 | Limited |
| Trap Dustshoot | 628 |  |
| Upstart Goblin | 617 | Limited |
| Thunder Dragon | 553 |  |
| Reinforcement of the Army | 537 | Semi-Limited |
| Raigeki Break | 521 |  |
| Magic Cylinder | 489 | Limited |
| Morphing Jar | 489 | Semi-Limited |
| Ceasefire | 424 | Limited |
| Zombyra the Dark | 424 |  |
| Asura Priest | 403 |  |
| Royal Decree | 392 |  |
| Scapegoat | 392 |  |
| Sakuretsu Armor | 381 |  |
| Dark Magician of Chaos | 360 |  |
| Swords of Revealing Light | 360 | Limited |
| Waboku | 360 |  |
| Chaos Sorcerer | 349 |  |
| Solemn Judgment | 349 |  |
| Dust Tornado | 338 |  |
| Poison of the Old Man | 338 |  |
| Spirit Reaper | 338 |  |
| Tsukuyomi | 328 |  |
| Cold Wave | 317 |  |
| Metamorphosis | 306 |  |
| Book of Moon | 296 |  |
| Creature Swap | 296 | Semi-Limited |
| Thunder Nyan Nyan | 296 |  |
| Bottomless Trap Hole | 285 |  |
| Dark Balter the Terrible | 285 |  |
| Don Zaloog | 285 |  |
| King Tiger Wanghu | 285 |  |
| Thousand-Eyes Restrict | 285 |  |
| Ryu Senshi | 284 |  |
| Fiend Skull Dragon | 279 |  |
| Airknight Parshath | 274 |  |
| Exchange | 274 |  |
| Dark Flare Knight | 269 |  |
| Berserk Gorilla | 263 |  |
| Cipher Soldier | 263 |  |
| Sasuke Samurai | 263 |  |
| Skilled Dark Magician | 263 |  |
| 4-Starred Ladybug of Doom | 253 |  |
| Dimension Fusion | 253 |  |
| Mystic Tomato | 253 |  |
| Prohibition | 242 |  |
| Roaring Ocean Snake | 242 |  |
| Royal Oppression | 242 |  |
| Time Seal | 242 |  |
| Emergency Provisions | 231 |  |
| Giant Trunade | 231 |  |
| Roulette Barrel | 231 |  |
| Soul Release | 231 |  |
| Spear Dragon | 231 |  |
| The Shallow Grave | 231 |  |
| Giltia the D. Knight | 225 |  |
| Reaper on the Nightmare | 224 |  |
| Axe of Despair | 220 |  |
| Black Illusion Ritual | 220 |  |
| Enemy Controller | 220 |  |
| Jar of Greed | 220 |  |
| Magic Jammer | 220 |  |
| Masked Sorcerer | 220 |  |
| Relinquished | 220 |  |
| Darkfire Dragon | 216 |  |
| Dragoness the Wicked Knight | 212 |  |
| Flame Ghost | 212 |  |
| Catapult Turtle | 210 |  |
| Dark Ruler Ha Des | 210 |  |
| Electric Snake | 210 |  |
| Gaia the Dragon Champion | 210 |  |
| Gigantes | 210 |  |
| Goblin Attack Force | 210 |  |
| Hysteric Fairy | 210 |  |
| Kuriboh | 210 |  |
| Magical Merchant | 210 |  |
| Musician King | 210 |  |
| Pyramid Turtle | 210 |  |
| Return from the Different Dimension | 210 |  |
| Spell Canceller | 210 |  |
| The Last Warrior from Another Planet | 210 |  |
| White Magical Hat | 210 |  |
| Zaborg the Thunder Monarch | 210 |  |
| B. Skull Dragon | 209 |  |
| Fusionist | 209 |  |
| Twin-Headed Thunder Dragon | 209 |  |
| Dark Paladin | 204 |  |
| Empress Judge | 204 |  |
| Karbonala Warrior | 204 |  |
| Punished Eagle | 203 |  |
| Flame Swordsman | 202 |  |
| Skull Knight | 202 |  |
| Bickuribox | 201 |  |
| Labyrinth Tank | 201 |  |
| Metal Dragon | 201 |  |
| Warrior of Tradition | 201 |  |
| Deepsea Shark | 200 |  |

### Warrior

| Card | DP | Limit |
|---|---:|---|
| Pot of Greed | 3000 | Limited |
| Heavy Storm | 2926 | Limited |
| Bell of Destruction | 2889 | Limited |
| Change of Heart | 2816 | Limited |
| Mystical Space Typhoon | 2816 | Limited |
| Snatch Steal | 2779 | Limited |
| Premature Burial | 2705 | Limited |
| Breaker the Magical Warrior | 2668 | Limited |
| Torrential Tribute | 2668 | Limited |
| The Forceful Sentry | 2631 | Limited |
| Tribe-Infecting Virus | 2595 | Limited |
| Confiscation | 2558 | Limited |
| Call Of The Haunted | 2521 | Limited |
| Painful Choice | 2484 | Limited |
| Black Luster Soldier - Envoy of Beginning | 2373 | Limited |
| Magical Scientist | 2042 | Limited |
| Jinzo | 1968 | Limited |
| Exiled Force | 1784 | Limited |
| Sinister Serpent | 1710 | Limited |
| Fiber Jar | 1673 | Limited |
| Swords of Revealing Light | 1526 | Limited |
| Mirage of Nightmare | 1452 | Limited |
| Nobleman of Crossout | 1415 | Semi-Limited |
| Dark Magician of Chaos | 936 | Limited |
| Reinforcement of the Army | 936 | Semi-Limited |
| D.D. Warrior Lady | 911 |  |
| Magic Cylinder | 899 | Limited |
| Injection Fairy Lily | 862 | Limited |
| Ceasefire | 825 | Limited |
| Dust Tornado | 813 |  |
| Sakuretsu Armor | 813 |  |
| Book of Moon | 801 |  |
| Cipher Soldier | 789 |  |
| Mobius the Frost Monarch | 739 |  |
| Scapegoat | 739 |  |
| Smashing Ground | 739 |  |
| Kycoo the Ghost Destroyer | 727 |  |
| Blade Knight | 678 |  |
| Card Destruction | 678 | Limited |
| Creature Swap | 659 | Semi-Limited |
| Upstart Goblin | 641 | Limited |
| Magician of Faith | 616 |  |
| Morphing Jar | 604 | Limited |
| Mystic Swordsman LV2 | 580 |  |
| Airknight Parshath | 543 |  |
| Bottomless Trap Hole | 543 |  |
| Don Zaloog | 543 |  |
| King Tiger Wanghu | 530 |  |
| Emergency Provisions | 506 |  |
| Cyber Jar | 494 | Limited |
| Gravekeeper's Spy | 469 |  |
| Royal Decree | 469 |  |
| Solemn Judgment | 469 |  |
| Trap Dustshoot | 469 |  |
| Raigeki Break | 457 |  |
| Enemy Controller | 432 |  |
| Giant Trunade | 432 |  |
| Metamorphosis | 432 |  |
| Mystik Wok | 420 |  |
| Berserk Gorilla | 408 |  |
| Asura Priest | 395 |  |
| Ninja Grandmaster Sasuke | 395 |  |
| Zombyra the Dark | 395 |  |
| D.D. Assailant | 383 |  |
| Chaos Sorcerer | 371 |  |
| Spirit Reaper | 371 |  |
| Jar of Greed | 358 |  |
| Return from the Different Dimension | 346 |  |
| Tsukuyomi | 346 |  |
| Monster Gate | 322 |  |
| Monster Reincarnation | 322 |  |
| Pikeru's Circle of Enchantment | 322 |  |
| Reasoning | 309 |  |
| Giant Rat | 285 |  |
| Hallowed Life Barrier | 285 |  |
| Zaborg the Thunder Monarch | 285 |  |
| Dark Balter the Terrible | 281 |  |
| Ryu Senshi | 280 |  |
| Dark Blade the Dragon Knight | 277 |  |
| Thousand-Eyes Restrict | 277 |  |
| Dekoichi the Battlechanted Locomotive | 272 |  |
| Dimension Fusion | 272 |  |
| Divine Wrath | 272 |  |
| Gyaku-Gire Panda | 272 |  |
| Magic Drain | 272 |  |
| Widespread Ruin | 272 |  |
| Fiend Skull Dragon | 269 |  |
| Abyss Soldier | 260 |  |
| Catapult Turtle | 260 |  |
| Compulsory Evacuation Device | 260 |  |
| Gigantes | 260 |  |
| Reaper on the Nightmare | 260 |  |
| Sacred Crane | 260 |  |
| Skilled Dark Magician | 260 |  |
| Token Thanksgiving | 260 |  |
| Xing Zhen Hu | 260 |  |
| Dark Flare Knight | 257 |  |
| Darkfire Dragon | 250 |  |
| Ojama King | 249 |  |
| Cannon Soldier | 248 |  |
| Gilasaurus | 248 |  |
| Goblin Attack Force | 248 |  |
| Just Desserts | 248 |  |
| Kuriboh | 248 |  |
| Last Will | 248 |  |
| Level Limit - Area B | 248 |  |
| Mask of Darkness | 248 |  |
| Masked Sorcerer | 248 |  |
| Mystic Plasma Zone | 248 |  |
| Ojama Trio | 248 |  |
| Secret Barrel | 248 |  |
| Waboku | 248 |  |
| Dragoness the Wicked Knight | 241 |  |
| The Last Warrior from Another Planet | 237 |  |
| Apprentice Magician | 236 |  |
| Gravity Bind | 236 |  |
| Magic Jammer | 236 |  |
| Messenger of Peace | 236 |  |
| Mystic Tomato | 236 |  |
| Sasuke Samurai | 236 |  |
| Spear Dragon | 236 |  |
| Spell Canceller | 236 |  |
| Wave-Motion Cannon | 236 |  |
| Giltia the D. Knight | 228 |  |
| Roaring Ocean Snake | 226 |  |
| Master of Oz | 225 |  |
| Big Shield Gardna | 223 |  |
| Blast with Chain | 223 |  |
| Chain Disappearance | 223 |  |
| Dark Magician | 223 |  |
| Dark Scorpion - Cliff the Trap Remover | 223 |  |
| Des Koala | 223 |  |
| Exchange | 223 |  |
| Final Attack Orders | 223 |  |
| Gearfried the Iron Knight | 223 |  |
| Magical Merchant | 223 |  |
| Newdoria | 223 |  |
| Nimble Momonga | 223 |  |
| Nobleman of Extermination | 223 |  |
| Poison of the Old Man | 223 |  |
| Royal Oppression | 223 |  |
| Trap Hole | 223 |  |
| Flame Ghost | 217 |  |
| Marauding Captain | 217 | Semi-Limited |
| Axe of Despair | 211 |  |
| Buster Blader | 211 |  |
| Chain Energy | 211 |  |
| Cyber-Stein | 211 |  |
| Different Dimension Capsule | 211 |  |
| Exile of the Wicked | 211 |  |
| Fairy Box | 211 |  |
| Gaia Power | 211 |  |
| Greenkappa | 211 |  |
| Light of Intervention | 211 |  |
| Mask of Restrict | 211 |  |
| Mirror Wall | 211 |  |
| Mother Grizzly | 211 |  |
| Night Assailant | 211 |  |
| Princess of Tsurugi | 211 |  |
| Prohibition | 211 |  |
| Pyramid Turtle | 211 |  |
| Robbin' Goblin | 211 |  |
| Seven Tools of the Bandit | 211 |  |
| Soul Exchange | 211 |  |
| Stealth Bird | 211 |  |
| The Fiend Megacyber | 211 |  |
| Windstorm of Etaqua | 211 |  |
| Twin-Headed Thunder Dragon | 209 |  |
| Musician King | 207 |  |
| Punished Eagle | 207 |  |
| Empress Judge | 205 |  |
| Flame Swordsman | 204 |  |
| Karbonala Warrior | 204 |  |
| Charubin the Fire Knight | 202 |  |
| Dark Paladin | 201 |  |
| Cyber Saurus | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Labyrinth Tank | 200 |  |
| Rabid Horseman | 200 |  |

### Goat

| Card | DP | Limit |
|---|---:|---|
| Pot of Greed | 3000 | Limited |
| Graceful Charity | 2999 | Limited |
| Heavy Storm | 2966 | Limited |
| Bell of Destruction | 2934 | Limited |
| Snatch Steal | 2921 | Limited |
| Mirror Force | 2870 | Limited |
| Breaker the Magical Warrior | 2826 | Limited |
| Mystical Space Typhoon | 2825 | Limited |
| Delinquent Duo | 2749 | Limited |
| Black Luster Soldier - Envoy of Beginning | 2657 | Limited |
| Tribe-Infecting Virus | 2653 | Limited |
| Torrential Tribute | 2465 | Limited |
| Sangan | 2283 | Limited |
| Sinister Serpent | 1964 | Limited |
| Nobleman of Crossout | 1528 | Semi-Limited |
| Premature Burial | 1516 | Limited |
| D.D. Warrior Lady | 1426 | Limited |
| Call Of The Haunted | 1309 | Limited |
| Exiled Force | 1298 | Limited |
| Swords of Revealing Light | 1257 | Limited |
| Card Destruction | 1155 | Limited |
| Sakuretsu Armor | 891 |  |
| Trap Dustshoot | 877 |  |
| Magician of Faith | 870 |  |
| Book of Moon | 866 |  |
| Upstart Goblin | 837 | Semi-Limited |
| Mind Control | 821 |  |
| Gravekeeper's Spy | 769 |  |
| Chaos Sorcerer | 763 |  |
| Tsukuyomi | 762 |  |
| Kycoo the Ghost Destroyer | 754 |  |
| Dust Tornado | 728 |  |
| Solemn Judgment | 717 |  |
| Morphing Jar | 710 | Limited |
| Night Assailant | 693 | Semi-Limited |
| Dekoichi the Battlechanted Locomotive | 689 |  |
| Reinforcement of the Army | 674 | Semi-Limited |
| Thunder Dragon | 670 |  |
| Jinzo | 642 | Limited |
| Mobius the Frost Monarch | 638 |  |
| Mystik Wok | 637 |  |
| Zombyra the Dark | 599 |  |
| Jar of Greed | 597 |  |
| Raigeki Break | 561 |  |
| Asura Priest | 543 |  |
| Cyber Jar | 538 | Limited |
| Blade Knight | 527 |  |
| Injection Fairy Lily | 510 | Limited |
| Lightning Vortex | 503 | Limited |
| Scapegoat | 501 |  |
| Mystic Swordsman LV2 | 490 |  |
| Metamorphosis | 465 |  |
| Cipher Soldier | 464 |  |
| Ceasefire | 459 | Limited |
| Ninja Grandmaster Sasuke | 425 |  |
| Don Zaloog | 421 |  |
| Creature Swap | 405 | Semi-Limited |
| Royal Decree | 395 |  |
| Dark Magician of Chaos | 383 | Limited |
| Smashing Ground | 379 |  |
| King Tiger Wanghu | 376 |  |
| Spirit Reaper | 369 |  |
| Magic Cylinder | 361 | Limited |
| Phoenix Wing Wind Blast | 345 |  |
| Xing Zhen Hu | 342 |  |
| Berserk Gorilla | 335 |  |
| Giant Trunade | 329 |  |
| Brain Control | 320 |  |
| Widespread Ruin | 311 |  |
| United We Stand | 307 | Limited |
| Zaborg the Thunder Monarch | 299 |  |
| Level Limit - Area B | 296 | Semi-Limited |
| Dark Scorpion - Cliff the Trap Remover | 290 |  |
| Kuriboh | 290 |  |
| Thousand-Eyes Restrict | 286 |  |
| Dark Balter the Terrible | 284 |  |
| Threatening Roar | 281 |  |
| Gatling Dragon | 280 |  |
| Ryu Senshi | 280 |  |
| Mystic Tomato | 278 |  |
| Abyss Soldier | 277 | Semi-Limited |
| D.D. Assailant | 277 |  |
| Dark Blade the Dragon Knight | 274 |  |
| Gravity Bind | 274 | Semi-Limited |
| Reaper on the Nightmare | 273 |  |
| Des Wombat | 272 |  |
| Fiend Skull Dragon | 272 |  |
| Airknight Parshath | 271 |  |
| Compulsory Evacuation Device | 271 |  |
| Darkfire Dragon | 270 |  |
| Gyaku-Gire Panda | 269 |  |
| King Dragun | 266 |  |
| The Last Warrior from Another Planet | 266 |  |
| Bottomless Trap Hole | 265 |  |
| Cyber-Stein | 265 |  |
| Ojama King | 264 |  |
| Giant Rat | 261 |  |
| Secret Barrel | 260 |  |
| Skilled Dark Magician | 260 |  |
| Magical Merchant | 258 |  |
| Dragoness the Wicked Knight | 257 |  |
| Master of Oz | 256 |  |
| Ojama Trio | 255 |  |
| Just Desserts | 252 |  |
| Return from the Different Dimension | 252 |  |
| Spell Canceller | 245 |  |
| Seven Tools of the Bandit | 238 |  |
| Enemy Controller | 236 |  |
| Monster Gate | 231 |  |
| Reasoning | 231 |  |
| Magic Jammer | 230 |  |
| Wave-Motion Cannon | 230 |  |
| Dark Mimic LV1 | 228 |  |
| Dimension Fusion | 228 |  |
| Last Will | 228 |  |
| Sacred Crane | 228 |  |
| Des Koala | 227 |  |
| Goblin Attack Force | 227 |  |
| Mask of Darkness | 227 |  |
| Messenger of Peace | 227 |  |
| Hysteric Fairy | 226 |  |
| Flame Ghost | 225 |  |
| Big Shield Gardna | 224 |  |
| Gigantes | 224 |  |
| Monster Reincarnation | 224 |  |
| Hallowed Life Barrier | 223 |  |
| Newdoria | 223 |  |
| Waboku | 223 |  |
| Roulette Barrel | 222 |  |
| Token Thanksgiving | 220 |  |
| Apprentice Magician | 219 |  |
| Chiron the Mage | 219 |  |
| Mage Power | 219 | Limited |
| Soul Release | 219 |  |
| Nobleman of Extermination | 217 |  |
| Chain Energy | 216 |  |
| Twin-Headed Thunder Dragon | 216 |  |
| Bazoo the Soul-Eater | 215 |  |
| Toon Table of Contents | 215 |  |
| Poison of the Old Man | 214 |  |
| Milus Radiant | 213 |  |
| Royal Oppression | 213 |  |
| Time Seal | 213 |  |
| Cannon Soldier | 212 |  |
| Soul Exchange | 212 |  |
| Golem Sentry | 211 |  |
| Megamorph | 211 |  |
| Pyramid Turtle | 211 |  |
| Toon Cannon Soldier | 211 |  |
| Cold Wave | 210 |  |
| Chain Disappearance | 209 |  |
| Black Illusion Ritual | 208 |  |
| Book of Life | 208 |  |
| Dark Flare Knight | 208 |  |
| Giltia the D. Knight | 208 |  |
| Greenkappa | 208 |  |
| Jirai Gumo | 208 |  |
| Magic Drain | 208 |  |
| Relinquished | 208 |  |
| Ryu Kokki | 208 |  |
| Emergency Provisions | 206 | Semi-Limited |
| Blast with Chain | 205 |  |
| Divine Wrath | 205 |  |
| Gearfried the Iron Knight | 205 |  |
| Senju of the Thousand Hands | 205 |  |
| Skull Lair | 205 |  |
| Banisher of the Light | 204 |  |
| Fake Trap | 204 |  |
| Light of Intervention | 204 |  |
| Prohibition | 204 |  |
| Thestalos the Firestorm Monarch | 204 |  |
| Acid Trap Hole | 203 |  |
| B. Skull Dragon | 203 |  |
| Charubin the Fire Knight | 203 |  |
| Electric Snake | 203 |  |
| Exchange | 203 |  |
| Final Attack Orders | 203 |  |
| Karbonala Warrior | 203 |  |
| Limiter Removal | 203 |  |
| Nimble Momonga | 203 |  |
| Penguin Soldier | 203 |  |
| Shield & Sword | 203 |  |
| Stealth Bird | 203 |  |
| Thousand Dragon | 203 |  |
| Toon World | 203 |  |
| Trap Master | 203 |  |
| Tribute to The Doomed | 203 |  |
| Axe of Despair | 202 |  |
| Bickuribox | 202 |  |
| Black Pendant | 202 |  |
| Cyber Saurus | 202 |  |
| D.D. Survivor | 202 |  |
| Dark Paladin | 202 |  |
| Fusionist | 202 |  |
| Marauding Captain | 202 | Semi-Limited |
| Mask of Restrict | 202 |  |
| Mirror Wall | 202 |  |
| Morphing Jar #2 | 202 |  |
| Roaring Ocean Snake | 202 |  |
| Robbin' Goblin | 202 |  |
| Solemn Wishes | 202 |  |
| Spear Dragon | 202 |  |
| The Shallow Grave | 202 |  |
| Time Wizard | 202 |  |
| Trap Hole | 202 |  |
| Deepsea Shark | 201 |  |
| Different Dimension Capsule | 201 |  |
| Dream Clown | 201 |  |
| Exodia the Forbidden One | 201 | Limited |
| Flame Swordsman | 201 |  |
| Gaia the Dragon Champion | 201 |  |
| Giant Soldier of Stone | 201 |  |
| Gilasaurus | 201 |  |
| Guardian Sphinx | 201 |  |
| Labyrinth Tank | 201 |  |
| Left Arm of the Forbidden One | 201 | Limited |
| Left Leg of the Forbidden One | 201 | Limited |
| Metal Dragon | 201 |  |
| Mother Grizzly | 201 |  |
| Musician King | 201 |  |
| Pikeru's Circle of Enchantment | 201 |  |
| Right Arm of the Forbidden One | 201 | Limited |
| Right Leg of the Forbidden One | 201 | Limited |
| Rush Recklessly | 201 |  |
| Skull Knight | 201 |  |
| The Fiend Megacyber | 201 |  |
| The Little Swordsman of Aile | 201 |  |
| The Wicked Worm Beast | 201 |  |
| Warrior of Tradition | 201 |  |
| Windstorm of Etaqua | 201 |  |
| Backup Soldier | 200 |  |
| Catapult Turtle | 200 |  |
| Dark Magician | 200 |  |
| Dark Ruler Ha Des | 200 |  |
| Dragon Zombie | 200 |  |
| Empress Judge | 200 |  |
| Flower Wolf | 200 |  |
| Gaia Power | 200 |  |
| Giant Germ | 200 |  |
| Graverobber | 200 |  |
| Hiro's Shadow Scout | 200 |  |
| Kaminari Attack | 200 |  |
| Kuwagata a | 200 |  |
| Kwagar Hercules | 200 |  |
| Lightforce Sword | 200 |  |
| Minar | 200 |  |
| Rabid Horseman | 200 |  |
| Restructer Revolution | 200 |  |
| Sonic Bird | 200 |  |
| Thunder Nyan Nyan | 200 |  |
| Two-Pronged Attack | 200 |  |
| Ultimate Offering | 200 |  |

### Cyber

| Card | DP | Limit |
|---|---:|---|
| Graceful Charity | 3000 | Limited |
| Heavy Storm | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Mirror Force | 2938 | Limited |
| Bell of Destruction | 2875 | Limited |
| Snatch Steal | 2875 | Limited |
| Torrential Tribute | 2875 | Limited |
| Mystical Space Typhoon | 2751 | Limited |
| Delinquent Duo | 2689 | Limited |
| Black Luster Soldier - Envoy of Beginning | 2626 | Limited |
| Premature Burial | 2564 | Limited |
| Breaker the Magical Warrior | 2502 | Limited |
| Sangan | 2191 | Limited |
| Tribe-Infecting Virus | 2191 | Limited |
| D.D. Warrior Lady | 2066 | Limited |
| Call Of The Haunted | 2004 | Limited |
| Exiled Force | 1879 | Limited |
| Jinzo | 1568 | Limited |
| Nobleman of Crossout | 1506 | Semi-Limited |
| Sinister Serpent | 1506 | Limited |
| Lightning Vortex | 1256 | Limited |
| Swords of Revealing Light | 1070 | Limited |
| Cyber Dragon | 987 |  |
| Book of Moon | 945 |  |
| Magician of Faith | 924 |  |
| Morphing Jar | 883 | Limited |
| Mobius the Frost Monarch | 841 |  |
| Ceasefire | 821 | Limited |
| Trap Dustshoot | 821 |  |
| Cyber-Stein | 800 |  |
| Spirit Reaper | 800 |  |
| Sakuretsu Armor | 779 |  |
| Threatening Roar | 758 |  |
| Brain Control | 717 |  |
| Royal Decree | 675 |  |
| Metamorphosis | 655 |  |
| King Tiger Wanghu | 634 |  |
| Kuriboh | 634 |  |
| Dust Tornado | 592 |  |
| Last Will | 592 |  |
| Scapegoat | 592 |  |
| Cyber Jar | 572 | Limited |
| Exarion Universe | 572 |  |
| Tsukuyomi | 572 |  |
| Bottomless Trap Hole | 551 |  |
| Mystik Wok | 551 |  |
| Zaborg the Thunder Monarch | 551 |  |
| Mystic Tomato | 530 |  |
| Chaos Sorcerer | 509 |  |
| Injection Fairy Lily | 509 | Limited |
| Magic Cylinder | 509 | Limited |
| Asura Priest | 489 |  |
| Gravekeeper's Spy | 489 |  |
| Mind Control | 489 |  |
| Limiter Removal | 447 |  |
| Giant Trunade | 426 |  |
| Mystic Swordsman LV2 | 426 |  |
| Creature Swap | 416 | Semi-Limited |
| Cold Wave | 405 |  |
| Cipher Soldier | 385 |  |
| Reinforcement of the Army | 385 | Semi-Limited |
| Smashing Ground | 385 |  |
| Airknight Parshath | 364 |  |
| D.D. Assailant | 364 |  |
| Dekoichi the Battlechanted Locomotive | 364 |  |
| Kycoo the Ghost Destroyer | 364 |  |
| Solemn Judgment | 364 |  |
| Waboku | 364 |  |
| Gravity Bind | 354 | Semi-Limited |
| Dark Magician of Chaos | 322 | Limited |
| Don Zaloog | 322 |  |
| Enemy Controller | 322 |  |
| Level Limit - Area B | 322 | Semi-Limited |
| United We Stand | 322 | Limited |
| Big Shield Gardna | 302 |  |
| Chiron the Mage | 302 |  |
| Megamorph | 302 |  |
| Ninja Grandmaster Sasuke | 302 |  |
| Wave-Motion Cannon | 302 |  |
| Dark Balter the Terrible | 289 |  |
| Thousand-Eyes Restrict | 289 |  |
| Cyber Twin Dragon | 287 |  |
| Fiend Skull Dragon | 287 |  |
| Gatling Dragon | 287 |  |
| Reaper on the Nightmare | 283 |  |
| Blade Knight | 281 |  |
| Dark Blade the Dragon Knight | 281 |  |
| Des Koala | 281 |  |
| Just Desserts | 281 |  |
| Magical Merchant | 281 |  |
| Nimble Momonga | 281 |  |
| Ojama Trio | 281 |  |
| Secret Barrel | 281 |  |
| King Dragun | 279 |  |
| Ryu Senshi | 279 |  |
| The Last Warrior from Another Planet | 279 |  |
| Cyber End Dragon | 275 |  |
| Abyss Soldier | 260 | Semi-Limited |
| Card Destruction | 260 | Limited |
| Chain Disappearance | 260 |  |
| Giant Rat | 260 |  |
| Gyaku-Gire Panda | 260 |  |
| Spell Canceller | 260 |  |
| Token Thanksgiving | 260 |  |
| Widespread Ruin | 260 |  |
| Master of Oz | 256 |  |
| Ojama King | 254 |  |
| Darkfire Dragon | 252 |  |
| Des Wombat | 239 |  |
| Dragoness the Wicked Knight | 239 |  |
| Electric Snake | 239 |  |
| Jar of Greed | 239 |  |
| Monster Gate | 239 |  |
| Monster Reincarnation | 239 |  |
| Poison of the Old Man | 239 |  |
| Raigeki Break | 239 |  |
| Reasoning | 239 |  |
| Sacred Crane | 239 |  |
| Upstart Goblin | 229 | Semi-Limited |
| Apprentice Magician | 219 |  |
| Cannon Soldier | 219 |  |
| Dark Flare Knight | 219 |  |
| Dark Scorpion - Cliff the Trap Remover | 219 |  |
| Different Dimension Capsule | 219 |  |
| Greenkappa | 219 |  |
| Return from the Different Dimension | 219 |  |
| Sasuke Samurai | 219 |  |
| Soul Exchange | 219 |  |
| Stealth Bird | 219 |  |
| Thestalos the Firestorm Monarch | 219 |  |
| Thunder Dragon | 219 |  |
| Xing Zhen Hu | 219 |  |
| Gaia the Dragon Champion | 212 |  |
| Flame Ghost | 202 |  |
| Fusionist | 202 |  |
| Giltia the D. Knight | 202 |  |
| Karbonala Warrior | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |

### Reaper

| Card | DP | Limit |
|---|---:|---|
| Dark Hole | 3000 | Limited |
| Mystical Space Typhoon | 3000 | Limited |
| Heavy Storm | 2986 | Limited |
| Snatch Steal | 2932 | Limited |
| Torrential Tribute | 2905 | Limited |
| Breaker the Magical Warrior | 2850 | Limited |
| Sangan | 2837 | Limited |
| Nobleman of Crossout | 2674 | Limited |
| Premature Burial | 2579 | Limited |
| Confiscation | 2388 | Limited |
| D.D. Warrior Lady | 2388 | Limited |
| Exiled Force | 2239 | Limited |
| Book of Moon | 2008 | Limited |
| Call Of The Haunted | 1858 | Limited |
| Scapegoat | 1274 | Limited |
| Magician of Faith | 1206 | Limited |
| Jinzo | 1192 | Limited |
| Spirit Reaper | 1029 |  |
| Tsukuyomi | 1029 | Limited |
| Cyber Dragon | 1002 |  |
| Reinforcement of the Army | 995 | Semi-Limited |
| Swords of Revealing Light | 934 | Limited |
| Dust Tornado | 916 |  |
| Mobius the Frost Monarch | 888 |  |
| Sakuretsu Armor | 866 |  |
| Smashing Ground | 752 |  |
| Morphing Jar | 730 | Limited |
| Cyber Jar | 716 | Limited |
| Metamorphosis | 716 | Limited |
| Kycoo the Ghost Destroyer | 712 |  |
| Mystic Swordsman LV2 | 693 |  |
| Ceasefire | 675 | Limited |
| Threatening Roar | 630 |  |
| Creature Swap | 628 | Semi-Limited |
| Mystic Tomato | 616 |  |
| Enemy Controller | 594 |  |
| Bottomless Trap Hole | 562 |  |
| D.D. Assailant | 544 |  |
| Don Zaloog | 544 |  |
| Brain Control | 539 |  |
| Zaborg the Thunder Monarch | 539 |  |
| Trap Dustshoot | 535 |  |
| Widespread Ruin | 521 |  |
| Solemn Judgment | 512 |  |
| Royal Decree | 508 |  |
| Chaos Sorcerer | 494 |  |
| Cyber-Stein | 485 |  |
| Lightning Vortex | 485 | Limited |
| Treeborn Frog | 471 |  |
| Dekoichi the Battlechanted Locomotive | 467 |  |
| Giant Trunade | 462 |  |
| Last Will | 444 |  |
| Injection Fairy Lily | 431 | Limited |
| Magic Cylinder | 431 | Limited |
| Cipher Soldier | 426 |  |
| King Tiger Wanghu | 426 |  |
| Thousand-Eyes Restrict | 405 | Limited |
| Thestalos the Firestorm Monarch | 403 |  |
| Return from the Different Dimension | 390 |  |
| D.D. Survivor | 385 |  |
| Dark Ruler Ha Des | 376 |  |
| Pot of Avarice | 367 |  |
| Magical Merchant | 363 |  |
| Raigeki Break | 363 |  |
| Gravekeeper's Spy | 349 |  |
| Waboku | 349 |  |
| Level Limit - Area B | 342 | Semi-Limited |
| Wave-Motion Cannon | 340 |  |
| Card Destruction | 335 | Limited |
| Kuriboh | 335 |  |
| Mask of Restrict | 335 |  |
| Soul Exchange | 322 |  |
| Bazoo the Soul-Eater | 317 |  |
| Limiter Removal | 308 | Limited |
| Night Assailant | 308 | Limited |
| Ninja Grandmaster Sasuke | 308 |  |
| United We Stand | 308 | Limited |
| Gravity Bind | 302 | Semi-Limited |
| Newdoria | 295 |  |
| Des Wombat | 290 |  |
| Drillroid | 281 |  |
| Stealth Bird | 281 |  |
| Giant Rat | 277 |  |
| Trap Hole | 277 |  |
| Dark Scorpion - Cliff the Trap Remover | 272 |  |
| Chain Disappearance | 268 |  |
| Cyber End Dragon | 268 |  |
| Cyber Twin Dragon | 268 |  |
| Hydrogeddon | 268 |  |
| Mother Grizzly | 263 |  |
| Phoenix Wing Wind Blast | 263 |  |
| Dark Balter the Terrible | 262 |  |
| Gatling Dragon | 260 |  |
| Ryu Senshi | 260 |  |
| Blade Knight | 258 |  |
| Messenger of Peace | 258 |  |
| The Last Warrior from Another Planet | 255 |  |
| Zombyra the Dark | 254 |  |
| Fiend Skull Dragon | 253 |  |
| King Dragun | 250 |  |
| Reaper on the Nightmare | 250 |  |
| Cold Wave | 249 |  |
| Des Koala | 249 |  |
| Spell Canceller | 249 |  |
| Airknight Parshath | 245 |  |
| Blue-Eyes Ultimate Dragon | 245 |  |
| Chiron the Mage | 245 |  |
| Dark Blade the Dragon Knight | 245 |  |
| Dark Mimic LV1 | 245 |  |
| Ojama King | 244 |  |
| Compulsory Evacuation Device | 240 |  |
| Gyaku-Gire Panda | 240 |  |
| Mystik Wok | 240 |  |
| Gigantes | 236 |  |
| Jar of Greed | 236 |  |
| Pyramid Turtle | 236 |  |
| Ryu Kokki | 236 |  |
| Skilled Dark Magician | 236 |  |
| Upstart Goblin | 234 | Semi-Limited |
| Berserk Gorilla | 231 |  |
| Book of Life | 231 |  |
| Exarion Universe | 231 |  |
| Hallowed Life Barrier | 231 |  |
| Just Desserts | 231 |  |
| Megamorph | 231 |  |
| Milus Radiant | 231 |  |
| Nobleman of Extermination | 231 |  |
| Ojama Trio | 231 |  |
| Apprentice Magician | 227 |  |
| Dark Magician of Chaos | 227 | Limited |
| Goblin Attack Force | 227 |  |
| Prohibition | 227 |  |
| Thunder Dragon | 227 |  |
| Master of Oz | 225 |  |
| Darkfire Dragon | 223 |  |
| Cannon Soldier | 222 |  |
| Cyber Blader | 222 |  |
| Mask of Darkness | 222 |  |
| Sand Moth | 222 |  |
| Sasuke Samurai | 222 |  |
| Secret Barrel | 222 |  |
| Seven Tools of the Bandit | 222 |  |
| Dragoness the Wicked Knight | 221 |  |
| Dark Flare Knight | 218 |  |
| Flying Kamakiri #1 | 218 |  |
| Goblin Elite Attack Force | 218 |  |
| Big Shield Gardna | 213 |  |
| Mage Power | 213 | Limited |
| Magic Jammer | 213 |  |
| Mind Control | 213 |  |
| Nimble Momonga | 213 |  |
| Poison of the Old Man | 213 |  |
| Skelengel | 213 |  |
| The Wicked Worm Beast | 213 |  |
| Tribute to The Doomed | 213 |  |
| Black Illusion Ritual | 209 |  |
| Blast with Chain | 209 |  |
| Chain Energy | 209 |  |
| Dimension Fusion | 209 |  |
| Divine Wrath | 209 |  |
| Electric Snake | 209 |  |
| Fissure | 209 |  |
| Princess of Tsurugi | 209 |  |
| Relinquished | 209 |  |
| Robbin' Goblin | 209 |  |
| Rush Recklessly | 209 |  |
| Time Seal | 209 |  |
| Warrior Elimination | 209 |  |
| Twin-Headed Thunder Dragon | 208 |  |
| Abyss Soldier | 206 | Semi-Limited |
| Emergency Provisions | 206 | Semi-Limited |
| Fusionist | 205 |  |
| Acid Trap Hole | 204 |  |
| Asura Priest | 204 |  |
| De-Spell | 204 |  |
| Flame Ghost | 204 |  |
| Giant Germ | 204 |  |
| Light of Intervention | 204 |  |
| Magic Drain | 204 |  |
| Marauding Captain | 204 |  |
| Masked Dragon | 204 |  |
| Mirror Wall | 204 |  |
| Reasoning | 204 |  |
| Remove Trap | 204 |  |
| Rising Air Current | 204 |  |
| Roulette Barrel | 204 |  |
| Royal Oppression | 204 |  |
| Sacred Crane | 204 |  |
| Skull Lair | 204 |  |
| Time Wizard | 204 |  |
| UFO Turtle | 204 |  |
| Warrior of Tradition | 204 |  |
| Empress Judge | 202 |  |
| Giltia the D. Knight | 202 |  |
| Dark Paladin | 201 |  |
| Flame Swordsman | 201 |  |
| Gaia the Dragon Champion | 201 |  |
| Kaminari Attack | 201 |  |
| Karbonala Warrior | 201 |  |
| B. Skull Dragon | 200 |  |
| Bickuribox | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Cyber Saurus | 200 |  |
| Deepsea Shark | 200 |  |
| Labyrinth Tank | 200 |  |
| Musician King | 200 |  |
| Roaring Ocean Snake | 200 |  |

<!-- /pack-prices -->
