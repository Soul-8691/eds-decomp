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

- **Type:** Usage · **Format:** Imperial · **Cards in pool:** 105 · **Weight sum:** 109272

| Card | Weight | Pull % |
|---|---:|---:|
| Aqua Madoor | 2900 | 2.6539 |
| Exchange | 2900 | 2.6539 |
| Kuriboh | 2900 | 2.6539 |
| Mask of Darkness | 2900 | 2.6539 |
| Megamorph | 2900 | 2.6539 |
| Nimble Momonga | 2900 | 2.6539 |
| Reinforcements | 2900 | 2.6539 |
| Seven Tools of the Bandit | 2900 | 2.6539 |
| Soul Exchange | 2900 | 2.6539 |
| Thunder Dragon | 2900 | 2.6539 |
| Backup Soldier | 2484 | 2.2732 |
| Acid Trap Hole | 2227 | 2.0380 |
| Magic Drain | 2227 | 2.0380 |
| Tribute to The Doomed | 2227 | 2.0380 |
| White Magical Hat | 2227 | 2.0380 |
| Exodia the Forbidden One | 1911 | 1.7488 |
| Giant Germ | 1911 | 1.7488 |
| Graverobber | 1911 | 1.7488 |
| Last Will | 1911 | 1.7488 |
| Left Arm of the Forbidden One | 1911 | 1.7488 |
| Left Leg of the Forbidden One | 1911 | 1.7488 |
| Magic Jammer | 1911 | 1.7488 |
| Morphing Jar #2 | 1911 | 1.7488 |
| Right Arm of the Forbidden One | 1911 | 1.7488 |
| Right Leg of the Forbidden One | 1911 | 1.7488 |
| Black Illusion Ritual | 1717 | 1.5713 |
| Masked Sorcerer | 1717 | 1.5713 |
| Relinquished | 1717 | 1.5713 |
| Senju of the Thousand Hands | 1717 | 1.5713 |
| Sonic Bird | 1717 | 1.5713 |
| Upstart Goblin | 1478 | 1.3526 |
| 4-Starred Ladybug of Doom | 1396 | 1.2775 |
| Waboku | 1330 | 1.2171 |
| Giant Soldier of Stone | 1274 | 1.1659 |
| Card Destruction | 1149 | 1.0515 |
| Time Seal | 1037 | 0.9490 |
| Goblin Attack Force | 1015 | 0.9289 |
| Giant Trunade | 995 | 0.9106 |
| The Fiend Megacyber | 976 | 0.8932 |
| Banisher of the Light | 887 | 0.8117 |
| Solemn Judgment | 833 | 0.7623 |
| Cannon Soldier | 791 | 0.7239 |
| Fissure | 769 | 0.7037 |
| Trap Hole | 769 | 0.7037 |
| Dust Tornado | 749 | 0.6854 |
| Magician of Faith | 749 | 0.6854 |
| Man-Eater Bug | 749 | 0.6854 |
| Mechanicalchaser | 749 | 0.6854 |
| Mystic Tomato | 743 | 0.6800 |
| Painful Choice | 726 | 0.6644 |
| Empress Judge | 710 | 0.6498 |
| Fusionist | 710 | 0.6498 |
| Giltia the D. Knight | 710 | 0.6498 |
| Rabid Horseman | 710 | 0.6498 |
| Heavy Storm | 666 | 0.6095 |
| Nobleman of Crossout | 650 | 0.5948 |
| Sangan | 650 | 0.5948 |
| Swords of Revealing Light | 640 | 0.5857 |
| Cyber Jar | 599 | 0.5482 |
| Ceasefire | 574 | 0.5253 |
| Morphing Jar | 568 | 0.5198 |
| Premature Burial | 562 | 0.5143 |
| Change of Heart | 522 | 0.4777 |
| Confiscation | 522 | 0.4777 |
| Dark Hole | 522 | 0.4777 |
| Delinquent Duo | 522 | 0.4777 |
| Jinzo | 522 | 0.4777 |
| Mirror Force | 522 | 0.4777 |
| Monster Reborn | 522 | 0.4777 |
| Pot of Greed | 522 | 0.4777 |
| Raigeki | 522 | 0.4777 |
| Snatch Steal | 522 | 0.4777 |
| The Forceful Sentry | 522 | 0.4777 |
| Witch of the Black Forest | 522 | 0.4777 |
| Gaia the Dragon Champion | 462 | 0.4228 |
| Bickuribox | 413 | 0.3780 |
| Skull Knight | 413 | 0.3780 |
| Charubin the Fire Knight | 378 | 0.3459 |
| Flame Ghost | 378 | 0.3459 |
| Cyber Saurus | 353 | 0.3230 |
| Flame Swordsman | 332 | 0.3038 |
| Gravity Bind | 290 | 0.2654 |
| Light of Intervention | 290 | 0.2654 |
| Prohibition | 290 | 0.2654 |
| Robbin' Goblin | 290 | 0.2654 |
| Kwagar Hercules | 279 | 0.2553 |
| Metal Dragon | 279 | 0.2553 |
| Deepsea Shark | 270 | 0.2471 |
| Punished Eagle | 270 | 0.2471 |
| Roaring Ocean Snake | 270 | 0.2471 |
| Dragoness the Wicked Knight | 262 | 0.2398 |
| Thousand Dragon | 262 | 0.2398 |
| Musician King | 255 | 0.2334 |
| Warrior of Tradition | 248 | 0.2270 |
| Twin-Headed Thunder Dragon | 236 | 0.2160 |
| Darkfire Dragon | 227 | 0.2077 |
| Labyrinth Tank | 227 | 0.2077 |
| B. Skull Dragon | 222 | 0.2032 |
| Thousand-Eyes Restrict | 222 | 0.2032 |
| Rush Recklessly | 171 | 0.1565 |
| Limiter Removal | 127 | 0.1162 |
| Solemn Wishes | 114 | 0.1043 |
| Mystical Space Typhoon | 74 | 0.0677 |
| Call Of The Haunted | 52 | 0.0476 |
| Imperial Order | 52 | 0.0476 |

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

- **Type:** Usage · **Format:** Android · **Cards in pool:** 102 · **Weight sum:** 107089

| Card | Weight | Pull % |
|---|---:|---:|
| Axe of Despair | 2791 | 2.6062 |
| Castle of Dark Illusions | 2791 | 2.6062 |
| Dark Elf | 2791 | 2.6062 |
| Fake Trap | 2791 | 2.6062 |
| Firegrass | 2791 | 2.6062 |
| Giant Germ | 2791 | 2.6062 |
| Giant Soldier of Stone | 2791 | 2.6062 |
| Giant Trunade | 2791 | 2.6062 |
| La Jinn the Mystical Genie of the Lamp | 2791 | 2.6062 |
| Megamorph | 2791 | 2.6062 |
| Soul Release | 2791 | 2.6062 |
| Summoned Skull | 2791 | 2.6062 |
| The Fiend Megacyber | 2791 | 2.6062 |
| Thunder Dragon | 2791 | 2.6062 |
| Banisher of the Light | 2144 | 2.0021 |
| Kuriboh | 2144 | 2.0021 |
| Nobleman of Extermination | 2144 | 2.0021 |
| Goblin Attack Force | 1841 | 1.7191 |
| Jirai Gumo | 1841 | 1.7191 |
| Magic Drain | 1841 | 1.7191 |
| Nimble Momonga | 1841 | 1.7191 |
| Waboku | 1841 | 1.7191 |
| White Magical Hat | 1841 | 1.7191 |
| Man-Eater Bug | 1523 | 1.4222 |
| Mask of Darkness | 1523 | 1.4222 |
| Seven Tools of the Bandit | 1523 | 1.4222 |
| Mage Power | 1424 | 1.3297 |
| Upstart Goblin | 1424 | 1.3297 |
| Zombyra the Dark | 1424 | 1.3297 |
| Exchange | 1346 | 1.2569 |
| United We Stand | 1229 | 1.1476 |
| Masked Sorcerer | 1144 | 1.0683 |
| Penguin Knight | 1144 | 1.0683 |
| 4-Starred Ladybug of Doom | 1109 | 1.0356 |
| Time Seal | 1049 | 0.9796 |
| Card Destruction | 1024 | 0.9562 |
| Solemn Judgment | 980 | 0.9151 |
| Cannon Soldier | 943 | 0.8806 |
| Dust Tornado | 805 | 0.7517 |
| Magic Cylinder | 805 | 0.7517 |
| Magician of Faith | 780 | 0.7284 |
| Swords of Revealing Light | 780 | 0.7284 |
| Trap Hole | 780 | 0.7284 |
| Jar of Greed | 764 | 0.7134 |
| Fissure | 757 | 0.7069 |
| Mystic Tomato | 757 | 0.7069 |
| Bazoo the Soul-Eater | 750 | 0.7004 |
| Gemini Elf | 750 | 0.7004 |
| Torrential Tribute | 750 | 0.7004 |
| Kycoo the Ghost Destroyer | 743 | 0.6938 |
| Ceasefire | 737 | 0.6882 |
| Flame Ghost | 682 | 0.6369 |
| Flower Wolf | 682 | 0.6369 |
| Giltia the D. Knight | 682 | 0.6369 |
| Karbonala Warrior | 682 | 0.6369 |
| Punished Eagle | 682 | 0.6369 |
| Cyber Jar | 662 | 0.6182 |
| Heavy Storm | 650 | 0.6070 |
| Nobleman of Crossout | 650 | 0.6070 |
| Premature Burial | 650 | 0.6070 |
| Morphing Jar | 629 | 0.5874 |
| Painful Choice | 530 | 0.4949 |
| Snatch Steal | 526 | 0.4912 |
| Change of Heart | 522 | 0.4874 |
| Confiscation | 522 | 0.4874 |
| Dark Hole | 522 | 0.4874 |
| Delinquent Duo | 522 | 0.4874 |
| Jinzo | 522 | 0.4874 |
| Mirror Force | 522 | 0.4874 |
| Monster Reborn | 522 | 0.4874 |
| Pot of Greed | 522 | 0.4874 |
| Raigeki | 522 | 0.4874 |
| Sangan | 522 | 0.4874 |
| The Forceful Sentry | 522 | 0.4874 |
| Witch of the Black Forest | 522 | 0.4874 |
| Kwagar Hercules | 520 | 0.4856 |
| Cyber Saurus | 444 | 0.4146 |
| Kaminari Attack | 444 | 0.4146 |
| Labyrinth Tank | 444 | 0.4146 |
| Skull Knight | 444 | 0.4146 |
| Charubin the Fire Knight | 397 | 0.3707 |
| Metal Dragon | 397 | 0.3707 |
| Thousand Dragon | 397 | 0.3707 |
| Bickuribox | 364 | 0.3399 |
| Musician King | 339 | 0.3166 |
| Fusionist | 320 | 0.2988 |
| Warrior of Tradition | 320 | 0.2988 |
| Dragoness the Wicked Knight | 304 | 0.2839 |
| Twin-Headed Thunder Dragon | 304 | 0.2839 |
| Darkfire Dragon | 290 | 0.2708 |
| Mirror Wall | 279 | 0.2605 |
| Flame Swordsman | 260 | 0.2428 |
| Gaia the Dragon Champion | 252 | 0.2353 |
| Thousand-Eyes Restrict | 245 | 0.2288 |
| B. Skull Dragon | 239 | 0.2232 |
| The Last Warrior from Another Planet | 223 | 0.2082 |
| Fairy Box | 184 | 0.1718 |
| Solemn Wishes | 134 | 0.1251 |
| Skull Lair | 77 | 0.0719 |
| Mystical Space Typhoon | 74 | 0.0691 |
| Call Of The Haunted | 52 | 0.0486 |
| Imperial Order | 52 | 0.0486 |

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

- **Type:** Usage · **Format:** Joey-Pegasus · **Cards in pool:** 105 · **Weight sum:** 93826

