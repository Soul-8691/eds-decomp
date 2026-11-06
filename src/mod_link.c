/*
 * mod_link: Link Monsters, the Extra Monster Zones and the Monster Zone pick of every summon. Not part of the
 * original ROM (wiki/game/link.md, include/link_monster.h).
 *
 * Extra Monster Zones. Each player keeps the engine's 5 monster records; one of them can stand in an EMZ (screen
 * left or right, drawn at the board's edges by tools/duel_board.py). gModLink.emzSerial holds the zone serial of the
 * monster in each EMZ: the serial travels with the zone record (DuelCmd_MoveToZone, SwapFieldCards), so control
 * changes keep it, and ModLinkZoneCleared (the ClearZone hook, through ModXyzZoneCleared) frees it. Every engine loop
 * over the 5 records covers an EMZ monster as is; only the drawing asks where a record stands (ModZoneFieldX/Y, from
 * GetAreaX/Y in card_canvas.c and DrawZoneTiles/ClearZoneTiles in duel_field_screen.c). A player holds at most 5
 * monsters in all, and at most one in an EMZ.
 *
 * Each EMZ counts as the Main Monster Zone at its screen edge: a monster in the left EMZ uses the record of Main
 * zone 1, one in the right EMZ that of zone 5 (mirrored for the opponent: 5 and 1), so that zone must be free and
 * stays unusable while it is there (EmzRecordOf). Link Arrows use the real columns: the left EMZ is above Main
 * column 2, the right one above column 4. Main Monster Zone columns run left to right on the screen; the opponent's
 * are mirrored, and its arrows are turned around.
 *
 * Zone pick. ModSummonZoneStep (SummonAction_Update, before the kind's step machine) lets the human choose the
 * Monster Zone of every summon but a Flip Summon (a free one, or a Tribute's), plus a free EMZ for an Extra Deck
 * monster (Master Rule 5); a Link Monster only goes to an EMZ or a zone one of its controller's Link Monsters points
 * to. The CPU keeps the engine's zone (a Link Monster: an EMZ if free, else a pointed zone). A Link Monster is always
 * face-up Attack Position (also ModSummonDefenseChoice).
 *
 * Link Summon: from the Fusion Deck pile's Special Summon list (shared with Synchro and Xyz, src/mod_synchro.c):
 * the materials (face-up monsters fitting the rule; a Link Monster counts as 1 or as its Link Rating), adding up to
 * the Link Rating; they go to the Graveyard; the Link Monster is Special Summoned to the place picked. Not in Link
 * Battles (the EMZ placement is not sent to the partner).
 * Like the other mod units, this one is @farcode and @rodata in units.txt and keeps its state in fixed EWRAM
 * (gModLink, symbols.ld).
 */

#include "global.h"
#include "gba.h"                    /* DPAD_*, A_BUTTON, B_BUTTON, START_BUTTON */
#include "main.h"                   /* gMain.newKeys, frameCounter */
#include "card_data.h"              /* gCardStats, IS_LINK_CARD, LINK_MARKERS, LINK_RATING */
#include "card_list_view.h"
#include "sprite.h"                 /* AddSprite, AddSpriteFlip */
#include "util.h"                   /* StrCopy, StrCat, StrCatNumber, MemClear16 */
#include "constants/card_stats.h"
#include "constants/duel.h"
#include "constants/duel_cmds.h"
#include "constants/sound.h"
#include "duel.h"
#include "duel_actions.h"           /* SendFieldCardToGrave */
#include "duel_cmd.h"               /* DuelCmd_Push */
#include "duel_flow.h"              /* gDuelCtrl.isLinkDuel */
#include "duel_screen.h"            /* gDuelZonePositions, gDuelScreen, ClearTileBlock4x4, DuelScreen_ScrollToZone */
#include "effect.h"
#include "sound.h"
#include "summon.h"                 /* gSummonAction, QueueSpecialSummonChoosePosition, CanSpecialSummon */
#include "text_box.h"
#include "link_monster.h"
#include "link_rules.h"             /* LINK_RULES (build/include, tools/newcards.py) */

int AiShouldSetMonster(u16 cardId, u16 faceUp);
extern const u32 gZoneMarkerAnimTiles[8];

#define MONSTER_ZONES       5
#define EMZ_SLOTS           2
#define EMZ_FIELD_Y         144             /* the board's middle row */
#define NO_PLACE            0xFF
#define SUMMON_STATUS       ZONE_STATUS_UNK14
#define PICK_OWN_FACE_UP    (PICK_FACE_UP_MONSTER | PICK_ATTACK_POSITION | PICK_DEFENSE_POSITION)
#define IS_TOKEN_NUMBER(no) ((u16)((no) - CARD_NUMBER_TOKEN_FIRST) < CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST)

/* Arrow sprites: OBJ tiles (free between the HUD's and the badges'), OBJ palette 12's entry 4 (the badges' palette
 * uses 1-3). */
