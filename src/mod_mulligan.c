/*
 * mod_mulligan: unlimited mulligans (shuffle the deck and draw five again) in duels when the testing ROM is in use
 * or the opponent is Debug. Not part of the original ROM.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "util.h"
#include "constants/duel.h"
#include "constants/sound.h"
#include "duel.h"
#include "duel_screen.h"
#include "sound.h"
#include "constants/game.h"
#include "mod_mulligan.h"

#define MULLIGAN_HAND       5

static void HandRemoveAt(int player, int idx)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    p->handCount--;
    for (i = idx; i < p->handCount; i++)
        CopyDuelCard(&p->hand[i], &p->hand[i + 1]);
}

static void MulliganRedrawDuelHand(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    while (p->handCount > 0) {
        AddCardToDeckBottom(player, &p->hand[0]);
        HandRemoveAt(player, 0);
    }
    ShuffleDeck(player, 8);
    for (i = 0; i < MULLIGAN_HAND && p->deckCount > 0; i++)
        DuelDrawCard(player);
    DrawAllAreaTiles();
    DuelScreen_DrawCursorInfo();
}

int ModMulliganAllowedInDuel(void)
{
#if MOD_RELEASE
    return ModDebugDuel();
#else
    return ModDebugDuel() || 1;
#endif
}

int ModMulliganAvailable(void)
{
    return ModMulliganAllowedInDuel() && gDuel.turnCount <= 1 && gDuel.turnPlayer == 0;
}

extern const u8 gModMulliganMenuGfx[];

void ModLoadMulliganMenuGfx(void)
{
    MemCopy16((void *)(OBJ_VRAM0 + (0x224 + 4 * CARDMENU_CMD_MULLIGAN) * 32), gModMulliganMenuGfx, 0x80);
    MemCopy16((void *)(OBJ_VRAM0 + (0x264 + 8 * CARDMENU_CMD_MULLIGAN) * 32), gModMulliganMenuGfx + 0x80, 0x100);
}

void ModMulliganFromDeckMenu(void)
{
    if (!ModMulliganAvailable())
        return;
    MulliganRedrawDuelHand(0);
    gDuel.cardMenu.open = 0;
    gDuel.cardMenu.state = 0;
    gDuel.cardMenu.confirmed = 0;
    gDuelScreen.showCursor = 1;
    PlaySE(SE_CONFIRM);
}