| Card | Weight | Pull % |
|---|---:|---:|
| Armed Ninja | 2144 | 2.2851 |
| Axe of Despair | 2144 | 2.2851 |
| Cold Wave | 2144 | 2.2851 |
| Giant Germ | 2144 | 2.2851 |
| Giant Rat | 2144 | 2.2851 |
| Giant Trunade | 2144 | 2.2851 |
| Guardian Sphinx | 2144 | 2.2851 |
| Kotodama | 2144 | 2.2851 |
| Last Will | 2144 | 2.2851 |
| Magic Drain | 2144 | 2.2851 |
| Penguin Knight | 2144 | 2.2851 |
| Reinforcement of the Army | 2144 | 2.2851 |
| Summoned Skull | 2144 | 2.2851 |
| White Magical Hat | 2144 | 2.2851 |
| Backup Soldier | 1841 | 1.9621 |
| Exchange | 1654 | 1.7628 |
| Kuriboh | 1654 | 1.7628 |
| Masked Sorcerer | 1654 | 1.7628 |
| Nimble Momonga | 1654 | 1.7628 |
| Seven Tools of the Bandit | 1654 | 1.7628 |
| Thunder Dragon | 1654 | 1.7628 |
| Card Destruction | 1424 | 1.5177 |
| Exodia the Forbidden One | 1424 | 1.5177 |
| Left Arm of the Forbidden One | 1424 | 1.5177 |
| Left Leg of the Forbidden One | 1424 | 1.5177 |
| Right Arm of the Forbidden One | 1424 | 1.5177 |
| Right Leg of the Forbidden One | 1424 | 1.5177 |
| Swords of Revealing Light | 1424 | 1.5177 |
| Time Seal | 1183 | 1.2608 |
| Man-Eater Bug | 1109 | 1.1820 |
| United We Stand | 1109 | 1.1820 |
| Waboku | 1001 | 1.0669 |
| Cannon Soldier | 961 | 1.0242 |
| Mage Power | 961 | 1.0242 |
| Trap Hole | 961 | 1.0242 |
| Upstart Goblin | 961 | 1.0242 |
| Dust Tornado | 926 | 0.9869 |
| Solemn Judgment | 926 | 0.9869 |
| Fissure | 824 | 0.8782 |
| Ceasefire | 805 | 0.8580 |
| Jar of Greed | 805 | 0.8580 |
| Magic Cylinder | 805 | 0.8580 |
| Cyber Jar | 757 | 0.8068 |
| Torrential Tribute | 757 | 0.8068 |
| Bazoo the Soul-Eater | 743 | 0.7919 |
| Gemini Elf | 743 | 0.7919 |
| Kycoo the Ghost Destroyer | 743 | 0.7919 |
| Magician of Faith | 743 | 0.7919 |
| Mystic Tomato | 743 | 0.7919 |
| Morphing Jar | 688 | 0.7333 |
| Nobleman of Crossout | 662 | 0.7056 |
| Graceful Charity | 650 | 0.6928 |
| Heavy Storm | 562 | 0.5990 |
| Painful Choice | 530 | 0.5649 |
| Premature Burial | 530 | 0.5649 |
| Change of Heart | 522 | 0.5563 |
| Confiscation | 522 | 0.5563 |
| Dark Hole | 522 | 0.5563 |
| Delinquent Duo | 522 | 0.5563 |
| Harpie's Feather Duster | 522 | 0.5563 |
| Jinzo | 522 | 0.5563 |
| Mirror Force | 522 | 0.5563 |
| Monster Reborn | 522 | 0.5563 |
| Pot of Greed | 522 | 0.5563 |
| Raigeki | 522 | 0.5563 |
| Sangan | 522 | 0.5563 |
| Sinister Serpent | 522 | 0.5563 |
| Snatch Steal | 522 | 0.5563 |
| The Forceful Sentry | 522 | 0.5563 |
| Witch of the Black Forest | 522 | 0.5563 |
| Bickuribox | 520 | 0.5542 |
| Charubin the Fire Knight | 520 | 0.5542 |
| Dark Balter the Terrible | 520 | 0.5542 |
| Empress Judge | 520 | 0.5542 |
| Fiend Skull Dragon | 520 | 0.5542 |
| Flame Ghost | 520 | 0.5542 |
| Flower Wolf | 520 | 0.5542 |
| Giltia the D. Knight | 520 | 0.5542 |
| Kaminari Attack | 520 | 0.5542 |
| Roaring Ocean Snake | 520 | 0.5542 |
| Ryu Senshi | 520 | 0.5542 |
| Twin-Headed Thunder Dragon | 520 | 0.5542 |
| Warrior of Tradition | 520 | 0.5542 |
| Cyber Saurus | 397 | 0.4231 |
| Dragoness the Wicked Knight | 397 | 0.4231 |
| Metal Dragon | 397 | 0.4231 |
| Musician King | 397 | 0.4231 |
| Skull Knight | 397 | 0.4231 |
| Darkfire Dragon | 339 | 0.3613 |
| Fusionist | 339 | 0.3613 |
| Labyrinth Tank | 339 | 0.3613 |
| Thousand-Eyes Restrict | 279 | 0.2974 |
| Flame Swordsman | 245 | 0.2611 |
| Gaia the Dragon Champion | 245 | 0.2611 |
| B. Skull Dragon | 223 | 0.2377 |
| Rush Recklessly | 214 | 0.2281 |
| Solemn Wishes | 214 | 0.2281 |
| The Last Warrior from Another Planet | 206 | 0.2196 |
| Mirror Wall | 165 | 0.1759 |
| Limiter Removal | 142 | 0.1513 |
| Scapegoat | 104 | 0.1108 |
| Skull Lair | 82 | 0.0874 |
| Mystical Space Typhoon | 74 | 0.0789 |
| Call Of The Haunted | 52 | 0.0554 |
| Imperial Order | 52 | 0.0554 |

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

- **Type:** Usage · **Format:** Fiber · **Cards in pool:** 132 · **Weight sum:** 201621

| Card | Weight | Pull % |
|---|---:|---:|
| Black Illusion Ritual | 5162 | 2.5602 |
| Bottomless Trap Hole | 5162 | 2.5602 |
| Don Zaloog | 5162 | 2.5602 |
| Flash Assailant | 5162 | 2.5602 |
| Flying Kamakiri #1 | 5162 | 2.5602 |
| Jirai Gumo | 5162 | 2.5602 |
| Man-Eater Bug | 5162 | 2.5602 |
| Megamorph | 5162 | 2.5602 |
| Metalmorph | 5162 | 2.5602 |
| Nobleman of Extermination | 5162 | 2.5602 |
| Relinquished | 5162 | 2.5602 |
| Senju of the Thousand Hands | 5162 | 2.5602 |
| Sonic Bird | 5162 | 2.5602 |
| Summoned Skull | 5162 | 2.5602 |
| Giant Trunade | 3941 | 1.9547 |
| Graverobber | 3941 | 1.9547 |
| Kuriboh | 3941 | 1.9547 |
| Mask of Darkness | 3941 | 1.9547 |
| Princess of Tsurugi | 3941 | 1.9547 |
| White Magical Hat | 3941 | 1.9547 |
| Acid Trap Hole | 3369 | 1.6710 |
| Last Will | 3369 | 1.6710 |
| Nimble Momonga | 3369 | 1.6710 |
| Soul Exchange | 3369 | 1.6710 |
| Magic Drain | 3016 | 1.4959 |
| Exchange | 2768 | 1.3729 |
| Blast with Chain | 2314 | 1.1477 |
| Gearfried the Iron Knight | 2314 | 1.1477 |
| Tribute to The Doomed | 2314 | 1.1477 |
| Electric Snake | 2213 | 1.0976 |
| The Fiend Megacyber | 2213 | 1.0976 |
| Waboku | 2213 | 1.0976 |
| Giant Germ | 2127 | 1.0549 |
| Magic Jammer | 1927 | 0.9558 |
| Seven Tools of the Bandit | 1874 | 0.9295 |
| Swords of Revealing Light | 1827 | 0.9062 |
| Thunder Dragon | 1827 | 0.9062 |
| Zombyra the Dark | 1827 | 0.9062 |
| Reinforcement of the Army | 1763 | 0.8744 |
| Goblin Attack Force | 1743 | 0.8645 |
| 4-Starred Ladybug of Doom | 1612 | 0.7995 |
| Solemn Judgment | 1612 | 0.7995 |
| Airknight Parshath | 1469 | 0.7286 |
| Dark Ruler Ha Des | 1431 | 0.7097 |
| Mage Power | 1365 | 0.6770 |
| Giant Rat | 1284 | 0.6368 |
| Giltia the D. Knight | 1278 | 0.6339 |
| Time Seal | 1273 | 0.6314 |
| Cannon Soldier | 1220 | 0.6051 |
| Spear Dragon | 1201 | 0.5957 |
| United We Stand | 1192 | 0.5912 |
| Upstart Goblin | 1142 | 0.5664 |
| Card Destruction | 1064 | 0.5277 |
| Punished Eagle | 971 | 0.4816 |
| Jar of Greed | 904 | 0.4484 |
| Fissure | 839 | 0.4161 |
| Asura Priest | 831 | 0.4122 |
| Ceasefire | 829 | 0.4112 |
| Trap Hole | 829 | 0.4112 |
| Empress Judge | 828 | 0.4107 |
| Flower Wolf | 828 | 0.4107 |
| Kwagar Hercules | 828 | 0.4107 |
| Creature Swap | 826 | 0.4097 |
| Dust Tornado | 822 | 0.4077 |
| Gemini Elf | 792 | 0.3928 |
| Magician of Faith | 792 | 0.3928 |
| Kycoo the Ghost Destroyer | 784 | 0.3888 |
| Bazoo the Soul-Eater | 779 | 0.3864 |
| Mystic Tomato | 762 | 0.3779 |
| Torrential Tribute | 743 | 0.3685 |
| Magic Cylinder | 683 | 0.3388 |
| Cyber Jar | 672 | 0.3333 |
| Morphing Jar | 669 | 0.3318 |
| Nobleman of Crossout | 663 | 0.3288 |
| Graceful Charity | 650 | 0.3224 |
| Metal Dragon | 630 | 0.3125 |
| Premature Burial | 586 | 0.2906 |
| Heavy Storm | 571 | 0.2832 |
| Charubin the Fire Knight | 563 | 0.2792 |
| Cyber Saurus | 563 | 0.2792 |
| Warrior of Tradition | 563 | 0.2792 |
| Yata-Garasu | 543 | 0.2693 |
| Bickuribox | 537 | 0.2663 |
| Skull Knight | 537 | 0.2663 |
| Painful Choice | 534 | 0.2649 |
| Jinzo | 529 | 0.2624 |
| Sinister Serpent | 529 | 0.2624 |
| Confiscation | 526 | 0.2609 |
| Delinquent Duo | 525 | 0.2604 |
| Injection Fairy Lily | 525 | 0.2604 |
| Change of Heart | 523 | 0.2594 |
| Exiled Force | 523 | 0.2594 |
| Fiber Jar | 523 | 0.2594 |
| Harpie's Feather Duster | 523 | 0.2594 |
| Monster Reborn | 523 | 0.2594 |
| Snatch Steal | 523 | 0.2594 |
| Dark Hole | 522 | 0.2589 |
| Mirror Force | 522 | 0.2589 |
| Pot of Greed | 522 | 0.2589 |
| Raigeki | 522 | 0.2589 |
| Sangan | 522 | 0.2589 |
| The Forceful Sentry | 522 | 0.2589 |
| Witch of the Black Forest | 522 | 0.2589 |
| Graceful Dice | 516 | 0.2559 |
| Gravity Bind | 516 | 0.2559 |
| Mirror Wall | 516 | 0.2559 |
| Rush Recklessly | 516 | 0.2559 |
| Flame Swordsman | 497 | 0.2465 |
| Flame Ghost | 480 | 0.2381 |
| Fusionist | 410 | 0.2034 |
| Thousand Dragon | 386 | 0.1914 |
| Gaia the Dragon Champion | 356 | 0.1766 |
| Roaring Ocean Snake | 356 | 0.1766 |
| Solemn Wishes | 336 | 0.1666 |
| Musician King | 321 | 0.1592 |
| Labyrinth Tank | 310 | 0.1538 |
| Darkfire Dragon | 293 | 0.1453 |
| Fiend Skull Dragon | 256 | 0.1270 |
| Dark Balter the Terrible | 250 | 0.1240 |
| Dragoness the Wicked Knight | 250 | 0.1240 |
| B. Skull Dragon | 249 | 0.1235 |
| Ryu Senshi | 234 | 0.1161 |
| Twin-Headed Thunder Dragon | 234 | 0.1161 |
| Chain Energy | 221 | 0.1096 |
| The Last Warrior from Another Planet | 210 | 0.1042 |
| Thousand-Eyes Restrict | 207 | 0.1027 |
| Limiter Removal | 141 | 0.0699 |
| Skull Lair | 110 | 0.0546 |
| Scapegoat | 82 | 0.0407 |
| Mystical Space Typhoon | 74 | 0.0367 |
| Call Of The Haunted | 52 | 0.0258 |
| Imperial Order | 52 | 0.0258 |

### Pharaonic Guardian

- **Type:** TCG · **Format:** Yata · **Cards in pool:** 19 · **Weight sum:** 85125

| Card | Weight | Pull % |
|---|---:|---:|
| Des Lacooda | 10000 | 11.7474 |
| Different Dimension Capsule | 10000 | 11.7474 |
| Gravekeeper's Spy | 10000 | 11.7474 |
| Metamorphosis | 10000 | 11.7474 |
| Raigeki Break | 10000 | 11.7474 |
| Reasoning | 10000 | 11.7474 |
| Trap Dustshoot | 10000 | 11.7474 |
| Book of Moon | 2500 | 2.9369 |
| King Tiger Wanghu | 2500 | 2.9369 |
| Newdoria | 2500 | 2.9369 |
| Pyramid Turtle | 2500 | 2.9369 |
| Spirit Reaper | 2500 | 2.9369 |
| Book of Life | 600 | 0.7048 |
| Mirage of Nightmare | 600 | 0.7048 |
| Reaper on the Nightmare | 600 | 0.7048 |
| Sasuke Samurai | 600 | 0.7048 |
| Don Zaloog | 100 | 0.1175 |
| Guardian Sphinx | 100 | 0.1175 |
| Bell of Destruction | 25 | 0.0294 |

### Yata

- **Type:** Usage · **Format:** Yata · **Cards in pool:** 132 · **Weight sum:** 149734

