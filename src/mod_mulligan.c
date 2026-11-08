/*
 * mod_mulligan: unlimited mulligans (shuffle the deck and draw five again). In duels when the testing ROM is in use
 * or the opponent is Debug; on deck-select screens for the same. Not part of the original ROM.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "util.h"
#include "card_data.h"
#include "constants/cards.h"
#include "constants/card_stats.h"
#include "constants/sound.h"
#include "duel.h"
#include "duel_cmd.h"
#include "duel_screen.h"
#include "sound.h"
#include "save.h"
#include "booster.h"
#include "constants/game.h"
#include "mod_mulligan.h"

extern const u16 gCardNumberToId[];
extern const u8 gCardNames[];

int ModStarterDeckForMulligan(int choice, u16 *out, int max);

#define MULLIGAN_HAND       5
#define CARD_NAME_OF(id)    (((const char (*)[CARD_NAME_SIZE])gCardNames)[(id) & CARD_ID_MASK])

/* Fixed EWRAM (symbols.ld): never use .bss here — the linker places .bss at gTextCanvas (0x02000000). */
struct ModMulliganWork {
    u16 previewDeck[DECK_MAX_CARDS];
    u16 previewHand[MULLIGAN_HAND];
    int previewDeckCount;
    u8 previewActive;
    u8 pad[2];
    u8 text[512];
};
extern struct ModMulliganWork gModMulligan;
STATIC_ASSERT(sizeof(struct ModMulliganWork) <= 0x0202E000 - 0x0202D800, ModMulliganFits);

static char *MulliganAppend(char *dst, const char *src)
{
    while (*src)
        *dst++ = *src++;
    *dst = 0;
    return dst;
}

void ModMulliganWorkClear(void)
{
    MemClear16(&gModMulligan, sizeof(gModMulligan));
}

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

static void MulliganSimDraw(void)
{
    struct ModMulliganWork *m = &gModMulligan;
    u16 pile[DECK_MAX_CARDS];
    int n = m->previewDeckCount;
    int i, j, passes;

    if (n <= 0)
        return;
    for (i = 0; i < n; i++)
        pile[i] = m->previewDeck[i];
    passes = n * 8;
    for (i = 0; i < passes; i++) {
        int a = Random() % n;
        int b = Random() % n;

        if (a != b) {
            u16 t = pile[a];
            pile[a] = pile[b];
            pile[b] = t;
        }
    }
    for (i = 0; i < MULLIGAN_HAND; i++)
        m->previewHand[i] = 0;
    for (i = 0; i < MULLIGAN_HAND && n > 0; i++) {
        m->previewHand[i] = pile[0];
        n--;
        for (j = 0; j < n; j++)
            pile[j] = pile[j + 1];
    }
}

static void MulliganFormatHandStatus(void)
{
    struct ModMulliganWork *m = &gModMulligan;
    char *p = (char *)m->text;
    int i;
    int first = 1;

    p = MulliganAppend(p, "Sample hand: ");
    for (i = 0; i < MULLIGAN_HAND; i++) {
        if (m->previewHand[i] == 0)
            continue;
        if (!first)
            p = MulliganAppend(p, ", ");
        first = 0;
        p = MulliganAppend(p, CARD_NAME_OF(m->previewHand[i]));
    }
    if (first)
        MulliganAppend(p, "(empty)");
}

int ModMulliganAllowedInDuel(void)
{
#if MOD_RELEASE
    return ModDebugDuel();
#else
    return ModDebugDuel() || 1;
#endif
}

int ModMulliganAllowedOnDeckSelect(void)
{
#if MOD_RELEASE
    return gMain.opponent == DUELIST_PEGASUS;
#else
    return 1;
#endif
}

int ModMulliganAvailable(void)
{
    return ModMulliganAllowedInDuel() && gDuel.turnCount <= 1 && gDuel.turnPlayer == 0;
}

void ModMulliganDeckSelectRoll(const u16 *deckIds, int deckCount)
{
    struct ModMulliganWork *m = &gModMulligan;
    int i;

    if (!deckCount || deckCount > DECK_MAX_CARDS)
        return;
    for (i = 0; i < deckCount; i++)
        m->previewDeck[i] = deckIds[i];
    m->previewDeckCount = deckCount;
    MulliganSimDraw();
    m->previewActive = 1;
    MulliganFormatHandStatus();
}

void ModMulliganDeckSelectRollStarter(int starterChoice)
{
    u16 deck[DECK_MAX_CARDS];
    int n = ModStarterDeckForMulligan(starterChoice, deck, DECK_MAX_CARDS);

    ModMulliganDeckSelectRoll(deck, n);
}

const u8 *ModMulliganDeckSelectStatus(void)
{
    if (!gModMulligan.previewActive)
        return NULL;
    return gModMulligan.text;
}

/* The card command menu's Mulligan icon (OBJ tile 0x224 + 4 * 13) and label (0x264 + 8 * 13), over the blank
 * slot 13 of the sheets LoadDuelUiGfx has just loaded (tools/mulligan_menu_gfx.py). */
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

