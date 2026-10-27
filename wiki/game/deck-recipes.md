---
title: Deck Recipes (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-26
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
The SRAM area is set up on the first visit (and after a New Game) with recipes 01-19 from `GetRecipePreset` (src/mod_decks.c). These are the
mod's deck tables, named after their .ydk files:

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
| 17 | LR-Chaos Turbo | Tea, the Chaos Turbo deck opponent |
| 18 | sdlkilla-Warrior | the Warrior deck opponent |
| 19 | geistD-Goat Control | the Goat Control deck opponent |

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
