/*
 * mod_title: the title screen says "Yak's YGO Hack" instead of the "Yu-Gi-Oh! The Eternal Duelist Soul" logo. Not
 * part of the original ROM.
 *   ModTitle_Load  Title_LoadGraphics, after the logo: clears BG0's map (the logo is not shown) and builds the text,
 *                  in the main menu's font as its highlighted labels are drawn (BuildMenuLabelEx, src/mod_recipes.c:
 *                  the letters of the menu sheet gMainMenuObjGfx, and drawn Y, G, O, H and ' it lacks), into OBJ tiles
 *                  0-63 (2D mapping, as the title's; its own sprites use tiles from 0x200) with the menu's colours in
 *                  OBJ palette 1.
 *   ModTitle_Draw  Title_DrawMenu, every frame: the highlighted half as four 32 x 16 affine sprites at twice the size
 *                  (scale 0x80), 208 pixels wide like the old logo, centred at x 120, from y 40.
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "util.h"
#include "sprite.h"

extern const u8 gMainMenuObjPal[];

/* src/mod_recipes.c */
struct LabelChunk {
    u8 item;
    u8 x0;
    u8 x1;
};
void BuildMenuLabelEx(const struct LabelChunk *chunks, int count, u16 *dst, int icons);
#define LABEL_DRAWN     0x80

/* Drawn letters (sDrawnGlyphs, mod_recipes.c): x1 is the cell width - 1 (glyph and a 1-pixel gap). */
#define DRAWN_Y         { LABEL_DRAWN | 0, 0, 8 }
#define DRAWN_G         { LABEL_DRAWN | 1, 0, 8 }
#define DRAWN_O         { LABEL_DRAWN | 2, 0, 8 }
#define DRAWN_H         { LABEL_DRAWN | 3, 0, 8 }
#define DRAWN_QUOTE     { LABEL_DRAWN | 4, 0, 2 }
#define DRAWN_SPACE     { LABEL_DRAWN | 5, 0, 3 }

/* "Yak's YGO Hack": a, c and k of the Campaign / Deck labels, s of Password. */
static const struct LabelChunk sTitleChunks[] = {
    DRAWN_Y, { 0, 40, 47 }, { 2, 56, 63 }, DRAWN_QUOTE, { 6, 50, 56 }, DRAWN_SPACE,
    DRAWN_Y, DRAWN_G, DRAWN_O, DRAWN_SPACE,
    DRAWN_H, { 0, 40, 47 }, { 2, 48, 54 }, { 2, 56, 62 },
};

#define TITLE_TILE      0x000           /* the label: plain at tiles 0-15, highlighted at 16-31 (two rows) */
#define TITLE_PALETTE   1
#define TITLE_X         (-8)            /* left edge of the doubled 256-pixel strip (text centred at 120) */
#define TITLE_Y         40              /* top of the doubled text */

void ModTitle_Load(void)
{
    MemClear16(gMain.bgMapBuffer[0], sizeof(gMain.bgMapBuffer[0]));    /* the old logo (BG0) */
    MemCopy16((void *)(OBJ_PLTT + TITLE_PALETTE * 0x20), gMainMenuObjPal, 0x20);
    BuildMenuLabelEx(sTitleChunks, ARRAY_COUNT(sTitleChunks), (u16 *)(OBJ_VRAM0 + TITLE_TILE * 0x20), 0);
}

void ModTitle_Draw(void)
{
    int k;

    /* A doubled 32 x 16 piece covers 64 x 32 pixels about its centre (x + 16, y + 8): piece k at x = left + 64k + 16. */
    for (k = 0; k < 4; k++)
        AddAffineSprite(((TITLE_Y + 8) << 16) | ((TITLE_X + 64 * k + 16) & 0x1FF), SPRITE_SHAPE_32x16,
                        (TITLE_TILE + 16 + k * 4) | OAM_ATTR2_PALETTE(TITLE_PALETTE), 0x80 << 16);
}
