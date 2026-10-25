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

/* ---- Spell Counters ----------------------------------------------------------------------------------------
 * A Spell Counter on a monster is a ZONE_LINK_ATK_300_PER_VALUE link from the monster's own card ID: the link's
 * count (high byte) is the number of counters, and the stat code (card_stats.c) already gives the monster 300
 * ATK per unit. The link goes with the card when it leaves the field. */

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
        if ((u8)z->linkKinds[i] == ZONE_LINK_ATK_300_PER_VALUE && (z->links[i] & CARD_ID_MASK) == id)
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

#ifdef HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING
/* ---- Black Luster Soldier - Envoy of the Beginning ----------------------------------------------------------
 * "Cannot be Normal Summoned/Set. Must first be Special Summoned (from your hand) by banishing 1 LIGHT and 1 DARK
 * monster from your GY. Once per turn, you can activate 1 of these effects.
 *  - Target 1 monster on the field; banish it. This card cannot attack the turn this effect is activated.
 *  - During the Damage Step, if this attacking card destroys an opponent's monster by battle: it can make a
 *    second attack in a row."
 *
 * Summon: IsSpecialSummonOnly (card_detail.c) makes the hand menu offer only the Special Summon commands;
 * CanSummonFromHand (summon_checks.c) asks CanPayBlackLusterSoldierCost; the summon steps (effect_hooks.c,
 * SUMMON_STEP_BLS_*) banish one LIGHT, then one DARK monster from the Graveyard through the card-list viewer, whose
 * list CollectEffectTargets (effect_target_collect.c) builds with BlackLusterSoldierBanishAttribute.
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
int CanPayBlackLusterSoldierCost(int player)
{
    return CountFreeMonsterZones(player) > 0 && HasGraveyardMonsterOfAttribute(player, ATTRIBUTE_LIGHT)
           && HasGraveyardMonsterOfAttribute(player, ATTRIBUTE_DARK);
}

/* The attribute the summon steps are banishing now: LIGHT first (summonSeq 0), then DARK. */
int BlackLusterSoldierBanishAttribute(void)
{
    return gDuel.cardMenu.summonSeq == 0 ? ATTRIBUTE_LIGHT : ATTRIBUTE_DARK;
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

    if (link->event != RESPONSE_BATTLE_DESTROYED) {
        /* Banish the target; this card cannot attack this turn. The effect is used either way. */
        if (!link->negated && link->numTargets == 1) {
            int targetPlayer = (u8)link->targets[0];
            int targetZone = link->targets[0] >> 8;
            if (DUEL_CARD_ID(&ZoneOf(targetPlayer, targetZone)->card) != 0)
                BanishFieldCard(targetPlayer, targetZone, 0);
        }
        if (here) {
            cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_CANNOT_ATTACK : DUEL_CMD_SET_CANNOT_ATTACK;
            DuelCmd_Push(cmd, link->zone, 1, 0);
            cmd = link->player ? DUEL_CMD_PLAYER | DUEL_CMD_SET_EFFECT_UNUSED : DUEL_CMD_SET_EFFECT_UNUSED;
            DuelCmd_Push(cmd, link->zone, 0, 0);
        }
        return EFFECT_STEP_DONE;
    }
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
#ifdef HAS_CARD_TRIBE_INFECTING_VIRUS
    {
        CARD_TRIBE_INFECTING_VIRUS, 0,
        EffectTribeInfectingVirusResolve,
        NULL,
        EffectTribeInfectingVirusPrepare,
        EffectTribeInfectingVirusChainA,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectDeclareTypeChainB,
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
