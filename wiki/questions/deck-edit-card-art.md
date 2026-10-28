---
title: Card Art Miniatures in Deck Edit
type: question
status: draft
confidence: high
sources: [rom-analysis]
updated: 2026-10-28
---

# Card art miniatures in Deck Edit

Answered: yes, over the ring of card frames left of the list. Built in `src/mod_field_icons.c` (see
[[card-icons]]); not tested in an emulator.

## What Deck Edit shows
- **Selected card:** its full 72x80 art on BG3 (256-colour, char block 2), two pages at `0x06008000 + page * 0x1680`
  (`UnpackCardArt8bpp`, gfx_util.c), BG palette entries 128-191 (page 0) or 192-255 (page 1).
- **The ring:** one frame per visible list row (two above, the cursor row, two below), each a 32x32 affine sprite
  (`DeckEdit_DrawFrameSlots`: `gCardFrameSprites[GetCardFrameIndex(id)]`, OamList layer 5, x -3). Checked in mGBA's
  OAM viewer (user screenshots): OAM entries 33-37 at rest (35-39 mid-scroll), x 509 (= -3), the cursor row at y 40
  with matrix 1.01, the others smaller (y 90: 1.62, so about 62%), palette 5 or 6. Every type's frame has its window
  at x 8-22, y 7-22 (as the duel icons).

## Video memory and palettes (`DeckEdit_InitListView`)
- **BG:** BG0-2 4bpp (char block 0), BG3 8bpp; BG palettes 0-7 and the two art pages: no free BG palette.
- **OBJ:** 2D mapping; four 8 KiB sheets cover the 32x32-tile grid: `gDeckEditObjTiles` (columns 0-15, rows 0-15),
  `gDeckEditCardStackObjTiles` (16-31, 0-15), `gDeckEditCardIconObjTiles` (0-15, 16-31, holding the Normal frame at
  (4, 20)), `gDeckEditCardFrameObjTiles` (16-31, 16-31, the other frames). Run-time writes: rows 2-3
  (`0x06010800`, the selected panel), row 11 column 31 on (`0x06012FE0`, the name's index letters).
- **OBJ palettes:** `gDeckEditObjPal` fills all 16; 11-15 are all zero.

> [!warning] Correction
> An earlier version of this page read each sheet one `.incbin` too early (in `data/rodata_08087FD0.s` a label's data
> is the `.incbin` after it), so its sheet renders and blank-tile map were of the wrong sheets. The facts above use
> the right offsets (`gDeckEditObjTiles` = gfx bank A + 0xAEDA8, then + 0x2000 each).

## The solution
- **Art piece:** a 16x32 affine sprite at the frame's x + 8 with the frame's attributes and matrix: the same centre,
  so it scales and moves with the frame. Layer 4 (in front of the frames).
- **Colours:** each card's own 15 colours (`tools/cardicons.py --data`, quantised from the art's centre 72x76 area
  averaged to the 15x16 window), the k-th drawn frame in OBJ palette 11 + k.
- **Tiles:** five 2x4-tile blocks blank in all four sheets and outside every frame template and run-time write:
  (8, 9), (28, 10), (22, 12), (8, 20), (10, 23). Rewritten every frame with the palette, so reloads need nothing.
- **Cards:** `DeckEdit_InitFrameSlot` and `DeckEdit_ScrollFrameSlots` record each ring slot's card
  (`gModDeckRing`, EWRAM 0x0202F360).
- **Other screens:** the list filter loads other sheets into the top half, so `ModDeckRingBegin` first compares a tile
  of each top sheet with Deck Edit's and draws no art otherwise.

## Vanilla bug seen while testing (2026-10-28): fixed
After Card View and back, the start of the list row above the cursor is covered by a dark block of about two tiles
(screenshot eds-6.png). The user confirmed it happens in the original game too, so it is not the mod's. It is on the
backgrounds, not a sprite (OAM checked).
- **Cause (found in the code):** `LoadDigitTiles` clears and draws 13 tiles from BG tile 0x300 (0x300-0x30C), and the
  seven list-row name buffers started at tile 0x30C (`DeckEdit_GetListRowVram` / `GetListRowTile`: VRAM + 0x6180,
  21 tiles each). `DeckEdit_InitListView` loads the digits last, after drawing the rows, so the name in buffer 0
  lost its first tile. Card View's return runs `DeckEdit_ResetListView` (listRowRing = 0), which puts the row above
  the cursor in buffer 0. Conversely, a later name drawn into buffer 0 overwrote the 13th digit glyph.
- **Fix (deck_edit_panel.c):** the buffers start one tile later, at tile 0x30D (VRAM + 0x61A0), and end at 0x39F;
  only those two functions address them, and no Deck Edit map uses tiles past 0x30C otherwise (frame map: tiles up
  to 0xFF). Built; not tested in an emulator.

