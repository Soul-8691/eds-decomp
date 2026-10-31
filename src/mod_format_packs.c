/*
 * Format booster packs for the main-menu Booster Packs shop (tools/format_packs.py).
 * Inverse-usage weights, Limited/Semi-Limited factors like card prices, portrait pack icons from card art.
 */

#include "global.h"
#include "gba.h"
#include "card_data.h"              /* gCardNumberToId, CARD_ART_* */
#include "booster.h"
#include "format_packs.h"
#include "format_pack_data.h"
#include "util.h"

extern int ModIsOpponentUnlocked(u16 duelist);
extern int ModPackShopIsOpen(void);
extern struct PackListWork gSceneWork;
extern struct PackOpenWork gPackOpenWork;

/* gPackListPal at 0x0863CC7C (256 BGR555 colours for pack covers). */
extern const u16 gPackListPal[];

#define PACK_COVER_W        56
#define PACK_COVER_H        112
#define PACK_COVER_TILES    98
#define PACK_COVER_BYTES    (PACK_COVER_TILES * 64)

static struct PackInfo sFormatPackInfoRows[FORMAT_PACK_COUNT];
static u8 sFormatPackInfoReady;

static u32 ColorDist(u16 a, u16 b)
{
    int dr = (a & 31) - (b & 31);
    int dg = (a >> 5 & 31) - (b >> 5 & 31);
    int db = (a >> 10 & 31) - (b >> 10 & 31);

    return dr * dr + dg * dg + db * db;
}

static u8 NearestPalIndex(u16 rgb)
{
    u32 best = 0xFFFFFFFF;
    u8 bestI = 1;
    u32 i;

    for (i = 1; i < 256; i++) {
        u32 d = ColorDist(rgb, gPackListPal[i]);
        if (d < best) {
            best = d;
            bestI = i;
        }
    }
    return bestI;
}

static u32 ArtPixel(u16 id, int x, int y)
{
    const u8 *src = (const u8 *)(CARD_ART_GFX_ADDR + id * CARD_ART_SIZE);
    u32 i = ((y >> 3) * 9 + (x >> 3)) * 64 + (y & 7) * 8 + (x & 7);
    u32 bit = i * 6;

    return ((src[bit >> 3] | src[(bit >> 3) + 1] << 8) >> (bit & 7)) & 63;
}

/* object-fit: cover — uniform scale to fill 56x112, centered on 72x80 art. */
static void BlitArtCover(u16 cardId, u8 *tiles)
{
    const u16 *artPal = (const u16 *)(CARD_ART_PALETTES_ADDR + cardId * CARD_ART_PALETTE_SIZE);
    u8 palMap[64];
    int x, y, px, py;

    for (x = 0; x < 64; x++)
        palMap[x] = NearestPalIndex(artPal[x]);

    for (y = 0; y < PACK_COVER_H; y++) {
        for (x = 0; x < PACK_COVER_W; x++) {
            int sx = 16 + x * 40 / PACK_COVER_W;
            int sy = y * 80 / PACK_COVER_H;
            u32 tileIdx;
            u8 idx;

            if (sx > 71)
                sx = 71;
            if (sy > 79)
                sy = 79;
            idx = palMap[ArtPixel(cardId, sx, sy)];
            px = x;
            py = y;
            tileIdx = (py >> 3) * 7 + (px >> 3);
            tiles[tileIdx * 64 + (py & 7) * 8 + (px & 7)] = idx;
        }
    }
}

int FormatPackIndexFromId(u16 packId)
{
    if (packId < FORMAT_PACK_ID_FIRST || packId >= FORMAT_PACK_ID_FIRST + FORMAT_PACK_COUNT)
        return -1;
    return (int)packId - FORMAT_PACK_ID_FIRST;
}

static void EnsureFormatPackInfo(void)
{
    int fi;

    if (sFormatPackInfoReady)
        return;
    for (fi = 0; fi < FORMAT_PACK_COUNT; fi++) {
        const char *name = sFormatPackNames[fi];
        int i;

        sFormatPackInfoRows[fi].id = FORMAT_PACK_ID_FIRST + fi;
        sFormatPackInfoRows[fi].pad2 = 0;
        sFormatPackInfoRows[fi].coverGfx = NULL;
        for (i = 0; i < 0x3F && name[i]; i++)
            sFormatPackInfoRows[fi].name[i] = name[i];
        sFormatPackInfoRows[fi].name[i] = 0;
    }
    sFormatPackInfoReady = 1;
}

