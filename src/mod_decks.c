/*
 * mod_decks: decks that replace the original ones. Not part of the original ROM.
 *   GetOpponentDeckOverride   opponent decks (gOpponentDecks); LoadOpponentDeck (ai_deck.c) asks it first
 *  *   BuildModStarterDeck       starter decks (Red, Green, Black); BuildStarterDeck (title_screen.c) asks it first
 *   GiveStartingTrunk         the cards a new game starts with in the trunk (BuildStarterDeck)
 *   GetRecipePreset           the decks the Deck Recipes start with (src/mod_recipes.c), named after their .ydk files
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

int Random(void);   /* util.h */

#define X2(card) card, card
#define X3(card) card, card, card

/* Yugi Muto: a competitive deck: strong Normal monsters, the classic floaters and searchers, and the best Magic
 * and Trap cards at their limits. */
static const u16 sYugiDeck[] = {
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
    X3(CARD_7_COLORED_FISH),
};

static const struct DeckList sYugiDeckList = { sYugiDeck, ARRAY_COUNT(sYugiDeck), 0 };

STATIC_ASSERT(ARRAY_COUNT(sYugiDeck) == 30, YugiDeckSize);

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

 // For later when these cards are actually coded
    // && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    // && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    // && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    // && defined(HAS_CARD_GATLING_DRAGON) \
    // && defined(HAS_CARD_KING_DRAGUN) \
    // && defined(HAS_CARD_OJAMA_KING) \
    // && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    // && defined(HAS_CARD_RYU_SENSHI) \
    // && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
 
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


/* Tristan Taylor: one of three decks at weighted random, per duel (GetOpponentDeckOverride): Soul Control 50%,
 * Vanilla Beat 30%, Stall Mill 20% (El Duelista DEX-Soul Control.ydk, ellietincan-Vanilla Beat.ydk,
 * DK-ZERO-Stall Mill.ydk; main decks only). */
#if defined(HAS_CARD_SOUL_EXCHANGE)
#define MOD_TRISTAN_SOUL_CONTROL 1
static const u16 sTristanSoulControl[] = {
    /* Monsters (20) */
    X3(CARD_MAN_EATER_BUG),
    CARD_TRAP_MASTER,
    CARD_WALL_OF_ILLUSION,
    X3(CARD_BATTLE_OX),
    X3(CARD_GIANT_SOLDIER_OF_STONE),
    X3(CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP),
    X3(CARD_NEO_THE_MAGIC_SWORDSMAN),
    X3(CARD_SUMMONED_SKULL),
    /* Spells (12) */
    CARD_CHANGE_OF_HEART,
    CARD_DARK_HOLE,
    X3(CARD_FISSURE),
    CARD_MONSTER_REBORN,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    X2(CARD_SOUL_EXCHANGE),
    X2(CARD_SWORDS_OF_REVEALING_LIGHT),
    /* Traps (8) */
    X3(CARD_REINFORCEMENTS),
    X3(CARD_TRAP_HOLE),
    CARD_ULTIMATE_OFFERING,
    CARD_WABOKU,
};
STATIC_ASSERT(ARRAY_COUNT(sTristanSoulControl) == 40, TristanSoulControlSize);
#endif
#ifdef MOD_TRISTAN_SOUL_CONTROL
static const struct DeckList sTristanSoulControlList = { sTristanSoulControl, ARRAY_COUNT(sTristanSoulControl), 0 };
#endif
#if 1
#define MOD_TRISTAN_VANILLA_BEAT 1
static const u16 sTristanVanillaBeat[] = {
    /* Monsters (20) */
    X3(CARD_MAN_EATER_BUG),
    X3(CARD_TRAP_MASTER),
    X3(CARD_BATTLE_OX),
    X3(CARD_GIANT_SOLDIER_OF_STONE),
    X3(CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP),
    X3(CARD_NEO_THE_MAGIC_SWORDSMAN),
    X2(CARD_SUMMONED_SKULL),
    /* Spells (11) */
    CARD_CHANGE_OF_HEART,
    CARD_DARK_HOLE,
    CARD_DE_SPELL,
    X3(CARD_FISSURE),
    CARD_MONSTER_REBORN,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    X2(CARD_SWORDS_OF_REVEALING_LIGHT),
    /* Traps (9) */
    X3(CARD_REINFORCEMENTS),
    X3(CARD_TRAP_HOLE),
    X3(CARD_WABOKU),
};
STATIC_ASSERT(ARRAY_COUNT(sTristanVanillaBeat) == 40, TristanVanillaBeatSize);
#endif
#ifdef MOD_TRISTAN_VANILLA_BEAT
static const struct DeckList sTristanVanillaBeatList = { sTristanVanillaBeat, ARRAY_COUNT(sTristanVanillaBeat), 0 };
#endif
#if defined(HAS_CARD_CARD_DESTRUCTION) \
    && defined(HAS_CARD_SOUL_EXCHANGE)
