/*
 * mod_duel_hud: the duel screen's life points and phase panel. Not part of the original ROM (wiki/game/duel-hud.md).
 * The board's middle row used to hold a YOU and an OPPONENT box with each player's life points and phase letters
 * (BG2 cells, DrawLifePoints and DrawPhaseIndicator in duel_info_bar.c, which now draw nothing). The board picture is
 * now gModDuelFieldImage (tools/duel_board.py): two Extra Monster Zones at its edges and one phase panel between them.
 *   ModDrawDuelHud  every frame the field shows (DrawFieldOverlay -> ModDrawCounterBadges, src/mod_field_icons.c):
 *                   - the life points as sprites fixed to the screen: "PLAYER" and yours at the bottom left (above the
 *                     info bar), "OPPONENT" and theirs at the top right (white labels, black digits) on a transparent background;
 *                   - over the phase panel (it scrolls with the board): the turn player's name (row 19) and the
 *                     phase letters D S M B M E (row 20), the current phase highlighted.
 * The sprites' tiles are OBJ tiles 0x3C4-0x3E9 (below the Spell Counter badges' 0x3F0) and their colours OBJ palettes
 * 13 and 14; both are written each frame, as other screens (the chain list, battles) reuse OBJ VRAM.
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "gba.h"                    /* OBJ_VRAM0, OBJ_PLTT */
#include "sprite.h"                 /* AddSprite, SPRITE_SHAPE_* */
#include "util.h"                   /* CopyDoubleWords */
#include "constants/duel.h"         /* DUEL_AREA_MONSTER */
#include "duel.h"                   /* gDuel, gDuelPlayers */
#include "link_monster.h"           /* ModDrawLinkArrows */

extern const u8 gDuelDigitsGfx[];           /* the LP digits: colour set 0 is tiles 0-9 */
extern const u8 gDuelDigitsPal[];
extern const u8 gPhaseIndicatorGfx[];       /* D S M B M E highlighted, then normal (12 tiles) */
extern const u8 gPhaseIndicatorPal[];
void RenderBoldGlyphTile(u8 ch, u16 *dst, u16 fg, u16 bg);     /* text_bg.c: an 8x8 glyph as a 4bpp tile */
s32 GetAreaX(int player, int area, int index);
s32 GetAreaY(u32 player, int area, int index);

#define TILE_PLAYER             0x3C4           /* "PLAYER", 6 tiles */
#define TILE_OPPONENT           0x3CA           /* "OPPONENT", 8 tiles */
#define TILE_DIGITS             0x3D2           /* 0-9 */
#define TILE_TURN               0x3DC           /* the turn player's name over the panel, 8 tiles */
#define TILE_PHASES             0x3E4           /* the six phase letters as drawn now */
#define PAL_PANEL               13              /* the phase letters' palette; 15 the turn name's colour */
#define PAL_LP                  14              /* the digits' palette; 14 the corner labels' colour */
#define COLOR_LABEL             14
#define COLOR_TURN              15
#define PRIORITY                (1 << 10)       /* behind the text boxes, as the counter badges */
#define TILE_BYTES              32

#define LP_DIGITS               5
#define PLAYER_X                4               /* bottom left, above the info bar (y 144) */
#define PLAYER_Y                128
#define OPPONENT_RIGHT          236             /* top right */
#define OPPONENT_Y              0
#define PHASE_LETTERS           6
#define INFO_BAR_Y              144

static const u8 sStrPlayer[] = "PLAYER";
static const u8 sStrOpponent[] = "OPPONENT";

void ModDrawDuelHudPanel(void);

static u16 *ObjTile(u32 tile)
{
    return (u16 *)(OBJ_VRAM0 + tile * TILE_BYTES);
}

/* A string as bold 8x8 glyph tiles from `tile`, colour `fg` on `bg` (0: transparent). */
static void RenderLabel(u32 tile, const u8 *s, u32 fg, u32 bg)
{
    for (; *s != 0; s++, tile++)
        RenderBoldGlyphTile(*s, ObjTile(tile), fg, bg);
}


