/*
 * mod_testing_automation: flags for mGBA TESTING.md scenario scripts (tools/testing_scenarios/).
 * Lua writes gModTestingFreezeOpponent each frame; duel and AI code consult it during Test Duel:
 *   bit 0      freeze the CPU (ModTestingFreezeOpponentActive)
 *   bit 1      a one-shot request to redraw the field (ModTestingTick: DrawAllAreaTiles), after a script wrote
 *              cards into the zones
 *   bits 4-7   the most Spell/Traps the CPU may Set (AiStepSetSpellTraps; 0 = no limit), so a CPU scenario's
 *              Normal Spell in its hand always finds a free Spell/Trap Zone (ModTestingCpuSetLimitReached)
 */
#include "global.h"
#include "duel.h"
#include "duel_screen.h"            /* DrawAllAreaTiles */
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

/* Every duel frame (ModDuelSettingsRun): a script's redraw request (bit 1) redraws the field once. */
void ModTestingTick(void)
{
#if !MOD_RELEASE
    if ((gModTestingFreezeOpponent & 2) && ModTestDuel()) {
        gModTestingFreezeOpponent &= ~2;
        DrawAllAreaTiles();
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
