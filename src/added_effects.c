/*
 * added_effects: the effects of the cards added to the game (config/new_cards.txt), which the original game does
 * not have. Not part of the original ROM.
 *
 * gAddedCardEffects continues gCardEffects: FindCardEffect looks a card number up here when the original table
 * has no row for it and returns CARD_EFFECT_ROWS_ORIGINAL + the index here; GetCardEffectRow maps such a row
 * back (include/effect.h). The rows reference their handlers by symbol, so they follow the code when it moves.
 * Summon triggers and the card menu's ACTIVATE bit are by card number in the duel code: ExecuteSummonAction and
 * ExecuteSummonActionAskPosition (summon_checks.c) for "when Normal Summoned", CanActivateMonsterEffect
 * (card_command_menu.c) for a monster's activated effect.
 * The code of a card is guarded by HAS_CARD_<NAME> (build/include/card_count.h, tools/newcards.py), so it drops
 * out when the card is taken off the list.
 */

#include "global.h"
#include "card_data.h"
#include "constants/cards.h"
#include "constants/duel.h"
#include "chain.h"
#include "duel.h"
#include "duel_actions.h"
#include "effect.h"
#include "effect_handlers.h"
#include "constants/card_stats.h"
#include "constants/duel_cmds.h"
#include "duel_cmd.h"
#include "text_box.h"
#include "duel_prompt.h"
#include "sound.h"
#include "constants/sound.h"
#include "card_list_view.h"
#include "summon.h"
#include "card_menu.h"
#include "gba.h"
#include "text.h"
#include "sprite.h"
#include "util.h"
#include "ai.h"
#include "battle.h"
#include "main.h"                   /* gMain.newKeys */
#include "duel_screen.h"            /* DuelCursor_PickTarget, gDuelScreen */
#include "duel_flow.h"              /* gDuelCtrl.isLinkDuel */

/* ---- Spell Counters ----------------------------------------------------------------------------------------
 * A Spell Counter on a monster is a ZONE_LINK_ATK_300_PER_VALUE link from the monster's own card ID: the link's
 * count (high byte) is the number of counters, and the stat code (card_stats.c) already gives the monster 300
 * ATK per unit. The link goes with the card when it leaves the field. */

/* The packed chain entry of a trigger (struct ChainEntry, chain.h), as in duel_field_moves.c. */
#define TRIGGER_ZONE(zone) ((zone) << 16)
#define TRIGGER_KIND(kind) ((kind) << 21)
#define TRIGGER_EVENT(event) ((event) << 25)
#define TRIGGER_PLAYER(player) ((player) << 31)

/* What the added cards remember during a duel: gModDuel (EWRAM 0x0202FF04, symbols.ld, free RAM below gScratchBuffer).
 * DuelPhase_Init clears it with ModDuelInit. A `static` variable here would go to .bss, which the linker puts at
 * 0x02000000, over gTextCanvas: keep such state in this block. */
struct ModDuelState {
    u16 battleKills;            /* Mystic Swordsman LV2: bit player * 5 + zone, destroyed a monster by battle this turn */
    u16 chaosSummoned;          /* Dark Magician of Chaos: bit player * 5 + zone, Normal or Special Summoned this turn */
    u8 tsukuyomiEndReturn[2];   /* Tsukuyomi: per player, bit zone, return it to the hand in the End Phase */
    u8 flipEffectsLocked;       /* flip effects cannot be activated this turn */
    u16 aiStandbyTurn;          /* AiStandbyActivations: the turn (gDuel.turnCount) it last ran */
    u16 dimensionReturn;        /* Return from the Different Dimension: bit player * 5 + zone, banish in the End Phase */
    u16 dimensionReturnIds[10]; /* ... and the card ID summoned into that zone */
    u8 aiThunderDragonSearched; /* the CPU's Thunder Dragon added a copy; it is discarded even with none left to find */
    u16 aiTributeFor;           /* AiPickTributeFor: the card number the CPU's tributes may not copy, 0xFFFF none */
    u8 sandMothPending;         /* Sand Moth: bit player, its effect's Special Summon is queued (swap its stats) */
    u8 mirageDrawn[2];          /* Mirage of Nightmare: per player, the cards to discard in their next Standby Phase */
    u8 standbyPart;             /* ModStandbyStep: Treeborn Frog, Mirage's discard, Mirage's draw */
    u16 roarTurn[2];            /* Threatening Roar: per player, gDuel.turnCount + 1 of the turn they cannot attack */
    u16 scientistReturn;        /* Magical Scientist: bit player * 5 + zone, return to the Fusion Deck in the End Phase */
    u16 scientistReturnIds[10]; /* ... and the card ID summoned into that zone */
    u16 aiScientistTurn;        /* AiUseMagicalScientist: gDuel.turnCount + 1 of its last summon for battle */
    u16 asuraTurn;              /* Asura Priest: gDuel.turnCount + 1 of the Battle Phase the two below are for */
    u8 asuraTargets[5];         /* ... per zone of the attacking player: bit t, it attacked the opponent's zone t ... */
    u16 asuraSerials[5][5];     /* ... where the monster had this DuelZone.serial (a new monster there is not it) */
    u16 noDamageTurn[2];        /* Hallowed Life Barrier: per player, gDuel.turnCount + 1 of the turn of no damage */
    u16 noEffectDamageTurn[2];  /* Pikeru's Circle of Enchantment: the same, for effect damage only */
    u8 rouletteDie1;            /* Roulette Barrel: first die roll this activation */
    u8 rouletteDie2;            /* Roulette Barrel: second die roll this activation */
    u16 mystikWokAtk;           /* Mystik Wok: ATK of the monster Tributed this activation */
    u16 mystikWokDef;           /* Mystik Wok: DEF of the monster Tributed this activation */
    u8 ddcReturnPending[2];     /* Different Dimension Capsule: return banished card to hand next Standby */
    u16 ddcReturnCardId[2];     /* ... card ID to find in the banished pile (face-down) */
    u8 exarionAskStep;          /* Exarion Universe: enum ExarionUniverseStep for this attack's prompt */
    u8 exarionPierce;           /* ... 1 if this attack inflicts piercing from its effect */
    u8 exarionPenaltyStacks[10]; /* ... per zone (player * 5 + zone): -400 ATK per stack until that player's turn ends */
};
extern struct ModDuelState gModDuel;
/* gModDuel (0x0202FF04, symbols.ld) must end before gScratchBuffer (0x02030000). */
STATIC_ASSERT(sizeof(struct ModDuelState) <= 0x02030000 - 0x0202FF04, ModDuelStateFits);

static inline struct DuelZone *ZoneOf(int player, int zone)
{
    return &gDuelPlayers[player & 1].zones[zone];
}

/* Spell Counters on the card in (player, zone). */
static inline int CountSpellCounters(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);
    int i;

    for (i = 0; i < z->numLinks; i++) {
        if (((u8)z->linkKinds[i] == ZONE_LINK_ATK_300_PER_VALUE || (u8)z->linkKinds[i] == ZONE_LINK_SPELL_COUNTER)
            && (z->links[i] & CARD_ID_MASK) == id)
            return z->linkKinds[i] >> 8;
    }
    return 0;
}

#ifdef HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR
/* ---- Breaker the Magical Warrior ---------------------------------------------------------------------------
 * "When this card is Normal Summoned: Place 1 Spell Counter on it (max. 1). Gains 300 ATK for each Spell Counter
 * on it. You can remove 1 Spell Counter from this card; target 1 Spell/Trap on the field; destroy that target."
 *
 * One row serves both effects; the link's event tells them apart:
 *   RESPONSE_SUMMONED   the trigger that ExecuteSummonAction queues when Breaker is Normal Summoned (loc0 is its
 *                       own zone): place the counter
 *   RESPONSE_NONE       the activated effect from the card menu (Main Phase, CanActivateMonsterEffect): needs a
 *                       counter; the cost removes it, then it destroys 1 Spell/Trap like Mystical Space Typhoon
 *                       (EffectAnySpellTrapCheck, EffectSpellTrapTargetChainB, EffectDestroyTargetResolve) */

static inline int IsBreakerSummonTrigger(struct ChainEntry *link)
{
    return link->event == RESPONSE_SUMMONED && link->loc0 == DUEL_LOC(link->player, link->zone);
}

int EffectBreakerTheMagicalWarriorPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelZone *z;

    if (fromHand)
        return 0;
    z = ZoneOf(card->player, card->zone);
    if (!z->isFaceUp || DUEL_CARD_ID(&z->card) != (card->card & CARD_ID_MASK))
        return 0;
    if (card->event == RESPONSE_SUMMONED)
        return IsBreakerSummonTrigger(card) && CountSpellCounters(card->player, card->zone) == 0;
    if (card->event != RESPONSE_NONE)
        return 0;
    return CountSpellCounters(card->player, card->zone) != 0;
}

/* The summon trigger needs no target; the activated effect needs a Spell/Trap card on the field. */
u16 EffectBreakerTheMagicalWarriorCheck(struct ChainEntry *card, u16 pos)
{
    if (card->event == RESPONSE_SUMMONED)
        return 1;
    return EffectAnySpellTrapCheck(card, pos);
}

/* Cost of the activated effect: remove the Spell Counter. */
int EffectBreakerTheMagicalWarriorChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (link->event != RESPONSE_SUMMONED)
        QueueRemoveZoneLink(link->player, link->card, DUEL_LOC(link->player, link->zone), ZONE_LINK_ATK_300_PER_VALUE);
    return 1;
}

int EffectBreakerTheMagicalWarriorChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (link->event == RESPONSE_SUMMONED) {
        link->numTargets = 0;
        return 1;
    }
    return EffectSpellTrapTargetChainB(link);
}

int EffectBreakerTheMagicalWarriorResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    struct DuelZone *z;

    if (link->event != RESPONSE_SUMMONED)
        return EffectDestroyTargetResolve(link);
    /* Place the counter if Breaker is still face up in its zone and has none (max. 1). */
    z = ZoneOf(link->player, link->zone);
    if (!link->negated && z->isFaceUp && DUEL_CARD_ID(&z->card) == (link->card & CARD_ID_MASK)
        && CountSpellCounters(link->player, link->zone) == 0)
        QueueAddZoneLink(link->player, link->card, DUEL_LOC(link->player, link->zone),
                         ZONE_LINK_KIND(ZONE_LINK_ATK_300_PER_VALUE, 1));
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR */

#ifdef HAS_LIGHT_DARK_BANISH_SUMMON
/* ---- Shared by Black Luster Soldier - Envoy of the Beginning and Chaos Sorcerer --------------------------------
 * Their summon (banish 1 LIGHT and 1 DARK monster from the Graveyard) and their "banish the target; this card cannot
 * attack this turn" effect, once per turn through effectUnused. */

/* 1 if the Graveyard of `player` has a monster of `attribute`. */
static int HasGraveyardMonsterOfAttribute(int player, int attribute)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    for (i = 0; i < p->graveCount; i++) {
        u32 stats = gCardStats[DUEL_CARD_ID(&p->graveyard[i]) & CARD_ID_MASK];
        if (CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && (int)CARD_STATS_ATTR(stats) == attribute)
            return 1;
    }
    return 0;
}

/* The summon cost: a free monster zone and a LIGHT and a DARK monster in the Graveyard (CanSummonFromHand). */
int CanPayLightDarkBanishCost(int player)
{
    return CountFreeMonsterZones(player) > 0 && HasGraveyardMonsterOfAttribute(player, ATTRIBUTE_LIGHT)
           && HasGraveyardMonsterOfAttribute(player, ATTRIBUTE_DARK);
}

/* Gigantes' cost: a free monster zone and an EARTH monster in the Graveyard (CanSummonFromHand). */
int CanPayEarthBanishCost(int player)
{
    return CountFreeMonsterZones(player) > 0 && HasGraveyardMonsterOfAttribute(player, ATTRIBUTE_EARTH);
}

/* The attribute the summon steps are banishing now: LIGHT first (summonSeq 0), then DARK; EARTH for Gigantes. */
int LightDarkBanishAttribute(void)
{
#ifdef HAS_CARD_GIGANTES
    if (gCardIdToNumber[gDuel.cardMenuCard & CARD_ID_MASK] == CARD_GIGANTES)
        return ATTRIBUTE_EARTH;
#endif
    return gDuel.cardMenu.summonSeq == 0 ? ATTRIBUTE_LIGHT : ATTRIBUTE_DARK;
}

/* How many monsters the summoning card banishes: 1 for Gigantes, else 2 (a LIGHT and a DARK). */
int BanishSummonCount(void)
{
#ifdef HAS_CARD_GIGANTES
    if (gCardIdToNumber[gDuel.cardMenuCard & CARD_ID_MASK] == CARD_GIGANTES)
        return 1;
#endif
    return 2;
}

/* Banish the target (targets[0]); the activating monster cannot attack this turn and its effect is used. */
int EffectBanishTargetNoAttackResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    struct DuelZone *z = ZoneOf(link->player, link->zone);
    u16 cmd;

    if (!link->negated && link->numTargets == 1) {
        int targetPlayer = (u8)link->targets[0];
        int targetZone = link->targets[0] >> 8;
        if (DUEL_CARD_ID(&ZoneOf(targetPlayer, targetZone)->card) != 0)
            BanishFieldCard(targetPlayer, targetZone, 0);
    }
    if (z->isFaceUp && DUEL_CARD_ID(&z->card) == (link->card & CARD_ID_MASK)) {
        cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_CANNOT_ATTACK : DUEL_CMD_SET_CANNOT_ATTACK;
        DuelCmd_Push(cmd, link->zone, 1, 0);
        cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_EFFECT_UNUSED : DUEL_CMD_SET_EFFECT_UNUSED;
        DuelCmd_Push(cmd, link->zone, 0, 0);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_LIGHT_DARK_BANISH_SUMMON */

#ifdef HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING
/* ---- Black Luster Soldier - Envoy of the Beginning ----------------------------------------------------------
 * "Cannot be Normal Summoned/Set. Must first be Special Summoned (from your hand) by banishing 1 LIGHT and 1 DARK
 * monster from your GY. Once per turn, you can activate 1 of these effects.
 *  - Target 1 monster on the field; banish it. This card cannot attack the turn this effect is activated.
 *  - During the Damage Step, if this attacking card destroys an opponent's monster by battle: it can make a
 *    second attack in a row."
 *
 * Summon: IsSpecialSummonOnly (card_detail.c) makes the hand menu offer only the Special Summon commands;
 * CanSummonFromHand (summon_checks.c) asks CanPayLightDarkBanishCost; the summon steps (effect_hooks.c,
 * SUMMON_STEP_BLS_*) banish one LIGHT, then one DARK monster from the Graveyard through the card-list viewer, whose
 * list CollectEffectTargets (effect_target_collect.c) builds with LightDarkBanishAttribute (shared with Chaos
 * Sorcerer).
 * Effects: one row, told apart by the link's event; both use the zone's effectUnused flag (once per turn, set again
 * by DuelCmd_TurnStart):
 *   RESPONSE_NONE              the menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect): target a monster
 *                              (Man-Eater Bug's EffectTargetableMonsterCheck / ChainB), banish it, no attack
 *   RESPONSE_BATTLE_DESTROYED  queued by BattleStage_DestroyMonsters (battle_phase2.c) when it attacked, destroyed
 *                              the target and survived: on Yes, its attack mark is cleared so it can attack again */

static const u8 sStrBlsAttackAgain[] = "Use Black Luster Soldier's effect to attack again?";

static inline int IsBlsBattleTrigger(struct ChainEntry *link)
{
    return link->event == RESPONSE_BATTLE_DESTROYED && link->loc0 == DUEL_LOC(link->player, link->zone);
}

int EffectBlackLusterSoldierPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelZone *z;

    if (fromHand)
        return 0;
    z = ZoneOf(card->player, card->zone);
    if (!z->isFaceUp || DUEL_CARD_ID(&z->card) != (card->card & CARD_ID_MASK) || !z->effectUnused)
        return 0;
    if (card->event == RESPONSE_BATTLE_DESTROYED)
        return IsBlsBattleTrigger(card);
    return card->event == RESPONSE_NONE;
}

/* The banish effect targets any monster on the field; the second attack needs no target. */
u16 EffectBlackLusterSoldierCheck(struct ChainEntry *card, u16 pos)
{
    if (card->event == RESPONSE_BATTLE_DESTROYED)
        return 1;
    return EffectTargetableMonsterCheck(card, pos);
}

int EffectBlackLusterSoldierChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (link->event == RESPONSE_BATTLE_DESTROYED) {
        link->numTargets = 0;
        return 1;
    }
    return EffectTargetableMonsterChainB(link, (int)prevLink);
}

int EffectBlackLusterSoldierResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    struct DuelZone *z = ZoneOf(link->player, link->zone);
    int here = z->isFaceUp && DUEL_CARD_ID(&z->card) == (link->card & CARD_ID_MASK);
    u16 cmd;

    if (link->event != RESPONSE_BATTLE_DESTROYED)
        return EffectBanishTargetNoAttackResolve(link, chainedTo);
    /* The second attack: the human is asked first (the CPU always takes it). */
    if (link->negated || !here)
        return EFFECT_STEP_DONE;
    if (!link->player && gChain.effectStep == EFFECT_STEP_START) {
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, sStrBlsAttackAgain);
        TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
        return EFFECT_STEP_2;
    }
    if (link->player || gTextBox.result) {
        gDuelPlayers[link->player & 1].attackedMask &= ~(1 << link->zone);
        cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_EFFECT_UNUSED : DUEL_CMD_SET_EFFECT_UNUSED;
        DuelCmd_Push(cmd, link->zone, 0, 0);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING */

#ifdef HAS_CARD_CHAOS_SORCERER
/* ---- Chaos Sorcerer --------------------------------------------------------------------------------------------
 * "Cannot be Normal Summoned/Set. Must first be Special Summoned (from your hand) by banishing 1 LIGHT and 1 DARK
 * monster from your GY. Once per turn: You can target 1 face-up monster on the field; banish it. This card cannot
 * attack the turn you activate this effect."
 * The summon is Black Luster Soldier - Envoy of the Beginning's (the same hooks, HAS_LIGHT_DARK_BANISH_SUMMON). The
 * effect is the card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), once per turn (effectUnused, set
 * again by DuelCmd_TurnStart); the target is a face-up monster (EffectTargetableFaceUpMonsterCheck / ChainB). */
int EffectChaosSorcererPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelZone *z;

    if (fromHand || card->event != RESPONSE_NONE)
        return 0;
    z = ZoneOf(card->player, card->zone);
    return z->isFaceUp && DUEL_CARD_ID(&z->card) == (card->card & CARD_ID_MASK) && z->effectUnused;
}
#endif /* HAS_CARD_CHAOS_SORCERER */

#ifdef HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END
/* ---- Chaos Emperor Dragon - Envoy of the End -------------------------------------------------------------------
 * Its first-edition text, as the user asked (the fetched one is the later errata, with "Once per turn", damage only for
 * the opponent's cards, and "You cannot activate other cards or effects during the turn you activate this card's
 * effect"): "This card can only be Special Summoned by removing 1 LIGHT and 1 DARK monster in your Graveyard from
 * play. Pay 1000 of your Life Points to send all cards in both players' hands and on the field to the Graveyard.
 * Inflict 300 points of damage to your opponent's Life Points for each card that is sent to the Graveyard by this
 * effect."
 * The summon is Black Luster Soldier - Envoy of the Beginning's (the same hooks, HAS_LIGHT_DARK_BANISH_SUMMON). The
 * effect is the card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no limit per turn:
 *   prepare  face up on the field and 1000 LP
 *   chainA   1000 LP (DUEL_CMD_LOSE_LP, a cost)
 *   resolve  every card on both fields (zones 0-10, as Final Destiny walks them) goes to the Graveyard
 *            (SendFieldCardToGrave, with its leave-the-field effects), then every card in both hands
 *            (DUEL_CMD_SEND_HAND_CARD_TO_GRAVEYARD: sent, not discarded, so no discard effect); then the opponent
 *            takes 300 for each card sent, from either player (counted before: a token goes nowhere and does not
 *            count; the Dragon itself does)
 * The CPU: AiWantsMonsterEffect (AiWantsChaosEmperorDragon). */
#define CED_COST 1000
#define CED_DAMAGE_PER_CARD 300

int EffectChaosEmperorDragonPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelZone *z;

    if (fromHand || card->event != RESPONSE_NONE || gDuelPlayers[card->player & 1].lifePoints < CED_COST)
        return 0;
    z = ZoneOf(card->player, card->zone);
    return z->isFaceUp && DUEL_CARD_ID(&z->card) == (card->card & CARD_ID_MASK);
}

int EffectChaosEmperorDragonChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_LOSE_LP : DUEL_CMD_LOSE_LP, CED_COST, 1, 0);
    return 1;
}

/* 1 if card `id` is a token (it leaves the field without going to a Graveyard). */
static int IsTokenCardId(u16 id)
{
    u16 number = ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK];

    return (u16)(number - CARD_NUMBER_TOKEN_FIRST) < CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST;
}

int EffectChaosEmperorDragonResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent = 1 - (link->player & 1);
    int side, player, zone, i, count, sent = 0;

    if (link->negated)
        return EFFECT_STEP_DONE;
    for (side = 0; side <= 1; side++) {
        player = side ? opponent : link->player & 1;
        for (zone = ZONE_MONSTER_0; zone <= ZONE_FIELD; zone++) {
            struct DuelZone *z = ZoneOf(player, zone);
            u16 id = DUEL_CARD_ID(&z->card);

            if (id == 0)
                continue;
            if (!IsTokenCardId(id))
                sent++;
            SendFieldCardToGrave(player, zone, 0, 1);
        }
    }
    for (side = 0; side <= 1; side++) {
        struct DuelPlayer *p;

        player = side ? opponent : link->player & 1;
        p = &gDuelPlayers[player];
        count = p->handCount;
        sent += count;
        for (i = 0; i < count; i++) {
            /* the hand keeps its holes until the last card, so the indices stay valid */
            DuelCmd_Push(player ? DUEL_CMD_PLAYER | DUEL_CMD_SEND_HAND_CARD_TO_GRAVEYARD
                                : DUEL_CMD_SEND_HAND_CARD_TO_GRAVEYARD,
                         i, i == count - 1, 0);
        }
    }
    if (sent > 0)
        LoseLifePoints(opponent, sent * CED_DAMAGE_PER_CARD);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END */

#ifdef HAS_CARD_TRIBE_INFECTING_VIRUS
/* ---- Tribe-Infecting Virus -----------------------------------------------------------------------------------
 * "Once per turn: You can discard 1 card, then declare 1 Monster Type; destroy all face-up monsters of the declared
 * Type on the field."
 * An activated effect from the card menu in the Main Phase (CanActivateMonsterEffect), once per turn through the
 * zone's effectUnused flag (DuelCmd_TurnStart sets it again). The pieces are the game's own: the discard cost of
 * Magic Jammer (DuelPrompt_PostDiscardCost), the type prompt of DNA Surgery (EffectDeclareTypeChainB: targets[0] is
 * the enum CardType) and the destruction of Eradicating Aerosol (DestroyFieldCard + OnCardDestroyedByEffect, the
 * opponent's monsters first). The Virus does not target, so face-up is the only condition. */

int EffectTribeInfectingVirusPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelZone *z;

    if (fromHand || card->event != RESPONSE_NONE)
        return 0;
    z = ZoneOf(card->player, card->zone);
    if (!z->isFaceUp || DUEL_CARD_ID(&z->card) != (card->card & CARD_ID_MASK) || !z->effectUnused)
        return 0;
    return gDuelPlayers[card->player & 1].handCount != 0;   /* a card to discard */
}

/* Cost: discard 1 card; the effect is used for this turn. */
int EffectTribeInfectingVirusChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    u16 cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_EFFECT_UNUSED : DUEL_CMD_SET_EFFECT_UNUSED;

    DuelPrompt_PostDiscardCost(link->player, 1, 0, 0);
    DuelCmd_Push(cmd, link->zone, 0, 0);
    return 1;
}

int EffectTribeInfectingVirusResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int round, side, zone;
    u32 type;

    if (link->negated || link->numTargets != 1)
        return EFFECT_STEP_DONE;
    type = link->targets[0];
    for (round = 0; round <= 1; round++) {
        side = round ? link->player : 1 - link->player;
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            struct DuelZone *z = ZoneOf(side, zone);
            if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp && GetZoneCardType(side, zone) == type) {
                DestroyFieldCard(side, zone, 1);
                OnCardDestroyedByEffect(link->player, side, zone);
            }
        }
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_TRIBE_INFECTING_VIRUS */

#ifdef HAS_CARD_BOTTOMLESS_TRAP_HOLE
/* ---- Bottomless Trap Hole ------------------------------------------------------------------------------------------
 * Normal Trap: "When your opponent Summons a monster(s) with 1500 or more ATK: Destroy that monster(s) with 1500 or
 * more ATK, and if you do, banish it."
 * Torrential Tribute's summon window (RESPONSE_SUMMONED, RESPONSE_FLIP_SUMMONED or RESPONSE_SPECIAL_SUMMONED, the
 * summoned monster at loc0); Trap Hole's own handlers test their card numbers and only Normal / Flip Summons. The
 * monster must be the opponent's, face up, with 1500 or more ATK, at activation and at resolution. It is banished
 * (BanishFieldCard: no Graveyard, so no Graveyard effects). */

#define BOTTOMLESS_MIN_ATK 1500

static int BottomlessTarget(struct ChainEntry *card)
{
    int player = DUEL_LOC_PLAYER(card->loc0);
    int zone = DUEL_LOC_ZONE(card->loc0);
    struct DuelZone *z;

    switch (card->event) {
    case RESPONSE_SUMMONED:
    case RESPONSE_FLIP_SUMMONED:
    case RESPONSE_SPECIAL_SUMMONED:
        break;
    default:
        return 0;
    }
    if (player == card->player || zone > ZONE_MONSTER_4)
        return 0;
    z = ZoneOf(player, zone);
    return DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp && (int)GetZoneCardAtk(player, zone) >= BOTTOMLESS_MIN_ATK;
}

int EffectBottomlessTrapHolePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand && BottomlessTarget(card);
}

int EffectBottomlessTrapHoleResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated && BottomlessTarget(link))
        BanishFieldCard(DUEL_LOC_PLAYER(link->loc0), DUEL_LOC_ZONE(link->loc0), 0);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_BOTTOMLESS_TRAP_HOLE */

#ifdef HAS_CARD_CHAIN_DISAPPEARANCE
/* ---- Chain Disappearance -------------------------------------------------------------------------------------------
 * Normal Trap, its original TCG text (the fetched one is the later errata, with no cost and either player's summon):
 * "Activate only when your opponent Normal Summons, Flip Summons or Special Summons a monster with 1000 or less ATK.
 * Pay 500 Life Points to remove from play the summoned monster and all cards with the same name from your opponent's
 * hand and Deck."
 *   window   Bottomless Trap Hole's (RESPONSE_SUMMONED, RESPONSE_FLIP_SUMMONED, RESPONSE_SPECIAL_SUMMONED, the summoned
 *            monster at loc0): the opponent's, face up, with 1000 or less ATK; checked again at resolution
 *   chainA   500 LP (DUEL_CMD_LOSE_LP, a cost; AiSelfLpLoss keeps the CPU from paying its last)
 *   resolve  as Chain Destruction (effect_resolve4.c), banishing: the monster (BanishFieldCard), every card of the
 *            opponent's hand with its name (IsSameCardName; DUEL_CMD_BANISH_HAND_CARD, the hand compacted after the
 *            last), every copy in their Deck (BanishDeckCopies), then the Deck is shuffled
 *   CPU      a Set copy goes off whenever the human's summon qualifies (AiShouldActivateAddedCard's default yes) */
#define CHAIN_DISAPPEARANCE_MAX_ATK 1000
#define CHAIN_DISAPPEARANCE_COST 500

void BanishDeckCopies(int player, u16 cardNo);     /* duel_card_actions.c */

static int ChainDisappearanceTarget(struct ChainEntry *card)
{
    int player = DUEL_LOC_PLAYER(card->loc0);
    int zone = DUEL_LOC_ZONE(card->loc0);
    struct DuelZone *z;

    switch (card->event) {
    case RESPONSE_SUMMONED:
    case RESPONSE_FLIP_SUMMONED:
    case RESPONSE_SPECIAL_SUMMONED:
        break;
    default:
        return 0;
    }
    if (player == card->player || zone > ZONE_MONSTER_4)
        return 0;
    z = ZoneOf(player, zone);
    return DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp
           && (int)GetZoneCardAtk(player, zone) <= CHAIN_DISAPPEARANCE_MAX_ATK;
}

int EffectChainDisappearancePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand && gDuelPlayers[card->player & 1].lifePoints >= CHAIN_DISAPPEARANCE_COST
           && ChainDisappearanceTarget(card);
}

int EffectChainDisappearanceChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_LOSE_LP : DUEL_CMD_LOSE_LP, CHAIN_DISAPPEARANCE_COST, 1, 0);
    return 1;
}

int EffectChainDisappearanceResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent = DUEL_LOC_PLAYER(link->loc0) & 1;
    struct DuelPlayer *p = &gDuelPlayers[opponent];
    u16 id;
    int i, count, last = -1;
    u16 cmd = opponent ? DUEL_CMD_PLAYER | DUEL_CMD_BANISH_HAND_CARD : DUEL_CMD_BANISH_HAND_CARD;

    if (link->negated || !ChainDisappearanceTarget(link))
        return EFFECT_STEP_DONE;
    id = DUEL_CARD_ID(&ZoneOf(opponent, DUEL_LOC_ZONE(link->loc0))->card);
    BanishFieldCard(opponent, DUEL_LOC_ZONE(link->loc0), 0);
    /* the hand keeps its holes until the last banished card, which compacts it, so the indices stay valid */
    count = p->handCount;
    for (i = 0; i < count; i++) {
        if (IsSameCardName(DUEL_CARD_ID(&p->hand[i]), id))
            last = i;
    }
    for (i = 0; i <= last; i++) {
        if (IsSameCardName(DUEL_CARD_ID(&p->hand[i]), id))
            DuelCmd_Push(cmd, i, i == last, 0);
    }
    BanishDeckCopies(opponent, ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK]);
    DuelCmd_Push(opponent ? DUEL_CMD_PLAYER | DUEL_CMD_SHUFFLE_DECK : DUEL_CMD_SHUFFLE_DECK, 0, 0, 0);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_CHAIN_DISAPPEARANCE */

#ifdef HAS_CARD_ROYAL_OPPRESSION
/* ---- Royal Oppression (YGOPRODeck id 93016201) -------------------------------------------------------------------
 * Continuous Trap: "Either player can pay 800 Life Points to negate the Special Summon of a monster(s), and/or an
 * effect that Special Summons a monster(s), and destroy those cards."
 *   prepare  on the field, 800+ LP, RESPONSE_SPECIAL_SUMMONED with the summoned monster at loc0 (either player's
 *            Special Summon; not Normal/Flip). Chains to the summon window like Horn of Heaven (effect_prepare1.c),
 *            but only Special Summons. Face-up reactivation like Skull Lair (card_command_menu.c, effect_activation.c,
 *            effect_targets4.c).
 *   chainA   800 LP (DUEL_CMD_LOSE_LP; AiSelfLpLoss)
 *   resolve  Horn of Heaven's negate (DUEL_CMD_CLEAR_ZONE_STATUS_FLAGS on loc0, then DestroyFieldCard)
 *   CPU      AiShouldActivateAddedCard always chains on the human's Special Summon when LP > 800;
 *            AiChainSummonTriggerResponse also chains a face-up copy. Not modelled: negating a Spell/Trap effect that
 *            Special Summons before the RESPONSE_SPECIAL_SUMMONED window. */

#define ROYAL_OPPRESSION_COST 800
#define PLAYER_CMD_RO(player, cmd) ((player) ? (DUEL_CMD_PLAYER | (cmd)) : (cmd))
#define RO_ZONE_SUMMON_FLAGS (ZONE_STATUS_UNK14 | ZONE_STATUS_NORMAL_SUMMONED | ZONE_STATUS_SPECIAL_SUMMONED)

static int RoyalOppressionSpecialSummonTarget(struct ChainEntry *card)
{
    int player = DUEL_LOC_PLAYER(card->loc0);
    int zone = DUEL_LOC_ZONE(card->loc0);

    if (card->event != RESPONSE_SPECIAL_SUMMONED)
        return 0;
    if (zone > ZONE_MONSTER_4)
        return 0;
    return DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0;
}

int EffectRoyalOppressionPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand && (int)gDuelPlayers[card->player & 1].lifePoints > ROYAL_OPPRESSION_COST
           && RoyalOppressionSpecialSummonTarget(card);
}

int EffectRoyalOppressionChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_LOSE_LP : DUEL_CMD_LOSE_LP, ROYAL_OPPRESSION_COST, 1,
                 0);
    return 1;
}

int EffectRoyalOppressionResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (link->negated || !RoyalOppressionSpecialSummonTarget(link))
        return EFFECT_STEP_DONE;
    if (gChain.effectStep == EFFECT_STEP_START) {
        DuelCmd_Push(PLAYER_CMD_RO((u8)link->loc0, DUEL_CMD_CLEAR_ZONE_STATUS_FLAGS), link->loc0 >> 8,
                     RO_ZONE_SUMMON_FLAGS, 0);
        return EFFECT_STEP_2;
    }
    DestroyFieldCard((u8)link->loc0, link->loc0 >> 8, 1);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_ROYAL_OPPRESSION */

#ifdef HAS_CARD_TORRENTIAL_TRIBUTE
/* ---- Torrential Tribute --------------------------------------------------------------------------------------
 * "When a monster(s) is Summoned: Destroy all monsters on the field."
 * A Normal Trap answering the response window that a summon opens (SummonAction_Update, EventResponse_Run): event
 * RESPONSE_SUMMONED, RESPONSE_FLIP_SUMMONED or RESPONSE_SPECIAL_SUMMONED (not RESPONSE_SET), with the summoned
 * monster at loc0, like Trap Hole (EffectTrapHolePrepare) but for any monster and either player's summon. The
 * effect is Dark Hole's (EffectDarkHoleResolve: every monster, the opponent's first). */
int EffectTorrentialTributePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    switch (card->event) {
    case RESPONSE_SUMMONED:
    case RESPONSE_FLIP_SUMMONED:
    case RESPONSE_SPECIAL_SUMMONED:
        return DUEL_CARD_ID(&ZoneOf(DUEL_LOC_PLAYER(card->loc0), DUEL_LOC_ZONE(card->loc0))->card) != 0;
    }
    return 0;
}
#endif /* HAS_CARD_TORRENTIAL_TRIBUTE */

#ifdef HAS_CARD_SAKURETSU_ARMOR
/* ---- Sakuretsu Armor -----------------------------------------------------------------------------------------
 * "When an opponent's monster declares an attack: Target the attacking monster; destroy that target."
 * Normal Trap during RESPONSE_ATTACK_DECLARED (loc0 = attacker, loc1 = attack target), like Negate Attack /
 * Enchanted Javelin. Only loc0 is a legal target; chainB sets it; EffectDestroyTargetResolve destroys it. */

static int SakuretsuAttackerValid(struct ChainEntry *card)
{
    u8 attackerPlayer = DUEL_LOC_PLAYER(card->loc0);
    int zone = DUEL_LOC_ZONE(card->loc0);
    struct DuelZone *z;

    if (attackerPlayer == card->player)
        return 0;
    z = ZoneOf(attackerPlayer & 1, zone);
    if (DUEL_CARD_ID(&z->card) == 0 || !z->isFaceUp || z->isDefense)
        return 0;
    return CanCardTargetZone(card->card, attackerPlayer, zone);
}

int EffectSakuretsuArmorPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (fromHand != 0)
        return 0;
    if (card->event != RESPONSE_ATTACK_DECLARED)
        return 0;
    return SakuretsuAttackerValid(card);
}

u16 EffectSakuretsuArmorCheck(struct ChainEntry *card, u16 pos)
{
    if (pos != card->loc0)
        return 0;
    return SakuretsuAttackerValid(card);
}

int EffectSakuretsuArmorChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    link->numTargets = 0;
    if (SakuretsuAttackerValid(link) == 0)
        return 1;
    link->targets[0] = link->loc0;
    link->numTargets = 1;
    return 1;
}

int EffectSakuretsuArmorResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    return EffectDestroyTargetResolve(link);
}
#endif /* HAS_CARD_SAKURETSU_ARMOR */

#ifdef HAS_CARD_D_D_WARRIOR_LADY
/* ---- D.D. Warrior Lady ----------------------------------------------------------------------------------------
 * "After damage calculation, if this card battled an opponent's monster: You can banish that monster, also banish
 * this card."
 * No effect row: the effect lives in BattleStage_DestroyMonsters (battle_phase2.c), which asks its controller before
 * the destroyed monsters leave the field (the human through PROMPT_CONFIRM_CARD_EFFECT with this text; the CPU always
 * says Yes). On Yes the battle goes on like a Dimensional Warrior battle: a destroyed monster is banished instead of
 * sent to the Graveyard, and a monster that survived is banished too. */
const u8 gStrDDWarriorLadyPrompt[] = "Use D.D. Warrior Lady's effect to banish both monsters?";
#endif /* HAS_CARD_D_D_WARRIOR_LADY */

#ifdef HAS_CARD_TRAP_DUSTSHOOT
/* ---- Trap Dustshoot -------------------------------------------------------------------------------------------
 * "Activate only if your opponent has 4 or more cards in their hand. Look at your opponent's hand, select 1 Monster
 * Card in it, and return that card to its owner's Deck."
 * The pick is Confiscation's (PROMPT_PICK_OPPONENT_HAND_CARD: the opponent's hand is revealed and the cursor picks a
 * card); a card that is not a monster plays the error sound and the prompt runs again. The CPU takes the first
 * monster in the human's hand. The card goes to the bottom of the Deck (ReturnHandCardToDeck), which is then
 * shuffled. With no monster in the opponent's hand nothing happens. */

/* Index of the first monster in the hand of `player`, or -1. */
static int FindHandMonster(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    for (i = 0; i < p->handCount; i++) {
        if (CARD_STATS_TYPE(gCardStats[DUEL_CARD_ID(&p->hand[i]) & CARD_ID_MASK]) <= CARD_TYPE_REPTILE)
            return i;
    }
    return -1;
}

int EffectTrapDustshootPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return gDuelPlayers[(1 - card->player) & 1].handCount >= 4;
}

int EffectTrapDustshootResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent = 1 - link->player;
    int slot;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (FindHandMonster(opponent) < 0)
            return EFFECT_STEP_DONE;
        if (link->player) {                 /* the CPU */
            slot = FindHandMonster(opponent);
            break;
        }
        DuelPrompt_Post(link->player, PROMPT_PICK_OPPONENT_HAND_CARD, 0, 0);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:                     /* promptResult: the picked hand slot */
        slot = gDuel.promptResult;
        if (CARD_STATS_TYPE(gCardStats[DUEL_CARD_ID(&gDuelPlayers[opponent & 1].hand[slot]) & CARD_ID_MASK])
            > CARD_TYPE_REPTILE) {
            PlaySE(SE_ERROR);
            DuelPrompt_Post(link->player, PROMPT_PICK_OPPONENT_HAND_CARD, 0, 0);
            return EFFECT_STEP_2;
        }
        break;
    default:
        return EFFECT_STEP_DONE;
    }
    ReturnHandCardToDeck(opponent, slot, 0);
    DuelCmd_Push(opponent ? DUEL_CMD_PLAYER | DUEL_CMD_SHUFFLE_DECK : DUEL_CMD_SHUFFLE_DECK, 0, 0, 0);
    return EFFECT_STEP_END;
}
#endif /* HAS_CARD_TRAP_DUSTSHOOT */

#ifdef HAS_CARD_RAIGEKI_BREAK
/* ---- Raigeki Break -------------------------------------------------------------------------------------------
 * "Discard 1 card, then target 1 card on the field; destroy it."
 * Normal Trap from the field (or hand): discard cost like Tribe-Infecting Virus / Magic Jammer
 * (DuelPrompt_PostDiscardCost), target any card on either side (monsters and Spell/Trap zones, CanCardTargetZone),
 * destroy with EffectDestroyTargetResolve. */

extern const u8 gStrDesignateFirstCardToDestroy[];      /* Greenkappa: any card on the field */

#define RAIGEKI_BREAK_POS(player, zone) ((u8)(player) | ((u8)(zone) << 8))
#define RAIGEKI_BREAK_PICK (PICK_BOTH_SIDES(PICK_ANY_MONSTER | PICK_ANY_SPELL_TRAP))

static int RaigekiBreakHasTarget(struct ChainEntry *card)
{
    int player;
    int zone;

    for (player = 0; player <= 1; player++) {
        for (zone = 0; zone <= ZONE_FIELD; zone++) {
            if (EffectRaigekiBreakCheck(card, RAIGEKI_BREAK_POS(player, zone)))
                return 1;
        }
    }
    return 0;
}

int EffectRaigekiBreakPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (chainLink != NULL)
        return 0;
    if (gDuelPlayers[card->player & 1].handCount < 1 + fromHand)
        return 0;
    return RaigekiBreakHasTarget(card);
}

u16 EffectRaigekiBreakCheck(struct ChainEntry *card, u16 pos)
{
    int player = (u8)pos;
    int zone = pos >> 8;
    struct DuelZone *target;

    if (!CanCardTargetZone(card->card, player, zone))
        return 0;
    if (zone <= ZONE_MONSTER_4) {
        target = ZoneOf(player, zone);
        return DUEL_CARD_ID(&target->card) != 0;
    }
    if (zone >= ZONE_SPELL_0 && zone <= ZONE_FIELD) {
        target = ZoneOf(player, zone);
        return DUEL_CARD_ID(&target->card) != 0;
    }
    return 0;
}

int EffectRaigekiBreakChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelPrompt_PostDiscardCost(link->player, 1, 0, 0);
    return 1;
}

int EffectRaigekiBreakChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (link->player) {
        int player;
        int zone;

        link->numTargets = 0;
        if (CanActivateEffect(link, prevLink, 0) == 0)
            return 1;
        for (player = 0; player <= 1; player++) {
            for (zone = 0; zone <= ZONE_FIELD; zone++) {
                if (EffectRaigekiBreakCheck(link, RAIGEKI_BREAK_POS(player, zone))) {
                    if (TryAddEffectTarget(link, player, zone) != 0)
                        return 1;
                }
            }
        }
        return 1;
    }
    switch (gChain.targetStep) {
    case 0:
        link->numTargets = 0;
        if (CanActivateEffect(link, prevLink, 0) == 0)
            return 1;
        TextBoxOpen(0x206, 0x712, TEXTBOX_FLAGS_DEFAULT, gStrDesignateFirstCardToDestroy);
        gChain.targetStep++;
        return 0;
    default:
        if (gMain.newKeys & B_BUTTON) {
            gChain.targetStep = 0;
            return 0;
        }
        if (DuelCursor_PickTarget(RAIGEKI_BREAK_PICK) != 0) {
            int pickPlayer = gDuelScreen.selPlayer;
            int pickZone = gDuelScreen.selArea + gDuelScreen.selIndex;

            if (EffectRaigekiBreakCheck(link, RAIGEKI_BREAK_POS(pickPlayer, pickZone))) {
                if (TryAddEffectTarget(link, pickPlayer, pickZone) != 0)
                    return 1;
            }
            PlaySE(SE_ERROR);
        }
        return 0;
    }
}

int EffectRaigekiBreakResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    return EffectDestroyTargetResolve(link);
}
#endif /* HAS_CARD_RAIGEKI_BREAK */

#ifdef HAS_CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE
/* ---- Dekoichi the Battlechanted Locomotive --------------------------------------------------------------------
 * "FLIP: Draw 1 card, then draw 1 more card for each 'Bokoichi the Freightening Car' you control."
 * A flip effect (HasFlipEffect, card_detail.c). Bokoichi counts only when it is one of the added cards. */
int EffectDekoichiResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int count = 1;

    if (link->negated)
        return EFFECT_STEP_DONE;
#ifdef HAS_CARD_BOKOICHI_THE_FREIGHTENING_CAR
    count += CountMonstersByNumber(link->player, CARD_BOKOICHI_THE_FREIGHTENING_CAR);
#endif
    DrawCards(link->player, count);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE */

/* ---- Monsters turned face down ----------------------------------------------------------------------------------
 * DuelCmd_FlipCard (duel_cmd_field.c) calls this when it turns the monster in (player, zone) face down: a face-down
 * monster loses its counters, so the Spell Counter link (ZONE_LINK_ATK_300_PER_VALUE, one per zone; Breaker the
 * Magical Warrior) goes. It runs inside the command, so the link is removed at once. */
/* The Spell Counters to show on the monster in (player, zone) (ModDrawCounterBadges, src/mod_field_icons.c): a face-up
 * Breaker the Magical Warrior's or Skilled Dark Magician's, else 0 (other cards' kind-11 links are not counters). */
int ModZoneSpellCounters(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    if (id == 0 || !z->isFaceUp)
        return 0;
    switch (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK]) {
#ifdef HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR
    case CARD_BREAKER_THE_MAGICAL_WARRIOR:
#endif
#ifdef HAS_CARD_SKILLED_DARK_MAGICIAN
    case CARD_SKILLED_DARK_MAGICIAN:
#endif
        return CountSpellCounters(player, zone);
    }
    return 0;
}

void OnMonsterTurnedFaceDown(int player, int zone)
{
    RemoveZoneLink(DUEL_LOC(player, zone), 0, ZONE_LINK_ATK_300_PER_VALUE);
    RemoveZoneLink(DUEL_LOC(player, zone), DUEL_CARD_ID(&ZoneOf(player, zone)->card), ZONE_LINK_SPELL_COUNTER);
    RemoveZoneLink(DUEL_LOC(player, zone), DUEL_CARD_ID(&ZoneOf(player, zone)->card), ZONE_LINK_SWAP_STATS);
}

#if defined(HAS_CARD_BOOK_OF_MOON) || defined(HAS_CARD_TSUKUYOMI) || defined(HAS_CARD_GOLEM_SENTRY) \
    || defined(HAS_CARD_GUARDIAN_SPHINX)
/* ---- Book of Moon / Tsukuyomi ------------------------------------------------------------------------------------
 * Book of Moon (Quick-Play Spell) and Tsukuyomi (FLIP) share the same effect: target 1 face-up monster on the
 * field; change it to face-down Defense Position. Targeting uses EffectTargetableFaceUpMonsterCheck / ChainB
 * (Chaos Sorcerer's pickers). The resolve first destroys the target's Equip Cards (they cannot equip a face-down
 * monster; a destroyed Premature Burial takes the monster with it). Then DUEL_CMD_CHANGE_POSITION puts a monster
 * in Attack Position into Defense Position, and DUEL_CMD_FLIP_CARD turns it face down, which removes its counters
 * (OnMonsterTurnedFaceDown). Tsukuyomi is listed in HasFlipEffect (card_detail.c). */
int EffectBookOfMoonResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    u16 equips[32];
    int count = 0;
    int player, zone, i;
    struct DuelZone *z;
    u16 cmd;

    if (link->negated || link->numTargets != 1)
        return EFFECT_STEP_DONE;
    player = (u8)link->targets[0];
    zone = link->targets[0] >> 8;
    z = ZoneOf(player, zone);
    if (zone > ZONE_MONSTER_4 || DUEL_CARD_ID(&z->card) == 0 || !z->isFaceUp)
        return EFFECT_STEP_DONE;
    for (i = 0; i < z->numLinks; i++) {
        u16 target = z->links[i];
        if ((z->linkKinds[i] == ZONE_LINK_EQUIP || z->linkKinds[i] == ZONE_LINK_EQUIP_ATK_200)
            && DUEL_CARD_ID(&ZoneOf(DUEL_LOC_PLAYER(target), DUEL_LOC_ZONE(target))->card) != 0)
            equips[count++] = target;
    }
    for (i = 0; i < count; i++)
        DestroyFieldCard(DUEL_LOC_PLAYER(equips[i]), DUEL_LOC_ZONE(equips[i]), 1);
    if (!z->isDefense) {
        cmd = player ? DUEL_CMD_PLAYER | DUEL_CMD_CHANGE_POSITION : DUEL_CMD_CHANGE_POSITION;
        DuelCmd_Push(cmd, zone, 0, 0);
    }
    cmd = player ? DUEL_CMD_PLAYER | DUEL_CMD_FLIP_CARD : DUEL_CMD_FLIP_CARD;
    DuelCmd_Push(cmd, zone, 0, 0);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_BOOK_OF_MOON || HAS_CARD_TSUKUYOMI || HAS_CARD_GOLEM_SENTRY || HAS_CARD_GUARDIAN_SPHINX */

#ifdef HAS_CARD_TSUKUYOMI
#ifndef CARD_NUMBER
#define CARD_NUMBER(id) (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[(id) & CARD_ID_MASK])
#endif
/* ---- Tsukuyomi (summon restriction, triggers, End Phase return) -----------------------------------------------
 * "Cannot be Special Summoned. If this card is Normal Summoned or flipped face-up: Target 1 face-up monster …
 * Once per turn, during the End Phase, if this card was Normal Summoned or flipped face-up this turn: Return it
 * to the hand."
 * The flip-to-face-down effect shares EffectBookOfMoonResolve; EffectTsukuyomiPrepare limits activation to
 * Normal Summon (face-up), Flip Summon and battle flip. TsukuyomiAfterNormalSummon (summon_action.c) marks End
 * Phase return and queues the field effect when Normal Summoned face-up. HasFlipEffect + SummonStep_Flip handle
 * Flip Summon; battle_phase2 marks End Phase return on battle flip. EndPhase_ReturnTsukuyomi (battle_phase3.c)
 * returns marked copies during the End Phase. */


#define TSUKUYOMI_MONSTER_TRIGGER(event) (((event) << 25) | (CHAIN_KIND_MONSTER << 21))

static inline u16 TsukuyomiPlayerCmd(int player, u16 cmd)
{
    return player ? cmd | DUEL_CMD_PLAYER : cmd;
}

int IsCannotSpecialSummon(u16 cardId)
{
    if (CARD_NUMBER(cardId) == CARD_TSUKUYOMI)
        return 1;
#ifdef HAS_CARD_YATA_GARASU
    if (CARD_NUMBER(cardId) == CARD_YATA_GARASU)    /* "This card cannot be Special Summoned." */
        return 1;
#endif
#ifdef HAS_CARD_ASURA_PRIEST
    if (CARD_NUMBER(cardId) == CARD_ASURA_PRIEST)   /* "Cannot be Special Summoned." */
        return 1;
#endif
    return 0;
}

void TsukuyomiMarkForEndReturn(int player, int zone)
{
    if (zone >= ZONE_MONSTER_0 && zone <= ZONE_MONSTER_4)
        gModDuel.tsukuyomiEndReturn[player & 1] |= (u8)(1 << zone);
}

static int IsTsukuyomiHere(struct ChainEntry *card)
{
    struct DuelZone *z = ZoneOf(card->player, card->zone);

    return z->isFaceUp && DUEL_CARD_ID(&z->card) == (card->card & CARD_ID_MASK);
}

int EffectTsukuyomiPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (fromHand != 0 || !IsTsukuyomiHere(card))
        return 0;
    switch (card->event) {
    case RESPONSE_FLIP_SUMMONED:
    case RESPONSE_BATTLE_FLIP_EFFECT:
    case RESPONSE_FLIPPED:
        return 1;
    case RESPONSE_SUMMONED:
        return card->loc0 == DUEL_LOC(card->player, card->zone);
    default:
        return 0;
    }
}

void TsukuyomiAfterNormalSummon(int player, int zone, u16 cardId, int faceUp)
{
    TsukuyomiMarkForEndReturn(player, zone);
    if (faceUp == 0)
        return;
    if (CountActiveCardsOnField(0, CARD_1530) != 0 || CountActiveCardsOnField(1, CARD_1530) != 0)
        return;
    if (FlipEffectsLockedForTurn())
        return;
    {
        struct ChainEntry test;

        test.player = player;
        test.zone = zone;
        test.card = cardId;
        test.event = RESPONSE_SUMMONED;
        test.loc0 = DUEL_LOC(player, zone);
        if (CanActivateEffect(&test, NULL, 0) == 0)
            return;
        {
            u32 trigger = (u32)(player & 1) << 31;
            u32 destination = (u32)zone << 16;

            destination |= TSUKUYOMI_MONSTER_TRIGGER(RESPONSE_SUMMONED);
            trigger |= destination | cardId;
            Chain_AddPending(trigger, DUEL_LOC(player, zone));
        }
    }
}

void EndPhase_ReturnTsukuyomi(int player)
{
    int zone;
    u8 mask = gModDuel.tsukuyomiEndReturn[player & 1];

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if ((mask & (1 << zone)) == 0)
            continue;
        {
            struct DuelZone *z = ZoneOf(player, zone);
            u16 id = DUEL_CARD_ID(&z->card);

            if (id != 0 && CARD_NUMBER(id) == CARD_TSUKUYOMI) {
                DuelCmd_Push(TsukuyomiPlayerCmd(player, DUEL_CMD_SHOW_CARD_EFFECT), id, 1, 0);
                ReturnFieldCardToHand(player, zone, 0);
            }
        }
    }
    gModDuel.tsukuyomiEndReturn[player & 1] = 0;
}
#endif /* HAS_CARD_TSUKUYOMI */

#if defined(HAS_CARD_NINJA_GRANDMASTER_SASUKE) || defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) \
    || defined(HAS_CARD_SASUKE_SAMURAI)
/* ---- Attackers that destroy the target before the Damage Step ---------------------------------------------------
 * Ninja Grandmaster Sasuke: "If this card attacks a Defense Position monster, destroy that monster immediately with
 * this card's effect without applying damage calculation."
 * Mystic Swordsman LV2: "If this card attacks a face-down Defense Position monster: Destroy the monster immediately
 * with this card's effect without flipping it face-up or applying damage calculation."
 * Sasuke Samurai (2026-10-30): "At the start of the Damage Step, if this card attacks a face-down Defense Position
 * monster: Destroy that face-down monster." Destroyed at the start of the Damage Step, it is never flipped and no
 * damage is calculated, so it is handled as Mystic Swordsman LV2.
 * No effect row: BattleStage_RevealDefender (battle_phase1.c) calls this before it flips the target, so a face-down
 * target is destroyed without being flipped (no flip effect). The attack is marked as made (DUEL_CMD_MARK_ATTACKED,
 * as the Damage Step would), the target is destroyed by the card's effect (DestroyFieldCard with its Graveyard
 * triggers, OnCardDestroyedByEffect), and the battle jumps to BATTLE_STAGE_END_ATTACK: no damage calculation, no
 * battle damage, no "destroyed by battle" effects (so no Mystic Swordsman level-up either). */
int DestroyTargetBeforeDamageStep(int player)
{
    struct DuelZone *attacker = ZoneOf(player, gBattle.atkSlot);
    struct DuelZone *target;
    int opponent = 1 - player;
    u16 id = DUEL_CARD_ID(&attacker->card);
    int destroy = 0;

    if (gBattle.direct || id == 0 || !attacker->isFaceUp)
        return 0;
    target = ZoneOf(opponent, gBattle.defSlot);
    if (DUEL_CARD_ID(&target->card) == 0)
        return 0;
    switch (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK]) {
#ifdef HAS_CARD_NINJA_GRANDMASTER_SASUKE
    case CARD_NINJA_GRANDMASTER_SASUKE:
        destroy = target->isDefense;
        break;
#endif
#ifdef HAS_CARD_MYSTIC_SWORDSMAN_LV2
    case CARD_MYSTIC_SWORDSMAN_LV2:
#endif
#ifdef HAS_CARD_SASUKE_SAMURAI
    case CARD_SASUKE_SAMURAI:
#endif
#if defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) || defined(HAS_CARD_SASUKE_SAMURAI)
        destroy = target->isDefense && !target->isFaceUp;
        break;
#endif
    }
    if (!destroy)
        return 0;
    DuelCmd_Push(player ? DUEL_CMD_PLAYER | DUEL_CMD_MARK_ATTACKED : DUEL_CMD_MARK_ATTACKED, gBattle.atkSlot, 1, 0);
    ShowCardEffect(player, id);
    DestroyFieldCard(opponent, gBattle.defSlot, 1);
    OnCardDestroyedByEffect(player, opponent, gBattle.defSlot);
    gDuel.battleStage = BATTLE_STAGE_END_ATTACK;
    gDuel.battleStep = 0;
    return 1;
}

/* AiEvalAttack (ai_picks.c): 1 if the CPU's monster in attackerZone destroys the human's Defense Position monster in
 * targetZone before the Damage Step (above), so the attack wins whatever that monster's DEF. */
int AiDestroysBeforeDamage(int attackerZone, int targetZone)
{
    struct DuelZone *attacker = ZoneOf(1, attackerZone);
    struct DuelZone *target = ZoneOf(0, targetZone);
    u16 id = DUEL_CARD_ID(&attacker->card);

    if (id == 0 || !attacker->isFaceUp || DUEL_CARD_ID(&target->card) == 0 || !target->isDefense)
        return 0;
    switch (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK]) {
#ifdef HAS_CARD_NINJA_GRANDMASTER_SASUKE
    case CARD_NINJA_GRANDMASTER_SASUKE:
        return 1;
#endif
#ifdef HAS_CARD_MYSTIC_SWORDSMAN_LV2
    case CARD_MYSTIC_SWORDSMAN_LV2:
#endif
#ifdef HAS_CARD_SASUKE_SAMURAI
    case CARD_SASUKE_SAMURAI:
#endif
#if defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) || defined(HAS_CARD_SASUKE_SAMURAI)
        return !target->isFaceUp;
#endif
    }
    return 0;
}
#endif /* HAS_CARD_NINJA_GRANDMASTER_SASUKE || HAS_CARD_MYSTIC_SWORDSMAN_LV2 || HAS_CARD_SASUKE_SAMURAI */

/* ---- Mod state of a duel -----------------------------------------------------------------------------------------
 * gModDuel (EWRAM 0x0202FF04, symbols.ld) holds what the added cards remember during a duel; DuelPhase_Init
 * (duel_main.c) clears it with ModDuelInit, since that RAM is not cleared otherwise. */

void ModDuelInit(void)
{
    gModDuel.battleKills = 0;
    gModDuel.chaosSummoned = 0;
    gModDuel.tsukuyomiEndReturn[0] = 0;
    gModDuel.tsukuyomiEndReturn[1] = 0;
    gModDuel.flipEffectsLocked = 0;
    gModDuel.aiStandbyTurn = 0xFFFF;
    gModDuel.dimensionReturn = 0;
    gModDuel.aiThunderDragonSearched = 0;
    gModDuel.aiTributeFor = 0xFFFF;
    gModDuel.sandMothPending = 0;
    gModDuel.mirageDrawn[0] = 0;
    gModDuel.mirageDrawn[1] = 0;
    gModDuel.standbyPart = 0;
    gModDuel.roarTurn[0] = 0;
    gModDuel.roarTurn[1] = 0;
    gModDuel.scientistReturn = 0;
    gModDuel.aiScientistTurn = 0;
    gModDuel.asuraTurn = 0;
    gModDuel.noDamageTurn[0] = 0;
    gModDuel.noDamageTurn[1] = 0;
    gModDuel.noEffectDamageTurn[0] = 0;
    gModDuel.noEffectDamageTurn[1] = 0;
    gModDuel.ddcReturnPending[0] = 0;
    gModDuel.ddcReturnPending[1] = 0;
    gModDuel.ddcReturnCardId[0] = 0;
    gModDuel.ddcReturnCardId[1] = 0;
    gModDuel.exarionAskStep = 0;
    gModDuel.exarionPierce = 0;
    MemClear16(gModDuel.exarionPenaltyStacks, sizeof(gModDuel.exarionPenaltyStacks));
#ifdef HAS_CARD_INJECTION_FAIRY_LILY
    InjectionFairyLilyResetBattle();
#endif
#ifdef HAS_CARD_EXARION_UNIVERSE
    ExarionUniverseResetBattle();
#endif
}

#if (defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV4)) \
    || (defined(HAS_CARD_DARK_MIMIC_LV1) && defined(HAS_CARD_DARK_MIMIC_LV3))
/* ---- LV monsters: send this card to the GY, Special Summon the next level from the Deck or hand -------------------
 * Shared by Mystic Swordsman LV2 (asks the human first) and Dark Mimic LV1 (activated from the card menu, so it does
 * not ask). LevelUpResolve's steps: the card `from` must still be face up in link's zone and a `to` in the Deck or
 * hand; it goes to the Graveyard (SendFieldCardToGrave, not destroyed) and a `to` is taken from the Deck (then
 * shuffled), else the hand, kept in gChain.effectCard; the next step, once those commands ran (the zone is free),
 * Special Summons it (QueueSpecialSummonChoosePosition picks the free zone when called). Not modelled: the hand/Deck
 * choice (the Deck is taken first). */

static inline int IsCardNumberAt(int player, int zone, u16 number)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    return id != 0 && z->isFaceUp && ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == number;
}

/* Index of the first card of `number` in the pile, or -1. */
static int FindCardNumberIn(struct DuelCard *pile, int count, u16 number)
{
    int i;

    for (i = 0; i < count; i++) {
        if (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[DUEL_CARD_ID(&pile[i]) & CARD_ID_MASK] == number)
            return i;
    }
    return -1;
}

static int HasLevelUpTarget(int player, u16 to)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];

    return FindCardNumberIn(p->deck, p->deckCount, to) >= 0 || FindCardNumberIn(p->hand, p->handCount, to) >= 0;
}

/* `question`: the human's Yes/No text, or NULL to go ahead at once (the CPU never asks). */
static int LevelUpResolve(struct ChainEntry *link, u16 from, u16 to, const u8 *question)
{
    struct DuelPlayer *p = &gDuelPlayers[link->player & 1];
    int i;
    u16 *halves;
    u16 cmd;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (!IsCardNumberAt(link->player, link->zone, from) || !HasLevelUpTarget(link->player, to))
            return EFFECT_STEP_DONE;
        if (link->player || question == NULL)
            return EFFECT_STEP_3;
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, question);
        TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        if (!gTextBox.result)
            return EFFECT_STEP_DONE;
        /* fall through */
    case EFFECT_STEP_3:
        if (!IsCardNumberAt(link->player, link->zone, from))
            return EFFECT_STEP_DONE;
        SendFieldCardToGrave(link->player, link->zone, 0, 1);
        if ((i = FindCardNumberIn(p->deck, p->deckCount, to)) >= 0) {
            CopyDuelCard((u32 *)&gChain.effectCard, (u32 *)&p->deck[i]);
            halves = (u16 *)&p->deck[i];
            cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_DECK : DUEL_CMD_REMOVE_CARD_FROM_DECK;
            DuelCmd_Push(cmd, halves[0], halves[1], 0);
            cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SHUFFLE_DECK : DUEL_CMD_SHUFFLE_DECK;
            DuelCmd_Push(cmd, 0, 0, 0);
        } else if ((i = FindCardNumberIn(p->hand, p->handCount, to)) >= 0) {
            CopyDuelCard((u32 *)&gChain.effectCard, (u32 *)&p->hand[i]);
            halves = (u16 *)&p->hand[i];
            cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_HAND : DUEL_CMD_REMOVE_CARD_FROM_HAND;
            DuelCmd_Push(cmd, halves[0], halves[1], 0);
            cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_COMPACT_HAND : DUEL_CMD_COMPACT_HAND;
            DuelCmd_Push(cmd, 0, 0, 0);
        } else {
            return EFFECT_STEP_DONE;
        }
        return EFFECT_STEP_4;
    case EFFECT_STEP_4:
        QueueSpecialSummonChoosePosition(link->player, &gChain.effectCard, 1, 0);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}
#endif

#if defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV4)
/* ---- Mystic Swordsman LV2: the level-up ---------------------------------------------------------------------------
 * "At the End Phase of a turn in which this card destroyed a monster by battle: You can send this card to the GY;
 * Special Summon 1 'Mystic Swordsman LV4' from your hand or Deck."
 *   BattleStage_DestroyMonsters (battle_phase2.c)  MysticSwordsmanAfterBattle: an LV2 (attacking or attacked) whose
 *                                                  opponent's monster is destroyed by the battle gets its bit in
 *                                                  gModDuel.battleKills.
 *   DuelPhase_End (battle_phase3.c)                EndPhase_MysticSwordsmanLevelUp: each marked zone that still holds
 *                                                  a face-up LV2 queues a trigger (RESPONSE_NONE, its zone), then the
 *                                                  marks are cleared.
 *   the trigger's resolve                          the human is asked (the CPU always says Yes), the LV2 is sent to
 *                                                  the Graveyard and an LV4 is taken from the Deck (else the hand);
 *                                                  the next step, once those commands ran (so the LV2's zone is free),
 *                                                  Special Summons it (QueueSpecialSummonChoosePosition). */

static const u8 sStrMysticSwordsmanLevelUp[] = "Send Mystic Swordsman LV2 to the Graveyard to Special Summon "
                                               "Mystic Swordsman LV4?";

void MysticSwordsmanAfterBattle(int player)
{
    int side, who, zone;

    for (side = 0; side <= 1; side++) {
        who = side ? 1 - player : player;
        zone = side ? gBattle.defSlot : gBattle.atkSlot;
        if (gBattle.direct && side)
            continue;
        if (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[gBattle.side[who].cardId & CARD_ID_MASK] == CARD_MYSTIC_SWORDSMAN_LV2
            && gBattle.side[1 - who].destroyed)
            gModDuel.battleKills |= 1 << (who * 5 + zone);
    }
}

void EndPhase_MysticSwordsmanLevelUp(int turnPlayer)
{
    int pass, player, zone;

    for (pass = 0; pass <= 1; pass++) {
        player = pass ? 1 - turnPlayer : turnPlayer;
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if ((gModDuel.battleKills & (1 << (player * 5 + zone)))
                && IsCardNumberAt(player, zone, CARD_MYSTIC_SWORDSMAN_LV2))
                Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                                     | TRIGGER_EVENT(RESPONSE_NONE) | DUEL_CARD_ID(&ZoneOf(player, zone)->card),
                                 DUEL_LOC(player, zone));
        }
    }
    gModDuel.battleKills = 0;
}

/* Index of the first Mystic Swordsman LV4 in the deck (or hand), or -1. */
int EffectMysticSwordsmanLv2Resolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    return LevelUpResolve(link, CARD_MYSTIC_SWORDSMAN_LV2, CARD_MYSTIC_SWORDSMAN_LV4, sStrMysticSwordsmanLevelUp);
}
#endif /* HAS_CARD_MYSTIC_SWORDSMAN_LV2 && HAS_CARD_MYSTIC_SWORDSMAN_LV4 */

#ifdef HAS_CARD_DARK_MIMIC_LV1
/* ---- Dark Mimic LV1 ------------------------------------------------------------------------------------------------
 * "FLIP: Draw 1 card. Once per turn, during your Standby Phase: You can send this card to the GY; Special Summon 1
 *  'Dark Mimic LV3' from your hand or Deck." (the card data's text)
 * One row, told apart by the event: a flip (HasFlipEffect, card_detail.c) draws 1; RESPONSE_OWN_STANDBY is the
 * level-up (LevelUpResolve), activated like Jigen Bakudan's Standby effect: the human from the card menu during
 * STANDBY_STEP_FIELD (CanActivateMonsterEffect, HasActivatableStandbyCard, the card menu's Standby case in
 * duel_cmd_queue.c), the CPU through AiStandbyActivations, which DuelPhase_Standby calls on its turn. */

int EffectDarkMimicLv1Prepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (card->event != RESPONSE_OWN_STANDBY)
        return 1;                       /* the flip effect */
#ifdef HAS_CARD_DARK_MIMIC_LV3
    return !fromHand && IsCardNumberAt(card->player, card->zone, CARD_DARK_MIMIC_LV1)
           && HasLevelUpTarget(card->player, CARD_DARK_MIMIC_LV3);
#else
    return 0;
#endif
}

int EffectDarkMimicLv1Resolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (link->event == RESPONSE_OWN_STANDBY) {
#ifdef HAS_CARD_DARK_MIMIC_LV3
        return LevelUpResolve(link, CARD_DARK_MIMIC_LV1, CARD_DARK_MIMIC_LV3, NULL);
#else
        return EFFECT_STEP_DONE;
#endif
    }
    if (!link->negated)
        DrawCards(link->player, 1);
    return EFFECT_STEP_DONE;
}

/* The CPU's Standby Phase activations (DuelPhase_Standby, STANDBY_STEP_FIELD, on the CPU's turn; that step otherwise
 * waits for the human's cursor): its first face-up Dark Mimic LV1 that may level up, once per turn. 1 if it queued
 * one. */
int AiStandbyActivations(int player)
{
    int zone;

    if (gModDuel.aiStandbyTurn == gDuel.turnCount)
        return 0;
    gModDuel.aiStandbyTurn = gDuel.turnCount;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);

        if (IsCardNumberAt(player, zone, CARD_DARK_MIMIC_LV1)
            && CanActivateEffectInZone(player, zone, RESPONSE_OWN_STANDBY)) {
            DuelCmd_Push(DUEL_CMD_PLAYER | DUEL_CMD_POINT_AT_CARD, player, (u8)zone << 8, 0);
            Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                                 | TRIGGER_EVENT(RESPONSE_OWN_STANDBY) | id,
                             0);
            return 1;
        }
    }
    return 0;
}
#endif /* HAS_CARD_DARK_MIMIC_LV1 */

#ifdef HAS_CARD_ZOMBYRA_THE_DARK
/* ---- Zombyra the Dark ----------------------------------------------------------------------------------------------
 * "This card cannot attack directly. Each time this card destroys a monster by battle, it loses 200 ATK."
 *   CanMonsterAttack (card_menu_input.c; the CPU's attacker list uses it too)   it may not attack while the opponent
 *                                                                              has no monster (that is a direct attack)
 *   BattleStage_DestroyMonsters (battle_phase2.c, AddedCardsAfterBattle)        when it survives a battle that
 *                                                                              destroyed the other monster, a self link
 *                                                                              ZONE_LINK_ATK_DOWN_200 (from its card ID;
 *                                                                              AddZoneLink raises the count of an
 *                                                                              existing one) takes 200 ATK more
 * The link goes with the card when it leaves the field. */
static void ZombyraAfterBattle(int player)
{
    int side, who, zone;

    for (side = 0; side <= 1; side++) {
        if (gBattle.direct && side)
            continue;
        who = side ? 1 - player : player;
        zone = side ? gBattle.defSlot : gBattle.atkSlot;
        if (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[gBattle.side[who].cardId & CARD_ID_MASK] == CARD_ZOMBYRA_THE_DARK
            && gBattle.side[1 - who].destroyed && !gBattle.side[who].destroyed)
            QueueAddZoneLink(who, gBattle.side[who].cardId & CARD_ID_MASK, DUEL_LOC(who, zone),
                             ZONE_LINK_KIND(ZONE_LINK_ATK_DOWN_200, 1));
    }
}
#endif /* HAS_CARD_ZOMBYRA_THE_DARK */

#if (defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV4)) \
    || defined(HAS_CARD_ZOMBYRA_THE_DARK)
/* BattleStage_DestroyMonsters calls this once per battle, before the destroyed monsters leave the field. */
void AddedCardsAfterBattle(int player)
{
#if defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV4)
    MysticSwordsmanAfterBattle(player);
#endif
#ifdef HAS_CARD_ZOMBYRA_THE_DARK
    ZombyraAfterBattle(player);
#endif
}
#endif

#ifdef HAS_CARD_SMASHING_GROUND
/* ---- Smashing Ground -----------------------------------------------------------------------------------------------
 * Normal Spell: "Destroy the 1 face-up monster your opponent controls that has the highest DEF (your choice, if tied)."
 * No target. Prepare: the opponent has a face-up monster. Resolve, like Widespread Ruin's highest-ATK search: the
 * highest DEF among the opponent's face-up monsters (GetZoneCardDef); on a tie the human picks one of them with the
 * cursor (another pick plays the error sound), the CPU takes the first. Then DestroyFieldCard +
 * OnCardDestroyedByEffect. */

static const u8 sStrSmashingGroundPick[] = "Select the monster with the highest DEF to destroy.";

/* The highest DEF among the face-up monsters of `player` (-1: none); *count gets how many have it, *first the first. */
static int HighestFaceUpDef(int player, int *count, int *first)
{
    int zone, best = -1;

    *count = 0;
    *first = -1;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);
        int def;

        if (DUEL_CARD_ID(&z->card) == 0 || !z->isFaceUp)
            continue;
        def = GetZoneCardDef(player, zone);
        if (def > best) {
            best = def;
            *count = 1;
            *first = zone;
        } else if (def == best) {
            (*count)++;
        }
    }
    return best;
}

int EffectSmashingGroundPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    int count, first;

    return HighestFaceUpDef(1 - card->player, &count, &first) >= 0;
}

static void SmashingGroundDestroy(struct ChainEntry *link, int opponent, int zone)
{
    DestroyFieldCard(opponent, zone, 1);
    OnCardDestroyedByEffect(link->player, opponent, zone);
}

int EffectSmashingGroundResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent = 1 - link->player;
    int best, count, first, zone;

    if (link->negated)
        return EFFECT_STEP_DONE;
    best = HighestFaceUpDef(opponent, &count, &first);
    if (best < 0)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (count == 1 || link->player) {
            SmashingGroundDestroy(link, opponent, first);
            return EFFECT_STEP_DONE;
        }
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, sStrSmashingGroundPick);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        /* the human picks among the tied monsters on the opponent's side (player 1) */
        if (DuelCursor_PickTarget(PICK_PLAYER1(PICK_FACE_UP_MONSTER_ANY)) == 0)
            return EFFECT_STEP_2;
        zone = gDuelScreen.selArea + gDuelScreen.selIndex;
        if (gDuelScreen.selPlayer != opponent || zone > ZONE_MONSTER_4 || !ZoneOf(opponent, zone)->isFaceUp
            || DUEL_CARD_ID(&ZoneOf(opponent, zone)->card) == 0 || (int)GetZoneCardDef(opponent, zone) != best) {
            PlaySE(SE_ERROR);
            return EFFECT_STEP_2;
        }
        SmashingGroundDestroy(link, opponent, zone);
        return EFFECT_STEP_DONE;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_SMASHING_GROUND */

#ifdef HAS_CARD_MAGICAL_MERCHANT
/* ---- Magical Merchant ------------------------------------------------------------------------------------------------
 * "FLIP: Excavate cards from the top of your Deck until you excavate a Spell/Trap. Add that card to your hand, also
 * send the remaining cards to the GY."
 * A flip effect (HasFlipEffect, card_detail.c). The top of the Deck is deck[0]. The monsters above the first
 * Spell/Trap go to the Graveyard (DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD, one by one); with no Spell/Trap at all the
 * whole Deck goes. The Spell/Trap is shown and added with Sangan's AddDeckCardToHand, which takes the first card of
 * that number in the Deck: that one, as every card above it is a monster. */
int EffectMagicalMerchantResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    struct DuelPlayer *p = &gDuelPlayers[link->player & 1];
    u16 cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD : DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD;
    int i;

    if (link->negated)
        return EFFECT_STEP_DONE;
    for (i = 0; i < p->deckCount; i++) {
        u16 id = DUEL_CARD_ID(&p->deck[i]);
        if (CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) > CARD_TYPE_REPTILE)
            break;
        DuelCmd_Push(cmd, ((u16 *)&p->deck[i])[0], ((u16 *)&p->deck[i])[1], 0);
    }
    if (i < p->deckCount) {
        u16 id = DUEL_CARD_ID(&p->deck[i]);
        ShowPickedCard(link->player, id);
        AddDeckCardToHand(link->player, ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK]);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_MAGICAL_MERCHANT */

#ifdef HAS_CARD_SPIRIT_REAPER
/* ---- Spirit Reaper -------------------------------------------------------------------------------------------------
 * "Cannot be destroyed by battle. After this card is targeted by an effect, destroy this card. When this card inflicts
 * battle damage to your opponent: Discard 1 random card from their hand."
 *   CalcBattle (campaign.c)                   its destroyed flag is cleared, as attacker or target
 *   TryAddEffectTarget (effect_targets1.c)     OnFieldCardTargeted: a face-up Spirit Reaper that an effect targets is
 *                                              destroyed at once (its own effect, so the targeting effect finds it gone)
 *   BattleStage_InflictDamage (battle_phase2.c) White Magical Hat's case: the opponent discards 1 random card
 * TryAddEffectTarget also records the picks of a few effects that Tribute rather than target (Catapult Turtle's
 * EffectTributeTargetChainB and Metamorphosis); those do not count. */
void OnFieldCardTargeted(struct ChainEntry *link, int player, int zone)
{
    struct DuelZone *z;
    u16 id;

    if (zone > ZONE_MONSTER_4)
        return;
    switch (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[link->card & CARD_ID_MASK]) {
    case CARD_CATAPULT_TURTLE:
    case CARD_CANNON_SOLDIER:
#ifdef HAS_CARD_TOON_CANNON_SOLDIER
    case CARD_TOON_CANNON_SOLDIER:
#endif
    case CARD_THE_LITTLE_SWORDSMAN_OF_AILE:
#ifdef HAS_CARD_METAMORPHOSIS
    case CARD_METAMORPHOSIS:
#endif
#ifdef HAS_CARD_MONSTER_GATE
    case CARD_MONSTER_GATE:
#endif
        return;     /* a Tribute, not a target */
    }
    z = ZoneOf(player, zone);
    id = DUEL_CARD_ID(&z->card);
    if (id == 0 || !z->isFaceUp || ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] != CARD_SPIRIT_REAPER)
        return;
    ShowCardEffect(player, id);
    DestroyFieldCard(player, zone, 1);
}
#endif /* HAS_CARD_SPIRIT_REAPER */

#ifdef HAS_CARD_DARK_MAGICIAN_OF_CHAOS
/* ---- Dark Magician of Chaos (the 2005 text) -------------------------------------------------------------------------
 * "When this card is Normal or Special Summoned, select 1 Spell Card from your Graveyard during the End Phase, and add
 *  it to your hand. If this card destroys a monster by battle, remove that monster from play. If this face-up card is
 *  removed from the field, remove it from play."
 *   SummonAction_Update (summon_action.c)  DarkMagicianOfChaosSummoned: a face-up Normal Summon or any Special Summon
 *                                          sets its bit in gModDuel.chaosSummoned (a Set or a Flip Summon does not)
 *   DuelPhase_End (battle_phase3.c)        EndPhase_DarkMagicianOfChaos: a trigger for each marked zone whose player
 *                                          has a Spell in the Graveyard (the summon was enough: the card need not be
 *                                          on the field any more); the marks are cleared. Its row is Magician of
 *                                          Faith's: the Graveyard Spells in the card-list viewer
 *                                          (EffectCardListTargetChainB, CollectEffectTargets case shared with it,
 *                                          AiPickCardListEntry's case too), added by EffectMaskOfDarknessResolve
 *   BattleStage_DestroyMonsters            with it in the battle, the destroyed monsters are banished instead of sent
 *                                          to the Graveyard (gBattle.ddWarriorLady = DD_BANISH_DESTROYED; survivors
 *                                          stay)
 *   SendFieldCardToGrave, TributeMonster, ReturnFieldCardToHand, ReturnFieldCardToDeck
 *                                          IsFaceUpDarkMagicianOfChaos: the face-up card is banished instead
 *                                          (BanishFieldCard) */

int IsFaceUpDarkMagicianOfChaos(int player, int zone)
{
    struct DuelZone *z;
    u16 id;

    if (zone > ZONE_MONSTER_4)
        return 0;
    z = ZoneOf(player, zone);
    id = DUEL_CARD_ID(&z->card);
    return id != 0 && z->isFaceUp
           && ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_DARK_MAGICIAN_OF_CHAOS;
}

void DarkMagicianOfChaosSummoned(int player, int zone, u16 id, int kind, int faceUp)
{
    if (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] != CARD_DARK_MAGICIAN_OF_CHAOS)
        return;
    if (kind == SUMMON_ACTION_FLIP || ((kind == SUMMON_ACTION_NORMAL || kind == SUMMON_ACTION_NORMAL_CHOOSE_POSITION)
                                       && !faceUp))
        return;
    gModDuel.chaosSummoned |= 1 << ((player & 1) * 5 + zone);
}

static int HasGraveyardSpell(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    for (i = 0; i < p->graveCount; i++) {
        if (CARD_STATS_TYPE(gCardStats[DUEL_CARD_ID(&p->graveyard[i]) & CARD_ID_MASK]) == CARD_TYPE_MAGIC)
            return 1;
    }
    return 0;
}

void EndPhase_DarkMagicianOfChaos(int turnPlayer)
{
    int pass, player, zone;

    for (pass = 0; pass <= 1; pass++) {
        player = pass ? 1 - turnPlayer : turnPlayer;
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if ((gModDuel.chaosSummoned & (1 << (player * 5 + zone))) && HasGraveyardSpell(player))
                Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                                     | TRIGGER_EVENT(RESPONSE_NONE)
                                     | gCardNumberToId[CARD_DARK_MAGICIAN_OF_CHAOS],
                                 DUEL_LOC(player, zone));
        }
    }
    gModDuel.chaosSummoned = 0;
}
#endif /* HAS_CARD_DARK_MAGICIAN_OF_CHAOS */

#ifdef HAS_CARD_ABYSS_SOLDIER
/* ---- Abyss Soldier -------------------------------------------------------------------------------------------------
 * "Once per turn: You can discard 1 WATER monster; return 1 card on the field to the hand."
 * The card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), once per turn (effectUnused, set again by
 * DuelCmd_TurnStart), like Tribe-Infecting Virus.
 *   chainA   the cost. The human gets the discard-cost prompt with DISCARD_FLAG_WATER (duel_turn_end.c: a pick must
 *            be a WATER monster, IsWaterMonster); the CPU discards its first WATER monster at once (the prompt's CPU
 *            pick ignores its flags). The effect is used for the turn.
 *   chainB   any card on the field it may target (monster, Spell/Trap and Field zones). The step does not ask
 *            CanActivateEffect again: the cost is paid by then. The CPU returns the human's strongest monster, else
 *            the human's first card.
 *   resolve  ReturnFieldCardToHand (to its owner's hand; a Fusion Monster goes to the Fusion Deck, a token is
 *            destroyed). */

static const u8 sStrAbyssSoldierPick[] = "Designate the card to return to the hand.";

#define ABYSS_SOLDIER_PICK (PICK_BOTH_SIDES(PICK_ANY_MONSTER | PICK_ANY_SPELL_TRAP))

int IsWaterMonster(u16 id)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];

    return CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_ATTR(stats) == ATTRIBUTE_WATER;
}

static int FindWaterMonsterInHand(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    for (i = 0; i < p->handCount; i++) {
        if (IsWaterMonster(DUEL_CARD_ID(&p->hand[i])) && !p->hand[i].planted && !p->hand[i].graverobbed)
            return i;
    }
    return -1;
}

static int AbyssSoldierCanTarget(struct ChainEntry *link, int player, int zone)
{
    if (zone > ZONE_FIELD || !CanCardTargetZone(link->card, player, zone))
        return 0;
    return DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0;
}

int EffectAbyssSoldierPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelZone *z;
    int player, zone;

    if (fromHand || card->event != RESPONSE_NONE)
        return 0;
    z = ZoneOf(card->player, card->zone);
    if (!z->isFaceUp || DUEL_CARD_ID(&z->card) != (card->card & CARD_ID_MASK) || !z->effectUnused
        || FindWaterMonsterInHand(card->player) < 0)
        return 0;
    for (player = 0; player <= 1; player++) {
        for (zone = 0; zone <= ZONE_FIELD; zone++) {
            if (AbyssSoldierCanTarget(card, player, zone))
                return 1;
        }
    }
    return 0;
}

u16 EffectAbyssSoldierCheck(struct ChainEntry *card, u16 pos)
{
    return AbyssSoldierCanTarget(card, (u8)pos, pos >> 8);
}

int EffectAbyssSoldierChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    u16 cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_EFFECT_UNUSED : DUEL_CMD_SET_EFFECT_UNUSED;

    if (link->player) {
        int i = FindWaterMonsterInHand(link->player);
        if (i >= 0)
            DiscardHandCard(link->player, i, 0, 1);
    } else {
        /* PROMPT_DISCARD_COST of 1 card: monsters only (1) and WATER only (4, DISCARD_FLAG_WATER) */
        DuelPrompt_Post(0, PROMPT_DISCARD_COST, 1, 1 | 4);
    }
    DuelCmd_Push(cmd, link->zone, 0, 0);
    return 1;
}

int EffectAbyssSoldierChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int opponent = 1 - link->player;
    int zone;

    if (link->player) {
        link->numTargets = 0;
        zone = CountMonsters(opponent) > 0 ? AiFindStrongestMonster(opponent, -1, 1, 1) : -1;
        if (zone >= 0 && AbyssSoldierCanTarget(link, opponent, zone) && TryAddEffectTarget(link, opponent, zone))
            return 1;
        for (zone = 0; zone <= ZONE_FIELD; zone++) {
            if (AbyssSoldierCanTarget(link, opponent, zone) && TryAddEffectTarget(link, opponent, zone))
                return 1;
        }
        return 1;
    }
    switch (gChain.targetStep) {
    case 0:
        link->numTargets = 0;
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, sStrAbyssSoldierPick);
        gChain.targetStep++;
        return 0;
    default:
        if (gMain.newKeys & B_BUTTON) {
            gChain.targetStep = 0;
            return 0;
        }
        if (DuelCursor_PickTarget(ABYSS_SOLDIER_PICK) != 0) {
            int pickPlayer = gDuelScreen.selPlayer;
            int pickZone = gDuelScreen.selArea + gDuelScreen.selIndex;

            if (AbyssSoldierCanTarget(link, pickPlayer, pickZone) && TryAddEffectTarget(link, pickPlayer, pickZone))
                return 1;
            PlaySE(SE_ERROR);
        }
        return 0;
    }
}

int EffectAbyssSoldierResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated && link->numTargets == 1) {
        int player = (u8)link->targets[0];
        int zone = link->targets[0] >> 8;

        if (DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0)
            ReturnFieldCardToHand(player, zone, 0);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_ABYSS_SOLDIER */

#ifdef HAS_CARD_BRAIN_CONTROL
/* ---- Brain Control -------------------------------------------------------------------------------------------------
 * Normal Spell: "Pay 800 LP, then target 1 face-up monster your opponent controls that can be Normal Summoned/Set;
 * take control of it until the End Phase."
 * Change of Heart's way, like Mind Control: EffectChangeOfHeartResolve moves the target and links it to this card
 * (ZONE_LINK_CARD_EFFECT), and DuelCmd_TurnEnd returns it (ReturnControlAtTurnEnd with CARD_BRAIN_CONTROL).
 *   prepare  more than 800 LP, a free monster zone, and a monster IsBrainControlTarget accepts
 *   chainA   the 800 LP (DUEL_CMD_LOSE_LP as a cost)
 *   chainB   EffectTakeControlChainB with Snatch Steal's face-up cursor (effect_targets2.c), wrapped: a monster that
 *            cannot be Normal Summoned/Set (a Fusion, Ritual or Special-Summon-only monster, a token) is refused, and
 *            the CPU, whose pick is the human's strongest monster, takes the strongest valid one instead */

#define BRAIN_CONTROL_COST 800

static int IsBrainControlTarget(struct ChainEntry *link, int player, int zone)
{
    struct DuelZone *z;
    u16 id;
    u32 stats;

    if (player == link->player || zone > ZONE_MONSTER_4)
        return 0;
    z = ZoneOf(player, zone);
    id = DUEL_CARD_ID(&z->card);
    if (id == 0 || !z->isFaceUp || !CanCardTargetZone(link->card, player, zone))
        return 0;
    stats = gCardStats[id & CARD_ID_MASK];
    if (CARD_STATS_TYPE(stats) > CARD_TYPE_REPTILE || CARD_STATS_KIND(stats) == CARD_KIND_FUSION
        || CARD_STATS_KIND(stats) == CARD_KIND_RITUAL || IsSpecialSummonOnly(id))
        return 0;
    return (u16)(((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] - CARD_NUMBER_TOKEN_FIRST)
           >= CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST;
}

u16 EffectBrainControlCheck(struct ChainEntry *card, u16 pos)
{
    return IsBrainControlTarget(card, (u8)pos, pos >> 8);
}

int EffectBrainControlPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    int zone;

    if (gDuelPlayers[card->player & 1].lifePoints <= BRAIN_CONTROL_COST || CountFreeMonsterZones(card->player) == 0)
        return 0;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (IsBrainControlTarget(card, 1 - card->player, zone))
            return 1;
    }
    return 0;
}

int EffectBrainControlChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_LOSE_LP : DUEL_CMD_LOSE_LP, BRAIN_CONTROL_COST, 1, 0);
    return 1;
}

int EffectBrainControlChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int opponent = 1 - link->player;

    if (!EffectTakeControlChainB(link))
        return 0;
    if (link->numTargets == 1 && IsBrainControlTarget(link, (u8)link->targets[0], link->targets[0] >> 8))
        return 1;
    link->numTargets = 0;
    if (!link->player) {
        int zone;

        /* the human picked a monster that cannot be Normal Summoned/Set: stay on the cursor, while there is a
         * valid one */
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (IsBrainControlTarget(link, opponent, zone)) {
                PlaySE(SE_ERROR);
                return 0;
            }
        }
        return 1;
    }
    {
        int zone, best = -1;
        u32 bestAtk = 0;

        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (IsBrainControlTarget(link, opponent, zone) && (best < 0 || GetZoneCardAtk(opponent, zone) > bestAtk)) {
                best = zone;
                bestAtk = GetZoneCardAtk(opponent, zone);
            }
        }
        if (best >= 0)
            TryAddEffectTarget(link, opponent, best);
    }
    return 1;
}
#endif /* HAS_CARD_BRAIN_CONTROL */

#ifdef HAS_CARD_XING_ZHEN_HU
/* ---- Xing Zhen Hu --------------------------------------------------------------------------------------------------
 * Continuous Trap: "Activate by selecting 2 Set Spell or Trap Cards on the field. The selected Spell or Trap Cards
 * cannot be activated." (the card data's text)
 * Like Spellbinding Circle on a monster: the resolve gives each target still Set a ZONE_LINK_CONTINUOUS link whose
 * value is Xing Zhen Hu's DUEL_LOC. CanActivateEffect (effect_activation.c) asks IsLockedByXingZhenHu first, so a
 * locked Set card is refused everywhere (card menu, response windows, CPU). The lock holds while that zone holds a
 * face-up, enabled Xing Zhen Hu; a target that leaves takes its links with it (ClearZone).
 *   prepare  two Set Spell/Trap cards on the field besides this one (the Set Xing Zhen Hu counts as Set)
 *   chainB   two different Set cards with the cursor (B starts again); the CPU takes the human's */

static const u8 sStrXingZhenHuPick[] = "Select 2 Set Spell or Trap Cards.";

#define XING_PICK (PICK_BOTH_SIDES(PICK_FACE_DOWN_SPELL_TRAP))

static int IsXingTarget(struct ChainEntry *card, int player, int zone)
{
    struct DuelZone *z;

    if (zone < ZONE_SPELL_0 || zone > ZONE_SPELL_4 || (player == card->player && zone == card->zone))
        return 0;
    z = ZoneOf(player, zone);
    return DUEL_CARD_ID(&z->card) != 0 && !z->isFaceUp && CanCardTargetZone(card->card, player, zone);
}

static int CountXingTargets(struct ChainEntry *card, int onlyPlayer)
{
    int player, zone, n = 0;

    for (player = 0; player <= 1; player++) {
        if (onlyPlayer >= 0 && player != onlyPlayer)
            continue;
        for (zone = ZONE_SPELL_0; zone <= ZONE_SPELL_4; zone++)
            n += IsXingTarget(card, player, zone);
    }
    return n;
}

/* 1 if the Set card in (player, zone) is locked: its zone holds a ZONE_LINK_CONTINUOUS link from a face-up, enabled
 * Xing Zhen Hu. Asked where the zone is known: the card menu (CardMenu_GetSpellTrapCommands) and the response windows
 * (CanActivateFieldCard). */
int IsZoneLockedByXingZhenHu(int player, int zone)
{
    struct DuelZone *z;
    int i;

    if (zone < ZONE_SPELL_0 || zone > ZONE_SPELL_4)
        return 0;
    z = ZoneOf(player, zone);
    if (DUEL_CARD_ID(&z->card) == 0)
        return 0;
    for (i = 0; i < z->numLinks; i++) {
        if ((u8)z->linkKinds[i] == ZONE_LINK_CONTINUOUS) {
            u16 loc = z->links[i];
            struct DuelZone *src = ZoneOf(DUEL_LOC_PLAYER(loc), DUEL_LOC_ZONE(loc));
            u16 id = DUEL_CARD_ID(&src->card);

            if (id != 0 && src->isFaceUp && !src->isDisabled
                && ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_XING_ZHEN_HU)
                return 1;
        }
    }
    return 0;
}

/* The same for a chain entry, from CanActivateEffect: only when its zone holds its card (CanActivateEffectOfCard
 * leaves the zone unset, so the card menu also asks IsZoneLockedByXingZhenHu). */
int IsLockedByXingZhenHu(struct ChainEntry *card)
{
    if (card->zone < ZONE_SPELL_0 || card->zone > ZONE_SPELL_4
        || DUEL_CARD_ID(&ZoneOf(card->player, card->zone)->card) != (card->card & CARD_ID_MASK))
        return 0;
    return IsZoneLockedByXingZhenHu(card->player, card->zone);
}

int EffectXingZhenHuPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand && CountXingTargets(card, -1) >= 2;
}

u16 EffectXingZhenHuCheck(struct ChainEntry *card, u16 pos)
{
    return IsXingTarget(card, (u8)pos, pos >> 8);
}

int EffectXingZhenHuChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int player, zone;

    if (link->player) {
        /* the CPU: the human's Set cards first, then its own */
        link->numTargets = 0;
        for (player = 0; player <= 1 && link->numTargets < 2; player++) {
            for (zone = ZONE_SPELL_0; zone <= ZONE_SPELL_4 && link->numTargets < 2; zone++) {
                if (IsXingTarget(link, player, zone))
                    TryAddEffectTarget(link, player, zone);
            }
        }
        return 1;
    }
    switch (gChain.targetStep) {
    case 0:
        link->numTargets = 0;
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, sStrXingZhenHuPick);
        gChain.targetStep++;
        return 0;
    default:
        if (gMain.newKeys & B_BUTTON) {
            gChain.targetStep = 0;
            return 0;
        }
        if (DuelCursor_PickTarget(XING_PICK) != 0) {
            player = gDuelScreen.selPlayer;
            zone = gDuelScreen.selArea + gDuelScreen.selIndex;
            if (IsXingTarget(link, player, zone)
                && !(link->numTargets == 1 && link->targets[0] == DUEL_LOC(player, zone))
                && TryAddEffectTarget(link, player, zone))
                return link->numTargets >= 2;
            PlaySE(SE_ERROR);
        }
        return 0;
    }
}

int EffectXingZhenHuResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int i;

    if (link->negated)
        return EFFECT_STEP_DONE;
    for (i = 0; i < link->numTargets && i < 2; i++) {
        int player = (u8)link->targets[i];
        int zone = link->targets[i] >> 8;
        struct DuelZone *z = ZoneOf(player, zone);

        if (DUEL_CARD_ID(&z->card) != 0 && !z->isFaceUp)
            QueueAddZoneLink(link->player, DUEL_LOC(link->player, link->zone), link->targets[i], ZONE_LINK_CONTINUOUS);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_XING_ZHEN_HU */

#ifdef HAS_CARD_BAZOO_THE_SOUL_EATER
/* ---- Bazoo the Soul-Eater ------------------------------------------------------------------------------------------
 * "Once per turn: You can banish up to 3 monsters from your Graveyard; this card gains 300 ATK for each, until the end
 * of your opponent's turn."
 * The engine's effect key 1513 (ENGINE_NUMBERS gives the card that number): the card menu's ACTIVATE once per turn
 * (effectUnused) with a monster in the player's Graveyard, EffectBanishGraveForAtkResolve (asks before each of up to 3
 * picks, then a ZONE_LINK_ATK_300_PER_VALUE link of the count), and DuelCmd_TurnStart re-arms it and drops the link at
 * its controller's next turn. That resolve is UI only, so the CPU gets BazooCpuResolve (effect_resolve11.c calls it):
 * its 3 weakest Graveyard monsters (fewer if it has fewer), then the same link and once-per-turn mark. */
int BazooCpuResolve(struct ChainEntry *link)
{
    struct DuelPlayer *p = &gDuelPlayers[link->player & 1];
    u8 used[80];
    int n, i, count = 0;
    u16 cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_BANISH_GRAVEYARD_CARD : DUEL_CMD_BANISH_GRAVEYARD_CARD;

    for (i = 0; i < p->graveCount && i < 80; i++)
        used[i] = 0;
    for (n = 0; n < 3; n++) {
        int best = -1;
        u32 bestAtk = 0;

        for (i = 0; i < p->graveCount && i < 80; i++) {
            u32 stats = gCardStats[DUEL_CARD_ID(&p->graveyard[i]) & CARD_ID_MASK];
            if (!used[i] && CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && (best < 0 || CARD_STATS_ATK(stats) < bestAtk)) {
                best = i;
                bestAtk = CARD_STATS_ATK(stats);
            }
        }
        if (best < 0)
            break;
        used[best] = 1;
        DuelCmd_Push(cmd, ((u16 *)&p->graveyard[best])[0], ((u16 *)&p->graveyard[best])[1], 0);
        count++;
    }
    if (count != 0)
        QueueAddZoneLink(link->player, link->card, DUEL_LOC(link->player, link->zone),
                         ZONE_LINK_KIND(ZONE_LINK_ATK_300_PER_VALUE, count));
    DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_EFFECT_UNUSED : DUEL_CMD_SET_EFFECT_UNUSED, link->zone, 0, 0);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_BAZOO_THE_SOUL_EATER */

#ifdef HAS_CARD_SACRED_CRANE
/* ---- Sacred Crane --------------------------------------------------------------------------------------------------
 * "If this card is Special Summoned: Draw 1 card." AddedCardsSummoned (from SummonAction_Update, summon_action.c, once
 * the monster is placed) queues its trigger for a Special Summon (SUMMON_ACTION_SPECIAL*, any position); the resolve
 * draws 1. Mandatory, as written. */
int EffectSacredCraneResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated)
        DrawCards(link->player, 1);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_SACRED_CRANE */

#ifdef HAS_CARD_SKILLED_DARK_MAGICIAN
/* ---- Skilled Dark Magician ----------------------------------------------------------------------------------------
 * "Each time a Spell Card is activated, place 1 Spell Counter on this card when that Spell resolves (max. 3). You can
 * Tribute this card with 3 Spell Counters on it; Special Summon 1 "Dark Magician" from your hand, Deck, or GY."
 *   Counters  a ZONE_LINK_SPELL_COUNTER link from its own card ID (no stat effect; AddZoneLink raises the count of an
 *             existing one). Chain_Resolve calls ModChainLinkResolved as each link finishes: a Magic card's link that
 *             was not negated gives every face-up one on the field with fewer than 3 a counter. Apprentice Magician
 *             can place one too. Turned face down, it loses them (OnMonsterTurnedFaceDown).
 *   Effect    the card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect) with 3 counters and a Dark Magician
 *             in the hand, Deck or Graveyard (CollectEffectTargets). The cost (chainA) Tributes it; the resolve lets
 *             the human pick the Dark Magician in the card list (the CPU takes the Graveyard's first, then the
 *             Deck's), removes it from its pile and Special Summons it (shuffling the Deck if it came from there). */
#define SDM_MAX_COUNTERS 3

int CollectEffectTargetsInt(int player, int cardNumber, int param) asm("CollectEffectTargets");

static const u8 sStrSelectDarkMagician[] = "Select a Dark Magician to Special Summon.";

static inline int IsSkilledDarkMagicianAt(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    return id != 0 && z->isFaceUp
           && ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_SKILLED_DARK_MAGICIAN;
}

/* Place 1 counter on the Skilled Dark Magician in (player, zone) if it has fewer than 3; 1 if placed. */
static int SkilledDarkMagicianAddCounter(int player, int zone)
{
    u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card) & CARD_ID_MASK;

    if (!IsSkilledDarkMagicianAt(player, zone) || CountSpellCounters(player, zone) >= SDM_MAX_COUNTERS)
        return 0;
    QueueAddZoneLink(player, id, DUEL_LOC(player, zone), ZONE_LINK_KIND(ZONE_LINK_SPELL_COUNTER, 1));
    return 1;
}

int EffectSkilledDarkMagicianPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (fromHand || card->event != RESPONSE_NONE || !IsSkilledDarkMagicianAt(card->player, card->zone)
        || DUEL_CARD_ID(&ZoneOf(card->player, card->zone)->card) != (card->card & CARD_ID_MASK))
        return 0;
    return CountSpellCounters(card->player, card->zone) >= SDM_MAX_COUNTERS
           && CollectEffectTargetsInt(card->player, CARD_SKILLED_DARK_MAGICIAN, 0) > 0;
}

/* Cost: Tribute this card (its zone is then free for the Dark Magician). */
int EffectSkilledDarkMagicianChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    TributeMonster(link->player, link->zone);
    return 1;
}

int EffectSkilledDarkMagicianResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int count, i;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        count = CollectEffectTargetsInt(link->player, CARD_SKILLED_DARK_MAGICIAN, 0);
        if (count == 0 || CountFreeMonsterZones(link->player) == 0)
            return EFFECT_STEP_DONE;
        if (link->player) {
            /* the list is hand, Deck, Graveyard: take the Graveyard's first, else the Deck's, else the hand's */
            int pick = 0;

            for (i = 0; i < count; i++) {
                if (gCardListView.sources[i] == CARDLIST_SRC_GRAVEYARD) {
                    pick = i;
                    break;
                }
                if (gCardListView.sources[i] == CARDLIST_SRC_DECK && gCardListView.sources[pick] == CARDLIST_SRC_HAND)
                    pick = i;
            }
            gCardListView.cursorRow = 0;
            gCardListView.top = pick;
            return EFFECT_STEP_3;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 5), TEXTBOX_FLAGS_DEFAULT, sStrSelectDarkMagician);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        CardListView_Open(link->player, CARDLIST_AREA_EFFECT_TARGETS, CARD_SKILLED_DARK_MAGICIAN, 0);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3: {
        /* the picked card word as its two halves (DuelCmd_Push operands) */
        u16 *picked = (u16 *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top];
        u16 cmd;

        switch (gCardListView.sources[gCardListView.cursorRow + gCardListView.top]) {
        case CARDLIST_SRC_DECK:
            cmd = DUEL_CMD_REMOVE_CARD_FROM_DECK;
            break;
        case CARDLIST_SRC_HAND:
            cmd = DUEL_CMD_REMOVE_CARD_FROM_HAND;
            break;
        default:
            cmd = DUEL_CMD_REMOVE_CARD_FROM_GRAVEYARD;
            break;
        }
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | cmd : cmd, picked[0], picked[1], 0);
        if (cmd == DUEL_CMD_REMOVE_CARD_FROM_HAND)
            DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_COMPACT_HAND : DUEL_CMD_COMPACT_HAND, 0, 0, 0);
        return EFFECT_STEP_4;
    }
    case EFFECT_STEP_4:
        QueueSpecialSummonChoosePosition(link->player,
            (struct DuelCard *)&gCardListView.cards[gCardListView.top + gCardListView.cursorRow], 1, 0);
        return EFFECT_STEP_5;
    case EFFECT_STEP_5:
        if (gCardListView.sources[gCardListView.top + gCardListView.cursorRow] == CARDLIST_SRC_DECK)
            DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SHUFFLE_DECK : DUEL_CMD_SHUFFLE_DECK, 0, 0, 0);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_SKILLED_DARK_MAGICIAN */

/* Chain_Resolve calls this as each chain link finishes resolving (its effect's commands have run). */
void ModChainLinkResolved(struct ChainEntry *link)
{
#ifdef HAS_CARD_SKILLED_DARK_MAGICIAN
    int player, zone;

    if (link->negated || CARD_STATS_TYPE(gCardStats[link->card & CARD_ID_MASK]) != CARD_TYPE_MAGIC)
        return;
    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (SkilledDarkMagicianAddCounter(player, zone))
                ShowCardEffect(player, DUEL_CARD_ID(&ZoneOf(player, zone)->card));
        }
    }
#endif
}

#if defined(HAS_CARD_APPRENTICE_MAGICIAN) \
    && (defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) || defined(HAS_CARD_SKILLED_DARK_MAGICIAN))
/* ---- Apprentice Magician ------------------------------------------------------------------------------------------
 * "If this card is Summoned: Target 1 face-up card on the field that you can place a Spell Counter on; place 1 Spell
 * Counter on that target. If this card is destroyed by battle: You can Special Summon 1 Level 2 or lower Spellcaster
 * monster from your Deck in face-down Defense Position."
 *   Counter  AddedCardsSummoned (every summon kind, face up): the cards here that hold Spell Counters are Breaker
 *            the Magical Warrior (max. 1, a ZONE_LINK_ATK_300_PER_VALUE link from its own card ID) and Skilled Dark
 *            Magician (max. 3, ZONE_LINK_SPELL_COUNTER), so the counter goes on the first face-up one with room: the
 *            summoner's first, else the opponent's (the effect is mandatory); with none, nothing. Applied at once
 *            (no chain link).
 *   Battle   a floater: its "destroyed in battle" trigger (SendBattleDestroyedCardToGraveyard), the Deck list of
 *            Spellcasters of Level 2 or lower (CollectEffectTargets) and EffectSpecialSummonFromDeckResolve, which
 *            summons it face-down in Defense Position for this card (asked first; the CPU picks its best). */
static void ApprenticeMagicianPlaceCounter(int player, u16 id)
{
    int pass, zone;

    for (pass = 0; pass <= 1; pass++) {
        int side = pass ? 1 - player : player;

        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            struct DuelZone *z = ZoneOf(side, zone);
            u16 target = DUEL_CARD_ID(&z->card);

#ifdef HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR
            if (target != 0 && z->isFaceUp
                && ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[target & CARD_ID_MASK] == CARD_BREAKER_THE_MAGICAL_WARRIOR
                && CountSpellCounters(side, zone) == 0) {
                ShowCardEffect(player, id);
                QueueAddZoneLink(side, target, DUEL_LOC(side, zone), ZONE_LINK_KIND(ZONE_LINK_ATK_300_PER_VALUE, 1));
                return;
            }
#endif
#ifdef HAS_CARD_SKILLED_DARK_MAGICIAN
            if (SkilledDarkMagicianAddCounter(side, zone)) {
                ShowCardEffect(player, id);
                return;
            }
#endif
        }
    }
}
#endif /* HAS_CARD_APPRENTICE_MAGICIAN && (Breaker || Skilled Dark Magician) */

#if defined(HAS_CARD_DARK_MAGICIAN_OF_CHAOS) || defined(HAS_CARD_SACRED_CRANE) || defined(HAS_CARD_APPRENTICE_MAGICIAN) \
    || defined(HAS_CARD_GOLEM_SENTRY) || defined(HAS_CARD_SAND_MOTH) || defined(HAS_CARD_GUARDIAN_SPHINX) \
    || defined(HAS_CARD_MARAUDING_CAPTAIN)
/* SummonAction_Update calls this for every summon once the monster is in (player, zone): `kind` is the
 * enum SummonActionKind, faceUp its position. */
#ifdef HAS_CARD_SAND_MOTH
void SandMothSummoned(int player, int zone, u16 id);     /* below */
#endif
void AddedCardsSummoned(int player, int zone, u16 id, int kind, int faceUp)
{
#ifdef HAS_CARD_DARK_MAGICIAN_OF_CHAOS
    DarkMagicianOfChaosSummoned(player, zone, id, kind, faceUp);
#endif
#if defined(HAS_CARD_APPRENTICE_MAGICIAN) \
    && (defined(HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR) || defined(HAS_CARD_SKILLED_DARK_MAGICIAN))
    if (faceUp && ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_APPRENTICE_MAGICIAN)
        ApprenticeMagicianPlaceCounter(player, id);
#endif
#ifdef HAS_CARD_SAND_MOTH
    SandMothSummoned(player, zone, id);
#endif
#ifdef HAS_CARD_MARAUDING_CAPTAIN
    if (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_MARAUDING_CAPTAIN && faceUp
        && (kind == SUMMON_ACTION_NORMAL || kind == SUMMON_ACTION_NORMAL_CHOOSE_POSITION))
        Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                             | TRIGGER_EVENT(RESPONSE_SUMMONED) | (id & CARD_ID_MASK),
                         DUEL_LOC(player, zone));
#endif
#if defined(HAS_CARD_GOLEM_SENTRY) || defined(HAS_CARD_GUARDIAN_SPHINX)
    if ((0
#ifdef HAS_CARD_GOLEM_SENTRY
         || ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_GOLEM_SENTRY
#endif
#ifdef HAS_CARD_GUARDIAN_SPHINX
         || ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_GUARDIAN_SPHINX
#endif
         ) && kind == SUMMON_ACTION_FLIP)
        Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                             | TRIGGER_EVENT(RESPONSE_FLIP_SUMMONED) | (id & CARD_ID_MASK),
                         DUEL_LOC(player, zone));
#endif
#ifdef HAS_CARD_SACRED_CRANE
    if (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_SACRED_CRANE
        && kind >= SUMMON_ACTION_SPECIAL)
        Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                             | TRIGGER_EVENT(RESPONSE_SPECIAL_SUMMONED) | (id & CARD_ID_MASK),
                         DUEL_LOC(player, zone));
#endif
}
#endif

#ifdef HAS_CARD_MONSTER_GATE
/* ---- Monster Gate --------------------------------------------------------------------------------------------------
 * Normal Spell: "Tribute 1 monster; excavate cards from the top of your Deck until you excavate a monster that can be
 * Normal Summoned/Set. Special Summon it, also send the other excavated cards to the GY."
 *   prepare  a monster the player may Tribute, and a Deck monster that can be Normal Summoned/Set
 *   chainB   the cost: Catapult Turtle's EffectTributeTargetChainB pick (a monster that cannot be Tributed is
 *            refused), Tributed at once as Metamorphosis does; the CPU Tributes its weakest monster
 *   resolve  from the top of the Deck (deck[0]): the cards before the first such monster go to the Graveyard, the
 *            monster leaves the Deck; the next step (the commands done) Special Summons it face up in the
 *            position the player picks (QueueSpecialSummonChoosePosition). With none, every card goes. */

/* A monster that can be Normal Summoned/Set: not Fusion, Ritual, Special-Summon-only or a token. */
static int IsGateMonster(u16 id)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];
    u16 number = ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK];

    return CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_KIND(stats) != CARD_KIND_FUSION
           && CARD_STATS_KIND(stats) != CARD_KIND_RITUAL && !IsSpecialSummonOnly(id)
           && (u16)(number - CARD_NUMBER_TOKEN_FIRST) >= CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST;
}

static int GateWeakestTribute(int player)
{
    int zone, best = -1;
    u32 bestAtk = 0;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (IsTributableMonster(player, zone) && (best < 0 || GetZoneCardAtk(player, zone) < bestAtk)) {
            best = zone;
            bestAtk = GetZoneCardAtk(player, zone);
        }
    }
    return best;
}

int EffectMonsterGatePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelPlayer *p = &gDuelPlayers[card->player & 1];
    int i;

    if (CountTributableMonsters(card->player, -1) == 0)
        return 0;
    for (i = 0; i < p->deckCount; i++) {
        if (IsGateMonster(DUEL_CARD_ID(&p->deck[i])))
            return 1;
    }
    return 0;
}

int EffectMonsterGateChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int zone;

    if (link->player) {
        link->numTargets = 0;
        zone = GateWeakestTribute(link->player);
        if (zone >= 0)
            TributeMonster(link->player, zone);
        return 1;
    }
    if (!EffectTributeTargetChainB(link))
        return 0;
    if (link->numTargets != 1)
        return 1;
    zone = link->targets[0] >> 8;
    if ((u8)link->targets[0] != link->player || !IsTributableMonster(link->player, zone)) {
        link->numTargets = 0;
        PlaySE(SE_ERROR);
        return 0;
    }
    link->numTargets = 0;       /* a cost, not a target */
    TributeMonster(link->player, zone);
    return 1;
}

int EffectMonsterGateResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    struct DuelPlayer *p = &gDuelPlayers[link->player & 1];
    u16 toGrave = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD : DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD;
    u16 remove = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_DECK : DUEL_CMD_REMOVE_CARD_FROM_DECK;
    int i;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        for (i = 0; i < p->deckCount; i++) {
            u16 *halves = (u16 *)&p->deck[i];

            if (IsGateMonster(DUEL_CARD_ID(&p->deck[i]))) {
                ShowRevealedCard(link->player, DUEL_CARD_ID(&p->deck[i]));
                CopyDuelCard((u32 *)&gChain.effectCard, (u32 *)&p->deck[i]);
                DuelCmd_Push(remove, halves[0], halves[1], 0);
                return EFFECT_STEP_2;
            }
            DuelCmd_Push(toGrave, halves[0], halves[1], 0);
        }
        return EFFECT_STEP_DONE;
    case EFFECT_STEP_2:
        if (CountFreeMonsterZones(link->player) > 0)
            QueueSpecialSummonChoosePosition(link->player, &gChain.effectCard, 1, 0);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_MONSTER_GATE */

#if defined(HAS_CARD_REASONING) && defined(HAS_CARD_MONSTER_GATE)
/* ---- Reasoning -----------------------------------------------------------------------------------------------------
 * Normal Spell: "Your opponent declares a monster Level from 1 to 12. Excavate cards from the top of your Deck until
 * you excavate a monster that can be Normal Summoned/Set, then, if that monster is the same Level as the one declared
 * by your opponent, send all excavated cards to the GY. If not, Special Summon the excavated monster, also send the
 * remaining cards to the GY."
 * The excavation is Monster Gate's (IsGateMonster, so this needs Monster Gate listed too). The declaration:
 *   the human declares (the CPU activated it): a text box with a Level menu (LevelMenu_Draw / LevelMenu_HandleInput,
 *   made like the Monster Type menu of DuelPrompt_SelectType: LEFT/RIGHT, A), starting at Level 4;
 *   the CPU declares (the human activated it): Level 4, the most common Level (it cannot see the Deck), shown in a
 *   text box.
 * The Level is kept in gChain.effectSubStep. The monster leaves the Deck, and the next step Special Summons it face up
 * in the position the player picks. */

#define REASONING_CPU_LEVEL 4

static const u8 sStrReasoningDeclare[] = "Declare a monster Level.";
static const u8 sStrReasoningCpuDeclares[] = "Your opponent declares Level 4.";
static const u8 sStrReasoningLevel[] = "Level ";

#define LEVEL_LABEL_PALETTE     (OBJ_PLTT + 15 * 0x20)
#define LEVEL_LABEL_TILE        0x364               /* the Type menu's label tiles (duel_prompt_handlers.c) */
#define LEVEL_LABEL_TILES       ((u16 *)(OBJ_VRAM0 + LEVEL_LABEL_TILE * 0x20))

static void LevelMenu_Draw(void)
{
    struct TextBox *box = &gTextBox;
    u32 x = (box->x + 1) << 3;
    u32 y = (box->revealRow - box->height) << 3;

    if (box->menuTimer == 0) {
        char label[12];
        int i, level = box->result + 1;

        for (i = 0; sStrReasoningLevel[i]; i++)
            label[i] = sStrReasoningLevel[i];
        if (level >= 10)
            label[i++] = '1';
        label[i++] = '0' + level % 10;
        label[i] = 0;
        MemCopy16((void *)LEVEL_LABEL_PALETTE, gSystemFontPal, 0x20);
        TextCanvasInit(0xC, 2);
        TextDrawString(1, 1, TEXT_SIZE_COLOR(10, 15), (const u8 *)label);
        TextDrawString(0, 0, TEXT_SIZE_COLOR(10, 2), (const u8 *)label);
        TextCanvasToTiles(LEVEL_LABEL_TILES, 0);
        box->menuTimer++;
    }
    AddSprite(x | (y << 16), SPRITE_SHAPE_32x8, LEVEL_LABEL_TILE | OAM_ATTR2_PALETTE(15));
    AddSprite((x + 0x20) | (y << 16), SPRITE_SHAPE_32x8, (LEVEL_LABEL_TILE + 4) | OAM_ATTR2_PALETTE(15));
    AddSprite(x | ((y + 8) << 16), SPRITE_SHAPE_32x8, (LEVEL_LABEL_TILE + 12) | OAM_ATTR2_PALETTE(15));
    AddSprite((x + 0x20) | ((y + 8) << 16), SPRITE_SHAPE_32x8, (LEVEL_LABEL_TILE + 16) | OAM_ATTR2_PALETTE(15));
}

static u16 LevelMenu_HandleInput(void)
{
    struct TextBox *box = &gTextBox;

    if (box->menuTimer == 0)
        return 0;
    if (box->menuState == TEXTBOX_MENU_STATE_SELECT) {
        box->menuState++;
        return 0;
    }
    if (gMain.newKeys & DPAD_LEFT) {
        box->result = (box->result + 11) % 12;
        box->menuTimer = 0;
        return 0;
    }
    if (gMain.newKeys & DPAD_RIGHT) {
        box->result = (box->result + 1) % 12;
        box->menuTimer = 0;
        return 0;
    }
    return (gMain.newKeys & A_BUTTON) != 0;
}

int EffectReasoningPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelPlayer *p = &gDuelPlayers[card->player & 1];
    int i;

    for (i = 0; i < p->deckCount; i++) {
        if (IsGateMonster(DUEL_CARD_ID(&p->deck[i])))
            return 1;
    }
    return 0;
}

int EffectReasoningResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    struct DuelPlayer *p = &gDuelPlayers[link->player & 1];
    u16 toGrave = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD : DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD;
    u16 remove = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_DECK : DUEL_CMD_REMOVE_CARD_FROM_DECK;
    int i;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (link->player) {
            /* the human declares */
            TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(15, 4), TEXTBOX_FLAGS_DEFAULT, sStrReasoningDeclare);
            TextBoxSetMenu(TEXTBOX_MENU_CUSTOM, LevelMenu_Draw, LevelMenu_HandleInput);
            gTextBox.result = REASONING_CPU_LEVEL - 1;
            return EFFECT_STEP_2;
        }
        gChain.effectSubStep = REASONING_CPU_LEVEL;
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 4), TEXTBOX_FLAGS_DEFAULT, sStrReasoningCpuDeclares);
        return EFFECT_STEP_3;
    case EFFECT_STEP_2:
        gChain.effectSubStep = gTextBox.result + 1;
        /* fall through */
    case EFFECT_STEP_3:
        for (i = 0; i < p->deckCount; i++) {
            u16 id = DUEL_CARD_ID(&p->deck[i]);
            u16 *halves = (u16 *)&p->deck[i];

            if (IsGateMonster(id)) {
                ShowRevealedCard(link->player, id);
                if ((int)CARD_STATS_LEVEL(gCardStats[id & CARD_ID_MASK]) == gChain.effectSubStep) {
                    DuelCmd_Push(toGrave, halves[0], halves[1], 0);     /* the declared Level: all to the GY */
                    return EFFECT_STEP_DONE;
                }
                CopyDuelCard((u32 *)&gChain.effectCard, (u32 *)&p->deck[i]);
                DuelCmd_Push(remove, halves[0], halves[1], 0);
                return EFFECT_STEP_4;
            }
            DuelCmd_Push(toGrave, halves[0], halves[1], 0);
        }
        return EFFECT_STEP_DONE;
    case EFFECT_STEP_4:
        if (CountFreeMonsterZones(link->player) > 0)
            QueueSpecialSummonChoosePosition(link->player, &gChain.effectCard, 1, 0);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_REASONING && HAS_CARD_MONSTER_GATE */

#if defined(HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION) || defined(HAS_CARD_DIMENSION_FUSION)
/* ---- Return from the Different Dimension ---------------------------------------------------------------------------
 * Normal Trap: "Pay half your Life Points; Special Summon as many of your banished monsters as possible. During the End
 * Phase, banish all monsters that were Special Summoned by this effect."
 *   prepare  a free monster zone and a monster in the player's banished pile (banished normally: not face down or
 *            until the End Phase; not a token)
 *   chainA   half the LP (DUEL_CMD_LOSE_LP, a cost)
 *   resolve  one monster per step while zones and monsters remain, the strongest ATK first: out of the banished pile
 *            (RemoveBanishedCardAt) and Special Summoned face up in the position the player picks
 *            (QueueSpecialSummonChoosePosition; the chain waits for each summon). Each one's zone and card ID go to
 *            gModDuel.dimensionReturn.
 *   DuelPhase_End (battle_phase3.c)  EndPhase_ReturnFromTheDifferentDimension banishes the marked monsters still in
 *            their zones, then clears the marks.
 * A monster that cannot be Special Summoned (IsCannotSpecialSummon) stays banished: QueueSpecialSummonChoosePosition
 * would drop it after it left the pile. Dimension Fusion shares the helpers. */

static int IsDimensionReturnMonster(struct DuelPlayer *p, int i)
{
    u16 id = DUEL_CARD_ID(&p->banished[i]);
    u16 number = ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK];

    return (p->banishedInfo[i] & 0xFF) == BANISH_NORMAL
           && CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) <= CARD_TYPE_REPTILE
           && !IsCannotSpecialSummon(id & CARD_ID_MASK)
           && (u16)(number - CARD_NUMBER_TOKEN_FIRST) >= CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST;
}

/* The banished-pile index of the player's strongest such monster, -1 for none. */
static int StrongestDimensionReturn(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i, best = -1;
    u32 bestAtk = 0;

    for (i = 0; i < p->banishedCount; i++) {
        u32 atk = CARD_STATS_ATK(gCardStats[DUEL_CARD_ID(&p->banished[i]) & CARD_ID_MASK]);
        if (IsDimensionReturnMonster(p, i) && (best < 0 || atk > bestAtk)) {
            best = i;
            bestAtk = atk;
        }
    }
    return best;
}

static int CountDimensionReturns(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i, n = 0;

    for (i = 0; i < p->banishedCount; i++)
        n += IsDimensionReturnMonster(p, i);
    return n;
}

/* Special Summon the player's strongest such monster (the position is the player's pick), marking its zone for the
 * End Phase if asked. 0 with no zone or no monster. */
static int DimensionReturnOne(int player, int mark)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    struct DuelCard card;
    int zone = FindFreeMonsterZone(player);
    int i = StrongestDimensionReturn(player);

    if (zone < 0 || i < 0)
        return 0;
    card = p->banished[i];
    RemoveBanishedCardAt(player, i);
    if (mark) {
        gModDuel.dimensionReturn |= 1 << ((player & 1) * 5 + zone);
        gModDuel.dimensionReturnIds[(player & 1) * 5 + zone] = DUEL_CARD_ID(&card);
    }
    QueueSpecialSummonChoosePosition(player, &card, 1, 0);
    return 1;
}
#endif /* HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION || HAS_CARD_DIMENSION_FUSION */

#ifdef HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION
int EffectReturnFromTheDifferentDimensionPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand && CountFreeMonsterZones(card->player) > 0 && StrongestDimensionReturn(card->player) >= 0;
}

int EffectReturnFromTheDifferentDimensionChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int half = gDuelPlayers[link->player & 1].lifePoints / 2;

    if (half > 0)
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_LOSE_LP : DUEL_CMD_LOSE_LP, half, 1, 0);
    return 1;
}

int EffectReturnFromTheDifferentDimensionResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (link->negated)
        return EFFECT_STEP_DONE;
    /* EFFECT_STEP_START and every later call: one more monster, if possible; done once the last summon is */
    return DimensionReturnOne(link->player, 1) ? EFFECT_STEP_2 : EFFECT_STEP_DONE;
}

void EndPhase_ReturnFromTheDifferentDimension(void)
{
    int player, zone;

    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            int bit = player * 5 + zone;
            if ((gModDuel.dimensionReturn & (1 << bit))
                && DUEL_CARD_ID(&ZoneOf(player, zone)->card) == gModDuel.dimensionReturnIds[bit])
                BanishFieldCard(player, zone, 0);
        }
    }
    gModDuel.dimensionReturn = 0;
}
#endif /* HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION */

#ifdef HAS_CARD_DIMENSION_FUSION
/* ---- Dimension Fusion ----------------------------------------------------------------------------------------------
 * Normal Spell: "Pay 2000 Life Points. Both players Special Summon as many of their removed from play monsters as
 * possible."
 *   prepare  2000 LP, and a free zone and a banished monster (Return from the Different Dimension's kind) for either
 *            player
 *   chainA   2000 LP (DUEL_CMD_LOSE_LP, a cost)
 *   resolve  one monster per step, the strongest ATK first (DimensionReturnOne): the activating player's until none
 *            can come, then the opponent's; each player picks their own monsters' positions. */

static int CanDimensionFuse(int player)
{
    return CountFreeMonsterZones(player) > 0 && StrongestDimensionReturn(player) >= 0;
}

int EffectDimensionFusionPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return gDuelPlayers[card->player & 1].lifePoints >= 2000
           && (CanDimensionFuse(card->player) || CanDimensionFuse(1 - card->player));
}

int EffectDimensionFusionChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_LOSE_LP : DUEL_CMD_LOSE_LP, 2000, 1, 0);
    return 1;
}

int EffectDimensionFusionResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (link->negated)
        return EFFECT_STEP_DONE;
    if (DimensionReturnOne(link->player, 0) || DimensionReturnOne(1 - link->player, 0))
        return EFFECT_STEP_2;
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_DIMENSION_FUSION */

#ifdef HAS_CARD_SKULL_LAIR
/* ---- Skull Lair -------------------------------------------------------------------------------------------------
 * Continuous Trap: "Remove from play any number of Monster Cards in your Graveyard to destroy 1 face-up monster on the
 * field whose Level is equal to the number of the cards you removed from play."
 * Like Ultimate Offering it can be activated again while face up (card_command_menu.c, effect_activation.c,
 * effect_targets4.c list it), each time:
 *   check    a face-up monster (either side) the card may target, of Level 1 to the monsters in the player's
 *            Graveyard (it is the target filter of the pick too)
 *   prepare  such a monster exists
 *   chainB   the target: EffectTargetableFaceUpMonsterChainB; the CPU's pick (the human's strongest) is replaced by
 *            its best legal one (SkullLairBestTarget: the human's strongest such monster)
 *   resolve  the target still face up and still legal: as many Graveyard monsters as its Level are banished (the
 *            human picks each one in the Graveyard list, as Bazoo's; the CPU banishes its weakest), then it is
 *            destroyed. */

/* Monsters in the player's Graveyard. */
static int GraveMonsterCount(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i, n = 0;

    for (i = 0; i < p->graveCount; i++) {
        if (CARD_STATS_TYPE(gCardStats[DUEL_CARD_ID(&p->graveyard[i]) & CARD_ID_MASK]) <= CARD_TYPE_REPTILE)
            n++;
    }
    return n;
}

/* The Level of the face-up monster at (player, zone), 0 for none. */
static int FaceUpMonsterLevel(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    if (zone > ZONE_MONSTER_4 || id == 0 || !z->isFaceUp)
        return 0;
    return CARD_STATS_LEVEL(gCardStats[id & CARD_ID_MASK]);
}

int EffectSkullLairCheck(struct ChainEntry *card, u16 pos)
{
    int level;

    if (!EffectTargetableFaceUpMonsterCheck(card, pos))
        return 0;
    level = FaceUpMonsterLevel((u8)pos, pos >> 8);
    return level >= 1 && level <= GraveMonsterCount(card->player);
}

/* The CPU's target: the human's strongest legal monster, else -1. */
static int SkullLairBestTarget(struct ChainEntry *link)
{
    int zone, best = -1;
    u32 bestAtk = 0;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (EffectSkullLairCheck(link, zone << 8 | (1 - link->player))
            && (best < 0 || GetZoneCardAtk(1 - link->player, zone) > bestAtk)) {
            best = zone;
            bestAtk = GetZoneCardAtk(1 - link->player, zone);
        }
    }
    return best;
}

int EffectSkullLairPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    int player, zone;

    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (EffectSkullLairCheck(card, zone << 8 | player))
                return 1;
        }
    }
    return 0;
}

int EffectSkullLairChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int zone;

    if (link->player) {
        link->numTargets = 0;
        zone = SkullLairBestTarget(link);
        if (zone >= 0)
            TryAddEffectTarget(link, 1 - link->player, zone);
        return 1;
    }
    if (!EffectTargetableFaceUpMonsterChainB(link))
        return 0;
    if (link->numTargets == 1 && !EffectSkullLairCheck(link, link->targets[0])) {
        link->numTargets = 0;       /* a Level the Graveyard cannot pay: pick again */
        PlaySE(SE_ERROR);
        return 0;
    }
    return 1;
}

/* The CPU banishes its weakest Graveyard monster. */
static void BanishWeakestGraveMonster(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i, best = -1;
    u32 bestAtk = 0;

    for (i = 0; i < p->graveCount; i++) {
        u32 stats = gCardStats[DUEL_CARD_ID(&p->graveyard[i]) & CARD_ID_MASK];

        if (CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && (best < 0 || CARD_STATS_ATK(stats) < bestAtk)) {
            best = i;
            bestAtk = CARD_STATS_ATK(stats);
        }
    }
    if (best >= 0) {
        u16 *halves = (u16 *)&p->graveyard[best];

        DuelCmd_Push(player ? DUEL_CMD_PLAYER | DUEL_CMD_BANISH_GRAVEYARD_CARD : DUEL_CMD_BANISH_GRAVEYARD_CARD,
                     halves[0], halves[1], 0);
    }
}

int EffectSkullLairResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int player, zone, i;

    if (link->negated || link->numTargets != 1)
        return EFFECT_STEP_DONE;
    player = (u8)link->targets[0];
    zone = link->targets[0] >> 8;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (!EffectSkullLairCheck(link, link->targets[0]))
            return EFFECT_STEP_DONE;
        gChain.effectSubStep = FaceUpMonsterLevel(player, zone);    /* monsters still to banish */
        if (link->player) {
            for (i = 0; i < gChain.effectSubStep; i++)
                BanishWeakestGraveMonster(link->player);
            return EFFECT_STEP_5;
        }
        /* fall through */
    case EFFECT_STEP_2:
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, gStrSelectGraveCardToBanishForAtk);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        CardListView_Open(link->player, -1, CARD_SKULL_LAIR, 0);
        return EFFECT_STEP_4;
    case EFFECT_STEP_4: {
        u16 *picked = (u16 *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top];

        DuelCmd_Push((int)(gCardListView.cards[gCardListView.cursorRow + gCardListView.top] << 19) < 0
                         ? DUEL_CMD_PLAYER | DUEL_CMD_BANISH_GRAVEYARD_CARD
                         : DUEL_CMD_BANISH_GRAVEYARD_CARD,
                     picked[0], picked[1], 0);
        if (--gChain.effectSubStep > 0)
            return EFFECT_STEP_2;
        return EFFECT_STEP_5;
    }
    case EFFECT_STEP_5:
        if (FaceUpMonsterLevel(player, zone) != 0) {
            DestroyFieldCardByEffect(player, zone);
            OnCardDestroyedByEffect(link->player, player, zone);
        }
        break;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_SKULL_LAIR */

#ifdef HAS_CARD_FIBER_JAR
/* ---- Fiber Jar ---------------------------------------------------------------------------------------------------
 * FLIP: "Each player shuffles all cards from their hand, field, and Graveyard into the Deck, then draws 5 cards."
 * (card_detail.c HasFlipEffect lists it.) For each player, all queued (deferred) commands:
 *   field     ReturnFieldCardToDeck on every zone, the Spell/Trap and Field zones first (an equip goes back
 *             before its monster's DestroyLinkedCards could send it to the Graveyard; DuelCmd_ReturnToDeck skips
 *             an emptied zone). Tokens are destroyed, as ReturnFieldCardToDeck does.
 *   hand      DUEL_CMD_RETURN_HAND_CARD_TO_DECK, last card first (the indices stay valid)
 *   Graveyard its Fusion Monsters to the Fusion Deck at once (TakeGraveyardCardAt, AddCardToFusionDeck), then
 *             DUEL_CMD_RETURN_GRAVEYARD_TO_DECK (the rest, with anything the field sent there; it would put a
 *             Fusion Monster in the Deck). A field Fusion Monster goes to the Fusion Deck (DuelCmd_ReturnToDeck).
 * then each Deck is shuffled and each player draws 5 (DrawCards). Fiber Jar itself goes back with the field. */
int EffectFiberJarResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int p, zone, i;

    if (link->negated)
        return EFFECT_STEP_DONE;
    for (p = 0; p <= 1; p++) {
        u16 player = p ? DUEL_CMD_PLAYER : 0;

        for (zone = ZONE_SPELL_0; zone <= ZONE_FIELD; zone++)
            ReturnFieldCardToDeck(p, zone);
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++)
            ReturnFieldCardToDeck(p, zone);
        for (i = gDuelPlayers[p].handCount - 1; i >= 0; i--)
            DuelCmd_Push(player | DUEL_CMD_RETURN_HAND_CARD_TO_DECK, i, 1, 0);
        /* the Graveyard's Fusion Monsters to the Fusion Deck now (DUEL_CMD_RETURN_GRAVEYARD_TO_DECK would put them
         * in the Deck: AddCardToDeckTop) */
        for (i = gDuelPlayers[p].graveCount - 1; i >= 0; i--) {
            if (IsFusionMonster(DUEL_CARD_ID(&gDuelPlayers[p].graveyard[i]))) {
                struct DuelCard card;

                TakeGraveyardCardAt(p, i, &card);
                AddCardToFusionDeck(p, &card);
            }
        }
        DuelCmd_Push(player | DUEL_CMD_RETURN_GRAVEYARD_TO_DECK, 0, 0, 0);
    }
    for (p = 0; p <= 1; p++) {
        DuelCmd_Push(p ? DUEL_CMD_PLAYER | DUEL_CMD_SHUFFLE_DECK : DUEL_CMD_SHUFFLE_DECK, 0, 0, 0);
        DrawCards(p, 5);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_FIBER_JAR */

/* ---- The CPU's flip-effect targets in the Battle Phase -------------------------------------------------------------
 * The CPU's flip monsters that remove or stop one of the human's monsters (Man-Eater Bug, Hane-Hane, Penguin Soldier,
 * Dark-Eyes Illusionist, and the added Night Assailant and Tsukuyomi) usually pick the human's strongest monster. In
 * the human's Battle Phase (flipped by an attack) they prefer the strongest (ATK) of the human's Attack Position
 * monsters that have not attacked yet (gDuelPlayers[0].attackedMask): those can still attack this turn. The target
 * pickers (EffectTargetableMonsterChainB, EffectOpponentMonsterChainB, EffectPenguinSoldierChainB,
 * EffectTargetableFaceUpMonsterChainB) ask AiFlipEffectPreferredTarget / AiFlipEffectTarget. */
static int IsBattleFlipRemoval(const struct ChainEntry *link)
{
    if (gDuel.phase != PHASE_BATTLE || (gDuel.turnPlayer & 1) != 0)
        return 0;
    switch (CARD_NUMBER(link->card)) {
    case CARD_MAN_EATER_BUG:
    case CARD_HANE_HANE:
    case CARD_PENGUIN_SOLDIER:
    case CARD_DARK_EYES_ILLUSIONIST:
#ifdef HAS_CARD_NIGHT_ASSAILANT
    case CARD_NIGHT_ASSAILANT:
#endif
#ifdef HAS_CARD_TSUKUYOMI
    case CARD_TSUKUYOMI:
#endif
        return 1;
    }
    return 0;
}

/* The human's monster zone to prefer (see above), or -1 (not such a flip, or no monster that can still attack). */
int AiFlipEffectPreferredTarget(struct ChainEntry *link, int skipZone)
{
    int zone, best = -1, bestAtk = -1;

    if (!IsBattleFlipRemoval(link))
        return -1;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(0, zone);

        if (zone == skipZone || DUEL_CARD_ID(&z->card) == 0 || !z->isFaceUp || z->isDefense
            || (gDuelPlayers[0].attackedMask >> zone & 1))
            continue;
        if ((int)GetZoneCardAtk(0, zone) > bestAtk) {
            bestAtk = GetZoneCardAtk(0, zone);
            best = zone;
        }
    }
    return best;
}

/* The CPU's pick among the human's monsters: the preferred one, else the strongest (AiFindStrongestMonster). */
int AiFlipEffectTarget(struct ChainEntry *link, int skipZone)
{
    int zone = AiFlipEffectPreferredTarget(link, skipZone);

    return zone >= 0 ? zone : AiFindStrongestMonster(0, skipZone, 1, 1);
}

#ifdef HAS_CARD_4_STARRED_LADYBUG_OF_DOOM
/* ---- 4-Starred Ladybug of Doom ------------------------------------------------------------------------------------
 * FLIP: "Destroy all Level 4 monsters your opponent controls." A resolve-only row, as Fiber Jar (card_detail.c
 * HasFlipEffect lists it). Every face-up monster of the opponent (of Ladybug's controller) whose Level is 4 is
 * destroyed, as Tribe-Infecting Virus destroys (DestroyFieldCard, OnCardDestroyedByEffect); a face-down monster has
 * no Level, so it stays. */
#define LADYBUG_LEVEL 4

int EffectLadybugOfDoomResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int side = 1 - (link->player & 1), zone;

    if (link->negated)
        return EFFECT_STEP_DONE;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(side, zone);
        u16 id = DUEL_CARD_ID(&z->card);

        if (id != 0 && z->isFaceUp && (int)CARD_STATS_LEVEL(gCardStats[id & CARD_ID_MASK]) == LADYBUG_LEVEL) {
            DestroyFieldCard(side, zone, 1);
            OnCardDestroyedByEffect(link->player, side, zone);
        }
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_4_STARRED_LADYBUG_OF_DOOM */

#ifdef HAS_CARD_DES_KOALA
/* ---- Des Koala (YGOPRODeck id 69579761) ---------------------------------------------------------------------------
 * Effect text (YGOPRODeck API): "FLIP: Inflict 400 damage to your opponent for each card in their hand."
 * Resolve-only row (HasFlipEffect, card_detail.c). */

#define DES_KOALA_DAMAGE_PER_HAND_CARD 400

int EffectDesKoalaResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated) {
        int opponent = 1 - (link->player & 1);
        int damage = gDuelPlayers[opponent].handCount * DES_KOALA_DAMAGE_PER_HAND_CARD;

        if (damage > 0)
            LoseLifePoints(opponent, damage);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_DES_KOALA */

#ifdef HAS_CARD_ROULETTE_BARREL
/* ---- Roulette Barrel (YGOPRODeck id 46303688) ---------------------------------------------------------------------
 * Effect text (YGOPRODeck API): "Once per turn: You can roll a six-sided die twice, choose 1 result, and destroy 1
 * monster on the field whose Level is equal to that result." The card menu's ACTIVATE in the Main Phase
 * (CanActivateMonsterEffect), once per turn (effectUnused). ChainA rolls twice (DUEL_CMD_ROLL_PLAIN_DIE), the human
 * picks a result; the chosen level is kept in gChain.effectSubStep for ChainB and resolve. */

static const u8 sStrRouletteBarrelMenu[] = "Choose a die result:\n    @3First roll\n    @2Second roll";

static int RouletteBarrelInZone(int player, int zone)
{
    u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);

    if (zone > ZONE_MONSTER_4 || id == 0)
        return 0;
    return ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_ROULETTE_BARREL;
}

static int RouletteHasFaceUpMonsterOfLevel(int level)
{
    int player, zone, lv;

    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            lv = FaceUpMonsterLevel(player, zone);
            if (lv == level)
                return 1;
        }
    }
    return 0;
}

static int AiRouletteChooseRoll(int roll1, int roll2)
{
    int rolls[2] = { roll1, roll2 };
    int i, bestRoll = roll1;
    u32 bestAtk = 0;
    int found = 0;

    for (i = 0; i < 2; i++) {
        int level = rolls[i], zone;

        if (!RouletteHasFaceUpMonsterOfLevel(level))
            continue;
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (FaceUpMonsterLevel(0, zone) != level)
                continue;
            found = 1;
            if ((u32)GetZoneCardAtk(0, zone) >= bestAtk) {
                bestAtk = GetZoneCardAtk(0, zone);
                bestRoll = level;
            }
        }
    }
    if (found)
        return bestRoll;
    for (i = 0; i < 2; i++) {
        if (RouletteHasFaceUpMonsterOfLevel(rolls[i]))
            return rolls[i];
    }
    return roll1;
}

static int RouletteBarrelCanActivateNow(int player, int zone)
{
    struct DuelZone *z;
    int level;

    if (zone > ZONE_MONSTER_4 || !RouletteBarrelInZone(player, zone))
        return 0;
    z = ZoneOf(player, zone);
    if (!z->isFaceUp || !z->effectUnused)
        return 0;
    for (level = 1; level <= 6; level++) {
        if (RouletteHasFaceUpMonsterOfLevel(level))
            return 1;
    }
    return 0;
}

int RouletteBarrelCanActivateInZone(int player, int zone)
{
    return RouletteBarrelCanActivateNow(player, zone);
}

int EffectRouletteBarrelPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (fromHand || card->event != RESPONSE_NONE)
        return 0;
    return RouletteBarrelCanActivateNow(card->player, card->zone);
}

u16 EffectRouletteBarrelCheck(struct ChainEntry *card, u16 pos)
{
    int player = (u8)pos;
    int zone = pos >> 8;
    int level;

    if (!EffectTargetableFaceUpMonsterCheck(card, pos))
        return 0;
    level = FaceUpMonsterLevel(player, zone);
    if (level < 1)
        return 0;
    if (gChain.effectSubStep >= 1 && gChain.effectSubStep <= 6)
        return level == gChain.effectSubStep;
    /* CanActivateEffect (before the die is chosen): any face-up Level 1-6 monster is a legal outcome. */
    return level <= 6;
}

int EffectRouletteBarrelChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (link->player) {
        gModDuel.rouletteDie1 = (u8)(Random() % 6 + 1);
        gModDuel.rouletteDie2 = (u8)(Random() % 6 + 1);
        gChain.effectSubStep = AiRouletteChooseRoll(gModDuel.rouletteDie1, gModDuel.rouletteDie2);
        return 1;
    }
    switch (gChain.costStep) {
    case 0:
        gModDuel.rouletteDie1 = (u8)(Random() % 6 + 1);
        gModDuel.rouletteDie2 = (u8)(Random() % 6 + 1);
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_ROLL_PLAIN_DIE : DUEL_CMD_ROLL_PLAIN_DIE,
                     gModDuel.rouletteDie1, 0, 0);
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_OPEN_DUEL_SCREEN : DUEL_CMD_OPEN_DUEL_SCREEN, 0, 0, 0);
        gChain.costStep++;
        return 0;
    case 1:
        if (gDuelCmd.running)
            return 0;
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_ROLL_PLAIN_DIE : DUEL_CMD_ROLL_PLAIN_DIE,
                     gModDuel.rouletteDie2, 0, 0);
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_OPEN_DUEL_SCREEN : DUEL_CMD_OPEN_DUEL_SCREEN, 0, 0, 0);
        gChain.costStep++;
        return 0;
    case 2:
        if (gDuelCmd.running)
            return 0;
        if (!RouletteHasFaceUpMonsterOfLevel(gModDuel.rouletteDie1)
            && !RouletteHasFaceUpMonsterOfLevel(gModDuel.rouletteDie2)) {
            gChain.effectSubStep = gModDuel.rouletteDie1;
            DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_OPEN_DUEL_SCREEN : DUEL_CMD_OPEN_DUEL_SCREEN,
                         0, 0, 0);
            gChain.costStep = 5;
            return 0;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(19, 6), TEXTBOX_FLAGS_DEFAULT, sStrRouletteBarrelMenu);
        TextBoxSetMenu(TEXTBOX_MENU_TWO_CHOICE, NULL, NULL);
        gChain.costStep++;
        return 0;
    case 5:
        if (gDuelCmd.running)
            return 0;
        return 1;
    default:
        if (gTextBox.active)
            return 0;
        gChain.effectSubStep = gTextBox.result ? gModDuel.rouletteDie2 : gModDuel.rouletteDie1;
        if (gChain.costStep == 4) {
            if (gDuelCmd.running)
                return 0;
            return 1;
        }
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_OPEN_DUEL_SCREEN : DUEL_CMD_OPEN_DUEL_SCREEN, 0, 0, 0);
        gChain.costStep = 4;
        return 0;
    }
}

int EffectRouletteBarrelChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int zone, best = -1;
    u32 bestAtk = 0;

    link->numTargets = 0;
    if (!RouletteHasFaceUpMonsterOfLevel(gChain.effectSubStep))
        return 1;

    if (link->player) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (EffectRouletteBarrelCheck(link, zone << 8 | (1 - link->player))
                && (best < 0 || GetZoneCardAtk(1 - link->player, zone) > bestAtk)) {
                best = zone;
                bestAtk = GetZoneCardAtk(1 - link->player, zone);
            }
        }
        if (best < 0) {
            for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
                if (EffectRouletteBarrelCheck(link, zone << 8 | link->player))
                    TryAddEffectTarget(link, link->player, zone);
            }
        } else
            TryAddEffectTarget(link, 1 - link->player, best);
        return 1;
    }
    if (!EffectTargetableFaceUpMonsterChainB(link))
        return 0;
    if (link->numTargets == 1 && !EffectRouletteBarrelCheck(link, link->targets[0])) {
        link->numTargets = 0;
        PlaySE(SE_ERROR);
        return 0;
    }
    return 1;
}

int EffectRouletteBarrelResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated && link->numTargets >= 1 && EffectRouletteBarrelCheck(link, link->targets[0])) {
        int targetPlayer = (u8)link->targets[0];
        int targetZone = link->targets[0] >> 8;

        DestroyFieldCard(targetPlayer, targetZone, 1);
        OnCardDestroyedByEffect(link->player, targetPlayer, targetZone);
    }
    if (!link->negated)
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_EFFECT_UNUSED : DUEL_CMD_SET_EFFECT_UNUSED,
                     link->zone, 0, 0);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_ROULETTE_BARREL */

#if defined(HAS_CARD_YATA_GARASU) || defined(HAS_CARD_ASURA_PRIEST)
/* The Spirit monsters that cannot be Special Summoned: Yata-Garasu and Asura Priest (2026-10-30). Each goes back to its
 * owner's hand in the End Phase of the turn it was Normal Summoned or flipped face up; since it always leaves then and
 * cannot be Special Summoned, every face-up one at an End Phase is such a one (EndPhase_ReturnSpiritMonsters). */
static int IsReturningSpirit(u16 id)
{
    u16 number = CARD_NUMBER(id);

#ifdef HAS_CARD_YATA_GARASU
    if (number == CARD_YATA_GARASU)
        return 1;
#endif
#ifdef HAS_CARD_ASURA_PRIEST
    if (number == CARD_ASURA_PRIEST)
        return 1;
#endif
    return 0;
}

/* DuelPhase_End (battle_phase3.c): every face-up Yata-Garasu and Asura Priest goes back to its owner's hand. */
void EndPhase_ReturnSpiritMonsters(void)
{
    int player, zone;

    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            struct DuelZone *z = ZoneOf(player, zone);
            u16 id = DUEL_CARD_ID(&z->card);

            if (id != 0 && z->isFaceUp && IsReturningSpirit(id)) {
                DuelCmd_Push(player ? DUEL_CMD_PLAYER | DUEL_CMD_SHOW_CARD_EFFECT : DUEL_CMD_SHOW_CARD_EFFECT, id, 1, 0);
                ReturnFieldCardToHand(player, zone, 0);
            }
        }
    }
}
#endif /* HAS_CARD_YATA_GARASU || HAS_CARD_ASURA_PRIEST */

#ifdef HAS_CARD_YATA_GARASU
/* ---- Yata-Garasu ---------------------------------------------------------------------------------------------------
 * "This card cannot be Special Summoned. This card returns to its owner's hand during the End Phase of the turn it is
 * Normal Summoned or flipped face-up. When this card inflicts Battle Damage to your opponent, they skip their next Draw
 * Phase."
 *   IsCannotSpecialSummon lists it (QueueSpecialSummon* refuse it; CanReviveGraveyardCard's IsSpecialSummonOnly path
 *   does not apply, but the summon itself is refused).
 *   EndPhase_ReturnSpiritMonsters (DuelPhase_End, battle_phase3.c; above): every face-up Yata-Garasu on the field goes
 *   back to its owner's hand. A face-up one at an End Phase was Normal Summoned or flipped face up that turn (it always
 *   leaves at the End Phase, and it cannot be Special Summoned), so no record of the turn is needed.
 *   Battle damage (BattleStage_InflictDamage steps 0 and 2, battle_phase2.c): DUEL_CMD_SKIP_NEXT_DRAW_PHASE (Time
 *   Seal's command) for the damaged player. */
/* Battle damage by Yata-Garasu (owned by `player`): the damaged player skips their next Draw Phase. */
void YataGarasuDealtDamage(int player, u16 id)
{
    int opponent = 1 - (player & 1);

    ShowCardEffect(player, id);
    DuelCmd_Push(opponent ? DUEL_CMD_PLAYER | DUEL_CMD_SKIP_NEXT_DRAW_PHASE : DUEL_CMD_SKIP_NEXT_DRAW_PHASE, 0, 0, 0);
}
#endif /* HAS_CARD_YATA_GARASU */

#ifdef HAS_CARD_MIND_CONTROL
/* ---- Mind Control --------------------------------------------------------------------------------------------------
 * Normal Spell: "Target 1 monster your opponent controls; take control of it until the End Phase, but it cannot
 * declare an attack or be Tributed."
 * The row is Change of Heart's: EffectChangeOfHeartPrepare, EffectOpponentMonsterCheck, EffectTakeControlChainB and
 * EffectChangeOfHeartResolve, which moves the monster to a free zone of the player and gives it a
 * ZONE_LINK_CARD_EFFECT link from the activating card, here Mind Control. That link is what the rest keys on:
 *   DuelCmd_TurnEnd (duel_cmd_turn.c)   ReturnControlAtTurnEnd(player, zone, CARD_MIND_CONTROL), as for Change of Heart
 *   CanMonsterAttack (card_menu_input.c) and IsTributableMonster (duel_zones.c)   IsMindControlled refuses it. */
int IsMindControlled(int player, int zone)
{
    return CountZoneLinksFromCard(player, zone, CARD_MIND_CONTROL) > 0;
}
#endif /* HAS_CARD_MIND_CONTROL */

#ifdef HAS_CARD_LIGHTNING_VORTEX
/* ---- Lightning Vortex ----------------------------------------------------------------------------------------------
 * Normal Spell: "Discard 1 card; destroy all face-up monsters your opponent controls."
 * Prepare: a card to discard (2 in hand when played from the hand) and a face-up monster on the opponent's side.
 * ChainA: the discard cost of Magic Jammer / Tribe-Infecting Virus (DuelPrompt_PostDiscardCost). Resolve: no target;
 * every face-up monster the opponent controls then is destroyed (DestroyFieldCard + OnCardDestroyedByEffect). */

static int CountOpponentFaceUpMonsters(int player)
{
    int zone, n = 0;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(1 - player, zone);
        if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp)
            n++;
    }
    return n;
}

int EffectLightningVortexPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return gDuelPlayers[card->player & 1].handCount >= 1 + fromHand && CountOpponentFaceUpMonsters(card->player) > 0;
}

int EffectLightningVortexChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelPrompt_PostDiscardCost(link->player, 1, 0, 0);
    return 1;
}

int EffectLightningVortexResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent = 1 - link->player;
    int zone;

    if (link->negated)
        return EFFECT_STEP_DONE;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(opponent, zone);
        if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp) {
            DestroyFieldCard(opponent, zone, 1);
            OnCardDestroyedByEffect(link->player, opponent, zone);
        }
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_LIGHTNING_VORTEX */

#ifdef HAS_CARD_DON_ZALOOG
/* ---- Don Zaloog ----------------------------------------------------------------------------------------------------
 * "When this card inflicts battle damage to your opponent: You can activate 1 of these effects.
 *  - Discard 1 random card from your opponent's hand.
 *  - Send the top 2 cards of your opponent's Deck to the GY."
 * BattleStage_InflictDamage (battle_phase2.c) queues its battle-damage trigger like Kycoo's: as the attacker
 * (RESPONSE_BATTLE_DAMAGE) and as the target when the attacker's player takes the damage
 * (RESPONSE_BATTLE_DEFLECTED_DAMAGE). The low nibble of loc1 is the damaged player. Pending triggers are not checked
 * by a prepare handler, so the resolve checks everything and asks: the human gets Yes/No for the discard, then for
 * the Deck; the CPU discards if it can, else sends the Deck cards. The discard is White Magical Hat's
 * (DuelPrompt_PostRandomDiscard, by an opponent's effect); the Deck cards go one by one
 * (DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD, the top card is deck[0]). */

static const u8 sStrDonZaloogDiscard[] = "Use Don Zaloog's effect to discard 1 random card from the opponent's hand?";
static const u8 sStrDonZaloogMill[] = "Use Don Zaloog's effect to send the top 2 cards of the opponent's Deck to the "
                                      "Graveyard?";

static void DonZaloogMill(int opponent)
{
    struct DuelPlayer *o = &gDuelPlayers[opponent & 1];
    u16 cmd = opponent ? DUEL_CMD_PLAYER | DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD : DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD;
    int i;

    for (i = 0; i < 2 && i < o->deckCount; i++)
        DuelCmd_Push(cmd, ((u16 *)&o->deck[i])[0], ((u16 *)&o->deck[i])[1], 0);
}

int EffectDonZaloogResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent = 1 - link->player;
    struct DuelPlayer *o = &gDuelPlayers[opponent & 1];

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if ((link->event != RESPONSE_BATTLE_DAMAGE && link->event != RESPONSE_BATTLE_DEFLECTED_DAMAGE)
            || (link->loc1 & 0xF) == link->player)
            return EFFECT_STEP_DONE;
        if (link->player) {
            ShowCardEffect(link->player, link->card & CARD_ID_MASK);
            if (o->handCount != 0)
                DuelPrompt_PostRandomDiscard(opponent, 1, 1);
            else
                DonZaloogMill(opponent);
            return EFFECT_STEP_DONE;
        }
        if (o->handCount != 0) {
            TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, sStrDonZaloogDiscard);
            TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
            return EFFECT_STEP_2;
        }
        goto ask_mill;
    case EFFECT_STEP_2:
        if (gTextBox.result) {
            DuelPrompt_PostRandomDiscard(opponent, 1, 1);
            return EFFECT_STEP_DONE;
        }
    ask_mill:
        if (o->deckCount == 0)
            return EFFECT_STEP_DONE;
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, sStrDonZaloogMill);
        TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        if (gTextBox.result)
            DonZaloogMill(opponent);
        return EFFECT_STEP_DONE;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_DON_ZALOOG */

#ifdef HAS_CARD_DARK_SCORPION_CLIFF_THE_TRAP_REMOVER
void AiPickSpellTrapToDestroy(struct ChainEntry *link); /* below: Mystical Space Typhoon CPU target */

/* ---- Dark Scorpion - Cliff the Trap Remover (YGOPRODeck id 06967870) ----------------------------------------------
 * "When this card inflicts Battle Damage to your opponent, you can activate 1 of these effects:
 *  ● Target 1 Spell/Trap Card on the field; destroy that target.
 *  ● Send the top 2 cards of their Deck to the Graveyard."
 * Queued from BattleStage_InflictDamage (battle_phase2.c) like Don Zaloog (RESPONSE_BATTLE_DAMAGE /
 * RESPONSE_BATTLE_DEFLECTED_DAMAGE). The human picks one effect from a two-line menu when both are possible; the CPU
 * destroys an opponent Spell/Trap when one exists, otherwise mills 2. */

static const u8 sStrCliffMenu[] = "Choose an effect:\n    @3Destroy 1 Spell/Trap\n    @2Mill 2 from their Deck";
static const u8 sStrCliffDestroy[] = "Select 1 Spell/Trap Card to destroy.";
static const u8 sStrCliffMill[] = "Use Cliff's effect to send the top 2 cards of the opponent's Deck to the "
                                  "Graveyard?";

#define CLIFF_CHOICE_DESTROY 0
#define CLIFF_CHOICE_MILL 1

static int CliffBattleDamageToOpponent(struct ChainEntry *link)
{
    if (link->event != RESPONSE_BATTLE_DAMAGE && link->event != RESPONSE_BATTLE_DEFLECTED_DAMAGE)
        return 0;
    return (link->loc1 & 0xF) != link->player;
}

static int CliffAnySpellTrapOnField(void)
{
    int player, zone;

    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_SPELL_0; zone <= ZONE_FIELD; zone++) {
            if (DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0)
                return 1;
        }
    }
    return 0;
}

static void CliffMillDeckTop(int opponent)
{
    struct DuelPlayer *o = &gDuelPlayers[opponent & 1];
    u16 cmd = opponent ? DUEL_CMD_PLAYER | DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD : DUEL_CMD_SEND_DECK_CARD_TO_GRAVEYARD;
    int i;

    for (i = 0; i < 2 && i < o->deckCount; i++)
        DuelCmd_Push(cmd, ((u16 *)&o->deck[i])[0], ((u16 *)&o->deck[i])[1], 0);
}

static int CliffDestroyAt(struct ChainEntry *link, int targetPlayer, int zone)
{
    if (zone > ZONE_FIELD || DUEL_CARD_ID(&ZoneOf(targetPlayer, zone)->card) == 0)
        return 0;
    if (!CanCardTargetZone(link->card, targetPlayer, zone))
        return 0;
    DestroyFieldCard(targetPlayer, zone, 1);
    OnCardDestroyedByEffect(link->player, targetPlayer, zone);
    return 1;
}

int EffectDarkScorpionCliffResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent = 1 - link->player;
    struct DuelPlayer *o = &gDuelPlayers[opponent & 1];
    int choice;

    if (link->negated || !CliffBattleDamageToOpponent(link))
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        ShowCardEffect(link->player, link->card & CARD_ID_MASK);
        if (link->player) {
            if (CliffAnySpellTrapOnField()) {
                link->numTargets = 0;
                AiPickSpellTrapToDestroy(link);
                if (link->numTargets >= 1) {
                    int targetPlayer = (u8)link->targets[0];
                    int targetZone = link->targets[0] >> 8;

                    CliffDestroyAt(link, targetPlayer, targetZone);
                }
            } else
                CliffMillDeckTop(opponent);
            return EFFECT_STEP_DONE;
        }
        if (!CliffAnySpellTrapOnField() && o->deckCount == 0)
            return EFFECT_STEP_DONE;
        if (!CliffAnySpellTrapOnField()) {
            TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, sStrCliffMill);
            TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
            return EFFECT_STEP_4;
        }
        if (o->deckCount == 0) {
            TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, sStrCliffDestroy);
            DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_OPEN_DUEL_SCREEN : DUEL_CMD_OPEN_DUEL_SCREEN,
                         0, 0, 0);
            return EFFECT_STEP_3;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, sStrCliffMenu);
        TextBoxSetMenu(TEXTBOX_MENU_TWO_CHOICE, NULL, NULL);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        choice = gTextBox.result ? CLIFF_CHOICE_MILL : CLIFF_CHOICE_DESTROY;
        gChain.effectSubStep = choice;
        if (choice == CLIFF_CHOICE_MILL) {
            CliffMillDeckTop(opponent);
            return EFFECT_STEP_DONE;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, sStrCliffDestroy);
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_OPEN_DUEL_SCREEN : DUEL_CMD_OPEN_DUEL_SCREEN, 0, 0,
                     0);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        if (gDuelCmd.running || gTextBox.active)
            return EFFECT_STEP_3;
        if (gMain.newKeys & B_BUTTON)
            return EFFECT_STEP_DONE;
        if (DuelCursor_PickTarget(PICK_BOTH_SIDES(PICK_ANY_SPELL_TRAP)) == 0)
            return EFFECT_STEP_3;
        if (CliffDestroyAt(link, gDuelScreen.selPlayer, gDuelScreen.selArea + gDuelScreen.selIndex))
            return EFFECT_STEP_DONE;
        PlaySE(SE_ERROR);
        return EFFECT_STEP_3;
    case EFFECT_STEP_4:
        if (gTextBox.result)
            CliffMillDeckTop(opponent);
        return EFFECT_STEP_DONE;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_DARK_SCORPION_CLIFF_THE_TRAP_REMOVER */

#ifdef HAS_CARD_CREATURE_SWAP
/* ---- Creature Swap -------------------------------------------------------------------------------------------------
 * Normal Spell: "Each player chooses 1 monster they control and switches control of those monsters with each other.
 * Those monsters cannot change their battle positions for the rest of this turn."
 * Nothing is targeted: the choices are made at resolution. The human picks one of their monsters with the cursor
 * (as in EffectGainOpponentMonsterStatsResolve), the CPU gives its weakest (AiFindWeakestMonster). The swap is
 * Invader of the Throne's (SwapFieldCards: DUEL_CMD_SWAP_ZONES and the control-change rules of the swapped cards),
 * then both monsters are locked in their position (DUEL_CMD_SET_POSITION_LOCKED; DuelCmd_TurnStart clears the flag at
 * the start of their controller's next turn). */

static const u8 sStrCreatureSwapPick[] = "Select one of your monsters to give to the opponent.";

int EffectCreatureSwapPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return CountMonsters(0) > 0 && CountMonsters(1) > 0;
}

int EffectCreatureSwapResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int humanZone, cpuZone;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (CountMonsters(0) == 0 || CountMonsters(1) == 0)
            return EFFECT_STEP_DONE;
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, sStrCreatureSwapPick);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        if (DuelCursor_PickTarget(PICK_ANY_MONSTER) == 0)
            return EFFECT_STEP_2;
        humanZone = gDuelScreen.selArea + gDuelScreen.selIndex;
        if (gDuelScreen.selPlayer != 0 || humanZone > ZONE_MONSTER_4 || DUEL_CARD_ID(&ZoneOf(0, humanZone)->card) == 0) {
            PlaySE(SE_ERROR);
            return EFFECT_STEP_2;
        }
        cpuZone = AiFindWeakestMonster(1, -1, 1, 1);
        if (cpuZone < 0 || DUEL_CARD_ID(&ZoneOf(1, cpuZone)->card) == 0)
            return EFFECT_STEP_DONE;
        SwapFieldCards(link->player, DUEL_LOC(0, humanZone), DUEL_LOC(1, cpuZone));
        /* after the swap the zones hold the other player's monster */
        DuelCmd_Push(DUEL_CMD_SET_POSITION_LOCKED, humanZone, 1, 0);
        DuelCmd_Push(DUEL_CMD_PLAYER | DUEL_CMD_SET_POSITION_LOCKED, cpuZone, 1, 0);
        return EFFECT_STEP_DONE;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_CREATURE_SWAP */

#ifdef HAS_CARD_INJECTION_FAIRY_LILY
/* ---- Injection Fairy Lily ------------------------------------------------------------------------------------------
 * "During either player's Damage Step, when this card attacks or is attacked: You can pay 2000 LP; this card gains
 * 3000 ATK during the Damage Step only."
 * EDS offers no face-up monster in a response window (only Set Spell/Trap cards and the hand), so, like D.D. Warrior
 * Lady, the effect lives in the battle code:
 *   BattleStage_DamageCalc, DAMAGE_STEP_MARK_ATTACKER   InjectionFairyLilyResetBattle
 *   BattleStage_DamageCalc, DAMAGE_STEP_CALC            InjectionFairyLilyDamageStep, before CalcBattle: the
 *                                                       attacker, then the target, when it is a face-up Lily whose
 *                                                       controller has more than 2000 LP. The human is asked
 *                                                       (PROMPT_CONFIRM_CARD_EFFECT); the CPU pays when the 3000
 *                                                       ATK turn the battle (or for a direct attack, with more than
 *                                                       4000 LP). Paying queues DUEL_CMD_LOSE_LP (a cost) and a
 *                                                       ZONE_LINK_ATK_BONUS link of 3000; the stage waits a frame
 *                                                       so that CalcBattle sees it.
 *   BattleStage_EndAttack                               InjectionFairyLilyEndAttack: the 3000 link goes again.
 * The state is gInjectionFairyLily (EWRAM 0x0202FF00, symbols.ld). */

const u8 gStrInjectionFairyLilyPrompt[] = "Pay 2000 LP to give Injection Fairy Lily 3000 more ATK?";

#define LILY_COST   2000
#define LILY_BONUS  3000

enum InjectionFairyLilyStep {
    LILY_STEP_ATTACKER = 0,
    LILY_STEP_ATTACKER_ASKED = 1,
    LILY_STEP_TARGET = 2,
    LILY_STEP_TARGET_ASKED = 3,
    LILY_STEP_DONE = 4,
};

struct InjectionFairyLilyState {
    u8 step;        /* enum InjectionFairyLilyStep */
    u8 boosted;     /* bit 0: the attacker has the 3000 link, bit 1: the target */
};
extern struct InjectionFairyLilyState gInjectionFairyLily;

/* The monster of the battle on `side` (0 the attacker, 1 the target) of the attack of `attacker`. */
static int BattleMonster(int attacker, int side, int *who, int *zone)
{
    if (side == 0) {
        *who = attacker;
        *zone = gBattle.atkSlot;
        return 1;
    }
    if (gBattle.direct)
        return 0;
    *who = 1 - attacker;
    *zone = gBattle.defSlot;
    return 1;
}

/* 1 if that monster is a face-up Injection Fairy Lily whose controller can pay. */
static int CanUseLily(int attacker, int side, int *who, int *zone)
{
    struct DuelZone *z;
    u16 id;

    if (!BattleMonster(attacker, side, who, zone))
        return 0;
    z = ZoneOf(*who, *zone);
    id = DUEL_CARD_ID(&z->card);
    return id != 0 && z->isFaceUp
           && ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK] == CARD_INJECTION_FAIRY_LILY
           && gDuelPlayers[*who & 1].lifePoints > LILY_COST;
}

/* The CPU pays when the 3000 ATK turn the battle (a direct attack: with LP to spare). */
static int CpuWantsLily(int attacker, int side, int who, int zone)
{
    int otherWho, otherZone;
    u32 atk = GetZoneCardAtk(who, zone);
    u32 value;

    if (!BattleMonster(attacker, 1 - side, &otherWho, &otherZone))
        return gDuelPlayers[who & 1].lifePoints > 2 * LILY_COST;
    if (DUEL_CARD_ID(&ZoneOf(otherWho, otherZone)->card) == 0)
        return 0;
    value = ZoneOf(otherWho, otherZone)->isDefense ? GetZoneCardDef(otherWho, otherZone)
                                                   : GetZoneCardAtk(otherWho, otherZone);
    return atk <= value && atk + LILY_BONUS > value;
}

static void ActivateLily(int side, int who, int zone)
{
    ShowCardEffect(who, DUEL_CARD_ID(&ZoneOf(who, zone)->card));
    DuelCmd_Push(who ? DUEL_CMD_PLAYER | DUEL_CMD_LOSE_LP : DUEL_CMD_LOSE_LP, LILY_COST, 1, 0);
    QueueAddZoneLink(who, LILY_BONUS, DUEL_LOC(who, zone), ZONE_LINK_ATK_BONUS);
    gInjectionFairyLily.boosted |= 1 << side;
}

void InjectionFairyLilyResetBattle(void)
{
    gInjectionFairyLily.step = LILY_STEP_ATTACKER;
    gInjectionFairyLily.boosted = 0;
}

/* Before CalcBattle; 1 while it waits (a prompt, or the commands of a payment), 0 when CalcBattle may run. */
int InjectionFairyLilyDamageStep(int player)
{
    int who, zone, side;

    for (;;) {
        switch (gInjectionFairyLily.step) {
        case LILY_STEP_ATTACKER:
        case LILY_STEP_TARGET:
            side = gInjectionFairyLily.step == LILY_STEP_TARGET;
            if (!CanUseLily(player, side, &who, &zone)) {
                gInjectionFairyLily.step += 2;
                break;
            }
            if (who != 0) {
                gInjectionFairyLily.step += 2;
                if (CpuWantsLily(player, side, who, zone)) {
                    ActivateLily(side, who, zone);
                    return 1;
                }
                break;
            }
            DuelPrompt_Post(0, PROMPT_CONFIRM_CARD_EFFECT, CARD_INJECTION_FAIRY_LILY, 0);
            gInjectionFairyLily.step++;
            return 1;
        case LILY_STEP_ATTACKER_ASKED:
        case LILY_STEP_TARGET_ASKED:
            side = gInjectionFairyLily.step == LILY_STEP_TARGET_ASKED;
            gInjectionFairyLily.step++;
            if (gDuel.promptResult && CanUseLily(player, side, &who, &zone)) {
                ActivateLily(side, who, zone);
                return 1;
            }
            break;
        default:
            return 0;
        }
    }
}

/* After the battle: the 3000 ATK of the Damage Step go again. */
void InjectionFairyLilyEndAttack(int player)
{
    int side, who, zone, i;

    for (side = 0; side <= 1; side++) {
        if (!(gInjectionFairyLily.boosted & (1 << side)) || !BattleMonster(player, side, &who, &zone))
            continue;
        for (i = 0; i < ZoneOf(who, zone)->numLinks; i++) {
            struct DuelZone *z = ZoneOf(who, zone);
            if ((u8)z->linkKinds[i] == ZONE_LINK_ATK_BONUS && z->links[i] == LILY_BONUS) {
                RemoveZoneLinkAt(who, zone, i);
                break;
            }
        }
    }
    InjectionFairyLilyResetBattle();
    gInjectionFairyLily.step = LILY_STEP_DONE;
}
#endif /* HAS_CARD_INJECTION_FAIRY_LILY */

#ifdef HAS_CARD_EXARION_UNIVERSE
/* ---- Exarion Universe ----------------------------------------------------------------------------------------------
 * (YGOPRODeck) "During your Battle Step, if this card attacks a Defense Position monster: You can make this card lose
 * exactly 400 ATK, and if it does, it will inflict piercing battle damage. These effects last until the end of this
 * turn."
 * Like Injection Fairy Lily, the optional effect is handled in the battle code (no face-up monster response window):
 *   BattleStage_DamageCalc, DAMAGE_STEP_MARK_ATTACKER   ExarionUniverseResetBattle
 *   BattleStage_DamageCalc, DAMAGE_STEP_CALC            ExarionUniverseDamageStep before CalcBattle: the attacking
 *                                                       Exarion vs a Defense Position monster, ATK at least 400. The
 *                                                       human is asked (PROMPT_CONFIRM_CARD_EFFECT); the CPU pays when
 *                                                       its ATK exceeds the DEF (piercing LP damage). Paying queues a
 *                                                       gModDuel.exarionPenaltyStacks (ModStatBonus: -400 ATK each,
 *                                                       cleared when that player's turn ends) and sets exarionPierce
 *                                                       for ModHasPiercing.
 *   DuelCmd_TurnEnd                                       ExarionUniverseClearTurnPenalty for the turn player.
 *   ModEndAttack                                        clears exarionPierce for the next attack. */

#define EXARION_ATK_COST 400

const u8 gStrExarionUniversePrompt[] =
    "Make Exarion Universe lose 400 ATK to inflict piercing battle damage?";

enum ExarionUniverseStep {
    EXARION_STEP_ASK = 0,
    EXARION_STEP_ASKED = 1,
    EXARION_STEP_DONE = 2,
};

static int AttackingExarion(int player, int *zone)
{
    struct DuelZone *z;

    *zone = gBattle.atkSlot;
    z = ZoneOf(player, *zone);
    if (DUEL_CARD_ID(&z->card) == 0 || !z->isFaceUp || z->isDefense)
        return 0;
    return ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[DUEL_CARD_ID(&z->card) & CARD_ID_MASK] == CARD_EXARION_UNIVERSE;
}

static int ExarionCanActivate(int player, int atkZone)
{
    int defPlayer, defZone;
    struct DuelZone *def;

    if (gBattle.direct)
        return 0;
    defPlayer = 1 - player;
    defZone = gBattle.defSlot;
    def = ZoneOf(defPlayer, defZone);
    if (DUEL_CARD_ID(&def->card) == 0 || !def->isDefense)
        return 0;
    return GetZoneCardAtk(player, atkZone) >= EXARION_ATK_COST;
}

static int CpuWantsExarion(int player, int atkZone)
{
    int defPlayer = 1 - player;
    int defZone = gBattle.defSlot;
    u32 atk = GetZoneCardAtk(player, atkZone);
    u32 def = GetZoneCardDef(defPlayer, defZone);

    return atk >= EXARION_ATK_COST && atk - EXARION_ATK_COST > def;
}

static void ActivateExarion(int player, int zone)
{
    u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);
    int idx = (player & 1) * 5 + zone;

    ShowCardEffect(player, id);
    if (gModDuel.exarionPenaltyStacks[idx] < 255)
        gModDuel.exarionPenaltyStacks[idx]++;
    gModDuel.exarionPierce = 1;
}

void ExarionUniverseClearTurnPenalty(int turnPlayer)
{
    int base = (turnPlayer & 1) * 5;
    int z;

    for (z = 0; z < 5; z++)
        gModDuel.exarionPenaltyStacks[base + z] = 0;
}

void ExarionUniverseResetBattle(void)
{
    gModDuel.exarionAskStep = EXARION_STEP_ASK;
    gModDuel.exarionPierce = 0;
}

int ExarionUniverseDamageStep(int player)
{
    int zone;

    switch (gModDuel.exarionAskStep) {
    case EXARION_STEP_ASK:
        if (!AttackingExarion(player, &zone) || !ExarionCanActivate(player, zone)) {
            gModDuel.exarionAskStep = EXARION_STEP_DONE;
            return 0;
        }
        if (player != 0) {
            gModDuel.exarionAskStep = EXARION_STEP_DONE;
            if (CpuWantsExarion(player, zone))
                ActivateExarion(player, zone);
            return 1;
        }
        DuelPrompt_Post(0, PROMPT_CONFIRM_CARD_EFFECT, CARD_EXARION_UNIVERSE, 0);
        gModDuel.exarionAskStep = EXARION_STEP_ASKED;
        return 1;
    case EXARION_STEP_ASKED:
        gModDuel.exarionAskStep = EXARION_STEP_DONE;
        if (gDuel.promptResult && AttackingExarion(player, &zone) && ExarionCanActivate(player, zone))
            ActivateExarion(player, zone);
        return 1;
    default:
        return 0;
    }
}
#endif /* HAS_CARD_EXARION_UNIVERSE */

#ifdef HAS_CARD_PHOENIX_WING_WIND_BLAST
/* ---- Phoenix Wing Wind Blast ---------------------------------------------------------------------------------------
 * Normal Spell: "Discard 1 card, then target 1 card your opponent controls; place that target on the top of the Deck."
 * Cost: the discard of Magic Jammer / Tribe-Infecting Virus (DuelPrompt_PostDiscardCost). Target: any card in the
 * opponent's monster, Spell/Trap or Field zones that the card may target (CanCardTargetZone), as Raigeki Break but on
 * one side. The target step does not ask CanActivateEffect again: the discard is paid by then, and the prepare
 * handler counts the hand. Resolve: ReturnFieldCardToDeck (to the top of its owner's Deck; a Fusion Monster goes to
 * the Fusion Deck and a token is destroyed, the game's rules for DUEL_CMD_RETURN_TO_DECK). */

static const u8 sStrPhoenixWingPick[] = "Designate the opponent's card to place on top of the Deck.";

#define PHOENIX_WING_PICK (PICK_PLAYER1(PICK_ANY_MONSTER | PICK_ANY_SPELL_TRAP))

u16 EffectPhoenixWingWindBlastCheck(struct ChainEntry *card, u16 pos)
{
    int player = (u8)pos;
    int zone = pos >> 8;

    if (player == card->player || zone > ZONE_FIELD || !CanCardTargetZone(card->card, player, zone))
        return 0;
    return DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0;
}

int EffectPhoenixWingWindBlastPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    int zone;

    /* a card to discard besides this one when it is played from the hand */
    if (gDuelPlayers[card->player & 1].handCount < 1 + fromHand)
        return 0;
    for (zone = 0; zone <= ZONE_FIELD; zone++) {
        if (EffectPhoenixWingWindBlastCheck(card, DUEL_LOC(1 - card->player, zone)))
            return 1;
    }
    return 0;
}

int EffectPhoenixWingWindBlastChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelPrompt_PostDiscardCost(link->player, 1, 0, 0);
    return 1;
}

int EffectPhoenixWingWindBlastChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int opponent = 1 - link->player;
    int zone;

    if (link->player) {
        /* CPU: the human's strongest monster, else its first Spell/Trap */
        link->numTargets = 0;
        zone = CountMonsters(opponent) > 0 ? AiFindStrongestMonster(opponent, -1, 1, 1) : -1;
        if (zone >= 0 && EffectPhoenixWingWindBlastCheck(link, DUEL_LOC(opponent, zone))
            && TryAddEffectTarget(link, opponent, zone))
            return 1;
        for (zone = 0; zone <= ZONE_FIELD; zone++) {
            if (EffectPhoenixWingWindBlastCheck(link, DUEL_LOC(opponent, zone)) && TryAddEffectTarget(link, opponent, zone))
                return 1;
        }
        return 1;
    }
    switch (gChain.targetStep) {
    case 0:
        link->numTargets = 0;
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, sStrPhoenixWingPick);
        gChain.targetStep++;
        return 0;
    default:
        if (gMain.newKeys & B_BUTTON) {
            gChain.targetStep = 0;
            return 0;
        }
        if (DuelCursor_PickTarget(PHOENIX_WING_PICK) != 0) {
            int pickPlayer = gDuelScreen.selPlayer;
            int pickZone = gDuelScreen.selArea + gDuelScreen.selIndex;

            if (EffectPhoenixWingWindBlastCheck(link, DUEL_LOC(pickPlayer, pickZone))
                && TryAddEffectTarget(link, pickPlayer, pickZone))
                return 1;
            PlaySE(SE_ERROR);
        }
        return 0;
    }
}

int EffectPhoenixWingWindBlastResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated && link->numTargets == 1) {
        int player = (u8)link->targets[0];
        int zone = link->targets[0] >> 8;

        if (DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0)
            ReturnFieldCardToDeck(player, zone);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_PHOENIX_WING_WIND_BLAST */

#ifdef HAS_CARD_METAMORPHOSIS
/* ---- Metamorphosis -------------------------------------------------------------------------------------------------
 * Normal Spell: "Tribute 1 monster. Special Summon 1 Fusion Monster from your Extra Deck with the same Level as the
 * Tributed monster."
 * Prepare: one of the player's monsters has the Level of a Fusion Monster in the Fusion Deck. ChainB picks it with
 * Catapult Turtle's EffectTributeTargetChainB (the CPU: its highest-Level such monster), refuses a monster with no
 * such Fusion, then Tributes it (the cost, paid once the pick is made, as Exiled Force's) and keeps its Level in
 * targets[1]. Resolve: Cyber-Stein's steps with a list of the Fusion Monsters of that Level (CollectEffectTargets,
 * effect_target_collect.c: IsMetamorphosisFusion with param = the Level); the CPU takes the highest ATK. */

static inline int MonsterLevel(u16 id)
{
    return CARD_STATS_LEVEL(gCardStats[id & CARD_ID_MASK]);
}

/* 1 if card `id` is a Fusion Monster of Level `level`. */
int IsMetamorphosisFusion(u16 id, int level)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];

    return CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_KIND(stats) == CARD_KIND_FUSION
           && (int)CARD_STATS_LEVEL(stats) == level;
}

/* 1 if the Fusion Deck of `player` holds a Fusion Monster of Level `level`. */
static int HasFusionOfLevel(int player, int level)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    for (i = 0; i < p->fusionCount; i++) {
        if (IsMetamorphosisFusion(DUEL_CARD_ID(&p->fusionDeck[i]), level))
            return 1;
    }
    return 0;
}

/* 1 if the monster in (player, zone) may be Tributed for Metamorphosis. */
static int IsMetamorphosisTribute(int player, int zone)
{
    u16 id;

    if (zone > ZONE_MONSTER_4)
        return 0;
    id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);
    return id != 0 && HasFusionOfLevel(player, MonsterLevel(id));
}

int EffectMetamorphosisPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    int zone;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (IsMetamorphosisTribute(card->player, zone))
            return 1;
    }
    return 0;
}

int EffectMetamorphosisChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int player, zone;

    if (link->player) {
        int best = -1, bestLevel = -1;

        link->numTargets = 0;
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (IsMetamorphosisTribute(link->player, zone)
                && MonsterLevel(DUEL_CARD_ID(&ZoneOf(link->player, zone)->card)) > bestLevel) {
                best = zone;
                bestLevel = MonsterLevel(DUEL_CARD_ID(&ZoneOf(link->player, zone)->card));
            }
        }
        if (best < 0 || !TryAddEffectTarget(link, link->player, best))
            return 1;
    } else {
        if (!EffectTributeTargetChainB(link))
            return 0;
        if (link->numTargets != 1)
            return 1;
        if ((u8)link->targets[0] != link->player || !IsMetamorphosisTribute(link->player, link->targets[0] >> 8)) {
            /* no Fusion Monster of its Level: stay on the cursor */
            link->numTargets = 0;
            PlaySE(SE_ERROR);
            return 0;
        }
    }
    player = (u8)link->targets[0];
    zone = link->targets[0] >> 8;
    AddEffectTarget(link, MonsterLevel(DUEL_CARD_ID(&ZoneOf(player, zone)->card)));
    TributeMonster(player, zone);
    return 1;
}

int EffectMetamorphosisResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int level = link->targets[1];
    u16 *picked;
    u16 cmd;

    if (link->negated || link->numTargets != 2)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (CountFreeMonsterZones(link->player) == 0
            || CollectEffectTargets(link->player, CARD_METAMORPHOSIS, level) == 0)
            return EFFECT_STEP_DONE;
        if (link->player) {
            int i, best = 0;

            for (i = 1; i < gCardListView.count; i++) {
                if (CARD_STATS_ATK(gCardStats[gCardListView.cards[i] & CARD_ID_MASK])
                    > CARD_STATS_ATK(gCardStats[gCardListView.cards[best] & CARD_ID_MASK]))
                    best = i;
            }
            gCardListView.cursorRow = 0;
            gCardListView.top = best;
            return EFFECT_STEP_3;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, gStrCyberSteinSelectPrompt);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        CardListView_Open(link->player, -1, CARD_METAMORPHOSIS, level);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        picked = (u16 *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top];
        cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK
                           : DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK;
        DuelCmd_Push(cmd, picked[0], picked[1], 0);
        return EFFECT_STEP_4;
    case EFFECT_STEP_4:
        QueueSpecialSummon(link->player,
                           (struct DuelCard *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top], 1, 0,
                           0);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_METAMORPHOSIS */

#ifdef HAS_CARD_MAGICAL_SCIENTIST
/* ---- Magical Scientist ---------------------------------------------------------------------------------------------
 * "You can pay 1000 Life Points; Special Summon 1 Level 6 or lower Fusion Monster from your Extra Deck in face-up
 * Attack or Defense Position. That Fusion Monster cannot attack your opponent directly, and is returned to the Extra
 * Deck at the end of this turn."
 * The card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no limit per turn, as Cyber-Stein's:
 *   prepare  face up on the field, 1000 LP, a free monster zone and such a Fusion Monster in the Fusion Deck
 *   chainA   1000 LP (DUEL_CMD_LOSE_LP, a cost)
 *   resolve  Metamorphosis' steps with a list of the Fusion Deck's Level 6 or lower Fusion Monsters
 *            (ModCollectEffectTargets: IsScientistFusion; the CPU takes the highest ATK), Special Summoned face up in
 *            the position the player picks (QueueSpecialSummonChoosePosition, which takes FindFreeMonsterZone's zone).
 *            The zone and card ID go to gModDuel.scientistReturn, as Return from the Different Dimension marks its own.
 *   DuelPhase_End (battle_phase3.c)  EndPhase_ReturnMagicalScientist returns each marked monster still in its zone
 *            (ReturnFieldCardToHand: a Fusion Monster goes to the Fusion Deck), then clears the marks.
 *   No direct attack  CanMonsterAttack (card_menu_input.c) asks ModCannotAttackDirectly, which refuses a marked
 *            monster's attack while the opponent has no monster, as for Zombyra the Dark.
 * The CPU: AiUseMagicalScientist, below AiSpecialSummonCyberDragon. */
#define SCIENTIST_COST 1000
#define SCIENTIST_MAX_LEVEL 6

/* 1 if card `id` is a Fusion Monster of Level 6 or lower that may be Special Summoned. */
static int IsScientistFusion(u16 id, int param)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];

    return CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_KIND(stats) == CARD_KIND_FUSION
           && (int)CARD_STATS_LEVEL(stats) <= SCIENTIST_MAX_LEVEL && !IsCannotSpecialSummon(id & CARD_ID_MASK);
}

/* 1 if the monster in (player, zone) was Special Summoned by Magical Scientist this turn. */
static int IsScientistSummoned(int player, int zone)
{
    int bit = (player & 1) * 5 + zone;

    return zone <= ZONE_MONSTER_4 && (gModDuel.scientistReturn & (1 << bit))
           && DUEL_CARD_ID(&ZoneOf(player, zone)->card) == gModDuel.scientistReturnIds[bit];
}

int EffectMagicalScientistPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelPlayer *p = &gDuelPlayers[card->player & 1];
    struct DuelZone *z;
    int i;

    if (fromHand || card->event != RESPONSE_NONE || p->lifePoints < SCIENTIST_COST
        || CountFreeMonsterZones(card->player) == 0 || !CanSpecialSummon(card->player))
        return 0;
    z = ZoneOf(card->player, card->zone);
    if (!z->isFaceUp || DUEL_CARD_ID(&z->card) != (card->card & CARD_ID_MASK))
        return 0;
    for (i = 0; i < p->fusionCount; i++) {
        if (IsScientistFusion(DUEL_CARD_ID(&p->fusionDeck[i]), 0))
            return 1;
    }
    return 0;
}

int EffectMagicalScientistChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_LOSE_LP : DUEL_CMD_LOSE_LP, SCIENTIST_COST, 1, 0);
    return 1;
}

int EffectMagicalScientistResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    struct DuelCard *picked;
    u16 cmd;
    int zone, bit;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (CountFreeMonsterZones(link->player) == 0
            || CollectEffectTargets(link->player, CARD_MAGICAL_SCIENTIST, 0) == 0)
            return EFFECT_STEP_DONE;
        if (link->player) {
            int i, best = 0;

            for (i = 1; i < gCardListView.count; i++) {
                if (CARD_STATS_ATK(gCardStats[gCardListView.cards[i] & CARD_ID_MASK])
                    > CARD_STATS_ATK(gCardStats[gCardListView.cards[best] & CARD_ID_MASK]))
                    best = i;
            }
            gCardListView.cursorRow = 0;
            gCardListView.top = best;
            return EFFECT_STEP_3;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 9), TEXTBOX_FLAGS_DEFAULT, gStrCyberSteinSelectPrompt);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        CardListView_Open(link->player, -1, CARD_MAGICAL_SCIENTIST, 0);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        picked = (struct DuelCard *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top];
        cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK
                           : DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK;
        DuelCmd_Push(cmd, ((u16 *)picked)[0], ((u16 *)picked)[1], 0);
        return EFFECT_STEP_4;
    case EFFECT_STEP_4:
        picked = (struct DuelCard *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top];
        zone = FindFreeMonsterZone(link->player);
        if (zone >= 0) {
            bit = (link->player & 1) * 5 + zone;
            gModDuel.scientistReturn |= 1 << bit;
            gModDuel.scientistReturnIds[bit] = DUEL_CARD_ID(picked);
        }
        QueueSpecialSummonChoosePosition(link->player, picked, 1, 0);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}

void EndPhase_ReturnMagicalScientist(void)
{
    int player, zone;

    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (IsScientistSummoned(player, zone))
                ReturnFieldCardToHand(player, zone, 0);
        }
    }
    gModDuel.scientistReturn = 0;
}
#endif /* HAS_CARD_MAGICAL_SCIENTIST */

/* CanMonsterAttack (card_menu_input.c): 1 if the monster in (player, zone) may not attack now because it cannot
 * attack directly and the opponent has no monster: Zombyra the Dark, and a monster Magical Scientist summoned. */
int ModCannotAttackDirectly(int player, int zone)
{
    u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);

    if (CountMonsters(1 - player) != 0)
        return 0;
#ifdef HAS_CARD_ZOMBYRA_THE_DARK
    if (CARD_NUMBER(id) == CARD_ZOMBYRA_THE_DARK)
        return 1;
#endif
#ifdef HAS_CARD_MAGICAL_SCIENTIST
    if (IsScientistSummoned(player, zone))
        return 1;
#endif
    return 0;
}

#ifdef HAS_CARD_REINFORCEMENT_OF_THE_ARMY
/* ---- Reinforcement of the Army -----------------------------------------------------------------------------------
 * Normal Spell: "Add 1 Level 4 or lower Warrior monster from your Deck to your hand."
 * Sangan's resolve (EffectSanganResolve) does the search: it lists CollectEffectTargets (effect_target_collect.c:
 * IsReinforcementTarget) for the card number, lets the human pick (the CPU: AiPickCardListEntry), adds the card and
 * shuffles. The prepare handler needs such a monster in the deck, so the Spell is not wasted. */

/* 1 if card `id` is a Level 4 or lower Warrior monster. */
int IsReinforcementTarget(u16 id)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];

    return CARD_STATS_TYPE(stats) == CARD_TYPE_WARRIOR && CARD_STATS_LEVEL(stats) <= 4;
}

int EffectReinforcementOfTheArmyPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelPlayer *p = &gDuelPlayers[card->player & 1];
    int i;

    for (i = 0; i < p->deckCount; i++) {
        if (IsReinforcementTarget(DUEL_CARD_ID(&p->deck[i])))
            return 1;
    }
    return 0;
}
#endif /* HAS_CARD_REINFORCEMENT_OF_THE_ARMY */

#ifdef HAS_CARD_TOON_TABLE_OF_CONTENTS
/* ---- Toon Table of Contents (YGOPRODeck id 89997728) -----------------------------------------------------------
 * Normal Spell: "Add 1 \"Toon\" card from your Deck to your hand." Resolves with EffectSanganResolve (pick from
 * CollectEffectTargets). No prepare: it may be activated even with no \"Toon\" card left in the Deck (like Thunder
 * Dragon after the prepare change); the resolve then shows gStrNoDeckCardsToAdd and shuffles. Deck list:
 * IsToonTableDeckTarget (IsToonMonster, e.g. Manga Ryu-Ran, or the card name contains \"Toon\"). skipToonFilter in
 * effect_target_collect.c so Toon monsters are listed without face-up Toon World. CPU: AiWantsSpell only when the
 * list is non-empty. */

int IsToonTableDeckTarget(u16 id)
{
    u16 number = CARD_NUMBER(id);
    const u8 *name;
    int i, j;
    static const char sToonNeedle[] = "Toon";

    if (IsToonMonster(number))
        return 1;
    name = gCardNames + (id & CARD_ID_MASK) * CARD_NAME_SIZE;
    for (i = 0; name[i] != 0 && i < CARD_NAME_SIZE; i++) {
        for (j = 0; sToonNeedle[j] != 0 && name[i + j] == (u8)sToonNeedle[j]; j++)
            ;
        if (sToonNeedle[j] == 0)
            return 1;
    }
    return 0;
}
#endif /* HAS_CARD_TOON_TABLE_OF_CONTENTS */

#ifdef HAS_CARD_TOKEN_THANKSGIVING
/* ---- Token Thanksgiving (YGOPRODeck id 57182235) -----------------------------------------------------------------
 * Normal Spell: "Destroy all tokens on the field. Increase your Life Points by the number of tokens destroyed x 800
 * points." No prepare: it may be activated with no tokens (0 LP). DestroyFieldCard runs Ojama Token's trigger.
 * CPU: AiWantsTokenThanksgiving when any token is on the field. */

#define TOKEN_THANKSGIVING_LP_PER_TOKEN 800

static int TokenThanksgivingIsToken(u16 id)
{
    u16 number = CARD_NUMBER(id);

    return (u16)(number - CARD_NUMBER_TOKEN_FIRST) < CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST;
}

static int CountTokensOnField(void)
{
    int player, zone, n = 0;

    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);

            if (id != 0 && TokenThanksgivingIsToken(id))
                n++;
        }
    }
    return n;
}

int EffectTokenThanksgivingResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int player, zone, count;

    if (link->negated)
        return EFFECT_STEP_DONE;
    count = CountTokensOnField();
    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);

            if (id != 0 && TokenThanksgivingIsToken(id))
                DestroyFieldCard(player, zone, 1);
        }
    }
    if (count > 0)
        GainLifePoints(link->player, count * TOKEN_THANKSGIVING_LP_PER_TOKEN);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_TOKEN_THANKSGIVING */

#ifdef HAS_CARD_CARD_DESTRUCTION
/* ---- Card Destruction ----------------------------------------------------------------------------------------------
 * Normal Spell: "Both players discard as many cards as possible from their hands, then each player draws the same
 * number of cards they discarded." No prepare: it may be activated like Pot of Greed. Each hand is discarded through
 * DiscardHandCard by index, with the hand compacted only after its last card (the indices stay valid until then), so
 * the discard triggers run (Night Assailant; Electric Snake and Minar for the opponent's hand, discarded by an
 * opponent's effect). Then the draws: a player whose deck runs out decks out as usual (player 0 cannot). */
int EffectCardDestructionResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int counts[2];
    int side, player, i;

    if (link->negated)
        return EFFECT_STEP_DONE;
    for (side = 0; side <= 1; side++) {
        player = side ? 1 - link->player : link->player;
        counts[side] = gDuelPlayers[player & 1].handCount;
        for (i = 0; i < counts[side]; i++)
            DiscardHandCard(player, i, side, i == counts[side] - 1);
    }
    for (side = 0; side <= 1; side++) {
        player = side ? 1 - link->player : link->player;
        if (counts[side] > 0)
            DrawCards(player, counts[side]);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_CARD_DESTRUCTION */

#ifdef HAS_CARD_COLD_WAVE
/* ---- Cold Wave (YGOPRODeck id 23171648) ---------------------------------------------------------------------------
 * Normal Spell: "This turn and the next turn, no player can play any Spell or Trap Cards." Same as engine key 1312
 * (EffectStartOfMainPhase1Prepare, EffectLockMagicTrapResolve): Main Phase 1 before any Spell/Trap activation or
 * Normal Summon; then both players get magicTrapLockTurns = 2 (DUEL_CMD_SET_MAGIC_TRAP_LOCK_TURNS). The hand menu
 * drops ACTIVATE after either flag is set (card_command_menu.c, as CARD_1312). CPU: AiWantsSpell whenever it may;
 * AiTryPlaySpellTrap also refuses while magicTrapLockTurns is set (ai_deck.c). */

int EffectColdWavePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    (void)chainLink;
    if (!fromHand)
        return 0;
    return EffectStartOfMainPhase1Prepare(card);
}
#endif /* HAS_CARD_COLD_WAVE */

#ifdef HAS_CARD_DIFFERENT_DIMENSION_CAPSULE
/* ---- Different Dimension Capsule -----------------------------------------------------------------------------------
 * Normal Trap (PGD, early text): select 1 card from the Deck and remove it from play face-down; during your next
 * Standby Phase, add that card to your hand (DUEL_CMD_RETURN_BANISHED_CARD_TO_HAND). Any main-Deck card may be
 * chosen; no Spell & Trap zone is used. */

static const u8 sStrDdcSelectDeckCard[] = "Select a card from your Deck.";
extern const u8 gStrNoDeckCardsToAdd[];

int CollectEffectTargetsInt(int player, int cardNumber, int param) asm("CollectEffectTargets");

static int IsDifferentDimensionCapsuleDeckTarget(u16 id, int param)
{
    int type = (int)CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]);

    (void)param;
    return type <= CARD_TYPE_REPTILE || type == CARD_TYPE_MAGIC || type == CARD_TYPE_TRAP;
}

int EffectDifferentDimensionCapsulePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    (void)chainLink;
    (void)fromHand;
    return CollectEffectTargetsInt(card->player, CARD_DIFFERENT_DIMENSION_CAPSULE, 0) > 0;
}

static int DifferentDimensionCapsuleStandbyStep(int player)
{
    int side = player & 1;
    int i;
    u16 cmd;

    if (!gModDuel.ddcReturnPending[side])
        return 1;
    if (gChain.resolving || gChain.building)
        return 1;
    for (i = 0; i < gDuelPlayers[side].banishedCount; i++) {
        u32 info = gDuelPlayers[side].banishedInfo[i];
        u16 id = gDuelPlayers[side].banished[i].id & CARD_ID_MASK;

        if ((u8)info == BANISH_FACE_DOWN && id == (gModDuel.ddcReturnCardId[side] & CARD_ID_MASK)) {
            ShowCardEffect(player, id);
            cmd = DUEL_CMD_RETURN_BANISHED_CARD_TO_HAND;
            if (player)
                cmd = DUEL_CMD_RETURN_BANISHED_CARD_TO_HAND | DUEL_CMD_PLAYER;
            DuelCmd_Push(cmd, i, 1, 0);
            gModDuel.ddcReturnPending[side] = 0;
            return 1;
        }
    }
    gModDuel.ddcReturnPending[side] = 0;
    return 1;
}

int EffectDifferentDimensionCapsuleResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    u16 remove;
    u16 *picked;
    struct DuelCard *banish;

    (void)chainedTo;
    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (CollectEffectTargetsInt(link->player, CARD_DIFFERENT_DIMENSION_CAPSULE, 0) == 0) {
            if (link->player)
                return EFFECT_STEP_DONE;
            TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, gStrNoDeckCardsToAdd);
            return EFFECT_STEP_END;
        }
        if (link->player) {
            AiPickCardListEntry(link->card);
            gCardListView.cursorRow = 0;
            gCardListView.top = gAiWork.listPick;
            return EFFECT_STEP_3;
        }
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, sStrDdcSelectDeckCard);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        CardListView_Open(link->player, -1, CARD_DIFFERENT_DIMENSION_CAPSULE, 0);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        picked = (u16 *)&gCardListView.cards[gCardListView.top + gCardListView.cursorRow];
        ShowPickedCard(link->player,
                       gCardListView.cards[gCardListView.top + gCardListView.cursorRow] & CARD_ID_MASK);
        CopyDuelCard((u32 *)&gChain.effectCard, &gCardListView.cards[gCardListView.top + gCardListView.cursorRow]);
        remove = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_DECK : DUEL_CMD_REMOVE_CARD_FROM_DECK;
        DuelCmd_Push(remove, picked[0], picked[1], 0);
        return EFFECT_STEP_4;
    case EFFECT_STEP_4: {
        int side = link->player & 1;

        banish = (struct DuelCard *)&gChain.effectCard;
        banish->owner = side;
        AddCardToBanishedFaceDown(banish);
        gModDuel.ddcReturnCardId[side] = banish->id & CARD_ID_MASK;
        gModDuel.ddcReturnPending[side] = 1;
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SHUFFLE_DECK : DUEL_CMD_SHUFFLE_DECK, 0, 0, 0);
        return EFFECT_STEP_END;
    }
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_DIFFERENT_DIMENSION_CAPSULE */

#ifdef HAS_CARD_JAR_OF_GREED
/* ---- Jar of Greed --------------------------------------------------------------------------------------------------
 * Normal Trap: "Draw 1 card." A row with no prepare or check, like Gift of the Mystical Elf's: CanActivateEffect
 * lets a Set Jar of Greed be activated in any response window, at Trap speed. */
int EffectJarOfGreedResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated)
        DrawCards(link->player, 1);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_JAR_OF_GREED */

#ifdef HAS_CARD_NIGHT_ASSAILANT
/* ---- Night Assailant ---------------------------------------------------------------------------------------------
 * "FLIP: Target 1 monster your opponent controls; destroy that target.
 *  When this card is sent from your hand to the GY: Target 1 Flip monster in your GY, except 'Night Assailant'; add
 *  it to your hand."
 * One row, told apart by the link's event:
 *   flip                 a flip effect (HasFlipEffect, card_detail.c): Crass Clown's opponent-monster target
 *                        (EffectOpponentMonsterCheck / ChainB), Man-Eater Bug's EffectDestroyTargetResolve
 *   RESPONSE_DISCARDED   the off-field trigger that DiscardHandCard (duel_field_moves.c) queues through
 *                        QueueNightAssailantTrigger, only when the Graveyard holds a target; the list
 *                        (CollectEffectTargets, effect_target_collect.c: IsNightAssailantTarget) goes through
 *                        Mask of Darkness's card-list viewer and EffectMaskOfDarknessResolve.
 * A "Flip monster" is one that HasFlipEffect lists (the coded flip effects). */

/* 1 if card `id` may be added by the discard effect: a Flip monster other than Night Assailant. */
int IsNightAssailantTarget(u16 id)
{
    u16 number = ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK];

    if (CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) > CARD_TYPE_REPTILE || number == CARD_NIGHT_ASSAILANT)
        return 0;
    return HasFlipEffect(number, 0) != 0;
}

/* Night Assailant (card ID `id`) was discarded from the hand of `player`: queue its trigger if the Graveyard has a
 * target (the trigger's zone field holds the player, as for card 1242). */
void QueueNightAssailantTrigger(int player, u16 id)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    for (i = 0; i < p->graveCount; i++) {
        if (IsNightAssailantTarget(DUEL_CARD_ID(&p->graveyard[i]))) {
            Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_ZONE(0x1F & player)
                                 | TRIGGER_EVENT(RESPONSE_DISCARDED) | TRIGGER_KIND(CHAIN_KIND_OFF_FIELD) | id,
                             0);
            return;
        }
    }
}

u16 EffectNightAssailantCheck(struct ChainEntry *card, u16 pos)
{
    if (card->event == RESPONSE_DISCARDED)
        return 1;
    return EffectOpponentMonsterCheck(card, pos);
}

int EffectNightAssailantChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (link->event == RESPONSE_DISCARDED)
        return EffectCardListTargetChainB(link);
    return EffectOpponentMonsterChainB(link);
}

int EffectNightAssailantResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (link->event == RESPONSE_DISCARDED)
        return EffectMaskOfDarknessResolve(link);
    return EffectDestroyTargetResolve(link);
}
#endif /* HAS_CARD_NIGHT_ASSAILANT */

#ifdef HAS_CARD_EXILED_FORCE
/* ---- Exiled Force ------------------------------------------------------------------------------------------------
 * "You can Tribute this card, then target 1 monster on the field; destroy that target."
 * The card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), with no once-per-turn limit: the card is
 * gone once used. The target is any other monster (Man-Eater Bug's EffectTargetableMonsterChainB, with a wrapper that
 * refuses Exiled Force itself). The Tribute is paid when the target is chosen rather than in chainA: the target step
 * calls CanActivateEffect, and the prepare handler fails once the card has left its zone. The destruction is
 * EffectDestroyTargetResolve's. */

static inline int IsExiledForceHere(struct ChainEntry *card)
{
    struct DuelZone *z = ZoneOf(card->player, card->zone);
    return z->isFaceUp && DUEL_CARD_ID(&z->card) == (card->card & CARD_ID_MASK);
}

int EffectExiledForcePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (fromHand || card->event != RESPONSE_NONE || !IsExiledForceHere(card))
        return 0;
    return CountMonsters(0) + CountMonsters(1) > 1;     /* a monster besides Exiled Force */
}

/* Any monster but Exiled Force itself. */
u16 EffectExiledForceCheck(struct ChainEntry *card, u16 pos)
{
    if (pos == DUEL_LOC(card->player, card->zone))
        return 0;
    return EffectTargetableMonsterCheck(card, pos);
}

int EffectExiledForceChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (!EffectTargetableMonsterChainB(link, (int)prevLink))
        return 0;
    if (link->numTargets == 1 && link->targets[0] == DUEL_LOC(link->player, link->zone)) {
        link->numTargets = 0;
        if (!link->player) {
            /* the human picked Exiled Force: stay on the cursor */
            PlaySE(SE_ERROR);
            return 0;
        }
        return 1;               /* the CPU found no other target: the effect does nothing */
    }
    if (link->numTargets == 1)
        TributeMonster(link->player, link->zone);
    return 1;
}

int EffectExiledForceResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    return EffectDestroyTargetResolve(link);
}
#endif /* HAS_CARD_EXILED_FORCE */

#if defined(HAS_CARD_GOLEM_SENTRY) || defined(HAS_CARD_GUARDIAN_SPHINX)
/* ---- Golem Sentry -------------------------------------------------------------------------------------------------
 * "Once per turn: You can change this card to face-down Defense Position. When this card is Flip Summoned: Target 1
 * monster your opponent controls; return that target to the hand."
 * One row, told apart by the link's event:
 *   RESPONSE_FLIP_SUMMONED  queued by AddedCardsSummoned for a Flip Summon (as Sacred Crane's trigger; the vanilla
 *                           Flip Summon list in summon_action.c gates on CanActivateEffectOfCard, which passes no
 *                           zone or event): an opponent's monster (Crass Clown's EffectOpponentMonsterCheck / ChainB; the CPU picks
 *                           the human's strongest), returned to its owner's hand (EffectReturnTargetToHandResolve)
 *   RESPONSE_NONE           the card menu's ACTIVATE in the Main Phase (CanActivateMonsterEffect), once per turn
 *                           (effectUnused, set again by DuelCmd_TurnStart): the target is the card itself, turned
 *                           face-down Defense Position by Book of Moon's EffectBookOfMoonResolve (its Equip Cards are
 *                           destroyed, its counters removed). The CPU uses it while the human controls a monster, so
 *                           its next Flip Summon has a target.
 * Guardian Sphinx: "Once per turn: You can change this card to face-down Defense Position. When this card is Flip
 * Summoned: Return all monsters your opponent controls to the hand." The same row handlers: the face-down change is the
 * same, and the Flip Summon trigger takes no target (IsGuardianSphinx) and returns every monster of the opponent's to
 * its owner's hand (ReturnFieldCardToHand). */
static inline int IsGolemSentryFlipTrigger(struct ChainEntry *link)
{
    return link->event == RESPONSE_FLIP_SUMMONED && link->loc0 == DUEL_LOC(link->player, link->zone);
}

static inline int IsGuardianSphinx(struct ChainEntry *link)
{
#ifdef HAS_CARD_GUARDIAN_SPHINX
    return ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[link->card & CARD_ID_MASK] == CARD_GUARDIAN_SPHINX;
#else
    return 0;
#endif
}

int EffectGolemSentryPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelZone *z;

    if (fromHand)
        return 0;
    z = ZoneOf(card->player, card->zone);
    if (!z->isFaceUp || DUEL_CARD_ID(&z->card) != (card->card & CARD_ID_MASK))
        return 0;
    if (card->event == RESPONSE_FLIP_SUMMONED)
        return IsGolemSentryFlipTrigger(card);
    return card->event == RESPONSE_NONE && z->effectUnused;
}

u16 EffectGolemSentryCheck(struct ChainEntry *card, u16 pos)
{
    if (card->event == RESPONSE_FLIP_SUMMONED)
        return IsGuardianSphinx(card) || EffectOpponentMonsterCheck(card, pos);
    return pos == DUEL_LOC(card->player, card->zone);
}

int EffectGolemSentryChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    u16 cmd;

    if (link->event == RESPONSE_FLIP_SUMMONED) {
        if (IsGuardianSphinx(link)) {
            link->numTargets = 0;       /* all of the opponent's monsters: no target */
            return 1;
        }
        return EffectOpponentMonsterChainB(link);
    }
    /* the face-down change: itself, and the effect is used for this turn */
    link->numTargets = 1;
    link->targets[0] = DUEL_LOC(link->player, link->zone);
    cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_EFFECT_UNUSED : DUEL_CMD_SET_EFFECT_UNUSED;
    DuelCmd_Push(cmd, link->zone, 0, 0);
    return 1;
}

int EffectGolemSentryResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (link->event == RESPONSE_FLIP_SUMMONED) {
        if (link->negated)
            return EFFECT_STEP_DONE;
        if (IsGuardianSphinx(link)) {
            int opponent = 1 - link->player, zone;

            for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
                if (DUEL_CARD_ID(&ZoneOf(opponent, zone)->card) != 0)
                    ReturnFieldCardToHand(opponent, zone, 0);
            }
            return EFFECT_STEP_DONE;
        }
        return EffectReturnTargetToHandResolve(link);
    }
    return EffectBookOfMoonResolve(link, chainedTo);
}
#endif /* HAS_CARD_GOLEM_SENTRY || HAS_CARD_GUARDIAN_SPHINX */

#ifdef HAS_CARD_POT_OF_AVARICE
/* ---- Pot of Avarice -----------------------------------------------------------------------------------------------
 * "Target 5 monsters in your GY; shuffle all 5 into the Deck, then draw 2 cards." (Normal Spell)
 * The prepare needs 5 monsters in the Graveyard (CollectEffectTargets). The 5 are picked when it resolves, as
 * Painful Choice picks its 5 (a link holds one card word): the human picks one at a time in the card list, each
 * leaving the Graveyard at once (DUEL_CMD_REMOVE_CARD_FROM_GRAVEYARD) and kept in gChain.scratch.effect.effectCards;
 * the CPU takes its 5 lowest-ATK monsters. Then each goes back: a Main Deck monster to the Deck
 * (DUEL_CMD_ADD_CARD_TO_DECK_TOP), a Fusion Monster to the Fusion Deck (AddCardToFusionDeck), the Deck is shuffled
 * and the player draws 2. With fewer than 5 monsters left when it resolves, nothing happens. */
#define AVARICE_COUNT 5

static const u8 sStrAvariceSelect[] = "Select 5 monsters in your Graveyard to return to the Deck.";
static const u8 sStrAvariceMore[] = "Select %d more.";

int CollectEffectTargetsInt(int player, int cardNumber, int param) asm("CollectEffectTargets");

int EffectPotOfAvaricePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return CollectEffectTargetsInt(card->player, CARD_POT_OF_AVARICE, 0) >= AVARICE_COUNT;
}

/* Take list entry `i` (a Graveyard monster) out of the Graveyard and keep it as pick `slot`. */
static void AvariceTake(struct ChainEntry *link, int i, int slot)
{
    u16 *picked = (u16 *)&gCardListView.cards[i];

    CopyDuelCard(&gChain.scratch.effect.effectCards[slot], (struct DuelCard *)picked);
    DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_GRAVEYARD
                              : DUEL_CMD_REMOVE_CARD_FROM_GRAVEYARD, picked[0], picked[1], 0);
}

int EffectPotOfAvariceResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    u8 text[0x40];
    int count, i, k;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        count = CollectEffectTargetsInt(link->player, CARD_POT_OF_AVARICE, 0);
        if (count < AVARICE_COUNT)
            return EFFECT_STEP_DONE;
        if (link->player) {
            /* the CPU: its 5 lowest-ATK monsters */
            u8 used[32];

            MemClear16(used, sizeof(used));
            for (k = 0; k < AVARICE_COUNT; k++) {
                int best = -1;
                u32 bestAtk = 0;

                for (i = 0; i < count && i < (int)sizeof(used); i++) {
                    u32 atk = CARD_STATS_ATK(gCardStats[gCardListView.cards[i] & CARD_ID_MASK]);

                    if (!used[i] && (best < 0 || atk < bestAtk)) {
                        best = i;
                        bestAtk = atk;
                    }
                }
                used[best] = 1;
                AvariceTake(link, best, k);
            }
            return EFFECT_STEP_4;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 7), TEXTBOX_FLAGS_DEFAULT, sStrAvariceSelect);
        gChain.effectSubStep = AVARICE_COUNT;
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:         /* open the Graveyard list (re-collected: the earlier picks are out) */
        CardListView_Open(link->player, CARDLIST_AREA_EFFECT_TARGETS, CARD_POT_OF_AVARICE, 0);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        gChain.effectSubStep--;
        AvariceTake(link, gCardListView.cursorRow + gCardListView.top, gChain.effectSubStep);
        if (gChain.effectSubStep != 0) {
            FormatInt((char *)text, (const char *)sStrAvariceMore, gChain.effectSubStep);
            TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 5), TEXTBOX_FLAGS_DEFAULT, text);
            return EFFECT_STEP_2;
        }
        return EFFECT_STEP_4;
    case EFFECT_STEP_4:         /* the removals have run: back to the Deck / Fusion Deck */
        for (k = 0; k < AVARICE_COUNT; k++) {
            struct DuelCard *card = &gChain.scratch.effect.effectCards[k];
            u16 *halves = (u16 *)card;

            if (CARD_STATS_KIND(gCardStats[DUEL_CARD_ID(card) & CARD_ID_MASK]) == CARD_KIND_FUSION)
                AddCardToFusionDeck(link->player, card);
            else
                DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_ADD_CARD_TO_DECK_TOP
                                          : DUEL_CMD_ADD_CARD_TO_DECK_TOP, halves[0], halves[1], 0);
        }
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SHUFFLE_DECK : DUEL_CMD_SHUFFLE_DECK, 1, 0, 0);
        return EFFECT_STEP_5;
    case EFFECT_STEP_5:
        DrawCards(link->player, 2);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_POT_OF_AVARICE */

#ifdef HAS_CARD_MONSTER_REINCARNATION
/* ---- Monster Reincarnation (YGOPRODeck id 74848038) ---------------------------------------------------------------
 * Normal Spell: "Discard 1 card, then target 1 monster in your Graveyard; add that target to your hand."
 *   prepare  at least one card in the hand to discard and a monster in the Graveyard (ModCollectEffectTargets)
 *   chainA   DuelPrompt_PostDiscardCost (any card; the CPU picks its own)
 *   chainB   EffectCardListTargetChainB (graveyard monsters)
 *   resolve  EffectMaskOfDarknessResolve (DUEL_CMD_RETURN_GRAVEYARD_CARD_TO_HAND)
 *   CPU      AiWantsMonsterReincarnation (empty monster field only); AiPickCardListEntry prefers a monster that beats
 *            the human's strongest field monster when they control one */

int CollectEffectTargetsInt(int player, int cardNumber, int param) asm("CollectEffectTargets");

int EffectMonsterReincarnationPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (chainLink != NULL)
        return 0;
    (void)fromHand;
    if (gDuelPlayers[card->player & 1].handCount < 1)
        return 0;
    return CollectEffectTargetsInt(card->player, CARD_MONSTER_REINCARNATION, 0) > 0;
}

int EffectMonsterReincarnationChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelPrompt_PostDiscardCost(link->player, 1, FALSE, FALSE);
    return 1;
}
#endif /* HAS_CARD_MONSTER_REINCARNATION */

#ifdef HAS_CARD_TOON_CANNON_SOLDIER
/* ---- Toon Cannon Soldier (YGOPRODeck id 79875176) ---------------------------------------------------------------
 * Toon Effect Monster: cannot attack the turn it is Summoned; destroyed if Toon World is destroyed; direct attack
 * while you control Toon World and the opponent controls no Toons; ignition "Tribute 1 monster; inflict 500 damage"
 * (Cannon Soldier handlers). Not IsSpecialSummonOnly: Normal Summon/Set like any Level 4 (summon_checks.c skips the
 * Toon World gate for this card only; Toon World still needed for direct attack). Uses the turn's Normal Summon
 * (summon_action.c; other Toons do not). No Special Summon-from-hand (summon_builders.c). gAddedCardEffects row. */
#endif /* HAS_CARD_TOON_CANNON_SOLDIER */

#ifdef HAS_CARD_NEWDORIA
/* ---- Newdoria -----------------------------------------------------------------------------------------------------
 * "If this card is destroyed by battle and sent to the GY: Target 1 monster on the field; destroy that target."
 * Queued with Sangan's and Gigantes' triggers (SendBattleDestroyedCardToGraveyard, duel_field_moves.c: destroyed by
 * battle and sent to the Graveyard, not banished). The target is any monster on the field (Man-Eater Bug's
 * EffectTargetableMonsterCheck / ChainB; the CPU picks the human's strongest), destroyed by
 * EffectDestroyTargetResolve. */
u16 EffectNewdoriaCheck(struct ChainEntry *card, u16 pos)
{
    return EffectTargetableMonsterCheck(card, pos);
}

int EffectNewdoriaChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    return EffectTargetableMonsterChainB(link, (int)prevLink);
}

int EffectNewdoriaResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    return EffectDestroyTargetResolve(link);
}
#endif /* HAS_CARD_NEWDORIA */

#ifdef HAS_CARD_BOOK_OF_LIFE
/* ---- Book of Life -------------------------------------------------------------------------------------------------
 * "Target 1 Zombie monster in your GY and 1 monster in your opponent's GY; Special Summon the first target, also
 * banish the second target." (Normal Spell)
 * The prepare needs a free monster zone and both lists (CollectEffectTargets: param 0 the player's Graveyard Zombies,
 * param 1 the opponent's Graveyard monsters). The cards are chosen when it resolves (a link holds one card word, not
 * two): the human picks each in the card list, the CPU takes the highest ATK of each. The Zombie leaves the Graveyard
 * (kept in gChain.effectCard), the opponent's monster is banished, then the Zombie is Special Summoned. The two parts
 * are independent ("also"): a list that is empty by then is skipped. */
static const u8 sStrBookOfLifeZombie[] = "Select a Zombie monster to Special Summon.";
static const u8 sStrBookOfLifeBanish[] = "Select a monster in your opponent's Graveyard to banish.";

int CollectEffectTargetsInt(int player, int cardNumber, int param) asm("CollectEffectTargets");

/* The list entry with the highest ATK (the CPU's pick). */
static int BookOfLifeBestEntry(int count)
{
    int i, best = 0;
    u32 bestAtk = 0;

    for (i = 0; i < count; i++) {
        u32 atk = CARD_STATS_ATK(gCardStats[gCardListView.cards[i] & CARD_ID_MASK]);

        if (i == 0 || atk > bestAtk) {
            best = i;
            bestAtk = atk;
        }
    }
    return best;
}

int EffectBookOfLifePrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return CountFreeMonsterZones(card->player) > 0 && CollectEffectTargetsInt(card->player, CARD_BOOK_OF_LIFE, 0) > 0
           && CollectEffectTargetsInt(card->player, CARD_BOOK_OF_LIFE, 1) > 0;
}

int EffectBookOfLifeResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int count;
    u16 *picked;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        gChain.effectCard.id = 0;
        count = CollectEffectTargetsInt(link->player, CARD_BOOK_OF_LIFE, 0);
        if (count == 0 || CountFreeMonsterZones(link->player) == 0)
            return EFFECT_STEP_4;                   /* no Zombie to summon: only the banish */
        if (link->player) {
            gCardListView.cursorRow = 0;
            gCardListView.top = BookOfLifeBestEntry(count);
            return EFFECT_STEP_3;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 5), TEXTBOX_FLAGS_DEFAULT, sStrBookOfLifeZombie);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        CardListView_Open(link->player, CARDLIST_AREA_EFFECT_TARGETS, CARD_BOOK_OF_LIFE, 0);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        /* the Zombie leaves the Graveyard; it is summoned once the banish is done */
        picked = (u16 *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top];
        CopyDuelCard(&gChain.effectCard, (struct DuelCard *)picked);
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_GRAVEYARD
                                  : DUEL_CMD_REMOVE_CARD_FROM_GRAVEYARD, picked[0], picked[1], 0);
        return EFFECT_STEP_4;
    case EFFECT_STEP_4:
        count = CollectEffectTargetsInt(link->player, CARD_BOOK_OF_LIFE, 1);
        if (count == 0)
            return EFFECT_STEP_7;
        if (link->player) {
            gCardListView.cursorRow = 0;
            gCardListView.top = BookOfLifeBestEntry(count);
            return EFFECT_STEP_6;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 5), TEXTBOX_FLAGS_DEFAULT, sStrBookOfLifeBanish);
        return EFFECT_STEP_5;
    case EFFECT_STEP_5:
        CardListView_Open(link->player, CARDLIST_AREA_EFFECT_TARGETS, CARD_BOOK_OF_LIFE, 1);
        return EFFECT_STEP_6;
    case EFFECT_STEP_6:
        BanishGraveyardCard(1 - link->player, (u16 *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top]);
        return EFFECT_STEP_7;
    case EFFECT_STEP_7:
        if (gChain.effectCard.id != 0)
            QueueSpecialSummonChoosePosition(link->player, &gChain.effectCard, 1, 0);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_BOOK_OF_LIFE */

/* ---- Kycoo the Ghost Destroyer -------------------------------------------------------------------------------
 * Reuses effect key 1511's handlers (EffectBanishOpponentGraveMonstersResolve, battle damage triggers in
 * battle_phase2.c). While face up, it blocks graveyard banish effects the same way CARD_1511 does. */

#define ENTRY_EVENT_BYTE(entry) (((u8 *)(entry))[3] >> 2)
#define LOC1_LOW_BYTE(entry) (((u8 *)(entry))[8])

int CollectEffectTargetsInt(int player, int cardNumber, int param) asm("CollectEffectTargets");

int CountActiveKycooOnField(int player)
{
    int n = CountActiveCardsOnField(player, CARD_1511);

#ifdef HAS_CARD_KYCOO_THE_GHOST_DESTROYER
    n += CountActiveCardsOnField(player, CARD_KYCOO_THE_GHOST_DESTROYER);
#endif
    return n;
}

#ifdef HAS_CARD_KYCOO_THE_GHOST_DESTROYER
/* Optional trigger after battle damage: the opponent took damage and has a monster in the Graveyard. */
int EffectKycooTheGhostDestroyerPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    if (fromHand != 0 || chainLink != NULL)
        return 0;
    switch (ENTRY_EVENT_BYTE(card)) {
    case RESPONSE_BATTLE_DAMAGE:
    case RESPONSE_BATTLE_DEFLECTED_DAMAGE:
        if ((LOC1_LOW_BYTE(card) & 0xF) == card->player)
            return 0;
        return CollectEffectTargetsInt(card->player, CARD_KYCOO_THE_GHOST_DESTROYER, 0) > 0;
    default:
        return 0;
    }
}

/* The resolve. Key 1511's (EffectBanishOpponentGraveMonstersResolve, effect_resolve11.c) has two bugs, so Kycoo no
 * longer uses it:
 *   - no CPU branch: it asked its Yes/No questions and opened the Graveyard list on the human's screen for the CPU's
 *     Kycoo too, so the human picked their own monsters to lose;
 *   - no limit: after each pick it returns EFFECT_STEP_START (0x80 in the ROM), whose case sets the picks left back
 *     to 2, so the human could banish their opponent's whole Graveyard.
 * The human: the same questions and list, up to KYCOO_MAX_BANISH picks (gChain.effectSubStep counts them down; a pick
 * goes back to EFFECT_STEP_2, the question). The CPU banishes, without asking, the two monsters of the human's
 * Graveyard with the highest ATK (the strongest revival targets), or the one there is. */
#define KYCOO_MAX_BANISH 2

int EffectKycooTheGhostDestroyerResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    u8 taken[KYCOO_MAX_BANISH];
    int n, i, k, best;
    u16 *picked;

    if (link->negated)
        return EFFECT_STEP_DONE;
    if (link->player == 0) {
        switch (gChain.effectStep) {
        case EFFECT_STEP_START:
            gChain.effectSubStep = KYCOO_MAX_BANISH;
            /* fall through */
        case EFFECT_STEP_2:
            if (gChain.effectSubStep == 0
                || CollectEffectTargetsInt(link->player, CARD_KYCOO_THE_GHOST_DESTROYER, 0) == 0)
                return EFFECT_STEP_DONE;
            TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT,
                        (const u8 *)(gChain.effectSubStep == KYCOO_MAX_BANISH
                                         ? gStrBanishOpponentGraveMonsterQuestion
                                         : gStrBanishAnotherOpponentGraveMonsterQuestion));
            TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
            return EFFECT_STEP_3;
        case EFFECT_STEP_3:
            if (gTextBox.result == 0)
                return EFFECT_STEP_DONE;
            TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT,
                        (const u8 *)gStrSelectOpponentGraveMonsterToBanish);
            return EFFECT_STEP_4;
        case EFFECT_STEP_4:
            CardListView_Open(link->player, -1, CARD_KYCOO_THE_GHOST_DESTROYER, 0);
            return EFFECT_STEP_5;
        case EFFECT_STEP_5:
            picked = (u16 *)&gCardListView.cards[gCardListView.top + gCardListView.cursorRow];
            DuelCmd_Push(DUEL_CMD_PLAYER | DUEL_CMD_BANISH_GRAVEYARD_CARD, picked[0], picked[1], 0);
            gChain.effectSubStep--;
            return EFFECT_STEP_2;
        }
        return EFFECT_STEP_DONE;
    }
    n = CollectEffectTargetsInt(link->player, CARD_KYCOO_THE_GHOST_DESTROYER, 0);
    for (k = 0; k < KYCOO_MAX_BANISH && k < n; k++) {
        best = -1;
        for (i = 0; i < gCardListView.count; i++) {
            if ((k > 0 && taken[0] == i)
                || (best >= 0 && CARD_STATS_ATK(gCardStats[gCardListView.cards[i] & CARD_ID_MASK])
                                     <= CARD_STATS_ATK(gCardStats[gCardListView.cards[best] & CARD_ID_MASK])))
                continue;
            best = i;
        }
        if (best < 0)
            break;
        taken[k] = best;
        DuelCmd_Push(DUEL_CMD_BANISH_GRAVEYARD_CARD, ((u16 *)&gCardListView.cards[best])[0],
                     ((u16 *)&gCardListView.cards[best])[1], 0);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_KYCOO_THE_GHOST_DESTROYER */

/* ---- Blade Knight --------------------------------------------------------------------------------------------
 * +400 ATK at 0–1 cards in hand is in card_stats.c. When its controller has no other monsters, Flip Effects of
 * monsters destroyed by battle with it do not activate (BattleStage_TriggerFlipEffect). */

#ifdef HAS_CARD_BLADE_KNIGHT
#define CARD_NUMBER(id) (((const u16 *)CARD_ID_TO_NUMBER_ADDR)[(id) & CARD_ID_MASK])

static const u8 sBladeKnightName[] = "Blade Knight";

int IsBladeKnightCard(u16 cardId)
{
    const u8 *name;
    int i;

    if ((cardId & CARD_ID_MASK) < 821)
        return 0;
    if (CARD_NUMBER(cardId) == CARD_BLADE_KNIGHT)
        return 1;
    name = gCardNames + (cardId & CARD_ID_MASK) * CARD_NAME_SIZE;
    for (i = 0; sBladeKnightName[i] != 0; i++) {
        if (name[i] != sBladeKnightName[i])
            return 0;
    }
    return name[i] == 0;
}

int BladeKnightNegatesBattleFlipEffect(void)
{
    int attackerPlayer;
    int defenderPlayer;
    u16 atkId;
    u16 flipNo;

    if (gBattle.direct || gBattle.flipCardId == 0)
        return 0;
    attackerPlayer = gDuel.turnPlayer & 1;
    defenderPlayer = (1 - attackerPlayer) & 1;
    atkId = DUEL_CARD_ID(&gDuelPlayers[attackerPlayer].zones[gBattle.atkSlot]);
    if (!IsBladeKnightCard(atkId))
        return 0;
    if (CountMonsters(attackerPlayer) != 1)
        return 0;
    /* Attacker must win (flip monster destroyed by this battle). */
    if (gBattle.side[attackerPlayer].battleValue <= gBattle.side[defenderPlayer].battleValue)
        return 0;
    if ((gBattle.flipCardId & CARD_ID_MASK)
        != (gBattle.side[defenderPlayer].cardId & CARD_ID_MASK))
        return 0;
    flipNo = CARD_NUMBER(gBattle.flipCardId);
    if (!HasFlipEffect(flipNo, 1) && !HasFlipEffect(flipNo, 0))
        return 0;
    return 1;
}
#endif /* HAS_CARD_BLADE_KNIGHT */

#ifdef HAS_CARD_DARK_RULER_HA_DES
/* ---- Dark Ruler Ha Des ---------------------------------------------------------------------------------------
 * "Cannot be Special Summoned from the GY. Negate the effects of monsters destroyed by battle with Fiend monsters
 * you control."
 *   CanReviveGraveyardCard (duel_ritual.c) refuses it: the revival cards cannot pick it.
 *   A monster destroyed in battle by a Fiend whose controller has a face-up Ha Des (HaDesNegatesBattleDestroyed):
 *   SendBattleDestroyedCardToGraveyard (duel_field_moves.c) queues none of its "destroyed in battle" effects
 *   (Sangan, Witch of the Black Forest, Mystic Tomato, ...), and a face-down one's Flip effect is not activated
 *   (HaDesNegatesBattleFlipEffect, beside Blade Knight's in battle_phase1.c / battle_phase2.c). */

/* 1 if the monster of `player` destroyed in this battle was destroyed by a Fiend of the other player, who
 * controls a face-up Dark Ruler Ha Des. */
int HaDesNegatesBattleDestroyed(int player)
{
    int other = 1 - (player & 1);
    u16 killer = gBattle.side[other].cardId & CARD_ID_MASK;

    if (gBattle.direct || killer == 0)
        return 0;
    if (CARD_STATS_TYPE(gCardStats[killer]) != CARD_TYPE_FIEND)
        return 0;
    return CountFaceUpMonstersByNumber(other, CARD_DARK_RULER_HA_DES) > 0;
}

/* 1 if the battle's Flip monster (the target, flipped face up) is destroyed by a Fiend of the attacker, who controls
 * a face-up Dark Ruler Ha Des. */
int HaDesNegatesBattleFlipEffect(void)
{
    int attackerPlayer = gDuel.turnPlayer & 1;
    int defenderPlayer = 1 - attackerPlayer;

    if (gBattle.direct || gBattle.flipCardId == 0)
        return 0;
    if ((gBattle.flipCardId & CARD_ID_MASK) != (gBattle.side[defenderPlayer].cardId & CARD_ID_MASK))
        return 0;
    if (gBattle.side[attackerPlayer].battleValue <= gBattle.side[defenderPlayer].battleValue)
        return 0;       /* the Flip monster survives */
    return HaDesNegatesBattleDestroyed(defenderPlayer);
}
#endif /* HAS_CARD_DARK_RULER_HA_DES */

/* ---- Flip effects locked for the rest of a turn (gModDuel.flipEffectsLocked; nothing sets it at present: it was
 * a mistaken Dark Ruler Ha Des effect) ---- */


int FlipEffectsLockedForTurn(void)
{
    return gModDuel.flipEffectsLocked != 0;
}

void LockFlipEffectsForTurn(void)
{
    gModDuel.flipEffectsLocked = 1;
}

void ClearFlipEffectsLockForTurn(void)
{
    gModDuel.flipEffectsLocked = 0;
}

/* ---- "Gravekeeper's" cards -----------------------------------------------------------------------------------
 * 1 if the card's name starts with "Gravekeeper's" (the archetype the Gravekeeper's cards refer to). */
static const char sGravekeeperPrefix[] = "Gravekeeper's";

int IsGravekeeperMonster(u16 id)
{
    const u8 *name = gCardNames + (id & CARD_ID_MASK) * CARD_NAME_SIZE;
    int i;

    if (CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) > CARD_TYPE_REPTILE)
        return 0;
    for (i = 0; sGravekeeperPrefix[i] != 0; i++) {
        if (name[i] != (u8)sGravekeeperPrefix[i])
            return 0;
    }
    return 1;
}

/* ---- Gravekeeper's Spy ----------------------------------------------------------------------------------------
 * "FLIP: Special Summon 1 'Gravekeeper's' monster with 1500 or less ATK from your Deck."
 * A flip effect (HasFlipEffect, card_detail.c), resolved by the floaters' handler EffectSpecialSummonFromDeckResolve:
 * it asks the human, lists the candidates (CollectEffectTargets, effect_target_collect.c: deck "Gravekeeper's"
 * monsters with ATK 1500 or less, not Fusion or Ritual), Special Summons the pick in Attack Position and shuffles. */

#ifdef HAS_CARD_SOUL_EXCHANGE
/* ---- Soul Exchange -------------------------------------------------------------------------------------------------
 * Normal Spell: "Target 1 monster your opponent controls; this turn, if you Tribute a monster, you must Tribute that
 * monster, as if you controlled it. You cannot conduct your Battle Phase the turn you activate this card."
 * The engine's effect key 1220 (tools/newcards.py ENGINE_NUMBERS gives the card that number): prepare
 * EffectCanTributeOpponentMonsterPrepare (Main Phase 1, a level 5+ monster to Tribute Summon or a Catapult Turtle-like
 * monster), chainA EffectSkipBattlePhaseChainA, chainB EffectTributeOpponentMonsterChainB, resolve
 * EffectTributeOpponentMonsterResolve (a Tribute Summon from the hand, or the Tribute of a Catapult Turtle, The Little
 * Swordsman of Aile or Cannon Soldier). Two fixes in effect_resolve8.c: the summon paths Tributed the opponent's zone 0
 * (the packed target was passed as the player), and the steps are cursor UI only, so the CPU gets these: */

/* The CPU targets the human's strongest monster. */
int SoulExchangeCpuChainB(struct ChainEntry *link)
{
    int zone = AiFindStrongestMonster(0, -1, 1, 1);

    link->numTargets = 0;
    if (zone >= 0)
        TryAddEffectTarget(link, 0, zone);
    return 1;
}

/* The CPU's own weakest monster it may Tribute, -1 for none. */
static int WeakestOwnTribute(int player)
{
    int zone, best = -1;
    u32 bestAtk = 0;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (IsTributableMonster(player, zone) && (best < 0 || GetZoneCardAtk(player, zone) < bestAtk)) {
            best = zone;
            bestAtk = GetZoneCardAtk(player, zone);
        }
    }
    return best;
}

/* The CPU Tribute Summons its strongest level 5+ hand monster with the target (and, for level 7+, its own weakest
 * monster). */
int SoulExchangeCpuResolve(struct ChainEntry *link)
{
    struct DuelPlayer *p = &gDuelPlayers[link->player & 1];
    int targetPlayer = (u8)link->targets[0];
    int targetZone = link->targets[0] >> 8;
    int i, best = -1, zone, own;
    u32 bestAtk = 0;

    if (link->numTargets < 1 || DUEL_CARD_ID(&ZoneOf(targetPlayer, targetZone)->card) == 0 || p->normalSummonUsed)
        return EFFECT_STEP_DONE;
    own = WeakestOwnTribute(link->player);
    for (i = 0; i < p->handCount; i++) {
        u16 id = DUEL_CARD_ID(&p->hand[i]);
        u32 stats = gCardStats[id & CARD_ID_MASK];
        int level = CARD_STATS_LEVEL(stats);

        if (CARD_STATS_TYPE(stats) > CARD_TYPE_REPTILE || IsSpecialSummonOnly(id) || level <= 4)
            continue;
        if (level <= 6 ? FindFreeMonsterZone(link->player) < 0 : own < 0)
            continue;
        if (best < 0 || CARD_STATS_ATK(stats) > bestAtk) {
            best = i;
            bestAtk = CARD_STATS_ATK(stats);
        }
    }
    if (best < 0)
        return EFFECT_STEP_DONE;
    TributeMonster(targetPlayer, targetZone);
    if (CARD_STATS_LEVEL(gCardStats[DUEL_CARD_ID(&p->hand[best]) & CARD_ID_MASK]) > 6) {
        TributeMonster(link->player, own);
        zone = own;
    } else {
        zone = FindFreeMonsterZone(link->player);
    }
    p->normalSummonUsed = 1;
    QueueNormalSummonChoosePosition(link->player, best, zone, 0);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_SOUL_EXCHANGE */

/* ---- The CPU and the added cards --------------------------------------------------------------------------------
 * The CPU's logic decides card by card, by card number (ai_deck.c, ai_summon.c, ai_turn_steps.c), so it never chose
 * an added card: AiPlaySpells only tries its fixed lists, AiShouldActivateSetCard says no to every card it does not
 * name, AiActivateMonsterEffects only knows gAiEffectMonsters, and the summon choice skips Special-Summon-only
 * monsters. (Added Traps and Quick-Play Spells were already Set: AiStepSetSpellTraps Sets any Trap it does not name.)
 * These fallbacks cover the added cards (IDs from CARD_ID_COUNT_ORIGINAL on), with a small policy per card:
 *   AiPlayAddedCards (end of AiPlaySpells' AI_SPELLS_LISTS)  in the Main Phase: Special Summon Black Luster Soldier /
 *       Chaos Sorcerer (banish the weakest LIGHT and DARK monsters of the Graveyard), activate the card-menu effects of
 *       face-up added monsters (CanActivateMonsterEffect), and play added Magic cards (AiTryPlaySpellTrap, which makes
 *       the hand and set-card checks and plays the card)
 *   AiShouldActivateAddedCard (end of AiShouldActivateSetCard)  a Set added card in an activation window; its prepare
 *       handler has accepted the window already (CanActivateFieldCard) */

#define AI_PLAYER 1
#define AI_HUMAN 0
#define CPU_CHAIN_KIND_MONSTER (0x80000000 | (CHAIN_KIND_MONSTER << 21))

static inline int IsAddedCardId(u16 id)
{
    return (id & CARD_ID_MASK) >= CARD_ID_COUNT_ORIGINAL;
}

static inline u16 CardNumberOfId(u16 id)
{
    return ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[id & CARD_ID_MASK];
}

#ifdef HAS_CARD_BERSERK_GORILLA
/* ---- Berserk Gorilla ----------------------------------------------------------------------------------------------
 * "If this card is in Defense Position: Destroy it. This card must attack, if able."
 *   Defense Position  ModDuelStateCheck (DuelMainStep, while nothing else is busy) destroys a face-up one in Defense
 *                     Position (a face-down one's effect does not apply), except during an attack (battle stages
 *                     2-11: a Set one flipped by the attack is destroyed once the attack is over).
 *   Must attack       the human cannot end the Battle Phase (B in the attacker selection, card_menu_input.c) while
 *                     one may still attack (BerserkGorillaBlocksBattleEnd), nor pick "Complete turn" in the Main
 *                     Phase menu instead of entering it (BerserkGorillaBlocksTurnEnd, duel_phases.c); the CPU attacks with it when it has no
 *                     better attack (AiPlanForcedAttack). AiShouldRetreatToDefense never turns it to Defense. */
static const u8 sStrBerserkGorillaMustAttack[] = "Berserk Gorilla must attack.";

/* The zone of a face-up Attack Position Berserk Gorilla of `player` that may still attack this Battle Phase (the
 * attacker selection's attackableMask, not in attackedMask), else -1. */
static int BerserkGorillaMustAttackZone(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int zone;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);
        u16 id = DUEL_CARD_ID(&z->card);

        if (id != 0 && z->isFaceUp && !z->isDefense && CardNumberOfId(id) == CARD_BERSERK_GORILLA
            && (p->attackableMask >> zone & 1) && !(p->attackedMask >> zone & 1))
            return zone;
    }
    return -1;
}

/* The human pressed B to end the Battle Phase: refused (1) while a Berserk Gorilla may still attack. */
int BerserkGorillaBlocksBattleEnd(int player)
{
    if (BerserkGorillaMustAttackZone(player) < 0)
        return 0;
    PlaySE(SE_ERROR);
    TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 5), TEXTBOX_FLAGS_DEFAULT, sStrBerserkGorillaMustAttack);
    return 1;
}

/* The human chose "Complete turn" in the Main Phase menu (DuelPhase_Main) while the Battle Phase can be entered:
 * refused (1) when a Berserk Gorilla could attack in it (the attackable mask is built as the Battle Phase builds it). */
int BerserkGorillaBlocksTurnEnd(int player)
{
    BuildAttackableMask((void *)gDuelPlayers, player, 0, 0);
    return BerserkGorillaBlocksBattleEnd(player);
}

/* The CPU has no attack it wants (AiPlanAttack): with a Berserk Gorilla that may still attack, plan its attack in
 * gAiWork.bestAttack (a winning one if AiFindAttackTarget finds it, else on the human's weakest monster, or direct
 * when the human has none) and return 1. */
int AiPlanForcedAttack(void)
{
    struct AttackPlan plan;
    int zone = BerserkGorillaMustAttackZone(AI_PLAYER), target;

    if (zone < 0)
        return 0;
    MemClear16(&plan, sizeof(plan));
    if (!AiFindAttackTarget(zone, &plan)) {
        MemClear16(&plan, sizeof(plan));
        target = CountMonsters(AI_HUMAN) > 0 ? AiFindWeakestMonster(AI_HUMAN, -1, 1, 1) : -1;
        if (target < 0)
            plan.isDirect = 1;
        else
            plan.targetZone = target;
    }
    plan.attackerZone = zone;
    plan.isValid = 1;
    MemCopy16(&gAiWork.bestAttack, &plan, sizeof(plan));
    return 1;
}
#endif /* HAS_CARD_BERSERK_GORILLA */

/* ---- Threatening Roar ---------------------------------------------------------------------------------------------
 * "Your opponent cannot declare an attack this turn." (Normal Trap)
 * The resolve stores the turn in gModDuel.roarTurn of the activating player's opponent (gDuel.turnCount + 1, 0 none).
 * ModAttacksBlocked then holds for that player for the rest of the turn: BuildAttackableMask (card_menu_input.c)
 * leaves their attackable mask empty, as Swords of Revealing Light does (no Battle Phase, no attacker to pick), and
 * the CPU's attacker choice (BattleStage_SelectAttacker) is skipped. An attack already declared goes on: the replay
 * check (Battle_CheckReplay) asks CanMonsterAttack, which is unchanged. The CPU activates it:
 *   before the human's Battle Phase   ModBattlePhaseStart (BattleStage_Start, card_menu_input.c, as the Battle Phase
 *                                     begins): while the human has a face-up monster with 2000 or more ATK, it flips its
 *                                     Set Threatening Roar (CanActivateFieldCard) and queues it, as its Standby
 *                                     activations do; it resolves before the attackable mask is built
 *   against an attack                 while the human has another monster that could attack after it
 *                                     (AiShouldActivateAddedCard) */
#define AI_ROAR_MIN_ATK 2000

int ModAttacksBlocked(int player)
{
    return gModDuel.roarTurn[player & 1] != 0 && gModDuel.roarTurn[player & 1] == (u16)(gDuel.turnCount + 1);
}

#ifdef HAS_CARD_THREATENING_ROAR
/* The CPU's Threatening Roar as the human's Battle Phase begins (ModBattlePhaseStart). */
static void AiRoarBeforeBattle(int player)
{
    int zone, strong = 0;

    if (player != AI_HUMAN || gDuelCtrl.isLinkDuel || ModAttacksBlocked(AI_HUMAN))
        return;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(AI_HUMAN, zone);

        if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp && (int)GetZoneCardAtk(AI_HUMAN, zone) >= AI_ROAR_MIN_ATK)
            strong = 1;
    }
    if (!strong)
        return;
    for (zone = ZONE_SPELL_0; zone <= ZONE_SPELL_4; zone++) {
        struct DuelZone *z = ZoneOf(AI_PLAYER, zone);
        u16 id = DUEL_CARD_ID(&z->card);
        struct ChainEntry ref;

        if (id == 0 || z->isFaceUp || CardNumberOfId(id) != CARD_THREATENING_ROAR)
            continue;
        MemClear16(&ref, sizeof(ref));
        ref.player = AI_PLAYER;
        if (!CanActivateFieldCard(&ref, AI_PLAYER, zone))
            continue;
        DuelCmd_Push(DUEL_CMD_PLAYER | DUEL_CMD_CHAIN_BANNER, 0, 0, 0);
        FlipFieldCard(AI_PLAYER, zone, 0);
        Chain_AddPending(TRIGGER_PLAYER(AI_PLAYER) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_SPELL_TRAP)
                             | TRIGGER_EVENT(RESPONSE_NONE) | id,
                         0);
        return;
    }
}
#endif /* HAS_CARD_THREATENING_ROAR */

/* BattleStage_Start (card_menu_input.c) calls this as `player`'s Battle Phase begins (before its attackable mask). */
void ModBattlePhaseStart(int player)
{
#ifdef HAS_CARD_ASURA_PRIEST
    int i;

    gModDuel.asuraTurn = gDuel.turnCount + 1;   /* Asura Priest's record is for this Battle Phase */
    for (i = 0; i < 5; i++)
        gModDuel.asuraTargets[i] = 0;
#endif
#ifdef HAS_CARD_THREATENING_ROAR
    AiRoarBeforeBattle(player);
#endif
}

#ifdef HAS_CARD_THREATENING_ROAR
int EffectThreateningRoarPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand;
}

int EffectThreateningRoarResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated)
        gModDuel.roarTurn[(1 - link->player) & 1] = gDuel.turnCount + 1;
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_THREATENING_ROAR */

#if defined(HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END) || defined(HAS_CARD_SECRET_BARREL)
static int CountHandAndFieldCards(int player);
#endif

#ifdef HAS_CARD_SECRET_BARREL
int AiWantsSecretBarrel(void);
#endif

#ifdef HAS_CARD_LEVEL_LIMIT_AREA_B
/* ---- Level Limit - Area B (YGOPRODeck id 3136426) -----------------------------------------------------------------
 * Effect text (YGOPRODeck API): "Change all face-up Level 4 or higher monsters to Defense Position."
 * Continuous Spell in the card pool; stays on the field when activated. Like Dragon Capture Jar (328): activation
 * and ApplyLevelLimitAreaB after a face-up summon; ModDuelStateCheck keeps attack-position Level 4+ down while it
 * is active; the human cannot manually switch a face-up Level 4+ monster to Attack (card_command_menu.c). */

static int LevelLimitAreaBActive(void)
{
    return CountActiveCardsOnField(0, CARD_LEVEL_LIMIT_AREA_B) > 0
           || CountActiveCardsOnField(1, CARD_LEVEL_LIMIT_AREA_B) > 0;
}

int ApplyLevelLimitAreaB(int player)
{
    int p, z, found = 0;

    if (!LevelLimitAreaBActive())
        return 0;
    for (p = 0; p <= 1; p++) {
        for (z = ZONE_MONSTER_0; z <= ZONE_MONSTER_4; z++) {
            struct DuelZone *zn = ZoneOf(p, z);
            u16 id = DUEL_CARD_ID(&zn->card);

            if (id != 0 && zn->isFaceUp && !zn->isDefense
                && (int)CARD_STATS_LEVEL(gCardStats[id & CARD_ID_MASK]) >= 4)
                found = 1;
        }
    }
    if (!found)
        return 0;
    DuelCmd_Push(player ? DUEL_CMD_PLAYER | DUEL_CMD_SHOW_CARD_EFFECT : DUEL_CMD_SHOW_CARD_EFFECT,
                 gCardNumberToId[CARD_LEVEL_LIMIT_AREA_B], 1, 0);
    for (p = 0; p <= 1; p++) {
        for (z = ZONE_MONSTER_0; z <= ZONE_MONSTER_4; z++) {
            struct DuelZone *zn = ZoneOf(p, z);
            u16 id = DUEL_CARD_ID(&zn->card);

            if (id != 0 && zn->isFaceUp && !zn->isDefense
                && (int)CARD_STATS_LEVEL(gCardStats[id & CARD_ID_MASK]) >= 4)
                ChangeBattlePosition(p, z, 0, 0);
        }
    }
    return 1;
}

int ModLevelLimitAreaBBlocksAtkPosition(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    if (!LevelLimitAreaBActive() || id == 0 || !z->isFaceUp)
        return 0;
    return (int)CARD_STATS_LEVEL(gCardStats[id & CARD_ID_MASK]) >= 4;
}

int EffectLevelLimitAreaBResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated)
        ApplyLevelLimitAreaB(link->player);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_LEVEL_LIMIT_AREA_B */

#ifdef HAS_CARD_FINAL_ATTACK_ORDERS
/* ---- Final Attack Orders (YGOPRODeck id 18710707) ---------------------------------------------------------------
 * Effect text (YGOPRODeck API): "All face-up monsters on the field are changed to Attack Position, and their battle
 * positions cannot be changed." Continuous Trap; stays face-up when activated. ApplyFinalAttackOrders switches every
 * face-up Defense Position monster to Attack (ChangeBattlePosition), on activation and after a face-up summon or from
 * ModDuelStateCheck while active. The human and CPU cannot change battle positions while it is active
 * (card_command_menu.c, AiCanChangePosition). CPU: AiPlayAddedCards (AiTryPlaySpellTrap). */

int FinalAttackOrdersActive(void)
{
    return CountActiveCardsOnField(0, CARD_FINAL_ATTACK_ORDERS) > 0
           || CountActiveCardsOnField(1, CARD_FINAL_ATTACK_ORDERS) > 0;
}

int ApplyFinalAttackOrders(int player)
{
    int p, z, found = 0;

    if (!FinalAttackOrdersActive())
        return 0;
    for (p = 0; p <= 1; p++) {
        for (z = ZONE_MONSTER_0; z <= ZONE_MONSTER_4; z++) {
            struct DuelZone *zn = ZoneOf(p, z);

            if (DUEL_CARD_ID(&zn->card) != 0 && zn->isFaceUp && zn->isDefense)
                found = 1;
        }
    }
    if (!found)
        return 0;
    DuelCmd_Push(player ? DUEL_CMD_PLAYER | DUEL_CMD_SHOW_CARD_EFFECT : DUEL_CMD_SHOW_CARD_EFFECT,
                 gCardNumberToId[CARD_FINAL_ATTACK_ORDERS], 1, 0);
    for (p = 0; p <= 1; p++) {
        for (z = ZONE_MONSTER_0; z <= ZONE_MONSTER_4; z++) {
            struct DuelZone *zn = ZoneOf(p, z);

            if (DUEL_CARD_ID(&zn->card) != 0 && zn->isFaceUp && zn->isDefense)
                ChangeBattlePosition(p, z, 0, 0);
        }
    }
    return 1;
}

int EffectFinalAttackOrdersResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated)
        ApplyFinalAttackOrders(link->player);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_FINAL_ATTACK_ORDERS */

#ifdef HAS_CARD_MYSTIK_WOK
/* ---- Mystik Wok (YGOPRODeck id 80161395) -------------------------------------------------------------------------
 * Quick-Play Spell: "Tribute 1 monster. Select either its ATK or DEF and gain that many Life Points."
 *   prepare  at least one monster may be Tributed
 *   chainB   EffectTributeTargetChainB (the CPU: its weakest by ATK); ATK/DEF are read before TributeMonster
 *   resolve  the human picks ATK or DEF from a two-line menu; the CPU takes whichever is higher
 *   CPU      AiWantsMystikWok when LP is low and the best stat on its weakest Tribute is worth it */

static const u8 sStrMystikWokMenu[] = "Mystik Wok:\n    @3Gain LP equal to its ATK\n    @2Gain LP equal to its DEF";
#define MYSTIK_WOK_MODE_ATK 0
#define MYSTIK_WOK_MODE_DEF 1
#define MYSTIK_WOK_AI_LOW_LP 6000
#define MYSTIK_WOK_AI_MIN_GAIN 1500

static int MystikWokWeakestTribute(int player)
{
    int zone, best = -1;
    u32 bestAtk = 0;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (IsTributableMonster(player, zone) && (best < 0 || GetZoneCardAtk(player, zone) < bestAtk)) {
            best = zone;
            bestAtk = GetZoneCardAtk(player, zone);
        }
    }
    return best;
}

static u32 MystikWokBestGainOnWeakestTribute(int player)
{
    int zone = MystikWokWeakestTribute(player);
    u32 atk, def;

    if (zone < 0)
        return 0;
    atk = GetZoneCardAtk(player, zone);
    def = GetZoneCardDef(player, zone);
    return atk >= def ? atk : def;
}

static int AiWantsMystikWok(void)
{
    u32 gain = MystikWokBestGainOnWeakestTribute(AI_PLAYER);

    if (gain < MYSTIK_WOK_AI_MIN_GAIN)
        return 0;
    return (int)gDuelPlayers[AI_PLAYER].lifePoints <= MYSTIK_WOK_AI_LOW_LP;
}

int EffectMystikWokPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return CountTributableMonsters(card->player, -1) > 0;
}

int EffectMystikWokChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int zone;

    if (link->player) {
        zone = MystikWokWeakestTribute(link->player);
        if (zone < 0)
            return 1;
        gModDuel.mystikWokAtk = (u16)GetZoneCardAtk(link->player, zone);
        gModDuel.mystikWokDef = (u16)GetZoneCardDef(link->player, zone);
        link->unk12 = gModDuel.mystikWokAtk >= gModDuel.mystikWokDef ? MYSTIK_WOK_MODE_ATK : MYSTIK_WOK_MODE_DEF;
        TributeMonster(link->player, zone);
        return 1;
    }
    if (!EffectTributeTargetChainB(link))
        return 0;
    if (link->numTargets != 1)
        return 1;
    zone = link->targets[0] >> 8;
    if ((u8)link->targets[0] != link->player || !IsTributableMonster(link->player, zone)) {
        link->numTargets = 0;
        PlaySE(SE_ERROR);
        return 0;
    }
    gModDuel.mystikWokAtk = (u16)GetZoneCardAtk(link->player, zone);
    gModDuel.mystikWokDef = (u16)GetZoneCardDef(link->player, zone);
    link->numTargets = 0;
    TributeMonster(link->player, zone);
    return 1;
}

int EffectMystikWokResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    u32 gain;

    if (link->negated)
        return EFFECT_STEP_DONE;
    if (link->player) {
        gain = link->unk12 == MYSTIK_WOK_MODE_DEF ? gModDuel.mystikWokDef : gModDuel.mystikWokAtk;
        if (gain != 0)
            GainLifePoints(link->player, gain);
        return EFFECT_STEP_DONE;
    }
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(19, 6), TEXTBOX_FLAGS_DEFAULT, sStrMystikWokMenu);
        TextBoxSetMenu(TEXTBOX_MENU_TWO_CHOICE, NULL, NULL);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        if (gTextBox.active)
            return EFFECT_STEP_2;
        gain = gTextBox.result ? gModDuel.mystikWokDef : gModDuel.mystikWokAtk;
        if (gain != 0)
            GainLifePoints(link->player, gain);
        return EFFECT_STEP_DONE;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_MYSTIK_WOK */

#ifdef HAS_CARD_HYSTERIC_FAIRY
/* ---- Hysteric Fairy (YGOPRODeck id 21297224) -----------------------------------------------------------------------
 * "Tribute 2 monsters on your side of the field to increase your Life Points by 1000 points."
 * Same tribute pick and resolve shape as ROM keys 1432/1442 (EffectTributeTwoMonsters*); the CPU picks its two
 * weakest tributable monsters. The CPU never activates it (AiWantsMonsterEffect returns 0). */

#define HYSTERIC_FAIRY_LP_GAIN 1000

static void HystericFairyTwoWeakestTributes(int player, int *zoneA, int *zoneB)
{
    int zone;
    int a = -1;
    int b = -1;
    u32 atkA = 0xffffffff;
    u32 atkB = 0xffffffff;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        u32 atk;

        if (!IsTributableMonster(player, zone))
            continue;
        atk = GetZoneCardAtk(player, zone);
        if (a < 0 || atk < atkA) {
            b = a;
            atkB = atkA;
            a = zone;
            atkA = atk;
        } else if (b < 0 || atk < atkB) {
            b = zone;
            atkB = atk;
        }
    }
    *zoneA = a;
    *zoneB = b;
}

int EffectHystericFairyPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    struct DuelZone *z;

    if (fromHand)
        return 0;
    if (card->event != RESPONSE_NONE)
        return 0;
    z = ZoneOf(card->player, card->zone);
    if (!z->isFaceUp || DUEL_CARD_ID(&z->card) != (card->card & CARD_ID_MASK))
        return 0;
    if (CountActiveCardsOnField(0, CARD_1418) > 0 || CountActiveCardsOnField(1, CARD_1418) > 0)
        return 0;
    return CountTributableMonsters(card->player, -1) > 1;
}

int EffectHystericFairyChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int z0, z1;

    (void)prevLink;
    if (link->player) {
        HystericFairyTwoWeakestTributes(link->player, &z0, &z1);
        link->numTargets = 0;
        if (z0 < 0 || z1 < 0)
            return 1;
        if (TryAddEffectTarget(link, link->player, z0) == 0)
            return 1;
        if (TryAddEffectTarget(link, link->player, z1) == 0)
            return 1;
        return 1;
    }
    return EffectTributeTwoMonstersChainB(link);
}

int EffectHystericFairyResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int i;

    (void)chainedTo;
    if (link->negated)
        return EFFECT_STEP_DONE;
    if (CountActiveCardsOnField(0, CARD_1418) > 0 || CountActiveCardsOnField(1, CARD_1418) > 0
        || link->numTargets != 2)
        return EFFECT_STEP_DONE;
    for (i = 0; i < 2; i++) {
        int targetPlayer = DUEL_LOC_PLAYER(link->targets[i]);
        int targetZone = DUEL_LOC_ZONE(link->targets[i]);

        if (targetPlayer != link->player)
            return EFFECT_STEP_DONE;
        if (DUEL_CARD_ID(&ZoneOf(targetPlayer, targetZone)->card) == 0)
            return EFFECT_STEP_DONE;
    }
    for (i = 0; i < 2; i++)
        TributeMonster(DUEL_LOC_PLAYER(link->targets[i]), DUEL_LOC_ZONE(link->targets[i]));
    GainLifePoints(link->player, HYSTERIC_FAIRY_LP_GAIN);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_HYSTERIC_FAIRY */

#ifdef HAS_CARD_SPELL_CANCELLER
/* ---- Spell Canceller (YGOPRODeck id 84636823) --------------------------------------------------------------------
 * "Spell Cards, and their effects on the field, cannot be activated. Negate all Spell effects on the field."
 * While a face-up copy is on either side, IsImperialOrderActive (duel_ritual.c) is true: UpdateSpellTrapNegation
 * disables face-up Magic cards and gDuel.magicNegated negates their effects (same engine path as Imperial Order).
 * CanActivateEffect and the hand Spell/Trap menu strip ACTIVATE from Magic cards (like Anti-Magic Fragrance, but
 * Traps are unaffected). No gAddedCardEffects row (continuous monster, like Jinzo). */
#endif /* HAS_CARD_SPELL_CANCELLER */

#ifdef HAS_CARD_GRAVITY_BIND
/* ---- Gravity Bind (YGOPRODeck id 85742772) -----------------------------------------------------------------------
 * Effect text (YGOPRODeck API): "Level 4 or higher monsters cannot attack." Continuous Trap; stays face-up when
 * activated. The attack rule is in CanMonsterAttack (card_menu_input.c), with vanilla effect key CARD_1323.
 * CPU: AiPlayAddedCards; AiShouldActivateAddedCard default yes (CARD_1323 in AiShouldActivateSetCard). */

int EffectGravityBindResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_GRAVITY_BIND */

#ifdef HAS_CARD_SECRET_BARREL
/* ---- Secret Barrel ------------------------------------------------------------------------------------------------
 * Normal Trap: "Inflict 200 damage to your opponent for each card in your opponent's hand and for each card they
 * control on the field." The CPU: AiWantsSecretBarrel, AiPlayAddedCards (Main Phase), AiShouldActivateAddedCard. */
#define SECRET_BARREL_DAMAGE_PER_CARD 200

int EffectSecretBarrelPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand && CountHandAndFieldCards((1 - card->player) & 1) > 0;
}

int EffectSecretBarrelResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated) {
        int opponent = (1 - link->player) & 1;
        int cards = CountHandAndFieldCards(opponent);

        if (cards > 0)
            LoseLifePoints(opponent, cards * SECRET_BARREL_DAMAGE_PER_CARD);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_SECRET_BARREL */

#ifdef HAS_CARD_OJAMA_TOKEN
/* ---- Ojama Token --------------------------------------------------------------------------------------------------
 * "When this Token is destroyed, inflict 300 points of damage to its controller." SendFieldCardToGrave
 * (duel_send_to_grave.c) calls this when the token is destroyed (battle or effect); banish instead of destroy
 * (Banisher of the Light, and the like) does not run it. */
#define OJAMA_TOKEN_DAMAGE 300

void OjamaTokenDestroyed(int player)
{
    LoseLifePoints(player & 1, OJAMA_TOKEN_DAMAGE);
}
#endif /* HAS_CARD_OJAMA_TOKEN */

#if defined(HAS_CARD_OJAMA_TRIO) && defined(HAS_CARD_OJAMA_TOKEN)
/* ---- Ojama Trio ---------------------------------------------------------------------------------------------------
 * Normal Trap: "Special Summon 3 'Ojama Tokens' (Beast-Type/LIGHT/Level 2/ATK 0/DEF 1000) in Defense Position on
 * your opponent's side of the field." The tokens are TOKEN_KIND_1 (card number 1921, tools/newcards.py
 * TOKEN_NUMBERS). */
#define OJAMA_TRIO_CMD(player, cmd) ((player) ? DUEL_CMD_PLAYER | (cmd) : (cmd))

int EffectOjamaTrioPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    int opponent = (1 - card->player) & 1;

    return !fromHand && CountFreeMonsterZones(opponent) > 2;
}

int EffectOjamaTrioResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated) {
        int opponent = (1 - link->player) & 1;
        int zone;
        int summoned;

        for (zone = ZONE_MONSTER_0, summoned = 0; zone <= ZONE_MONSTER_4 && summoned < 3; zone++) {
            if (IsMonsterZoneFree(opponent, zone)) {
                DuelCmd_Push(OJAMA_TRIO_CMD(opponent, DUEL_CMD_SUMMON_TOKEN), (u8)zone | link->zone << 8,
                             TOKEN_KIND_1, 0);
                summoned++;
            }
        }
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_OJAMA_TRIO && HAS_CARD_OJAMA_TOKEN */

#ifdef HAS_CARD_THESTALOS_THE_FIRESTORM_MONARCH
/* ---- Thestalos the Firestorm Monarch ------------------------------------------------------------------------------
 * "If this card is Tribute Summoned: Discard 1 random card from your opponent's hand, then, if it was a Monster Card,
 * inflict damage to your opponent equal to its original Level x 100."
 * The two Normal Summon paths (summon_checks.c) queue its RESPONSE_SUMMONED trigger in Zaborg's case; it is Level 6,
 * so that summon always has a Tribute. Mandatory, no target. The resolve discards a random card of the opponent's
 * hand (DiscardHandCard, by the opponent's effect) and, for a monster, takes its printed Level x 100 from the
 * opponent's LP (LoseLifePoints). */
int EffectThestalosResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent = 1 - link->player;
    struct DuelPlayer *p = &gDuelPlayers[opponent & 1];
    int i;
    u32 stats;

    if (link->negated || p->handCount == 0)
        return EFFECT_STEP_DONE;
    i = (u32)Random() % p->handCount;
    stats = gCardStats[DUEL_CARD_ID(&p->hand[i]) & CARD_ID_MASK];
    DiscardHandCard(opponent, i, 1, 1);
    if (CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_LEVEL(stats) > 0)
        LoseLifePoints(opponent, CARD_STATS_LEVEL(stats) * 100);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_THESTALOS_THE_FIRESTORM_MONARCH */

/* ---- Marauding Captain --------------------------------------------------------------------------------------------
 * "Your opponent cannot target Warrior monsters for attacks, except this one. When this card is Normal Summoned: You
 * can Special Summon 1 Level 4 or lower monster from your hand."
 *   attacks   ModAttackTargetLocked: a face-up Warrior of `player` that is not Marauding Captain cannot be attacked
 *             while `player` has a face-up Marauding Captain in another zone (two of them lock each other). The
 *             human's target pick (BattleStage_SelectTarget, card_menu_input.c) refuses it; the CPU's attack plans
 *             (AiFindAttackTarget, ai_picks.c) skip it.
 *   summon    AddedCardsSummoned queues its trigger (RESPONSE_SUMMONED, its zone) for a face-up Normal Summon. The
 *             human is asked, then picks a Level 4 or lower monster of the hand (ModCollectEffectTargets); the CPU
 *             always takes its highest ATK one. It leaves the hand and is Special Summoned, its position chosen. */
int ModAttackTargetLocked(int player, int zone)
{
#ifdef HAS_CARD_MARAUDING_CAPTAIN
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);
    int i;

    if (id == 0 || !z->isFaceUp || CardNumberOfId(id) == CARD_MARAUDING_CAPTAIN
        || CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) != CARD_TYPE_WARRIOR)
        return 0;
    for (i = ZONE_MONSTER_0; i <= ZONE_MONSTER_4; i++) {
        struct DuelZone *c = ZoneOf(player, i);

        if (i != zone && c->isFaceUp && CardNumberOfId(DUEL_CARD_ID(&c->card)) == CARD_MARAUDING_CAPTAIN)
            return 1;
    }
#endif
    return 0;
}

#ifdef HAS_CARD_MARAUDING_CAPTAIN
static const u8 sStrMaraudingCaptainPrompt[] = "Special Summon a Level 4 or lower monster from your hand?";

int CollectEffectTargetsInt(int player, int cardNumber, int param) asm("CollectEffectTargets");

static inline int IsMaraudingCaptainTrigger(struct ChainEntry *link)
{
    return link->event == RESPONSE_SUMMONED && link->loc0 == DUEL_LOC(link->player, link->zone);
}

int EffectMaraudingCaptainPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand && IsMaraudingCaptainTrigger(card) && CountFreeMonsterZones(card->player) > 0
           && CanSpecialSummon(card->player);
}

int EffectMaraudingCaptainResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int count, i, best;
    u32 bestAtk;
    u16 *picked;

    if (link->negated)
        return EFFECT_STEP_DONE;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        count = CollectEffectTargetsInt(link->player, CARD_MARAUDING_CAPTAIN, 0);
        if (count == 0 || CountFreeMonsterZones(link->player) == 0)
            return EFFECT_STEP_DONE;
        if (link->player) {
            best = 0;
            bestAtk = 0;
            for (i = 0; i < count; i++) {
                u32 atk = CARD_STATS_ATK(gCardStats[gCardListView.cards[i] & CARD_ID_MASK]);

                if (i == 0 || atk > bestAtk) {
                    best = i;
                    bestAtk = atk;
                }
            }
            gCardListView.cursorRow = 0;
            gCardListView.top = best;
            return EFFECT_STEP_3;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 7), TEXTBOX_FLAGS_DEFAULT, sStrMaraudingCaptainPrompt);
        TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        if (!gTextBox.result)
            return EFFECT_STEP_DONE;
        CardListView_Open(link->player, CARDLIST_AREA_EFFECT_TARGETS, CARD_MARAUDING_CAPTAIN, 0);
        return EFFECT_STEP_3;
    case EFFECT_STEP_3:
        picked = (u16 *)&gCardListView.cards[gCardListView.cursorRow + gCardListView.top];
        CopyDuelCard(&gChain.effectCard, (struct DuelCard *)picked);
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_HAND : DUEL_CMD_REMOVE_CARD_FROM_HAND,
                     picked[0], picked[1], 0);
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_COMPACT_HAND : DUEL_CMD_COMPACT_HAND, 0, 0, 0);
        return EFFECT_STEP_4;
    case EFFECT_STEP_4:
        QueueSpecialSummonChoosePosition(link->player, &gChain.effectCard, 1, 0);
        return EFFECT_STEP_END;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_MARAUDING_CAPTAIN */

/* ---- Asura Priest: attacks every opponent monster once -------------------------------------------------------------
 * "This card can attack all monsters your opponent controls once each." (Its Spirit rule and "Cannot be Special
 * Summoned" are with Yata-Garasu's: EndPhase_ReturnSpiritMonsters, IsCannotSpecialSummon.)
 *   record   for the Battle Phase of turn gModDuel.asuraTurn - 1 (set, and the record emptied, as each Battle Phase
 *            starts: ModBattlePhaseStart): per zone of the attacking player, the monsters it attacked, as the
 *            opponent's zone (asuraTargets, a bit per zone) and that monster's DuelZone.serial there (asuraSerials;
 *            gBattle.zoneSerialSnapshot holds the target's at the declaration). The serial is stamped when a card is
 *            placed and kept when it is flipped, so a monster that came into the zone later (recruited by Mystic
 *            Tomato, Special Summoned, ...) is a new one it may attack
 *   again    ModEndAttack: after an attack on a monster (not a direct one) by a face-up Asura Priest still in its zone,
 *            the target is recorded, and while the opponent controls a monster it has not attacked, its attack mark
 *            (attackedMask) is cleared so it may attack again (as Black Luster Soldier's second attack does)
 *   targets  ModAttackTargetBlocked: the human's target pick (BattleStage_SelectTarget, card_menu_input.c) and the
 *            CPU's attack plans (AiFindAttackTarget, ai_picks.c) refuse a monster it has attacked; with the mark still
 *            set after its last monster, it cannot follow with a direct attack, and after a direct attack it is done */
#ifdef HAS_CARD_ASURA_PRIEST
static int IsAsuraAttacker(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    return id != 0 && z->isFaceUp && CardNumberOfId(id) == CARD_ASURA_PRIEST;
}

/* 1 if the Asura Priest of `player` in `zone` already attacked the monster now in the opponent's zone `target`. */
static int AsuraAttackedMonster(int player, int zone, int target)
{
    return gModDuel.asuraTurn == (u16)(gDuel.turnCount + 1) && (gModDuel.asuraTargets[zone] & (1 << target))
           && gModDuel.asuraSerials[zone][target] == ZoneOf(1 - (player & 1), target)->serial;
}

static void AsuraEndAttack(int player)
{
    int zone = gBattle.atkSlot, opponent = 1 - (player & 1), i;

    if (gBattle.direct || !IsAsuraAttacker(player, zone) || gModDuel.asuraTurn != (u16)(gDuel.turnCount + 1)
        || DUEL_CARD_ID(&ZoneOf(player, zone)->card) != (gBattle.side[player].cardId & CARD_ID_MASK))
        return;
    gModDuel.asuraTargets[zone] |= 1 << gBattle.defSlot;
    gModDuel.asuraSerials[zone][gBattle.defSlot] = gBattle.zoneSerialSnapshot[opponent];
    for (i = ZONE_MONSTER_0; i <= ZONE_MONSTER_4; i++) {
        if (DUEL_CARD_ID(&ZoneOf(opponent, i)->card) != 0 && !AsuraAttackedMonster(player, zone, i)) {
            gDuelPlayers[player & 1].attackedMask &= ~(1 << zone);
            return;
        }
    }
}
#endif

/* The attacker in (attacker, attackerZone) may not target the monster in (1 - attacker, zone): Marauding Captain's lock
 * (ModAttackTargetLocked) or a zone Asura Priest already attacked this Battle Phase. */
int ModAttackTargetBlocked(int attacker, int attackerZone, int zone)
{
    if (ModAttackTargetLocked(1 - (attacker & 1), zone))
        return 1;
#ifdef HAS_CARD_ASURA_PRIEST
    if (attackerZone >= ZONE_MONSTER_0 && attackerZone <= ZONE_MONSTER_4 && IsAsuraAttacker(attacker, attackerZone)
        && AsuraAttackedMonster(attacker, attackerZone, zone))
        return 1;
#endif
    return 0;
}

/* ---- Cipher Soldier ------------------------------------------------------------------------------------------------
 * "If this card battles a Warrior-Type monster, during damage calculation: This card gains 2000 ATK and DEF during that
 * damage calculation only."
 *   stats    GetZoneCardStats (card_stats.c) adds ModStatBonus, which has ModBattleStatBonus, to a face-up monster's
 *            ATK and DEF: while the
 *            battle is in BATTLE_STAGE_DAMAGE_CALC (CalcBattle runs there and keeps its results in gBattle, so the
 *            later stages need nothing), a face-up Cipher Soldier that is the attacker or the attack target gains 2000
 *            when the other monster is a Warrior (its printed Type: reading the other monster's stats from here would
 *            loop between two Cipher Soldiers)
 *   CPU      AiEvalAttack (ai_picks.c) adds ModCipherBonus to both sides of the battles it predicts */
#define CIPHER_BONUS 2000

/* 2000 if (player, zone) is a face-up Cipher Soldier and (otherPlayer, otherZone) a Warrior monster, else 0. */
int ModCipherBonus(int player, int zone, int otherPlayer, int otherZone)
{
#ifdef HAS_CARD_CIPHER_SOLDIER
    struct DuelZone *z = ZoneOf(player, zone), *o = ZoneOf(otherPlayer, otherZone);
    u16 id = DUEL_CARD_ID(&z->card), other = DUEL_CARD_ID(&o->card);

    if (id != 0 && z->isFaceUp && CardNumberOfId(id) == CARD_CIPHER_SOLDIER && other != 0
        && CARD_STATS_TYPE(gCardStats[other & CARD_ID_MASK]) == CARD_TYPE_WARRIOR)
        return CIPHER_BONUS;
#endif
    return 0;
}

/* The ATK and DEF a face-up monster gains in the battle being calculated now (Cipher Soldier). */
static int ModBattleStatBonus(int player, int zone)
{
    int attacker = gDuel.turnPlayer & 1;

    if (gDuel.phase != PHASE_BATTLE || gDuel.battleStage != BATTLE_STAGE_DAMAGE_CALC || gBattle.direct)
        return 0;
    if ((player & 1) == attacker && zone == gBattle.atkSlot)
        return ModCipherBonus(player, zone, 1 - attacker, gBattle.defSlot);
    if ((player & 1) != attacker && zone == gBattle.defSlot)
        return ModCipherBonus(player, zone, attacker, gBattle.atkSlot);
    return 0;
}

/* ---- Gyaku-Gire Panda ---------------------------------------------------------------------------------------------
 * "Increase the ATK of this card by 500 points for each monster on your opponent's side of the field. When this card
 * attacks with an ATK that is higher than the DEF of your opponent's Defense Position monster, inflict the difference
 * as Battle Damage to your opponent's Life Points."
 *   ATK       ModStatBonus: 500 for each monster the opponent controls, face up or down (CountMonsters)
 *   piercing  ModHasPiercing lists it (CalcBattle, campaign.c) */
#define PANDA_ATK_PER_MONSTER 500

/* GetZoneCardStats (card_stats.c, step 4a, a face-up monster): the ATK it gains in the low halfword and the DEF in the
 * high one (signed): Gyaku-Gire Panda's ATK, and Cipher Soldier's battle bonus (ModBattleStatBonus). */
int ModStatBonus(int player, int zone)
{
    int atk = 0, def = 0, battle = ModBattleStatBonus(player, zone);

    atk += battle;
    def += battle;
#ifdef HAS_CARD_GYAKU_GIRE_PANDA
    if (CardNumberOfId(DUEL_CARD_ID(&ZoneOf(player, zone)->card)) == CARD_GYAKU_GIRE_PANDA)
        atk += PANDA_ATK_PER_MONSTER * CountMonsters(1 - (player & 1));
#endif
#ifdef HAS_CARD_EXARION_UNIVERSE
    if (CardNumberOfId(DUEL_CARD_ID(&ZoneOf(player, zone)->card)) == CARD_EXARION_UNIVERSE) {
        int stacks = gModDuel.exarionPenaltyStacks[(player & 1) * 5 + zone];

        atk -= EXARION_ATK_COST * stacks;
    }
#endif
    return (atk & 0xFFFF) | (def << 16);
}

/* ---- The end of each attack ---------------------------------------------------------------------------------------
 * BattleStage_EndAttack (battle_phase3.c) calls ModEndAttack first, once each attack (and its Damage Step) is over:
 *   Injection Fairy Lily   its 3000 ATK go (InjectionFairyLilyEndAttack)
 *   Spear Dragon           "If this card attacks, it is changed to Defense Position at the end of the Damage Step.":
 *                          the attacker, still face up in Attack Position in its zone, turns to Defense Position
 *                          (ChangeBattlePosition), as Goblin Attack Force does at the end of the Battle Phase. Its
 *                          other effect, piercing damage, is ModHasPiercing's.
 *   Asura Priest           may attack again while an opponent monster it has not attacked remains (AsuraEndAttack) */
void ModEndAttack(int player)
{
#ifdef HAS_CARD_SPEAR_DRAGON
    struct DuelZone *z = ZoneOf(player, gBattle.atkSlot);
    u16 id = DUEL_CARD_ID(&z->card);
#endif

#ifdef HAS_CARD_EXARION_UNIVERSE
    gModDuel.exarionPierce = 0;
#endif
#ifdef HAS_CARD_INJECTION_FAIRY_LILY
    InjectionFairyLilyEndAttack(player);
#endif
#ifdef HAS_CARD_ASURA_PRIEST
    AsuraEndAttack(player);
#endif
#ifdef HAS_CARD_SPEAR_DRAGON
    if (id != 0 && id == (gBattle.side[player].cardId & CARD_ID_MASK) && z->isFaceUp && !z->isDefense
        && CardNumberOfId(id) == CARD_SPEAR_DRAGON) {
        ShowCardEffect(player, id);
        ChangeBattlePosition(player, gBattle.atkSlot, 0, 0);
    }
#endif
}

/* ---- The added cards' effect-target lists -----------------------------------------------------------------------
 * CollectEffectTargets (effect_target_collect.c) calls ModCollectEffectTargets for a card number it has no case for;
 * its filters (Toon monsters without Toon World, planted cards, Prohibition) then run as for its own lists. The
 * lists fill gCardListView (cards, sources, count). */
static void ModAddTarget(u32 word, u16 source)
{
    gCardListView.cards[gCardListView.count] = word;
    gCardListView.sources[gCardListView.count] = source;
    gCardListView.count++;
}

/* Add `player`'s cards of a pile (`count` card words) that `accept` takes. */
static void ModAddPileTargets(const struct DuelCard *pile, int count, u16 source, int (*accept)(u16 id, int param),
                              int param)
{
    int i;

    for (i = 0; i < count; i++) {
        u16 id = DUEL_CARD_ID(&pile[i]);

        if (accept(id, param))
            ModAddTarget(*(const u32 *)&pile[i], source);
    }
}

static int IsMonsterId(u16 id, int param)
{
    return CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) <= CARD_TYPE_REPTILE;
}

static int IsNumberId(u16 id, int number)
{
    u16 n = CardNumberOfId(id);

    return n == number || (number == CARD_DARK_MAGICIAN && n == CARD_DARK_MAGICIAN_ALT);
}

static int IsTypeId(u16 id, int type)
{
    return (int)CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) == type;
}

static int IsWeakDragonId(u16 id, int param)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];

    return CARD_STATS_TYPE(stats) == CARD_TYPE_DRAGON && CARD_STATS_ATK(stats) * CARD_STATS_POINTS_SCALE <= 1500;
}

/* A monster of Level 4 or lower that can be Special Summoned from the hand: not a Fusion or Ritual monster. */
static int IsLowLevelMainDeckMonsterId(u16 id, int param)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];

    return CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_LEVEL(stats) <= 4
           && CARD_STATS_KIND(stats) != CARD_KIND_FUSION && CARD_STATS_KIND(stats) != CARD_KIND_RITUAL
           && !IsSpecialSummonOnly(id);
}

/* CollectEffectTargets calls this after its own filters: a list whose card Special Summons the pick loses the monsters
 * that cannot be Special Summoned (IsCannotSpecialSummon: Tsukuyomi, Yata-Garasu). The summon itself refuses them
 * (QueueSpecialSummon*), but only after the effect took the card out of its pile, so the card vanished (Monster Reborn
 * on Yata-Garasu; also Mystic Tomato on Tsukuyomi, Flying Kamakiri #1 or Marauding Captain on either). The search,
 * banish and return lists (Sangan, Witch of the Black Forest, Kycoo, ...) keep them. */
static int IsSummoningTargetList(u16 cardNumber)
{
    switch (cardNumber) {
    case CARD_MONSTER_REBORN:
    case CARD_LAST_WILL:
    case CARD_GIANT_RAT:
    case CARD_UFO_TURTLE:
    case CARD_SHINING_FAIRY:
    case CARD_MOTHER_GRIZZLY:
    case CARD_FLYING_KAMAKIRI_1:
    case CARD_MYSTIC_TOMATO:
    case CARD_APPRENTICE_MAGICIAN:
    case CARD_PYRAMID_TURTLE:
    case CARD_GRAVEKEEPERS_SPY:
    case CARD_CALL_OF_THE_HAUNTED:
    case CARD_THE_SHALLOW_GRAVE:
    case CARD_PREMATURE_BURIAL:
    case CARD_1520:                         /* Gilasaurus: the opponent's revival */
    case CARD_SPEAR_CRETIN:
    case CARD_1318:
#ifdef HAS_CARD_MARAUDING_CAPTAIN
    case CARD_MARAUDING_CAPTAIN:
#endif
#ifdef HAS_CARD_BOOK_OF_LIFE
    case CARD_BOOK_OF_LIFE:
#endif
        return 1;
    }
    return 0;
}

void ModFilterEffectTargets(u16 cardNumber)
{
    int i, j;

    if (!IsSummoningTargetList(cardNumber))
        return;
    for (i = 0; i < gCardListView.count;) {
        if (!IsCannotSpecialSummon(gCardListView.cards[i] & CARD_ID_MASK)) {
            i++;
            continue;
        }
        for (j = i; j + 1 < gCardListView.count; j++) {
            gCardListView.cards[j] = gCardListView.cards[j + 1];
            gCardListView.sources[j] = gCardListView.sources[j + 1];
        }
        gCardListView.count--;
    }
}

void ModCollectEffectTargets(int player, u16 cardNumber, int param)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    struct DuelPlayer *o = &gDuelPlayers[(1 - player) & 1];

    switch (cardNumber) {
#ifdef HAS_CARD_SKILLED_DARK_MAGICIAN
    case CARD_SKILLED_DARK_MAGICIAN:        /* Dark Magician in the hand, then the Deck, then the Graveyard */
        ModAddPileTargets(p->hand, p->handCount, CARDLIST_SRC_HAND, IsNumberId, CARD_DARK_MAGICIAN);
        ModAddPileTargets(p->deck, p->deckCount, CARDLIST_SRC_DECK, IsNumberId, CARD_DARK_MAGICIAN);
        ModAddPileTargets(p->graveyard, p->graveCount, CARDLIST_SRC_GRAVEYARD, IsNumberId, CARD_DARK_MAGICIAN);
        break;
#endif
#ifdef HAS_CARD_BOOK_OF_LIFE
    case CARD_BOOK_OF_LIFE:                 /* param 0: the player's Graveyard Zombies; 1: the opponent's monsters */
        if (param)
            ModAddPileTargets(o->graveyard, o->graveCount, CARDLIST_SRC_GRAVEYARD, IsMonsterId, 0);
        else
            ModAddPileTargets(p->graveyard, p->graveCount, CARDLIST_SRC_GRAVEYARD, IsTypeId, CARD_TYPE_ZOMBIE);
        break;
#endif
#ifdef HAS_CARD_POT_OF_AVARICE
    case CARD_POT_OF_AVARICE:               /* the player's Graveyard monsters */
        ModAddPileTargets(p->graveyard, p->graveCount, CARDLIST_SRC_GRAVEYARD, IsMonsterId, 0);
        break;
#endif
#ifdef HAS_CARD_MONSTER_REINCARNATION
    case CARD_MONSTER_REINCARNATION:        /* the player's Graveyard monsters */
        ModAddPileTargets(p->graveyard, p->graveCount, CARDLIST_SRC_GRAVEYARD, IsMonsterId, 0);
        break;
#endif
#ifdef HAS_CARD_HYDROGEDDON
    case CARD_HYDROGEDDON:                  /* Hydrogeddons in the Deck */
        ModAddPileTargets(p->deck, p->deckCount, CARDLIST_SRC_DECK, IsNumberId, CARD_HYDROGEDDON);
        break;
#endif
#ifdef HAS_CARD_MASKED_DRAGON
    case CARD_MASKED_DRAGON:                /* Deck Dragons with 1500 or less ATK */
        ModAddPileTargets(p->deck, p->deckCount, CARDLIST_SRC_DECK, IsWeakDragonId, 0);
        break;
#endif
#ifdef HAS_CARD_MARAUDING_CAPTAIN
    case CARD_MARAUDING_CAPTAIN:            /* the hand's Level 4 or lower monsters (not Fusion or Ritual ones) */
        ModAddPileTargets(p->hand, p->handCount, CARDLIST_SRC_HAND, IsLowLevelMainDeckMonsterId, 0);
        break;
#endif
#ifdef HAS_CARD_MAGIC_DRAIN
    case CARD_MAGIC_DRAIN:                  /* the player's hand Spell Cards (to discard against Magic Drain) */
        ModAddPileTargets(p->hand, p->handCount, CARDLIST_SRC_HAND, IsTypeId, CARD_TYPE_MAGIC);
        break;
#endif
#ifdef HAS_CARD_MAGICAL_SCIENTIST
    case CARD_MAGICAL_SCIENTIST:            /* the Fusion Deck's Level 6 or lower Fusion Monsters */
        ModAddPileTargets(p->fusionDeck, p->fusionCount, CARDLIST_SRC_FUSION_DECK, IsScientistFusion, 0);
        break;
#endif
#ifdef HAS_CARD_DIFFERENT_DIMENSION_CAPSULE
    case CARD_DIFFERENT_DIMENSION_CAPSULE:    /* any main-Deck card (Set in a Spell & Trap zone) */
        ModAddPileTargets(p->deck, p->deckCount, CARDLIST_SRC_DECK, IsDifferentDimensionCapsuleDeckTarget, 0);
        break;
#endif
    }
}

#ifdef HAS_CARD_MAGIC_DRAIN
/* ---- Magic Drain --------------------------------------------------------------------------------------------------
 * "When your opponent activates a Spell Card: They can discard 1 Spell Card to negate this card's effect, otherwise
 * negate the activation of their Spell Card, and if you do, destroy it." (Counter Trap)
 * The prepare wants the opponent's Spell as the link it answers (as Magic Jammer's, without the discard cost). The
 * resolve: the Spell's player, if they hold a Spell Card (CollectEffectTargets: the hand's Magic cards), may discard
 * one to stop it. The human is asked (Yes/No) and picks it in the card list; the CPU always discards its first one.
 * Otherwise the Spell is negated and destroyed as by the original's Counter Traps (DUEL_CMD_NEGATE_ACTIVATION, then
 * DUEL_CMD_SET_SPELL_TRAP_DISABLED on its zone: EffectNegateChainedCardResolve). The CPU chains it to the human's
 * Spells (AiChainSolemnJudgment, before Solemn Judgment). */
static const u8 sStrMagicDrainPrompt[] = "Discard 1 Spell Card to negate Magic Drain?";

int CollectEffectTargetsInt(int player, int cardNumber, int param) asm("CollectEffectTargets");

int EffectMagicDrainPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    u16 id;

    if (fromHand || chainLink == NULL || chainLink->player == card->player)
        return 0;
    id = chainLink->card & CARD_ID_MASK;
    return CARD_STATS_TYPE(gCardStats[id]) == CARD_TYPE_MAGIC && CardNumberOfId(id) != CARD_1539;
}

#define MAGIC_DRAIN_STEP_DISCARD EFFECT_STEP_3
#define MAGIC_DRAIN_STEP_NEGATE EFFECT_STEP_4

int EffectMagicDrainResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int opponent, i;
    u32 word;

    if (link->negated || chainedTo == NULL || CARD_STATS_TYPE(gCardStats[chainedTo->card & CARD_ID_MASK])
                                                  != CARD_TYPE_MAGIC)
        return EFFECT_STEP_DONE;
    opponent = chainedTo->player;
    switch (gChain.effectStep) {
    case EFFECT_STEP_START:
        if (CollectEffectTargetsInt(opponent, CARD_MAGIC_DRAIN, 0) == 0)
            return MAGIC_DRAIN_STEP_NEGATE;
        if (opponent != 0) {
            gCardListView.cursorRow = 0;    /* the CPU discards its first Spell */
            gCardListView.top = 0;
            return MAGIC_DRAIN_STEP_DISCARD;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 7), TEXTBOX_FLAGS_DEFAULT, sStrMagicDrainPrompt);
        TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
        return EFFECT_STEP_2;
    case EFFECT_STEP_2:
        if (!gTextBox.result)
            return MAGIC_DRAIN_STEP_NEGATE;
        CardListView_Open(opponent, CARDLIST_AREA_EFFECT_TARGETS, CARD_MAGIC_DRAIN, 0);
        return MAGIC_DRAIN_STEP_DISCARD;
    case MAGIC_DRAIN_STEP_DISCARD:
        /* the picked Spell's hand index: its card word */
        word = gCardListView.cards[gCardListView.cursorRow + gCardListView.top];
        for (i = 0; i < gDuelPlayers[opponent & 1].handCount; i++) {
            if (*(u32 *)&gDuelPlayers[opponent & 1].hand[i] == word) {
                DiscardHandCard(opponent, i, 0, 1);
                return EFFECT_STEP_DONE;    /* Magic Drain's effect is negated */
            }
        }
        /* fall through: not found, the Spell is negated */
    case MAGIC_DRAIN_STEP_NEGATE:
        DuelCmd_Push(link->player ? DUEL_CMD_PLAYER | DUEL_CMD_NEGATE_ACTIVATION : DUEL_CMD_NEGATE_ACTIVATION, 1, 0, 0);
        DuelCmd_Push(chainedTo->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_SPELL_TRAP_DISABLED
                                       : DUEL_CMD_SET_SPELL_TRAP_DISABLED, chainedTo->zone, 1, 0);
        return EFFECT_STEP_DONE;
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_MAGIC_DRAIN */

#ifdef HAS_CARD_BLAST_WITH_CHAIN
/* ---- Blast with Chain ---------------------------------------------------------------------------------------------
 * "Target 1 face-up monster you control; equip this card to that target. It gains 500 ATK. If this card is destroyed
 * by a card effect while equipped: Target 1 card on the field; destroy that target."
 * An Equip Trap, as Kunai with Chain (tools/newcards.py's SUBTYPE_OVERRIDES: subtype Equip, so it stays on the field,
 * Chain_CardGoesToGrave). One row, told apart by the event:
 *   activation   the target is a face-up monster of its controller (the human's cursor; the CPU takes the attacked
 *                monster when it answers an attack on one of its own, else its strongest face-up one); the resolve
 *                equips it (EquipCard) as Kunai's does, or destroys the card when the target is gone. +500 ATK:
 *                CalcZoneCardStats' Equip case (card_stats.c), beside Kunai's.
 *   destroyed    OnCardDestroyedByEffect (effect_hooks.c) calls BlastWithChainDestroyed for every card an effect
 *                destroys: a face-up Blast with Chain that equips a monster (FindMonsterWithLinkTo) queues its
 *                trigger from the Graveyard (RESPONSE_TRAP_TO_GRAVE). An Equip lost with its monster is not
 *                destroyed by an effect, so it does not go off. The target is any card on the field (the CPU: the
 *                human's strongest monster, else a Spell/Trap of the human's); it is destroyed.
 * The CPU activates it in answer to an attack while it has a face-up monster (AiShouldActivateAddedCard). */
static int CountFaceUpMonsters(int player);     /* below */

static const u8 sStrBlastWithChainEquip[] = "Select a face-up monster you control to equip.";
static const u8 sStrBlastWithChainDestroy[] = "Select a card on the field to destroy.";

static inline int IsBlastWithChainTrigger(struct ChainEntry *link)
{
    return link->event == RESPONSE_TRAP_TO_GRAVE;
}

/* Queue the "destroyed while equipped" trigger of the Blast with Chain in (player, zone) (mandatory, from the
 * Graveyard, in the next chain). */
static void QueueBlastWithChainTrigger(int player, u16 id)
{
    Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_KIND(CHAIN_KIND_OFF_FIELD)
                         | TRIGGER_EVENT(RESPONSE_TRAP_TO_GRAVE) | (id & CARD_ID_MASK),
                     0);
}

void BlastWithChainDestroyed(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    if (zone < ZONE_SPELL_0 || zone > ZONE_SPELL_4 || id == 0 || !z->isFaceUp
        || CardNumberOfId(id) != CARD_BLAST_WITH_CHAIN || FindMonsterWithLinkTo(player, zone) == 0xFFFF)
        return;
    QueueBlastWithChainTrigger(player, id);
}

/* EquipCard (duel_send_to_grave.c): Gearfried the Iron Knight (key 1351) destroyed the Equip in (player, zone) as soon
 * as it was equipped (in Goat Format its effect is continuous). A Blast with Chain was equipped and is destroyed by a
 * card effect, so it pops; its equip link command has not run yet, so FindMonsterWithLinkTo is not asked. */
void BlastWithChainEquipDestroyed(int player, int zone)
{
    u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);

    if (CardNumberOfId(id) == CARD_BLAST_WITH_CHAIN)
        QueueBlastWithChainTrigger(player, id);
}

/* AiChainSolemnJudgment: the human's Spell/Trap `entry` would destroy a Set Blast with Chain of the CPU's (it targets
 * one, or it is Heavy Storm / Harpie's Feather Duster): the CPU chains it onto a face-up monster of its own, so the
 * destruction pops it. 1 if it chained it. */
static int AiChainBlastWithChain(struct ChainEntry *entry)
{
    int zone, hit = 0;

    if (CountFaceUpMonsters(AI_PLAYER) == 0)
        return 0;
    switch (CardNumberOfId(entry->card)) {
    case CARD_HEAVY_STORM:
    case CARD_HARPIES_FEATHER_DUSTER:
        hit = 1;
        break;
    default:
        if (entry->numTargets >= 1 && (u8)entry->targets[0] == AI_PLAYER) {
            zone = entry->targets[0] >> 8;
            hit = zone >= ZONE_SPELL_0 && zone <= ZONE_SPELL_4
                  && CardNumberOfId(DUEL_CARD_ID(&ZoneOf(AI_PLAYER, zone)->card)) == CARD_BLAST_WITH_CHAIN;
        }
        break;
    }
    return hit && AiChainSetCard(entry, CARD_BLAST_WITH_CHAIN) != 0;
}

/* A card in (player, zone) of the field. */
static int IsFieldCard(int player, int zone)
{
    return zone <= ZONE_FIELD && DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0;
}

int EffectBlastWithChainPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    int zone;

    if (IsBlastWithChainTrigger(card)) {
        for (zone = 0; zone <= ZONE_FIELD; zone++) {
            if (IsFieldCard(0, zone) || IsFieldCard(1, zone))
                return 1;
        }
        return 0;
    }
    if (fromHand)
        return 0;
    return CountFaceUpMonsters(card->player) > 0;
}

u16 EffectBlastWithChainCheck(struct ChainEntry *card, u16 pos)
{
    int player = (u8)pos, zone = pos >> 8;
    struct DuelZone *z;

    if (IsBlastWithChainTrigger(card))
        return IsFieldCard(player, zone);
    if (player != card->player || zone > ZONE_MONSTER_4)
        return 0;
    z = ZoneOf(player, zone);
    return DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp;
}

/* The CPU's picks. */
static void AiBlastWithChainTarget(struct ChainEntry *link)
{
    int zone, best = -1, bestAtk = -1;

    link->numTargets = 0;
    if (IsBlastWithChainTrigger(link)) {
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (IsFieldCard(AI_HUMAN, zone) && (int)GetZoneCardAtk(AI_HUMAN, zone) > bestAtk) {
                best = zone;
                bestAtk = GetZoneCardAtk(AI_HUMAN, zone);
            }
        }
        for (zone = ZONE_SPELL_0; best < 0 && zone <= ZONE_FIELD; zone++) {
            if (IsFieldCard(AI_HUMAN, zone))
                best = zone;
        }
        if (best >= 0)
            TryAddEffectTarget(link, AI_HUMAN, best);
        return;
    }
    /* answering an attack on one of its face-up monsters: that one */
    if (link->event == RESPONSE_ATTACK_DECLARED && (u8)link->loc1 == AI_PLAYER) {
        zone = link->loc1 >> 8;
        if (zone <= ZONE_MONSTER_4 && EffectBlastWithChainCheck(link, DUEL_LOC(AI_PLAYER, zone))
            && TryAddEffectTarget(link, AI_PLAYER, zone))
            return;
    }
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (EffectBlastWithChainCheck(link, DUEL_LOC(AI_PLAYER, zone))
            && (int)GetZoneCardAtk(AI_PLAYER, zone) > bestAtk) {
            best = zone;
            bestAtk = GetZoneCardAtk(AI_PLAYER, zone);
        }
    }
    if (best >= 0)
        TryAddEffectTarget(link, AI_PLAYER, best);
}

int EffectBlastWithChainChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    u32 mask;

    if (link->player) {
        AiBlastWithChainTarget(link);
        return 1;
    }
    switch (gChain.targetStep) {
    case 0:
        link->numTargets = 0;
        TextBoxOpen(0x206, 0x712, TEXTBOX_FLAGS_DEFAULT,
                    IsBlastWithChainTrigger(link) ? sStrBlastWithChainDestroy : sStrBlastWithChainEquip);
        gChain.targetStep++;
        return 0;
    default:
        mask = IsBlastWithChainTrigger(link) ? PICK_BOTH_SIDES(PICK_ANY_MONSTER | PICK_ANY_SPELL_TRAP)
                                             : PICK_FACE_UP_MONSTER_ANY;
        if (DuelCursor_PickTarget(mask) != 0) {
            if (TryAddEffectTarget(link, gDuelScreen.selPlayer, gDuelScreen.selArea + gDuelScreen.selIndex) != 0)
                return 1;
            PlaySE(SE_ERROR);
        }
        return 0;
    }
}

int EffectBlastWithChainResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int player, zone;
    struct DuelZone *z;

    if (link->negated || link->numTargets != 1)
        return EFFECT_STEP_DONE;
    player = (u8)link->targets[0];
    zone = link->targets[0] >> 8;
    z = ZoneOf(player, zone);
    if (IsBlastWithChainTrigger(link)) {
        if (DUEL_CARD_ID(&z->card) == 0)
            return EFFECT_STEP_DONE;
        if (link->player != player)
            DuelCmd_Push(player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_DESTROYED_BY_OPPONENT_FLAG
                                : DUEL_CMD_SET_DESTROYED_BY_OPPONENT_FLAG, zone, 1, 0);
        DestroyFieldCardByEffect(player, zone);
        OnCardDestroyedByEffect(link->player, player, zone);
        return EFFECT_STEP_DONE;
    }
    /* the equip, as Kunai with Chain's */
    if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp && link->player == player)
        EquipCard(link->player, link->player | (link->zone << 8), link->targets[0]);
    else
        DestroyFieldCard(link->player, link->zone, 1);
    return EFFECT_STEP_DONE;
}
#else
void BlastWithChainDestroyed(int player, int zone)
{
}

void BlastWithChainEquipDestroyed(int player, int zone)
{
}
#endif /* HAS_CARD_BLAST_WITH_CHAIN */

/* ---- The CPU's Tribute Summon upgrades ----------------------------------------------------------------------------
 * AiChooseSummonWithTribute (ai_summon.c) takes a Tribute Summon only when its battle simulation deals more damage
 * (or as much while destroying more). A wall swapped for a stronger wall changes neither, so the CPU kept Aqua Madoor
 * (1200/2000) rather than Tribute it for Gyakutenno Megami (1800/2000). Added: the summon is also better, when its
 * simulated damage is no worse, if it is an upgrade: every monster it Tributes (its current ATK / DEF) has less ATK and
 * no more DEF than the summoned monster's printed ones. tributeMask is AiSimSummon's: each nonzero nibble names a
 * tribute's monster zone in bits 0-2. */
int AiTributeIsUpgrade(u16 cardId, u16 tributeMask)
{
    u32 stats = gCardStats[cardId & CARD_ID_MASK];
    int atk = CARD_STATS_ATK(stats) * CARD_STATS_POINTS_SCALE, def = CARD_STATS_DEF(stats) * CARD_STATS_POINTS_SCALE;
    int any = 0, i;

    for (i = 0; i < 4; i++, tributeMask >>= 4) {
        struct ZoneCardStats tribute;
        int zone = tributeMask & 7;

        if ((tributeMask & 0xF) == 0)
            continue;
        if (DUEL_CARD_ID(&ZoneOf(AI_PLAYER, zone)->card) == 0)
            return 0;
        GetZoneCardStats(AI_PLAYER, zone, &tribute);
        if ((int)tribute.atk >= atk || (int)tribute.def > def)
            return 0;
        any = 1;
    }
    return any;
}

/* ---- Added Fusion recipes -----------------------------------------------------------------------------------------
 * The added cards' recipes of 2 or 3 materials (materials[2] 0 for 2). effect_fusion.c asks for them after the ROM's
 * gFusionRecipes2 / gFusionRecipes3: FindFusionMaterials, IsMaterialOfFusion, and the Polymerization material count
 * (IsModFusionResult). They take no Fusion substitute ("A Fusion Summon of this card can only be done with the above
 * Fusion Materials"). */
static const struct FusionRecipe sModFusionRecipes[] = {
#if defined(HAS_CARD_CYBER_TWIN_DRAGON) && defined(HAS_CARD_CYBER_DRAGON)
    { CARD_CYBER_TWIN_DRAGON, { CARD_CYBER_DRAGON, CARD_CYBER_DRAGON, 0 } },
#endif
#if defined(HAS_CARD_CYBER_END_DRAGON) && defined(HAS_CARD_CYBER_DRAGON)
    { CARD_CYBER_END_DRAGON, { CARD_CYBER_DRAGON, CARD_CYBER_DRAGON, CARD_CYBER_DRAGON } },
#endif
    { 0, { 0, 0, 0 } },
};

/* The added recipe whose result is Fusion monster number `fusionNum`, or NULL. */
static const struct FusionRecipe *ModFusionRecipeOf(u16 fusionNum)
{
    u32 i;

    for (i = 0; i + 1 < ARRAY_COUNT(sModFusionRecipes); i++) {
        if (sModFusionRecipes[i].result == fusionNum)
            return &sModFusionRecipes[i];
    }
    return NULL;
}

/* The number of materials of an added recipe: 2 or 3; 0 if Fusion monster number `fusionNum` has none. */
int IsModFusionResult(u16 fusionNum)
{
    const struct FusionRecipe *recipe = ModFusionRecipeOf(fusionNum);

    if (recipe == NULL)
        return 0;
    return recipe->materials[2] != 0 ? 3 : 2;
}

/* IsMaterialOfFusion for an added recipe: 1 if `materialNum` is one of its materials (no substitute). */
int ModIsMaterialOfFusion(u16 fusionNum, u16 materialNum)
{
    const struct FusionRecipe *recipe = ModFusionRecipeOf(fusionNum);
    int i;

    if (recipe == NULL || materialNum == 0)
        return 0;
    for (i = 0; i < 3; i++) {
        if (recipe->materials[i] == materialNum)
            return 1;
    }
    return 0;
}

/* FindFusionMaterials for an added recipe, after the ROM's tables: the player's materials (each a different card; the
 * field, then the hand, as FindFusionMaterialSlot looks), none a substitute, and room for the Fusion monster. The
 * slots go in FindFusionMaterials' order: slots[0] is the recipe's last material. 1 if found. */
int ModFindFusionMaterials(int player, u16 fusionNum, u16 *slots)
{
    const struct FusionRecipe *recipe = ModFusionRecipeOf(fusionNum);
    int count, i, allInHand = 1;

    if (recipe == NULL)
        return 0;
    count = recipe->materials[2] != 0 ? 3 : 2;
    for (i = 0; i < count; i++) {
        u16 excludeA = i >= 1 ? slots[count - 1] : FUSION_SLOT_NONE;
        u16 excludeB = i >= 2 ? slots[count - 2] : FUSION_SLOT_NONE;
        u16 slot = FindFusionMaterialSlot(player, recipe->materials[i], excludeA, excludeB);

        if (slot == FUSION_SLOT_NONE || IsFusionSubstitute(CardNumberOfId(GetFusionSlotCardId(player, slot))))
            return 0;
        slots[count - 1 - i] = slot;
        if (!(slot & FUSION_SLOT_HAND))
            allInHand = 0;
    }
    if (allInHand && CountMonsters(player) == 5)
        return 0;
    return 1;
}

/* CalcBattle (campaign.c): 1 if the attacking card inflicts piercing battle damage by itself (Airknight Parshath,
 * Cyber End Dragon, Spear Dragon). */
int ModHasPiercing(u16 cardId)
{
    switch (CardNumberOfId(cardId)) {
#ifdef HAS_CARD_AIRKNIGHT_PARSHATH
    case CARD_AIRKNIGHT_PARSHATH:
#endif
#ifdef HAS_CARD_CYBER_END_DRAGON
    case CARD_CYBER_END_DRAGON:
#endif
#ifdef HAS_CARD_SPEAR_DRAGON
    case CARD_SPEAR_DRAGON:
#endif
#ifdef HAS_CARD_GYAKU_GIRE_PANDA
    case CARD_GYAKU_GIRE_PANDA:
#endif
        return 1;
#ifdef HAS_CARD_EXARION_UNIVERSE
    case CARD_EXARION_UNIVERSE:
        return gModDuel.exarionPierce;
#endif
    }
    return 0;
}

/* ---- The CPU's recruiters summon another copy ---------------------------------------------------------------------
 * EffectSpecialSummonFromDeckResolve (effect_resolve6.c: Mystic Tomato, Giant Rat, UFO Turtle, Shining Fairy, Mother
 * Grizzly, Flying Kamakiri #1, and the added Pyramid Turtle, Apprentice Magician, Hydrogeddon) asks this before
 * AiPickCardListEntry for the CPU: a copy of the recruiter itself in the Deck list, so the next one can recruit again.
 * -1 for none, and always with the Exodia strategy (AI_FLAG_EXODIA: Mystic Tomato digs for Sangan / the Witch). */
int AiRecruiterCopyEntry(u16 cardId)
{
    u16 number = CardNumberOfId(cardId);
    int count, i;

    if (gDuelCtrl.aiFlags & AI_FLAG_EXODIA)
        return -1;
    count = CollectEffectTargetsInt(AI_PLAYER, number, 0);
    for (i = 0; i < count; i++) {
        if (CardNumberOfId(gCardListView.cards[i] & CARD_ID_MASK) == number)
            return i;
    }
    return -1;
}

/* ---- The CPU's Set step: strong monsters go face up ------------------------------------------------------------------
 * AiStepMainPhase's AI_MAIN_SET_MONSTER (ai_steps.c) Sets the monster it picks (on the first turn every summon goes
 * there, and later when no summon simulation is better). Changed: a monster with AI_SUMMON_ATTACK_ATK (1400) or more
 * printed ATK and no flip effect is Normal Summoned face up in Attack Position instead (with or without a Tribute),
 * unless the human controls a face-up monster with more ATK, which would run it over. (Until 2026-10-30: 1700 or more,
 * and only while the human controlled no monster.) */
#define AI_SUMMON_ATTACK_ATK 1400

int AiSummonFaceUpOnOpenField(u16 id)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];
    u16 number = CardNumberOfId(id);
    int atk = CARD_STATS_ATK(stats) * CARD_STATS_POINTS_SCALE, zone;

    if (CARD_STATS_TYPE(stats) > CARD_TYPE_REPTILE || atk < AI_SUMMON_ATTACK_ATK)
        return 0;
    if (HasFlipEffect(number, 0) || HasFlipEffect(number, 1))
        return 0;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(AI_HUMAN, zone);

        if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp && (int)GetZoneCardAtk(AI_HUMAN, zone) > atk)
            return 0;
    }
    return 1;
}

#ifdef HAS_CARD_SAND_MOTH
/* ---- Sand Moth ----------------------------------------------------------------------------------------------------
 * "When this face-down Defense Position card is destroyed and sent to the Graveyard, except by battle, switch the
 * original ATK and DEF of this card and Special Summon it."
 *   trigger  SendFieldCardToGrave (duel_send_to_grave.c; battle uses SendBattleDestroyedCardToGraveyard) calls
 *            SandMothDestroyed for a destroyed (not Tributed or sent) face-down Defense Position Sand Moth that goes
 *            to the Graveyard (not banished): a trigger from the Graveyard (CHAIN_KIND_OFF_FIELD), mandatory.
 *   resolve  it leaves the Graveyard and is Special Summoned, the player choosing its face-up position;
 *            gModDuel.sandMothPending marks the summon, and AddedCardsSummoned gives the summoned Sand Moth a
 *            ZONE_LINK_SWAP_STATS link from its own ID.
 *   stats    CalcZoneCardStats (card_stats.c) swaps the printed ATK and DEF of a face-up monster with that link
 *            (ModHasSwappedStats) before its modifiers: 2000/1000. Turned face down, it loses the link
 *            (OnMonsterTurnedFaceDown). */
void SandMothDestroyed(int player, u16 id)
{
    Chain_AddPending(TRIGGER_PLAYER(player & 1) | TRIGGER_KIND(CHAIN_KIND_OFF_FIELD)
                         | TRIGGER_EVENT(RESPONSE_MONSTER_TO_GRAVE) | (id & CARD_ID_MASK),
                     0);
}

int EffectSandMothResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (link->negated || CountFreeMonsterZones(link->player) == 0 || !CanSpecialSummon(link->player))
        return EFFECT_STEP_DONE;
    if (!RemoveGraveyardCardByNumber(link->player, CARD_SAND_MOTH, (u32 *)&gChain.effectCard))
        return EFFECT_STEP_DONE;
    ShowCardEffect(link->player, link->card & CARD_ID_MASK);
    gModDuel.sandMothPending |= 1 << (link->player & 1);
    QueueSpecialSummonChoosePosition(link->player, &gChain.effectCard, 1, ZONE_STATUS_FROM_GRAVEYARD);
    return EFFECT_STEP_DONE;
}

/* AddedCardsSummoned: the Sand Moth its own effect summons gets its swapped stats. */
void SandMothSummoned(int player, int zone, u16 id)
{
    if (CardNumberOfId(id) != CARD_SAND_MOTH || !(gModDuel.sandMothPending & (1 << (player & 1))))
        return;
    gModDuel.sandMothPending &= ~(1 << (player & 1));
    QueueAddZoneLink(player, id & CARD_ID_MASK, DUEL_LOC(player, zone), ZONE_LINK_KIND(ZONE_LINK_SWAP_STATS, 1));
}
#endif /* HAS_CARD_SAND_MOTH */

/* CalcZoneCardStats (card_stats.c): 1 if the monster in (player, zone) has a ZONE_LINK_SWAP_STATS link from its own
 * card ID (Sand Moth): its printed ATK and DEF are swapped. */
int ModHasSwappedStats(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);
    int i;

    for (i = 0; i < z->numLinks; i++) {
        if ((u8)z->linkKinds[i] == ZONE_LINK_SWAP_STATS && (z->links[i] & CARD_ID_MASK) == id)
            return 1;
    }
    return 0;
}

#ifdef HAS_CARD_TREEBORN_FROG
/* ---- Treeborn Frog ------------------------------------------------------------------------------------------------
 * "Once per turn, during your Standby Phase, if this card is in your Graveyard and you do not control "Treeborn
 * Frog": You can Special Summon this card. You must control no Spell/Trap Cards to activate and to resolve this
 * effect."
 * DuelPhase_Standby (duel_phases.c) runs TreebornFrogStandbyStep right after Sinister Serpent's step, in the same way
 * (on gDuel.phaseSubStep, not a chain link; once per turn, as the Standby Phase runs once):
 *   step 0  done unless the player has one in the Graveyard, no face-up Treeborn Frog, no Spell/Trap card (the Field
 *           Spell included), a free monster zone and CanSpecialSummon. The CPU says Yes; the human is asked.
 *   step 1  on Yes, with still no Spell/Trap card: it leaves the Graveyard (RemoveGraveyardCardByNumber) and is
 *           Special Summoned face up, the player choosing its position (QueueSpecialSummonChoosePosition). */
static const u8 sStrTreebornFrogPrompt[] = "Special Summon Treeborn Frog from the Graveyard?";

static int TreebornFrogCanSummon(int player)
{
    return CountGraveyardCardsByNumber(player, CARD_TREEBORN_FROG) > 0
           && CountFaceUpMonstersByNumber(player, CARD_TREEBORN_FROG) == 0
           && CountSpellTrapsFiltered(player, 0, 0, 1) == 0 && CountFreeMonsterZones(player) > 0
           && CanSpecialSummon(player);
}

int TreebornFrogStandbyStep(int player)
{
    struct DuelCard card;

    switch (gDuel.phaseSubStep) {
    case 0:
        if (!TreebornFrogCanSummon(player))
            return 1;
        if (player != 0) {
            gTextBox.result = 1;            /* the CPU always summons it */
        } else {
            TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 7), TEXTBOX_FLAGS_DEFAULT, sStrTreebornFrogPrompt);
            TextBoxSetMenu(TEXTBOX_MENU_YES_NO, NULL, NULL);
        }
        break;
    case 1:
        if (gTextBox.result != 0 && TreebornFrogCanSummon(player)
            && RemoveGraveyardCardByNumber(player, CARD_TREEBORN_FROG, (u32 *)&card)) {
            ShowCardEffect(player, gCardNumberToId[CARD_TREEBORN_FROG]);
            QueueSpecialSummonChoosePosition(player, &card, 1, ZONE_STATUS_FROM_GRAVEYARD);
        }
        break;
    default:
        return 1;
    }
    gDuel.phaseSubStep++;
    return 0;
}
#else
int TreebornFrogStandbyStep(int player)
{
    return 1;
}
#endif /* HAS_CARD_TREEBORN_FROG */

#ifdef HAS_CARD_MIRAGE_OF_NIGHTMARE
/* ---- Mirage of Nightmare ------------------------------------------------------------------------------------------
 * "Once per turn, during your opponent's Standby Phase: Draw cards until you have 4 cards in your hand. If you do,
 * during your next Standby Phase after that: Randomly discard the same number of cards you drew (or your entire hand,
 * if you do not have enough cards)." (Continuous Spell)
 * Its activation does nothing (EffectMirageOfNightmareResolve; a Continuous Spell stays on the field). The rest runs
 * in ModStandbyStep, in every Standby Phase (`player` is the turn player), on gDuel.phaseSubStep:
 *   discard  the turn player's draws from their last Mirage draw (gModDuel.mirageDrawn) are discarded at random, one
 *            per step, while a face-up Mirage of Nightmare is still on their field (destroyed in between, there is
 *            nothing to discard); the count is cleared either way
 *   draw     the other player, with a face-up Mirage of Nightmare and fewer than 4 cards in hand, draws up to 4
 *            (mandatory, nobody is asked); the number drawn is kept for their next Standby Phase */

int EffectMirageOfNightmareResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    return EFFECT_STEP_DONE;
}

/* The zone of a face-up Mirage of Nightmare of `player`, -1 for none. */
static int FaceUpMirageZone(int player)
{
    int zone;

    for (zone = ZONE_SPELL_0; zone <= ZONE_SPELL_4; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);

        if (z->isFaceUp && !z->isDisabled && CardNumberOfId(DUEL_CARD_ID(&z->card)) == CARD_MIRAGE_OF_NIGHTMARE)
            return zone;
    }
    return -1;
}

/* The discard part: 1 when done. */
static int MirageDiscardStep(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int zone = FaceUpMirageZone(player);

    if (gModDuel.mirageDrawn[player & 1] == 0)
        return 1;
    if (zone < 0 || p->handCount == 0) {
        gModDuel.mirageDrawn[player & 1] = 0;
        return 1;
    }
    if (gDuel.phaseSubStep == 0)
        ShowCardEffect(player, DUEL_CARD_ID(&ZoneOf(player, zone)->card));
    gDuel.phaseSubStep++;
    gModDuel.mirageDrawn[player & 1]--;
    DiscardHandCard(player, (u32)Random() % p->handCount, 0, 1);    /* one per step: the command runs in between */
    return 0;
}

/* The draw part (for the turn player's opponent): 1 when done. */
static int MirageDrawStep(int player)
{
    int owner = 1 - player;
    struct DuelPlayer *p = &gDuelPlayers[owner & 1];
    int zone = FaceUpMirageZone(owner), count = 4 - p->handCount;

    /* mandatory: no question */
    if (zone < 0 || count <= 0)
        return 1;
    ShowCardEffect(owner, DUEL_CARD_ID(&ZoneOf(owner, zone)->card));
    DrawCards(owner, count);
    gModDuel.mirageDrawn[owner & 1] += count;
    return 1;
}
#endif /* HAS_CARD_MIRAGE_OF_NIGHTMARE */

/* DuelPhase_Standby (duel_phases.c) runs this after Sinister Serpent's step, each frame until it returns 1: Treeborn
 * Frog's summon, then Mirage of Nightmare's discard and draw (gModDuel.standbyPart; gDuel.phaseSubStep restarts for
 * each part). */
int ModStandbyStep(int player)
{
    int done;

#ifdef HAS_CARD_DIFFERENT_DIMENSION_CAPSULE
    DifferentDimensionCapsuleStandbyStep(player);
#endif
    switch (gModDuel.standbyPart) {
    case 0:
        done = TreebornFrogStandbyStep(player);
        break;
#ifdef HAS_CARD_MIRAGE_OF_NIGHTMARE
    case 1:
        done = MirageDiscardStep(player);
        break;
    case 2:
        done = MirageDrawStep(player);
        break;
#endif
    default:
        gModDuel.standbyPart = 0;
        return 1;
    }
    if (done) {
        gModDuel.standbyPart++;
        gDuel.phaseSubStep = 0;
    }
    return 0;
}

#ifdef HAS_CARD_CYBER_DRAGON
/* ---- Cyber Dragon -------------------------------------------------------------------------------------------------
 * "If only your opponent controls a monster, you can Special Summon this card (from your hand)."
 * Like key 1350 (the original's Fiend Megacyber rule): CanSummonFromHand (summon_checks.c) allows it, the hand card
 * menu (card_command_menu.c) adds the Special Summon commands, and the summon steps (effect_hooks.c) queue
 * QueueSpecialSummonFromHand for them (no Tribute). It is still a Level 5 Normal Summon with one Tribute. The CPU
 * Special Summons it first thing in its Main Phase (AiStepMainPhase calls AiSpecialSummonCyberDragon before it picks
 * a Normal Summon), in Attack Position. */
int CanCyberDragonSpecialSummon(int player)
{
    return CountMonsters(player) == 0 && CountMonsters(1 - player) > 0 && CountFreeMonsterZones(player) > 0
           && CanSpecialSummon(player);
}

#endif /* HAS_CARD_CYBER_DRAGON */

#ifdef HAS_CARD_GILASAURUS
/* ---- Gilasaurus ----------------------------------------------------------------------------------------------------
 * "You can Special Summon this card (from your hand). If Summoned this way: Activate this effect; your opponent can
 * Special Summon 1 monster from their GY."
 * Engine key 1520 (tools/newcards.py ENGINE_NUMBERS): the hand menu adds the Special Summon commands
 * (card_command_menu.c), and the summon steps (effect_hooks.c) queue QueueSpecialSummonFromHand and, while the
 * opponent has a monster to revive, a free zone and may Special Summon, a RESPONSE_SPECIAL_SUMMONED trigger. Its
 * resolve, EffectOpponentGraveSummonResolve (effect_resolve11.c), asks the opponent (PROMPT_CONFIRM_GRAVEYARD_SUMMON;
 * the CPU says Yes), who picks a monster from the Call of the Haunted list (the CPU: AiPickCardListEntry) and its
 * position. The CPU's own Special Summon is in AiSpecialSummonCyberDragon below (AiGilasaurusSpecialSummon). */
int CollectEffectTargetsGila(int player, int cardNumber, int param) asm("CollectEffectTargets");

#define AI_GILASAURUS_SAFE_ATK 1500     /* the human may revive a monster weaker than this */

/* 1 if the opponent of `player` may answer a Gilasaurus Special Summon (the test effect_hooks.c makes for key 1520);
 * it leaves that opponent's candidates in gCardListView. */
static int GilasaurusOpponentCanRevive(int player)
{
    return CollectEffectTargetsGila(1 - player, CARD_CALL_OF_THE_HAUNTED, 0) != 0
           && CountFreeMonsterZones(1 - player) > 0 && CanSpecialSummon(1 - player);
}

/* The CPU Special Summons Gilasaurus when the human's best revival is harmless (no monster, or under
 * AI_GILASAURUS_SAFE_ATK ATK), or as the Tribute for a Level 5-6 monster in its hand that it could not summon
 * otherwise. */
static int AiWantsGilasaurus(void)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    int i, best = 0;

    if (!GilasaurusOpponentCanRevive(AI_PLAYER))
        return 1;
    for (i = 0; i < gCardListView.count; i++) {
        int atk = CARD_STATS_ATK(gCardStats[gCardListView.cards[i] & CARD_ID_MASK]) * CARD_STATS_POINTS_SCALE;
        if (atk > best)
            best = atk;
    }
    if (best < AI_GILASAURUS_SAFE_ATK)
        return 1;
    if (p->normalSummonUsed || CountTributableMonsters(AI_PLAYER, -1) > 0)
        return 0;
    for (i = 0; i < p->handCount; i++) {
        u32 stats = gCardStats[DUEL_CARD_ID(&p->hand[i]) & CARD_ID_MASK];
        if (CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_KIND(stats) != CARD_KIND_FUSION
            && CARD_STATS_KIND(stats) != CARD_KIND_RITUAL && CARD_STATS_LEVEL(stats) >= 5
            && CARD_STATS_LEVEL(stats) <= 6)
            return 1;
    }
    return 0;
}

/* Hand card i is Gilasaurus: Special Summon it in Attack Position, with its trigger as effect_hooks.c queues it. */
static void AiGilasaurusSpecialSummon(int i)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    u16 id = DUEL_CARD_ID(&p->hand[i]);
    int zone = FindFreeMonsterZone(AI_PLAYER);

    QueueSpecialSummonFromHand(AI_PLAYER, i, zone, 0, 1);
    if (GilasaurusOpponentCanRevive(AI_PLAYER))
        Chain_AddPending(TRIGGER_PLAYER(AI_PLAYER) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                             | TRIGGER_EVENT(RESPONSE_SPECIAL_SUMMONED) | id,
                         0);
}
#endif /* HAS_CARD_GILASAURUS */

/* The CPU's Special Summons from the hand by their own rules, first thing in its Main Phase (AiStepMainPhase, before
 * it picks a Normal Summon, which would change the monster counts): Cyber Dragon (only the human controls a monster),
 * The Fiend Megacyber (key 1350: the human controls 2 or more monsters more), in Attack Position, and Gilasaurus
 * (AiWantsGilasaurus). 1 if it queued one. */
int AiSpecialSummonCyberDragon(void)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    int i;

    if (CountFreeMonsterZones(AI_PLAYER) == 0 || !CanSpecialSummon(AI_PLAYER))
        return 0;
    for (i = 0; i < p->handCount; i++) {
        u16 number = CardNumberOfId(DUEL_CARD_ID(&p->hand[i]));
        int ok = 0;

#ifdef HAS_CARD_CYBER_DRAGON
        if (number == CARD_CYBER_DRAGON)
            ok = CanCyberDragonSpecialSummon(AI_PLAYER);
#endif
#ifdef HAS_CARD_THE_FIEND_MEGACYBER
        if (number == CARD_THE_FIEND_MEGACYBER)
            ok = CountMonsters(AI_PLAYER) + 1 < CountMonsters(AI_HUMAN);
#endif
#ifdef HAS_CARD_GILASAURUS
        if (number == CARD_GILASAURUS && AiWantsGilasaurus()) {
            AiGilasaurusSpecialSummon(i);
            return 1;
        }
#endif
        if (ok) {
            QueueSpecialSummonFromHand(AI_PLAYER, i, FindFreeMonsterZone(AI_PLAYER), 0, 1);
            return 1;
        }
    }
    return 0;
}


#ifdef HAS_CARD_MAGICAL_SCIENTIST
/* ---- Magical Scientist (the CPU) -----------------------------------------------------------------------------------
 * AiStepMainPhase (ai_steps.c) calls AiUseMagicalScientist first thing, with AiSpecialSummonCyberDragon, so it runs
 * again after each activation. In order:
 *   burn     a face-up Catapult Turtle or Cannon Soldier of the CPU's Tributes a monster Magical Scientist summoned
 *            (AiPickEffectTribute, summon_builders.c, asks AiPickScientistTribute first: the strongest such one)
 *   summon   a face-up Magical Scientist pays 1000 LP while the CPU keeps more than 1000:
 *              with Catapult Turtle or Cannon Soldier face up, for the burn above
 *              else once per turn in Main Phase 1 with 4000 LP or more, when its strongest Fusion Monster would beat
 *              a face-up monster of the human's (it cannot attack directly)
 * AiWantsMonsterEffect keeps AiPlayAddedCards from activating it otherwise. */
#define AI_SCIENTIST_BATTLE_MIN_LP 4000

static int IsScientistBurner(u16 number)
{
    if (number == CARD_CATAPULT_TURTLE || number == CARD_CANNON_SOLDIER)
        return 1;
#ifdef HAS_CARD_TOON_CANNON_SOLDIER
    if (number == CARD_TOON_CANNON_SOLDIER)
        return 1;
#endif
    return 0;
}

/* The CPU's strongest monster Magical Scientist summoned (not excludeZone), -1 for none. */
int AiPickScientistTribute(int excludeZone)
{
    int zone, best = -1;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (zone != excludeZone && IsScientistSummoned(AI_PLAYER, zone)
            && (best < 0 || GetZoneCardAtk(AI_PLAYER, zone) > GetZoneCardAtk(AI_PLAYER, best)))
            best = zone;
    }
    return best;
}

/* The highest ATK of the Fusion Monsters Magical Scientist could summon for the CPU. */
static int AiBestScientistFusionAtk(void)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    int i, best = -1;

    for (i = 0; i < p->fusionCount; i++) {
        u16 id = DUEL_CARD_ID(&p->fusionDeck[i]);
        int atk = CARD_STATS_ATK(gCardStats[id & CARD_ID_MASK]) * CARD_STATS_POINTS_SCALE;

        if (IsScientistFusion(id, 0) && atk > best)
            best = atk;
    }
    return best;
}

/* 1 if a Fusion Monster of `atk` ATK would destroy a face-up monster of the human's in battle. */
static int AiScientistHasBattleTarget(int atk)
{
    int zone;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(AI_HUMAN, zone);

        if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp
            && atk > (int)(z->isDefense ? GetZoneCardDef(AI_HUMAN, zone) : GetZoneCardAtk(AI_HUMAN, zone)))
            return 1;
    }
    return 0;
}

static void AiActivateMonsterAt(int zone)
{
    u16 id = DUEL_CARD_ID(&ZoneOf(AI_PLAYER, zone)->card);

    DuelCmd_Push(DUEL_CMD_PLAYER | DUEL_CMD_POINT_AT_CARD, AI_PLAYER, (u8)zone << 8, 0);
    Chain_AddPending(((u32)(zone & 0x1F) << 16) | id | CPU_CHAIN_KIND_MONSTER, 0);
}

int AiUseMagicalScientist(void)
{
    int zone, scientist = -1, burner = 0, atk;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(AI_PLAYER, zone);
        u16 id = DUEL_CARD_ID(&z->card);
        u16 number;

        if (id == 0 || !z->isFaceUp)
            continue;
        number = CardNumberOfId(id);
        if (IsScientistBurner(number)) {
            burner = 1;
            if (AiPickScientistTribute(zone) >= 0 && CanActivateMonsterEffect(id, AI_PLAYER, zone)) {
                AiActivateMonsterAt(zone);
                return 1;
            }
        }
        if (number == CARD_MAGICAL_SCIENTIST && scientist < 0 && CanActivateMonsterEffect(id, AI_PLAYER, zone))
            scientist = zone;
    }
    if (scientist < 0 || gDuelPlayers[AI_PLAYER].lifePoints <= SCIENTIST_COST)
        return 0;
    if (!burner) {
        atk = AiBestScientistFusionAtk();
        if (gAiState.step != AI_STEP_MAIN_PHASE_1 || gDuelPlayers[AI_PLAYER].lifePoints < AI_SCIENTIST_BATTLE_MIN_LP
            || gModDuel.aiScientistTurn == (u16)(gDuel.turnCount + 1) || !AiScientistHasBattleTarget(atk))
            return 0;
        gModDuel.aiScientistTurn = gDuel.turnCount + 1;
    }
    AiActivateMonsterAt(scientist);
    return 1;
}
#endif /* HAS_CARD_MAGICAL_SCIENTIST */


/* ---- The CPU counts the ATK boosts it will get when it judges positions -------------------------------------------
 * When the CPU chooses Attack or Defense (the summon simulations, the position-change step and AiShouldRetreatToDefense)
 * it judges a monster by the ATK it will have once the boosts it plays are on it:
 *   hand Equips  the best ATK it would get from one Equip Spell in the CPU's hand that the CPU plays (gAiEquipSpells,
 *                or an added card: AiPlayAddedCards) and whose target check accepts the monster. Each is tried for
 *                real: the card goes into a free Spell/Trap zone, linked to the monster (ZONE_LINK_EQUIP), and
 *                GetZoneCardAtk reads the result (card_stats.c applies it exactly), then both zones are put back.
 *   on summon    Breaker the Magical Warrior's Spell Counter (+300), in the summon simulation.
 *   hand size    the cards that leave the hand are taken off its count while the ATK is read (the summoned monster,
 *                and the Equip being tried), so Blade Knight's +400 (1 or fewer cards in hand), Muka Muka and Flash
 *                Assailant are judged with the hand they will have.
 * AiSimSummon (ai_summon.c) calls AiSimSummonBoosts inside the simulation sandbox, which keeps the best Equip on the
 * simulated monster; the others use AiProjectedAtk, which opens its own sandbox (AiBackupDuelState, which cannot be
 * nested). */
extern const u16 gAiEquipSpells[];      /* 0x08086400: the Equip Magic AiPlaySpells plays */
#define AI_EQUIP_SPELL_COUNT 35

void AiBackupDuelState(void);
void AiRestoreDuelState(void);

/* 1 if the CPU plays the Equip Spell `id` from its hand. */
static int AiPlaysEquip(u16 id)
{
    u32 stats = gCardStats[id & CARD_ID_MASK];
    u16 number = CardNumberOfId(id);
    int i;

    if (CARD_STATS_TYPE(stats) != CARD_TYPE_MAGIC || CARD_STATS_SUBTYPE(stats) != SPELL_EQUIP)
        return 0;
    if (IsAddedCardId(id))
        return 1;
    for (i = 0; i < AI_EQUIP_SPELL_COUNT; i++) {
        if (gAiEquipSpells[i] == number)
            return 1;
    }
    return 0;
}

/* The CPU's monster in `zone` with hand Equip `handIndex` (or -1: none) in Spell/Trap zone `st`, its hand
 * `handCount` cards (less the Equip); returns its ATK. Inside a sandbox: the Equip and the count stay. */
static u32 AiApplyHandEquip(int zone, int st, int handIndex, int handCount)
{
    struct DuelZone *s = ZoneOf(AI_PLAYER, st);

    gDuelPlayers[AI_PLAYER].handCount = handIndex >= 0 ? handCount - 1 : handCount;
    if (handIndex >= 0) {
        s->card = gDuelPlayers[AI_PLAYER].hand[handIndex];
        s->isFaceUp = 1;
        s->isDisabled = 0;
        AddZoneLink(DUEL_LOC(AI_PLAYER, zone), DUEL_LOC(AI_PLAYER, st), ZONE_LINK_EQUIP);
    }
    return GetZoneCardAtk(AI_PLAYER, zone);
}

/* Inside a sandbox: the best ATK of the CPU's monster in `zone` with one of its hand Equips, or without, with
 * `played` cards (the summoned monster) already out of the hand; `keep` leaves the best Equip on it and the hand
 * count reduced. */
static u32 AiBestAtkWithHandEquip(int zone, int keep, int played)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    struct DuelZone savedSpell, savedMonster;
    int handCount = p->handCount, left = handCount - played;
    u32 best, atk;
    int st, i, bestIndex = -1;

    if (left < 0)
        left = 0;
    p->handCount = left;
    best = GetZoneCardAtk(AI_PLAYER, zone);
    p->handCount = handCount;
    for (st = ZONE_SPELL_0; st <= ZONE_SPELL_4; st++) {
        if (DUEL_CARD_ID(&ZoneOf(AI_PLAYER, st)->card) == 0)
            break;
    }
    if (st > ZONE_SPELL_4) {
        if (keep)
            p->handCount = left;
        return best;                    /* no room to play an Equip */
    }
    MemCopy16(&savedSpell, ZoneOf(AI_PLAYER, st), sizeof(struct DuelZone));
    MemCopy16(&savedMonster, ZoneOf(AI_PLAYER, zone), sizeof(struct DuelZone));
    for (i = 0; i < handCount; i++) {
        u16 id = DUEL_CARD_ID(&p->hand[i]);
        struct ChainEntry entry;
        int row;

        if (!AiPlaysEquip(id) || !CanActivateEffectOfCard(AI_PLAYER, id, 1))
            continue;
        row = FindCardEffect(id);
        if (row < 0 || CARD_EFFECT_ROW(row).check == NULL)
            continue;
        MemClear16(&entry, sizeof(entry));
        entry.card = id;
        entry.player = AI_PLAYER;
        entry.zone = st;
        if (!CARD_EFFECT_ROW(row).check(&entry, DUEL_LOC(AI_PLAYER, zone)))
            continue;
        atk = AiApplyHandEquip(zone, st, i, left);
        p->handCount = handCount;
        MemCopy16(ZoneOf(AI_PLAYER, st), &savedSpell, sizeof(struct DuelZone));
        MemCopy16(ZoneOf(AI_PLAYER, zone), &savedMonster, sizeof(struct DuelZone));
        if (atk > best) {
            best = atk;
            bestIndex = i;
        }
    }
    if (keep)
        AiApplyHandEquip(zone, st, bestIndex, left);
    return best;
}

/* AiSimSummon: the simulated monster in `zone` gets its summon boosts (Breaker's counter) and the best hand Equip.
 * Inside the simulation sandbox. */
#ifdef HAS_CARD_THUNDER_NYAN_NYAN
/* ---- Thunder Nyan Nyan ---------------------------------------------------------------------------------------------
 * "If there is a non-LIGHT monster on your side of the field, destroy this face-up card."
 *   rule  ModDuelStateCheck (DuelMainStep, while nothing else is busy; not during an attack, as for Berserk Gorilla)
 *         destroys a face-up Thunder Nyan Nyan whose controller has a face-up monster that is not LIGHT (its effective
 *         attribute, GetZoneCardStats). A face-down monster has no attribute and does not count; a face-down Nyan
 *         Nyan is not destroyed.
 *   CPU   AiSimSummonBoosts (the summon simulation's sandbox) removes the CPU's Nyan Nyans the rule would destroy, so
 *         the CPU does not summon it next to a non-LIGHT monster, nor a non-LIGHT monster next to it, unless the
 *         simulation still comes out ahead. */

/* 1 if `player` controls a face-up monster whose attribute is not LIGHT. */
static int HasFaceUpNonLightMonster(int player)
{
    struct ZoneCardStats info;
    int zone;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);

        if (DUEL_CARD_ID(&z->card) == 0 || !z->isFaceUp)
            continue;
        GetZoneCardStats(player, zone, &info);
        if (info.attribute != ATTRIBUTE_LIGHT)
            return 1;
    }
    return 0;
}

/* The zone of a face-up Thunder Nyan Nyan of `player` that its rule destroys now, -1 for none. */
static int DoomedThunderNyanNyan(int player)
{
    int zone;

    if (!HasFaceUpNonLightMonster(player))
        return -1;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);
        u16 id = DUEL_CARD_ID(&z->card);

        if (id != 0 && z->isFaceUp && CardNumberOfId(id) == CARD_THUNDER_NYAN_NYAN)
            return zone;
    }
    return -1;
}
#endif /* HAS_CARD_THUNDER_NYAN_NYAN */

void AiSimSummonBoosts(int zone)
{
    struct DuelZone *z = ZoneOf(AI_PLAYER, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    if (id == 0)
        return;
#ifdef HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR
    if (CardNumberOfId(id) == CARD_BREAKER_THE_MAGICAL_WARRIOR && CountSpellCounters(AI_PLAYER, zone) == 0)
        AddZoneLink(DUEL_LOC(AI_PLAYER, zone), id, ZONE_LINK_KIND(ZONE_LINK_ATK_300_PER_VALUE, 1));
#endif
    AiBestAtkWithHandEquip(zone, 1, 1);    /* the summoned monster has left the hand */
#ifdef HAS_CARD_THUNDER_NYAN_NYAN
    {
        int doomed;

        /* the sandbox is restored afterwards: clear the Nyan Nyans the rule would destroy */
        while ((doomed = DoomedThunderNyanNyan(AI_PLAYER)) >= 0)
            MemClear16(ZoneOf(AI_PLAYER, doomed), sizeof(struct DuelZone));
    }
#endif
}

/* The ATK the CPU's monster in `zone` will have face up with the best Equip from its hand (its own sandbox; not
 * inside another one). */
u32 AiProjectedAtk(int zone)
{
    u32 atk;

    if (DUEL_CARD_ID(&ZoneOf(AI_PLAYER, zone)->card) == 0)
        return 0;
    AiBackupDuelState();
    ZoneOf(AI_PLAYER, zone)->isFaceUp = 1;
    atk = AiBestAtkWithHandEquip(zone, 0, 0);
    AiRestoreDuelState();
    return atk;
}

/* ---- The CPU's tributes and costs are not copies of the card it summons -------------------------------------------
 * The CPU never Tributes a copy of the monster it summons (the same card number), whether for a Tribute Summon or
 * for a Special Summon that sends its cost from the field to the Graveyard (the Toons' Tributes, ai_strategy.c),
 * unless the summoned card has a summon trigger (AiHasSummonTrigger), which makes the swap worth it. The CPU's
 * summon code asks AiPickTributeFor, which tells AiPickTributeMonster (summon_builders.c) the card through
 * gModDuel.aiTributeFor; AiPickTributeMonster skips that card's copies (AiTributeIsSameCard). The other callers
 * (Panther Warrior's attack cost, the tribute prompt of effects) keep 0xFFFF: no filter. Graveyard banish costs
 * (Black Luster Soldier, Chaos Sorcerer, Gigantes) are not affected. */

/* Cards with a trigger when they are summoned: the summon trigger list (summon_checks.c), and the added cards'
 * summon effects (AddedCardsSummoned, Tsukuyomi). */
static int AiHasSummonTrigger(u16 number)
{
    switch (number) {
    case CARD_DRAGON_SEEKER:
    case CARD_SENJU_OF_THE_THOUSAND_HANDS:
    case CARD_SONIC_BIRD:
    case CARD_1240:
    case CARD_1246:
    case CARD_1332:
#ifdef HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR
    case CARD_BREAKER_THE_MAGICAL_WARRIOR:
#endif
#ifdef HAS_CARD_ZABORG_THE_THUNDER_MONARCH
    case CARD_ZABORG_THE_THUNDER_MONARCH:
#endif
#ifdef HAS_CARD_MOBIUS_THE_FROST_MONARCH
    case CARD_MOBIUS_THE_FROST_MONARCH:
#endif
#ifdef HAS_CARD_DARK_MAGICIAN_OF_CHAOS
    case CARD_DARK_MAGICIAN_OF_CHAOS:
#endif
#ifdef HAS_CARD_SACRED_CRANE
    case CARD_SACRED_CRANE:
#endif
#ifdef HAS_CARD_APPRENTICE_MAGICIAN
    case CARD_APPRENTICE_MAGICIAN:
#endif
#ifdef HAS_CARD_TSUKUYOMI
    case CARD_TSUKUYOMI:
#endif
        return 1;
    }
    return 0;
}

/* The card number whose copies the CPU may not Tribute for summoning card `id`, or 0xFFFF for none. */
static u16 AiCostExcludedNumber(u16 id)
{
    u16 number = CardNumberOfId(id);

    return AiHasSummonTrigger(number) ? 0xFFFF : number;
}

int AiPickTributeMonster(int excludeZone, u16 allowFusion);

/* AiPickTributeMonster(excludeZone, 0) for the CPU's summon of card `id`: never a copy of it (see above). */
int AiPickTributeFor(u16 id, int excludeZone)
{
    int zone;

    gModDuel.aiTributeFor = AiCostExcludedNumber(id);
    zone = AiPickTributeMonster(excludeZone, 0);
    gModDuel.aiTributeFor = 0xFFFF;
    return zone;
}

/* AiPickTributeMonster: 1 if the CPU's monster in `zone` is a copy of the card it is summoning. */
int AiTributeIsSameCard(int zone)
{
    u16 id = DUEL_CARD_ID(&ZoneOf(AI_PLAYER, zone)->card);

    return gModDuel.aiTributeFor != 0xFFFF && id != 0 && CardNumberOfId(id) == gModDuel.aiTributeFor;
}

/* ---- The CPU never pays itself to 0 LP ------------------------------------------------------------------------------
 * CanActivateEffect (effect_activation.c) refuses the CPU's activation of a card whose own LP cost or self-damage
 * would bring its LP to 0 (not in link duels, where player 1 is a human). AiSelfLpLoss gives the most the card can cost
 * its controller:
 *   costs        EffectPayLifePointsChainA's (effect_checks.c): Ultimate Offering 500, Premature Burial 800, Monster
 *                Eye, Seven Tools of the Bandit, Confiscation, Delinquent Duo, Seal of the Ancients 1000, Gale Dogra
 *                3000, Cyber Stein 5000 (Solemn Judgment's half never reaches 0); the added Brain Control 800 and
 *                Dimension Fusion 2000
 *   self-damage  Tremendous Fire 500; Time Wizard's lost coin toss, half the total ATK of its controller's monsters
 * The maintenance costs are paid only above the cost + 1000 (DuelPhase_Standby), and Bell of Destruction's CPU check
 * already refuses an ATK that reaches its LP (AiShouldActivateSetCard). */
static int AiSelfLpLoss(struct ChainEntry *card)
{
    int zone, atk = 0;

    switch (CardNumberOfId(card->card)) {
    case CARD_ULTIMATE_OFFERING:
    case CARD_TREMENDOUS_FIRE:
#ifdef HAS_CARD_CHAIN_DISAPPEARANCE
    case CARD_CHAIN_DISAPPEARANCE:
#endif
        return 500;
    case CARD_PREMATURE_BURIAL:
#ifdef HAS_CARD_BRAIN_CONTROL
    case CARD_BRAIN_CONTROL:
#endif
#ifdef HAS_CARD_ROYAL_OPPRESSION
    case CARD_ROYAL_OPPRESSION:
#endif
        return 800;
    case CARD_MONSTER_EYE:
    case CARD_SEVEN_TOOLS_OF_THE_BANDIT:
    case CARD_CONFISCATION:
    case CARD_DELINQUENT_DUO:
    case CARD_SEAL_OF_THE_ANCIENTS:
#ifdef HAS_CARD_MAGICAL_SCIENTIST
    case CARD_MAGICAL_SCIENTIST:
#endif
#ifdef HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END
    case CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END:
#endif
        return 1000;
#ifdef HAS_CARD_DIMENSION_FUSION
    case CARD_DIMENSION_FUSION:
        return 2000;
#endif
    case CARD_GALE_DOGRA:
        return 3000;
    case CARD_CYBER_STEIN:
        return 5000;
    case CARD_TIME_WIZARD:
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            if (DUEL_CARD_ID(&ZoneOf(card->player, zone)->card) != 0)
                atk += GetZoneCardAtk(card->player, zone);
        }
        return atk / 2;
    }
    return 0;
}

/* 1 if the CPU may not activate `card` because it would bring its own LP to 0. */
int AiActivationWouldLoseLp(struct ChainEntry *card)
{
    int loss;

    if ((card->player & 1) != AI_PLAYER || gDuelCtrl.isLinkDuel)
        return 0;
    loss = AiSelfLpLoss(card);
    return loss > 0 && loss >= (int)gDuelPlayers[AI_PLAYER].lifePoints;
}

/* ---- Thunder Dragon (the CPU) -------------------------------------------------------------------------------------
 * The vanilla CPU never uses Thunder Dragon's "discard this card; add up to 2 Thunder Dragon from your Deck to your
 * hand". AiStepMainPhase (ai_steps.c) calls AiUseThunderDragon first in AI_MAIN_SUMMON, before it picks a summon:
 * a Thunder Dragon in the CPU's hand is discarded and its effect queued as the card menu does for the human
 * (card_command_menu.c). EffectThunderDragonResolve (effect_resolve1.c) adds only 1 for the CPU and calls
 * ModThunderDragonSearched, and the step runs again after the chain, so each new copy is discarded in turn: the
 * second search takes the last copy, and the third activation finds none (the resolve only shuffles). With three
 * copies all three end in the Graveyard. A copy is discarded only while one is left in the Deck, or when it was just
 * added by a search (aiThunderDragonSearched): a lone copy (none left to find, not just searched) is kept.
 * EffectThunderDragonPrepare (effect_activation.c) no longer wants a copy in the Deck (anyone may activate it to find
 * none), so the CPU asks CanActivateEffectOfCard like the card menu. */
void ModThunderDragonSearched(void)
{
    gModDuel.aiThunderDragonSearched = 1;
}

int AiUseThunderDragon(void)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    int i, inDeck = 0;

    for (i = 0; i < p->deckCount; i++) {
        if (CardNumberOfId(DUEL_CARD_ID(&p->deck[i])) == CARD_THUNDER_DRAGON)
            inDeck = 1;
    }
    if (!inDeck && !gModDuel.aiThunderDragonSearched)
        return 0;                       /* a lone copy: kept */
    for (i = 0; i < p->handCount; i++) {
        u16 id = DUEL_CARD_ID(&p->hand[i]);

        if (CardNumberOfId(id) == CARD_THUNDER_DRAGON && CanActivateEffectOfCard(AI_PLAYER, id, 1)) {
            gModDuel.aiThunderDragonSearched = 0;
            DiscardHandCard(AI_PLAYER, i, 0, 1);
            Chain_AddPending(((u32)AI_PLAYER << 31) | CHAIN_KIND_OFF_FIELD << 21 | id, 0);
            return 1;
        }
    }
    return 0;
}

#ifdef HAS_CARD_RYU_KOKKI
/* ---- Ryu Kokki ----------------------------------------------------------------------------------------------------
 * "At the end of the Damage Step, if this card battled a Warrior or Spellcaster-Type monster: Destroy that monster."
 * BattleStage_DestroyMonsters (battle_phase2.c) calls RyuKokkiAfterBattle once the battle's own destruction is queued:
 * a Ryu Kokki in the battle that survived it (not destroyed, still in its zone, its battle effect not blocked as for
 * Zone Eater) destroys the other monster by its effect when that one is a Warrior or Spellcaster (its current type,
 * GetZoneCardStats), was not destroyed by the battle and is still in its zone. Not when D.D. Warrior Lady's Yes
 * banishes both. Mandatory, so nobody is asked; applied at once, not as a chain link. */
extern u16 IsBattleEffectBlockedFor(int player) asm("IsBattleEffectBlocked");

void RyuKokkiAfterBattle(int player)
{
    int side;

    if (gBattle.direct || gBattle.ddWarriorLady == DD_WARRIOR_LADY_YES)
        return;
    for (side = 0; side <= 1; side++) {
        int who = side ? 1 - player : player, other = 1 - who;
        int zone = who == player ? gBattle.atkSlot : gBattle.defSlot;
        int otherZone = other == player ? gBattle.atkSlot : gBattle.defSlot;
        struct DuelZone *z = ZoneOf(who, zone), *oz = ZoneOf(other, otherZone);
        struct ZoneCardStats info;

        if (CardNumberOfId(gBattle.side[who].cardId) != CARD_RYU_KOKKI || gBattle.side[who].destroyed
            || DUEL_CARD_ID(&z->card) != (gBattle.side[who].cardId & CARD_ID_MASK) || IsBattleEffectBlockedFor(who))
            continue;
        if (gBattle.side[other].destroyed || DUEL_CARD_ID(&oz->card) == 0
            || DUEL_CARD_ID(&oz->card) != (gBattle.side[other].cardId & CARD_ID_MASK))
            continue;
        GetZoneCardStats(other, otherZone, &info);
        if (info.type != CARD_TYPE_WARRIOR && info.type != CARD_TYPE_SPELLCASTER)
            continue;
        ShowCardEffect(who, gBattle.side[who].cardId);
        DuelCmd_Push(other ? DUEL_CMD_PLAYER | DUEL_CMD_SET_DESTROYED_BY_OPPONENT_FLAG
                           : DUEL_CMD_SET_DESTROYED_BY_OPPONENT_FLAG, otherZone, 1, 0);
        DestroyFieldCardByEffect(other, otherZone);
        OnCardDestroyedByEffect(who, other, otherZone);
    }
}
#endif /* HAS_CARD_RYU_KOKKI */

#ifdef HAS_CARD_DRILLROID
/* ---- Drillroid ----------------------------------------------------------------------------------------------------
 * "Before damage calculation, if this card attacks a Defense Position monster: Destroy that monster."
 * BattleStage_DamageCalc (battle_phase1.c) calls DrillroidBeforeDamageCalc at DAMAGE_STEP_CALC, after the attack is
 * marked and before CalcBattle: the target has been revealed (a face-down one flipped, its flip effect noted by
 * BattleStage_RevealDefender). A face-up Drillroid attacking a Defense Position monster destroys it by its effect
 * (the destroyed-by-opponent flag, DestroyFieldCard with its Graveyard triggers, OnCardDestroyedByEffect). As for a
 * flipped Blast Sphere, CalcBattle fills the battle record and its destroyed flags and damage are cleared; the
 * battle then goes on at BATTLE_STAGE_FLIP_EFFECT: no damage calculation and no battle damage, but a flipped
 * target's flip effect still activates. */
int DrillroidBeforeDamageCalc(int player)
{
    struct DuelZone *attacker = ZoneOf(player, gBattle.atkSlot);
    struct DuelZone *target;
    int opponent = 1 - player;
    u16 id = DUEL_CARD_ID(&attacker->card);

    if (gBattle.direct || id == 0 || !attacker->isFaceUp || CardNumberOfId(id) != CARD_DRILLROID)
        return 0;
    target = ZoneOf(opponent, gBattle.defSlot);
    if (DUEL_CARD_ID(&target->card) == 0 || !target->isDefense)
        return 0;
    CalcBattle(player, 0);
    gBattle.side[0].destroyed = 0;
    gBattle.side[1].destroyed = 0;
    gBattle.side[0].damage = 0;
    gBattle.side[1].damage = 0;
    ShowCardEffect(player, id);
    DuelCmd_Push(opponent ? DUEL_CMD_PLAYER | DUEL_CMD_SET_DESTROYED_BY_OPPONENT_FLAG
                          : DUEL_CMD_SET_DESTROYED_BY_OPPONENT_FLAG, gBattle.defSlot, 1, 0);
    DestroyFieldCard(opponent, gBattle.defSlot, 1);
    OnCardDestroyedByEffect(player, opponent, gBattle.defSlot);
    gDuel.battleStage = BATTLE_STAGE_FLIP_EFFECT;
    gDuel.battleStep = 0;
    return 1;
}
#endif /* HAS_CARD_DRILLROID */

#ifdef HAS_CARD_HYDROGEDDON
/* ---- Hydrogeddon --------------------------------------------------------------------------------------------------
 * "When this card destroys an opponent's monster by battle and sends it to the GY: You can Special Summon 1
 * "Hydrogeddon" from your Deck."
 * BattleStage_DestroyMonsters (battle_phase2.c) calls HydrogeddonAfterBattle once the battle's destruction is queued:
 * a Hydrogeddon in the battle (attacking or attacked) that was not destroyed by it, whose opponent's monster was
 * destroyed by it and goes to the Graveyard (not banished by D.D. Warrior Lady's Yes, D.D. Assailant or Dark Magician of Chaos),
 * queues its trigger (RESPONSE_BATTLE_DESTROYED, its zone, as Black Luster Soldier's) when its Deck has a
 * Hydrogeddon and it has a free monster zone. It need not be on the field any more: the flip effect of the monster
 * it destroyed resolves before this stage (BATTLE_STAGE_FLIP_EFFECT), so a Man-Eater Bug may have destroyed it; the
 * trigger is then one from off the field (CHAIN_KIND_OFF_FIELD, as Sangan's). The resolve is the recruiters' EffectSpecialSummonFromDeckResolve (the
 * Deck list is a CollectEffectTargets case): the human is asked, the CPU always summons; the position is chosen
 * (QueueSpecialSummonChoosePosition, as for Pyramid Turtle). */
static int DeckHasCardNumber(int player, u16 number)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    for (i = 0; i < p->deckCount; i++) {
        if (CardNumberOfId(DUEL_CARD_ID(&p->deck[i])) == number)
            return 1;
    }
    return 0;
}

void HydrogeddonAfterBattle(int player)
{
    int side;

    if (gBattle.direct || gBattle.ddWarriorLady == DD_WARRIOR_LADY_YES || gBattle.ddWarriorLady == DD_BANISH_DESTROYED)
        return;
    for (side = 0; side <= 1; side++) {
        int who = side ? 1 - player : player, other = 1 - who;
        int zone = who == player ? gBattle.atkSlot : gBattle.defSlot;
        u16 id = gBattle.side[who].cardId & CARD_ID_MASK;

        if (CardNumberOfId(id) != CARD_HYDROGEDDON || gBattle.side[who].destroyed || !gBattle.side[other].destroyed)
            continue;
        if (!DeckHasCardNumber(who, CARD_HYDROGEDDON) || CountFreeMonsterZones(who) == 0)
            continue;
        if (DUEL_CARD_ID(&ZoneOf(who, zone)->card) == id)
            Chain_AddPending(TRIGGER_PLAYER(who & 1) | TRIGGER_ZONE(zone) | TRIGGER_KIND(CHAIN_KIND_MONSTER)
                                 | TRIGGER_EVENT(RESPONSE_BATTLE_DESTROYED) | id,
                             DUEL_LOC(who, zone));
        else    /* it left the field since (a flipped Man-Eater Bug destroyed it): it still activates */
            Chain_AddPending(TRIGGER_PLAYER(who & 1) | TRIGGER_KIND(CHAIN_KIND_OFF_FIELD)
                                 | TRIGGER_EVENT(RESPONSE_BATTLE_DESTROYED) | id,
                             0);
    }
}
#endif /* HAS_CARD_HYDROGEDDON */

#ifdef HAS_CARD_MOBIUS_THE_FROST_MONARCH
/* ---- Mobius the Frost Monarch -------------------------------------------------------------------------------------
 * "If this card is Tribute Summoned: You can target up to 2 Spells/Traps on the field; destroy them."
 * The two Normal Summon paths (summon_checks.c) queue its RESPONSE_SUMMONED trigger in Zaborg's case; it is Level 6, so
 * that summon always has a Tribute. Check: a Spell/Trap is on the field (EffectAnySpellTrapCheck, as Mystical Space
 * Typhoon). Targets (EffectMobiusChainB):
 *   human  the first card (B: none, the effect is not used), then a second one (B: only the first; skipped when no
 *          other Spell/Trap is on the field), Greenkappa's prompts
 *   CPU    up to 2 of the human's Spells/Traps, face-up ones first; never its own
 * EffectMobiusResolve destroys each target still on the field, as EffectDestroyTargetResolve does one. */
extern const u8 gStrDesignateSecondCardToDestroy[];     /* Greenkappa */

#define MOBIUS_MAX_TARGETS 2

/* 1 if a Spell/Trap card other than `skip` (a DUEL_LOC, 0xFFFF for none) is on the field. */
static int MobiusHasOtherSpellTrap(u16 skip)
{
    int player, zone;

    for (player = 0; player <= 1; player++) {
        for (zone = ZONE_SPELL_0; zone <= ZONE_FIELD; zone++) {
            if (DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0 && DUEL_LOC(player, zone) != skip)
                return 1;
        }
    }
    return 0;
}

int EffectMobiusChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (link->player) {
        int pass, zone;

        link->numTargets = 0;
        for (pass = 0; pass <= 1 && link->numTargets < MOBIUS_MAX_TARGETS; pass++) {
            for (zone = ZONE_SPELL_0; zone <= ZONE_FIELD && link->numTargets < MOBIUS_MAX_TARGETS; zone++) {
                struct DuelZone *z = ZoneOf(AI_HUMAN, zone);

                /* pass 0: the face-up cards (Continuous, Equip, Field), pass 1: the face-down ones */
                if (DUEL_CARD_ID(&z->card) != 0 && (pass == 0) == (z->isFaceUp != 0))
                    TryAddEffectTarget(link, AI_HUMAN, zone);
            }
        }
        return 1;
    }
    switch (gChain.targetStep) {
    case 0:
        link->numTargets = 0;
        TextBoxOpen(0x206, 0x712, TEXTBOX_FLAGS_DEFAULT, gStrDesignateFirstCardToDestroy);
        gChain.targetStep++;
        return 0;
    case 1:
        if (gMain.newKeys & B_BUTTON)
            return 1;                       /* "You can": no target, nothing happens */
        if (DuelCursor_PickTarget(PICK_BOTH_SIDES(PICK_ANY_SPELL_TRAP)) != 0
            && TryAddEffectTarget(link, gDuelScreen.selPlayer, gDuelScreen.selArea + gDuelScreen.selIndex) != 0) {
            if (!MobiusHasOtherSpellTrap(link->targets[0]))
                return 1;
            gChain.targetStep++;
        }
        return 0;
    case 2:
        TextBoxOpen(0x206, 0x712, TEXTBOX_FLAGS_DEFAULT, gStrDesignateSecondCardToDestroy);
        gChain.targetStep++;
        return 0;
    default:
        if (gMain.newKeys & B_BUTTON)
            return 1;                       /* only the first one */
        if (DuelCursor_PickTarget(PICK_BOTH_SIDES(PICK_ANY_SPELL_TRAP)) != 0) {
            int player = gDuelScreen.selPlayer, zone = gDuelScreen.selArea + gDuelScreen.selIndex;

            if (DUEL_LOC(player, zone) != link->targets[0] && TryAddEffectTarget(link, player, zone) != 0)
                return 1;
            PlaySE(SE_ERROR);
        }
        return 0;
    }
}

int EffectMobiusResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int i;

    if (link->negated)
        return EFFECT_STEP_DONE;
    for (i = 0; i < link->numTargets && i < MOBIUS_MAX_TARGETS; i++) {
        int player = (u8)link->targets[i], zone = link->targets[i] >> 8;

        if (DUEL_CARD_ID(&ZoneOf(player, zone)->card) == 0)
            continue;
        if (link->player != player)
            DuelCmd_Push(player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_DESTROYED_BY_OPPONENT_FLAG
                                : DUEL_CMD_SET_DESTROYED_BY_OPPONENT_FLAG, zone, 1, 0);
        DestroyFieldCardByEffect(player, zone);
        OnCardDestroyedByEffect(link->player, player, zone);
    }
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_MOBIUS_THE_FROST_MONARCH */

#ifdef HAS_CARD_KING_TIGER_WANGHU
/* ---- King Tiger Wanghu --------------------------------------------------------------------------------------------
 * "When a monster with 1400 or less ATK is Normal or Special Summoned: Destroy it." ExecuteSummonAction
 * (summon_action.c) asks KingTigerWanghuOnSummon after a Normal Summon or a Special Summon, before the opponent's
 * response window: a face-up summoned monster (a Set or face-down one shows no ATK) with 1400 or less ATK is destroyed
 * while a face-up, non-negated King Tiger Wanghu is on either side of the field. Flip Summons do not count. */
#define WANGHU_MAX_ATK 1400

/* 1 if the monster just summoned at (player, zone) was destroyed (then no response window is opened for it). */
int KingTigerWanghuOnSummon(int player, int zone)
{
    struct DuelZone *s = ZoneOf(player, zone);
    int p, z;

    if (DUEL_CARD_ID(&s->card) == 0 || !s->isFaceUp || (int)GetZoneCardAtk(player, zone) > WANGHU_MAX_ATK)
        return 0;
    for (p = 0; p <= 1; p++) {
        for (z = ZONE_MONSTER_0; z <= ZONE_MONSTER_4; z++) {
            struct DuelZone *w = ZoneOf(p, z);
            u16 id = DUEL_CARD_ID(&w->card);

            if (id != 0 && w->isFaceUp && !w->isDisabled && CardNumberOfId(id) == CARD_KING_TIGER_WANGHU
                && !(p == player && z == zone)) {
                ShowCardEffect(p, id);
                DestroyFieldCard(player, zone, 1);
                OnCardDestroyedByEffect(p, player, zone);
                return 1;
            }
        }
    }
    return 0;
}
#endif /* HAS_CARD_KING_TIGER_WANGHU */

int AiShouldActivateAddedCard(struct ChainEntry *entry); /* below: added-card Set/face-up chain policy */

/* ---- The CPU answers a summon trigger with a summon trap ----------------------------------------------------------
 * A monster's own summon trigger (Mobius, Zaborg, Breaker, Sonic Bird, ...) is a chain link with the summon's event
 * (RESPONSE_SUMMONED and the like) and the monster at loc0, and a response keeps that event and those locations
 * (Chain_AskResponse), so Trap Hole and the other summon traps may be chained to it. The human can; the CPU's chain
 * answers (AiTryChainResponse) only knew a list of cards, so it waited for the summon's own response window, which
 * comes after the trigger resolved (Mobius could destroy the trap first). AiTryChainResponse now asks this first:
 * for a summon of the human's, the first Set card that the summon window would use (spell speed 2 or more,
 * CanActivateFieldCard against the link's event and locations, AiShouldActivateSetCard) is chained. */
int AiChainSummonTriggerResponse(struct ChainEntry *entry)
{
    struct ChainEntry ref;
    int zone;

    if (entry->player != AI_HUMAN || (entry->event != RESPONSE_SUMMONED && entry->event != RESPONSE_FLIP_SUMMONED
                                      && entry->event != RESPONSE_SPECIAL_SUMMONED))
        return 0;
#ifdef HAS_CARD_ROYAL_OPPRESSION
    if (entry->event == RESPONSE_SPECIAL_SUMMONED) {
        for (zone = ZONE_SPELL_0; zone <= ZONE_SPELL_4; zone++) {
            struct DuelZone *z = ZoneOf(AI_PLAYER, zone);
            u16 id = DUEL_CARD_ID(&z->card);

            if (id == 0 || !z->isFaceUp || CardNumberOfId(id) != CARD_ROYAL_OPPRESSION)
                continue;
            MemCopy16(&ref, entry, sizeof(ref));
            if (CanActivateFieldCard(&ref, AI_PLAYER, zone) && AiShouldActivateAddedCard(&ref))
                return AiChainSetCard(entry, CARD_ROYAL_OPPRESSION);
        }
    }
#endif
    for (zone = ZONE_SPELL_0; zone <= ZONE_SPELL_4; zone++) {
        struct DuelZone *z = ZoneOf(AI_PLAYER, zone);
        u16 id = DUEL_CARD_ID(&z->card);

        if (id == 0 || z->isFaceUp || GetCardSpellSpeed(id) <= SPELL_SPEED_1)
            continue;
        MemCopy16(&ref, entry, sizeof(ref));    /* the summon's event and locations; the card is filled in */
        if (CanActivateFieldCard(&ref, AI_PLAYER, zone) && AiShouldActivateSetCard(&ref))
            return AiChainSetCard(entry, CardNumberOfId(id));
    }
    return 0;
}

/* Checks of the added cards' continuous rules, once per idle frame of the duel (DuelMainStep). 1 if one queued
 * something (the duel step waits for it). */
int ModDuelStateCheck(void)
{
#if defined(HAS_CARD_BERSERK_GORILLA) || defined(HAS_CARD_THUNDER_NYAN_NYAN) \
    || defined(HAS_CARD_LEVEL_LIMIT_AREA_B) || defined(HAS_CARD_FINAL_ATTACK_ORDERS)
    int player, zone;

    if (gDuel.phase == PHASE_BATTLE && gDuel.battleStage >= BATTLE_STAGE_SELECT_TARGET
        && gDuel.battleStage <= BATTLE_STAGE_END_ATTACK)
        return 0;
#ifdef HAS_CARD_LEVEL_LIMIT_AREA_B
    if (ApplyLevelLimitAreaB(AI_HUMAN))
        return 1;
#endif
#ifdef HAS_CARD_FINAL_ATTACK_ORDERS
    if (ApplyFinalAttackOrders(AI_HUMAN))
        return 1;
#endif
    for (player = 0; player <= 1; player++) {
#ifdef HAS_CARD_BERSERK_GORILLA
        for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
            struct DuelZone *z = ZoneOf(player, zone);
            u16 id = DUEL_CARD_ID(&z->card);

            if (id != 0 && z->isFaceUp && z->isDefense && CardNumberOfId(id) == CARD_BERSERK_GORILLA) {
                ShowCardEffect(player, id);
                DestroyFieldCard(player, zone, 1);
                return 1;
            }
        }
#endif
#ifdef HAS_CARD_THUNDER_NYAN_NYAN
        zone = DoomedThunderNyanNyan(player);
        if (zone >= 0) {
            ShowCardEffect(player, DUEL_CARD_ID(&ZoneOf(player, zone)->card));
            DestroyFieldCard(player, zone, 1);
            return 1;
        }
#endif
    }
#endif
    return 0;
}


static int CountFaceUpMonsters(int player)
{
    int zone, n = 0;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);
        if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp)
            n++;
    }
    return n;
}

static int CountSpellTrapCards(int player)
{
    int zone, n = 0;

    for (zone = ZONE_SPELL_0; zone <= ZONE_FIELD; zone++) {
        if (DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0)
            n++;
    }
    return n;
}

/* The highest ATK of the face-up monsters of `player`, -1 for none. */
static int StrongestFaceUpAtk(int player)
{
    int zone, best = -1;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);
        if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp && (int)GetZoneCardAtk(player, zone) > best)
            best = GetZoneCardAtk(player, zone);
    }
    return best;
}

#ifdef HAS_CARD_TRIBE_INFECTING_VIRUS
/* The Monster Type whose face-up monsters on the field would cost the human the most (theirs minus the CPU's);
 * *gain gets that difference. */
static int AiBestVirusType(int *gain)
{
    int type, zone, side, best = 0, bestGain = 0;

    for (type = 1; type <= CARD_TYPE_REPTILE; type++) {
        int g = 0;
        for (side = 0; side <= 1; side++) {
            for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
                struct DuelZone *z = ZoneOf(side, zone);
                if (DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp && (int)GetZoneCardType(side, zone) == type)
                    g += side == AI_HUMAN ? 1 : -1;
            }
        }
        if (g > bestGain) {
            bestGain = g;
            best = type;
        }
    }
    *gain = bestGain;
    return best;
}

/* Tribe-Infecting Virus' type declaration: DNA Surgery's prompt for the human; the CPU declares AiBestVirusType
 * (the prompt has no CPU branch). */
int EffectTribeInfectingVirusChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int gain;

    if (link->player) {
        link->numTargets = 0;
        AddEffectTarget(link, AiBestVirusType(&gain));
        return 1;
    }
    return EffectDeclareTypeChainB(link);
}
#endif

#if defined(HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END) || defined(HAS_CARD_SECRET_BARREL)
/* The cards of `player` in the hand and on the field (monster, Spell/Trap and Field zones). */
static int CountHandAndFieldCards(int player)
{
    int zone, n = gDuelPlayers[player & 1].handCount;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_FIELD; zone++) {
        if (DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0)
            n++;
    }
    return n;
}

/* Main Phase (AiPlayAddedCards) and Set-card windows (AiShouldActivateAddedCard). */
int AiWantsSecretBarrel(void)
{
    int cards = CountHandAndFieldCards(AI_HUMAN);
    int damage = cards * SECRET_BARREL_DAMAGE_PER_CARD;

    if (cards == 0)
        return 0;
    if (damage >= (int)gDuelPlayers[AI_HUMAN].lifePoints)
        return 1;
    return damage >= 600;
}
#endif

#ifdef HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END

/* The CPU wipes the board with Chaos Emperor Dragon when its damage would win (300 per card in both hands and on both
 * fields, the Dragon included; tokens are counted here too, so it may fall a little short), or when the human would
 * lose AI_CED_CARD_LEAD more cards than the CPU (not counting the Dragon itself). AiSelfLpLoss keeps it from paying its
 * last 1000 LP. */
#define AI_CED_CARD_LEAD 3

static int AiWantsChaosEmperorDragon(void)
{
    int human = CountHandAndFieldCards(AI_HUMAN), cpu = CountHandAndFieldCards(AI_PLAYER) - 1;

    return (human + cpu + 1) * CED_DAMAGE_PER_CARD >= gDuelPlayers[AI_HUMAN].lifePoints
           || human >= cpu + AI_CED_CARD_LEAD;
}
#endif

/* Should the CPU use the card-menu effect of its added monster `number` now? (CanActivateMonsterEffect said it may.) */
static int AiWantsMonsterEffect(u16 number, int zone)
{
    switch (number) {
#ifdef HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR
    case CARD_BREAKER_THE_MAGICAL_WARRIOR:      /* the target step takes the human's cards first */
        return CountSpellTrapCards(AI_HUMAN) > 0;
#endif
#ifdef HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING
    case CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING:
        return CountMonsters(AI_HUMAN) > 0;
#endif
#ifdef HAS_CARD_CHAOS_SORCERER
    case CARD_CHAOS_SORCERER:
        return CountFaceUpMonsters(AI_HUMAN) > 0;
#endif
#ifdef HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END
    case CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END:
        return AiWantsChaosEmperorDragon();
#endif
#ifdef HAS_CARD_TRIBE_INFECTING_VIRUS
    case CARD_TRIBE_INFECTING_VIRUS: {
        int gain;
        AiBestVirusType(&gain);
        return gain > 0 && gDuelPlayers[AI_PLAYER].handCount > 0;
    }
#endif
#ifdef HAS_CARD_EXILED_FORCE
    case CARD_EXILED_FORCE:                     /* only for a human monster stronger than itself */
        return CountMonsters(AI_HUMAN) > 0
               && (StrongestFaceUpAtk(AI_HUMAN) > (int)GetZoneCardAtk(AI_PLAYER, zone)
                   || CountFaceUpMonsters(AI_HUMAN) < CountMonsters(AI_HUMAN));
#endif
#ifdef HAS_CARD_ABYSS_SOLDIER
    case CARD_ABYSS_SOLDIER:
        return CountMonsters(AI_HUMAN) > 0 || CountSpellTrapCards(AI_HUMAN) > 0;
#endif
#ifdef HAS_CARD_THOUSAND_EYES_RESTRICT
    case CARD_THOUSAND_EYES_RESTRICT:
        return CountMonsters(AI_HUMAN) > 0;
#endif
#ifdef HAS_CARD_GOLEM_SENTRY
    case CARD_GOLEM_SENTRY:                     /* face down, so its next Flip Summon can bounce a monster */
        return CountMonsters(AI_HUMAN) > 0;
#endif
#ifdef HAS_CARD_ROULETTE_BARREL
    case CARD_ROULETTE_BARREL: {
        int level;
        for (level = 1; level <= 6; level++) {
            if (RouletteHasFaceUpMonsterOfLevel(level))
                return 1;
        }
        return 0;
    }
#endif
#ifdef HAS_CARD_GUARDIAN_SPHINX
    case CARD_GUARDIAN_SPHINX:                  /* face down, so its next Flip Summon bounces the human's monsters */
        return CountMonsters(AI_HUMAN) > 0;
#endif
#ifdef HAS_CARD_MAGICAL_SCIENTIST
    case CARD_MAGICAL_SCIENTIST:                /* AiUseMagicalScientist decides */
        return 0;
#endif
#ifdef HAS_CARD_HYSTERIC_FAIRY
    case CARD_HYSTERIC_FAIRY:                   /* two Tributes for 1000 LP is never worth it */
        return 0;
#endif
    }
    return 1;
}

#ifdef HAS_CARD_ENEMY_CONTROLLER
/* ---- Enemy Controller ----------------------------------------------------------------------------------------------
 * Quick-Play Spell: "Activate 1 of these effects.
 *   - Target 1 face-up monster your opponent controls; change that target's battle position.
 *   - Tribute 1 monster, then target 1 face-up monster your opponent controls; take control of that target until the
 *     End Phase."
 *   prepare  the opponent has a face-up monster the card may target
 *   chainA   the choice (kept in the link's unused unk12): the human picks from a two-line menu, offered only with a
 *            monster to Tribute; the CPU takes control when it can, else changes the position
 *   chainB   control: first the Tribute (the cost), Catapult Turtle's EffectTributeTargetChainB pick, a monster that
 *            cannot be Tributed refused, Tributed at once (as Metamorphosis does); the CPU Tributes its weakest. Then
 *            the target: a face-up monster of the opponent's (the human's cursor; the CPU, for control the highest ATK,
 *            for the position its first one)
 *   resolve  the target must still be face up there: the position changes (ChangeBattlePosition), or Change of
 *            Heart's resolve moves it to the player and links it (ZONE_LINK_CARD_EFFECT from Enemy Controller), and
 *            DuelCmd_TurnEnd gives it back (ReturnControlAtTurnEnd, duel_cmd_turn.c). Unlike Mind Control's, the monster
 *            may attack and be Tributed.
 *   CPU      AiWantsSpell: in Main Phase 1, to take a monster with AI_EC_GAIN more ATK than the weakest one it would
 *            Tribute */
static const u8 sStrEnemyControllerMenu[] = "Enemy Controller:\n    @3Change battle position\n    @2Take control";
static const u8 sStrEnemyControllerTarget[] = "Select a face-up monster of your opponent's.";
#define EC_MODE_POSITION 1
#define EC_MODE_CONTROL 2           /* the Tribute is still to be paid */
#define EC_MODE_CONTROL_PAID 3
#define AI_EC_GAIN 500

/* 1 if the face-up monster of `player` in `zone` may be targeted by card `card`. */
static int IsEnemyControllerTarget(u16 card, int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);

    return zone <= ZONE_MONSTER_4 && DUEL_CARD_ID(&z->card) != 0 && z->isFaceUp && CanCardTargetZone(card, player, zone);
}

/* The face-up target of the opponent of `player` with the highest ATK, -1 for none. */
static int EnemyControllerBestTarget(u16 card, int player)
{
    int opponent = 1 - (player & 1), zone, best = -1;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (IsEnemyControllerTarget(card, opponent, zone)
            && (best < 0 || GetZoneCardAtk(opponent, zone) > GetZoneCardAtk(opponent, best)))
            best = zone;
    }
    return best;
}

/* The monster of `player` with the lowest ATK that may be Tributed, -1 for none. */
static int EnemyControllerWeakestTribute(int player)
{
    int zone, best = -1;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (IsTributableMonster(player, zone)
            && (best < 0 || GetZoneCardAtk(player, zone) < GetZoneCardAtk(player, best)))
            best = zone;
    }
    return best;
}

int EffectEnemyControllerPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return EnemyControllerBestTarget(card->card, card->player) >= 0;
}

int EffectEnemyControllerChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int canControl = CountTributableMonsters(link->player, -1) > 0;

    if (link->player) {
        link->unk12 = canControl ? EC_MODE_CONTROL : EC_MODE_POSITION;
        return 1;
    }
    switch (gChain.costStep) {
    case 0:
        if (!canControl) {
            link->unk12 = EC_MODE_POSITION;
            return 1;
        }
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(19, 6), TEXTBOX_FLAGS_DEFAULT, sStrEnemyControllerMenu);
        TextBoxSetMenu(TEXTBOX_MENU_TWO_CHOICE, NULL, NULL);
        gChain.costStep++;
        return 0;
    default:
        /* the chain calls a cost handler every frame, text box or not: wait for the menu to close */
        if (gTextBox.active)
            return 0;
        link->unk12 = gTextBox.result ? EC_MODE_CONTROL : EC_MODE_POSITION;
        return 1;
    }
}

int EffectEnemyControllerChainB(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int opponent = 1 - (link->player & 1), zone;

    /* the Tribute first (the control effect's cost) */
    if (link->unk12 == EC_MODE_CONTROL) {
        if (link->player) {
            zone = EnemyControllerWeakestTribute(link->player);
            if (zone < 0) {
                link->numTargets = 0;
                return 1;
            }
            TributeMonster(link->player, zone);
        } else {
            if (!EffectTributeTargetChainB(link))
                return 0;
            if (link->numTargets != 1 || (u8)link->targets[0] != link->player
                || !IsTributableMonster(link->player, link->targets[0] >> 8)) {
                link->numTargets = 0;
                PlaySE(SE_ERROR);
                return 0;
            }
            TributeMonster(link->player, link->targets[0] >> 8);
            gChain.targetStep = 0;
        }
        link->numTargets = 0;
        link->unk12 = EC_MODE_CONTROL_PAID;
        return 0;
    }
    /* the target */
    if (link->player) {
        link->numTargets = 0;
        zone = EnemyControllerBestTarget(link->card, link->player);
        if (link->unk12 == EC_MODE_POSITION) {
            for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
                if (IsEnemyControllerTarget(link->card, opponent, zone))
                    break;
            }
        }
        if (zone >= ZONE_MONSTER_0 && zone <= ZONE_MONSTER_4)
            TryAddEffectTarget(link, opponent, zone);
        return 1;
    }
    switch (gChain.targetStep) {
    case 0:
        link->numTargets = 0;
        if (EnemyControllerBestTarget(link->card, link->player) < 0)
            return 1;
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, sStrEnemyControllerTarget);
        gChain.targetStep++;
        return 0;
    default:
        if (DuelCursor_PickTarget(PICK_PLAYER1(PICK_FACE_UP_MONSTER_ANY)) == 0)
            return 0;
        zone = gDuelScreen.selArea + gDuelScreen.selIndex;
        if (gDuelScreen.selPlayer == opponent && IsEnemyControllerTarget(link->card, opponent, zone)
            && TryAddEffectTarget(link, opponent, zone) != 0)
            return 1;
        PlaySE(SE_ERROR);
        return 0;
    }
}

/* 1 if a link of the chain other than `self` names (player, zone): its event locations or its targets. The engine
 * knows a card by its zone, so a link still to resolve that named a monster gone from the zone (Trap Hole on a summon
 * whose monster was then Tributed) would hit whatever came there next. */
static int IsZoneNamedByChain(struct ChainEntry *self, int player, int zone)
{
    u16 loc = DUEL_LOC(player, zone);
    int i, t;

    for (i = 0; i < gChain.linkCount; i++) {
        struct ChainEntry *e = &gChain.links[i];

        if (e == self)
            continue;
        if (e->loc0 == loc || e->loc1 == loc)
            return 1;
        for (t = 0; t < e->numTargets; t++) {
            if (e->targets[t] == loc)
                return 1;
        }
    }
    return 0;
}

/* The free monster zone of `player` the taken monster goes to: the first one no other link names, else the first. */
static int EnemyControllerFreeZone(struct ChainEntry *self, int player)
{
    int zone, first = -1;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        if (!IsMonsterZoneFree(player, zone))
            continue;
        if (first < 0)
            first = zone;
        if (!IsZoneNamedByChain(self, player, zone))
            return zone;
    }
    return first;
}

int EffectEnemyControllerResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    int player, zone, freeZone;

    if (link->negated || link->numTargets != 1)
        return EFFECT_STEP_DONE;
    player = (u8)link->targets[0];
    zone = link->targets[0] >> 8;
    if (player == link->player || !IsEnemyControllerTarget(link->card, player, zone))
        return EFFECT_STEP_DONE;
    if (link->unk12 == EC_MODE_POSITION) {
        ChangeBattlePosition(player, zone, 0, 0);
        return EFFECT_STEP_DONE;
    }
    /* Change of Heart's move and link, into a zone no other link of the chain names: a Trap Hole chained below on
     * the summon of the monster Tributed for the cost names that monster's zone, which is free now (the first free
     * one, where Change of Heart's resolve would put it), and would destroy the taken monster there */
    freeZone = EnemyControllerFreeZone(link, link->player);
    if (freeZone < 0)
        return EFFECT_STEP_DONE;
    MoveFieldCard(link->player, link->targets[0], DUEL_LOC(link->player, freeZone));
    QueueAddZoneLink(link->player, link->card, DUEL_LOC(link->player, freeZone), ZONE_LINK_CARD_EFFECT);
    return EFFECT_STEP_DONE;
}

/* AiWantsSpell: take a face-up monster of the human's in Main Phase 1 when it has AI_EC_GAIN more ATK than the CPU's
 * weakest monster it may Tribute (the Tribute frees the zone the stolen monster goes to). */
static int AiWantsEnemyController(void)
{
    int target, tribute;

    if (gAiState.step != AI_STEP_MAIN_PHASE_1)
        return 0;
    target = EnemyControllerBestTarget(gCardNumberToId[CARD_ENEMY_CONTROLLER], AI_PLAYER);
    tribute = EnemyControllerWeakestTribute(AI_PLAYER);
    return target >= 0 && tribute >= 0
           && (int)GetZoneCardAtk(AI_HUMAN, target) >= (int)GetZoneCardAtk(AI_PLAYER, tribute) + AI_EC_GAIN;
}
#endif /* HAS_CARD_ENEMY_CONTROLLER */

#ifdef HAS_CARD_POISON_OF_THE_OLD_MAN
/* ---- Poison of the Old Man -----------------------------------------------------------------------------------------
 * Quick-Play Spell: "Activate 1 of these effects; ● Gain 1200 LP. ● Inflict 800 damage to your opponent."
 *   chainA   the choice, kept in the link's unused unk12 as Enemy Controller's: the human picks from a two-line menu
 *            (the handler waits while the box is open: Chain_Build calls it every frame); the CPU decides
 *            (AiPoisonOfTheOldManChoice)
 *   resolve  GainLifePoints (1200) for the player, or LoseLifePoints (800) for the opponent
 *   CPU      AiPoisonOfTheOldManChoice: the damage when it wins the duel, the LP while it has POTOM_AI_LOW_LP or
 *            less, else the damage; AiWantsSpell plays it whenever it may (AiPlayAddedCards, Main Phase) */
static const u8 sStrPoisonOfTheOldManMenu[] = "Poison of the Old Man:\n    @3Gain 1200 LP\n    @2Inflict 800 damage";
#define POTOM_GAIN          1200
#define POTOM_DAMAGE        800
#define POTOM_MODE_GAIN     1
#define POTOM_MODE_DAMAGE   2
#define POTOM_AI_LOW_LP     3000

static int AiPoisonOfTheOldManChoice(int player)
{
    if ((int)gDuelPlayers[(1 - player) & 1].lifePoints <= POTOM_DAMAGE)
        return POTOM_MODE_DAMAGE;
    return (int)gDuelPlayers[player & 1].lifePoints <= POTOM_AI_LOW_LP ? POTOM_MODE_GAIN : POTOM_MODE_DAMAGE;
}

int EffectPoisonOfTheOldManChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    if (link->player) {
        link->unk12 = AiPoisonOfTheOldManChoice(link->player);
        return 1;
    }
    switch (gChain.costStep) {
    case 0:
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(19, 6), TEXTBOX_FLAGS_DEFAULT, sStrPoisonOfTheOldManMenu);
        TextBoxSetMenu(TEXTBOX_MENU_TWO_CHOICE, NULL, NULL);
        gChain.costStep++;
        return 0;
    default:
        if (gTextBox.active)
            return 0;
        link->unk12 = gTextBox.result ? POTOM_MODE_DAMAGE : POTOM_MODE_GAIN;
        return 1;
    }
}

int EffectPoisonOfTheOldManResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (link->negated)
        return EFFECT_STEP_DONE;
    if (link->unk12 == POTOM_MODE_GAIN)
        GainLifePoints(link->player, POTOM_GAIN);
    else
        LoseLifePoints(1 - (link->player & 1), POTOM_DAMAGE);
    return EFFECT_STEP_DONE;
}
#endif /* HAS_CARD_POISON_OF_THE_OLD_MAN */

/* InflictBattleDamage (duel_card_actions.c): 1 if `player` takes no damage now (Hallowed Life Barrier). LP paid as a
 * cost does not go through it, nor through LoseLifePoints. */
int ModTakesNoDamage(int player)
{
#ifdef HAS_CARD_HALLOWED_LIFE_BARRIER
    return gModDuel.noDamageTurn[player & 1] == (u16)(gDuel.turnCount + 1);
#else
    return 0;
#endif
}

/* LoseLifePoints (duel_card_actions.c), the effect damage: 1 if `player` takes none now (Hallowed Life Barrier, or
 * Pikeru's Circle of Enchantment). */
int ModTakesNoEffectDamage(int player)
{
    if (ModTakesNoDamage(player))
        return 1;
#ifdef HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT
    return gModDuel.noEffectDamageTurn[player & 1] == (u16)(gDuel.turnCount + 1);
#else
    return 0;
#endif
}

#ifdef HAS_CARD_HALLOWED_LIFE_BARRIER
/* ---- Hallowed Life Barrier -----------------------------------------------------------------------------------------
 * Normal Trap, its original TCG text (the fetched one is the later "damage you take from your opponent's cards and
 * effects becomes 0"): "Discard 1 card. You take no damage this turn."
 *   prepare  a card in the hand to discard
 *   chainA   the cost: the engine's discard prompt (DuelPrompt_PostDiscardCost, any card; the CPU's own pick)
 *   resolve  gModDuel.noDamageTurn[player] = this turn: ModTakesNoDamage stops the damage of LoseLifePoints (effect
 *            damage) and InflictBattleDamage (battle damage, and with it the battle damage response) for the player
 *            until the turn ends. LP paid as a cost goes straight to DUEL_CMD_LOSE_LP and is still paid.
 *   CPU      AiShouldActivateAddedCard: against an attack of the human's that would deal HLB_AI_MIN_DAMAGE or more,
 *            or all of its LP (AiIncomingAttackDamage); AiChainSolemnJudgment (AiTryChainResponse): chained to the
 *            human's burn Spell or Trap that would deal HLB_AI_MIN_BURN or more, or all of its LP (AiIncomingBurn) */
#define HLB_AI_MIN_DAMAGE 1500
#define HLB_AI_MIN_BURN 800

int EffectHallowedLifeBarrierPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    return !fromHand && gDuelPlayers[card->player & 1].handCount > 0;
}

int EffectHallowedLifeBarrierChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    DuelPrompt_PostDiscardCost(link->player, 1, FALSE, FALSE);
    return 1;
}

int EffectHallowedLifeBarrierResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated)
        gModDuel.noDamageTurn[link->player & 1] = gDuel.turnCount + 1;
    return EFFECT_STEP_DONE;
}

/* The battle damage the CPU takes from the attack being declared (gBattle): the attacker's ATK when direct, the
 * difference against its face-up Attack Position monster; 0 against a Defense Position one (piercing not counted). */
static int AiIncomingAttackDamage(void)
{
    struct DuelZone *def;
    int atk = GetZoneCardAtk(AI_HUMAN, gBattle.atkSlot);

    if (gBattle.direct)
        return atk;
    def = ZoneOf(AI_PLAYER, gBattle.defSlot);
    if (DUEL_CARD_ID(&def->card) == 0)
        return atk;
    if (def->isDefense || !def->isFaceUp)
        return 0;
    atk -= GetZoneCardAtk(AI_PLAYER, gBattle.defSlot);
    return atk > 0 ? atk : 0;
}

/* The damage the human's Spell/Trap link `entry` deals the CPU: the burn cards (Sparks, Hinotama, Final Flame, Ookazi,
 * Tremendous Fire, Just Desserts, and Poison of the Old Man when its damage was chosen); 0 for any other card. */
static int AiIncomingBurn(struct ChainEntry *entry)
{
    switch (CardNumberOfId(entry->card)) {
    case CARD_SPARKS:
        return 200;
    case CARD_HINOTAMA:
        return 500;
    case CARD_FINAL_FLAME:
        return 600;
    case CARD_OOKAZI:
        return 800;
    case CARD_TREMENDOUS_FIRE:
        return 1000;
    case CARD_JUST_DESSERTS:
        return 500 * CountMonsters(AI_PLAYER);
#ifdef HAS_CARD_POISON_OF_THE_OLD_MAN
    case CARD_POISON_OF_THE_OLD_MAN:
        return entry->unk12 == POTOM_MODE_DAMAGE ? POTOM_DAMAGE : 0;
#endif
#ifdef HAS_CARD_SECRET_BARREL
    case CARD_SECRET_BARREL:
        return CountHandAndFieldCards(AI_PLAYER) * SECRET_BARREL_DAMAGE_PER_CARD;
#endif
    }
    return 0;
}

/* AiChainSolemnJudgment: chain a Set Hallowed Life Barrier to the human's burn card that hurts. 1 if it did. */
static int AiChainHallowedLifeBarrier(struct ChainEntry *entry)
{
    int damage = AiIncomingBurn(entry);

    if (damage < HLB_AI_MIN_BURN && damage < (int)gDuelPlayers[AI_PLAYER].lifePoints)
        return 0;
    if (damage <= 0 || ModTakesNoEffectDamage(AI_PLAYER))
        return 0;
    return AiChainSetCard(entry, CARD_HALLOWED_LIFE_BARRIER) != 0;
}
#endif /* HAS_CARD_HALLOWED_LIFE_BARRIER */

#ifdef HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT
/* ---- Pikeru's Circle of Enchantment -------------------------------------------------------------------------------
 * Normal Trap: "Damage to you from card effects becomes 0 until the end of this turn."
 *   resolve  gModDuel.noEffectDamageTurn[player] = this turn: ModTakesNoEffectDamage stops the damage of
 *            LoseLifePoints (every effect damage) for the player until the turn ends. Battle damage
 *            (InflictBattleDamage) and LP paid as a cost are not effect damage.
 *   CPU      only chained to the human's burn Spell or Trap (AiChainPikerusCircle, in AiChainSolemnJudgment before
 *            Hallowed Life Barrier, which costs a card): PIKERU_AI_MIN_BURN or more, or all of its LP
 *            (AiIncomingBurn); AiShouldActivateAddedCard keeps it Set otherwise */
#define PIKERU_AI_MIN_BURN 500

int EffectPikerusCircleResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated)
        gModDuel.noEffectDamageTurn[link->player & 1] = gDuel.turnCount + 1;
    return EFFECT_STEP_DONE;
}

static int AiChainPikerusCircle(struct ChainEntry *entry)
{
    int damage = AiIncomingBurn(entry);

    if (damage <= 0 || ModTakesNoEffectDamage(AI_PLAYER))
        return 0;
    if (damage < PIKERU_AI_MIN_BURN && damage < (int)gDuelPlayers[AI_PLAYER].lifePoints)
        return 0;
    return AiChainSetCard(entry, CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT) != 0;
}
#endif /* HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT */

#ifdef HAS_CARD_EMERGENCY_PROVISIONS
/* ---- Emergency Provisions ------------------------------------------------------------------------------------------
 * Quick-Play Spell: "Send any number of other Spell/Trap Cards you control to the Graveyard; gain 1000 LP for each card
 * sent to the Graveyard this way."
 *   prepare  another Spell/Trap of the player's on the field (zones 5-10, the Field Spell too)
 *   chainA   the cost, counted in the link's unused unk12. The human: a prompt, then cursor picks of their other
 *            Spells/Traps, each sent at once (SendFieldCardToGrave), until B (after at least one) or none is left. The
 *            CPU: the ones the link it answers targets, when that targets its Spells/Traps (Mystical Space Typhoon,
 *            Dust Tornado, ...), else all of its others (against Heavy Storm, Harpie's Feather Duster, ...)
 *   resolve  GainLifePoints, 1000 a card
 *   CPU      a Set copy is chained (AiChainSolemnJudgment) to the human's Heavy Storm or Harpie's Feather Duster, or to
 *            a card that targets its Spells/Traps (AiChainEmergencyProvisions); from the hand in its Main Phase
 *            (AiWantsSpell) only with EP_AI_LOW_LP or less and 2 or more other Spells/Traps */
static const u8 sStrEmergencyProvisionsPick[] = "Select the Spell/Trap Cards to send to the Graveyard. Press B when done.";
#define EP_GAIN         1000
#define EP_AI_LOW_LP    1500

/* 1 if (player, zone) holds a Spell/Trap of the player's that Emergency Provisions in `own` may send. */
static int IsProvisionsCost(int player, int zone, int own)
{
    return zone >= ZONE_SPELL_0 && zone <= ZONE_FIELD && zone != own
           && DUEL_CARD_ID(&ZoneOf(player, zone)->card) != 0;
}

static int CountProvisionsCosts(int player, int own)
{
    int zone, n = 0;

    for (zone = ZONE_SPELL_0; zone <= ZONE_FIELD; zone++)
        n += IsProvisionsCost(player, zone, own);
    return n;
}

/* The zone of the Emergency Provisions `card` itself, which it may not send: card->zone when it holds that card (always
 * so for a chain link, as when the cost is paid: exactly this copy is kept), else the player's first Spell/Trap zone
 * with that card (CanActivateEffectOfCard leaves the zone of its entry unset), -1 when none is on the field. */
static int ProvisionsOwnZone(struct ChainEntry *card)
{
    u16 id = card->card & CARD_ID_MASK;
    int zone = card->zone;

    if (zone >= ZONE_SPELL_0 && zone <= ZONE_FIELD && DUEL_CARD_ID(&ZoneOf(card->player, zone)->card) == id)
        return zone;
    for (zone = ZONE_SPELL_0; zone <= ZONE_SPELL_4; zone++) {
        if (DUEL_CARD_ID(&ZoneOf(card->player, zone)->card) == id)
            return zone;
    }
    return -1;
}

int EffectEmergencyProvisionsPrepare(struct ChainEntry *card, struct ChainEntry *chainLink, u16 fromHand)
{
    /* from the hand it is not on the field yet: every Spell/Trap counts, a Set copy too. On the field the check may
     * not give its zone (CanActivateEffectOfCard leaves it unset), but leaving out any one copy of it counts the same */
    return CountProvisionsCosts(card->player, fromHand ? -1 : ProvisionsOwnZone(card)) > 0;
}

/* 1 if link `prev` targets a Spell/Trap of `player`'s other than zone `own`. */
static int TargetsProvisionsCost(struct ChainEntry *prev, int player, int own)
{
    int t;

    if (prev == NULL)
        return 0;
    for (t = 0; t < prev->numTargets; t++) {
        if ((u8)prev->targets[t] == player && IsProvisionsCost(player, prev->targets[t] >> 8, own))
            return 1;
    }
    return 0;
}

int EffectEmergencyProvisionsChainA(struct ChainEntry *link, struct ChainEntry *prevLink)
{
    int zone, t, own = ProvisionsOwnZone(link);

    if (link->player) {
        link->unk12 = 0;
        if (TargetsProvisionsCost(prevLink, link->player, own)) {
            for (t = 0; t < prevLink->numTargets; t++) {
                zone = prevLink->targets[t] >> 8;
                if ((u8)prevLink->targets[t] == link->player && IsProvisionsCost(link->player, zone, own)) {
                    SendFieldCardToGrave(link->player, zone, 0, 1);
                    link->unk12++;
                }
            }
            return 1;
        }
        for (zone = ZONE_SPELL_0; zone <= ZONE_FIELD; zone++) {
            if (IsProvisionsCost(link->player, zone, own)) {
                SendFieldCardToGrave(link->player, zone, 0, 1);
                link->unk12++;
            }
        }
        return 1;
    }
    switch (gChain.costStep) {
    case 0:
        link->unk12 = 0;
        TextBoxOpen(TEXTBOX_POS(6, 2), TEXTBOX_SIZE(18, 7), TEXTBOX_FLAGS_DEFAULT, sStrEmergencyProvisionsPick);
        gChain.costStep++;
        return 0;
    default:
        if (CountProvisionsCosts(link->player, own) == 0)
            return 1;
        if (gMain.newKeys & B_BUTTON) {
            if (link->unk12 > 0)
                return 1;
            PlaySE(SE_ERROR);
            return 0;
        }
        if (DuelCursor_PickTarget(PICK_ANY_SPELL_TRAP) == 0)
            return 0;
        zone = gDuelScreen.selArea + gDuelScreen.selIndex;
        if (gDuelScreen.selPlayer != link->player || !IsProvisionsCost(link->player, zone, own)) {
            PlaySE(SE_ERROR);
            return 0;
        }
        SendFieldCardToGrave(link->player, zone, 0, 1);
        link->unk12++;
        return 0;
    }
}

int EffectEmergencyProvisionsResolve(struct ChainEntry *link, struct ChainEntry *chainedTo)
{
    if (!link->negated && link->unk12 > 0)
        GainLifePoints(link->player, EP_GAIN * link->unk12);
    return EFFECT_STEP_DONE;
}

/* AiChainSolemnJudgment: chain a Set Emergency Provisions to the human's card that would clear the CPU's
 * Spells/Traps. 1 if it did. */
static int AiChainEmergencyProvisions(struct ChainEntry *entry)
{
    u16 number = CardNumberOfId(entry->card);

    if (CountProvisionsCosts(AI_PLAYER, -1) < 2)    /* itself and at least one more */
        return 0;
    if (number != CARD_HEAVY_STORM && number != CARD_HARPIES_FEATHER_DUSTER
        && !TargetsProvisionsCost(entry, AI_PLAYER, -1))
        return 0;
    return AiChainSetCard(entry, CARD_EMERGENCY_PROVISIONS) != 0;
}

/* AiWantsSpell: from the hand only when low on LP, with 2 or more other Spells/Traps to turn into LP. */
static int AiWantsEmergencyProvisions(void)
{
    return (int)gDuelPlayers[AI_PLAYER].lifePoints <= EP_AI_LOW_LP && CountProvisionsCosts(AI_PLAYER, -1) >= 2;
}
#endif /* HAS_CARD_EMERGENCY_PROVISIONS */

#ifdef HAS_CARD_MONSTER_REINCARNATION
static int AiWantsMonsterReincarnation(void)
{
    if (CountMonsters(AI_PLAYER) != 0)
        return 0;
    if (gDuelPlayers[AI_PLAYER].handCount < 1)
        return 0;
    return CollectEffectTargetsInt(AI_PLAYER, CARD_MONSTER_REINCARNATION, 0) > 0;
}
#endif /* HAS_CARD_MONSTER_REINCARNATION */

#ifdef HAS_CARD_TOON_TABLE_OF_CONTENTS
static int AiWantsToonTableOfContents(void)
{
    return CollectEffectTargetsInt(AI_PLAYER, CARD_TOON_TABLE_OF_CONTENTS, 0) > 0;
}
#endif /* HAS_CARD_TOON_TABLE_OF_CONTENTS */

#ifdef HAS_CARD_TOKEN_THANKSGIVING
static int AiWantsTokenThanksgiving(void)
{
    return CountTokensOnField() > 0;
}
#endif /* HAS_CARD_TOKEN_THANKSGIVING */

/* Should the CPU play its added Magic card `number` in its Main Phase now? (AiTryPlaySpellTrap checks that it may.) */
static int AiWantsSpell(u16 number)
{
    switch (number) {
#ifdef HAS_CARD_ENEMY_CONTROLLER
    case CARD_ENEMY_CONTROLLER:                 /* to take a much stronger monster */
        return AiWantsEnemyController();
#endif
#ifdef HAS_CARD_POISON_OF_THE_OLD_MAN
    case CARD_POISON_OF_THE_OLD_MAN:            /* always: AiPoisonOfTheOldManChoice picks the effect */
        return 1;
#endif
#ifdef HAS_CARD_MYSTIK_WOK
    case CARD_MYSTIK_WOK:
        return AiWantsMystikWok();
#endif
#ifdef HAS_CARD_MONSTER_REINCARNATION
    case CARD_MONSTER_REINCARNATION:
        return AiWantsMonsterReincarnation();
#endif
#ifdef HAS_CARD_TOON_TABLE_OF_CONTENTS
    case CARD_TOON_TABLE_OF_CONTENTS:
        return AiWantsToonTableOfContents();
#endif
#ifdef HAS_CARD_TOKEN_THANKSGIVING
    case CARD_TOKEN_THANKSGIVING:
        return AiWantsTokenThanksgiving();
#endif
#ifdef HAS_CARD_EMERGENCY_PROVISIONS
    case CARD_EMERGENCY_PROVISIONS:             /* only when low on LP */
        return AiWantsEmergencyProvisions();
#endif
#ifdef HAS_CARD_CARD_DESTRUCTION
    case CARD_CARD_DESTRUCTION:                 /* when the human holds more cards than the CPU would discard */
        return gDuelPlayers[AI_PLAYER].handCount - 1 < gDuelPlayers[AI_HUMAN].handCount;
#endif
#ifdef HAS_CARD_CREATURE_SWAP
    case CARD_CREATURE_SWAP: {                  /* give the weakest monster for a stronger one */
        int weakest = AiFindWeakestMonster(AI_PLAYER, -1, 1, 1);
        return weakest >= 0 && StrongestFaceUpAtk(AI_HUMAN) > (int)GetZoneCardAtk(AI_PLAYER, weakest);
    }
#endif
#ifdef HAS_CARD_SCAPEGOAT
    case CARD_SCAPEGOAT:                        /* kept for the human's attack (AiShouldActivateAddedCard) */
        return 0;
#endif
#ifdef HAS_CARD_MONSTER_GATE
    case CARD_MONSTER_GATE: {                   /* Tribute a weak monster for whatever the Deck gives */
        int zone = GateWeakestTribute(AI_PLAYER);
        return zone >= 0 && GetZoneCardAtk(AI_PLAYER, zone) < 1500;
    }
#endif
#ifdef HAS_CARD_BOOK_OF_MOON
    case CARD_BOOK_OF_MOON:                     /* clears the way for the CPU's attack */
        return CountFaceUpMonsters(AI_HUMAN) > 0 && CountMonsters(AI_PLAYER) > 0;
#endif
    }
    return 1;
}

#ifdef HAS_LIGHT_DARK_BANISH_SUMMON
/* The Graveyard index of the weakest monster of `attribute` of the CPU (not `skip`), -1 for none. */
static int WeakestGraveyardMonsterOf(int attribute, int skip)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    int i, best = -1;
    u32 bestAtk = 0;

    for (i = 0; i < p->graveCount; i++) {
        u32 stats = gCardStats[DUEL_CARD_ID(&p->graveyard[i]) & CARD_ID_MASK];
        if (i != skip && CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && (int)CARD_STATS_ATTR(stats) == attribute
            && (best < 0 || CARD_STATS_ATK(stats) < bestAtk)) {
            best = i;
            bestAtk = CARD_STATS_ATK(stats);
        }
    }
    return best;
}

/* Special Summon Black Luster Soldier or Chaos Sorcerer from the CPU's hand, banishing the weakest LIGHT and DARK
 * monsters of its Graveyard, or Gigantes, banishing the weakest EARTH one (the human's summon steps are a card-list
 * UI with no CPU branch). 1 if it did. */
static int AiLightDarkBanishSummon(void)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    int i, light, dark;
    u16 cards[2][2];

    if (CountFreeMonsterZones(AI_PLAYER) == 0)
        return 0;
    for (i = 0; i < p->handCount; i++) {
        u16 number = CardNumberOfId(DUEL_CARD_ID(&p->hand[i]));
#ifdef HAS_CARD_GIGANTES
        if (number == CARD_GIGANTES) {      /* Gigantes: banish the weakest EARTH monster */
            int earth = WeakestGraveyardMonsterOf(ATTRIBUTE_EARTH, -1);

            if (earth < 0)
                continue;
            cards[0][0] = ((u16 *)&p->graveyard[earth])[0];
            cards[0][1] = ((u16 *)&p->graveyard[earth])[1];
            ShowCardEffect(AI_PLAYER, DUEL_CARD_ID(&p->hand[i]));
            BanishGraveyardCard(AI_PLAYER, cards[0]);
            QueueSpecialSummonFromHand(AI_PLAYER, i, FindFreeMonsterZone(AI_PLAYER), 0, 1);
            return 1;
        }
#endif
        if (!CanPayLightDarkBanishCost(AI_PLAYER))
            continue;
        if (
#ifdef HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING
            number != CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING &&
#endif
#ifdef HAS_CARD_CHAOS_SORCERER
            number != CARD_CHAOS_SORCERER &&
#endif
#ifdef HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END
            number != CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END &&
#endif
            1)
            continue;
        light = WeakestGraveyardMonsterOf(ATTRIBUTE_LIGHT, -1);
        dark = WeakestGraveyardMonsterOf(ATTRIBUTE_DARK, light);
        if (light < 0 || dark < 0)
            return 0;
        /* copy both card words first: the banish commands only run later, but keep the halves stable */
        cards[0][0] = ((u16 *)&p->graveyard[light])[0];
        cards[0][1] = ((u16 *)&p->graveyard[light])[1];
        cards[1][0] = ((u16 *)&p->graveyard[dark])[0];
        cards[1][1] = ((u16 *)&p->graveyard[dark])[1];
        ShowCardEffect(AI_PLAYER, DUEL_CARD_ID(&p->hand[i]));
        BanishGraveyardCard(AI_PLAYER, cards[0]);
        BanishGraveyardCard(AI_PLAYER, cards[1]);
        QueueSpecialSummonFromHand(AI_PLAYER, i, FindFreeMonsterZone(AI_PLAYER), 0, 1);
        return 1;
    }
    return 0;
}
#endif

/* ---- Mystical Space Typhoon, Gust, Driving Snow (the CPU's target) -------------------------------------------------
 * EffectSpellTrapTargetChainB (effect_targets3.c) asks this for the CPU. Only the human's cards, zones 5-10: the
 * first face-up Magic or Trap (Continuous Traps such as Call of the Haunted or Royal Decree, Equips, the Field Spell),
 * else the first face-down card. None: no target. */
void AiPickSpellTrapToDestroy(struct ChainEntry *link)
{
    int pass, zone;

    link->numTargets = 0;
    for (pass = 0; pass <= 1; pass++) {
        for (zone = ZONE_SPELL_0; zone <= ZONE_FIELD; zone++) {
            struct DuelZone *z = ZoneOf(AI_HUMAN, zone);

            /* pass 0: face up, pass 1: face down */
            if (DUEL_CARD_ID(&z->card) != 0 && (pass == 0) == (z->isFaceUp != 0)
                && TryAddEffectTarget(link, AI_HUMAN, zone) != 0)
                return;
        }
    }
}

/* ---- Nobleman of Crossout (the CPU) -------------------------------------------------------------------------------
 * The vanilla CPU never plays Nobleman of Crossout: it is in neither of AiPlaySpells' lists (gAiGenericSpells,
 * gAiEquipSpells; checked in the ROM) and its target step (EffectNoblemanOfCrossoutChainB, effect_targets3.c) only
 * waits for the human's cursor. AiPlayAddedCards plays it while the human controls a face-down monster, and the
 * target step calls AiNoblemanOfCrossoutTarget for the CPU: the human's first face-down monster it may target. */
static int HasFaceDownMonster(int player)
{
    int zone;

    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);

        if (DUEL_CARD_ID(&z->card) != 0 && !z->isFaceUp)
            return 1;
    }
    return 0;
}

void AiNoblemanOfCrossoutTarget(struct ChainEntry *link)
{
    int zone;

    link->numTargets = 0;
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(AI_HUMAN, zone);

        if (DUEL_CARD_ID(&z->card) != 0 && !z->isFaceUp && TryAddEffectTarget(link, AI_HUMAN, zone) != 0)
            return;
    }
}

int AiPlayAddedCards(void)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    int zone, i;

#ifdef HAS_LIGHT_DARK_BANISH_SUMMON
    if (CountFreeMonsterZones(AI_PLAYER) > 0 && AiLightDarkBanishSummon())
        return 1;
#endif
    /* the card-menu effects of face-up added monsters */
    for (zone = ZONE_MONSTER_0; zone <= ZONE_MONSTER_4; zone++) {
        struct DuelZone *z = ZoneOf(AI_PLAYER, zone);
        u16 id = DUEL_CARD_ID(&z->card);

        if (id == 0 || !z->isFaceUp || !IsAddedCardId(id) || !CanActivateMonsterEffect(id, AI_PLAYER, zone)
            || !AiWantsMonsterEffect(CardNumberOfId(id), zone))
            continue;
        DuelCmd_Push(DUEL_CMD_PLAYER | DUEL_CMD_POINT_AT_CARD, AI_PLAYER, (u8)zone << 8, 0);
        Chain_AddPending(((u32)(zone & 0x1F) << 16) | id | CPU_CHAIN_KIND_MONSTER, 0);
        return 1;
    }
    /* Nobleman of Crossout on a face-down monster of the human's (see AiNoblemanOfCrossoutTarget) */
    if (HasFaceDownMonster(AI_HUMAN) && AiTryPlaySpellTrap(CARD_NOBLEMAN_OF_CROSSOUT))
        return 1;
#ifdef HAS_CARD_SECRET_BARREL
    /* Set Secret Barrel: flip and resolve in the Main Phase when the human holds enough cards */
    if (AiWantsSecretBarrel() && AiTryPlaySpellTrap(CARD_SECRET_BARREL))
        return 1;
#endif
#ifdef HAS_CARD_LEVEL_LIMIT_AREA_B
    if (AiTryPlaySpellTrap(CARD_LEVEL_LIMIT_AREA_B))
        return 1;
#endif
#ifdef HAS_CARD_GRAVITY_BIND
    if (AiTryPlaySpellTrap(CARD_GRAVITY_BIND))
        return 1;
#endif
#ifdef HAS_CARD_FINAL_ATTACK_ORDERS
    if (AiTryPlaySpellTrap(CARD_FINAL_ATTACK_ORDERS))
        return 1;
#endif
#ifdef HAS_CARD_ROYAL_OPPRESSION
    if (AiTryPlaySpellTrap(CARD_ROYAL_OPPRESSION))
        return 1;
#endif
    /* added Magic cards, in the hand or Set */
    for (i = 0; i < p->handCount; i++) {
        u16 id = DUEL_CARD_ID(&p->hand[i]);
        if (IsAddedCardId(id) && CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) == CARD_TYPE_MAGIC
            && AiWantsSpell(CardNumberOfId(id)) && AiTryPlaySpellTrap(CardNumberOfId(id)))
            return 1;
    }
    for (zone = ZONE_SPELL_0; zone <= ZONE_SPELL_4; zone++) {
        u16 id = DUEL_CARD_ID(&ZoneOf(AI_PLAYER, zone)->card);
        if (id != 0 && IsAddedCardId(id) && CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) == CARD_TYPE_MAGIC
            && AiWantsSpell(CardNumberOfId(id)) && AiTryPlaySpellTrap(CardNumberOfId(id)))
            return 1;
    }
    return 0;
}

/* AiStepChangePositions (ai_steps.c), for a CPU monster that can beat none of the human's monsters: 1 if it is face
 * up in Attack Position and a face-up monster of the human (in either position: it can attack next turn) has higher
 * ATK, so the CPU switches it to Defense Position instead of leaving it to be run over. A strong monster
 * (RETREAT_KEEP_ATK or more) stays in Attack Position unless the strongest threat outclasses it by RETREAT_GAP. */
#define RETREAT_KEEP_ATK    1700
#define RETREAT_GAP         1000

int AiShouldRetreatToDefense(int zone)
{
    struct DuelZone *z = ZoneOf(1, zone);
    u32 atk, best = 0;
    int i;

    if (DUEL_CARD_ID(&z->card) == 0 || !z->isFaceUp || z->isDefense)
        return 0;
#ifdef HAS_CARD_BERSERK_GORILLA
    if (CardNumberOfId(DUEL_CARD_ID(&z->card)) == CARD_BERSERK_GORILLA)
        return 0;       /* in Defense Position it would destroy itself */
#endif
    atk = AiProjectedAtk(zone);     /* with the best Equip in its hand (see AiProjectedAtk) */
    for (i = ZONE_MONSTER_0; i <= ZONE_MONSTER_4; i++) {
        struct DuelZone *h = ZoneOf(0, i);

        if (DUEL_CARD_ID(&h->card) != 0 && h->isFaceUp && GetZoneCardAtk(0, i) > best)
            best = GetZoneCardAtk(0, i);
    }
    if (best <= atk)
        return 0;
    return atk < RETREAT_KEEP_ATK || best - atk >= RETREAT_GAP;
}

/* ---- Solemn Judgment (the CPU) -----------------------------------------------------------------------------------
 * The vanilla CPU never activates Solemn Judgment: AiShouldActivateSetCard has no case for it (so summons are never
 * answered) and AiTryChainResponse never chains it. Now the CPU uses it while its LP are 4000 or less (it pays half):
 *   summons     AiShouldActivateAddedCard (the end of AiShouldActivateSetCard): the human's Normal or Special Summon,
 *               and a Flip Summon only of a flip-effect monster (HasFlipEffect); the activation window's loc0 is the
 *               human's monster
 *   Spell/Trap  AiTryChainResponse calls AiChainSolemnJudgment for the human's Magic or Trap card, after its
 *               cheaper answers (Imperial Order, Magic Jammer, Royal Decree, Seven Tools of the Bandit), also against
 *               Harpie's Feather Duster */
#define AI_SOLEMN_MAX_LP 4000

int AiChainSolemnJudgment(struct ChainEntry *entry)
{
    u32 type = CARD_STATS_TYPE(gCardStats[entry->card & CARD_ID_MASK]);

    if (entry->player != AI_HUMAN || (type != CARD_TYPE_MAGIC && type != CARD_TYPE_TRAP))
        return 0;
#ifdef HAS_CARD_BLAST_WITH_CHAIN
    if (AiChainBlastWithChain(entry))   /* its own destruction pops it */
        return 1;
#endif
#ifdef HAS_CARD_MAGIC_DRAIN
    /* Magic Drain first: it costs nothing */
    if (type == CARD_TYPE_MAGIC && AiChainSetCard(entry, CARD_MAGIC_DRAIN))
        return 1;
#endif
#ifdef HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT
    /* a burn card: Pikeru's Circle first, it costs nothing */
    if (AiChainPikerusCircle(entry))
        return 1;
#endif
#ifdef HAS_CARD_HALLOWED_LIFE_BARRIER
    /* a burn card: Hallowed Life Barrier before Solemn Judgment's half of the LP */
    if (AiChainHallowedLifeBarrier(entry))
        return 1;
#endif
#ifdef HAS_CARD_EMERGENCY_PROVISIONS
    /* a card that clears the CPU's Spells/Traps: turn them into LP first */
    if (AiChainEmergencyProvisions(entry))
        return 1;
#endif
    if (gDuelPlayers[AI_PLAYER].lifePoints > AI_SOLEMN_MAX_LP)
        return 0;
    return AiChainSetCard(entry, CARD_SOLEMN_JUDGMENT) != 0;
}

int AiShouldActivateAddedCard(struct ChainEntry *entry)
{
    if (CardNumberOfId(entry->card) == CARD_SOLEMN_JUDGMENT) {  /* see AiChainSolemnJudgment */
        int player = DUEL_LOC_PLAYER(entry->loc0), zone = DUEL_LOC_ZONE(entry->loc0);
        u16 number;

        if (player != AI_HUMAN || zone > ZONE_MONSTER_4 || gDuelPlayers[AI_PLAYER].lifePoints > AI_SOLEMN_MAX_LP)
            return 0;
        if (entry->event != RESPONSE_FLIP_SUMMONED)
            return 1;               /* a Normal or Special Summon */
        /* a Flip Summon only of a flip-effect monster */
        number = CardNumberOfId(DUEL_CARD_ID(&ZoneOf(player, zone)->card));
        return HasFlipEffect(number, 0) || HasFlipEffect(number, 1);
    }
    if (!IsAddedCardId(entry->card))
        return 0;
    switch (CardNumberOfId(entry->card)) {
#ifdef HAS_CARD_THREATENING_ROAR
    case CARD_THREATENING_ROAR:                 /* against an attack, when another of the human's could attack after */
        return entry->event == RESPONSE_ATTACK_DECLARED && gDuel.turnPlayer == AI_HUMAN
               && CountMonsters(AI_HUMAN) >= 2;
#endif
#ifdef HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT
    case CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT:    /* only chained to burn (AiChainPikerusCircle) */
        return 0;
#endif
#ifdef HAS_CARD_HALLOWED_LIFE_BARRIER
    case CARD_HALLOWED_LIFE_BARRIER: {          /* against an attack that hurts */
        int damage;

        if (entry->event != RESPONSE_ATTACK_DECLARED || gDuel.turnPlayer != AI_HUMAN)
            return 0;
        damage = AiIncomingAttackDamage();
        return damage >= HLB_AI_MIN_DAMAGE || damage >= (int)gDuelPlayers[AI_PLAYER].lifePoints;
    }
#endif
#ifdef HAS_CARD_BLAST_WITH_CHAIN
    case CARD_BLAST_WITH_CHAIN:                 /* against an attack, on a face-up monster of its own */
        return entry->event == RESPONSE_ATTACK_DECLARED && CountFaceUpMonsters(AI_PLAYER) > 0;
#endif
#ifdef HAS_CARD_TORRENTIAL_TRIBUTE
    case CARD_TORRENTIAL_TRIBUTE:               /* it destroys the CPU's monsters too */
        return CountMonsters(AI_HUMAN) > CountMonsters(AI_PLAYER);
#endif
#ifdef HAS_CARD_SCAPEGOAT
    case CARD_SCAPEGOAT:                        /* against an attack, or with nothing to defend */
        return entry->event == RESPONSE_ATTACK_DECLARED || CountMonsters(AI_PLAYER) == 0;
#endif
#ifdef HAS_CARD_OJAMA_TRIO
    case CARD_OJAMA_TRIO:                       /* the human needs three empty monster zones */
        return CountFreeMonsterZones(AI_HUMAN) > 2;
#endif
#ifdef HAS_CARD_SECRET_BARREL
    case CARD_SECRET_BARREL:
        return AiWantsSecretBarrel();
#endif
#ifdef HAS_CARD_COMPULSORY_EVACUATION_DEVICE
    case CARD_COMPULSORY_EVACUATION_DEVICE:     /* against an attack or a summon (the target step takes the strongest) */
        return CountMonsters(AI_HUMAN) > 0
               && (entry->event == RESPONSE_ATTACK_DECLARED || entry->event == RESPONSE_SUMMONED
                   || entry->event == RESPONSE_FLIP_SUMMONED || entry->event == RESPONSE_SPECIAL_SUMMONED);
#endif
#ifdef HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION
    case CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION:     /* two or more monsters back, with LP to spare */
        return CountDimensionReturns(AI_PLAYER) >= 2 && CountFreeMonsterZones(AI_PLAYER) >= 2
               && gDuelPlayers[AI_PLAYER].lifePoints > 2000;
#endif
#ifdef HAS_CARD_DIMENSION_FUSION
    case CARD_DIMENSION_FUSION: {               /* two or more monsters back, more than the human gets */
        int mine = CountDimensionReturns(AI_PLAYER), theirs = CountDimensionReturns(1 - AI_PLAYER);
        if (mine > CountFreeMonsterZones(AI_PLAYER))
            mine = CountFreeMonsterZones(AI_PLAYER);
        if (theirs > CountFreeMonsterZones(1 - AI_PLAYER))
            theirs = CountFreeMonsterZones(1 - AI_PLAYER);
        return mine >= 2 && mine > theirs && gDuelPlayers[AI_PLAYER].lifePoints > 3000;
    }
#endif
#ifdef HAS_CARD_ROYAL_OPPRESSION
    case CARD_ROYAL_OPPRESSION: {
        int summonPlayer = DUEL_LOC_PLAYER(entry->loc0);
        int summonZone = DUEL_LOC_ZONE(entry->loc0);

        if (entry->event != RESPONSE_SPECIAL_SUMMONED || summonPlayer != AI_HUMAN
            || summonZone > ZONE_MONSTER_4
            || (int)gDuelPlayers[AI_PLAYER].lifePoints <= ROYAL_OPPRESSION_COST)
            return 0;
        return 1;
    }
#endif
#ifdef HAS_CARD_SKULL_LAIR
    case CARD_SKULL_LAIR: {                     /* destroy one of the human's monsters worth 1000+ ATK */
        struct ChainEntry probe;
        int zone;

        probe.player = AI_PLAYER;
        probe.card = gCardNumberToId[CARD_SKULL_LAIR];
        zone = SkullLairBestTarget(&probe);
        return zone >= 0 && GetZoneCardAtk(1 - AI_PLAYER, zone) >= 1000;
    }
#endif
#ifdef HAS_CARD_XING_ZHEN_HU
    case CARD_XING_ZHEN_HU:                     /* only on two of the human's Set cards */
        return CountXingTargets(entry, AI_HUMAN) >= 2;
#endif
#ifdef HAS_CARD_BOOK_OF_MOON
    case CARD_BOOK_OF_MOON:                     /* against the attacker (the target step takes the strongest) */
        return entry->event == RESPONSE_ATTACK_DECLARED && CountFaceUpMonsters(AI_HUMAN) > 0;
#endif
    }
    return 1;
}

/* ---- The effect table of the added cards ------------------------------------------------------------------- */

/* Rows like gCardEffects' (struct CardEffect), any order, ending with number 0xFFFF. */
const struct CardEffect gAddedCardEffects[] = {
#ifdef HAS_CARD_BREAKER_THE_MAGICAL_WARRIOR
    {
        CARD_BREAKER_THE_MAGICAL_WARRIOR, 0,
        EffectBreakerTheMagicalWarriorResolve,
        EffectBreakerTheMagicalWarriorCheck,
        EffectBreakerTheMagicalWarriorPrepare,
        EffectBreakerTheMagicalWarriorChainA,
        EffectBreakerTheMagicalWarriorChainB,
    },
#endif
#ifdef HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING
    {
        CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING, 0,
        EffectBlackLusterSoldierResolve,
        EffectBlackLusterSoldierCheck,
        EffectBlackLusterSoldierPrepare,
        NULL,
        EffectBlackLusterSoldierChainB,
    },
#endif
#if defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2) && defined(HAS_CARD_MYSTIC_SWORDSMAN_LV4)
    {
        CARD_MYSTIC_SWORDSMAN_LV2, 0,
        EffectMysticSwordsmanLv2Resolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_MOBIUS_THE_FROST_MONARCH
    {
        CARD_MOBIUS_THE_FROST_MONARCH, 0,
        EffectMobiusResolve,
        (u16 (*)(struct ChainEntry *, u16))EffectAnySpellTrapCheck,
        NULL,
        NULL,
        EffectMobiusChainB,
    },
#endif
#ifdef HAS_CARD_THREATENING_ROAR
    {
        CARD_THREATENING_ROAR, 0,
        EffectThreateningRoarResolve,
        NULL,
        EffectThreateningRoarPrepare,
        NULL,
        NULL,
    },
#endif
#if defined(HAS_CARD_OJAMA_TRIO) && defined(HAS_CARD_OJAMA_TOKEN)
    {
        CARD_OJAMA_TRIO, 0,
        EffectOjamaTrioResolve,
        NULL,
        EffectOjamaTrioPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_THESTALOS_THE_FIRESTORM_MONARCH
    {
        CARD_THESTALOS_THE_FIRESTORM_MONARCH, 0,
        EffectThestalosResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_ZABORG_THE_THUNDER_MONARCH
    /* Zaborg the Thunder Monarch: "When this card is Tribute Summoned: Target 1 monster on the field; destroy that
     * target." The two Normal Summon paths (ExecuteSummonAction / ExecuteSummonActionAskPosition, summon_checks.c)
     * queue its RESPONSE_SUMMONED trigger in Breaker's case; it is Level 5, so that summon always has a Tribute. The
     * handlers are Man-Eater Bug's (any monster, itself too when it is the only one; mandatory, as on the card). */
    {
        CARD_ZABORG_THE_THUNDER_MONARCH, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectDestroyTargetResolve,
        (u16 (*)(struct ChainEntry *, u16))EffectTargetableMonsterCheck,
        NULL,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectTargetableMonsterChainB,
    },
#endif
#ifdef HAS_CARD_SMASHING_GROUND
    {
        CARD_SMASHING_GROUND, 0,
        EffectSmashingGroundResolve,
        NULL,
        EffectSmashingGroundPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_MAGICAL_MERCHANT
    {
        CARD_MAGICAL_MERCHANT, 0,
        EffectMagicalMerchantResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_MONSTER_REINCARNATION
    {
        CARD_MONSTER_REINCARNATION, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectMaskOfDarknessResolve,
        NULL,
        EffectMonsterReincarnationPrepare,
        EffectMonsterReincarnationChainA,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectCardListTargetChainB,
    },
#endif
#ifdef HAS_CARD_TOON_CANNON_SOLDIER
    {
        CARD_TOON_CANNON_SOLDIER, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectCatapultTurtleResolve,
        NULL,
        NULL,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectTributeTargetChainB,
    },
#endif
#ifdef HAS_CARD_DARK_MAGICIAN_OF_CHAOS
    {
        CARD_DARK_MAGICIAN_OF_CHAOS, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectMaskOfDarknessResolve,
        NULL,
        NULL,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectCardListTargetChainB,
    },
#endif
#ifdef HAS_CARD_ABYSS_SOLDIER
    {
        CARD_ABYSS_SOLDIER, 0,
        EffectAbyssSoldierResolve,
        EffectAbyssSoldierCheck,
        EffectAbyssSoldierPrepare,
        EffectAbyssSoldierChainA,
        EffectAbyssSoldierChainB,
    },
#endif
#ifdef HAS_CARD_BRAIN_CONTROL
    {
        CARD_BRAIN_CONTROL, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectChangeOfHeartResolve,
        EffectBrainControlCheck,
        EffectBrainControlPrepare,
        EffectBrainControlChainA,
        EffectBrainControlChainB,
    },
#endif
#ifdef HAS_CARD_COMPULSORY_EVACUATION_DEVICE
    /* Compulsory Evacuation Device, Normal Trap: "Target 1 monster on the field; return that target to the hand."
     * Hane-Hane's handlers (any monster, EffectReturnTargetToHandResolve: to its owner's hand, a Fusion Monster to the
     * Fusion Deck, a token destroyed). No prepare: a Set copy may answer any window in which a monster can be
     * targeted (EffectTargetableMonsterCheck). The CPU uses it against an attack or a summon (AiShouldActivateAddedCard). */
    {
        CARD_COMPULSORY_EVACUATION_DEVICE, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectReturnTargetToHandResolve,
        (u16 (*)(struct ChainEntry *, u16))EffectTargetableMonsterCheck,
        NULL,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectTargetableMonsterChainB,
    },
#endif
#ifdef HAS_CARD_XING_ZHEN_HU
    {
        CARD_XING_ZHEN_HU, 0,
        EffectXingZhenHuResolve,
        EffectXingZhenHuCheck,
        EffectXingZhenHuPrepare,
        NULL,
        EffectXingZhenHuChainB,
    },
#endif
#ifdef HAS_CARD_DARK_MIMIC_LV1
    {
        CARD_DARK_MIMIC_LV1, 0,
        EffectDarkMimicLv1Resolve,
        NULL,
        EffectDarkMimicLv1Prepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_CHAIN_DISAPPEARANCE
    {
        CARD_CHAIN_DISAPPEARANCE, 0,
        EffectChainDisappearanceResolve,
        NULL,
        EffectChainDisappearancePrepare,
        EffectChainDisappearanceChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_BOTTOMLESS_TRAP_HOLE
    {
        CARD_BOTTOMLESS_TRAP_HOLE, 0,
        EffectBottomlessTrapHoleResolve,
        NULL,
        EffectBottomlessTrapHolePrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_APPRENTICE_MAGICIAN
    /* Apprentice Magician's battle effect: a floater (see ApprenticeMagicianPlaceCounter above). */
    {
        CARD_APPRENTICE_MAGICIAN, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSpecialSummonFromDeckResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_PYRAMID_TURTLE
    /* Pyramid Turtle: "When this card is sent to the Graveyard as a result of battle, you may select 1 Zombie-Type
     * monster with a DEF of 2000 or less from your Deck and Special Summon it to the field in face up Attack or Defense
     * Position (no Tribute is required for monsters of Level 5 or more). The Deck is then shuffled."
     * A floater like Mystic Tomato: its "destroyed in battle" trigger in SendBattleDestroyedCardToGraveyard
     * (duel_field_moves.c; banished instead, it does not trigger), the floaters' resolve (asked, picked in the
     * card-list viewer, either face-up position as for Gravekeeper's Spy, shuffle) and a CollectEffectTargets case
     * (effect_target_collect.c: deck Zombies with DEF 2000 or less, any Level). */
    {
        CARD_PYRAMID_TURTLE, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSpecialSummonFromDeckResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_SACRED_CRANE
    {
        CARD_SACRED_CRANE, 0,
        EffectSacredCraneResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_MONSTER_GATE
    {
        CARD_MONSTER_GATE, 0,
        EffectMonsterGateResolve,
        NULL,
        EffectMonsterGatePrepare,
        NULL,
        EffectMonsterGateChainB,
    },
#endif
#if defined(HAS_CARD_REASONING) && defined(HAS_CARD_MONSTER_GATE)
    {
        CARD_REASONING, 0,
        EffectReasoningResolve,
        NULL,
        EffectReasoningPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION
    {
        CARD_RETURN_FROM_THE_DIFFERENT_DIMENSION, 0,
        EffectReturnFromTheDifferentDimensionResolve,
        NULL,
        EffectReturnFromTheDifferentDimensionPrepare,
        EffectReturnFromTheDifferentDimensionChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_DIMENSION_FUSION
    {
        CARD_DIMENSION_FUSION, 0,
        EffectDimensionFusionResolve,
        NULL,
        EffectDimensionFusionPrepare,
        EffectDimensionFusionChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_SECRET_BARREL
    {
        CARD_SECRET_BARREL, 0,
        EffectSecretBarrelResolve,
        NULL,
        EffectSecretBarrelPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_LEVEL_LIMIT_AREA_B
    {
        CARD_LEVEL_LIMIT_AREA_B, 0,
        EffectLevelLimitAreaBResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_FINAL_ATTACK_ORDERS
    {
        CARD_FINAL_ATTACK_ORDERS, 0,
        EffectFinalAttackOrdersResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_GRAVITY_BIND
    {
        CARD_GRAVITY_BIND, 0,
        EffectGravityBindResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_ROYAL_OPPRESSION
    {
        CARD_ROYAL_OPPRESSION, 0,
        EffectRoyalOppressionResolve,
        NULL,
        EffectRoyalOppressionPrepare,
        EffectRoyalOppressionChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_SKULL_LAIR
    {
        CARD_SKULL_LAIR, 0,
        EffectSkullLairResolve,
        (u16 (*)(struct ChainEntry *, u16))EffectSkullLairCheck,
        EffectSkullLairPrepare,
        NULL,
        EffectSkullLairChainB,
    },
#endif
#ifdef HAS_CARD_GIGANTES
    /* Gigantes: "If this card is destroyed by battle: Destroy all Spells and Traps on the field." Its trigger is
     * queued with Sangan's (SendBattleDestroyedCardToGraveyard, duel_field_moves.c); Heavy Storm's resolve destroys
     * every Spell/Trap and Field zone card of both sides. The summon is the banish-summon steps above. */
    {
        CARD_GIGANTES, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectHeavyStormResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_4_STARRED_LADYBUG_OF_DOOM
    {
        CARD_4_STARRED_LADYBUG_OF_DOOM, 0,
        EffectLadybugOfDoomResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_DES_KOALA
    {
        CARD_DES_KOALA, 0,
        EffectDesKoalaResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_ROULETTE_BARREL
    {
        CARD_ROULETTE_BARREL, 0,
        EffectRouletteBarrelResolve,
        EffectRouletteBarrelCheck,
        EffectRouletteBarrelPrepare,
        EffectRouletteBarrelChainA,
        EffectRouletteBarrelChainB,
    },
#endif
#ifdef HAS_CARD_FIBER_JAR
    {
        CARD_FIBER_JAR, 0,
        EffectFiberJarResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT
    {
        CARD_PIKERUS_CIRCLE_OF_ENCHANTMENT, 0,
        EffectPikerusCircleResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_EMERGENCY_PROVISIONS
    {
        CARD_EMERGENCY_PROVISIONS, 0,
        EffectEmergencyProvisionsResolve,
        NULL,
        EffectEmergencyProvisionsPrepare,
        EffectEmergencyProvisionsChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_HALLOWED_LIFE_BARRIER
    {
        CARD_HALLOWED_LIFE_BARRIER, 0,
        EffectHallowedLifeBarrierResolve,
        NULL,
        EffectHallowedLifeBarrierPrepare,
        EffectHallowedLifeBarrierChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_POISON_OF_THE_OLD_MAN
    {
        CARD_POISON_OF_THE_OLD_MAN, 0,
        EffectPoisonOfTheOldManResolve,
        NULL,
        NULL,
        EffectPoisonOfTheOldManChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_MYSTIK_WOK
    {
        CARD_MYSTIK_WOK, 0,
        EffectMystikWokResolve,
        NULL,
        EffectMystikWokPrepare,
        NULL,
        EffectMystikWokChainB,
    },
#endif
#ifdef HAS_CARD_ENEMY_CONTROLLER
    {
        CARD_ENEMY_CONTROLLER, 0,
        EffectEnemyControllerResolve,
        NULL,
        EffectEnemyControllerPrepare,
        EffectEnemyControllerChainA,
        EffectEnemyControllerChainB,
    },
#endif
#ifdef HAS_CARD_MIND_CONTROL
    {
        CARD_MIND_CONTROL, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectChangeOfHeartResolve,
        (u16 (*)(struct ChainEntry *, u16))EffectOpponentMonsterCheck,
        (int (*)(struct ChainEntry *, struct ChainEntry *, u16))EffectChangeOfHeartPrepare,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectTakeControlChainB,
    },
#endif
#ifdef HAS_CARD_LIGHTNING_VORTEX
    {
        CARD_LIGHTNING_VORTEX, 0,
        EffectLightningVortexResolve,
        NULL,
        EffectLightningVortexPrepare,
        EffectLightningVortexChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_DON_ZALOOG
    {
        CARD_DON_ZALOOG, 0,
        EffectDonZaloogResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_DARK_SCORPION_CLIFF_THE_TRAP_REMOVER
    {
        CARD_DARK_SCORPION_CLIFF_THE_TRAP_REMOVER, 0,
        EffectDarkScorpionCliffResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_CREATURE_SWAP
    {
        CARD_CREATURE_SWAP, 0,
        EffectCreatureSwapResolve,
        NULL,
        EffectCreatureSwapPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_PHOENIX_WING_WIND_BLAST
    {
        CARD_PHOENIX_WING_WIND_BLAST, 0,
        EffectPhoenixWingWindBlastResolve,
        EffectPhoenixWingWindBlastCheck,
        EffectPhoenixWingWindBlastPrepare,
        EffectPhoenixWingWindBlastChainA,
        EffectPhoenixWingWindBlastChainB,
    },
#endif
#ifdef HAS_CARD_MAGICAL_SCIENTIST
    {
        CARD_MAGICAL_SCIENTIST, 0,
        EffectMagicalScientistResolve,
        NULL,
        EffectMagicalScientistPrepare,
        EffectMagicalScientistChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_METAMORPHOSIS
    {
        CARD_METAMORPHOSIS, 0,
        EffectMetamorphosisResolve,
        NULL,
        EffectMetamorphosisPrepare,
        NULL,
        EffectMetamorphosisChainB,
    },
#endif
#if defined(HAS_CARD_SCAPEGOAT) && defined(HAS_CARD_SHEEP_TOKEN)     /* the tokens are the Sheep Token card */
    /* Quick-Play Spell: "Special Summon 4 'Sheep Tokens' (Beast/EARTH/Level 1/ATK 0/DEF 0) in Defense Position. You
     * cannot Summon other monsters the turn you activate this card." The game's own handlers of effect key 1245
     * (the OCG Scapegoat of the engine EDS was built on): the prepare needs 4 free monster zones and no summon this
     * turn; the resolve summons four TOKEN_KIND_2 tokens, card number 1922, which is the Sheep Token of the added
     * cards (tools/newcards.py TOKEN_NUMBERS), and locks the player's Normal and Special Summons for the turn. */
    {
        CARD_SCAPEGOAT, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSummonFourTokensResolve,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *, u16))EffectNoSummonFourFreeZonesPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_BOOK_OF_MOON
    {
        CARD_BOOK_OF_MOON, 0,
        EffectBookOfMoonResolve,
        (u16 (*)(struct ChainEntry *, u16))EffectTargetableFaceUpMonsterCheck,
        NULL,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectTargetableFaceUpMonsterChainB,
    },
#endif
#ifdef HAS_CARD_TSUKUYOMI
    {
        CARD_TSUKUYOMI, 0,
        EffectBookOfMoonResolve,
        (u16 (*)(struct ChainEntry *, u16))EffectTargetableFaceUpMonsterCheck,
        EffectTsukuyomiPrepare,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectTargetableFaceUpMonsterChainB,
    },
#endif
#ifdef HAS_CARD_REINFORCEMENT_OF_THE_ARMY
    {
        CARD_REINFORCEMENT_OF_THE_ARMY, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSanganResolve,
        NULL,
        EffectReinforcementOfTheArmyPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_TOON_TABLE_OF_CONTENTS
    {
        CARD_TOON_TABLE_OF_CONTENTS, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSanganResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_TOKEN_THANKSGIVING
    {
        CARD_TOKEN_THANKSGIVING, 0,
        EffectTokenThanksgivingResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_CARD_DESTRUCTION
    {
        CARD_CARD_DESTRUCTION, 0,
        EffectCardDestructionResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_COLD_WAVE
    {
        CARD_COLD_WAVE, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectLockMagicTrapResolve,
        NULL,
        EffectColdWavePrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_DIFFERENT_DIMENSION_CAPSULE
    {
        CARD_DIFFERENT_DIMENSION_CAPSULE, 0,
        EffectDifferentDimensionCapsuleResolve,
        NULL,
        EffectDifferentDimensionCapsulePrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_JAR_OF_GREED
    {
        CARD_JAR_OF_GREED, 0,
        EffectJarOfGreedResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_NIGHT_ASSAILANT
    {
        CARD_NIGHT_ASSAILANT, 0,
        EffectNightAssailantResolve,
        EffectNightAssailantCheck,
        NULL,
        NULL,
        EffectNightAssailantChainB,
    },
#endif
#ifdef HAS_CARD_SKILLED_DARK_MAGICIAN
    {
        CARD_SKILLED_DARK_MAGICIAN, 0,
        EffectSkilledDarkMagicianResolve,
        NULL,
        EffectSkilledDarkMagicianPrepare,
        EffectSkilledDarkMagicianChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_BOOK_OF_LIFE
    {
        CARD_BOOK_OF_LIFE, 0,
        EffectBookOfLifeResolve,
        NULL,
        EffectBookOfLifePrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_NEWDORIA
    {
        CARD_NEWDORIA, 0,
        EffectNewdoriaResolve,
        EffectNewdoriaCheck,
        NULL,
        NULL,
        EffectNewdoriaChainB,
    },
#endif
#ifdef HAS_CARD_MARAUDING_CAPTAIN
    {
        CARD_MARAUDING_CAPTAIN, 0,
        EffectMaraudingCaptainResolve,
        NULL,
        EffectMaraudingCaptainPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_MAGIC_DRAIN
    {
        CARD_MAGIC_DRAIN, 0,
        EffectMagicDrainResolve,
        NULL,
        EffectMagicDrainPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_BLAST_WITH_CHAIN
    {
        CARD_BLAST_WITH_CHAIN, 0,
        EffectBlastWithChainResolve,
        EffectBlastWithChainCheck,
        EffectBlastWithChainPrepare,
        NULL,
        EffectBlastWithChainChainB,
    },
#endif
#ifdef HAS_CARD_MIRAGE_OF_NIGHTMARE
    {
        CARD_MIRAGE_OF_NIGHTMARE, 0,
        EffectMirageOfNightmareResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_MASKED_DRAGON
    /* Masked Dragon: a floater (the battle-destroyed trigger list of SendBattleDestroyedCardToGraveyard, a
     * CollectEffectTargets case: Deck Dragons with 1500 or less ATK); the position is chosen. */
    {
        CARD_MASKED_DRAGON, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSpecialSummonFromDeckResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_SAND_MOTH
    {
        CARD_SAND_MOTH, 0,
        EffectSandMothResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_HYDROGEDDON
    /* Hydrogeddon: Special Summon a Hydrogeddon from the Deck (see HydrogeddonAfterBattle). */
    {
        CARD_HYDROGEDDON, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSpecialSummonFromDeckResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_HYSTERIC_FAIRY
    {
        CARD_HYSTERIC_FAIRY, 0,
        EffectHystericFairyResolve,
        NULL,
        EffectHystericFairyPrepare,
        NULL,
        EffectHystericFairyChainB,
    },
#endif
#ifdef HAS_CARD_POT_OF_AVARICE
    {
        CARD_POT_OF_AVARICE, 0,
        EffectPotOfAvariceResolve,
        NULL,
        EffectPotOfAvaricePrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_GUARDIAN_SPHINX
    {
        CARD_GUARDIAN_SPHINX, 0,
        EffectGolemSentryResolve,
        EffectGolemSentryCheck,
        EffectGolemSentryPrepare,
        NULL,
        EffectGolemSentryChainB,
    },
#endif
#ifdef HAS_CARD_GOLEM_SENTRY
    {
        CARD_GOLEM_SENTRY, 0,
        EffectGolemSentryResolve,
        EffectGolemSentryCheck,
        EffectGolemSentryPrepare,
        NULL,
        EffectGolemSentryChainB,
    },
#endif
#ifdef HAS_CARD_EXILED_FORCE
    {
        CARD_EXILED_FORCE, 0,
        EffectExiledForceResolve,
        EffectExiledForceCheck,
        EffectExiledForcePrepare,
        NULL,
        EffectExiledForceChainB,
    },
#endif
#ifdef HAS_CARD_KYCOO_THE_GHOST_DESTROYER
    {
        CARD_KYCOO_THE_GHOST_DESTROYER, 0,
        EffectKycooTheGhostDestroyerResolve,
        NULL,
        EffectKycooTheGhostDestroyerPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_CHAOS_SORCERER
    {
        CARD_CHAOS_SORCERER, 0,
        EffectBanishTargetNoAttackResolve,
        (u16 (*)(struct ChainEntry *, u16))EffectTargetableFaceUpMonsterCheck,
        EffectChaosSorcererPrepare,
        NULL,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectTargetableFaceUpMonsterChainB,
    },
#endif
#ifdef HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END
    {
        CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END, 0,
        EffectChaosEmperorDragonResolve,
        NULL,
        EffectChaosEmperorDragonPrepare,
        EffectChaosEmperorDragonChainA,
        NULL,
    },
#endif
#ifdef HAS_CARD_TRIBE_INFECTING_VIRUS
    {
        CARD_TRIBE_INFECTING_VIRUS, 0,
        EffectTribeInfectingVirusResolve,
        NULL,
        EffectTribeInfectingVirusPrepare,
        EffectTribeInfectingVirusChainA,
        EffectTribeInfectingVirusChainB,
    },
#endif
#ifdef HAS_CARD_TORRENTIAL_TRIBUTE
    {
        CARD_TORRENTIAL_TRIBUTE, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectDarkHoleResolve,
        NULL,
        EffectTorrentialTributePrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_SAKURETSU_ARMOR
    {
        CARD_SAKURETSU_ARMOR, 0,
        EffectSakuretsuArmorResolve,
        EffectSakuretsuArmorCheck,
        EffectSakuretsuArmorPrepare,
        NULL,
        EffectSakuretsuArmorChainB,
    },
#endif
#ifdef HAS_CARD_TRAP_DUSTSHOOT
    {
        CARD_TRAP_DUSTSHOOT, 0,
        EffectTrapDustshootResolve,
        NULL,
        EffectTrapDustshootPrepare,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_RAIGEKI_BREAK
    {
        CARD_RAIGEKI_BREAK, 0,
        EffectRaigekiBreakResolve,
        EffectRaigekiBreakCheck,
        EffectRaigekiBreakPrepare,
        EffectRaigekiBreakChainA,
        EffectRaigekiBreakChainB,
    },
#endif
#ifdef HAS_CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE
    {
        CARD_DEKOICHI_THE_BATTLECHANTED_LOCOMOTIVE, 0,
        EffectDekoichiResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
#ifdef HAS_CARD_GRAVEKEEPERS_SPY
    {
        CARD_GRAVEKEEPERS_SPY, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSpecialSummonFromDeckResolve,
        NULL,
        NULL,
        NULL,
        NULL,
    },
#endif
    { 0xFFFF },
};

/* Row `row` of the effect table: gCardEffects, then gAddedCardEffects (FindCardEffect's numbering). */
const struct CardEffect *GetCardEffectRow(int row)
{
    if (row >= CARD_EFFECT_ROWS_ORIGINAL)
        return &gAddedCardEffects[row - CARD_EFFECT_ROWS_ORIGINAL];
    return &gCardEffects[row];
}

/*
 * The monster in (player, zone) is about to be banished. First detach it from the Call of the Haunted (not
 * disabled) or Premature Burial that revived it, then destroy that card. Otherwise DestroyLinkedCards would
 * destroy the card while the monster is still linked, and its "destroy the revived monster" step would send
 * the monster (still in its zone, since the banish is only queued) to the Graveyard as well. That queues
 * Sangan's or Witch of the Black Forest's trigger for a banished monster.
 */
void DetachRevivingCards(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    int i = 0;

    while (i < z->numLinks) {
        u16 target = z->links[i];
        u8 kind = z->linkKinds[i];
        int linkPlayer = DUEL_LOC_PLAYER(target);
        int linkZone = DUEL_LOC_ZONE(target);
        struct DuelZone *linked = ZoneOf(linkPlayer, linkZone);
        u16 number = ((const u16 *)CARD_ID_TO_NUMBER_ADDR)[DUEL_CARD_ID(&linked->card) & CARD_ID_MASK];

        if ((kind == ZONE_LINK_CONTINUOUS && number == CARD_CALL_OF_THE_HAUNTED && !linked->isDisabled)
            || (kind == ZONE_LINK_EQUIP && number == CARD_PREMATURE_BURIAL)) {
            RemoveZoneLinkAt(player, zone, i);
            DestroyFieldCard(linkPlayer, linkZone, 1);
        } else {
            i++;
        }
    }
}
