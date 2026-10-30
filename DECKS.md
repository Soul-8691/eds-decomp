# Decks

The starting decks and the opponents' decks of Yak's YGO Hack, as defined in `src/mod_decks.c`. Deck names follow the `.ydk` files they come from (`decks/`, "author-Deck").

- **Deck chances.** A format opponent picks one of its decks at weighted random at the start of every duel.
- **Banlists.** Opponent decks keep to their format's Forbidden/Limited list: when a deck is loaded, any card over its limit is dropped.
- **Deck Recipes.** Every deck below (except the vanilla ones) is also a preset in the Deck Recipes menu.

## Starting decks

A new game offers the three Yugi-Kaiba format decks (lists under [Yugi-Kaiba](#1-yugi-kaiba)):

| Starter | Deck | Format |
|---|---|---|
| Red | Soul Control (El Duelista DEX-Soul Control) | Yugi-Kaiba |
| Green | Vanilla Beat (ellietincan-Vanilla Beat) | Yugi-Kaiba |
| Black | Stall Mill (DK-ZERO-Stall Mill) | Yugi-Kaiba |

## Format opponents

Formats unlock in this order; in the release build each takes 10 wins to open the next.

| # | Format | Banlist | Decks (chance) |
|---|---|---|---|
| 1 | Yugi-Kaiba | May 2002 | Soul Control 50%, Vanilla Beat 30%, Stall Mill 20% |
| 2 | Critter | July 2002 | Masked Control 60%, Muka Muka Turbo 40% |
| 3 | Treasure | October 2002 | Tomato Control 80%, Recruiter Control 10%, Germ Burn 10% |
| 4 | Imperial | December 2002 | Tomato Control 80%, Relinquished 20% |
| 5 | Android | April 2003 | Tomato Control 80%, Vanilla Beat 20% |
| 6 | Joey-Pegasus | May 2003 | Tomato Control 80%, Vanilla Beat 20% |
| 7 | Fiber | July 2003 | Tomato Control 80%, Recruiter Control 20% |
| 8 | Yata | August 2003 | Tomato Control 50%, Warrior Tomato Control 40%, Gearfried 10% |
| 9 | Scientist | November 2003 | Tomato Control 60%, Scientist FTK 40% |
| 10 | Vampire | February 2004 | Warrior Control 80%, Scientist FTK 10%, Warrior Tomato Control 10% |
| 11 | Chaos | April 2004 | Chaos Control 80%, Chaos Warrior 20% |
| 12 | Warrior | October 2004 | Goat Control 40%, Warrior 30%, Warrior Control 20%, Chaos Return 10% |
| 13 | Goat | April 2005 | Vanilla (unchanged) |
| 14 | Cyber | April 2005 | Vanilla (unchanged) |
| 15 | Reaper | October 2005 | Vanilla (unchanged) |

### 1. Yugi-Kaiba

Banlist: May 2002.

<details>
<summary>Soul Control: 50% (El Duelista DEX-Soul Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (20)**

- 3x Man-Eater Bug
- 1x Trap Master
- 1x Wall of Illusion
- 3x Battle Ox
- 3x Giant Soldier of Stone
- 3x La Jinn the Mystical Genie of the Lamp
- 3x Neo the Magic Swordsman
- 3x Summoned Skull

**Spells (12)**

- 1x Change of Heart
- 1x Dark Hole
- 3x Fissure
- 1x Monster Reborn
- 1x Pot of Greed
- 1x Raigeki
- 2x Soul Exchange
- 2x Swords of Revealing Light

**Traps (8)**

- 3x Reinforcements
- 3x Trap Hole
- 1x Ultimate Offering
- 1x Waboku

**Fusion Deck (15)**

- 2x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 2x Flame Ghost
- 2x Flame Swordsman
- 2x Flower Wolf
- 2x Fusionist
- 1x Karbonala Warrior
- 3x Metal Dragon

**Side Deck (15)**

- 2x Hane-Hane
- 1x Trap Master
- 1x Wall of Illusion
- 2x Judge Man
- 2x Card Destruction
- 2x Ookazi
- 1x De-Spell
- 1x Remove Trap
- 1x Ultimate Offering
- 2x Waboku

</details>

<details>
<summary>Vanilla Beat: 30% (ellietincan-Vanilla Beat) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (20)**

- 3x Man-Eater Bug
- 3x Trap Master
- 3x Battle Ox
- 3x Giant Soldier of Stone
- 3x La Jinn the Mystical Genie of the Lamp
- 3x Neo the Magic Swordsman
- 2x Summoned Skull

**Spells (11)**

- 1x Change of Heart
- 1x Dark Hole
- 1x De-Spell
- 3x Fissure
- 1x Monster Reborn
- 1x Pot of Greed
- 1x Raigeki
- 2x Swords of Revealing Light

**Traps (9)**

- 3x Reinforcements
- 3x Trap Hole
- 3x Waboku

**Fusion Deck (15)**

- 3x Dragoness the Wicked Knight
- 3x Flame Swordsman
- 3x Gaia the Dragon Champion
- 3x Karbonala Warrior
- 3x Metal Dragon

**Side Deck (15)**

- 3x Wall of Illusion
- 3x Aqua Madoor
- 1x Judge Man
- 1x Summoned Skull
- 2x Card Destruction
- 2x De-Spell
- 3x Soul Exchange

</details>

<details>
<summary>Stall Mill: 20% (DK-ZERO-Stall Mill) (43 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (21)**

- 3x Gyakutenno Megami
- 3x Giant Soldier of Stone
- 3x Aqua Madoor
- 3x Wall of Illusion
- 3x Mystical Elf
- 3x Spirit of the Harp
- 3x Man-Eater Bug

**Spells (13)**

- 1x Raigeki
- 1x Dark Hole
- 1x Monster Reborn
- 1x Change of Heart
- 1x Pot of Greed
- 2x Swords of Revealing Light
- 3x Fissure
- 1x Card Destruction
- 2x Soul Exchange

**Traps (9)**

- 3x Ultimate Offering
- 3x Waboku
- 3x Trap Hole

**Fusion Deck (15)**

- 3x Gaia the Dragon Champion
- 3x Metal Dragon
- 3x Flame Swordsman
- 3x Flower Wolf
- 3x Flame Ghost

**Side Deck (15)**

- 3x La Jinn the Mystical Genie of the Lamp
- 3x Battle Ox
- 3x Neo the Magic Swordsman
- 3x Reinforcements
- 2x The Wicked Worm Beast
- 1x Card Destruction

</details>

### 2. Critter

Banlist: July 2002.

<details>
<summary>Masked Control: 60% (Jazz-Masked Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (18)**

- 1x Cannon Soldier
- 1x Jirai Gumo
- 1x Kuriboh
- 2x Magician of Faith
- 2x Masked Sorcerer
- 2x Muka Muka
- 2x Sangan
- 1x White Magical Hat
- 2x Witch of the Black Forest
- 1x Giant Soldier of Stone
- 3x La Jinn the Mystical Genie of the Lamp

**Spells (13)**

- 1x Change of Heart
- 1x Dark Hole
- 2x Fissure
- 2x Heavy Storm
- 2x Last Will
- 1x Monster Reborn
- 1x Pot of Greed
- 1x Raigeki
- 2x Swords of Revealing Light

**Traps (9)**

- 1x Mirror Force
- 3x Solemn Judgment
- 2x Trap Hole
- 3x Waboku

**Fusion Deck (15)**

- 3x B. Skull Dragon
- 3x Gaia the Dragon Champion
- 3x Labyrinth Tank
- 3x Thousand Dragon
- 3x Twin-Headed Thunder Dragon

**Side Deck (15)**

- 1x Kuriboh
- 1x Muka Muka
- 3x Thunder Dragon
- 2x Giant Soldier of Stone
- 2x Card Destruction
- 1x Fissure
- 2x Fake Trap
- 2x Robbin' Goblin
- 1x Trap Hole

</details>

<details>
<summary>Muka Muka Turbo: 40% (Gyomberman-Muka Muka Turbo) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (20)**

- 1x Cannon Soldier
- 3x Magician of Faith
- 1x Masked Sorcerer
- 3x Muka Muka
- 2x Sangan
- 3x Thunder Dragon
- 2x Witch of the Black Forest
- 2x Giant Soldier of Stone
- 3x La Jinn the Mystical Genie of the Lamp

**Spells (15)**

- 1x Change of Heart
- 1x Dark Hole
- 3x Fissure
- 2x Heavy Storm
- 3x Last Will
- 1x Monster Reborn
- 1x Pot of Greed
- 1x Raigeki
- 2x Swords of Revealing Light

**Traps (5)**

- 1x Mirror Force
- 1x Robbin' Goblin
- 3x Trap Hole

**Fusion Deck (15)**

- 1x Charubin the Fire Knight
- 1x Cyber Saurus
- 1x Darkfire Dragon
- 1x Deepsea Shark
- 1x Dragoness the Wicked Knight
- 1x Empress Judge
- 1x Flame Ghost
- 1x Fusionist
- 1x Gaia the Dragon Champion
- 1x Giltia the D. Knight
- 1x Karbonala Warrior
- 1x Metal Dragon
- 1x Musician King
- 1x Punished Eagle
- 1x Roaring Ocean Snake

**Side Deck (15)**

- 2x Man-Eater Bug
- 3x Wall of Illusion
- 2x Card Destruction
- 2x Seven Tools of the Bandit
- 3x Solemn Judgment
- 2x Waboku
- 1x Robbin' Goblin

</details>

### 3. Treasure

Banlist: October 2002.

<details>
<summary>Tomato Control: 80% (dump truck-Tomato Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (18)**

- 1x Cannon Soldier
- 3x Magician of Faith
- 3x Man-Eater Bug
- 3x Mystic Tomato
- 2x Sangan
- 2x Witch of the Black Forest
- 1x Giant Soldier of Stone
- 3x Mechanicalchaser

**Spells (17)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 3x Fissure
- 1x Heavy Storm
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (5)**

- 1x Mirror Force
- 1x Seven Tools of the Bandit
- 3x Trap Hole

**Fusion Deck (15)**

- 3x Charubin the Fire Knight
- 3x Darkfire Dragon
- 3x Dragoness the Wicked Knight
- 3x Flame Ghost
- 3x Flame Swordsman

**Side Deck (15)**

- 2x Banisher of the Light
- 1x Jirai Gumo
- 2x Masked Sorcerer
- 1x Giant Trunade
- 1x Heavy Storm
- 2x Tribute to The Doomed
- 2x Seven Tools of the Bandit
- 1x Solemn Judgment
- 3x Waboku

</details>

<details>
<summary>Recruiter Control: 10% (SolEmerald69-Recruiter Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (17)**

- 3x Mystic Tomato
- 2x Sangan
- 2x Witch of the Black Forest
- 1x Cannon Soldier
- 3x Mother Grizzly
- 1x Catapult Turtle
- 2x Mechanicalchaser
- 2x Magician of Faith
- 1x Cyber Jar

**Spells (18)**

- 1x Painful Choice
- 1x Pot of Greed
- 2x Last Will
- 1x Monster Reborn
- 1x Raigeki
- 1x Dark Hole
- 1x Change of Heart
- 1x Snatch Steal
- 1x Delinquent Duo
- 1x Confiscation
- 1x The Forceful Sentry
- 3x Mystical Space Typhoon
- 1x Heavy Storm
- 2x Rush Recklessly

**Traps (5)**

- 1x Mirror Force
- 2x Trap Hole
- 2x Waboku

**Fusion Deck (15)**

- 3x B. Skull Dragon
- 3x Twin-Headed Thunder Dragon
- 3x Labyrinth Tank
- 2x Thousand Dragon
- 2x Bickuribox
- 2x Musician King

**Side Deck (15)**

- 1x Man-Eater Bug
- 1x Wall of Illusion
- 2x Jirai Gumo
- 2x Tribute to The Doomed
- 2x Fissure
- 1x Heavy Storm
- 2x Solemn Judgment
- 2x Magic Jammer
- 2x Seven Tools of the Bandit

</details>

<details>
<summary>Germ Burn: 10% (bsocks-Germ Burn) (40 cards + 15 Side Deck)</summary>

**Monsters (18)**

- 1x Cannon Soldier
- 1x Cyber Jar
- 3x Giant Germ
- 3x Mystic Tomato
- 3x Nimble Momonga
- 2x Sangan
- 3x Spear Cretin
- 2x Witch of the Black Forest

**Spells (15)**

- 3x Chain Energy
- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 3x Giant Trunade
- 1x Monster Reborn
- 1x Pot of Greed
- 1x Raigeki
- 1x The Forceful Sentry
- 1x Toll

**Traps (7)**

- 3x Just Desserts
- 3x Magic Jammer
- 1x Mirror Force

**Side Deck (15)**

- 3x Minar
- 3x Princess of Tsurugi
- 3x Seven Tools of the Bandit
- 3x Solemn Judgment
- 3x Waboku

</details>

### 4. Imperial

Banlist: December 2002.

<details>
<summary>Tomato Control: 80% (Aretos-Tomato Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (17)**

- 3x Mechanicalchaser
- 1x Giant Soldier of Stone
- 1x Jinzo
- 2x Magician of Faith
- 2x Man-Eater Bug
- 1x Morphing Jar
- 3x Mystic Tomato
- 2x Sangan
- 1x Witch of the Black Forest
- 1x Cannon Soldier

**Spells (18)**

- 3x Mystical Space Typhoon
- 2x Nobleman of Crossout
- 1x Fissure
- 1x Raigeki
- 1x Dark Hole
- 1x Change of Heart
- 1x Snatch Steal
- 1x Delinquent Duo
- 1x Confiscation
- 1x The Forceful Sentry
- 1x Heavy Storm
- 1x Painful Choice
- 1x Pot of Greed
- 1x Monster Reborn
- 1x Premature Burial

**Traps (5)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 1x Trap Hole
- 1x Dust Tornado

**Fusion Deck (15)**

- 1x B. Skull Dragon
- 1x Twin-Headed Thunder Dragon
- 1x Labyrinth Tank
- 1x Thousand Dragon
- 1x Roaring Ocean Snake
- 1x Punished Eagle
- 1x Kwagar Hercules
- 1x Warrior of Tradition
- 1x Deepsea Shark
- 1x Musician King
- 1x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x White Magical Hat
- 1x Masked Sorcerer
- 1x Man-Eater Bug
- 1x Giant Soldier of Stone
- 3x Exchange
- 1x Rush Recklessly
- 1x Heavy Storm
- 2x Solemn Wishes
- 1x Trap Hole
- 1x Acid Trap Hole
- 1x Dust Tornado
- 1x Ceasefire

</details>

<details>
<summary>Relinquished: 20% (bsocks-Relinquished) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (19)**

- 1x Jinzo
- 3x Magician of Faith
- 3x Mystic Tomato
- 2x Sangan
- 2x Senju of the Thousand Hands
- 2x Sonic Bird
- 1x Witch of the Black Forest
- 3x Mechanicalchaser
- 2x Relinquished

**Spells (15)**

- 2x Black Illusion Ritual
- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Pot of Greed
- 1x Premature Burial
- 1x Raigeki
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (6)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 3x Solemn Judgment

**Fusion Deck (15)**

- 3x B. Skull Dragon
- 3x Labyrinth Tank
- 3x Metal Dragon
- 3x Thousand-Eyes Restrict
- 3x Twin-Headed Thunder Dragon

**Side Deck (15)**

- 1x Cannon Soldier
- 3x Man-Eater Bug
- 2x Nobleman of Crossout
- 1x Ceasefire
- 3x Dust Tornado
- 3x Solemn Wishes
- 2x Waboku

</details>

### 5. Android

Banlist: April 2003.

<details>
<summary>Tomato Control: 80% (Jazz-Tomato Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (14)**

- 2x Bazoo the Soul-Eater
- 1x Cannon Soldier
- 1x Jinzo
- 2x Kycoo the Ghost Destroyer
- 1x Masked Sorcerer
- 3x Mystic Tomato
- 1x Sangan
- 1x Witch of the Black Forest
- 2x Gemini Elf

**Spells (14)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 1x Snatch Steal
- 1x Swords of Revealing Light
- 1x The Forceful Sentry

**Traps (12)**

- 1x Call Of The Haunted
- 1x Fairy Box
- 1x Imperial Order
- 3x Jar of Greed
- 1x Magic Cylinder
- 1x Mirror Force
- 1x Skull Lair
- 1x Time Seal
- 2x Torrential Tribute

**Fusion Deck (15)**

- 3x Darkfire Dragon
- 3x Dragoness the Wicked Knight
- 3x The Last Warrior from Another Planet
- 3x Thousand-Eyes Restrict
- 3x Warrior of Tradition

**Side Deck (15)**

- 1x Morphing Jar
- 2x Penguin Knight
- 2x Fissure
- 2x Heavy Storm
- 2x Nobleman of Crossout
- 1x Fake Trap
- 3x Solemn Judgment
- 2x Trap Hole

</details>

<details>
<summary>Vanilla Beat: 20% (paoloNS-Vanilla Beat) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (15)**

- 3x Bazoo the Soul-Eater
- 1x Cyber Jar
- 1x Jinzo
- 3x Kycoo the Ghost Destroyer
- 1x Magician of Faith
- 1x Morphing Jar
- 1x Sangan
- 1x Witch of the Black Forest
- 3x Gemini Elf

**Spells (15)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 2x Heavy Storm
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (10)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Jar of Greed
- 1x Mirror Force
- 1x Skull Lair
- 3x Solemn Judgment
- 2x Torrential Tribute

**Fusion Deck (15)**

- 3x Bickuribox
- 3x B. Skull Dragon
- 3x Charubin the Fire Knight
- 3x Cyber Saurus
- 3x Twin-Headed Thunder Dragon

**Side Deck (15)**

- 1x 4-Starred Ladybug of Doom
- 2x Kuriboh
- 2x Nobleman of Crossout
- 1x Premature Burial
- 1x Swords of Revealing Light
- 1x Ceasefire
- 3x Dust Tornado
- 1x Fairy Box
- 1x Skull Lair
- 1x Time Seal
- 1x Torrential Tribute

</details>

### 6. Joey-Pegasus

Banlist: May 2003.

<details>
<summary>Tomato Control: 80% (AJTBLS-Tomato Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (16)**

- 2x Bazoo the Soul-Eater
- 1x Jinzo
- 2x Kycoo the Ghost Destroyer
- 2x Magician of Faith
- 3x Mystic Tomato
- 1x Sangan
- 1x Sinister Serpent
- 3x Thunder Dragon
- 1x Witch of the Black Forest

**Spells (17)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 2x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Heavy Storm
- 1x Monster Reborn
- 2x Mystical Space Typhoon
- 1x Nobleman of Crossout
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (7)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 1x Time Seal
- 3x Torrential Tribute

**Fusion Deck (15)**

- 3x Dark Balter the Terrible
- 3x Fiend Skull Dragon
- 3x Giltia the D. Knight
- 3x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Guardian Sphinx
- 1x Kycoo the Ghost Destroyer
- 2x Masked Sorcerer
- 1x Gemini Elf
- 1x Last Will
- 1x Mage Power
- 1x Premature Burial
- 1x Reinforcement of the Army
- 1x Ceasefire
- 1x Magic Cylinder
- 3x Solemn Judgment
- 1x Time Seal

</details>

<details>
<summary>Vanilla Beat: 20% (ellietincan-Vanilla Beat (JP)) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (13)**

- 2x Bazoo the Soul-Eater
- 1x Jinzo
- 3x Kycoo the Ghost Destroyer
- 1x Magician of Faith
- 1x Sangan
- 1x Sinister Serpent
- 1x Witch of the Black Forest
- 3x Gemini Elf

**Spells (18)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Fissure
- 2x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Heavy Storm
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (9)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 3x Jar of Greed
- 1x Mirror Force
- 1x Skull Lair
- 2x Torrential Tribute

**Fusion Deck (15)**

- 3x B. Skull Dragon
- 3x Flame Swordsman
- 3x Gaia the Dragon Champion
- 3x St. Joan
- 3x The Last Warrior from Another Planet

**Side Deck (15)**

- 1x Bazoo the Soul-Eater
- 1x Cannon Soldier
- 2x Magician of Faith
- 3x Mystic Tomato
- 1x Fissure
- 1x Nobleman of Crossout
- 1x Premature Burial
- 1x Dust Tornado
- 1x Torrential Tribute
- 3x Waboku

</details>

### 7. Fiber

Banlist: July 2003.

<details>
<summary>Tomato Control: 80% (Aretos-Tomato Control (Fiber)) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (15)**

- 3x Gemini Elf
- 1x Bazoo the Soul-Eater
- 1x Kycoo the Ghost Destroyer
- 1x Witch of the Black Forest
- 1x Sangan
- 3x Mystic Tomato
- 1x Sinister Serpent
- 1x Injection Fairy Lily
- 1x Exiled Force
- 1x Fiber Jar
- 1x Magician of Faith

**Spells (16)**

- 1x Snatch Steal
- 1x Change of Heart
- 1x Delinquent Duo
- 1x Confiscation
- 1x The Forceful Sentry
- 3x Mystical Space Typhoon
- 1x Raigeki
- 1x Dark Hole
- 1x Nobleman of Crossout
- 1x Painful Choice
- 1x Pot of Greed
- 2x Graceful Charity
- 1x Monster Reborn

**Traps (9)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 1x Torrential Tribute
- 1x Trap Hole
- 1x Time Seal
- 3x Jar of Greed

**Fusion Deck (15)**

- 1x B. Skull Dragon
- 1x Twin-Headed Thunder Dragon
- 1x Labyrinth Tank
- 1x The Last Warrior from Another Planet
- 1x Roaring Ocean Snake
- 1x Ryu Senshi
- 1x Super Robolady
- 1x Super Roboyarou
- 1x Dark Balter the Terrible
- 1x Fiend Skull Dragon
- 1x Musician King
- 1x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 2x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Jinzo
- 1x Kycoo the Ghost Destroyer
- 1x Asura Priest
- 1x Yata-Garasu
- 2x Creature Swap
- 1x Scapegoat
- 1x Harpie's Feather Duster
- 1x Nobleman of Crossout
- 1x Fissure
- 1x Torrential Tribute
- 1x Trap Hole
- 1x Skull Lair
- 2x Dust Tornado

</details>

<details>
<summary>Recruiter Control: 20% (GBO-Recruiter Control) (40 cards + 14 Side Deck)</summary>

**Monsters (13)**

- 1x Dark Ruler Ha Des
- 1x Exiled Force
- 3x Giant Rat
- 1x Injection Fairy Lily
- 1x Jinzo
- 3x Mystic Tomato
- 1x Sangan
- 1x Witch of the Black Forest
- 1x Yata-Garasu

**Spells (22)**

- 1x Change of Heart
- 1x Confiscation
- 2x Creature Swap
- 1x Dark Hole
- 1x Delinquent Duo
- 2x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Heavy Storm
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 2x Nobleman of Crossout
- 1x Painful Choice
- 1x Pot of Greed
- 1x Premature Burial
- 1x Raigeki
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (5)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Magic Cylinder
- 1x Mirror Force
- 1x Torrential Tribute

**Side Deck (14)**

- 2x Gearfried the Iron Knight
- 1x Goblin Attack Force
- 2x Fissure
- 2x Reinforcement of the Army
- 2x Blast with Chain
- 3x Dust Tornado
- 2x Torrential Tribute

</details>

### 8. Yata

Banlist: August 2003.

<details>
<summary>Tomato Control: 50% (Shigeki Kitamura-Tomato Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (16)**

- 1x Bazoo the Soul-Eater
- 1x Exiled Force
- 1x Fiber Jar
- 1x Jinzo
- 1x Magician of Faith
- 3x Mystic Tomato
- 1x Sangan
- 1x Sinister Serpent
- 1x Spirit Reaper
- 1x Witch of the Black Forest
- 1x Yata-Garasu
- 1x Zombyra the Dark
- 2x Gemini Elf

**Spells (18)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Heavy Storm
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Pot of Greed
- 1x Premature Burial
- 1x Raigeki
- 1x Scapegoat
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (6)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 1x Bell of Destruction
- 2x Torrential Tribute

**Fusion Deck (15)**

- 3x Dark Balter the Terrible
- 3x Fiend Skull Dragon
- 3x Giltia the D. Knight
- 3x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Airknight Parshath
- 1x Cannon Soldier
- 1x Don Zaloog
- 1x Goblin Attack Force
- 1x Guardian Sphinx
- 1x Kycoo the Ghost Destroyer
- 1x Spear Dragon
- 1x Gemini Elf
- 1x Creature Swap
- 2x Nobleman of Crossout
- 1x Painful Choice
- 1x Swords of Revealing Light
- 1x Ceasefire
- 1x Magic Cylinder

</details>

<details>
<summary>Warrior Tomato Control: 40% (ellietincan-Warrior Tomato Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (16)**

- 2x Don Zaloog
- 1x Exiled Force
- 1x Fiber Jar
- 1x Injection Fairy Lily
- 1x Jinzo
- 2x Magician of Faith
- 3x Mystic Tomato
- 1x Sangan
- 1x Sinister Serpent
- 1x Spirit Reaper
- 1x Witch of the Black Forest
- 1x Yata-Garasu

**Spells (17)**

- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Nobleman of Crossout
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 2x Reinforcement of the Army
- 1x The Forceful Sentry

**Traps (7)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 1x Bell of Destruction
- 3x Torrential Tribute

**Fusion Deck (15)**

- 3x B. Skull Dragon
- 3x Flame Swordsman
- 3x Gaia the Dragon Champion
- 3x Ryu Senshi
- 3x St. Joan

**Side Deck (14)**

- 1x Bazoo the Soul-Eater
- 1x Cyber Jar
- 2x Marauding Captain
- 1x Morphing Jar
- 2x Morphing Jar #2
- 2x Gemini Elf
- 1x Change of Heart
- 1x Heavy Storm
- 1x Premature Burial
- 1x Snatch Steal
- 1x Trap Dustshoot

</details>

<details>
<summary>Gearfried: 10% (AlephYa2-Gearfried) (40 cards + 2 Fusion Deck + 15 Side Deck)</summary>

**Monsters (12)**

- 2x Don Zaloog
- 1x Exiled Force
- 3x Gearfried the Iron Knight
- 1x Injection Fairy Lily
- 1x Witch of the Black Forest
- 1x Zombyra the Dark
- 3x Gemini Elf

**Spells (18)**

- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Fissure
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Nobleman of Crossout
- 1x Pot of Greed
- 1x Raigeki
- 2x Reinforcement of the Army
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (10)**

- 3x Blast with Chain
- 1x Call Of The Haunted
- 1x Imperial Order
- 2x Magic Drain
- 1x Mirror Force
- 1x Mirror Wall
- 1x Bell of Destruction

**Fusion Deck (2)**

- 2x Thousand-Eyes Restrict

**Side Deck (15)**

- 2x Electric Snake
- 3x Gravekeeper's Spy
- 1x Jinzo
- 3x King Tiger Wanghu
- 1x Book of Moon
- 1x Change of Heart
- 1x Fissure
- 1x Heavy Storm
- 1x Nobleman of Crossout
- 1x Dust Tornado

</details>

### 9. Scientist

Banlist: November 2003.

<details>
<summary>Tomato Control: 60% (Aretos-Tomato Control (Scientist)) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (17)**

- 1x Breaker the Magical Warrior
- 1x Exiled Force
- 1x Fiber Jar
- 2x Gravekeeper's Spy
- 1x Jinzo
- 1x King Tiger Wanghu
- 2x Magical Scientist
- 1x Magician of Faith
- 3x Mystic Tomato
- 1x Sangan
- 1x Sinister Serpent
- 1x Tribe-Infecting Virus
- 1x Witch of the Black Forest

**Spells (16)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (7)**

- 1x Bell of Destruction
- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Jar of Greed
- 1x Mirror Force
- 2x Torrential Tribute

**Fusion Deck (15)**

- 2x Dark Balter the Terrible
- 1x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 1x Empress Judge
- 1x Fiend Skull Dragon
- 3x Punished Eagle
- 3x Roaring Ocean Snake
- 1x Ryu Senshi
- 2x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Airknight Parshath
- 1x Book of Moon
- 1x Catapult Turtle
- 1x Ceasefire
- 2x Dust Tornado
- 1x Heavy Storm
- 2x King Tiger Wanghu
- 1x Magical Scientist
- 1x Newdoria
- 2x Nobleman of Crossout
- 1x Spirit Reaper
- 1x Torrential Tribute

</details>

<details>
<summary>Scientist FTK: 40% (DK-ZERO-Scientist FTK) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (17)**

- 3x Cannon Soldier
- 3x Catapult Turtle
- 3x Gilasaurus
- 3x Magical Scientist
- 3x Mystic Tomato
- 1x Sangan
- 1x Witch of the Black Forest

**Spells (20)**

- 1x Card Destruction
- 1x Dark Hole
- 1x Giant Trunade
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Heavy Storm
- 3x Last Will
- 3x Metamorphosis
- 1x Monster Reborn
- 1x Painful Choice
- 1x Pot of Greed
- 1x Premature Burial
- 1x Raigeki
- 3x Reasoning

**Traps (3)**

- 1x Bell of Destruction
- 1x Call Of The Haunted
- 1x Imperial Order

**Fusion Deck (15)**

- 2x Dark Balter the Terrible
- 1x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 1x Empress Judge
- 1x Fiend Skull Dragon
- 1x Punished Eagle
- 1x Roaring Ocean Snake
- 2x Ryu Senshi
- 1x Super Robolady
- 1x Super Roboyarou
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Change of Heart
- 1x Cyber Jar
- 1x Dark Ruler Ha Des
- 1x Fiber Jar
- 1x Jinzo
- 1x Mirage of Nightmare
- 1x Mirror Force
- 3x Mystical Space Typhoon
- 1x Sinister Serpent
- 1x Snatch Steal
- 3x Torrential Tribute

</details>

Cards here with no coded effect yet: Super Robolady, Super Roboyarou, Dark Balter the Terrible, Fiend Skull Dragon
and Ryu Senshi.

### 10. Vampire

Banlist: February 2004.

<details>
<summary>Warrior Control: 80% (Woawa-Warrior Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (15)**

- 1x Breaker the Magical Warrior
- 3x D.D. Warrior Lady
- 1x Exiled Force
- 1x Fiber Jar
- 3x Gravekeeper's Spy
- 1x Injection Fairy Lily
- 1x Sangan
- 1x Sinister Serpent
- 1x Tribe-Infecting Virus
- 1x Witch of the Black Forest
- 1x Zombyra the Dark

**Spells (19)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Heavy Storm
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Nobleman of Crossout
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 2x Reinforcement of the Army
- 1x The Forceful Sentry

**Traps (6)**

- 1x Bell of Destruction
- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 2x Torrential Tribute

**Fusion Deck (15)**

- 3x Dark Balter the Terrible
- 3x Dark Flare Knight
- 3x Fiend Skull Dragon
- 3x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 2x Book of Moon
- 1x Dust Tornado
- 1x Jinzo
- 1x Magical Scientist
- 1x Nobleman of Crossout
- 3x Poison of the Old Man
- 2x Royal Decree
- 1x Sasuke Samurai
- 1x Snatch Steal
- 1x The Little Swordsman of Aile
- 1x Torrential Tribute

</details>

<details>
<summary>Scientist FTK: 10% (ellietincan-Scientist FTK) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (17)**

- 1x Breaker the Magical Warrior
- 1x Cannon Soldier
- 3x Catapult Turtle
- 1x Cyber Jar
- 1x Fiber Jar
- 3x Gravekeeper's Spy
- 1x Magical Scientist
- 3x Mystic Tomato
- 1x Sangan
- 1x Tribe-Infecting Virus
- 1x Witch of the Black Forest

**Spells (18)**

- 1x Change of Heart
- 1x Dark Hole
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Heavy Storm
- 3x Last Will
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Premature Burial
- 1x Raigeki
- 1x The Forceful Sentry

**Traps (5)**

- 1x Bell of Destruction
- 1x Call Of The Haunted
- 1x Ceasefire
- 1x Imperial Order
- 1x Mirror Force

**Fusion Deck (15)**

- 2x Dark Balter the Terrible
- 3x Dark Flare Knight
- 3x Empress Judge
- 1x Punished Eagle
- 3x Roaring Ocean Snake
- 1x Ryu Senshi
- 2x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x D.D. Warrior Lady
- 1x Exiled Force
- 1x Giant Trunade
- 1x Jinzo
- 1x Magic Cylinder
- 3x Mother Grizzly
- 3x Royal Decree
- 1x Sinister Serpent
- 1x Swords of Revealing Light
- 2x Torrential Tribute

</details>

<details>
<summary>Warrior Tomato Control: 10% (AJTBLS-Warrior Tomato Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (18)**

- 1x Breaker the Magical Warrior
- 1x Exiled Force
- 1x Fiber Jar
- 2x Gemini Elf
- 1x Injection Fairy Lily
- 1x Jinzo
- 1x Magical Scientist
- 2x Magician of Faith
- 3x Mystic Tomato
- 1x Sangan
- 1x Sinister Serpent
- 1x Tribe-Infecting Virus
- 1x Witch of the Black Forest
- 1x Yata-Garasu

**Spells (18)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Heavy Storm
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 2x Mystical Space Typhoon
- 1x Nobleman of Crossout
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 1x Reinforcement of the Army
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (4)**

- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 1x Torrential Tribute

**Fusion Deck (15)**

- 3x Dark Balter the Terrible
- 3x Fiend Skull Dragon
- 3x Giltia the D. Knight
- 3x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Cold Wave
- 1x Electric Snake
- 1x Goblin Attack Force
- 1x Guardian Sphinx
- 1x Kycoo the Ghost Destroyer
- 1x Mage Power
- 1x Magic Cylinder
- 2x Masked Sorcerer
- 1x Nobleman of Crossout
- 1x Premature Burial
- 1x Scapegoat
- 2x Time Seal
- 1x White Magical Hat

</details>

Sasuke Samurai (Warrior Control's Side Deck) was added to the game for these decks.

### 11. Chaos

Banlist: April 2004.

<details>
<summary>Chaos Control: 80% (ellietincan-Chaos Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (17)**

- 1x Black Luster Soldier - Envoy of Beginning
- 1x Breaker the Magical Warrior
- 1x Chaos Emperor Dragon - Envoy of the End
- 3x D.D. Warrior Lady
- 1x Exiled Force
- 1x Injection Fairy Lily
- 2x Kycoo the Ghost Destroyer
- 1x Magical Scientist
- 1x Sangan
- 2x Shining Fairy
- 1x Sinister Serpent
- 1x Tribe-Infecting Virus
- 1x Witch of the Black Forest

**Spells (17)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Premature Burial
- 1x Raigeki
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (6)**

- 1x Bell of Destruction
- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 2x Torrential Tribute

**Fusion Deck (15)**

- 2x Dark Balter the Terrible
- 3x Dark Flare Knight
- 2x Fiend Skull Dragon
- 3x Roaring Ocean Snake
- 2x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Chaos Sorcerer
- 1x Fiber Jar
- 1x Kycoo the Ghost Destroyer
- 2x Magician of Faith
- 2x Nobleman of Crossout
- 2x Reinforcement of the Army
- 3x Sakuretsu Armor
- 1x Skilled Dark Magician
- 1x Torrential Tribute
- 1x Yata-Garasu

</details>

<details>
<summary>Chaos Warrior: 20% (indoors-Chaos Warrior) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (17)**

- 1x Black Luster Soldier - Envoy of Beginning
- 1x Breaker the Magical Warrior
- 1x Chaos Emperor Dragon - Envoy of the End
- 3x D.D. Warrior Lady
- 1x Exiled Force
- 1x Fiber Jar
- 2x Kycoo the Ghost Destroyer
- 1x Magical Scientist
- 1x Magician of Faith
- 1x Sangan
- 1x Thunder Nyan Nyan
- 1x Tribe-Infecting Virus
- 1x Witch of the Black Forest
- 1x Yata-Garasu

**Spells (17)**

- 1x Change of Heart
- 1x Confiscation
- 1x Dark Hole
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Harpie's Feather Duster
- 1x Mirage of Nightmare
- 1x Monster Reborn
- 3x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Raigeki
- 1x Reinforcement of the Army
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (6)**

- 1x Bell of Destruction
- 1x Call Of The Haunted
- 1x Imperial Order
- 1x Mirror Force
- 2x Torrential Tribute

**Fusion Deck (15)**

- 1x B. Skull Dragon
- 3x Dark Balter the Terrible
- 1x Dark Paladin
- 3x Fiend Skull Dragon
- 1x Fusionist
- 3x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Heavy Storm
- 1x Kycoo the Ghost Destroyer
- 1x Morphing Jar
- 2x Nobleman of Crossout
- 1x Premature Burial
- 3x Sakuretsu Armor
- 1x Soul Release
- 2x Thunder Nyan Nyan
- 1x Torrential Tribute
- 2x Waboku

</details>

Every card was already in the game (Black Skull Dragon is the original B. Skull Dragon). Cards here with no coded effect yet: Dark Balter the Terrible, Ryu Senshi, Fiend Skull Dragon, Dark Flare Knight and Dark Paladin.

### 12. Warrior

Banlist: October 2004.

<details>
<summary>Goat Control: 40% (Gyomberman-Goat Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (17)**

- 1x Asura Priest
- 1x Black Luster Soldier - Envoy of Beginning
- 1x Breaker the Magical Warrior
- 3x D.D. Warrior Lady
- 1x Exiled Force
- 2x Gravekeeper's Spy
- 1x Injection Fairy Lily
- 2x Magician of Faith
- 1x Sinister Serpent
- 1x Tribe-Infecting Virus
- 1x Tsukuyomi
- 1x Zaborg the Thunder Monarch
- 1x Zombyra the Dark

**Spells (19)**

- 1x Book of Moon
- 1x Change of Heart
- 1x Confiscation
- 2x Creature Swap
- 1x Heavy Storm
- 2x Metamorphosis
- 1x Mystical Space Typhoon
- 1x Nobleman of Crossout
- 1x Painful Choice
- 1x Pot of Greed
- 2x Reinforcement of the Army
- 3x Scapegoat
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (4)**

- 1x Bell of Destruction
- 3x Royal Decree

**Fusion Deck (15)**

- 1x Dark Balter the Terrible
- 1x Dark Blade the Dragon Knight
- 1x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 1x Fiend Skull Dragon
- 1x Master of Oz
- 1x Ojama King
- 1x Reaper on the Nightmare
- 2x Ryu Senshi
- 1x St. Joan
- 1x The Last Warrior from Another Planet
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Cipher Soldier
- 1x Gyaku-Gire Panda
- 1x Magical Scientist
- 1x Mystic Swordsman LV2
- 1x Ninja Grandmaster Sasuke
- 1x Nobleman of Crossout
- 3x Sakuretsu Armor
- 1x Torrential Tribute
- 3x Trap Dustshoot
- 1x Tsukuyomi
- 1x Zombyra the Dark

</details>

<details>
<summary>Warrior: 30% (ellietincan-Warrior) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (10)**

- 1x Breaker the Magical Warrior
- 3x D.D. Warrior Lady
- 1x Don Zaloog
- 1x Exiled Force
- 3x Kycoo the Ghost Destroyer
- 1x Tribe-Infecting Virus

**Spells (9)**

- 1x Confiscation
- 1x Painful Choice
- 3x Poison of the Old Man
- 1x Pot of Greed
- 2x Reinforcement of the Army
- 1x The Forceful Sentry

**Traps (21)**

- 1x Bell of Destruction
- 3x Chain Disappearance
- 3x Compulsory Evacuation Device
- 1x Hallowed Life Barrier
- 3x Magic Drain
- 3x Magic Jammer
- 3x Raigeki Break
- 3x Solemn Judgment
- 1x Torrential Tribute

**Fusion Deck (15)**

- 2x Dark Balter the Terrible
- 3x Dark Blade the Dragon Knight
- 3x Dark Flare Knight
- 2x Fiend Skull Dragon
- 2x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Black Luster Soldier - Envoy of Beginning
- 2x Blade Knight
- 1x Change of Heart
- 1x Chaos Sorcerer
- 1x Heavy Storm
- 1x Magical Scientist
- 1x Mystical Space Typhoon
- 3x Sakuretsu Armor
- 3x Smashing Ground
- 1x Snatch Steal

</details>

<details>
<summary>Warrior Control: 20% (Patrick Smith-Warrior Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (16)**

- 1x Black Luster Soldier - Envoy of Beginning
- 2x Blade Knight
- 1x Breaker the Magical Warrior
- 3x D.D. Assailant
- 3x D.D. Warrior Lady
- 1x Fiber Jar
- 2x Kycoo the Ghost Destroyer
- 1x Magical Scientist
- 1x Sinister Serpent
- 1x Tribe-Infecting Virus

**Spells (18)**

- 1x Emergency Provisions
- 3x Enemy Controller
- 1x Heavy Storm
- 1x Mirage of Nightmare
- 1x Mystical Space Typhoon
- 1x Painful Choice
- 1x Pot of Greed
- 1x Premature Burial
- 2x Reinforcement of the Army
- 3x Scapegoat
- 2x Smashing Ground
- 1x Snatch Steal

**Traps (6)**

- 1x Bell of Destruction
- 1x Call Of The Haunted
- 3x Sakuretsu Armor
- 1x Torrential Tribute

**Fusion Deck (15)**

- 2x Dark Balter the Terrible
- 2x Dark Blade the Dragon Knight
- 1x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 1x Fiend Skull Dragon
- 1x Giltia the D. Knight
- 1x Ojama King
- 1x Reaper on the Nightmare
- 2x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Ceasefire
- 1x Cipher Soldier
- 1x Confiscation
- 3x Dust Tornado
- 2x Giant Trunade
- 2x Hallowed Life Barrier
- 1x Jinzo
- 2x Nobleman of Crossout
- 1x Pikeru's Circle of Enchantment
- 1x The Forceful Sentry

</details>

<details>
<summary>Chaos Return: 10% (paoloNS-Chaos Return) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (19)**

- 1x Black Luster Soldier - Envoy of Beginning
- 1x Blade Knight
- 1x Breaker the Magical Warrior
- 1x Chaos Sorcerer
- 3x D.D. Warrior Lady
- 1x Don Zaloog
- 1x Exiled Force
- 1x Fiber Jar
- 2x Kycoo the Ghost Destroyer
- 1x Magical Merchant
- 1x Magical Scientist
- 1x Magician of Faith
- 1x Morphing Jar
- 1x Sinister Serpent
- 1x Spirit Reaper
- 1x Tribe-Infecting Virus

**Spells (15)**

- 1x Change of Heart
- 1x Confiscation
- 1x Heavy Storm
- 1x Mirage of Nightmare
- 1x Mystical Space Typhoon
- 1x Nobleman of Crossout
- 1x Painful Choice
- 1x Pot of Greed
- 1x Premature Burial
- 2x Reinforcement of the Army
- 2x Smashing Ground
- 1x Snatch Steal
- 1x The Forceful Sentry

**Traps (6)**

- 1x Bell of Destruction
- 1x Magic Cylinder
- 2x Return from the Different Dimension
- 1x Sakuretsu Armor
- 1x Torrential Tribute

**Fusion Deck (15)**

- 3x Dark Balter the Terrible
- 3x Dark Blade the Dragon Knight
- 2x Empress Judge
- 1x Ojama King
- 3x Ryu Senshi
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Ceasefire
- 3x Cipher Soldier
- 2x Dust Tornado
- 1x Jinzo
- 3x King Tiger Wanghu
- 1x Ninja Grandmaster Sasuke
- 2x Pikeru's Circle of Enchantment
- 2x Sakuretsu Armor

</details>

Every card was already in the game. Cards here with no coded effect yet: Cipher Soldier, Gyaku-Gire Panda, Hallowed Life Barrier, Poison of the Old Man, Emergency Provisions, Pikeru's Circle of Enchantment, and the Fusion Monsters Dark Balter the Terrible, Ryu Senshi, Dark Blade the Dragon Knight, Fiend Skull Dragon, Reaper on the Nightmare, The Last Warrior from Another Planet, Ojama King and Dark Flare Knight.

### 13-15. Goat to Reaper

Vanilla (unchanged): these opponents still use the original game's decks.

## Deck opponents (debug build only)

A testing page after the formats, left out of `make release`. Each opponent always uses its one deck, all Goat format (April 2005).

| Opponent | Deck | Portrait |
|---|---|---|
| Chaos Turbo | LR-Chaos Turbo | Black Luster Soldier - Envoy of the Beginning |
| Warrior | sdlkilla-Warrior | D.D. Warrior Lady |
| Goat Control | geistD-Goat Control | Thousand-Eyes Restrict |

<details>
<summary>Chaos Turbo (LR-Chaos Turbo) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (20)**

- 1x Black Luster Soldier - Envoy of Beginning
- 1x Breaker the Magical Warrior
- 2x Chaos Sorcerer
- 2x Dekoichi the Battlechanted Locomotive
- 3x Gravekeeper's Spy
- 2x Magician of Faith
- 2x Night Assailant
- 1x Sangan
- 1x Sinister Serpent
- 3x Thunder Dragon
- 1x Tribe-Infecting Virus
- 1x Tsukuyomi

**Spells (13)**

- 1x Book of Moon
- 1x Card Destruction
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Heavy Storm
- 1x Mind Control
- 1x Mystical Space Typhoon
- 2x Nobleman of Crossout
- 1x Pot of Greed
- 1x Snatch Steal
- 2x Upstart Goblin

**Traps (7)**

- 1x Bell of Destruction
- 1x Jar of Greed
- 1x Mirror Force
- 2x Phoenix Wing Wind Blast
- 1x Torrential Tribute
- 1x Trap Dustshoot

**Fusion Deck (15)**

- 1x Dark Balter the Terrible
- 1x Dark Blade the Dragon Knight
- 1x Dark Flare Knight
- 1x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 1x Fiend Skull Dragon
- 1x Gatling Dragon
- 1x King Dragun
- 1x Ojama King
- 1x Reaper on the Nightmare
- 1x Ryu Senshi
- 1x The Last Warrior from Another Planet
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Kycoo the Ghost Destroyer
- 2x Metamorphosis
- 1x Mind Control
- 1x Mobius the Frost Monarch
- 2x Mystik Wok
- 3x Royal Decree
- 2x Scapegoat
- 1x Swords of Revealing Light
- 1x Trap Dustshoot
- 1x Zombyra the Dark

</details>

<details>
<summary>Warrior (sdlkilla-Warrior) (40 cards + 1 Fusion Deck + 15 Side Deck)</summary>

**Monsters (13)**

- 1x Black Luster Soldier - Envoy of Beginning
- 3x Blade Knight
- 1x Breaker the Magical Warrior
- 1x D.D. Warrior Lady
- 1x Don Zaloog
- 1x Exiled Force
- 2x Kycoo the Ghost Destroyer
- 1x Mystic Swordsman LV2
- 1x Ninja Grandmaster Sasuke
- 1x Tribe-Infecting Virus

**Spells (11)**

- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Heavy Storm
- 1x Mystical Space Typhoon
- 2x Nobleman of Crossout
- 1x Pot of Greed
- 1x Premature Burial
- 2x Reinforcement of the Army
- 1x Snatch Steal

**Traps (16)**

- 1x Bell of Destruction
- 1x Call Of The Haunted
- 2x Dust Tornado
- 2x Jar of Greed
- 1x Mirror Force
- 3x Sakuretsu Armor
- 3x Solemn Judgment
- 1x Torrential Tribute
- 2x Trap Dustshoot

**Fusion Deck (1)**

- 1x Thousand-Eyes Restrict

**Side Deck (15)**

- 1x Chaos Sorcerer
- 2x Cipher Soldier
- 1x Dark Scorpion - Cliff the Trap Remover
- 1x Dust Tornado
- 3x Gravekeeper's Spy
- 1x Mind Control
- 2x Mobius the Frost Monarch
- 1x Mystik Wok
- 2x Royal Oppression
- 1x Trap Dustshoot

</details>

<details>
<summary>Goat Control (geistD-Goat Control) (40 cards + 15 Fusion Deck + 15 Side Deck)</summary>

**Monsters (15)**

- 1x Abyss Soldier
- 1x Black Luster Soldier - Envoy of Beginning
- 1x Breaker the Magical Warrior
- 1x Cyber Jar
- 1x D.D. Warrior Lady
- 1x Dark Magician of Chaos
- 2x Magical Merchant
- 2x Magician of Faith
- 1x Morphing Jar
- 1x Sangan
- 1x Sinister Serpent
- 1x Tribe-Infecting Virus
- 1x Tsukuyomi

**Spells (14)**

- 1x Creature Swap
- 1x Delinquent Duo
- 1x Graceful Charity
- 1x Heavy Storm
- 3x Metamorphosis
- 2x Nobleman of Crossout
- 1x Pot of Greed
- 1x Premature Burial
- 2x Scapegoat
- 1x Snatch Steal

**Traps (11)**

- 1x Bell of Destruction
- 3x Jar of Greed
- 1x Mirror Force
- 3x Solemn Judgment
- 1x Torrential Tribute
- 2x Trap Dustshoot

**Fusion Deck (15)**

- 1x Dark Balter the Terrible
- 1x Dark Blade the Dragon Knight
- 1x Darkfire Dragon
- 1x Dragoness the Wicked Knight
- 1x Fiend Skull Dragon
- 1x Gatling Dragon
- 1x Giltia the D. Knight
- 1x King Dragun
- 1x Ojama King
- 1x Reaper on the Nightmare
- 1x Ryu Senshi
- 1x The Last Warrior from Another Planet
- 3x Thousand-Eyes Restrict

**Side Deck (15)**

- 2x Berserk Gorilla
- 3x Dust Tornado
- 2x Emergency Provisions
- 1x Hysteric Fairy
- 1x Kuriboh
- 1x Mystical Space Typhoon
- 1x Poison of the Old Man
- 3x Sakuretsu Armor
- 1x Swords of Revealing Light

</details>
