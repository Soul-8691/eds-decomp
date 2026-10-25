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
#include "ai.h"
#include "battle.h"
#include "main.h"                   /* gMain.newKeys */
#include "duel_screen.h"            /* DuelCursor_PickTarget, gDuelScreen */

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

/* The attribute the summon steps are banishing now: LIGHT first (summonSeq 0), then DARK. */
int LightDarkBanishAttribute(void)
{
    return gDuel.cardMenu.summonSeq == 0 ? ATTRIBUTE_LIGHT : ATTRIBUTE_DARK;
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
void OnMonsterTurnedFaceDown(int player, int zone)
{
    RemoveZoneLink(DUEL_LOC(player, zone), 0, ZONE_LINK_ATK_300_PER_VALUE);
}

#if defined(HAS_CARD_BOOK_OF_MOON) || defined(HAS_CARD_TSUKUYOMI)
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
#endif /* HAS_CARD_BOOK_OF_MOON || HAS_CARD_TSUKUYOMI */

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

static u8 sTsukuyomiEndReturnMask[2];

#define TSUKUYOMI_MONSTER_TRIGGER(event) (((event) << 25) | (CHAIN_KIND_MONSTER << 21))

static inline u16 TsukuyomiPlayerCmd(int player, u16 cmd)
{
    return player ? cmd | DUEL_CMD_PLAYER : cmd;
}

int IsCannotSpecialSummon(u16 cardId)
{
    if (CARD_NUMBER(cardId) == CARD_TSUKUYOMI)
        return 1;
    return 0;
}

void TsukuyomiMarkForEndReturn(int player, int zone)
{
    if (zone >= ZONE_MONSTER_0 && zone <= ZONE_MONSTER_4)
        sTsukuyomiEndReturnMask[player & 1] |= (u8)(1 << zone);
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
    u8 mask = sTsukuyomiEndReturnMask[player & 1];

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
    sTsukuyomiEndReturnMask[player & 1] = 0;
}
#endif /* HAS_CARD_TSUKUYOMI */

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

/* The packed chain entry of a trigger (struct ChainEntry, chain.h), as in duel_field_moves.c. */
#define TRIGGER_ZONE(zone) ((zone) << 16)
#define TRIGGER_KIND(kind) ((kind) << 21)
#define TRIGGER_EVENT(event) ((event) << 25)
#define TRIGGER_PLAYER(player) ((player) << 31)

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

/* ---- Dark Ruler Ha Des ---------------------------------------------------------------------------------------
 * "When this card is Tribute Summoned: Flip Effects cannot be activated for the rest of this turn." */

static u8 sFlipEffectsLockedForTurn;

int FlipEffectsLockedForTurn(void)
{
    return sFlipEffectsLockedForTurn != 0;
}

void LockFlipEffectsForTurn(void)
{
    sFlipEffectsLockedForTurn = 1;
}

void ClearFlipEffectsLockForTurn(void)
{
    sFlipEffectsLockedForTurn = 0;
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
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectBanishOpponentGraveMonstersResolve,
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
