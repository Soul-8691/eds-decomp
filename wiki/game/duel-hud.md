---
title: Duel screen life points, phase panel and Extra Monster Zones (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-06
---
# Duel screen life points, phase panel and Extra Monster Zones (mod)

Added on 2026-11-06. None of this is in the original game. Built (`make demo`); **not tested in an emulator**.

## The original layout
- The board picture (`gDuelFieldImage`, 0x0867BB7C, loaded on BG1 by `DuelScreen_Init` with
  `LoadBgImage4bppMap1`) has the two players' rows of zones, and between them (rows 18-21) a YOU box (columns 4-12)
  and an OPPONENT box (columns 17-25). Its "OPPONENT" label cut into the bottom edges of the opponent's last two
  monster zones (row 17).
- Each box held that player's life points and phase letters as BG2 cells: `DrawLifePoints` (row 19 column 7 and
  row 20 column 20, `DrawBgNumber`) and `DrawPhaseIndicator` (row 20 columns 6-11 and row 19 columns 19-24), plus
  two "LP" label cells written by `DuelScreen_Init` (row 19 column 5, row 20 column 18). BG2 scrolls with the board.
- The pack format (`LoadBgImage4bppMap1`): u16 colour count, palette at +8; u16 tile count (8-byte header), the
  tiles; u16 cell count (8-byte header), the cells as u16 position (column bits 0-5, row bits 8-15) and u16 tile.

## The new layout
- **Board** (`tools/duel_board.py` -> `build/assets/duel_board.bin`, `gModDuelFieldImage` in
  `data/rodata_08087FD0.s`; `DuelScreen_Init` loads it instead): the same tiles, new cells.
  - The boxes are gone (rows 18-21, columns 4-25 cleared) and the opponent's zone bottoms are whole again.
  - **Extra Monster Zones:** empty zone frames at columns 5-8 and 21-24 (the edges, above the first and fifth Main
    Monster Zones), drawn with the zone frame tiles. They are only drawn: nothing can be placed there (no Link
    support yet, and the engine has 5 monster zones per player).
  - **Phase panel:** a 10x4 box at columns 10-19 from the old boxes' corner, edge and striped fill tiles.
- **Sprites** (`ModDrawDuelHud`, `src/mod_duel_hud.c`, far code; called each frame from `ModDrawCounterBadges`,
  i.e. `DrawFieldOverlay`, while the field shows):
  - **Life points**, fixed to the screen: "PLAYER" and your LP at the bottom left (x 4, y 128-143, above the info
    bar), "OPPONENT" and theirs at the top right (right edge x 236, y 0-15). White bold 8x8 font
    (`RenderBoldGlyphTile`) and the game's LP digits (`gDuelDigitsGfx`, black), on a transparent background. They read
    `gDuelPlayers[].lifePoints` every frame, so they count along with an LP change.
  - **Phase panel contents**, scrolling with the board (from `GetAreaX/Y` of player 0's monster zone 2): the turn
    player's name ("PLAYER" / "OPPONENT", dark navy) on row 19 and the six phase letters D S M B M E
    (`gPhaseIndicatorGfx`) on row 20, the current phase highlighted. Not drawn while the panel is scrolled off
    screen.
  - OBJ tiles 0x3C4-0x3E9 (below the Spell Counter badges' 0x3F0), OBJ palettes 13 (the phase letters'; entry 15
    the turn name) and 14 (the digits', which keep their black; entry 14 the labels' white). Written every frame, as the chain list and the
    battle screens reuse OBJ VRAM. Sprite priority 1, as the badges.
- **Original functions changed** (size-limited `.text`; all three got smaller): `DrawLifePoints` and
  `DrawPhaseIndicator` (duel_info_bar.c) draw nothing; `DuelScreen_Init` (duel_field_screen.c) loads
  `gModDuelFieldImage` and no longer writes the "LP" label cells; `DuelCmd_ChangeLifePoints` (duel_cmd_moves.c)
  shows the +/- amount by the new LP spots (x `player * 0x88 + 8`, y `0x6C - player * 0x5A`: above yours, under
  the opponent's). Demo `.text` now ends at 0x0807F34C.

> [!question] To check in an emulator
> Whether the corner LP overlap other sprites (the hand strips, the phase banners) or the text boxes (the sprites
> are priority 1, behind a priority-0 BG), and whether OBJ palettes 13-14 are free on every duel screen.

Related: [[xyz]], [[added-cards]].
