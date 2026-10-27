/*
 * mod_banlists: Banlists, the main menu's tenth item: the Forbidden/Limited list Deck Edit uses. Not part of the
 * original ROM.
 *   GetBanlistCopyLimit   copies of a card the saved decks may hold under the banlist in use: 0 forbidden (or not
 *                         in the banlist's card pool, card_pools/<name>.txt), 1 limited, 2 semi-limited, else
 *                         CARD_COPY_LIMIT_NONE (this build has no 3-copy limit).
 *                         src/mod_save.c asks it (IsBelowCardCopyLimit, AddCardToSaved*, TrimSavedDecksToCopyLimits);
 *                         the CPU's decks keep the game's own list (GetCardCopyLimit, RemoveOverLimitDeckCards).
 *   BanlistsMenu_BuildLabel  the "Banlists" menu label (MainMenu_Init, tiles 0x280-0x2BF; BuildMenuLabel)
 *   CB_Banlists           the scene: the banlists, A puts one in use, B goes back
 * The banlists come from banlists/<name>_ban.json (tools/banlist.py --all: build/include/banlists.h), in the order
 * of BANLISTS there; the game's original list is not one of them. The one in use is saved in gSaveData.banlist
 * (index + 1; 0, as in a new game or a save from before, is BANLIST_DEFAULT, Yugi-Kaiba). Putting another one in
 * use moves the saved decks' copies over its limits back to the trunk.
 * Like the other mod units, this one is @farcode and @rodata in units.txt and keeps its state at a fixed EWRAM
 * address (gBanlistsMenu, symbols.ld), never in .bss.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "util.h"
#include "palette.h"
#include "bg.h"
#include "sprite.h"
#include "text.h"
#include "save.h"
#include "sound.h"
#include "card_data.h"
#include "constants/cards.h"
#include "constants/sound.h"

struct FormatLimit {
    u16 number;     /* card number */
    u16 limit;      /* copies allowed: 0 forbidden, 1 limited, 2 semi-limited */
};

struct Banlist {
    const char *name;
    const char *date;
    const struct FormatLimit *limits;   /* sorted by card number */
    u16 count;
    u8 counts[3];                       /* forbidden, limited, semi-limited */
    const u16 *pool;                    /* the card pool: sorted card numbers */
    u16 poolCount;
};

#include "banlists.h"

extern const u8 gMainMenuObjPal[];
extern const u16 gMainMenuSkyImage[];

/* src/mod_recipes.c */
struct LabelChunk {
    u8 item;
    u8 x0;
    u8 x1;
};
void BuildMenuLabel(const struct LabelChunk *chunks, int count, u16 *dst);

/* ---- The banlist in use ---- */

int GetActiveBanlist(void)
{
    u16 v = gSaveData.banlist;

    return v >= 1 && v <= BANLIST_COUNT ? v - 1 : BANLIST_DEFAULT;
}