#define MOD_TRISTAN_STALL_MILL 1
static const u16 sTristanStallMill[] = {
    /* Monsters (21) */
    X3(CARD_GYAKUTENNO_MEGAMI),
    X3(CARD_GIANT_SOLDIER_OF_STONE),
    X3(CARD_AQUA_MADOOR),
    X3(CARD_WALL_OF_ILLUSION),
    X3(CARD_MYSTICAL_ELF),
    X3(CARD_SPIRIT_OF_THE_HARP),
    X3(CARD_MAN_EATER_BUG),
    /* Spells (13) */
    CARD_RAIGEKI,
    CARD_DARK_HOLE,
    CARD_MONSTER_REBORN,
    CARD_CHANGE_OF_HEART,
    CARD_POT_OF_GREED,
    X2(CARD_SWORDS_OF_REVEALING_LIGHT),
    X3(CARD_FISSURE),
    CARD_CARD_DESTRUCTION,
    X2(CARD_SOUL_EXCHANGE),
    /* Traps (9) */
    X3(CARD_ULTIMATE_OFFERING),
    X3(CARD_WABOKU),
    X3(CARD_TRAP_HOLE),
};
STATIC_ASSERT(ARRAY_COUNT(sTristanStallMill) == 43, TristanStallMillSize);
#endif
#ifdef MOD_TRISTAN_STALL_MILL
static const struct DeckList sTristanStallMillList = { sTristanStallMill, ARRAY_COUNT(sTristanStallMill), 0 };
#endif

#if defined(MOD_TRISTAN_SOUL_CONTROL) && defined(MOD_TRISTAN_VANILLA_BEAT) && defined(MOD_TRISTAN_STALL_MILL)
#define MOD_TRISTAN_DECKS 1

/* Tristan's deck for this duel: Soul Control 5 in 10, Vanilla Beat 3 in 10, Stall Mill 2 in 10. LoadOpponentDeck
 * asks once per duel. */
static const struct DeckList *TristanDeck(void)
{
    u32 roll = (u32)Random() % 10;

    if (roll < 5)
        return &sTristanSoulControlList;
    if (roll < 8)
        return &sTristanVanillaBeatList;
    return &sTristanStallMillList;
}
#endif

/* The Critter format opponent (Rex Raptor's slot, src/mod_opponents.c): one of two decks per duel (CritterDeck):
 * Masked Control 60%, Muka Muka Turbo 40% (decks/Jazz-Masked Control.ydk, decks/Gyomberman-Muka Muka Turbo.ydk; main
 * and extra decks: LoadOpponentDeck puts the Fusion Monsters in the Fusion Deck). Thousand Dragon is the .ydk's
 * 41462083. */
