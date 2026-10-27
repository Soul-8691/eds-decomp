/*
 * mod_banlist: the format check of the format opponents (src/mod_opponents.c). Not part of the original ROM.
 *   BanlistBlocksDuel     OpponentSelect_HandleInput (campaign_select.c) asks it when an opponent is picked: for a
 *                         format opponent (ModOpponentFormat) it checks the player's saved Deck, Side Deck and
 *                         Fusion Deck, and if any card is illegal it opens the warning (and the duel does not start)
 *   BanlistView_Run       the warning: called first by OpponentSelect_HandleInput every frame; while it is open it
 *                         draws and handles input instead of the opponent select
 * A card is illegal when it is not in the format's card pool or when the copies of it (all its card IDs, alternate
 * art included) exceed its limit: the format's Forbidden/Limited list, else 3 (FormatCardLimit, src/mod_banlists.c:
 * banlists/<name>_ban.json and card_pools/<name>.txt).
 * The warning is drawn the way the select screen draws names: a 32 x 10 tile text canvas copied into the OBJ tiles
 * of the name banners (0x2C0; 2D mapping, 32 tiles a row) and shown as 8 x 5 sprites of 32 x 16 over the darkened
 * background. Closing it reloads the page (OpponentSelect_LoadPage), which redraws the names there.
 * Like added_effects, this unit is @farcode and @rodata at the end in units.txt.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "card_data.h"
#include "save.h"
#include "sprite.h"
#include "text.h"
#include "sound.h"
#include "constants/sound.h"
#include "constants/game.h"
#include "campaign.h"

/* src/mod_opponents.c, src/mod_banlists.c */
int ModOpponentFormat(u16 duelist);
int FormatCardLimit(int banlist, u16 number);
const char *BanlistName(int banlist);
const char *BanlistDate(int banlist);

#define NOT_IN_POOL             0xFF    /* FormatCardLimit: not in the card pool */
#define BANLIST_MAX_ENTRIES     128
#define BANLIST_VISIBLE         5       /* entry lines shown at once */

/* A card of the player's decks, its copies and the limit it breaks (NOT_IN_POOL: not in the card pool). */
struct BanlistEntry {
    u16 id;
    u16 number;
    u8 copies;
    u8 limit;
};

/* The warning's state: gBanlistView (EWRAM 0x0202E000, symbols.ld: free RAM after the Deck Edit lists). */
struct BanlistView {
    u8 active;
    u8 scroll;
    u8 count;
    u8 format;              /* the banlist of the opponent picked */
    struct BanlistEntry entries[BANLIST_MAX_ENTRIES];
};
extern struct BanlistView gBanlistView;

#define OBJ_TILE(n)     (OBJ_VRAM0 + (n) * 0x20)    /* 4bpp tile n, as in attr2 (as in campaign_select.c) */
#define CANVAS_TILE     0x2C0           /* OBJ tile of the name banners (main_menu.c OPPSEL_TILE_NAME_BANNERS) */
#define CANVAS_Y        40              /* screen y of the canvas */
#define LINE_HEIGHT     11
#define TEXT_PALETTE    3               /* OBJ palette of the select screen's text */
#define TEXT_COLOR      TEXT_SIZE_COLOR(10, 1)
#define SHADOW_COLOR    TEXT_SIZE_COLOR(10, 5)
#define LINE_CHARS      45              /* 5 px per character from x 8: what fits on the 240-px screen */

static const char sStrHeader[] = "Deck not legal for ";
static const char sStrFooter[] = "UP/DOWN: scroll  A/B: back";
static const char sStrNotInPool[] = " (not in pool)";
static const char sStrForbidden[] = " (forbidden)";
static const char sStrLimit[] = " (limit ";
static const char sStrEllipsis[] = "..";

/* Fill gBanlistView.entries with the illegal cards of the player's decks, in name order; returns their count. */
static int CollectIllegalCards(int format)
{
    struct BanlistView *v = &gBanlistView;
    int k, i, n = 0, out = 0;
    u16 id;

    for (k = 0; (id = gCardIdsByName[k]) != 0; k++) {
        u32 copies = GetTrunkDeckCopies(id) + GetTrunkSideCopies(id) + GetTrunkFusionCopies(id);
        u16 number = gCardIdToNumber[id & CARD_ID_MASK];

        if (copies == 0 || number == 0xFFFF)
            continue;
        if (number >= CARD_NUMBER_ALT_ART)
            number -= CARD_NUMBER_ALT_ART;
        for (i = 0; i < n; i++) {
            if (v->entries[i].number == number)
                break;
        }
        if (i == n) {
            if (n == BANLIST_MAX_ENTRIES)
                continue;
            v->entries[n].id = id;
            v->entries[n].number = number;
            v->entries[n].copies = 0;
            n++;
        }
        v->entries[i].copies += copies;
    }
    for (i = 0; i < n; i++) {
        struct BanlistEntry e = v->entries[i];
        int limit = FormatCardLimit(format, e.number);

        if (limit == NOT_IN_POOL || e.copies > limit) {
            e.limit = limit;
            v->entries[out++] = e;
        }
    }
    return out;
}

