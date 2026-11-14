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

50 format packs (TCG sets interleaved with usage packs per Campaign format).

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

- **Type:** Usage · **Format:** Yugi-Kaiba · **Cards in pool:** 74 · **Weight sum:** 120585

| Card | Weight | Pull % |
|---|---:|---:|
| Destroyer Golem | 4393 | 3.6431 |
| Dragon Zombie | 4393 | 3.6431 |
| Koumori Dragon | 4393 | 3.6431 |
| Petit Dragon | 4393 | 3.6431 |
| Sparks | 4393 | 3.6431 |
| The Stern Mystic | 4393 | 3.6431 |
| Wasteland | 4393 | 3.6431 |
| Dian Keto the Cure Master | 3358 | 2.7848 |
| Forest | 3358 | 2.7848 |
| Invigoration | 3358 | 2.7848 |
| Ryu-Kishin Powered | 3358 | 2.7848 |
| Sword of Dark Destruction | 3358 | 2.7848 |
| Two-Pronged Attack | 3358 | 2.7848 |
| Exodia the Forbidden One | 2873 | 2.3826 |
| Left Arm of the Forbidden One | 2873 | 2.3826 |
| Left Leg of the Forbidden One | 2873 | 2.3826 |
| Right Arm of the Forbidden One | 2873 | 2.3826 |
| Right Leg of the Forbidden One | 2873 | 2.3826 |
| Castle Walls | 2574 | 2.1346 |
| Curse of Dragon | 2574 | 2.1346 |
| Lord of D. | 2574 | 2.1346 |
| The Flute of Summoning Dragon | 2574 | 2.1346 |
| Yami | 2207 | 1.8302 |
| Final Flame | 2082 | 1.7266 |
| Reverse Trap | 2082 | 1.7266 |
| Armed Ninja | 1894 | 1.5707 |
| Umi  | 1821 | 1.5101 |
| Reaper of the Cards | 1701 | 1.4106 |
| Last Will | 1566 | 1.2987 |
| Great White | 1530 | 1.2688 |
| Spirit of the Harp | 1384 | 1.1477 |
| Gyakutenno Megami | 1361 | 1.1287 |
| Hane-Hane | 1361 | 1.1287 |
| Remove Trap | 1339 | 1.1104 |
| Mystical Elf | 1163 | 0.9645 |
| Stop Defense | 1151 | 0.9545 |
| Ookazi | 1052 | 0.8724 |
| The Wicked Worm Beast | 1028 | 0.8525 |
| Just Desserts | 1014 | 0.8409 |
| Aqua Madoor | 872 | 0.7231 |
| Judge Man | 872 | 0.7231 |
| Trap Master | 843 | 0.6991 |
| De-Spell | 826 | 0.6850 |
| Wall of Illusion | 777 | 0.6444 |
| Soul Exchange | 767 | 0.6361 |
| Summoned Skull | 758 | 0.6286 |
| Battle Ox | 754 | 0.6253 |
| Waboku | 750 | 0.6220 |
| Giant Soldier of Stone | 747 | 0.6195 |
| Neo the Magic Swordsman | 747 | 0.6195 |
| Reinforcements | 747 | 0.6195 |
| La Jinn the Mystical Genie of the Lamp | 745 | 0.6178 |
| Fissure | 743 | 0.6162 |
| Man-Eater Bug | 743 | 0.6162 |
| Trap Hole | 743 | 0.6162 |
| Card Destruction | 718 | 0.5954 |
| Swords of Revealing Light | 655 | 0.5432 |
| Change of Heart | 522 | 0.4329 |
| Dark Hole | 522 | 0.4329 |
| Monster Reborn | 522 | 0.4329 |
| Pot of Greed | 522 | 0.4329 |
| Raigeki | 522 | 0.4329 |
| Charubin the Fire Knight | 396 | 0.3284 |
| Flame Ghost | 277 | 0.2297 |
| Karbonala Warrior | 270 | 0.2239 |
| Darkfire Dragon | 257 | 0.2131 |
| Dragoness the Wicked Knight | 252 | 0.2090 |
| Flame Swordsman | 252 | 0.2090 |
| Fusionist | 240 | 0.1990 |
| Gaia the Dragon Champion | 240 | 0.1990 |
| Flower Wolf | 229 | 0.1899 |
| Metal Dragon | 219 | 0.1816 |
| Dragon Capture Jar | 160 | 0.1327 |
| Ultimate Offering | 80 | 0.0663 |

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

- **Type:** Usage · **Format:** Critter · **Cards in pool:** 88 · **Weight sum:** 99369

| Card | Weight | Pull % |
|---|---:|---:|
| Catapult Turtle | 3491 | 3.5132 |
| Dream Clown | 3491 | 3.5132 |
| Ookazi | 3491 | 3.5132 |
| Remove Trap | 3491 | 3.5132 |
| Share the Pain | 3491 | 3.5132 |
| Soul Exchange | 3491 | 3.5132 |
| The Wicked Worm Beast | 3491 | 3.5132 |
| Shield & Sword | 2675 | 2.6920 |
| Stim-Pack | 2675 | 2.6920 |
| Stop Defense | 2675 | 2.6920 |
| Summoned Skull | 2675 | 2.6920 |
| Giga-tech Wolf | 2292 | 2.3066 |
| Jinzo #7 | 2056 | 2.0691 |
| Tribute to The Doomed | 2056 | 2.0691 |
| Crass Clown | 1891 | 1.9030 |
| Polymerization | 1891 | 1.9030 |
| Polymerization | 1891 | 1.9030 |
| Aqua Madoor | 1766 | 1.7772 |
| Dark Elf | 1766 | 1.7772 |
| Tremendous Fire | 1766 | 1.7772 |
| De-Spell | 1668 | 1.6786 |
| Princess of Tsurugi | 1668 | 1.6786 |
| Reinforcements | 1668 | 1.6786 |
| Mask of Darkness | 1587 | 1.5971 |
| Trap Master | 1587 | 1.5971 |
| Kuriboh | 1520 | 1.5297 |
| Horn of Heaven | 1412 | 1.4210 |
| Magic Jammer | 1261 | 1.2690 |
| Fake Trap | 1158 | 1.1654 |
| Exodia the Forbidden One | 1022 | 1.0285 |
| Left Arm of the Forbidden One | 1022 | 1.0285 |
| Left Leg of the Forbidden One | 1022 | 1.0285 |
| Right Arm of the Forbidden One | 1022 | 1.0285 |
| Right Leg of the Forbidden One | 1022 | 1.0285 |
| Jirai Gumo | 985 | 0.9913 |
| 7 Colored Fish | 974 | 0.9802 |
| Wall of Illusion | 974 | 0.9802 |
| White Magical Hat | 952 | 0.9580 |
| Seven Tools of the Bandit | 855 | 0.8604 |
| Thunder Dragon | 814 | 0.8192 |
| Man-Eater Bug | 809 | 0.8141 |
| Last Will | 784 | 0.7890 |
| Masked Sorcerer | 767 | 0.7719 |
| Muka Muka | 767 | 0.7719 |
| Solemn Judgment | 767 | 0.7719 |
| Giant Soldier of Stone | 763 | 0.7678 |
| La Jinn the Mystical Genie of the Lamp | 755 | 0.7598 |
| Magician of Faith | 751 | 0.7558 |
| Waboku | 751 | 0.7558 |
| Cannon Soldier | 747 | 0.7517 |
| Fissure | 743 | 0.7477 |
| Trap Hole | 743 | 0.7477 |
| Card Destruction | 729 | 0.7336 |
| Swords of Revealing Light | 663 | 0.6672 |
| Heavy Storm | 650 | 0.6541 |
| Sangan | 650 | 0.6541 |
| Witch of the Black Forest | 650 | 0.6541 |
| Change of Heart | 522 | 0.5253 |
| Dark Hole | 522 | 0.5253 |
| Mirror Force | 522 | 0.5253 |
| Monster Reborn | 522 | 0.5253 |
| Pot of Greed | 522 | 0.5253 |
| Raigeki | 522 | 0.5253 |
| Flame Swordsman | 456 | 0.4589 |
| Skull Knight | 400 | 0.4025 |
| Flower Wolf | 380 | 0.3824 |
| Metal Dragon | 363 | 0.3653 |
| Giltia the D. Knight | 336 | 0.3381 |
| Flame Ghost | 325 | 0.3271 |
| Karbonala Warrior | 325 | 0.3271 |
| Roaring Ocean Snake | 315 | 0.3170 |
| Empress Judge | 306 | 0.3079 |
| Deepsea Shark | 291 | 0.2928 |
| Fusionist | 291 | 0.2928 |
| Punished Eagle | 291 | 0.2928 |
| Bickuribox | 278 | 0.2798 |
| Cyber Saurus | 278 | 0.2798 |
| Musician King | 272 | 0.2737 |
| Darkfire Dragon | 267 | 0.2687 |
| Dragoness the Wicked Knight | 267 | 0.2687 |
| Charubin the Fire Knight | 262 | 0.2637 |
| Labyrinth Tank | 245 | 0.2466 |
| Thousand Dragon | 245 | 0.2466 |
| Gaia the Dragon Champion | 223 | 0.2244 |
| B. Skull Dragon | 209 | 0.2103 |
| Twin-Headed Thunder Dragon | 200 | 0.2013 |
| Ultimate Offering | 141 | 0.1419 |
| Robbin' Goblin | 77 | 0.0775 |

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

- **Type:** Usage · **Format:** Treasure · **Cards in pool:** 84 · **Weight sum:** 78341

| Card | Weight | Pull % |
|---|---:|---:|
| Catapult Turtle | 2305 | 2.9423 |
| Crass Clown | 2305 | 2.9423 |
| Flash Assailant | 2305 | 2.9423 |
| Gaia Power | 2305 | 2.9423 |
| Hercules Beetle | 2305 | 2.9423 |
| Horn of Heaven | 2305 | 2.9423 |
| Kuriboh | 2305 | 2.9423 |
| Mother Grizzly | 2305 | 2.9423 |
| Share the Pain | 2305 | 2.9423 |
| Slot Machine | 2305 | 2.9423 |
| White Magical Hat | 2305 | 2.9423 |
| Giant Rat | 1776 | 2.2670 |
| La Jinn the Mystical Genie of the Lamp | 1776 | 2.2670 |
| Wall of Illusion | 1776 | 2.2670 |
| Giant Germ | 1528 | 1.9504 |
| Just Desserts | 1528 | 1.9504 |
| Minar | 1528 | 1.9504 |
| Princess of Tsurugi | 1528 | 1.9504 |
| Soul Exchange | 1528 | 1.9504 |
| Spear Cretin | 1528 | 1.9504 |
| Summoned Skull | 1528 | 1.9504 |
| Magic Jammer | 1375 | 1.7551 |
| Thunder Dragon | 1268 | 1.6186 |
| Giant Soldier of Stone | 1187 | 1.5152 |
| Swords of Revealing Light | 1187 | 1.5152 |
| Masked Sorcerer | 1123 | 1.4335 |
| Card Destruction | 1096 | 1.3990 |
| Nimble Momonga | 990 | 1.2637 |
| Jirai Gumo | 957 | 1.2216 |
| Last Will | 929 | 1.1858 |
| Upstart Goblin | 929 | 1.1858 |
| Solemn Judgment | 880 | 1.1233 |
| Tribute to The Doomed | 860 | 1.0978 |
| Banisher of the Light | 841 | 1.0735 |
| Giant Trunade | 824 | 1.0518 |
| Seven Tools of the Bandit | 824 | 1.0518 |
| Cyber Jar | 808 | 1.0314 |
| Magician of Faith | 793 | 1.0122 |
| Fissure | 779 | 0.9944 |
| Man-Eater Bug | 779 | 0.9944 |
| Mechanicalchaser | 779 | 0.9944 |
| Trap Hole | 779 | 0.9944 |
| Waboku | 779 | 0.9944 |
| Cannon Soldier | 743 | 0.9484 |
| Mystic Tomato | 743 | 0.9484 |
| Heavy Storm | 681 | 0.8693 |
| Sangan | 650 | 0.8297 |
| Witch of the Black Forest | 650 | 0.8297 |
| Painful Choice | 597 | 0.7621 |
| Skull Knight | 560 | 0.7148 |
| Snatch Steal | 545 | 0.6957 |
| Monster Reborn | 529 | 0.6753 |
| Change of Heart | 522 | 0.6663 |
| Confiscation | 522 | 0.6663 |
| Dark Hole | 522 | 0.6663 |
| Delinquent Duo | 522 | 0.6663 |
| Mirror Force | 522 | 0.6663 |
| Pot of Greed | 522 | 0.6663 |
| Raigeki | 522 | 0.6663 |
| The Forceful Sentry | 522 | 0.6663 |
| Cyber Saurus | 365 | 0.4659 |
| Deepsea Shark | 365 | 0.4659 |
| Empress Judge | 365 | 0.4659 |
| Flower Wolf | 365 | 0.4659 |
| Karbonala Warrior | 365 | 0.4659 |
| Metal Dragon | 365 | 0.4659 |
| Musician King | 365 | 0.4659 |
| Fusionist | 327 | 0.4174 |
| Labyrinth Tank | 327 | 0.4174 |
| Thousand Dragon | 327 | 0.4174 |
| Twin-Headed Thunder Dragon | 327 | 0.4174 |
| Bickuribox | 280 | 0.3574 |
| Gaia the Dragon Champion | 280 | 0.3574 |
| B. Skull Dragon | 264 | 0.3370 |
| Charubin the Fire Knight | 208 | 0.2655 |
| Flame Ghost | 208 | 0.2655 |
| Flame Swordsman | 208 | 0.2655 |
| Darkfire Dragon | 203 | 0.2591 |
| Dragoness the Wicked Knight | 203 | 0.2591 |
| Chain Energy | 152 | 0.1940 |
| Toll | 152 | 0.1940 |
| Robbin' Goblin | 112 | 0.1430 |
| Rush Recklessly | 112 | 0.1430 |
| Mystical Space Typhoon | 77 | 0.0983 |

### Pharaoh's Servant

- **Type:** TCG · **Format:** Imperial · **Cards in pool:** 88 · **Weight sum:** 563350

| Card | Weight | Pull % |
|---|---:|---:|
| 4-Starred Ladybug of Doom | 10000 | 1.7751 |
| 7 Completed | 10000 | 1.7751 |
| Armored Glass | 10000 | 1.7751 |
| Attack and Receive | 10000 | 1.7751 |
| Bite Shoes | 10000 | 1.7751 |
| Bombardment Beetle | 10000 | 1.7751 |
| Bubonic Vermin | 10000 | 1.7751 |
| Burning Land | 10000 | 1.7751 |
| Cold Wave | 10000 | 1.7751 |
| Cyber Falcon | 10000 | 1.7751 |
| DNA Surgery | 10000 | 1.7751 |
| Darkfire Soldier #1 | 10000 | 1.7751 |
| Darkfire Soldier #2 | 10000 | 1.7751 |
| Deepsea Warrior | 10000 | 1.7751 |
| Drill Bug | 10000 | 1.7751 |
| Driving Snow | 10000 | 1.7751 |
| Earthshaker | 10000 | 1.7751 |
| Enchanted Javelin | 10000 | 1.7751 |
| Flame Champion | 10000 | 1.7751 |
| Flying Kamakiri #2 | 10000 | 1.7751 |
| Gamble | 10000 | 1.7751 |
| Gift of The Mystical Elf | 10000 | 1.7751 |
| Gradius | 10000 | 1.7751 |
| Ground Collapse | 10000 | 1.7751 |
| Gust | 10000 | 1.7751 |
| Infinite Dismissal | 10000 | 1.7751 |
| Insect Barrier | 10000 | 1.7751 |
| Inspection | 10000 | 1.7751 |
| Invitation to a Dark Sleep | 10000 | 1.7751 |
| Kiseitai | 10000 | 1.7751 |
| Light of Intervention | 10000 | 1.7751 |
| Major Riot | 10000 | 1.7751 |
| Metal Detector | 10000 | 1.7751 |
| Minor Goblin Official | 10000 | 1.7751 |
| Mr. Volcano | 10000 | 1.7751 |
| Mystic Probe | 10000 | 1.7751 |
| Rain of Mercy | 10000 | 1.7751 |
| Respect Play | 10000 | 1.7751 |
| Shadow of Eyes | 10000 | 1.7751 |
| Solemn Wishes | 10000 | 1.7751 |
| Solomon's Lawbook | 10000 | 1.7751 |
| Steel Ogre Grotto #2 | 10000 | 1.7751 |
| Sword Hunter | 10000 | 1.7751 |
| The Eye of Truth | 10000 | 1.7751 |
| The Regulation of Tribe | 10000 | 1.7751 |
| Thousand-Eyes Idol | 10000 | 1.7751 |
| Three-Headed Geedo | 10000 | 1.7751 |
| Time Seal | 10000 | 1.7751 |
| Twin-Headed Fire Dragon | 10000 | 1.7751 |
| Type Zero Magic Crusher | 10000 | 1.7751 |
| World Suppression | 10000 | 1.7751 |
| Numinous Healer | 7000 | 1.2426 |
| Appropriate | 2500 | 0.4438 |
| Forced Requisition | 2500 | 0.4438 |
| Gravity Bind | 2500 | 0.4438 |
| Hayabusa Knight | 2500 | 0.4438 |
| Lightforce Sword | 2500 | 0.4438 |
| Mad Sword Beast | 2500 | 0.4438 |
| Magic Drain | 2500 | 0.4438 |
| Michizure | 2500 | 0.4438 |
| Monster Recovery | 2500 | 0.4438 |
| Morphing Jar #2 | 2500 | 0.4438 |
| Nobleman of Extermination | 2500 | 0.4438 |
| Prohibition | 2500 | 0.4438 |
| Shift | 2500 | 0.4438 |
| Skull Invitation | 2500 | 0.4438 |
| The Shallow Grave | 2500 | 0.4438 |
| Vampire Baby | 2500 | 0.4438 |
| Backup Soldier | 600 | 0.1065 |
| Dust Tornado | 600 | 0.1065 |
| Gearfried the Iron Knight | 600 | 0.1065 |
| Graverobber | 600 | 0.1065 |
| Limiter Removal | 600 | 0.1065 |
| Magical Hats | 600 | 0.1065 |
| Mirror Wall | 600 | 0.1065 |
| Nobleman of Crossout | 600 | 0.1065 |
| Parasite Paracide | 600 | 0.1065 |
| Buster Blader | 100 | 0.0178 |
| Call Of The Haunted | 100 | 0.0178 |
| Ceasefire | 100 | 0.0178 |
| Chain Destruction | 100 | 0.0178 |
| Goblin Attack Force | 100 | 0.0178 |
| Premature Burial | 100 | 0.0178 |
| The Fiend Megacyber | 100 | 0.0178 |
| The Legendary Fisherman | 100 | 0.0178 |
| Thousand-Eyes Restrict | 100 | 0.0178 |
| Imperial Order | 25 | 0.0044 |
| Jinzo | 25 | 0.0044 |

### Imperial

- **Type:** Usage · **Format:** Imperial · **Cards in pool:** 106 · **Weight sum:** 109562

| Card | Weight | Pull % |
|---|---:|---:|
| Aqua Madoor | 2900 | 2.6469 |
| Exchange | 2900 | 2.6469 |
| Kuriboh | 2900 | 2.6469 |
| Mask of Darkness | 2900 | 2.6469 |
| Megamorph | 2900 | 2.6469 |
| Nimble Momonga | 2900 | 2.6469 |
| Reinforcements | 2900 | 2.6469 |
| Seven Tools of the Bandit | 2900 | 2.6469 |
| Soul Exchange | 2900 | 2.6469 |
| Thunder Dragon | 2900 | 2.6469 |
| Backup Soldier | 2484 | 2.2672 |
| Acid Trap Hole | 2227 | 2.0326 |
| Magic Drain | 2227 | 2.0326 |
| Tribute to The Doomed | 2227 | 2.0326 |
| White Magical Hat | 2227 | 2.0326 |
| Exodia the Forbidden One | 1911 | 1.7442 |
| Giant Germ | 1911 | 1.7442 |
| Graverobber | 1911 | 1.7442 |
| Last Will | 1911 | 1.7442 |
| Left Arm of the Forbidden One | 1911 | 1.7442 |
| Left Leg of the Forbidden One | 1911 | 1.7442 |
| Magic Jammer | 1911 | 1.7442 |
| Morphing Jar #2 | 1911 | 1.7442 |
| Right Arm of the Forbidden One | 1911 | 1.7442 |
| Right Leg of the Forbidden One | 1911 | 1.7442 |
| Black Illusion Ritual | 1717 | 1.5671 |
| Masked Sorcerer | 1717 | 1.5671 |
| Relinquished | 1717 | 1.5671 |
| Senju of the Thousand Hands | 1717 | 1.5671 |
| Sonic Bird | 1717 | 1.5671 |
| Upstart Goblin | 1478 | 1.3490 |
| 4-Starred Ladybug of Doom | 1396 | 1.2742 |
| Waboku | 1330 | 1.2139 |
| Giant Soldier of Stone | 1274 | 1.1628 |
| Card Destruction | 1149 | 1.0487 |
| Time Seal | 1037 | 0.9465 |
| Goblin Attack Force | 1015 | 0.9264 |
| Giant Trunade | 995 | 0.9082 |
| The Fiend Megacyber | 976 | 0.8908 |
| Banisher of the Light | 887 | 0.8096 |
| Solemn Judgment | 833 | 0.7603 |
| Cannon Soldier | 791 | 0.7220 |
| Fissure | 769 | 0.7019 |
| Trap Hole | 769 | 0.7019 |
| Dust Tornado | 749 | 0.6836 |
| Magician of Faith | 749 | 0.6836 |
| Man-Eater Bug | 749 | 0.6836 |
| Mechanicalchaser | 749 | 0.6836 |
| Mystic Tomato | 743 | 0.6782 |
| Painful Choice | 726 | 0.6626 |
| Empress Judge | 710 | 0.6480 |
| Fusionist | 710 | 0.6480 |
| Giltia the D. Knight | 710 | 0.6480 |
| Rabid Horseman | 710 | 0.6480 |
| Heavy Storm | 666 | 0.6079 |
| Nobleman of Crossout | 650 | 0.5933 |
| Sangan | 650 | 0.5933 |
| Swords of Revealing Light | 640 | 0.5841 |
| Cyber Jar | 599 | 0.5467 |
| Ceasefire | 574 | 0.5239 |
| Morphing Jar | 568 | 0.5184 |
| Premature Burial | 562 | 0.5130 |
| Change of Heart | 522 | 0.4764 |
| Confiscation | 522 | 0.4764 |
| Dark Hole | 522 | 0.4764 |
| Delinquent Duo | 522 | 0.4764 |
| Jinzo | 522 | 0.4764 |
| Mirror Force | 522 | 0.4764 |
| Monster Reborn | 522 | 0.4764 |
| Pot of Greed | 522 | 0.4764 |
| Raigeki | 522 | 0.4764 |
| Snatch Steal | 522 | 0.4764 |
| The Forceful Sentry | 522 | 0.4764 |
| Witch of the Black Forest | 522 | 0.4764 |
| Gaia the Dragon Champion | 462 | 0.4217 |
| Bickuribox | 413 | 0.3770 |
| Skull Knight | 413 | 0.3770 |
| Charubin the Fire Knight | 378 | 0.3450 |
| Flame Ghost | 378 | 0.3450 |
| Cyber Saurus | 353 | 0.3222 |
| Flame Swordsman | 332 | 0.3030 |
| Gravity Bind | 290 | 0.2647 |
| Light of Intervention | 290 | 0.2647 |
| Prohibition | 290 | 0.2647 |
| Robbin' Goblin | 290 | 0.2647 |
| Type Zero Magic Crusher | 290 | 0.2647 |
| Kwagar Hercules | 279 | 0.2547 |
| Metal Dragon | 279 | 0.2547 |
| Deepsea Shark | 270 | 0.2464 |
| Punished Eagle | 270 | 0.2464 |
| Roaring Ocean Snake | 270 | 0.2464 |
| Dragoness the Wicked Knight | 262 | 0.2391 |
| Thousand Dragon | 262 | 0.2391 |
| Musician King | 255 | 0.2327 |
| Warrior of Tradition | 248 | 0.2264 |
| Twin-Headed Thunder Dragon | 236 | 0.2154 |
| Darkfire Dragon | 227 | 0.2072 |
| Labyrinth Tank | 227 | 0.2072 |
| B. Skull Dragon | 222 | 0.2026 |
| Thousand-Eyes Restrict | 222 | 0.2026 |
| Rush Recklessly | 171 | 0.1561 |
| Limiter Removal | 127 | 0.1159 |
| Solemn Wishes | 114 | 0.1041 |
| Mystical Space Typhoon | 74 | 0.0675 |
| Call Of The Haunted | 52 | 0.0475 |
| Imperial Order | 52 | 0.0475 |

### Labyrinth of Nightmare

- **Type:** TCG · **Format:** Android · **Cards in pool:** 86 · **Weight sum:** 542050

| Card | Weight | Pull % |
|---|---:|---:|
| Amazoness Archer | 10000 | 1.8448 |
| Aqua Spirit | 10000 | 1.8448 |
| Bait Doll | 10000 | 1.8448 |
| Blind Destruction | 10000 | 1.8448 |
| Chosen One | 10000 | 1.8448 |
| Collected Power | 10000 | 1.8448 |
| Crimson Sentry | 10000 | 1.8448 |
| Cure Mermaid | 10000 | 1.8448 |
| Curse of the Masked Beast | 10000 | 1.8448 |
| Cyclon Laser | 10000 | 1.8448 |
| Dancing Fairy | 10000 | 1.8448 |
| Deal of Phantom | 10000 | 1.8448 |
| Dragonic Attack | 10000 | 1.8448 |
| Dreamsprite | 10000 | 1.8448 |
| Ekibyo Drakmord | 10000 | 1.8448 |
| Fairy Box | 10000 | 1.8448 |
| Fairy Guardian | 10000 | 1.8448 |
| Fire Sorcerer | 10000 | 1.8448 |
| Fusion Gate | 10000 | 1.8448 |
| Garuda the Wind Spirit | 10000 | 1.8448 |
| Grand Tiki Elder | 10000 | 1.8448 |
| Graverobber's Retribution | 10000 | 1.8448 |
| Humanoid Slime | 10000 | 1.8448 |
| Humanoid Worm Drake | 10000 | 1.8448 |
| Hysteric Fairy | 10000 | 1.8448 |
| Jam Defender | 10000 | 1.8448 |
| Lady Assailant of Flames | 10000 | 1.8448 |
| Lady Panther | 10000 | 1.8448 |
| Maryokutai | 10000 | 1.8448 |
| Mask of Weakness | 10000 | 1.8448 |
| Melchid the Four-Face Beast | 10000 | 1.8448 |
| Miracle Dig | 10000 | 1.8448 |
| Offerings to the Doomed | 10000 | 1.8448 |
| Scroll of Bewitchment | 10000 | 1.8448 |
| Skull Lair | 10000 | 1.8448 |
| Soul of Purity and Light | 10000 | 1.8448 |
| Spirit Elimination | 10000 | 1.8448 |
| Spirit of Flames | 10000 | 1.8448 |
| St. Joan | 10000 | 1.8448 |
| Summoner of Illusions | 10000 | 1.8448 |
| Supply | 10000 | 1.8448 |
| The Dark Door | 10000 | 1.8448 |
| The Emperor's Holiday | 10000 | 1.8448 |
| The Forgiving Maiden | 10000 | 1.8448 |
| The Rock Spirit | 10000 | 1.8448 |
| The Unfriendly Amazon | 10000 | 1.8448 |
| Tornado Wall | 10000 | 1.8448 |
| Vengeful Bog Spirit | 10000 | 1.8448 |
| Worm Drake | 10000 | 1.8448 |
| Zombyra the Dark | 10000 | 1.8448 |
| Destruction Punch | 2500 | 0.4612 |
| Gilasaurus | 2500 | 0.4612 |
| Infinite Cards | 2500 | 0.4612 |
| Jam Breeding Machine | 2500 | 0.4612 |
| Jowgen the Spiritualist | 2500 | 0.4612 |
| Mask of Brutality | 2500 | 0.4612 |
| Nuvia the Wicked | 2500 | 0.4612 |
| Spirit Message "A" | 2500 | 0.4612 |
| Spirit Message "I" | 2500 | 0.4612 |
| Spirit Message "L" | 2500 | 0.4612 |
| Spirit Message "N" | 2500 | 0.4612 |
| Spirit of the Breeze | 2500 | 0.4612 |
| Spiritualism | 2500 | 0.4612 |
| Tornado Bird | 2500 | 0.4612 |
| Bazoo the Soul-Eater | 600 | 0.1107 |
| Dark Spirit of the Silent | 600 | 0.1107 |
| De-Fusion | 600 | 0.1107 |
| Fire Princess | 600 | 0.1107 |
| Jar of Greed | 600 | 0.1107 |
| Kycoo the Ghost Destroyer | 600 | 0.1107 |
| Mask of Dispel | 600 | 0.1107 |
| Mask of the Accursed | 600 | 0.1107 |
| Revival Jam | 600 | 0.1107 |
| Riryoku Field | 600 | 0.1107 |
| Card of Safe Return | 100 | 0.0184 |
| Dark Necrofear | 100 | 0.0184 |
| Destiny Board | 100 | 0.0184 |
| Mage Power | 100 | 0.0184 |
| Mask of Restrict | 100 | 0.0184 |
| Royal Command | 100 | 0.0184 |
| The Last Warrior from Another Planet | 100 | 0.0184 |
| The Masked Beast | 100 | 0.0184 |
| Torrential Tribute | 100 | 0.0184 |
| United We Stand | 100 | 0.0184 |
| Gemini Elf | 25 | 0.0046 |
| Magic Cylinder | 25 | 0.0046 |

### Android

- **Type:** Usage · **Format:** Android · **Cards in pool:** 107 · **Weight sum:** 113607

