/*
 * mod_settings: Settings, the main menu's eleventh item (duel speed: DEFAULT / ALWAYS / SUPER). Not part of the
 * original ROM. The choice is stored in gSaveData.options bits 2-3 (src/save.h). L during a duel opens the same
 * screen inside the duel loop (ModDuelSettingsRun) and B goes back to the duel.
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
#include "duel.h"
#include "duel_flow.h"
#include "duel_screen.h"
#include "constants/sound.h"
#include "mod_duel_speed.h"
#include "chain.h"

extern const u8 gMainMenuObjPal[];
extern const u16 gMainMenuSkyImage[];

struct LabelChunk {
    u8 item;
    u8 x0;
    u8 x1;
};
void BuildMenuLabel(const struct LabelChunk *chunks, int count, u16 *dst);
#define LABEL_DRAWN     0x80

/* ---- Duel speed (gSaveData.options) ---- */

int ModGetSpeedMode(void)
{
    if (gSaveData.options & OPTION_SUPER_TURBO_ON)
        return MOD_SPEED_SUPER;
    if (gSaveData.options & OPTION_TURBO_ON)
        return MOD_SPEED_ALWAYS;
    return MOD_SPEED_DEFAULT;
}

void ModSetSpeedMode(int mode)
{
    gSaveData.options &= ~(OPTION_TURBO_ON | OPTION_SUPER_TURBO_ON);
    if (mode == MOD_SPEED_ALWAYS)
        gSaveData.options |= OPTION_TURBO_ON;
    else if (mode == MOD_SPEED_SUPER)
        gSaveData.options |= OPTION_SUPER_TURBO_ON;
}

void ModDuelApplySpeedSettings(void)
{
    if (ModGetSpeedMode() != MOD_SPEED_DEFAULT)
        gDuelScreen.fast = 1;
}

int ModDuelFastForwardActive(void)
{
    int mode = ModGetSpeedMode();

    if (mode == MOD_SPEED_SUPER || mode == MOD_SPEED_ALWAYS)
        return 1;
    return (gMain.heldKeys & B_BUTTON) || gDuelScreen.fast != 0;
}

int ModDuelFastTimerSkip(int base)
{
    if (!ModDuelFastForwardActive())
        return 0;
    if (ModGetSpeedMode() == MOD_SPEED_SUPER)
        return base * 2;
    return base;
}

int ModDuelFastAnimAdvance(int oldStep)
{
    if (ModGetSpeedMode() == MOD_SPEED_SUPER)
        return oldStep + 8;
    return oldStep + 4;
}

static int DeckReorderUsesShoulderButtons(void)
{
    switch (gChain.scratch.deckReorder.mode) {
    case DECK_REORDER_SELECT:
    case DECK_REORDER_SWAP_LEFT:
    case DECK_REORDER_SWAP_RIGHT:
        return 1;
    default:
        return 0;
    }
}

/* ---- Settings scene (EWRAM gSettingsMenu) ---- */

/* gSettingsMenu.duelState: Settings opened with L inside a duel, run from DuelMainStep the way the card menu's
 * Card Detail runs (fade the duel screen out, full-screen scene, DuelScreen_Init, fade back in). */
enum SettingsDuelState {
    SETTINGS_DUEL_NONE = 0,
    SETTINGS_DUEL_FADE_OUT = 1,
    SETTINGS_DUEL_RUN = 2,
    SETTINGS_DUEL_FADE_IN = 3,
};

struct SettingsMenu {
    u8 cursor;
    u8 duelState;
    u8 step;
    u8 subStep;
};

extern struct SettingsMenu gSettingsMenu;

static int SettingsScene_Step(int inDuel);

int ModDuelSettingsHandleInput(void)
{
    struct SettingsMenu *s = &gSettingsMenu;

    if (!(gMain.newKeys & L_BUTTON) || !gDuelScreen.active || gDuelCtrl.isLinkDuel)
        return 0;
    if (gDuel.cardMenu.open || DeckReorderUsesShoulderButtons())
        return 0;
    PlaySE(SE_CONFIRM);
    s->duelState = SETTINGS_DUEL_FADE_OUT;
    return 1;
}

int ModDuelSettingsRun(void)
{
    struct SettingsMenu *s = &gSettingsMenu;

    switch (s->duelState) {
    case SETTINGS_DUEL_FADE_OUT:
        DrawFieldOverlay();
        DrawHandCards();
        if (DuelScreen_FadeOutStep()) {
            gDuelScreen.uiGfxLoaded = 0;
            gDuelScreen.active = 0;
            ResetBgScroll();
            s->step = 0;
            s->subStep = 0;
            s->duelState = SETTINGS_DUEL_RUN;
        }
        return 1;
    case SETTINGS_DUEL_RUN:
        if (SettingsScene_Step(1)) {
            gDuelScreen.fast = ModGetSpeedMode() != MOD_SPEED_DEFAULT;
            DuelScreen_Init();
            DuelScreen_DrawCursorInfo();
            s->duelState = SETTINGS_DUEL_FADE_IN;
        }
        return 1;
    case SETTINGS_DUEL_FADE_IN:
        DrawFieldOverlay();
        DrawHandCards();
        if (DuelScreen_FadeInStep())
            s->duelState = SETTINGS_DUEL_NONE;
        return 1;
    default:
        s->duelState = SETTINGS_DUEL_NONE;
        return 0;
    }
}

/* ---- Menu label ---- */

/* "Settings": drawn S (sDrawnGlyphs, mod_recipes.c), e of Deck, tt of Battle, i, n and g of Campaign, s of Password. */
static const struct LabelChunk sSettingsLabel[] = {
    { LABEL_DRAWN | 6, 0, 8 },
    { 2, 40, 47 },
    { 1, 77, 83 },
    { 1, 77, 83 },
    { 0, 79, 82 },
    { 0, 92, 98 },
    { 0, 83, 90 },
    { 6, 50, 55 },
};

