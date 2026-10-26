/*
 * mod_recipes: Deck Recipes, the main menu's eighth item. Not part of the original ROM.
 *   DeckRecipes_BuildLabel  the "Deck Recipes" menu label, plain and highlighted, put together from the letters of
 *                           the other labels in gMainMenuObjGfx (MainMenu_Init calls it for tiles 0x200-0x23F,
 *                           the rows after the sheet; the scene uses it as its title)
 *   CB_DeckRecipes          the scene (MainMenu_Launch starts it)
 *   DeckRecipes_OnNewGame   BuildStarterDeck (title_screen.c): the new game's deck belongs to no recipe yet
 * Up to RECIPE_COUNT recipes, each a name and a saved Deck, Side Deck and Fusion Deck list, live in SRAM after the
 * save image (RECIPE_SRAM; the game's save is 0x2170 bytes of the 32 KiB). The first ones start as the decks of
 * GetRecipePreset (src/mod_decks.c), named after their .ydk files; like any recipe they can be replaced or cleared.
 * A recipe saved with SELECT takes the name of the recipe in use (none: "Custom deck"). One recipe is in use: it is the player's current
 * deck, stored back into its slot whenever the scene opens or switches. Switching to another recipe puts every card
 * of the three decks back into the trunk, then takes the recipe's cards out of the trunk; a card the trunk no longer
 * holds (or that the copy limits refuse) is left out and counted as missing.
 * The scene looks like the main menu: its sky, palette and the label as the title; the list is drawn as in
 * mod_banlist (a 32-tile text canvas copied into the OBJ tiles, 2D mapping, and shown as 32 x 16 sprites).
 * Like the other mod units, this one is @farcode and @rodata in units.txt and keeps its state at a fixed EWRAM
 * address (gDeckRecipes, symbols.ld), never in .bss.
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
#include "constants/sound.h"

/* src/mod_decks.c */
int GetRecipePreset(int n, const char **name, const u16 **cards, u32 *count);

extern const u8 gMainMenuObjPal[], gMainMenuObjGfx[];
extern const u16 gMainMenuSkyImage[];

#define RECIPE_COUNT        50
#define RECIPE_NONE         0xFF
#define RECIPE_SRAM         ((u8 *)SRAM + 0x4000)
#define RECIPE_MAGIC        0x32504352      /* "RCP2" */
#define RECIPE_NAME_SIZE    32
#define RECIPE_SUMMARY_SIZE (4 + RECIPE_NAME_SIZE)  /* the sizes and the name, read for the list */

/* One recipe in SRAM. */
struct RecipeSlot {
    u8 deckSize;
    u8 sideSize;
    u8 fusionSize;
    u8 used;
    char name[RECIPE_NAME_SIZE];    /* empty: "Custom deck" */
    u16 deck[DECK_MAX_CARDS];
    u16 side[SIDE_DECK_MAX_CARDS];
    u16 fusion[FUSION_DECK_MAX_CARDS];
};

struct RecipeHeader {
    u32 magic;
    u8 active;
    u8 pad[11];
};

#define RECIPE_SLOT_SRAM(n) (RECIPE_SRAM + sizeof(struct RecipeHeader) + (n) * sizeof(struct RecipeSlot))
typedef char mod_recipes_check_sram[sizeof(struct RecipeHeader) + RECIPE_COUNT * sizeof(struct RecipeSlot)
                                    <= 0x4000 ? 1 : -1];

enum RecipeConfirm {
    CONFIRM_NONE,
    CONFIRM_SAVE,
    CONFIRM_CLEAR,
};

/* The scene's state: gDeckRecipes (EWRAM 0x0202E400, symbols.ld: free RAM after gBanlistView). */
struct DeckRecipes {
    u8 cursor;
    u8 scroll;
    u8 active;
    u8 confirm;                     /* enum RecipeConfirm: the first press of SELECT/START on confirmSlot */
    u8 confirmSlot;
    u8 pad[3];
    u8 sizes[RECIPE_COUNT][RECIPE_SUMMARY_SIZE];  /* deckSize, sideSize, fusionSize, used, name of every slot */
    char status[64];
    struct RecipeSlot buf;
};
extern struct DeckRecipes gDeckRecipes;

/* ---- The menu label ---- */

/* A run of columns of a label in gMainMenuObjGfx (2D, 32 tiles a row: item i is the 16-pixel row i + 1, the plain
 * label at x 0-127 and the highlighted one 128 pixels to the right). */