#define TILE_ARROW_UP       0x3EA
#define TILE_ARROW_LEFT     0x3EB
#define TILE_ARROW_CORNER   0x3EC           /* pointing up-left */
#define ARROW_PALETTE       12
#define ARROW_COLOR         4
#define FLIP_H              0x1000
#define FLIP_V              0x2000

enum LinkMaterialKind {
    LINK_MAT_ANY,
    LINK_MAT_NORMAL,
    LINK_MAT_EFFECT,
    LINK_MAT_TYPE,
    LINK_MAT_ATTRIBUTE,
    LINK_MAT_LEVEL,
};

/* gModLink (EWRAM 0x0202D700, symbols.ld). */
struct ModLink {
    u16 emzSerial[EMZ_SLOTS];       /* the zone serial of the monster standing in each EMZ */
    u8 emzUsed[EMZ_SLOTS];
    /* the summon in progress goes to an EMZ: (pendingPlayer, pendingZone) stands in pendingSlot until it arrives */
    u8 pendingActive, pendingPlayer, pendingZone, pendingSlot;
    /* the zone of the summon in progress */
    u8 zoneFixed, fixedZone, fixedSlot;     /* set by ModSummonSetZone: no prompt */
    u8 pickDone, picking, pickIndex, candCount;
    u8 cands[MONSTER_ZONES + EMZ_SLOTS];    /* 0-4: a record's Main zone; 16 + s: EMZ slot s */
    u8 pickPlayer;
    /* the human's Link Summon */
    u8 materials, value, cancelled, placing;
    u8 restrictMask;                /* ModSummonRestrictZones: the next summon only to these Main zones */
    u8 pad[1];
    u32 proxy[2];                   /* Proxy Dragon's state (src/added_effects.c, ModLinkProxyState) */
    u32 link;                       /* the card word of the Link Monster being summoned */
    u16 summoningId;
    u16 pad2;
    char text[96];
};
extern struct ModLink gModLink;
STATIC_ASSERT(sizeof(struct ModLink) <= 0x100, ModLinkFits);

/* { number, count, or more, kind, value } */
static const u16 sRules[][5] = LINK_RULES;

/* The marker bits in order (TL T TR L R BL B BR) as (dx, dy), dy +1 toward the opponent of player 0. */
static const s8 sDir[8][2] = {
    { -1, 1 }, { 0, 1 }, { 1, 1 }, { -1, 0 }, { 1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 },
};
static const s16 sEmzFieldX[EMZ_SLOTS] = { 40, 168 };
static const u8 sEmzGridX[EMZ_SLOTS] = { 1, 3 };     /* for the arrows: above Main column 2 and 4 */

static const u8 sStrPickZone[] = "Select a zone for the monster. (+Control Pad, A)";
static const u8 sStrPickPlace[] = "Select a zone for the Link Monster. (+Control Pad, A)";

static inline struct DuelZone *ZoneOf(int player, int zone)
{
    return &gDuelPlayers[player & 1].zones[zone];
}

/* ---- Extra Monster Zones ---- */

void ModLinkReset(void)
{
    MemClear16(&gModLink, sizeof(gModLink));
}

int ModEmzSlotOf(int player, int zone)
{
    struct DuelZone *z;
    int s;

    if (zone >= MONSTER_ZONES)
        return -1;
    if (gModLink.pendingActive && gModLink.pendingPlayer == (player & 1) && gModLink.pendingZone == zone)
        return gModLink.pendingSlot;
    z = ZoneOf(player, zone);
    if (DUEL_CARD_ID(&z->card) == 0)
        return -1;
    for (s = 0; s < EMZ_SLOTS; s++) {
        if (gModLink.emzUsed[s] && gModLink.emzSerial[s] == z->serial)
            return s;
    }
    return -1;
}

/* The slot's monster record as player * 5 + zone, or -1 (a pending summon counts). */
static int EmzOccupant(int slot)
{
    int player, zone;

    for (player = 0; player <= 1; player++) {
        for (zone = 0; zone < MONSTER_ZONES; zone++) {
            if (ModEmzSlotOf(player, zone) == slot)
                return player * MONSTER_ZONES + zone;
        }
    }
    return -1;
}

/* 1 if `player` has a monster in an EMZ (not counting the records in `except`). */
static int HasEmzMonster(int player, u32 except)
{
    int zone;

    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        if (!(except & (1 << zone)) && ModEmzSlotOf(player, zone) >= 0)
            return 1;
    }
    return 0;
}

s32 ModZoneFieldX(u32 player, u32 zone)
{
    int s = ModEmzSlotOf(player, zone);

    return s >= 0 ? sEmzFieldX[s] : gDuelZonePositions[player & 1][zone].x;
}

s32 ModZoneFieldY(u32 player, u32 zone)
{
    int s = ModEmzSlotOf(player, zone);

    return s >= 0 ? EMZ_FIELD_Y : gDuelZonePositions[player & 1][zone].y;
}

