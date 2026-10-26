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
The SRAM area is set up on the first visit with recipes 01-06 from `GetRecipePreset` (src/mod_decks.c). These are the
mod's deck tables, named after their .ydk files:

| Slot | Name | Deck table |
|---|---|---|
| 01 | El Duelista DEX-Soul Control | Tristan's Soul Control |
| 02 | ellietincan-Vanilla Beat | Tristan's Vanilla Beat |
| 03 | DK-ZERO-Stall Mill | Tristan's Stall Mill |
| 04 | LR-Chaos Turbo | Red starter and Tea |
| 05 | sdlkilla-Warrior | Joey (Green starter) |
| 06 | geistD-Goat Control | Bakura (Black starter) |

- Fusion Monsters in a table go to the recipe's Fusion Deck. There are no Side Decks, as with the opponents' decks.
- A table that this build leaves out (its cards aren't all in the card list) is skipped.
- Presets are ordinary recipes: SELECT replaces one with the current deck, and START clears it.
- **Names.** A recipe saved with SELECT takes the name of the recipe in use, and stays named when Deck Edit changes
  it. A recipe with no name shows "Custom deck".

## Storage
- **SRAM.** From `0x0E004000`, after the 0x2170-byte save. A header (magic `"RCP2"`, the active slot) is followed
  by 50 slots. Each slot holds the three list sizes, a used flag, `name[32]`, and `deck[60]`, `side[15]`,
  `fusion[20]` (226 bytes). A different magic (including the first build's `"RCP1"`) sets the area up again.
- **Recipe in use.** One recipe is in use: it is the current deck, and it is stored back into its slot when the
  scene opens, on every switch, and on exit (Deck Edit changes reach it this way).
- **New Game.** `BuildStarterDeck` calls `DeckRecipes_OnNewGame`, which clears the active slot so the starter deck
  doesn't overwrite a recipe. On the next visit, the current deck takes the first empty slot.
- **RAM.** The scene state is `gDeckRecipes` (EWRAM `0x0202E400`, symbols.ld).

## Switching (A)
1. Store the current deck into the recipe in use.
2. Put every Deck, Side Deck and Fusion Deck card back into the trunk (`AddCardToTrunk`, list counters to 0).
3. Take the new recipe's cards out of the trunk (`AddCardToSaved*Deck`, then `RemoveCardFromTrunk`). A card the trunk
   doesn't hold, or that the copy limits refuse, is counted as missing and reported.
4. `SaveGame`.

An empty recipe gives an empty deck, which can't duel (`DECK_MIN_CARDS`) until it's built in Deck Edit.

## Other keys
- **SELECT, twice:** saves the current deck into the slot.
- **START, twice:** clears the slot. The recipe in use can't be cleared.
- **L/R:** page by 8.
- **B:** back to the menu.

## Screen
The menu's sky (BG1) and palette, with the highlighted label as the title. The list, status and help are drawn on a
32 x 16 tile text canvas (`TextCanvasToTiles` into OBJ tile 0x40), shown as 32 x 16 sprites as in `mod_banlist`.
Rows read `> 01 * name` (`*` marks the recipe in use), with `D40 S0 F3` at x = 186. Colours from the menu palette: rows 8, cursor 7, status 3, help 10, shadow 1.

Related: [[main-menu]], [[save-type]], [[added-cards]].
