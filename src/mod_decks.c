/*
 * mod_decks: decks that replace the original ones. Not part of the original ROM.
 *   GetOpponentDeckOverride   opponent decks (gOpponentDecks); LoadOpponentDeck (ai_deck.c) asks it first
 *   BuildModStarterDeck       starter decks; BuildStarterDeck (title_screen.c) asks it first
 *   GiveStartingTrunk         the cards a new game starts with in the trunk (BuildStarterDeck)
 * Like added_effects, this unit is @farcode and @rodata at the end in units.txt, so it does not move the game's
 * code or data. Decks keep to the Forbidden/Limited list (gCardCopyLimits): LoadOpponentDeck drops every copy of
 * a card that is over its limit, and AddCardToSavedDeck refuses the copies over it.
 */

#include "global.h"
#include "ai.h"
#include "card_data.h"
#include "save.h"
#include "constants/cards.h"
#include "constants/game.h"
#include "booster.h"

#define X2(card) card, card
#define X3(card) card, card, card

/* Yugi Muto: a competitive deck: strong Normal monsters, the classic floaters and searchers, and the best Magic
 * and Trap cards at their limits. */
static const u16 sYugiDeck[] = {
    /* Monsters (19) */
    CARD_JINZO,
    X3(CARD_SUMMONED_SKULL),
    X3(CARD_VORSE_RAIDER),
    X2(CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP),
    X2(CARD_SANGAN),
    X2(CARD_WITCH_OF_THE_BLACK_FOREST),
    X2(CARD_MAN_EATER_BUG),
    X2(CARD_MYSTIC_TOMATO),
    CARD_SINISTER_SERPENT,
    CARD_CYBER_JAR,
    /* Magic (15) */
    CARD_RAIGEKI,
    CARD_DARK_HOLE,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_MONSTER_REBORN,
    CARD_POT_OF_GREED,
    CARD_CHANGE_OF_HEART,
    CARD_SNATCH_STEAL,
    CARD_HEAVY_STORM,
    X2(CARD_SWORDS_OF_REVEALING_LIGHT),
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    X2(CARD_GRACEFUL_CHARITY),
    /* Traps (6) */
    CARD_MIRROR_FORCE,
    CARD_CALL_OF_THE_HAUNTED,
    X3(CARD_TRAP_HOLE),
    CARD_WABOKU,
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

/* Copies of every card a new game puts in the trunk. */
#define STARTING_TRUNK_COPIES 8

/* A new game starts with STARTING_TRUNK_COPIES of every card in the trunk: all card IDs, original and added, except
 * the monster tokens (card numbers 1920-1999), the same cards as the debug "Get all card" (DebugGetAllCards). */
void GiveStartingTrunk(void)
{
    u32 id, k;

    for (id = 1; id <= CARD_ID_LAST; id++) {
        u16 number = gCardIdToNumber[id];
        if (number == 0xFFFF || (u16)(number - CARD_NUMBER_TOKEN_FIRST) < CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST)
            continue;
        for (k = 0; k < STARTING_TRUNK_COPIES; k++)
            AddCardToTrunk(id);
    }
}

#if defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR)
/* The Red starter deck: Black Luster Soldier - Envoy of the Beginning with LIGHT and DARK monsters to banish for
 * it: strong Normal monsters, the LIGHT/DARK floaters Shining Fairy and Mystic Tomato, Breaker the Magical Warrior,
 * and Dark Hole (limited: 1 in the deck). 40 cards. */
static const u16 sRedStarterDeck[] = {
    X3(X2(CARD_CHAOS_SORCERER)), X2(CARD_CHAOS_SORCERER),
    X3(CARD_BREAKER_THE_MAGICAL_WARRIOR),
    X3(CARD_SHINING_FAIRY),
    X3(CARD_MYSTIC_TOMATO),
    CARD_DARK_HOLE,
    /* Normal monsters: level 4 */
    X3(CARD_VORSE_RAIDER),                          /* DARK 1900 */
    X3(CARD_DUNAMES_DARK_WITCH),                    /* LIGHT 1800 */
    X3(CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP), /* DARK 1800 */
    X3(CARD_NEO_THE_MAGIC_SWORDSMAN),               /* LIGHT 1700 */
    X2(CARD_MECHANICALCHASER),                      /* DARK 1850 */
    X2(CARD_GUARDIAN_OF_THE_THRONE_ROOM),           /* LIGHT 1650 */
    /* Normal monsters: Tribute Summons */
    X2(CARD_SUMMONED_SKULL),                        /* DARK 2500, level 6 */
    X2(CARD_BLUE_EYES_WHITE_DRAGON),                /* LIGHT 3000, level 8 */
    CARD_DARK_MAGICIAN,                             /* DARK 2500, level 7 */
    CARD_RED_EYES_B_DRAGON,                         /* DARK 2400, level 7 */
};

STATIC_ASSERT(ARRAY_COUNT(sRedStarterDeck) == 40, RedStarterDeckSize);

/* Build the saved Deck of starter deck `choice` (enum StarterDeck) when this build replaces it; returns 1 then,
 * 0 to let BuildStarterDeck build the original. */
int BuildModStarterDeck(int choice)
{
    u32 i;

    if (choice != STARTER_DECK_RED)
        return 0;
    for (i = 0; i < ARRAY_COUNT(sRedStarterDeck); i++)
        AddCardToSavedDeck(gCardNumberToId[sRedStarterDeck[i] & CARD_ID_MASK]);
    return 1;
}
#else
int BuildModStarterDeck(int choice)
{
    return 0;
}
#endif
