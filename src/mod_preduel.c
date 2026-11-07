/*
 * mod_preduel: a screen before the duel for the opponents (src/mod_opponents.c), with no dialogue. Not part of the
 * original ROM.
 *   CB_ModPreDuelImage  Campaign_SelectOpponent (campaign_steps.c) sub-step 6, where these opponents used to skip
 *                       the pre-duel dialogue (Solomon and Debug after their Deck Recipe): returns 1 at once for an
 *                       opponent with no entry and for the later duels of a match. Otherwise the bustup screen's
 *                       layout in Mode 4, page 0: the picture in rows 0-95 over the dialogue box (gDialogueBoxGfx[1],
 *                       as Bustup_LoadScene), a fade in from black, A, B or START, a fade out to black, then 1.
 * Per opponent (gModPreDuelTable by duelist ID, tools/preduel_text.py; Hex Maniac Advance: data.preduel.table):
 *   picture  its own 240x96 picture (duelists/<name>.png, tools/preduel_image.py: 224 colours and 8bpp tiles), or
 *            none (a null pointer, e.g. set in Hex Maniac Advance), and then
 *   scene    its duelist's scene from the original pre-duel dialogue (the scene set GetSceneSet gives, as
 *            Bustup_LoadSceneSet loads it: 240x96 bitmap, colours 16-255, OBJ colours and tiles), from an
 *            uncompressed copy (tools/preduel_scenes.py; HMA: graphics.duelists.<duelist>.scene and .eyes), with the
 *            scene set's animation tracks: the eyes blink (Bustup_TickBlink) every frame of the screen;
 *   qr       the data holding its QR code (its own, or Yugi-Kaiba's formatlibrary.com code), or none: a white square at
 *            the left of the box, 2 px a module;
 *   text     gModPreDuelText: 5 lines of up to 38 characters (data.preduel.text), drawn inside the box's border
 *            (x 24-216) in the bustup's 8x10 Latin font (DrawGlyph8bpp); the lines start with spaces where the QR
 *            code is, so without one the text can use the whole box.
 * Palette: the box palette's colours 0-31 (the box uses 16-31; its fill, 31, made black as the scenes have it), then
 * the picture's colours, 32-255. Its buffers are the bustup's (gBustupBitmapBuffer, gBustupBoxBitmap), free while no
 * dialogue runs. Like the other mod units, this one is @farcode and @rodata in units.txt.
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
#include "constants/game.h"     /* DUELIST_* */
#include "bustup.h"             /* DrawGlyph8bpp, gBustup, GetSceneSet, Bustup_InitState, Bustup_TickBlink */
#include "sprite.h"             /* AnimBlockInit, AnimStateTick, AnimBlockDraw, OamListFlush, OamListClear */

extern const u16 *const gDialogueBoxGfx[];      /* 0x08139F5C: LZSS dialogue box bitmaps */
extern const u16 gDialogueBoxPalette[];         /* 0x0874E104: box palette (the box uses colours 16-31) */
void CopyTileSheetTo2D(const u8 *src, u8 *dst, u16 mode);     /* collection.c */

/* data/mod_preduel_data.s */
struct PreDuelEntry {
    const u8 *picture;      /* tools/preduel_image.py data, or NULL: the scene */
    const u8 *qr;           /* the data holding the QR code, or NULL */
    const u8 *scene;        /* gModPreDuelScenes entry (tools/preduel_scenes.py), or NULL */
};
#define PREDUEL_LINES       5
#define PREDUEL_LINE        48                  /* 47 characters and a NUL; 38 fit in the box */
extern const struct PreDuelEntry gModPreDuelTable[25];
extern const char gModPreDuelText[25][PREDUEL_LINES][PREDUEL_LINE];