| Card | Weight | Pull % |
|---|---:|---:|
| Cold Wave | 3681 | 2.4584 |
| Graverobber | 3681 | 2.4584 |
| Just Desserts | 3681 | 2.4584 |
| Last Will | 3681 | 2.4584 |
| Magic Drain | 3681 | 2.4584 |
| Mask of Darkness | 3681 | 2.4584 |
| Masked Sorcerer | 3681 | 2.4584 |
| Morphing Jar #2 | 3148 | 2.1024 |
| Book of Life | 2819 | 1.8827 |
| Giant Trunade | 2819 | 1.8827 |
| Magic Jammer | 2819 | 1.8827 |
| Princess of Tsurugi | 2819 | 1.8827 |
| Waboku | 2819 | 1.8827 |
| Barrel Dragon | 2415 | 1.6129 |
| Giant Rat | 2415 | 1.6129 |
| Kuriboh | 2415 | 1.6129 |
| Marauding Captain | 2415 | 1.6129 |
| Mechanicalchaser | 2415 | 1.6129 |
| Pyramid Turtle | 2415 | 1.6129 |
| Reasoning | 2415 | 1.6129 |
| Spear Dragon | 2415 | 1.6129 |
| United We Stand | 2415 | 1.6129 |
| White Magical Hat | 2415 | 1.6129 |
| Giant Germ | 2165 | 1.4459 |
| Nimble Momonga | 2165 | 1.4459 |
| Guardian Sphinx | 1991 | 1.3297 |
| Card Destruction | 1859 | 1.2415 |
| Goblin Attack Force | 1755 | 1.1721 |
| Sasuke Samurai | 1755 | 1.1721 |
| Seven Tools of the Bandit | 1755 | 1.1721 |
| Blast with Chain | 1670 | 1.1153 |
| Gearfried the Iron Knight | 1670 | 1.1153 |
| Raigeki Break | 1599 | 1.0679 |
| Solemn Judgment | 1599 | 1.0679 |
| Time Seal | 1599 | 1.0679 |
| Trap Dustshoot | 1599 | 1.0679 |
| 4-Starred Ladybug of Doom | 1538 | 1.0272 |
| Cannon Soldier | 1485 | 0.9918 |
| Trap Hole | 1485 | 0.9918 |
| Metamorphosis | 1438 | 0.9604 |
| Upstart Goblin | 1438 | 0.9604 |
| Airknight Parshath | 1396 | 0.9323 |
| Electric Snake | 1359 | 0.9076 |
| Swords of Revealing Light | 1325 | 0.8849 |
| Jar of Greed | 1217 | 0.8128 |
| Dark Ruler Ha Des | 1174 | 0.7841 |
| Newdoria | 1174 | 0.7841 |
| Asura Priest | 1136 | 0.7587 |
| King Tiger Wanghu | 1073 | 0.7166 |
| Bazoo the Soul-Eater | 1059 | 0.7073 |
| Kycoo the Ghost Destroyer | 1059 | 0.7073 |
| Creature Swap | 1016 | 0.6785 |
| Fissure | 999 | 0.6672 |
| Cyber Jar | 943 | 0.6298 |
| Warrior of Tradition | 906 | 0.6051 |
| Dust Tornado | 897 | 0.5991 |
| Gravekeeper's Spy | 883 | 0.5897 |
| Zombyra the Dark | 859 | 0.5737 |
| Magician of Faith | 848 | 0.5663 |
| Magic Cylinder | 842 | 0.5623 |
| Reinforcement of the Army | 827 | 0.5523 |
| Don Zaloog | 822 | 0.5490 |
| Gemini Elf | 822 | 0.5490 |
| Morphing Jar | 812 | 0.5423 |
| Mystic Tomato | 808 | 0.5396 |
| Spirit Reaper | 779 | 0.5203 |
| Torrential Tribute | 750 | 0.5009 |
| Ceasefire | 701 | 0.4682 |
| Premature Burial | 701 | 0.4682 |
| Punished Eagle | 689 | 0.4601 |
| Nobleman of Crossout | 653 | 0.4361 |
| Fusionist | 588 | 0.3927 |
| Skull Knight | 588 | 0.3927 |
| Painful Choice | 575 | 0.3840 |
| Yata-Garasu | 575 | 0.3840 |
| Sinister Serpent | 560 | 0.3740 |
| Fiber Jar | 549 | 0.3667 |
| Injection Fairy Lily | 547 | 0.3653 |
| Heavy Storm | 539 | 0.3600 |
| Confiscation | 530 | 0.3540 |
| Delinquent Duo | 528 | 0.3526 |
| Exiled Force | 528 | 0.3526 |
| Sangan | 528 | 0.3526 |
| Jinzo | 526 | 0.3513 |
| Monster Reborn | 526 | 0.3513 |
| Metal Dragon | 525 | 0.3506 |
| Thousand Dragon | 525 | 0.3506 |
| Bell of Destruction | 522 | 0.3486 |
| Change of Heart | 522 | 0.3486 |
| Dark Hole | 522 | 0.3486 |
| Graceful Charity | 522 | 0.3486 |
| Harpie's Feather Duster | 522 | 0.3486 |
| Mirror Force | 522 | 0.3486 |
| Pot of Greed | 522 | 0.3486 |
| Raigeki | 522 | 0.3486 |
| Snatch Steal | 522 | 0.3486 |
| The Forceful Sentry | 522 | 0.3486 |
| Witch of the Black Forest | 522 | 0.3486 |
| Flower Wolf | 481 | 0.3212 |
| Giltia the D. Knight | 481 | 0.3212 |
| Charubin the Fire Knight | 448 | 0.2992 |
| Flame Ghost | 422 | 0.2818 |
| Labyrinth Tank | 422 | 0.2818 |
| Karbonala Warrior | 401 | 0.2678 |
| Cyber Saurus | 383 | 0.2558 |
| Forced Requisition | 368 | 0.2458 |
| Mirror Wall | 368 | 0.2458 |
| Musician King | 368 | 0.2458 |
| Empress Judge | 354 | 0.2364 |
| Flame Swordsman | 354 | 0.2364 |
| Roaring Ocean Snake | 354 | 0.2364 |
| Bickuribox | 343 | 0.2291 |
| Twin-Headed Thunder Dragon | 281 | 0.1877 |
| Gaia the Dragon Champion | 271 | 0.1810 |
| Darkfire Dragon | 258 | 0.1723 |
| Dragoness the Wicked Knight | 244 | 0.1630 |
| Limiter Removal | 241 | 0.1610 |
| B. Skull Dragon | 227 | 0.1516 |
| The Last Warrior from Another Planet | 212 | 0.1416 |
| Reaper on the Nightmare | 208 | 0.1389 |
| Fiend Skull Dragon | 192 | 0.1282 |
| Dark Balter the Terrible | 188 | 0.1256 |
| Chain Energy | 185 | 0.1236 |
| Ryu Senshi | 183 | 0.1222 |
| Thousand-Eyes Restrict | 179 | 0.1195 |
| Solemn Wishes | 153 | 0.1022 |
| Book of Moon | 107 | 0.0715 |
| Scapegoat | 94 | 0.0628 |
| Mystical Space Typhoon | 74 | 0.0494 |
| Call Of The Haunted | 53 | 0.0354 |
| Imperial Order | 52 | 0.0347 |
| Mirage of Nightmare | 52 | 0.0347 |

### Magician's Force

- **Type:** TCG · **Format:** Scientist · **Cards in pool:** 17 · **Weight sum:** 86025

| Card | Weight | Pull % |
|---|---:|---:|
| Dark Scorpion - Cliff the Trap Remover | 10000 | 11.6245 |
| Magical Merchant | 10000 | 11.6245 |
| Magical Scientist | 10000 | 11.6245 |
| My Body as a Shield | 10000 | 11.6245 |
| Poison of the Old Man | 10000 | 11.6245 |
| Roulette Barrel | 10000 | 11.6245 |
| Wave-Motion Cannon | 10000 | 11.6245 |
| Secret Barrel | 7000 | 8.1372 |
| Apprentice Magician | 2500 | 2.9061 |
| Big Bang Shot | 2500 | 2.9061 |
| Des Koala | 2500 | 2.9061 |
| Skilled Dark Magician | 600 | 0.6975 |
| Tribe-Infecting Virus | 600 | 0.6975 |
| Breaker the Magical Warrior | 100 | 0.1162 |
| Dark Paladin | 100 | 0.1162 |
| Spell Canceller | 100 | 0.1162 |
| Dark Magician Girl | 25 | 0.0291 |

### Scientist

- **Type:** Usage · **Format:** Scientist · **Cards in pool:** 88 · **Weight sum:** 70179

| Card | Weight | Pull % |
|---|---:|---:|
| Apprentice Magician | 1804 | 2.5706 |
| Asura Priest | 1804 | 2.5706 |
| Dark Magician | 1804 | 2.5706 |
| Dark Ruler Ha Des | 1804 | 2.5706 |
| Don Zaloog | 1804 | 2.5706 |
| Kycoo the Ghost Destroyer | 1804 | 2.5706 |
| Nimble Momonga | 1804 | 2.5706 |
| Sasuke Samurai | 1804 | 2.5706 |
| Skilled Dark Magician | 1804 | 2.5706 |
| Spell Canceller | 1804 | 2.5706 |
| Waboku | 1804 | 2.5706 |
| Creature Swap | 1552 | 2.2115 |
| Reinforcement of the Army | 1552 | 2.2115 |
| Airknight Parshath | 1396 | 1.9892 |
| Goblin Attack Force | 1396 | 1.9892 |
| Jar of Greed | 1396 | 1.9892 |
| Cannon Soldier | 1205 | 1.7170 |
| Fissure | 1205 | 1.7170 |
| Giant Trunade | 1205 | 1.7170 |
| Raigeki Break | 1205 | 1.7170 |
| Reasoning | 1205 | 1.7170 |
| Swords of Revealing Light | 1205 | 1.7170 |
| Upstart Goblin | 1205 | 1.7170 |
| Gilasaurus | 1087 | 1.5489 |
| Magician of Faith | 1087 | 1.5489 |
| Newdoria | 1087 | 1.5489 |
| Gravekeeper's Spy | 1005 | 1.4321 |
| Metamorphosis | 1005 | 1.4321 |
| Trap Dustshoot | 1005 | 1.4321 |
| Cyber Jar | 943 | 1.3437 |
| Dust Tornado | 943 | 1.3437 |
| Last Will | 943 | 1.3437 |
| Spirit Reaper | 943 | 1.3437 |
| Catapult Turtle | 893 | 1.2725 |
| King Tiger Wanghu | 893 | 1.2725 |
| Mystic Tomato | 819 | 1.1670 |
| Nobleman of Crossout | 778 | 1.1086 |
| Torrential Tribute | 765 | 1.0901 |
| Card Destruction | 743 | 1.0587 |
| Ceasefire | 743 | 1.0587 |
| Magical Scientist | 743 | 1.0587 |
| Premature Burial | 650 | 0.9262 |
| Heavy Storm | 618 | 0.8806 |
| Yata-Garasu | 618 | 0.8806 |
| Injection Fairy Lily | 592 | 0.8436 |
| The Forceful Sentry | 592 | 0.8436 |
| Confiscation | 571 | 0.8136 |
| Delinquent Duo | 571 | 0.8136 |
| Exiled Force | 571 | 0.8136 |
| Jinzo | 571 | 0.8136 |
| Sinister Serpent | 571 | 0.8136 |
| Breaker the Magical Warrior | 552 | 0.7866 |
| Fiber Jar | 536 | 0.7638 |
| Mirror Force | 536 | 0.7638 |
| Monster Reborn | 536 | 0.7638 |
| Tribe-Infecting Virus | 536 | 0.7638 |
| Bell of Destruction | 522 | 0.7438 |
| Change of Heart | 522 | 0.7438 |
| Dark Hole | 522 | 0.7438 |
| Graceful Charity | 522 | 0.7438 |
| Harpie's Feather Duster | 522 | 0.7438 |
| Painful Choice | 522 | 0.7438 |
| Pot of Greed | 522 | 0.7438 |
| Raigeki | 522 | 0.7438 |
| Sangan | 522 | 0.7438 |
| Snatch Steal | 522 | 0.7438 |
| Witch of the Black Forest | 522 | 0.7438 |
| Dark Paladin | 435 | 0.6198 |
| Reaper on the Nightmare | 435 | 0.6198 |
| The Last Warrior from Another Planet | 435 | 0.6198 |
| Darkfire Dragon | 218 | 0.3106 |
| Dragoness the Wicked Knight | 196 | 0.2793 |
| Mask of Restrict | 180 | 0.2565 |
| Empress Judge | 174 | 0.2479 |
| Fiend Skull Dragon | 174 | 0.2479 |
| Punished Eagle | 174 | 0.2479 |
| Roaring Ocean Snake | 174 | 0.2479 |
| Dark Balter the Terrible | 168 | 0.2394 |
| Ryu Senshi | 168 | 0.2394 |
| Thousand-Eyes Restrict | 168 | 0.2394 |
| Royal Decree | 139 | 0.1981 |
| Scapegoat | 139 | 0.1981 |
| Poison of the Old Man | 120 | 0.1710 |
| Book of Moon | 89 | 0.1268 |
| Mystical Space Typhoon | 74 | 0.1054 |
| Mirage of Nightmare | 55 | 0.0784 |
| Call Of The Haunted | 53 | 0.0755 |
| Imperial Order | 52 | 0.0741 |

### Dark Crisis

- **Type:** TCG · **Format:** Vampire · **Cards in pool:** 9 · **Weight sum:** 53725

| Card | Weight | Pull % |
|---|---:|---:|
| Final Attack Orders | 10000 | 18.6133 |
| Gyaku-Gire Panda | 10000 | 18.6133 |
| Ojama Trio | 10000 | 18.6133 |
| Sakuretsu Armor | 10000 | 18.6133 |
| Token Thanksgiving | 10000 | 18.6133 |
| Tsukuyomi | 2500 | 4.6533 |
| D.D. Warrior Lady | 600 | 1.1168 |
| Dark Flare Knight | 600 | 1.1168 |
| Vampire Lord | 25 | 0.0465 |

### Vampire

