/*
 * TCG booster packs for the main-menu Booster Packs shop and the pack a Campaign win gives (tools/format_packs.py,
 * packs/*_tcg.json).
 * Rarity-weighted pulls; pack cover art is pre-baked at build time (tools/format_pack_covers.py).
 */

#include "global.h"
#include "gba.h"
#include "card_data.h"
#include "booster.h"
#include "format_packs.h"
#include "format_pack_data.h"
#include "util.h"
#include "main.h"                 /* gMain: seqIndex1, seqState1, rewardPack (ModRewardPack) */

extern int ModPackShopIsOpen(void);
extern int ModIsFormatUnlocked(int order);
extern int ModFormatWins(int order);
extern struct PackListWork gSceneWork;
extern struct PackOpenWork gPackOpenWork;

#define MOD_PACK_SHOP_LIST_MAX  64

/* The list's state: gModPackList (EWRAM 0x0202F010, symbols.ld), never .bss, which links at 0x02000000 over
 * gTextCanvas: drawing the list's text lines overwrote the rows. */
struct ModPackList {
    u16 rows[MOD_PACK_SHOP_LIST_MAX];
    u16 count;
    u8 active;
    u8 pad;
    struct PackInfo info;       /* the format pack ModPackInfoAtRow returned last */
};
extern struct ModPackList gModPackList;

#define PACK_COVER_TILES    98
#define PACK_COVER_BYTES    (PACK_COVER_TILES * 64)

extern const u8 gFormatPackCovers[];

int FormatPackIndexFromId(u16 packId)
{
    if (packId < FORMAT_PACK_ID_FIRST || packId >= FORMAT_PACK_ID_FIRST + FORMAT_PACK_COUNT)
        return -1;
    return (int)packId - FORMAT_PACK_ID_FIRST;
}

int ModFormatPackShopCost(u16 packId, u32 *costOut)
{
    int fi = FormatPackIndexFromId(packId);

    if (fi < 0)
        return 0;
    if (sFormatPackFlags[fi] & FORMAT_PACK_FLAG_USAGE)
        *costOut = FORMAT_PACK_COST_USAGE;
    else
        *costOut = FORMAT_PACK_COST_TCG;
    return 1;
}

/* The PackInfo of format pack fi, in gModPackList.info (valid until the next call). */
static const struct PackInfo *FormatPackInfo(int fi)
{
    struct PackInfo *info = &gModPackList.info;
    const char *name = sFormatPackNames[fi];
    int i;

    info->id = FORMAT_PACK_ID_FIRST + fi;
    info->pad2 = 0;
    if (sFormatPackPortraitNumber[fi] != 0)
        info->coverGfx = &gFormatPackCovers[fi * PACK_COVER_BYTES];
    else
        info->coverGfx = NULL;
    for (i = 0; i < 0x3F && name[i]; i++)
        info->name[i] = name[i];
    info->name[i] = 0;
    return info;
}

u16 ModGetSelectedPackId(u16 packRowIndex)
{
    if (packRowIndex >= VANILLA_PACK_INFO_COUNT)
        return FORMAT_PACK_ID_FIRST + (packRowIndex - VANILLA_PACK_INFO_COUNT);
    return gPackInfo[packRowIndex].id;
}

const struct PackInfo *ModPackInfoAtRow(u16 packRowIndex)
{
    if (packRowIndex >= VANILLA_PACK_INFO_COUNT)
        return FormatPackInfo(packRowIndex - VANILLA_PACK_INFO_COUNT);
    return &gPackInfo[packRowIndex];
}

