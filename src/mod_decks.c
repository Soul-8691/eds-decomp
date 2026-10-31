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
#include "main.h"               /* gMain.duelFormat, matchDuelCount */
#include "constants/duel.h"     /* DUEL_FORMAT_MATCH */
#include "util.h"               /* MemCopy16 */
#include "gba.h"                /* SRAM */

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

/* Joey Wheeler: the "sdlkilla-Warrior" deck (sdlkilla-Warrior.ydk, main, extra and side deck): Warriors with
 * Reinforcement of the Army, D.D. Warrior Lady, Blade Knight, Exiled Force and Ninja Grandmaster Sasuke, and a Trap
 * line-up. */
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
    && defined(HAS_CARD_TRAP_DUSTSHOOT) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_CIPHER_SOLDIER) \
    && defined(HAS_CARD_MOBIUS_THE_FROST_MONARCH) \
    && defined(HAS_CARD_MIND_CONTROL) \
    && defined(HAS_CARD_MYSTIK_WOK) \
    && defined(HAS_CARD_DARK_SCORPION_CLIFF_THE_TRAP_REMOVER) \
    && defined(HAS_CARD_GRAVEKEEPERS_SPY) \
    && defined(HAS_CARD_CHAOS_SORCERER) \
    && defined(HAS_CARD_ROYAL_OPPRESSION)
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
    /* Fusion Deck (1) */
    CARD_THOUSAND_EYES_RESTRICT,
};
STATIC_ASSERT(ARRAY_COUNT(sJoeyDeck) == 40 + 1, JoeyDeckSize);
/* sdlkilla-Warrior.ydk's Side Deck (15). */
static const u16 sJoeyDeckSide[] = {
    X2(CARD_CIPHER_SOLDIER),
    X2(CARD_MOBIUS_THE_FROST_MONARCH),
    CARD_DARK_SCORPION_CLIFF_THE_TRAP_REMOVER,
    X3(CARD_GRAVEKEEPERS_SPY),
    CARD_CHAOS_SORCERER,
    CARD_MIND_CONTROL,
    CARD_MYSTIK_WOK,
    CARD_TRAP_DUSTSHOOT,
    X2(CARD_ROYAL_OPPRESSION),
    CARD_DUST_TORNADO,
};
STATIC_ASSERT(ARRAY_COUNT(sJoeyDeckSide) == 15, JoeyDeckSideSize);
#endif
#ifdef MOD_JOEY_DECK
static const struct DeckList sJoeyDeckList = { sJoeyDeck, ARRAY_COUNT(sJoeyDeck), 0 };
#endif

/* Bakura: the "geistD-Goat Control" deck (geistD-Goat Control.ydk, main, extra and side deck): Scapegoat and
 * Metamorphosis into Thousand-Eyes Restrict, flip monsters and Tsukuyomi, Black Luster Soldier and Dark Magician of
 * Chaos. */
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
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_GATLING_DRAGON) \
    && defined(HAS_CARD_KING_DRAGUN) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_BERSERK_GORILLA) \
    && defined(HAS_CARD_HYSTERIC_FAIRY) \
    && defined(HAS_CARD_EMERGENCY_PROVISIONS) \
    && defined(HAS_CARD_POISON_OF_THE_OLD_MAN) \
    && defined(HAS_CARD_SAKURETSU_ARMOR)
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
    CARD_DARK_BALTER_THE_TERRIBLE,
    CARD_DARK_BLADE_THE_DRAGON_KNIGHT,
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    CARD_FIEND_SKULL_DRAGON,
    CARD_GATLING_DRAGON,
    CARD_GILTIA_THE_D_KNIGHT,
    CARD_KING_DRAGUN,
    CARD_OJAMA_KING,
    CARD_REAPER_ON_THE_NIGHTMARE,
    CARD_RYU_SENSHI,
    CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET,
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sBakuraDeck) == 40 + 15, BakuraDeckSize);
/* geistD-Goat Control.ydk's Side Deck (15). */
static const u16 sBakuraDeckSide[] = {
    X2(CARD_BERSERK_GORILLA),
    CARD_HYSTERIC_FAIRY,
    CARD_KURIBOH,
    X2(CARD_EMERGENCY_PROVISIONS),
    CARD_MYSTICAL_SPACE_TYPHOON,
    CARD_POISON_OF_THE_OLD_MAN,
    CARD_SWORDS_OF_REVEALING_LIGHT,
    X3(CARD_DUST_TORNADO),
    X3(CARD_SAKURETSU_ARMOR),
};
STATIC_ASSERT(ARRAY_COUNT(sBakuraDeckSide) == 15, BakuraDeckSideSize);
#endif
#ifdef MOD_BAKURA_DECK
static const struct DeckList sBakuraDeckList = { sBakuraDeck, ARRAY_COUNT(sBakuraDeck), 0 };
#endif

/* Tea: the "LR-Chaos Turbo" deck (LR-Chaos Turbo.ydk, main, extra and side deck): Black Luster Soldier and Chaos
 * Sorcerer fed by LIGHT and DARK flip monsters, Thunder Dragon and the discard/draw Spells. */
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
    && defined(HAS_CARD_TRAP_DUSTSHOOT) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_DARK_FLARE_KNIGHT) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_GATLING_DRAGON) \
    && defined(HAS_CARD_KING_DRAGUN) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_MOBIUS_THE_FROST_MONARCH) \
    && defined(HAS_CARD_ZOMBYRA_THE_DARK) \
    && defined(HAS_CARD_METAMORPHOSIS) \
    && defined(HAS_CARD_MYSTIK_WOK) \
    && defined(HAS_CARD_SCAPEGOAT)
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
    /* Fusion Deck (15) */
    CARD_DARK_BALTER_THE_TERRIBLE,
    CARD_DARK_BLADE_THE_DRAGON_KNIGHT,
    CARD_DARK_FLARE_KNIGHT,
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    CARD_FIEND_SKULL_DRAGON,
    CARD_GATLING_DRAGON,
    CARD_KING_DRAGUN,
    CARD_OJAMA_KING,
    CARD_REAPER_ON_THE_NIGHTMARE,
    CARD_RYU_SENSHI,
    CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET,
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sRedStarterDeck) == 40 + 15, RedStarterDeckSize);
/* LR-Chaos Turbo.ydk's Side Deck (15). */
static const u16 sRedDeckSide[] = {
    CARD_KYCOO_THE_GHOST_DESTROYER,
    CARD_MOBIUS_THE_FROST_MONARCH,
    CARD_ZOMBYRA_THE_DARK,
    X2(CARD_METAMORPHOSIS),
    CARD_MIND_CONTROL,
    X2(CARD_MYSTIK_WOK),
    X2(CARD_SCAPEGOAT),
    CARD_SWORDS_OF_REVEALING_LIGHT,
    X3(CARD_ROYAL_DECREE),
    CARD_TRAP_DUSTSHOOT,
};
STATIC_ASSERT(ARRAY_COUNT(sRedDeckSide) == 15, RedDeckSideSize);
#endif
#ifdef MOD_RED_DECK
static const struct DeckList sRedDeckList = { sRedStarterDeck, ARRAY_COUNT(sRedStarterDeck), 0 };
#endif


/* Tristan Taylor: one of three decks at weighted random, per duel (GetOpponentDeckOverride): Soul Control 50%,
 * Vanilla Beat 30%, Stall Mill 20% (El Duelista DEX-Soul Control.ydk, ellietincan-Vanilla Beat.ydk,
 * DK-ZERO-Stall Mill.ydk; main and extra decks, the side decks after each). */
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
    /* Fusion Deck (15) */
    X2(CARD_DARKFIRE_DRAGON),
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    X2(CARD_FLAME_GHOST),
    X2(CARD_FLAME_SWORDSMAN),
    X2(CARD_FLOWER_WOLF),
    X2(CARD_FUSIONIST),
    CARD_KARBONALA_WARRIOR,
    X3(CARD_METAL_DRAGON),
};
STATIC_ASSERT(ARRAY_COUNT(sTristanSoulControl) == 40 + 15, TristanSoulControlSize);
#endif
#ifdef MOD_TRISTAN_SOUL_CONTROL
static const struct DeckList sTristanSoulControlList = { sTristanSoulControl, ARRAY_COUNT(sTristanSoulControl), 0 };
/* El Duelista DEX-Soul Control.ydk's Side Deck (15). */
static const u16 sTristanSoulControlSide[] = {
    X2(CARD_HANE_HANE),
    CARD_TRAP_MASTER,
    CARD_WALL_OF_ILLUSION,
    X2(CARD_JUDGE_MAN),
    X2(CARD_CARD_DESTRUCTION),
    X2(CARD_OOKAZI),
    CARD_DE_SPELL,
    CARD_REMOVE_TRAP,
    CARD_ULTIMATE_OFFERING,
    X2(CARD_WABOKU),
};
STATIC_ASSERT(ARRAY_COUNT(sTristanSoulControlSide) == 15, TristanSoulControlSideSize);
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
    /* Fusion Deck (15) */
    X3(CARD_DRAGONESS_THE_WICKED_KNIGHT),
    X3(CARD_FLAME_SWORDSMAN),
    X3(CARD_GAIA_THE_DRAGON_CHAMPION),
    X3(CARD_KARBONALA_WARRIOR),
    X3(CARD_METAL_DRAGON),
};
STATIC_ASSERT(ARRAY_COUNT(sTristanVanillaBeat) == 40 + 15, TristanVanillaBeatSize);
#endif
#ifdef MOD_TRISTAN_VANILLA_BEAT
static const struct DeckList sTristanVanillaBeatList = { sTristanVanillaBeat, ARRAY_COUNT(sTristanVanillaBeat), 0 };
/* ellietincan-Vanilla Beat.ydk's Side Deck (15). */
static const u16 sTristanVanillaBeatSide[] = {
    X3(CARD_WALL_OF_ILLUSION),
    X3(CARD_AQUA_MADOOR),
    CARD_JUDGE_MAN,
    CARD_SUMMONED_SKULL,
    X2(CARD_CARD_DESTRUCTION),
    X2(CARD_DE_SPELL),
    X3(CARD_SOUL_EXCHANGE),
};
STATIC_ASSERT(ARRAY_COUNT(sTristanVanillaBeatSide) == 15, TristanVanillaBeatSideSize);
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
    /* Fusion Deck (15) */
    X3(CARD_GAIA_THE_DRAGON_CHAMPION),
    X3(CARD_METAL_DRAGON),
    X3(CARD_FLAME_SWORDSMAN),
    X3(CARD_FLOWER_WOLF),
    X3(CARD_FLAME_GHOST),
};
STATIC_ASSERT(ARRAY_COUNT(sTristanStallMill) == 43 + 15, TristanStallMillSize);
#endif
#ifdef MOD_TRISTAN_STALL_MILL
static const struct DeckList sTristanStallMillList = { sTristanStallMill, ARRAY_COUNT(sTristanStallMill), 0 };
/* DK-ZERO-Stall Mill.ydk's Side Deck (15). */
static const u16 sTristanStallMillSide[] = {
    X3(CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP),
    X3(CARD_BATTLE_OX),
    X3(CARD_NEO_THE_MAGIC_SWORDSMAN),
    X3(CARD_REINFORCEMENTS),
    X2(CARD_THE_WICKED_WORM_BEAST),
    CARD_CARD_DESTRUCTION,
};
STATIC_ASSERT(ARRAY_COUNT(sTristanStallMillSide) == 15, TristanStallMillSideSize);
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
    X3(CARD_B_SKULL_DRAGON),
    X3(CARD_GAIA_THE_DRAGON_CHAMPION),
    X3(CARD_LABYRINTH_TANK),
    X3(CARD_THOUSAND_DRAGON),
    X3(CARD_TWIN_HEADED_THUNDER_DRAGON),
};
STATIC_ASSERT(ARRAY_COUNT(sCritterMaskedControl) == 40 + 15, CritterMaskedControlSize);
static const struct DeckList sCritterMaskedControlList = { sCritterMaskedControl, ARRAY_COUNT(sCritterMaskedControl), 0 };
/* Jazz-Masked Control.ydk's Side Deck (15). */
static const u16 sCritterMaskedControlSide[] = {
    CARD_KURIBOH,
    CARD_MUKA_MUKA,
    X3(CARD_THUNDER_DRAGON),
    X2(CARD_GIANT_SOLDIER_OF_STONE),
    X2(CARD_CARD_DESTRUCTION),
    CARD_FISSURE,
    X2(CARD_FAKE_TRAP),
    X2(CARD_ROBBIN_GOBLIN),
    CARD_TRAP_HOLE,
};
STATIC_ASSERT(ARRAY_COUNT(sCritterMaskedControlSide) == 15, CritterMaskedControlSideSize);

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
/* Gyomberman-Muka Muka Turbo.ydk's Side Deck (15). */
static const u16 sCritterMukaMukaTurboSide[] = {
    X2(CARD_MAN_EATER_BUG),
    X3(CARD_WALL_OF_ILLUSION),
    X2(CARD_CARD_DESTRUCTION),
    X2(CARD_SEVEN_TOOLS_OF_THE_BANDIT),
    X3(CARD_SOLEMN_JUDGMENT),
    X2(CARD_WABOKU),
    CARD_ROBBIN_GOBLIN,
};
STATIC_ASSERT(ARRAY_COUNT(sCritterMukaMukaTurboSide) == 15, CritterMukaMukaTurboSideSize);

#ifdef MOD_CRITTER_MASKED_CONTROL
/* The Critter opponent's deck for this duel: Masked Control 6 in 10, Muka Muka Turbo 4 in 10. */
static const struct DeckList *CritterDeck(void)
{
    return (u32)Random() % 10 < 6 ? &sCritterMaskedControlList : &sCritterMukaMukaTurboList;
}
#endif

/* The Treasure format opponent (Espa Roba's slot, src/mod_opponents.c): one of three decks per duel (TreasureDeck):
 * Tomato Control 80%, Recruiter Control 10%, Germ Burn 10% (decks/dump truck-Tomato Control.ydk,
 * decks/SolEmerald69-Recruiter Control.ydk, decks/bsocks-Germ Burn.ydk; main and extra decks). Thousand Dragon is
 * the .ydk's 41462083. */
static const u16 sTreasureTomatoControl[] = {
    /* Monsters (18) */
    CARD_CANNON_SOLDIER,
    X3(CARD_MAGICIAN_OF_FAITH),
    X3(CARD_MAN_EATER_BUG),
    X3(CARD_MYSTIC_TOMATO),
    X2(CARD_SANGAN),
    X2(CARD_WITCH_OF_THE_BLACK_FOREST),
    CARD_GIANT_SOLDIER_OF_STONE,
    X3(CARD_MECHANICALCHASER),
    /* Spells (17) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    X3(CARD_FISSURE),
    CARD_HEAVY_STORM,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (5) */
    CARD_MIRROR_FORCE,
    CARD_SEVEN_TOOLS_OF_THE_BANDIT,
    X3(CARD_TRAP_HOLE),
    /* Fusion Deck (15) */
    X3(CARD_CHARUBIN_THE_FIRE_KNIGHT),
    X3(CARD_DARKFIRE_DRAGON),
    X3(CARD_DRAGONESS_THE_WICKED_KNIGHT),
    X3(CARD_FLAME_GHOST),
    X3(CARD_FLAME_SWORDSMAN),
};
STATIC_ASSERT(ARRAY_COUNT(sTreasureTomatoControl) == 40 + 15, TreasureTomatoControlSize);
static const struct DeckList sTreasureTomatoControlList = { sTreasureTomatoControl, ARRAY_COUNT(sTreasureTomatoControl), 0 };
/* dump truck-Tomato Control.ydk's Side Deck (15). */
static const u16 sTreasureTomatoControlSide[] = {
    X2(CARD_BANISHER_OF_THE_LIGHT),
    CARD_JIRAI_GUMO,
    X2(CARD_MASKED_SORCERER),
    CARD_GIANT_TRUNADE,
    CARD_HEAVY_STORM,
    X2(CARD_TRIBUTE_TO_THE_DOOMED),
    X2(CARD_SEVEN_TOOLS_OF_THE_BANDIT),
    CARD_SOLEMN_JUDGMENT,
    X3(CARD_WABOKU),
};
STATIC_ASSERT(ARRAY_COUNT(sTreasureTomatoControlSide) == 15, TreasureTomatoControlSideSize);