- **Type:** Usage · **Format:** Vampire · **Cards in pool:** 111 · **Weight sum:** 122928

| Card | Weight | Pull % |
|---|---:|---:|
| Book of Life | 2305 | 1.8751 |
| Bottomless Trap Hole | 2305 | 1.8751 |
| Cannon Soldier | 2305 | 1.8751 |
| Catapult Turtle | 2305 | 1.8751 |
| Cipher Soldier | 2305 | 1.8751 |
| Cold Wave | 2305 | 1.8751 |
| Dark Scorpion - Cliff the Trap Remover | 2305 | 1.8751 |
| Des Koala | 2305 | 1.8751 |
| Electric Snake | 2305 | 1.8751 |
| Gemini Elf | 2305 | 1.8751 |
| Goblin Attack Force | 2305 | 1.8751 |
| Guardian Sphinx | 2305 | 1.8751 |
| Gyaku-Gire Panda | 2305 | 1.8751 |
| Last Will | 2305 | 1.8751 |
| Masked Sorcerer | 2305 | 1.8751 |
| Mother Grizzly | 2305 | 1.8751 |
| Ojama Trio | 2305 | 1.8751 |
| Pyramid Turtle | 2305 | 1.8751 |
| Raigeki Break | 2305 | 1.8751 |
| Secret Barrel | 2305 | 1.8751 |
| Spirit Reaper | 2305 | 1.8751 |
| Summoned Skull | 2305 | 1.8751 |
| Time Seal | 2305 | 1.8751 |
| Waboku | 2305 | 1.8751 |
| White Magical Hat | 2305 | 1.8751 |
| Creature Swap | 1978 | 1.6091 |
| Marauding Captain | 1978 | 1.6091 |
| Morphing Jar | 1978 | 1.6091 |
| Giant Trunade | 1776 | 1.4447 |
| Jar of Greed | 1776 | 1.4447 |
| Just Desserts | 1776 | 1.4447 |
| Kycoo the Ghost Destroyer | 1776 | 1.4447 |
| Solemn Judgment | 1776 | 1.4447 |
| Trap Hole | 1776 | 1.4447 |
| Dark Ruler Ha Des | 1528 | 1.2430 |
| Mage Power | 1528 | 1.2430 |
| Swords of Revealing Light | 1528 | 1.2430 |
| Trap Dustshoot | 1528 | 1.2430 |
| Upstart Goblin | 1528 | 1.2430 |
| Vampire Lord | 1528 | 1.2430 |
| Mystic Tomato | 1375 | 1.1185 |
| The Little Swordsman of Aile | 1375 | 1.1185 |
| Metamorphosis | 1268 | 1.0315 |
| Card Destruction | 1187 | 0.9656 |
| Magic Cylinder | 1187 | 0.9656 |
| Dust Tornado | 1123 | 0.9135 |
| Sasuke Samurai | 1123 | 0.9135 |
| Cyber Jar | 1027 | 0.8354 |
| King Tiger Wanghu | 1027 | 0.8354 |
| Magician of Faith | 990 | 0.8053 |
| Sakuretsu Armor | 903 | 0.7346 |
| Ceasefire | 860 | 0.6996 |
| Don Zaloog | 860 | 0.6996 |
| Gravekeeper's Spy | 860 | 0.6996 |
| Zombyra the Dark | 841 | 0.6841 |
| D.D. Warrior Lady | 766 | 0.6231 |
| Torrential Tribute | 743 | 0.6044 |
| Premature Burial | 681 | 0.5540 |
| Reinforcement of the Army | 681 | 0.5540 |
| Nobleman of Crossout | 670 | 0.5450 |
| Painful Choice | 610 | 0.4962 |
| Yata-Garasu | 584 | 0.4751 |
| Injection Fairy Lily | 563 | 0.4580 |
| Empress Judge | 560 | 0.4556 |
| Giltia the D. Knight | 560 | 0.4556 |
| Skull Knight | 560 | 0.4556 |
| Heavy Storm | 554 | 0.4507 |
| Delinquent Duo | 545 | 0.4433 |
| Fiber Jar | 545 | 0.4433 |
| Jinzo | 545 | 0.4433 |
| Magical Scientist | 537 | 0.4368 |
| Sinister Serpent | 537 | 0.4368 |
| Tribe-Infecting Virus | 537 | 0.4368 |
| Bell of Destruction | 529 | 0.4303 |
| Confiscation | 529 | 0.4303 |
| Exiled Force | 529 | 0.4303 |
| Snatch Steal | 529 | 0.4303 |
| Breaker the Magical Warrior | 522 | 0.4246 |
| Change of Heart | 522 | 0.4246 |
| Dark Hole | 522 | 0.4246 |
| Graceful Charity | 522 | 0.4246 |
| Harpie's Feather Duster | 522 | 0.4246 |
| Mirror Force | 522 | 0.4246 |
| Monster Reborn | 522 | 0.4246 |
| Pot of Greed | 522 | 0.4246 |
| Raigeki | 522 | 0.4246 |
| Sangan | 522 | 0.4246 |
| The Forceful Sentry | 522 | 0.4246 |
| Witch of the Black Forest | 522 | 0.4246 |
| B. Skull Dragon | 427 | 0.3474 |
| Flame Ghost | 427 | 0.3474 |
| Twin-Headed Thunder Dragon | 427 | 0.3474 |
| Punished Eagle | 365 | 0.2969 |
| Roaring Ocean Snake | 250 | 0.2034 |
| The Last Warrior from Another Planet | 250 | 0.2034 |
| Darkfire Dragon | 239 | 0.1944 |
| Dragoness the Wicked Knight | 239 | 0.1944 |
| Reaper on the Nightmare | 208 | 0.1692 |
| Dark Flare Knight | 177 | 0.1440 |
| Fiend Skull Dragon | 171 | 0.1391 |
| Dark Balter the Terrible | 168 | 0.1367 |
| Ryu Senshi | 168 | 0.1367 |
| Thousand-Eyes Restrict | 168 | 0.1367 |
| Poison of the Old Man | 152 | 0.1236 |
| Scapegoat | 126 | 0.1025 |
| Book of Moon | 95 | 0.0773 |
| Royal Decree | 92 | 0.0748 |
| Mystical Space Typhoon | 74 | 0.0602 |
| Call Of The Haunted | 53 | 0.0431 |
| Mirage of Nightmare | 53 | 0.0431 |
| Imperial Order | 52 | 0.0423 |

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

- **Type:** Usage · **Format:** Chaos · **Cards in pool:** 138 · **Weight sum:** 174597

| Card | Weight | Pull % |
|---|---:|---:|
| Catapult Turtle | 3840 | 2.1994 |
| Dark Ruler Ha Des | 3840 | 2.1994 |
| Electric Snake | 3840 | 2.1994 |
| Gigantes | 3840 | 2.1994 |
| Goblin Attack Force | 3840 | 2.1994 |
| Hysteric Fairy | 3840 | 2.1994 |
| Kuriboh | 3840 | 2.1994 |
| Magical Merchant | 3840 | 2.1994 |
| Pyramid Turtle | 3840 | 2.1994 |
| Return from the Different Dimension | 3840 | 2.1994 |
| Spell Canceller | 3840 | 2.1994 |
| White Magical Hat | 3840 | 2.1994 |
| Zaborg the Thunder Monarch | 3840 | 2.1994 |
| Axe of Despair | 2939 | 1.6833 |
| Black Illusion Ritual | 2939 | 1.6833 |
| Jar of Greed | 2939 | 1.6833 |
| Magic Jammer | 2939 | 1.6833 |
| Masked Sorcerer | 2939 | 1.6833 |
| Relinquished | 2939 | 1.6833 |
| Giant Trunade | 2517 | 1.4416 |
| Roulette Barrel | 2517 | 1.4416 |
| Soul Release | 2517 | 1.4416 |
| Spear Dragon | 2517 | 1.4416 |
| The Shallow Grave | 2517 | 1.4416 |
| Vampire Lord | 2517 | 1.4416 |
| Time Seal | 2257 | 1.2927 |
| 4-Starred Ladybug of Doom | 2074 | 1.1879 |
| Dimension Fusion | 2074 | 1.1879 |
| Mystic Tomato | 2074 | 1.1879 |
| Berserk Gorilla | 1937 | 1.1094 |
| Cipher Soldier | 1937 | 1.1094 |
| Sasuke Samurai | 1937 | 1.1094 |
| Skilled Dark Magician | 1937 | 1.1094 |
| Airknight Parshath | 1828 | 1.0470 |
| Exchange | 1828 | 1.0470 |
| Bottomless Trap Hole | 1739 | 0.9960 |
| Don Zaloog | 1739 | 0.9960 |
| King Tiger Wanghu | 1739 | 0.9960 |
| Creature Swap | 1665 | 0.9536 |
| Thunder Nyan Nyan | 1665 | 0.9536 |
| Metamorphosis | 1601 | 0.9170 |
| Cold Wave | 1546 | 0.8855 |
| Tsukuyomi | 1497 | 0.8574 |
| Dust Tornado | 1453 | 0.8322 |
| Spirit Reaper | 1453 | 0.8322 |
| Chaos Sorcerer | 1415 | 0.8104 |
| Solemn Judgment | 1415 | 0.8104 |
| Dark Magician of Chaos | 1379 | 0.7898 |
| Swords of Revealing Light | 1379 | 0.7898 |
| Waboku | 1379 | 0.7898 |
| Sakuretsu Armor | 1318 | 0.7549 |
| Asura Priest | 1266 | 0.7251 |
| Ceasefire | 1221 | 0.6993 |
| Zombyra the Dark | 1221 | 0.6993 |
| Magic Cylinder | 1115 | 0.6386 |
| Morphing Jar | 1115 | 0.6386 |
| Raigeki Break | 1074 | 0.6151 |
| Reinforcement of the Army | 1056 | 0.6048 |
| Thunder Dragon | 1039 | 0.5951 |
| Upstart Goblin | 979 | 0.5607 |
| Trap Dustshoot | 971 | 0.5561 |
| Deepsea Shark | 946 | 0.5418 |
| Cyber Jar | 874 | 0.5006 |
| Gravekeeper's Spy | 864 | 0.4949 |
| Torrential Tribute | 789 | 0.4519 |
| D.D. Warrior Lady | 778 | 0.4456 |
| Magician of Faith | 778 | 0.4456 |
| Kycoo the Ghost Destroyer | 771 | 0.4416 |
| Exiled Force | 761 | 0.4359 |
| Card Destruction | 752 | 0.4307 |
| Yata-Garasu | 727 | 0.4164 |
| Bickuribox | 720 | 0.4124 |
| Labyrinth Tank | 720 | 0.4124 |
| Metal Dragon | 720 | 0.4124 |
| Warrior of Tradition | 720 | 0.4124 |
| Injection Fairy Lily | 698 | 0.3998 |
| Nobleman of Crossout | 698 | 0.3998 |
| Magical Scientist | 634 | 0.3631 |
| Jinzo | 630 | 0.3608 |
| Flame Swordsman | 614 | 0.3517 |
| Skull Knight | 614 | 0.3517 |
| Premature Burial | 610 | 0.3494 |
| Heavy Storm | 599 | 0.3431 |
| Fiber Jar | 580 | 0.3322 |
| Sinister Serpent | 553 | 0.3167 |
| Punished Eagle | 548 | 0.3139 |
| Confiscation | 540 | 0.3093 |
| Sangan | 540 | 0.3093 |
| Breaker the Magical Warrior | 538 | 0.3081 |
| Delinquent Duo | 538 | 0.3081 |
| Mirror Force | 533 | 0.3053 |
| Snatch Steal | 531 | 0.3041 |
| Dark Hole | 529 | 0.3030 |
| Painful Choice | 529 | 0.3030 |
| Tribe-Infecting Virus | 529 | 0.3030 |
| The Forceful Sentry | 527 | 0.3018 |
| Harpie's Feather Duster | 525 | 0.3007 |
| Witch of the Black Forest | 525 | 0.3007 |
| Bell of Destruction | 524 | 0.3001 |
| Black Luster Soldier - Envoy of Beginning | 524 | 0.3001 |
| Change of Heart | 524 | 0.3001 |
| Chaos Emperor Dragon - Envoy of the End | 524 | 0.3001 |
| Graceful Charity | 522 | 0.2990 |
| Monster Reborn | 522 | 0.2990 |
| Pot of Greed | 522 | 0.2990 |
| Raigeki | 522 | 0.2990 |
| Dark Paladin | 502 | 0.2875 |
| Empress Judge | 502 | 0.2875 |
| Karbonala Warrior | 502 | 0.2875 |
| B. Skull Dragon | 400 | 0.2291 |
| Fusionist | 400 | 0.2291 |
| Twin-Headed Thunder Dragon | 400 | 0.2291 |
| Gaia the Dragon Champion | 384 | 0.2199 |
| Musician King | 384 | 0.2199 |
| The Last Warrior from Another Planet | 384 | 0.2199 |
| Dragoness the Wicked Knight | 357 | 0.2045 |
| Flame Ghost | 357 | 0.2045 |
| Darkfire Dragon | 320 | 0.1833 |
| Enemy Controller | 293 | 0.1678 |
| Reaper on the Nightmare | 278 | 0.1592 |
| Giltia the D. Knight | 274 | 0.1569 |
| Emergency Provisions | 251 | 0.1438 |
| Prohibition | 225 | 0.1289 |
| Roaring Ocean Snake | 225 | 0.1289 |
| Royal Oppression | 225 | 0.1289 |
| Dark Flare Knight | 188 | 0.1077 |
| Fiend Skull Dragon | 178 | 0.1019 |
| Ryu Senshi | 174 | 0.0997 |
| Dark Balter the Terrible | 173 | 0.0991 |
| Thousand-Eyes Restrict | 173 | 0.0991 |
| Book of Moon | 166 | 0.0951 |
| Poison of the Old Man | 145 | 0.0830 |
| Royal Decree | 129 | 0.0739 |
| Scapegoat | 129 | 0.0739 |
| Mystical Space Typhoon | 74 | 0.0424 |
| Call Of The Haunted | 55 | 0.0315 |
| Mirage of Nightmare | 54 | 0.0309 |
| Imperial Order | 52 | 0.0298 |

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