| Card | Weight | Pull % |
|---|---:|---:|
| Axe of Despair | 2791 | 2.4567 |
| Castle of Dark Illusions | 2791 | 2.4567 |
| Dark Elf | 2791 | 2.4567 |
| Dark Necrofear | 2791 | 2.4567 |
| Fake Trap | 2791 | 2.4567 |
| Firegrass | 2791 | 2.4567 |
| Giant Germ | 2791 | 2.4567 |
| Giant Soldier of Stone | 2791 | 2.4567 |
| Giant Trunade | 2791 | 2.4567 |
| Hayabusa Knight | 2791 | 2.4567 |
| La Jinn the Mystical Genie of the Lamp | 2791 | 2.4567 |
| Megamorph | 2791 | 2.4567 |
| Soul Release | 2791 | 2.4567 |
| Summoned Skull | 2791 | 2.4567 |
| The Fiend Megacyber | 2791 | 2.4567 |
| Thunder Dragon | 2791 | 2.4567 |
| Banisher of the Light | 2144 | 1.8872 |
| Kuriboh | 2144 | 1.8872 |
| Nobleman of Extermination | 2144 | 1.8872 |
| Goblin Attack Force | 1841 | 1.6205 |
| Jirai Gumo | 1841 | 1.6205 |
| Magic Drain | 1841 | 1.6205 |
| Nimble Momonga | 1841 | 1.6205 |
| Waboku | 1841 | 1.6205 |
| White Magical Hat | 1841 | 1.6205 |
| Man-Eater Bug | 1523 | 1.3406 |
| Mask of Darkness | 1523 | 1.3406 |
| Seven Tools of the Bandit | 1523 | 1.3406 |
| Mage Power | 1424 | 1.2534 |
| Upstart Goblin | 1424 | 1.2534 |
| Zombyra the Dark | 1424 | 1.2534 |
| Exchange | 1346 | 1.1848 |
| United We Stand | 1229 | 1.0818 |
| Masked Sorcerer | 1144 | 1.0070 |
| Penguin Knight | 1144 | 1.0070 |
| 4-Starred Ladybug of Doom | 1109 | 0.9762 |
| Time Seal | 1049 | 0.9234 |
| Card Destruction | 1024 | 0.9014 |
| Solemn Judgment | 980 | 0.8626 |
| Cannon Soldier | 943 | 0.8301 |
| Dust Tornado | 805 | 0.7086 |
| Magic Cylinder | 805 | 0.7086 |
| Magician of Faith | 780 | 0.6866 |
| Swords of Revealing Light | 780 | 0.6866 |
| Trap Hole | 780 | 0.6866 |
| Jar of Greed | 764 | 0.6725 |
| Fissure | 757 | 0.6663 |
| Mystic Tomato | 757 | 0.6663 |
| Bazoo the Soul-Eater | 750 | 0.6602 |
| Gemini Elf | 750 | 0.6602 |
| Torrential Tribute | 750 | 0.6602 |
| Kycoo the Ghost Destroyer | 743 | 0.6540 |
| Ceasefire | 737 | 0.6487 |
| Flame Ghost | 682 | 0.6003 |
| Flower Wolf | 682 | 0.6003 |
| Giltia the D. Knight | 682 | 0.6003 |
| Karbonala Warrior | 682 | 0.6003 |
| Punished Eagle | 682 | 0.6003 |
| Cyber Jar | 662 | 0.5827 |
| Heavy Storm | 650 | 0.5721 |
| Nobleman of Crossout | 650 | 0.5721 |
| Premature Burial | 650 | 0.5721 |
| Morphing Jar | 629 | 0.5537 |
| Painful Choice | 530 | 0.4665 |
| Snatch Steal | 526 | 0.4630 |
| Change of Heart | 522 | 0.4595 |
| Confiscation | 522 | 0.4595 |
| Dark Hole | 522 | 0.4595 |
| Delinquent Duo | 522 | 0.4595 |
| Jinzo | 522 | 0.4595 |
| Mirror Force | 522 | 0.4595 |
| Monster Reborn | 522 | 0.4595 |
| Pot of Greed | 522 | 0.4595 |
| Raigeki | 522 | 0.4595 |
| Sangan | 522 | 0.4595 |
| The Forceful Sentry | 522 | 0.4595 |
| Witch of the Black Forest | 522 | 0.4595 |
| Kwagar Hercules | 520 | 0.4577 |
| Cyber Saurus | 444 | 0.3908 |
| Kaminari Attack | 444 | 0.3908 |
| Labyrinth Tank | 444 | 0.3908 |
| Skull Knight | 444 | 0.3908 |
| Charubin the Fire Knight | 397 | 0.3495 |
| Humanoid Worm Drake | 397 | 0.3495 |
| Metal Dragon | 397 | 0.3495 |
| Thousand Dragon | 397 | 0.3495 |
| Bickuribox | 364 | 0.3204 |
| Musician King | 339 | 0.2984 |
| Fusionist | 320 | 0.2817 |
| Warrior of Tradition | 320 | 0.2817 |
| Dragoness the Wicked Knight | 304 | 0.2676 |
| Twin-Headed Thunder Dragon | 304 | 0.2676 |
| Darkfire Dragon | 290 | 0.2553 |
| Mirror Wall | 279 | 0.2456 |
| Royal Command | 279 | 0.2456 |
| Flame Swordsman | 260 | 0.2289 |
| St. Joan | 260 | 0.2289 |
| Gaia the Dragon Champion | 252 | 0.2218 |
| Thousand-Eyes Restrict | 245 | 0.2157 |
| B. Skull Dragon | 239 | 0.2104 |
| The Last Warrior from Another Planet | 223 | 0.1963 |
| Fairy Box | 184 | 0.1620 |
| Solemn Wishes | 134 | 0.1180 |
| Skull Lair | 77 | 0.0678 |
| Mystical Space Typhoon | 74 | 0.0651 |
| Call Of The Haunted | 52 | 0.0458 |
| Imperial Order | 52 | 0.0458 |

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

- **Type:** Usage · **Format:** Joey-Pegasus · **Cards in pool:** 110 · **Weight sum:** 99032

| Card | Weight | Pull % |
|---|---:|---:|
| Armed Ninja | 2144 | 2.1650 |
| Axe of Despair | 2144 | 2.1650 |
| Cold Wave | 2144 | 2.1650 |
| Giant Germ | 2144 | 2.1650 |
| Giant Rat | 2144 | 2.1650 |
| Giant Trunade | 2144 | 2.1650 |
| Guardian Sphinx | 2144 | 2.1650 |
| Hayabusa Knight | 2144 | 2.1650 |
| Jowgen the Spiritualist | 2144 | 2.1650 |
| Kotodama | 2144 | 2.1650 |
| Last Will | 2144 | 2.1650 |
| Magic Drain | 2144 | 2.1650 |
| Penguin Knight | 2144 | 2.1650 |
| Reinforcement of the Army | 2144 | 2.1650 |
| Summoned Skull | 2144 | 2.1650 |
| White Magical Hat | 2144 | 2.1650 |
| Backup Soldier | 1841 | 1.8590 |
| Exchange | 1654 | 1.6702 |
| Kuriboh | 1654 | 1.6702 |
| Masked Sorcerer | 1654 | 1.6702 |
| Nimble Momonga | 1654 | 1.6702 |
| Seven Tools of the Bandit | 1654 | 1.6702 |
| Thunder Dragon | 1654 | 1.6702 |
| Card Destruction | 1424 | 1.4379 |
| Exodia the Forbidden One | 1424 | 1.4379 |
| Left Arm of the Forbidden One | 1424 | 1.4379 |
| Left Leg of the Forbidden One | 1424 | 1.4379 |
| Right Arm of the Forbidden One | 1424 | 1.4379 |
| Right Leg of the Forbidden One | 1424 | 1.4379 |
| Swords of Revealing Light | 1424 | 1.4379 |
| Time Seal | 1183 | 1.1946 |
| Man-Eater Bug | 1109 | 1.1198 |
| United We Stand | 1109 | 1.1198 |
| Waboku | 1001 | 1.0108 |
| Cannon Soldier | 961 | 0.9704 |
| Mage Power | 961 | 0.9704 |
| Trap Hole | 961 | 0.9704 |
| Upstart Goblin | 961 | 0.9704 |
| Dust Tornado | 926 | 0.9351 |
| Solemn Judgment | 926 | 0.9351 |
| Fissure | 824 | 0.8321 |
| Ceasefire | 805 | 0.8129 |
| Jar of Greed | 805 | 0.8129 |
| Magic Cylinder | 805 | 0.8129 |
| Cyber Jar | 757 | 0.7644 |
| Torrential Tribute | 757 | 0.7644 |
| Bazoo the Soul-Eater | 743 | 0.7503 |
| Gemini Elf | 743 | 0.7503 |
| Kycoo the Ghost Destroyer | 743 | 0.7503 |
| Magician of Faith | 743 | 0.7503 |
| Mystic Tomato | 743 | 0.7503 |
| Morphing Jar | 688 | 0.6947 |
| Nobleman of Crossout | 662 | 0.6685 |
| Graceful Charity | 650 | 0.6564 |
| Heavy Storm | 562 | 0.5675 |
| Painful Choice | 530 | 0.5352 |
| Premature Burial | 530 | 0.5352 |
| Change of Heart | 522 | 0.5271 |
| Confiscation | 522 | 0.5271 |
| Dark Hole | 522 | 0.5271 |
| Delinquent Duo | 522 | 0.5271 |
| Harpie's Feather Duster | 522 | 0.5271 |
| Jinzo | 522 | 0.5271 |
| Mirror Force | 522 | 0.5271 |
| Monster Reborn | 522 | 0.5271 |
| Pot of Greed | 522 | 0.5271 |
| Raigeki | 522 | 0.5271 |
| Sangan | 522 | 0.5271 |
| Sinister Serpent | 522 | 0.5271 |
| Snatch Steal | 522 | 0.5271 |
| The Forceful Sentry | 522 | 0.5271 |
| Witch of the Black Forest | 522 | 0.5271 |
| Bickuribox | 520 | 0.5251 |
| Charubin the Fire Knight | 520 | 0.5251 |
| Dark Balter the Terrible | 520 | 0.5251 |
| Empress Judge | 520 | 0.5251 |
| Fiend Skull Dragon | 520 | 0.5251 |
| Flame Ghost | 520 | 0.5251 |
| Flower Wolf | 520 | 0.5251 |
| Giltia the D. Knight | 520 | 0.5251 |
| Humanoid Worm Drake | 520 | 0.5251 |
| Kaminari Attack | 520 | 0.5251 |
| Roaring Ocean Snake | 520 | 0.5251 |
| Ryu Senshi | 520 | 0.5251 |
| Twin-Headed Thunder Dragon | 520 | 0.5251 |
| Warrior of Tradition | 520 | 0.5251 |
| Cyber Saurus | 397 | 0.4009 |
| Dragoness the Wicked Knight | 397 | 0.4009 |
| Metal Dragon | 397 | 0.4009 |
| Musician King | 397 | 0.4009 |
| Skull Knight | 397 | 0.4009 |
| Darkfire Dragon | 339 | 0.3423 |
| Fusionist | 339 | 0.3423 |
| Labyrinth Tank | 339 | 0.3423 |
| Thousand-Eyes Restrict | 279 | 0.2817 |
| Flame Swordsman | 245 | 0.2474 |
| Gaia the Dragon Champion | 245 | 0.2474 |
| St. Joan | 233 | 0.2353 |
| B. Skull Dragon | 223 | 0.2252 |
| Rush Recklessly | 214 | 0.2161 |
| Solemn Wishes | 214 | 0.2161 |
| The Last Warrior from Another Planet | 206 | 0.2080 |
| Mirror Wall | 165 | 0.1666 |
| Offerings to the Doomed | 165 | 0.1666 |
| Limiter Removal | 142 | 0.1434 |
| Scapegoat | 104 | 0.1050 |
| Skull Lair | 82 | 0.0828 |
| Mystical Space Typhoon | 74 | 0.0747 |
| Call Of The Haunted | 52 | 0.0525 |
| Imperial Order | 52 | 0.0525 |

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

- **Type:** Usage · **Format:** Fiber · **Cards in pool:** 143 · **Weight sum:** 233899

| Card | Weight | Pull % |
|---|---:|---:|
| Black Illusion Ritual | 5162 | 2.2069 |
| Bottomless Trap Hole | 5162 | 2.2069 |
| Don Zaloog | 5162 | 2.2069 |
| Ekibyo Drakmord | 5162 | 2.2069 |
| Flash Assailant | 5162 | 2.2069 |
| Flying Kamakiri #1 | 5162 | 2.2069 |
| Jirai Gumo | 5162 | 2.2069 |
| Man-Eater Bug | 5162 | 2.2069 |
| Megamorph | 5162 | 2.2069 |
| Metalmorph | 5162 | 2.2069 |
| Nobleman of Extermination | 5162 | 2.2069 |
| Relinquished | 5162 | 2.2069 |
| Senju of the Thousand Hands | 5162 | 2.2069 |
| Sonic Bird | 5162 | 2.2069 |
| Summoned Skull | 5162 | 2.2069 |
| Cure Mermaid | 3941 | 1.6849 |
| Dark Necrofear | 3941 | 1.6849 |
| Giant Trunade | 3941 | 1.6849 |
| Graverobber | 3941 | 1.6849 |
| Jowgen the Spiritualist | 3941 | 1.6849 |
| Kuriboh | 3941 | 1.6849 |
| Mad Sword Beast | 3941 | 1.6849 |
| Mask of Darkness | 3941 | 1.6849 |
| Princess of Tsurugi | 3941 | 1.6849 |
| The Rock Spirit | 3941 | 1.6849 |
| White Magical Hat | 3941 | 1.6849 |
| Acid Trap Hole | 3369 | 1.4404 |
| Hayabusa Knight | 3369 | 1.4404 |
| Last Will | 3369 | 1.4404 |
| Nimble Momonga | 3369 | 1.4404 |
| Soul Exchange | 3369 | 1.4404 |
| Magic Drain | 3016 | 1.2894 |
| Amazoness Archer | 2768 | 1.1834 |
| Exchange | 2768 | 1.1834 |
| Blast with Chain | 2314 | 0.9893 |
| Gearfried the Iron Knight | 2314 | 0.9893 |
| Tribute to The Doomed | 2314 | 0.9893 |
| Electric Snake | 2213 | 0.9461 |
| The Fiend Megacyber | 2213 | 0.9461 |
| Waboku | 2213 | 0.9461 |
| Giant Germ | 2127 | 0.9094 |
| Magic Jammer | 1927 | 0.8239 |
| Seven Tools of the Bandit | 1874 | 0.8012 |
| Swords of Revealing Light | 1827 | 0.7811 |
| Thunder Dragon | 1827 | 0.7811 |
| Zombyra the Dark | 1827 | 0.7811 |
| Reinforcement of the Army | 1763 | 0.7537 |
| Goblin Attack Force | 1743 | 0.7452 |
| 4-Starred Ladybug of Doom | 1612 | 0.6892 |
| Solemn Judgment | 1612 | 0.6892 |
| Airknight Parshath | 1469 | 0.6280 |
| Dark Ruler Ha Des | 1431 | 0.6118 |
| Mage Power | 1365 | 0.5836 |
| Giant Rat | 1284 | 0.5490 |
| Giltia the D. Knight | 1278 | 0.5464 |
| Time Seal | 1273 | 0.5443 |
| Cannon Soldier | 1220 | 0.5216 |
| Spear Dragon | 1201 | 0.5135 |
| United We Stand | 1192 | 0.5096 |
| Upstart Goblin | 1142 | 0.4882 |
| Card Destruction | 1064 | 0.4549 |
| Punished Eagle | 971 | 0.4151 |
| Jar of Greed | 904 | 0.3865 |
| Fissure | 839 | 0.3587 |
| Asura Priest | 831 | 0.3553 |
| Ceasefire | 829 | 0.3544 |
| Trap Hole | 829 | 0.3544 |
| Empress Judge | 828 | 0.3540 |
| Flower Wolf | 828 | 0.3540 |
| Kwagar Hercules | 828 | 0.3540 |
| Creature Swap | 826 | 0.3531 |
| Dust Tornado | 822 | 0.3514 |
| Gemini Elf | 792 | 0.3386 |
| Magician of Faith | 792 | 0.3386 |
| Kycoo the Ghost Destroyer | 784 | 0.3352 |
| Bazoo the Soul-Eater | 779 | 0.3330 |
| Mystic Tomato | 762 | 0.3258 |
| Torrential Tribute | 743 | 0.3177 |
| Magic Cylinder | 683 | 0.2920 |
| Cyber Jar | 672 | 0.2873 |
| Morphing Jar | 669 | 0.2860 |
| Nobleman of Crossout | 663 | 0.2835 |
| Graceful Charity | 650 | 0.2779 |
| Metal Dragon | 630 | 0.2693 |
| Premature Burial | 586 | 0.2505 |
| Heavy Storm | 571 | 0.2441 |
| Charubin the Fire Knight | 563 | 0.2407 |
| Cyber Saurus | 563 | 0.2407 |
| Warrior of Tradition | 563 | 0.2407 |
| Yata-Garasu | 543 | 0.2322 |
| Bickuribox | 537 | 0.2296 |
| Skull Knight | 537 | 0.2296 |
| St. Joan | 537 | 0.2296 |
| Painful Choice | 534 | 0.2283 |
| Jinzo | 529 | 0.2262 |
| Sinister Serpent | 529 | 0.2262 |
| Confiscation | 526 | 0.2249 |
| Delinquent Duo | 525 | 0.2245 |
| Injection Fairy Lily | 525 | 0.2245 |
| Change of Heart | 523 | 0.2236 |
| Exiled Force | 523 | 0.2236 |
| Fiber Jar | 523 | 0.2236 |
| Harpie's Feather Duster | 523 | 0.2236 |
| Monster Reborn | 523 | 0.2236 |
| Snatch Steal | 523 | 0.2236 |
| Dark Hole | 522 | 0.2232 |
| Mirror Force | 522 | 0.2232 |
| Pot of Greed | 522 | 0.2232 |
| Raigeki | 522 | 0.2232 |
| Sangan | 522 | 0.2232 |
| The Forceful Sentry | 522 | 0.2232 |
| Witch of the Black Forest | 522 | 0.2232 |
| Blind Destruction | 516 | 0.2206 |
| Graceful Dice | 516 | 0.2206 |
| Gravity Bind | 516 | 0.2206 |
| Mirror Wall | 516 | 0.2206 |
| Rush Recklessly | 516 | 0.2206 |
| Flame Swordsman | 497 | 0.2125 |
| Flame Ghost | 480 | 0.2052 |
| Fusionist | 410 | 0.1753 |
| Thousand Dragon | 386 | 0.1650 |
| Gaia the Dragon Champion | 356 | 0.1522 |
| Roaring Ocean Snake | 356 | 0.1522 |
| Solemn Wishes | 336 | 0.1437 |
| Musician King | 321 | 0.1372 |
| Labyrinth Tank | 310 | 0.1325 |
| Darkfire Dragon | 293 | 0.1253 |
| Fiend Skull Dragon | 256 | 0.1094 |
| Dark Balter the Terrible | 250 | 0.1069 |
| Dragoness the Wicked Knight | 250 | 0.1069 |
| B. Skull Dragon | 249 | 0.1065 |
| Ryu Senshi | 234 | 0.1000 |
| Twin-Headed Thunder Dragon | 234 | 0.1000 |
| Chain Energy | 221 | 0.0945 |
| Offerings to the Doomed | 221 | 0.0945 |
| The Last Warrior from Another Planet | 210 | 0.0898 |
| Thousand-Eyes Restrict | 207 | 0.0885 |
| Limiter Removal | 141 | 0.0603 |
| Skull Lair | 110 | 0.0470 |
| Scapegoat | 82 | 0.0351 |
| Mystical Space Typhoon | 74 | 0.0316 |
| Call Of The Haunted | 52 | 0.0222 |
| Imperial Order | 52 | 0.0222 |

### Pharaonic Guardian

- **Type:** TCG · **Format:** Yata · **Cards in pool:** 21 · **Weight sum:** 95225

| Card | Weight | Pull % |
|---|---:|---:|
| Des Lacooda | 10000 | 10.5014 |
| Different Dimension Capsule | 10000 | 10.5014 |
| Gravekeeper's Spy | 10000 | 10.5014 |
| Metamorphosis | 10000 | 10.5014 |
| Raigeki Break | 10000 | 10.5014 |
| Reasoning | 10000 | 10.5014 |
| Swarm of Locusts | 10000 | 10.5014 |
| Trap Dustshoot | 10000 | 10.5014 |
| Book of Moon | 2500 | 2.6254 |
| King Tiger Wanghu | 2500 | 2.6254 |
| Newdoria | 2500 | 2.6254 |
| Pyramid Turtle | 2500 | 2.6254 |
| Spirit Reaper | 2500 | 2.6254 |
| Book of Life | 600 | 0.6301 |
| Mirage of Nightmare | 600 | 0.6301 |
| Reaper on the Nightmare | 600 | 0.6301 |
| Sasuke Samurai | 600 | 0.6301 |
| Don Zaloog | 100 | 0.1050 |
| Guardian Sphinx | 100 | 0.1050 |
| Nightmare Wheel | 100 | 0.1050 |
| Bell of Destruction | 25 | 0.0263 |

### Yata

- **Type:** Usage · **Format:** Yata · **Cards in pool:** 136 · **Weight sum:** 154170

| Card | Weight | Pull % |
|---|---:|---:|
| Cold Wave | 3681 | 2.3876 |
| Graverobber | 3681 | 2.3876 |
| Just Desserts | 3681 | 2.3876 |
| Last Will | 3681 | 2.3876 |
| Magic Drain | 3681 | 2.3876 |
| Mask of Darkness | 3681 | 2.3876 |
| Masked Sorcerer | 3681 | 2.3876 |
| Morphing Jar #2 | 3148 | 2.0419 |
| Book of Life | 2819 | 1.8285 |
| Giant Trunade | 2819 | 1.8285 |
| Magic Jammer | 2819 | 1.8285 |
| Princess of Tsurugi | 2819 | 1.8285 |
| Waboku | 2819 | 1.8285 |
| Amazoness Archer | 2415 | 1.5665 |
| Barrel Dragon | 2415 | 1.5665 |
| Giant Rat | 2415 | 1.5665 |
| Kuriboh | 2415 | 1.5665 |
| Marauding Captain | 2415 | 1.5665 |
| Mechanicalchaser | 2415 | 1.5665 |
| Pyramid Turtle | 2415 | 1.5665 |
| Reasoning | 2415 | 1.5665 |
| Spear Dragon | 2415 | 1.5665 |
| United We Stand | 2415 | 1.5665 |
| White Magical Hat | 2415 | 1.5665 |
| Giant Germ | 2165 | 1.4043 |
| Nimble Momonga | 2165 | 1.4043 |
| Guardian Sphinx | 1991 | 1.2914 |
| Card Destruction | 1859 | 1.2058 |
| Goblin Attack Force | 1755 | 1.1384 |
| Sasuke Samurai | 1755 | 1.1384 |
| Seven Tools of the Bandit | 1755 | 1.1384 |
| Blast with Chain | 1670 | 1.0832 |
| Gearfried the Iron Knight | 1670 | 1.0832 |
| Raigeki Break | 1599 | 1.0372 |
| Solemn Judgment | 1599 | 1.0372 |
| Time Seal | 1599 | 1.0372 |
| Trap Dustshoot | 1599 | 1.0372 |
| 4-Starred Ladybug of Doom | 1538 | 0.9976 |
| Cure Mermaid | 1538 | 0.9976 |
| Cannon Soldier | 1485 | 0.9632 |
| Trap Hole | 1485 | 0.9632 |
| Metamorphosis | 1438 | 0.9327 |
| Upstart Goblin | 1438 | 0.9327 |
| Airknight Parshath | 1396 | 0.9055 |
| Electric Snake | 1359 | 0.8815 |
| Swords of Revealing Light | 1325 | 0.8594 |
| Jar of Greed | 1217 | 0.7894 |
| Dark Ruler Ha Des | 1174 | 0.7615 |
| Newdoria | 1174 | 0.7615 |
| Asura Priest | 1136 | 0.7368 |
| King Tiger Wanghu | 1073 | 0.6960 |
| Bazoo the Soul-Eater | 1059 | 0.6869 |
| Kycoo the Ghost Destroyer | 1059 | 0.6869 |
| Creature Swap | 1016 | 0.6590 |
| Fissure | 999 | 0.6480 |
| Cyber Jar | 943 | 0.6117 |
| Warrior of Tradition | 906 | 0.5877 |
| Dust Tornado | 897 | 0.5818 |
| Gravekeeper's Spy | 883 | 0.5727 |
| Zombyra the Dark | 859 | 0.5572 |
| Magician of Faith | 848 | 0.5500 |
| Magic Cylinder | 842 | 0.5462 |
| Reinforcement of the Army | 827 | 0.5364 |
| Don Zaloog | 822 | 0.5332 |
| Gemini Elf | 822 | 0.5332 |
| Morphing Jar | 812 | 0.5267 |
| Mystic Tomato | 808 | 0.5241 |
| Spirit Reaper | 779 | 0.5053 |
| Torrential Tribute | 750 | 0.4865 |
| Ceasefire | 701 | 0.4547 |
| Premature Burial | 701 | 0.4547 |
| Punished Eagle | 689 | 0.4469 |
| Nobleman of Crossout | 653 | 0.4236 |
| Fusionist | 588 | 0.3814 |
| Skull Knight | 588 | 0.3814 |
| Painful Choice | 575 | 0.3730 |
| Yata-Garasu | 575 | 0.3730 |
| Sinister Serpent | 560 | 0.3632 |
| Fiber Jar | 549 | 0.3561 |
| Injection Fairy Lily | 547 | 0.3548 |
| Heavy Storm | 539 | 0.3496 |
| Confiscation | 530 | 0.3438 |
| Delinquent Duo | 528 | 0.3425 |
| Exiled Force | 528 | 0.3425 |
| Sangan | 528 | 0.3425 |
| Jinzo | 526 | 0.3412 |
| Monster Reborn | 526 | 0.3412 |
| Metal Dragon | 525 | 0.3405 |
| Thousand Dragon | 525 | 0.3405 |
| Bell of Destruction | 522 | 0.3386 |
| Change of Heart | 522 | 0.3386 |
| Dark Hole | 522 | 0.3386 |
| Graceful Charity | 522 | 0.3386 |
| Harpie's Feather Duster | 522 | 0.3386 |
| Mirror Force | 522 | 0.3386 |
| Pot of Greed | 522 | 0.3386 |
| Raigeki | 522 | 0.3386 |
| Snatch Steal | 522 | 0.3386 |
| The Forceful Sentry | 522 | 0.3386 |
| Witch of the Black Forest | 522 | 0.3386 |
| Flower Wolf | 481 | 0.3120 |
| Giltia the D. Knight | 481 | 0.3120 |
| Charubin the Fire Knight | 448 | 0.2906 |
| Flame Ghost | 422 | 0.2737 |
| Labyrinth Tank | 422 | 0.2737 |
| Karbonala Warrior | 401 | 0.2601 |
| Cyber Saurus | 383 | 0.2484 |
| Forced Requisition | 368 | 0.2387 |
| Mirror Wall | 368 | 0.2387 |
| Musician King | 368 | 0.2387 |
| Empress Judge | 354 | 0.2296 |
| Flame Swordsman | 354 | 0.2296 |
| Roaring Ocean Snake | 354 | 0.2296 |
| Bickuribox | 343 | 0.2225 |
| Twin-Headed Thunder Dragon | 281 | 0.1823 |
| Gaia the Dragon Champion | 271 | 0.1758 |
| St. Joan | 267 | 0.1732 |
| Darkfire Dragon | 258 | 0.1673 |
| Dragoness the Wicked Knight | 244 | 0.1583 |
| Limiter Removal | 241 | 0.1563 |
| B. Skull Dragon | 227 | 0.1472 |
| Nightmare Wheel | 216 | 0.1401 |
| The Last Warrior from Another Planet | 212 | 0.1375 |
| Reaper on the Nightmare | 208 | 0.1349 |
| Fiend Skull Dragon | 192 | 0.1245 |
| Dark Balter the Terrible | 188 | 0.1219 |
| Chain Energy | 185 | 0.1200 |
| Ryu Senshi | 183 | 0.1187 |
| Thousand-Eyes Restrict | 179 | 0.1161 |
| Solemn Wishes | 153 | 0.0992 |
| Book of Moon | 107 | 0.0694 |
| Scapegoat | 94 | 0.0610 |
| Mystical Space Typhoon | 74 | 0.0480 |
| Call Of The Haunted | 53 | 0.0344 |
| Imperial Order | 52 | 0.0337 |
| Mirage of Nightmare | 52 | 0.0337 |

### Magician's Force

- **Type:** TCG · **Format:** Scientist · **Cards in pool:** 18 · **Weight sum:** 96025

| Card | Weight | Pull % |
|---|---:|---:|
| Dark Scorpion - Cliff the Trap Remover | 10000 | 10.4140 |
| Magical Merchant | 10000 | 10.4140 |
| Magical Scientist | 10000 | 10.4140 |
| My Body as a Shield | 10000 | 10.4140 |
| Old Vindictive Magician | 10000 | 10.4140 |
| Poison of the Old Man | 10000 | 10.4140 |
| Roulette Barrel | 10000 | 10.4140 |
| Wave-Motion Cannon | 10000 | 10.4140 |
| Secret Barrel | 7000 | 7.2898 |
| Apprentice Magician | 2500 | 2.6035 |
| Big Bang Shot | 2500 | 2.6035 |
| Des Koala | 2500 | 2.6035 |
| Skilled Dark Magician | 600 | 0.6248 |
| Tribe-Infecting Virus | 600 | 0.6248 |
| Breaker the Magical Warrior | 100 | 0.1041 |
| Dark Paladin | 100 | 0.1041 |
| Spell Canceller | 100 | 0.1041 |
| Dark Magician Girl | 25 | 0.0260 |

### Scientist

- **Type:** Usage · **Format:** Scientist · **Cards in pool:** 92 · **Weight sum:** 74955

