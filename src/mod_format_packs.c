/*
 * TCG booster packs for the main-menu Booster Packs shop (tools/format_packs.py, packs/*_tcg.json).
 * Rarity-weighted pulls; pack cover art is the pool's rarest card (tools/format_packs.py portrait pick).
 */

#include "global.h"
#include "gba.h"
#include "card_data.h"              /* gCardNumberToId, CARD_ART_* */
#include "booster.h"
#include "format_packs.h"
#include "format_pack_data.h"
#include "util.h"

extern int ModPackShopIsOpen(void);
extern int ModIsFormatUnlocked(int order);
extern int ModFormatWins(int order);
extern struct PackListWork gSceneWork;
extern struct PackOpenWork gPackOpenWork;

#define MOD_PACK_SHOP_LIST_MAX  64

static u16 sModPackShopRows[MOD_PACK_SHOP_LIST_MAX];
static u16 sModPackShopCount;
static u8 sModPackShopListActive;

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

u16 ModPackList_Count(void)
{
    if (sModPackShopCount != 0)
        return sModPackShopCount;
    if (ModPackShopIsOpen() && gSceneWork.packCount != 0)
        return gSceneWork.packCount;
    return gSceneWork.packCount;
}

/* Shop slot (0..n-1) -> gPackInfo row when the EWRAM table is missing (packRows holds only 32). */
static u16 ModPackList_RowForShopSlot(u16 slot)
{
    u16 pos = 0;
    int fi;

    for (fi = 0; fi < FORMAT_PACK_COUNT; fi++) {
        if (!ModIsFormatPackUnlocked(fi))
            continue;
        if (pos == slot)
            return VANILLA_PACK_INFO_COUNT + (u16)fi;
        pos++;
    }
    return 0;
}

u16 ModPackList_RowAt(u16 index)
{
    u16 n;
    u16 slot;

    if (sModPackShopCount != 0) {
        n = sModPackShopCount;
        return sModPackShopRows[index % n];
    }
    n = gSceneWork.packCount;
    if (n == 0)
        return 0;
    slot = index % n;
    if (slot < 0x20)
        return gSceneWork.packRows[slot];
    if (ModPackShopIsOpen())
        return ModPackList_RowForShopSlot(slot);
    return gSceneWork.packRows[slot];
}

void ModPackShopList_Begin(void)
{
    sModPackShopListActive = 1;
    sModPackShopCount = 0;
}

void ModPackShopList_Reset(void)
{
    sModPackShopListActive = 0;
    sModPackShopCount = 0;
}

void ModPackShopList_SyncSceneWork(void)
{
    u16 i;
    u16 n = sModPackShopCount;

    gSceneWork.packCount = n;
    for (i = 0; i < n && i < 0x20; i++)
        gSceneWork.packRows[i] = sModPackShopRows[i];
}

static int ModIsFormatPackUnlocked(int fi)
{
    u8 fmt;

#if !MOD_RELEASE
    if (ModPackShopIsOpen())
        return 1;
#endif
    fmt = sFormatPackFormatIndex[fi];
    if (sFormatPackFlags[fi] & FORMAT_PACK_FLAG_USAGE)
        return ModFormatWins(fmt) >= FORMAT_PACK_USAGE_WINS;
    /* TCG sets: only when their campaign format is unlocked (e.g. Metal Raiders with Critter, not Yugi-Kaiba). */
    return ModIsFormatUnlocked(fmt);
}

static int ModPackList_AppendFormatRow(int fi)
{
    u16 row = VANILLA_PACK_INFO_COUNT + fi;

    if (sModPackShopListActive || ModPackShopIsOpen()) {
        if (sModPackShopCount >= MOD_PACK_SHOP_LIST_MAX)
            return 0;
        sModPackShopRows[sModPackShopCount++] = row;
        return 1;
    }
    if (gSceneWork.packCount >= 0x20)
        return 0;
    gSceneWork.packRows[gSceneWork.packCount++] = row;
    return 1;
}

int ModPackList_AddFormatPackById(u16 packId)
{
    int fi = FormatPackIndexFromId(packId);

    if (fi < 0 || !ModIsFormatPackUnlocked(fi))
        return 0;
    return ModPackList_AppendFormatRow(fi);
}

void ModPackList_AddUnlockedPacks(void)
{
    int fi;

    if (!ModPackShopIsOpen()) {
        sModPackShopListActive = 0;
        sModPackShopCount = 0;
        PackList_AddUnlockedPacks();
        return;
    }
    ModPackShopList_Begin();
    for (fi = 0; fi < FORMAT_PACK_COUNT; fi++) {
        if (ModIsFormatPackUnlocked(fi))
            ModPackList_AppendFormatRow(fi);
    }
    ModPackShopList_SyncSceneWork();
    sModPackShopListActive = 0;
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

/* Random() is 15-bit (see sprite.c); widen before % FORMAT_PACK_ROULETTE_SIZE. */
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