- **Type:** TCG · **Format:** Warrior · **Cards in pool:** 16 · **Weight sum:** 160000

| Card | Weight | Pull % |
|---|---:|---:|
| Book of Life | 10000 | 6.2500 |
| Compulsory Evacuation Device | 10000 | 6.2500 |
| Creature Swap | 10000 | 6.2500 |
| Dust Tornado | 10000 | 6.2500 |
| Giant Trunade | 10000 | 6.2500 |
| Heavy Storm | 10000 | 6.2500 |
| Magic Jammer | 10000 | 6.2500 |
| Mystical Space Typhoon | 10000 | 6.2500 |
| Nobleman of Crossout | 10000 | 6.2500 |
| Pot of Greed | 10000 | 6.2500 |
| Pyramid Turtle | 10000 | 6.2500 |
| Ryu Kokki | 10000 | 6.2500 |
| Snatch Steal | 10000 | 6.2500 |
| Spirit Reaper | 10000 | 6.2500 |
| Torrential Tribute | 10000 | 6.2500 |
| Vampire Lord | 10000 | 6.2500 |

### Warrior

- **Type:** Usage · **Format:** Warrior · **Cards in pool:** 182 · **Weight sum:** 250694

| Card | Weight | Pull % |
|---|---:|---:|
| Axe of Despair | 3644 | 1.4536 |
| Big Bang Shot | 3644 | 1.4536 |
| Buster Blader | 3644 | 1.4536 |
| Cyber-Stein | 3644 | 1.4536 |
| Different Dimension Capsule | 3644 | 1.4536 |
| Exile of the Wicked | 3644 | 1.4536 |
| Gaia Power | 3644 | 1.4536 |
| Greenkappa | 3644 | 1.4536 |
| Mother Grizzly | 3644 | 1.4536 |
| Night Assailant | 3644 | 1.4536 |
| Princess of Tsurugi | 3644 | 1.4536 |
| Pyramid Turtle | 3644 | 1.4536 |
| Seven Tools of the Bandit | 3644 | 1.4536 |
| Soul Exchange | 3644 | 1.4536 |
| Stealth Bird | 3644 | 1.4536 |
| The Fiend Megacyber | 3644 | 1.4536 |
| Windstorm of Etaqua | 3644 | 1.4536 |
| Marauding Captain | 3117 | 1.2433 |
| Big Shield Gardna | 2791 | 1.1133 |
| Blast with Chain | 2791 | 1.1133 |
| Chain Disappearance | 2791 | 1.1133 |
| Dark Magician | 2791 | 1.1133 |
| Dark Scorpion - Cliff the Trap Remover | 2791 | 1.1133 |
| Des Koala | 2791 | 1.1133 |
| Exchange | 2791 | 1.1133 |
| Gearfried the Iron Knight | 2791 | 1.1133 |
| Magical Merchant | 2791 | 1.1133 |
| Newdoria | 2791 | 1.1133 |
| Nimble Momonga | 2791 | 1.1133 |
| Nobleman of Extermination | 2791 | 1.1133 |
| Trap Hole | 2791 | 1.1133 |
| Apprentice Magician | 2391 | 0.9538 |
| Magic Jammer | 2391 | 0.9538 |
| Mystic Tomato | 2391 | 0.9538 |
| Sasuke Samurai | 2391 | 0.9538 |
| Spear Dragon | 2391 | 0.9538 |
| Spell Canceller | 2391 | 0.9538 |
| Cannon Soldier | 2144 | 0.8552 |
| Gilasaurus | 2144 | 0.8552 |
| Goblin Attack Force | 2144 | 0.8552 |
| Just Desserts | 2144 | 0.8552 |
| Kuriboh | 2144 | 0.8552 |
| Last Will | 2144 | 0.8552 |
| Mask of Darkness | 2144 | 0.8552 |
| Masked Sorcerer | 2144 | 0.8552 |
| Mystic Plasma Zone | 2144 | 0.8552 |
| Ojama Trio | 2144 | 0.8552 |
| Secret Barrel | 2144 | 0.8552 |
| Waboku | 2144 | 0.8552 |
| Abyss Soldier | 1971 | 0.7862 |
| Catapult Turtle | 1971 | 0.7862 |
| Compulsory Evacuation Device | 1971 | 0.7862 |
| Gigantes | 1971 | 0.7862 |
| Sacred Crane | 1971 | 0.7862 |
| Skilled Dark Magician | 1971 | 0.7862 |
| Token Thanksgiving | 1971 | 0.7862 |
| Dekoichi the Battlechanted Locomotive | 1841 | 0.7344 |
| Dimension Fusion | 1841 | 0.7344 |
| Divine Wrath | 1841 | 0.7344 |
| Gyaku-Gire Panda | 1841 | 0.7344 |
| Magic Drain | 1841 | 0.7344 |
| Vampire Lord | 1841 | 0.7344 |
| Widespread Ruin | 1841 | 0.7344 |
| Giant Rat | 1738 | 0.6933 |
| Hallowed Life Barrier | 1738 | 0.6933 |
| Zaborg the Thunder Monarch | 1738 | 0.6933 |
| Reasoning | 1583 | 0.6314 |
| Monster Gate | 1523 | 0.6075 |
| Monster Reincarnation | 1523 | 0.6075 |
| Pikeru's Circle of Enchantment | 1523 | 0.6075 |
| Return from the Different Dimension | 1424 | 0.5680 |
| Tsukuyomi | 1424 | 0.5680 |
| Jar of Greed | 1383 | 0.5517 |
| Chaos Sorcerer | 1346 | 0.5369 |
| Spirit Reaper | 1346 | 0.5369 |
| D.D. Assailant | 1313 | 0.5237 |
| Asura Priest | 1283 | 0.5118 |
| Ninja Grandmaster Sasuke | 1283 | 0.5118 |
| Zombyra the Dark | 1283 | 0.5118 |
| Berserk Gorilla | 1255 | 0.5006 |
| Giant Trunade | 1205 | 0.4807 |
| Metamorphosis | 1205 | 0.4807 |
| Raigeki Break | 1163 | 0.4639 |
| Gravekeeper's Spy | 1144 | 0.4563 |
| Solemn Judgment | 1144 | 0.4563 |
| Trap Dustshoot | 1144 | 0.4563 |
| Cyber Jar | 1109 | 0.4424 |
| King Tiger Wanghu | 1063 | 0.4240 |
| Airknight Parshath | 1049 | 0.4184 |
| Bottomless Trap Hole | 1049 | 0.4184 |
| Don Zaloog | 1049 | 0.4184 |
| Mystic Swordsman LV2 | 1012 | 0.4037 |
| Morphing Jar | 990 | 0.3949 |
| Magician of Faith | 980 | 0.3909 |
| Upstart Goblin | 961 | 0.3833 |
| Creature Swap | 947 | 0.3778 |
| Blade Knight | 934 | 0.3726 |
| Card Destruction | 934 | 0.3726 |
| Kycoo the Ghost Destroyer | 903 | 0.3602 |
| Cyber Saurus | 897 | 0.3578 |
| Fusionist | 897 | 0.3578 |
| Gaia the Dragon Champion | 897 | 0.3578 |
| Labyrinth Tank | 897 | 0.3578 |
| Rabid Horseman | 897 | 0.3578 |
| Mobius the Frost Monarch | 896 | 0.3574 |
| Smashing Ground | 896 | 0.3574 |
| Cipher Soldier | 869 | 0.3466 |
| Dust Tornado | 857 | 0.3419 |
| Sakuretsu Armor | 857 | 0.3419 |
| Ceasefire | 851 | 0.3395 |
| Injection Fairy Lily | 835 | 0.3331 |
| Magic Cylinder | 819 | 0.3267 |
| D.D. Warrior Lady | 815 | 0.3251 |
| Dark Magician of Chaos | 805 | 0.3211 |
| Reinforcement of the Army | 805 | 0.3211 |
| Dark Paladin | 682 | 0.2720 |
| Nobleman of Crossout | 681 | 0.2716 |
| Swords of Revealing Light | 662 | 0.2641 |
| Fiber Jar | 639 | 0.2549 |
| Sinister Serpent | 634 | 0.2529 |
| Exiled Force | 624 | 0.2489 |
| Jinzo | 603 | 0.2405 |
| Magical Scientist | 595 | 0.2373 |
| Charubin the Fire Knight | 582 | 0.2322 |
| Black Luster Soldier - Envoy of Beginning | 565 | 0.2254 |
| Painful Choice | 556 | 0.2218 |
| Confiscation | 550 | 0.2194 |
| Tribe-Infecting Virus | 548 | 0.2186 |
| The Forceful Sentry | 545 | 0.2174 |
| Breaker the Magical Warrior | 542 | 0.2162 |
| Torrential Tribute | 542 | 0.2162 |
| Premature Burial | 540 | 0.2154 |
| Snatch Steal | 535 | 0.2134 |
| Change of Heart | 533 | 0.2126 |
| Bell of Destruction | 528 | 0.2106 |
| Heavy Storm | 526 | 0.2098 |
| Pot of Greed | 522 | 0.2082 |
| Flame Swordsman | 520 | 0.2074 |
| Karbonala Warrior | 520 | 0.2074 |
| Empress Judge | 477 | 0.1903 |
| Musician King | 418 | 0.1667 |
| Punished Eagle | 418 | 0.1667 |
| Twin-Headed Thunder Dragon | 397 | 0.1584 |
| Chain Energy | 364 | 0.1452 |
| Fairy Box | 364 | 0.1452 |
| Light of Intervention | 364 | 0.1452 |
| Mask of Restrict | 364 | 0.1452 |
| Mirror Wall | 364 | 0.1452 |
| Prohibition | 364 | 0.1452 |
| Robbin' Goblin | 364 | 0.1452 |
| Flame Ghost | 311 | 0.1241 |
| Final Attack Orders | 279 | 0.1113 |
| Poison of the Old Man | 279 | 0.1113 |
| Royal Oppression | 279 | 0.1113 |
| Master of Oz | 273 | 0.1089 |
| Roaring Ocean Snake | 269 | 0.1073 |
| Giltia the D. Knight | 260 | 0.1037 |
| Gravity Bind | 239 | 0.0953 |
| Messenger of Peace | 239 | 0.0953 |
| Wave-Motion Cannon | 239 | 0.0953 |
| The Last Warrior from Another Planet | 236 | 0.0941 |
| Dragoness the Wicked Knight | 228 | 0.0909 |
| Level Limit - Area B | 214 | 0.0854 |
| Ojama King | 212 | 0.0846 |
| Darkfire Dragon | 210 | 0.0838 |
| Dark Flare Knight | 201 | 0.0802 |
| Reaper on the Nightmare | 197 | 0.0786 |
| Xing Zhen Hu | 197 | 0.0786 |
| Fiend Skull Dragon | 187 | 0.0746 |
| Dark Blade the Dragon Knight | 179 | 0.0714 |
| Thousand-Eyes Restrict | 179 | 0.0714 |
| Ryu Senshi | 177 | 0.0706 |
| Dark Balter the Terrible | 176 | 0.0702 |
| Mystik Wok | 122 | 0.0487 |
| Enemy Controller | 120 | 0.0479 |
| Royal Decree | 114 | 0.0455 |
| Emergency Provisions | 109 | 0.0435 |
| Scapegoat | 89 | 0.0355 |
| Book of Moon | 86 | 0.0343 |
| Mirage of Nightmare | 67 | 0.0267 |
| Call Of The Haunted | 55 | 0.0219 |
| Mystical Space Typhoon | 53 | 0.0211 |

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

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 31 · **Weight sum:** 201500

| Card | Weight | Pull % |
|---|---:|---:|
| Apprentice Magician | 10000 | 4.9628 |
| Different Dimension Capsule | 10000 | 4.9628 |
| Final Attack Orders | 10000 | 4.9628 |
| Gravekeeper's Spy | 10000 | 4.9628 |
| Gyaku-Gire Panda | 10000 | 4.9628 |
| Magical Merchant | 10000 | 4.9628 |
| Metamorphosis | 10000 | 4.9628 |
| My Body as a Shield | 10000 | 4.9628 |
| Ojama Trio | 10000 | 4.9628 |
| Poison of the Old Man | 10000 | 4.9628 |
| Raigeki Break | 10000 | 4.9628 |
| Reasoning | 10000 | 4.9628 |
| Roulette Barrel | 10000 | 4.9628 |
| Sakuretsu Armor | 10000 | 4.9628 |
| Secret Barrel | 10000 | 4.9628 |
| Token Thanksgiving | 10000 | 4.9628 |
| Tsukuyomi | 10000 | 4.9628 |
| Wave-Motion Cannon | 10000 | 4.9628 |
| Big Bang Shot | 2500 | 1.2407 |
| Des Koala | 2500 | 1.2407 |
| Magical Scientist | 2500 | 1.2407 |
| Newdoria | 2500 | 1.2407 |
| Reaper on the Nightmare | 2500 | 1.2407 |
| Skilled Dark Magician | 2500 | 1.2407 |
| Spirit Reaper | 2500 | 1.2407 |
| Tribe-Infecting Virus | 2500 | 1.2407 |
| D.D. Warrior Lady | 600 | 0.2978 |
| Dark Flare Knight | 600 | 0.2978 |
| Breaker the Magical Warrior | 100 | 0.0496 |
| Dark Paladin | 100 | 0.0496 |
| Spell Canceller | 100 | 0.0496 |

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