static char *AppendStr(char *dst, const char *src, int max)
{
    while (*src && max-- > 0)
        *dst++ = *src++;
    *dst = 0;
    return dst;
}

static int TextLen(const char *s)
{
    int n = 0;

    while (s[n])
        n++;
    return n;
}

static char *AppendNum(char *dst, u32 n)
{
    char digits[10];
    int len = 0;

    do {
        digits[len++] = '0' + n % 10;
        n /= 10;
    } while (n != 0);
    while (len > 0)
        *dst++ = digits[--len];
    *dst = 0;
    return dst;
}

static void DrawLine(int y, const char *text)
{
    TextDrawString(9, y + 1, SHADOW_COLOR, (const u8 *)text);
    TextDrawString(8, y, TEXT_COLOR, (const u8 *)text);
}

/* Draw the header, the visible entries and the footer into the canvas, then into the OBJ tiles. */
static void RenderView(void)
{
    struct BanlistView *v = &gBanlistView;
    char line[96];
    char reason[16];
    char *p;
    int i, room;

    TextCanvasInitEx(32, 10, 0, 0);
    p = AppendStr(line, sStrHeader, sizeof(sStrHeader));
    p = AppendStr(p, BanlistName(v->format), 20);
    p = AppendStr(p, " (", 2);
    p = AppendStr(p, BanlistDate(v->format), 20);
    AppendStr(p, "):", 2);
    DrawLine(1, line);
    for (i = 0; i < BANLIST_VISIBLE && v->scroll + i < v->count; i++) {
        struct BanlistEntry *e = &v->entries[v->scroll + i];
        const char *name = (const char *)(gCardNames + (e->id & CARD_ID_MASK) * CARD_NAME_SIZE);

        p = AppendStr(line, "x", 1);
        p = AppendNum(p, e->copies);
        p = AppendStr(p, " ", 1);
        if (e->limit == NOT_IN_POOL) {
            AppendStr(reason, sStrNotInPool, sizeof(sStrNotInPool));
        } else if (e->limit == 0) {
            AppendStr(reason, sStrForbidden, sizeof(sStrForbidden));
        } else {
            AppendStr(AppendNum(AppendStr(reason, sStrLimit, sizeof(sStrLimit)), e->limit), ")", 1);
        }
        room = LINE_CHARS - (p - line) - TextLen(reason);
        if (TextLen(name) > room) {
            p = AppendStr(p, name, room - (sizeof(sStrEllipsis) - 1));
            p = AppendStr(p, sStrEllipsis, sizeof(sStrEllipsis));
        } else {
            p = AppendStr(p, name, room);
        }
        AppendStr(p, reason, sizeof(reason));
        DrawLine(1 + (i + 1) * LINE_HEIGHT, line);
    }
    p = AppendStr(line, sStrFooter, sizeof(sStrFooter));
    p = AppendStr(p, "   ", 3);
    p = AppendNum(p, v->scroll + 1);
    p = AppendStr(p, "-", 1);
    p = AppendNum(p, v->scroll + i);
    p = AppendStr(p, " / ", 3);
    AppendNum(p, v->count);
    DrawLine(1 + (BANLIST_VISIBLE + 1) * LINE_HEIGHT, line);
    TextCanvasToTiles((u16 *)OBJ_TILE(CANVAS_TILE), 0);
}

int BanlistBlocksDuel(u16 duelist)
{
    struct BanlistView *v = &gBanlistView;

    v->active = 0;
#if !MOD_RELEASE
    return 0;                       /* make (testing): no format check against the opponents */
#endif
    if (ModOpponentFormat(duelist) < 0)
        return 0;
    v->format = ModOpponentFormat(duelist);
    v->count = CollectIllegalCards(v->format);
    if (v->count == 0)
        return 0;
    v->scroll = 0;
    v->active = 1;
    PlaySE(SE_ERROR);
    REG_BLDCNT = 0x00C4;    /* BG2 (the Mode-4 picture) darkened; the sprites stay bright */
    REG_BLDY = 10;
    RenderView();
    return 1;
}

int BanlistView_Run(void)
{
    struct BanlistView *v = &gBanlistView;
    int r, c;

    if (!v->active)
        return 0;
    if (gMain.newKeys & (A_BUTTON | B_BUTTON)) {
        PlaySE(SE_CONFIRM);
        v->active = 0;
        REG_BLDCNT = 0;
        REG_BLDY = 0;
        OpponentSelect_LoadPage(gOpponentSelect.page);  /* the name banners come back */
        return 1;
    }
    if ((gMain.newKeys & DPAD_UP) && v->scroll > 0) {
        v->scroll--;
        PlaySE(SE_CURSOR);
        RenderView();
    }
    if ((gMain.newKeys & DPAD_DOWN) && v->scroll + BANLIST_VISIBLE < v->count) {
        v->scroll++;
        PlaySE(SE_CURSOR);
        RenderView();
    }
    for (r = 0; r < 5; r++) {
        for (c = 0; c < 8; c++)
            AddSprite(((CANVAS_Y + r * 16) << 16) | (c * 32), SPRITE_SHAPE_32x16,
                      (CANVAS_TILE + r * 0x40 + c * 4) | OAM_ATTR2_PALETTE(TEXT_PALETTE));
    }
    return 1;
}

