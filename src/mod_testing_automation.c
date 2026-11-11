/*
 * mod_testing_automation: flags for mGBA TESTING.md scenario scripts (tools/testing_scenarios/).
 * Lua writes gModTestingFreezeOpponent each frame; duel and AI code consult it during Test Duel:
 *   bit 0      freeze the CPU (ModTestingFreezeOpponentActive)
 *   bit 2      no chain window ("Chain: Activating / Resolving", ChainListScreen_Run closes at once): cleaner test
 *              videos (ModTestingHideChainList)
 *   bit 1      a one-shot request to redraw the field (ModTestingTick: DrawAllAreaTiles), after a script wrote
 *              cards into the zones
 *   bits 4-7   the most Spell/Traps the CPU may Set (AiStepSetSpellTraps; 0 = no limit), so a CPU scenario's
 *              Normal Spell in its hand always finds a free Spell/Trap Zone (ModTestingCpuSetLimitReached)
 * and gModTestingOnlyCard (u16, 0x0202D800): in a CPU scenario, the card under test (a card ID; 0 = off). The CPU
 * may then activate no other card (ModTestingCpuActivationBlocked), so its videos show that card's effect only.
 * and gModTestingFlags2 (u8, 0x0202D804), for cleaner test videos:
 *   bit 0      the card-showing animations (DUEL_CMD_SHOW_CARD_ASSEMBLE .. _UNROLL_SIDEWAYS, 0x71-0x76) finish at once
 *              (ModTestingSkipCardShow); the card of the last one skipped goes to gModTestingLastShown (u16,
 *              0x0202D806), where a script sees an effect "shown" (a Continuous Spell/Trap's)
 *   bit 1      the prompts (text boxes: messages, Yes/No, choices) work as usual but are not drawn
 *              (ModTestingHidePrompts): the script's key presses answer them unseen
 *   bit 2      no battle animations: the "Attack" / "Direct Attack" commands (0x33, 0x34: the cursor, sword and
 *              banner) end at once, and the full-screen battle scene is not played (ModTestingHideBattle,
 *              BattleStage_DamageCalc)
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
#include "duel_cmd.h"               /* gDuelCmd, DUEL_CMD_ID_MASK */
#include "constants/duel_cmds.h"    /* DUEL_CMD_SHOW_CARD_* */
#include "gba.h"                    /* L_BUTTON */
#include "main.h"                   /* gMain.heldKeys */
#include "duel_flow.h"              /* gDuelCtrl.isLinkDuel */

u8 gModTestingFreezeOpponent;

/*
 * 1 while the CPU must not act: no plays of its own (AiRunStep ends its Main and Battle Phase) and no responses (it
 * chains nothing, activates no Set card). Either a test script's freeze (bit 0 of gModTestingFreezeOpponent), or the
 * player holding L in a debug duel: any duel of a testing build (make, make demo), or one against Debug in make
 * release (ModDebugDuel). Never in a Link Battle. (L+R opens the in-duel settings, ModDuelSettingsHandleInput.)
 */
int ModOpponentFrozen(void)
{
    if (gDuelCtrl.isLinkDuel)
        return 0;
    if (ModTestingFreezeOpponentActive())
        return 1;
#if MOD_RELEASE
    if (!ModDebugDuel())
        return 0;
#endif
    return (gMain.heldKeys & L_BUTTON) != 0;
}

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

/* DuelCmd_Dispatch (duel_cmd_queue.c), on a command's first frame: 1 when it is a card-showing animation the player's
 * Settings hide (Hide card animations, every build), or one a test script asks to skip (gModTestingFlags2 bit 0, Test Duel of a testing build); the caller ends the command at once. The
 * card is recorded in gModTestingLastShown. */
int ModHideCardAnimations(void);    /* src/mod_settings.c: the player's setting */

int ModTestingSkipCardShow(void)
{
    u32 id = gDuelCmd.cmd & DUEL_CMD_ID_MASK;
    int cardShow = id >= DUEL_CMD_SHOW_CARD_ASSEMBLE && id <= DUEL_CMD_SHOW_CARD_UNROLL_SIDEWAYS;

    if (gDuelCmd.step != 0)
        return 0;
    if (cardShow && ModHideCardAnimations())    /* Settings: Hide card animations (make release too) */
        return 1;
#if MOD_RELEASE
    return 0;
#else
    if (!ModTestDuel())
        return 0;
    if ((id == DUEL_CMD_ATTACK || id == DUEL_CMD_DIRECT_ATTACK) && (gModTestingFlags2 & 4))
        return 1;                       /* the attack's cursor, sword and banner */
    if (!cardShow || !(gModTestingFlags2 & 1))
        return 0;
    gModTestingLastShown = gDuelCmd.arg2;
    return 1;
#endif
}

/* BattleStage_DamageCalc (battle_phase1.c): 1 when a test script asks for no battle scene (bit 2). */
int ModHideBattleScenes(void);      /* src/mod_settings.c: the player's setting */

int ModTestingHideBattle(void)
{
    if (ModHideBattleScenes())          /* Settings: Hide battle scenes (make release too) */
        return 1;
#if MOD_RELEASE
    return 0;
#else
    return (gModTestingFlags2 & 4) && ModTestDuel();
#endif
}

/* TextBoxDrawTilemap (duel_info_bar.c) and DuelMainStep (duel_main.c): 1 when a test script asks for invisible prompts
 * (bit 1): the box is not drawn, but opens, waits and takes input as usual. */
int ModTestingHidePrompts(void)
{
#if MOD_RELEASE
    return 0;
#else
    return (gModTestingFlags2 & 2) && ModTestDuel();
#endif
}

/* ChainListScreen_Run (duel_card_actions.c): 1 when a test script asks for no chain window (bit 2), in a Test Duel of a
 * testing build. */
int ModTestingHideChainList(void)
{
#if MOD_RELEASE
    return 0;
#else
    return ModTestDuel() && (gModTestingFreezeOpponent & 4) != 0;
#endif
}

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
