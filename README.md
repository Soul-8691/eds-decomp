# Yak's YGO Hack

A hack of *Yu-Gi-Oh! The Eternal Duelist Soul* (GBA) that follows the
[Format Library](https://www.formatlibrary.com/) pantheon of TCG formats, from Yugi-Kaiba up to Reaper. About 50
Goat-era cards have been added so far, with roughly 50 more planned.

## The formats

Each format is a Campaign "duelist". You start with Yugi-Kaiba only; beating a format **10 times** unlocks the
next one.

| # | Format | Banlist |
|---|---|---|
| 1 | Yugi-Kaiba | May 2002 |
| 2 | Critter | July 2002 |
| 3 | Treasure | October 2002 |
| 4 | Imperial | December 2002 |
| 5 | Android | April 2003 |
| 6 | Joey-Pegasus | May 2003 |
| 7 | Fiber | July 2003 |
| 8 | Yata | August 2003 |
| 9 | Scientist | November 2003 |
| 10 | Vampire | February 2004 |
| 11 | Chaos | April 2004 |
| 12 | Warrior | October 2004 |
| 13 | Goat | April 2005 |
| 14 | Cyber | April 2005 |
| 15 | Reaper | October 2005 |

- **Decks.** Each format duelist picks one of 2-3 (sometimes more) strong decks from its format, at weighted
  random. The first 7 duelists are fully coded. [DECKS.md](DECKS.md) lists every deck and its chance.
- **Your deck.** It has to be legal in that format's banlist and card pool to duel them.
- **Portraits.** Each duelist's portrait is its format's namesake card (or the Format Library picture when the
  format isn't named after a card).
- **No dialogue.** Duelists don't talk before or after a duel.
- **Event challengers.** On calendar event days (Championship, Duel Ceremony, Grandpa Cup, Rare Hunter), the
  challenger is drawn from the formats you've unlocked whose banlist your cards can make a 40-card deck under, with
  its original portrait and event dialogue. After the dialogue, your banlist switches to the challenger's format and
  the Deck Recipes screen opens: load a recipe, press SELECT for Deck Edit, or B to duel. In the release, a deck that
  isn't legal in the format (or is under 40 cards) can't duel until you fix it.
- **Deck Recipes.** Up to 50 saved decks, about 20 of them preset to staple decks of the first seven formats and
  Goat. Loading a recipe takes the cards from your trunk; any you don't have are left out (the game tells you which,
  and why). The starting decks and the format presets come with the Side and Extra Decks of their .ydk files. The
  deck recipes will eventually include all 15 TCG formats.
- **Banlists** (main menu). Choose the banlist your deck is built under. Cards your current deck can no longer hold
  go back to the trunk. Before switching, the game checks that your cards can still make a Main Deck of at least
  40 cards under the new banlist (counting its limits and card pool); if they can't, the switch is refused.
- **Duelist Points (DP).**
  - You start with 5000 DP and earn more by winning duels.
  - The card password machine now sells cards: it shows the price and asks before buying.
  - Prices run from 5 to 3000 DP, depending on how much the card is played in the furthest format you've unlocked.
  Cards not in the current furthest format you've unlocked are priced at 3000 DP. [CARD_PRICES.md](CARD_PRICES.md) lists each format's prices.
- **Booster Packs** (main menu). Open any unlocked pack for 1000 DP each for now; prices will change in the release.
  Packs will cover the TCG/OCG sets up to Reaper/Victory format, plus attribute/race/Spell/Trap packs once there are
  enough cards.
- **Card art in duels.** Face-up cards show a small picture of their artwork inside their icon: on the field
  (monsters, Spells/Traps, the Field zone, the top card of each Graveyard and banished pile), in your hand, in the
  card menu, when reordering cards, and while cards move, flip or change position.
- **Still backgrounds.** The field background no longer scrolls during duels, and the title screen's background
  no longer scrolls either.
- **Smarter CPU.**
  - It switches a monster that can't beat any of yours to Defense Position when you have a face-up monster with
    higher ATK, instead of leaving it to be run over. A monster with 1700 ATK or more stays in Attack Position
    unless your strongest monster has at least 1000 more ATK than it.
  - It crashes into your monsters with equal ATK when it has no better attack, unless another of its monsters can
    beat yours outright.
- **Debug mode** (testing builds only, not in the release). You start with 8 copies of every card, and you can't
  deck out. Decks can be under 40 cards, can hold any number of copies of a card, and ignore banlists. Switching
  banlists skips the 40-card Main Deck check. Every format is unlocked, and the Record (Duel Score) screen
  shows every duelist on every page.

### Planned

- 8 actual duelists: Yugi Muto, Joey, Kaiba, Pegasus, Marik, Yami Marik, Yami Bakura and Yami Yugi.
- The OCG formats up to Victory, unlocked after beating Reaper.

## How the new cards are added

- **Coded cards.** Every card listed in [`new_cards.txt`](new_cards.txt) (in the repository root) is fully coded,
  effects included.
- **Card data.** New cards are listed by name in `config/new_cards.txt`. The build fetches each card's stats, text
  and artwork from [YGOPRODeck](https://ygoprodeck.com/api-guide/) and appends it to the game's card tables
  (`tools/newcards.py`).
- **Effects.** Effects are written in C on top of the original duel engine, mostly in `src/added_effects.c`. Each
  card gets the same handlers the game's own cards use: activation check, cost, targeting, resolution. Small hooks
  in the engine cover timings it doesn't already have, and each card gets a rule for when the computer should use it.
- **Engine leftovers.** The engine already contains several cards that never made it into EDS, such as Magic
  Cylinder, Soul Exchange, Fairy Box and Thousand-Eyes Restrict. Those reuse that existing code instead of being
  written from scratch.

## Playing

Ready-made IPS patches are in the [`patches/`](patches/) folder: a release build and a debug build (the demo, up to
Fiber). Apply one to your own USA ROM of *The Eternal Duelist Soul* (AY5E) with any IPS patcher, or let an emulator
that auto-loads patches, such as mGBA, apply it.

## Building

This hack is built on a [matching decompilation](wiki/overview.md) of the game. You need your own copy of the USA ROM;
it is never included.

```
(cd ../agbcc && ./build.sh)    # once: pret/agbcc checked out next to this repository
make setup                     # once: put your ROM in roms/ and extract the game data
make -j$(nproc)                # eds.gba: the debug/testing build
make -j$(nproc) release        # eds_release.gba: the release build
```

You need `make`, `cpp`, `python3` and `binutils-arm-none-eabi`, plus network access the first time new cards are
fetched. See `CLAUDE.md` and the `wiki/` for details on the decompilation and the hack.
