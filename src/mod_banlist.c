/*
 * mod_banlist: the Yugi-Kaiba format (May 2002), which Tristan's duels require. Not part of the original ROM.
 *   BanlistBlocksDuel     OpponentSelect_HandleInput (campaign_select.c) asks it when an opponent is picked: for
 *                         Tristan it checks the player's saved Deck, Side Deck and Fusion Deck, and if any card is
 *                         illegal it opens the warning (and the duel does not start)
 *   BanlistView_Run       the warning: called first by OpponentSelect_HandleInput every frame; while it is open it
 *                         draws and handles input instead of the opponent select
 * A card is illegal when it is not in the card pool (yugi-kaiba.txt) or when the copies of it (all its card IDs,
 * alternate art included) exceed its limit: the Forbidden/Limited list (yugi-kaiba_ban.json), else 3. The tables
 * come from tools/banlist.py (build/include/format_yugi_kaiba.h).
 * The warning is drawn the way the select screen draws names: a 32 x 10 tile text canvas copied into the OBJ tiles
 * of the name banners (0x2C0; 2D mapping, 32 tiles a row) and shown as 8 x 5 sprites of 32 x 16 over the darkened
 * background. Closing it reloads the page (OpponentSelect_LoadPage), which redraws the names there.
 * Tristan is shown as the "Yugi-Kaiba" opponent (his name, a La Jinn portrait, no dialogue): see the end of the unit.
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
#include "constants/cards.h"

/* The opponent select's tables (campaign_select.c). */
extern u16 gOpponentSelectDuelists[];
extern struct Mode4Bitmap gOpponentSelectPageBgs[5];

struct FormatLimit {
    u16 number;     /* card number */
    u16 limit;      /* copies allowed: 0 forbidden, 1 limited, 2 semi-limited */
};

#include "format_yugi_kaiba.h"

#define FORMAT_DEFAULT_LIMIT    3
#define NOT_IN_POOL             0xFF
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
    u8 pad;
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

static const char sStrHeader[] = "Deck not legal for Tristan (" FORMAT_NAME "):";
static const char sStrFooter[] = "UP/DOWN: scroll  A/B: back";
static const char sStrNotInPool[] = " (not in pool)";
static const char sStrForbidden[] = " (forbidden)";
static const char sStrLimit[] = " (limit ";
static const char sStrEllipsis[] = "..";

