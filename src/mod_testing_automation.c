/*
 * mod_testing_automation: flags for mGBA TESTING.md scenario scripts (tools/testing_scenarios/).
 * Lua writes gModTestingFreezeOpponent each frame; duel and AI code consult it during Test Duel.
 */
#include "global.h"
#include "mod_testing_automation.h"

u8 gModTestingFreezeOpponent;

int ModTestingFreezeOpponentActive(void)
{
#if MOD_RELEASE
    return 0;
#else
    return ModTestDuel() && gModTestingFreezeOpponent != 0;
#endif
}
