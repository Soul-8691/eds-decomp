/*
 * mod_list_filter: the List Filter screen's third page. Not part of the original ROM.
 *   ListRefine_Reset        ListFilter_Reset: every field back to "no limit"
 *   ListRefine_Open         the sort page's A (Magic and Trap: the filter page's A) opens the page
 *   ListRefine_Update       the page's input, from ListFilter_Update (LIST_FILTER_PHASE_REFINE)
 *   ListRefine_ApplyToList  DeckEdit_FilterAndSortList: drops the cards of its result that fail the page's fields
 *   ModCompareCardsByType, ModCompareCardsByAttribute  the sort page's Type and Attribute: alphabetical type or
 *                           attribute group, then by card name (case-insensitive)
 * Fields: a name's first letter, one Type, one Attribute, and from/to ranges of ATK, DEF and Level. Magic and Trap
 * lists only have the name's letter. A limit other than the name's keeps monsters only: the stats word of a Trap
 * carries attribute 1 (Light), so non-monsters must not reach those tests. The Gods count as the sort comparators
 * count them (ATK and DEF 4000, Level 10) and as their own Divine-Beast Type and Divine Attribute.
 * The page is text on BG1, palette 15 (free in gListFilterBgPal). Its 450 tiles do not fit in the screen's character
 * base after its 0x300 tiles (a map entry's tile number is 10 bits), so BG1 uses character base 2 (0x06008000, free
 * and cleared by ListFilter_Init) while the page shows. BG0 shows only the list's icon. Both maps go back on leaving.
 * The state is in gDeckEdit.unusedLists, so like the filter and sort cursors it starts clean with each visit.
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "bg.h"                     /* CopyMapRect, CropMapBlock */
#include "card_data.h"              /* gCardStats, gCardNames, CARD_ID_MASK, CARD_NAME_SIZE, CARD_STATS_* */
#include "constants/card_stats.h"   /* enum CardType, enum CardAttribute */
#include "constants/sound.h"        /* SE_* */
#include "deck_edit.h"              /* gDeckEdit, enum ListFilter, enum ListFilterPhase, gListSortOptionAnims */
#include "gba.h"                    /* keys, REG_DISPCNT, BG_PLTT, BG_VRAM, CpuSet */
#include "main.h"                   /* gMain */
#include "sound.h"                  /* PlaySE */
#include "sprite.h"                 /* ANIM_FINISHED */
#include "text.h"                   /* the text canvas */

enum RefineRow {
    ROW_NAME,
    ROW_TYPE,
    ROW_ATTRIBUTE,
    ROW_ATK_FROM,
    ROW_ATK_TO,
    ROW_DEF_FROM,
    ROW_DEF_TO,
    ROW_LEVEL_FROM,
    ROW_LEVEL_TO,
    ROW_COUNT,
};

#define NAME_OTHER      27      /* ROW_NAME: 0 any, 1..26 A..Z, this one anything else */
#define POINTS_STEP     100     /* one step of the ATK and DEF rows */
#define POINTS_TO_MAX   50      /* ROW_ATK_TO, ROW_DEF_TO: no upper limit */
#define LEVEL_MIN       1
#define LEVEL_MAX       12
#define DIVINE_POINTS   4000
#define DIVINE_LEVEL    10
#define BIG_STEP        5       /* L and R */

/* gDeckEdit.unusedLists: free since the card lists moved out (gDeckEditCardLists). */
struct ListRefine {
    u8 row;                     /* cursor, enum RefineRow */
    u8 value[ROW_COUNT];
};
#define REFINE ((struct ListRefine *)gDeckEdit.unusedLists)

/* The text area: BG1 rows 5-19. The rows above keep the panel's title on BG2 and the list's icon on BG0. Tile 0 of the
 * character base stays empty for the map's other cells. */
#define SORT_MAP        ((u16 *)0x0600C000)
#define TEXT_MAP        ((u16 *)0x0600D000)
#define BG1CNT_SCREEN   0x1A01      /* as ListFilter_Init sets it */
#define BG1CNT_TEXT     (BG1CNT_SCREEN | (2 << 2))
#define TEXT_TILE_FIRST 1
#define TEXT_TILES      ((u16 *)(0x06008000 + TEXT_TILE_FIRST * 0x20))
#define TEXT_PAL        15
#define TEXT_FIRST_ROW  5
#define TEXT_WIDTH      30
#define TEXT_HEIGHT     15
#define ROW_HEIGHT      10
#define ROWS_Y          2
#define HELP_Y          96
#define LABEL_X         14
#define VALUE_X         110

