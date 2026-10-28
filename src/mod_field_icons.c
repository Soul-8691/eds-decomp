/*
 * mod_field_icons: the duel field's card cells with a miniature of each face-up card's art. Not part of the
 * original ROM.
 *   ModLoadFieldIconGfx   LoadDuelBgGfx (duel_info_bar.c) calls it for BG2's graphics: the 64-colour art palette
 *                         and BG2's shared tiles in 8bpp
 *   ModFieldCardTile      DrawZoneTiles / DrawAreaTiles (duel_field_screen.c): builds a face-up card's cell (its
 *                         type's frame and its art) in the cell's own tiles and returns the first tile for the map
 *   ModLoadCardSpriteGfx  LoadDuelUiGfx: the sprites' art palette and fallback icons, and an empty slot cache
 *   ModAddCardSprite, ModAddAffineCardSprite  a face-up card's 32x32 sprite with its art: the hand
 *                         (DrawHandCards), the card menu's card (campaign.c), the deck reorder row (duel_cursor.c) and
 *                         the change-position, flip and move animations (duel_card_anim.c)
 *   ModChainIconsClear, ModChainIconBuild, ModChainIconTile  the chain list's card icons with their art
 *                         (ChainListScreen_Run, ChainListScreen_DrawRows)
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
 * The sprites: the OBJ icon tiles 0x80-0x1FF (six types x four poses, 4bpp) become the six plain icons in pose 0
 * (4bpp, 0x80 + 16 * type, palette 1: the fallback) and SPRITE_SLOTS 256-colour 32x32 slots from 0xE0 (32 tiles of
 * 32 bytes each). A face-up card's sprite takes the slot that already holds that card in that pose, else the one
 * drawn longest ago that was not drawn this frame (gModCardSprites, by gMain.frameCounter), built from the sprite
 * frame of its type and pose (pose 0 upright, 1-2 the flip's narrower frames: the art squeezed into their window)
 * with the art in OBJ palettes 8-11 (entries 128-191; 0-6 and 15 are the duel's). With every slot drawn this frame,
 * it falls back to the plain icon. LoadDuelUiGfx (which every overlay that reuses the OBJ tiles calls on its way
 * out) empties the cache.
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "card_data.h"
#include "sprite.h"
#include "util.h"

int GetCardIconBgTile(u16 id);                  /* card_canvas.c */
int GetCardIconObjTile(u16 id);                 /* card_canvas.c: 0x80 + 0x40 * type */
extern const u8 gCardIconNormalGfx[], gCardIconEffectGfx[], gCardIconFusionGfx[], gCardIconRitualGfx[],
    gCardIconMagicGfx[], gCardIconTrapGfx[];    /* 4 poses of 4x4 4bpp tiles each */

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
#define DATA_OBJ_FRAMES         0x3080          /* 6 types x 3 poses x 1 KiB */
#define DATA_POSE_MAPS          0x7880          /* 3 poses x POSE_MAP_SIZE */
#define DATA_MINIS              0x7980          /* 17x17 bytes per card ID */
#define POSE_MAP_SIZE           80              /* x0, width, y0, height, the art column of each column (32), the
                                                   art row of each row (32) */
#define SPRITE_POSES            3
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

/* ---- The card sprites ---- */

#define OBJ_TILE8_ADDR(n)       (OBJ_VRAM0 + (n) * 32)  /* OBJ tile n (32-byte units, 1D mapping) */
#define OBJ_TILE_FALLBACK       0x080           /* + 16 * type: the plain 4bpp icons, pose 0 */
#define OBJ_TILE_SLOTS          0x0E0           /* + 32 * slot: the 256-colour slots, up to 0x1FF */
#define SPRITE_SLOTS            9
#define OBJ_ART_PALETTE_ENTRY   128             /* OBJ palettes 8-11 */
#define OBJ_ART_SHIFT           (ART_PALETTE_ENTRY - OBJ_ART_PALETTE_ENTRY)
#define OBJ_PALETTE_ICON        0x1000          /* attr2: OBJ palette 1, the fallback icons' */
#define ATTR0_256COLOR          0x2000

