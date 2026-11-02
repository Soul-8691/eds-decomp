# Yak's YGO Hack

<p align="center">
  <img src="screenshots/Title Screen.png" width="24%" alt="Title Screen">
  <img src="screenshots/Deck Edit.png" width="24%" alt="Deck Edit">
  <img src="cards/Breaker the Magical Warrior.png" width="24%" alt="Breaker the Magical Warrior">
  <img src="cards/Black Luster Soldier - Envoy of the Beginning.png" width="24%" alt="Black Luster Soldier - Envoy of the Beginning">
</p>
<p align="center">
  <img src="cards/Cyber Dragon.png" width="24%" alt="Cyber Dragon">
  <img src="cards/Treeborn Frog.png" width="24%" alt="Treeborn Frog">
  <img src="cards/Torrential Tribute.png" width="24%" alt="Torrential Tribute">
  <img src="cards/Trap Dustshoot.png" width="24%" alt="Trap Dustshoot">
</p>
<p align="center">
  <img src="cards/Card Destruction.png" width="24%" alt="Card Destruction">
  <img src="cards/Reinforcement of the Army.png" width="24%" alt="Reinforcement of the Army">
  <img src="cards/Pot of Avarice.png" width="24%" alt="Pot of Avarice">
  <img src="cards/Thousand-Eyes Restrict.png" width="24%" alt="Thousand-Eyes Restrict">
</p>
<p align="center">
  <img src="cards/Cyber Twin Dragon.png" width="24%" alt="Cyber Twin Dragon">
  <img src="cards/Cyber End Dragon.png" width="24%" alt="Cyber End Dragon">
  <img src="screenshots/Password.png" width="24%" alt="Password">
  <img src="screenshots/Deck Recipes (Main Menu).png" width="24%" alt="Deck Recipes (Main Menu)">
</p>
<p align="center">
  <img src="screenshots/Booster Packs (Main Menu).png" width="24%" alt="Booster Packs (Main Menu)">
  <img src="screenshots/Banlists (Main Menu).png" width="24%" alt="Banlists (Main Menu)">
  <img src="screenshots/Settings (Main Menu).png" width="24%" alt="Settings (Main Menu)">
  <img src="screenshots/Deck Recipes.png" width="24%" alt="Deck Recipes">
</p>
<p align="center">
  <img src="screenshots/Pack Opener.png" width="24%" alt="Pack Opener">
  <img src="screenshots/Banlists.png" width="24%" alt="Banlists">
  <img src="screenshots/Settings.png" width="24%" alt="Settings">
  <img src="screenshots/Duelists (Screen 1).png" width="24%" alt="Duelists (Screen 1)">
</p>
<p align="center">
  <img src="screenshots/Duelists (Screen 2).png" width="24%" alt="Duelists (Screen 2)">
  <img src="screenshots/Duelists (Screen 3).png" width="24%" alt="Duelists (Screen 3)">
  <img src="screenshots/Duelists (Screen 4).png" width="24%" alt="Duelists (Screen 4)">
  <img src="screenshots/Card Icons.png" width="24%" alt="Card Icons">
</p>
<p align="center">
  <img src="screenshots/Card Icons (Up Close) - Summon-Set Order.png" width="24%" alt="Card Icons (Up Close) - Summon-Set Order">
  <img src="screenshots/Activate-Set Order.png" width="24%" alt="Activate-Set Order">
  <img src="screenshots/Counters.png" width="24%" alt="Counters">
</p>

