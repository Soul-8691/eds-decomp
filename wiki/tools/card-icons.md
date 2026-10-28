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
- **Not changed.** The OBJ card icons (hand cards, card movement animations) keep the plain type icon.
- Built, not tested in an emulator.

Related: [[added-cards]].