/* The slot cache (EWRAM 0x0202F300, symbols.ld). */
struct ModCardSpriteSlot {
    u16 id;                         /* 0: empty */
    u16 frame;                      /* gMain.frameCounter when last drawn */
    u8 pose;
    u8 pad[3];
};
extern struct ModCardSpriteSlot gModCardSprites[SPRITE_SLOTS];

static const u8 *const sFallbackIcons[6] = {
    gCardIconNormalGfx, gCardIconEffectGfx, gCardIconFusionGfx, gCardIconRitualGfx, gCardIconMagicGfx,
    gCardIconTrapGfx,
};

void ModLoadCardSpriteGfx(void)
{
    u32 i;

    CopyDoubleWords((void *)(OBJ_PLTT + OBJ_ART_PALETTE_ENTRY * 2), gModFieldIconData + DATA_PALETTE, 64 * 2);
    for (i = 0; i < 6; i++)
        CopyDoubleWords((void *)OBJ_TILE8_ADDR(OBJ_TILE_FALLBACK + i * CELL_TILES), sFallbackIcons[i],
                        CELL_TILES * TILE4_SIZE);
    for (i = 0; i < SPRITE_SLOTS; i++)
        gModCardSprites[i].id = 0;
}

/* Build card `id`'s sprite in pose `pose` into slot `slot`. */
static void BuildCardSprite(u32 slot, u16 id, u32 type, u32 pose)
{
    u8 *dst = (u8 *)OBJ_TILE8_ADDR(OBJ_TILE_SLOTS + slot * 32);
    const u8 *map = gModFieldIconData + DATA_POSE_MAPS + pose * POSE_MAP_SIZE;
    const u8 *mini;
    u32 x0 = map[0], w = map[1], y0 = map[2], h = map[3], r, c;

    CopyDoubleWords(dst, gModFieldIconData + DATA_OBJ_FRAMES + (type * SPRITE_POSES + pose) * FRAME_SIZE,
                    FRAME_SIZE);
    if (id >= CARD_ID_COUNT)
        return;
    mini = gModFieldIconData + DATA_MINIS + id * (MINI * MINI);
    for (r = 0; r < h; r++) {
        for (c = 0; c < w; c++) {
            u32 color = mini[map[4 + 32 + r] * MINI + map[4 + c]];

            if (color != 0)
                PutCellPixel(dst, x0 + c, y0 + r, color - OBJ_ART_SHIFT);
        }
    }
}

/* The attr2 tile of face-up card `id`'s sprite in pose `pose` (0-2), with *is8bpp set for a 256-colour slot, else
 * the plain 4bpp icon (pose 0, with its palette). */
static u16 CardSpriteTile(u16 id, u32 pose, u32 *is8bpp)
{
    u32 type = (GetCardIconObjTile(id) - 0x80) / 0x40;
    u16 now = gMain.frameCounter;
    s32 best = -1;
    u32 i;

    id &= CARD_ID_MASK;
    if (pose >= SPRITE_POSES)
        pose = 0;
    for (i = 0; i < SPRITE_SLOTS; i++) {
        struct ModCardSpriteSlot *s = &gModCardSprites[i];

        if (s->id == id && s->pose == pose && id != 0) {
            s->frame = now;
            *is8bpp = 1;
            return OBJ_TILE_SLOTS + i * 32;
        }
        if (s->id == 0) {
            if (best < 0 || gModCardSprites[best].id != 0)
                best = i;                                   /* an empty slot first */
        } else if (s->frame != now && (best < 0 || (gModCardSprites[best].id != 0
                   && (u16)(now - s->frame) > (u16)(now - gModCardSprites[best].frame)))) {
            best = i;                                       /* else the one drawn longest ago */
        }
    }
    if (best < 0 || id == 0) {
        *is8bpp = 0;
        return (OBJ_TILE_FALLBACK + type * CELL_TILES) | OBJ_PALETTE_ICON;
    }
    BuildCardSprite(best, id, type, pose);
    gModCardSprites[best].id = id;
    gModCardSprites[best].pose = pose;
    gModCardSprites[best].frame = now;
    *is8bpp = 1;
    return OBJ_TILE_SLOTS + best * 32;
}

/* AddSprite for face-up card `id` (pose 0); `attr2` holds the other bits (priority). */
void ModAddCardSprite(u32 yx, u16 shape, u16 id, u16 attr2)
{
    u32 is8bpp;
    u16 tile = CardSpriteTile(id, 0, &is8bpp);

    AddSprite(yx, is8bpp ? shape | ATTR0_256COLOR : shape, tile | attr2);
}