#define COLOR_BOX       1
#define COLOR_SHADOW    TEXT_SIZE_COLOR(10, 2)
#define COLOR_ROW       TEXT_SIZE_COLOR(10, 3)
#define COLOR_CURSOR    TEXT_SIZE_COLOR(10, 4)
#define COLOR_HELP      TEXT_SIZE_COLOR(10, 5)

#define RGB5(r, g, b)   ((r) | ((g) << 5) | ((b) << 10))

static const u16 sTextPal[16] = {
    0,
    RGB5(2, 3, 9),      /* the box */
    RGB5(0, 0, 2),      /* shadow */
    RGB5(31, 31, 31),   /* a row */
    RGB5(31, 27, 6),    /* the cursor's row */
    RGB5(21, 24, 29),   /* help */
};

/* A card's race key: its stats word's type, or RACE_KEY_EX + enum RaceEx for the races EDS lacks (gCardTypeEx; their
 * stats word holds the closest EDS type), so a Psychic is not listed as a Spellcaster. An added Divine-Beast is the
 * Gods' type. */
#define RACE_KEY_EX     32
#define RACE_KEY_COUNT  (RACE_KEY_EX + RACE_EX_COUNT)

/* Alphabetical. */
static const struct {
    u8 type;                    /* race key */
    const char *name;
} sTypes[] = {
    { CARD_TYPE_AQUA, "Aqua" },
    { CARD_TYPE_BEAST, "Beast" },
    { CARD_TYPE_BEAST_WARRIOR, "Beast-Warrior" },
    { RACE_KEY_EX + RACE_EX_CREATOR_GOD, "Creator God" },
    { RACE_KEY_EX + RACE_EX_CYBERSE, "Cyberse" },
    { CARD_TYPE_DINOSAUR, "Dinosaur" },
    { CARD_TYPE_DIVINE, "Divine-Beast" },
    { CARD_TYPE_DRAGON, "Dragon" },
    { CARD_TYPE_FAIRY, "Fairy" },
    { CARD_TYPE_FIEND, "Fiend" },
    { CARD_TYPE_FISH, "Fish" },
    { RACE_KEY_EX + RACE_EX_ILLUSION, "Illusion" },
    { CARD_TYPE_INSECT, "Insect" },
    { CARD_TYPE_MACHINE, "Machine" },
    { CARD_TYPE_PLANT, "Plant" },
    { RACE_KEY_EX + RACE_EX_PSYCHIC, "Psychic" },
    { CARD_TYPE_PYRO, "Pyro" },
    { CARD_TYPE_REPTILE, "Reptile" },
    { CARD_TYPE_ROCK, "Rock" },
    { CARD_TYPE_SEA_SERPENT, "Sea Serpent" },
    { CARD_TYPE_SPELLCASTER, "Spellcaster" },
    { CARD_TYPE_THUNDER, "Thunder" },
    { CARD_TYPE_WARRIOR, "Warrior" },
    { CARD_TYPE_WINGED_BEAST, "Winged Beast" },
    { RACE_KEY_EX + RACE_EX_WYRM, "Wyrm" },
    { CARD_TYPE_ZOMBIE, "Zombie" },
};

static u32 RaceKey(u32 id)
{
    u32 ex = CARD_RACE_EX(id);

    if (ex == RACE_EX_DIVINE_BEAST)
        return CARD_TYPE_DIVINE;
    if (ex != RACE_EX_NONE && ex < RACE_EX_COUNT)
        return RACE_KEY_EX + ex;
    return CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]);
}

/* The Divine Attribute has no stats value: the Gods carry 2 (Dark), and are told apart by their Type. */
#define ATTRIBUTE_DIVINE 7