struct LabelChunk {
    u8 item;
    u8 x0;
    u8 x1;
};

static const struct LabelChunk sLabelChunks[] = {
    { 2, 31, 67 },      /* "Deck " of Deck Edit */
    { 3, 39, 65 },      /* "Rec" of Record */
    { 0, 79, 82 },      /* "i" of Campaign */
    { 0, 61, 69 },      /* "p" of Campaign */
    { 2, 40, 47 },      /* "e" of Deck */
    { 6, 50, 55 },      /* "s" of Password */
};

#define LABEL_WIDTH     91                      /* the chunks' widths */
#define LABEL_X         (63 - LABEL_WIDTH / 2)  /* centred as the other labels */
#define ICON_WIDTH      11                      /* the card icons of the highlighted labels (Deck Edit's row) */
#define ICON_LEFT_X     143
#define ICON_RIGHT_X    229

static u32 SheetPixel(int x, int y)
{
    u8 b = gMainMenuObjGfx[((y >> 3) * 32 + (x >> 3)) * 32 + (y & 7) * 4 + ((x & 7) >> 1)];

    return (x & 1) ? b >> 4 : b & 15;
}

static u32 GetPixel(u16 *dst, int x, int y)
{
    u32 i = ((y >> 3) * 32 + (x >> 3)) * 16 + (y & 7) * 2 + ((x & 7) >> 2);

    return (dst[i] >> ((x & 3) * 4)) & 15;
}

static void PutPixel(u16 *dst, int x, int y, u32 color)
{
    u32 i = ((y >> 3) * 32 + (x >> 3)) * 16 + (y & 7) * 2 + ((x & 7) >> 2);
    u32 shift = (x & 3) * 4;

    dst[i] = (dst[i] & ~(15 << shift)) | (color << shift);
}

/* A plain pixel of the chunk's item at (x, y), x within the chunk. */
static int ChunkPixel(const struct LabelChunk *c, int x, int y)
{
    if (x < c->x0 || x > c->x1 || y < 0 || y > 15)
        return 0;
    return SheetPixel(x, (c->item + 1) * 16 + y) != 0;
}

/*
 * Writes the label into the two tile rows at dst (2D mapping): plain at x 0-127, highlighted at x 128-255, as the
 * sheet's own labels. The plain letters get Deck Edit's colours (9 above the x-height, 8 below). The highlighted
 * letters keep the sheet's pixels that touch the chunk's plain ones (the fill and its 1-pixel outline); an outline
 * pixel does not cover a pixel already drawn. The card icons sit 4 pixels from the outline on each side.
 */
void DeckRecipes_BuildLabel(u16 *dst)
{
    const struct LabelChunk *c;
    int x, y, i, j, dx = LABEL_X;

    MemClear16(dst, 64 * 32);
    for (c = sLabelChunks; c < sLabelChunks + ARRAY_COUNT(sLabelChunks); c++) {
        int sy = (c->item + 1) * 16;

        for (y = 0; y < 16; y++) {
            for (x = c->x0; x <= c->x1; x++) {
                if (SheetPixel(x, sy + y))
                    PutPixel(dst, dx + x - c->x0, y, y <= 4 ? 9 : 8);
            }
            for (x = c->x0 - 1; x <= c->x1 + 1; x++) {
                u32 v = SheetPixel(x + 128, sy + y);
                int near = 0;

                if (v == 0)
                    continue;
                for (i = -1; i <= 1; i++) {
                    for (j = -1; j <= 1; j++)
                        near |= ChunkPixel(c, x + i, y + j);
                }
                if (!near || (v == 1 && GetPixel(dst, dx + x - c->x0 + 128, y)))
                    continue;
                PutPixel(dst, dx + x - c->x0 + 128, y, v);
            }
        }
        dx += c->x1 - c->x0 + 1;
    }
    for (y = 0; y < 16; y++) {
        for (i = 0; i < ICON_WIDTH; i++) {
            PutPixel(dst, LABEL_X + 128 - 16 + i, y, SheetPixel(ICON_LEFT_X + i, 3 * 16 + y));
            PutPixel(dst, LABEL_X + LABEL_WIDTH - 1 + 128 + 6 + i, y, SheetPixel(ICON_RIGHT_X + i, 3 * 16 + y));
        }
    }
}

/* ---- The recipes in SRAM ---- */