| Card | Weight | Pull % |
|---|---:|---:|
| Amazoness Archer | 1804 | 2.4068 |
| Apprentice Magician | 1804 | 2.4068 |
| Asura Priest | 1804 | 2.4068 |
| Dark Magician | 1804 | 2.4068 |
| Dark Ruler Ha Des | 1804 | 2.4068 |
| Don Zaloog | 1804 | 2.4068 |
| Kycoo the Ghost Destroyer | 1804 | 2.4068 |
| Nimble Momonga | 1804 | 2.4068 |
| Sasuke Samurai | 1804 | 2.4068 |
| Skilled Dark Magician | 1804 | 2.4068 |
| Spell Canceller | 1804 | 2.4068 |
| Waboku | 1804 | 2.4068 |
| Creature Swap | 1552 | 2.0706 |
| Reinforcement of the Army | 1552 | 2.0706 |
| Airknight Parshath | 1396 | 1.8625 |
| Goblin Attack Force | 1396 | 1.8625 |
| Jar of Greed | 1396 | 1.8625 |
| Jowgen the Spiritualist | 1396 | 1.8625 |
| Spiritualism | 1396 | 1.8625 |
| Cannon Soldier | 1205 | 1.6076 |
| Fissure | 1205 | 1.6076 |
| Giant Trunade | 1205 | 1.6076 |
| Raigeki Break | 1205 | 1.6076 |
| Reasoning | 1205 | 1.6076 |
| Swords of Revealing Light | 1205 | 1.6076 |
| Upstart Goblin | 1205 | 1.6076 |
| Gilasaurus | 1087 | 1.4502 |
| Magician of Faith | 1087 | 1.4502 |
| Newdoria | 1087 | 1.4502 |
| Gravekeeper's Spy | 1005 | 1.3408 |
| Metamorphosis | 1005 | 1.3408 |
| Trap Dustshoot | 1005 | 1.3408 |
| Cyber Jar | 943 | 1.2581 |
| Dust Tornado | 943 | 1.2581 |
| Last Will | 943 | 1.2581 |
| Spirit Reaper | 943 | 1.2581 |
| Catapult Turtle | 893 | 1.1914 |
| King Tiger Wanghu | 893 | 1.1914 |
| Mystic Tomato | 819 | 1.0927 |
| Nobleman of Crossout | 778 | 1.0380 |
| Torrential Tribute | 765 | 1.0206 |
| Card Destruction | 743 | 0.9913 |
| Ceasefire | 743 | 0.9913 |
| Magical Scientist | 743 | 0.9913 |
| Premature Burial | 650 | 0.8672 |
| Heavy Storm | 618 | 0.8245 |
| Yata-Garasu | 618 | 0.8245 |
| Injection Fairy Lily | 592 | 0.7898 |
| The Forceful Sentry | 592 | 0.7898 |
| Confiscation | 571 | 0.7618 |
| Delinquent Duo | 571 | 0.7618 |
| Exiled Force | 571 | 0.7618 |
| Jinzo | 571 | 0.7618 |
| Sinister Serpent | 571 | 0.7618 |
| Breaker the Magical Warrior | 552 | 0.7364 |
| Fiber Jar | 536 | 0.7151 |
| Mirror Force | 536 | 0.7151 |
| Monster Reborn | 536 | 0.7151 |
| Tribe-Infecting Virus | 536 | 0.7151 |
| Bell of Destruction | 522 | 0.6964 |
| Change of Heart | 522 | 0.6964 |
| Dark Hole | 522 | 0.6964 |
| Graceful Charity | 522 | 0.6964 |
| Harpie's Feather Duster | 522 | 0.6964 |
| Painful Choice | 522 | 0.6964 |
| Pot of Greed | 522 | 0.6964 |
| Raigeki | 522 | 0.6964 |
| Sangan | 522 | 0.6964 |
| Snatch Steal | 522 | 0.6964 |
| Witch of the Black Forest | 522 | 0.6964 |
| Dark Paladin | 435 | 0.5803 |
| Reaper on the Nightmare | 435 | 0.5803 |
| The Last Warrior from Another Planet | 435 | 0.5803 |
| Darkfire Dragon | 218 | 0.2908 |
| Dragoness the Wicked Knight | 196 | 0.2615 |
| Mask of Restrict | 180 | 0.2401 |
| Offerings to the Doomed | 180 | 0.2401 |
| Empress Judge | 174 | 0.2321 |
| Fiend Skull Dragon | 174 | 0.2321 |
| Punished Eagle | 174 | 0.2321 |
| Roaring Ocean Snake | 174 | 0.2321 |
| Dark Balter the Terrible | 168 | 0.2241 |
| Ryu Senshi | 168 | 0.2241 |
| Thousand-Eyes Restrict | 168 | 0.2241 |
| Royal Decree | 139 | 0.1854 |
| Scapegoat | 139 | 0.1854 |
| Poison of the Old Man | 120 | 0.1601 |
| Book of Moon | 89 | 0.1187 |
| Mystical Space Typhoon | 74 | 0.0987 |
| Mirage of Nightmare | 55 | 0.0734 |
| Call Of The Haunted | 53 | 0.0707 |
| Imperial Order | 52 | 0.0694 |

### Dark Crisis

- **Type:** TCG · **Format:** Vampire · **Cards in pool:** 10 · **Weight sum:** 53825

| Card | Weight | Pull % |
|---|---:|---:|
| Final Attack Orders | 10000 | 18.5787 |
| Gyaku-Gire Panda | 10000 | 18.5787 |
| Ojama Trio | 10000 | 18.5787 |
| Sakuretsu Armor | 10000 | 18.5787 |
| Token Thanksgiving | 10000 | 18.5787 |
| Tsukuyomi | 2500 | 4.6447 |
| D.D. Warrior Lady | 600 | 1.1147 |
| Dark Flare Knight | 600 | 1.1147 |
| Interdimensional Matter Transporter | 100 | 0.1858 |
| Vampire Lord | 25 | 0.0464 |

### Vampire

- **Type:** Usage · **Format:** Vampire · **Cards in pool:** 114 · **Weight sum:** 127168

| Card | Weight | Pull % |
|---|---:|---:|
| Book of Life | 2305 | 1.8126 |
| Bottomless Trap Hole | 2305 | 1.8126 |
| Cannon Soldier | 2305 | 1.8126 |
| Catapult Turtle | 2305 | 1.8126 |
| Cipher Soldier | 2305 | 1.8126 |
| Cold Wave | 2305 | 1.8126 |
| Dark Scorpion - Cliff the Trap Remover | 2305 | 1.8126 |
| Des Koala | 2305 | 1.8126 |
| Electric Snake | 2305 | 1.8126 |
| Gemini Elf | 2305 | 1.8126 |
| Goblin Attack Force | 2305 | 1.8126 |
| Guardian Sphinx | 2305 | 1.8126 |
| Gyaku-Gire Panda | 2305 | 1.8126 |
| Last Will | 2305 | 1.8126 |
| Masked Sorcerer | 2305 | 1.8126 |
| Mother Grizzly | 2305 | 1.8126 |
| Ojama Trio | 2305 | 1.8126 |
| Pyramid Turtle | 2305 | 1.8126 |
| Raigeki Break | 2305 | 1.8126 |
| Secret Barrel | 2305 | 1.8126 |
| Slate Warrior | 2305 | 1.8126 |
| Spirit Reaper | 2305 | 1.8126 |
| Summoned Skull | 2305 | 1.8126 |
| Time Seal | 2305 | 1.8126 |
| Waboku | 2305 | 1.8126 |
| White Magical Hat | 2305 | 1.8126 |
| Creature Swap | 1978 | 1.5554 |
| Marauding Captain | 1978 | 1.5554 |
| Morphing Jar | 1978 | 1.5554 |
| Giant Trunade | 1776 | 1.3966 |
| Jar of Greed | 1776 | 1.3966 |
| Just Desserts | 1776 | 1.3966 |
| Kycoo the Ghost Destroyer | 1776 | 1.3966 |
| Solemn Judgment | 1776 | 1.3966 |
| Trap Hole | 1776 | 1.3966 |
| Dark Ruler Ha Des | 1528 | 1.2016 |
| Mage Power | 1528 | 1.2016 |
| Swords of Revealing Light | 1528 | 1.2016 |
| Trap Dustshoot | 1528 | 1.2016 |
| Upstart Goblin | 1528 | 1.2016 |
| Vampire Lord | 1528 | 1.2016 |
| Amazoness Archer | 1375 | 1.0812 |
| Mystic Tomato | 1375 | 1.0812 |
| The Little Swordsman of Aile | 1375 | 1.0812 |
| Metamorphosis | 1268 | 0.9971 |
| Card Destruction | 1187 | 0.9334 |
| Magic Cylinder | 1187 | 0.9334 |
| Dust Tornado | 1123 | 0.8831 |
| Sasuke Samurai | 1123 | 0.8831 |
| Cyber Jar | 1027 | 0.8076 |
| King Tiger Wanghu | 1027 | 0.8076 |
| Magician of Faith | 990 | 0.7785 |
| Sakuretsu Armor | 903 | 0.7101 |
| Ceasefire | 860 | 0.6763 |
| Don Zaloog | 860 | 0.6763 |
| Gravekeeper's Spy | 860 | 0.6763 |
| Zombyra the Dark | 841 | 0.6613 |
| D.D. Warrior Lady | 766 | 0.6024 |
| Torrential Tribute | 743 | 0.5843 |
| Premature Burial | 681 | 0.5355 |
| Reinforcement of the Army | 681 | 0.5355 |
| Nobleman of Crossout | 670 | 0.5269 |
| Painful Choice | 610 | 0.4797 |
| Yata-Garasu | 584 | 0.4592 |
| Injection Fairy Lily | 563 | 0.4427 |
| Empress Judge | 560 | 0.4404 |
| Giltia the D. Knight | 560 | 0.4404 |
| Skull Knight | 560 | 0.4404 |
| St. Joan | 560 | 0.4404 |
| Heavy Storm | 554 | 0.4356 |
| Delinquent Duo | 545 | 0.4286 |
| Fiber Jar | 545 | 0.4286 |
| Jinzo | 545 | 0.4286 |
| Magical Scientist | 537 | 0.4223 |
| Sinister Serpent | 537 | 0.4223 |
| Tribe-Infecting Virus | 537 | 0.4223 |
| Bell of Destruction | 529 | 0.4160 |
| Confiscation | 529 | 0.4160 |
| Exiled Force | 529 | 0.4160 |
| Snatch Steal | 529 | 0.4160 |
| Breaker the Magical Warrior | 522 | 0.4105 |
| Change of Heart | 522 | 0.4105 |
| Dark Hole | 522 | 0.4105 |
| Graceful Charity | 522 | 0.4105 |
| Harpie's Feather Duster | 522 | 0.4105 |
| Mirror Force | 522 | 0.4105 |
| Monster Reborn | 522 | 0.4105 |
| Pot of Greed | 522 | 0.4105 |
| Raigeki | 522 | 0.4105 |
| Sangan | 522 | 0.4105 |
| The Forceful Sentry | 522 | 0.4105 |
| Witch of the Black Forest | 522 | 0.4105 |
| B. Skull Dragon | 427 | 0.3358 |
| Flame Ghost | 427 | 0.3358 |
| Twin-Headed Thunder Dragon | 427 | 0.3358 |
| Punished Eagle | 365 | 0.2870 |
| Roaring Ocean Snake | 250 | 0.1966 |
| The Last Warrior from Another Planet | 250 | 0.1966 |
| Darkfire Dragon | 239 | 0.1879 |
| Dragoness the Wicked Knight | 239 | 0.1879 |
| Reaper on the Nightmare | 208 | 0.1636 |
| Dark Flare Knight | 177 | 0.1392 |
| Fiend Skull Dragon | 171 | 0.1345 |
| Dark Balter the Terrible | 168 | 0.1321 |
| Ryu Senshi | 168 | 0.1321 |
| Thousand-Eyes Restrict | 168 | 0.1321 |
| Poison of the Old Man | 152 | 0.1195 |
| Scapegoat | 126 | 0.0991 |
| Book of Moon | 95 | 0.0747 |
| Royal Decree | 92 | 0.0723 |
| Mystical Space Typhoon | 74 | 0.0582 |
| Call Of The Haunted | 53 | 0.0417 |
| Mirage of Nightmare | 53 | 0.0417 |
| Imperial Order | 52 | 0.0409 |

### Starter Deck- Yugi Evolution

- **Type:** TCG · **Format:** Chaos · **Cards in pool:** 45 · **Weight sum:** 421300

| Card | Weight | Pull % |
|---|---:|---:|
| Axe of Despair | 10000 | 2.3736 |
| Backup Soldier | 10000 | 2.3736 |
| Beaver Warrior | 10000 | 2.3736 |
| Card Destruction | 10000 | 2.3736 |
| Celtic Guardian | 10000 | 2.3736 |
| Change of Heart | 10000 | 2.3736 |
| Dark Hole | 10000 | 2.3736 |
| Dian Keto the Cure Master | 10000 | 2.3736 |
| Feral Imp | 10000 | 2.3736 |
| Fissure | 10000 | 2.3736 |
| Gaia The Fierce Knight | 10000 | 2.3736 |
| Gazelle the King of Mythical Beasts | 10000 | 2.3736 |
| Giant Rat | 10000 | 2.3736 |
| Giant Soldier of Stone | 10000 | 2.3736 |
| Great White | 10000 | 2.3736 |
| Kuriboh | 10000 | 2.3736 |
| Last Will | 10000 | 2.3736 |
| Magic Jammer | 10000 | 2.3736 |
| Malevolent Nuzzler | 10000 | 2.3736 |
| Man-Eater Bug | 10000 | 2.3736 |
| Monster Reborn | 10000 | 2.3736 |
| Mystic Plasma Zone | 10000 | 2.3736 |
| Mystical Elf | 10000 | 2.3736 |
| Mystical Space Typhoon | 10000 | 2.3736 |
| Neo the Magic Swordsman | 10000 | 2.3736 |
| Pot of Greed | 10000 | 2.3736 |
| Raigeki Break | 10000 | 2.3736 |
| Remove Trap | 10000 | 2.3736 |
| Sangan | 10000 | 2.3736 |
| Seven Tools of the Bandit | 10000 | 2.3736 |
| Shift | 10000 | 2.3736 |
| Sonic Bird | 10000 | 2.3736 |
| Spellbinding Circle | 10000 | 2.3736 |
| Summoned Skull | 10000 | 2.3736 |
| Swords of Revealing Light | 10000 | 2.3736 |
| The Eye of Truth | 10000 | 2.3736 |
| The Reliable Guardian | 10000 | 2.3736 |
| Trap Hole | 10000 | 2.3736 |
| Waboku | 10000 | 2.3736 |
| Wall of Illusion | 10000 | 2.3736 |
| Winged Dragon,Guardian of the Fortress #1 | 10000 | 2.3736 |
| Zombyra the Dark | 10000 | 2.3736 |
| Black Luster Ritual | 600 | 0.1424 |
| Dark Magician | 600 | 0.1424 |
| Black Luster Soldier | 100 | 0.0237 |

### Starter Deck- Kaiba Evolution

- **Type:** TCG · **Format:** Chaos · **Cards in pool:** 39 · **Weight sum:** 380600

| Card | Weight | Pull % |
|---|---:|---:|
| Battle Ox | 10000 | 2.6274 |
| Change of Heart | 10000 | 2.6274 |
| De-Spell | 10000 | 2.6274 |
| Dust Tornado | 10000 | 2.6274 |
| Fissure | 10000 | 2.6274 |
| Gift of The Mystical Elf | 10000 | 2.6274 |
| Hane-Hane | 10000 | 2.6274 |
| Hyozanryu | 10000 | 2.6274 |
| Jar of Greed | 10000 | 2.6274 |
| Just Desserts | 10000 | 2.6274 |
| Kojikocy | 10000 | 2.6274 |
| Koumori Dragon | 10000 | 2.6274 |
| La Jinn the Mystical Genie of the Lamp | 10000 | 2.6274 |
| Light of Intervention | 10000 | 2.6274 |
| Lord of D. | 10000 | 2.6274 |
| Mad Sword Beast | 10000 | 2.6274 |
| Megamorph | 10000 | 2.6274 |
| Monster Reborn | 10000 | 2.6274 |
| Mountain | 10000 | 2.6274 |
| Mysterious Puppeteer | 10000 | 2.6274 |
| Mystic Horseman | 10000 | 2.6274 |
| Mystic Tomato | 10000 | 2.6274 |
| Nobleman of Crossout | 10000 | 2.6274 |
| Ookazi | 10000 | 2.6274 |
| Premature Burial | 10000 | 2.6274 |
| Riryoku Field | 10000 | 2.6274 |
| Rogue Doll | 10000 | 2.6274 |
| Rush Recklessly | 10000 | 2.6274 |
| Ryu-Kishin Powered | 10000 | 2.6274 |
| Skull Red Bird | 10000 | 2.6274 |
| Soul Exchange | 10000 | 2.6274 |
| The Flute of Summoning Dragon | 10000 | 2.6274 |
| The Inexperienced Spy | 10000 | 2.6274 |
| Trap Hole | 10000 | 2.6274 |
| Trap Master | 10000 | 2.6274 |
| Tribute to The Doomed | 10000 | 2.6274 |
| Uraby | 10000 | 2.6274 |
| Witch of the Black Forest | 10000 | 2.6274 |
| Shadow Spell | 600 | 0.1576 |

### Invasion of Chaos

- **Type:** TCG · **Format:** Chaos · **Cards in pool:** 15 · **Weight sum:** 77925

| Card | Weight | Pull % |
|---|---:|---:|
| Chaos Sorcerer | 10000 | 12.8329 |
| Cursed Seal of the Forbidden Spell | 10000 | 12.8329 |
| Gigantes | 10000 | 12.8329 |
| Ryu Kokki | 10000 | 12.8329 |
| Sacred Crane | 10000 | 12.8329 |
| Smashing Ground | 10000 | 12.8329 |
| Stealth Bird | 10000 | 12.8329 |
| Berserk Gorilla | 2500 | 3.2082 |
| Chain Disappearance | 2500 | 3.2082 |
| Compulsory Evacuation Device | 2500 | 3.2082 |
| Black Luster Soldier - Envoy of Beginning | 100 | 0.1283 |
| Dark Magician of Chaos | 100 | 0.1283 |
| Dimension Fusion | 100 | 0.1283 |
| Manticore of Darkness | 100 | 0.1283 |
| Chaos Emperor Dragon - Envoy of the End | 25 | 0.0321 |

### Chaos

- **Type:** Usage · **Format:** Chaos · **Cards in pool:** 142 · **Weight sum:** 185261

| Card | Weight | Pull % |
|---|---:|---:|
| Amazoness Archer | 3840 | 2.0728 |
| Catapult Turtle | 3840 | 2.0728 |
| Dark Ruler Ha Des | 3840 | 2.0728 |
| Electric Snake | 3840 | 2.0728 |
| Gigantes | 3840 | 2.0728 |
| Goblin Attack Force | 3840 | 2.0728 |
| Hysteric Fairy | 3840 | 2.0728 |
| Kuriboh | 3840 | 2.0728 |
| Magical Merchant | 3840 | 2.0728 |
| Pyramid Turtle | 3840 | 2.0728 |
| Return from the Different Dimension | 3840 | 2.0728 |
| Spell Canceller | 3840 | 2.0728 |
| White Magical Hat | 3840 | 2.0728 |
| Zaborg the Thunder Monarch | 3840 | 2.0728 |
| Axe of Despair | 2939 | 1.5864 |
| Black Illusion Ritual | 2939 | 1.5864 |
| Cursed Seal of the Forbidden Spell | 2939 | 1.5864 |
| Jar of Greed | 2939 | 1.5864 |
| Magic Jammer | 2939 | 1.5864 |
| Masked Sorcerer | 2939 | 1.5864 |
| Relinquished | 2939 | 1.5864 |
| Spiritualism | 2939 | 1.5864 |
| Giant Trunade | 2517 | 1.3586 |
| Roulette Barrel | 2517 | 1.3586 |
| Soul Release | 2517 | 1.3586 |
| Spear Dragon | 2517 | 1.3586 |
| The Shallow Grave | 2517 | 1.3586 |
| Vampire Lord | 2517 | 1.3586 |
| Time Seal | 2257 | 1.2183 |
| 4-Starred Ladybug of Doom | 2074 | 1.1195 |
| Dimension Fusion | 2074 | 1.1195 |
| Mystic Tomato | 2074 | 1.1195 |
| Berserk Gorilla | 1937 | 1.0456 |
| Cipher Soldier | 1937 | 1.0456 |
| Sasuke Samurai | 1937 | 1.0456 |
| Skilled Dark Magician | 1937 | 1.0456 |
| Airknight Parshath | 1828 | 0.9867 |
| Exchange | 1828 | 0.9867 |
| Bottomless Trap Hole | 1739 | 0.9387 |
| Don Zaloog | 1739 | 0.9387 |
| King Tiger Wanghu | 1739 | 0.9387 |
| Creature Swap | 1665 | 0.8987 |
| Thunder Nyan Nyan | 1665 | 0.8987 |
| Metamorphosis | 1601 | 0.8642 |
| Cold Wave | 1546 | 0.8345 |
| Tsukuyomi | 1497 | 0.8080 |
| Dust Tornado | 1453 | 0.7843 |
| Spirit Reaper | 1453 | 0.7843 |
| Chaos Sorcerer | 1415 | 0.7638 |
| Solemn Judgment | 1415 | 0.7638 |
| Dark Magician of Chaos | 1379 | 0.7444 |
| Swords of Revealing Light | 1379 | 0.7444 |
| Waboku | 1379 | 0.7444 |
| Sakuretsu Armor | 1318 | 0.7114 |
| Asura Priest | 1266 | 0.6834 |
| Ceasefire | 1221 | 0.6591 |
| Zombyra the Dark | 1221 | 0.6591 |
| Magic Cylinder | 1115 | 0.6019 |
| Morphing Jar | 1115 | 0.6019 |
| Raigeki Break | 1074 | 0.5797 |
| Reinforcement of the Army | 1056 | 0.5700 |
| Thunder Dragon | 1039 | 0.5608 |
| Upstart Goblin | 979 | 0.5284 |
| Trap Dustshoot | 971 | 0.5241 |
| Deepsea Shark | 946 | 0.5106 |
| St. Joan | 946 | 0.5106 |
| Cyber Jar | 874 | 0.4718 |
| Gravekeeper's Spy | 864 | 0.4664 |
| Torrential Tribute | 789 | 0.4259 |
| D.D. Warrior Lady | 778 | 0.4199 |
| Magician of Faith | 778 | 0.4199 |
| Kycoo the Ghost Destroyer | 771 | 0.4162 |
| Exiled Force | 761 | 0.4108 |
| Card Destruction | 752 | 0.4059 |
| Yata-Garasu | 727 | 0.3924 |
| Bickuribox | 720 | 0.3886 |
| Labyrinth Tank | 720 | 0.3886 |
| Metal Dragon | 720 | 0.3886 |
| Warrior of Tradition | 720 | 0.3886 |
| Injection Fairy Lily | 698 | 0.3768 |
| Nobleman of Crossout | 698 | 0.3768 |
| Magical Scientist | 634 | 0.3422 |
| Jinzo | 630 | 0.3401 |
| Flame Swordsman | 614 | 0.3314 |
| Skull Knight | 614 | 0.3314 |
| Premature Burial | 610 | 0.3293 |
| Heavy Storm | 599 | 0.3233 |
| Fiber Jar | 580 | 0.3131 |
| Sinister Serpent | 553 | 0.2985 |
| Punished Eagle | 548 | 0.2958 |
| Confiscation | 540 | 0.2915 |
| Sangan | 540 | 0.2915 |
| Breaker the Magical Warrior | 538 | 0.2904 |
| Delinquent Duo | 538 | 0.2904 |
| Mirror Force | 533 | 0.2877 |
| Snatch Steal | 531 | 0.2866 |
| Dark Hole | 529 | 0.2855 |
| Painful Choice | 529 | 0.2855 |
| Tribe-Infecting Virus | 529 | 0.2855 |
| The Forceful Sentry | 527 | 0.2845 |
| Harpie's Feather Duster | 525 | 0.2834 |
| Witch of the Black Forest | 525 | 0.2834 |
| Bell of Destruction | 524 | 0.2828 |
| Black Luster Soldier - Envoy of Beginning | 524 | 0.2828 |
| Change of Heart | 524 | 0.2828 |
| Chaos Emperor Dragon - Envoy of the End | 524 | 0.2828 |
| Graceful Charity | 522 | 0.2818 |
| Monster Reborn | 522 | 0.2818 |
| Pot of Greed | 522 | 0.2818 |
| Raigeki | 522 | 0.2818 |
| Dark Paladin | 502 | 0.2710 |
| Empress Judge | 502 | 0.2710 |
| Karbonala Warrior | 502 | 0.2710 |
| B. Skull Dragon | 400 | 0.2159 |
| Fusionist | 400 | 0.2159 |
| Twin-Headed Thunder Dragon | 400 | 0.2159 |
| Gaia the Dragon Champion | 384 | 0.2073 |
| Musician King | 384 | 0.2073 |
| The Last Warrior from Another Planet | 384 | 0.2073 |
| Dragoness the Wicked Knight | 357 | 0.1927 |
| Flame Ghost | 357 | 0.1927 |
| Darkfire Dragon | 320 | 0.1727 |
| Enemy Controller | 293 | 0.1582 |
| Reaper on the Nightmare | 278 | 0.1501 |
| Giltia the D. Knight | 274 | 0.1479 |
| Emergency Provisions | 251 | 0.1355 |
| Prohibition | 225 | 0.1215 |
| Roaring Ocean Snake | 225 | 0.1215 |
| Royal Oppression | 225 | 0.1215 |
| Dark Flare Knight | 188 | 0.1015 |
| Fiend Skull Dragon | 178 | 0.0961 |
| Ryu Senshi | 174 | 0.0939 |
| Dark Balter the Terrible | 173 | 0.0934 |
| Thousand-Eyes Restrict | 173 | 0.0934 |
| Book of Moon | 166 | 0.0896 |
| Poison of the Old Man | 145 | 0.0783 |
| Royal Decree | 129 | 0.0696 |
| Scapegoat | 129 | 0.0696 |
| Mystical Space Typhoon | 74 | 0.0399 |
| Call Of The Haunted | 55 | 0.0297 |
| Mirage of Nightmare | 54 | 0.0291 |
| Imperial Order | 52 | 0.0281 |

### Ancient Sanctuary

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 7 · **Weight sum:** 40200

| Card | Weight | Pull % |
|---|---:|---:|
| Monster Gate | 10000 | 24.8756 |
| Mystik Wok | 10000 | 24.8756 |
| Night Assailant | 10000 | 24.8756 |
| Level Limit - Area B | 7000 | 17.4129 |
| Draining Shield | 2500 | 6.2189 |
| Zaborg the Thunder Monarch | 600 | 1.4925 |
| Enemy Controller | 100 | 0.2488 |

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

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 235 · **Weight sum:** 1815300

| Card | Weight | Pull % |
|---|---:|---:|
| 4-Starred Ladybug of Doom | 10000 | 0.5509 |
| Amazoness Archer | 10000 | 0.5509 |
| Ameba | 10000 | 0.5509 |
| Aqua Madoor | 10000 | 0.5509 |
| Armed Ninja | 10000 | 0.5509 |
| Armored Lizard | 10000 | 0.5509 |
| Armored Zombie | 10000 | 0.5509 |
| Attack and Receive | 10000 | 0.5509 |
| Backup Soldier | 10000 | 0.5509 |
| Banisher of the Light | 10000 | 0.5509 |
| Beaver Warrior | 10000 | 0.5509 |
| Big Eye | 10000 | 0.5509 |
| Blast Juggler | 10000 | 0.5509 |
| Bombardment Beetle | 10000 | 0.5509 |
| Bubonic Vermin | 10000 | 0.5509 |
| Burning Land | 10000 | 0.5509 |
| Card of Safe Return | 10000 | 0.5509 |
| Celtic Guardian | 10000 | 0.5509 |
| Ceremonial Bell | 10000 | 0.5509 |
| Chain Destruction | 10000 | 0.5509 |
| Chain Energy | 10000 | 0.5509 |
| Chorus of Sanctuary | 10000 | 0.5509 |
| Chosen One | 10000 | 0.5509 |
| Cocoon of Evolution | 10000 | 0.5509 |
| Confiscation | 10000 | 0.5509 |
| Cure Mermaid | 10000 | 0.5509 |
| Curse of Dragon | 10000 | 0.5509 |
| Curse of the Masked Beast | 10000 | 0.5509 |
| DNA Surgery | 10000 | 0.5509 |
| Dancing Fairy | 10000 | 0.5509 |
| Dark King of the Abyss | 10000 | 0.5509 |
| Darklord Marie | 10000 | 0.5509 |
| Deepsea Warrior | 10000 | 0.5509 |
| Dragon Capture Jar | 10000 | 0.5509 |
| Drill Bug | 10000 | 0.5509 |
| Electric Snake | 10000 | 0.5509 |
| Elegant Egotist | 10000 | 0.5509 |
| Enchanted Javelin | 10000 | 0.5509 |
| Eternal Rest | 10000 | 0.5509 |
| Fairy Box | 10000 | 0.5509 |
| Fairy's Hand Mirror | 10000 | 0.5509 |
| Fake Trap | 10000 | 0.5509 |
| Feral Imp | 10000 | 0.5509 |
| Final Destiny | 10000 | 0.5509 |
| Fire Princess | 10000 | 0.5509 |
| Flame Manipulator | 10000 | 0.5509 |
| Flash Assailant | 10000 | 0.5509 |
| Flying Kamakiri #1 | 10000 | 0.5509 |
| Forced Requisition | 10000 | 0.5509 |
| Gaia Power | 10000 | 0.5509 |
| Gamble | 10000 | 0.5509 |
| Giant Germ | 10000 | 0.5509 |
| Giant Soldier of Stone | 10000 | 0.5509 |
| Gift of The Mystical Elf | 10000 | 0.5509 |
| Gradius | 10000 | 0.5509 |
| Grand Tiki Elder | 10000 | 0.5509 |
| Gravekeeper's Servant | 10000 | 0.5509 |
| Graverobber | 10000 | 0.5509 |
| Gravity Bind | 10000 | 0.5509 |
| Griggle | 10000 | 0.5509 |
| Ground Collapse | 10000 | 0.5509 |
| Hane-Hane | 10000 | 0.5509 |
| Harpie Lady | 10000 | 0.5509 |
| Harpie Lady Sisters | 10000 | 0.5509 |
| Hinotama | 10000 | 0.5509 |
| Hiro's Shadow Scout | 10000 | 0.5509 |
| Hitotsu-Me Giant | 10000 | 0.5509 |
| Horn of Light | 10000 | 0.5509 |
| Horn of the Unicorn | 10000 | 0.5509 |
| Humanoid Slime | 10000 | 0.5509 |
| Humanoid Worm Drake | 10000 | 0.5509 |
| Hyozanryu | 10000 | 0.5509 |
| Hysteric Fairy | 10000 | 0.5509 |
| Infinite Cards | 10000 | 0.5509 |
| Infinite Dismissal | 10000 | 0.5509 |
| Insect Barrier | 10000 | 0.5509 |
| Invader of the Throne | 10000 | 0.5509 |
| Jam Breeding Machine | 10000 | 0.5509 |
| Jam Defender | 10000 | 0.5509 |
| Jinzo #7 | 10000 | 0.5509 |
| Karate Man | 10000 | 0.5509 |
| Kazejin | 10000 | 0.5509 |
| Kiseitai | 10000 | 0.5509 |
| Kojikocy | 10000 | 0.5509 |
| Kotodama | 10000 | 0.5509 |
| Labyrinth Wall | 10000 | 0.5509 |
| Larvae Moth | 10000 | 0.5509 |
| Left Arm of the Forbidden One | 10000 | 0.5509 |
| Left Leg of the Forbidden One | 10000 | 0.5509 |
| Light of Intervention | 10000 | 0.5509 |
| Lightforce Sword | 10000 | 0.5509 |
| Luminous Spark | 10000 | 0.5509 |
| Maha Vailo | 10000 | 0.5509 |
| Malevolent Nuzzler | 10000 | 0.5509 |
| Mammoth Graveyard | 10000 | 0.5509 |
| Manga Ryu-Ran | 10000 | 0.5509 |
| Masaki the Legendary Swordsman | 10000 | 0.5509 |
| Mask of Brutality | 10000 | 0.5509 |
| Mask of Darkness | 10000 | 0.5509 |
| Mask of Dispel | 10000 | 0.5509 |
| Mask of Restrict | 10000 | 0.5509 |
| Mask of Weakness | 10000 | 0.5509 |
| Mask of the Accursed | 10000 | 0.5509 |
| Masked Sorcerer | 10000 | 0.5509 |
| Melchid the Four-Face Beast | 10000 | 0.5509 |
| Messenger of Peace | 10000 | 0.5509 |
| Minar | 10000 | 0.5509 |
| Minor Goblin Official | 10000 | 0.5509 |
| Mirror Wall | 10000 | 0.5509 |
| Molten Destruction | 10000 | 0.5509 |
| Monster Recovery | 10000 | 0.5509 |
| Mother Grizzly | 10000 | 0.5509 |
| Mystic Lamp | 10000 | 0.5509 |
| Mystic Plasma Zone | 10000 | 0.5509 |
| Mystical Elf | 10000 | 0.5509 |
| Nobleman of Extermination | 10000 | 0.5509 |
| Numinous Healer | 10000 | 0.5509 |
| Nuvia the Wicked | 10000 | 0.5509 |
| Painful Choice | 10000 | 0.5509 |
| Parasite Paracide | 10000 | 0.5509 |
| Penguin Knight | 10000 | 0.5509 |
| Petit Moth | 10000 | 0.5509 |
| Polymerization | 10000 | 0.5509 |
| Polymerization | 10000 | 0.5509 |
| Rain of Mercy | 10000 | 0.5509 |
| Reaper of the Cards | 10000 | 0.5509 |
| Red Archery Girl | 10000 | 0.5509 |
| Red Medicine | 10000 | 0.5509 |
| Respect Play | 10000 | 0.5509 |
| Revival Jam | 10000 | 0.5509 |
| Right Arm of the Forbidden One | 10000 | 0.5509 |
| Right Leg of the Forbidden One | 10000 | 0.5509 |
| Rising Air Current | 10000 | 0.5509 |
| Rush Recklessly | 10000 | 0.5509 |
| Sanga of the Thunder | 10000 | 0.5509 |
| Scroll of Bewitchment | 10000 | 0.5509 |
| Senju of the Thousand Hands | 10000 | 0.5509 |
| Shadow of Eyes | 10000 | 0.5509 |
| Shift | 10000 | 0.5509 |
| Silver Fang | 10000 | 0.5509 |
| Skull Invitation | 10000 | 0.5509 |
| Skull Servant | 10000 | 0.5509 |
| Slot Machine | 10000 | 0.5509 |
| Solemn Wishes | 10000 | 0.5509 |
| Sonic Bird | 10000 | 0.5509 |
| Soul Release | 10000 | 0.5509 |
| Spirit of the Breeze | 10000 | 0.5509 |
| St. Joan | 10000 | 0.5509 |
| Stop Defense | 10000 | 0.5509 |
| Suijin | 10000 | 0.5509 |
| Sword Hunter | 10000 | 0.5509 |
| Tailor of the Fickle | 10000 | 0.5509 |
| The Cheerful Coffin | 10000 | 0.5509 |
| The Eye of Truth | 10000 | 0.5509 |
| The Forgiving Maiden | 10000 | 0.5509 |
| The Reliable Guardian | 10000 | 0.5509 |
| The Shallow Grave | 10000 | 0.5509 |
| Thousand-Eyes Idol | 10000 | 0.5509 |
| Time Seal | 10000 | 0.5509 |
| Toll | 10000 | 0.5509 |
| Toon Mermaid | 10000 | 0.5509 |
| Toon World | 10000 | 0.5509 |
| Tornado Wall | 10000 | 0.5509 |
| Type Zero Magic Crusher | 10000 | 0.5509 |
| UFO Turtle | 10000 | 0.5509 |
| Umi  | 10000 | 0.5509 |
| Umiiruka | 10000 | 0.5509 |
| Upstart Goblin | 10000 | 0.5509 |
| Uraby | 10000 | 0.5509 |
| Vampire Baby | 10000 | 0.5509 |
| White Magical Hat | 10000 | 0.5509 |
| Winged Dragon,Guardian of the Fortress #1 | 10000 | 0.5509 |
| Worm Drake | 10000 | 0.5509 |
| Axe of Despair | 2500 | 0.1377 |
| Black Pendant | 2500 | 0.1377 |
| Cyber Jar | 2500 | 0.1377 |
| Dust Tornado | 2500 | 0.1377 |
| Fissure | 2500 | 0.1377 |
| Flame Swordsman | 2500 | 0.1377 |
| Gaia The Fierce Knight | 2500 | 0.1377 |
| Gaia the Dragon Champion | 2500 | 0.1377 |
| Gearfried the Iron Knight | 2500 | 0.1377 |
| Giant Rat | 2500 | 0.1377 |
| Giant Trunade | 2500 | 0.1377 |
| Hayabusa Knight | 2500 | 0.1377 |
| Jar of Greed | 2500 | 0.1377 |
| Limiter Removal | 2500 | 0.1377 |
| Mad Sword Beast | 2500 | 0.1377 |
| Magic Drain | 2500 | 0.1377 |
| Magical Hats | 2500 | 0.1377 |
| Magician of Faith | 2500 | 0.1377 |
| Man-Eater Bug | 2500 | 0.1377 |
| Morphing Jar #2 | 2500 | 0.1377 |
| Mystic Tomato | 2500 | 0.1377 |
| Nimble Momonga | 2500 | 0.1377 |
| Spellbinding Circle | 2500 | 0.1377 |
| The Fiend Megacyber | 2500 | 0.1377 |
| The Forceful Sentry | 2500 | 0.1377 |
| The Legendary Fisherman | 2500 | 0.1377 |
| Toon Summoned Skull | 2500 | 0.1377 |
| Trap Hole | 2500 | 0.1377 |
| Tribute to The Doomed | 2500 | 0.1377 |
| B. Skull Dragon | 600 | 0.0331 |
| Blue-Eyes Toon Dragon | 600 | 0.0331 |
| Buster Blader | 600 | 0.0331 |
| Call Of The Haunted | 600 | 0.0331 |
| Ceasefire | 600 | 0.0331 |
| Dark Hole | 600 | 0.0331 |
| Delinquent Duo | 600 | 0.0331 |
| Exchange | 600 | 0.0331 |
| Goblin Attack Force | 600 | 0.0331 |
| Megamorph | 600 | 0.0331 |
| Mystical Space Typhoon | 600 | 0.0331 |
| Nobleman of Crossout | 600 | 0.0331 |
| Pot of Greed | 600 | 0.0331 |
| Premature Burial | 600 | 0.0331 |
| Red-Eyes B. Dragon | 600 | 0.0331 |
| Relinquished | 600 | 0.0331 |
| Summoned Skull | 600 | 0.0331 |
| Swords of Revealing Light | 600 | 0.0331 |
| The Masked Beast | 600 | 0.0331 |
| Big Shield Gardna | 100 | 0.0055 |
| Change of Heart | 100 | 0.0055 |
| Dark Magician | 100 | 0.0055 |
| Dark Sage | 100 | 0.0055 |
| Exodia the Forbidden One | 100 | 0.0055 |
| Imperial Order | 100 | 0.0055 |
| Jinzo | 100 | 0.0055 |
| Mage Power | 100 | 0.0055 |
| Monster Reborn | 100 | 0.0055 |
| Raigeki | 100 | 0.0055 |
| Snatch Steal | 100 | 0.0055 |
| Thousand-Eyes Restrict | 100 | 0.0055 |
| Torrential Tribute | 100 | 0.0055 |
| United We Stand | 100 | 0.0055 |

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

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 13 · **Weight sum:** 130000