#define MOD_TREASURE_RECRUITER_CONTROL 1
static const u16 sTreasureRecruiterControl[] = {
    /* Monsters (17) */
    X3(CARD_MYSTIC_TOMATO),
    X2(CARD_SANGAN),
    X2(CARD_WITCH_OF_THE_BLACK_FOREST),
    CARD_CANNON_SOLDIER,
    X3(CARD_MOTHER_GRIZZLY),
    CARD_CATAPULT_TURTLE,
    X2(CARD_MECHANICALCHASER),
    X2(CARD_MAGICIAN_OF_FAITH),
    CARD_CYBER_JAR,
    /* Spells (18) */
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    X2(CARD_LAST_WILL),
    CARD_MONSTER_REBORN,
    CARD_RAIGEKI,
    CARD_DARK_HOLE,
    CARD_CHANGE_OF_HEART,
    CARD_SNATCH_STEAL,
    CARD_DELINQUENT_DUO,
    CARD_CONFISCATION,
    CARD_THE_FORCEFUL_SENTRY,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_HEAVY_STORM,
    X2(CARD_RUSH_RECKLESSLY),
    /* Traps (5) */
    CARD_MIRROR_FORCE,
    X2(CARD_TRAP_HOLE),
    X2(CARD_WABOKU),
    /* Fusion Deck (15) */
    X3(CARD_B_SKULL_DRAGON),
    X3(CARD_TWIN_HEADED_THUNDER_DRAGON),
    X3(CARD_LABYRINTH_TANK),
    X2(CARD_THOUSAND_DRAGON),
    X2(CARD_BICKURIBOX),
    X2(CARD_MUSICIAN_KING),
};
STATIC_ASSERT(ARRAY_COUNT(sTreasureRecruiterControl) == 40 + 15, TreasureRecruiterControlSize);
static const struct DeckList sTreasureRecruiterControlList = { sTreasureRecruiterControl,
                                                               ARRAY_COUNT(sTreasureRecruiterControl), 0 };
/* SolEmerald69-Recruiter Control.ydk's Side Deck (15). */
static const u16 sTreasureRecruiterControlSide[] = {
    CARD_MAN_EATER_BUG,
    CARD_WALL_OF_ILLUSION,
    X2(CARD_JIRAI_GUMO),
    X2(CARD_TRIBUTE_TO_THE_DOOMED),
    X2(CARD_FISSURE),
    CARD_HEAVY_STORM,
    X2(CARD_SOLEMN_JUDGMENT),
    X2(CARD_MAGIC_JAMMER),
    X2(CARD_SEVEN_TOOLS_OF_THE_BANDIT),
};
STATIC_ASSERT(ARRAY_COUNT(sTreasureRecruiterControlSide) == 15, TreasureRecruiterControlSideSize);

static const u16 sTreasureGermBurn[] = {
    /* Monsters (18) */
    CARD_CANNON_SOLDIER,
    CARD_CYBER_JAR,
    X3(CARD_GIANT_GERM),
    X3(CARD_MYSTIC_TOMATO),
    X3(CARD_NIMBLE_MOMONGA),
    X2(CARD_SANGAN),
    X3(CARD_SPEAR_CRETIN),
    X2(CARD_WITCH_OF_THE_BLACK_FOREST),
    /* Spells (15) */
    X3(CARD_CHAIN_ENERGY),
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    X3(CARD_GIANT_TRUNADE),
    CARD_MONSTER_REBORN,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    CARD_THE_FORCEFUL_SENTRY,
    CARD_TOLL,
    /* Traps (7) */
    X3(CARD_JUST_DESSERTS),
    X3(CARD_MAGIC_JAMMER),
    CARD_MIRROR_FORCE,
};
STATIC_ASSERT(ARRAY_COUNT(sTreasureGermBurn) == 40, TreasureGermBurnSize);
static const struct DeckList sTreasureGermBurnList = { sTreasureGermBurn, ARRAY_COUNT(sTreasureGermBurn), 0 };
/* bsocks-Germ Burn.ydk's Side Deck (15). */
static const u16 sTreasureGermBurnSide[] = {
    X3(CARD_MINAR),
    X3(CARD_PRINCESS_OF_TSURUGI),
    X3(CARD_SEVEN_TOOLS_OF_THE_BANDIT),
    X3(CARD_SOLEMN_JUDGMENT),
    X3(CARD_WABOKU),
};
STATIC_ASSERT(ARRAY_COUNT(sTreasureGermBurnSide) == 15, TreasureGermBurnSideSize);

/* The Treasure opponent's deck for this duel: Tomato Control 8 in 10, Recruiter Control 1 in 10, Germ Burn 1 in 10
 * (Recruiter Control's share goes to Tomato Control when it is not in the build). */
static const struct DeckList *TreasureDeck(void)
{
    u32 roll = (u32)Random() % 10;

    if (roll == 9)
        return &sTreasureGermBurnList;
#ifdef MOD_TREASURE_RECRUITER_CONTROL
    if (roll == 8)
        return &sTreasureRecruiterControlList;
#endif
    return &sTreasureTomatoControlList;
}

/* The Imperial format opponent (Mako Tsunami's slot, src/mod_opponents.c): one of two decks per duel (ImperialDeck):
 * Tomato Control 80%, Relinquished 20% (decks/Aretos-Tomato Control.ydk, decks/bsocks-Relinquished.ydk; main and
 * extra decks). Thousand Dragon is the .ydk's 41462083. Both run B. Skull Dragon and Thousand-Eyes
 * Restrict. */
#if defined(HAS_CARD_THOUSAND_EYES_RESTRICT)
#define MOD_IMPERIAL_DECKS 1
static const u16 sImperialTomatoControl[] = {
    /* Monsters (17) */
    X3(CARD_MECHANICALCHASER),
    CARD_GIANT_SOLDIER_OF_STONE,
    CARD_JINZO,
    X2(CARD_MAGICIAN_OF_FAITH),
    X2(CARD_MAN_EATER_BUG),
    CARD_MORPHING_JAR,
    X3(CARD_MYSTIC_TOMATO),
    X2(CARD_SANGAN),
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_CANNON_SOLDIER,
    /* Spells (18) */
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_FISSURE,
    CARD_RAIGEKI,
    CARD_DARK_HOLE,
    CARD_CHANGE_OF_HEART,
    CARD_SNATCH_STEAL,
    CARD_DELINQUENT_DUO,
    CARD_CONFISCATION,
    CARD_THE_FORCEFUL_SENTRY,
    CARD_HEAVY_STORM,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_MONSTER_REBORN,
    CARD_PREMATURE_BURIAL,
    /* Traps (5) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_TRAP_HOLE,
    CARD_DUST_TORNADO,
    /* Fusion Deck (15) */
    CARD_B_SKULL_DRAGON,
    CARD_TWIN_HEADED_THUNDER_DRAGON,
    CARD_LABYRINTH_TANK,
    CARD_THOUSAND_DRAGON,
    CARD_ROARING_OCEAN_SNAKE,
    CARD_PUNISHED_EAGLE,
    CARD_KWAGAR_HERCULES,
    CARD_WARRIOR_OF_TRADITION,
    CARD_DEEPSEA_SHARK,
    CARD_MUSICIAN_KING,
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sImperialTomatoControl) == 40 + 15, ImperialTomatoControlSize);
static const struct DeckList sImperialTomatoControlList = { sImperialTomatoControl,
                                                            ARRAY_COUNT(sImperialTomatoControl), 0 };
/* Aretos-Tomato Control.ydk's Side Deck (15). */
static const u16 sImperialTomatoControlSide[] = {
    CARD_WHITE_MAGICAL_HAT,
    CARD_MASKED_SORCERER,
    CARD_MAN_EATER_BUG,
    CARD_GIANT_SOLDIER_OF_STONE,
    X3(CARD_EXCHANGE),
    CARD_RUSH_RECKLESSLY,
    CARD_HEAVY_STORM,
    X2(CARD_SOLEMN_WISHES),
    CARD_TRAP_HOLE,
    CARD_ACID_TRAP_HOLE,
    CARD_DUST_TORNADO,
    CARD_CEASEFIRE,
};
STATIC_ASSERT(ARRAY_COUNT(sImperialTomatoControlSide) == 15, ImperialTomatoControlSideSize);

static const u16 sImperialRelinquished[] = {
    /* Monsters (19) */
    CARD_JINZO,
    X3(CARD_MAGICIAN_OF_FAITH),
    X3(CARD_MYSTIC_TOMATO),
    X2(CARD_SANGAN),
    X2(CARD_SENJU_OF_THE_THOUSAND_HANDS),
    X2(CARD_SONIC_BIRD),
    CARD_WITCH_OF_THE_BLACK_FOREST,
    X3(CARD_MECHANICALCHASER),
    X2(CARD_RELINQUISHED),
    /* Spells (15) */
    X2(CARD_BLACK_ILLUSION_RITUAL),
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    CARD_RAIGEKI,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (6) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    X3(CARD_SOLEMN_JUDGMENT),
    /* Fusion Deck (15) */
    X3(CARD_B_SKULL_DRAGON),
    X3(CARD_LABYRINTH_TANK),
    X3(CARD_METAL_DRAGON),
    X3(CARD_THOUSAND_EYES_RESTRICT),
    X3(CARD_TWIN_HEADED_THUNDER_DRAGON),
};
STATIC_ASSERT(ARRAY_COUNT(sImperialRelinquished) == 40 + 15, ImperialRelinquishedSize);
static const struct DeckList sImperialRelinquishedList = { sImperialRelinquished, ARRAY_COUNT(sImperialRelinquished), 0 };
/* bsocks-Relinquished.ydk's Side Deck (15). */
static const u16 sImperialRelinquishedSide[] = {
    CARD_CANNON_SOLDIER,
    X3(CARD_MAN_EATER_BUG),
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_CEASEFIRE,
    X3(CARD_DUST_TORNADO),
    X3(CARD_SOLEMN_WISHES),
    X2(CARD_WABOKU),
};
STATIC_ASSERT(ARRAY_COUNT(sImperialRelinquishedSide) == 15, ImperialRelinquishedSideSize);

/* The Imperial opponent's deck for this duel: Tomato Control 8 in 10, Relinquished 2 in 10. */
static const struct DeckList *ImperialDeck(void)
{
    return (u32)Random() % 10 < 8 ? &sImperialTomatoControlList : &sImperialRelinquishedList;
}
#endif

/* The Android format opponent (Mai Valentine's slot, src/mod_opponents.c): one of two decks per duel (AndroidDeck):
 * Tomato Control 80%, Vanilla Beat 20% (decks/Jazz-Tomato Control.ydk, decks/paoloNS-Vanilla Beat.ydk; main and
 * extra decks). Fairy Box (21598948) was added to config/new_cards.txt for Tomato Control. */
