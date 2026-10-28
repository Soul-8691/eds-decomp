/*
 * mod_field_icons: the duel field's card cells with a miniature of each face-up card's art. Not part of the
 * original ROM.
 *   ModLoadFieldIconGfx   LoadDuelBgGfx (duel_info_bar.c) calls it for BG2's graphics: the 64-colour art palette
 *                         and BG2's shared tiles in 8bpp
 *   ModFieldCardTile      DrawZoneTiles / DrawAreaTiles (duel_field_screen.c): builds a face-up card's cell (its
 *                         type's frame and its art) in the cell's own tiles and returns the first tile for the map
 * BG2 (the card cells, life points and phase letters) is a 256-colour layer (DuelScreen_InitBgCnt). Its tiles keep
 * the numbers they had in 4bpp, now 64-byte tiles from character block 1, so the map writers did not change: the
 * card back 0x70 (pose 0) and 0xA0 (pose 3, sideways), the thick pile 0x230, the held-zone mark 0x240, the digits
 * 0x244, the 'LP' label 0x280 and the phase letters 0x282. Each sheet keeps its 4bpp palette as the 8bpp entries
 * 16 * bank + colour (bank 1 for the card back, pile and mark, 3 for the digits, 4 for the label and letters), so
 * it looks the same. The 4bpp tiles of the other layers stay where they were (the system font and the board
 * 0x00-0x6F; the text cells, box frame and box text 0x28E-0x3AF, which the 8bpp tiles 0x147-0x1D7 cover) and the
 * cells' tiles fill the rest (sCellTiles: 26 cells of 16 tiles, one per face-up place: per player 5 monster, 5
 * Spell/Trap, the field, the Graveyard's and the banished pile's top card). 8bpp tile 0 is the font's blank tiles
 * 0-1, so a cleared cell stays empty.
 * A cell is the frame of the card's type and pose (the card_icons PNGs, entries 32-47: BG palette 2) with the 17x17
 * miniature (entries 192-255: BG palettes 12-15, which the duel leaves free) at x 7-23, y 7-23, or turned clockwise
 * with the frame for a sideways (defense) card. The data is gModFieldIconData (tools/cardicons.py --data,
 * data/mod_field_icons_data.s). The battle scene reuses this VRAM and palette, and DuelScreen_Init redraws all.
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "gba.h"
#include "card_data.h"
#include "util.h"

int GetCardIconBgTile(u16 id);                  /* card_canvas.c */

extern const u8 gCardIconBackGfx[];             /* card back, 4 poses of 4x4 tiles */
extern const u8 gThickPileGfx[];
extern const u8 gHeldZoneMarkGfx[];
extern const u8 gDuelDigitsGfx[];
extern const u8 gLpLabelGfx[];
extern const u8 gPhaseIndicatorGfx[];
extern const u8 gModFieldIconData[];            /* tools/cardicons.py --data */

#define TILE8_ADDR(n)           (VRAM + 0x4000 + (n) * 64)     /* 8bpp tile n of character block 1 */
#define TILE4_SIZE              32
#define TILE8_SIZE              64
#define CELL_TILES              16              /* 4x4 */

/* BG2's shared tiles, at their 4bpp numbers (duel_info_bar.c BG_TILE_*, duel_screen.h FIELD_TILE_*). */
#define TILE_CARD_BACK          0x070
#define TILE_CARD_BACK_SIDEWAYS 0x0A0
#define TILE_THICK_PILE         0x230
#define TILE_HELD_ZONE_MARK     0x240           /* 4 tiles */
#define TILE_DIGITS             0x244           /* 60 tiles */
#define TILE_LP_LABEL           0x280           /* 2 tiles */
#define TILE_PHASE_LETTERS      0x282           /* 12 tiles */

/* gModFieldIconData */
#define DATA_PALETTE            0x0000          /* 64 BGR555 colours */
#define DATA_FRAMES             0x0080          /* 6 types x 2 poses x 1 KiB */
#define DATA_MINIS              0x3080          /* 17x17 bytes per card ID */
#define FRAME_SIZE              (CELL_TILES * TILE8_SIZE)
#define MINI                    17
#define MINI_X                  7
#define MINI_Y                  7
#define ART_PALETTE_ENTRY       192
#define ICON_POSE_SIDEWAYS      0x30            /* GetCardIconBgTile's pose offset (FIELD_TILE_POSE_SIDEWAYS) */
#define ICON_TILE_FIRST         0xB0            /* GetCardIconBgTile: 0xB0 + 0x40 * type */

#define PLACES_PER_PLAYER       13              /* zones 0-10, the Graveyard (11), the banished pile (12) */