| Card | Weight | Pull % |
|---|---:|---:|
| Call Of The Haunted | 10000 | 7.6923 |
| Ceasefire | 10000 | 7.6923 |
| Creature Swap | 10000 | 7.6923 |
| Heavy Storm | 10000 | 7.6923 |
| Interdimensional Matter Transporter | 10000 | 7.6923 |
| Masked Dragon | 10000 | 7.6923 |
| Mystical Space Typhoon | 10000 | 7.6923 |
| Nobleman of Crossout | 10000 | 7.6923 |
| Pot of Greed | 10000 | 7.6923 |
| Premature Burial | 10000 | 7.6923 |
| Red-Eyes B. Dragon | 10000 | 7.6923 |
| Snatch Steal | 10000 | 7.6923 |
| Swords of Revealing Light | 10000 | 7.6923 |

### Structure Deck- Zombie Madness

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 17 · **Weight sum:** 170000

| Card | Weight | Pull % |
|---|---:|---:|
| Book of Life | 10000 | 5.8824 |
| Card of Safe Return | 10000 | 5.8824 |
| Compulsory Evacuation Device | 10000 | 5.8824 |
| Creature Swap | 10000 | 5.8824 |
| Dust Tornado | 10000 | 5.8824 |
| Giant Trunade | 10000 | 5.8824 |
| Heavy Storm | 10000 | 5.8824 |
| Magic Jammer | 10000 | 5.8824 |
| Mystical Space Typhoon | 10000 | 5.8824 |
| Nobleman of Crossout | 10000 | 5.8824 |
| Pot of Greed | 10000 | 5.8824 |
| Pyramid Turtle | 10000 | 5.8824 |
| Ryu Kokki | 10000 | 5.8824 |
| Snatch Steal | 10000 | 5.8824 |
| Spirit Reaper | 10000 | 5.8824 |
| Torrential Tribute | 10000 | 5.8824 |
| Vampire Lord | 10000 | 5.8824 |

### Warrior

- **Type:** Usage · **Format:** Warrior · **Cards in pool:** 191 · **Weight sum:** 275667

| Card | Weight | Pull % |
|---|---:|---:|
| Axe of Despair | 3644 | 1.3219 |
| Big Bang Shot | 3644 | 1.3219 |
| Buster Blader | 3644 | 1.3219 |
| Cyber-Stein | 3644 | 1.3219 |
| Different Dimension Capsule | 3644 | 1.3219 |
| Ekibyo Drakmord | 3644 | 1.3219 |
| Exile of the Wicked | 3644 | 1.3219 |
| Fusion Gate | 3644 | 1.3219 |
| Gaia Power | 3644 | 1.3219 |
| Greenkappa | 3644 | 1.3219 |
| Jowgen the Spiritualist | 3644 | 1.3219 |
| Mother Grizzly | 3644 | 1.3219 |
| Night Assailant | 3644 | 1.3219 |
| Princess of Tsurugi | 3644 | 1.3219 |
| Pyramid Turtle | 3644 | 1.3219 |
| Revival Jam | 3644 | 1.3219 |
| Seven Tools of the Bandit | 3644 | 1.3219 |
| Slate Warrior | 3644 | 1.3219 |
| Soul Exchange | 3644 | 1.3219 |
| Stealth Bird | 3644 | 1.3219 |
| The Fiend Megacyber | 3644 | 1.3219 |
| Windstorm of Etaqua | 3644 | 1.3219 |
| Marauding Captain | 3117 | 1.1307 |
| Big Shield Gardna | 2791 | 1.0125 |
| Blast with Chain | 2791 | 1.0125 |
| Chain Disappearance | 2791 | 1.0125 |
| Cursed Seal of the Forbidden Spell | 2791 | 1.0125 |
| Dark Magician | 2791 | 1.0125 |
| Dark Scorpion - Cliff the Trap Remover | 2791 | 1.0125 |
| Des Koala | 2791 | 1.0125 |
| Exchange | 2791 | 1.0125 |
| Gearfried the Iron Knight | 2791 | 1.0125 |
| Magical Merchant | 2791 | 1.0125 |
| Newdoria | 2791 | 1.0125 |
| Nimble Momonga | 2791 | 1.0125 |
| Nobleman of Extermination | 2791 | 1.0125 |
| Trap Hole | 2791 | 1.0125 |
| Apprentice Magician | 2391 | 0.8674 |
| Magic Jammer | 2391 | 0.8674 |
| Mystic Tomato | 2391 | 0.8674 |
| Sasuke Samurai | 2391 | 0.8674 |
| Spear Dragon | 2391 | 0.8674 |
| Spell Canceller | 2391 | 0.8674 |
| Cannon Soldier | 2144 | 0.7777 |
| Gilasaurus | 2144 | 0.7777 |
| Goblin Attack Force | 2144 | 0.7777 |
| Just Desserts | 2144 | 0.7777 |
| Kuriboh | 2144 | 0.7777 |
| Last Will | 2144 | 0.7777 |
| Mask of Darkness | 2144 | 0.7777 |
| Masked Sorcerer | 2144 | 0.7777 |
| Mystic Plasma Zone | 2144 | 0.7777 |
| Ojama Trio | 2144 | 0.7777 |
| Secret Barrel | 2144 | 0.7777 |
| Waboku | 2144 | 0.7777 |
| Abyss Soldier | 1971 | 0.7150 |
| Catapult Turtle | 1971 | 0.7150 |
| Compulsory Evacuation Device | 1971 | 0.7150 |
| Gigantes | 1971 | 0.7150 |
| Sacred Crane | 1971 | 0.7150 |
| Skilled Dark Magician | 1971 | 0.7150 |
| Swarm of Locusts | 1971 | 0.7150 |
| Token Thanksgiving | 1971 | 0.7150 |
| Dekoichi the Battlechanted Locomotive | 1841 | 0.6678 |
| Dimension Fusion | 1841 | 0.6678 |
| Divine Wrath | 1841 | 0.6678 |
| Gyaku-Gire Panda | 1841 | 0.6678 |
| Magic Drain | 1841 | 0.6678 |
| Vampire Lord | 1841 | 0.6678 |
| Widespread Ruin | 1841 | 0.6678 |
| Giant Rat | 1738 | 0.6305 |
| Hallowed Life Barrier | 1738 | 0.6305 |
| Zaborg the Thunder Monarch | 1738 | 0.6305 |
| Reasoning | 1583 | 0.5742 |
| Monster Gate | 1523 | 0.5525 |
| Monster Reincarnation | 1523 | 0.5525 |
| Pikeru's Circle of Enchantment | 1523 | 0.5525 |
| Amazoness Archer | 1471 | 0.5336 |
| Return from the Different Dimension | 1424 | 0.5166 |
| Tsukuyomi | 1424 | 0.5166 |
| Jar of Greed | 1383 | 0.5017 |
| Chaos Sorcerer | 1346 | 0.4883 |
| Spirit Reaper | 1346 | 0.4883 |
| D.D. Assailant | 1313 | 0.4763 |
| Asura Priest | 1283 | 0.4654 |
| Ninja Grandmaster Sasuke | 1283 | 0.4654 |
| Zombyra the Dark | 1283 | 0.4654 |
| Berserk Gorilla | 1255 | 0.4553 |
| Giant Trunade | 1205 | 0.4371 |
| Metamorphosis | 1205 | 0.4371 |
| Raigeki Break | 1163 | 0.4219 |
| Gravekeeper's Spy | 1144 | 0.4150 |
| Solemn Judgment | 1144 | 0.4150 |
| Trap Dustshoot | 1144 | 0.4150 |
| Cyber Jar | 1109 | 0.4023 |
| King Tiger Wanghu | 1063 | 0.3856 |
| Airknight Parshath | 1049 | 0.3805 |
| Bottomless Trap Hole | 1049 | 0.3805 |
| Don Zaloog | 1049 | 0.3805 |
| Mystic Swordsman LV2 | 1012 | 0.3671 |
| Morphing Jar | 990 | 0.3591 |
| Magician of Faith | 980 | 0.3555 |
| Upstart Goblin | 961 | 0.3486 |
| Creature Swap | 947 | 0.3435 |
| Blade Knight | 934 | 0.3388 |
| Card Destruction | 934 | 0.3388 |
| Kycoo the Ghost Destroyer | 903 | 0.3276 |
| Cyber Saurus | 897 | 0.3254 |
| Fusionist | 897 | 0.3254 |
| Gaia the Dragon Champion | 897 | 0.3254 |
| Labyrinth Tank | 897 | 0.3254 |
| Rabid Horseman | 897 | 0.3254 |
| Mobius the Frost Monarch | 896 | 0.3250 |
| Smashing Ground | 896 | 0.3250 |
| Cipher Soldier | 869 | 0.3152 |
| Dust Tornado | 857 | 0.3109 |
| Sakuretsu Armor | 857 | 0.3109 |
| Ceasefire | 851 | 0.3087 |
| Injection Fairy Lily | 835 | 0.3029 |
| Magic Cylinder | 819 | 0.2971 |
| D.D. Warrior Lady | 815 | 0.2956 |
| Dark Magician of Chaos | 805 | 0.2920 |
| Reinforcement of the Army | 805 | 0.2920 |
| Dark Paladin | 682 | 0.2474 |
| Nobleman of Crossout | 681 | 0.2470 |
| Swords of Revealing Light | 662 | 0.2401 |
| Fiber Jar | 639 | 0.2318 |
| Sinister Serpent | 634 | 0.2300 |
| Exiled Force | 624 | 0.2264 |
| Jinzo | 603 | 0.2187 |
| Magical Scientist | 595 | 0.2158 |
| Charubin the Fire Knight | 582 | 0.2111 |
| Black Luster Soldier - Envoy of Beginning | 565 | 0.2050 |
| Painful Choice | 556 | 0.2017 |
| Confiscation | 550 | 0.1995 |
| Tribe-Infecting Virus | 548 | 0.1988 |
| The Forceful Sentry | 545 | 0.1977 |
| Breaker the Magical Warrior | 542 | 0.1966 |
| Torrential Tribute | 542 | 0.1966 |
| Premature Burial | 540 | 0.1959 |
| Snatch Steal | 535 | 0.1941 |
| Change of Heart | 533 | 0.1933 |
| Bell of Destruction | 528 | 0.1915 |
| Heavy Storm | 526 | 0.1908 |
| Pot of Greed | 522 | 0.1894 |
| Flame Swordsman | 520 | 0.1886 |
| Karbonala Warrior | 520 | 0.1886 |
| St. Joan | 520 | 0.1886 |
| Empress Judge | 477 | 0.1730 |
| Musician King | 418 | 0.1516 |
| Punished Eagle | 418 | 0.1516 |
| Twin-Headed Thunder Dragon | 397 | 0.1440 |
| Chain Energy | 364 | 0.1320 |
| Fairy Box | 364 | 0.1320 |
| Light of Intervention | 364 | 0.1320 |
| Mask of Restrict | 364 | 0.1320 |
| Mirror Wall | 364 | 0.1320 |
| Prohibition | 364 | 0.1320 |
| Robbin' Goblin | 364 | 0.1320 |
| Flame Ghost | 311 | 0.1128 |
| Final Attack Orders | 279 | 0.1012 |
| Poison of the Old Man | 279 | 0.1012 |
| Royal Oppression | 279 | 0.1012 |
| Master of Oz | 273 | 0.0990 |
| Roaring Ocean Snake | 269 | 0.0976 |
| Giltia the D. Knight | 260 | 0.0943 |
| Gravity Bind | 239 | 0.0867 |
| Messenger of Peace | 239 | 0.0867 |
| Wave-Motion Cannon | 239 | 0.0867 |
| The Last Warrior from Another Planet | 236 | 0.0856 |
| Dragoness the Wicked Knight | 228 | 0.0827 |
| Level Limit - Area B | 214 | 0.0776 |
| Ojama King | 212 | 0.0769 |
| Darkfire Dragon | 210 | 0.0762 |
| Dark Flare Knight | 201 | 0.0729 |
| Reaper on the Nightmare | 197 | 0.0715 |
| Xing Zhen Hu | 197 | 0.0715 |
| Fiend Skull Dragon | 187 | 0.0678 |
| Dark Blade the Dragon Knight | 179 | 0.0649 |
| Thousand-Eyes Restrict | 179 | 0.0649 |
| Ryu Senshi | 177 | 0.0642 |
| Dark Balter the Terrible | 176 | 0.0638 |
| Mystik Wok | 122 | 0.0443 |
| Enemy Controller | 120 | 0.0435 |
| Royal Decree | 114 | 0.0414 |
| Emergency Provisions | 109 | 0.0395 |
| Scapegoat | 89 | 0.0323 |
| Book of Moon | 86 | 0.0312 |
| Mirage of Nightmare | 67 | 0.0243 |
| Call Of The Haunted | 55 | 0.0200 |
| Mystical Space Typhoon | 53 | 0.0192 |

### Flaming Eternity

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 9 · **Weight sum:** 30060

| Card | Weight | Pull % |
|---|---:|---:|
| Golem Sentry | 10000 | 33.2668 |
| The Dark - Hex-Sealed Fusion | 10000 | 33.2668 |
| Threatening Roar | 10000 | 33.2668 |
| Chiron the Mage | 10 | 0.0333 |
| Gatling Dragon | 10 | 0.0333 |
| Granmarg the Rock Monarch | 10 | 0.0333 |
| King Dragun | 10 | 0.0333 |
| Lightning Vortex | 10 | 0.0333 |
| Phoenix Wing Wind Blast | 10 | 0.0333 |

### Dark Revelation Volume 1

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 34 · **Weight sum:** 211700

| Card | Weight | Pull % |
|---|---:|---:|
| Apprentice Magician | 10000 | 4.7237 |
| Different Dimension Capsule | 10000 | 4.7237 |
| Final Attack Orders | 10000 | 4.7237 |
| Gravekeeper's Spy | 10000 | 4.7237 |
| Gyaku-Gire Panda | 10000 | 4.7237 |
| Magical Merchant | 10000 | 4.7237 |
| Metamorphosis | 10000 | 4.7237 |
| My Body as a Shield | 10000 | 4.7237 |
| Ojama Trio | 10000 | 4.7237 |
| Old Vindictive Magician | 10000 | 4.7237 |
| Poison of the Old Man | 10000 | 4.7237 |
| Raigeki Break | 10000 | 4.7237 |
| Reasoning | 10000 | 4.7237 |
| Roulette Barrel | 10000 | 4.7237 |
| Sakuretsu Armor | 10000 | 4.7237 |
| Secret Barrel | 10000 | 4.7237 |
| Token Thanksgiving | 10000 | 4.7237 |
| Tsukuyomi | 10000 | 4.7237 |
| Wave-Motion Cannon | 10000 | 4.7237 |
| Big Bang Shot | 2500 | 1.1809 |
| Des Koala | 2500 | 1.1809 |
| Magical Scientist | 2500 | 1.1809 |
| Newdoria | 2500 | 1.1809 |
| Reaper on the Nightmare | 2500 | 1.1809 |
| Skilled Dark Magician | 2500 | 1.1809 |
| Spirit Reaper | 2500 | 1.1809 |
| Tribe-Infecting Virus | 2500 | 1.1809 |
| D.D. Warrior Lady | 600 | 0.2834 |
| Dark Flare Knight | 600 | 0.2834 |
| Breaker the Magical Warrior | 100 | 0.0472 |
| Dark Paladin | 100 | 0.0472 |
| Interdimensional Matter Transporter | 100 | 0.0472 |
| Nightmare Wheel | 100 | 0.0472 |
| Spell Canceller | 100 | 0.0472 |

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

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 17 · **Weight sum:** 170000

| Card | Weight | Pull % |
|---|---:|---:|
| 7 Colored Fish | 10000 | 5.8824 |
| Call Of The Haunted | 10000 | 5.8824 |
| Creature Swap | 10000 | 5.8824 |
| Dust Tornado | 10000 | 5.8824 |
| Gravity Bind | 10000 | 5.8824 |
| Heavy Storm | 10000 | 5.8824 |
| Mobius the Frost Monarch | 10000 | 5.8824 |
| Mother Grizzly | 10000 | 5.8824 |
| Mystical Space Typhoon | 10000 | 5.8824 |
| Pot of Greed | 10000 | 5.8824 |
| Premature Burial | 10000 | 5.8824 |
| Snatch Steal | 10000 | 5.8824 |
| Star Boy | 10000 | 5.8824 |
| Tornado Wall | 10000 | 5.8824 |
| Torrential Tribute | 10000 | 5.8824 |
| Tribe-Infecting Virus | 10000 | 5.8824 |
| Xing Zhen Hu | 10000 | 5.8824 |

### The Lost Millennium

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 3 · **Weight sum:** 10020

| Card | Weight | Pull % |
|---|---:|---:|
| Des Wombat | 10000 | 99.8004 |
| Brain Control | 10 | 0.0998 |
| D.D. Survivor | 10 | 0.0998 |

### Dark Beginning 2

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 143 · **Weight sum:** 960700

| Card | Weight | Pull % |
|---|---:|---:|
| Aqua Spirit | 10000 | 1.0409 |
| Asura Priest | 10000 | 1.0409 |
| Axe Raider | 10000 | 1.0409 |
| Baby Dragon | 10000 | 1.0409 |
| Battle Steer | 10000 | 1.0409 |
| Black Illusion Ritual | 10000 | 1.0409 |
| Blackland Fire Dragon | 10000 | 1.0409 |
| Bladefly | 10000 | 1.0409 |
| Blind Destruction | 10000 | 1.0409 |
| Block Attack | 10000 | 1.0409 |
| Book of Moon | 10000 | 1.0409 |
| Bottomless Trap Hole | 10000 | 1.0409 |
| Crass Clown | 10000 | 1.0409 |
| Cyclon Laser | 10000 | 1.0409 |
| Dark Spirit of the Silent | 10000 | 1.0409 |
| Dark-Piercing Light | 10000 | 1.0409 |
| De-Fusion | 10000 | 1.0409 |
| Destiny Board | 10000 | 1.0409 |
| Destruction Punch | 10000 | 1.0409 |
| Dragon Piper | 10000 | 1.0409 |
| Dragon Seeker | 10000 | 1.0409 |
| Dream Clown | 10000 | 1.0409 |
| Ekibyo Drakmord | 10000 | 1.0409 |
| Exiled Force | 10000 | 1.0409 |
| Fusion Sage | 10000 | 1.0409 |
| Gazelle the King of Mythical Beasts | 10000 | 1.0409 |
| Gemini Elf | 10000 | 1.0409 |
| Gilasaurus | 10000 | 1.0409 |
| Goddess with the Third Eye | 10000 | 1.0409 |
| Graverobber's Retribution | 10000 | 1.0409 |
| Great Moth | 10000 | 1.0409 |
| Greenkappa | 10000 | 1.0409 |
| Hoshiningen | 10000 | 1.0409 |
| Illusionist Faceless Mage | 10000 | 1.0409 |
| Jirai Gumo | 10000 | 1.0409 |
| King Tiger Wanghu | 10000 | 1.0409 |
| King of Yamimakai | 10000 | 1.0409 |
| Launcher Spider | 10000 | 1.0409 |
| Little Chimera | 10000 | 1.0409 |
| Lord of the Lamp | 10000 | 1.0409 |
| Machine King | 10000 | 1.0409 |
| Maryokutai | 10000 | 1.0409 |
| Mechanicalchaser | 10000 | 1.0409 |
| Milus Radiant | 10000 | 1.0409 |
| Miracle Dig | 10000 | 1.0409 |
| Mirage of Nightmare | 10000 | 1.0409 |
| Muka Muka | 10000 | 1.0409 |
| Mystic Horseman | 10000 | 1.0409 |
| Needle Worm | 10000 | 1.0409 |
| Princess of Tsurugi | 10000 | 1.0409 |
| Rabid Horseman | 10000 | 1.0409 |
| Restructer Revolution | 10000 | 1.0409 |
| Robbin' Goblin | 10000 | 1.0409 |
| Ryu-Kishin Powered | 10000 | 1.0409 |
| Saggi the Dark Clown | 10000 | 1.0409 |
| Shadow Ghoul | 10000 | 1.0409 |
| Share the Pain | 10000 | 1.0409 |
| Shield & Sword | 10000 | 1.0409 |
| Skull Lair | 10000 | 1.0409 |
| Soul of Purity and Light | 10000 | 1.0409 |
| Soul of the Pure | 10000 | 1.0409 |
| Spirit Message "A" | 10000 | 1.0409 |
| Spirit Message "I" | 10000 | 1.0409 |
| Spirit Message "L" | 10000 | 1.0409 |
| Spirit Message "N" | 10000 | 1.0409 |
| Star Boy | 10000 | 1.0409 |
| Swarm of Locusts | 10000 | 1.0409 |
| The Bistro Butcher | 10000 | 1.0409 |
| The Dark Door | 10000 | 1.0409 |
| The Emperor's Holiday | 10000 | 1.0409 |
| The Immortal of Thunder | 10000 | 1.0409 |
| The Rock Spirit | 10000 | 1.0409 |
| The Unhappy Maiden | 10000 | 1.0409 |
| The Wicked Worm Beast | 10000 | 1.0409 |
| Thousand Dragon | 10000 | 1.0409 |
| Thunder Dragon | 10000 | 1.0409 |
| Tiger Axe | 10000 | 1.0409 |
| Toon Cannon Soldier | 10000 | 1.0409 |
| Toon Table of Contents | 10000 | 1.0409 |
| Tornado Bird | 10000 | 1.0409 |
| Trap Dustshoot | 10000 | 1.0409 |
| Tremendous Fire | 10000 | 1.0409 |
| Twin-Headed Thunder Dragon | 10000 | 1.0409 |
| Two-Headed King Rex | 10000 | 1.0409 |
| Vengeful Bog Spirit | 10000 | 1.0409 |
| Witch's Apprentice | 10000 | 1.0409 |
| Yata-Garasu | 10000 | 1.0409 |
| Zombyra the Dark | 10000 | 1.0409 |
| Bazoo the Soul-Eater | 2500 | 0.2602 |
| Blast with Chain | 2500 | 0.2602 |
| Cyber-Stein | 2500 | 0.2602 |
| Dark Balter the Terrible | 2500 | 0.2602 |
| Emergency Provisions | 2500 | 0.2602 |
| Fiber Jar | 2500 | 0.2602 |
| Fiend Skull Dragon | 2500 | 0.2602 |
| Fusion Gate | 2500 | 0.2602 |
| Graceful Charity | 2500 | 0.2602 |
| Horn of Heaven | 2500 | 0.2602 |
| Jowgen the Spiritualist | 2500 | 0.2602 |
| Kuriboh | 2500 | 0.2602 |
| Kycoo the Ghost Destroyer | 2500 | 0.2602 |
| Marauding Captain | 2500 | 0.2602 |
| Morphing Jar | 2500 | 0.2602 |
| Penguin Soldier | 2500 | 0.2602 |
| Pyramid Turtle | 2500 | 0.2602 |
| Reinforcement of the Army | 2500 | 0.2602 |
| Ring of Magnetism | 2500 | 0.2602 |
| Riryoku Field | 2500 | 0.2602 |
| Royal Command | 2500 | 0.2602 |
| Royal Oppression | 2500 | 0.2602 |
| Ryu Senshi | 2500 | 0.2602 |
| Sangan | 2500 | 0.2602 |
| Sasuke Samurai | 2500 | 0.2602 |
| Spiritualism | 2500 | 0.2602 |
| Thunder Nyan Nyan | 2500 | 0.2602 |
| Witch of the Black Forest | 2500 | 0.2602 |
| Airknight Parshath | 600 | 0.0625 |
| Book of Life | 600 | 0.0625 |
| Cannon Soldier | 600 | 0.0625 |
| Catapult Turtle | 600 | 0.0625 |
| Creature Swap | 600 | 0.0625 |
| Don Zaloog | 600 | 0.0625 |
| Heavy Storm | 600 | 0.0625 |
| Magic Jammer | 600 | 0.0625 |
| Mirror Force | 600 | 0.0625 |
| Royal Decree | 600 | 0.0625 |
| Seven Tools of the Bandit | 600 | 0.0625 |
| Solemn Judgment | 600 | 0.0625 |
| Spear Dragon | 600 | 0.0625 |
| The Last Warrior from Another Planet | 600 | 0.0625 |
| Time Wizard | 600 | 0.0625 |
| Total Defense Shogun | 600 | 0.0625 |
| Barrel Dragon | 100 | 0.0104 |
| Blade Knight | 100 | 0.0104 |
| Dark Necrofear | 100 | 0.0104 |
| Dark Ruler Ha Des | 100 | 0.0104 |
| Guardian Sphinx | 100 | 0.0104 |
| Injection Fairy Lily | 100 | 0.0104 |
| Insect Queen | 100 | 0.0104 |
| Obnoxious Celtic Guard | 100 | 0.0104 |
| Perfectly Ultimate Great Moth | 100 | 0.0104 |
| Swift Gaia the Fierce Knight | 100 | 0.0104 |
| Vampire Lord | 100 | 0.0104 |