/* Alphabetical. */
static const struct {
    u8 attribute;
    const char *name;
} sAttributes[] = {
    { ATTRIBUTE_DARK, "Dark" },
    { ATTRIBUTE_DIVINE, "Divine" },
    { ATTRIBUTE_EARTH, "Earth" },
    { ATTRIBUTE_FIRE, "Fire" },
    { ATTRIBUTE_LIGHT, "Light" },
    { ATTRIBUTE_WATER, "Water" },
    { ATTRIBUTE_WIND, "Wind" },
};

/* Sort keys: sTypes and sAttributes as ranks, indexed by the card's value. Non-monsters last. */
static const u8 sTypeRank[RACE_KEY_COUNT] = {
    [CARD_TYPE_AQUA] = 1,
    [CARD_TYPE_BEAST] = 2,
    [CARD_TYPE_BEAST_WARRIOR] = 3,
    [RACE_KEY_EX + RACE_EX_CREATOR_GOD] = 4,
    [RACE_KEY_EX + RACE_EX_CYBERSE] = 5,
    [CARD_TYPE_DINOSAUR] = 6,
    [CARD_TYPE_DIVINE] = 7,
    [CARD_TYPE_DRAGON] = 8,
    [CARD_TYPE_FAIRY] = 9,
    [CARD_TYPE_FIEND] = 10,
    [CARD_TYPE_FISH] = 11,
    [RACE_KEY_EX + RACE_EX_ILLUSION] = 12,
    [CARD_TYPE_INSECT] = 13,
    [CARD_TYPE_MACHINE] = 14,
    [CARD_TYPE_PLANT] = 15,
    [RACE_KEY_EX + RACE_EX_PSYCHIC] = 16,
    [CARD_TYPE_PYRO] = 17,
    [CARD_TYPE_REPTILE] = 18,
    [CARD_TYPE_ROCK] = 19,
    [CARD_TYPE_SEA_SERPENT] = 20,
    [CARD_TYPE_SPELLCASTER] = 21,
    [CARD_TYPE_THUNDER] = 22,
    [CARD_TYPE_WARRIOR] = 23,
    [CARD_TYPE_WINGED_BEAST] = 24,
    [RACE_KEY_EX + RACE_EX_WYRM] = 25,
    [CARD_TYPE_ZOMBIE] = 26,
    [CARD_TYPE_MAGIC] = 27,
    [CARD_TYPE_TRAP] = 28,
    [CARD_TYPE_TICKET] = 29,
};

static const u8 sAttributeRank[8] = {
    [ATTRIBUTE_DARK] = 1,
    [ATTRIBUTE_DIVINE] = 2,
    [ATTRIBUTE_EARTH] = 3,
    [ATTRIBUTE_FIRE] = 4,
    [ATTRIBUTE_LIGHT] = 5,
    [ATTRIBUTE_WATER] = 6,
    [ATTRIBUTE_WIND] = 7,
};

static const char *const sRowLabels[ROW_COUNT] = {
    "Name starts with",
    "Type",
    "Attribute",
    "ATK from",
    "ATK to",
    "DEF from",
    "DEF to",
    "Level from",
    "Level to",
};

static const u8 sRowMin[ROW_COUNT] = { 0, 0, 0, 0, 0, 0, 0, LEVEL_MIN, LEVEL_MIN };
static const u8 sRowMax[ROW_COUNT] = {
    NAME_OTHER, ARRAY_COUNT(sTypes), ARRAY_COUNT(sAttributes),
    POINTS_TO_MAX - 1, POINTS_TO_MAX, POINTS_TO_MAX - 1, POINTS_TO_MAX, LEVEL_MAX, LEVEL_MAX,
};

extern const u8 gListFilterFilterPageMap[];

static const char sStrAny[] = "Any";
static const char sStrMax[] = "Max";
static const char sStrOther[] = "Number or symbol";
static const char sStrCursor[] = ">";
static const char sStrHelp1[] = "Left/Right: change   L/R: by 5";
static const char sStrHelp2[] = "A: filter   B: back   Select: clear";

/* ---- Card tests ---- */

static u32 CardStatsWord(u32 id)
{
    return gCardStats[id & CARD_ID_MASK];
}

static int IsMonsterType(u32 type)
{
    return type != CARD_TYPE_TRAP && type != CARD_TYPE_MAGIC && type != CARD_TYPE_TICKET;
}

static u32 AttributeOf(u32 stats)
{
    return CARD_STATS_TYPE(stats) == CARD_TYPE_DIVINE ? ATTRIBUTE_DIVINE : CARD_STATS_ATTR(stats);
}

