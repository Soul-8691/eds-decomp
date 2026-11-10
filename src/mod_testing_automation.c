/*
 * mod_testing_automation: flags for mGBA TESTING.md scenario scripts (tools/testing_scenarios/).
 * Lua writes gModTestingFreezeOpponent each frame; duel and AI code consult it during Test Duel:
 *   bit 0      freeze the CPU (ModTestingFreezeOpponentActive)
 *   bit 1      a one-shot request to redraw the field (ModTestingTick: DrawAllAreaTiles), after a script wrote
 *              cards into the zones
 *   bits 4-7   the most Spell/Traps the CPU may Set (AiStepSetSpellTraps; 0 = no limit), so a CPU scenario's
 *              Normal Spell in its hand always finds a free Spell/Trap Zone (ModTestingCpuSetLimitReached)
 * and gModTestingOnlyCard (u16, 0x0202D800): in a CPU scenario, the card under test (a card ID; 0 = off). The CPU
 * may then activate no other card (ModTestingCpuActivationBlocked), so its videos show that card's effect only.
 * and gModTestingSelect (u16, 0x0202D802): a one-shot request to put the field cursor on a zone, so the info bar
 * shows that card (a video showing a changed ATK): bit 15 the request, bit 8 the player, bits 0-3 the zone (0-4
 * monsters, 5-9 Spell/Traps, 10 Field). Done by ModTestingTick (DuelCursor_Select, DuelScreen_DrawCursorInfo).
 */
#include "global.h"
#include "duel.h"
#include "duel_screen.h"            /* DrawAllAreaTiles */
#include "card_data.h"              /* CARD_ID_MASK, CARD_ID_TO_NUMBER_ADDR */
#include "chain.h"                  /* struct ChainEntry */
#include "mod_testing_automation.h"

u8 gModTestingFreezeOpponent;

int ModTestingFreezeOpponentActive(void)
{
#if MOD_RELEASE
    return 0;
#else
    return ModTestDuel() && (gModTestingFreezeOpponent & 1) != 0;
#endif
}

/* 1 when gModTestingOnlyCard is set (a CPU scenario of a testing build's Test Duel) and the CPU (player 1) would
 * activate a card other than it (compared by card number, so an alternate artwork counts as the card). */
int ModTestingCpuCardBlocked(int player, u16 id)
{
#if MOD_RELEASE
    return 0;
#else
    const u16 *number = (const u16 *)CARD_ID_TO_NUMBER_ADDR;
    u16 only = gModTestingOnlyCard;

    if ((player & 1) != 1 || only == 0 || !ModTestDuel())
        return 0;
    return number[id & CARD_ID_MASK] != number[only & CARD_ID_MASK];
#endif
}

/* CanActivateEffect (effect_activation.c): the same for a chain entry about to be activated. */
int ModTestingCpuActivationBlocked(struct ChainEntry *card)
{
    return ModTestingCpuCardBlocked(card->player, card->card);
}

void DuelCursor_Select(s32 player, s32 area, s32 index);     /* duel_field_view.c */
void DuelScreen_DrawCursorInfo(void);                       /* mod_info_bar.c */

/* Every duel frame (ModDuelSettingsRun): a script's redraw request (bit 1) redraws the field once; its cursor
 * request (gModTestingSelect) selects a zone. */
void ModTestingTick(void)
{
#if !MOD_RELEASE
    u16 sel;

    if (!ModTestDuel())
        return;
    if (gModTestingFreezeOpponent & 2) {
        gModTestingFreezeOpponent &= ~2;
        DrawAllAreaTiles();
    }
    sel = gModTestingSelect;
    if (sel & 0x8000) {
        int player = (sel >> 8) & 1;
        int zone = sel & 0xF;

        gModTestingSelect = 0;
        if (zone <= ZONE_MONSTER_4)
            DuelCursor_Select(player, DUEL_AREA_MONSTER, zone);
        else if (zone <= ZONE_SPELL_4)
            DuelCursor_Select(player, DUEL_AREA_SPELL_TRAP, zone - ZONE_SPELL_0);
        else
            DuelCursor_Select(player, DUEL_AREA_FIELD, 0);
        gDuelScreen.showCursor = 1;
        DuelScreen_DrawCursorInfo();
    }
#endif
}

int ModTestingCpuSetLimitReached(void)
{
#if MOD_RELEASE
    return 0;
#else
    u32 limit = gModTestingFreezeOpponent >> 4, used = 0;
    int z;

    if (limit == 0 || !ModTestDuel())
        return 0;
    for (z = ZONE_SPELL_0; z <= ZONE_SPELL_4; z++) {
        if (gDuelPlayers[1].zones[z].card.id != 0)
            used++;
    }
    return used >= limit;
#endif
}