### Dark Revelation Volume 2

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 22 · **Weight sum:** 90800

| Card | Weight | Pull % |
|---|---:|---:|
| Chaos Sorcerer | 10000 | 11.0132 |
| Cursed Seal of the Forbidden Spell | 10000 | 11.0132 |
| Gigantes | 10000 | 11.0132 |
| Monster Gate | 10000 | 11.0132 |
| Mystik Wok | 10000 | 11.0132 |
| Sacred Crane | 10000 | 11.0132 |
| Smashing Ground | 10000 | 11.0132 |
| Berserk Gorilla | 2500 | 2.7533 |
| Chain Disappearance | 2500 | 2.7533 |
| Compulsory Evacuation Device | 2500 | 2.7533 |
| Draining Shield | 2500 | 2.7533 |
| Level Limit - Area B | 2500 | 2.7533 |
| Night Assailant | 2500 | 2.7533 |
| Stealth Bird | 2500 | 2.7533 |
| Dimension Fusion | 600 | 0.6608 |
| Enemy Controller | 600 | 0.6608 |
| Manticore of Darkness | 600 | 0.6608 |
| Ryu Kokki | 600 | 0.6608 |
| Zaborg the Thunder Monarch | 600 | 0.6608 |
| Black Luster Soldier - Envoy of Beginning | 100 | 0.1101 |
| Chaos Emperor Dragon - Envoy of the End | 100 | 0.1101 |
| Dark Magician of Chaos | 100 | 0.1101 |

### Structure Deck- Warrior's Triumph

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 19 · **Weight sum:** 190000

| Card | Weight | Pull % |
|---|---:|---:|
| Blast with Chain | 10000 | 5.2632 |
| Call Of The Haunted | 10000 | 5.2632 |
| D.D. Warrior Lady | 10000 | 5.2632 |
| Exiled Force | 10000 | 5.2632 |
| Gearfried the Iron Knight | 10000 | 5.2632 |
| Giant Trunade | 10000 | 5.2632 |
| Goblin Attack Force | 10000 | 5.2632 |
| Heavy Storm | 10000 | 5.2632 |
| Lightning Vortex | 10000 | 5.2632 |
| Magic Jammer | 10000 | 5.2632 |
| Marauding Captain | 10000 | 5.2632 |
| Mystic Swordsman LV2 | 10000 | 5.2632 |
| Mystical Space Typhoon | 10000 | 5.2632 |
| Ninja Grandmaster Sasuke | 10000 | 5.2632 |
| Obnoxious Celtic Guard | 10000 | 5.2632 |
| Reinforcement of the Army | 10000 | 5.2632 |
| Royal Decree | 10000 | 5.2632 |
| Snatch Steal | 10000 | 5.2632 |
| Swift Gaia the Fierce Knight | 10000 | 5.2632 |

### Goat

- **Type:** Usage · **Format:** Goat · **Cards in pool:** 282 · **Weight sum:** 968934

| Card | Weight | Pull % |
|---|---:|---:|
| Aqua Spirit | 13242 | 1.3667 |
| Backup Soldier | 13242 | 1.3667 |
| Catapult Turtle | 13242 | 1.3667 |
| Dark Magician | 13242 | 1.3667 |
| Dark Necrofear | 13242 | 1.3667 |
| Dark Ruler Ha Des | 13242 | 1.3667 |
| Dragon Zombie | 13242 | 1.3667 |
| Ekibyo Drakmord | 13242 | 1.3667 |
| Fire Princess | 13242 | 1.3667 |
| Gaia Power | 13242 | 1.3667 |
| Giant Germ | 13242 | 1.3667 |
| Graverobber | 13242 | 1.3667 |
| Hiro's Shadow Scout | 13242 | 1.3667 |
| Interdimensional Matter Transporter | 13242 | 1.3667 |
| Kuwagata a | 13242 | 1.3667 |
| Lightforce Sword | 13242 | 1.3667 |
| Mask of Brutality | 13242 | 1.3667 |
| Minar | 13242 | 1.3667 |
| Restructer Revolution | 13242 | 1.3667 |
| Sonic Bird | 13242 | 1.3667 |
| The Forgiving Maiden | 13242 | 1.3667 |
| Thunder Nyan Nyan | 13242 | 1.3667 |
| Two-Pronged Attack | 13242 | 1.3667 |
| Giant Soldier of Stone | 10065 | 1.0388 |
| Gilasaurus | 10065 | 1.0388 |
| Guardian Sphinx | 10065 | 1.0388 |
| Mother Grizzly | 10065 | 1.0388 |
| Pikeru's Circle of Enchantment | 10065 | 1.0388 |
| Slate Warrior | 10065 | 1.0388 |
| The Fiend Megacyber | 10065 | 1.0388 |
| Different Dimension Capsule | 8576 | 0.8851 |
| Dream Clown | 8576 | 0.8851 |
| Exodia the Forbidden One | 8576 | 0.8851 |
| Left Arm of the Forbidden One | 8576 | 0.8851 |
| Left Leg of the Forbidden One | 8576 | 0.8851 |
| Right Arm of the Forbidden One | 8576 | 0.8851 |
| Right Leg of the Forbidden One | 8576 | 0.8851 |
| The Little Swordsman of Aile | 8576 | 0.8851 |
| The Wicked Worm Beast | 8576 | 0.8851 |
| Windstorm of Etaqua | 8576 | 0.8851 |
| D.D. Survivor | 7657 | 0.7902 |
| Spear Dragon | 7657 | 0.7902 |
| Time Wizard | 7657 | 0.7902 |
| Marauding Captain | 7310 | 0.7544 |
| Axe of Despair | 7013 | 0.7238 |
| Black Pendant | 7013 | 0.7238 |
| Morphing Jar #2 | 7013 | 0.7238 |
| Summoner of Illusions | 7013 | 0.7238 |
| The Shallow Grave | 7013 | 0.7238 |
| Trap Hole | 7013 | 0.7238 |
| Acid Trap Hole | 6528 | 0.6737 |
| Des Lacooda | 6528 | 0.6737 |
| Exchange | 6528 | 0.6737 |
| Nimble Momonga | 6528 | 0.6737 |
| Trap Master | 6528 | 0.6737 |
| Tribute to The Doomed | 6528 | 0.6737 |
| Electric Snake | 6145 | 0.6342 |
| Penguin Soldier | 6145 | 0.6342 |
| Shield & Sword | 6145 | 0.6342 |
| Stealth Bird | 6145 | 0.6342 |
| Banisher of the Light | 5832 | 0.6019 |
| The Rock Spirit | 5832 | 0.6019 |
| Fake Trap | 5569 | 0.5748 |
| Old Vindictive Magician | 5569 | 0.5748 |
| Thestalos the Firestorm Monarch | 5569 | 0.5748 |
| Blast with Chain | 5344 | 0.5515 |
| Divine Wrath | 5344 | 0.5515 |
| Gearfried the Iron Knight | 5344 | 0.5515 |
| Senju of the Thousand Hands | 5149 | 0.5314 |
| Swift Gaia the Fierce Knight | 4824 | 0.4979 |
| Swarm of Locusts | 4686 | 0.4836 |
| Book of Life | 4345 | 0.4484 |
| Granmarg the Rock Monarch | 4345 | 0.4484 |
| Greenkappa | 4345 | 0.4484 |
| Jirai Gumo | 4345 | 0.4484 |
| Black Illusion Ritual | 4250 | 0.4386 |
| Magic Drain | 4250 | 0.4386 |
| Relinquished | 4250 | 0.4386 |
| Ryu Kokki | 4250 | 0.4386 |
| Chain Disappearance | 4079 | 0.4210 |
| Cold Wave | 4003 | 0.4131 |
| Megamorph | 3864 | 0.3988 |
| Golem Sentry | 3801 | 0.3923 |
| Pyramid Turtle | 3741 | 0.3861 |
| Toon Cannon Soldier | 3741 | 0.3861 |
| Vampire Lord | 3712 | 0.3831 |
| Soul Exchange | 3685 | 0.3803 |
| Cannon Soldier | 3631 | 0.3747 |
| Milus Radiant | 3532 | 0.3645 |
| Time Seal | 3532 | 0.3645 |
| Toon Table of Contents | 3401 | 0.3510 |
| Bazoo the Soul-Eater | 3360 | 0.3468 |
| Big Bang Shot | 3322 | 0.3429 |
| Flower Wolf | 3308 | 0.3414 |
| Nobleman of Extermination | 3216 | 0.3319 |
| Apprentice Magician | 3091 | 0.3190 |
| Chiron the Mage | 3091 | 0.3190 |
| Soul Release | 3091 | 0.3190 |
| Mage Power | 3062 | 0.3160 |
| Token Thanksgiving | 3008 | 0.3104 |
| Roulette Barrel | 2887 | 0.2980 |
| Newdoria | 2864 | 0.2956 |
| Waboku | 2864 | 0.2956 |
| Hallowed Life Barrier | 2843 | 0.2934 |
| Big Shield Gardna | 2822 | 0.2912 |
| Gigantes | 2822 | 0.2912 |
| Cursed Seal of the Forbidden Spell | 2801 | 0.2891 |
| Monster Reincarnation | 2801 | 0.2891 |
| Hysteric Fairy | 2724 | 0.2811 |
| Des Koala | 2706 | 0.2793 |
| Goblin Attack Force | 2688 | 0.2774 |
| Mask of Darkness | 2688 | 0.2774 |
| Dark Mimic LV1 | 2671 | 0.2757 |
| Sacred Crane | 2671 | 0.2757 |
| Dimension Fusion | 2638 | 0.2723 |
| Last Will | 2638 | 0.2723 |
| Magic Jammer | 2591 | 0.2674 |
| Monster Gate | 2547 | 0.2629 |
| Reasoning | 2547 | 0.2629 |
| Kaminari Attack | 2510 | 0.2590 |
| Seven Tools of the Bandit | 2372 | 0.2448 |
| Spell Canceller | 2217 | 0.2288 |
| Rabid Horseman | 2136 | 0.2204 |
| Just Desserts | 2101 | 0.2168 |
| Return from the Different Dimension | 2101 | 0.2168 |
| Ojama Trio | 2053 | 0.2119 |
| Jowgen the Spiritualist | 2028 | 0.2093 |
| Magical Merchant | 2010 | 0.2074 |
| Secret Barrel | 1993 | 0.2057 |
| Skilled Dark Magician | 1987 | 0.2051 |
| Giant Rat | 1981 | 0.2045 |
| Bottomless Trap Hole | 1933 | 0.1995 |
| Cyber-Stein | 1933 | 0.1995 |
| Gyaku-Gire Panda | 1888 | 0.1949 |
| Airknight Parshath | 1870 | 0.1930 |
| Compulsory Evacuation Device | 1865 | 0.1925 |
| Des Wombat | 1861 | 0.1921 |
| Abyss Soldier | 1810 | 0.1868 |
| D.D. Assailant | 1810 | 0.1868 |
| Mystic Tomato | 1798 | 0.1856 |
| Threatening Roar | 1775 | 0.1832 |
| Kwagar Hercules | 1743 | 0.1799 |
| Dark Scorpion - Cliff the Trap Remover | 1706 | 0.1761 |
| Kuriboh | 1706 | 0.1761 |
| Amazoness Archer | 1693 | 0.1747 |
| Zaborg the Thunder Monarch | 1648 | 0.1701 |
| United We Stand | 1600 | 0.1651 |
| Widespread Ruin | 1580 | 0.1631 |
| Brain Control | 1537 | 0.1586 |
| Giant Trunade | 1494 | 0.1542 |
| Berserk Gorilla | 1473 | 0.1520 |
| Empress Judge | 1446 | 0.1492 |
| Phoenix Wing Wind Blast | 1434 | 0.1480 |
| Magic Cylinder | 1379 | 0.1423 |
| Spirit Reaper | 1355 | 0.1398 |
| King Tiger Wanghu | 1334 | 0.1377 |
| Smashing Ground | 1327 | 0.1370 |
| Card of Safe Return | 1324 | 0.1366 |
| Spirit Message "A" | 1324 | 0.1366 |
| Spirit Message "L" | 1324 | 0.1366 |
| Spirit Message "N" | 1324 | 0.1366 |
| Ultimate Offering | 1324 | 0.1366 |
| Dark Magician of Chaos | 1316 | 0.1358 |
| Creature Swap | 1264 | 0.1305 |
| Don Zaloog | 1230 | 0.1269 |
| Ninja Grandmaster Sasuke | 1220 | 0.1259 |
| Skull Knight | 1193 | 0.1231 |
| Warrior of Tradition | 1193 | 0.1231 |
| Ceasefire | 1161 | 0.1198 |
| Deepsea Shark | 1159 | 0.1196 |
| Metal Dragon | 1159 | 0.1196 |
| Cipher Soldier | 1154 | 0.1191 |
| Metamorphosis | 1151 | 0.1188 |
| Mystic Swordsman LV2 | 1115 | 0.1151 |
| Gaia the Dragon Champion | 1099 | 0.1134 |
| Lightning Vortex | 1098 | 0.1133 |
| Injection Fairy Lily | 1089 | 0.1124 |
| Humanoid Worm Drake | 1073 | 0.1107 |
| Labyrinth Tank | 1073 | 0.1107 |
| Blade Knight | 1068 | 0.1102 |
| Cyber Jar | 1056 | 0.1090 |
| Asura Priest | 1051 | 0.1085 |
| Raigeki Break | 1032 | 0.1065 |
| Rush Recklessly | 1006 | 0.1038 |
| Jar of Greed | 997 | 0.1029 |
| Zombyra the Dark | 996 | 0.1028 |
| Mobius the Frost Monarch | 964 | 0.0995 |
| Jinzo | 961 | 0.0992 |
| Thunder Dragon | 940 | 0.0970 |
| Reinforcement of the Army | 938 | 0.0968 |
| Dekoichi the Battlechanted Locomotive | 927 | 0.0957 |
| Night Assailant | 925 | 0.0955 |
| Morphing Jar | 914 | 0.0943 |
| Solemn Judgment | 910 | 0.0939 |
| Dust Tornado | 903 | 0.0932 |
| Kycoo the Ghost Destroyer | 888 | 0.0916 |
| Tsukuyomi | 884 | 0.0912 |
| Chaos Sorcerer | 883 | 0.0911 |
| Gravekeeper's Spy | 880 | 0.0908 |
| Offerings to the Doomed | 857 | 0.0884 |
| Mind Control | 854 | 0.0881 |
| Upstart Goblin | 847 | 0.0874 |
| Flame Swordsman | 836 | 0.0863 |
| Musician King | 836 | 0.0863 |
| Magician of Faith | 832 | 0.0859 |
| Trap Dustshoot | 829 | 0.0856 |
| Sakuretsu Armor | 823 | 0.0849 |
| Fusionist | 789 | 0.0814 |
| Mask of Restrict | 765 | 0.0790 |
| Mirror Wall | 765 | 0.0790 |
| Bickuribox | 744 | 0.0768 |
| Card Destruction | 738 | 0.0762 |
| Cyber Saurus | 737 | 0.0761 |
| Swords of Revealing Light | 713 | 0.0736 |
| St. Joan | 712 | 0.0735 |
| Exiled Force | 704 | 0.0727 |
| Robbin' Goblin | 701 | 0.0723 |
| Solemn Wishes | 701 | 0.0723 |
| Dark Paladin | 690 | 0.0712 |
| Roaring Ocean Snake | 680 | 0.0702 |
| D.D. Warrior Lady | 679 | 0.0701 |
| Premature Burial | 663 | 0.0684 |
| Nobleman of Crossout | 662 | 0.0683 |
| Charubin the Fire Knight | 657 | 0.0678 |
| Toon World | 652 | 0.0673 |
| Thousand Dragon | 640 | 0.0661 |
| B. Skull Dragon | 636 | 0.0656 |
| Karbonala Warrior | 628 | 0.0648 |
| Final Attack Orders | 614 | 0.0634 |
| Limiter Removal | 614 | 0.0634 |
| Sinister Serpent | 603 | 0.0622 |
| Prohibition | 583 | 0.0602 |
| Sangan | 572 | 0.0590 |
| Torrential Tribute | 557 | 0.0575 |
| Light of Intervention | 556 | 0.0574 |
| Tribe-Infecting Virus | 544 | 0.0561 |
| Black Luster Soldier - Envoy of Beginning | 543 | 0.0560 |
| Delinquent Duo | 537 | 0.0554 |
| Skull Lair | 534 | 0.0551 |
| Breaker the Magical Warrior | 532 | 0.0549 |
| Mirror Force | 529 | 0.0546 |
| Bell of Destruction | 526 | 0.0543 |
| Snatch Steal | 526 | 0.0543 |
| Heavy Storm | 524 | 0.0541 |
| Graceful Charity | 522 | 0.0539 |
| Pot of Greed | 522 | 0.0539 |
| Royal Command | 482 | 0.0497 |
| Emergency Provisions | 475 | 0.0490 |
| Dark Flare Knight | 424 | 0.0438 |
| Giltia the D. Knight | 419 | 0.0432 |
| Royal Oppression | 358 | 0.0369 |
| Poison of the Old Man | 344 | 0.0355 |
| My Body as a Shield | 332 | 0.0343 |
| Twin-Headed Thunder Dragon | 330 | 0.0341 |
| Chain Energy | 328 | 0.0339 |
| Flame Ghost | 276 | 0.0285 |
| Messenger of Peace | 270 | 0.0279 |
| Wave-Motion Cannon | 257 | 0.0265 |
| Enemy Controller | 241 | 0.0249 |
| Nightmare Wheel | 239 | 0.0247 |
| Master of Oz | 203 | 0.0210 |
| Dragoness the Wicked Knight | 202 | 0.0208 |
| Ojama King | 194 | 0.0200 |
| King Dragun | 191 | 0.0197 |
| The Last Warrior from Another Planet | 191 | 0.0197 |
| Darkfire Dragon | 187 | 0.0193 |
| Fiend Skull Dragon | 185 | 0.0191 |
| Gravity Bind | 184 | 0.0190 |
| Reaper on the Nightmare | 184 | 0.0190 |
| Dark Blade the Dragon Knight | 183 | 0.0189 |
| Gatling Dragon | 179 | 0.0185 |
| Ryu Senshi | 178 | 0.0184 |
| Dark Balter the Terrible | 175 | 0.0181 |
| Thousand-Eyes Restrict | 173 | 0.0179 |
| Level Limit - Area B | 167 | 0.0172 |
| Xing Zhen Hu | 144 | 0.0149 |
| Royal Decree | 128 | 0.0132 |
| Scapegoat | 110 | 0.0114 |
| Mystik Wok | 96 | 0.0099 |
| Book of Moon | 83 | 0.0086 |
| Call Of The Haunted | 70 | 0.0072 |
| Mystical Space Typhoon | 53 | 0.0055 |

### Cybernetic Revolution

- **Type:** TCG · **Format:** Cyber · **Cards in pool:** 6 · **Weight sum:** 10050

| Card | Weight | Pull % |
|---|---:|---:|
| Drillroid | 10000 | 99.5025 |
| Cyber Dragon | 10 | 0.0995 |
| Cyber End Dragon | 10 | 0.0995 |
| Cyber Twin Dragon | 10 | 0.0995 |
| Dimension Wall | 10 | 0.0995 |
| Goblin Elite Attack Force | 10 | 0.0995 |

### Cyber

- **Type:** Usage · **Format:** Cyber · **Cards in pool:** 142 · **Weight sum:** 164290

| Card | Weight | Pull % |
|---|---:|---:|
| Amazoness Archer | 2978 | 1.8126 |
| Apprentice Magician | 2978 | 1.8126 |
| Cannon Soldier | 2978 | 1.8126 |
| Dark Scorpion - Cliff the Trap Remover | 2978 | 1.8126 |
| Different Dimension Capsule | 2978 | 1.8126 |
| Greenkappa | 2978 | 1.8126 |
| Return from the Different Dimension | 2978 | 1.8126 |
| Sasuke Samurai | 2978 | 1.8126 |
| Soul Exchange | 2978 | 1.8126 |
| Stealth Bird | 2978 | 1.8126 |
| Thestalos the Firestorm Monarch | 2978 | 1.8126 |
| Thunder Dragon | 2978 | 1.8126 |
| Upstart Goblin | 2550 | 1.5521 |
| Des Wombat | 2286 | 1.3914 |
| Electric Snake | 2286 | 1.3914 |
| Jar of Greed | 2286 | 1.3914 |
| Jowgen the Spiritualist | 2286 | 1.3914 |
| Monster Gate | 2286 | 1.3914 |
| Monster Reincarnation | 2286 | 1.3914 |
| Raigeki Break | 2286 | 1.3914 |
| Reasoning | 2286 | 1.3914 |
| Sacred Crane | 2286 | 1.3914 |
| Abyss Soldier | 1962 | 1.1942 |
| Card Destruction | 1962 | 1.1942 |
| Chain Disappearance | 1962 | 1.1942 |
| Giant Rat | 1962 | 1.1942 |
| Gyaku-Gire Panda | 1962 | 1.1942 |
| Spell Canceller | 1962 | 1.1942 |
| Token Thanksgiving | 1962 | 1.1942 |
| Widespread Ruin | 1962 | 1.1942 |
| Blade Knight | 1761 | 1.0719 |
| Des Koala | 1761 | 1.0719 |
| Just Desserts | 1761 | 1.0719 |
| Magical Merchant | 1761 | 1.0719 |
| Nimble Momonga | 1761 | 1.0719 |
| Ojama Trio | 1761 | 1.0719 |
| Secret Barrel | 1761 | 1.0719 |
| Big Shield Gardna | 1621 | 0.9867 |
| Chiron the Mage | 1621 | 0.9867 |
| Cursed Seal of the Forbidden Spell | 1621 | 0.9867 |
| Megamorph | 1621 | 0.9867 |
| Ninja Grandmaster Sasuke | 1621 | 0.9867 |
| Dark Magician of Chaos | 1516 | 0.9228 |
| Don Zaloog | 1516 | 0.9228 |
| United We Stand | 1516 | 0.9228 |
| Airknight Parshath | 1364 | 0.8302 |
| D.D. Assailant | 1364 | 0.8302 |
| Dekoichi the Battlechanted Locomotive | 1364 | 0.8302 |
| Kycoo the Ghost Destroyer | 1364 | 0.8302 |
| Solemn Judgment | 1364 | 0.8302 |
| Waboku | 1364 | 0.8302 |
| Cipher Soldier | 1307 | 0.7955 |
| Reinforcement of the Army | 1307 | 0.7955 |
| Smashing Ground | 1307 | 0.7955 |
| Cold Wave | 1258 | 0.7657 |
| Creature Swap | 1236 | 0.7523 |
| Giant Trunade | 1215 | 0.7395 |
| Mystic Swordsman LV2 | 1215 | 0.7395 |
| Asura Priest | 1114 | 0.6781 |
| Gravekeeper's Spy | 1114 | 0.6781 |
| Mind Control | 1114 | 0.6781 |
| Chaos Sorcerer | 1087 | 0.6616 |
| Injection Fairy Lily | 1087 | 0.6616 |
| Magic Cylinder | 1087 | 0.6616 |
| Mystic Tomato | 1063 | 0.6470 |
| Bottomless Trap Hole | 1040 | 0.6330 |
| Zaborg the Thunder Monarch | 1040 | 0.6330 |
| Cyber Jar | 1019 | 0.6202 |
| Exarion Universe | 1019 | 0.6202 |
| Tsukuyomi | 1019 | 0.6202 |
| Dust Tornado | 1000 | 0.6087 |
| Last Will | 1000 | 0.6087 |
| King Tiger Wanghu | 966 | 0.5880 |
| Kuriboh | 966 | 0.5880 |
| Metamorphosis | 950 | 0.5782 |
| Brain Control | 909 | 0.5533 |
| Threatening Roar | 885 | 0.5387 |
| Sakuretsu Armor | 874 | 0.5320 |
| Cyber-Stein | 863 | 0.5253 |
| Spirit Reaper | 863 | 0.5253 |
| Ceasefire | 853 | 0.5192 |
| Trap Dustshoot | 853 | 0.5192 |
| Mobius the Frost Monarch | 844 | 0.5137 |
| Morphing Jar | 826 | 0.5028 |
| Magician of Faith | 809 | 0.4924 |
| Cyber Dragon | 787 | 0.4790 |
| Swords of Revealing Light | 761 | 0.4632 |
| Karbonala Warrior | 729 | 0.4437 |
| Roaring Ocean Snake | 729 | 0.4437 |
| Twin-Headed Thunder Dragon | 729 | 0.4437 |
| Lightning Vortex | 713 | 0.4340 |
| Nobleman of Crossout | 665 | 0.4048 |
| Sinister Serpent | 665 | 0.4048 |
| Jinzo | 655 | 0.3987 |
| Exiled Force | 613 | 0.3731 |
| D.D. Warrior Lady | 592 | 0.3603 |
| Sangan | 580 | 0.3530 |
| Tribe-Infecting Virus | 580 | 0.3530 |
| Flame Ghost | 556 | 0.3384 |
| Fusionist | 556 | 0.3384 |
| Giltia the D. Knight | 556 | 0.3384 |
| Breaker the Magical Warrior | 554 | 0.3372 |
| Premature Burial | 550 | 0.3348 |
| Black Luster Soldier - Envoy of Beginning | 545 | 0.3317 |
| Delinquent Duo | 541 | 0.3293 |
| Bell of Destruction | 529 | 0.3220 |
| Snatch Steal | 529 | 0.3220 |
| Torrential Tribute | 529 | 0.3220 |
| Mirror Force | 525 | 0.3196 |
| Graceful Charity | 522 | 0.3177 |
| Heavy Storm | 522 | 0.3177 |
| Pot of Greed | 522 | 0.3177 |
| Gaia the Dragon Champion | 341 | 0.2076 |
| Dark Flare Knight | 297 | 0.1808 |
| Xing Zhen Hu | 297 | 0.1808 |
| Dragoness the Wicked Knight | 228 | 0.1388 |
| Poison of the Old Man | 228 | 0.1388 |
| Darkfire Dragon | 207 | 0.1260 |
| Ojama King | 204 | 0.1242 |
| Master of Oz | 201 | 0.1223 |
| Cyber End Dragon | 181 | 0.1102 |
| King Dragun | 177 | 0.1077 |
| Ryu Senshi | 177 | 0.1077 |
| The Last Warrior from Another Planet | 177 | 0.1077 |
| Dark Blade the Dragon Knight | 176 | 0.1071 |
| Reaper on the Nightmare | 174 | 0.1059 |
| Cyber Twin Dragon | 171 | 0.1041 |
| Fiend Skull Dragon | 171 | 0.1041 |
| Gatling Dragon | 171 | 0.1041 |
| Dark Balter the Terrible | 170 | 0.1035 |
| Thousand-Eyes Restrict | 170 | 0.1035 |
| Wave-Motion Cannon | 162 | 0.0986 |
| Enemy Controller | 151 | 0.0919 |
| Level Limit - Area B | 151 | 0.0919 |
| Gravity Bind | 139 | 0.0846 |
| Limiter Removal | 117 | 0.0712 |
| Mystik Wok | 104 | 0.0633 |
| Scapegoat | 100 | 0.0609 |
| Royal Decree | 93 | 0.0566 |
| Book of Moon | 80 | 0.0487 |
| Call Of The Haunted | 59 | 0.0359 |
| Mystical Space Typhoon | 53 | 0.0323 |

### Elemental Energy

- **Type:** TCG · **Format:** Reaper · **Cards in pool:** 7 · **Weight sum:** 30040

| Card | Weight | Pull % |
|---|---:|---:|
| Dark Deal | 10000 | 33.2889 |
| Dark World Lightning | 10000 | 33.2889 |
| Hydrogeddon | 10000 | 33.2889 |
| Cyber Blader | 10 | 0.0333 |
| Goldd, Wu-Lord of Dark World | 10 | 0.0333 |
| Pot of Avarice | 10 | 0.0333 |
| Sillva, Warlord of Dark World | 10 | 0.0333 |

### Structure Deck- Spellcaster's Judgment

- **Type:** TCG · **Format:** Reaper · **Cards in pool:** 20 · **Weight sum:** 200000

| Card | Weight | Pull % |
|---|---:|---:|
| Apprentice Magician | 10000 | 5.0000 |
| Breaker the Magical Warrior | 10000 | 5.0000 |
| Call Of The Haunted | 10000 | 5.0000 |
| Chaos Sorcerer | 10000 | 5.0000 |
| Dark Magician | 10000 | 5.0000 |
| Divine Wrath | 10000 | 5.0000 |
| Gemini Elf | 10000 | 5.0000 |
| Heavy Storm | 10000 | 5.0000 |
| Lightning Vortex | 10000 | 5.0000 |
| Mage Power | 10000 | 5.0000 |
| Magic Cylinder | 10000 | 5.0000 |
| Magician of Faith | 10000 | 5.0000 |
| Mystic Box | 10000 | 5.0000 |
| Mystical Space Typhoon | 10000 | 5.0000 |
| Nightmare's Steelcage | 10000 | 5.0000 |
| Nobleman of Crossout | 10000 | 5.0000 |
| Premature Burial | 10000 | 5.0000 |
| Skilled Dark Magician | 10000 | 5.0000 |
| Swords of Revealing Light | 10000 | 5.0000 |
| Tsukuyomi | 10000 | 5.0000 |

### Shadow of Infinity

- **Type:** TCG · **Format:** Reaper · **Cards in pool:** 3 · **Weight sum:** 10020

| Card | Weight | Pull % |
|---|---:|---:|
| Sand Moth | 10000 | 99.8004 |
| Karma Cut | 10 | 0.0998 |
| Treeborn Frog | 10 | 0.0998 |

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

- **Type:** Usage · **Format:** Reaper · **Cards in pool:** 232 · **Weight sum:** 437871

