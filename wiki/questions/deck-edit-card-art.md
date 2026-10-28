---
title: Card Art Miniatures in Deck Edit (feasibility)
type: question
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-28
---

# Card art miniatures in Deck Edit

Could Deck Edit show the 17x17 art miniatures of [[card-icons]], as the duel does? Answer: only with a rework of
Deck Edit's sprite memory; the background has no room at all.

## What Deck Edit shows
- **Selected card:** its full 72x80 art on BG3 (256-colour, char block 2), two pages at `0x06008000 + page * 0x1680`
  (`LoadCardArt8bpp` / `UnpackCardArt8bpp`, gfx_util.c), palette entries 128-191 (page 0) or 192-255 (page 1).
- **The cards around the cursor:** a ring of up to 6 "frame slots" (`DeckEdit_InitFrameSlot`, `DeckEdit_ScrollFrameSlots`,
  `DeckEdit_DrawFrameSlots`, deck_edit_widgets.c): affine sprite groups `gCardFrameSprites[GetCardFrameIndex(id)]`
  from the 4bpp sheet `gDeckEditCardFrameObjTiles`, scaled for the ring's perspective. Type-coloured frames, no art.

## Video memory and palettes (`DeckEdit_InitListView`, deck_edit_view.c)
- **BG:** BG0-2 4bpp (char block 0), BG3 8bpp (char block 2); BG palettes 0-7 loaded (stat icons, `gDeckEditBgPals*`),
  8-15 the two art pages. No free BG palette.
- **OBJ:** 2D mapping. Four 8 KiB sheets cover the whole 32x32-tile grid: `gDeckEditObjTiles` (columns 0-15, rows
  0-15), `gDeckEditCardStackObjTiles` (16-31, 0-15), `gDeckEditCardIconObjTiles` (0-15, 16-31),
  `gDeckEditCardFrameObjTiles` (16-31, 16-31). Their blank tiles (51, 66, 76 and 71 of 256, checked in the asset) are
  scattered: no 8x4-tile blank block (a 256-colour 32x32 sprite), few 4x4 ones. At run time Deck Edit also writes
  rows 2-3 (`0x06010800`, the selected panel) and `0x06012FE0` (the selected card's name).
- **OBJ palettes:** `gDeckEditObjPal` fills all 16; banks 11-15 are all zero (checked), so 80 entries are free.

## Ways it could be done
- **Repack the sprite sheets** to free a block, then draw a 256-colour miniature sprite over each ring frame, sharing
  the frame group's affine matrix and offset. Needs the ring code (partly nonmatching) and the sheet layout changed.
- **4bpp miniatures with one palette each:** each ring card quantised to 15 colours in one of the free palettes
  11-15 (five at once), as 4x4-tile 4bpp sprites: half the memory, still needs freed 4x4 blocks.