void ModLinkZoneCleared(int player, int zone)
{
    struct DuelZone *z;
    int s;

    if (zone >= MONSTER_ZONES)
        return;
    z = ZoneOf(player, zone);
    if (DUEL_CARD_ID(&z->card) == 0)
        return;
    for (s = 0; s < EMZ_SLOTS; s++) {
        if (gModLink.emzUsed[s] && gModLink.emzSerial[s] == z->serial) {
            gModLink.emzUsed[s] = 0;
            ClearTileBlock4x4(sEmzFieldX[s] / 8, EMZ_FIELD_Y / 8);
        }
    }
}

int ModLinkArrived(int player, int zone, u16 id)
{
    int linkSummoned = gModLink.summoningId != 0 && (id & CARD_ID_MASK) == gModLink.summoningId;

    if (linkSummoned)
        gModLink.summoningId = 0;
    if (gModLink.pendingActive && gModLink.pendingPlayer == (player & 1) && gModLink.pendingZone == zone) {
        gModLink.emzSerial[gModLink.pendingSlot] = ZoneOf(player, zone)->serial;
        gModLink.emzUsed[gModLink.pendingSlot] = 1;
        gModLink.pendingActive = 0;
    }
    return linkSummoned;
}

/* ---- Link Arrows ---- */

/* The grid place of record (player, zone): x 0-4, y 0 player 0's Main row, 1 the EMZ row, 2 player 1's Main row. */
static void GridOf(int player, int zone, int *gx, int *gy)
{
    int s = ModEmzSlotOf(player, zone);

    if (s >= 0) {
        *gx = sEmzGridX[s];
        *gy = 1;
    } else if ((player & 1) == 0) {
        *gx = zone;
        *gy = 0;
    } else {
        *gx = 4 - zone;
        *gy = 2;
    }
}

int ModLinkPointsTo(int player, int zone, u8 *out)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);
    u32 markers;
    int gx, gy, i, n = 0;

    if (zone >= MONSTER_ZONES || id == 0 || !z->isFaceUp || !IS_LINK_CARD(id))
        return 0;
    markers = LINK_MARKERS(id);
    GridOf(player, zone, &gx, &gy);
    for (i = 0; i < 8; i++) {
        int x, y;

        if (!(markers & (1 << i)))
            continue;
        if ((player & 1) == 0) {
            x = gx + sDir[i][0];
            y = gy + sDir[i][1];
        } else {
            x = gx - sDir[i][0];
            y = gy - sDir[i][1];
        }
        if (x < 0 || x > 4)
            continue;
        if (y == 0)
            out[n++] = x;
        else if (y == 2)
            out[n++] = 8 + (4 - x);
        else if (y == 1 && (x == sEmzGridX[0] || x == sEmzGridX[1]))
            out[n++] = LINK_PLACE_EMZ(x == sEmzGridX[0] ? 0 : 1);
    }
    return n;
}

/* The monster record at a place (player * 5 + zone), or -1 for none (an empty zone, or a Main column whose
 * record stands in an EMZ). */
static int PlaceOccupant(int place)
{
    int player, zone;

    if (LINK_PLACE_IS_EMZ(place))
        return EmzOccupant(place - 16);
    player = place >> 3;
    zone = place & 7;
    if (DUEL_CARD_ID(&ZoneOf(player, zone)->card) == 0 || ModEmzSlotOf(player, zone) >= 0)
        return -1;
    return player * MONSTER_ZONES + zone;
}

int ModLinkPointedMonsters(int player, int zone, u8 *out)
{
    u8 places[8];
    int n = ModLinkPointsTo(player, zone, places), i, k = 0;

    for (i = 0; i < n; i++) {
        int occupant = PlaceOccupant(places[i]);

        if (occupant >= 0)
            out[k++] = occupant;
    }
    return k;
}

/* 1 if record (player, zone) is empty and free to take a card. */
static int RecordFree(int player, int zone)
{
    return DUEL_CARD_ID(&ZoneOf(player, zone)->card) == 0 && IsMonsterZoneFree(player, zone) != 0
           && !(gModLink.pendingActive && gModLink.pendingPlayer == (player & 1) && gModLink.pendingZone == zone);
}

/* The records whose Main zone `player`'s Link Monsters point to (only `onlyZone`'s when 0-4; never those in
 * `skip`, which are leaving), as a zone mask; empty or not. */
static u32 PointedZones(int player, int onlyZone, u32 skip)
{
    u32 mask = 0;
    int zone, i;

    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        u8 places[8];
        int n;

        if ((onlyZone >= 0 && onlyZone < MONSTER_ZONES && zone != onlyZone) || (skip & (1 << zone)))
            continue;
        n = ModLinkPointsTo(player, zone, places);
        for (i = 0; i < n; i++) {
            if (!LINK_PLACE_IS_EMZ(places[i]) && (places[i] >> 3) == (player & 1))
                mask |= 1 << (places[i] & 7);
        }
    }
    return mask;
}