| Card | Weight | Pull % |
|---|---:|---:|
| Acid Trap Hole | 5422 | 1.2383 |
| Asura Priest | 5422 | 1.2383 |
| Bait Doll | 5422 | 1.2383 |
| Dark Deal | 5422 | 1.2383 |
| Dark Necrofear | 5422 | 1.2383 |
| Dark World Lightning | 5422 | 1.2383 |
| De-Spell | 5422 | 1.2383 |
| Dimension Wall | 5422 | 1.2383 |
| Draining Shield | 5422 | 1.2383 |
| Giant Germ | 5422 | 1.2383 |
| Karma Cut | 5422 | 1.2383 |
| Magic Drain | 5422 | 1.2383 |
| Marauding Captain | 5422 | 1.2383 |
| Masked Dragon | 5422 | 1.2383 |
| Reasoning | 5422 | 1.2383 |
| Remove Trap | 5422 | 1.2383 |
| Rising Air Current | 5422 | 1.2383 |
| Roulette Barrel | 5422 | 1.2383 |
| Sacred Crane | 5422 | 1.2383 |
| Time Wizard | 5422 | 1.2383 |
| UFO Turtle | 5422 | 1.2383 |
| Abyss Soldier | 4629 | 1.0572 |
| Amazoness Archer | 4138 | 0.9450 |
| Black Illusion Ritual | 4138 | 0.9450 |
| Blast with Chain | 4138 | 0.9450 |
| Dimension Fusion | 4138 | 0.9450 |
| Divine Wrath | 4138 | 0.9450 |
| Electric Snake | 4138 | 0.9450 |
| Fissure | 4138 | 0.9450 |
| Nightmare's Steelcage | 4138 | 0.9450 |
| Princess of Tsurugi | 4138 | 0.9450 |
| Relinquished | 4138 | 0.9450 |
| Time Seal | 4138 | 0.9450 |
| Warrior Elimination | 4138 | 0.9450 |
| Aqua Spirit | 3537 | 0.8078 |
| Big Shield Gardna | 3537 | 0.8078 |
| Granmarg the Rock Monarch | 3537 | 0.8078 |
| Mage Power | 3537 | 0.8078 |
| Magic Jammer | 3537 | 0.8078 |
| Mind Control | 3537 | 0.8078 |
| Nimble Momonga | 3537 | 0.8078 |
| Skelengel | 3537 | 0.8078 |
| The Wicked Worm Beast | 3537 | 0.8078 |
| Tribute to The Doomed | 3537 | 0.8078 |
| Des Lacooda | 3165 | 0.7228 |
| Flying Kamakiri #1 | 3165 | 0.7228 |
| Goblin Elite Attack Force | 3165 | 0.7228 |
| Slate Warrior | 3165 | 0.7228 |
| Cannon Soldier | 2905 | 0.6634 |
| Mask of Darkness | 2905 | 0.6634 |
| Old Vindictive Magician | 2905 | 0.6634 |
| Sand Moth | 2905 | 0.6634 |
| Sasuke Samurai | 2905 | 0.6634 |
| Secret Barrel | 2905 | 0.6634 |
| Seven Tools of the Bandit | 2905 | 0.6634 |
| Swarm of Locusts | 2905 | 0.6634 |
| Apprentice Magician | 2709 | 0.6187 |
| Dark Magician of Chaos | 2709 | 0.6187 |
| Goblin Attack Force | 2709 | 0.6187 |
| Thunder Dragon | 2709 | 0.6187 |
| Berserk Gorilla | 2555 | 0.5835 |
| Book of Life | 2555 | 0.5835 |
| Exarion Universe | 2555 | 0.5835 |
| Hallowed Life Barrier | 2555 | 0.5835 |
| Just Desserts | 2555 | 0.5835 |
| Megamorph | 2555 | 0.5835 |
| Milus Radiant | 2555 | 0.5835 |
| Nobleman of Extermination | 2555 | 0.5835 |
| Ojama Trio | 2555 | 0.5835 |
| Sillva, Warlord of Dark World | 2555 | 0.5835 |
| Vampire Lord | 2555 | 0.5835 |
| Upstart Goblin | 2488 | 0.5682 |
| Gigantes | 2428 | 0.5545 |
| Jar of Greed | 2428 | 0.5545 |
| Pyramid Turtle | 2428 | 0.5545 |
| Ryu Kokki | 2428 | 0.5545 |
| Skilled Dark Magician | 2428 | 0.5545 |
| Compulsory Evacuation Device | 2322 | 0.5303 |
| Cursed Seal of the Forbidden Spell | 2322 | 0.5303 |
| Gyaku-Gire Panda | 2322 | 0.5303 |
| Airknight Parshath | 2231 | 0.5095 |
| Chiron the Mage | 2231 | 0.5095 |
| Dark Mimic LV1 | 2231 | 0.5095 |
| Cold Wave | 2152 | 0.4915 |
| Des Koala | 2152 | 0.4915 |
| Spell Canceller | 2152 | 0.4915 |
| Zombyra the Dark | 2082 | 0.4755 |
| Blade Knight | 2021 | 0.4616 |
| Mother Grizzly | 1965 | 0.4488 |
| Phoenix Wing Wind Blast | 1965 | 0.4488 |
| Chain Disappearance | 1915 | 0.4373 |
| Hydrogeddon | 1915 | 0.4373 |
| Dark Scorpion - Cliff the Trap Remover | 1869 | 0.4268 |
| Goldd, Wu-Lord of Dark World | 1869 | 0.4268 |
| Giant Rat | 1827 | 0.4172 |
| Trap Hole | 1827 | 0.4172 |
| Drillroid | 1789 | 0.4086 |
| Stealth Bird | 1789 | 0.4086 |
| Des Wombat | 1720 | 0.3928 |
| Newdoria | 1689 | 0.3857 |
| Night Assailant | 1607 | 0.3670 |
| Ninja Grandmaster Sasuke | 1607 | 0.3670 |
| United We Stand | 1607 | 0.3670 |
| Bazoo the Soul-Eater | 1560 | 0.3563 |
| Soul Exchange | 1539 | 0.3515 |
| Card Destruction | 1480 | 0.3380 |
| Kuriboh | 1480 | 0.3380 |
| Gravekeeper's Spy | 1429 | 0.3264 |
| Waboku | 1429 | 0.3264 |
| Magical Merchant | 1385 | 0.3163 |
| Raigeki Break | 1385 | 0.3163 |
| Pot of Avarice | 1371 | 0.3131 |
| Dark Ruler Ha Des | 1345 | 0.3072 |
| B. Skull Dragon | 1343 | 0.3067 |
| Cyber Saurus | 1343 | 0.3067 |
| Deepsea Shark | 1343 | 0.3067 |
| Labyrinth Tank | 1343 | 0.3067 |
| Musician King | 1343 | 0.3067 |
| D.D. Survivor | 1320 | 0.3015 |
| Return from the Different Dimension | 1309 | 0.2989 |
| Thestalos the Firestorm Monarch | 1277 | 0.2916 |
| Cipher Soldier | 1229 | 0.2807 |
| King Tiger Wanghu | 1229 | 0.2807 |
| Injection Fairy Lily | 1220 | 0.2786 |
| Magic Cylinder | 1220 | 0.2786 |
| Last Will | 1195 | 0.2729 |
| Giant Trunade | 1165 | 0.2661 |
| Dekoichi the Battlechanted Locomotive | 1158 | 0.2645 |
| Treeborn Frog | 1151 | 0.2629 |
| Cyber-Stein | 1131 | 0.2583 |
| Lightning Vortex | 1131 | 0.2583 |
| Chaos Sorcerer | 1118 | 0.2553 |
| Solemn Judgment | 1095 | 0.2501 |
| Widespread Ruin | 1084 | 0.2476 |
| Trap Dustshoot | 1068 | 0.2439 |
| Brain Control | 1063 | 0.2428 |
| Zaborg the Thunder Monarch | 1063 | 0.2428 |
| D.D. Assailant | 1058 | 0.2416 |
| Don Zaloog | 1058 | 0.2416 |
| Bottomless Trap Hole | 1039 | 0.2373 |
| Bickuribox | 1021 | 0.2332 |
| Charubin the Fire Knight | 1021 | 0.2332 |
| Humanoid Worm Drake | 1021 | 0.2332 |
| Roaring Ocean Snake | 1021 | 0.2332 |
| Mystic Tomato | 989 | 0.2259 |
| Creature Swap | 980 | 0.2238 |
| Threatening Roar | 978 | 0.2234 |
| Ceasefire | 944 | 0.2156 |
| Mystic Swordsman LV2 | 932 | 0.2128 |
| Kycoo the Ghost Destroyer | 920 | 0.2101 |
| Cyber Jar | 917 | 0.2094 |
| Metamorphosis | 917 | 0.2094 |
| Morphing Jar | 909 | 0.2076 |
| Smashing Ground | 896 | 0.2046 |
| Dark Paladin | 870 | 0.1987 |
| Gaia the Dragon Champion | 870 | 0.1987 |
| Sakuretsu Armor | 840 | 0.1918 |
| Mobius the Frost Monarch | 831 | 0.1898 |
| Dust Tornado | 820 | 0.1873 |
| Swords of Revealing Light | 813 | 0.1857 |
| Reinforcement of the Army | 791 | 0.1806 |
| Cyber Dragon | 789 | 0.1802 |
| Spirit Reaper | 780 | 0.1781 |
| Tsukuyomi | 780 | 0.1781 |
| Flame Swordsman | 777 | 0.1774 |
| Kaminari Attack | 777 | 0.1774 |
| Karbonala Warrior | 777 | 0.1774 |
| Jinzo | 734 | 0.1676 |
| Magician of Faith | 731 | 0.1669 |
| Empress Judge | 711 | 0.1624 |
| Giltia the D. Knight | 662 | 0.1512 |
| St. Joan | 623 | 0.1423 |
| Exiled Force | 580 | 0.1325 |
| Confiscation | 568 | 0.1297 |
| D.D. Warrior Lady | 568 | 0.1297 |
| Flame Ghost | 565 | 0.1290 |
| Premature Burial | 553 | 0.1263 |
| Nobleman of Crossout | 546 | 0.1247 |
| Light of Intervention | 542 | 0.1238 |
| Mirror Wall | 542 | 0.1238 |
| Royal Command | 542 | 0.1238 |
| Royal Oppression | 542 | 0.1238 |
| Skull Lair | 542 | 0.1238 |
| Warrior of Tradition | 542 | 0.1238 |
| Breaker the Magical Warrior | 535 | 0.1222 |
| Sangan | 535 | 0.1222 |
| Torrential Tribute | 531 | 0.1213 |
| Snatch Steal | 530 | 0.1210 |
| Dark Hole | 526 | 0.1201 |
| Heavy Storm | 526 | 0.1201 |
| Fusionist | 504 | 0.1151 |
| Emergency Provisions | 462 | 0.1055 |
| Twin-Headed Thunder Dragon | 431 | 0.0984 |
| Chain Energy | 413 | 0.0943 |
| Robbin' Goblin | 413 | 0.0943 |
| Rush Recklessly | 413 | 0.0943 |
| Offerings to the Doomed | 353 | 0.0806 |
| Poison of the Old Man | 353 | 0.0806 |
| Nightmare Wheel | 316 | 0.0722 |
| Dark Flare Knight | 313 | 0.0715 |
| Dragoness the Wicked Knight | 297 | 0.0678 |
| Cyber Blader | 292 | 0.0667 |
| My Body as a Shield | 290 | 0.0662 |
| Darkfire Dragon | 288 | 0.0658 |
| Master of Oz | 276 | 0.0630 |
| Prohibition | 270 | 0.0617 |
| Mystik Wok | 232 | 0.0530 |
| Ojama King | 224 | 0.0512 |
| Blue-Eyes Ultimate Dragon | 222 | 0.0507 |
| Dark Blade the Dragon Knight | 222 | 0.0507 |
| King Dragun | 214 | 0.0489 |
| Reaper on the Nightmare | 213 | 0.0486 |
| Fiend Skull Dragon | 209 | 0.0477 |
| The Last Warrior from Another Planet | 206 | 0.0470 |
| Messenger of Peace | 202 | 0.0461 |
| Gatling Dragon | 199 | 0.0454 |
| Ryu Senshi | 199 | 0.0454 |
| Dark Balter the Terrible | 198 | 0.0452 |
| Cyber End Dragon | 191 | 0.0436 |
| Cyber Twin Dragon | 191 | 0.0436 |
| Gravity Bind | 164 | 0.0375 |
| Limiter Removal | 160 | 0.0365 |
| Mask of Restrict | 148 | 0.0338 |
| Wave-Motion Cannon | 146 | 0.0333 |
| Level Limit - Area B | 145 | 0.0331 |
| Thousand-Eyes Restrict | 127 | 0.0290 |
| Royal Decree | 110 | 0.0251 |
| Enemy Controller | 100 | 0.0228 |
| Scapegoat | 71 | 0.0162 |
| Call Of The Haunted | 62 | 0.0142 |
| Book of Moon | 60 | 0.0137 |
| Mystical Space Typhoon | 52 | 0.0119 |

_1099 card names in JSON/usage had no ROM match (omitted from pools)._

<!-- /pack-chances -->

<!-- pack-prices -->

## Password machine prices (DP)

Same formula as `tools/cardprices.py` / `src/mod_dp.c`. Each table lists only cards that format's usage file mentions. At runtime, **GetCardCost** charges a card outside the newest format's card pool **3000 DP**; otherwise it uses the newest unlocked format whose decks use the card.

- **Usage score:** main+side copies per deck × 1000 + extra copies per deck × 1000/10; ×3 if Limited, ×1.5 if Semi-Limited.
- **Price:** the score, kept between **200 DP** and **3000 DP** (a Limited one-of in every deck: 1000 × 3 = 3000; a Semi-Limited two-of: 2000 × 1.5 = 3000).
- **In an unlocked pool, in no unlocked usage table:** **100 DP** (`DEFAULT_CARD_COST`).
- **In no unlocked pool, or outside the newest pool:** **3000 DP** (`NON_CARD_POOL_COST`).

