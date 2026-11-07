#ifndef GUARD_FORMAT_PACKS_H
#define GUARD_FORMAT_PACKS_H

#include "global.h"
#include "booster.h"

/* Pack shop TCG set packs (601..). FORMAT_PACK_COUNT is in build/include/format_pack_data.h. */
#define FORMAT_PACK_ID_FIRST  601
#define VANILLA_PACK_INFO_COUNT 23
#define FORMAT_PACK_COST_TCG    250
#define FORMAT_PACK_COST_USAGE  500
/* Usage pack opener: rainbow name on pulls priced at least this much in that pack's format (cardprices.py). */
#define USAGE_PACK_OPENER_SHINE_MIN_DP  500

int FormatPackIndexFromId(u16 packId);
int ModFormatPackShopCost(u16 packId, u32 *costOut);
u16 ModGetSelectedPackId(u16 packRowIndex);
const struct PackInfo *ModPackInfoAtRow(u16 packRowIndex);
void ModPackList_AddUnlockedPacks(void);
int ModCountUnlockedFormatPacks(void);
int ModPackList_AddFormatPackById(u16 packId);
int ModFormatPackLoadCoverGfx(u16 slot, u16 packId);
/* Usage-table DP for one card in format order (BANLISTS index); 0 if that format's table has no row. */
u32 GetCardUsagePriceInFormat(int formatOrder, u16 cardNumber);
int ModGenerateFormatPackCards(u16 *cardNumbers, u16 packId);
/* GetRewardPack (booster_get_pack.c): the pack a Campaign reward opens instead of an original pack (events, the
 * magazines): a random TCG format pack the player has unlocked, picked on the reward's first frame and kept. */
u32 ModRewardPack(u32 packId);
/* Pack reveal: rainbow name colour (palette 15) for this card number in the opened pack. */
int ModPackOpenCardNameShines(u16 cardNumber);
int ModPackShopIsOpen(void);

/* Shop list may exceed gSceneWork.packRows[0x20]; when active, rows live in mod EWRAM. */
u16 ModPackList_Count(void);
u16 ModPackList_RowAt(u16 index);
void ModPackShopList_Begin(void);
void ModPackShopList_Reset(void);
void ModPackShopList_SyncSceneWork(void);

#endif /* GUARD_FORMAT_PACKS_H */