static void ReadHeader(struct RecipeHeader *h)
{
    ReadSram(RECIPE_SRAM, (u8 *)h, sizeof(*h));
}

static void WriteActive(u8 active)
{
    struct RecipeHeader h;

    MemClear16(&h, sizeof(h));
    h.magic = RECIPE_MAGIC;
    h.active = active;
    WriteSramEx((u8 *)&h, RECIPE_SRAM, sizeof(h));
}

static void ReadSlot(int n)
{
    ReadSram(RECIPE_SLOT_SRAM(n), (u8 *)&gDeckRecipes.buf, sizeof(struct RecipeSlot));
}

static void WriteSlot(int n)
{
    struct DeckRecipes *s = &gDeckRecipes;

    WriteSramEx((u8 *)&s->buf, RECIPE_SLOT_SRAM(n), sizeof(struct RecipeSlot));
    MemCopy16(s->sizes[n], &s->buf, RECIPE_SUMMARY_SIZE);
}

/* The player's current Deck, Side Deck and Fusion Deck into buf, named as the recipe in use. */
static void CurrentDeckToBuf(void)
{
    struct RecipeSlot *b = &gDeckRecipes.buf;
    char name[RECIPE_NAME_SIZE];
    int i;

    if (gDeckRecipes.active < RECIPE_COUNT)
        MemCopy16(name, gDeckRecipes.sizes[gDeckRecipes.active] + 4, RECIPE_NAME_SIZE);
    else
        name[0] = 0;
    MemClear16(b, sizeof(*b));
    for (i = 0; i < RECIPE_NAME_SIZE - 1 && name[i]; i++)
        b->name[i] = name[i];
    b->used = 1;
    b->deckSize = gSaveData.deckSize;
    b->sideSize = gSaveData.sideDeckSize;
    b->fusionSize = gSaveData.fusionDeckSize;
    MemCopy16(b->deck, gSaveData.deck, sizeof(b->deck));
    MemCopy16(b->side, gSaveData.sideDeck, sizeof(b->side));
    MemCopy16(b->fusion, gSaveData.fusionDeck, sizeof(b->fusion));
}

/* The recipe in use takes the player's current deck (the Deck Edit changes since the last time). */
static void StoreActive(void)
{
    if (gDeckRecipes.active < RECIPE_COUNT) {
        CurrentDeckToBuf();
        WriteSlot(gDeckRecipes.active);
    }
}

/* Every card of the three decks back to the trunk. */
static void ReturnDecksToTrunk(void)
{
    int i;

    for (i = 0; i < gSaveData.deckSize; i++) {
        AddCardToTrunk(gSaveData.deck[i]);
        SetTrunkDeckCopies(gSaveData.deck[i], 0);
    }
    for (i = 0; i < gSaveData.sideDeckSize; i++) {
        AddCardToTrunk(gSaveData.sideDeck[i]);
        SetTrunkSideCopies(gSaveData.sideDeck[i], 0);
    }
    for (i = 0; i < gSaveData.fusionDeckSize; i++) {
        AddCardToTrunk(gSaveData.fusionDeck[i]);
        SetTrunkFusionCopies(gSaveData.fusionDeck[i], 0);
    }
    gSaveData.deckSize = 0;
    gSaveData.sideDeckSize = 0;
    gSaveData.fusionDeckSize = 0;
}

/* Take ids[0..n) out of the trunk into a saved list (add, which grows *size); returns the cards left out. */
static int TakeFromTrunk(const u16 *ids, int n, void (*add)(u16), const u16 *size)
{
    int i, missing = 0;

    for (i = 0; i < n; i++) {
        u16 id = ids[i];
        u16 before = *size;

        if (gSaveData.trunk[id].count != 0)
            add(id);
        if (*size != before)
            RemoveCardFromTrunk(id);
        else
            missing++;
    }
    return missing;
}

/* Recipe n becomes the deck in use; returns the cards the trunk could not give. */
static int SwitchTo(int n)
{
    struct RecipeSlot *b = &gDeckRecipes.buf;
    int missing;

    StoreActive();
    ReturnDecksToTrunk();
    ReadSlot(n);
    missing = TakeFromTrunk(b->deck, b->used ? b->deckSize : 0, AddCardToSavedDeck, &gSaveData.deckSize);
    missing += TakeFromTrunk(b->side, b->used ? b->sideSize : 0, AddCardToSavedSideDeck, &gSaveData.sideDeckSize);
    missing += TakeFromTrunk(b->fusion, b->used ? b->fusionSize : 0, AddCardToSavedFusionDeck,
                             &gSaveData.fusionDeckSize);
    gDeckRecipes.active = n;
    WriteActive(n);
    StoreActive();
    SaveGame();
    return missing;
}