static int InPointsRange(u32 points, u32 from, u32 to)
{
    return points >= from * POINTS_STEP && (to == POINTS_TO_MAX || points <= to * POINTS_STEP);
}

static int MatchesName(u32 id, u32 letter)
{
    u32 c = gCardNames[(id & CARD_ID_MASK) * CARD_NAME_SIZE];

    if (c >= 'a' && c <= 'z')
        c -= 'a' - 'A';
    if (c >= 'A' && c <= 'Z')
        return letter == c - 'A' + 1;
    return letter == NAME_OTHER;
}

static int LimitsSet(const u8 *v)
{
    return v[ROW_TYPE] != 0 || v[ROW_ATTRIBUTE] != 0
        || v[ROW_ATK_FROM] != 0 || v[ROW_ATK_TO] != POINTS_TO_MAX
        || v[ROW_DEF_FROM] != 0 || v[ROW_DEF_TO] != POINTS_TO_MAX
        || v[ROW_LEVEL_FROM] != LEVEL_MIN || v[ROW_LEVEL_TO] != LEVEL_MAX;
}

static int MatchesLimits(u32 id, const u8 *v)
{
    u32 stats = CardStatsWord(id);
    u32 type = CARD_STATS_TYPE(stats);
    u32 atk, def, level;

    if (!IsMonsterType(type))
        return 0;
    if (v[ROW_TYPE] != 0 && sTypes[v[ROW_TYPE] - 1].type != RaceKey(id))
        return 0;
    if (v[ROW_ATTRIBUTE] != 0 && sAttributes[v[ROW_ATTRIBUTE] - 1].attribute != AttributeOf(stats))
        return 0;
    if (type == CARD_TYPE_DIVINE) {
        atk = DIVINE_POINTS;
        def = DIVINE_POINTS;
        level = DIVINE_LEVEL;
    } else {
        atk = CARD_STATS_ATK(stats) * CARD_STATS_POINTS_SCALE;
        def = CARD_STATS_DEF(stats) * CARD_STATS_POINTS_SCALE;
        level = CARD_STATS_LEVEL(stats);
    }
    return InPointsRange(atk, v[ROW_ATK_FROM], v[ROW_ATK_TO])
        && InPointsRange(def, v[ROW_DEF_FROM], v[ROW_DEF_TO])
        && level >= v[ROW_LEVEL_FROM] && level <= v[ROW_LEVEL_TO];
}

/* The end of DeckEdit_FilterAndSortList: `cards` is row 1 of `list`, already filtered and sorted. */
void ListRefine_ApplyToList(u8 list, u16 *cards)
{
    const u8 *v = REFINE->value;
    int limits = LimitsSet(v);
    u32 count = gDeckEdit.listCount[1][list];
    u32 kept = 0;
    u32 i;

    for (i = 0; i < count; i++) {
        u32 id = cards[i];

        if (v[ROW_NAME] != 0 && !MatchesName(id, v[ROW_NAME]))
            continue;
        if (limits && !MatchesLimits(id, v))
            continue;
        cards[kept++] = id;
    }
    gDeckEdit.listCount[1][list] = kept;
}

/* ---- Sort comparators (DeckEdit_FilterAndSortList): non-zero when a goes before b ---- */

static u32 NameLess(const u8 *nameA, const u8 *nameB)
{
    u32 ca;
    u32 cb;

    for (;;) {
        ca = *nameA;
        cb = *nameB;
        if (ca >= 'a' && ca <= 'z')
            ca -= 'a' - 'A';
        if (cb >= 'a' && cb <= 'z')
            cb -= 'a' - 'A';
        if (ca != cb)
            return ca < cb;
        if (ca == 0)
            return 0;
        nameA++;
        nameB++;
    }
}

static u32 RankedFirst(u32 rankA, u32 rankB, int a, int b)
{
    const u8 *nameA;
    const u8 *nameB;

    if (rankA != rankB)
        return rankA < rankB;
    nameA = &gCardNames[(a & CARD_ID_MASK) * CARD_NAME_SIZE];
    nameB = &gCardNames[(b & CARD_ID_MASK) * CARD_NAME_SIZE];
    return NameLess(nameA, nameB);
}