u32 ModLinkPointedFreeZones(int player, int onlyZone)
{
    u32 mask = PointedZones(player, onlyZone, 0), free = 0;
    int zone;

    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        if ((mask & (1 << zone)) && RecordFree(player, zone))
            free |= 1 << zone;
    }
    return free;
}

/* The monster record an EMZ of `player` uses: the Main Monster Zone at the same screen edge (zone 1 for the left
 * EMZ, 5 for the right; the opponent's row is mirrored, so theirs are 5 and 1). */
static int EmzRecordOf(int player, int slot)
{
    return (slot == 0) == ((player & 1) == 0) ? 0 : 4;
}

/* That record, if it is free (or one of `leaving`, the materials); else -1. */
static int EmzRecord(int player, int slot, u32 leaving)
{
    int zone = EmzRecordOf(player, slot);

    return RecordFree(player, zone) || (leaving & (1 << zone)) ? zone : -1;
}

/* ---- The zone pick of a summon ---- */

void *ModLinkProxyState(void)
{
    return gModLink.proxy;
}

void ModSummonRestrictZones(u32 mask)
{
    gModLink.restrictMask = mask;
}

void ModSummonSetZone(int zone, int emzSlot)
{
    gModLink.zoneFixed = 1;
    gModLink.fixedZone = zone;
    gModLink.fixedSlot = emzSlot < 0 ? NO_PLACE : emzSlot;
}

/* The summon in gSummonAction goes to place `cand` (0-4 a record, 16 + s an EMZ). */
static void ApplyPlace(struct SummonAction *a, int cand)
{
    if (LINK_PLACE_IS_EMZ(cand)) {
        int zone = EmzRecord(a->player, cand - 16, 0);

        if (zone < 0)
            return;
        a->zone = zone;
        gModLink.pendingActive = 1;
        gModLink.pendingPlayer = a->player;
        gModLink.pendingZone = zone;
        gModLink.pendingSlot = cand - 16;
    } else {
        a->zone = cand;
    }
}

/* The places the summon in gSummonAction may go, into gModLink.cands; their count. */
static int BuildCandidates(struct SummonAction *a)
{
    int player = a->player, zone, s, n = 0;
    u16 id = a->cardId & CARD_ID_MASK;
    int link = IS_LINK_CARD(id);
    int extra = CARD_STATS_KIND(gCardStats[id]) == CARD_KIND_FUSION;
    u32 tributes = 0;

    if (a->kind == SUMMON_ACTION_NORMAL || a->kind == SUMMON_ACTION_NORMAL_CHOOSE_POSITION
        || a->kind == SUMMON_ACTION_SPECIAL_FROM_HAND) {
        if (a->hasTribute1 && a->tribute1Player == player)
            tributes |= 1 << a->tribute1Zone;
        if (a->hasTribute2 && a->tribute2Player == player)
            tributes |= 1 << a->tribute2Zone;
    }
    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        int ok = RecordFree(player, zone) || (tributes & (1 << zone));

        if (ok && link)
            ok = (PointedZones(player, -1, 0) & (1 << zone)) != 0;
        if (ok && gModLink.restrictMask)
            ok = (gModLink.restrictMask & (1 << zone)) != 0;
        if (ok)
            gModLink.cands[n++] = zone;
    }
    if (gModLink.restrictMask) {
        gModLink.restrictMask = 0;
        return n;
    }
    if (extra || link) {
        for (s = 0; s < EMZ_SLOTS; s++) {
            if (!HasEmzMonster(player, 0) && EmzOccupant(s) < 0 && EmzRecord(player, s, 0) >= 0)
                gModLink.cands[n++] = LINK_PLACE_EMZ(s);
        }
    }
    return n;
}

/* Screen position (top-left of the 32x32 zone) of candidate `cand` of `player`. */
static void PlaceScreenXY(int player, int cand, s32 *x, s32 *y)
{
    if (LINK_PLACE_IS_EMZ(cand)) {
        *x = sEmzFieldX[cand - 16];
        *y = EMZ_FIELD_Y - gDuelScreen.scroll;
    } else {
        *x = gDuelZonePositions[player & 1][cand].x;
        *y = gDuelZonePositions[player & 1][cand].y - gDuelScreen.scroll;
    }
}

/* Text box input of the zone pick: the Control Pad steps through the places, A takes the marked one. */
static u16 PickPlaceInput(void)
{
    if (gMain.newKeys & (DPAD_RIGHT | DPAD_DOWN)) {
        gModLink.pickIndex = (gModLink.pickIndex + 1) % gModLink.candCount;
        PlaySE(SE_CURSOR);
    } else if (gMain.newKeys & (DPAD_LEFT | DPAD_UP)) {
        gModLink.pickIndex = (gModLink.pickIndex + gModLink.candCount - 1) % gModLink.candCount;
        PlaySE(SE_CURSOR);
    } else if (gMain.newKeys & A_BUTTON) {
        PlaySE(SE_CONFIRM);
        return 1;
    }
    return 0;
}