/* Preset n into buf (0 past the last one): Fusion Monsters to the Fusion Deck, the rest to the Deck. */
static int PresetToBuf(int n)
{
    struct RecipeSlot *b = &gDeckRecipes.buf;
    const char *name;
    const u16 *cards;
    u32 count, i;

    MemClear16(b, sizeof(*b));
    if (!GetRecipePreset(n, &name, &cards, &count))
        return 0;
    b->used = 1;
    for (i = 0; i < RECIPE_NAME_SIZE - 1 && name[i]; i++)
        b->name[i] = name[i];
    for (i = 0; i < count; i++) {
        u16 id = gCardNumberToId[cards[i] & CARD_ID_MASK];
        u32 stats = gCardStats[id & CARD_ID_MASK];

        if (CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE && CARD_STATS_KIND(stats) == CARD_KIND_FUSION) {
            if (b->fusionSize < FUSION_DECK_MAX_CARDS)
                b->fusion[b->fusionSize++] = id;
        } else if (b->deckSize < DECK_MAX_CARDS) {
            b->deck[b->deckSize++] = id;
        }
    }
    return 1;
}

/* Read the slot summaries; set up the SRAM area the first time (the presets, then empty slots); give the current
 * deck a slot when it has none. */
static void LoadRecipes(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    struct RecipeHeader h;
    int n;

    ReadHeader(&h);
    if (h.magic != RECIPE_MAGIC) {
        for (n = 0; n < RECIPE_COUNT; n++) {
            PresetToBuf(n);
            WriteSlot(n);
        }
        h.active = RECIPE_NONE;
        WriteActive(RECIPE_NONE);
    }
    for (n = 0; n < RECIPE_COUNT; n++) {
        ReadSram(RECIPE_SLOT_SRAM(n), s->sizes[n], RECIPE_SUMMARY_SIZE);
        s->sizes[n][RECIPE_SUMMARY_SIZE - 1] = 0;
    }
    s->active = h.active < RECIPE_COUNT ? h.active : RECIPE_NONE;
    if (s->active == RECIPE_NONE) {
        for (n = 0; n < RECIPE_COUNT; n++) {
            if (!s->sizes[n][3]) {
                s->active = n;
                WriteActive(n);
                break;
            }
        }
    }
    StoreActive();
}

void DeckRecipes_OnNewGame(void)
{
    struct RecipeHeader h;

    ReadHeader(&h);
    if (h.magic == RECIPE_MAGIC)
        WriteActive(RECIPE_NONE);
}

/* ---- The scene ---- */

#define TITLE_TILE      0x000           /* the label (two tile rows) */
#define CANVAS_TILE     0x040           /* the text canvas, 32 x 16 tiles */
#define CANVAS_ROWS     16
#define CANVAS_Y        26
#define ROWS_VISIBLE    8
#define ROW_HEIGHT      12
#define OBJ_TILE(n)     (OBJ_VRAM0 + (n) * 0x20)

/* Colours of the main menu palette (gMainMenuObjPal). */
#define COLOR_ROW       TEXT_SIZE_COLOR(10, 8)      /* the plain labels' blue */
#define COLOR_CURSOR    TEXT_SIZE_COLOR(10, 7)      /* the highlighted labels' yellow */
#define COLOR_STATUS    TEXT_SIZE_COLOR(10, 3)
#define COLOR_HELP      TEXT_SIZE_COLOR(10, 10)
#define COLOR_SHADOW    TEXT_SIZE_COLOR(10, 1)

#define COUNTS_X        186     /* the "D40 S0 F3" column */

static const char sStrEmpty[] = "(empty)";
static const char sStrCustom[] = "Custom deck";
static const char sStrHelp1[] = "A: use   B: back   L/R: page   *: in use";
static const char sStrHelp2[] = "SELECT: save deck here   START: clear";
static const char sStrRecipe[] = "Recipe ";
static const char sStrNowUsing[] = " is now your deck.";
static const char sStrMissing[] = " cards were not in the trunk.";
static const char sStrMissing1[] = " card was not in the trunk.";
static const char sStrAlready[] = " is already your deck.";
static const char sStrIsCurrent[] = " is your current deck.";
static const char sStrPressSelect[] = "Press SELECT again to save your deck as ";
static const char sStrPressStart[] = "Press START again to clear recipe ";
static const char sStrSaved[] = "Your deck was saved as recipe ";
static const char sStrCleared[] = " was cleared.";
static const char sStrIsEmpty[] = " is empty.";
static const char sStrInUseClear[] = " is in use; it cannot be cleared.";