#if defined(HAS_CARD_BAZOO_THE_SOUL_EATER) && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_JAR_OF_GREED) && defined(HAS_CARD_MAGIC_CYLINDER) && defined(HAS_CARD_SKULL_LAIR) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) && defined(HAS_CARD_FAIRY_BOX)
#define MOD_ANDROID_TOMATO_CONTROL 1
static const u16 sAndroidTomatoControl[] = {
    /* Monsters (14) */
    X2(CARD_BAZOO_THE_SOUL_EATER),
    CARD_CANNON_SOLDIER,
    CARD_JINZO,
    X2(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_MASKED_SORCERER,
    X3(CARD_MYSTIC_TOMATO),
    CARD_SANGAN,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    X2(CARD_GEMINI_ELF),
    /* Spells (14) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    CARD_SNATCH_STEAL,
    CARD_SWORDS_OF_REVEALING_LIGHT,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (12) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_FAIRY_BOX,
    CARD_IMPERIAL_ORDER,
    X3(CARD_JAR_OF_GREED),
    CARD_MAGIC_CYLINDER,
    CARD_MIRROR_FORCE,
    CARD_SKULL_LAIR,
    CARD_TIME_SEAL,
    X2(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X3(CARD_DARKFIRE_DRAGON),
    X3(CARD_DRAGONESS_THE_WICKED_KNIGHT),
    X3(CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET),
    X3(CARD_THOUSAND_EYES_RESTRICT),
    X3(CARD_WARRIOR_OF_TRADITION),
};
STATIC_ASSERT(ARRAY_COUNT(sAndroidTomatoControl) == 40 + 15, AndroidTomatoControlSize);
static const struct DeckList sAndroidTomatoControlList = { sAndroidTomatoControl, ARRAY_COUNT(sAndroidTomatoControl), 0 };
/* Jazz-Tomato Control.ydk's Side Deck (15). */
static const u16 sAndroidTomatoControlSide[] = {
    CARD_MORPHING_JAR,
    X2(CARD_PENGUIN_KNIGHT),
    X2(CARD_FISSURE),
    X2(CARD_HEAVY_STORM),
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_FAKE_TRAP,
    X3(CARD_SOLEMN_JUDGMENT),
    X2(CARD_TRAP_HOLE),
};
STATIC_ASSERT(ARRAY_COUNT(sAndroidTomatoControlSide) == 15, AndroidTomatoControlSideSize);
#endif

#if defined(HAS_CARD_BAZOO_THE_SOUL_EATER) && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_JAR_OF_GREED) && defined(HAS_CARD_SKULL_LAIR) && defined(HAS_CARD_TORRENTIAL_TRIBUTE)
#define MOD_ANDROID_VANILLA_BEAT 1
static const u16 sAndroidVanillaBeat[] = {
    /* Monsters (15) */
    X3(CARD_BAZOO_THE_SOUL_EATER),
    CARD_CYBER_JAR,
    CARD_JINZO,
    X3(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_MAGICIAN_OF_FAITH,
    CARD_MORPHING_JAR,
    CARD_SANGAN,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    X3(CARD_GEMINI_ELF),
    /* Spells (15) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    X2(CARD_HEAVY_STORM),
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (10) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_JAR_OF_GREED,
    CARD_MIRROR_FORCE,
    CARD_SKULL_LAIR,
    X3(CARD_SOLEMN_JUDGMENT),
    X2(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X3(CARD_BICKURIBOX),
    X3(CARD_B_SKULL_DRAGON),
    X3(CARD_CHARUBIN_THE_FIRE_KNIGHT),
    X3(CARD_CYBER_SAURUS),
    X3(CARD_TWIN_HEADED_THUNDER_DRAGON),
};
STATIC_ASSERT(ARRAY_COUNT(sAndroidVanillaBeat) == 40 + 15, AndroidVanillaBeatSize);
static const struct DeckList sAndroidVanillaBeatList = { sAndroidVanillaBeat, ARRAY_COUNT(sAndroidVanillaBeat), 0 };
/* paoloNS-Vanilla Beat.ydk's Side Deck (15). */
static const u16 sAndroidVanillaBeatSide[] = {
    CARD_4_STARRED_LADYBUG_OF_DOOM,
    X2(CARD_KURIBOH),
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_PREMATURE_BURIAL,
    CARD_SWORDS_OF_REVEALING_LIGHT,
    CARD_CEASEFIRE,
    X3(CARD_DUST_TORNADO),
    CARD_FAIRY_BOX,
    CARD_SKULL_LAIR,
    CARD_TIME_SEAL,
    CARD_TORRENTIAL_TRIBUTE,
};
STATIC_ASSERT(ARRAY_COUNT(sAndroidVanillaBeatSide) == 15, AndroidVanillaBeatSideSize);
#endif

#if defined(MOD_ANDROID_TOMATO_CONTROL) && defined(MOD_ANDROID_VANILLA_BEAT)
#define MOD_ANDROID_DECKS 1
/* The Android opponent's deck for this duel: Tomato Control 8 in 10, Vanilla Beat 2 in 10. */
static const struct DeckList *AndroidDeck(void)
{
    return (u32)Random() % 10 < 8 ? &sAndroidTomatoControlList : &sAndroidVanillaBeatList;
}
#endif

/* The Joey-Pegasus format opponent (Weevil Underwood's slot, src/mod_opponents.c): one of two decks per duel
 * (JoeyPegasusDeck): Tomato Control 80%, Vanilla Beat 20% (decks/AJTBLS-Tomato Control.ydk,
 * decks/ellietincan-Vanilla Beat (JP).ydk; main and extra decks). Not coded yet: Dark Balter the Terrible, Fiend Skull
 * Dragon and Ryu Senshi (Tomato Control's Fusion Deck), Skull Lair and The Last Warrior from Another Planet (Vanilla
 * Beat). */
#if defined(HAS_CARD_BAZOO_THE_SOUL_EATER) && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) && defined(HAS_CARD_FIEND_SKULL_DRAGON) && defined(HAS_CARD_RYU_SENSHI)
#define MOD_JOEY_PEGASUS_TOMATO_CONTROL 1
static const u16 sJoeyPegasusTomatoControl[] = {
    /* Monsters (16) */
    X2(CARD_BAZOO_THE_SOUL_EATER),
    CARD_JINZO,
    X2(CARD_KYCOO_THE_GHOST_DESTROYER),
    X2(CARD_MAGICIAN_OF_FAITH),
    X3(CARD_MYSTIC_TOMATO),
    CARD_SANGAN,
    CARD_SINISTER_SERPENT,
    X3(CARD_THUNDER_DRAGON),
    CARD_WITCH_OF_THE_BLACK_FOREST,
    /* Spells (17) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    X2(CARD_GRACEFUL_CHARITY),
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_HEAVY_STORM,
    CARD_MONSTER_REBORN,
    X2(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (7) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_TIME_SEAL,
    X3(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X3(CARD_DARK_BALTER_THE_TERRIBLE),
    X3(CARD_FIEND_SKULL_DRAGON),
    X3(CARD_GILTIA_THE_D_KNIGHT),
    X3(CARD_RYU_SENSHI),
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sJoeyPegasusTomatoControl) == 40 + 15, JoeyPegasusTomatoControlSize);
static const struct DeckList sJoeyPegasusTomatoControlList = { sJoeyPegasusTomatoControl,
                                                               ARRAY_COUNT(sJoeyPegasusTomatoControl), 0 };
/* AJTBLS-Tomato Control.ydk's Side Deck (15). */
static const u16 sJoeyPegasusTomatoControlSide[] = {
    CARD_GUARDIAN_SPHINX,
    CARD_KYCOO_THE_GHOST_DESTROYER,
    X2(CARD_MASKED_SORCERER),
    CARD_GEMINI_ELF,
    CARD_LAST_WILL,
    CARD_MAGE_POWER,
    CARD_PREMATURE_BURIAL,
    CARD_REINFORCEMENT_OF_THE_ARMY,
    CARD_CEASEFIRE,
    CARD_MAGIC_CYLINDER,
    X3(CARD_SOLEMN_JUDGMENT),
    CARD_TIME_SEAL,
};
STATIC_ASSERT(ARRAY_COUNT(sJoeyPegasusTomatoControlSide) == 15, JoeyPegasusTomatoControlSideSize);
#endif

#if defined(HAS_CARD_BAZOO_THE_SOUL_EATER) && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_JAR_OF_GREED) && defined(HAS_CARD_SKULL_LAIR) && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_ST_JOAN) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET)
#define MOD_JOEY_PEGASUS_VANILLA_BEAT 1
static const u16 sJoeyPegasusVanillaBeat[] = {
    /* Monsters (13) */
    X2(CARD_BAZOO_THE_SOUL_EATER),
    CARD_JINZO,
    X3(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_MAGICIAN_OF_FAITH,
    CARD_SANGAN,
    CARD_SINISTER_SERPENT,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    X3(CARD_GEMINI_ELF),
    /* Spells (18) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_FISSURE,
    X2(CARD_GRACEFUL_CHARITY),
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_HEAVY_STORM,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (9) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    X3(CARD_JAR_OF_GREED),
    CARD_MIRROR_FORCE,
    CARD_SKULL_LAIR,
    X2(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X3(CARD_B_SKULL_DRAGON),
    X3(CARD_FLAME_SWORDSMAN),
    X3(CARD_GAIA_THE_DRAGON_CHAMPION),
    X3(CARD_ST_JOAN),
    X3(CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET),
};
STATIC_ASSERT(ARRAY_COUNT(sJoeyPegasusVanillaBeat) == 40 + 15, JoeyPegasusVanillaBeatSize);
static const struct DeckList sJoeyPegasusVanillaBeatList = { sJoeyPegasusVanillaBeat,
                                                             ARRAY_COUNT(sJoeyPegasusVanillaBeat), 0 };
/* ellietincan-Vanilla Beat (JP).ydk's Side Deck (15). */
static const u16 sJoeyPegasusVanillaBeatSide[] = {
    CARD_BAZOO_THE_SOUL_EATER,
    CARD_CANNON_SOLDIER,
    X2(CARD_MAGICIAN_OF_FAITH),
    X3(CARD_MYSTIC_TOMATO),
    CARD_FISSURE,
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_PREMATURE_BURIAL,
    CARD_DUST_TORNADO,
    CARD_TORRENTIAL_TRIBUTE,
    X3(CARD_WABOKU),
};
STATIC_ASSERT(ARRAY_COUNT(sJoeyPegasusVanillaBeatSide) == 15, JoeyPegasusVanillaBeatSideSize);
#endif

#if defined(MOD_JOEY_PEGASUS_TOMATO_CONTROL) && defined(MOD_JOEY_PEGASUS_VANILLA_BEAT)
#define MOD_JOEY_PEGASUS_DECKS 1
/* The Joey-Pegasus opponent's deck for this duel: Tomato Control 8 in 10, Vanilla Beat 2 in 10. */
static const struct DeckList *JoeyPegasusDeck(void)
{
    return (u32)Random() % 10 < 8 ? &sJoeyPegasusTomatoControlList : &sJoeyPegasusVanillaBeatList;
}
#endif

/* The Fiber format opponent (the Rare Hunter's slot, src/mod_opponents.c): one of two decks per duel (FiberDeck):
 * Tomato Control 80%, Recruiter Control 20% (decks/Aretos-Tomato Control (Fiber).ydk, decks/GBO-Recruiter
 * Control.ydk; main and extra decks). Super Robolady (75923050) was added to config/new_cards.txt for Tomato
 * Control. */
#if defined(HAS_CARD_BAZOO_THE_SOUL_EATER) && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_INJECTION_FAIRY_LILY) && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_FIBER_JAR) \
    && defined(HAS_CARD_JAR_OF_GREED) && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_SUPER_ROBOLADY) && defined(HAS_CARD_SUPER_ROBOYAROU) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT)
#define MOD_FIBER_TOMATO_CONTROL 1
static const u16 sFiberTomatoControl[] = {
    /* Monsters (15) */
    X3(CARD_GEMINI_ELF),
    CARD_BAZOO_THE_SOUL_EATER,
    CARD_KYCOO_THE_GHOST_DESTROYER,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_SANGAN,
    X3(CARD_MYSTIC_TOMATO),
    CARD_SINISTER_SERPENT,
    CARD_INJECTION_FAIRY_LILY,
    CARD_EXILED_FORCE,
    CARD_FIBER_JAR,
    CARD_MAGICIAN_OF_FAITH,
    /* Spells (16) */
    CARD_SNATCH_STEAL,
    CARD_CHANGE_OF_HEART,
    CARD_DELINQUENT_DUO,
    CARD_CONFISCATION,
    CARD_THE_FORCEFUL_SENTRY,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_RAIGEKI,
    CARD_DARK_HOLE,
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    X2(CARD_GRACEFUL_CHARITY),
    CARD_MONSTER_REBORN,
    /* Traps (9) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_TORRENTIAL_TRIBUTE,
    CARD_TRAP_HOLE,
    CARD_TIME_SEAL,
    X3(CARD_JAR_OF_GREED),
    /* Fusion Deck (15) */
    CARD_B_SKULL_DRAGON,
    CARD_TWIN_HEADED_THUNDER_DRAGON,
    CARD_LABYRINTH_TANK,
    CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET,
    CARD_ROARING_OCEAN_SNAKE,
    CARD_RYU_SENSHI,
    CARD_SUPER_ROBOLADY,
    CARD_SUPER_ROBOYAROU,
    CARD_DARK_BALTER_THE_TERRIBLE,
    CARD_FIEND_SKULL_DRAGON,
    CARD_MUSICIAN_KING,
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    X2(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sFiberTomatoControl) == 40 + 15, FiberTomatoControlSize);
static const struct DeckList sFiberTomatoControlList = { sFiberTomatoControl, ARRAY_COUNT(sFiberTomatoControl), 0 };
/* Aretos-Tomato Control (Fiber).ydk's Side Deck (15). */
static const u16 sFiberTomatoControlSide[] = {
    CARD_JINZO,
    CARD_KYCOO_THE_GHOST_DESTROYER,
    CARD_ASURA_PRIEST,
    CARD_YATA_GARASU,
    X2(CARD_CREATURE_SWAP),
    CARD_SCAPEGOAT,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_FISSURE,
    CARD_TORRENTIAL_TRIBUTE,
    CARD_TRAP_HOLE,
    CARD_SKULL_LAIR,
    X2(CARD_DUST_TORNADO),
};
STATIC_ASSERT(ARRAY_COUNT(sFiberTomatoControlSide) == 15, FiberTomatoControlSideSize);
#endif

#if defined(HAS_CARD_DARK_RULER_HA_DES) && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_INJECTION_FAIRY_LILY) \
    && defined(HAS_CARD_YATA_GARASU) && defined(HAS_CARD_CREATURE_SWAP) && defined(HAS_CARD_MAGIC_CYLINDER) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE)
#define MOD_FIBER_RECRUITER_CONTROL 1
static const u16 sFiberRecruiterControl[] = {
    /* Monsters (13) */
    CARD_DARK_RULER_HA_DES,
    CARD_EXILED_FORCE,
    X3(CARD_GIANT_RAT),
    CARD_INJECTION_FAIRY_LILY,
    CARD_JINZO,
    X3(CARD_MYSTIC_TOMATO),
    CARD_SANGAN,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_YATA_GARASU,
    /* Spells (22) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    X2(CARD_CREATURE_SWAP),
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    X2(CARD_GRACEFUL_CHARITY),
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_HEAVY_STORM,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    CARD_RAIGEKI,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (5) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MAGIC_CYLINDER,
    CARD_MIRROR_FORCE,
    CARD_TORRENTIAL_TRIBUTE,
};
STATIC_ASSERT(ARRAY_COUNT(sFiberRecruiterControl) == 40, FiberRecruiterControlSize);
static const struct DeckList sFiberRecruiterControlList = { sFiberRecruiterControl,
                                                            ARRAY_COUNT(sFiberRecruiterControl), 0 };
/* GBO-Recruiter Control.ydk's Side Deck (14). Left out: Gradius' Option (no use without Gradius). */
static const u16 sFiberRecruiterControlSide[] = {
    X2(CARD_GEARFRIED_THE_IRON_KNIGHT),
    CARD_GOBLIN_ATTACK_FORCE,
    X2(CARD_FISSURE),
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    X2(CARD_BLAST_WITH_CHAIN),
    X3(CARD_DUST_TORNADO),
    X2(CARD_TORRENTIAL_TRIBUTE),
};
STATIC_ASSERT(ARRAY_COUNT(sFiberRecruiterControlSide) == 14, FiberRecruiterControlSideSize);
#endif

#if defined(MOD_FIBER_TOMATO_CONTROL) && defined(MOD_FIBER_RECRUITER_CONTROL)
#define MOD_FIBER_DECKS 1
/* The Fiber opponent's deck for this duel: Tomato Control 8 in 10, Recruiter Control 2 in 10. */
static const struct DeckList *FiberDeck(void)
{
    return (u32)Random() % 10 < 8 ? &sFiberTomatoControlList : &sFiberRecruiterControlList;
}
#endif

/* The Yata format opponent (Arkana's slot, src/mod_opponents.c): one of three decks per duel (YataDeck): Tomato
 * Control 50%, Warrior Tomato Control 40%, Gearfried 10% (decks/Shigeki Kitamura-Tomato Control.ydk,
 * decks/ellietincan-Warrior Tomato Control.ydk, decks/AlephYa2-Gearfried.ydk; main and extra decks). Mirage of
 * Nightmare (41482598, in all three) was added to config/new_cards.txt for them. Dice Jar (3549275, in Warrior Tomato
 * Control's Side Deck) was added and removed again (2026-10-29): that Side Deck has 14 cards. */
#if defined(HAS_CARD_BAZOO_THE_SOUL_EATER) && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_FIBER_JAR) \
    && defined(HAS_CARD_SPIRIT_REAPER) && defined(HAS_CARD_YATA_GARASU) && defined(HAS_CARD_ZOMBYRA_THE_DARK) \
    && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) && defined(HAS_CARD_SCAPEGOAT) && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_RYU_SENSHI) && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_AIRKNIGHT_PARSHATH) && defined(HAS_CARD_DON_ZALOOG) && defined(HAS_CARD_GOBLIN_ATTACK_FORCE) \
    && defined(HAS_CARD_GUARDIAN_SPHINX) && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_SPEAR_DRAGON) && defined(HAS_CARD_CREATURE_SWAP) && defined(HAS_CARD_MAGIC_CYLINDER)
#define MOD_YATA_TOMATO_CONTROL 1
static const u16 sYataTomatoControl[] = {
    /* Monsters (16) */
    CARD_BAZOO_THE_SOUL_EATER,
    CARD_EXILED_FORCE,
    CARD_FIBER_JAR,
    CARD_JINZO,
    CARD_MAGICIAN_OF_FAITH,
    X3(CARD_MYSTIC_TOMATO),
    CARD_SANGAN,
    CARD_SINISTER_SERPENT,
    CARD_SPIRIT_REAPER,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_YATA_GARASU,
    CARD_ZOMBYRA_THE_DARK,
    X2(CARD_GEMINI_ELF),
    /* Spells (18) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_HEAVY_STORM,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    CARD_RAIGEKI,
    CARD_SCAPEGOAT,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (6) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    X2(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X3(CARD_DARK_BALTER_THE_TERRIBLE),
    X3(CARD_FIEND_SKULL_DRAGON),
    X3(CARD_GILTIA_THE_D_KNIGHT),
    X3(CARD_RYU_SENSHI),
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sYataTomatoControl) == 40 + 15, YataTomatoControlSize);
static const struct DeckList sYataTomatoControlList = { sYataTomatoControl, ARRAY_COUNT(sYataTomatoControl), 0 };
/* Shigeki Kitamura-Tomato Control.ydk's Side Deck (15). */
static const u16 sYataTomatoControlSide[] = {
    CARD_AIRKNIGHT_PARSHATH,
    CARD_CANNON_SOLDIER,
    CARD_DON_ZALOOG,
    CARD_GOBLIN_ATTACK_FORCE,
    CARD_GUARDIAN_SPHINX,
    CARD_KYCOO_THE_GHOST_DESTROYER,
    CARD_SPEAR_DRAGON,
    CARD_GEMINI_ELF,
    CARD_CREATURE_SWAP,
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_PAINFUL_CHOICE,
    CARD_SWORDS_OF_REVEALING_LIGHT,
    CARD_CEASEFIRE,
    CARD_MAGIC_CYLINDER,
};
STATIC_ASSERT(ARRAY_COUNT(sYataTomatoControlSide) == 15, YataTomatoControlSideSize);
#endif

#if defined(HAS_CARD_DON_ZALOOG) && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_FIBER_JAR) \
    && defined(HAS_CARD_INJECTION_FAIRY_LILY) && defined(HAS_CARD_SPIRIT_REAPER) && defined(HAS_CARD_YATA_GARASU) \
    && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) && defined(HAS_CARD_RYU_SENSHI) && defined(HAS_CARD_ST_JOAN) \
    && defined(HAS_CARD_BAZOO_THE_SOUL_EATER) && defined(HAS_CARD_MARAUDING_CAPTAIN) \
    && defined(HAS_CARD_TRAP_DUSTSHOOT)
#define MOD_YATA_WARRIOR_TOMATO_CONTROL 1
static const u16 sYataWarriorTomatoControl[] = {
    /* Monsters (16) */
    X2(CARD_DON_ZALOOG),
    CARD_EXILED_FORCE,
    CARD_FIBER_JAR,
    CARD_INJECTION_FAIRY_LILY,
    CARD_JINZO,
    X2(CARD_MAGICIAN_OF_FAITH),
    X3(CARD_MYSTIC_TOMATO),
    CARD_SANGAN,
    CARD_SINISTER_SERPENT,
    CARD_SPIRIT_REAPER,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_YATA_GARASU,
    /* Spells (17) */
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (7) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    X3(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X3(CARD_B_SKULL_DRAGON),
    X3(CARD_FLAME_SWORDSMAN),
    X3(CARD_GAIA_THE_DRAGON_CHAMPION),
    X3(CARD_RYU_SENSHI),
    X3(CARD_ST_JOAN),
};
STATIC_ASSERT(ARRAY_COUNT(sYataWarriorTomatoControl) == 40 + 15, YataWarriorTomatoControlSize);
static const struct DeckList sYataWarriorTomatoControlList = { sYataWarriorTomatoControl,
                                                               ARRAY_COUNT(sYataWarriorTomatoControl), 0 };
/* ellietincan-Warrior Tomato Control.ydk's Side Deck (15), without its Dice Jar (14). */
static const u16 sYataWarriorTomatoControlSide[] = {
    CARD_BAZOO_THE_SOUL_EATER,
    CARD_CYBER_JAR,
    X2(CARD_MARAUDING_CAPTAIN),
    CARD_MORPHING_JAR,
    X2(CARD_MORPHING_JAR_2),
    X2(CARD_GEMINI_ELF),
    CARD_CHANGE_OF_HEART,
    CARD_HEAVY_STORM,
    CARD_PREMATURE_BURIAL,
    CARD_SNATCH_STEAL,
    CARD_TRAP_DUSTSHOOT,
};
STATIC_ASSERT(ARRAY_COUNT(sYataWarriorTomatoControlSide) == 14, YataWarriorTomatoControlSideSize);
#endif

#if defined(HAS_CARD_DON_ZALOOG) && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_GEARFRIED_THE_IRON_KNIGHT) \
    && defined(HAS_CARD_INJECTION_FAIRY_LILY) && defined(HAS_CARD_ZOMBYRA_THE_DARK) \
    && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_BLAST_WITH_CHAIN) && defined(HAS_CARD_MAGIC_DRAIN) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) && defined(HAS_CARD_GRAVEKEEPERS_SPY) \
    && defined(HAS_CARD_KING_TIGER_WANGHU) && defined(HAS_CARD_BOOK_OF_MOON)
#define MOD_YATA_GEARFRIED 1
static const u16 sYataGearfried[] = {
    /* Monsters (12) */
    X2(CARD_DON_ZALOOG),
    CARD_EXILED_FORCE,
    X3(CARD_GEARFRIED_THE_IRON_KNIGHT),
    CARD_INJECTION_FAIRY_LILY,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_ZOMBYRA_THE_DARK,
    X3(CARD_GEMINI_ELF),
    /* Spells (18) */
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_FISSURE,
    CARD_GRACEFUL_CHARITY,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (10) */
    X3(CARD_BLAST_WITH_CHAIN),
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    X2(CARD_MAGIC_DRAIN),
    CARD_MIRROR_FORCE,
    CARD_MIRROR_WALL,
    CARD_BELL_OF_DESTRUCTION,
    /* Fusion Deck (2) */
    X2(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sYataGearfried) == 40 + 2, YataGearfriedSize);