/* The CPU's place for a Link Monster: an EMZ if it may take one, else the first pointed zone; others keep theirs. */
static int CpuPlace(struct SummonAction *a)
{
    int i;

    if (!IS_LINK_CARD(a->cardId & CARD_ID_MASK))
        return -1;
    for (i = 0; i < gModLink.candCount; i++) {
        if (LINK_PLACE_IS_EMZ(gModLink.cands[i]))
            return gModLink.cands[i];
    }
    return gModLink.candCount > 0 ? gModLink.cands[0] : -1;
}

u16 ModSummonZoneStep(void)
{
    struct SummonAction *a = &gSummonAction;
    struct ModLink *s = &gModLink;
    int cand;

    if (a->step != 0) {
        s->pickDone = 0;
        return 1;
    }
    if (s->pickDone || a->kind == SUMMON_ACTION_FLIP)
        return 1;
    if (IS_LINK_CARD(a->cardId & CARD_ID_MASK)) {      /* always face-up Attack Position */
        a->isDefense = 0;
        a->isFaceUp = 1;
    }
    if (s->picking) {
        if (gTextBox.active)
            return 0;
        s->picking = 0;
        ApplyPlace(a, s->cands[s->pickIndex]);
        s->pickDone = 1;
        return 1;
    }
    if (s->zoneFixed) {
        s->zoneFixed = 0;
        a->zone = s->fixedZone;
        if (s->fixedSlot != NO_PLACE) {
            s->pendingActive = 1;
            s->pendingPlayer = a->player;
            s->pendingZone = s->fixedZone;
            s->pendingSlot = s->fixedSlot;
        }
        s->pickDone = 1;
        return 1;
    }
    s->pendingActive = 0;           /* a summon that never arrived */
    s->candCount = BuildCandidates(a);
    s->pickDone = 1;
    if (s->candCount == 0)
        return 1;
    if (a->player != 0 || gDuelCtrl.isLinkDuel) {
        cand = CpuPlace(a);
        if (cand >= 0)
            ApplyPlace(a, cand);
        return 1;
    }
    if (s->candCount == 1) {
        ApplyPlace(a, s->cands[0]);
        return 1;
    }
    s->pickDone = 0;
    s->picking = 1;
    s->pickPlayer = a->player;
    s->pickIndex = 0;
    for (cand = 0; cand < s->candCount; cand++) {   /* start on the engine's zone */
        if (s->cands[cand] == a->zone)
            s->pickIndex = cand;
    }
    DuelScreen_ScrollToZone(0, DUEL_AREA_MONSTER);
    TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 5), TEXTBOX_FLAGS_DEFAULT,
                IS_LINK_CARD(a->cardId & CARD_ID_MASK) ? sStrPickPlace : sStrPickZone);
    TextBoxSetMenu(TEXTBOX_MENU_CUSTOM, NULL, PickPlaceInput);
    return 0;
}

u16 ModSummonDefenseChoice(int player, u16 cardId, int faceUp)
{
    if (IS_LINK_CARD(cardId & CARD_ID_MASK))
        return 0;
    return AiShouldSetMonster(cardId, faceUp);
}

/* ---- Drawing ---- */

/* 8x8 arrow tiles (colour ARROW_COLOR): up, left, up-left. */
static const u8 sArrowRows[3][8] = {
    { 0x18, 0x3C, 0x7E, 0xFF, 0x00, 0x00, 0x00, 0x00 },
    { 0x10, 0x30, 0x70, 0xF0, 0xF0, 0x70, 0x30, 0x10 },
    { 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00 },
};

static void BuildArrowTiles(void)
{
    int t, y, x;

    for (t = 0; t < 3; t++) {
        vu32 *dst = (vu32 *)(OBJ_VRAM0 + (TILE_ARROW_UP + t) * 32);

        for (y = 0; y < 8; y++) {
            u32 row = 0;

            for (x = 0; x < 8; x++) {
                if (sArrowRows[t][y] & (0x80 >> x))
                    row |= ARROW_COLOR << (x * 4);
            }
            dst[y] = row;
        }
    }
}

/* The arrow sprite of marker bit `bit` (TL T TR L R BL B BR, as seen on screen) around the zone at (x, y). */
static void AddArrow(int bit, s32 x, s32 y)
{
    static const s8 sOffset[8][2] = {
        { -3, -3 }, { 12, -4 }, { 27, -3 }, { -4, 12 }, { 28, 12 }, { -3, 27 }, { 12, 28 }, { 27, 27 },
    };
    static const u16 sTile[8] = {
        TILE_ARROW_CORNER, TILE_ARROW_UP, TILE_ARROW_CORNER, TILE_ARROW_LEFT,
        TILE_ARROW_LEFT, TILE_ARROW_CORNER, TILE_ARROW_UP, TILE_ARROW_CORNER,
    };
    static const u16 sFlip[8] = { 0, 0, FLIP_H, 0, FLIP_H, FLIP_V, FLIP_V, FLIP_H | FLIP_V };
    s32 sx = x + sOffset[bit][0], sy = y + sOffset[bit][1];

    if (sy < -8 || sy >= 144 || sx < -8 || sx >= 240)
        return;
    AddSpriteFlip(((u32)(sy & 0xFF) << 16) | (sx & 0x1FF), SPRITE_SHAPE_8x8,
                  ARROW_PALETTE << 12 | 1 << 10 | sTile[bit], sFlip[bit]);
}