static char *AppendStr(char *dst, const char *src)
{
    while (*src)
        *dst++ = *src++;
    *dst = 0;
    return dst;
}

static char *AppendNum(char *dst, u32 n, int width)
{
    char digits[10];
    int len = 0;

    do {
        digits[len++] = '0' + n % 10;
        n /= 10;
    } while (n != 0);
    while (len < width)
        digits[len++] = '0';
    while (len > 0)
        *dst++ = digits[--len];
    *dst = 0;
    return dst;
}

/* "Recipe NN" at the start of the status line; returns its end. */
static char *StatusRecipe(int n)
{
    return AppendNum(AppendStr(gDeckRecipes.status, sStrRecipe), n + 1, 2);
}

static void DrawText(int x, int y, u16 color, const char *text)
{
    TextDrawString(x + 1, y + 1, COLOR_SHADOW, (const u8 *)text);
    TextDrawString(x, y, color, (const u8 *)text);
}

static void DrawLine(int y, u16 color, const char *text)
{
    DrawText(8, y, color, text);
}

static void Render(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    char line[64];
    char *p;
    int i;

    TextCanvasInitEx(32, CANVAS_ROWS, 0, 0);
    for (i = 0; i < ROWS_VISIBLE; i++) {
        int n = s->scroll + i;
        u16 color = n == s->cursor ? COLOR_CURSOR : COLOR_ROW;
        const char *name = (const char *)s->sizes[n] + 4;
        int deck = s->sizes[n][0], side = s->sizes[n][1], fusion = s->sizes[n][2];

        if (n == s->active) {
            deck = gSaveData.deckSize;
            side = gSaveData.sideDeckSize;
            fusion = gSaveData.fusionDeckSize;
        }
        p = AppendStr(line, n == s->cursor ? "> " : "   ");
        p = AppendNum(p, n + 1, 2);
        p = AppendStr(p, n == s->active ? " * " : "   ");
        if (n != s->active && !s->sizes[n][3]) {
            AppendStr(p, sStrEmpty);
            DrawLine(i * ROW_HEIGHT, color, line);
            continue;
        }
        AppendStr(p, name[0] ? name : sStrCustom);
        DrawLine(i * ROW_HEIGHT, color, line);
        p = AppendNum(AppendStr(line, "D"), deck, 1);
        p = AppendNum(AppendStr(p, " S"), side, 1);
        AppendNum(AppendStr(p, " F"), fusion, 1);
        DrawText(COUNTS_X, i * ROW_HEIGHT, color, line);
    }
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 3, COLOR_STATUS, s->status);
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 15, COLOR_HELP, sStrHelp1);
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 25, COLOR_HELP, sStrHelp2);
    TextCanvasToTiles((u16 *)OBJ_TILE(CANVAS_TILE), 0);
}

static void DrawSprites(void)
{
    int r, c;

    for (c = 0; c < 4; c++)
        AddSprite((6 << 16) | (0x38 + c * 32), SPRITE_SHAPE_32x16, TITLE_TILE + 0x10 + c * 4);
    for (r = 0; r < CANVAS_ROWS / 2; r++) {
        for (c = 0; c < 8; c++)
            AddSprite(((CANVAS_Y + r * 16) << 16) | (c * 32), SPRITE_SHAPE_32x16, CANVAS_TILE + r * 0x40 + c * 4);
    }
}

static void MoveCursor(int n)
{
    struct DeckRecipes *s = &gDeckRecipes;

    if (n == s->cursor)
        return;
    s->cursor = n;
    if (s->cursor < s->scroll)
        s->scroll = s->cursor;
    if (s->cursor >= s->scroll + ROWS_VISIBLE)
        s->scroll = s->cursor - ROWS_VISIBLE + 1;
    s->confirm = CONFIRM_NONE;
    s->status[0] = 0;
    PlaySE(SE_CURSOR);
}

