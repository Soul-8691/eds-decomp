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
    { 0xFFFF },
};

/* Row `row` of the effect table: gCardEffects, then gAddedCardEffects (FindCardEffect's numbering). */
const struct CardEffect *GetCardEffectRow(int row)
{
    if (row >= CARD_EFFECT_ROWS_ORIGINAL)
        return &gAddedCardEffects[row - CARD_EFFECT_ROWS_ORIGINAL];
    return &gCardEffects[row];
}