u32 ModCompareCardsByType(int a, int b)
{
    return RankedFirst(sTypeRank[RaceKey(a)], sTypeRank[RaceKey(b)], a, b);
}

u32 ModCompareCardsByAttribute(int a, int b)
{
    return RankedFirst(sAttributeRank[AttributeOf(CardStatsWord(a))], sAttributeRank[AttributeOf(CardStatsWord(b))], a, b);
}

/* ---- The page ---- */

static int OnlyNameRow(void)
{
    return gDeckEdit.filterSel == LIST_FILTER_MAGIC || gDeckEdit.filterSel == LIST_FILTER_TRAP;
}

static int RowCount(void)
{
    return OnlyNameRow() ? ROW_NAME + 1 : ROW_COUNT;
}

static void ClearLimits(u8 *v)
{
    int i;

    for (i = ROW_TYPE; i < ROW_COUNT; i++)
        v[i] = 0;
    v[ROW_ATK_TO] = POINTS_TO_MAX;
    v[ROW_DEF_TO] = POINTS_TO_MAX;
    v[ROW_LEVEL_FROM] = LEVEL_MIN;
    v[ROW_LEVEL_TO] = LEVEL_MAX;
}

void ListRefine_Reset(void)
{
    REFINE->row = 0;
    REFINE->value[ROW_NAME] = 0;
    ClearLimits(REFINE->value);
}

static void AppendNum(char *dst, u32 n)
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
}

static const char *RowValueText(int row, char *buf)
{
    u32 v = REFINE->value[row];

    switch (row) {
    case ROW_NAME:
        if (v == 0)
            return sStrAny;
        if (v == NAME_OTHER)
            return sStrOther;
        buf[0] = 'A' + v - 1;
        buf[1] = 0;
        return buf;
    case ROW_TYPE:
        return v == 0 ? sStrAny : sTypes[v - 1].name;
    case ROW_ATTRIBUTE:
        return v == 0 ? sStrAny : sAttributes[v - 1].name;
    case ROW_LEVEL_FROM:
    case ROW_LEVEL_TO:
        AppendNum(buf, v);
        return buf;
    case ROW_ATK_TO:
    case ROW_DEF_TO:
        if (v == POINTS_TO_MAX)
            return sStrMax;
        /* fall through */
    default:
        AppendNum(buf, v * POINTS_STEP);
        return buf;
    }
}

static void DrawText(int x, int y, u16 color, const char *text)
{
    TextDrawString(x + 1, y + 1, COLOR_SHADOW, (const u8 *)text);
    TextDrawString(x, y, color, (const u8 *)text);
}

static void Render(void)
{
    char buf[12];
    int i, count = RowCount();

    TextCanvasInitEx(TEXT_WIDTH, TEXT_HEIGHT, 0, 0);
    for (i = 0; i < count; i++) {
        u16 color = i == REFINE->row ? COLOR_CURSOR : COLOR_ROW;
        int y = ROWS_Y + i * ROW_HEIGHT;

        if (i == REFINE->row)
            DrawText(LABEL_X - 8, y, color, sStrCursor);
        DrawText(LABEL_X, y, color, sRowLabels[i]);
        DrawText(VALUE_X, y, color, RowValueText(i, buf));
    }
    DrawText(LABEL_X, HELP_Y, COLOR_HELP, sStrHelp1);
    DrawText(LABEL_X, HELP_Y + ROW_HEIGHT + 1, COLOR_HELP, sStrHelp2);
    TextCanvasToTiles(TEXT_TILES, COLOR_BOX);
}

/* As ListFilter_Init does for the two pages. */
static void DrawListIcon(u16 *map)
{
    CropMapBlock((u16 *)gListFilterListIconMap, 0, gDeckEdit.curList * 5, 7, map, 0x14, 0, 7, 5, 0);
}

static void RestorePages(void)
{
    REG_BG1CNT = BG1CNT_SCREEN;
    CopyMapRect(gListFilterFilterPageMap, TEXT_MAP, 30, 20);
    DrawListIcon(TEXT_MAP);
    CopyMapRect(gListFilterSortPageMap, SORT_MAP, 30, 20);
    DrawListIcon(SORT_MAP);
}