- **Type:** TCG · **Format:** Goat · **Cards in pool:** 113 · **Weight sum:** 727900

| Card | Weight | Pull % |
|---|---:|---:|
| Asura Priest | 10000 | 1.3738 |
| Axe Raider | 10000 | 1.3738 |
| Baby Dragon | 10000 | 1.3738 |
| Battle Steer | 10000 | 1.3738 |
| Black Illusion Ritual | 10000 | 1.3738 |
| Blackland Fire Dragon | 10000 | 1.3738 |
| Bladefly | 10000 | 1.3738 |
| Block Attack | 10000 | 1.3738 |
| Book of Moon | 10000 | 1.3738 |
| Bottomless Trap Hole | 10000 | 1.3738 |
| Crass Clown | 10000 | 1.3738 |
| Dark-Piercing Light | 10000 | 1.3738 |
| Dragon Piper | 10000 | 1.3738 |
| Dragon Seeker | 10000 | 1.3738 |
| Dream Clown | 10000 | 1.3738 |
| Exiled Force | 10000 | 1.3738 |
| Fusion Sage | 10000 | 1.3738 |
| Gazelle the King of Mythical Beasts | 10000 | 1.3738 |
| Gemini Elf | 10000 | 1.3738 |
| Gilasaurus | 10000 | 1.3738 |
| Goddess with the Third Eye | 10000 | 1.3738 |
| Great Moth | 10000 | 1.3738 |
| Greenkappa | 10000 | 1.3738 |
| Hoshiningen | 10000 | 1.3738 |
| Illusionist Faceless Mage | 10000 | 1.3738 |
| Jirai Gumo | 10000 | 1.3738 |
| King Tiger Wanghu | 10000 | 1.3738 |
| King of Yamimakai | 10000 | 1.3738 |
| Launcher Spider | 10000 | 1.3738 |
| Little Chimera | 10000 | 1.3738 |
| Lord of the Lamp | 10000 | 1.3738 |
| Machine King | 10000 | 1.3738 |
| Mechanicalchaser | 10000 | 1.3738 |
| Milus Radiant | 10000 | 1.3738 |
| Mirage of Nightmare | 10000 | 1.3738 |
| Muka Muka | 10000 | 1.3738 |
| Mystic Horseman | 10000 | 1.3738 |
| Needle Worm | 10000 | 1.3738 |
| Princess of Tsurugi | 10000 | 1.3738 |
| Rabid Horseman | 10000 | 1.3738 |
| Restructer Revolution | 10000 | 1.3738 |
| Robbin' Goblin | 10000 | 1.3738 |
| Ryu-Kishin Powered | 10000 | 1.3738 |
| Saggi the Dark Clown | 10000 | 1.3738 |
| Shadow Ghoul | 10000 | 1.3738 |
| Share the Pain | 10000 | 1.3738 |
| Shield & Sword | 10000 | 1.3738 |
| Skull Lair | 10000 | 1.3738 |
| Soul of the Pure | 10000 | 1.3738 |
| Star Boy | 10000 | 1.3738 |
| The Bistro Butcher | 10000 | 1.3738 |
| The Immortal of Thunder | 10000 | 1.3738 |
| The Unhappy Maiden | 10000 | 1.3738 |
| The Wicked Worm Beast | 10000 | 1.3738 |
| Thousand Dragon | 10000 | 1.3738 |
| Thunder Dragon | 10000 | 1.3738 |
| Tiger Axe | 10000 | 1.3738 |
| Toon Cannon Soldier | 10000 | 1.3738 |
| Toon Table of Contents | 10000 | 1.3738 |
| Trap Dustshoot | 10000 | 1.3738 |
| Tremendous Fire | 10000 | 1.3738 |
| Twin-Headed Thunder Dragon | 10000 | 1.3738 |
| Two-Headed King Rex | 10000 | 1.3738 |
| Witch's Apprentice | 10000 | 1.3738 |
| Yata-Garasu | 10000 | 1.3738 |
| Zombyra the Dark | 10000 | 1.3738 |
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
| Vampire Lord | 100 | 0.0137 |

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

- **Type:** Usage · **Format:** Goat · **Cards in pool:** 256 · **Weight sum:** 818725

| Card | Weight | Pull % |
|---|---:|---:|
| Backup Soldier | 13242 | 1.6174 |
| Catapult Turtle | 13242 | 1.6174 |
| Dark Magician | 13242 | 1.6174 |
| Dark Ruler Ha Des | 13242 | 1.6174 |
| Dragon Zombie | 13242 | 1.6174 |
| Gaia Power | 13242 | 1.6174 |
| Giant Germ | 13242 | 1.6174 |
| Graverobber | 13242 | 1.6174 |
| Hiro's Shadow Scout | 13242 | 1.6174 |
| Kuwagata a | 13242 | 1.6174 |
| Lightforce Sword | 13242 | 1.6174 |
| Minar | 13242 | 1.6174 |
| Restructer Revolution | 13242 | 1.6174 |
| Sonic Bird | 13242 | 1.6174 |
| Thunder Nyan Nyan | 13242 | 1.6174 |
| Two-Pronged Attack | 13242 | 1.6174 |
| Giant Soldier of Stone | 10065 | 1.2294 |
| Gilasaurus | 10065 | 1.2294 |
| Guardian Sphinx | 10065 | 1.2294 |
| Mother Grizzly | 10065 | 1.2294 |
| Pikeru's Circle of Enchantment | 10065 | 1.2294 |
| The Fiend Megacyber | 10065 | 1.2294 |
| Different Dimension Capsule | 8576 | 1.0475 |
| Dream Clown | 8576 | 1.0475 |
| Exodia the Forbidden One | 8576 | 1.0475 |
| Left Arm of the Forbidden One | 8576 | 1.0475 |
| Left Leg of the Forbidden One | 8576 | 1.0475 |
| Right Arm of the Forbidden One | 8576 | 1.0475 |
| Right Leg of the Forbidden One | 8576 | 1.0475 |
| The Little Swordsman of Aile | 8576 | 1.0475 |
| The Wicked Worm Beast | 8576 | 1.0475 |
| Windstorm of Etaqua | 8576 | 1.0475 |
| D.D. Survivor | 7657 | 0.9352 |
| Spear Dragon | 7657 | 0.9352 |
| Time Wizard | 7657 | 0.9352 |
| Marauding Captain | 7310 | 0.8929 |
| Axe of Despair | 7013 | 0.8566 |
| Black Pendant | 7013 | 0.8566 |
| Morphing Jar #2 | 7013 | 0.8566 |
| The Shallow Grave | 7013 | 0.8566 |
| Trap Hole | 7013 | 0.8566 |
| Acid Trap Hole | 6528 | 0.7973 |
| Des Lacooda | 6528 | 0.7973 |
| Exchange | 6528 | 0.7973 |
| Nimble Momonga | 6528 | 0.7973 |
| Trap Master | 6528 | 0.7973 |
| Tribute to The Doomed | 6528 | 0.7973 |
| Electric Snake | 6145 | 0.7506 |
| Penguin Soldier | 6145 | 0.7506 |
| Shield & Sword | 6145 | 0.7506 |
| Stealth Bird | 6145 | 0.7506 |
| Banisher of the Light | 5832 | 0.7123 |
| Fake Trap | 5569 | 0.6802 |
| Thestalos the Firestorm Monarch | 5569 | 0.6802 |
| Blast with Chain | 5344 | 0.6527 |
| Divine Wrath | 5344 | 0.6527 |
| Gearfried the Iron Knight | 5344 | 0.6527 |
| Senju of the Thousand Hands | 5149 | 0.6289 |
| Book of Life | 4345 | 0.5307 |
| Greenkappa | 4345 | 0.5307 |
| Jirai Gumo | 4345 | 0.5307 |
| Black Illusion Ritual | 4250 | 0.5191 |
| Magic Drain | 4250 | 0.5191 |
| Relinquished | 4250 | 0.5191 |
| Ryu Kokki | 4250 | 0.5191 |
| Chain Disappearance | 4079 | 0.4982 |
| Cold Wave | 4003 | 0.4889 |
| Megamorph | 3864 | 0.4720 |
| Golem Sentry | 3801 | 0.4643 |
| Pyramid Turtle | 3741 | 0.4569 |
| Toon Cannon Soldier | 3741 | 0.4569 |
| Vampire Lord | 3712 | 0.4534 |
| Soul Exchange | 3685 | 0.4501 |
| Cannon Soldier | 3631 | 0.4435 |
| Milus Radiant | 3532 | 0.4314 |
| Time Seal | 3532 | 0.4314 |
| Toon Table of Contents | 3401 | 0.4154 |
| Bazoo the Soul-Eater | 3360 | 0.4104 |
| Big Bang Shot | 3322 | 0.4058 |
| Flower Wolf | 3308 | 0.4040 |
| Nobleman of Extermination | 3216 | 0.3928 |
| Apprentice Magician | 3091 | 0.3775 |
| Chiron the Mage | 3091 | 0.3775 |
| Soul Release | 3091 | 0.3775 |
| Mage Power | 3062 | 0.3740 |
| Token Thanksgiving | 3008 | 0.3674 |
| Roulette Barrel | 2887 | 0.3526 |
| Newdoria | 2864 | 0.3498 |
| Waboku | 2864 | 0.3498 |
| Hallowed Life Barrier | 2843 | 0.3472 |
| Big Shield Gardna | 2822 | 0.3447 |
| Gigantes | 2822 | 0.3447 |
| Monster Reincarnation | 2801 | 0.3421 |
| Hysteric Fairy | 2724 | 0.3327 |
| Des Koala | 2706 | 0.3305 |
| Goblin Attack Force | 2688 | 0.3283 |
| Mask of Darkness | 2688 | 0.3283 |
| Dark Mimic LV1 | 2671 | 0.3262 |
| Sacred Crane | 2671 | 0.3262 |
| Dimension Fusion | 2638 | 0.3222 |
| Last Will | 2638 | 0.3222 |
| Magic Jammer | 2591 | 0.3165 |
| Monster Gate | 2547 | 0.3111 |
| Reasoning | 2547 | 0.3111 |
| Kaminari Attack | 2510 | 0.3066 |
| Seven Tools of the Bandit | 2372 | 0.2897 |
| Spell Canceller | 2217 | 0.2708 |
| Rabid Horseman | 2136 | 0.2609 |
| Just Desserts | 2101 | 0.2566 |
| Return from the Different Dimension | 2101 | 0.2566 |
| Ojama Trio | 2053 | 0.2508 |
| Magical Merchant | 2010 | 0.2455 |
| Secret Barrel | 1993 | 0.2434 |
| Skilled Dark Magician | 1987 | 0.2427 |
| Giant Rat | 1981 | 0.2420 |
| Bottomless Trap Hole | 1933 | 0.2361 |
| Cyber-Stein | 1933 | 0.2361 |
| Gyaku-Gire Panda | 1888 | 0.2306 |
| Airknight Parshath | 1870 | 0.2284 |
| Compulsory Evacuation Device | 1865 | 0.2278 |
| Des Wombat | 1861 | 0.2273 |
| Abyss Soldier | 1810 | 0.2211 |
| D.D. Assailant | 1810 | 0.2211 |
| Mystic Tomato | 1798 | 0.2196 |
| Threatening Roar | 1775 | 0.2168 |
| Kwagar Hercules | 1743 | 0.2129 |
| Dark Scorpion - Cliff the Trap Remover | 1706 | 0.2084 |
| Kuriboh | 1706 | 0.2084 |
| Zaborg the Thunder Monarch | 1648 | 0.2013 |
| United We Stand | 1600 | 0.1954 |
| Widespread Ruin | 1580 | 0.1930 |
| Brain Control | 1537 | 0.1877 |
| Giant Trunade | 1494 | 0.1825 |
| Berserk Gorilla | 1473 | 0.1799 |
| Empress Judge | 1446 | 0.1766 |
| Phoenix Wing Wind Blast | 1434 | 0.1752 |
| Magic Cylinder | 1379 | 0.1684 |
| Spirit Reaper | 1355 | 0.1655 |
| King Tiger Wanghu | 1334 | 0.1629 |
| Smashing Ground | 1327 | 0.1621 |
| Ultimate Offering | 1324 | 0.1617 |
| Dark Magician of Chaos | 1316 | 0.1607 |
| Creature Swap | 1264 | 0.1544 |
| Don Zaloog | 1230 | 0.1502 |
| Ninja Grandmaster Sasuke | 1220 | 0.1490 |
| Skull Knight | 1193 | 0.1457 |
| Warrior of Tradition | 1193 | 0.1457 |
| Ceasefire | 1161 | 0.1418 |
| Deepsea Shark | 1159 | 0.1416 |
| Metal Dragon | 1159 | 0.1416 |
| Cipher Soldier | 1154 | 0.1410 |
| Metamorphosis | 1151 | 0.1406 |
| Mystic Swordsman LV2 | 1115 | 0.1362 |
| Gaia the Dragon Champion | 1099 | 0.1342 |
| Lightning Vortex | 1098 | 0.1341 |
| Injection Fairy Lily | 1089 | 0.1330 |
| Labyrinth Tank | 1073 | 0.1311 |
| Blade Knight | 1068 | 0.1304 |
| Cyber Jar | 1056 | 0.1290 |
| Asura Priest | 1051 | 0.1284 |
| Raigeki Break | 1032 | 0.1260 |
| Rush Recklessly | 1006 | 0.1229 |
| Jar of Greed | 997 | 0.1218 |
| Zombyra the Dark | 996 | 0.1217 |
| Mobius the Frost Monarch | 964 | 0.1177 |
| Jinzo | 961 | 0.1174 |
| Thunder Dragon | 940 | 0.1148 |
| Reinforcement of the Army | 938 | 0.1146 |
| Dekoichi the Battlechanted Locomotive | 927 | 0.1132 |
| Night Assailant | 925 | 0.1130 |
| Morphing Jar | 914 | 0.1116 |
| Solemn Judgment | 910 | 0.1111 |
| Dust Tornado | 903 | 0.1103 |
| Kycoo the Ghost Destroyer | 888 | 0.1085 |
| Tsukuyomi | 884 | 0.1080 |
| Chaos Sorcerer | 883 | 0.1079 |
| Gravekeeper's Spy | 880 | 0.1075 |
| Mind Control | 854 | 0.1043 |
| Upstart Goblin | 847 | 0.1035 |
| Flame Swordsman | 836 | 0.1021 |
| Musician King | 836 | 0.1021 |
| Magician of Faith | 832 | 0.1016 |
| Trap Dustshoot | 829 | 0.1013 |
| Sakuretsu Armor | 823 | 0.1005 |
| Fusionist | 789 | 0.0964 |
| Mask of Restrict | 765 | 0.0934 |
| Mirror Wall | 765 | 0.0934 |
| Bickuribox | 744 | 0.0909 |
| Card Destruction | 738 | 0.0901 |
| Cyber Saurus | 737 | 0.0900 |
| Swords of Revealing Light | 713 | 0.0871 |
| Exiled Force | 704 | 0.0860 |
| Robbin' Goblin | 701 | 0.0856 |
| Solemn Wishes | 701 | 0.0856 |
| Dark Paladin | 690 | 0.0843 |
| Roaring Ocean Snake | 680 | 0.0831 |
| D.D. Warrior Lady | 679 | 0.0829 |
| Premature Burial | 663 | 0.0810 |
| Nobleman of Crossout | 662 | 0.0809 |
| Charubin the Fire Knight | 657 | 0.0802 |
| Toon World | 652 | 0.0796 |
| Thousand Dragon | 640 | 0.0782 |
| B. Skull Dragon | 636 | 0.0777 |
| Karbonala Warrior | 628 | 0.0767 |
| Final Attack Orders | 614 | 0.0750 |
| Limiter Removal | 614 | 0.0750 |
| Sinister Serpent | 603 | 0.0737 |
| Prohibition | 583 | 0.0712 |
| Sangan | 572 | 0.0699 |
| Torrential Tribute | 557 | 0.0680 |
| Light of Intervention | 556 | 0.0679 |
| Tribe-Infecting Virus | 544 | 0.0664 |
| Black Luster Soldier - Envoy of Beginning | 543 | 0.0663 |
| Delinquent Duo | 537 | 0.0656 |
| Skull Lair | 534 | 0.0652 |
| Breaker the Magical Warrior | 532 | 0.0650 |
| Mirror Force | 529 | 0.0646 |
| Bell of Destruction | 526 | 0.0642 |
| Snatch Steal | 526 | 0.0642 |
| Heavy Storm | 524 | 0.0640 |
| Graceful Charity | 522 | 0.0638 |
| Pot of Greed | 522 | 0.0638 |
| Emergency Provisions | 475 | 0.0580 |
| Dark Flare Knight | 424 | 0.0518 |
| Giltia the D. Knight | 419 | 0.0512 |
| Royal Oppression | 358 | 0.0437 |
| Poison of the Old Man | 344 | 0.0420 |
| My Body as a Shield | 332 | 0.0406 |
| Twin-Headed Thunder Dragon | 330 | 0.0403 |
| Chain Energy | 328 | 0.0401 |
| Flame Ghost | 276 | 0.0337 |
| Messenger of Peace | 270 | 0.0330 |
| Wave-Motion Cannon | 257 | 0.0314 |
| Enemy Controller | 241 | 0.0294 |
| Master of Oz | 203 | 0.0248 |
| Dragoness the Wicked Knight | 202 | 0.0247 |
| Ojama King | 194 | 0.0237 |
| King Dragun | 191 | 0.0233 |
| The Last Warrior from Another Planet | 191 | 0.0233 |
| Darkfire Dragon | 187 | 0.0228 |
| Fiend Skull Dragon | 185 | 0.0226 |
| Gravity Bind | 184 | 0.0225 |
| Reaper on the Nightmare | 184 | 0.0225 |
| Dark Blade the Dragon Knight | 183 | 0.0224 |
| Gatling Dragon | 179 | 0.0219 |
| Ryu Senshi | 178 | 0.0217 |
| Dark Balter the Terrible | 175 | 0.0214 |
| Thousand-Eyes Restrict | 173 | 0.0211 |
| Level Limit - Area B | 167 | 0.0204 |
| Xing Zhen Hu | 144 | 0.0176 |
| Royal Decree | 128 | 0.0156 |
| Scapegoat | 110 | 0.0134 |
| Mystik Wok | 96 | 0.0117 |
| Book of Moon | 83 | 0.0101 |
| Call Of The Haunted | 70 | 0.0085 |
| Mystical Space Typhoon | 53 | 0.0065 |

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