/* Returns 1 when the player leaves. */
static int HandleInput(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    u16 keys = gMain.newKeys;
    int n = s->cursor, missing;
    char *p;

    if (keys & B_BUTTON) {
        PlaySE(SE_CANCEL);
        return 1;
    }
    if (keys & DPAD_UP)
        MoveCursor((n + RECIPE_COUNT - 1) % RECIPE_COUNT);
    else if (keys & DPAD_DOWN)
        MoveCursor((n + 1) % RECIPE_COUNT);
    else if (keys & L_BUTTON)
        MoveCursor(n >= ROWS_VISIBLE ? n - ROWS_VISIBLE : 0);
    else if (keys & R_BUTTON)
        MoveCursor(n + ROWS_VISIBLE < RECIPE_COUNT ? n + ROWS_VISIBLE : RECIPE_COUNT - 1);
    else if (keys & A_BUTTON) {
        s->confirm = CONFIRM_NONE;
        if (n == s->active) {
            AppendStr(StatusRecipe(n), sStrAlready);
            PlaySE(SE_ERROR);
        } else {
            missing = SwitchTo(n);
            p = AppendStr(StatusRecipe(n), sStrNowUsing);
            if (missing != 0)
                AppendStr(AppendNum(AppendStr(p, " "), missing, 1), missing == 1 ? sStrMissing1 : sStrMissing);
            PlaySE(SE_CONFIRM);
        }
    } else if (keys & SELECT_BUTTON) {
        if (n == s->active) {
            AppendStr(StatusRecipe(n), sStrIsCurrent);
            s->confirm = CONFIRM_NONE;
            PlaySE(SE_ERROR);
        } else if (s->confirm != CONFIRM_SAVE || s->confirmSlot != n) {
            AppendStr(AppendNum(AppendStr(s->status, sStrPressSelect), n + 1, 2), ".");
            s->confirm = CONFIRM_SAVE;
            s->confirmSlot = n;
            PlaySE(SE_CURSOR);
        } else {
            CurrentDeckToBuf();
            WriteSlot(n);
            AppendStr(AppendNum(AppendStr(s->status, sStrSaved), n + 1, 2), ".");
            s->confirm = CONFIRM_NONE;
            PlaySE(SE_CONFIRM);
        }
    } else if (keys & START_BUTTON) {
        if (n == s->active) {
            AppendStr(StatusRecipe(n), sStrInUseClear);
            s->confirm = CONFIRM_NONE;
            PlaySE(SE_ERROR);
        } else if (!s->sizes[n][3]) {
            AppendStr(StatusRecipe(n), sStrIsEmpty);
            s->confirm = CONFIRM_NONE;
            PlaySE(SE_ERROR);
        } else if (s->confirm != CONFIRM_CLEAR || s->confirmSlot != n) {
            AppendStr(AppendNum(AppendStr(s->status, sStrPressStart), n + 1, 2), ".");
            s->confirm = CONFIRM_CLEAR;
            s->confirmSlot = n;
            PlaySE(SE_CURSOR);
        } else {
            MemClear16(&s->buf, sizeof(s->buf));
            WriteSlot(n);
            AppendStr(StatusRecipe(n), sStrCleared);
            s->confirm = CONFIRM_NONE;
            PlaySE(SE_CONFIRM);
        }
    } else {
        return 0;
    }
    Render();
    return 0;
}

/*
 * The scene callback. gMain.seqIndex1 is the step (cleared by SetMainCallback) and gMain.seqState1 the sub-state:
 *   0  set up the screen as MainMenu_Init does, the title and the recipes
 *   1  fade in
 *   2  input until B
 *   3  fade out; returns 1 when done (MainLoop goes back to the main menu, which saves the game)
 */
u16 CB_DeckRecipes(void)
{
    struct DeckRecipes *s = &gDeckRecipes;

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
            DeckRecipes_BuildLabel((u16 *)OBJ_TILE(TITLE_TILE));
            MemClear16(s, sizeof(*s) - sizeof(s->buf));
            LoadRecipes();
            s->cursor = s->active < RECIPE_COUNT ? s->active : 0;
            s->scroll = s->cursor >= ROWS_VISIBLE ? s->cursor - ROWS_VISIBLE / 2 : 0;
            if (s->scroll > RECIPE_COUNT - ROWS_VISIBLE)
                s->scroll = RECIPE_COUNT - ROWS_VISIBLE;
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
            StoreActive();
            FadeOutBGM();
            gMain.seqIndex1++;
        }
        return 0;
    default:
        DrawSprites();
        return FadeToBlack(2) != 0;
    }
}