/* AddAffineSprite for face-up card `id` in pose `pose` (0-2: the flip's frames). */
void ModAddAffineCardSprite(u32 yx, u16 shape, u16 id, u32 pose, u16 attr2, u32 scaleAngle)
{
    u32 is8bpp;
    u16 tile = CardSpriteTile(id, pose, &is8bpp);
    u8 count = gMain.oamCount;

    AddAffineSprite(yx, shape, tile | attr2, scaleAngle);
    /* AddAffineSprite switches on the exact shape, so the 256-colour bit goes into the entry it added */
    if (is8bpp && gMain.oamCount != count)
        gMain.oamBuffer[count].attr0 |= ATTR0_256COLOR;
}

/* ---- The chain list ---- */

/* The chain-list overlay (ChainListScreen_Run, duel_card_actions.c) clears the OBJ tiles and uses 2D mapping (32
 * tiles a row): the header in tile rows 0-1, the four links' 32x4-tile blocks in rows 2-17. The links' cards get
 * 256-colour 32x32 icons with their art in tile rows 18-21, card k at columns 8k-8k+7 (an 8bpp tile is two tile
 * columns), drawn instead of the 4bpp icon it copied into each block. The art palette is OBJ palettes 8-11 (from
 * LoadDuelUiGfx; loaded again here), the frame's colours OBJ palette 1 (the overlay loads gCardIconPal there). */
#define CHAIN_ICON_TILE         (18 * 32)       /* + 8 * row */
#define CHAIN_ICON_BLOCK_SIZE   (4 * 32 * 32)   /* tile rows 18-21 */

/* The attr2 tile of chain row `row`'s card icon. */
u16 ModChainIconTile(u32 row)
{
    return CHAIN_ICON_TILE + row * 8;
}

/* Empty the four icons (rows without a link stay blank) and load the art palette. */
void ModChainIconsClear(void)
{
    CopyDoubleWords((void *)(OBJ_PLTT + OBJ_ART_PALETTE_ENTRY * 2), gModFieldIconData + DATA_PALETTE, 64 * 2);
    MemClear16((void *)OBJ_TILE8_ADDR(CHAIN_ICON_TILE), CHAIN_ICON_BLOCK_SIZE);
}

/* Pixel (x, y) of a 2D-mapped 32x32 8bpp sprite from OBJ tile `tile` becomes entry `color`. */
static void PutSprite2dPixel(u32 tile, u32 x, u32 y, u32 color)
{
    u32 offset = (tile + (y >> 3) * 32 + (x >> 3) * 2) * 32 + (y & 7) * 8 + (x & 7);
    vu16 *p = (vu16 *)(OBJ_VRAM0 + (offset & ~1));

    if (offset & 1)
        *p = (*p & 0x00FF) | color << 8;
    else
        *p = (*p & 0xFF00) | color;
}

/* Build chain row `row`'s icon for card `id`: its type's sprite frame (pose 0) and its art. */
void ModChainIconBuild(u32 row, u16 id)
{
    u32 tile = ModChainIconTile(row);
    u32 type = (GetCardIconObjTile(id) - 0x80) / 0x40;
    const u8 *frame = gModFieldIconData + DATA_OBJ_FRAMES + type * SPRITE_POSES * FRAME_SIZE;
    const u8 *mini;
    u32 r, i, j;

    for (r = 0; r < 4; r++)     /* a tile row of the frame: 4 tiles of 64 bytes */
        CopyDoubleWords((void *)OBJ_TILE8_ADDR(tile + r * 32), frame + r * 4 * TILE8_SIZE, 4 * TILE8_SIZE);
    id &= CARD_ID_MASK;
    if (id == 0 || id >= CARD_ID_COUNT)
        return;
    mini = gModFieldIconData + DATA_MINIS + id * (MINI * MINI);
    for (j = 0; j < MINI; j++) {
        for (i = 0; i < MINI; i++) {
            u32 color = mini[j * MINI + i];

            if (color != 0)
                PutSprite2dPixel(tile, MINI_X + i, MINI_Y + j, color - OBJ_ART_SHIFT);
        }
    }
}