#if defined(HAS_CARD_BLACK_SKULL_DRAGON)
#define MOD_CRITTER_MASKED_CONTROL 1
static const u16 sCritterMaskedControl[] = {
    /* Monsters (18) */
    CARD_CANNON_SOLDIER,
    CARD_JIRAI_GUMO,
    CARD_KURIBOH,
    X2(CARD_MAGICIAN_OF_FAITH),
    X2(CARD_MASKED_SORCERER),
    X2(CARD_MUKA_MUKA),
    X2(CARD_SANGAN),
    CARD_WHITE_MAGICAL_HAT,
    X2(CARD_WITCH_OF_THE_BLACK_FOREST),
    CARD_GIANT_SOLDIER_OF_STONE,
    X3(CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP),
    /* Spells (13) */
    CARD_CHANGE_OF_HEART,
    CARD_DARK_HOLE,
    X2(CARD_FISSURE),
    X2(CARD_HEAVY_STORM),
    X2(CARD_LAST_WILL),
    CARD_MONSTER_REBORN,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    X2(CARD_SWORDS_OF_REVEALING_LIGHT),
    /* Traps (9) */
    CARD_MIRROR_FORCE,
    X3(CARD_SOLEMN_JUDGMENT),
    X2(CARD_TRAP_HOLE),
    X3(CARD_WABOKU),
    /* Fusion Deck (15) */
    X3(CARD_BLACK_SKULL_DRAGON),
    X3(CARD_GAIA_THE_DRAGON_CHAMPION),
    X3(CARD_LABYRINTH_TANK),
    X3(CARD_THOUSAND_DRAGON),
    X3(CARD_TWIN_HEADED_THUNDER_DRAGON),
};
STATIC_ASSERT(ARRAY_COUNT(sCritterMaskedControl) == 40 + 15, CritterMaskedControlSize);
static const struct DeckList sCritterMaskedControlList = { sCritterMaskedControl, ARRAY_COUNT(sCritterMaskedControl), 0 };
#endif

#define MOD_CRITTER_MUKA_MUKA_TURBO 1
static const u16 sCritterMukaMukaTurbo[] = {
    /* Monsters (20) */
    CARD_CANNON_SOLDIER,
    X3(CARD_MAGICIAN_OF_FAITH),
    CARD_MASKED_SORCERER,
    X3(CARD_MUKA_MUKA),
    X2(CARD_SANGAN),
    X3(CARD_THUNDER_DRAGON),
    X2(CARD_WITCH_OF_THE_BLACK_FOREST),
    X2(CARD_GIANT_SOLDIER_OF_STONE),
    X3(CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP),
    /* Spells (15) */
    CARD_CHANGE_OF_HEART,
    CARD_DARK_HOLE,
    X3(CARD_FISSURE),
    X2(CARD_HEAVY_STORM),
    X3(CARD_LAST_WILL),
    CARD_MONSTER_REBORN,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    X2(CARD_SWORDS_OF_REVEALING_LIGHT),
    /* Traps (5) */
    CARD_MIRROR_FORCE,
    CARD_ROBBIN_GOBLIN,
    X3(CARD_TRAP_HOLE),
    /* Fusion Deck (15) */
    CARD_CHARUBIN_THE_FIRE_KNIGHT,
    CARD_CYBER_SAURUS,
    CARD_DARKFIRE_DRAGON,
    CARD_DEEPSEA_SHARK,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    CARD_EMPRESS_JUDGE,
    CARD_FLAME_GHOST,
    CARD_FUSIONIST,
    CARD_GAIA_THE_DRAGON_CHAMPION,
    CARD_GILTIA_THE_D_KNIGHT,
    CARD_KARBONALA_WARRIOR,
    CARD_METAL_DRAGON,
    CARD_MUSICIAN_KING,
    CARD_PUNISHED_EAGLE,
    CARD_ROARING_OCEAN_SNAKE,
};
STATIC_ASSERT(ARRAY_COUNT(sCritterMukaMukaTurbo) == 40 + 15, CritterMukaMukaTurboSize);
static const struct DeckList sCritterMukaMukaTurboList = { sCritterMukaMukaTurbo, ARRAY_COUNT(sCritterMukaMukaTurbo), 0 };

