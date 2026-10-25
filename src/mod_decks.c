/*
 * mod_decks: opponent decks that replace the original ones (gOpponentDecks). Not part of the original ROM.
 * LoadOpponentDeck (ai_deck.c) asks GetOpponentDeckOverride first. Like added_effects, this unit is @farcode and
 * @rodata at the end in units.txt, so it does not move the game's code or data.
 */

#include "global.h"
#include "ai.h"
#include "constants/cards.h"
#include "constants/game.h"

/* Yugi Muto: Magic and Trap cards only, none of them strong: small burn, healing, Field Magic and simple Traps.
 * At most 3 copies of each and none on the Forbidden/Limited list, so RemoveOverLimitDeckCards keeps them all. */
static const u16 sYugiDeck[] = {
    CARD_SPARKS, CARD_SPARKS, CARD_SPARKS,
    CARD_HINOTAMA, CARD_HINOTAMA, CARD_HINOTAMA,
    CARD_OOKAZI, CARD_OOKAZI,
    CARD_DIAN_KETO_THE_CURE_MASTER, CARD_DIAN_KETO_THE_CURE_MASTER, CARD_DIAN_KETO_THE_CURE_MASTER,
    CARD_RED_MEDICINE, CARD_RED_MEDICINE, CARD_RED_MEDICINE,
    CARD_SOUL_OF_THE_PURE, CARD_SOUL_OF_THE_PURE,
    CARD_GOBLINS_SECRET_REMEDY, CARD_GOBLINS_SECRET_REMEDY,
    CARD_YAMI, CARD_YAMI,
    CARD_SOGEN, CARD_SOGEN,
    CARD_UMI, CARD_UMI,
    CARD_FOREST, CARD_FOREST,
    CARD_TRAP_HOLE, CARD_TRAP_HOLE,
    CARD_REINFORCEMENTS, CARD_REINFORCEMENTS, CARD_REINFORCEMENTS,
    CARD_CASTLE_WALLS, CARD_CASTLE_WALLS, CARD_CASTLE_WALLS,
    CARD_WABOKU, CARD_WABOKU, CARD_WABOKU,
    CARD_FAKE_TRAP, CARD_FAKE_TRAP, CARD_FAKE_TRAP,
};

static const struct DeckList sYugiDeckList = { sYugiDeck, ARRAY_COUNT(sYugiDeck), 0 };

STATIC_ASSERT(ARRAY_COUNT(sYugiDeck) == 40, YugiDeckSize);

/* The replacement deck of opponent `opponent` (enum DuelistId), or NULL to keep the original. */
const struct DeckList *GetOpponentDeckOverride(int opponent)
{
    switch (opponent) {
    case DUELIST_YUGI:
        return &sYugiDeckList;
    }
    return NULL;
}