static int InPool(u16 number)
{
    int lo = 0, hi = FORMAT_POOL_COUNT - 1;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (sFormatPool[mid] == number)
            return 1;
        if (sFormatPool[mid] < number)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

static int LimitOf(u16 number)
{
    int i;

    for (i = 0; i < FORMAT_LIMIT_COUNT; i++) {
        if (sFormatLimits[i].number == number)
            return sFormatLimits[i].limit;
    }
    return FORMAT_DEFAULT_LIMIT;
}

/* Fill gBanlistView.entries with the illegal cards of the player's decks, in name order; returns their count. */
static int CollectIllegalCards(void)
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
        int limit = InPool(e.number) ? LimitOf(e.number) : NOT_IN_POOL;

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
    DrawLine(1, sStrHeader);
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
    if (duelist != DUELIST_TRISTAN)
        return 0;
    v->count = CollectIllegalCards();
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

/* ---- Tristan as the "Yugi-Kaiba" opponent ----
 *   ModOpponentSelectName       OpponentSelect_LoadPage: his name on the opponent select (and its banner) is
 *                               "Yugi-Kaiba"
 *   ModOpponentSelectPortraits  OpponentSelect_LoadPage, after the page bitmap: his portrait is La Jinn the Mystical
 *                               Genie of the Lamp's card art, scaled into the portrait's circle
 *   ModSkipsDuelDialogue        Campaign_SelectOpponent / Campaign_ShowDuelResult: no dialogue before or after his
 *                               duels
 * The portrait: the art (72 x 80, 6 bits a pixel, 9 x 10 tiles of 8 x 8, gCardArtGfx) is sampled into the circle
 * (centre slot position + (31, 33), radius 23.5: measured on page 0's bitmap; every slot has the same ring). Its
 * 64 colours map to the nearest colour the page bitmap uses; a colour with none close takes a palette entry the
 * bitmap leaves unused (page 0 leaves 27 of them besides 0). Both Mode 4 frames get the pixels (16-bit writes). */

#define YK_DUELIST          DUELIST_TRISTAN
#define PORTRAIT_DX         31
#define PORTRAIT_DY         33
#define PORTRAIT_D2         (47 * 47)       /* (2 * radius)^2, in half pixels */
#define PORTRAIT_SCALE      376             /* art pixels per half screen pixel, / 512 (72 / 49 * 256) */
#define COLOR_NEAR          27              /* squared distance in 5-bit units (about 24 in 8-bit) */

static const u8 sYugiKaibaName[] = "Yugi-Kaiba";

const u8 *ModOpponentSelectName(u16 duelist, const u8 *name)
{
    return duelist == YK_DUELIST ? sYugiKaibaName : name;
}

int ModSkipsDuelDialogue(u16 duelist)
{
    return duelist == YK_DUELIST;
}

static u32 ColorDist(u16 a, u16 b)
{
    int dr = (a & 31) - (b & 31), dg = (a >> 5 & 31) - (b >> 5 & 31), db = (a >> 10 & 31) - (b >> 10 & 31);

    return dr * dr + dg * dg + db * db;
}

/* Pixel (x, y) of card art id: a 6-bit colour index. */
static u32 ArtPixel(u16 id, int x, int y)
{
    const u8 *src = (const u8 *)(CARD_ART_GFX_ADDR + id * CARD_ART_SIZE);
    u32 i = ((y >> 3) * 9 + (x >> 3)) * 64 + (y & 7) * 8 + (x & 7);
    u32 bit = i * 6;

    return ((src[bit >> 3] | src[(bit >> 3) + 1] << 8) >> (bit & 7)) & 63;
}

static void DrawArtPortrait(int page, int slot, u16 id)
{
    const u16 *artPal = (const u16 *)(CARD_ART_PALETTES_ADDR + id * CARD_ART_PALETTE_SIZE);
    const u8 *bitmap = gOpponentSelectPageBgs[page].bitmap;
    u16 *pal = (u16 *)PLTT;
    u8 used[256];
    u8 map[64];
    int i, j, x, y, freeIndex = 1;
    int cx = gOpponentSelectSlotPos[slot].x + PORTRAIT_DX, cy = gOpponentSelectSlotPos[slot].y + PORTRAIT_DY;

    for (i = 0; i < 256; i++)
        used[i] = 0;
    for (i = 0; i < 240 * 160; i++)
        used[bitmap[i]] = 1;
    for (i = 0; i < 64; i++) {
        u32 best = 0xFFFFFFFF;

        map[i] = 0;
        for (j = 1; j < 256; j++) {
            if (used[j] && ColorDist(artPal[i], pal[j]) < best) {
                best = ColorDist(artPal[i], pal[j]);
                map[i] = j;
            }
        }
        if (best > COLOR_NEAR) {
            while (freeIndex < 256 && used[freeIndex])
                freeIndex++;
            if (freeIndex < 256) {
                pal[freeIndex] = artPal[i];
                used[freeIndex] = 1;    /* now this colour's */
                map[i] = freeIndex;
            }
        }
    }
    for (y = cy - 24; y <= cy + 24; y++) {
        for (x = cx - 24; x <= cx + 24; x++) {
            int dx = 2 * (x - cx) + 1, dy = 2 * (y - cy) + 1;
            int ax, ay;
            u32 offset, c;
            u16 *p0, *p1;

            if (dx * dx + dy * dy > PORTRAIT_D2 || x < 0 || x >= 240 || y < 0 || y >= 160)
                continue;
            ax = 36 + dx * PORTRAIT_SCALE / 512;
            ay = 40 + dy * PORTRAIT_SCALE / 512;
            ax = ax < 0 ? 0 : ax > 71 ? 71 : ax;
            ay = ay < 0 ? 0 : ay > 79 ? 79 : ay;
            c = map[ArtPixel(id, ax, ay)];
            offset = y * 240 + x;
            p0 = (u16 *)(MODE4_FRAME0 + (offset & ~1));
            p1 = (u16 *)(MODE4_FRAME1 + (offset & ~1));
            if (offset & 1) {
                *p0 = (*p0 & 0x00FF) | c << 8;
                *p1 = (*p1 & 0x00FF) | c << 8;
            } else {
                *p0 = (*p0 & 0xFF00) | c;
                *p1 = (*p1 & 0xFF00) | c;
            }
        }
    }
}

void ModOpponentSelectPortraits(int page)
{
    int slot;

    for (slot = 0; slot < 5; slot++) {
        if (gOpponentSelectDuelists[page * 5 + slot] == YK_DUELIST && IsOpponentUnlocked(YK_DUELIST))
            DrawArtPortrait(page, slot, gCardNumberToId[CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP]);
    }
}
