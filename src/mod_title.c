/*
 * mod_title: the title screen. Not part of the original ROM.
 *   ModTitle_Load            Title_LoadGraphics, after the original logo (whose map it clears) and in place of the
 *                            flames' and the coin's packs: "Yak's YGO Hack" on BG0 and the flames (BG1, now
 *                            256-colour) and the coin (BG2), from gModTitleImages; and the two small lines' text.
 *   ModTitle_LoadCopyright   Title_FadeIn at the white flash, in place of the copyright line's pack: the copyright
 *                            line on BG2 (row 17).
 *   ModTitle_LoadWave        Title_LoadGraphics: the flames' wave table, or none (gModTitleFlameWave 0: still).
 *   ModTitle_Draw            Title_DrawMenu, every frame: the two small lines.
 * gModTitleImages (data/mod_title_data.s, tools/title_images.py) holds every image in a plain layout, its colours
 * then its tiles in row-major order, so Hex Maniac Advance can show and edit them (graphics.title.*):
 *   text       the whole of BG0 (240 x 160, now 4bpp): "Yak's YGO Hack" at y 40 at its final size
 *              (title/yaks_ygo_hack.png, the main menu's highlighted letters at 2x: no scaling here), BG palette 8
 *              (entries 128-143);
 *   flame      the whole of BG1 (240 x 160, now 8bpp), the flames at x 72, y 0, its colours in entries 0-127 (entry 0
 *              is the screen's backdrop);
 *   coin       96 x 96, 4bpp, palette 11, BG2 map from column 9, 4bpp tiles 0x2F8-0x387;
 *   copyright  224 x 16, 4bpp, palette 9, BG2 map row 17, 4bpp tiles 0x2C0-0x2F7.
 * The two full-screen images (600 tiles each) don't fit in the background VRAM as they are, so only their distinct
 * tiles are loaded (LoadScreenImage): a blank tile is tile 0 (cleared), and a tile equal to one already loaded shares
 * it. Charblock 1 bytes: 0x0000-0x003F tile 0; the text's 4bpp tiles from 0x0040 up (4bpp tile numbers reach only
 * 0x8000); 0x5800-0x72FF the copyright line, the coin and the grid (4bpp tiles 0x2C0-0x397); the flames' 8bpp tiles
 * from 0x7300 to 0xBFFF (8bpp tiles 0x1CC-0x2FF), then down from 0x5800 to the text's end. Room for the whole text as
 * distinct tiles; the flames get at least 308 distinct tiles, and past the free room a tile shows blank.
 * The background grid (BG3) is the original's (already a plain 4 x 4-tile pack). The small lines, in the 8-pixel
 * font in the top corners: the version (gModTitleVersion, make demo and make release-demo only: MOD_DEMO) at the left
 * and gModTitleRelease or gModTitleDebug (MOD_RELEASE) at the right, in OBJ tiles 64-79 with OBJ palette 15. The
 * strings are data, so HMA can change them (text.title.*).
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "util.h"
#include "sprite.h"
#include "text.h"

/* The small lines' text (HMA: text.title.version, .release, .debug; NUL-terminated, at most 15 characters). */
const char gModTitleVersion[16] = "v1.0.2.10";
const char gModTitleRelease[16] = "Release";
const char gModTitleDebug[16] = "Debug";
/* The flames' wave during the fade-in (Title_HBlank shifts BG1 by gMain.hblankScroll): 1 waves, 0 keeps them still.
 * A byte of data, so Hex Maniac Advance can switch it (MatchedWord data.constants.title.flamewave). */
const u8 gModTitleFlameWave[1] = { 1 };
/* The flames' layer priority (BG1CNT bits 0-1, 0 front to 3 back; MatchedWord data.constants.title.flamepriority).
 * 1, the original, is in front of the coin and the copyright line (BG2, priority 2); 3 puts the flames behind them
 * (still in front of the grid, BG3: at equal priority the lower BG is in front). */
const u8 gModTitleFlamePriority[1] = { 1 };

extern const u16 gTitleLogoWave[];      /* 0x08198830: 16 BG1 HOFS values */

extern const u8 gModTitleImages[];      /* data/mod_title_data.s: u32 offsets[4], then text, flame, coin, copyright */
enum { IMAGE_TEXT, IMAGE_FLAME, IMAGE_COIN, IMAGE_COPYRIGHT };