u16 ModGetSelectedPackId(u16 packRowIndex)
{
    if (packRowIndex >= VANILLA_PACK_INFO_COUNT)
        return FORMAT_PACK_ID_FIRST + (packRowIndex - VANILLA_PACK_INFO_COUNT);
    return gPackInfo[packRowIndex].id;
}

const struct PackInfo *ModPackInfoAtRow(u16 packRowIndex)
{
    if (packRowIndex >= VANILLA_PACK_INFO_COUNT) {
        EnsureFormatPackInfo();
        return &sFormatPackInfoRows[packRowIndex - VANILLA_PACK_INFO_COUNT];
    }
    return &gPackInfo[packRowIndex];
}

int ModPackList_AddFormatPackById(u16 packId)
{
    int fi = FormatPackIndexFromId(packId);
    u16 *cnt;
    u16 *list;

    if (fi < 0)
        return 0;
    cnt = &gSceneWork.packCount;
    if (*cnt >= 0x20)
        return 0;
    list = cnt - 0x20;
    list[*cnt] = VANILLA_PACK_INFO_COUNT + fi;
    (*cnt)++;
    return 1;
}

void ModPackList_AddUnlockedPacks(void)
{
    int fi;

    if (!ModPackShopIsOpen()) {
        PackList_AddUnlockedPacks();
        return;
    }
    for (fi = 0; fi < FORMAT_PACK_COUNT; fi++) {
        if (ModIsOpponentUnlocked(sFormatPackDuelist[fi]))
            ModPackList_AddFormatPackById(FORMAT_PACK_ID_FIRST + fi);
    }
}

int ModFormatPackLoadCoverGfx(u16 slot, u16 packId)
{
    int fi = FormatPackIndexFromId(packId);
    u16 number;
    u16 cardId;
    u16 tile;
    u8 cover[PACK_COVER_BYTES];

    if (fi < 0)
        return 0;
    number = sFormatPackPortraitNumber[fi];
    if (number == 0)
        return 0;
    cardId = gCardNumberToId[number];
    if (cardId == 0)
        return 0;
    BlitArtCover(cardId, cover);
    tile = slot * 0x62 + 0x10;
    {
        vu32 *dma = (vu32 *)0x040000D4;
        dma[0] = (u32)cover;
        dma[1] = (u32)&gSceneWork.bgTiles[tile << 6];
        dma[2] = 0x80000000 | (PACK_COVER_BYTES >> 1);
        while (dma[2] & 0x80000000)
            ;
    }
    return 1;
}

static u16 WeightForNumber(const struct FormatPackPool *pool, u16 number)
{
    u16 i;

    for (i = 0; i < pool->count; i++) {
        if (pool->entries[i].number == number)
            return pool->entries[i].weight;
    }
    return 65535;
}

static int CardAlreadyPicked(const u16 *cardNumbers, int count, u16 number)
{
    int j;

    for (j = 0; j < count; j++) {
        if (cardNumbers[j] == number)
            return 1;
    }
    return 0;
}

/* Random() is 15-bit (see sprite.c); widen before % FORMAT_PACK_ROULETTE_SIZE (65536). */
static u32 RandomBelow(u32 n)
{
    u32 r;

    if (n <= 1)
        return 0;
    r = (u32)Random();
    if (n > 0x8000)
        r |= (u32)Random() << 15;
    return r % n;
}

int ModGenerateFormatPackCards(u16 *cardNumbers, u16 packId)
{
    int fi = FormatPackIndexFromId(packId);
    const struct FormatPackPool *pool;
    const u16 *roulette;
    u16 rareNumber = 0;
    u16 rareWeight = 65535;
    int i;

    if (fi < 0)
        return -1;
    pool = &sFormatPackPools[fi];
    roulette = sFormatPackRoulettes[fi];
    if (pool->count == 0 || roulette == NULL)
        return -1;
    for (i = 0; i < 5; i++) {
        u16 n;
        int tries;

        for (tries = 0; tries < 64; tries++) {
            n = roulette[RandomBelow(FORMAT_PACK_ROULETTE_SIZE)];
            if (!CardAlreadyPicked(cardNumbers, i, n))
                break;
        }
        cardNumbers[i] = n;
        {
            u16 w = WeightForNumber(pool, n);

            if (w < rareWeight) {
                rareWeight = w;
                rareNumber = n;
            }
        }
    }
    for (i = 0; i < 25; i++) {
        int a = Random() % 5;
        int b = Random() % 5;
        u16 t = cardNumbers[a];
        cardNumbers[a] = cardNumbers[b];
        cardNumbers[b] = t;
    }
    gPackOpenWork.rareCardNumber = rareNumber;
    return 7;
}
