/*
 * mod_save: save-data helpers of this build. Not part of the original ROM.
 *
 * The per-deck copy counters of struct TrunkEntry (save.h) are 2 bits in the original game, which caps a card at 3
 * copies per list. They keep those 2 bits as their low bits and have high bits in the bytes the original game
 * never used, so a card can fill a whole list. Every reader and writer goes through these functions. The saved-deck
 * functions that use them (IsBelowCardCopyLimit, AddCardToSaved*, RemoveCardFromSaved*, TrimSavedDecksToCopyLimits)
 * moved here from collection.c. This unit is @farcode in units.txt, so it does not grow the game's code.
 */

#include "global.h"
#include "card_data.h"
#include "constants/cards.h"
#include "save.h"

/* From collection.c: the debug check of a card ID. */
void DebugCheckCardId(u16 id);

/* Single entries of gCardNumberToId read through their own symbols (card_data.h lists them). */
extern const u16 gCardNumberToId_FlameSwordsman[];
extern const u16 gCardNumberToId_DarkMagician[];
extern const u16 gCardNumberToId_GaiaTheFierceKnight[];
extern const u16 gCardNumberToId_CelticGuardian[];
extern const u16 gCardNumberToId_HarpieLady[];
extern const u16 gCardNumberToId_TigerAxe[];
extern const u16 gCardNumberToId_ThousandDragon[];
extern const u16 gCardNumberToId_PendulumMachine[];
extern const u16 gCardNumberToId_LauncherSpider[];
extern const u16 gCardNumberToId_Polymerization[];
extern const u16 gCardNumberToId_1210[];
extern const u16 gCardNumberToId_1249[];

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

/* ---- Saved decks (moved from collection.c) ---- */

/* Returns 1 if the player owns fewer copies of card `id` (plus its related cards) than the
 * limit. Prints that share a limit (alt arts, the Dark Magician and Harpie Lady variants)
 * pool their Deck/Side Deck copies; the Fusion Deck copies pool only for the cases that use
 * ADD_N3. */
#define ADD_PAIR(x) do { u16 rel = (x); sum += GetTrunkDeckCopies(rel) + GetTrunkSideCopies(rel); } while (0)
#define ADD_N3(x) sum += GetTrunkFusionCopies(x)
#define NUMBER_TO_ID(n) (gCardNumberToId[(n) & CARD_ID_MASK])
u32 IsBelowCardCopyLimit(u16 id) {
    s32 sum = GetTrunkDeckCopies(id) + GetTrunkSideCopies(id) + GetTrunkFusionCopies(id);
    s32 limit = GetCardCopyLimit(id);
    u16 key = gCardIdToNumber[id & CARD_ID_MASK];
    switch (key) {
    case CARD_POLYMERIZATION:
        ADD_PAIR(NUMBER_TO_ID(CARD_POLYMERIZATION + 0x1F));
        break;
    case CARD_POLYMERIZATION_ALT:
        ADD_PAIR(gCardNumberToId_Polymerization[0]);
        break;
    case CARD_DARK_MAGICIAN:
        ADD_PAIR(gCardNumberToId_1210[0]);
        ADD_PAIR((u16)(gCardNumberToId_DarkMagician[0] + 1));
        break;
    case CARD_1210:
        ADD_PAIR(gCardNumberToId_DarkMagician[0]);
        ADD_PAIR(NUMBER_TO_ID(CARD_1210));
        break;
    case CARD_DARK_MAGICIAN_ALT:
        ADD_PAIR(gCardNumberToId_DarkMagician[0]);
        ADD_PAIR((u16)(gCardNumberToId_DarkMagician[0] + 1));
        break;
    case CARD_HARPIE_LADY:
        ADD_PAIR(gCardNumberToId_1249[0]);
        break;
    case 1249: /* no EDS card (no CARD_ name); pairs with CARD_HARPIE_LADY */
        ADD_PAIR(gCardNumberToId_HarpieLady[0]);
        break;
    case CARD_BLUE_EYES_WHITE_DRAGON:
        ADD_PAIR((u16)(NUMBER_TO_ID(0) + 1));
        break;
    case CARD_FLAME_SWORDSMAN:
        ADD_N3((u16)(gCardNumberToId_FlameSwordsman[0] + 1));
        break;
    case CARD_GAIA_THE_FIERCE_KNIGHT:
        ADD_PAIR((u16)(gCardNumberToId_GaiaTheFierceKnight[0] + 1));
        break;
    case CARD_CELTIC_GUARDIAN:
        ADD_PAIR((u16)(gCardNumberToId_CelticGuardian[0] + 1));
        break;
    case CARD_TIGER_AXE:
        ADD_PAIR((u16)(gCardNumberToId_TigerAxe[0] + 1));
        break;
    case CARD_THOUSAND_DRAGON:
        ADD_N3((u16)(gCardNumberToId_ThousandDragon[0] + 1));
        break;
    case CARD_PENDULUM_MACHINE:
        ADD_PAIR((u16)(gCardNumberToId_PendulumMachine[0] + 1));
        break;
    case CARD_LAUNCHER_SPIDER:
        ADD_PAIR((u16)(gCardNumberToId_LauncherSpider[0] + 1));
        break;
    case CARD_BLUE_EYES_WHITE_DRAGON_ALT:
        ADD_PAIR(NUMBER_TO_ID(0));
        break;
    case CARD_FLAME_SWORDSMAN_ALT:
        ADD_N3(gCardNumberToId_FlameSwordsman[0]);
        break;
    case CARD_GAIA_THE_FIERCE_KNIGHT_ALT:
        ADD_PAIR(gCardNumberToId_GaiaTheFierceKnight[0]);
        break;
    case CARD_CELTIC_GUARDIAN_ALT:
        ADD_PAIR(gCardNumberToId_CelticGuardian[0]);
        break;
    case CARD_TIGER_AXE_ALT:
        ADD_PAIR(gCardNumberToId_TigerAxe[0]);
        break;
    case CARD_THOUSAND_DRAGON_ALT:
        ADD_N3(gCardNumberToId_ThousandDragon[0]);
        break;
    case CARD_PENDULUM_MACHINE_ALT:
        ADD_PAIR(gCardNumberToId_PendulumMachine[0]);
        break;
    case CARD_LAUNCHER_SPIDER_ALT:
        ADD_PAIR(gCardNumberToId_LauncherSpider[0]);
        break;
    }
    return sum < limit;
}
#undef ADD_PAIR
#undef ADD_N3
#undef NUMBER_TO_ID