void ModDrawLinkArrows(void)
{
    int player, zone, bit;

    ((vu16 *)(OBJ_PLTT + ARROW_PALETTE * 32))[ARROW_COLOR] = 0x027F;    /* orange */
    BuildArrowTiles();
    for (player = 0; player <= 1; player++) {
        for (zone = 0; zone < MONSTER_ZONES; zone++) {
            struct DuelZone *z = ZoneOf(player, zone);
            u16 id = DUEL_CARD_ID(&z->card);
            u32 markers;
            s32 x, y;

            if (id == 0 || !z->isFaceUp || !IS_LINK_CARD(id))
                continue;
            markers = LINK_MARKERS(id);
            x = ModZoneFieldX(player, zone);
            y = ModZoneFieldY(player, zone) - gDuelScreen.scroll;
            for (bit = 0; bit < 8; bit++) {
                if (markers & (1 << bit))
                    AddArrow(player ? 7 - bit : bit, x, y);     /* the opponent's are turned around */
            }
        }
    }
    if (gModLink.picking && gModLink.candCount > 0) {           /* the zone pick's marker */
        s32 x, y;

        PlaceScreenXY(gModLink.pickPlayer, gModLink.cands[gModLink.pickIndex], &x, &y);
        AddSprite(((u32)((y + 8) & 0xFF) << 16) | ((x + 8) & 0x1FF), SPRITE_SHAPE_16x16,
                  gZoneMarkerAnimTiles[(gMain.frameCounter >> 3) & 7] + MARKER_TILE_CAN_ATTACK + 0x5400);
    }
}

/* ---- The Link Summon ---- */

static const u16 *RuleOf(u16 id)
{
    u16 number = gCardIdToNumber[id & CARD_ID_MASK];
    u32 i;

    if (!IS_LINK_CARD(id))
        return NULL;
    for (i = 0; sRules[i][0] != 0; i++) {
        if (sRules[i][0] == number)
            return sRules[i];
    }
    return NULL;
}

/* 1 if the face-up monster in (player, zone) fits material rule `rule`. */
static int MaterialFits(int player, int zone, const u16 *rule)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);
    u32 stats;

    if (id == 0 || !z->isFaceUp)
        return 0;
    stats = gCardStats[id & CARD_ID_MASK];
    if (CARD_STATS_TYPE(stats) > CARD_TYPE_REPTILE || IS_TOKEN_NUMBER(gCardIdToNumber[id & CARD_ID_MASK]))
        return rule[3] == LINK_MAT_ANY && CARD_STATS_TYPE(stats) <= CARD_TYPE_REPTILE;   /* a token: "monsters" */
    switch (rule[3]) {
    case LINK_MAT_NORMAL:
        return CARD_STATS_KIND(stats) == CARD_KIND_NORMAL && !IS_EFFECT_CARD_EX(id);
    case LINK_MAT_EFFECT:
        return CARD_STATS_KIND(stats) == CARD_KIND_EFFECT || IS_EFFECT_CARD_EX(id);
    case LINK_MAT_TYPE:
        return GetZoneCardType(player, zone) == rule[4];
    case LINK_MAT_ATTRIBUTE:
        return GetZoneCardAttribute(player, zone) == rule[4];
    case LINK_MAT_LEVEL:
        return !IS_EXTRA_NON_FUSION(id) && CARD_STATS_LEVEL(stats) == rule[4];
    }
    return 1;
}

static u32 FittingMask(int player, const u16 *rule)
{
    u32 mask = 0;
    int zone;

    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        if (MaterialFits(player, zone, rule))
            mask |= 1 << zone;
    }
    return mask;
}

static int CountBits(u32 mask)
{
    int n = 0;

    for (; mask != 0; mask &= mask - 1)
        n++;
    return n;
}

/* 1 if all the materials in `mask` add up to exactly `target` (a Link Monster counts as 1 or as its Link
 * Rating). */
static int ValuesAll(int player, u32 mask, int target)
{
    int zone;

    if (mask == 0)
        return target == 0;
    if (target <= 0)
        return 0;
    for (zone = 0; !(mask & (1 << zone)); zone++)
        ;
    {
        u16 id = DUEL_CARD_ID(&ZoneOf(player, zone)->card);
        u32 rest = mask & ~(1 << zone);

        return ValuesAll(player, rest, target - 1)
               || (IS_LINK_CARD(id) && LINK_RATING(id) > 1 && ValuesAll(player, rest, target - (int)LINK_RATING(id)));
    }
}

