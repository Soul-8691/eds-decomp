#ifndef GUARD_LINK_MONSTER_H
#define GUARD_LINK_MONSTER_H

/*
 * Link Monsters and the Extra Monster Zones (src/mod_link.c; wiki/game/link.md). Not part of the original game.
 * A player's 5 monster records stay; one can stand in an Extra Monster Zone (EMZ), keyed by its zone serial.
 * A Link Monster is kind FUSION in its stats word (the Extra Deck) and CARD_KIND_EX_LINK in gCardTypeEx; its stats
 * Level is its Link Rating and gCardLinkMarkers holds its arrows.
 */

#include "global.h"

struct SummonAction;

/* The pointed-at places ModLinkPointsTo writes: a Main Monster Zone (player * 8 + column 0-4) or an EMZ. */
#define LINK_PLACE_EMZ(slot)    (16 + (slot))
#define LINK_PLACE_IS_EMZ(p)    ((p) >= 16)

/* The EMZ slot (0 screen left, 1 screen right) the monster record (player, zone) stands in, or -1. */
int ModEmzSlotOf(int player, int zone);
/* Field pixel position of monster record (player, zone) (its EMZ when it stands in one); other areas as
 * gDuelZonePositions. GetAreaX / GetAreaY (card_canvas.c), DrawZoneTiles / ClearZoneTiles (duel_field_screen.c). */
s32 ModZoneFieldX(u32 player, u32 zone);
s32 ModZoneFieldY(u32 player, u32 zone);
/* ModXyzZoneCleared (the ClearZone hook): the monster in (player, zone) leaves; its EMZ is free again. */
void ModLinkZoneCleared(int player, int zone);
/* AddedCardsSummoned: `id` arrived in (player, zone); 1 if it was being Link Summoned. */
int ModLinkArrived(int player, int zone, u16 id);
/* Duel start. */
void ModLinkReset(void);

/* The places the face-up Link Monster in (player, zone) points to (LINK_PLACE_*), into out[8]; their count. */
int ModLinkPointsTo(int player, int zone, u8 *out);
/* The monster records (player * 5 + zone) the Link Monster in (player, zone) points to, into out[8]; their count. */
int ModLinkPointedMonsters(int player, int zone, u8 *out);
/* Zone mask of `player`'s empty records whose Main Monster Zone a face-up Link Monster `player` controls points
 * to (only `onlyZone`'s arrows when it is 0-4). */
u32 ModLinkPointedFreeZones(int player, int onlyZone);

/* SummonAction_Update (summon_action.c), before the kind's step machine: the human picks the Monster Zone (or the
 * EMZ) of the summon, and a Link Monster is put in face-up Attack Position. 0 while waiting, else 1. */
u16 ModSummonZoneStep(void);
/* SummonStep_SpecialChoosePosition: the position answer (0 Attack, 1 Defense) for the CPU, and for a Link Monster
 * (always Attack). */
u16 ModSummonDefenseChoice(int player, u16 cardId, int faceUp);
/* The next summon goes to (player's) record `zone` (and EMZ slot `emzSlot`, or -1), with no zone prompt. */
void ModSummonSetZone(int zone, int emzSlot);
/* The next summon's zone pick offers only these Main Monster Zones (a mask; no EMZ): Link Spider. */
void ModSummonRestrictZones(u32 mask);

/* The shared Extra Deck list (src/mod_synchro.c): 1 if `player` can Link Summon `linkId` now. */
int ModLinkCanSummon(int player, u16 linkId);
/* ModSynchroMenuRun, once the human picked a Link Monster: materials, place and summon, one step per call (`start`
 * on the first). 1 when done or cancelled. */
int ModLinkMenuRun(u32 link, int start);
/* AiPlayAddedCards: the CPU Link Summons when the Link Monster beats its strongest material. 1 if it did. */
int AiTryLinkSummon(void);
/* ModFusionKindFrame (src/mod_xyz.c): the Link frame with Link Monster `id`'s arrows painted on (a RAM copy). */
const u8 *ModLinkFrame(u16 id);
/* ModDrawDuelHud: the arrows of every face-up Link Monster, and the zone pick's marker. */
void ModDrawLinkArrows(void);

#endif /* GUARD_LINK_MONSTER_H */