#ifdef MOD_CRITTER_MASKED_CONTROL
/* The Critter opponent's deck for this duel: Masked Control 6 in 10, Muka Muka Turbo 4 in 10. */
static const struct DeckList *CritterDeck(void)
{
    return (u32)Random() % 10 < 6 ? &sCritterMaskedControlList : &sCritterMukaMukaTurboList;
}
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
#ifdef MOD_TRISTAN_DECKS
    case DUELIST_TRISTAN:
        return TristanDeck();
#endif
    case DUELIST_REX:                   /* the Critter format opponent */
#ifdef MOD_CRITTER_MASKED_CONTROL
        return CritterDeck();
#else
        return &sCritterMukaMukaTurboList;
#endif
#ifdef MOD_JOEY_DECK
    case DUELIST_JOEY:
        return &sJoeyDeckList;
#endif
#ifdef MOD_BAKURA_DECK
    case DUELIST_BAKURA:
        return &sBakuraDeckList;
#endif
    /* The deck opponents of page 4 (src/mod_opponents.c). */
#ifdef MOD_RED_DECK
    case DUELIST_SIMON:
        return &sRedDeckList;
#endif
#ifdef MOD_JOEY_DECK
    case DUELIST_PEGASUS:
        return &sJoeyDeckList;
#endif
#ifdef MOD_BAKURA_DECK
    case DUELIST_GRANDPA:
        return &sBakuraDeckList;
#endif
    }
    return NULL;
}

/* The first Deck Recipes (src/mod_recipes.c), in order, named as their .ydk files. A deck this build leaves out
 * (its cards are not all listed) is skipped. */
struct RecipePreset {
    const char *name;
    const struct DeckList *list;
};

static const char sNameSoulControl[] = "El Duelista DEX-Soul Control";
static const char sNameVanillaBeat[] = "ellietincan-Vanilla Beat";
static const char sNameStallMill[] = "DK-ZERO-Stall Mill";
static const char sNameMaskedControl[] = "Jazz-Masked Control";
static const char sNameMukaMukaTurbo[] = "Gyomberman-Muka Muka Turbo";
static const char sNameChaosTurbo[] = "LR-Chaos Turbo";
static const char sNameWarrior[] = "sdlkilla-Warrior";
static const char sNameGoatControl[] = "geistD-Goat Control";

static const struct RecipePreset sRecipePresets[] = {
#ifdef MOD_TRISTAN_SOUL_CONTROL
    { sNameSoulControl, &sTristanSoulControlList },
#endif
#ifdef MOD_TRISTAN_VANILLA_BEAT
    { sNameVanillaBeat, &sTristanVanillaBeatList },
#endif
#ifdef MOD_TRISTAN_STALL_MILL
    { sNameStallMill, &sTristanStallMillList },
#endif
#ifdef MOD_CRITTER_MASKED_CONTROL
    { sNameMaskedControl, &sCritterMaskedControlList },
#endif
    { sNameMukaMukaTurbo, &sCritterMukaMukaTurboList },
#ifdef MOD_RED_DECK
    { sNameChaosTurbo, &sRedDeckList },
#endif
#ifdef MOD_JOEY_DECK
    { sNameWarrior, &sJoeyDeckList },
#endif
#ifdef MOD_BAKURA_DECK
    { sNameGoatControl, &sBakuraDeckList },
#endif
    { NULL, NULL },
};

/* Preset recipe n: its name, and its cards (card numbers) and count; 0 past the last one. */
int GetRecipePreset(int n, const char **name, const u16 **cards, u32 *count)
{
    if (n < 0 || n >= (int)ARRAY_COUNT(sRecipePresets) - 1)
        return 0;
    *name = sRecipePresets[n].name;
    *cards = sRecipePresets[n].list->cards;
    *count = sRecipePresets[n].list->count;
    return 1;
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
