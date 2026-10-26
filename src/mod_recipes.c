/*
 * mod_recipes: Deck Recipes, the main menu's eighth item. Not part of the original ROM.
 *   BuildMenuLabel          a menu label, plain and highlighted, put together from the letters of the other labels
 *                           in gMainMenuObjGfx (also for mod_packs' "Booster Packs")
 *   DeckRecipes_BuildLabel  the "Deck Recipes" label (MainMenu_Init calls it for tiles 0x200-0x23F, the rows after
 *                           the sheet; the scene uses it as its title)
 *   CB_DeckRecipes          the scene (MainMenu_Launch starts it)
 *   DeckRecipes_OnNewGame   BuildStarterDeck (title_screen.c): a New Game clears the recipes back to the presets
 * Up to RECIPE_COUNT recipes, each a name and a saved Deck, Side Deck and Fusion Deck list, live in SRAM after the
 * save image (RECIPE_SRAM; the game's save is 0x2170 bytes of the 32 KiB). The first ones start as the decks of
 * GetRecipePreset (src/mod_decks.c), named after their .ydk files; like any recipe they can be replaced or cleared.
 * A recipe changes only when the player saves to it, renames it or clears it. A on a slot opens its actions:
 *   Load       put every card of the three decks back into the trunk, then take the recipe's cards out of it; a card
 *              the trunk no longer holds (or that the copy limits refuse) is left out and counted as missing. When
 *              the current deck is not saved in a recipe, a second A is asked for.
 *   Save here  the current deck into the slot: an empty slot asks for a name (the name entry), a used one keeps its
 *              name after a second A
 *   Rename     the name entry
 *   Clear      after a second A
 * The recipe last loaded or saved is marked (header.active); it is only a mark, and it is never written back.
 * A load that leaves cards out lists them (MODE_LEFT_OUT) with the reason: the Deck Edit banlist in use (forbidden,
 * limited, not in its card pool: src/mod_banlists.c) or no copy left in the trunk.
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
/* src/mod_banlists.c: the Deck Edit banlist in use */
int GetActiveBanlist(void);
s32 GetBanlistCopyLimit(u32 id);
int IsInActiveBanlistPool(u32 id);
const char *BanlistName(int banlist);

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

enum RecipeMode {
    MODE_LIST,                      /* the list; A opens the actions */
    MODE_MENU,                      /* the actions of the cursor's slot (enum RecipeAction) */
    MODE_NAME,                      /* the name entry */
    MODE_LEFT_OUT,                  /* the cards the last load left out */
};

/* Why a load left a card out (struct LeftOut). */
enum LeftOutReason {
    LEFT_OUT_FORBIDDEN,             /* the banlist allows 0 */
    LEFT_OUT_LIMITED,               /* the banlist allows `limit` and the deck has them */
    LEFT_OUT_NOT_IN_POOL,           /* not in the banlist's card pool */
    LEFT_OUT_NO_COPY,               /* no copy left in the trunk */
    LEFT_OUT_LIST_FULL,             /* the list holds its maximum */
};

#define LEFT_OUT_MAX    64

struct LeftOut {
    u16 id;
    u8 count;
    u8 reason;                      /* enum LeftOutReason */
    u8 limit;
    u8 pad;
};

enum RecipeAction {
    ACTION_LOAD,
    ACTION_SAVE,
    ACTION_RENAME,
    ACTION_CLEAR,
    ACTION_CANCEL,
    ACTION_COUNT,
};

#define RECIPE_NAME_MAX     28      /* what fits a list row before the counts */