#define IMAGE_FIRST_COLOR   32
#define IMAGE_COLORS        224
#define QR_SIZE_OFFSET      (IMAGE_COLORS * 2)
#define QR_SPRITE_OFFSET    (QR_SIZE_OFFSET + 4 + 16 * 2)   /* after the size and the QR sprite's palette */
#define QR_ROWS             32                              /* the sprite: 32x32, 4x4 4bpp tiles */
#define IMAGE_TILES_OFFSET  (QR_SPRITE_OFFSET + QR_ROWS * QR_ROWS / 2)
#define IMAGE_WIDTH         240
#define IMAGE_HEIGHT        96
#define BOX_WIDTH           240
#define BOX_HEIGHT          64
#define QR_X                6                   /* the white square: x 6-69, the box's full height */
#define QR_MODULE           2
#define TEXT_X              24                  /* the text's left edge: inside the box's border (x 23-216) */
#define TEXT_RIGHT          216                 /* a glyph (5 px of 8) is drawn only up to here */
#define NOTE_Y              3
#define NOTE_LINE           12
#define NOTE_ADVANCE        5                   /* the Latin font's advance, as Calendar_DrawStringShadow */
#define COLOR_WHITE         1                   /* BG colours 1-15 are free here: the box uses 16-31 */
#define COLOR_DARK          31                  /* the box fill, black */
#define IMAGE_BYTES         (96 * 240)          /* rows 0-95 */
#define BOX_BYTES           (64 * 240)          /* rows 96-159 */
#define FADE_STEP           2
#define BOX_FILL_COLOR      31                  /* the box's backdrop */
#define GLYPH_WIDTH         8
/* A scene copy (gModPreDuelScenes entry): BG colours, the bitmap's tiles, OBJ colours, the OBJ tile sheet. */
#define SCENE_BITMAP        (256 * 2)
#define SCENE_OBJ_COLORS    (SCENE_BITMAP + IMAGE_WIDTH * IMAGE_HEIGHT)
#define SCENE_OBJ_TILES     (SCENE_OBJ_COLORS + 256 * 2)
#define OBJ_VRAM_BITMAP     (VRAM + 0x14000)    /* OBJ tiles in the bitmap modes */
#define TILE_COLORS_16      0x10                /* CopyTileSheetTo2D: 16 rows of 16 4bpp tiles */

enum {
    PREDUEL_LOAD,
    PREDUEL_FADE_IN,
    PREDUEL_WAIT,
    PREDUEL_FADE_OUT,
};

/* Opponent `duelist`'s pre-duel entry, or NULL for none. */
static const struct PreDuelEntry *PreDuelEntryOf(u16 duelist)
{
    const struct PreDuelEntry *e;

    if (duelist >= ARRAY_COUNT(gModPreDuelTable))
        return NULL;
    e = &gModPreDuelTable[duelist];
    return e->picture != NULL || e->qr != NULL || e->scene != NULL || gModPreDuelText[duelist][0][0] != 0 ? e : NULL;
}

static void LzDecompress(const u8 *blob, void *dest)
{
    LZSSDecompress((u8 *)blob + 4, dest, blob[0] | blob[1] << 8 | blob[2] << 16 | blob[3] << 24);
}