/* 1 if some of the materials in `mask` add up to exactly `target` (0: none needed). */
static int ValuesReach(int player, u32 mask, int target)
{
    u32 sub;

    if (target == 0)
        return 1;
    for (sub = mask; sub != 0; sub = (sub - 1) & mask) {
        if (ValuesAll(player, sub, target))
            return 1;
    }
    return 0;
}

/* The places a Link Monster of `player` may go once the materials in `leaving` are gone (into out); their count. */
static int LinkPlaces(int player, u32 leaving, u8 *out)
{
    u32 pointed = PointedZones(player, -1, leaving);
    int zone, s, n = 0;

    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        int empty = RecordFree(player, zone) || (leaving & (1 << zone));

        if (empty && (pointed & (1 << zone)) && ModEmzSlotOf(player, zone) < 0)
            out[n++] = zone;
    }
    if (!HasEmzMonster(player, leaving)) {
        for (s = 0; s < EMZ_SLOTS; s++) {
            int occupant = EmzOccupant(s);

            if ((occupant < 0 || (occupant / MONSTER_ZONES == (player & 1) && (leaving & (1 << (occupant % 5)))))
                && EmzRecord(player, s, leaving) >= 0)
                out[n++] = LINK_PLACE_EMZ(s);
        }
    }
    return n;
}

/* 1 if some set of `player`'s fitting materials adds up to the Link Rating and leaves a place for it. */
/* 1 if the materials in `mask` make a Link Summon of `rule` with Link Rating `rating`: they add up to it, and
 * there are at least the rule's count of them (at most, without "or more"). */
static int MaterialSetFits(int player, u32 mask, const u16 *rule, int rating)
{
    int n = CountBits(mask);

    return n >= rule[1] && (rule[2] || n <= rule[1] || n <= rating) && ValuesAll(player, mask, rating);
}

static int AnyMaterialSet(int player, u16 linkId, const u16 *rule, u32 *found)
{
    u32 fitting = FittingMask(player, rule), sub;
    int rating = LINK_RATING(linkId);
    u8 places[8];

    for (sub = fitting; sub != 0; sub = (sub - 1) & fitting) {
        if (!MaterialSetFits(player, sub, rule, rating) || LinkPlaces(player, sub, places) == 0)
            continue;
        if (found)
            *found = sub;
        return 1;
    }
    return 0;
}

int ModLinkCanSummon(int player, u16 linkId)
{
    const u16 *rule = RuleOf(linkId);

    if (rule == NULL || gDuelCtrl.isLinkDuel || IsCannotSpecialSummon(linkId & CARD_ID_MASK))
        return 0;
    return AnyMaterialSet(player, linkId, rule, NULL);
}

/* Send the materials in `mask` to the Graveyard, take the Link Monster out of the Extra Deck and queue its Special
 * Summon to `place` (a record, or an EMZ). */
static void PerformLinkSummon(int player, u32 link, u32 mask, int place)
{
    u16 cmd = player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK : DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK;
    int zone, record = -1;

    if (LINK_PLACE_IS_EMZ(place)) {
        record = EmzRecord(player, place - 16, mask);
    } else {
        record = place;
    }
    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        if (mask & (1 << zone))
            SendFieldCardToGrave(player, zone, 0, 1);
    }
    gModLink.link = link;
    DuelCmd_Push(cmd, ((u16 *)&gModLink.link)[0], ((u16 *)&gModLink.link)[1], 0);
    gModLink.summoningId = DUEL_CARD_ID((struct DuelCard *)&gModLink.link);
    ModSummonSetZone(record, LINK_PLACE_IS_EMZ(place) ? place - 16 : -1);
    QueueSpecialSummonChoosePosition(player, (struct DuelCard *)&gModLink.link, 1, SUMMON_STATUS);
}

/* "Select Link Materials: 2 left. (B: cancel)" */
static void BuildMaterialPrompt(int left)
{
    StrCopy(gModLink.text, "Select Link Materials: ");
    StrCatNumber(gModLink.text, left);
    StrCat(gModLink.text, " left. (B: cancel)");
}

/* Text box input of the material picks: each must leave the rest reachable (a Link Monster counts as its Link
 * Rating when that still fits, else 1). */