A hack of *Yu-Gi-Oh! The Eternal Duelist Soul* (GBA) that follows the
[Format Library](https://www.formatlibrary.com/) pantheon of TCG formats, from Yugi-Kaiba up to Reaper. The **demo**
build (`make demo`) adds **152** cards (repo-root [`new_cards.txt`](new_cards.txt) above `------`, in alphabetical
order); **[NEW_CARDS.md](NEW_CARDS.md)** shows each of them in Deck Edit's Card View. **`make`** / **`make release`**
use the full list in [`config/new_cards.txt`](config/new_cards.txt) (**343** names today, still growing).

## The formats

Each format is a Campaign "duelist". You start with Yugi-Kaiba only; beat the current format duelist **20 times**
(in release; every format is open in the debug build) to unlock the next one.

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
  random. The first 15 duelists are fully coded. [DECKS.md](DECKS.md) lists every deck and its chance.
- **Your deck.** It has to be legal in that format's banlist and card pool to duel them.
- **Portraits.** Each duelist's portrait is its format's namesake card (or the Format Library picture when the
  format isn't named after a card).
- **No dialogue.** Duelists don't talk before or after a duel.
- **Event challengers.** On calendar event days (Championship, Duel Ceremony, Grandpa Cup, Rare Hunter), the
  challenger is drawn from the formats you've unlocked whose banlist your cards can make a 40-card deck under, with
  its original portrait and event dialogue. After the dialogue, your banlist switches to the challenger's format and
  the Deck Recipes screen opens: load a recipe, press SELECT for Deck Edit, or B to duel. In the release, a deck that
  isn't legal in the format (or is under 40 cards) can't duel until you fix it.
- **Deck Recipes.** Up to 60 saved decks. In the debug build (`make`), the first slots are premade staple decks of the
  first fifteen formats (including Goat), named after their .ydk files; the release build starts with empty slots until
  you save your own. Loading a recipe takes the cards from your trunk; any you don't have are left out (the game tells
  you which, and why). Premade recipes include Side and Extra Decks from their .ydk files. The deck recipes will
  eventually include all 15 TCG formats in release too.
- **Banlists** (main menu). Choose the banlist your deck is built under. Cards your current deck can no longer hold
  go back to the trunk. Before switching, the game checks that your cards can still make a Main Deck of at least
  40 cards under the new banlist (counting its limits and card pool); if they can't, the switch is refused.
- **Settings** (main menu, or **L** during a duel). Pick one duel speed: **DEFAULT** (hold B to fast-forward),
  **ALWAYS** (always fast-forward), or **SUPER** (always fast-forward with doubled skip amounts). The choice is
  saved; leaving settings with **B** returns to the duel when opened with **L**.
- **Duelist Points (DP).**
  - You start with 5000 DP and earn more by winning duels.
  - The card password machine now sells cards: it shows the price and asks before buying.
  - Cards that appear in a format's usage statistics are priced from 200 to 3000 DP within that format (usage share,
    with Limited/Semi-Limited multipliers, then linearly scaled). Cards with no usage entry cost **100** DP.
  [CARD_PRICES.md](CARD_PRICES.md) lists each format's scaled prices.
