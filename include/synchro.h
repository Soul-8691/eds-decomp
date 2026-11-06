#ifndef GUARD_SYNCHRO_H
#define GUARD_SYNCHRO_H

/*
 * Synchro Summoning (src/mod_synchro.c; wiki/game/synchro.md). Not part of the original game.
 * The card types are in gCardTypeEx (include/card_data.h); a Synchro Monster lives in the Fusion Deck (the Extra
 * Deck), being kind FUSION in its stats word.
 */

#include "global.h"

/* The cardNumber of CollectEffectTargets / CardListView_Open for the list of the Synchro Monsters the player can
 * Synchro Summon now (no card has this number). */
#define CARD_LIST_KEY_SYNCHRO   0x7FF0

/* ModCollectEffectTargets (src/added_effects.c): 1 if the Fusion Deck card `id` of `player` can be Synchro Summoned. */
int ModSynchroListAccepts(u16 id, int player);
/* CardMenu_GetAvailableCommands (card_menu_input.c), the Fusion Deck pile: 1 if `player` can Synchro Summon. */
int ModSynchroAvailable(int player);
/* CardMenu_Execute (duel_cmd_queue.c), the Special Summon command on the Fusion Deck: the human's Synchro Summon,
 * one step per call; clears gDuel.cardMenu.confirmed when done. */
void ModSynchroMenuRun(void);
/* AiPlayAddedCards (src/added_effects.c): the CPU Synchro Summons when it gains by it. 1 if it did. */
int AiTrySynchroSummon(void);
/* 1 if the monster in (player, zone) was Synchro Summoned (Junk Warrior's trigger, the revival rule). */
int ModWasSynchroSummoned(int player, int zone);
/* 1 if card `id` was Synchro Summoned into (player, zone): the zone need not still hold it (a battle's copy). */
int ModWasSynchroSummonedId(int player, int zone, u16 id);
/* Duel start (ModDuelReset, src/added_effects.c): clear the Synchro state. */
void ModSynchroReset(void);
/* AddedCardsSummoned: `id` arrived in (player, zone); 1 if it was being Synchro Summoned (now marked). */
int ModSynchroArrived(int player, int zone, u16 id);
/* The Synchro Monster being summoned right now (card ID, 0 none): AddedCardsSummoned's Synchro Summon triggers. */
u16 ModSynchroSummoningId(void);
/* Card View's type line of an added monster (CardDetail_DrawInfo, title_menu.c). */
void ModCardTypeLine(u16 cardId, char *buf);
/* The race name of a monster (its real race when EDS lacks it). */
const char *ModCardRaceName(u16 id);

#endif /* GUARD_SYNCHRO_H */