#define SCREEN_TILES        (30 * 20)
#define FLAME_COLORS        128
#define TEXT_PALETTE        8           /* entries 128-143, after the flames' */
#define FIXED_FIRST         0x5800      /* charblock 1 bytes of the copyright line, the coin and the grid (4bpp) */
#define FIXED_END           0x7300
#define BLOCK_END           0xC000      /* the end of BG VRAM (0x06010000) */
#define COIN_TILE           0x2F8       /* 4bpp tile numbers */
#define COIN_PALETTE        11
#define COIN_MAP_COL        9
#define COPYRIGHT_TILE      0x2C0
#define COPYRIGHT_PALETTE   9
#define COPYRIGHT_W         28
#define COPYRIGHT_MAP_ROW   17
#define CHARBLOCK1          (VRAM + 0x4000)

/* The small lines: a canvas of 16 tiles, the left half for the version, the right half for the build. */
#define INFO_TILE       64
#define INFO_PALETTE    15
#define INFO_HALF_WIDTH 64
#define INFO_CHAR_WIDTH 4               /* the 8-pixel font's advance */
#define INFO_Y          2
#define INFO_LEFT_X     2
#define INFO_RIGHT_X    (240 - 2 - INFO_HALF_WIDTH)
#define INFO_SHADOW     TEXT_SIZE_COLOR(8, 1)
#define INFO_COLOR      TEXT_SIZE_COLOR(8, 2)

static const u16 sInfoPal[4] = { 0, 0x0000, 0x7FFF, 0 };    /* transparent, black, white */

static const u8 *Image(int which)
{
    return gModTitleImages + ((const u32 *)gModTitleImages)[which];
}

/* A w x h-tile image's tiles at `tile` of charblock 1 (bpp 4 or 8) and its map entries from map cell `cell` of BG map
 * buffer `map`, with palette `palette` (4bpp). */
static void LoadImage(const u8 *tiles, int w, int h, int bpp, int tile, int map, int cell, int palette)
{
    int x, y;

    MemCopy16((void *)(CHARBLOCK1 + tile * bpp * 8), tiles, w * h * bpp * 8);
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++)
            gMain.bgMapBuffer[map][cell + y * 32 + x] = (tile + y * w + x) | palette << 12;
    }
}

/* The distinct tiles of one image loaded so far (in gBustupBitmapBuffer, free on the title): their pixels' hash,
 * source and tile number; and where the next one goes (charblock 1 bytes). */
struct TilePool {
    u32 count;
    u32 words;                      /* a tile's size in words: 8 (4bpp) or 16 (8bpp) */
    u32 next, end;                  /* the run being filled: [next, end) */
    u32 next2, end2;                /* the run after it (empty: next2 == end2) */
    u32 hash[SCREEN_TILES];
    const u32 *src[SCREEN_TILES];
    u16 tile[SCREEN_TILES];
};
extern u8 gBustupBitmapBuffer[];

static void PoolStart(struct TilePool *pool, u32 words, u32 next, u32 end, u32 next2, u32 end2)
{
    pool->count = 0;
    pool->words = words;
    pool->next = next;
    pool->end = end;
    pool->next2 = next2;
    pool->end2 = end2;
}

/* The tile number for the tile at `tile`: 0 for a blank one, the pool's tile with the same pixels, or a new one
 * loaded at pool->next, moving on to the second run when the first is full (0 when there is no room). */
static u32 PoolAdd(struct TilePool *pool, const u32 *tile)
{
    u32 h = 0, k, bytes = pool->words * 4;
    int i, any = 0;

    for (i = 0; i < pool->words; i++) {
        h = (h << 5 | h >> 27) ^ tile[i];
        any |= tile[i] != 0;
    }
    if (!any)
        return 0;
    for (k = 0; k < pool->count; k++) {
        if (pool->hash[k] == h) {
            const u32 *s = pool->src[k];

            for (i = 0; i < pool->words && s[i] == tile[i]; i++)
                ;
            if (i == pool->words)
                return pool->tile[k];
        }
    }
    if (pool->next + bytes > pool->end) {
        pool->next = pool->next2;
        pool->end = pool->end2;
        pool->end2 = pool->next2;
    }
    if (pool->next + bytes > pool->end)
        return 0;
    k = pool->count++;
    pool->hash[k] = h;
    pool->src[k] = tile;
    pool->tile[k] = pool->next / bytes;
    MemCopy16((void *)(CHARBLOCK1 + pool->next), tile, bytes);
    pool->next += bytes;
    return pool->tile[k];
}

/* A full-screen image (30 x 20 tiles, row-major) as BG map buffer `map` (palette `palette` for 4bpp), its distinct
 * tiles from the pool. */
static void LoadScreenImage(struct TilePool *pool, const u8 *tiles, int map, int palette)
{
    int i;

    for (i = 0; i < SCREEN_TILES; i++) {
        u32 t = PoolAdd(pool, (const u32 *)(tiles + i * pool->words * 4));

        gMain.bgMapBuffer[map][(i / 30) * 32 + i % 30] = t ? t | palette << 12 : 0;
    }
}

