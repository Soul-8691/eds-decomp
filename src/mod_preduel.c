/*
 * mod_preduel: a picture before the duel for the format opponents that have one (Yugi-Kaiba: yugi-kaiba.png), with no
 * dialogue. Not part of the original ROM.
 *   CB_ModPreDuelImage  Campaign_SelectOpponent (campaign_steps.c) sub-step 6, where these opponents used to skip
 *                       the pre-duel dialogue: returns 1 at once for an opponent with no picture and for the later
 *                       duels of a match. Otherwise the bustup screen's layout in Mode 4, page 0: the picture in rows
 *                       0-95 over the empty dialogue box (gDialogueBoxGfx[1], as Bustup_LoadScene), a fade in from
 *                       black, A, B or START, a fade out to black, then 1. No text, no BGM change.
 * Palette: the box palette's colours 0-31 (the box uses 16-31; its fill, 31, made black as the scenes have it), then the picture's 224 colours, 32-255. The picture is
 * built by tools/preduel_image.py into data/mod_preduel_data.s (layout in the tool's docstring). Its buffers are the
 * bustup's (gBustupBitmapBuffer, gBustupBoxBitmap), free while no dialogue runs.
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "bg.h"                 /* LZSSDecompress */
#include "util.h"               /* MemCopy16 */
#include "palette.h"            /* SetBrightnessBlack, FadeFromBlack, FadeToBlack */
#include "bustup.h"             /* gBustupBitmapBuffer, gBustupBoxBitmap */
#include "sound.h"              /* PlaySE */
#include "constants/sound.h"
#include "constants/game.h"     /* DUELIST_TRISTAN */

extern const u16 *const gDialogueBoxGfx[];      /* 0x08139F5C: LZSS dialogue box bitmaps */
extern const u16 gDialogueBoxPalette[];         /* 0x0874E104: box palette (the box uses colours 16-31) */
extern const u8 gModPreDuelYugiKaiba[];         /* data/mod_preduel_data.s */

int ModOpponentFormat(u16 duelist);             /* src/mod_opponents.c */

#define IMAGE_FIRST_COLOR   32
#define IMAGE_COLORS        224
#define IMAGE_LZ_OFFSET     (IMAGE_COLORS * 2)
#define IMAGE_BYTES         (96 * 240)          /* rows 0-95 */
#define BOX_BYTES           (64 * 240)          /* rows 96-159 */
#define FADE_STEP           2
#define BOX_FILL_COLOR      31                  /* the box's backdrop */

enum {
    PREDUEL_LOAD,
    PREDUEL_FADE_IN,
    PREDUEL_WAIT,
    PREDUEL_FADE_OUT,
};

static const u8 *PreDuelImage(u16 duelist)
{
    if (ModOpponentFormat(duelist) < 0)
        return NULL;
    switch (duelist) {
    case DUELIST_TRISTAN:               /* Yugi-Kaiba */
        return gModPreDuelYugiKaiba;
    }
    return NULL;
}

static void LzDecompress(const u8 *blob, void *dest)
{
    LZSSDecompress((u8 *)blob + 4, dest, blob[0] | blob[1] << 8 | blob[2] << 16 | blob[3] << 24);
}

static void PreDuelLoad(const u8 *image)
{
    SetBrightnessBlack();       /* first, and no forced blank (it shows white): the old screen stays, darkened */
    LzDecompress(image + IMAGE_LZ_OFFSET, gBustupBitmapBuffer);
    CpuSet(gBustupBitmapBuffer, (void *)VRAM, IMAGE_BYTES / 2);
    LzDecompress((const u8 *)gDialogueBoxGfx[1], gBustupBoxBitmap);
    CpuSet(gBustupBoxBitmap, (void *)(VRAM + IMAGE_BYTES), BOX_BYTES / 2);
    MemCopy16((void *)BG_PLTT, gDialogueBoxPalette, IMAGE_FIRST_COLOR * 2);
    ((u16 *)BG_PLTT)[BOX_FILL_COLOR] = 0;  /* black, as in the scene sets' palettes (the box palette's is grey) */
    MemCopy16((void *)(BG_PLTT + IMAGE_FIRST_COLOR * 2), image, IMAGE_COLORS * 2);
    REG_BG2PA = 0x100;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 0x100;
    REG_BG2X = 0;
    REG_BG2Y = 0;
    REG_DISPCNT = DISPCNT_MODE_4 | DISPCNT_BG2_ON;
}

u16 CB_ModPreDuelImage(void)
{
    const u8 *image = PreDuelImage(gMain.opponent);

    if (image == NULL || gMain.matchDuelCount != 0)
        return 1;
    switch (gMain.seqIndex1) {
    case PREDUEL_LOAD:
        PreDuelLoad(image);
        gMain.seqIndex1++;
        break;
    case PREDUEL_FADE_IN:
        if (FadeFromBlack(FADE_STEP))
            gMain.seqIndex1++;
        break;
    case PREDUEL_WAIT:
        if (gMain.newKeys & (A_BUTTON | B_BUTTON | START_BUTTON)) {
            PlaySE(SE_CONFIRM);
            gMain.seqIndex1++;
        }
        break;
    case PREDUEL_FADE_OUT:
        if (FadeToBlack(FADE_STEP))
            return 1;
        break;
    }
    return 0;
}
