#ifndef GUARD_FORMAT_PACKS_H
#define GUARD_FORMAT_PACKS_H

#include "global.h"
#include "booster.h"

/* Pack shop TCG set packs (601..). FORMAT_PACK_COUNT is in build/include/format_pack_data.h. */
#define FORMAT_PACK_ID_FIRST  601
#define VANILLA_PACK_INFO_COUNT 23
#define FORMAT_PACK_COST_TCG    250
#define FORMAT_PACK_COST_USAGE  500

int FormatPackIndexFromId(u16 packId);
int ModFormatPackShopCost(u16 packId, u32 *costOut);
u16 ModGetSelectedPackId(u16 packRowIndex);
const struct PackInfo *ModPackInfoAtRow(u16 packRowIndex);
void ModPackList_AddUnlockedPacks(void);
int ModPackList_AddFormatPackById(u16 packId);
int ModFormatPackLoadCoverGfx(u16 slot, u16 packId);
int ModGenerateFormatPackCards(u16 *cardNumbers, u16 packId);
int ModPackShopIsOpen(void);

/* Shop list may exceed gSceneWork.packRows[0x20]; when active, rows live in mod EWRAM. */
u16 ModPackList_Count(void);
u16 ModPackList_RowAt(u16 index);
void ModPackShopList_Begin(void);
void ModPackShopList_Reset(void);
void ModPackShopList_SyncSceneWork(void);

#endif /* GUARD_FORMAT_PACKS_H */
