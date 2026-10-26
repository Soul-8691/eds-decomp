/*
 * mod_decks: decks that replace the original ones. Not part of the original ROM.
 *   GetOpponentDeckOverride   opponent decks (gOpponentDecks); LoadOpponentDeck (ai_deck.c) asks it first
 *   BuildModStarterDeck       starter decks (Red, Green, Black); BuildStarterDeck (title_screen.c) asks it first
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

/* Joey Wheeler: the "sdlkilla-Warrior" deck (sdlkilla-Warrior.ydk, main deck only): Warriors with Reinforcement of the
 * Army, D.D. Warrior Lady, Blade Knight, Exiled Force and Ninja Grandmaster Sasuke, and a Trap line-up. */
#if defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_BLADE_KNIGHT) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_DON_ZALOOG) \
    && defined(HAS_CARD_EXILED_FORCE) \
    && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) \
    && defined(HAS_CARD_NINJA_GRANDMASTER_SASUKE) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_JAR_OF_GREED) \
    && defined(HAS_CARD_SAKURETSU_ARMOR) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_TRAP_DUSTSHOOT)
#define MOD_JOEY_DECK 1
static const u16 sJoeyDeck[] = {
    /* Monsters (13) */
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    X3(CARD_BLADE_KNIGHT),
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_D_D_WARRIOR_LADY,
    CARD_DON_ZALOOG,
    CARD_EXILED_FORCE,
    X2(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_MYSTIC_SWORDSMAN_LV2,
    CARD_NINJA_GRANDMASTER_SASUKE,
    CARD_TRIBE_INFECTING_VIRUS,
    /* Spells (11) */
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HEAVY_STORM,
    CARD_MYSTICAL_SPACE_TYPHOON,
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    CARD_SNATCH_STEAL,
    /* Traps (16) */
    X2(CARD_JAR_OF_GREED),
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    X3(CARD_SAKURETSU_ARMOR),
    X3(CARD_SOLEMN_JUDGMENT),
    CARD_TORRENTIAL_TRIBUTE,
    X2(CARD_TRAP_DUSTSHOOT),
    X2(CARD_DUST_TORNADO),
    CARD_CALL_OF_THE_HAUNTED,
};
STATIC_ASSERT(ARRAY_COUNT(sJoeyDeck) == 40, JoeyDeckSize);
#endif
#ifdef MOD_JOEY_DECK
static const struct DeckList sJoeyDeckList = { sJoeyDeck, ARRAY_COUNT(sJoeyDeck), 0 };
#endif

/* Bakura: the "geistD-Goat Control" deck (geistD-Goat Control.ydk, main and extra deck): Scapegoat and Metamorphosis into
 * Thousand-Eyes Restrict, flip monsters and Tsukuyomi, Black Luster Soldier and Dark Magician of Chaos. */
#if defined(HAS_CARD_ABYSS_SOLDIER) \
    && defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_DARK_MAGICIAN_OF_CHAOS) \
    && defined(HAS_CARD_MAGICAL_MERCHANT) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_TSUKUYOMI) \
    && defined(HAS_CARD_CREATURE_SWAP) \
    && defined(HAS_CARD_METAMORPHOSIS) \
    && defined(HAS_CARD_SCAPEGOAT) \
    && defined(HAS_CARD_JAR_OF_GREED) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_TRAP_DUSTSHOOT) \
    // && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    // && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    // && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    // && defined(HAS_CARD_GATLING_DRAGON) \
    // && defined(HAS_CARD_KING_DRAGUN) \
    // && defined(HAS_CARD_OJAMA_KING) \
    // && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    // && defined(HAS_CARD_RYU_SENSHI) \
    // && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT)
#define MOD_BAKURA_DECK 1
static const u16 sBakuraDeck[] = {
    /* Monsters (15) */
    CARD_ABYSS_SOLDIER,
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_CYBER_JAR,
    CARD_D_D_WARRIOR_LADY,
    CARD_DARK_MAGICIAN_OF_CHAOS,
    X2(CARD_MAGICAL_MERCHANT),
    X2(CARD_MAGICIAN_OF_FAITH),
    CARD_MORPHING_JAR,
    CARD_SANGAN,
    CARD_SINISTER_SERPENT,
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_TSUKUYOMI,
    /* Spells (14) */
    CARD_CREATURE_SWAP,
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HEAVY_STORM,
    X3(CARD_METAMORPHOSIS),
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    X2(CARD_SCAPEGOAT),
    CARD_SNATCH_STEAL,
    /* Traps (11) */
    X3(CARD_JAR_OF_GREED),
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    X3(CARD_SOLEMN_JUDGMENT),
    CARD_TORRENTIAL_TRIBUTE,
    X2(CARD_TRAP_DUSTSHOOT),
    /* Fusion Deck (15), the .ydk's extra deck: LoadOpponentDeck (AddCardNumberToDeckTop) puts Fusion Monsters there */
    // CARD_DARK_BALTER_THE_TERRIBLE,
    // CARD_DARK_BLADE_THE_DRAGON_KNIGHT,
    // CARD_DARKFIRE_DRAGON,
    // CARD_DRAGONESS_THE_WICKED_KNIGHT,
    // CARD_FIEND_SKULL_DRAGON,
    // CARD_GATLING_DRAGON,
    // CARD_GILTIA_THE_D_KNIGHT,
    // CARD_KING_DRAGUN,
    // CARD_OJAMA_KING,
    // CARD_REAPER_ON_THE_NIGHTMARE,
    // CARD_RYU_SENSHI,
    // CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET,
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sBakuraDeck) == 40 + 3, BakuraDeckSize);
#endif
#ifdef MOD_BAKURA_DECK
static const struct DeckList sBakuraDeckList = { sBakuraDeck, ARRAY_COUNT(sBakuraDeck), 0 };
#endif