- **Type:** Usage · **Format:** Cyber · **Cards in pool:** 139 · **Weight sum:** 157405

| Card | Weight | Pull % |
|---|---:|---:|
| Apprentice Magician | 2978 | 1.8919 |
| Cannon Soldier | 2978 | 1.8919 |
| Dark Scorpion - Cliff the Trap Remover | 2978 | 1.8919 |
| Different Dimension Capsule | 2978 | 1.8919 |
| Greenkappa | 2978 | 1.8919 |
| Return from the Different Dimension | 2978 | 1.8919 |
| Sasuke Samurai | 2978 | 1.8919 |
| Soul Exchange | 2978 | 1.8919 |
| Stealth Bird | 2978 | 1.8919 |
| Thestalos the Firestorm Monarch | 2978 | 1.8919 |
| Thunder Dragon | 2978 | 1.8919 |
| Upstart Goblin | 2550 | 1.6200 |
| Des Wombat | 2286 | 1.4523 |
| Electric Snake | 2286 | 1.4523 |
| Jar of Greed | 2286 | 1.4523 |
| Monster Gate | 2286 | 1.4523 |
| Monster Reincarnation | 2286 | 1.4523 |
| Raigeki Break | 2286 | 1.4523 |
| Reasoning | 2286 | 1.4523 |
| Sacred Crane | 2286 | 1.4523 |
| Abyss Soldier | 1962 | 1.2465 |
| Card Destruction | 1962 | 1.2465 |
| Chain Disappearance | 1962 | 1.2465 |
| Giant Rat | 1962 | 1.2465 |
| Gyaku-Gire Panda | 1962 | 1.2465 |
| Spell Canceller | 1962 | 1.2465 |
| Token Thanksgiving | 1962 | 1.2465 |
| Widespread Ruin | 1962 | 1.2465 |
| Blade Knight | 1761 | 1.1188 |
| Des Koala | 1761 | 1.1188 |
| Just Desserts | 1761 | 1.1188 |
| Magical Merchant | 1761 | 1.1188 |
| Nimble Momonga | 1761 | 1.1188 |
| Ojama Trio | 1761 | 1.1188 |
| Secret Barrel | 1761 | 1.1188 |
| Big Shield Gardna | 1621 | 1.0298 |
| Chiron the Mage | 1621 | 1.0298 |
| Megamorph | 1621 | 1.0298 |
| Ninja Grandmaster Sasuke | 1621 | 1.0298 |
| Dark Magician of Chaos | 1516 | 0.9631 |
| Don Zaloog | 1516 | 0.9631 |
| United We Stand | 1516 | 0.9631 |
| Airknight Parshath | 1364 | 0.8666 |
| D.D. Assailant | 1364 | 0.8666 |
| Dekoichi the Battlechanted Locomotive | 1364 | 0.8666 |
| Kycoo the Ghost Destroyer | 1364 | 0.8666 |
| Solemn Judgment | 1364 | 0.8666 |
| Waboku | 1364 | 0.8666 |
| Cipher Soldier | 1307 | 0.8303 |
| Reinforcement of the Army | 1307 | 0.8303 |
| Smashing Ground | 1307 | 0.8303 |
| Cold Wave | 1258 | 0.7992 |
| Creature Swap | 1236 | 0.7852 |
| Giant Trunade | 1215 | 0.7719 |
| Mystic Swordsman LV2 | 1215 | 0.7719 |
| Asura Priest | 1114 | 0.7077 |
| Gravekeeper's Spy | 1114 | 0.7077 |
| Mind Control | 1114 | 0.7077 |
| Chaos Sorcerer | 1087 | 0.6906 |
| Injection Fairy Lily | 1087 | 0.6906 |
| Magic Cylinder | 1087 | 0.6906 |
| Mystic Tomato | 1063 | 0.6753 |
| Bottomless Trap Hole | 1040 | 0.6607 |
| Zaborg the Thunder Monarch | 1040 | 0.6607 |
| Cyber Jar | 1019 | 0.6474 |
| Exarion Universe | 1019 | 0.6474 |
| Tsukuyomi | 1019 | 0.6474 |
| Dust Tornado | 1000 | 0.6353 |
| Last Will | 1000 | 0.6353 |
| King Tiger Wanghu | 966 | 0.6137 |
| Kuriboh | 966 | 0.6137 |
| Metamorphosis | 950 | 0.6035 |
| Brain Control | 909 | 0.5775 |
| Threatening Roar | 885 | 0.5622 |
| Sakuretsu Armor | 874 | 0.5553 |
| Cyber-Stein | 863 | 0.5483 |
| Spirit Reaper | 863 | 0.5483 |
| Ceasefire | 853 | 0.5419 |
| Trap Dustshoot | 853 | 0.5419 |
| Mobius the Frost Monarch | 844 | 0.5362 |
| Morphing Jar | 826 | 0.5248 |
| Magician of Faith | 809 | 0.5140 |
| Cyber Dragon | 787 | 0.5000 |
| Swords of Revealing Light | 761 | 0.4835 |
| Karbonala Warrior | 729 | 0.4631 |
| Roaring Ocean Snake | 729 | 0.4631 |
| Twin-Headed Thunder Dragon | 729 | 0.4631 |
| Lightning Vortex | 713 | 0.4530 |
| Nobleman of Crossout | 665 | 0.4225 |
| Sinister Serpent | 665 | 0.4225 |
| Jinzo | 655 | 0.4161 |
| Exiled Force | 613 | 0.3894 |
| D.D. Warrior Lady | 592 | 0.3761 |
| Sangan | 580 | 0.3685 |
| Tribe-Infecting Virus | 580 | 0.3685 |
| Flame Ghost | 556 | 0.3532 |
| Fusionist | 556 | 0.3532 |
| Giltia the D. Knight | 556 | 0.3532 |
| Breaker the Magical Warrior | 554 | 0.3520 |
| Premature Burial | 550 | 0.3494 |
| Black Luster Soldier - Envoy of Beginning | 545 | 0.3462 |
| Delinquent Duo | 541 | 0.3437 |
| Bell of Destruction | 529 | 0.3361 |
| Snatch Steal | 529 | 0.3361 |
| Torrential Tribute | 529 | 0.3361 |
| Mirror Force | 525 | 0.3335 |
| Graceful Charity | 522 | 0.3316 |
| Heavy Storm | 522 | 0.3316 |
| Pot of Greed | 522 | 0.3316 |
| Gaia the Dragon Champion | 341 | 0.2166 |
| Dark Flare Knight | 297 | 0.1887 |
| Xing Zhen Hu | 297 | 0.1887 |
| Dragoness the Wicked Knight | 228 | 0.1448 |
| Poison of the Old Man | 228 | 0.1448 |
| Darkfire Dragon | 207 | 0.1315 |
| Ojama King | 204 | 0.1296 |
| Master of Oz | 201 | 0.1277 |
| Cyber End Dragon | 181 | 0.1150 |
| King Dragun | 177 | 0.1124 |
| Ryu Senshi | 177 | 0.1124 |
| The Last Warrior from Another Planet | 177 | 0.1124 |
| Dark Blade the Dragon Knight | 176 | 0.1118 |
| Reaper on the Nightmare | 174 | 0.1105 |
| Cyber Twin Dragon | 171 | 0.1086 |
| Fiend Skull Dragon | 171 | 0.1086 |
| Gatling Dragon | 171 | 0.1086 |
| Dark Balter the Terrible | 170 | 0.1080 |
| Thousand-Eyes Restrict | 170 | 0.1080 |
| Wave-Motion Cannon | 162 | 0.1029 |
| Enemy Controller | 151 | 0.0959 |
| Level Limit - Area B | 151 | 0.0959 |
| Gravity Bind | 139 | 0.0883 |
| Limiter Removal | 117 | 0.0743 |
| Mystik Wok | 104 | 0.0661 |
| Scapegoat | 100 | 0.0635 |
| Royal Decree | 93 | 0.0591 |
| Book of Moon | 80 | 0.0508 |
| Call Of The Haunted | 59 | 0.0375 |
| Mystical Space Typhoon | 53 | 0.0337 |

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

