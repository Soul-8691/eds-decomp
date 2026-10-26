/*
 * mod_dp: Duelist Points (DP). Not part of the original ROM.
 *   GetDP / AddDP / SpendDP   the player's DP, kept in the save image (gSaveData.dpLow/dpHigh, save.h: halfwords the
 *                             original game never uses). A save without DP yet (a New Game, or a save from before
 *                             DP: gSaveData.dpInit is not DP_INIT_MARK) starts with DP_START.
 *   GetPackCost               DP to open a booster pack (src/mod_packs.c), by pack id
 *   GetCardCost               DP to buy a card with the password machine (password_trade.c), by card
 *   AwardDuelDP               DP earned by beating a Campaign duelist (Campaign_RecordDuelResult)
 *   DpText_*                  a line of text drawn into OBJ tiles and shown as 32 x 8 sprites (a 32 x 2 tile canvas,
 *                             so it works with 1D and 2D OBJ mapping), for the screens that show DP:
 *                             the main menu (MainMenuDP_*), the Booster Packs list (PackListDP_*) and the password
 *                             machine (PasswordDP_*).
 * Like the other mod units, this one is @farcode and @rodata in units.txt; its state is in gSaveData and in gModDP
 * (EWRAM, symbols.ld), never in .bss.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "util.h"
#include "sprite.h"
#include "text.h"
#include "save.h"
#include "booster.h"
#include "card_data.h"
#include "constants/cards.h"
#include "constants/game.h"

#define DP_START        5000
#define DP_MAX          999999
#define DP_INIT_MARK    0xD5

/* ---- Prices and rewards (edit these) ---- */

#define DEFAULT_PACK_COST       1000
#define DEFAULT_CARD_COST       500
#define DEFAULT_DUELIST_REWARD  1000

struct PackCost {
    u16 packId;
    u16 cost;
};

/* DP to open each pack (packs not listed: DEFAULT_PACK_COST). */
static const struct PackCost sPackCosts[] = {
    { PACK_VOL_1, 1000 },
    { PACK_VOL_2, 1000 },
    { PACK_VOL_3, 1000 },
    { PACK_VOL_4, 1000 },
    { PACK_VOL_5, 1000 },
    { PACK_VOL_6, 1000 },
    { PACK_VOL_7, 1000 },
    { PACK_LOB_EWD, 1000 },
    { PACK_PHANTOM_OF_G, 1000 },
    { PACK_MAGIC_RULER, 1000 },
    { PACK_PHARAOHS_SERVANT, 1000 },
    { PACK_CURSE_OF_ANUBIS, 1000 },
    { PACK_PREMIUM_3, 1000 },
    { PACK_CELEMONY, 1000 },
    { PACK_EXPERT_1, 1000 },
    { PACK_EXPERT_2, 1000 },
    { PACK_EXPERT_3, 1000 },
    { PACK_DUELIST_PACK, 1000 },
    { PACK_THE_FINAL_DUELIST, 1000 },
    { PACK_RARE_SELECTIONS, 1000 },
    { PACK_EXPERT_4, 1000 },
    { PACK_EXPERT_5, 1000 },
    { PACK_LIMITED_COLLECTION, 1000 },
    { PACK_WEEKLY_YUGIOH, 1000 },
    { PACK_YUGIOH_MAGAZINE, 1000 },
    { PACK_WEEKLY_YUGIOH_B, 1000 },
    { PACK_GRANDPA_CUP_PRIZE, 1000 },
};

struct CardCost {
    u16 number;     /* card number (CARD_*) */
    u16 cost;
};

/* DP to buy a card with a password, for the cards that do not cost DEFAULT_CARD_COST, e.g.
 * { CARD_BLUE_EYES_WHITE_DRAGON, 3000 }. */
static const struct CardCost sCardCosts[] = {
    { 0, 0 },
};

/* DP for beating each Campaign duelist, by enum DuelistId. */
static const u16 sDuelistRewards[] = {
    [DUELIST_YUGI] = 1000,
    [DUELIST_TEA] = 1000,
    [DUELIST_JOEY] = 1000,
    [DUELIST_TRISTAN] = 1000,
    [DUELIST_BAKURA] = 1000,
    [DUELIST_REX] = 1000,
    [DUELIST_ESPA_ROBA] = 1000,
    [DUELIST_WEEVIL] = 1000,
    [DUELIST_MAKO] = 1000,
    [DUELIST_MAI] = 1000,
    [DUELIST_RARE_HUNTER] = 1000,
    [DUELIST_ARKANA] = 1000,
    [DUELIST_STRINGS] = 1000,
    [DUELIST_UMBRA_LUMIS] = 1000,
    [DUELIST_MARIK] = 1000,
    [DUELIST_KAIBA] = 1000,
    [DUELIST_ISHIZU] = 1000,
    [DUELIST_SHADI] = 1000,
    [DUELIST_YAMI_BAKURA] = 1000,
    [DUELIST_YAMI_YUGI] = 1000,
    [DUELIST_DUEL_COMPUTER] = 1000,
    [DUELIST_SIMON] = 1000,
    [DUELIST_PEGASUS] = 1000,
    [DUELIST_GRANDPA] = 1000,
};

