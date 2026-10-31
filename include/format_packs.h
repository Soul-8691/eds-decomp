#ifndef GUARD_FORMAT_PACKS_H
#define GUARD_FORMAT_PACKS_H

#include "global.h"
#include "booster.h"

/* Pack shop format packs (601..615). See tools/format_packs.py. */
#define FORMAT_PACK_ID_FIRST  601
#define FORMAT_PACK_COUNT     15
#define VANILLA_PACK_INFO_COUNT 23

int FormatPackIndexFromId(u16 packId);
u16 ModGetSelectedPackId(u16 packRowIndex);
const struct PackInfo *ModPackInfoAtRow(u16 packRowIndex);
void ModPackList_AddUnlockedPacks(void);
int ModPackList_AddFormatPackById(u16 packId);
int ModFormatPackLoadCoverGfx(u16 slot, u16 packId);
int ModGenerateFormatPackCards(u16 *cardNumbers, u16 packId);
int ModPackShopIsOpen(void);

#endif /* GUARD_FORMAT_PACKS_H */
