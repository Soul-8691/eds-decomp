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
#include "ai.h"
#include "battle.h"
#include "main.h"                   /* gMain.newKeys */
#include "duel_screen.h"            /* DuelCursor_PickTarget, gDuelScreen */

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
};
extern struct ModDuelState gModDuel;

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

#if defined(HAS_CARD_NINJA_GRANDMASTER_SASUKE) || defined(HAS_CARD_MYSTIC_SWORDSMAN_LV2)
/* ---- Attackers that destroy the target before the Damage Step ---------------------------------------------------
 * Ninja Grandmaster Sasuke: "If this card attacks a Defense Position monster, destroy that monster immediately with
 * this card's effect without applying damage calculation."
 * Mystic Swordsman LV2: "If this card attacks a face-down Defense Position monster: Destroy the monster immediately
 * with this card's effect without flipping it face-up or applying damage calculation."
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
#endif /* HAS_CARD_NINJA_GRANDMASTER_SASUKE || HAS_CARD_MYSTIC_SWORDSMAN_LV2 */

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
#ifdef HAS_CARD_INJECTION_FAIRY_LILY
    InjectionFairyLilyResetBattle();
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
    case CARD_THE_LITTLE_SWORDSMAN_OF_AILE:
#ifdef HAS_CARD_METAMORPHOSIS
    case CARD_METAMORPHOSIS:
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
    }
    return 1;
}

/* Should the CPU play its added Magic card `number` in its Main Phase now? (AiTryPlaySpellTrap checks that it may.) */
static int AiWantsSpell(u16 number)
{
    switch (number) {
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
 * monsters of its Graveyard (the human's summon steps are a card-list UI with no CPU branch). 1 if it did. */
static int AiLightDarkBanishSummon(void)
{
    struct DuelPlayer *p = &gDuelPlayers[AI_PLAYER];
    int i, light, dark;
    u16 cards[2][2];

    if (!CanPayLightDarkBanishCost(AI_PLAYER))
        return 0;
    for (i = 0; i < p->handCount; i++) {
        u16 number = CardNumberOfId(DUEL_CARD_ID(&p->hand[i]));
        if (
#ifdef HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING
            number != CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING &&
#endif
#ifdef HAS_CARD_CHAOS_SORCERER
            number != CARD_CHAOS_SORCERER &&
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

int AiShouldActivateAddedCard(struct ChainEntry *entry)
{
    if (!IsAddedCardId(entry->card))
        return 0;
    switch (CardNumberOfId(entry->card)) {
#ifdef HAS_CARD_TORRENTIAL_TRIBUTE
    case CARD_TORRENTIAL_TRIBUTE:               /* it destroys the CPU's monsters too */
        return CountMonsters(AI_HUMAN) > CountMonsters(AI_PLAYER);
#endif
#ifdef HAS_CARD_SCAPEGOAT
    case CARD_SCAPEGOAT:                        /* against an attack, or with nothing to defend */
        return entry->event == RESPONSE_ATTACK_DECLARED || CountMonsters(AI_PLAYER) == 0;
#endif
#ifdef HAS_CARD_COMPULSORY_EVACUATION_DEVICE
    case CARD_COMPULSORY_EVACUATION_DEVICE:     /* against an attack or a summon (the target step takes the strongest) */
        return CountMonsters(AI_HUMAN) > 0
               && (entry->event == RESPONSE_ATTACK_DECLARED || entry->event == RESPONSE_SUMMONED
                   || entry->event == RESPONSE_FLIP_SUMMONED || entry->event == RESPONSE_SPECIAL_SUMMONED);
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
#ifdef HAS_CARD_PYRAMID_TURTLE
    /* Pyramid Turtle: "When this card is destroyed by battle and sent to the GY: You can Special Summon 1 Zombie monster
     * with 2000 or less DEF from your Deck." A floater like Mystic Tomato: its "destroyed in battle" trigger in
     * SendBattleDestroyedCardToGraveyard (duel_field_moves.c; banished instead, it does not trigger), the floaters'
     * resolve (asked, picked in the card-list viewer, Attack Position, shuffle) and a CollectEffectTargets case
     * (effect_target_collect.c: deck Zombies with DEF 2000 or less). */
    {
        CARD_PYRAMID_TURTLE, 0,
        (int (*)(struct ChainEntry *, struct ChainEntry *))EffectSpecialSummonFromDeckResolve,
        NULL,
        NULL,
        NULL,
        NULL,
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