/* ---- DP ---- */

static void SetDP(u32 dp)
{
    if (dp > DP_MAX)
        dp = DP_MAX;
    gSaveData.dpLow = dp;
    gSaveData.dpHigh = dp >> 16;
    gSaveData.dpInit = DP_INIT_MARK;
}

u32 GetDP(void)
{
    if (gSaveData.dpInit != DP_INIT_MARK)
        SetDP(DP_START);
    return gSaveData.dpLow | (u32)gSaveData.dpHigh << 16;
}

void AddDP(u32 amount)
{
    SetDP(GetDP() + amount);
}

/* Take amount DP; 0 (and nothing taken) when the player has less. */
int SpendDP(u32 amount)
{
    u32 dp = GetDP();

    if (dp < amount)
        return 0;
    SetDP(dp - amount);
    return 1;
}

u32 GetPackCost(u16 packId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sPackCosts); i++) {
        if (sPackCosts[i].packId == packId)
            return sPackCosts[i].cost;
    }
    return DEFAULT_PACK_COST;
}

u32 GetCardCost(u16 id)
{
    u16 number = gCardIdToNumber[id & CARD_ID_MASK];
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sCardCosts); i++) {
        if (sCardCosts[i].number == number && sCardCosts[i].cost != 0)
            return sCardCosts[i].cost;
    }
    return DEFAULT_CARD_COST;
}

void AwardDuelDP(u16 duelist)
{
    AddDP(duelist < ARRAY_COUNT(sDuelistRewards) && sDuelistRewards[duelist] ? sDuelistRewards[duelist]
                                                                              : DEFAULT_DUELIST_REWARD);
}

/* ---- Text lines as sprites ---- */

/* A line's tiles: a 32 x 2 tile canvas (row stride 32, as 2D mapping needs; 1D mapping reads it the same way). */
#define LINE_TILES      64
#define OBJ_TILE(n)     ((u16 *)(OBJ_VRAM0 + (n) * 0x20))

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

/* Draw text into the line at OBJ tile `tile`; returns its width in pixels. */
static int DpText_Render(u32 tile, const char *text, u16 color, u16 shadow)
{
    TextCanvasInitEx(32, 2, 0, 0);
    TextDrawString(1, 3, shadow, (const u8 *)text);
    TextDrawString(0, 2, color, (const u8 *)text);
    TextCanvasToTiles(OBJ_TILE(tile), 0);
    return gTextCanvas.right + 1;
}

/* Show the line at OBJ tile `tile` at (x, y): enough 32 x 8 sprites for width pixels. */
static void DpText_Draw(int x, int y, u32 tile, u32 palette, int width)
{
    int r, k;

    for (r = 0; r < 2; r++) {
        for (k = 0; k * 32 < width && k < 8; k++)
            AddSprite((((y + r * 8) & 0xFF) << 16) | ((x + k * 32) & 0x1FF), SPRITE_SHAPE_32x8,
                      (tile + r * 32 + k * 4) | OAM_ATTR2_PALETTE(palette));
    }
}

/* The screens' state: gModDP (EWRAM 0x0202F100, symbols.ld). */
struct ModDP {
    u16 menuWidth;
    u16 packTopWidth;
    u16 packBottomWidth;
    u16 passwordWidth;
    u32 packShownDP;        /* what the pack list lines show (rendered again when it changes) */
    u16 packShownId;
    u16 packShownFlag;      /* 1: they show "Not enough DP!" */
    u16 packNotEnough;      /* frames left of "Not enough DP!" on the pack list */
    u16 passwordNotEnoughWidth;
    u32 passwordShownDP;
};
extern struct ModDP gModDP;

static const char sStrDP[] = "DP ";
static const char sStrDPSuffix[] = " DP";
static const char sStrYouHave[] = "You have ";
static const char sStrPackHelp[] = "    A: open   B: back";
static const char sStrNotEnough[] = "Not enough DP!";
static const char sStrNeed[] = "Need ";
static const char sStrNeedSuffix[] = " DP!";

/* ---- Main menu (2D mapping, palette 0 = gMainMenuObjPal: colour 10 near-white, 1 black) ---- */

#define MENU_DP_TILE    0x2C0           /* rows 22-23, after the added labels */

void MainMenuDP_Render(void)
{
    char line[24];

    AppendNum(AppendStr(line, sStrDP), GetDP());
    gModDP.menuWidth = DpText_Render(MENU_DP_TILE, line, TEXT_SIZE_COLOR(10, 10), TEXT_SIZE_COLOR(10, 1));
}

void MainMenuDP_Draw(void)
{
    DpText_Draw(236 - gModDP.menuWidth, 144, MENU_DP_TILE, 0, gModDP.menuWidth);
}