static const struct DeckList sYataGearfriedList = { sYataGearfried, ARRAY_COUNT(sYataGearfried), 0 };
/* AlephYa2-Gearfried.ydk's Side Deck (15). */
static const u16 sYataGearfriedSide[] = {
    X2(CARD_ELECTRIC_SNAKE),
    X3(CARD_GRAVEKEEPERS_SPY),
    CARD_JINZO,
    X3(CARD_KING_TIGER_WANGHU),
    CARD_BOOK_OF_MOON,
    CARD_CHANGE_OF_HEART,
    CARD_FISSURE,
    CARD_HEAVY_STORM,
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_DUST_TORNADO,
};
STATIC_ASSERT(ARRAY_COUNT(sYataGearfriedSide) == 15, YataGearfriedSideSize);
#endif

#if defined(MOD_YATA_TOMATO_CONTROL) && defined(MOD_YATA_WARRIOR_TOMATO_CONTROL) && defined(MOD_YATA_GEARFRIED)
#define MOD_YATA_DECKS 1
/* The Yata opponent's deck for this duel: Tomato Control 5 in 10, Warrior Tomato Control 4 in 10, Gearfried 1 in 10. */
static const struct DeckList *YataDeck(void)
{
    u32 roll = (u32)Random() % 10;

    if (roll < 5)
        return &sYataTomatoControlList;
    if (roll < 9)
        return &sYataWarriorTomatoControlList;
    return &sYataGearfriedList;
}
#endif

/* The Scientist format opponent (Umbra & Lumis' slot, src/mod_opponents.c): one of two decks per duel
 * (ScientistDeck): Tomato Control 60%, Scientist FTK 40% (decks/Aretos-Tomato Control (Scientist).ydk,
 * decks/DK-ZERO-Scientist FTK.ydk; main and extra decks). Every card was already in the game. */
#if defined(HAS_CARD_MAGICAL_SCIENTIST) && defined(HAS_CARD_GRAVEKEEPERS_SPY) && defined(HAS_CARD_KING_TIGER_WANGHU) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_FIBER_JAR) && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) && defined(HAS_CARD_JAR_OF_GREED) && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) && defined(HAS_CARD_NEWDORIA) && defined(HAS_CARD_SPIRIT_REAPER) \
    && defined(HAS_CARD_AIRKNIGHT_PARSHATH) && defined(HAS_CARD_BOOK_OF_MOON)
#define MOD_SCIENTIST_TOMATO_CONTROL 1
static const u16 sScientistTomatoControl[] = {
    /* Monsters (17) */
    X3(CARD_MYSTIC_TOMATO),
    X2(CARD_MAGICAL_SCIENTIST),
    X2(CARD_GRAVEKEEPERS_SPY),
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_SANGAN,
    CARD_JINZO,
    CARD_KING_TIGER_WANGHU,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_EXILED_FORCE,
    CARD_SINISTER_SERPENT,
    CARD_FIBER_JAR,
    CARD_MAGICIAN_OF_FAITH,
    /* Spells (16) */
    CARD_GRACEFUL_CHARITY,
    CARD_POT_OF_GREED,
    CARD_MIRAGE_OF_NIGHTMARE,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_PAINFUL_CHOICE,
    CARD_RAIGEKI,
    CARD_DARK_HOLE,
    CARD_MONSTER_REBORN,
    CARD_CHANGE_OF_HEART,
    CARD_SNATCH_STEAL,
    CARD_DELINQUENT_DUO,
    CARD_CONFISCATION,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (7) */
    X2(CARD_TORRENTIAL_TRIBUTE),
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_JAR_OF_GREED,
    /* Fusion Deck (15) */
    X3(CARD_ROARING_OCEAN_SNAKE),
    X3(CARD_PUNISHED_EAGLE),
    CARD_EMPRESS_JUDGE,
    CARD_RYU_SENSHI,
    X2(CARD_DARK_BALTER_THE_TERRIBLE),
    CARD_FIEND_SKULL_DRAGON,
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    X2(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sScientistTomatoControl) == 40 + 15, ScientistTomatoControlSize);
static const struct DeckList sScientistTomatoControlList = { sScientistTomatoControl,
                                                             ARRAY_COUNT(sScientistTomatoControl), 0 };
/* Aretos-Tomato Control (Scientist).ydk's Side Deck (15). */
static const u16 sScientistTomatoControlSide[] = {
    X2(CARD_KING_TIGER_WANGHU),
    CARD_NEWDORIA,
    CARD_SPIRIT_REAPER,
    CARD_MAGICAL_SCIENTIST,
    CARD_CATAPULT_TURTLE,
    CARD_AIRKNIGHT_PARSHATH,
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_BOOK_OF_MOON,
    CARD_HEAVY_STORM,
    X2(CARD_DUST_TORNADO),
    CARD_TORRENTIAL_TRIBUTE,
    CARD_CEASEFIRE,
};
STATIC_ASSERT(ARRAY_COUNT(sScientistTomatoControlSide) == 15, ScientistTomatoControlSideSize);
#endif

#if defined(HAS_CARD_GILASAURUS) && defined(HAS_CARD_MAGICAL_SCIENTIST) && defined(HAS_CARD_CARD_DESTRUCTION) \
    && defined(HAS_CARD_METAMORPHOSIS) && defined(HAS_CARD_REASONING) && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) && defined(HAS_CARD_RYU_SENSHI) && defined(HAS_CARD_SUPER_ROBOLADY) \
    && defined(HAS_CARD_SUPER_ROBOYAROU) && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_DARK_RULER_HA_DES) && defined(HAS_CARD_FIBER_JAR) && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE)
#define MOD_SCIENTIST_FTK 1
static const u16 sScientistFtk[] = {
    /* Monsters (17) */
    X3(CARD_CANNON_SOLDIER),
    X3(CARD_CATAPULT_TURTLE),
    X3(CARD_GILASAURUS),
    X3(CARD_MAGICAL_SCIENTIST),
    X3(CARD_MYSTIC_TOMATO),
    CARD_SANGAN,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    /* Spells (20) */
    CARD_CARD_DESTRUCTION,
    CARD_DARK_HOLE,
    CARD_GIANT_TRUNADE,
    CARD_GRACEFUL_CHARITY,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_HEAVY_STORM,
    X3(CARD_LAST_WILL),
    X3(CARD_METAMORPHOSIS),
    CARD_MONSTER_REBORN,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    CARD_RAIGEKI,
    X3(CARD_REASONING),
    /* Traps (3) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_BELL_OF_DESTRUCTION,
    /* Fusion Deck (15) */
    X2(CARD_DARK_BALTER_THE_TERRIBLE),
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    CARD_EMPRESS_JUDGE,
    CARD_FIEND_SKULL_DRAGON,
    CARD_PUNISHED_EAGLE,
    CARD_ROARING_OCEAN_SNAKE,
    X2(CARD_RYU_SENSHI),
    CARD_SUPER_ROBOLADY,
    CARD_SUPER_ROBOYAROU,
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sScientistFtk) == 40 + 15, ScientistFtkSize);
static const struct DeckList sScientistFtkList = { sScientistFtk, ARRAY_COUNT(sScientistFtk), 0 };
/* DK-ZERO-Scientist FTK.ydk's Side Deck (15). */
static const u16 sScientistFtkSide[] = {
    CARD_CYBER_JAR,
    CARD_DARK_RULER_HA_DES,
    CARD_FIBER_JAR,
    CARD_JINZO,
    CARD_SINISTER_SERPENT,
    CARD_CHANGE_OF_HEART,
    CARD_MIRAGE_OF_NIGHTMARE,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_SNATCH_STEAL,
    CARD_MIRROR_FORCE,
    X3(CARD_TORRENTIAL_TRIBUTE),
};
STATIC_ASSERT(ARRAY_COUNT(sScientistFtkSide) == 15, ScientistFtkSideSize);
#endif

#if defined(MOD_SCIENTIST_TOMATO_CONTROL) && defined(MOD_SCIENTIST_FTK)
#define MOD_SCIENTIST_DECKS 1
/* The Scientist opponent's deck for this duel: Tomato Control 6 in 10, Scientist FTK 4 in 10. */
static const struct DeckList *ScientistDeck(void)
{
    return (u32)Random() % 10 < 6 ? &sScientistTomatoControlList : &sScientistFtkList;
}
#endif

/* The Vampire format opponent (Marik's slot, src/mod_opponents.c): one of three decks per duel (VampireDeck): Warrior
 * Control 80%, Scientist FTK 10%, Warrior Tomato Control 10% (decks/Woawa-Warrior Control.ydk,
 * decks/ellietincan-Scientist FTK.ydk, decks/AJTBLS-Warrior Tomato Control.ydk; main and extra decks). Sasuke Samurai
 * (16222645, Warrior Control's Side Deck) was added to config/new_cards.txt for them. */
#if defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) && defined(HAS_CARD_D_D_WARRIOR_LADY) && defined(HAS_CARD_EXILED_FORCE) \
    && defined(HAS_CARD_FIBER_JAR) && defined(HAS_CARD_GRAVEKEEPERS_SPY) && defined(HAS_CARD_INJECTION_FAIRY_LILY) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) && defined(HAS_CARD_ZOMBYRA_THE_DARK) \
    && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_FLARE_KNIGHT) && defined(HAS_CARD_FIEND_SKULL_DRAGON) && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) && defined(HAS_CARD_MAGICAL_SCIENTIST) \
    && defined(HAS_CARD_SASUKE_SAMURAI) && defined(HAS_CARD_BOOK_OF_MOON) && defined(HAS_CARD_POISON_OF_THE_OLD_MAN)
#define MOD_VAMPIRE_WARRIOR_CONTROL 1
static const u16 sVampireWarriorControl[] = {
    /* Monsters (15) */
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    X3(CARD_D_D_WARRIOR_LADY),
    CARD_EXILED_FORCE,
    CARD_FIBER_JAR,
    X3(CARD_GRAVEKEEPERS_SPY),
    CARD_INJECTION_FAIRY_LILY,
    CARD_SANGAN,
    CARD_SINISTER_SERPENT,
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_ZOMBYRA_THE_DARK,
    /* Spells (19) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_HEAVY_STORM,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (6) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    X2(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X3(CARD_DARK_BALTER_THE_TERRIBLE),
    X3(CARD_DARK_FLARE_KNIGHT),
    X3(CARD_FIEND_SKULL_DRAGON),
    X3(CARD_RYU_SENSHI),
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sVampireWarriorControl) == 40 + 15, VampireWarriorControlSize);
static const struct DeckList sVampireWarriorControlList = { sVampireWarriorControl,
                                                            ARRAY_COUNT(sVampireWarriorControl), 0 };
/* Woawa-Warrior Control.ydk's Side Deck (15). */
static const u16 sVampireWarriorControlSide[] = {
    CARD_JINZO,
    CARD_MAGICAL_SCIENTIST,
    CARD_SASUKE_SAMURAI,
    CARD_THE_LITTLE_SWORDSMAN_OF_AILE,
    X2(CARD_BOOK_OF_MOON),
    CARD_NOBLEMAN_OF_CROSSOUT,
    X3(CARD_POISON_OF_THE_OLD_MAN),
    CARD_SNATCH_STEAL,
    CARD_DUST_TORNADO,
    X2(CARD_ROYAL_DECREE),
    CARD_TORRENTIAL_TRIBUTE,
};
STATIC_ASSERT(ARRAY_COUNT(sVampireWarriorControlSide) == 15, VampireWarriorControlSideSize);
#endif

#if defined(HAS_CARD_MAGICAL_SCIENTIST) && defined(HAS_CARD_GRAVEKEEPERS_SPY) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_FIBER_JAR) && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) && defined(HAS_CARD_DARK_FLARE_KNIGHT) \
    && defined(HAS_CARD_RYU_SENSHI) && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) && defined(HAS_CARD_D_D_WARRIOR_LADY) && defined(HAS_CARD_EXILED_FORCE) \
    && defined(HAS_CARD_MAGIC_CYLINDER) && defined(HAS_CARD_TORRENTIAL_TRIBUTE)
#define MOD_VAMPIRE_SCIENTIST_FTK 1
static const u16 sVampireScientistFtk[] = {
    /* Monsters (17) */
    CARD_MAGICAL_SCIENTIST,
    X3(CARD_CATAPULT_TURTLE),
    X3(CARD_GRAVEKEEPERS_SPY),
    X3(CARD_MYSTIC_TOMATO),
    CARD_SANGAN,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_CANNON_SOLDIER,
    CARD_CYBER_JAR,
    CARD_FIBER_JAR,
    /* Spells (18) */
    X3(CARD_LAST_WILL),
    CARD_PAINFUL_CHOICE,
    CARD_THE_FORCEFUL_SENTRY,
    CARD_POT_OF_GREED,
    CARD_GRACEFUL_CHARITY,
    CARD_MIRAGE_OF_NIGHTMARE,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_HEAVY_STORM,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_DARK_HOLE,
    CARD_RAIGEKI,
    CARD_CHANGE_OF_HEART,
    CARD_PREMATURE_BURIAL,
    CARD_MONSTER_REBORN,
    /* Traps (5) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    CARD_CEASEFIRE,
    /* Fusion Deck (15) */
    X3(CARD_DARK_FLARE_KNIGHT),
    X3(CARD_ROARING_OCEAN_SNAKE),
    CARD_PUNISHED_EAGLE,
    X3(CARD_EMPRESS_JUDGE),
    CARD_RYU_SENSHI,
    X2(CARD_DARK_BALTER_THE_TERRIBLE),
    X2(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sVampireScientistFtk) == 40 + 15, VampireScientistFtkSize);
static const struct DeckList sVampireScientistFtkList = { sVampireScientistFtk, ARRAY_COUNT(sVampireScientistFtk), 0 };
/* ellietincan-Scientist FTK.ydk's Side Deck (15). */
static const u16 sVampireScientistFtkSide[] = {
    CARD_JINZO,
    CARD_D_D_WARRIOR_LADY,
    CARD_EXILED_FORCE,
    X3(CARD_MOTHER_GRIZZLY),
    CARD_SINISTER_SERPENT,
    CARD_SWORDS_OF_REVEALING_LIGHT,
    CARD_GIANT_TRUNADE,
    CARD_MAGIC_CYLINDER,
    X2(CARD_TORRENTIAL_TRIBUTE),
    X3(CARD_ROYAL_DECREE),
};
STATIC_ASSERT(ARRAY_COUNT(sVampireScientistFtkSide) == 15, VampireScientistFtkSideSize);
#endif

#if defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_FIBER_JAR) \
    && defined(HAS_CARD_INJECTION_FAIRY_LILY) && defined(HAS_CARD_MAGICAL_SCIENTIST) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) && defined(HAS_CARD_YATA_GARASU) && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) && defined(HAS_CARD_FIEND_SKULL_DRAGON) && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) && defined(HAS_CARD_GOBLIN_ATTACK_FORCE) \
    && defined(HAS_CARD_GUARDIAN_SPHINX) && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) && defined(HAS_CARD_COLD_WAVE) \
    && defined(HAS_CARD_MAGE_POWER) && defined(HAS_CARD_SCAPEGOAT) && defined(HAS_CARD_MAGIC_CYLINDER)
#define MOD_VAMPIRE_WARRIOR_TOMATO_CONTROL 1
static const u16 sVampireWarriorTomatoControl[] = {
    /* Monsters (18) */
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_EXILED_FORCE,
    CARD_FIBER_JAR,
    CARD_INJECTION_FAIRY_LILY,
    CARD_JINZO,
    CARD_MAGICAL_SCIENTIST,
    X2(CARD_MAGICIAN_OF_FAITH),
    X3(CARD_MYSTIC_TOMATO),
    CARD_SANGAN,
    CARD_SINISTER_SERPENT,
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    CARD_YATA_GARASU,
    X2(CARD_GEMINI_ELF),
    /* Spells (18) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_HEAVY_STORM,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_MONSTER_REBORN,
    X2(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_RAIGEKI,
    CARD_REINFORCEMENT_OF_THE_ARMY,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (4) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_TORRENTIAL_TRIBUTE,
    /* Fusion Deck (15) */
    X3(CARD_DARK_BALTER_THE_TERRIBLE),
    X3(CARD_FIEND_SKULL_DRAGON),
    X3(CARD_GILTIA_THE_D_KNIGHT),
    X3(CARD_RYU_SENSHI),
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sVampireWarriorTomatoControl) == 40 + 15, VampireWarriorTomatoControlSize);
static const struct DeckList sVampireWarriorTomatoControlList = { sVampireWarriorTomatoControl,
                                                                  ARRAY_COUNT(sVampireWarriorTomatoControl), 0 };
/* AJTBLS-Warrior Tomato Control.ydk's Side Deck (15). */
static const u16 sVampireWarriorTomatoControlSide[] = {
    CARD_ELECTRIC_SNAKE,
    CARD_GOBLIN_ATTACK_FORCE,
    CARD_GUARDIAN_SPHINX,
    CARD_KYCOO_THE_GHOST_DESTROYER,
    X2(CARD_MASKED_SORCERER),
    CARD_WHITE_MAGICAL_HAT,
    CARD_COLD_WAVE,
    CARD_MAGE_POWER,
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_PREMATURE_BURIAL,
    CARD_SCAPEGOAT,
    CARD_MAGIC_CYLINDER,
    X2(CARD_TIME_SEAL),
};
STATIC_ASSERT(ARRAY_COUNT(sVampireWarriorTomatoControlSide) == 15, VampireWarriorTomatoControlSideSize);
#endif

