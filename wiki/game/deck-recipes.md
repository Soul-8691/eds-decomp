---
title: Deck Recipes (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-30
---
# Deck Recipes (mod)

An eighth main-menu item added by this build (`src/mod_recipes.c`, a `@farcode`/`@rodata` unit). The player keeps up to
50 deck recipes and switches between them. Checked in the source and the build; not tested in an emulator.

## Main menu
- `MAIN_MENU_DECK_RECIPES = 7`, `MAIN_MENU_COUNT = 8` (`include/main_menu.h`). `gMainMenuTable` still has 7 entries, so
  `MainMenu_Launch` starts `CB_DeckRecipes` for item 7 itself. The "MENU" header is no longer drawn, and the rows are centred (`MAIN_MENU_TOP_Y`: 16 with 8 rows, 8 with the 9 rows since [[pack-shop]]; 16 px apart; the original started at y = 0x1D under the header).
- **Label.** `DeckRecipes_BuildLabel` (through the generic `BuildMenuLabel`, also used by [[pack-shop]]) builds the label at run time from the other labels in `gMainMenuObjGfx`
  (`0x087DE878`, 2D, 32 tiles a row; item i is pixel row (i+1)*16, plain at x 0-127, highlighted at x+128).
  - Letter sources: "Deck " from Deck Edit, "Rec" from Record, "i" and "p" from Campaign, "e" from Deck, "s" from
    Password.
  - Plain letters are recoloured to Deck Edit's colours: 9 down to row 4, then 8.
  - Highlighted letters keep the sheet pixels in the 8-neighbour dilation of the copied plain pixels. The sheet's
    highlight is the plain glyph plus a 1-pixel black outline, with a few anti-alias pixels (colours 5 and 10)
    inside that dilation. Verified against the ROM with a script.
  - The card icons sit 4 px from the outline on each side, as in every highlighted label.
  - `MainMenu_Init` writes the label to tiles 0x200-0x23F, the two tile rows after the 0x4000-byte sheet.

## Preset recipes
In **`make`** (`MOD_RELEASE` 0) only, the SRAM area is set up on the first visit (and after a New Game) with recipes
01-39 from `GetRecipePreset` (src/mod_decks.c). **`make release`** leaves every slot empty until the player saves a
deck (`GetRecipePreset` returns 0). These are the
mod's deck tables, named after their .ydk files. Every preset carries the .ydk's Side Deck too (`s<Deck>Side`,
passed by `GetRecipePreset` and put in the recipe's side list by `PresetToBuf`) and has its .ydk's Extra Deck as
its Fusion Deck. Presets 31-33 (Chaos Turbo, Warrior, Goat Control; 22-24 until the Vampire, Chaos and Warrior decks came) got theirs on 2026-10-30: until then they held
the Main Deck only (Goat Control 3 of its 15 Fusion Monsters) and no Side Deck. Checked against the .ydk files (passcodes mapped through
assets/cards/passwords.csv and build/new_cards.tsv): identical, except Gradius' Option, left out of GBO-Recruiter
Control's Side Deck (4-Starred Ladybug of Doom is an added card now, [[added-cards]]). The three starter decks
(`BuildModStarterDeck`) get the same Side and Fusion Decks.

| Slot | Name | Deck table |
|---|---|---|
| 01 | El Duelista DEX-Soul Control | Red starter, Tristan's Soul Control |
| 02 | ellietincan-Vanilla Beat | Green starter, Tristan's Vanilla Beat |
| 03 | DK-ZERO-Stall Mill | Black starter, Tristan's Stall Mill |
| 04 | Jazz-Masked Control | the Critter opponent (60%) |
| 05 | Gyomberman-Muka Muka Turbo | the Critter opponent (40%) |
| 06 | dump truck-Tomato Control | the Treasure opponent (80%) |
| 07 | SolEmerald69-Recruiter Control | the Treasure opponent (10%) |
| 08 | bsocks-Germ Burn | the Treasure opponent (10%) |
| 09 | Aretos-Tomato Control | the Imperial opponent (80%) |
| 10 | bsocks-Relinquished | the Imperial opponent (20%) |
| 11 | Jazz-Tomato Control | the Android opponent (80%) |
| 12 | paoloNS-Vanilla Beat | the Android opponent (20%) |
| 13 | AJTBLS-Tomato Control | the Joey-Pegasus opponent (80%) |
| 14 | ellietincan-Vanilla Beat (JP) | the Joey-Pegasus opponent (20%) |
| 15 | Aretos-Tomato Control (Fiber) | the Fiber opponent (80%) |
| 16 | GBO-Recruiter Control | the Fiber opponent (20%) |
| 17 | Shigeki Kitamura-Tomato Control | the Yata opponent (50%) |
| 18 | ellietincan-Warrior Tomato Control | the Yata opponent (40%) |
| 19 | AlephYa2-Gearfried | the Yata opponent (10%) |
| 20 | Aretos-Tomato Control (Scientist) | the Scientist opponent (60%) |
| 21 | DK-ZERO-Scientist FTK | the Scientist opponent (40%) |
| 22 | Woawa-Warrior Control | the Vampire opponent (80%) |
| 23 | ellietincan-Scientist FTK | the Vampire opponent (10%) |
| 24 | AJTBLS-Warrior Tomato Control | the Vampire opponent (10%) |
| 25 | ellietincan-Chaos Control | the Chaos opponent (80%) |
| 26 | indoors-Chaos Warrior | the Chaos opponent (20%) |
| 27 | Gyomberman-Goat Control | the Warrior opponent (40%) |
| 28 | ellietincan-Warrior | the Warrior opponent (30%) |
| 29 | Patrick Smith-Warrior Control | the Warrior opponent (20%) |
| 30 | paoloNS-Chaos Return | the Warrior opponent (10%) |
| 31 | LR-Chaos Turbo | Tea, the Chaos Turbo deck opponent (debug build); the Goat opponent (40%) |
| 32 | sdlkilla-Warrior | the Warrior deck opponent (debug build); the Goat opponent (20%) |
| 33 | geistD-Goat Control | the Goat Control deck opponent (debug build); the Goat opponent (15%) |
| 34 | Nova-Chaos Control | the Goat opponent (10%; N♡va-Chaos Control.ydk, the font has no heart) |
| 35 | wolfe_KMH-Panda Burn | the Goat opponent (5%) |
| 36 | Dingosig-Chaos Return | the Goat opponent (5%) |
| 37 | paede17-Reasoning Gate Turbo | the Goat opponent (5%) |
| 38 | Brian Long-Goat Control | the Cyber opponent (50%) |
| 39 | splevin-Stein Monarch | the Cyber opponent (50%) |
| 40 | splevin-Stein Monarch | the Reaper opponent (40%) |
| 41 | sdlkilla-Warrior | the Reaper opponent (20%) |
| 42 | brahimpoke-Bazoo Return | the Reaper opponent (15%) |
| 43 | ellietincan-Chaos Control | the Reaper opponent (15%) |
| 44 | Roland Gress-Tomato Control | the Reaper opponent (10%) |

"ellietincan-Warrior Tomato Control" is 34 characters, so the slot stores the first 31 (`RECIPE_NAME_SIZE` 32), and
the list row shows 26 and "..". "Aretos-Tomato Control (Scientist)" (33) is cut the same way.

> [!note] Existing saves
> The presets are written only when the area is set up (first visit, or after a New Game), so a save made before
> 2026-10-30 has no Scientist presets until then.

- Fusion Monsters in a table go to the recipe's Fusion Deck. There are no Side Decks, as with the opponents' decks.
- A table that this build leaves out (its cards aren't all in the card list) is skipped.
- Presets are ordinary recipes: they can be replaced, renamed or cleared.

## Storage
- **SRAM.** From `0x0E004000`, after the 0x2170-byte save. A header (magic `"RCP2"`, the active slot) is followed
  by 50 slots. Each slot holds the three list sizes, a used flag, `name[32]`, and `deck[60]`, `side[15]`,
  `fusion[20]` (226 bytes). A different magic (including the first build's `"RCP1"`) sets the area up again.
- **New Game.** `BuildStarterDeck` calls `DeckRecipes_OnNewGame`, which clears the recipes: it erases the header's
  magic, so the next visit sets the area up again with the presets and empty slots. The starter deck then takes
  the first empty slot (07).
- **RAM.** The scene state is `gDeckRecipes` (EWRAM `0x0202E400`, symbols.ld).

## Load and save (rewritten 2026-10-26)
A recipe is a template: it changes only when the player saves to it, renames it or clears it.

- **The mark.** `header.active` marks the recipe last loaded or saved (`*` in the list). It is only a mark. The first
  model wrote the current deck back into the "recipe in use" on every visit, switch and exit, so loading a preset
  and saving to a new slot overwrote the preset too (reported bug).
- **A on a slot** opens its actions in place of the help line (LEFT/RIGHT, A, B):
  - **Load.** If the current deck isn't the marked recipe's lists (or there's no mark) and has cards, a second A
    is asked for ("Your deck is not saved"). Then every card of the three decks goes back to the trunk
    (`ReturnDecksToTrunk`) and the recipe's cards come out of it (`AddCardToSaved*Deck`, then
    `RemoveCardFromTrunk`). Cards the trunk lacks, or that the banlist refuses, are left out; the recipe keeps them.
    Then the slot is marked and the game saves.
  - **Left-out list.** If cards were left out, a list opens (`MODE_LEFT_OUT`): "N card(s) left out (banlist: X)",
    then "xN name" with the reason: forbidden, limited to N, not in pool (the Deck Edit banlist in use), none in
    trunk, or list full. The banlist wins when the list already holds what it allows. Up/Down scroll, A or B
    closes. Example: Soul Control under the default Goat banlist leaves out Change of Heart, Dark Hole, Monster
    Reborn and Raigeki (forbidden) and a second Swords of Revealing Light (limited to 1).
  - **Save here.** An empty slot opens the name entry (starting from the marked recipe's name). A used slot asks
    for a second A and keeps its name. The slot is marked.
  - **Rename.** The name entry.
  - **Clear.** After a second A; clearing the marked slot removes the mark.
- **Name entry.** A 13 x 6 grid of keys: A-Z, a-z, 0-9, `- / . ' & ! ? ( ) : + , #`, and Spc/Del/End. Controls:
  A types, B deletes, START (or End) finishes, SELECT cancels. Names are up to 28 characters, which fits a list
  row before the counts; an empty name shows as "Custom deck".
- **First visit after set-up** (first use, or after a New Game). The current deck is saved, unnamed, into the first
  empty slot and marked.

## Screen
The menu's sky (BG1) and palette, with the highlighted label as the title. The list, status and help are drawn on a
32 x 16 tile text canvas (`TextCanvasToTiles` into OBJ tile 0x40), shown as 32 x 16 sprites as in `mod_banlist`.
Rows read `> 01 * name` (`*` marks the recipe in use), with `D40 S0 F3` at x = 186. Colours from the menu palette: rows 8, cursor 7, status 3, help 10, shadow 1.

Related: [[main-menu]], [[save-type]], [[added-cards]].

List rows cut a name longer than 28 characters to 26 and "..".