/* The three Add functions append one copy of `id` to a saved list when the card is below its copy limit in
 * that list and the list has room. */
void AddCardToSavedDeck(u16 id) {
    u32 copies;
    DebugCheckCardId(id);
    copies = GetTrunkDeckCopies(id);
    if (copies < GetCardCopyLimit(id) && gSaveData.deckSize < DECK_MAX_CARDS) {
        SetTrunkDeckCopies(id, copies + 1);
        gSaveData.deck[gSaveData.deckSize++] = id;
    }
}
void AddCardToSavedSideDeck(u16 id) {
    u32 copies;
    DebugCheckCardId(id);
    copies = GetTrunkSideCopies(id);
    if (copies < GetCardCopyLimit(id) && gSaveData.sideDeckSize < SIDE_DECK_MAX_CARDS) {
        SetTrunkSideCopies(id, copies + 1);
        gSaveData.sideDeck[gSaveData.sideDeckSize++] = id;
    }
}
void AddCardToSavedFusionDeck(u16 id) {
    u32 copies;
    DebugCheckCardId(id);
    copies = GetTrunkFusionCopies(id);
    if (copies < GetCardCopyLimit(id) && gSaveData.fusionDeckSize < FUSION_DECK_MAX_CARDS) {
        SetTrunkFusionCopies(id, copies + 1);
        gSaveData.fusionDeck[gSaveData.fusionDeckSize++] = id;
    }
}

/* Remove the first `id` from list[0..*size) and close the gap. */
static void RemoveFromSavedList(u16 *list, u16 *size, u16 id)
{
    int i;
    for (i = 0; i < *size; i++) {
        if (list[i] == id) {
            (*size)--;
            for (; i < *size; i++)
                list[i] = list[i + 1];
            return;
        }
    }
}
/* The three Remove functions take one copy of `id` out of a saved list (its counter and the list entry). */
void RemoveCardFromSavedDeck(u16 id)
{
    u32 copies;
    DebugCheckCardId(id);
    copies = GetTrunkDeckCopies(id);
    if (copies != 0) {
        SetTrunkDeckCopies(id, copies - 1);
        RemoveFromSavedList(gSaveData.deck, &gSaveData.deckSize, id);
    }
}
void RemoveCardFromSavedSideDeck(u16 id)
{
    u32 copies;
    DebugCheckCardId(id);
    copies = GetTrunkSideCopies(id);
    if (copies != 0) {
        SetTrunkSideCopies(id, copies - 1);
        RemoveFromSavedList(gSaveData.sideDeck, &gSaveData.sideDeckSize, id);
    }
}
void RemoveCardFromSavedFusionDeck(u16 id)
{
    u32 copies;
    DebugCheckCardId(id);
    copies = GetTrunkFusionCopies(id);
    if (copies != 0) {
        SetTrunkFusionCopies(id, copies - 1);
        RemoveFromSavedList(gSaveData.fusionDeck, &gSaveData.fusionDeckSize, id);
    }
}

/* Trim the Deck + Side Deck copies of every card in `list` to its copy limit, moving the excess (Side Deck
 * copies first) back to the trunk. */
static void TrimSavedListToCopyLimits(u16 *list, u16 *size)
{
    u32 i = 0;
    while (i < *size) {
        u16 id = list[i];
        s32 limit = GetCardCopyLimit(id);
        int removed = 0;
        while ((s32)(GetTrunkDeckCopies(id) + GetTrunkSideCopies(id)) > limit) {
            if (GetTrunkSideCopies(id) != 0)
                RemoveCardFromSavedSideDeck(id);
            else
                RemoveCardFromSavedDeck(id);
            AddCardToTrunk(id);
            removed = 1;
        }
        if (!removed)       /* a removal may have moved another card to index i: look at it again */
            i++;
    }
}
/* Trim the saved Deck and Side Deck copy counts to the card-specific limits. */
void TrimSavedDecksToCopyLimits(void)
{
    TrimSavedListToCopyLimits(gSaveData.deck, &gSaveData.deckSize);
    TrimSavedListToCopyLimits(gSaveData.sideDeck, &gSaveData.sideDeckSize);
}