#if defined(MOD_VAMPIRE_WARRIOR_CONTROL) && defined(MOD_VAMPIRE_SCIENTIST_FTK) \
    && defined(MOD_VAMPIRE_WARRIOR_TOMATO_CONTROL)
#define MOD_VAMPIRE_DECKS 1
/* The Vampire opponent's deck for this duel: Warrior Control 8 in 10, Scientist FTK 1 in 10, Warrior Tomato Control
 * 1 in 10. */
static const struct DeckList *VampireDeck(void)
{
    u32 roll = (u32)Random() % 10;

    if (roll < 8)
        return &sVampireWarriorControlList;
    if (roll < 9)
        return &sVampireScientistFtkList;
    return &sVampireWarriorTomatoControlList;
}
#endif

/* The Chaos format opponent (Strings' slot, src/mod_opponents.c): one of two decks per duel (ChaosDeck): Chaos Control
 * 80%, Chaos Warrior 20% (decks/ellietincan-Chaos Control.ydk, decks/indoors-Chaos Warrior.ydk; main and extra decks).
 * Every card was already in the game: Chaos Warrior's Black Skull Dragon (11901678) is the original B. Skull Dragon,
 * which has no passcode in EDS, and its Dark Paladin (98502113) was added as 98502115. */
#if defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END) && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_INJECTION_FAIRY_LILY) \
    && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) && defined(HAS_CARD_MAGICAL_SCIENTIST) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_FLARE_KNIGHT) && defined(HAS_CARD_FIEND_SKULL_DRAGON) && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) && defined(HAS_CARD_CHAOS_SORCERER) && defined(HAS_CARD_FIBER_JAR) \
    && defined(HAS_CARD_SKILLED_DARK_MAGICIAN) && defined(HAS_CARD_YATA_GARASU) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) && defined(HAS_CARD_SAKURETSU_ARMOR)
#define MOD_CHAOS_CONTROL 1
static const u16 sChaosControl[] = {
    /* Monsters (17) */
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END,
    X3(CARD_D_D_WARRIOR_LADY),
    CARD_EXILED_FORCE,
    CARD_INJECTION_FAIRY_LILY,
    X2(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_MAGICAL_SCIENTIST,
    CARD_SANGAN,
    X2(CARD_SHINING_FAIRY),
    CARD_SINISTER_SERPENT,
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    /* Spells (17) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_DARK_HOLE,
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HARPIES_FEATHER_DUSTER,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_MONSTER_REBORN,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    CARD_RAIGEKI,
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (6) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    X2(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X2(CARD_DARK_BALTER_THE_TERRIBLE),
    X3(CARD_DARK_FLARE_KNIGHT),
    X2(CARD_FIEND_SKULL_DRAGON),
    X3(CARD_ROARING_OCEAN_SNAKE),
    X2(CARD_RYU_SENSHI),
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sChaosControl) == 40 + 15, ChaosControlSize);
static const struct DeckList sChaosControlList = { sChaosControl, ARRAY_COUNT(sChaosControl), 0 };
/* ellietincan-Chaos Control.ydk's Side Deck (15). */
static const u16 sChaosControlSide[] = {
    CARD_CHAOS_SORCERER,
    CARD_FIBER_JAR,
    CARD_KYCOO_THE_GHOST_DESTROYER,
    X2(CARD_MAGICIAN_OF_FAITH),
    CARD_SKILLED_DARK_MAGICIAN,
    CARD_YATA_GARASU,
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    X3(CARD_SAKURETSU_ARMOR),
    CARD_TORRENTIAL_TRIBUTE,
};
STATIC_ASSERT(ARRAY_COUNT(sChaosControlSide) == 15, ChaosControlSideSize);
#endif

#if defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) && defined(HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) && defined(HAS_CARD_EXILED_FORCE) && defined(HAS_CARD_YATA_GARASU) \
    && defined(HAS_CARD_FIBER_JAR) && defined(HAS_CARD_MAGICAL_SCIENTIST) && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_THUNDER_NYAN_NYAN) && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_RYU_SENSHI) && defined(HAS_CARD_FIEND_SKULL_DRAGON) && defined(HAS_CARD_DARK_PALADIN) \
    && defined(HAS_CARD_SAKURETSU_ARMOR)
#define MOD_CHAOS_WARRIOR 1
static const u16 sChaosWarrior[] = {
    /* Monsters (17) */
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_SANGAN,
    CARD_WITCH_OF_THE_BLACK_FOREST,
    X2(CARD_KYCOO_THE_GHOST_DESTROYER),
    X3(CARD_D_D_WARRIOR_LADY),
    CARD_EXILED_FORCE,
    CARD_YATA_GARASU,
    CARD_FIBER_JAR,
    CARD_MAGICAL_SCIENTIST,
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_MAGICIAN_OF_FAITH,
    CARD_THUNDER_NYAN_NYAN,
    /* Spells (17) */
    CARD_POT_OF_GREED,
    CARD_GRACEFUL_CHARITY,
    CARD_DELINQUENT_DUO,
    CARD_CONFISCATION,
    CARD_THE_FORCEFUL_SENTRY,
    CARD_RAIGEKI,
    CARD_DARK_HOLE,
    CARD_MONSTER_REBORN,
    CARD_CHANGE_OF_HEART,
    CARD_SNATCH_STEAL,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_PAINFUL_CHOICE,
    CARD_HARPIES_FEATHER_DUSTER,
    X3(CARD_MYSTICAL_SPACE_TYPHOON),
    CARD_REINFORCEMENT_OF_THE_ARMY,
    /* Traps (6) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_IMPERIAL_ORDER,
    CARD_MIRROR_FORCE,
    CARD_BELL_OF_DESTRUCTION,
    X2(CARD_TORRENTIAL_TRIBUTE),
    /* Fusion Deck (15) */
    X3(CARD_THOUSAND_EYES_RESTRICT),
    X3(CARD_DARK_BALTER_THE_TERRIBLE),
    X3(CARD_RYU_SENSHI),
    X3(CARD_FIEND_SKULL_DRAGON),
    CARD_B_SKULL_DRAGON,
    CARD_FUSIONIST,
    CARD_DARK_PALADIN,
};
STATIC_ASSERT(ARRAY_COUNT(sChaosWarrior) == 40 + 15, ChaosWarriorSize);
static const struct DeckList sChaosWarriorList = { sChaosWarrior, ARRAY_COUNT(sChaosWarrior), 0 };
/* indoors-Chaos Warrior.ydk's Side Deck (15). */
static const u16 sChaosWarriorSide[] = {
    CARD_MORPHING_JAR,
    CARD_KYCOO_THE_GHOST_DESTROYER,
    X2(CARD_THUNDER_NYAN_NYAN),
    CARD_PREMATURE_BURIAL,
    CARD_HEAVY_STORM,
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_SOUL_RELEASE,
    CARD_TORRENTIAL_TRIBUTE,
    X2(CARD_WABOKU),
    X3(CARD_SAKURETSU_ARMOR),
};
STATIC_ASSERT(ARRAY_COUNT(sChaosWarriorSide) == 15, ChaosWarriorSideSize);
#endif

#if defined(MOD_CHAOS_CONTROL) && defined(MOD_CHAOS_WARRIOR)
#define MOD_CHAOS_DECKS 1
/* The Chaos opponent's deck for this duel: Chaos Control 8 in 10, Chaos Warrior 2 in 10. */
static const struct DeckList *ChaosDeck(void)
{
    return (u32)Random() % 10 < 8 ? &sChaosControlList : &sChaosWarriorList;
}
#endif

/* The Warrior format opponent (Kaiba's slot, src/mod_opponents.c): one of four decks per duel (WarriorDeck): Goat
 * Control 40%, Warrior 30%, Warrior Control 20%, Chaos Return 10% (decks/Gyomberman-Goat Control.ydk,
 * decks/ellietincan-Warrior.ydk, decks/Patrick Smith-Warrior Control.ydk, decks/paoloNS-Chaos Return.ydk; main and
 * extra decks). Every card was already in the game. */
#if defined(HAS_CARD_ASURA_PRIEST) \
    && defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_EXILED_FORCE) \
    && defined(HAS_CARD_GRAVEKEEPERS_SPY) \
    && defined(HAS_CARD_INJECTION_FAIRY_LILY) \
    && defined(HAS_CARD_ZABORG_THE_THUNDER_MONARCH) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_TSUKUYOMI) \
    && defined(HAS_CARD_ZOMBYRA_THE_DARK) \
    && defined(HAS_CARD_CREATURE_SWAP) \
    && defined(HAS_CARD_METAMORPHOSIS) \
    && defined(HAS_CARD_BOOK_OF_MOON) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_SCAPEGOAT) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_MASTER_OF_OZ) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_ST_JOAN) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_CIPHER_SOLDIER) \
    && defined(HAS_CARD_GYAKU_GIRE_PANDA) \
    && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) \
    && defined(HAS_CARD_NINJA_GRANDMASTER_SASUKE) \
    && defined(HAS_CARD_MAGICAL_SCIENTIST) \
    && defined(HAS_CARD_SAKURETSU_ARMOR) \
    && defined(HAS_CARD_TRAP_DUSTSHOOT) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE)
#define MOD_WARRIOR_GOAT_CONTROL 1
static const u16 sWarriorGoatControl[] = {
    /* Monsters (17) */
    CARD_ASURA_PRIEST,
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    X3(CARD_D_D_WARRIOR_LADY),
    CARD_EXILED_FORCE,
    X2(CARD_GRAVEKEEPERS_SPY),
    CARD_INJECTION_FAIRY_LILY,
    X2(CARD_MAGICIAN_OF_FAITH),
    CARD_ZABORG_THE_THUNDER_MONARCH,
    CARD_SINISTER_SERPENT,
    CARD_TRIBE_INFECTING_VIRUS,
    CARD_TSUKUYOMI,
    CARD_ZOMBYRA_THE_DARK,
    /* Spells (19) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_PAINFUL_CHOICE,
    CARD_THE_FORCEFUL_SENTRY,
    CARD_POT_OF_GREED,
    CARD_HEAVY_STORM,
    X2(CARD_CREATURE_SWAP),
    X2(CARD_METAMORPHOSIS),
    CARD_BOOK_OF_MOON,
    CARD_MYSTICAL_SPACE_TYPHOON,
    CARD_NOBLEMAN_OF_CROSSOUT,
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    X3(CARD_SCAPEGOAT),
    CARD_SNATCH_STEAL,
    /* Traps (4) */
    CARD_BELL_OF_DESTRUCTION,
    X3(CARD_ROYAL_DECREE),
    /* Fusion Deck (15) */
    CARD_DARK_BALTER_THE_TERRIBLE,
    CARD_DARK_BLADE_THE_DRAGON_KNIGHT,
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    CARD_FIEND_SKULL_DRAGON,
    CARD_MASTER_OF_OZ,
    CARD_OJAMA_KING,
    CARD_REAPER_ON_THE_NIGHTMARE,
    X2(CARD_RYU_SENSHI),
    CARD_ST_JOAN,
    CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET,
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sWarriorGoatControl) == 40 + 15, WarriorGoatControlSize);
static const struct DeckList sWarriorGoatControlList = { sWarriorGoatControl, ARRAY_COUNT(sWarriorGoatControl), 0 };
/* Gyomberman-Goat Control.ydk's Side Deck (15). */
static const u16 sWarriorGoatControlSide[] = {
    CARD_CIPHER_SOLDIER,
    CARD_GYAKU_GIRE_PANDA,
    CARD_MYSTIC_SWORDSMAN_LV2,
    CARD_NINJA_GRANDMASTER_SASUKE,
    CARD_ZOMBYRA_THE_DARK,
    CARD_MAGICAL_SCIENTIST,
    CARD_TSUKUYOMI,
    CARD_NOBLEMAN_OF_CROSSOUT,
    X3(CARD_SAKURETSU_ARMOR),
    X3(CARD_TRAP_DUSTSHOOT),
    CARD_TORRENTIAL_TRIBUTE,
};
STATIC_ASSERT(ARRAY_COUNT(sWarriorGoatControlSide) == 15, WarriorGoatControlSideSize);
#endif

#if defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_DON_ZALOOG) \
    && defined(HAS_CARD_EXILED_FORCE) \
    && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_POISON_OF_THE_OLD_MAN) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_CHAIN_DISAPPEARANCE) \
    && defined(HAS_CARD_COMPULSORY_EVACUATION_DEVICE) \
    && defined(HAS_CARD_HALLOWED_LIFE_BARRIER) \
    && defined(HAS_CARD_MAGIC_DRAIN) \
    && defined(HAS_CARD_RAIGEKI_BREAK) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_DARK_FLARE_KNIGHT) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_BLADE_KNIGHT) \
    && defined(HAS_CARD_CHAOS_SORCERER) \
    && defined(HAS_CARD_MAGICAL_SCIENTIST) \
    && defined(HAS_CARD_SMASHING_GROUND) \
    && defined(HAS_CARD_SAKURETSU_ARMOR)
#define MOD_WARRIOR_WARRIOR 1
static const u16 sWarriorWarrior[] = {
    /* Monsters (10) */
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    X3(CARD_D_D_WARRIOR_LADY),
    CARD_DON_ZALOOG,
    CARD_EXILED_FORCE,
    X3(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_TRIBE_INFECTING_VIRUS,
    /* Spells (9) */
    CARD_CONFISCATION,
    CARD_PAINFUL_CHOICE,
    X3(CARD_POISON_OF_THE_OLD_MAN),
    CARD_POT_OF_GREED,
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (21) */
    X3(CARD_CHAIN_DISAPPEARANCE),
    X3(CARD_COMPULSORY_EVACUATION_DEVICE),
    CARD_HALLOWED_LIFE_BARRIER,
    X3(CARD_MAGIC_DRAIN),
    X3(CARD_MAGIC_JAMMER),
    X3(CARD_RAIGEKI_BREAK),
    CARD_BELL_OF_DESTRUCTION,
    X3(CARD_SOLEMN_JUDGMENT),
    CARD_TORRENTIAL_TRIBUTE,
    /* Fusion Deck (15) */
    X2(CARD_DARK_BALTER_THE_TERRIBLE),
    X3(CARD_DARK_BLADE_THE_DRAGON_KNIGHT),
    X3(CARD_DARK_FLARE_KNIGHT),
    X2(CARD_FIEND_SKULL_DRAGON),
    X2(CARD_RYU_SENSHI),
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sWarriorWarrior) == 40 + 15, WarriorWarriorSize);
static const struct DeckList sWarriorWarriorList = { sWarriorWarrior, ARRAY_COUNT(sWarriorWarrior), 0 };
/* ellietincan-Warrior.ydk's Side Deck (15). */
static const u16 sWarriorWarriorSide[] = {
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    X2(CARD_BLADE_KNIGHT),
    CARD_CHAOS_SORCERER,
    CARD_MAGICAL_SCIENTIST,
    CARD_CHANGE_OF_HEART,
    CARD_HEAVY_STORM,
    CARD_MYSTICAL_SPACE_TYPHOON,
    X3(CARD_SMASHING_GROUND),
    CARD_SNATCH_STEAL,
    X3(CARD_SAKURETSU_ARMOR),
};
STATIC_ASSERT(ARRAY_COUNT(sWarriorWarriorSide) == 15, WarriorWarriorSideSize);
#endif

#if defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_BLADE_KNIGHT) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_D_D_ASSAILANT) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_FIBER_JAR) \
    && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_MAGICAL_SCIENTIST) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_EMERGENCY_PROVISIONS) \
    && defined(HAS_CARD_ENEMY_CONTROLLER) \
    && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_SCAPEGOAT) \
    && defined(HAS_CARD_SMASHING_GROUND) \
    && defined(HAS_CARD_SAKURETSU_ARMOR) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_CIPHER_SOLDIER) \
    && defined(HAS_CARD_HALLOWED_LIFE_BARRIER) \
    && defined(HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT)