- **Type:** Usage · **Format:** Reaper · **Cards in pool:** 210 · **Weight sum:** 365991

| Card | Weight | Pull % |
|---|---:|---:|
| Acid Trap Hole | 5422 | 1.4815 |
| Asura Priest | 5422 | 1.4815 |
| De-Spell | 5422 | 1.4815 |
| Giant Germ | 5422 | 1.4815 |
| Magic Drain | 5422 | 1.4815 |
| Marauding Captain | 5422 | 1.4815 |
| Masked Dragon | 5422 | 1.4815 |
| Reasoning | 5422 | 1.4815 |
| Remove Trap | 5422 | 1.4815 |
| Rising Air Current | 5422 | 1.4815 |
| Roulette Barrel | 5422 | 1.4815 |
| Sacred Crane | 5422 | 1.4815 |
| Time Wizard | 5422 | 1.4815 |
| UFO Turtle | 5422 | 1.4815 |
| Abyss Soldier | 4629 | 1.2648 |
| Black Illusion Ritual | 4138 | 1.1306 |
| Blast with Chain | 4138 | 1.1306 |
| Dimension Fusion | 4138 | 1.1306 |
| Divine Wrath | 4138 | 1.1306 |
| Electric Snake | 4138 | 1.1306 |
| Fissure | 4138 | 1.1306 |
| Princess of Tsurugi | 4138 | 1.1306 |
| Relinquished | 4138 | 1.1306 |
| Time Seal | 4138 | 1.1306 |
| Warrior Elimination | 4138 | 1.1306 |
| Big Shield Gardna | 3537 | 0.9664 |
| Mage Power | 3537 | 0.9664 |
| Magic Jammer | 3537 | 0.9664 |
| Mind Control | 3537 | 0.9664 |
| Nimble Momonga | 3537 | 0.9664 |
| Skelengel | 3537 | 0.9664 |
| The Wicked Worm Beast | 3537 | 0.9664 |
| Tribute to The Doomed | 3537 | 0.9664 |
| Des Lacooda | 3165 | 0.8648 |
| Flying Kamakiri #1 | 3165 | 0.8648 |
| Goblin Elite Attack Force | 3165 | 0.8648 |
| Cannon Soldier | 2905 | 0.7937 |
| Mask of Darkness | 2905 | 0.7937 |
| Sand Moth | 2905 | 0.7937 |
| Sasuke Samurai | 2905 | 0.7937 |
| Secret Barrel | 2905 | 0.7937 |
| Seven Tools of the Bandit | 2905 | 0.7937 |
| Apprentice Magician | 2709 | 0.7402 |
| Dark Magician of Chaos | 2709 | 0.7402 |
| Goblin Attack Force | 2709 | 0.7402 |
| Thunder Dragon | 2709 | 0.7402 |
| Berserk Gorilla | 2555 | 0.6981 |
| Book of Life | 2555 | 0.6981 |
| Exarion Universe | 2555 | 0.6981 |
| Hallowed Life Barrier | 2555 | 0.6981 |
| Just Desserts | 2555 | 0.6981 |
| Megamorph | 2555 | 0.6981 |
| Milus Radiant | 2555 | 0.6981 |
| Nobleman of Extermination | 2555 | 0.6981 |
| Ojama Trio | 2555 | 0.6981 |
| Vampire Lord | 2555 | 0.6981 |
| Upstart Goblin | 2488 | 0.6798 |
| Gigantes | 2428 | 0.6634 |
| Jar of Greed | 2428 | 0.6634 |
| Pyramid Turtle | 2428 | 0.6634 |
| Ryu Kokki | 2428 | 0.6634 |
| Skilled Dark Magician | 2428 | 0.6634 |
| Compulsory Evacuation Device | 2322 | 0.6344 |
| Gyaku-Gire Panda | 2322 | 0.6344 |
| Airknight Parshath | 2231 | 0.6096 |
| Chiron the Mage | 2231 | 0.6096 |
| Dark Mimic LV1 | 2231 | 0.6096 |
| Cold Wave | 2152 | 0.5880 |
| Des Koala | 2152 | 0.5880 |
| Spell Canceller | 2152 | 0.5880 |
| Zombyra the Dark | 2082 | 0.5689 |
| Blade Knight | 2021 | 0.5522 |
| Mother Grizzly | 1965 | 0.5369 |
| Phoenix Wing Wind Blast | 1965 | 0.5369 |
| Chain Disappearance | 1915 | 0.5232 |
| Hydrogeddon | 1915 | 0.5232 |
| Dark Scorpion - Cliff the Trap Remover | 1869 | 0.5107 |
| Giant Rat | 1827 | 0.4992 |
| Trap Hole | 1827 | 0.4992 |
| Drillroid | 1789 | 0.4888 |
| Stealth Bird | 1789 | 0.4888 |
| Des Wombat | 1720 | 0.4700 |
| Newdoria | 1689 | 0.4615 |
| Night Assailant | 1607 | 0.4391 |
| Ninja Grandmaster Sasuke | 1607 | 0.4391 |
| United We Stand | 1607 | 0.4391 |
| Bazoo the Soul-Eater | 1560 | 0.4262 |
| Soul Exchange | 1539 | 0.4205 |
| Card Destruction | 1480 | 0.4044 |
| Kuriboh | 1480 | 0.4044 |
| Gravekeeper's Spy | 1429 | 0.3904 |
| Waboku | 1429 | 0.3904 |
| Magical Merchant | 1385 | 0.3784 |
| Raigeki Break | 1385 | 0.3784 |
| Pot of Avarice | 1371 | 0.3746 |
| Dark Ruler Ha Des | 1345 | 0.3675 |
| B. Skull Dragon | 1343 | 0.3669 |
| Cyber Saurus | 1343 | 0.3669 |
| Deepsea Shark | 1343 | 0.3669 |
| Labyrinth Tank | 1343 | 0.3669 |
| Musician King | 1343 | 0.3669 |
| D.D. Survivor | 1320 | 0.3607 |
| Return from the Different Dimension | 1309 | 0.3577 |
| Thestalos the Firestorm Monarch | 1277 | 0.3489 |
| Cipher Soldier | 1229 | 0.3358 |
| King Tiger Wanghu | 1229 | 0.3358 |
| Injection Fairy Lily | 1220 | 0.3333 |
| Magic Cylinder | 1220 | 0.3333 |
| Last Will | 1195 | 0.3265 |
| Giant Trunade | 1165 | 0.3183 |
| Dekoichi the Battlechanted Locomotive | 1158 | 0.3164 |
| Treeborn Frog | 1151 | 0.3145 |
| Cyber-Stein | 1131 | 0.3090 |
| Lightning Vortex | 1131 | 0.3090 |
| Chaos Sorcerer | 1118 | 0.3055 |
| Solemn Judgment | 1095 | 0.2992 |
| Widespread Ruin | 1084 | 0.2962 |
| Trap Dustshoot | 1068 | 0.2918 |
| Brain Control | 1063 | 0.2904 |
| Zaborg the Thunder Monarch | 1063 | 0.2904 |
| D.D. Assailant | 1058 | 0.2891 |
| Don Zaloog | 1058 | 0.2891 |
| Bottomless Trap Hole | 1039 | 0.2839 |
| Bickuribox | 1021 | 0.2790 |
| Charubin the Fire Knight | 1021 | 0.2790 |
| Roaring Ocean Snake | 1021 | 0.2790 |
| Mystic Tomato | 989 | 0.2702 |
| Creature Swap | 980 | 0.2678 |
| Threatening Roar | 978 | 0.2672 |
| Ceasefire | 944 | 0.2579 |
| Mystic Swordsman LV2 | 932 | 0.2547 |
| Kycoo the Ghost Destroyer | 920 | 0.2514 |
| Cyber Jar | 917 | 0.2506 |
| Metamorphosis | 917 | 0.2506 |
| Morphing Jar | 909 | 0.2484 |
| Smashing Ground | 896 | 0.2448 |
| Dark Paladin | 870 | 0.2377 |
| Gaia the Dragon Champion | 870 | 0.2377 |
| Sakuretsu Armor | 840 | 0.2295 |
| Mobius the Frost Monarch | 831 | 0.2271 |
| Dust Tornado | 820 | 0.2240 |
| Swords of Revealing Light | 813 | 0.2221 |
| Reinforcement of the Army | 791 | 0.2161 |
| Cyber Dragon | 789 | 0.2156 |
| Spirit Reaper | 780 | 0.2131 |
| Tsukuyomi | 780 | 0.2131 |
| Flame Swordsman | 777 | 0.2123 |
| Kaminari Attack | 777 | 0.2123 |
| Karbonala Warrior | 777 | 0.2123 |
| Jinzo | 734 | 0.2006 |
| Magician of Faith | 731 | 0.1997 |
| Empress Judge | 711 | 0.1943 |
| Giltia the D. Knight | 662 | 0.1809 |
| Exiled Force | 580 | 0.1585 |
| Confiscation | 568 | 0.1552 |
| D.D. Warrior Lady | 568 | 0.1552 |
| Flame Ghost | 565 | 0.1544 |
| Premature Burial | 553 | 0.1511 |
| Nobleman of Crossout | 546 | 0.1492 |
| Light of Intervention | 542 | 0.1481 |
| Mirror Wall | 542 | 0.1481 |
| Royal Oppression | 542 | 0.1481 |
| Skull Lair | 542 | 0.1481 |
| Warrior of Tradition | 542 | 0.1481 |
| Breaker the Magical Warrior | 535 | 0.1462 |
| Sangan | 535 | 0.1462 |
| Torrential Tribute | 531 | 0.1451 |
| Snatch Steal | 530 | 0.1448 |
| Dark Hole | 526 | 0.1437 |
| Heavy Storm | 526 | 0.1437 |
| Fusionist | 504 | 0.1377 |
| Emergency Provisions | 462 | 0.1262 |
| Twin-Headed Thunder Dragon | 431 | 0.1178 |
| Chain Energy | 413 | 0.1128 |
| Robbin' Goblin | 413 | 0.1128 |
| Rush Recklessly | 413 | 0.1128 |
| Poison of the Old Man | 353 | 0.0965 |
| Dark Flare Knight | 313 | 0.0855 |
| Dragoness the Wicked Knight | 297 | 0.0811 |
| Cyber Blader | 292 | 0.0798 |
| My Body as a Shield | 290 | 0.0792 |
| Darkfire Dragon | 288 | 0.0787 |
| Master of Oz | 276 | 0.0754 |
| Prohibition | 270 | 0.0738 |
| Mystik Wok | 232 | 0.0634 |
| Ojama King | 224 | 0.0612 |
| Blue-Eyes Ultimate Dragon | 222 | 0.0607 |
| Dark Blade the Dragon Knight | 222 | 0.0607 |
| King Dragun | 214 | 0.0585 |
| Reaper on the Nightmare | 213 | 0.0582 |
| Fiend Skull Dragon | 209 | 0.0571 |
| The Last Warrior from Another Planet | 206 | 0.0563 |
| Messenger of Peace | 202 | 0.0552 |
| Gatling Dragon | 199 | 0.0544 |
| Ryu Senshi | 199 | 0.0544 |
| Dark Balter the Terrible | 198 | 0.0541 |
| Cyber End Dragon | 191 | 0.0522 |
| Cyber Twin Dragon | 191 | 0.0522 |
| Gravity Bind | 164 | 0.0448 |
| Limiter Removal | 160 | 0.0437 |
| Mask of Restrict | 148 | 0.0404 |
| Wave-Motion Cannon | 146 | 0.0399 |
| Level Limit - Area B | 145 | 0.0396 |
| Thousand-Eyes Restrict | 127 | 0.0347 |
| Royal Decree | 110 | 0.0301 |
| Enemy Controller | 100 | 0.0273 |
| Scapegoat | 71 | 0.0194 |
| Call Of The Haunted | 62 | 0.0169 |
| Book of Moon | 60 | 0.0164 |
| Mystical Space Typhoon | 52 | 0.0142 |

_1219 card names in JSON/usage had no ROM match (omitted from pools)._

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
| [Vampire](#vampire) | 111 |
| [Chaos](#chaos) | 138 |
| [Warrior](#warrior) | 182 |
| [Goat](#goat) | 256 |
| [Cyber](#cyber) | 139 |
| [Reaper](#reaper) | 210 |

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
| Vampire Lord | 318 | Limited |
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
| Vampire Lord | 231 | Limited |
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
| Vampire Lord | 272 | Limited |
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
| Big Bang Shot | 211 |  |
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
| Big Bang Shot | 216 |  |
| Chain Energy | 216 |  |
| My Body as a Shield | 216 |  |
| Twin-Headed Thunder Dragon | 216 |  |
| Bazoo the Soul-Eater | 215 |  |
| Toon Table of Contents | 215 |  |
| Poison of the Old Man | 214 |  |
| Milus Radiant | 213 |  |
| Royal Oppression | 213 |  |
| Time Seal | 213 |  |
| Cannon Soldier | 212 |  |
| Soul Exchange | 212 |  |
| Vampire Lord | 212 | Semi-Limited |
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
| Des Lacooda | 203 |  |
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
| Vampire Lord | 231 |  |
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
| My Body as a Shield | 222 |  |
| Sand Moth | 222 |  |
| Sasuke Samurai | 222 |  |
| Secret Barrel | 222 |  |
| Seven Tools of the Bandit | 222 |  |
| Dragoness the Wicked Knight | 221 |  |
| Dark Flare Knight | 218 |  |
| Des Lacooda | 218 |  |
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