/* LIST_FILTER_PHASE_FILTER (Magic, Trap) or LIST_FILTER_PHASE_SORT confirmed: BG0 is on, the option's animation off. */
void ListRefine_Open(void)
{
    int r, c;
    u16 *map = TEXT_MAP;

    if (OnlyNameRow()) {
        REFINE->row = 0;
        ClearLimits(REFINE->value);
    }
    CpuSet(sTextPal, (u16 *)BG_PLTT + TEXT_PAL * 16, 16);
    for (r = 0; r < 32; r++) {
        for (c = 0; c < 32; c++) {
            SORT_MAP[r * 32 + c] = 0;
            if (r >= TEXT_FIRST_ROW && r < TEXT_FIRST_ROW + TEXT_HEIGHT && c < TEXT_WIDTH)
                map[r * 32 + c] = (TEXT_PAL << 12) | (TEXT_TILE_FIRST + (r - TEXT_FIRST_ROW) * TEXT_WIDTH + c);
            else
                map[r * 32 + c] = 0;
        }
    }
    DrawListIcon(SORT_MAP);
    Render();
    REG_BG1CNT = BG1CNT_TEXT;
    REG_DISPCNT |= 0x200;
    gDeckEdit.subPhase = LIST_FILTER_PHASE_REFINE;
}

static void ChangeValue(int delta)
{
    u8 *v = REFINE->value;
    int row = REFINE->row;
    int n = v[row] + delta;

    /* One step wraps around; L and R stop at the ends. */
    if (n < sRowMin[row])
        n = delta == -1 ? sRowMax[row] : sRowMin[row];
    else if (n > sRowMax[row])
        n = delta == 1 ? sRowMin[row] : sRowMax[row];
    v[row] = n;
    /* A range stays in order: the row that was not changed gives way. */
    if (row >= ROW_ATK_FROM) {
        if ((row - ROW_ATK_FROM) % 2 == 0) {
            if (v[row + 1] < n)
                v[row + 1] = n;
        } else if (v[row - 1] > n) {
            v[row - 1] = n;
        }
    }
}

static void Back(void)
{
    RestorePages();
    if (OnlyNameRow()) {
        REG_DISPCNT &= 0xFEFF;
        gDeckEdit.subPhase = LIST_FILTER_PHASE_FILTER;
        gDeckEdit.anims.anims[gDeckEdit.filterSel].active = ANIM_FINISHED;
    } else {
        REG_DISPCNT &= 0xFDFF;
        gDeckEdit.subPhase = LIST_FILTER_PHASE_SORT;
        gDeckEdit.anims.anims[gListSortOptionAnims[gDeckEdit.sortSel]].active = ANIM_FINISHED;
    }
}

static void Confirm(void)
{
    gDeckEdit.commandMenu.filter[gDeckEdit.curList] = gDeckEdit.filterSel;
    gDeckEdit.commandMenu.sort[gDeckEdit.curList] = gDeckEdit.sortSel;
    gDeckEdit.commandMenu.anim = CMDMENU_REFRESH;
    RestorePages();
    REG_DISPCNT &= 0xFDFF;
    ListFilter_ShowNowFiltering();
    gDeckEdit.subPhase = LIST_FILTER_PHASE_APPLY;
}

void ListRefine_Update(void)
{
    u16 keys = gMain.newKeys;
    int count = RowCount();

    if (keys & A_BUTTON) {
        PlaySE(SE_CONFIRM);
        Confirm();
        return;
    }
    if (keys & B_BUTTON) {
        PlaySE(SE_CANCEL);
        Back();
        return;
    }
    if (keys & DPAD_UP)
        REFINE->row = (REFINE->row + count - 1) % count;
    else if (keys & DPAD_DOWN)
        REFINE->row = (REFINE->row + 1) % count;
    else if (keys & DPAD_LEFT)
        ChangeValue(-1);
    else if (keys & DPAD_RIGHT)
        ChangeValue(1);
    else if (keys & L_BUTTON)
        ChangeValue(-BIG_STEP);
    else if (keys & R_BUTTON)
        ChangeValue(BIG_STEP);
    else if (keys & SELECT_BUTTON)
        ListRefine_Reset();
    else
        return;
    PlaySE(SE_CURSOR);
    Render();
}
