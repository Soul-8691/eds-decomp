/*
 * mod_save: save-data helpers of this build. Not part of the original ROM.
 *
 * The per-deck copy counters of struct TrunkEntry (save.h) are 2 bits in the original game, which caps a card at 3
 * copies per list. They keep those 2 bits as their low bits and have high bits in the bytes the original game
 * never used, so a card can fill a whole list. Every reader and writer goes through these functions. This unit is
 * @farcode in units.txt, so it does not grow the game's code.
 */

#include "global.h"
#include "save.h"

u32 GetTrunkDeckCopies(u16 id)
{
    struct TrunkEntry *e = &gSaveData.trunk[id];
    return e->deckCopies | e->deckCopiesHi << 2;
}

u32 GetTrunkSideCopies(u16 id)
{
    struct TrunkEntry *e = &gSaveData.trunk[id];
    return e->sideCopies | e->sideCopiesHi << 2;
}

u32 GetTrunkFusionCopies(u16 id)
{
    struct TrunkEntry *e = &gSaveData.trunk[id];
    return e->fusionCopies | e->fusionCopiesHi << 2;
}

void SetTrunkDeckCopies(u16 id, u32 copies)
{
    struct TrunkEntry *e = &gSaveData.trunk[id];
    e->deckCopies = copies & 3;
    e->deckCopiesHi = copies >> 2;
}

void SetTrunkSideCopies(u16 id, u32 copies)
{
    struct TrunkEntry *e = &gSaveData.trunk[id];
    e->sideCopies = copies & 3;
    e->sideCopiesHi = copies >> 2;
}

void SetTrunkFusionCopies(u16 id, u32 copies)
{
    struct TrunkEntry *e = &gSaveData.trunk[id];
    e->fusionCopies = copies & 3;
    e->fusionCopiesHi = copies >> 2;
}
