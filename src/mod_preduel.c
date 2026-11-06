/*
 * mod_preduel: a picture before the duel for the format opponents that have one (Yugi-Kaiba: yugi-kaiba.png), with no
 * dialogue. Not part of the original ROM.
 *   CB_ModPreDuelImage  Campaign_SelectOpponent (campaign_steps.c) sub-step 6, where these opponents used to skip
 *                       the pre-duel dialogue: returns 1 at once for an opponent with no picture and for the later
 *                       duels of a match. Otherwise the bustup screen's layout in Mode 4, page 0: the picture in rows
 *                       0-95 over the empty dialogue box (gDialogueBoxGfx[1], as Bustup_LoadScene), a fade in from
 *                       black, A, B or START, a fade out to black, then 1. No dialogue, no BGM change.
 * QR code: a picture's data may hold one (Yugi-Kaiba's: yugi-kaiba.svg, formatlibrary.com's page for the format). It
 * goes on a white square at the left of the box, 2 px a module, with a note at its right (sStrQrNote, in the bustup's
 * 8x10 Latin font, DrawGlyph8bpp) to trust it only if it leads to formatlibrary.com. Both are drawn into the decoded
 * box bitmap before it is copied to VRAM.
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
#include "bustup.h"             /* DrawGlyph8bpp */

extern const u16 *const gDialogueBoxGfx[];      /* 0x08139F5C: LZSS dialogue box bitmaps */
extern const u16 gDialogueBoxPalette[];         /* 0x0874E104: box palette (the box uses colours 16-31) */
extern const u8 gModPreDuelYugiKaiba[];         /* data/mod_preduel_data.s */

int ModOpponentFormat(u16 duelist);             /* src/mod_opponents.c */

#define IMAGE_FIRST_COLOR   32
#define IMAGE_COLORS        224
#define QR_SIZE_OFFSET      (IMAGE_COLORS * 2)
#define QR_ROWS_OFFSET      (QR_SIZE_OFFSET + 4)
#define QR_ROWS             32
#define IMAGE_LZ_OFFSET     (QR_ROWS_OFFSET + QR_ROWS * 4)
#define BOX_WIDTH           240
#define BOX_HEIGHT          64
#define QR_X                6                   /* the white square: x 6-69, the box's full height */
#define QR_MODULE           2
#define NOTE_X              76
#define NOTE_Y              3
#define NOTE_LINE           12
#define NOTE_ADVANCE        5                   /* the Latin font's advance, as Calendar_DrawStringShadow */
#define COLOR_WHITE         1                   /* BG colours 1-15 are free here: the box uses 16-31 */
#define COLOR_DARK          31                  /* the box fill, black */
#define IMAGE_BYTES         (96 * 240)          /* rows 0-95 */
#define BOX_BYTES           (64 * 240)          /* rows 96-159 */
#define FADE_STEP           2
#define BOX_FILL_COLOR      31                  /* the box's backdrop */

/* The note beside the QR code: lines of at most 31 characters, an empty string ends it. */
static const u8 sStrQrNote[][32] = {
    "Scan for formatlibrary.com,",
    "a site about retro Yu-Gi-Oh!",
    "formats. Do not trust this",
    "code unless it leads to",
    "formatlibrary.com.",
    "",
};

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

static u32 ReadU32(const u8 *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

/* The QR code (if any) and its note into the decoded box bitmap (BOX_WIDTH x BOX_HEIGHT, one byte per pixel). */
static void DrawQrCode(const u8 *image, u8 *box)
{
    u32 size = ReadU32(image + QR_SIZE_OFFSET);
    u32 border, r, c, x, y;
    u32 i;
    const u8 *ch;

    if (size == 0)
        return;
    border = (BOX_HEIGHT - size * QR_MODULE) / 2;
    for (y = 0; y < BOX_HEIGHT; y++) {
        for (x = 0; x < BOX_HEIGHT; x++)
            box[y * BOX_WIDTH + QR_X + x] = COLOR_WHITE;
    }
    for (r = 0; r < size; r++) {
        u32 bits = ReadU32(image + QR_ROWS_OFFSET + r * 4);

        for (c = 0; c < size; c++) {
            if (bits & (1u << c)) {
                u8 *px = &box[(border + r * QR_MODULE) * BOX_WIDTH + QR_X + border + c * QR_MODULE];

                px[0] = px[1] = px[BOX_WIDTH] = px[BOX_WIDTH + 1] = COLOR_DARK;
            }
        }
    }
    for (i = 0; sStrQrNote[i][0] != 0; i++) {
        u8 *dest = &box[(NOTE_Y + i * NOTE_LINE) * BOX_WIDTH + NOTE_X];

        for (ch = sStrQrNote[i]; *ch != 0; ch++, dest += NOTE_ADVANCE)
            DrawGlyph8bpp(dest, COLOR_WHITE, *ch);
    }
}

static void PreDuelLoad(const u8 *image)
{
    SetBrightnessBlack();       /* first, and no forced blank (it shows white): the old screen stays, darkened */
    LzDecompress(image + IMAGE_LZ_OFFSET, gBustupBitmapBuffer);
    CpuSet(gBustupBitmapBuffer, (void *)VRAM, IMAGE_BYTES / 2);
    LzDecompress((const u8 *)gDialogueBoxGfx[1], gBustupBoxBitmap);
    DrawQrCode(image, gBustupBoxBitmap);
    CpuSet(gBustupBoxBitmap, (void *)(VRAM + IMAGE_BYTES), BOX_BYTES / 2);
    MemCopy16((void *)BG_PLTT, gDialogueBoxPalette, IMAGE_FIRST_COLOR * 2);
    ((u16 *)BG_PLTT)[BOX_FILL_COLOR] = 0;
    ((u16 *)BG_PLTT)[COLOR_WHITE] = 0x7FFF;  /* black, as in the scene sets' palettes (the box palette's is grey) */
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