static void DrawInfoText(int x, const char *text)
{
    TextDrawString(x + 1, 1, INFO_SHADOW, (const u8 *)text);
    TextDrawString(x, 0, INFO_COLOR, (const u8 *)text);
}

static void LoadInfoLines(void)
{
    const char *build = MOD_RELEASE ? gModTitleRelease : gModTitleDebug;

    MemCopy16((void *)(OBJ_PLTT + INFO_PALETTE * 0x20), sInfoPal, sizeof(sInfoPal));
    TextCanvasInit(INFO_HALF_WIDTH / 4, 1);
#if MOD_DEMO
    DrawInfoText(0, gModTitleVersion);
#endif
    /* Right-aligned, leaving a pixel for the shadow. */
    DrawInfoText(INFO_HALF_WIDTH * 2 - 1 - StrLen((const u8 *)build) * INFO_CHAR_WIDTH, build);
    TextCanvasToTiles((u16 *)(OBJ_VRAM0 + INFO_TILE * 0x20), 0);
}

static void DrawInfoHalf(int x, int tile)
{
    int k;

    for (k = 0; k < 2; k++)
        AddSprite((INFO_Y << 16) | (x + k * 32), SPRITE_SHAPE_32x8,
                  (tile + k * 4) | OAM_ATTR2_PALETTE(INFO_PALETTE));
}

void ModTitle_Load(void)
{
    struct TilePool *pool = (struct TilePool *)gBustupBitmapBuffer;
    const u8 *img;
    u32 textEnd;

    MemClear16((void *)CHARBLOCK1, 64);     /* tile 0: blank (8bpp; 4bpp tiles 0 and 1) */
    /* "Yak's YGO Hack" on BG0, now 4bpp (over the original logo), palette 8 */
    REG_BG0CNT &= ~BGCNT_256COLOR;
    MemClear16(gMain.bgMapBuffer[0], sizeof(gMain.bgMapBuffer[0]));
    img = Image(IMAGE_TEXT);
    MemCopy16((void *)(BG_PLTT + TEXT_PALETTE * 32), img, 32);
    PoolStart(pool, 8, 64, FIXED_FIRST, 0, 0);
    LoadScreenImage(pool, img + 32, 0, TEXT_PALETTE);
    textEnd = (pool->next + 63) & ~63;
    /* the flames on BG1, now 256-colour: its colours in entries 0-127 (0: the backdrop), its tiles above the
     * fixed 4bpp ones, then in the room the text left */
    REG_BG1CNT = (REG_BG1CNT & ~3) | BGCNT_256COLOR | (gModTitleFlamePriority[0] & 3);
    img = Image(IMAGE_FLAME);
    MemCopy16((void *)BG_PLTT, img, FLAME_COLORS * 2);
    MemClear16(gMain.bgMapBuffer[1], sizeof(gMain.bgMapBuffer[1]));
    PoolStart(pool, 16, FIXED_END, BLOCK_END, textEnd, FIXED_FIRST);
    LoadScreenImage(pool, img + FLAME_COLORS * 2, 1, 0);
    /* the coin on BG2 */
    img = Image(IMAGE_COIN);
    MemCopy16((void *)(BG_PLTT + COIN_PALETTE * 32), img, 32);
    LoadImage(img + 32, 12, 12, 4, COIN_TILE, 2, COIN_MAP_COL, COIN_PALETTE);
    LoadInfoLines();
}

/* Title_LoadGraphics: the wave table for Title_HBlank, or zeros (no wave) when gModTitleFlameWave is 0. */
void ModTitle_LoadWave(void)
{
    if (gModTitleFlameWave[0])
        MemCopy16(gMain.hblankScroll, gTitleLogoWave, 0x20);
    else
        MemClear16(gMain.hblankScroll, 0x20);
}

void ModTitle_LoadCopyright(void)
{
    const u8 *img = Image(IMAGE_COPYRIGHT);

    MemCopy16((void *)(BG_PLTT + COPYRIGHT_PALETTE * 32), img, 32);
    LoadImage(img + 32, COPYRIGHT_W, 2, 4, COPYRIGHT_TILE, 2, COPYRIGHT_MAP_ROW * 32, COPYRIGHT_PALETTE);
}

void ModTitle_Draw(void)
{
#if MOD_DEMO
    DrawInfoHalf(INFO_LEFT_X, INFO_TILE);
#endif
    DrawInfoHalf(INFO_RIGHT_X, INFO_TILE + INFO_HALF_WIDTH / 8);
}