/* ---- Booster Packs list (1D mapping, OBJ unused: tiles from 0x200, palette 15 = gSystemFontPal) ---- */

#define PACK_TOP_TILE       0x200
#define PACK_BOTTOM_TILE    0x240
#define FONT_PALETTE        15
#define FONT_COLOR          TEXT_SIZE_COLOR(10, 2)
#define FONT_SHADOW         TEXT_SIZE_COLOR(10, 15)

/* The list's lines: "<pack name>   <cost> DP" at the top, "You have <DP> DP   A: open   B: back" (or "Not enough
 * DP!") at the bottom. Drawn each frame while the list is up; rendered again when the pack or the DP change. */
void PackListDP_Draw(u16 packId, const char *packName)
{
    struct ModDP *s = &gModDP;
    char line[96];
    char *p;

    if (s->packShownId != packId || s->packShownDP != GetDP() || s->packShownFlag != (s->packNotEnough != 0)) {
        s->packShownId = packId;
        s->packShownDP = GetDP();
        s->packShownFlag = s->packNotEnough != 0;
        MemCopy16((void *)(OBJ_PLTT + FONT_PALETTE * 0x20), gSystemFontPal, 0x20);
        p = AppendStr(line, packName);
        p = AppendStr(p, "   ");
        AppendStr(AppendNum(p, GetPackCost(packId)), sStrDPSuffix);
        s->packTopWidth = DpText_Render(PACK_TOP_TILE, line, FONT_COLOR, FONT_SHADOW);
        if (s->packNotEnough) {
            p = AppendStr(line, sStrNotEnough);
        } else {
            p = AppendStr(AppendNum(AppendStr(line, sStrYouHave), GetDP()), sStrDPSuffix);
            p = AppendStr(p, sStrPackHelp);
        }
        s->packBottomWidth = DpText_Render(PACK_BOTTOM_TILE, line, FONT_COLOR, FONT_SHADOW);
    }
    if (s->packNotEnough)
        s->packNotEnough--;
    DpText_Draw(120 - s->packTopWidth / 2, 2, PACK_TOP_TILE, FONT_PALETTE, s->packTopWidth);
    DpText_Draw(120 - s->packBottomWidth / 2, 146, PACK_BOTTOM_TILE, FONT_PALETTE, s->packBottomWidth);
}

/* A refused pack: "Not enough DP!" for two seconds. */
void PackListDP_NotEnough(void)
{
    gModDP.packNotEnough = 120;
}

/* The list is (re)built: render its lines again. */
void PackListDP_Reset(void)
{
    gModDP.packShownId = 0xFFFF;
    gModDP.packNotEnough = 0;
}

/* ---- Password machine (2D mapping, its graphics in tiles 0-0x2BF and palette 1: rows 22+ and palette 15) ---- */

#define PASSWORD_DP_TILE        0x2C0
#define PASSWORD_NOT_ENOUGH_TILE 0x300

/* 1 if the player can buy card id (enough DP, and room in the trunk). */
int CanBuyCard(u16 id)
{
    return GetDP() >= GetCardCost(id) && gSaveData.trunk[id & CARD_ID_MASK].count < 0x3FF;
}

/* "DP <n>" at the top right, during the password entry, the roll and the reveal. */
void PasswordDP_Draw(void)
{
    struct ModDP *s = &gModDP;
    char line[24];

    if (s->passwordWidth == 0 || s->passwordShownDP != GetDP()) {
        s->passwordShownDP = GetDP();
        MemCopy16((void *)(OBJ_PLTT + FONT_PALETTE * 0x20), gSystemFontPal, 0x20);
        AppendNum(AppendStr(line, sStrDP), GetDP());
        s->passwordWidth = DpText_Render(PASSWORD_DP_TILE, line, FONT_COLOR, FONT_SHADOW);
    }
    DpText_Draw(236 - s->passwordWidth, 0, PASSWORD_DP_TILE, FONT_PALETTE, s->passwordWidth);
}

/* Password_RollAndCheck found card id but the player cannot buy it: render "Need <cost> DP!". */
void PasswordDP_SetNotEnough(u16 id)
{
    char line[24];

    AppendStr(AppendNum(AppendStr(line, sStrNeed), GetCardCost(id)), sStrNeedSuffix);
    gModDP.passwordNotEnoughWidth = DpText_Render(PASSWORD_NOT_ENOUGH_TILE, line, FONT_COLOR, FONT_SHADOW);
}

/* The password scene starts: render its line again. */
void PasswordDP_Reset(void)
{
    gModDP.passwordWidth = 0;
}

/* "Need <cost> DP!" where the digits are (Password_ShowUsed: a valid password the player cannot pay for). */
void PasswordDP_DrawNotEnough(void)
{
    DpText_Draw(24, 8, PASSWORD_NOT_ENOUGH_TILE, FONT_PALETTE, gModDP.passwordNotEnoughWidth);
}