/* The scene's state: gDeckRecipes (EWRAM 0x0202E400, symbols.ld: free RAM after gBanlistView; ends by 0x0202EC80). */
struct DeckRecipes {
    u8 cursor;
    u8 scroll;
    u8 active;                      /* the recipe last loaded or saved, RECIPE_NONE */
    u8 mode;                        /* enum RecipeMode */
    u8 action;                      /* MODE_MENU: the action under the cursor */
    u8 confirm;                     /* MODE_MENU: 1 after the first A of an action that asks for two */
    u8 nameForSave;                 /* MODE_NAME: 1 = save the deck under the name, 0 = rename */
    u8 keyRow;                      /* MODE_NAME: the key under the cursor */
    u8 keyCol;
    u8 nameLen;
    u8 pad[2];
    char name[RECIPE_NAME_SIZE];    /* MODE_NAME: the name being entered */
    u8 sizes[RECIPE_COUNT][RECIPE_SUMMARY_SIZE];  /* deckSize, sideSize, fusionSize, used, name of every slot */
    char status[64];
    u8 leftOutCount;                /* MODE_LEFT_OUT: entries */
    u8 leftOutScroll;
    u8 pad2[2];
    struct LeftOut leftOut[LEFT_OUT_MAX];
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
 * pixel does not cover a pixel already drawn. The card icons sit 4 pixels from the outline on each side. The text
 * is centred as the sheet's labels (on x 63); at most 96 pixels wide, or the icons leave the label.
 */
void BuildMenuLabel(const struct LabelChunk *chunks, int count, u16 *dst)
{
    const struct LabelChunk *c;
    int x, y, i, j, dx, width = 0;

    for (c = chunks; c < chunks + count; c++)
        width += c->x1 - c->x0 + 1;
    dx = 63 - width / 2;
    MemClear16(dst, 64 * 32);
    for (y = 0; y < 16; y++) {
        for (i = 0; i < ICON_WIDTH; i++) {
            PutPixel(dst, dx + 128 - 16 + i, y, SheetPixel(ICON_LEFT_X + i, 3 * 16 + y));
            PutPixel(dst, dx + width - 1 + 128 + 6 + i, y, SheetPixel(ICON_RIGHT_X + i, 3 * 16 + y));
        }
    }
    for (c = chunks; c < chunks + count; c++) {
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
}

void DeckRecipes_BuildLabel(u16 *dst)
{
    BuildMenuLabel(sLabelChunks, ARRAY_COUNT(sLabelChunks), dst);
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

/* The player's current Deck, Side Deck and Fusion Deck into buf, with the given name. */
static void CurrentDeckToBuf(const char *name)
{
    struct RecipeSlot *b = &gDeckRecipes.buf;
    int i;

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

/* The name of slot n (empty for none). */
static const char *SlotName(int n)
{
    return (const char *)gDeckRecipes.sizes[n] + 4;
}

static void SetActive(int n)
{
    gDeckRecipes.active = n;
    WriteActive(n);
}

/* 1 when loading another recipe would lose the current deck: it has cards and differs from the recipe last loaded
 * or saved (there is none, or its lists differ). */
static int IsDeckUnsaved(void)
{
    struct RecipeSlot *b = &gDeckRecipes.buf;
    int i;

    if (gSaveData.deckSize + gSaveData.sideDeckSize + gSaveData.fusionDeckSize == 0)
        return 0;
    if (gDeckRecipes.active >= RECIPE_COUNT)
        return 1;
    ReadSlot(gDeckRecipes.active);
    if (!b->used || b->deckSize != gSaveData.deckSize || b->sideSize != gSaveData.sideDeckSize
        || b->fusionSize != gSaveData.fusionDeckSize)
        return 1;
    for (i = 0; i < b->deckSize; i++) {
        if (b->deck[i] != gSaveData.deck[i])
            return 1;
    }
    for (i = 0; i < b->sideSize; i++) {
        if (b->side[i] != gSaveData.sideDeck[i])
            return 1;
    }
    for (i = 0; i < b->fusionSize; i++) {
        if (b->fusion[i] != gSaveData.fusionDeck[i])
            return 1;
    }
    return 0;
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
/* Note card id as left out for `reason` (one entry per card and reason). */
static void NoteLeftOut(u16 id, int reason, int limit)
{
    struct DeckRecipes *s = &gDeckRecipes;
    int i;

    for (i = 0; i < s->leftOutCount; i++) {
        if (s->leftOut[i].id == id && s->leftOut[i].reason == reason) {
            s->leftOut[i].count++;
            return;
        }
    }
    if (s->leftOutCount < LEFT_OUT_MAX) {
        s->leftOut[i].id = id;
        s->leftOut[i].count = 1;
        s->leftOut[i].reason = reason;
        s->leftOut[i].limit = limit;
        s->leftOutCount++;
    }
}

/* Take ids[0..n) out of the trunk into a saved list (add, which grows *size; copies, its copies of a card); returns
 * the cards left out, noted with their reason: the banlist first (the list already holds what it allows), then the
 * trunk, then a full list. */
static int TakeFromTrunk(const u16 *ids, int n, void (*add)(u16), const u16 *size, u32 (*copies)(u16), int max)
{
    int i, missing = 0;

    for (i = 0; i < n; i++) {
        u16 id = ids[i];
        u16 before = *size;
        s32 limit;

        if (gSaveData.trunk[id].count != 0)
            add(id);
        if (*size != before) {
            RemoveCardFromTrunk(id);
            continue;
        }
        missing++;
        limit = GetBanlistCopyLimit(id);
        if ((s32)copies(id) >= limit) {
            if (!IsInActiveBanlistPool(id))
                NoteLeftOut(id, LEFT_OUT_NOT_IN_POOL, 0);
            else
                NoteLeftOut(id, limit == 0 ? LEFT_OUT_FORBIDDEN : LEFT_OUT_LIMITED, limit);
        } else if (gSaveData.trunk[id].count == 0) {
            NoteLeftOut(id, LEFT_OUT_NO_COPY, 0);
        } else if (*size >= max) {
            NoteLeftOut(id, LEFT_OUT_LIST_FULL, 0);
        } else {
            NoteLeftOut(id, LEFT_OUT_NO_COPY, 0);
        }
    }
    return missing;
}

/* Load recipe n as the player's deck (the recipe itself is not changed); returns the cards the trunk could not
 * give. */
static int SwitchTo(int n)
{
    struct RecipeSlot *b = &gDeckRecipes.buf;
    int missing;

    ReturnDecksToTrunk();
    ReadSlot(n);
    gDeckRecipes.leftOutCount = 0;
    gDeckRecipes.leftOutScroll = 0;
    missing = TakeFromTrunk(b->deck, b->used ? b->deckSize : 0, AddCardToSavedDeck, &gSaveData.deckSize,
                            GetTrunkDeckCopies, DECK_MAX_CARDS);
    missing += TakeFromTrunk(b->side, b->used ? b->sideSize : 0, AddCardToSavedSideDeck, &gSaveData.sideDeckSize,
                             GetTrunkSideCopies, SIDE_DECK_MAX_CARDS);
    missing += TakeFromTrunk(b->fusion, b->used ? b->fusionSize : 0, AddCardToSavedFusionDeck,
                             &gSaveData.fusionDeckSize, GetTrunkFusionCopies, FUSION_DECK_MAX_CARDS);
    SetActive(n);
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

/* Read the slot summaries. The first time (or after a New Game) set the SRAM area up: the presets, then empty
 * slots, and the current deck (unnamed) in the first empty one. */
static void LoadRecipes(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    struct RecipeHeader h;
    int n, setUp;

    ReadHeader(&h);
    setUp = h.magic != RECIPE_MAGIC;
    if (setUp) {
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
    if (setUp && gSaveData.deckSize + gSaveData.sideDeckSize + gSaveData.fusionDeckSize != 0) {
        for (n = 0; n < RECIPE_COUNT; n++) {
            if (!s->sizes[n][3]) {
                CurrentDeckToBuf("");
                WriteSlot(n);
                SetActive(n);
                break;
            }
        }
    }
}

/* A New Game clears the recipes: the header loses its magic, so the next visit sets the area up again (the presets,
 * then empty slots, the new deck taking the first empty one). */
void DeckRecipes_OnNewGame(void)
{
    struct RecipeHeader h;

    MemClear16(&h, sizeof(h));
    WriteSramEx((u8 *)&h, RECIPE_SRAM, sizeof(h));
}

/* ---- The scene ---- */

#define TITLE_TILE      0x000           /* the label (two tile rows) */
#define CANVAS_TILE     0x040           /* the text canvas, 32 x 16 tiles */
#define CANVAS_ROWS     16
#define CANVAS_Y        22
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
static const char sStrHelp1[] = "A: options   B: back   L/R: page";
static const char sStrHelp2[] = "*: the recipe last loaded or saved";
static const char sStrMenuHelp[] = "LEFT/RIGHT: choose   A: OK   B: cancel";
static const char sStrNameHelp1[] = "A: type   B: delete";
static const char sStrNameHelp2[] = "START: done   SELECT: cancel";
static const char sStrNamePrompt[] = "Name: ";
static const char sStrRecipe[] = "Recipe ";
static const char sStrLoaded[] = " is now your deck.";
static const char sStrMissing[] = " cards were left out.";
static const char sStrMissing1[] = " card was left out.";
static const char sStrUnsaved[] = "Your deck is not saved. A again: load anyway.";
static const char sStrReplace[] = "A again: replace it with your deck.";
static const char sStrClearAsk[] = "A again: clear it.";
static const char sStrSaved[] = "Your deck was saved as recipe ";
static const char sStrRenamed[] = " was renamed.";
static const char sStrCleared[] = " was cleared.";
static const char sStrIsEmpty[] = " is empty.";
static const char sStrLeftOutHead[] = " card(s) left out (banlist: ";
static const char sStrLeftOutHelp1[] = "UP/DOWN: scroll   A/B: close";
static const char sStrLeftOutHelp2[] = "Deck Edit's banlist is set in Banlists.";
static const char sStrReasonForbidden[] = "forbidden";
static const char sStrReasonLimited[] = "limited to ";
static const char sStrReasonPool[] = "not in pool";
static const char sStrReasonNoCopy[] = "none in trunk";
static const char sStrReasonFull[] = "list full";

#define LEFT_OUT_ROWS       7           /* entry rows under the heading */
#define LEFT_OUT_NAME_CHARS 26          /* "xN " + name + ".." ends by x 163, before the reason */
#define LEFT_OUT_REASON_X   170

static const char *const sActionNames[ACTION_COUNT] = {
    "Load", "Save here", "Rename", "Clear", "Cancel",
};

/* The name entry's keys: 6 rows of 13. KEY_SPACE, KEY_DELETE and KEY_DONE are drawn as words. */
#define KEY_ROWS    6
#define KEY_COLS    13
#define KEY_SPACE   1
#define KEY_DELETE  2
#define KEY_DONE    3
#define KEY_X       14
#define KEY_STEP    17
#define KEY_Y       14

static const u8 sKeys[KEY_ROWS][KEY_COLS] = {
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M' },
    { 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' },
    { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm' },
    { 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' },
    { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '/', '.' },
    { '\'', '&', '!', '?', '(', ')', ':', '+', ',', '#', KEY_SPACE, KEY_DELETE, KEY_DONE },
};

static const char sStrKeySpace[] = "Spc";
static const char sStrKeyDelete[] = "Del";
static const char sStrKeyDone[] = "End";

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

static void RenderList(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    char line[64];
    char *p;
    int i;

    for (i = 0; i < ROWS_VISIBLE; i++) {
        int n = s->scroll + i;
        u16 color = n == s->cursor ? COLOR_CURSOR : COLOR_ROW;
        const char *name = SlotName(n);

        p = AppendStr(line, n == s->cursor ? "> " : "   ");
        p = AppendNum(p, n + 1, 2);
        p = AppendStr(p, n == s->active ? " * " : "   ");
        if (!s->sizes[n][3]) {
            AppendStr(p, sStrEmpty);
            DrawLine(i * ROW_HEIGHT, color, line);
            continue;
        }
        if (!name[0]) {
            AppendStr(p, sStrCustom);
        } else {
            int k;

            /* a name longer than RECIPE_NAME_MAX (a preset's .ydk name) is cut before the counts */
            for (k = 0; name[k] && (k < RECIPE_NAME_MAX - 2 || (k < RECIPE_NAME_MAX && !name[RECIPE_NAME_MAX])); k++)
                *p++ = name[k];
            *p = 0;
            if (name[k])
                AppendStr(p, "..");
        }
        DrawLine(i * ROW_HEIGHT, color, line);
        p = AppendNum(AppendStr(line, "D"), s->sizes[n][0], 1);
        p = AppendNum(AppendStr(p, " S"), s->sizes[n][1], 1);
        AppendNum(AppendStr(p, " F"), s->sizes[n][2], 1);
        DrawText(COUNTS_X, i * ROW_HEIGHT, color, line);
    }
    if (s->mode == MODE_MENU) {
        int x = 8;

        for (i = 0; i < ACTION_COUNT; i++) {
            DrawText(x, ROWS_VISIBLE * ROW_HEIGHT, i == s->action ? COLOR_CURSOR : COLOR_ROW, sActionNames[i]);
            x += gTextCanvas.right - x + 14;
        }
        DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 10, COLOR_HELP, sStrMenuHelp);
    } else {
        DrawLine(ROWS_VISIBLE * ROW_HEIGHT, COLOR_HELP, sStrHelp1);     /* right under the list */
        DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 10, COLOR_HELP, sStrHelp2);
    }
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 23, COLOR_STATUS, s->status);
}

static void RenderNameEntry(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    char line[48];
    int r, c;

    AppendStr(AppendStr(AppendStr(line, sStrNamePrompt), s->name), s->nameLen < RECIPE_NAME_MAX ? "_" : "");
    DrawLine(0, COLOR_CURSOR, line);
    for (r = 0; r < KEY_ROWS; r++) {
        for (c = 0; c < KEY_COLS; c++) {
            u8 key = sKeys[r][c];
            u16 color = r == s->keyRow && c == s->keyCol ? COLOR_CURSOR : COLOR_ROW;

            if (key == KEY_SPACE) {
                DrawText(KEY_X + c * KEY_STEP - 2, KEY_Y + r * ROW_HEIGHT, color, sStrKeySpace);
            } else if (key == KEY_DELETE) {
                DrawText(KEY_X + c * KEY_STEP - 2, KEY_Y + r * ROW_HEIGHT, color, sStrKeyDelete);
            } else if (key == KEY_DONE) {
                DrawText(KEY_X + c * KEY_STEP - 2, KEY_Y + r * ROW_HEIGHT, color, sStrKeyDone);
            } else {
                line[0] = key;
                line[1] = 0;
                DrawText(KEY_X + c * KEY_STEP, KEY_Y + r * ROW_HEIGHT, color, line);
            }
        }
    }
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT, COLOR_HELP, sStrNameHelp1);
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 10, COLOR_HELP, sStrNameHelp2);
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 23, COLOR_STATUS, s->status);
}

/* The cards the last load left out: a heading (count, banlist), then "xN name" and the reason. */
static void RenderLeftOut(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    char line[64];
    char *p;
    int i, total = 0;

    for (i = 0; i < s->leftOutCount; i++)
        total += s->leftOut[i].count;
    p = AppendNum(line, total, 1);
    p = AppendStr(p, sStrLeftOutHead);
    AppendStr(AppendStr(p, BanlistName(GetActiveBanlist())), "):");
    DrawLine(0, COLOR_STATUS, line);
    for (i = 0; i < LEFT_OUT_ROWS && s->leftOutScroll + i < s->leftOutCount; i++) {
        struct LeftOut *e = &s->leftOut[s->leftOutScroll + i];
        const char *name = (const char *)(gCardNames + (e->id & CARD_ID_MASK) * CARD_NAME_SIZE);
        int k;

        p = AppendNum(AppendStr(line, "x"), e->count, 1);
        p = AppendStr(p, " ");
        for (k = 0; k < LEFT_OUT_NAME_CHARS && name[k]; k++)
            *p++ = name[k];
        *p = 0;
        if (name[k])
            AppendStr(p, "..");
        DrawLine((i + 1) * ROW_HEIGHT, COLOR_ROW, line);
        switch (e->reason) {
        case LEFT_OUT_FORBIDDEN:
            AppendStr(line, sStrReasonForbidden);
            break;
        case LEFT_OUT_LIMITED:
            AppendNum(AppendStr(line, sStrReasonLimited), e->limit, 1);
            break;
        case LEFT_OUT_NOT_IN_POOL:
            AppendStr(line, sStrReasonPool);
            break;
        case LEFT_OUT_LIST_FULL:
            AppendStr(line, sStrReasonFull);
            break;
        default:
            AppendStr(line, sStrReasonNoCopy);
            break;
        }
        DrawText(LEFT_OUT_REASON_X, (i + 1) * ROW_HEIGHT, COLOR_CURSOR, line);
    }
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT, COLOR_HELP, sStrLeftOutHelp1);
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 10, COLOR_HELP, sStrLeftOutHelp2);
    DrawLine(ROWS_VISIBLE * ROW_HEIGHT + 23, COLOR_STATUS, s->status);
}

static void Render(void)
{
    TextCanvasInitEx(32, CANVAS_ROWS, 0, 0);
    if (gDeckRecipes.mode == MODE_NAME)
        RenderNameEntry();
    else if (gDeckRecipes.mode == MODE_LEFT_OUT)
        RenderLeftOut();
    else
        RenderList();
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
    struct DeckRecipes *s = &gDeckRecipes;

    if (n == s->cursor)
        return;
    s->cursor = n;
    if (s->cursor < s->scroll)
        s->scroll = s->cursor;
    if (s->cursor >= s->scroll + ROWS_VISIBLE)
        s->scroll = s->cursor - ROWS_VISIBLE + 1;
    s->status[0] = 0;
    PlaySE(SE_CURSOR);
}

/* Open the name entry for the cursor's slot, starting from name. */
static void StartNameEntry(int forSave, const char *name)
{
    struct DeckRecipes *s = &gDeckRecipes;

    s->mode = MODE_NAME;
    s->nameForSave = forSave;
    s->keyRow = 0;
    s->keyCol = 0;
    MemClear16(s->name, sizeof(s->name));
    for (s->nameLen = 0; s->nameLen < RECIPE_NAME_MAX && name[s->nameLen]; s->nameLen++)
        s->name[s->nameLen] = name[s->nameLen];
    StatusRecipe(s->cursor);
}

/* START or End in the name entry: save the deck under the name, or rename the slot. */
static void FinishNameEntry(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    int n = s->cursor, i;

    if (s->nameForSave) {
        CurrentDeckToBuf(s->name);
        WriteSlot(n);
        SetActive(n);
        AppendStr(AppendNum(AppendStr(s->status, sStrSaved), n + 1, 2), ".");
    } else {
        ReadSlot(n);
        MemClear16(s->buf.name, sizeof(s->buf.name));
        for (i = 0; i < RECIPE_NAME_SIZE - 1 && s->name[i]; i++)
            s->buf.name[i] = s->name[i];
        WriteSlot(n);
        AppendStr(StatusRecipe(n), sStrRenamed);
    }
    s->mode = MODE_LIST;
    PlaySE(SE_CONFIRM);
}

static void HandleNameEntry(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    u16 keys = gMain.newKeys;

    if (keys & SELECT_BUTTON) {
        s->mode = MODE_LIST;
        s->status[0] = 0;
        PlaySE(SE_CANCEL);
    } else if (keys & START_BUTTON) {
        FinishNameEntry();
    } else if (keys & B_BUTTON) {
        if (s->nameLen > 0) {
            s->name[--s->nameLen] = 0;
            PlaySE(SE_CANCEL);
        }
    } else if (keys & DPAD_UP) {
        s->keyRow = (s->keyRow + KEY_ROWS - 1) % KEY_ROWS;
        PlaySE(SE_CURSOR);
    } else if (keys & DPAD_DOWN) {
        s->keyRow = (s->keyRow + 1) % KEY_ROWS;
        PlaySE(SE_CURSOR);
    } else if (keys & DPAD_LEFT) {
        s->keyCol = (s->keyCol + KEY_COLS - 1) % KEY_COLS;
        PlaySE(SE_CURSOR);
    } else if (keys & DPAD_RIGHT) {
        s->keyCol = (s->keyCol + 1) % KEY_COLS;
        PlaySE(SE_CURSOR);
    } else if (keys & A_BUTTON) {
        u8 key = sKeys[s->keyRow][s->keyCol];

        if (key == KEY_DONE) {
            FinishNameEntry();
        } else if (key == KEY_DELETE) {
            if (s->nameLen > 0)
                s->name[--s->nameLen] = 0;
            PlaySE(SE_CANCEL);
        } else if (s->nameLen < RECIPE_NAME_MAX) {
            s->name[s->nameLen++] = key == KEY_SPACE ? ' ' : key;
            s->name[s->nameLen] = 0;
            PlaySE(SE_CONFIRM);
        } else {
            PlaySE(SE_ERROR);
        }
    } else {
        return;
    }
    Render();
}

/* A in the actions of the cursor's slot. */
static void DoAction(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    int n = s->cursor, used = s->sizes[n][3], missing;
    char *p;

    switch (s->action) {
    case ACTION_LOAD:
        if (!used) {
            AppendStr(StatusRecipe(n), sStrIsEmpty);
            break;
        }
        if (!s->confirm && IsDeckUnsaved()) {
            AppendStr(s->status, sStrUnsaved);
            s->confirm = 1;
            PlaySE(SE_CURSOR);
            return;
        }
        missing = SwitchTo(n);
        p = AppendStr(StatusRecipe(n), sStrLoaded);
        if (missing != 0)
            AppendStr(AppendNum(AppendStr(p, " "), missing, 1), missing == 1 ? sStrMissing1 : sStrMissing);
        s->mode = s->leftOutCount != 0 ? MODE_LEFT_OUT : MODE_LIST;
        PlaySE(SE_CONFIRM);
        return;
    case ACTION_SAVE:
        if (!used) {
            StartNameEntry(1, s->active < RECIPE_COUNT ? SlotName(s->active) : "");
            PlaySE(SE_CONFIRM);
            return;
        }
        if (!s->confirm) {
            AppendStr(AppendStr(StatusRecipe(n), ": "), sStrReplace);
            s->confirm = 1;
            PlaySE(SE_CURSOR);
            return;
        }
        ReadSlot(n);
        MemCopy16(s->name, s->buf.name, sizeof(s->name));
        CurrentDeckToBuf(s->name);
        WriteSlot(n);
        SetActive(n);
        AppendStr(AppendNum(AppendStr(s->status, sStrSaved), n + 1, 2), ".");
        s->mode = MODE_LIST;
        PlaySE(SE_CONFIRM);
        return;
    case ACTION_RENAME:
        if (!used) {
            AppendStr(StatusRecipe(n), sStrIsEmpty);
            break;
        }
        StartNameEntry(0, SlotName(n));
        PlaySE(SE_CONFIRM);
        return;
    case ACTION_CLEAR:
        if (!used) {
            AppendStr(StatusRecipe(n), sStrIsEmpty);
            break;
        }
        if (!s->confirm) {
            AppendStr(AppendStr(StatusRecipe(n), ": "), sStrClearAsk);
            s->confirm = 1;
            PlaySE(SE_CURSOR);
            return;
        }
        MemClear16(&s->buf, sizeof(s->buf));
        WriteSlot(n);
        if (s->active == n)
            SetActive(RECIPE_NONE);
        AppendStr(StatusRecipe(n), sStrCleared);
        s->mode = MODE_LIST;
        PlaySE(SE_CONFIRM);
        return;
    default:
        s->mode = MODE_LIST;
        s->status[0] = 0;
        PlaySE(SE_CANCEL);
        return;
    }
    s->mode = MODE_LIST;        /* refused: the slot is empty */
    PlaySE(SE_ERROR);
}

static void HandleMenu(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    u16 keys = gMain.newKeys;

    if (keys & B_BUTTON) {
        s->mode = MODE_LIST;
        s->status[0] = 0;
        PlaySE(SE_CANCEL);
    } else if (keys & DPAD_LEFT) {
        s->action = (s->action + ACTION_COUNT - 1) % ACTION_COUNT;
        s->confirm = 0;
        s->status[0] = 0;
        PlaySE(SE_CURSOR);
    } else if (keys & DPAD_RIGHT) {
        s->action = (s->action + 1) % ACTION_COUNT;
        s->confirm = 0;
        s->status[0] = 0;
        PlaySE(SE_CURSOR);
    } else if (keys & A_BUTTON) {
        DoAction();
    } else {
        return;
    }
    Render();
}

/* Returns 1 when the player leaves. */
static int HandleInput(void)
{
    struct DeckRecipes *s = &gDeckRecipes;
    u16 keys = gMain.newKeys;
    int n = s->cursor;

    if (s->mode == MODE_NAME) {
        HandleNameEntry();
        return 0;
    }
    if (s->mode == MODE_MENU) {
        HandleMenu();
        return 0;
    }
    if (s->mode == MODE_LEFT_OUT) {
        if (keys & (A_BUTTON | B_BUTTON)) {
            s->mode = MODE_LIST;
            PlaySE(SE_CANCEL);
        } else if ((keys & DPAD_UP) && s->leftOutScroll > 0) {
            s->leftOutScroll--;
            PlaySE(SE_CURSOR);
        } else if ((keys & DPAD_DOWN) && s->leftOutScroll + LEFT_OUT_ROWS < s->leftOutCount) {
            s->leftOutScroll++;
            PlaySE(SE_CURSOR);
        } else {
            return 0;
        }
        Render();
        return 0;
    }
    if (keys & B_BUTTON) {
        PlaySE(SE_CANCEL);
        return 1;
    }
    if (keys & DPAD_UP) {
        MoveCursor((n + RECIPE_COUNT - 1) % RECIPE_COUNT);
    } else if (keys & DPAD_DOWN) {
        MoveCursor((n + 1) % RECIPE_COUNT);
    } else if (keys & L_BUTTON) {
        MoveCursor(n >= ROWS_VISIBLE ? n - ROWS_VISIBLE : 0);
    } else if (keys & R_BUTTON) {
        MoveCursor(n + ROWS_VISIBLE < RECIPE_COUNT ? n + ROWS_VISIBLE : RECIPE_COUNT - 1);
    } else if (keys & A_BUTTON) {
        s->mode = MODE_MENU;
        s->action = s->sizes[n][3] ? ACTION_LOAD : ACTION_SAVE;
        s->confirm = 0;
        s->status[0] = 0;
        PlaySE(SE_CONFIRM);
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
            FadeOutBGM();
            gMain.seqIndex1++;
        }
        return 0;
    default:
        DrawSprites();
        return FadeToBlack(2) != 0;
    }
}