/* Sprites of `count` consecutive 8x8 tiles from `tile` at (x, y), in 32x8 pieces. */
static void AddRow(int x, int y, u32 tile, u32 count, u32 pal)
{
    while (count > 0) {
        u32 n = count >= 4 ? 4 : count >= 2 ? 2 : 1;
        u16 shape = n == 4 ? SPRITE_SHAPE_32x8 : n == 2 ? SPRITE_SHAPE_16x8 : SPRITE_SHAPE_8x8;

        AddSprite(((u32)(y & 0xFF) << 16) | (x & 0x1FF), shape, pal << 12 | PRIORITY | tile);
        x += n * 8;
        tile += n;
        count -= n;
    }
}

/* A player's life points as digit sprites from x (right-aligned to `right` when nonzero). */
static void AddLifePoints(int x, int right, int y, u32 lp)
{
    u8 digits[LP_DIGITS];
    int n = 0, i;

    do {
        digits[n++] = lp % 10;
        lp /= 10;
    } while (lp != 0 && n < LP_DIGITS);
    if (right)
        x = right - n * 8;
    for (i = n - 1; i >= 0; i--, x += 8)
        AddSprite(((u32)y << 16) | x, SPRITE_SHAPE_8x8, PAL_LP << 12 | PRIORITY | (TILE_DIGITS + digits[i]));
}

void ModDrawDuelHud(void)
{
    ModDrawLinkArrows();                            /* src/mod_link.c */
    ModDrawDuelHudPanel();
}

void ModDrawDuelHudPanel(void)
{
    vu16 *panelPal = (vu16 *)(OBJ_PLTT + PAL_PANEL * 32), *lpPal = (vu16 *)(OBJ_PLTT + PAL_LP * 32);
    const u8 *turnName = gDuel.turnPlayer == 0 ? sStrPlayer : sStrOpponent;
    u32 turnLength = gDuel.turnPlayer == 0 ? sizeof(sStrPlayer) - 1 : sizeof(sStrOpponent) - 1;
    int centre, top, i;

    CopyDoubleWords((void *)panelPal, gPhaseIndicatorPal, 32);
    CopyDoubleWords((void *)lpPal, gDuelDigitsPal, 32);
    panelPal[COLOR_TURN] = 0x2482;                  /* dark navy */
    lpPal[COLOR_LABEL] = 0x7FFF;                    /* the labels white; the digits keep the sheet's black */

    /* the life points, fixed to the screen corners, on a transparent background */
    RenderLabel(TILE_PLAYER, sStrPlayer, COLOR_LABEL, 0);
    RenderLabel(TILE_OPPONENT, sStrOpponent, COLOR_LABEL, 0);
    CopyDoubleWords(ObjTile(TILE_DIGITS), gDuelDigitsGfx, 10 * TILE_BYTES);
    AddRow(PLAYER_X, PLAYER_Y, TILE_PLAYER, sizeof(sStrPlayer) - 1, PAL_LP);
    AddLifePoints(PLAYER_X, 0, PLAYER_Y + 8, gDuelPlayers[0].lifePoints);
    AddRow(OPPONENT_RIGHT - (sizeof(sStrOpponent) - 1) * 8, OPPONENT_Y, TILE_OPPONENT, sizeof(sStrOpponent) - 1, PAL_LP);
    AddLifePoints(0, OPPONENT_RIGHT, OPPONENT_Y + 8, gDuelPlayers[1].lifePoints);

    /* the phase panel: the board's middle row, between player 0's monster row and the opponent's */
    centre = GetAreaX(0, DUEL_AREA_MONSTER, 2) + 16;
    top = GetAreaY(0, DUEL_AREA_MONSTER, 2) - 32;
    if (top + 24 <= 0 || top + 8 >= INFO_BAR_Y)
        return;                                     /* scrolled away */
    RenderLabel(TILE_TURN, turnName, COLOR_TURN, 0);
    for (i = 0; i < PHASE_LETTERS; i++) {
        u32 src = i == gDuel.phase ? i : PHASE_LETTERS + i;     /* highlighted, else normal */

        CopyDoubleWords(ObjTile(TILE_PHASES + i), gPhaseIndicatorGfx + src * TILE_BYTES, TILE_BYTES);
    }
    if (top + 8 >= 0)
        AddRow(centre - (int)turnLength * 4, top + 8, TILE_TURN, turnLength, PAL_PANEL);
    if (top + 16 < INFO_BAR_Y)
        AddRow(centre - PHASE_LETTERS * 4, top + 16, TILE_PHASES, PHASE_LETTERS, PAL_PANEL);
}