| Format | Cards priced |
|---|---:|
| [Yugi-Kaiba](#yugi-kaiba) | 75 |
| [Critter](#critter) | 88 |
| [Treasure](#treasure) | 84 |
| [Imperial](#imperial) | 106 |
| [Android](#android) | 107 |
| [Joey-Pegasus](#joey-pegasus) | 110 |
| [Fiber](#fiber) | 143 |
| [Yata](#yata) | 136 |
| [Scientist](#scientist) | 92 |
| [Vampire](#vampire) | 114 |
| [Chaos](#chaos) | 142 |
| [Warrior](#warrior) | 191 |
| [Goat](#goat) | 282 |
| [Cyber](#cyber) | 142 |
| [Reaper](#reaper) | 232 |

### Yugi-Kaiba

| Card | DP | Limit |
|---|---:|---|
| Change of Heart | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Fissure | 3000 |  |
| Man-Eater Bug | 3000 |  |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Trap Hole | 3000 |  |
| La Jinn the Mystical Genie of the Lamp | 2976 |  |
| Reinforcements | 2935 |  |
| Swords of Revealing Light | 2927 | Semi-Limited |
| Neo the Magic Swordsman | 2911 |  |
| Giant Soldier of Stone | 2894 |  |
| Waboku | 2837 |  |
| Battle Ox | 2789 |  |
| Summoned Skull | 2715 |  |
| Soul Exchange | 2415 |  |
| Wall of Illusion | 2098 |  |
| Card Destruction | 1927 | Semi-Limited |
| Ultimate Offering | 1626 |  |
| Aqua Madoor | 1431 |  |
| Trap Master | 1268 |  |
| Judge Man | 1252 |  |
| De-Spell | 1000 |  |
| Just Desserts | 1000 |  |
| Ookazi | 976 |  |
| Mystical Elf | 756 |  |
| Stop Defense | 610 |  |
| The Wicked Worm Beast | 602 |  |
| Spirit of the Harp | 504 |  |
| Gyakutenno Megami | 488 |  |
| Great White | 390 |  |
| Remove Trap | 260 |  |
| Dragon Capture Jar | 252 |  |
| Hane-Hane | 244 |  |
| Last Will | 228 |  |
| Armed Ninja | 200 |  |
| Blue-Eyes White Dragon | 200 |  |
| Castle Walls | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Curse of Dragon | 200 |  |
| Darkfire Dragon | 200 |  |
| Destroyer Golem | 200 |  |
| Dian Keto the Cure Master | 200 |  |
| Dragon Zombie | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Exodia the Forbidden One | 200 | Limited |
| Final Flame | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flower Wolf | 200 |  |
| Forest | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Invigoration | 200 |  |
| Karbonala Warrior | 200 |  |
| Koumori Dragon | 200 |  |
| Left Arm of the Forbidden One | 200 | Limited |
| Left Leg of the Forbidden One | 200 | Limited |
| Lord of D. | 200 |  |
| Metal Dragon | 200 |  |
| Petit Dragon | 200 |  |
| Reaper of the Cards | 200 |  |
| Reverse Trap | 200 |  |
| Right Arm of the Forbidden One | 200 | Limited |
| Right Leg of the Forbidden One | 200 | Limited |
| Ryu-Kishin Powered | 200 |  |
| Sparks | 200 |  |
| Sword of Dark Destruction | 200 |  |
| The Flute of Summoning Dragon | 200 |  |
| The Stern Mystic | 200 |  |
| Two-Pronged Attack | 200 |  |
| Umi  | 200 |  |
| Wasteland | 200 |  |
| Yami | 200 |  |

### Critter

| Card | DP | Limit |
|---|---:|---|
| Change of Heart | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Heavy Storm | 3000 | Semi-Limited |
| Mirror Force | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Semi-Limited |
| Witch of the Black Forest | 3000 | Semi-Limited |
| Trap Hole | 2985 |  |
| Fissure | 2926 |  |
| La Jinn the Mystical Genie of the Lamp | 2824 |  |
| Swords of Revealing Light | 2824 | Semi-Limited |
| Magician of Faith | 2676 |  |
| Solemn Judgment | 2647 |  |
| Waboku | 2618 |  |
| Muka Muka | 2353 |  |
| Thunder Dragon | 2294 |  |
| Card Destruction | 1941 | Semi-Limited |
| Masked Sorcerer | 1868 |  |
| Last Will | 1779 |  |
| Giant Soldier of Stone | 1676 |  |
| Man-Eater Bug | 1618 |  |
| Robbin' Goblin | 1456 |  |
| Seven Tools of the Bandit | 1441 |  |
| Cannon Soldier | 1206 |  |
| Wall of Illusion | 971 |  |
| 7 Colored Fish | 779 |  |
| Jirai Gumo | 632 |  |
| White Magical Hat | 574 |  |
| Magic Jammer | 485 |  |
| Exodia the Forbidden One | 397 | Limited |
| Left Arm of the Forbidden One | 397 | Limited |
| Left Leg of the Forbidden One | 397 | Limited |
| Right Arm of the Forbidden One | 397 | Limited |
| Right Leg of the Forbidden One | 397 | Limited |
| Fake Trap | 368 |  |
| Horn of Heaven | 309 |  |
| Trap Master | 250 |  |
| Reinforcements | 221 |  |
| Dark Elf | 206 |  |
| Mask of Darkness | 206 |  |
| Tremendous Fire | 206 |  |
| Aqua Madoor | 200 |  |
| B. Skull Dragon | 200 |  |
| Bickuribox | 200 |  |
| Catapult Turtle | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Crass Clown | 200 |  |
| Cyber Saurus | 200 |  |
| Darkfire Dragon | 200 |  |
| De-Spell | 200 |  |
| Deepsea Shark | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Dream Clown | 200 |  |
| Empress Judge | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flower Wolf | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Giga-tech Wolf | 200 |  |
| Giltia the D. Knight | 200 |  |
| Jinzo #7 | 200 |  |
| Karbonala Warrior | 200 |  |
| Kuriboh | 200 |  |
| Labyrinth Tank | 200 |  |
| Metal Dragon | 200 |  |
| Musician King | 200 |  |
| Ookazi | 200 |  |
| Polymerization | 200 |  |
| Princess of Tsurugi | 200 |  |
| Punished Eagle | 200 |  |
| Remove Trap | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Share the Pain | 200 |  |
| Shield & Sword | 200 |  |
| Skull Knight | 200 |  |
| Soul Exchange | 200 |  |
| Stim-Pack | 200 |  |
| Stop Defense | 200 |  |
| Summoned Skull | 200 |  |
| The Wicked Worm Beast | 200 |  |
| Thousand Dragon | 200 |  |
| Tribute to The Doomed | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Ultimate Offering | 200 |  |

### Treasure

| Card | DP | Limit |
|---|---:|---|
| Change of Heart | 3000 | Limited |
| Confiscation | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Delinquent Duo | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Mystic Tomato | 3000 |  |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Semi-Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Semi-Limited |
| Monster Reborn | 2870 | Limited |
| Heavy Storm | 2609 | Semi-Limited |
| Mystical Space Typhoon | 2609 |  |
| Snatch Steal | 2609 | Limited |
| Mechanicalchaser | 2522 |  |
| Trap Hole | 2522 |  |
| Fissure | 2478 |  |
| Man-Eater Bug | 2348 |  |
| Magician of Faith | 2174 |  |
| Waboku | 2000 |  |
| Painful Choice | 1957 | Limited |
| Banisher of the Light | 1696 |  |
| Seven Tools of the Bandit | 1565 |  |
| Solemn Judgment | 1478 |  |
| Giant Trunade | 1304 |  |
| Nimble Momonga | 1304 |  |
| Tribute to The Doomed | 1174 |  |
| Cannon Soldier | 1000 |  |
| Cyber Jar | 783 | Limited |
| Jirai Gumo | 783 |  |
| Rush Recklessly | 783 |  |
| Last Will | 696 |  |
| Thunder Dragon | 652 |  |
| Card Destruction | 587 | Semi-Limited |
| Upstart Goblin | 522 | Limited |
| Magic Jammer | 478 |  |
| Swords of Revealing Light | 457 | Semi-Limited |
| Robbin' Goblin | 435 |  |
| Chain Energy | 391 |  |
| Giant Germ | 391 |  |
| Just Desserts | 391 |  |
| Minar | 391 |  |
| Princess of Tsurugi | 391 |  |
| Spear Cretin | 391 |  |
| Giant Soldier of Stone | 348 |  |
| Masked Sorcerer | 348 |  |
| Giant Rat | 261 |  |
| B. Skull Dragon | 200 |  |
| Bickuribox | 200 |  |
| Catapult Turtle | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Crass Clown | 200 |  |
| Cyber Saurus | 200 |  |
| Darkfire Dragon | 200 |  |
| Deepsea Shark | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Empress Judge | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flash Assailant | 200 |  |
| Flower Wolf | 200 |  |
| Fusionist | 200 |  |
| Gaia Power | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Hercules Beetle | 200 |  |
| Horn of Heaven | 200 |  |
| Karbonala Warrior | 200 |  |
| Kuriboh | 200 |  |
| La Jinn the Mystical Genie of the Lamp | 200 |  |
| Labyrinth Tank | 200 |  |
| Metal Dragon | 200 |  |
| Mother Grizzly | 200 |  |
| Musician King | 200 |  |
| Share the Pain | 200 |  |
| Skull Knight | 200 |  |
| Slot Machine | 200 |  |
| Soul Exchange | 200 |  |
| Summoned Skull | 200 |  |
| Thousand Dragon | 200 |  |
| Toll | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Wall of Illusion | 200 |  |
| White Magical Hat | 200 |  |

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
| Mystic Tomato | 3000 |  |
| Mystical Space Typhoon | 3000 |  |
| Nobleman of Crossout | 3000 | Semi-Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Semi-Limited |
| Snatch Steal | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Call Of The Haunted | 2929 | Limited |
| Mechanicalchaser | 2929 |  |
| Magician of Faith | 2667 |  |
| Heavy Storm | 2643 | Semi-Limited |
| Trap Hole | 2571 |  |
| Man-Eater Bug | 2429 |  |
| Fissure | 2381 |  |
| Premature Burial | 2357 | Limited |
| Morphing Jar | 2286 | Limited |
| Ceasefire | 2214 | Limited |
| Dust Tornado | 2000 |  |
| Cyber Jar | 1929 | Limited |
| Solemn Judgment | 1714 |  |
| Swords of Revealing Light | 1571 | Limited |
| Painful Choice | 1071 | Limited |
| Banisher of the Light | 881 |  |
| Cannon Soldier | 881 |  |
| Goblin Attack Force | 738 |  |
| Time Seal | 714 |  |
| Giant Trunade | 690 |  |
| The Fiend Megacyber | 643 |  |
| Solemn Wishes | 595 |  |
| Waboku | 452 |  |
| Card Destruction | 286 | Limited |
| Giant Soldier of Stone | 238 |  |
| Giant Germ | 214 |  |
| Limiter Removal | 214 | Limited |
| Relinquished | 214 |  |
| 4-Starred Ladybug of Doom | 200 |  |
| Acid Trap Hole | 200 |  |
| Aqua Madoor | 200 |  |
| B. Skull Dragon | 200 |  |
| Backup Soldier | 200 | Semi-Limited |
| Bickuribox | 200 |  |
| Black Illusion Ritual | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Cyber Saurus | 200 |  |
| Darkfire Dragon | 200 |  |
| Deepsea Shark | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Empress Judge | 200 |  |
| Exchange | 200 |  |
| Exodia the Forbidden One | 200 | Limited |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Giltia the D. Knight | 200 |  |
| Graverobber | 200 |  |
| Gravity Bind | 200 |  |
| Kuriboh | 200 |  |
| Kwagar Hercules | 200 |  |
| Labyrinth Tank | 200 |  |
| Last Will | 200 |  |
| Left Arm of the Forbidden One | 200 | Limited |
| Left Leg of the Forbidden One | 200 | Limited |
| Light of Intervention | 200 |  |
| Magic Drain | 200 |  |
| Magic Jammer | 200 |  |
| Mask of Darkness | 200 |  |
| Masked Sorcerer | 200 |  |
| Megamorph | 200 |  |
| Metal Dragon | 200 |  |
| Morphing Jar #2 | 200 | Semi-Limited |
| Musician King | 200 |  |
| Nimble Momonga | 200 |  |
| Prohibition | 200 |  |
| Punished Eagle | 200 |  |
| Rabid Horseman | 200 |  |
| Reinforcements | 200 |  |
| Right Arm of the Forbidden One | 200 | Limited |
| Right Leg of the Forbidden One | 200 | Limited |
| Roaring Ocean Snake | 200 |  |
| Robbin' Goblin | 200 |  |
| Rush Recklessly | 200 |  |
| Senju of the Thousand Hands | 200 |  |
| Seven Tools of the Bandit | 200 |  |
| Skull Knight | 200 |  |
| Sonic Bird | 200 |  |
| Soul Exchange | 200 |  |
| Thousand Dragon | 200 |  |
| Thousand-Eyes Restrict | 200 |  |
| Thunder Dragon | 200 |  |
| Tribute to The Doomed | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Type Zero Magic Crusher | 200 |  |
| Upstart Goblin | 200 | Limited |
| Warrior of Tradition | 200 |  |
| White Magical Hat | 200 |  |

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
| Nobleman of Crossout | 3000 | Semi-Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Mystical Space Typhoon | 2974 |  |
| Heavy Storm | 2961 | Semi-Limited |
| Imperial Order | 2921 | Limited |
| Snatch Steal | 2921 | Limited |
| Mystic Tomato | 2842 |  |
| Painful Choice | 2842 | Limited |
| Jar of Greed | 2605 |  |
| Gemini Elf | 2579 |  |
| Kycoo the Ghost Destroyer | 2553 |  |
| Torrential Tribute | 2421 |  |
| Fissure | 2158 |  |
| Bazoo the Soul-Eater | 2132 |  |
| Trap Hole | 1868 |  |
| Morphing Jar | 1658 | Limited |
| Premature Burial | 1500 | Limited |
| Cyber Jar | 1421 | Limited |
| Dust Tornado | 1368 |  |
| Magician of Faith | 1184 |  |
| Solemn Judgment | 1184 |  |
| Skull Lair | 1079 |  |
| Ceasefire | 1026 | Limited |
| Swords of Revealing Light | 868 | Limited |
| Magic Cylinder | 789 | Limited |
| Cannon Soldier | 500 |  |
| 4-Starred Ladybug of Doom | 395 |  |
| Card Destruction | 395 | Limited |
| Time Seal | 395 |  |
| Masked Sorcerer | 368 |  |
| Penguin Knight | 316 |  |
| Man-Eater Bug | 263 |  |
| Seven Tools of the Bandit | 263 |  |
| Solemn Wishes | 263 |  |
| Exchange | 237 |  |
| Nimble Momonga | 237 |  |
| United We Stand | 237 | Limited |
| Zombyra the Dark | 237 |  |
| Axe of Despair | 200 |  |
| B. Skull Dragon | 200 |  |
| Banisher of the Light | 200 |  |
| Bickuribox | 200 |  |
| Castle of Dark Illusions | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Cyber Saurus | 200 |  |
| Dark Elf | 200 |  |
| Dark Necrofear | 200 |  |
| Darkfire Dragon | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Fairy Box | 200 |  |
| Fake Trap | 200 |  |
| Firegrass | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flower Wolf | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Giant Germ | 200 |  |
| Giant Soldier of Stone | 200 |  |
| Giant Trunade | 200 |  |
| Giltia the D. Knight | 200 |  |
| Goblin Attack Force | 200 |  |
| Hayabusa Knight | 200 |  |
| Humanoid Worm Drake | 200 |  |
| Jirai Gumo | 200 |  |
| Kaminari Attack | 200 |  |
| Karbonala Warrior | 200 |  |
| Kuriboh | 200 |  |
| Kwagar Hercules | 200 |  |
| La Jinn the Mystical Genie of the Lamp | 200 |  |
| Labyrinth Tank | 200 |  |
| Mage Power | 200 | Limited |
| Magic Drain | 200 |  |
| Mask of Darkness | 200 |  |
| Megamorph | 200 |  |
| Metal Dragon | 200 |  |
| Mirror Wall | 200 |  |
| Musician King | 200 |  |
| Nobleman of Extermination | 200 |  |
| Punished Eagle | 200 |  |
| Royal Command | 200 |  |
| Skull Knight | 200 |  |
| Soul Release | 200 |  |
| St. Joan | 200 |  |
| Summoned Skull | 200 |  |
| The Fiend Megacyber | 200 |  |
| The Last Warrior from Another Planet | 200 |  |
| Thousand Dragon | 200 |  |
| Thousand-Eyes Restrict | 200 |  |
| Thunder Dragon | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Upstart Goblin | 200 | Limited |
| Waboku | 200 |  |
| Warrior of Tradition | 200 |  |
| White Magical Hat | 200 |  |

### Joey-Pegasus

| Card | DP | Limit |
|---|---:|---|
| Call Of The Haunted | 3000 | Limited |
| Change of Heart | 3000 | Limited |
| Confiscation | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Delinquent Duo | 3000 | Limited |
| Graceful Charity | 3000 | Semi-Limited |
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
| Mystic Tomato | 2895 |  |
| Mystical Space Typhoon | 2842 |  |
| Painful Choice | 2842 | Limited |
| Premature Burial | 2842 | Limited |
| Magician of Faith | 2632 |  |
| Torrential Tribute | 2474 |  |
| Heavy Storm | 2368 | Limited |
| Gemini Elf | 2316 |  |
| Nobleman of Crossout | 2289 | Semi-Limited |
| Bazoo the Soul-Eater | 2263 |  |
| Kycoo the Ghost Destroyer | 2105 |  |
| Jar of Greed | 2000 |  |
| Solemn Judgment | 1421 |  |
| Morphing Jar | 1263 | Limited |
| Fissure | 1211 |  |
| Cyber Jar | 947 | Limited |
| Waboku | 842 |  |
| Ceasefire | 789 | Limited |
| Magic Cylinder | 789 | Limited |
| Skull Lair | 737 |  |
| Man-Eater Bug | 684 |  |
| Trap Hole | 684 |  |
| Dust Tornado | 579 |  |
| Cannon Soldier | 526 |  |
| Scapegoat | 526 |  |
| Mage Power | 474 | Limited |
| Upstart Goblin | 474 | Limited |
| Time Seal | 368 |  |
| Nimble Momonga | 316 |  |
| Thunder Dragon | 316 |  |
| United We Stand | 316 | Limited |
| Mirror Wall | 211 |  |
| Armed Ninja | 200 |  |
| Axe of Despair | 200 |  |
| B. Skull Dragon | 200 |  |
| Backup Soldier | 200 | Semi-Limited |
| Bickuribox | 200 |  |
| Card Destruction | 200 | Limited |
| Charubin the Fire Knight | 200 |  |
| Cold Wave | 200 |  |
| Cyber Saurus | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Darkfire Dragon | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Empress Judge | 200 |  |
| Exchange | 200 |  |
| Exodia the Forbidden One | 200 | Limited |
| Fiend Skull Dragon | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flower Wolf | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Giant Germ | 200 |  |
| Giant Rat | 200 |  |
| Giant Trunade | 200 |  |
| Giltia the D. Knight | 200 |  |
| Guardian Sphinx | 200 |  |
| Hayabusa Knight | 200 |  |
| Humanoid Worm Drake | 200 |  |
| Jowgen the Spiritualist | 200 |  |
| Kaminari Attack | 200 |  |
| Kotodama | 200 |  |
| Kuriboh | 200 |  |
| Labyrinth Tank | 200 |  |
| Last Will | 200 |  |
| Left Arm of the Forbidden One | 200 | Limited |
| Left Leg of the Forbidden One | 200 | Limited |
| Limiter Removal | 200 | Limited |
| Magic Drain | 200 |  |
| Masked Sorcerer | 200 |  |
| Metal Dragon | 200 |  |
| Musician King | 200 |  |
| Offerings to the Doomed | 200 |  |
| Penguin Knight | 200 |  |
| Reinforcement of the Army | 200 |  |
| Right Arm of the Forbidden One | 200 | Limited |
| Right Leg of the Forbidden One | 200 | Limited |
| Roaring Ocean Snake | 200 |  |
| Rush Recklessly | 200 |  |
| Ryu Senshi | 200 |  |
| Seven Tools of the Bandit | 200 |  |
| Skull Knight | 200 |  |
| Solemn Wishes | 200 |  |
| St. Joan | 200 |  |
| Summoned Skull | 200 |  |
| Swords of Revealing Light | 200 | Limited |
| The Last Warrior from Another Planet | 200 |  |
| Thousand-Eyes Restrict | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Warrior of Tradition | 200 |  |
| White Magical Hat | 200 |  |

### Fiber

| Card | DP | Limit |
|---|---:|---|
| Dark Hole | 3000 | Limited |
| Graceful Charity | 3000 | Semi-Limited |
| Imperial Order | 3000 | Limited |
| Mirror Force | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Limited |
| The Forceful Sentry | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Harpie's Feather Duster | 2984 | Limited |
| Mystical Space Typhoon | 2984 |  |
| Snatch Steal | 2984 | Limited |
| Call Of The Haunted | 2968 | Limited |
| Change of Heart | 2968 | Limited |
| Exiled Force | 2968 | Limited |
| Fiber Jar | 2968 | Limited |
| Monster Reborn | 2968 | Limited |
| Delinquent Duo | 2935 | Limited |
| Injection Fairy Lily | 2935 | Limited |
| Confiscation | 2919 | Limited |
| Jinzo | 2871 | Limited |
| Sinister Serpent | 2871 | Limited |
| Painful Choice | 2774 | Limited |
| Torrential Tribute | 2651 |  |
| Yata-Garasu | 2629 | Limited |
| Mystic Tomato | 2602 |  |
| Nobleman of Crossout | 2597 | Semi-Limited |
| Heavy Storm | 2242 | Limited |
| Premature Burial | 2065 | Limited |
| Gemini Elf | 1640 |  |
| Bazoo the Soul-Eater | 1435 |  |
| Morphing Jar | 1371 | Limited |
| Cyber Jar | 1355 | Limited |
| Dust Tornado | 1323 |  |
| Kycoo the Ghost Destroyer | 1323 |  |
| Magic Cylinder | 1290 | Limited |
| Trap Hole | 1285 |  |
| Creature Swap | 1282 | Semi-Limited |
| Jar of Greed | 1231 |  |
| Scapegoat | 1091 |  |
| Magician of Faith | 1070 |  |
| Fissure | 876 |  |
| Asura Priest | 806 |  |
| Ceasefire | 726 | Limited |
| Giant Rat | 575 |  |
| Spear Dragon | 366 |  |
| Card Destruction | 355 | Limited |
| Time Seal | 339 |  |
| Skull Lair | 323 |  |
| Cannon Soldier | 306 |  |
| Upstart Goblin | 290 | Limited |
| United We Stand | 258 | Limited |
| Thunder Dragon | 242 |  |
| Solemn Judgment | 237 |  |
| 4-Starred Ladybug of Doom | 200 |  |
| Acid Trap Hole | 200 |  |
| Airknight Parshath | 200 |  |
| Amazoness Archer | 200 |  |
| B. Skull Dragon | 200 |  |
| Bickuribox | 200 |  |
| Black Illusion Ritual | 200 |  |
| Blast with Chain | 200 |  |
| Blind Destruction | 200 |  |
| Bottomless Trap Hole | 200 |  |
| Chain Energy | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Cure Mermaid | 200 |  |
| Cyber Saurus | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Dark Necrofear | 200 |  |
| Dark Ruler Ha Des | 200 |  |
| Darkfire Dragon | 200 |  |
| Don Zaloog | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Ekibyo Drakmord | 200 |  |
| Electric Snake | 200 |  |
| Empress Judge | 200 |  |
| Exchange | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flash Assailant | 200 |  |
| Flower Wolf | 200 |  |
| Flying Kamakiri #1 | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Gearfried the Iron Knight | 200 |  |
| Giant Germ | 200 |  |
| Giant Trunade | 200 |  |
| Giltia the D. Knight | 200 |  |
| Goblin Attack Force | 200 |  |
| Graceful Dice | 200 |  |
| Graverobber | 200 |  |
| Gravity Bind | 200 |  |
| Hayabusa Knight | 200 |  |
| Jirai Gumo | 200 |  |
| Jowgen the Spiritualist | 200 |  |
| Kuriboh | 200 |  |
| Kwagar Hercules | 200 |  |
| Labyrinth Tank | 200 |  |
| Last Will | 200 |  |
| Limiter Removal | 200 | Limited |
| Mad Sword Beast | 200 |  |
| Mage Power | 200 | Limited |
| Magic Drain | 200 |  |
| Magic Jammer | 200 |  |
| Man-Eater Bug | 200 |  |
| Mask of Darkness | 200 |  |
| Megamorph | 200 |  |
| Metal Dragon | 200 |  |
| Metalmorph | 200 |  |
| Mirror Wall | 200 |  |
| Musician King | 200 |  |
| Nimble Momonga | 200 |  |
| Nobleman of Extermination | 200 |  |
| Offerings to the Doomed | 200 |  |
| Princess of Tsurugi | 200 |  |
| Punished Eagle | 200 |  |
| Reinforcement of the Army | 200 | Semi-Limited |
| Relinquished | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Rush Recklessly | 200 |  |
| Ryu Senshi | 200 |  |
| Senju of the Thousand Hands | 200 |  |
| Seven Tools of the Bandit | 200 |  |
| Skull Knight | 200 |  |
| Solemn Wishes | 200 |  |
| Sonic Bird | 200 |  |
| Soul Exchange | 200 |  |
| St. Joan | 200 |  |
| Summoned Skull | 200 |  |
| Swords of Revealing Light | 200 | Limited |
| The Fiend Megacyber | 200 |  |
| The Last Warrior from Another Planet | 200 |  |
| The Rock Spirit | 200 |  |
| Thousand Dragon | 200 |  |
| Thousand-Eyes Restrict | 200 |  |
| Tribute to The Doomed | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Waboku | 200 |  |
| Warrior of Tradition | 200 |  |
| White Magical Hat | 200 |  |
| Zombyra the Dark | 200 |  |

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
| Mystical Space Typhoon | 2987 |  |
| Imperial Order | 2962 | Limited |
| Mirage of Nightmare | 2962 | Limited |
| Jinzo | 2923 | Limited |
| Monster Reborn | 2923 | Limited |
| Delinquent Duo | 2885 | Limited |
| Exiled Force | 2885 | Limited |
| Sangan | 2885 | Limited |
| Confiscation | 2846 | Limited |
| Call Of The Haunted | 2731 | Limited |
| Heavy Storm | 2692 | Limited |
| Injection Fairy Lily | 2577 | Limited |
| Fiber Jar | 2538 | Limited |
| Nobleman of Crossout | 2500 | Semi-Limited |
| Sinister Serpent | 2385 | Limited |
| Mystic Tomato | 2192 |  |
| Painful Choice | 2192 | Limited |
| Yata-Garasu | 2192 | Limited |
| Torrential Tribute | 2115 |  |
| Gravekeeper's Spy | 1449 |  |
| Gemini Elf | 1346 |  |
| Ceasefire | 1192 | Limited |
| Premature Burial | 1192 | Limited |
| Don Zaloog | 1154 |  |
| Spirit Reaper | 1115 |  |
| Reinforcement of the Army | 1058 | Semi-Limited |
| Dust Tornado | 1013 |  |
| Zombyra the Dark | 1000 |  |
| Magician of Faith | 897 |  |
| Morphing Jar | 769 | Limited |
| Scapegoat | 769 |  |
| King Tiger Wanghu | 718 |  |
| Magic Cylinder | 692 | Limited |
| Creature Swap | 654 | Semi-Limited |
| Book of Moon | 577 |  |
| Cyber Jar | 500 | Limited |
| Fissure | 500 |  |
| Kycoo the Ghost Destroyer | 487 |  |
| Electric Snake | 436 |  |
| Bazoo the Soul-Eater | 385 |  |
| Jar of Greed | 372 |  |
| Blast with Chain | 308 |  |
| Dark Ruler Ha Des | 308 |  |
| Gearfried the Iron Knight | 308 |  |
| Asura Priest | 295 |  |
| Metamorphosis | 295 |  |
| Newdoria | 269 |  |
| Chain Energy | 231 |  |
| Solemn Judgment | 218 |  |
| Thousand-Eyes Restrict | 214 |  |
| Solemn Wishes | 205 |  |
| 4-Starred Ladybug of Doom | 200 |  |
| Airknight Parshath | 200 |  |
| Amazoness Archer | 200 |  |
| B. Skull Dragon | 200 |  |
| Barrel Dragon | 200 |  |
| Bickuribox | 200 |  |
| Book of Life | 200 |  |
| Cannon Soldier | 200 |  |
| Card Destruction | 200 | Limited |
| Charubin the Fire Knight | 200 |  |
| Cold Wave | 200 |  |
| Cure Mermaid | 200 |  |
| Cyber Saurus | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Darkfire Dragon | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Empress Judge | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flower Wolf | 200 |  |
| Forced Requisition | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Giant Germ | 200 |  |
| Giant Rat | 200 |  |
| Giant Trunade | 200 |  |
| Giltia the D. Knight | 200 |  |
| Goblin Attack Force | 200 |  |
| Graverobber | 200 |  |
| Guardian Sphinx | 200 |  |
| Just Desserts | 200 |  |
| Karbonala Warrior | 200 |  |
| Kuriboh | 200 |  |
| Labyrinth Tank | 200 |  |
| Last Will | 200 |  |
| Limiter Removal | 200 |  |
| Magic Drain | 200 |  |
| Magic Jammer | 200 |  |
| Marauding Captain | 200 | Semi-Limited |
| Mask of Darkness | 200 |  |
| Masked Sorcerer | 200 |  |
| Mechanicalchaser | 200 |  |
| Metal Dragon | 200 |  |
| Mirror Wall | 200 |  |
| Morphing Jar #2 | 200 | Semi-Limited |
| Musician King | 200 |  |
| Nightmare Wheel | 200 |  |
| Nimble Momonga | 200 |  |
| Princess of Tsurugi | 200 |  |
| Punished Eagle | 200 |  |
| Pyramid Turtle | 200 |  |
| Raigeki Break | 200 |  |
| Reaper on the Nightmare | 200 |  |
| Reasoning | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Ryu Senshi | 200 |  |
| Sasuke Samurai | 200 |  |
| Seven Tools of the Bandit | 200 |  |
| Skull Knight | 200 |  |
| Spear Dragon | 200 |  |
| St. Joan | 200 |  |
| Swords of Revealing Light | 200 | Limited |
| The Last Warrior from Another Planet | 200 |  |
| Thousand Dragon | 200 |  |
| Time Seal | 200 |  |
| Trap Dustshoot | 200 |  |
| Trap Hole | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| United We Stand | 200 | Limited |
| Upstart Goblin | 200 | Limited |
| Waboku | 200 |  |
| Warrior of Tradition | 200 |  |
| White Magical Hat | 200 |  |

### Scientist

| Card | DP | Limit |
|---|---:|---|
| Bell of Destruction | 3000 | Limited |
| Change of Heart | 3000 | Limited |
| Dark Hole | 3000 | Limited |
| Graceful Charity | 3000 | Limited |
| Harpie's Feather Duster | 3000 | Limited |
| Imperial Order | 3000 | Limited |
| Mystical Space Typhoon | 3000 |  |
| Painful Choice | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Sangan | 3000 | Limited |
| Snatch Steal | 3000 | Limited |
| Witch of the Black Forest | 3000 | Limited |
| Call Of The Haunted | 2750 | Limited |
| Fiber Jar | 2750 | Limited |
| Mirror Force | 2750 | Limited |
| Monster Reborn | 2750 | Limited |
| Tribe-Infecting Virus | 2750 | Limited |
| Magical Scientist | 2583 |  |
| Breaker the Magical Warrior | 2500 | Limited |
| Mirage of Nightmare | 2500 | Limited |
| Torrential Tribute | 2500 |  |
| Confiscation | 2250 | Limited |
| Delinquent Duo | 2250 | Limited |
| Exiled Force | 2250 | Limited |
| Jinzo | 2250 | Limited |
| Sinister Serpent | 2250 | Limited |
| Injection Fairy Lily | 2000 | Limited |
| The Forceful Sentry | 2000 | Limited |
| Mystic Tomato | 1917 |  |
| Heavy Storm | 1750 | Limited |
| Yata-Garasu | 1750 | Limited |
| King Tiger Wanghu | 1500 |  |
| Nobleman of Crossout | 1500 | Semi-Limited |
| Premature Burial | 1500 | Limited |
| Last Will | 1333 |  |
| Catapult Turtle | 1250 |  |
| Card Destruction | 1000 | Limited |
| Ceasefire | 1000 | Limited |
| Gilasaurus | 1000 |  |
| Metamorphosis | 1000 |  |
| Book of Moon | 833 |  |
| Dust Tornado | 833 |  |
| Gravekeeper's Spy | 833 |  |
| Trap Dustshoot | 833 |  |
| Cannon Soldier | 667 |  |
| Poison of the Old Man | 583 |  |
| Reasoning | 583 |  |
| Cyber Jar | 500 | Limited |
| Magician of Faith | 500 |  |
| Spirit Reaper | 500 |  |
| Jowgen the Spiritualist | 417 |  |
| Scapegoat | 417 |  |
| Newdoria | 333 |  |
| Spiritualism | 333 |  |
| Airknight Parshath | 250 |  |
| Apprentice Magician | 250 |  |
| Creature Swap | 250 | Semi-Limited |
| Fissure | 250 |  |
| Giant Trunade | 250 |  |
| Jar of Greed | 250 |  |
| Nimble Momonga | 250 |  |
| Raigeki Break | 250 |  |
| Reinforcement of the Army | 250 | Semi-Limited |
| Royal Decree | 250 |  |
| Skilled Dark Magician | 250 |  |
| Swords of Revealing Light | 250 | Limited |
| Upstart Goblin | 250 | Limited |
| Waboku | 250 |  |
| Roaring Ocean Snake | 233 |  |
| Punished Eagle | 217 |  |
| Thousand-Eyes Restrict | 217 |  |
| Dark Balter the Terrible | 208 |  |
| Amazoness Archer | 200 |  |
| Asura Priest | 200 |  |
| Dark Magician | 200 |  |
| Dark Paladin | 200 |  |
| Dark Ruler Ha Des | 200 |  |
| Darkfire Dragon | 200 |  |
| Don Zaloog | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Empress Judge | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Goblin Attack Force | 200 |  |
| Kycoo the Ghost Destroyer | 200 |  |
| Mask of Restrict | 200 |  |
| Offerings to the Doomed | 200 |  |
| Reaper on the Nightmare | 200 |  |
| Ryu Senshi | 200 |  |
| Sasuke Samurai | 200 |  |
| Spell Canceller | 200 |  |
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
| Bell of Destruction | 2870 | Limited |
| Confiscation | 2870 | Limited |
| Exiled Force | 2870 | Limited |
| Imperial Order | 2870 | Limited |
| Mystical Space Typhoon | 2870 |  |
| Snatch Steal | 2870 | Limited |
| Call Of The Haunted | 2739 | Limited |
| Magical Scientist | 2739 | Limited |
| Mirage of Nightmare | 2739 | Limited |
| Sinister Serpent | 2739 | Limited |
| Tribe-Infecting Virus | 2739 | Limited |
| D.D. Warrior Lady | 2652 |  |
| Delinquent Duo | 2609 | Limited |
| Fiber Jar | 2609 | Limited |
| Jinzo | 2609 | Limited |
| Nobleman of Crossout | 2543 | Semi-Limited |
| Heavy Storm | 2478 | Limited |
| Reinforcement of the Army | 2478 | Semi-Limited |
| Injection Fairy Lily | 2348 | Limited |
| Torrential Tribute | 2304 |  |
| Yata-Garasu | 2087 | Limited |
| Gravekeeper's Spy | 1826 |  |
| Painful Choice | 1826 | Limited |
| Premature Burial | 1304 | Limited |
| Sakuretsu Armor | 1174 |  |
| Royal Decree | 1087 |  |
| King Tiger Wanghu | 913 |  |
| Don Zaloog | 870 |  |
| Magician of Faith | 783 |  |
| Zombyra the Dark | 783 |  |
| Book of Moon | 739 |  |
| Ceasefire | 652 | Limited |
| Dust Tornado | 609 |  |
| Scapegoat | 478 |  |
| Mystic Tomato | 435 |  |
| Cyber Jar | 391 | Limited |
| Metamorphosis | 391 |  |
| Poison of the Old Man | 348 |  |
| Sasuke Samurai | 304 |  |
| Thousand-Eyes Restrict | 270 |  |
| Card Destruction | 261 | Limited |
| Just Desserts | 261 |  |
| Magic Cylinder | 261 | Limited |
| Solemn Judgment | 261 |  |
| Trap Dustshoot | 261 |  |
| Ryu Senshi | 243 |  |
| Dark Balter the Terrible | 235 |  |
| Amazoness Archer | 200 |  |
| B. Skull Dragon | 200 |  |
| Book of Life | 200 |  |
| Bottomless Trap Hole | 200 |  |
| Cannon Soldier | 200 |  |
| Catapult Turtle | 200 |  |
| Cipher Soldier | 200 |  |
| Cold Wave | 200 |  |
| Creature Swap | 200 | Semi-Limited |
| Dark Flare Knight | 200 |  |
| Dark Ruler Ha Des | 200 |  |
| Dark Scorpion - Cliff the Trap Remover | 200 |  |
| Darkfire Dragon | 200 |  |
| Des Koala | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Electric Snake | 200 |  |
| Empress Judge | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Flame Ghost | 200 |  |
| Gemini Elf | 200 |  |
| Giant Trunade | 200 |  |
| Giltia the D. Knight | 200 |  |
| Goblin Attack Force | 200 |  |
| Guardian Sphinx | 200 |  |
| Gyaku-Gire Panda | 200 |  |
| Jar of Greed | 200 |  |
| Kycoo the Ghost Destroyer | 200 |  |
| Last Will | 200 |  |
| Mage Power | 200 | Limited |
| Marauding Captain | 200 | Semi-Limited |
| Masked Sorcerer | 200 |  |
| Morphing Jar | 200 | Semi-Limited |
| Mother Grizzly | 200 |  |
| Ojama Trio | 200 |  |
| Punished Eagle | 200 |  |
| Pyramid Turtle | 200 |  |
| Raigeki Break | 200 |  |
| Reaper on the Nightmare | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Secret Barrel | 200 |  |
| Skull Knight | 200 |  |
| Slate Warrior | 200 |  |
| Spirit Reaper | 200 |  |
| St. Joan | 200 |  |
| Summoned Skull | 200 |  |
| Swords of Revealing Light | 200 | Limited |
| The Last Warrior from Another Planet | 200 |  |
| The Little Swordsman of Aile | 200 |  |
| Time Seal | 200 |  |
| Trap Hole | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Upstart Goblin | 200 | Limited |
| Vampire Lord | 200 | Limited |
| Waboku | 200 |  |
| White Magical Hat | 200 |  |

### Chaos

| Card | DP | Limit |
|---|---:|---|
| Graceful Charity | 3000 | Limited |
| Imperial Order | 3000 | Limited |
| Monster Reborn | 3000 | Limited |
| Mystical Space Typhoon | 3000 |  |
| Pot of Greed | 3000 | Limited |
| Raigeki | 3000 | Limited |
| Bell of Destruction | 2966 | Limited |
| Black Luster Soldier - Envoy of Beginning | 2966 | Limited |
| Change of Heart | 2966 | Limited |
| Chaos Emperor Dragon - Envoy of the End | 2966 | Limited |
| Harpie's Feather Duster | 2931 | Limited |
| Witch of the Black Forest | 2931 | Limited |
| The Forceful Sentry | 2897 | Limited |
| Dark Hole | 2862 | Limited |
| Painful Choice | 2862 | Limited |
| Tribe-Infecting Virus | 2862 | Limited |
| Snatch Steal | 2828 | Limited |
| Mirror Force | 2793 | Limited |
| Breaker the Magical Warrior | 2724 | Limited |
| Delinquent Duo | 2724 | Limited |
| Confiscation | 2690 | Limited |
| Sangan | 2690 | Limited |
| Mirage of Nightmare | 2655 | Limited |
| Sinister Serpent | 2483 | Limited |
| Call Of The Haunted | 2414 | Limited |
| Nobleman of Crossout | 2328 | Semi-Limited |
| D.D. Warrior Lady | 2276 |  |
| Fiber Jar | 2138 | Limited |
| Kycoo the Ghost Destroyer | 2023 |  |
| Torrential Tribute | 1943 |  |
| Heavy Storm | 1931 | Limited |
| Premature Burial | 1828 | Limited |
| Jinzo | 1655 | Limited |
| Magical Scientist | 1621 | Limited |
| Gravekeeper's Spy | 1471 |  |
| Magician of Faith | 1391 |  |
| Injection Fairy Lily | 1207 | Limited |
| Thunder Dragon | 1138 |  |
| Yata-Garasu | 1069 | Limited |
| Trap Dustshoot | 1057 |  |
| Card Destruction | 966 | Limited |
| Exiled Force | 931 | Limited |
| Cyber Jar | 621 | Limited |
| Raigeki Break | 506 |  |
| Morphing Jar | 500 | Semi-Limited |
| Reinforcement of the Army | 500 | Semi-Limited |
| Upstart Goblin | 448 | Limited |
| Sakuretsu Armor | 437 |  |
| Dark Magician of Chaos | 425 |  |
| Royal Decree | 425 |  |
| Solemn Judgment | 402 |  |
| Scapegoat | 368 |  |
| Cold Wave | 356 |  |
| Poison of the Old Man | 333 |  |
| Magic Cylinder | 310 | Limited |
| Waboku | 264 |  |
| Zombyra the Dark | 264 |  |
| Thousand-Eyes Restrict | 259 |  |
| Ceasefire | 241 | Limited |
| Asura Priest | 230 |  |
| Dust Tornado | 207 |  |
| Metamorphosis | 207 |  |
| Spirit Reaper | 207 |  |
| 4-Starred Ladybug of Doom | 200 |  |
| Airknight Parshath | 200 |  |
| Amazoness Archer | 200 |  |
| Axe of Despair | 200 |  |
| B. Skull Dragon | 200 |  |
| Berserk Gorilla | 200 |  |
| Bickuribox | 200 |  |
| Black Illusion Ritual | 200 |  |
| Book of Moon | 200 |  |
| Bottomless Trap Hole | 200 |  |
| Catapult Turtle | 200 |  |
| Chaos Sorcerer | 200 |  |
| Cipher Soldier | 200 |  |
| Creature Swap | 200 | Semi-Limited |
| Cursed Seal of the Forbidden Spell | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Dark Flare Knight | 200 |  |
| Dark Paladin | 200 |  |
| Dark Ruler Ha Des | 200 |  |
| Darkfire Dragon | 200 |  |
| Deepsea Shark | 200 |  |
| Dimension Fusion | 200 |  |
| Don Zaloog | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Electric Snake | 200 |  |
| Emergency Provisions | 200 |  |
| Empress Judge | 200 |  |
| Enemy Controller | 200 |  |
| Exchange | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Giant Trunade | 200 |  |
| Gigantes | 200 |  |
| Giltia the D. Knight | 200 |  |
| Goblin Attack Force | 200 |  |
| Hysteric Fairy | 200 |  |
| Jar of Greed | 200 |  |
| Karbonala Warrior | 200 |  |
| King Tiger Wanghu | 200 |  |
| Kuriboh | 200 |  |
| Labyrinth Tank | 200 |  |
| Magic Jammer | 200 |  |
| Magical Merchant | 200 |  |
| Masked Sorcerer | 200 |  |
| Metal Dragon | 200 |  |
| Musician King | 200 |  |
| Mystic Tomato | 200 |  |
| Prohibition | 200 |  |
| Punished Eagle | 200 |  |
| Pyramid Turtle | 200 |  |
| Reaper on the Nightmare | 200 |  |
| Relinquished | 200 |  |
| Return from the Different Dimension | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Roulette Barrel | 200 |  |
| Royal Oppression | 200 |  |
| Ryu Senshi | 200 |  |
| Sasuke Samurai | 200 |  |
| Skilled Dark Magician | 200 |  |
| Skull Knight | 200 |  |
| Soul Release | 200 |  |
| Spear Dragon | 200 |  |
| Spell Canceller | 200 |  |
| Spiritualism | 200 |  |
| St. Joan | 200 |  |
| Swords of Revealing Light | 200 | Limited |
| The Last Warrior from Another Planet | 200 |  |
| The Shallow Grave | 200 |  |
| Thunder Nyan Nyan | 200 |  |
| Time Seal | 200 |  |
| Tsukuyomi | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Vampire Lord | 200 | Limited |
| Warrior of Tradition | 200 |  |
| White Magical Hat | 200 |  |
| Zaborg the Thunder Monarch | 200 |  |

### Warrior

| Card | DP | Limit |
|---|---:|---|
| Pot of Greed | 3000 | Limited |
| Heavy Storm | 2921 | Limited |
| Bell of Destruction | 2882 | Limited |
| Change of Heart | 2803 | Limited |
| Mystical Space Typhoon | 2803 | Limited |
| Snatch Steal | 2763 | Limited |
| Premature Burial | 2684 | Limited |
| Breaker the Magical Warrior | 2645 | Limited |
| Torrential Tribute | 2645 | Limited |
| The Forceful Sentry | 2605 | Limited |
| Tribe-Infecting Virus | 2566 | Limited |
| Confiscation | 2526 | Limited |
| Call Of The Haunted | 2487 | Limited |
| Painful Choice | 2447 | Limited |
| Nobleman of Crossout | 2368 | Semi-Limited |
| Black Luster Soldier - Envoy of Beginning | 2329 | Limited |
| D.D. Warrior Lady | 1987 |  |
| Magical Scientist | 1974 | Limited |
| Jinzo | 1895 | Limited |
| Exiled Force | 1697 | Limited |
| Sinister Serpent | 1618 | Limited |
| Fiber Jar | 1579 | Limited |
| Sakuretsu Armor | 1579 |  |
| Dust Tornado | 1539 |  |
| Reinforcement of the Army | 1441 | Semi-Limited |
| Swords of Revealing Light | 1421 | Limited |
| Mirage of Nightmare | 1342 | Limited |
| Scapegoat | 1197 |  |
| Cipher Soldier | 1039 |  |
| Mobius the Frost Monarch | 1013 |  |
| Kycoo the Ghost Destroyer | 987 |  |
| Book of Moon | 908 |  |
| Smashing Ground | 908 |  |
| Blade Knight | 829 |  |
| Dark Magician of Chaos | 789 | Limited |
| Gravekeeper's Spy | 750 |  |
| Magic Cylinder | 750 | Limited |
| Solemn Judgment | 737 |  |
| Trap Dustshoot | 737 |  |
| Injection Fairy Lily | 711 | Limited |
| Creature Swap | 691 | Semi-Limited |
| Ceasefire | 671 | Limited |
| King Tiger Wanghu | 658 |  |
| Magician of Faith | 658 |  |
| Bottomless Trap Hole | 645 |  |
| Royal Decree | 566 |  |
| Card Destruction | 513 | Limited |
| Giant Trunade | 513 |  |
| Mystic Swordsman LV2 | 513 |  |
| Upstart Goblin | 474 | Limited |
| Metamorphosis | 461 |  |
| Raigeki Break | 461 |  |
| Morphing Jar | 434 | Limited |
| Berserk Gorilla | 408 |  |
| D.D. Assailant | 408 |  |
| Don Zaloog | 382 |  |
| Mystik Wok | 382 |  |
| Airknight Parshath | 368 |  |
| Jar of Greed | 368 |  |
| Monster Gate | 368 |  |
| Emergency Provisions | 355 |  |
| Reasoning | 355 |  |
| Chaos Sorcerer | 329 |  |
| Enemy Controller | 329 |  |
| Cyber Jar | 316 | Limited |
| Pikeru's Circle of Enchantment | 289 |  |
| Asura Priest | 250 |  |
| Zombyra the Dark | 250 |  |
| Monster Reincarnation | 237 |  |
| Ninja Grandmaster Sasuke | 224 |  |
| Tsukuyomi | 224 |  |
| Thousand-Eyes Restrict | 217 |  |
| Spirit Reaper | 211 |  |
| Abyss Soldier | 200 |  |
| Amazoness Archer | 200 |  |
| Apprentice Magician | 200 |  |
| Axe of Despair | 200 |  |
| Big Bang Shot | 200 |  |
| Big Shield Gardna | 200 |  |
| Blast with Chain | 200 |  |
| Buster Blader | 200 |  |
| Cannon Soldier | 200 |  |
| Catapult Turtle | 200 |  |
| Chain Disappearance | 200 |  |
| Chain Energy | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Compulsory Evacuation Device | 200 |  |
| Cursed Seal of the Forbidden Spell | 200 |  |
| Cyber Saurus | 200 |  |
| Cyber-Stein | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Dark Blade the Dragon Knight | 200 |  |
| Dark Flare Knight | 200 |  |
| Dark Magician | 200 |  |
| Dark Paladin | 200 |  |
| Dark Scorpion - Cliff the Trap Remover | 200 |  |
| Darkfire Dragon | 200 |  |
| Dekoichi the Battlechanted Locomotive | 200 |  |
| Des Koala | 200 |  |
| Different Dimension Capsule | 200 |  |
| Dimension Fusion | 200 |  |
| Divine Wrath | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Ekibyo Drakmord | 200 |  |
| Empress Judge | 200 |  |
| Exchange | 200 |  |
| Exile of the Wicked | 200 |  |
| Fairy Box | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Final Attack Orders | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Fusion Gate | 200 |  |
| Fusionist | 200 |  |
| Gaia Power | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Gearfried the Iron Knight | 200 |  |
| Giant Rat | 200 |  |
| Gigantes | 200 |  |
| Gilasaurus | 200 |  |
| Giltia the D. Knight | 200 |  |
| Goblin Attack Force | 200 |  |
| Gravity Bind | 200 |  |
| Greenkappa | 200 |  |
| Gyaku-Gire Panda | 200 |  |
| Hallowed Life Barrier | 200 |  |
| Jowgen the Spiritualist | 200 |  |
| Just Desserts | 200 |  |
| Karbonala Warrior | 200 |  |
| Kuriboh | 200 |  |
| Labyrinth Tank | 200 |  |
| Last Will | 200 |  |
| Level Limit - Area B | 200 |  |
| Light of Intervention | 200 |  |
| Magic Drain | 200 |  |
| Magic Jammer | 200 |  |
| Magical Merchant | 200 |  |
| Marauding Captain | 200 | Semi-Limited |
| Mask of Darkness | 200 |  |
| Mask of Restrict | 200 |  |
| Masked Sorcerer | 200 |  |
| Master of Oz | 200 |  |
| Messenger of Peace | 200 |  |
| Mirror Wall | 200 |  |
| Mother Grizzly | 200 |  |
| Musician King | 200 |  |
| Mystic Plasma Zone | 200 |  |
| Mystic Tomato | 200 |  |
| Newdoria | 200 |  |
| Night Assailant | 200 |  |
| Nimble Momonga | 200 |  |
| Nobleman of Extermination | 200 |  |
| Ojama King | 200 |  |
| Ojama Trio | 200 |  |
| Poison of the Old Man | 200 |  |
| Princess of Tsurugi | 200 |  |
| Prohibition | 200 |  |
| Punished Eagle | 200 |  |
| Pyramid Turtle | 200 |  |
| Rabid Horseman | 200 |  |
| Reaper on the Nightmare | 200 |  |
| Return from the Different Dimension | 200 |  |
| Revival Jam | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Robbin' Goblin | 200 |  |
| Royal Oppression | 200 |  |
| Ryu Senshi | 200 |  |
| Sacred Crane | 200 |  |
| Sasuke Samurai | 200 |  |
| Secret Barrel | 200 |  |
| Seven Tools of the Bandit | 200 |  |
| Skilled Dark Magician | 200 |  |
| Slate Warrior | 200 |  |
| Soul Exchange | 200 |  |
| Spear Dragon | 200 |  |
| Spell Canceller | 200 |  |
| St. Joan | 200 |  |
| Stealth Bird | 200 |  |
| Swarm of Locusts | 200 |  |
| The Fiend Megacyber | 200 |  |
| The Last Warrior from Another Planet | 200 |  |
| Token Thanksgiving | 200 |  |
| Trap Hole | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Vampire Lord | 200 | Limited |
| Waboku | 200 |  |
| Wave-Motion Cannon | 200 |  |
| Widespread Ruin | 200 |  |
| Windstorm of Etaqua | 200 |  |
| Xing Zhen Hu | 200 |  |
| Zaborg the Thunder Monarch | 200 |  |

### Goat

| Card | DP | Limit |
|---|---:|---|
| Pot of Greed | 2999 | Limited |
| Graceful Charity | 2997 | Limited |
| Heavy Storm | 2962 | Limited |
| Bell of Destruction | 2928 | Limited |
| Snatch Steal | 2916 | Limited |
| Mirror Force | 2860 | Limited |
| Nobleman of Crossout | 2817 | Semi-Limited |
| Breaker the Magical Warrior | 2813 | Limited |
| Mystical Space Typhoon | 2811 | Limited |
| Delinquent Duo | 2730 | Limited |
| Black Luster Soldier - Envoy of Beginning | 2631 | Limited |
| Tribe-Infecting Virus | 2627 | Limited |
| Torrential Tribute | 2426 | Limited |
| Sangan | 2231 | Limited |
| Sinister Serpent | 1889 | Limited |
| Sakuretsu Armor | 1886 |  |
| Trap Dustshoot | 1734 |  |
| Gravekeeper's Spy | 1614 |  |
| Thunder Dragon | 1511 |  |
| Solemn Judgment | 1468 |  |
| Magician of Faith | 1444 |  |
| Premature Burial | 1409 | Limited |
| Upstart Goblin | 1355 | Semi-Limited |
| D.D. Warrior Lady | 1313 | Limited |
| Call Of The Haunted | 1188 | Limited |
| Exiled Force | 1176 | Limited |
| Book of Moon | 1174 |  |
| Dekoichi the Battlechanted Locomotive | 1141 |  |
| Chaos Sorcerer | 1135 |  |
| Mind Control | 1132 |  |
| Swords of Revealing Light | 1132 | Limited |
| Dust Tornado | 1129 |  |
| Kycoo the Ghost Destroyer | 1120 |  |
| Card Destruction | 1023 | Limited |
| Night Assailant | 1015 | Semi-Limited |
| Reinforcement of the Army | 940 | Semi-Limited |
| Jar of Greed | 937 |  |
| Raigeki Break | 831 |  |
| Scapegoat | 785 |  |
| Blade Knight | 780 |  |
| Mystik Wok | 775 |  |
| Tsukuyomi | 741 |  |
| Zombyra the Dark | 690 |  |
| Metamorphosis | 672 |  |
| Mobius the Frost Monarch | 641 |  |
| Morphing Jar | 546 | Limited |
| Royal Decree | 515 |  |
| Jinzo | 474 | Limited |
| Asura Priest | 449 |  |
| Cipher Soldier | 432 |  |
| Creature Swap | 380 | Semi-Limited |
| Mystic Swordsman LV2 | 369 |  |
| King Tiger Wanghu | 365 |  |
| Cyber Jar | 362 | Limited |
| Injection Fairy Lily | 332 | Limited |
| Giant Trunade | 326 |  |
| Lightning Vortex | 325 | Limited |
| Phoenix Wing Wind Blast | 318 |  |
| Smashing Ground | 293 |  |
| Ninja Grandmaster Sasuke | 288 |  |
| Ceasefire | 277 | Limited |
| Berserk Gorilla | 270 |  |
| Don Zaloog | 242 |  |
| Thousand-Eyes Restrict | 228 |  |
| Spirit Reaper | 211 |  |
| Abyss Soldier | 200 | Semi-Limited |
| Acid Trap Hole | 200 |  |
| Airknight Parshath | 200 |  |
| Amazoness Archer | 200 |  |
| Apprentice Magician | 200 |  |
| Aqua Spirit | 200 |  |
| Axe of Despair | 200 |  |
| B. Skull Dragon | 200 |  |
| Backup Soldier | 200 |  |
| Banisher of the Light | 200 |  |
| Bazoo the Soul-Eater | 200 |  |
| Bickuribox | 200 |  |
| Big Bang Shot | 200 |  |
| Big Shield Gardna | 200 |  |
| Black Illusion Ritual | 200 |  |
| Black Pendant | 200 |  |
| Blast with Chain | 200 |  |
| Book of Life | 200 |  |
| Bottomless Trap Hole | 200 |  |
| Brain Control | 200 |  |
| Cannon Soldier | 200 |  |
| Card of Safe Return | 200 |  |
| Catapult Turtle | 200 |  |
| Chain Disappearance | 200 |  |
| Chain Energy | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Chiron the Mage | 200 |  |
| Cold Wave | 200 |  |
| Compulsory Evacuation Device | 200 |  |
| Cursed Seal of the Forbidden Spell | 200 |  |
| Cyber Saurus | 200 |  |
| Cyber-Stein | 200 |  |
| D.D. Assailant | 200 |  |
| D.D. Survivor | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Dark Blade the Dragon Knight | 200 |  |
| Dark Flare Knight | 200 |  |
| Dark Magician | 200 |  |
| Dark Magician of Chaos | 200 | Limited |
| Dark Mimic LV1 | 200 |  |
| Dark Necrofear | 200 |  |
| Dark Paladin | 200 |  |
| Dark Ruler Ha Des | 200 |  |
| Dark Scorpion - Cliff the Trap Remover | 200 |  |
| Darkfire Dragon | 200 |  |
| Deepsea Shark | 200 |  |
| Des Koala | 200 |  |
| Des Lacooda | 200 |  |
| Des Wombat | 200 |  |
| Different Dimension Capsule | 200 |  |
| Dimension Fusion | 200 |  |
| Divine Wrath | 200 |  |
| Dragon Zombie | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Dream Clown | 200 |  |
| Ekibyo Drakmord | 200 |  |
| Electric Snake | 200 |  |
| Emergency Provisions | 200 | Semi-Limited |
| Empress Judge | 200 |  |
| Enemy Controller | 200 |  |
| Exchange | 200 |  |
| Exodia the Forbidden One | 200 | Limited |
| Fake Trap | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Final Attack Orders | 200 |  |
| Fire Princess | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flower Wolf | 200 |  |
| Fusionist | 200 |  |
| Gaia Power | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Gatling Dragon | 200 |  |
| Gearfried the Iron Knight | 200 |  |
| Giant Germ | 200 |  |
| Giant Rat | 200 |  |
| Giant Soldier of Stone | 200 |  |
| Gigantes | 200 |  |
| Gilasaurus | 200 |  |
| Giltia the D. Knight | 200 |  |
| Goblin Attack Force | 200 |  |
| Golem Sentry | 200 |  |
| Granmarg the Rock Monarch | 200 |  |
| Graverobber | 200 |  |
| Gravity Bind | 200 | Semi-Limited |
| Greenkappa | 200 |  |
| Guardian Sphinx | 200 |  |
| Gyaku-Gire Panda | 200 |  |
| Hallowed Life Barrier | 200 |  |
| Hiro's Shadow Scout | 200 |  |
| Humanoid Worm Drake | 200 |  |
| Hysteric Fairy | 200 |  |
| Interdimensional Matter Transporter | 200 |  |
| Jirai Gumo | 200 |  |
| Jowgen the Spiritualist | 200 |  |
| Just Desserts | 200 |  |
| Kaminari Attack | 200 |  |
| Karbonala Warrior | 200 |  |
| King Dragun | 200 |  |
| Kuriboh | 200 |  |
| Kuwagata a | 200 |  |
| Kwagar Hercules | 200 |  |
| Labyrinth Tank | 200 |  |
| Last Will | 200 |  |
| Left Arm of the Forbidden One | 200 | Limited |
| Left Leg of the Forbidden One | 200 | Limited |
| Level Limit - Area B | 200 | Semi-Limited |
| Light of Intervention | 200 |  |
| Lightforce Sword | 200 |  |
| Limiter Removal | 200 |  |
| Mage Power | 200 | Limited |
| Magic Cylinder | 200 | Limited |
| Magic Drain | 200 |  |
| Magic Jammer | 200 |  |
| Magical Merchant | 200 |  |
| Marauding Captain | 200 | Semi-Limited |
| Mask of Brutality | 200 |  |
| Mask of Darkness | 200 |  |
| Mask of Restrict | 200 |  |
| Master of Oz | 200 |  |
| Megamorph | 200 |  |
| Messenger of Peace | 200 |  |
| Metal Dragon | 200 |  |
| Milus Radiant | 200 |  |
| Minar | 200 |  |
| Mirror Wall | 200 |  |
| Monster Gate | 200 |  |
| Monster Reincarnation | 200 |  |
| Morphing Jar #2 | 200 |  |
| Mother Grizzly | 200 |  |
| Musician King | 200 |  |
| My Body as a Shield | 200 |  |
| Mystic Tomato | 200 |  |
| Newdoria | 200 |  |
| Nightmare Wheel | 200 |  |
| Nimble Momonga | 200 |  |
| Nobleman of Extermination | 200 |  |
| Offerings to the Doomed | 200 |  |
| Ojama King | 200 |  |
| Ojama Trio | 200 |  |
| Old Vindictive Magician | 200 |  |
| Penguin Soldier | 200 |  |
| Pikeru's Circle of Enchantment | 200 |  |
| Poison of the Old Man | 200 |  |
| Prohibition | 200 |  |
| Pyramid Turtle | 200 |  |
| Rabid Horseman | 200 |  |
| Reaper on the Nightmare | 200 |  |
| Reasoning | 200 |  |
| Relinquished | 200 |  |
| Restructer Revolution | 200 |  |
| Return from the Different Dimension | 200 |  |
| Right Arm of the Forbidden One | 200 | Limited |
| Right Leg of the Forbidden One | 200 | Limited |
| Roaring Ocean Snake | 200 |  |
| Robbin' Goblin | 200 |  |
| Roulette Barrel | 200 |  |
| Royal Command | 200 |  |
| Royal Oppression | 200 |  |
| Rush Recklessly | 200 |  |
| Ryu Kokki | 200 |  |
| Ryu Senshi | 200 |  |
| Sacred Crane | 200 |  |
| Secret Barrel | 200 |  |
| Senju of the Thousand Hands | 200 |  |
| Seven Tools of the Bandit | 200 |  |
| Shield & Sword | 200 |  |
| Skilled Dark Magician | 200 |  |
| Skull Knight | 200 |  |
| Skull Lair | 200 |  |
| Slate Warrior | 200 |  |
| Solemn Wishes | 200 |  |
| Sonic Bird | 200 |  |
| Soul Exchange | 200 |  |
| Soul Release | 200 |  |
| Spear Dragon | 200 |  |
| Spell Canceller | 200 |  |
| Spirit Message "A" | 200 |  |
| Spirit Message "L" | 200 |  |
| Spirit Message "N" | 200 |  |
| St. Joan | 200 |  |
| Stealth Bird | 200 |  |
| Summoner of Illusions | 200 |  |
| Swarm of Locusts | 200 |  |
| Swift Gaia the Fierce Knight | 200 |  |
| The Fiend Megacyber | 200 |  |
| The Forgiving Maiden | 200 |  |
| The Last Warrior from Another Planet | 200 |  |
| The Little Swordsman of Aile | 200 |  |
| The Rock Spirit | 200 |  |
| The Shallow Grave | 200 |  |
| The Wicked Worm Beast | 200 |  |
| Thestalos the Firestorm Monarch | 200 |  |
| Thousand Dragon | 200 |  |
| Threatening Roar | 200 |  |
| Thunder Nyan Nyan | 200 |  |
| Time Seal | 200 |  |
| Time Wizard | 200 |  |
| Token Thanksgiving | 200 |  |
| Toon Cannon Soldier | 200 |  |
| Toon Table of Contents | 200 |  |
| Toon World | 200 |  |
| Trap Hole | 200 |  |
| Trap Master | 200 |  |
| Tribute to The Doomed | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| Two-Pronged Attack | 200 |  |
| Ultimate Offering | 200 |  |
| United We Stand | 200 | Limited |
| Vampire Lord | 200 | Semi-Limited |
| Waboku | 200 |  |
| Warrior of Tradition | 200 |  |
| Wave-Motion Cannon | 200 |  |
| Widespread Ruin | 200 |  |
| Windstorm of Etaqua | 200 |  |
| Xing Zhen Hu | 200 |  |
| Zaborg the Thunder Monarch | 200 |  |

### Cyber

| Card | DP | Limit |
|---|---:|---|
| Graceful Charity | 3000 | Limited |
| Heavy Storm | 3000 | Limited |
| Pot of Greed | 3000 | Limited |
| Mirror Force | 2933 | Limited |
| Bell of Destruction | 2867 | Limited |
| Snatch Steal | 2867 | Limited |
| Torrential Tribute | 2867 | Limited |
| Mystical Space Typhoon | 2733 | Limited |
| Nobleman of Crossout | 2700 | Semi-Limited |
| Delinquent Duo | 2667 | Limited |
| Black Luster Soldier - Envoy of Beginning | 2600 | Limited |
| Premature Burial | 2533 | Limited |
| Breaker the Magical Warrior | 2467 | Limited |
| Sangan | 2133 | Limited |
| Tribe-Infecting Virus | 2133 | Limited |
| Cyber Dragon | 2044 |  |
| D.D. Warrior Lady | 2000 | Limited |
| Call Of The Haunted | 1933 | Limited |
| Trap Dustshoot | 1911 |  |
| Exiled Force | 1800 | Limited |
| Book of Moon | 1467 |  |
| Jinzo | 1467 | Limited |
| Sinister Serpent | 1400 | Limited |
| Magician of Faith | 1311 |  |
| Threatening Roar | 1311 |  |
| Cyber-Stein | 1178 |  |
| Sakuretsu Armor | 1156 |  |
| Lightning Vortex | 1133 | Limited |
| Mobius the Frost Monarch | 1089 |  |
| Scapegoat | 1089 |  |
| Metamorphosis | 1000 |  |
| Royal Decree | 978 |  |
| Swords of Revealing Light | 933 | Limited |
| Mystic Tomato | 911 |  |
| Zaborg the Thunder Monarch | 911 |  |
| King Tiger Wanghu | 822 |  |
| Dust Tornado | 800 |  |
| Limiter Removal | 778 |  |
| Spirit Reaper | 756 |  |
| Morphing Jar | 733 | Limited |
| Gravekeeper's Spy | 711 |  |
| Kuriboh | 711 |  |
| Ceasefire | 667 | Limited |
| Brain Control | 644 |  |
| Giant Trunade | 578 |  |
| Last Will | 578 |  |
| Mystik Wok | 556 |  |
| Tsukuyomi | 556 |  |
| Exarion Universe | 533 |  |
| Cold Wave | 467 |  |
| Dekoichi the Battlechanted Locomotive | 467 |  |
| Mind Control | 467 |  |
| Solemn Judgment | 444 |  |
| Bottomless Trap Hole | 400 |  |
| Chaos Sorcerer | 400 |  |
| Creature Swap | 400 | Semi-Limited |
| Cyber Jar | 400 | Limited |
| Reinforcement of the Army | 367 | Semi-Limited |
| Asura Priest | 333 |  |
| Gravity Bind | 333 | Semi-Limited |
| Injection Fairy Lily | 333 | Limited |
| Magic Cylinder | 333 | Limited |
| Cipher Soldier | 311 |  |
| D.D. Assailant | 311 |  |
| Kycoo the Ghost Destroyer | 311 |  |
| Megamorph | 289 |  |
| Smashing Ground | 289 |  |
| Level Limit - Area B | 267 | Semi-Limited |
| Nimble Momonga | 267 |  |
| Secret Barrel | 267 |  |
| Just Desserts | 244 |  |
| Mystic Swordsman LV2 | 244 |  |
| Ojama Trio | 244 |  |
| Waboku | 244 |  |
| Airknight Parshath | 222 |  |
| Abyss Soldier | 200 | Semi-Limited |
| Amazoness Archer | 200 |  |
| Apprentice Magician | 200 |  |
| Big Shield Gardna | 200 |  |
| Blade Knight | 200 |  |
| Cannon Soldier | 200 |  |
| Card Destruction | 200 | Limited |
| Chain Disappearance | 200 |  |
| Chiron the Mage | 200 |  |
| Cursed Seal of the Forbidden Spell | 200 |  |
| Cyber End Dragon | 200 |  |
| Cyber Twin Dragon | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Dark Blade the Dragon Knight | 200 |  |
| Dark Flare Knight | 200 |  |
| Dark Magician of Chaos | 200 | Limited |
| Dark Scorpion - Cliff the Trap Remover | 200 |  |
| Darkfire Dragon | 200 |  |
| Des Koala | 200 |  |
| Des Wombat | 200 |  |
| Different Dimension Capsule | 200 |  |
| Don Zaloog | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Electric Snake | 200 |  |
| Enemy Controller | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Flame Ghost | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Gatling Dragon | 200 |  |
| Giant Rat | 200 |  |
| Giltia the D. Knight | 200 |  |
| Greenkappa | 200 |  |
| Gyaku-Gire Panda | 200 |  |
| Jar of Greed | 200 |  |
| Jowgen the Spiritualist | 200 |  |
| Karbonala Warrior | 200 |  |
| King Dragun | 200 |  |
| Magical Merchant | 200 |  |
| Master of Oz | 200 |  |
| Monster Gate | 200 |  |
| Monster Reincarnation | 200 |  |
| Ninja Grandmaster Sasuke | 200 |  |
| Ojama King | 200 |  |
| Poison of the Old Man | 200 |  |
| Raigeki Break | 200 |  |
| Reaper on the Nightmare | 200 |  |
| Reasoning | 200 |  |
| Return from the Different Dimension | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Ryu Senshi | 200 |  |
| Sacred Crane | 200 |  |
| Sasuke Samurai | 200 |  |
| Soul Exchange | 200 |  |
| Spell Canceller | 200 |  |
| Stealth Bird | 200 |  |
| The Last Warrior from Another Planet | 200 |  |
| Thestalos the Firestorm Monarch | 200 |  |
| Thousand-Eyes Restrict | 200 |  |
| Thunder Dragon | 200 |  |
| Token Thanksgiving | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| United We Stand | 200 | Limited |
| Upstart Goblin | 200 | Semi-Limited |
| Wave-Motion Cannon | 200 |  |
| Widespread Ruin | 200 |  |
| Xing Zhen Hu | 200 |  |

### Reaper

| Card | DP | Limit |
|---|---:|---|
| Dark Hole | 2929 | Limited |
| Mystical Space Typhoon | 2929 | Limited |
| Heavy Storm | 2915 | Limited |
| Snatch Steal | 2858 | Limited |
| Torrential Tribute | 2829 | Limited |
| Breaker the Magical Warrior | 2773 | Limited |
| Sangan | 2758 | Limited |
| Nobleman of Crossout | 2588 | Limited |
| Premature Burial | 2488 | Limited |
| Confiscation | 2289 | Limited |
| D.D. Warrior Lady | 2289 | Limited |
| Exiled Force | 2133 | Limited |
| Cyber Dragon | 1986 |  |
| Sakuretsu Armor | 1910 |  |
| Book of Moon | 1891 | Limited |
| Spirit Reaper | 1834 |  |
| Call Of The Haunted | 1735 | Limited |
| Dust Tornado | 1720 |  |
| Mobius the Frost Monarch | 1526 |  |
| Reinforcement of the Army | 1386 | Semi-Limited |
| Smashing Ground | 1313 |  |
| Threatening Roar | 1209 |  |
| Scapegoat | 1123 | Limited |
| Kycoo the Ghost Destroyer | 1062 |  |
| Magician of Faith | 1052 | Limited |
| Jinzo | 1038 | Limited |
| Mystic Tomato | 1000 |  |
| Trap Dustshoot | 910 |  |
| Creature Swap | 867 | Semi-Limited |
| Tsukuyomi | 867 | Limited |
| Solemn Judgment | 863 |  |
| D.D. Assailant | 825 |  |
| Zaborg the Thunder Monarch | 796 |  |
| Swords of Revealing Light | 768 | Limited |
| Enemy Controller | 758 |  |
| Mystic Swordsman LV2 | 744 |  |
| Last Will | 720 |  |
| Royal Decree | 720 |  |
| Treeborn Frog | 701 |  |
| Widespread Ruin | 682 |  |
| Dekoichi the Battlechanted Locomotive | 668 |  |
| Bottomless Trap Hole | 654 |  |
| Brain Control | 635 |  |
| Giant Trunade | 583 |  |
| Morphing Jar | 555 | Limited |
| Chaos Sorcerer | 540 |  |
| Cyber Jar | 540 | Limited |
| Metamorphosis | 540 | Limited |
| Thestalos the Firestorm Monarch | 521 |  |
| Ceasefire | 498 | Limited |
| King Tiger Wanghu | 479 |  |
| Raigeki Break | 436 |  |
| Cipher Soldier | 431 |  |
| Return from the Different Dimension | 427 |  |
| Dark Ruler Ha Des | 408 |  |
| Don Zaloog | 393 |  |
| Mask of Restrict | 384 |  |
| Cyber-Stein | 379 |  |
| Gravekeeper's Spy | 351 |  |
| Bazoo the Soul-Eater | 318 |  |
| Lightning Vortex | 299 | Limited |
| Magical Merchant | 299 |  |
| Level Limit - Area B | 291 | Semi-Limited |
| Waboku | 289 |  |
| Soul Exchange | 261 |  |
| Wave-Motion Cannon | 256 |  |
| Injection Fairy Lily | 242 | Limited |
| Magic Cylinder | 242 | Limited |
| Kuriboh | 237 |  |
| Giant Rat | 232 |  |
| D.D. Survivor | 227 |  |
| Pot of Avarice | 218 |  |
| Thousand-Eyes Restrict | 215 | Limited |
| Hydrogeddon | 213 |  |
| Stealth Bird | 209 |  |
| Gravity Bind | 206 | Semi-Limited |
| Abyss Soldier | 200 | Semi-Limited |
| Acid Trap Hole | 200 |  |
| Airknight Parshath | 200 |  |
| Amazoness Archer | 200 |  |
| Apprentice Magician | 200 |  |
| Aqua Spirit | 200 |  |
| Asura Priest | 200 |  |
| B. Skull Dragon | 200 |  |
| Bait Doll | 200 |  |
| Berserk Gorilla | 200 |  |
| Bickuribox | 200 |  |
| Big Shield Gardna | 200 |  |
| Black Illusion Ritual | 200 |  |
| Blade Knight | 200 |  |
| Blast with Chain | 200 |  |
| Blue-Eyes Ultimate Dragon | 200 |  |
| Book of Life | 200 |  |
| Cannon Soldier | 200 |  |
| Card Destruction | 200 | Limited |
| Chain Disappearance | 200 |  |
| Chain Energy | 200 |  |
| Charubin the Fire Knight | 200 |  |
| Chiron the Mage | 200 |  |
| Cold Wave | 200 |  |
| Compulsory Evacuation Device | 200 |  |
| Cursed Seal of the Forbidden Spell | 200 |  |
| Cyber Blader | 200 |  |
| Cyber End Dragon | 200 |  |
| Cyber Saurus | 200 |  |
| Cyber Twin Dragon | 200 |  |
| Dark Balter the Terrible | 200 |  |
| Dark Blade the Dragon Knight | 200 |  |
| Dark Deal | 200 |  |
| Dark Flare Knight | 200 |  |
| Dark Magician of Chaos | 200 | Limited |
| Dark Mimic LV1 | 200 |  |
| Dark Necrofear | 200 |  |
| Dark Paladin | 200 |  |
| Dark Scorpion - Cliff the Trap Remover | 200 |  |
| Dark World Lightning | 200 |  |
| Darkfire Dragon | 200 |  |
| De-Spell | 200 |  |
| Deepsea Shark | 200 |  |
| Des Koala | 200 |  |
| Des Lacooda | 200 |  |
| Des Wombat | 200 |  |
| Dimension Fusion | 200 |  |
| Dimension Wall | 200 |  |
| Divine Wrath | 200 |  |
| Dragoness the Wicked Knight | 200 |  |
| Draining Shield | 200 |  |
| Drillroid | 200 |  |
| Electric Snake | 200 |  |
| Emergency Provisions | 200 | Semi-Limited |
| Empress Judge | 200 |  |
| Exarion Universe | 200 |  |
| Fiend Skull Dragon | 200 |  |
| Fissure | 200 |  |
| Flame Ghost | 200 |  |
| Flame Swordsman | 200 |  |
| Flying Kamakiri #1 | 200 |  |
| Fusionist | 200 |  |
| Gaia the Dragon Champion | 200 |  |
| Gatling Dragon | 200 |  |
| Giant Germ | 200 |  |
| Gigantes | 200 |  |
| Giltia the D. Knight | 200 |  |
| Goblin Attack Force | 200 |  |
| Goblin Elite Attack Force | 200 |  |
| Goldd, Wu-Lord of Dark World | 200 |  |
| Granmarg the Rock Monarch | 200 |  |
| Gyaku-Gire Panda | 200 |  |
| Hallowed Life Barrier | 200 |  |
| Humanoid Worm Drake | 200 |  |
| Jar of Greed | 200 |  |
| Just Desserts | 200 |  |
| Kaminari Attack | 200 |  |
| Karbonala Warrior | 200 |  |
| Karma Cut | 200 |  |
| King Dragun | 200 |  |
| Labyrinth Tank | 200 |  |
| Light of Intervention | 200 |  |
| Limiter Removal | 200 | Limited |
| Mage Power | 200 | Limited |
| Magic Drain | 200 |  |
| Magic Jammer | 200 |  |
| Marauding Captain | 200 |  |
| Mask of Darkness | 200 |  |
| Masked Dragon | 200 |  |
| Master of Oz | 200 |  |
| Megamorph | 200 |  |
| Messenger of Peace | 200 |  |
| Milus Radiant | 200 |  |
| Mind Control | 200 |  |
| Mirror Wall | 200 |  |
| Mother Grizzly | 200 |  |
| Musician King | 200 |  |
| My Body as a Shield | 200 |  |
| Mystik Wok | 200 |  |
| Newdoria | 200 |  |
| Night Assailant | 200 | Limited |
| Nightmare Wheel | 200 |  |
| Nightmare's Steelcage | 200 |  |
| Nimble Momonga | 200 |  |
| Ninja Grandmaster Sasuke | 200 |  |
| Nobleman of Extermination | 200 |  |
| Offerings to the Doomed | 200 |  |
| Ojama King | 200 |  |
| Ojama Trio | 200 |  |
| Old Vindictive Magician | 200 |  |
| Phoenix Wing Wind Blast | 200 |  |
| Poison of the Old Man | 200 |  |
| Princess of Tsurugi | 200 |  |
| Prohibition | 200 |  |
| Pyramid Turtle | 200 |  |
| Reaper on the Nightmare | 200 |  |
| Reasoning | 200 |  |
| Relinquished | 200 |  |
| Remove Trap | 200 |  |
| Rising Air Current | 200 |  |
| Roaring Ocean Snake | 200 |  |
| Robbin' Goblin | 200 |  |
| Roulette Barrel | 200 |  |
| Royal Command | 200 |  |
| Royal Oppression | 200 |  |
| Rush Recklessly | 200 |  |
| Ryu Kokki | 200 |  |
| Ryu Senshi | 200 |  |
| Sacred Crane | 200 |  |
| Sand Moth | 200 |  |
| Sasuke Samurai | 200 |  |
| Secret Barrel | 200 |  |
| Seven Tools of the Bandit | 200 |  |
| Sillva, Warlord of Dark World | 200 |  |
| Skelengel | 200 |  |
| Skilled Dark Magician | 200 |  |
| Skull Lair | 200 |  |
| Slate Warrior | 200 |  |
| Spell Canceller | 200 |  |
| St. Joan | 200 |  |
| Swarm of Locusts | 200 |  |
| The Last Warrior from Another Planet | 200 |  |
| The Wicked Worm Beast | 200 |  |
| Thunder Dragon | 200 |  |
| Time Seal | 200 |  |
| Time Wizard | 200 |  |
| Trap Hole | 200 |  |
| Tribute to The Doomed | 200 |  |
| Twin-Headed Thunder Dragon | 200 |  |
| UFO Turtle | 200 |  |
| United We Stand | 200 | Limited |
| Upstart Goblin | 200 | Semi-Limited |
| Vampire Lord | 200 |  |
| Warrior Elimination | 200 |  |
| Warrior of Tradition | 200 |  |
| Zombyra the Dark | 200 |  |

<!-- /pack-prices -->