#define MOD_WARRIOR_WARRIOR_CONTROL 1
static const u16 sWarriorWarriorControl[] = {
    /* Monsters (16) */
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    X2(CARD_BLADE_KNIGHT),
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    X3(CARD_D_D_ASSAILANT),
    X3(CARD_D_D_WARRIOR_LADY),
    CARD_FIBER_JAR,
    X2(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_MAGICAL_SCIENTIST,
    CARD_SINISTER_SERPENT,
    CARD_TRIBE_INFECTING_VIRUS,
    /* Spells (18) */
    CARD_EMERGENCY_PROVISIONS,
    X3(CARD_ENEMY_CONTROLLER),
    CARD_HEAVY_STORM,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_MYSTICAL_SPACE_TYPHOON,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    X3(CARD_SCAPEGOAT),
    X2(CARD_SMASHING_GROUND),
    CARD_SNATCH_STEAL,
    /* Traps (6) */
    CARD_CALL_OF_THE_HAUNTED,
    CARD_BELL_OF_DESTRUCTION,
    X3(CARD_SAKURETSU_ARMOR),
    CARD_TORRENTIAL_TRIBUTE,
    /* Fusion Deck (15) */
    X2(CARD_DARK_BALTER_THE_TERRIBLE),
    X2(CARD_DARK_BLADE_THE_DRAGON_KNIGHT),
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    CARD_FIEND_SKULL_DRAGON,
    CARD_GILTIA_THE_D_KNIGHT,
    CARD_OJAMA_KING,
    CARD_REAPER_ON_THE_NIGHTMARE,
    X2(CARD_RYU_SENSHI),
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sWarriorWarriorControl) == 40 + 15, WarriorWarriorControlSize);
static const struct DeckList sWarriorWarriorControlList = { sWarriorWarriorControl, ARRAY_COUNT(sWarriorWarriorControl), 0 };
/* Patrick Smith-Warrior Control.ydk's Side Deck (15). */
static const u16 sWarriorWarriorControlSide[] = {
    CARD_CIPHER_SOLDIER,
    CARD_JINZO,
    CARD_CONFISCATION,
    X2(CARD_GIANT_TRUNADE),
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_THE_FORCEFUL_SENTRY,
    CARD_CEASEFIRE,
    X3(CARD_DUST_TORNADO),
    X2(CARD_HALLOWED_LIFE_BARRIER),
    CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT,
};
STATIC_ASSERT(ARRAY_COUNT(sWarriorWarriorControlSide) == 15, WarriorWarriorControlSideSize);
#endif

#if defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_BLADE_KNIGHT) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_CHAOS_SORCERER) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_DON_ZALOOG) \
    && defined(HAS_CARD_EXILED_FORCE) \
    && defined(HAS_CARD_FIBER_JAR) \
    && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_MAGICAL_MERCHANT) \
    && defined(HAS_CARD_MAGICAL_SCIENTIST) \
    && defined(HAS_CARD_SPIRIT_REAPER) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_MIRAGE_OF_NIGHTMARE) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_SMASHING_GROUND) \
    && defined(HAS_CARD_MAGIC_CYLINDER) \
    && defined(HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION) \
    && defined(HAS_CARD_SAKURETSU_ARMOR) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_CIPHER_SOLDIER) \
    && defined(HAS_CARD_KING_TIGER_WANGHU) \
    && defined(HAS_CARD_NINJA_GRANDMASTER_SASUKE) \
    && defined(HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT)
#define MOD_WARRIOR_CHAOS_RETURN 1
static const u16 sWarriorChaosReturn[] = {
    /* Monsters (19) */
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    CARD_BLADE_KNIGHT,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_CHAOS_SORCERER,
    X3(CARD_D_D_WARRIOR_LADY),
    CARD_DON_ZALOOG,
    CARD_EXILED_FORCE,
    CARD_FIBER_JAR,
    X2(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_MAGICAL_MERCHANT,
    CARD_MAGICAL_SCIENTIST,
    CARD_MAGICIAN_OF_FAITH,
    CARD_MORPHING_JAR,
    CARD_SINISTER_SERPENT,
    CARD_SPIRIT_REAPER,
    CARD_TRIBE_INFECTING_VIRUS,
    /* Spells (15) */
    CARD_CHANGE_OF_HEART,
    CARD_CONFISCATION,
    CARD_HEAVY_STORM,
    CARD_MIRAGE_OF_NIGHTMARE,
    CARD_MYSTICAL_SPACE_TYPHOON,
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_PAINFUL_CHOICE,
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    X2(CARD_REINFORCEMENT_OF_THE_ARMY),
    X2(CARD_SMASHING_GROUND),
    CARD_SNATCH_STEAL,
    CARD_THE_FORCEFUL_SENTRY,
    /* Traps (6) */
    CARD_MAGIC_CYLINDER,
    X2(CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION),
    CARD_BELL_OF_DESTRUCTION,
    CARD_SAKURETSU_ARMOR,
    CARD_TORRENTIAL_TRIBUTE,
    /* Fusion Deck (15) */
    X3(CARD_DARK_BALTER_THE_TERRIBLE),
    X3(CARD_DARK_BLADE_THE_DRAGON_KNIGHT),
    X2(CARD_EMPRESS_JUDGE),
    CARD_OJAMA_KING,
    X3(CARD_RYU_SENSHI),
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sWarriorChaosReturn) == 40 + 15, WarriorChaosReturnSize);
static const struct DeckList sWarriorChaosReturnList = { sWarriorChaosReturn, ARRAY_COUNT(sWarriorChaosReturn), 0 };
/* paoloNS-Chaos Return.ydk's Side Deck (15). */
static const u16 sWarriorChaosReturnSide[] = {
    X3(CARD_CIPHER_SOLDIER),
    CARD_JINZO,
    X3(CARD_KING_TIGER_WANGHU),
    CARD_NINJA_GRANDMASTER_SASUKE,
    CARD_CEASEFIRE,
    X2(CARD_DUST_TORNADO),
    X2(CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT),
    X2(CARD_SAKURETSU_ARMOR),
};
STATIC_ASSERT(ARRAY_COUNT(sWarriorChaosReturnSide) == 15, WarriorChaosReturnSideSize);
#endif
#if defined(MOD_WARRIOR_GOAT_CONTROL) && defined(MOD_WARRIOR_WARRIOR) && defined(MOD_WARRIOR_WARRIOR_CONTROL) \
    && defined(MOD_WARRIOR_CHAOS_RETURN)
#define MOD_WARRIOR_DECKS 1
/* The Warrior opponent's deck for this duel: Goat Control 4 in 10, Warrior 3 in 10, Warrior Control 2 in 10, Chaos
 * Return 1 in 10. */
static const struct DeckList *WarriorDeck(void)
{
    u32 roll = (u32)Random() % 10;

    if (roll < 4)
        return &sWarriorGoatControlList;
    if (roll < 7)
        return &sWarriorWarriorList;
    if (roll < 9)
        return &sWarriorWarriorControlList;
    return &sWarriorChaosReturnList;
}
#endif

/* The testing opponent (make only; the Duel Computer's slot, src/mod_opponents.c): the deck of the .ydk in TEST_YDK
 * (the Makefile), made into build/include/test_deck.h by tools/testdeck.py on every build. LoadOpponentDeck leaves
 * its copy counts as they are (no RemoveOverLimitDeckCards). */
#if !MOD_RELEASE
#define TEST_DECK_WANT_CARDS
#endif
#include "test_deck.h"
#if defined(MOD_TEST_DECK) && !MOD_RELEASE
static const struct DeckList sTestDeckList = { sTestDeck, ARRAY_COUNT(sTestDeck), 0 };
#endif

/* The deck of Solomon and Debug (Grandpa's and Pegasus' slots on the deck opponents' page, src/mod_opponents.c, in both
 * builds): the Deck Recipe the player picks or builds before the duel (CB_OpponentRecipes, src/mod_recipes.c), as card
 * numbers: its Main Deck, then its Fusion Deck (LoadOpponentDeck puts Fusion Monsters there); its Side Deck is not
 * used. gModOpponentRecipe (EWRAM 0x0202F400, symbols.ld) keeps it for the whole match. LoadOpponentDeck leaves its
 * copy counts as they are. */
#define MOD_OPPONENT_RECIPE_VALID 0x50C0

struct ModOpponentRecipe {
    struct DeckList list;
    u16 valid;                                          /* MOD_OPPONENT_RECIPE_VALID: list is set */
    u16 building;                                       /* MOD_RECIPE_BUILDING: ModRecipeBuildBegin's collection */
    u16 cards[DECK_MAX_CARDS + FUSION_DECK_MAX_CARDS];
    u16 buildLimit;                                     /* while building: the copies a deck may hold of a card */
    u16 pad;
};
extern struct ModOpponentRecipe gModOpponentRecipe;

/* CB_OpponentRecipes: the picked recipe's Main Deck and Fusion Deck (card IDs). */
void ModSetOpponentRecipe(const u16 *deck, int deckCount, const u16 *fusion, int fusionCount)
{
    int i, n = 0;

    for (i = 0; i < deckCount && n < DECK_MAX_CARDS; i++)
        gModOpponentRecipe.cards[n++] = gCardIdToNumber[deck[i] & CARD_ID_MASK];
    for (i = 0; i < fusionCount && i < FUSION_DECK_MAX_CARDS; i++)
        gModOpponentRecipe.cards[n++] = gCardIdToNumber[fusion[i] & CARD_ID_MASK];
    gModOpponentRecipe.list.cards = gModOpponentRecipe.cards;
    gModOpponentRecipe.list.count = n;
    gModOpponentRecipe.list.pad = 0;
    gModOpponentRecipe.valid = MOD_OPPONENT_RECIPE_VALID;
}

/* Building the deck of Solomon or Debug (CB_OpponentRecipes, SELECT): the player's save is written to SRAM (SaveGame),
 * then gSaveData gets a temporary collection: `copies` of every card in the trunk (the cards of GiveStartingTrunk)
 * and empty decks, and GetBanlistCopyLimit gives `limit` for every card, ignoring the banlists (src/mod_banlists.c).
 * Deck Edit and the recipes then work on it as usual; SaveGame (main.c) writes nothing meanwhile. ModRecipeBuildEnd
 * reads the collection back from SRAM, so the player's own cards, decks and banlist return as they were (a reset also
 * finds them there). Against Debug the player's own deck comes from such a collection too (CB_OpponentRecipes): it
 * stays for the whole match, and ModMatchRestorePlayerDeck ends it when the match is decided. */
#define MOD_RECIPE_BUILDING     0x5B1D

int ModRecipeBuilding(void)
{
    return gModOpponentRecipe.building == MOD_RECIPE_BUILDING;
}

/* GetBanlistCopyLimit while building. */
int ModRecipeBuildLimit(void)
{
    return gModOpponentRecipe.buildLimit;
}

void ModRecipeBuildBegin(int copies, int limit)
{
    u32 id, total = 0;

    SaveGame();
    for (id = 1; id < ARRAY_COUNT(gSaveData.trunk); id++) {
        u16 number = id <= CARD_ID_LAST ? gCardIdToNumber[id] : 0xFFFF;
        int n = number == 0xFFFF
                || (u16)(number - CARD_NUMBER_TOKEN_FIRST) < CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST
                ? 0 : copies;

        gSaveData.trunk[id].count = n;
        SetTrunkDeckCopies(id, 0);
        SetTrunkSideCopies(id, 0);
        SetTrunkFusionCopies(id, 0);
        total += n;
    }
    gSaveData.trunkSize = total;
    gSaveData.deckSize = 0;
    gSaveData.sideDeckSize = 0;
    gSaveData.fusionDeckSize = 0;
    gModOpponentRecipe.buildLimit = limit;
    gModOpponentRecipe.building = MOD_RECIPE_BUILDING;
}

/* Only the collection comes back from SRAM (the trunk, the three decks and the banlist: gSaveData +0x8 to +0x20D0), so
 * what changed meanwhile (the duel records, the day of a duel with Debug) stays. */
void ModRecipeBuildEnd(void)
{
    if (!ModRecipeBuilding())
        return;
    ReadSram((u8 *)SRAM + 8, (u8 *)gSaveData.trunk, (u32)&gSaveData.duelRecords - (u32)gSaveData.trunk);
    gModOpponentRecipe.building = 0;
}

/* The Goat format opponent (Ishizu's slot, src/mod_opponents.c): one of seven decks per duel (GoatDeck): LR-Chaos
 * Turbo 40%, sdlkilla-Warrior 20%, geistD-Goat Control 15% (the deck opponents' decks, above), Chaos Control 10%,
 * Panda Burn, Chaos Return and Reasoning Gate Turbo 5% each (decks/N\u2661va-Chaos Control.ydk,
 * decks/wolfe_KMH-Panda Burn.ydk, decks/Dingosig-Chaos Return.ydk, decks/paede17-Reasoning Gate Turbo.ydk; main and
 * extra decks). Every card was already in the game. */
#if defined(HAS_CARD_GRAVEKEEPERS_SPY) \
    && defined(HAS_CARD_SPIRIT_REAPER) \
    && defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_CHAOS_SORCERER) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_TSUKUYOMI) \
    && defined(HAS_CARD_METAMORPHOSIS) \
    && defined(HAS_CARD_SCAPEGOAT) \
    && defined(HAS_CARD_JAR_OF_GREED) \
    && defined(HAS_CARD_RAIGEKI_BREAK) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_GATLING_DRAGON) \
    && defined(HAS_CARD_KING_DRAGUN) \
    && defined(HAS_CARD_MASTER_OF_OZ) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) \
    && defined(HAS_CARD_MIND_CONTROL) \
    && defined(HAS_CARD_TRAP_DUSTSHOOT)
#define MOD_GOAT_CHAOS_CONTROL 1
static const u16 sGoatChaosControl[] = {
    /* Monsters (18) */
    X2(CARD_GRAVEKEEPERS_SPY),
    CARD_MYSTIC_TOMATO,
    X3(CARD_MAGICIAN_OF_FAITH),
    X3(CARD_THUNDER_DRAGON),
    CARD_SINISTER_SERPENT,
    X3(CARD_SPIRIT_REAPER),
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    CARD_CHAOS_SORCERER,
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_SANGAN,
    CARD_TSUKUYOMI,
    /* Spells (14) */
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_POT_OF_GREED,
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_SNATCH_STEAL,
    CARD_HEAVY_STORM,
    X3(CARD_METAMORPHOSIS),
    X2(CARD_SCAPEGOAT),
    X2(CARD_UPSTART_GOBLIN),
    /* Traps (8) */
    X3(CARD_JAR_OF_GREED),
    X2(CARD_RAIGEKI_BREAK),
    CARD_BELL_OF_DESTRUCTION,
    CARD_TORRENTIAL_TRIBUTE,
    CARD_MIRROR_FORCE,
    /* Fusion Deck (15) */
    CARD_DARK_BALTER_THE_TERRIBLE,
    CARD_DARK_BLADE_THE_DRAGON_KNIGHT,
    CARD_DARKFIRE_DRAGON,
    CARD_DRAGONESS_THE_WICKED_KNIGHT,
    CARD_FIEND_SKULL_DRAGON,
    CARD_GATLING_DRAGON,
    CARD_KING_DRAGUN,
    CARD_MASTER_OF_OZ,
    CARD_OJAMA_KING,
    CARD_REAPER_ON_THE_NIGHTMARE,
    CARD_RYU_SENSHI,
    CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET,
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sGoatChaosControl) == 40 + 15, GoatChaosControlSize);
static const struct DeckList sGoatChaosControlList = { sGoatChaosControl, ARRAY_COUNT(sGoatChaosControl), 0 };
/* N\u2661va-Chaos Control.ydk's Side Deck (15). */
static const u16 sGoatChaosControlSide[] = {
    CARD_TSUKUYOMI,
    X3(CARD_KYCOO_THE_GHOST_DESTROYER),
    CARD_MYSTIC_SWORDSMAN_LV2,
    CARD_MYSTICAL_SPACE_TYPHOON,
    X2(CARD_MIND_CONTROL),
    X3(CARD_MESSENGER_OF_PEACE),
    X3(CARD_ROYAL_DECREE),
    CARD_TRAP_DUSTSHOOT,
};
STATIC_ASSERT(ARRAY_COUNT(sGoatChaosControlSide) == 15, GoatChaosControlSideSize);
#endif

#if defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_EXILED_FORCE) \
    && defined(HAS_CARD_GYAKU_GIRE_PANDA) \
    && defined(HAS_CARD_INJECTION_FAIRY_LILY) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_BOOK_OF_MOON) \
    && defined(HAS_CARD_OJAMA_TRIO) \
    && defined(HAS_CARD_DES_KOALA) \
    && defined(HAS_CARD_LEVEL_LIMIT_AREA_B) \
    && defined(HAS_CARD_SECRET_BARREL) \
    && defined(HAS_CARD_MAGIC_CYLINDER) \
    && defined(HAS_CARD_GRAVITY_BIND) \
    && defined(HAS_CARD_MASTER_OF_OZ) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_GATLING_DRAGON) \
    && defined(HAS_CARD_KING_DRAGUN) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_DARK_FLARE_KNIGHT) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_GOBLIN_ATTACK_FORCE) \
    && defined(HAS_CARD_D_D_ASSAILANT) \
    && defined(HAS_CARD_GIGANTES) \
    && defined(HAS_CARD_KYCOO_THE_GHOST_DESTROYER) \
    && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) \
    && defined(HAS_CARD_REINFORCEMENT_OF_THE_ARMY) \
    && defined(HAS_CARD_FINAL_ATTACK_ORDERS)