- **Booster Packs** (main menu). TCG sets from `packs/*_tcg.json` (including **Dark Revelation Volume 1**, imported
  with `tools/import_yugipedia_drv1.py`). Only ROM cards are in each pool; pull rates follow listed rarities; the cover
  art is the pool's rarest card. A TCG set unlocks when its format opponent is unlocked (for example Metal Raiders
  with Critter). Each format also has a **usage-weighted pack** (cards priced from that format's usage statistics);
  those unlock after **10 wins** against that format duelist. Format packs cost **250** DP (TCG sets) or **500** DP
  (usage packs); vanilla campaign booster IDs still cost **1000** DP when they appear. Free in debug (`make`). The shop
  list can hold **64** unlocked format rows (interleaved by format); `gSceneWork` still shows 32 at a time while
  scrolling. **Campaign wins** against format duelists (not Solomon or Debug) use this **same unlocked list**: pick
  **one free pack** after the duel. When a win opens new packs (usage pack at 10 wins in that format, or TCG sets
  when the next format unlocks at **20** wins), the game shows the **"new packs are available"** message before the
  list.
- **Format Library usage stats ([Mobius](https://github.com/Soul-8691/Mobius)).** Card DP and the usage-weighted
  booster pools in this hack come from [Format Library](https://www.formatlibrary.com/) tournament usage data.
  **[Mobius](https://github.com/Soul-8691/Mobius)** is a companion Python app for exploring those stats in a table
  (ban-weighted counts, Limited/Semi-Limited multipliers, YGOPRODeck names and art). Clone that repository, then
  with Python 3.9 or newer run `pip install -r requirements.txt` and `python main.py`. A format appears in the
  dropdown when it has both a pool file under `data/format_library/pools/` and a matching usage file under
  `data/format_library/usage/` (usage is read from each JSON's top-level `main` array; pool cards with no entry count
  as zero). On first launch the app downloads the full YGOPRODeck v7 database to
  `data/cache/ygopro/card_database.json`; run `python download_ygopro_database.py` to refresh it. Card images cache
  under `data/cache/ygopro/images/` as you scroll. For one card's usage across formats as a line chart, run
  `python graph_card_usage.py "Monster Reborn"` (optional `--start yugi-kaiba --end goat` and `-o chart.png`; format
  stems match the pool filenames, e.g. `yugi-kaiba`, `cat`). See Mobius's README for chart semantics and options.
- **Card art in duels.** Face-up cards show a small picture of their artwork inside their icon: on the field
  (monsters, Spells/Traps, the Field zone, the top card of each Graveyard and banished pile), in your hand, in the
  card menu, in the chain list, when reordering cards, and while cards move, flip or change position. Deck Edit's
  column of cards beside the list shows each card's art too.
- **Still backgrounds.** The field background no longer scrolls during duels, the title screen's background
  no longer scrolls either, and the pack pull screen's patterned background no longer scrolls (Campaign packs too).
- **Fixes.** Deck Edit no longer blots out the first letters of the row above the cursor after Card View (a
  bug in the original game). In the hand/field card menu, **Summon** comes
  before **Set**, **Activate** comes before **Set** for Spells you can activate or Set, and **Card Info** is always the
  last option (Left/Right order matches the row).
- **Solomon and Debug.** The opponent page after the formats has Solomon (top left) and Debug (top right), open from
  the start: before the duel you pick one of your own Deck Recipes for them to play. Beating them gives no DP, so
  they're for practice and testing decks. Against Debug you first pick your own deck the same way (a recipe, or
  one you build from 8 of every card with no limits), then Debug's; your cards and decks aren't touched, and your
  pick is used for the whole match. Debug's duels use debug mode's rules: its hand is shown face up, you can
  read its face-down cards (info bar and Card View), open its main and fusion deck piles in order, and neither side
  can deck out.
  - **Building their deck.** Press SELECT on the recipe list to build their deck instead. Deck Edit opens with a
    temporary collection of 3 copies of every card and no banlist (Debug: 8 copies of every card, and no copy
    limit). Back on the list, A opens the recipe actions:
    load a recipe into their deck to start from, or save their deck to a slot. SELECT goes back to Deck Edit, B
    starts the duel with their deck, and START discards it. Your own cards, decks and banlist come back unchanged either way
    (they stay saved while you build, so a reset loses nothing).
- **Best-of-3 matches.** Every Campaign duel against the CPU is a best-of-3 match, not only on match days.
  Between games you can side, swapping cards between your Main Deck and Side Deck. Your deck goes back to how it
  was once the match ends. The CPU uses the same deck for every game of the match. Records, DP and the pack
  reward count the match, not each game.
- **Smarter CPU.** Campaign opponents play much stronger than vanilla EDS. See [AI_IMPROVEMENTS.md](AI_IMPROVEMENTS.md)
  for battle and position rules, card use, flip-summon timing, and what the CPU can and cannot see.
- **Cards can fail to find.** Cards like Thunder Dragon and Toon Table of Contents can be activated even with no copy left in the Deck; the search then finds nothing.
- **Debug mode** (testing builds only, not in the release). You start with 8 copies of every card. On **New Game**,
  choosing the **red** starter deck leaves your saved Main, Fusion and Side Decks **empty** (build from the trunk);
  green and black still load their premade Yugi-Kaiba recipe decks. **Deck-out is off** in every Campaign duel
  (format opponents included); Link Battles still use normal deck-out rules. In the **release** build, deck-out is
  blocked only vs **Debug** (and **Test Duel** is not offered). Decks can be under 40 cards, can hold any number of copies of a card, and ignore banlists. Switching
  banlists skips the 40-card Main Deck check. Every format is unlocked, and the Record (Duel Score) screen
  shows every duelist on every page. The opponent's hand is shown face up during
  duels, and the cursor shows the info of all their cards (face-down ones too), with Card View.   You can open your deck pile from the command menu (Draw / Surrender / Card View) and the opponent's main or fusion
  deck pile with **Card View** (draw order on the main deck; works even when their deck is empty).
  The deck
  opponents' page (fourth page, right slot) has **Test Duel**, whose deck is the `.ydk` in `TEST_YDK` (default
  `/mnt/c/ProjectIgnis/deck/test.ydk`) as read when the ROM is built. Only cards in **that** build exist: for
  `make demo`, every passcode in the deck must be among the coded cards in repo-root `new_cards.txt` above `------`;
  passcodes that are missing are dropped, and if none remain the opponent starts with **no deck** (not the vanilla
  Duel Computer deck). An empty `.ydk` does the same.

### Planned

- 8 actual duelists: Yugi Muto, Joey, Kaiba, Pegasus, Marik, Yami Marik, Yami Bakura and Yami Yugi.
- The OCG formats up to Victory, unlocked after beating Reaper.

## How the new cards are added

- **Coded cards.** Every card listed in [`new_cards.txt`](new_cards.txt) above the `------` line is in the demo
  ROM with stats, text and art; the lines below the separator are planned, not in the demo ROM yet.
  [NEW_CARDS.md](NEW_CARDS.md) shows a screenshot of each of them in the game (taken by
  `tools/card_screenshots.lua` in mGBA; optional `cards/_capture_only.txt` for new entries only;
  `tools/gen_new_cards_md.py` refreshes the gallery). Reorder `new_cards.txt` only together with **`make demo`**
  so card IDs stay in sync before re-running the script.
- **Fusion monsters not coded yet.** These demo-list Fusion Monsters are **not** fully implemented (no fusion
  effects yet; they do not behave like their real cards):
  Dark Balter the Terrible, Ryu Senshi, Gatling Dragon, Dark Blade the Dragon Knight, Fiend Skull Dragon,
  Reaper on the Nightmare, The Last Warrior from Another Planet, King Dragun, Ojama King, Master of Oz,
  Dark Flare Knight, Dark Paladin, and Cyber Blader.
- **Card data.** `make` / `make release` use every name in `config/new_cards.txt`. `make demo` is the same as `make`
  (debug/testing rules, `MOD_RELEASE` 0) but only the coded block at the top of repo-root `new_cards.txt`.
  `make release-demo` is the release rules with that same demo list. The build fetches each card's stats, text and
  artwork from [YGOPRODeck](https://ygoprodeck.com/api-guide/) and appends it to the game's card tables
  (`tools/newcards.py`).
- **Effects.** Effects are written in C on top of the original duel engine, mostly in `src/added_effects.c`. Each
  card gets the same handlers the game's own cards use: activation check, cost, targeting, resolution. Small hooks
  in the engine cover timings it doesn't already have, and each card gets a rule for when the computer should use it.
- **Engine leftovers.** The engine already contains several cards that never made it into EDS, such as Magic
  Cylinder, Soul Exchange, Fairy Box and Thousand-Eyes Restrict. Those reuse that existing code instead of being
  written from scratch. The full inventory (157 unused card numbers and what each handler does) is in
  [LEFTOVER_EFFECTS.md](https://github.com/Soul-8691/eds-decomp/blob/binary/LEFTOVER_EFFECTS.md) on the
  [`binary`](https://github.com/Soul-8691/eds-decomp/tree/binary) branch.

## Playing

Ready-made **BPS** patches are in the [`patches/`](patches/) folder: a release build and a debug build (the demo, up
to Reaper). You need your own USA ROM of *The Eternal Duelist Soul* (AY5E). Apply a patch in the browser with
[Rom Patcher JS](https://www.marcrobledo.com/RomPatcher.js/): open the site, choose your ROM as the input file, choose
the `.bps` from `patches/`, then download the patched ROM.

## Building

This hack is built on a [matching decompilation](wiki/overview.md) of the game. You need your own copy of the USA ROM;
it is never included.

```
(cd ../agbcc && ./build.sh)    # once: pret/agbcc checked out next to this repository
make setup                     # once: baserom.gba in repo root or your ROM in roms/, then extract assets
make -j$(nproc)                # eds.gba: the debug/testing build (config/new_cards.txt)
make -j$(nproc) release        # eds_release.gba: the release build
make -j$(nproc) demo           # eds_demo.gba: debug build, cards in new_cards.txt above ------
make -j$(nproc) release-demo   # eds_release_demo.gba: release build, same demo card pool
```

You need `make`, `cpp`, `python3` and `binutils-arm-none-eabi`, plus network access the first time new cards are
fetched. See `CLAUDE.md` and the `wiki/` for details on the decompilation and the hack.