/* The Red starter deck and Tea: the "LR-Chaos Turbo" deck (LR-Chaos Turbo.ydk, main deck only): Black Luster Soldier and
 * Chaos Sorcerer fed by LIGHT and DARK flip monsters, Thunder Dragon and the discard/draw Spells. */
#if defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_CHAOS_SORCERER) \
    && defined(HAS_CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE) \
    && defined(HAS_CARD_GRAVEKEEPERS_SPY) \
    && defined(HAS_CARD_NIGHT_ASSAILANT) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_TSUKUYOMI) \
    && defined(HAS_CARD_BOOK_OF_MOON) \
    && defined(HAS_CARD_CARD_DESTRUCTION) \
    && defined(HAS_CARD_MIND_CONTROL) \
    && defined(HAS_CARD_JAR_OF_GREED) \
    && defined(HAS_CARD_PHOENIX_WING_WIND_BLAST) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_TRAP_DUSTSHOOT)
#define MOD_RED_DECK 1
static const u16 sRedStarterDeck[] = {
    /* Monsters (20) */
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    X2(CARD_CHAOS_SORCERER),
    X2(CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE),
    X3(CARD_GRAVEKEEPERS_SPY),
    X2(CARD_MAGICIAN_OF_FAITH),
    X2(CARD_NIGHT_ASSAILANT),
    CARD_SANGAN,
    CARD_SINISTER_SERPENT,
    X3(CARD_THUNDER_DRAGON),
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_TSUKUYOMI,
    /* Spells (13) */
    CARD_BOOK_OF_MOON,
    CARD_CARD_DESTRUCTION,
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HEAVY_STORM,
    CARD_MIND_CONTROL,
    CARD_MYSTICAL_SPACE_TYPHOON,
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_POT_OF_GREED,
    CARD_SNATCH_STEAL,
    X2(CARD_UPSTART_GOBLIN),
    /* Traps (7) */
    CARD_JAR_OF_GREED,
    CARD_MIRROR_FORCE,
    X2(CARD_PHOENIX_WING_WIND_BLAST),
    CARD_BELL_OF_DESTRUCTION,
    CARD_TORRENTIAL_TRIBUTE,
    CARD_TRAP_DUSTSHOOT,
};
STATIC_ASSERT(ARRAY_COUNT(sRedStarterDeck) == 40, RedStarterDeckSize);
#endif
#ifdef MOD_RED_DECK
static const struct DeckList sRedDeckList = { sRedStarterDeck, ARRAY_COUNT(sRedStarterDeck), 0 };
#endif

/* The replacement deck of opponent `opponent` (enum DuelistId), or NULL to keep the original. */
const struct DeckList *GetOpponentDeckOverride(int opponent)
{
    switch (opponent) {
    case DUELIST_YUGI:
        return &sYugiDeckList;
#ifdef MOD_RED_DECK
    case DUELIST_TEA:
        return &sRedDeckList;
#endif
#ifdef MOD_JOEY_DECK
    case DUELIST_JOEY:
        return &sJoeyDeckList;
#endif
#ifdef MOD_BAKURA_DECK
    case DUELIST_BAKURA:
        return &sBakuraDeckList;
#endif
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


/* Build the saved Deck (and Fusion Deck) of starter deck `choice` (enum StarterDeck) when this build replaces it:
 * Red is LR-Chaos Turbo, Green Joey's Warrior deck, Black Bakura's Goat Control deck (with its Fusion Monsters). A
 * Fusion Monster goes to the saved Fusion Deck. Returns 1 then, 0 to let BuildStarterDeck build the original. */
int BuildModStarterDeck(int choice)
{
    const struct DeckList *list = NULL;
    u32 i;

    switch (choice) {
#ifdef MOD_RED_DECK
    case STARTER_DECK_RED:
        list = &sRedDeckList;
        break;
#endif
#ifdef MOD_JOEY_DECK
    case STARTER_DECK_GREEN:
        list = &sJoeyDeckList;
        break;
#endif
#ifdef MOD_BAKURA_DECK
    case STARTER_DECK_BLACK:
        list = &sBakuraDeckList;
        break;
#endif
    }
    if (list == NULL)
        return 0;
    for (i = 0; i < list->count; i++) {
        u16 id = gCardNumberToId[list->cards[i] & CARD_ID_MASK];
        u32 stats = gCardStats[id & CARD_ID_MASK];

        if (CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_KIND(stats) == CARD_KIND_FUSION)
            AddCardToSavedFusionDeck(id);
        else
            AddCardToSavedDeck(id);
    }
    return 1;
}