/* 1 if card number `number` (base number) is in banlist b's card pool. */
static int InBanlistPool(const struct Banlist *b, u16 number)
{
    int lo = 0, hi = b->poolCount - 1;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;

        if (b->pool[mid] == number)
            return 1;
        if (b->pool[mid] < number)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

/* A card outside the banlist's card pool counts as forbidden (0), so Deck Edit, the saved-deck functions, Deck
 * Recipes and the switch of banlists all keep it out of the decks. */
s32 GetBanlistCopyLimit(u32 id)
{
    const struct Banlist *b = &sBanlists[GetActiveBanlist()];
    u16 number = gCardIdToNumber[id & CARD_ID_MASK];
    int lo = 0, hi = b->count - 1;

#if !MOD_RELEASE
    return CARD_COPY_LIMIT_NONE;    /* make (testing): the deck builder ignores the banlist and its card pool */
#endif
    if (number >= CARD_NUMBER_ALT_ART && number != 0xFFFF)
        number -= CARD_NUMBER_ALT_ART;
    if (!InBanlistPool(b, number))
        return 0;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;

        if (b->limits[mid].number == number)
            return b->limits[mid].limit;
        if (b->limits[mid].number < number)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return CARD_COPY_LIMIT_NONE;
}

/* 1 if card id is in the card pool of the Deck Edit banlist in use. */
int IsInActiveBanlistPool(u32 id)
{
    u16 number = gCardIdToNumber[id & CARD_ID_MASK];

    if (number >= CARD_NUMBER_ALT_ART && number != 0xFFFF)
        number -= CARD_NUMBER_ALT_ART;
    return InBanlistPool(&sBanlists[GetActiveBanlist()], number);
}

/* For the format opponents' check (src/mod_banlist.c): the copies of card number `number` (base number) a deck
 * may hold in banlist n's format: 0xFF if not in its card pool, its Forbidden/Limited limit, else 3. */
int FormatCardLimit(int n, u16 number)
{
    const struct Banlist *b = &sBanlists[n];
    int lo = 0, hi = b->count - 1;

    if (!InBanlistPool(b, number))
        return 0xFF;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;

        if (b->limits[mid].number == number)
            return b->limits[mid].limit;
        if (b->limits[mid].number < number)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 3;
}

const char *BanlistName(int n)
{
    return sBanlists[n].name;
}

const char *BanlistDate(int n)
{
    return sBanlists[n].date;
}

/* Put banlist n in use and move the saved decks' copies over its limits back to the trunk; returns the cards moved. */
static int SetActiveBanlist(int n)
{
    int before = gSaveData.deckSize + gSaveData.sideDeckSize + gSaveData.fusionDeckSize;
    int i = 0;

    gSaveData.banlist = n + 1;
    TrimSavedDecksToCopyLimits();
    while (i < gSaveData.fusionDeckSize) {
        u16 id = gSaveData.fusionDeck[i];

        if ((s32)GetTrunkFusionCopies(id) > GetBanlistCopyLimit(id)) {
            RemoveCardFromSavedFusionDeck(id);
            AddCardToTrunk(id);
        } else {
            i++;
        }
    }
    SaveGame();
    return before - (gSaveData.deckSize + gSaveData.sideDeckSize + gSaveData.fusionDeckSize);
}

/* ---- The menu label ---- */

/* "Banlists": Ba of Battle, n of Campaign, l of Battle, i of Campaign, s of Password, t of Battle, s of Password. */
static const struct LabelChunk sLabelChunks[] = {
    { 1, 59, 67 },
    { 1, 69, 76 },
    { 0, 92, 98 },
    { 1, 92, 94 },
    { 0, 79, 81 },
    { 6, 50, 56 },
    { 1, 77, 83 },
    { 6, 50, 55 },
};

void BanlistsMenu_BuildLabel(u16 *dst)
{
    BuildMenuLabel(sLabelChunks, ARRAY_COUNT(sLabelChunks), dst);
}

/* ---- The scene (laid out as Deck Recipes: the menu's sky, the label as the title, a text canvas as sprites) ---- */

struct BanlistsMenu {
    u8 cursor;
    u8 scroll;
    u8 pad[2];
    char status[64];
};
extern struct BanlistsMenu gBanlistsMenu;

#define TITLE_TILE      0x000
#define CANVAS_TILE     0x040
#define CANVAS_ROWS     16
#define CANVAS_Y        22
#define ROWS_VISIBLE    8
#define ROW_HEIGHT      12
#define COUNTS_X        196     /* "10/39/13" */
#define OBJ_TILE(n)     (OBJ_VRAM0 + (n) * 0x20)

/* Colours of the main menu palette (gMainMenuObjPal), as Deck Recipes. */
#define COLOR_ROW       TEXT_SIZE_COLOR(10, 8)
#define COLOR_CURSOR    TEXT_SIZE_COLOR(10, 7)
#define COLOR_STATUS    TEXT_SIZE_COLOR(10, 3)
#define COLOR_HELP      TEXT_SIZE_COLOR(10, 10)
#define COLOR_SHADOW    TEXT_SIZE_COLOR(10, 1)

static const char sStrHelp1[] = "A: use this banlist   B: back   *: in use";
static const char sStrHelp2[] = "Forbidden / Limited / Semi-Limited cards";
static const char sStrNowUsing[] = " is now the Deck Edit banlist.";
static const char sStrAlready[] = " is already in use.";
static const char sStrMoved[] = " card(s) went back to the trunk.";

static char *AppendStr(char *dst, const char *src)
{
    while (*src)
        *dst++ = *src++;
    *dst = 0;
    return dst;
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

static void DrawText(int x, int y, u16 color, const char *text)
{
    TextDrawString(x + 1, y + 1, COLOR_SHADOW, (const u8 *)text);
    TextDrawString(x, y, color, (const u8 *)text);
}

static void Render(void)
{
    struct BanlistsMenu *s = &gBanlistsMenu;
    char line[64];
    char *p;
    int i, active = GetActiveBanlist();

    TextCanvasInitEx(32, CANVAS_ROWS, 0, 0);
    for (i = 0; i < ROWS_VISIBLE && s->scroll + i < BANLIST_COUNT; i++) {
        int n = s->scroll + i;
        const struct Banlist *b = &sBanlists[n];
        u16 color = n == s->cursor ? COLOR_CURSOR : COLOR_ROW;

        p = AppendStr(line, n == s->cursor ? "> " : "   ");
        p = AppendStr(p, n == active ? "* " : "   ");
        p = AppendStr(p, b->name);
        AppendStr(AppendStr(AppendStr(p, " ("), b->date), ")");
        DrawText(8, i * ROW_HEIGHT, color, line);
        p = AppendNum(line, b->counts[0]);
        p = AppendNum(AppendStr(p, "/"), b->counts[1]);
        AppendNum(AppendStr(p, "/"), b->counts[2]);
        DrawText(COUNTS_X, i * ROW_HEIGHT, color, line);
    }
    DrawText(8, ROWS_VISIBLE * ROW_HEIGHT, COLOR_HELP, sStrHelp1);
    DrawText(8, ROWS_VISIBLE * ROW_HEIGHT + 10, COLOR_HELP, sStrHelp2);
    DrawText(8, ROWS_VISIBLE * ROW_HEIGHT + 23, COLOR_STATUS, s->status);
    TextCanvasToTiles((u16 *)OBJ_TILE(CANVAS_TILE), 0);
}

static void DrawSprites(void)
{
    int r, c;

    for (c = 0; c < 4; c++)
        AddSprite((4 << 16) | (0x38 + c * 32), SPRITE_SHAPE_32x16, TITLE_TILE + 0x10 + c * 4);
    for (r = 0; r < CANVAS_ROWS / 2; r++) {
        for (c = 0; c < 8; c++)
            AddSprite(((CANVAS_Y + r * 16) << 16) | (c * 32), SPRITE_SHAPE_32x16, CANVAS_TILE + r * 0x40 + c * 4);
    }
}

static void MoveCursor(int n)
{
    struct BanlistsMenu *s = &gBanlistsMenu;

    s->cursor = n;
    if (s->cursor < s->scroll)
        s->scroll = s->cursor;
    if (s->cursor >= s->scroll + ROWS_VISIBLE)
        s->scroll = s->cursor - ROWS_VISIBLE + 1;
    s->status[0] = 0;
    PlaySE(SE_CURSOR);
}

/* Returns 1 when the player leaves. */
static int HandleInput(void)
{
    struct BanlistsMenu *s = &gBanlistsMenu;
    u16 keys = gMain.newKeys;
    int n = s->cursor, moved;
    char *p;

    if (keys & B_BUTTON) {
        PlaySE(SE_CANCEL);
        return 1;
    }
    if (keys & DPAD_UP) {
        MoveCursor((n + BANLIST_COUNT - 1) % BANLIST_COUNT);
    } else if (keys & DPAD_DOWN) {
        MoveCursor((n + 1) % BANLIST_COUNT);
    } else if (keys & A_BUTTON) {
        p = AppendStr(s->status, sBanlists[n].name);
        if (n == GetActiveBanlist()) {
            AppendStr(p, sStrAlready);
            PlaySE(SE_ERROR);
        } else {
            moved = SetActiveBanlist(n);
            if (moved != 0)
                AppendStr(AppendNum(AppendStr(p, ": "), moved), sStrMoved);
            else
                AppendStr(p, sStrNowUsing);
            PlaySE(SE_CONFIRM);
        }
    } else {
        return 0;
    }
    Render();
    return 0;
}

/* The scene callback, stepped as CB_DeckRecipes (gMain.seqIndex1 / seqState1): set up, fade in, input, fade out;
 * returns 1 when done (MainLoop goes back to the main menu, which saves the game). */
u16 CB_Banlists(void)
{
    struct BanlistsMenu *s = &gBanlistsMenu;

    switch (gMain.seqIndex1) {
    case 0:
        switch (gMain.seqState1++) {
        case 0:
            REG_DISPCNT = 0;
            break;
        case 1:
            SetBrightnessBlack();
            ResetVideo();
            ResetBgScroll();
            REG_BG1CNT = BGCNT_256COLOR | BGCNT_CHARBASE(1);
            gMain.vblankFlags = VBLANK_COPY_OAM | VBLANK_COPY_BG_MAPS;
            PlayBGMNoTrack(SONG_MAIN_MENU);
            break;
        default:
            CopyDoubleWords((void *)OBJ_PLTT, gMainMenuObjPal, 0x20);
            LoadBgImage(0, 0, 0x20, gMainMenuSkyImage);
            BanlistsMenu_BuildLabel((u16 *)OBJ_TILE(TITLE_TILE));
            MemClear16(s, sizeof(*s));
            s->cursor = GetActiveBanlist();
            s->scroll = s->cursor >= ROWS_VISIBLE ? s->cursor - ROWS_VISIBLE + 1 : 0;
            Render();
            gMain.seqIndex1++;
            gMain.seqState1 = 0;
            break;
        }
        return 0;
    case 1:
        REG_DISPCNT = DISPCNT_BG1_ON | DISPCNT_OBJ_ON;
        DrawSprites();
        if (FadeFromBlack(1))
            gMain.seqIndex1++;
        return 0;
    case 2:
        DrawSprites();
        if (HandleInput()) {
            FadeOutBGM();
            gMain.seqIndex1++;
        }
        return 0;
    default:
        DrawSprites();
        return FadeToBlack(2) != 0;
    }
}