static u32 ReadU32(const u8 *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

/* 1 if QR module (r, c) is dark: pixel (c, r) of the 4bpp sprite. */
static u32 QrModule(const u8 *image, u32 r, u32 c)
{
    u8 b = image[QR_SPRITE_OFFSET + ((r / 8) * (QR_ROWS / 8) + c / 8) * 32 + (r % 8) * 4 + (c % 8) / 2];

    return (c & 1 ? b >> 4 : b & 15) != 0;
}

/* 8bpp tiles (30 x 12) as the Mode 4 bitmap, each pixel + `add` (32 for a picture's colour - 32, 0 for a scene). */
static void UntilePicture(const u8 *tiles, u8 *bitmap, u8 add)
{
    u32 x, y;

    for (y = 0; y < IMAGE_HEIGHT; y++) {
        const u8 *row = tiles + ((y / 8) * (IMAGE_WIDTH / 8)) * 64 + (y % 8) * 8;

        for (x = 0; x < IMAGE_WIDTH; x += 8) {
            const u8 *src = row + (x / 8) * 64;
            u8 *dest = bitmap + y * IMAGE_WIDTH + x;
            u32 i;

            for (i = 0; i < 8; i++)
                dest[i] = src[i] + add;
        }
    }
}

/* The QR code of `image` (if it has one) into the decoded box bitmap (BOX_WIDTH x BOX_HEIGHT, a byte a pixel). */
static void DrawQrCode(const u8 *image, u8 *box)
{
    u32 size = ReadU32(image + QR_SIZE_OFFSET);
    u32 border, r, c, x, y;

    if (size == 0)
        return;
    border = (BOX_HEIGHT - size * QR_MODULE) / 2;
    for (y = 0; y < BOX_HEIGHT; y++) {
        for (x = 0; x < BOX_HEIGHT; x++)
            box[y * BOX_WIDTH + QR_X + x] = COLOR_WHITE;
    }
    for (r = 0; r < size; r++) {
        for (c = 0; c < size; c++) {
            if (QrModule(image, r, c)) {
                u8 *px = &box[(border + r * QR_MODULE) * BOX_WIDTH + QR_X + border + c * QR_MODULE];

                px[0] = px[1] = px[BOX_WIDTH] = px[BOX_WIDTH + 1] = COLOR_DARK;
            }
        }
    }
}

/* Opponent `duelist`'s text into the box bitmap: each line from TEXT_X, a space only moving on. */
static void DrawPreDuelText(u16 duelist, u8 *box)
{
    int line, i;

    for (line = 0; line < PREDUEL_LINES; line++) {
        const char *s = gModPreDuelText[duelist][line];
        int x = TEXT_X;

        for (i = 0; i < PREDUEL_LINE && s[i] != 0 && x + NOTE_ADVANCE <= TEXT_RIGHT; i++, x += NOTE_ADVANCE) {
            if (s[i] != ' ')
                DrawGlyph8bpp(&box[(NOTE_Y + line * NOTE_LINE) * BOX_WIDTH + x], COLOR_WHITE, (u8)s[i]);
        }
    }
}

/* An opponent without a picture: its duelist's scene from the copy, as Bustup_LoadSceneSet loads a scene set (the
 * bitmap, BG colours 16-255, OBJ colours and tiles) with the set's animation tracks; the eyes then blink. */
static void LoadScene(const u8 *scene, u16 duelist)
{
    const struct SceneSet *set = GetSceneSet(duelist);

    UntilePicture(scene + SCENE_BITMAP, gBustupBitmapBuffer, 0);
    MemCopy16((void *)(BG_PLTT + 16 * 2), scene + 16 * 2, (256 - 16) * 2);
    MemCopy16((void *)OBJ_PLTT, scene + SCENE_OBJ_COLORS, 256 * 2);
    CopyTileSheetTo2D(scene + SCENE_OBJ_TILES, (u8 *)OBJ_VRAM_BITMAP, TILE_COLORS_16);
    gBustup.animCount = set->anim != NULL ? AnimBlockInit((struct AnimSeq **)set->anim, (u8 *)gBustup.anims) : 0;
    Bustup_ResetBlink();
}

/* Every frame of a scene: the blink, the tracks and their sprites, as Bustup_Update. */
static void TickScene(void)
{
    u8 i;

    Bustup_TickBlink(gBustup.anims);
    for (i = 0; i < gBustup.animCount; i++)
        AnimStateTick(&gBustup.anims[i]);
    for (i = 0; i < gBustup.animCount; i++)
        AnimBlockDraw((u8 *)&gBustup.anims[i], 0, 0, 0, 1, 0, 0, 0, 0, (u32)&gBustup.sprites);
    OamListFlush(&gBustupSprites);
    OamListClear((u8 *)&gBustupSprites);
}

static void PreDuelLoad(const struct PreDuelEntry *e, u16 duelist)
{
    SetBrightnessBlack();       /* first, and no forced blank (it shows white): the old screen stays, darkened */
    Bustup_InitState();         /* the bustup state (scene tracks, sprite list), as the dialogue starts it */
    MemCopy16((void *)BG_PLTT, gDialogueBoxPalette, IMAGE_FIRST_COLOR * 2);
    ((u16 *)BG_PLTT)[BOX_FILL_COLOR] = 0;
    if (e->picture != NULL) {
        UntilePicture(e->picture + IMAGE_TILES_OFFSET, gBustupBitmapBuffer, IMAGE_FIRST_COLOR);
        MemCopy16((void *)(BG_PLTT + IMAGE_FIRST_COLOR * 2), e->picture, IMAGE_COLORS * 2);
    } else if (e->scene != NULL) {
        LoadScene(e->scene, duelist);   /* (its colours 16-31 are the box's, as in the dialogue) */
    } else {
        int i;

        for (i = 0; i < IMAGE_BYTES; i++)
            gBustupBitmapBuffer[i] = BOX_FILL_COLOR;
    }
    ((u16 *)BG_PLTT)[COLOR_WHITE] = 0x7FFF;
    CpuSet(gBustupBitmapBuffer, (void *)VRAM, IMAGE_BYTES / 2);
    LzDecompress((const u8 *)gDialogueBoxGfx[1], gBustupBoxBitmap);
    if (e->qr != NULL)
        DrawQrCode(e->qr, gBustupBoxBitmap);
    DrawPreDuelText(duelist, gBustupBoxBitmap);
    CpuSet(gBustupBoxBitmap, (void *)(VRAM + IMAGE_BYTES), BOX_BYTES / 2);
    REG_BG2PA = 0x100;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 0x100;
    REG_BG2X = 0;
    REG_BG2Y = 0;
    REG_DISPCNT = DISPCNT_MODE_4 | DISPCNT_BG2_ON | (e->picture == NULL && e->scene != NULL ? DISPCNT_OBJ_ON : 0);
}

u16 CB_ModPreDuelImage(void)
{
    const struct PreDuelEntry *e = PreDuelEntryOf(gMain.opponent);

    if (e == NULL || gMain.matchDuelCount != 0)
        return 1;
    if (gMain.seqIndex1 != PREDUEL_LOAD && e->picture == NULL && e->scene != NULL)
        TickScene();                    /* the eyes blink */
    switch (gMain.seqIndex1) {
    case PREDUEL_LOAD:
        PreDuelLoad(e, gMain.opponent);
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
        if (FadeToBlack(FADE_STEP)) {
            OamListClear((u8 *)&gBustupSprites);     /* no eyes left on the next screen */
            OamListFlush(&gBustupSprites);
            return 1;
        }
        break;
    }
    return 0;
}
