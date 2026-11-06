#ifndef GUARD_XYZ_H
#define GUARD_XYZ_H

/*
 * Xyz Summoning (src/mod_xyz.c; wiki/game/xyz.md). Not part of the original game.
 * An Xyz Monster is kind FUSION in its stats word (the Extra Deck) and CARD_KIND_EX_XYZ in gCardTypeEx; its stats
 * Level is its Rank. Its Xyz Materials are kept in gModXyz, by the zone serial of the monster they are attached to.
 */

#include "global.h"

struct DuelCard;

/* The cardNumber of CollectEffectTargets / CardListView_Open for the list of one Xyz Monster's materials (the
 * detach pick; ModXyzMaterialView). No card has this number. */
#define CARD_LIST_KEY_XYZ_MATERIALS 0x7FF1

/* 1 if `player` can Xyz Summon the Extra Deck card `xyzId` now (its materials are on the field). */
int ModXyzCanSummon(int player, u16 xyzId);
/* ModSynchroMenuRun (src/mod_synchro.c), once the human picked an Xyz Monster (card word `xyz`) from the Extra Deck
 * list: the material picks and the summon, one step per call (`start` on the first). 1 when done or cancelled. */
int ModXyzMenuRun(u32 xyz, int start);
/* AiPlayAddedCards: the CPU Xyz Summons when the Xyz Monster beats its strongest material. 1 if it did. */
int AiTryXyzSummon(void);
/* AddedCardsSummoned: `id` arrived in (player, zone); 1 if it was being Xyz Summoned (its materials now attach). */
int ModXyzArrived(int player, int zone, u16 id);
/* ClearZone (duel_card_lists.c): the monster in (player, zone) leaves the field; its materials go to the
 * Graveyard. */
void ModXyzZoneCleared(int player, int zone);
/* Duel start (ModDuelInit, src/added_effects.c). */
void ModXyzReset(void);

/* The number of Xyz Materials of the monster in (player, zone). */
int XyzMaterialCount(int player, int zone);
/* Detach 1 material of (player, zone). 1 if it is done (the CPU's lowest-ATK one, or the human's only choice); 2 if
 * the human's list is open (CARD_LIST_KEY_XYZ_MATERIALS): call XyzDetachPicked after it closes; 0 if none. */
int XyzDetachBegin(int player, int zone);
/* Detach the material picked in the open list (gCardListView's cursor entry) from (player, zone). */
void XyzDetachPicked(int player, int zone);
/* The materials the detach list shows (CollectEffectTargets' CARD_LIST_KEY_XYZ_MATERIALS case). */
const struct DuelCard *ModXyzMaterialView(int *count);

/* An Xyz Monster's "once per turn" effect: 1 if the monster in (player, zone) used it this turn; Mark records it. */
int ModXyzEffectUsed(int player, int zone);
void ModXyzMarkEffectUsed(int player, int zone);

#endif /* GUARD_XYZ_H */