static u16 PickMaterialInput(void)
{
    u16 linkId = DUEL_CARD_ID((struct DuelCard *)&gModLink.link);
    const u16 *rule = RuleOf(linkId);
    int rating = LINK_RATING(linkId), zone, left, value;
    u32 free;
    u16 id;
    u8 places[8];

    if (rule == NULL || (gMain.newKeys & B_BUTTON)) {
        gModLink.cancelled = 1;
        PlaySE(SE_CANCEL);
        return 1;
    }
    if (!DuelCursor_PickTarget(PICK_OWN_FACE_UP))
        return 0;
    zone = gDuelScreen.selIndex;
    free = FittingMask(0, rule) & ~gModLink.materials;
    if (gDuelScreen.selArea != DUEL_AREA_MONSTER || gDuelScreen.selPlayer != 0 || zone >= MONSTER_ZONES
        || !(free & (1 << zone))) {
        PlaySE(SE_ERROR);
        return 0;
    }
    id = DUEL_CARD_ID(&ZoneOf(0, zone)->card);
    left = rating - gModLink.value;
    value = IS_LINK_CARD(id) && (int)LINK_RATING(id) <= left
                    && (left - (int)LINK_RATING(id) == 0
                        || ValuesReach(0, free & ~(1 << zone), left - (int)LINK_RATING(id)))
                ? (int)LINK_RATING(id) : 1;
    if (value > left || (left - value > 0 && !ValuesReach(0, free & ~(1 << zone), left - value))
        || (left - value == 0 && (!MaterialSetFits(0, gModLink.materials | (1 << zone), rule, rating)
                                  || LinkPlaces(0, gModLink.materials | (1 << zone), places) == 0))) {
        PlaySE(SE_ERROR);
        return 0;
    }
    PlaySE(SE_CONFIRM);
    gModLink.materials |= 1 << zone;
    gModLink.value += value;
    return gModLink.value >= rating;
}

int ModLinkMenuRun(u32 link, int start)
{
    struct ModLink *s = &gModLink;
    u16 linkId = DUEL_CARD_ID((struct DuelCard *)&link);
    const u16 *rule;
    u8 places[8];
    int n, i;

    if (gCardListView.active || gTextBox.active)
        return 0;
    rule = RuleOf(linkId);
    if (rule == NULL)
        return 1;
    if (start) {
        s->link = link;
        s->materials = 0;
        s->value = 0;
        s->cancelled = 0;
        s->placing = 0;
        if (!ModLinkCanSummon(0, linkId))
            return 1;
        BuildMaterialPrompt(LINK_RATING(linkId));
        TextBoxOpen(0x206, 0x612, TEXTBOX_FLAGS_DEFAULT, (const u8 *)s->text);
        TextBoxSetMenu(TEXTBOX_MENU_CUSTOM, NULL, PickMaterialInput);
        return 0;
    }
    if (s->cancelled || s->value < LINK_RATING(linkId))
        return 1;
    if (!s->placing) {
        n = LinkPlaces(0, s->materials, places);
        if (n == 0)
            return 1;
        if (n == 1) {
            PerformLinkSummon(0, s->link, s->materials, places[0]);
            return 1;
        }
        /* the place pick, with the zone pick's input (the candidates are this summon's places) */
        for (i = 0; i < n && i < (int)sizeof(s->cands); i++)
            s->cands[i] = places[i];
        s->candCount = i;
        s->pickIndex = 0;
        s->pickPlayer = 0;
        s->picking = 1;
        s->placing = 1;
        TextBoxOpen(TEXTBOX_POS(5, 2), TEXTBOX_SIZE(20, 5), TEXTBOX_FLAGS_DEFAULT, sStrPickPlace);
        TextBoxSetMenu(TEXTBOX_MENU_CUSTOM, NULL, PickPlaceInput);
        return 0;
    }
    s->picking = 0;
    PerformLinkSummon(0, s->link, s->materials, s->cands[s->pickIndex]);
    return 1;
}

/* ---- The CPU ---- */

int AiTryLinkSummon(void)
{
    struct DuelPlayer *p = &gDuelPlayers[1];
    int i, best = -1, bestGain = 0, bestPlace = 0;
    u32 bestMask = 0;

    if (gDuel.turnPlayer != 1 || (gDuel.phase != PHASE_MAIN1 && gDuel.phase != PHASE_MAIN2) || !CanSpecialSummon(1)
        || gDuelCtrl.isLinkDuel)
        return 0;
    for (i = 0; i < p->fusionCount; i++) {
        u16 id = DUEL_CARD_ID(&p->fusionDeck[i]);
        const u16 *rule = RuleOf(id);
        u32 mask;
        u8 places[8];
        int zone, strongest = 0, gain, k;

        if (rule == NULL || IsCannotSpecialSummon(id & CARD_ID_MASK) || !AnyMaterialSet(1, id, rule, &mask))
            continue;
        for (zone = 0; zone < MONSTER_ZONES; zone++) {
            if ((mask & (1 << zone)) && (int)GetZoneCardAtk(1, zone) > strongest)
                strongest = GetZoneCardAtk(1, zone);
        }
        gain = (int)CARD_STATS_ATK(gCardStats[id & CARD_ID_MASK]) - strongest;
        if (gain <= bestGain)
            continue;
        k = LinkPlaces(1, mask, places);
        best = i;
        bestGain = gain;
        bestMask = mask;
        bestPlace = places[0];
        for (zone = 0; zone < k; zone++) {          /* an EMZ first */
            if (LINK_PLACE_IS_EMZ(places[zone]))
                bestPlace = places[zone];
        }
    }
    if (best < 0)
        return 0;
    PerformLinkSummon(1, *(u32 *)&p->fusionDeck[best], bestMask, bestPlace);
    return 1;
}