#define MOD_GOAT_PANDA_BURN 1
static const u16 sGoatPandaBurn[] = {
    /* Monsters (14) */
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_CYBER_JAR,
    CARD_EXILED_FORCE,
    X3(CARD_GIANT_RAT),
    X3(CARD_GYAKU_GIRE_PANDA),
    CARD_INJECTION_FAIRY_LILY,
    CARD_MORPHING_JAR,
    CARD_D_D_WARRIOR_LADY,
    CARD_BIG_SHIELD_GARDNA,
    CARD_DES_KOALA,
    /* Spells (13) */
    CARD_POT_OF_GREED,
    CARD_GRACEFUL_CHARITY,
    X3(CARD_GIANT_TRUNADE),
    CARD_PREMATURE_BURIAL,
    CARD_SNATCH_STEAL,
    X2(CARD_BOOK_OF_MOON),
    CARD_HEAVY_STORM,
    CARD_SWORDS_OF_REVEALING_LIGHT,
    X2(CARD_LEVEL_LIMIT_AREA_B),
    /* Traps (13) */
    CARD_CEASEFIRE,
    X3(CARD_OJAMA_TRIO),
    CARD_BELL_OF_DESTRUCTION,
    X3(CARD_JUST_DESSERTS),
    X3(CARD_SECRET_BARREL),
    CARD_MAGIC_CYLINDER,
    CARD_GRAVITY_BIND,
    /* Fusion Deck (15) */
    CARD_MASTER_OF_OZ,
    CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET,
    CARD_GATLING_DRAGON,
    CARD_KING_DRAGUN,
    CARD_DARK_BLADE_THE_DRAGON_KNIGHT,
    CARD_OJAMA_KING,
    CARD_DARK_FLARE_KNIGHT,
    CARD_RYU_SENSHI,
    CARD_DARK_BALTER_THE_TERRIBLE,
    CARD_FIEND_SKULL_DRAGON,
    CARD_REAPER_ON_THE_NIGHTMARE,
    CARD_DARKFIRE_DRAGON,
    CARD_FLAME_GHOST,
    X2(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sGoatPandaBurn) == 40 + 15, GoatPandaBurnSize);
static const struct DeckList sGoatPandaBurnList = { sGoatPandaBurn, ARRAY_COUNT(sGoatPandaBurn), 0 };
/* wolfe_KMH-Panda Burn.ydk's Side Deck (15). */
static const u16 sGoatPandaBurnSide[] = {
    X3(CARD_GOBLIN_ATTACK_FORCE),
    X2(CARD_D_D_ASSAILANT),
    X2(CARD_GIGANTES),
    CARD_KYCOO_THE_GHOST_DESTROYER,
    CARD_MYSTIC_SWORDSMAN_LV2,
    CARD_NOBLEMAN_OF_CROSSOUT,
    CARD_REINFORCEMENT_OF_THE_ARMY,
    X3(CARD_FINAL_ATTACK_ORDERS),
    CARD_MIRROR_FORCE,
};
STATIC_ASSERT(ARRAY_COUNT(sGoatPandaBurnSide) == 15, GoatPandaBurnSideSize);
#endif

#if defined(HAS_CARD_BAZOO_THE_SOUL_EATER) \
    && defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_BLADE_KNIGHT) \
    && defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) \
    && defined(HAS_CARD_CHAOS_SORCERER) \
    && defined(HAS_CARD_D_D_WARRIOR_LADY) \
    && defined(HAS_CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE) \
    && defined(HAS_CARD_GRAVEKEEPERS_SPY) \
    && defined(HAS_CARD_ZABORG_THE_THUNDER_MONARCH) \
    && defined(HAS_CARD_BOOK_OF_MOON) \
    && defined(HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE) \
    && defined(HAS_CARD_TRAP_DUSTSHOOT) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_GATLING_DRAGON) \
    && defined(HAS_CARD_KING_DRAGUN) \
    && defined(HAS_CARD_MASTER_OF_OZ) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_ASURA_PRIEST) \
    && defined(HAS_CARD_MOBIUS_THE_FROST_MONARCH) \
    && defined(HAS_CARD_ROULETTE_BARREL) \
    && defined(HAS_CARD_TRIBE_INFECTING_VIRUS) \
    && defined(HAS_CARD_MIND_CONTROL) \
    && defined(HAS_CARD_MYSTIK_WOK) \
    && defined(HAS_CARD_SAKURETSU_ARMOR) \
    && defined(HAS_CARD_XING_ZHEN_HU)
#define MOD_GOAT_CHAOS_RETURN 1
static const u16 sGoatChaosReturn[] = {
    /* Monsters (18) */
    CARD_BAZOO_THE_SOUL_EATER,
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    X2(CARD_BLADE_KNIGHT),
    CARD_BREAKER_THE_MAGICAL_WARRIOR,
    CARD_CHAOS_SORCERER,
    CARD_D_D_WARRIOR_LADY,
    X3(CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE),
    X3(CARD_GRAVEKEEPERS_SPY),
    CARD_SANGAN,
    X2(CARD_SHINING_FAIRY),
    X2(CARD_ZABORG_THE_THUNDER_MONARCH),
    /* Spells (10) */
    X2(CARD_BOOK_OF_MOON),
    CARD_DELINQUENT_DUO,
    CARD_GRACEFUL_CHARITY,
    CARD_HEAVY_STORM,
    CARD_MYSTICAL_SPACE_TYPHOON,
    X2(CARD_NOBLEMAN_OF_CROSSOUT),
    CARD_POT_OF_GREED,
    CARD_SNATCH_STEAL,
    /* Traps (12) */
    X2(CARD_DUST_TORNADO),
    CARD_MIRROR_FORCE,
    X2(CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION),
    CARD_BELL_OF_DESTRUCTION,
    X3(CARD_SOLEMN_JUDGMENT),
    CARD_TORRENTIAL_TRIBUTE,
    X2(CARD_TRAP_DUSTSHOOT),
    /* Fusion Deck (15) */
    X2(CARD_DARK_BALTER_THE_TERRIBLE),
    CARD_DARK_BLADE_THE_DRAGON_KNIGHT,
    X2(CARD_FIEND_SKULL_DRAGON),
    CARD_GATLING_DRAGON,
    X2(CARD_KING_DRAGUN),
    CARD_MASTER_OF_OZ,
    CARD_OJAMA_KING,
    CARD_REAPER_ON_THE_NIGHTMARE,
    CARD_RYU_SENSHI,
    X2(CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET),
    CARD_THOUSAND_EYES_RESTRICT,
};
STATIC_ASSERT(ARRAY_COUNT(sGoatChaosReturn) == 40 + 15, GoatChaosReturnSize);
static const struct DeckList sGoatChaosReturnList = { sGoatChaosReturn, ARRAY_COUNT(sGoatChaosReturn), 0 };
/* Dingosig-Chaos Return.ydk's Side Deck (15). */
static const u16 sGoatChaosReturnSide[] = {
    CARD_ASURA_PRIEST,
    CARD_KURIBOH,
    CARD_MOBIUS_THE_FROST_MONARCH,
    CARD_ROULETTE_BARREL,
    CARD_TRIBE_INFECTING_VIRUS,
    X2(CARD_MIND_CONTROL),
    X2(CARD_MYSTIK_WOK),
    CARD_SWORDS_OF_REVEALING_LIGHT,
    CARD_DUST_TORNADO,
    X3(CARD_SAKURETSU_ARMOR),
    CARD_XING_ZHEN_HU,
};
STATIC_ASSERT(ARRAY_COUNT(sGoatChaosReturnSide) == 15, GoatChaosReturnSideSize);
#endif

#if defined(HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING) \
    && defined(HAS_CARD_CHAOS_SORCERER) \
    && defined(HAS_CARD_DARK_MAGICIAN_OF_CHAOS) \
    && defined(HAS_CARD_SACRED_CRANE) \
    && defined(HAS_CARD_TOON_CANNON_SOLDIER) \
    && defined(HAS_CARD_CARD_DESTRUCTION) \
    && defined(HAS_CARD_DIMENSION_FUSION) \
    && defined(HAS_CARD_LIGHTNING_VORTEX) \
    && defined(HAS_CARD_MONSTER_GATE) \
    && defined(HAS_CARD_MONSTER_REINCARNATION) \
    && defined(HAS_CARD_REASONING) \
    && defined(HAS_CARD_SCAPEGOAT) \
    && defined(HAS_CARD_TOON_TABLE_OF_CONTENTS) \
    && defined(HAS_CARD_JAR_OF_GREED) \
    && defined(HAS_CARD_DARK_BALTER_THE_TERRIBLE) \
    && defined(HAS_CARD_DARK_BLADE_THE_DRAGON_KNIGHT) \
    && defined(HAS_CARD_DARK_PALADIN) \
    && defined(HAS_CARD_FIEND_SKULL_DRAGON) \
    && defined(HAS_CARD_KING_DRAGUN) \
    && defined(HAS_CARD_MASTER_OF_OZ) \
    && defined(HAS_CARD_OJAMA_KING) \
    && defined(HAS_CARD_REAPER_ON_THE_NIGHTMARE) \
    && defined(HAS_CARD_RYU_SENSHI) \
    && defined(HAS_CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET) \
    && defined(HAS_CARD_THOUSAND_EYES_RESTRICT) \
    && defined(HAS_CARD_AIRKNIGHT_PARSHATH) \
    && defined(HAS_CARD_SPELL_CANCELLER) \
    && defined(HAS_CARD_TOKEN_THANKSGIVING) \
    && defined(HAS_CARD_RAIGEKI_BREAK) \
    && defined(HAS_CARD_THREATENING_ROAR) \
    && defined(HAS_CARD_TORRENTIAL_TRIBUTE)
#define MOD_GOAT_REASONING_GATE_TURBO 1
static const u16 sGoatReasoningGateTurbo[] = {
    /* Monsters (8) */
    CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING,
    X2(CARD_CHAOS_SORCERER),
    CARD_DARK_MAGICIAN_OF_CHAOS,
    CARD_JINZO,
    X2(CARD_SACRED_CRANE),
    CARD_TOON_CANNON_SOLDIER,
    /* Spells (28) */
    CARD_CARD_DESTRUCTION,
    X3(CARD_DIMENSION_FUSION),
    X3(CARD_GIANT_TRUNADE),
    CARD_GRACEFUL_CHARITY,
    CARD_HEAVY_STORM,
    CARD_LIGHTNING_VORTEX,
    X3(CARD_MONSTER_GATE),
    CARD_MONSTER_REINCARNATION,
    CARD_POT_OF_GREED,
    CARD_PREMATURE_BURIAL,
    X3(CARD_REASONING),
    X3(CARD_SCAPEGOAT),
    CARD_SNATCH_STEAL,
    X3(CARD_TOON_TABLE_OF_CONTENTS),
    X2(CARD_UPSTART_GOBLIN),
    /* Traps (4) */
    CARD_CALL_OF_THE_HAUNTED,
    X3(CARD_JAR_OF_GREED),
    /* Fusion Deck (15) */
    CARD_DARK_BALTER_THE_TERRIBLE,
    CARD_DARK_BLADE_THE_DRAGON_KNIGHT,
    CARD_DARK_PALADIN,
    CARD_DARKFIRE_DRAGON,
    CARD_FIEND_SKULL_DRAGON,
    CARD_FLAME_GHOST,
    CARD_KING_DRAGUN,
    CARD_MASTER_OF_OZ,
    CARD_OJAMA_KING,
    CARD_REAPER_ON_THE_NIGHTMARE,
    CARD_RYU_SENSHI,
    CARD_THE_LAST_WARRIOR_FROM_ANOTHER_PLANET,
    X3(CARD_THOUSAND_EYES_RESTRICT),
};
STATIC_ASSERT(ARRAY_COUNT(sGoatReasoningGateTurbo) == 40 + 15, GoatReasoningGateTurboSize);
static const struct DeckList sGoatReasoningGateTurboList = { sGoatReasoningGateTurbo, ARRAY_COUNT(sGoatReasoningGateTurbo), 0 };
/* paede17-Reasoning Gate Turbo.ydk's Side Deck (15). */
static const u16 sGoatReasoningGateTurboSide[] = {
    CARD_AIRKNIGHT_PARSHATH,
    X2(CARD_SPELL_CANCELLER),
    CARD_SWORDS_OF_REVEALING_LIGHT,
    X2(CARD_TOKEN_THANKSGIVING),
    CARD_MIRROR_FORCE,
    CARD_RAIGEKI_BREAK,
    CARD_BELL_OF_DESTRUCTION,
    X2(CARD_ROYAL_DECREE),
    X3(CARD_THREATENING_ROAR),
    CARD_TORRENTIAL_TRIBUTE,
};
STATIC_ASSERT(ARRAY_COUNT(sGoatReasoningGateTurboSide) == 15, GoatReasoningGateTurboSideSize);
#endif
#if defined(MOD_RED_DECK) && defined(MOD_JOEY_DECK) && defined(MOD_BAKURA_DECK) && defined(MOD_GOAT_CHAOS_CONTROL) \
    && defined(MOD_GOAT_PANDA_BURN) && defined(MOD_GOAT_CHAOS_RETURN) && defined(MOD_GOAT_REASONING_GATE_TURBO)
#define MOD_GOAT_DECKS 1
/* The Goat opponent's deck for this duel, in twentieths: Chaos Turbo 8, Warrior 4, Goat Control 3, Chaos Control 2,
 * Panda Burn, Chaos Return and Reasoning Gate Turbo 1 each. */
static const struct DeckList *GoatDeck(void)
{
    u32 roll = (u32)Random() % 20;

    if (roll < 8)
        return &sRedDeckList;
    if (roll < 12)
        return &sJoeyDeckList;
    if (roll < 15)
        return &sBakuraDeckList;
    if (roll < 17)
        return &sGoatChaosControlList;
    if (roll < 18)
        return &sGoatPandaBurnList;
    if (roll < 19)
        return &sGoatChaosReturnList;
    return &sGoatReasoningGateTurboList;
}
#endif

/* The replacement deck of opponent `opponent` (enum DuelistId), or NULL to keep the original: a new pick (the
 * weighted random ones roll again); GetOpponentDeckOverride keeps it for the match. */
static const struct DeckList *PickOpponentDeckOverride(int opponent)
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
#ifdef MOD_FIBER_DECKS
    case DUELIST_RARE_HUNTER:           /* the Fiber format opponent */
        return FiberDeck();
#endif
#ifdef MOD_YATA_DECKS
    case DUELIST_ARKANA:                /* the Yata format opponent */
        return YataDeck();
#endif
#ifdef MOD_SCIENTIST_DECKS
    case DUELIST_UMBRA_LUMIS:           /* the Scientist format opponent */
        return ScientistDeck();
#endif
#ifdef MOD_VAMPIRE_DECKS
    case DUELIST_MARIK:                 /* the Vampire format opponent */
        return VampireDeck();
#endif
#ifdef MOD_CHAOS_DECKS
    case DUELIST_STRINGS:               /* the Chaos format opponent */
        return ChaosDeck();
#endif
#ifdef MOD_WARRIOR_DECKS
    case DUELIST_KAIBA:                 /* the Warrior format opponent */
        return WarriorDeck();
#endif
#ifdef MOD_GOAT_DECKS
    case DUELIST_ISHIZU:                /* the Goat format opponent */
        return GoatDeck();
#endif
#ifdef MOD_JOEY_PEGASUS_DECKS
    case DUELIST_WEEVIL:                /* the Joey-Pegasus format opponent */
        return JoeyPegasusDeck();
#endif
#ifdef MOD_ANDROID_DECKS
    case DUELIST_MAI:                   /* the Android format opponent */
        return AndroidDeck();
#endif
#ifdef MOD_IMPERIAL_DECKS
    case DUELIST_MAKO:                  /* the Imperial format opponent */
        return ImperialDeck();
#endif
    case DUELIST_ESPA_ROBA:             /* the Treasure format opponent */
        return TreasureDeck();
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
    case DUELIST_GRANDPA:               /* Solomon (make release too): the Deck Recipe the player picked or built */
    case DUELIST_PEGASUS:               /* Debug, the same */
        return gModOpponentRecipe.valid == MOD_OPPONENT_RECIPE_VALID ? &gModOpponentRecipe.list : NULL;
#if defined(MOD_TEST_DECK) && !MOD_RELEASE
    case DUELIST_DUEL_COMPUTER:         /* "Test Duel": test.ydk */
        return &sTestDeckList;
#endif
    }
    return NULL;
}

/* The first Deck Recipes (src/mod_recipes.c), in order, named as their .ydk files. A deck this build leaves out
 * (its cards are not all listed) is skipped. */
struct RecipePreset {
    const char *name;
    const struct DeckList *list;   /* the Main Deck and the Fusion Deck (its Fusion Monsters) */
    const u16 *side;               /* the Side Deck (card numbers), NULL for none */
    u32 sideCount;
};