/* The first 8bpp tile of each face-up place's cell, [player * PLACES_PER_PLAYER + place]: the free runs of
 * character blocks 1-3 between the 4bpp tiles of the other layers and BG2's shared tiles. */
static const u16 sCellTiles[2 * PLACES_PER_PLAYER] = {
    0x038, 0x048, 0x058,                                            /* 0x038-0x06F */
    0x080, 0x090,                                                   /* 0x080-0x09F (card back poses 1-2) */
    0x0B0, 0x0C0, 0x0D0, 0x0E0, 0x0F0, 0x100, 0x110, 0x120, 0x130,  /* 0x0B0-0x146 */
    0x1D8, 0x1E8, 0x1F8, 0x208, 0x218,                              /* 0x1D8-0x22F */
    0x28E, 0x29E, 0x2AE, 0x2BE, 0x2CE, 0x2DE, 0x2EE,                /* 0x28E-0x2FF */
};

/* `count` 4bpp tiles from src as 8bpp tiles at tile n, colour c of the 4bpp palette becoming entry 16 * bank + c. */
static void Tiles4To8(u32 n, const u8 *src, u32 count, u32 bank)
{
    vu16 *dst = (vu16 *)TILE8_ADDR(n);
    u32 base = bank * 16, i;

    for (i = 0; i < count * TILE4_SIZE; i++) {
        u32 lo = src[i] & 15, hi = src[i] >> 4;

        *dst++ = (lo ? lo + base : 0) | (hi ? hi + base : 0) << 8;
    }
}

void ModLoadFieldIconGfx(void)
{
    CopyDoubleWords((void *)(BG_PLTT + ART_PALETTE_ENTRY * 2), gModFieldIconData + DATA_PALETTE, 64 * 2);
    Tiles4To8(TILE_CARD_BACK, gCardIconBackGfx, CELL_TILES, 1);
    Tiles4To8(TILE_CARD_BACK_SIDEWAYS, gCardIconBackGfx + 3 * CELL_TILES * TILE4_SIZE, CELL_TILES, 1);
    Tiles4To8(TILE_THICK_PILE, gThickPileGfx, CELL_TILES, 1);
    Tiles4To8(TILE_HELD_ZONE_MARK, gHeldZoneMarkGfx, 4, 1);
    Tiles4To8(TILE_DIGITS, gDuelDigitsGfx, 60, 3);
    Tiles4To8(TILE_LP_LABEL, gLpLabelGfx, 2, 4);
    Tiles4To8(TILE_PHASE_LETTERS, gPhaseIndicatorGfx, 12, 4);
}

/* Pixel (x, y) of a cell's 4x4 8bpp tiles at `cell` becomes entry `color` (VRAM takes halfword writes only). */
static void PutCellPixel(u8 *cell, u32 x, u32 y, u32 color)
{
    u32 offset = ((y >> 3) * 4 + (x >> 3)) * TILE8_SIZE + (y & 7) * 8 + (x & 7);
    vu16 *p = (vu16 *)(cell + (offset & ~1));

    if (offset & 1)
        *p = (*p & 0x00FF) | color << 8;
    else
        *p = (*p & 0xFF00) | color;
}

/* Build the cell of face-up card `id` at place `place` (a zone 0-10, 11 the Graveyard, 12 the banished pile) of
 * `player`, sideways for a defense-position card, and return its first tile for FillTileBlock4x4. */
u16 ModFieldCardTile(u32 player, u32 place, u16 id, u32 sideways)
{
    u32 tile = sCellTiles[(player & 1) * PLACES_PER_PLAYER + place];
    u32 type = (GetCardIconBgTile(id) - ICON_TILE_FIRST) / 0x40;
    u8 *cell = (u8 *)TILE8_ADDR(tile);
    const u8 *mini;
    u32 i, j;

    sideways = sideways ? 1 : 0;
    CopyDoubleWords(cell, gModFieldIconData + DATA_FRAMES + (type * 2 + sideways) * FRAME_SIZE, FRAME_SIZE);
    id &= CARD_ID_MASK;
    if (id == 0 || id >= CARD_ID_COUNT)
        return tile;
    mini = gModFieldIconData + DATA_MINIS + id * (MINI * MINI);
    for (j = 0; j < MINI; j++) {
        for (i = 0; i < MINI; i++) {
            u32 color = mini[j * MINI + i];

            if (color == 0)
                continue;
            if (sideways)
                PutCellPixel(cell, 31 - (MINI_Y + j), MINI_X + i, color);   /* turned clockwise */
            else
                PutCellPixel(cell, MINI_X + i, MINI_Y + j, color);
        }
    }
    return tile;
}
