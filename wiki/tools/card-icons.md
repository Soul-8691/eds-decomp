---
title: Card Icons with Art (tools/cardicons.py)
type: tool
status: draft
confidence: high
sources: [rom-analysis]
updated: 2026-10-28
---

# Card icons with art

`tools/cardicons.py` builds the duel field's per-card 32x32 card icons, with a 17x17 miniature of the card's
art. `make` builds the game's data with it (`--data`, below); `make card-icons` writes PNG previews.

- **Icons.** `card_icons/<type>.png` (normal, effect, fusion, ritual monster, spell, trap): pose 0 of the game's
  `gCardIcon*Gfx` with `gCardIconPal` (index 0 transparent). Verified: `effect_monster.png` equals the decoded
  `gCardIconEffectGfx` pose 0 (4x4 tiles of 8x8, 4bpp) and its palette equals `gCardIconPal`.
  `fusion_monster.png` was written from `gCardIconFusionGfx` the same way.
- **Type.** As `GetCardIconGfx`: Magic, Trap, else by frame kind (stats bits 18-19).
- **Miniature.** The centre 72x72 of the 72x80 art (rows 4-75), area-averaged in linear light to 17x17, drawn at
  x 7-23, y 7-23: one pixel left of the black square (x 8-22, y 7-22), top aligned with it.
- **Palette.** One shared 64-colour palette for every miniature (median cut, then k-means over all miniature
  pixels; colours on the 15-bit grid), meant for BG palettes 12-15, which the duel leaves unused.
- **Output** (`build/card_icons/`, never committed: the art is the ROM's): `NNNN.png` per card ID (palette 0-15
  the icon's, 16-79 the shared art palette), `palette.bin` (64 BGR555), `palette.png`, `sheet.png`, `index.tsv`.


## In duels (`src/mod_field_icons.c`)
- **BG2 is 256-colour.** `DuelScreen_InitBgCnt` adds `BGCNT_256COLOR` to BG2 (card cells, life points, phase
  letters; char block 1). BG0, BG1 and BG3 stay 4bpp.
- **Shared tiles keep their numbers.** `LoadDuelBgGfx` no longer loads BG2's 4bpp sheets; `ModLoadFieldIconGfx`
  converts them to 8bpp at the same tile numbers (64-byte tiles): card back 0x70 (pose 0) and 0xA0 (pose 3),
  thick pile 0x230, held-zone mark 0x240, digits 0x244, 'LP' 0x280, phase letters 0x282. A 4bpp colour c of
  palette bank b becomes entry 16b + c, so the map writers (and their palette bits, now ignored) are unchanged.
  It also loads the art palette into entries 192-255 (BG palettes 12-15).
- **VRAM.** Char block 1 from 0x06004000. Kept 4bpp: tiles 0x00-0x6F (font, board 0x10-0x58, field background
  0x60-0x6F) and 0x28E-0x3AF (text cells 0x28E-0x2CD, box frame 0x2CE, box text 0x2D7) = 8bpp 0x147-0x1D7.
  8bpp tile 0 is the font's blank tiles 0-1 (checked: all zero), so cleared map cells stay empty.
- **Cells.** `ModFieldCardTile(player, place, id, sideways)` copies the type's frame (`GetCardIconBgTile` order;
  upright or rotated clockwise, which equals the game's pose 3: checked) into the place's 16 tiles and writes the
  miniature (rotated: upright (x, y) -> (31 - y, x)) with halfword read-modify-writes. 26 places (per player 5
  monster, 5 Spell/Trap, the field, the Graveyard top, the banished top), `sCellTiles`: 0x38-0x6F, 0x80-0x9F, 0xB0-0x146,
  0x1D8-0x22F, 0x28E-0x2FF. Called from `DrawZoneTiles` (face up) and `DrawAreaTiles` (Graveyard, banished).
- **Redraws.** The battle scene clears char blocks 1-2 and BG palettes 4-15; every way back (`DuelScreen_Init`,
  from the battle scene, card detail and card list) reloads and redraws.
- **Data.** `build/assets/field_icons.bin` (341,009 bytes: palette, 12 frames, 17x17 per card ID) through
  `data/mod_field_icons_data.s` (`gModFieldIconData`, @rodata). Checked offline by rebuilding cells from it.
- **Sprites.** The OBJ card icons get the art too (`ModAddCardSprite`, `ModAddAffineCardSprite`): the hand
  (`DrawHandCards`), the card menu's card (`CardMenu_DrawCardPreview`, including the turn-over frames), the deck
  reorder row (`DeckReorder_DrawCards` / `DrawSwap`), the change-position, flip and move animations, and the
  unused `DuelPrompt_DrawPositionChoice`.
  - **VRAM.** The OBJ icon tiles 0x80-0x1FF (6 types x 4 poses, 4bpp; the duel's banners fill 0x364 on) now hold
    the six plain icons in pose 0 (4bpp, 0x80 + 16 * type: the fallback) and 9 256-colour 32x32 slots at
    0xE0 + 32 * slot.
  - **Cache.** `gModCardSprites` (EWRAM 0x0202F300; 0x0202D000-0x0202FFFF is unused by the game) keys each slot by
    card and pose and stamps it with `gMain.frameCounter`. A miss takes an empty slot, else the one drawn longest
    ago that was not drawn this frame; with all 9 drawn this frame the sprite falls back to the plain icon.
    `LoadDuelUiGfx` (which every overlay that reuses the OBJ tiles calls on its way out) empties it.
  - **Poses.** The flip uses face poses 0-2 (`gCardFlipTiles` entries 0x1000 / 0x1010 / 0x1020); poses 1-2 are
    narrower perspective frames, so the art is squeezed into their window (per-pose column and row maps from the
    black window's box: x 10-20 / 13-18, y 8-22). Defense is the hardware's affine turn, so no rotated art.
  - **Colours.** OBJ palette 1 (`gCardIconPal`) as entries 16-31 for the frames; the art palette in OBJ palettes
    8-11 (entries 128-191: the duel uses 0-6 and 15). `AddAffineSprite` switches on the exact shape, so the
    256-colour bit is set in the entry it added.
  - **Data.** `field_icons.bin` grew to 359,697 bytes: sprite frames (6 types x 3 poses) at +0x3080, pose maps at
    +0x7880, miniatures at +0x7980. Checked offline by rebuilding sprites from it.
- **Chain list.** `ChainListScreen_Run` clears the OBJ tiles (`UnloadDuelUiGfx`: 2D mapping, 32 tiles a row) and
  fills the header (tile rows 0-1) and the four links' 32x4-tile blocks (rows 2-17). Each link's card now gets a
  256-colour 32x32 icon with its art in tile rows 18-21 (card k at columns 8k-8k+7: `ModChainIconBuild`, frame
  pose 0 plus the 17x17 art at (7, 7)), and `ChainListScreen_DrawRows` draws it (attr0 256-colour, tile
  `ModChainIconTile`) instead of the 4bpp icon it copied into the block's first four columns (`GetCardIconGfx`).
  `ModChainIconsClear` blanks the four icons and reloads the art palette (OBJ palettes 8-11; the overlay loads
  only palettes 0 and 1). `LoadDuelUiGfx` at the end restores the duel's tiles as before.
- **Deck Edit.** The ring of card frames left of the list gets the art too: a 16x32 affine sprite per frame with
  the card's own 15 colours (OBJ palettes 11-15); see [[deck-edit-card-art]]. Data: 224 bytes per card ID after the
  miniatures (from the next multiple of 4).
- Built, not tested in an emulator.

Related: [[added-cards]].