static const char sNameSoulControl[] = "El Duelista DEX-Soul Control";
static const char sNameVanillaBeat[] = "ellietincan-Vanilla Beat";
static const char sNameStallMill[] = "DK-ZERO-Stall Mill";
static const char sNameMaskedControl[] = "Jazz-Masked Control";
static const char sNameMukaMukaTurbo[] = "Gyomberman-Muka Muka Turbo";
static const char sNameTomatoControl[] = "dump truck-Tomato Control";
static const char sNameRecruiterControl[] = "SolEmerald69-Recruiter Control";
static const char sNameGermBurn[] = "bsocks-Germ Burn";
static const char sNameImperialTomato[] = "Aretos-Tomato Control";
static const char sNameRelinquished[] = "bsocks-Relinquished";
static const char sNameAndroidTomato[] = "Jazz-Tomato Control";
static const char sNameAndroidVanilla[] = "paoloNS-Vanilla Beat";
static const char sNameJoeyPegasusTomato[] = "AJTBLS-Tomato Control";
static const char sNameJoeyPegasusVanilla[] = "ellietincan-Vanilla Beat (JP)";
static const char sNameFiberTomato[] = "Aretos-Tomato Control (Fiber)";
static const char sNameFiberRecruiter[] = "GBO-Recruiter Control";
static const char sNameYataTomato[] = "Shigeki Kitamura-Tomato Control";
static const char sNameYataWarriorTomato[] = "ellietincan-Warrior Tomato Control";
static const char sNameYataGearfried[] = "AlephYa2-Gearfried";
static const char sNameScientistTomato[] = "Aretos-Tomato Control (Scientist)";
static const char sNameScientistFtk[] = "DK-ZERO-Scientist FTK";
static const char sNameVampireWarriorControl[] = "Woawa-Warrior Control";
static const char sNameVampireScientistFtk[] = "ellietincan-Scientist FTK";
static const char sNameVampireWarriorTomato[] = "AJTBLS-Warrior Tomato Control";
static const char sNameChaosControl[] = "ellietincan-Chaos Control";
static const char sNameChaosWarrior[] = "indoors-Chaos Warrior";
static const char sNameWarriorGoatControl[] = "Gyomberman-Goat Control";
static const char sNameWarriorWarrior[] = "ellietincan-Warrior";
static const char sNameWarriorWarriorControl[] = "Patrick Smith-Warrior Control";
static const char sNameWarriorChaosReturn[] = "paoloNS-Chaos Return";
static const char sNameChaosTurbo[] = "LR-Chaos Turbo";
static const char sNameWarrior[] = "sdlkilla-Warrior";
static const char sNameGoatControl[] = "geistD-Goat Control";
static const char sNameGoatChaosControl[] = "Nova-Chaos Control";      /* N\u2661va: the game's font has no heart */
static const char sNameGoatPandaBurn[] = "wolfe_KMH-Panda Burn";
static const char sNameGoatChaosReturn[] = "Dingosig-Chaos Return";
static const char sNameGoatReasoningGateTurbo[] = "paede17-Reasoning Gate Turbo";

static const struct RecipePreset sRecipePresets[] = {
#ifdef MOD_TRISTAN_SOUL_CONTROL
    { sNameSoulControl, &sTristanSoulControlList, sTristanSoulControlSide, ARRAY_COUNT(sTristanSoulControlSide) },
#endif
#ifdef MOD_TRISTAN_VANILLA_BEAT
    { sNameVanillaBeat, &sTristanVanillaBeatList, sTristanVanillaBeatSide, ARRAY_COUNT(sTristanVanillaBeatSide) },
#endif
#ifdef MOD_TRISTAN_STALL_MILL
    { sNameStallMill, &sTristanStallMillList, sTristanStallMillSide, ARRAY_COUNT(sTristanStallMillSide) },
#endif
#ifdef MOD_CRITTER_MASKED_CONTROL
    { sNameMaskedControl, &sCritterMaskedControlList, sCritterMaskedControlSide, ARRAY_COUNT(sCritterMaskedControlSide) },
#endif
    { sNameMukaMukaTurbo, &sCritterMukaMukaTurboList, sCritterMukaMukaTurboSide, ARRAY_COUNT(sCritterMukaMukaTurboSide) },
    { sNameTomatoControl, &sTreasureTomatoControlList, sTreasureTomatoControlSide, ARRAY_COUNT(sTreasureTomatoControlSide) },
#ifdef MOD_TREASURE_RECRUITER_CONTROL
    { sNameRecruiterControl, &sTreasureRecruiterControlList, sTreasureRecruiterControlSide, ARRAY_COUNT(sTreasureRecruiterControlSide) },
#endif
    { sNameGermBurn, &sTreasureGermBurnList, sTreasureGermBurnSide, ARRAY_COUNT(sTreasureGermBurnSide) },
#ifdef MOD_IMPERIAL_DECKS
    { sNameImperialTomato, &sImperialTomatoControlList, sImperialTomatoControlSide, ARRAY_COUNT(sImperialTomatoControlSide) },
    { sNameRelinquished, &sImperialRelinquishedList, sImperialRelinquishedSide, ARRAY_COUNT(sImperialRelinquishedSide) },
#endif
#ifdef MOD_ANDROID_TOMATO_CONTROL
    { sNameAndroidTomato, &sAndroidTomatoControlList, sAndroidTomatoControlSide, ARRAY_COUNT(sAndroidTomatoControlSide) },
#endif
#ifdef MOD_ANDROID_VANILLA_BEAT
    { sNameAndroidVanilla, &sAndroidVanillaBeatList, sAndroidVanillaBeatSide, ARRAY_COUNT(sAndroidVanillaBeatSide) },
#endif
#ifdef MOD_JOEY_PEGASUS_TOMATO_CONTROL
    { sNameJoeyPegasusTomato, &sJoeyPegasusTomatoControlList, sJoeyPegasusTomatoControlSide, ARRAY_COUNT(sJoeyPegasusTomatoControlSide) },
#endif
#ifdef MOD_JOEY_PEGASUS_VANILLA_BEAT
    { sNameJoeyPegasusVanilla, &sJoeyPegasusVanillaBeatList, sJoeyPegasusVanillaBeatSide, ARRAY_COUNT(sJoeyPegasusVanillaBeatSide) },
#endif
#ifdef MOD_FIBER_TOMATO_CONTROL
    { sNameFiberTomato, &sFiberTomatoControlList, sFiberTomatoControlSide, ARRAY_COUNT(sFiberTomatoControlSide) },
#endif
#ifdef MOD_FIBER_RECRUITER_CONTROL
    { sNameFiberRecruiter, &sFiberRecruiterControlList, sFiberRecruiterControlSide, ARRAY_COUNT(sFiberRecruiterControlSide) },
#endif
#ifdef MOD_YATA_TOMATO_CONTROL
    { sNameYataTomato, &sYataTomatoControlList, sYataTomatoControlSide, ARRAY_COUNT(sYataTomatoControlSide) },
#endif
#ifdef MOD_YATA_WARRIOR_TOMATO_CONTROL
    { sNameYataWarriorTomato, &sYataWarriorTomatoControlList, sYataWarriorTomatoControlSide, ARRAY_COUNT(sYataWarriorTomatoControlSide) },
#endif
#ifdef MOD_YATA_GEARFRIED
    { sNameYataGearfried, &sYataGearfriedList, sYataGearfriedSide, ARRAY_COUNT(sYataGearfriedSide) },
#endif
#ifdef MOD_SCIENTIST_TOMATO_CONTROL
    { sNameScientistTomato, &sScientistTomatoControlList, sScientistTomatoControlSide, ARRAY_COUNT(sScientistTomatoControlSide) },
#endif
#ifdef MOD_SCIENTIST_FTK
    { sNameScientistFtk, &sScientistFtkList, sScientistFtkSide, ARRAY_COUNT(sScientistFtkSide) },
#endif
#ifdef MOD_VAMPIRE_WARRIOR_CONTROL
    { sNameVampireWarriorControl, &sVampireWarriorControlList, sVampireWarriorControlSide,
      ARRAY_COUNT(sVampireWarriorControlSide) },
#endif
#ifdef MOD_VAMPIRE_SCIENTIST_FTK
    { sNameVampireScientistFtk, &sVampireScientistFtkList, sVampireScientistFtkSide,
      ARRAY_COUNT(sVampireScientistFtkSide) },
#endif
#ifdef MOD_VAMPIRE_WARRIOR_TOMATO_CONTROL
    { sNameVampireWarriorTomato, &sVampireWarriorTomatoControlList, sVampireWarriorTomatoControlSide,
      ARRAY_COUNT(sVampireWarriorTomatoControlSide) },
#endif
#ifdef MOD_CHAOS_CONTROL
    { sNameChaosControl, &sChaosControlList, sChaosControlSide, ARRAY_COUNT(sChaosControlSide) },
#endif
#ifdef MOD_CHAOS_WARRIOR
    { sNameChaosWarrior, &sChaosWarriorList, sChaosWarriorSide, ARRAY_COUNT(sChaosWarriorSide) },
#endif
#ifdef MOD_WARRIOR_GOAT_CONTROL
    { sNameWarriorGoatControl, &sWarriorGoatControlList, sWarriorGoatControlSide,
      ARRAY_COUNT(sWarriorGoatControlSide) },
#endif
#ifdef MOD_WARRIOR_WARRIOR
    { sNameWarriorWarrior, &sWarriorWarriorList, sWarriorWarriorSide, ARRAY_COUNT(sWarriorWarriorSide) },
#endif
#ifdef MOD_WARRIOR_WARRIOR_CONTROL
    { sNameWarriorWarriorControl, &sWarriorWarriorControlList, sWarriorWarriorControlSide,
      ARRAY_COUNT(sWarriorWarriorControlSide) },
#endif
#ifdef MOD_WARRIOR_CHAOS_RETURN
    { sNameWarriorChaosReturn, &sWarriorChaosReturnList, sWarriorChaosReturnSide,
      ARRAY_COUNT(sWarriorChaosReturnSide) },
#endif
#ifdef MOD_RED_DECK
    { sNameChaosTurbo, &sRedDeckList, sRedDeckSide, ARRAY_COUNT(sRedDeckSide) },
#endif
#ifdef MOD_JOEY_DECK
    { sNameWarrior, &sJoeyDeckList, sJoeyDeckSide, ARRAY_COUNT(sJoeyDeckSide) },
#endif
#ifdef MOD_BAKURA_DECK
    { sNameGoatControl, &sBakuraDeckList, sBakuraDeckSide, ARRAY_COUNT(sBakuraDeckSide) },
#endif
#ifdef MOD_GOAT_CHAOS_CONTROL
    { sNameGoatChaosControl, &sGoatChaosControlList, sGoatChaosControlSide, ARRAY_COUNT(sGoatChaosControlSide) },
#endif
#ifdef MOD_GOAT_PANDA_BURN
    { sNameGoatPandaBurn, &sGoatPandaBurnList, sGoatPandaBurnSide, ARRAY_COUNT(sGoatPandaBurnSide) },
#endif
#ifdef MOD_GOAT_CHAOS_RETURN
    { sNameGoatChaosReturn, &sGoatChaosReturnList, sGoatChaosReturnSide, ARRAY_COUNT(sGoatChaosReturnSide) },
#endif
#ifdef MOD_GOAT_REASONING_GATE_TURBO
    { sNameGoatReasoningGateTurbo, &sGoatReasoningGateTurboList, sGoatReasoningGateTurboSide,
      ARRAY_COUNT(sGoatReasoningGateTurboSide) },
#endif
    { NULL, NULL, NULL, 0 },
};

/* Preset recipe n: its name, its cards (card numbers) and count, and its Side Deck's; 0 past the last one. */
int GetRecipePreset(int n, const char **name, const u16 **cards, u32 *count, const u16 **side, u32 *sideCount)
{
    if (n < 0 || n >= (int)ARRAY_COUNT(sRecipePresets) - 1)
        return 0;
    *name = sRecipePresets[n].name;
    *cards = sRecipePresets[n].list->cards;
    *count = sRecipePresets[n].list->count;
    *side = sRecipePresets[n].side;
    *sideCount = sRecipePresets[n].sideCount;
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
 * the three Yugi-Kaiba decks (Tristan's): Red is El Duelista DEX-Soul Control, Green ellietincan-Vanilla Beat, Black
 * DK-ZERO-Stall Mill. A Fusion Monster goes to the saved Fusion Deck, and the .ydk's Side Deck to the saved one. Returns 1 then, 0 to let BuildStarterDeck build the original. */
int BuildModStarterDeck(int choice)
{
    const struct DeckList *list = NULL;
    const u16 *side = NULL;
    u32 sideCount = 0, i;

    switch (choice) {
#ifdef MOD_TRISTAN_SOUL_CONTROL
    case STARTER_DECK_RED:
        list = &sTristanSoulControlList;
        side = sTristanSoulControlSide;
        sideCount = ARRAY_COUNT(sTristanSoulControlSide);
        break;
#endif
#ifdef MOD_TRISTAN_VANILLA_BEAT
    case STARTER_DECK_GREEN:
        list = &sTristanVanillaBeatList;
        side = sTristanVanillaBeatSide;
        sideCount = ARRAY_COUNT(sTristanVanillaBeatSide);
        break;
#endif
#ifdef MOD_TRISTAN_STALL_MILL
    case STARTER_DECK_BLACK:
        list = &sTristanStallMillList;
        side = sTristanStallMillSide;
        sideCount = ARRAY_COUNT(sTristanStallMillSide);
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
    for (i = 0; i < sideCount; i++)
        AddCardToSavedSideDeck(gCardNumberToId[side[i] & CARD_ID_MASK]);
    return 1;
}

/* ---- Best-of-3 matches (Campaign) -----------------------------------------------------------------------------------
 * Every Campaign duel is a best-of-3 match (Campaign_ShowDuelResult, campaign_steps.c). gModMatch (EWRAM 0x0202D400,
 * symbols.ld) keeps what the match needs between its duels:
 *   the CPU's deck    GetOpponentDeckOverride picks it on the match's first duel (a weighted random pick for the
 *                     opponents with several decks) and returns the same one for the later duels
 *   the player's deck the saved Deck and Side Deck when the match starts (ModMatchSavePlayerDeck, from
 *                     Campaign_SetupDuel); the side-deck swap between the duels changes them, and
 *                     ModMatchRestorePlayerDeck puts them back when the match is decided
 * `valid` holds MOD_MATCH_VALID while the fields are set (the RAM is not cleared at power-on). */
#define MOD_MATCH_VALID 0xA5C3

struct ModMatch {
    const struct DeckList *opponentDeck;    /* the CPU's deck for this match (NULL: the original deck) */
    u16 opponent;                           /* whose deck it is */
    u16 deckValid;                          /* MOD_MATCH_VALID: opponentDeck is set */
    u16 playerValid;                        /* MOD_MATCH_VALID: the player's decks below are saved */
    u16 deckSize;
    u16 sideDeckSize;
    u16 deck[DECK_MAX_CARDS];
    u16 sideDeck[SIDE_DECK_MAX_CARDS];
};
extern struct ModMatch gModMatch;

/* 1 on the later duels of a match against `opponent`. */
static int IsLaterMatchDuel(int opponent)
{
    return gMain.duelFormat == DUEL_FORMAT_MATCH && gMain.matchDuelCount > 0 && gModMatch.deckValid == MOD_MATCH_VALID
           && gModMatch.opponent == opponent;
}

/* The replacement deck of opponent `opponent` (enum DuelistId), or NULL to keep the original: the same one for
 * every duel of a match (see above). */
const struct DeckList *GetOpponentDeckOverride(int opponent)
{
    if (IsLaterMatchDuel(opponent))
        return gModMatch.opponentDeck;
    gModMatch.opponentDeck = PickOpponentDeckOverride(opponent);
    gModMatch.opponent = opponent;
    gModMatch.deckValid = MOD_MATCH_VALID;
    return gModMatch.opponentDeck;
}

/* Add `delta` copies of each card of `cards` to the saved Deck (side 0) or Side Deck (side 1) counters of the
 * trunk. */
static void AdjustSavedCopies(const u16 *cards, int count, int side, int delta)
{
    int i;

    for (i = 0; i < count; i++) {
        u16 id = cards[i];
        int copies = side ? GetTrunkSideCopies(id) : GetTrunkDeckCopies(id);

        copies += delta;
        if (copies < 0)
            copies = 0;
        if (side)
            SetTrunkSideCopies(id, copies);
        else
            SetTrunkDeckCopies(id, copies);
    }
}

/* Campaign_SetupDuel, on a match's first duel: remember the saved Deck and Side Deck. */
void ModMatchSavePlayerDeck(void)
{
    MemCopy16(gModMatch.deck, gSaveData.deck, sizeof(gModMatch.deck));
    MemCopy16(gModMatch.sideDeck, gSaveData.sideDeck, sizeof(gModMatch.sideDeck));
    gModMatch.deckSize = gSaveData.deckSize;
    gModMatch.sideDeckSize = gSaveData.sideDeckSize;
    gModMatch.playerValid = MOD_MATCH_VALID;
}

/* Campaign_ShowDuelResult, once the match is decided: undo the side-deck swaps (the lists and the trunk's
 * per-card Deck / Side Deck counters). */
void ModMatchRestorePlayerDeck(void)
{
    if (ModRecipeBuilding()) {      /* a match with Debug: the player's own collection back, then the save */
        gModMatch.playerValid = 0;
        ModRecipeBuildEnd();
        SaveGame();
        return;
    }
    if (gModMatch.playerValid != MOD_MATCH_VALID)
        return;
    gModMatch.playerValid = 0;
    AdjustSavedCopies(gSaveData.deck, gSaveData.deckSize, 0, -1);
    AdjustSavedCopies(gSaveData.sideDeck, gSaveData.sideDeckSize, 1, -1);
    MemCopy16(gSaveData.deck, gModMatch.deck, sizeof(gModMatch.deck));
    MemCopy16(gSaveData.sideDeck, gModMatch.sideDeck, sizeof(gModMatch.sideDeck));
    gSaveData.deckSize = gModMatch.deckSize;
    gSaveData.sideDeckSize = gModMatch.sideDeckSize;
    AdjustSavedCopies(gSaveData.deck, gSaveData.deckSize, 0, 1);
    AdjustSavedCopies(gSaveData.sideDeck, gSaveData.sideDeckSize, 1, 1);
}