u16 ModPackList_Count(void)
{
    if (gModPackList.count != 0)
        return gModPackList.count;
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

    if (gModPackList.count != 0) {
        n = gModPackList.count;
        return gModPackList.rows[index % n];
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
    gModPackList.active = 1;
    gModPackList.count = 0;
}

void ModPackShopList_Reset(void)
{
    gModPackList.active = 0;
    gModPackList.count = 0;
}

void ModPackShopList_SyncSceneWork(void)
{
    u16 i;
    u16 n = gModPackList.count;

    gSceneWork.packCount = n;
    for (i = 0; i < n && i < 0x20; i++)
        gSceneWork.packRows[i] = gModPackList.rows[i];
}

/* Wins against a format that open its usage pack (FORMAT_PACK_USAGE_WINS, tools/format_packs.py). A byte of data
 * rather than an immediate, so Hex Maniac Advance can change it in any build (MatchedWord
 * data.constants.packs.format.usagewins, tools/hma_toml.py). */
const u8 gModUsagePackWins[1] = { FORMAT_PACK_USAGE_WINS };

static int ModIsFormatPackUnlocked(int fi)
{
    u8 fmt;

#if !MOD_RELEASE
    return 1;
#endif
    fmt = sFormatPackFormatIndex[fi];
    if (sFormatPackFlags[fi] & FORMAT_PACK_FLAG_USAGE)
        return ModFormatWins(fmt) >= gModUsagePackWins[0];
    /* TCG sets: only when their campaign format is unlocked (e.g. Metal Raiders with Critter, not Yugi-Kaiba). */
    return ModIsFormatUnlocked(fmt);
}

static int ModPackList_AppendFormatRow(int fi)
{
    u16 row = VANILLA_PACK_INFO_COUNT + fi;

    if (gModPackList.active || ModPackShopIsOpen()) {
        if (gModPackList.count >= MOD_PACK_SHOP_LIST_MAX)
            return 0;
        gModPackList.rows[gModPackList.count++] = row;
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

/* How many format-pack rows are unlocked (TCG sets when their format opens; usage packs at FORMAT_PACK_USAGE_WINS). */
int ModCountUnlockedFormatPacks(void)
{
    int fi, n = 0;

#if !MOD_RELEASE
    return FORMAT_PACK_COUNT;
#endif
    for (fi = 0; fi < FORMAT_PACK_COUNT; fi++) {
        if (ModIsFormatPackUnlocked(fi))
            n++;
    }
    return n;
}

/* The pick-a-pack list, in the Booster Packs shop and after a Campaign win alike (CB_PackShop, CB_RewardPack). */
void ModPackList_AddUnlockedPacks(void)
{
    int fi;

    ModPackShopList_Begin();
    for (fi = 0; fi < FORMAT_PACK_COUNT; fi++) {
        if (ModIsFormatPackUnlocked(fi))
            ModPackList_AppendFormatRow(fi);
    }
    ModPackShopList_SyncSceneWork();
    gModPackList.active = 0;
}

int ModFormatPackLoadCoverGfx(u16 slot, u16 packId)
{
    int fi = FormatPackIndexFromId(packId);
    const u8 *src;
    u16 tile;

    if (fi < 0 || sFormatPackPortraitNumber[fi] == 0)
        return 0;
    src = &gFormatPackCovers[fi * PACK_COVER_BYTES];
    tile = slot * 0x62 + 0x10;
    {
        vu32 *dma = (vu32 *)0x040000D4;
        dma[0] = (u32)src;
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

int ModPackOpenCardNameShines(u16 cardNumber)
{
    u16 packId = gPackOpenWork.openPackId;
    int fi = FormatPackIndexFromId(packId);

    if (fi >= 0) {
        const struct FormatPackPool *pool = &sFormatPackPools[fi];

        if (sFormatPackFlags[fi] & FORMAT_PACK_FLAG_USAGE) {
            int formatOrder = sFormatPackFormatIndex[fi];
            u32 price = GetCardUsagePriceInFormat(formatOrder, cardNumber);

            return price >= USAGE_PACK_OPENER_SHINE_MIN_DP;
        }
        if (WeightForNumber(pool, cardNumber) <= FORMAT_PACK_TCG_SHINE_MAX_WEIGHT)
            return 1;
        return 0;
    }
    return cardNumber == gPackOpenWork.rareCardNumber && gPackOpenWork.rareCardNumber != 9999;
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

/* Random() is 15-bit (see sprite.c); widened to 30 bits for n above 0x8000. */
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

/* The pool entry where the running sum of the weights passes r (0 <= r < the total): each card's chance is
 * weight / total. The pool (data.packs.format.pools in Hex Maniac Advance) is all a pull reads. */
static u16 WeightedPick(const struct FormatPackPool *pool, u32 r)
{
    int i;

    for (i = 0; i < pool->count - 1; i++) {
        if (r < pool->entries[i].weight)
            break;
        r -= pool->entries[i].weight;
    }
    return pool->entries[i].number;
}

/* The pack a Campaign reward opens (GetRewardPack, booster_get_pack.c; every frame of the reward): for an original
 * pack (the event rewards: Duelist Pack, Grandpa Cup prize, Limited Collection, ...; the Weekly Yu-Gi-Oh! and magazine
 * deliveries), a random TCG format pack (not a usage pack) the player has unlocked. It is picked on the reward's
 * first frame (step 0, state 0) and kept in gMain.rewardPack for the rest. 0 (the shop's list) and format packs stay;
 * with no TCG pack unlocked the original pack stays. */
u32 ModRewardPack(u32 packId)
{
    u8 open[FORMAT_PACK_COUNT];
    int fi, n = 0;

    if (packId == 0 || FormatPackIndexFromId(packId) >= 0)
        return packId;
    if (!(gMain.seqIndex1 == 0 && gMain.seqState1 == 0) && FormatPackIndexFromId(gMain.rewardPack) >= 0)
        return gMain.rewardPack;                        /* the pick of this reward */
    for (fi = 0; fi < FORMAT_PACK_COUNT; fi++) {
        if (!(sFormatPackFlags[fi] & FORMAT_PACK_FLAG_USAGE) && ModIsFormatPackUnlocked(fi))
            open[n++] = fi;
    }
    if (n == 0)
        return packId;
    return FORMAT_PACK_ID_FIRST + open[RandomBelow(n)];
}

int ModGenerateFormatPackCards(u16 *cardNumbers, u16 packId)
{
    int fi = FormatPackIndexFromId(packId);
    const struct FormatPackPool *pool;
    u32 total = 0;
    int i;

    if (fi < 0)
        return -1;
    pool = &sFormatPackPools[fi];
    for (i = 0; i < pool->count; i++)
        total += pool->entries[i].weight;
    if (total == 0)
        return -1;
    gPackOpenWork.openPackId = packId;
    for (i = 0; i < 5; i++) {
        u16 n;
        int tries;

        for (tries = 0; tries < 64; tries++) {
            n = WeightedPick(pool, RandomBelow(total));
            if (!CardAlreadyPicked(cardNumbers, i, n))
                break;
        }
        cardNumbers[i] = n;
    }
    for (i = 0; i < 25; i++) {
        int a = Random() % 5;
        int b = Random() % 5;
        u16 t = cardNumbers[a];
        cardNumbers[a] = cardNumbers[b];
        cardNumbers[b] = t;
    }
    gPackOpenWork.rareCardNumber = 9999;
    return 7;
}