void SettingsMenu_BuildLabel(u16 *dst)
{
    BuildMenuLabel(sSettingsLabel, ARRAY_COUNT(sSettingsLabel), dst);
}

#define TITLE_TILE      0x000
#define CANVAS_TILE     0x040
#define CANVAS_ROWS     12
#define CANVAS_Y        28
#define ROW_HEIGHT      14
#define OBJ_TILE(n)     (OBJ_VRAM0 + (n) * 0x20)

#define COLOR_ROW       TEXT_SIZE_COLOR(10, 8)
#define COLOR_CURSOR    TEXT_SIZE_COLOR(10, 7)
#define COLOR_HELP      TEXT_SIZE_COLOR(10, 10)
#define COLOR_SHADOW    TEXT_SIZE_COLOR(10, 1)

static const char sStrSpeedHeader[] = "Duel speed";
static const char *const sSpeedModeNames[] = { "DEFAULT", "ALWAYS", "SUPER" };
static const char sStrHelp[] = "A: select   B: back";
static const char sStrHelpL[] = "L in duel: settings";

static void DrawText(int x, int y, u16 color, const char *text)
{
    TextDrawString(x + 1, y + 1, COLOR_SHADOW, (const u8 *)text);
    TextDrawString(x, y, color, (const u8 *)text);
}

static void Render(void)
{
    struct SettingsMenu *s = &gSettingsMenu;
    char line[48];
    int i;
    int active = ModGetSpeedMode();

    TextCanvasInitEx(32, CANVAS_ROWS, 0, 0);
    DrawText(8, 0, COLOR_ROW, sStrSpeedHeader);
    for (i = 0; i < 3; i++) {
        u16 color = s->cursor == i ? COLOR_CURSOR : COLOR_ROW;
        char *p = line;

        *p++ = s->cursor == i ? '>' : ' ';
        *p++ = ' ';
        {
            const char *src = sSpeedModeNames[i];

            while (*src)
                *p++ = *src++;
        }
        if (active == i) {
            *p++ = ' ';
            *p++ = '*';
        }
        *p = 0;
        DrawText(8, (i + 1) * ROW_HEIGHT, color, line);
    }
    DrawText(8, 4 * ROW_HEIGHT + 4, COLOR_HELP, sStrHelpL);
    DrawText(8, 4 * ROW_HEIGHT + 18, COLOR_HELP, sStrHelp);
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

static void SelectCursor(void)
{
    struct SettingsMenu *s = &gSettingsMenu;

    if (ModGetSpeedMode() != s->cursor) {
        ModSetSpeedMode(s->cursor);
        PlaySE(SE_CONFIRM);
    } else {
        PlaySE(SE_CURSOR);
    }
    Render();
}

static int HandleInput(void)
{
    struct SettingsMenu *s = &gSettingsMenu;
    u16 keys = gMain.newKeys;

    if (keys & B_BUTTON) {
        PlaySE(SE_CANCEL);
        return 1;
    }
    if (keys & DPAD_UP)
        s->cursor = (s->cursor + 2) % 3;
    else if (keys & DPAD_DOWN)
        s->cursor = (s->cursor + 1) % 3;
    else if (keys & A_BUTTON)
        SelectCursor();
    else
        return 0;
    if (keys & (DPAD_UP | DPAD_DOWN))
        PlaySE(SE_CURSOR);
    if (keys & (DPAD_UP | DPAD_DOWN))
        Render();
    return 0;
}

/* One frame of the scene: set up, fade in, input, fade out; 1 when it has faded out. In a duel (inDuel) the duel
 * music keeps playing. */
static int SettingsScene_Step(int inDuel)
{
    struct SettingsMenu *s = &gSettingsMenu;

    switch (s->step) {
    case 0:
        switch (s->subStep++) {
        case 0:
            REG_DISPCNT = 0;
            break;
        case 1:
            SetBrightnessBlack();
            ResetVideo();
            ResetBgScroll();
            REG_BG1CNT = BGCNT_256COLOR | BGCNT_CHARBASE(1);
            gMain.vblankFlags = VBLANK_COPY_OAM | VBLANK_COPY_BG_MAPS;
            if (!inDuel)
                PlayBGMNoTrack(SONG_MAIN_MENU);
            break;
        default:
            CopyDoubleWords((void *)OBJ_PLTT, gMainMenuObjPal, 0x20);
            LoadBgImage(0, 0, 0x20, gMainMenuSkyImage);
            SettingsMenu_BuildLabel((u16 *)OBJ_TILE(TITLE_TILE));
            s->cursor = ModGetSpeedMode();
            Render();
            s->step++;
            s->subStep = 0;
            break;
        }
        return 0;
    case 1:
        REG_DISPCNT = DISPCNT_BG1_ON | DISPCNT_OBJ_ON;
        DrawSprites();
        if (FadeFromBlack(1))
            s->step++;
        return 0;
    case 2:
        DrawSprites();
        if (HandleInput()) {
            if (!inDuel)
                FadeOutBGM();
            s->step++;
        }
        return 0;
    default:
        DrawSprites();
        return FadeToBlack(2) != 0;
    }
}

/* Main menu item 10. Returns 1 when done (MainLoop goes back to the main menu, which saves the game). */
u16 CB_Settings(void)
{
    struct SettingsMenu *s = &gSettingsMenu;

    if (gMain.seqIndex1 == 0) {
        s->duelState = SETTINGS_DUEL_NONE;
        s->step = 0;
        s->subStep = 0;
        gMain.seqIndex1 = 1;
    }
    return SettingsScene_Step(0);
}
