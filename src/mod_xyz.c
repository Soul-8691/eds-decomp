/*
 * mod_xyz: Xyz Monsters and their Xyz Materials. Not part of the original ROM (wiki/game/xyz.md, include/xyz.h).
 * The card data: gCardTypeEx's kind CARD_KIND_EX_XYZ (tools/newcards.py); the stats word says kind FUSION (the Extra
 * Deck) and its Level is the Rank. build/include/xyz_rules.h lists each Xyz Monster's materials.
 *
 * Materials. gModXyz.materials holds every attached card word with the DuelZone.serial of the Xyz Monster it is
 * under. The serial travels with the zone record (DuelCmd_MoveToZone copies it, SwapFieldCards swaps it), so a
 * control change keeps the materials; every way off the field ends in ClearZone (duel_card_lists.c), which calls
 * ModXyzZoneCleared: the materials of that serial go to their owners' Graveyards.
 *
 * The Xyz Summon: the materials (face-up monsters with the rule's Level, type and attribute; not Xyz Monsters or
 * tokens) leave the field (BanishFieldCard's clean-up, without the banished pile) and wait as pending materials; the
 * Xyz Monster is Special Summoned from the Extra Deck, and ModXyzArrived (AddedCardsSummoned) attaches them under its
 * zone's serial. It starts no chain: the usual Special Summon window follows.
 *   the human   the Fusion Deck pile's Special Summon command (ModSynchroAvailable / ModSynchroMenuRun list Synchros
 *               and Xyz Monsters together); for an Xyz pick, ModXyzMenuRun: the materials with the cursor
 *   the CPU     AiTryXyzSummon (AiPlayAddedCards)
 * Like the other mod units, this one is @farcode and @rodata in units.txt and keeps its state in fixed EWRAM
 * (gModXyz, symbols.ld), never in .bss.
 */

#include "global.h"
#include "card_data.h"              /* gCardStats, gCardTypeEx, CARD_STATS_*, IS_XYZ_CARD */
#include "card_list_view.h"         /* CardListView_Open, gCardListView */
#include "constants/card_stats.h"
#include "constants/duel.h"         /* DUEL_AREA_*, ZONE_*, PICK_* */
#include "constants/duel_cmds.h"    /* DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK, DUEL_CMD_PLAYER */
#include "constants/sound.h"        /* SE_* */
#include "duel.h"                   /* gDuel, gDuelPlayers, ClearZone, RemoveLinksToZone, AddCardToGraveyard */
#include "duel_actions.h"           /* DestroyLinkedCards */
#include "duel_cmd.h"               /* DuelCmd_Push */
#include "duel_screen.h"            /* DuelCursor_PickTarget, gDuelScreen, DrawAllAreaTiles */
#include "effect.h"                 /* CollectEffectTargets, DetachRevivingCards */
#include "sound.h"                  /* PlaySE */
#include "summon.h"                 /* QueueSpecialSummonChoosePosition, CanSpecialSummon */
#include "text_box.h"               /* TextBoxOpen, TextBoxSetMenu */
#include "util.h"                   /* StrCopy, StrCat, StrCatNumber, MemClear16 */
#include "gba.h"                    /* B_BUTTON, START_BUTTON */
#include "main.h"                   /* gMain.newKeys */
#include "xyz.h"
#include "xyz_rules.h"              /* XYZ_RULES (build/include, tools/newcards.py) */
#include "link_monster.h"           /* ModLinkZoneCleared */

void BigBangShotLeaves(int player, int zone);   /* src/added_effects.c */

#define MONSTER_ZONES       5
#define MATERIALS_MAX       32
#define VIEW_MAX            12
#define PICK_OWN_FACE_UP    (PICK_FACE_UP_MONSTER | PICK_ATTACK_POSITION | PICK_DEFENSE_POSITION)
#define SUMMON_STATUS       ZONE_STATUS_UNK14   /* as the Ritual and Synchro Summons pass: summoned properly */
#define CARD_WORD_KEEP      0x1FFF              /* a material keeps its ID and owner, no summon flags */
#define IS_TOKEN_NUMBER(no) ((u16)((no) - CARD_NUMBER_TOKEN_FIRST) < CARD_NUMBER_TOKEN_END - CARD_NUMBER_TOKEN_FIRST)

enum MaterialState {
    MATERIAL_FREE = 0,
    MATERIAL_PENDING,               /* taken for the Xyz Summon in progress, not attached yet */
    MATERIAL_ATTACHED,
};

enum XyzStep {
    XYZ_STEP_PICK = 0,              /* the material prompt is open */
    XYZ_STEP_DONE,
};

struct XyzMaterial {
    u16 serial;                     /* the DuelZone.serial of the Xyz Monster it is under */
    u8 state;                       /* enum MaterialState */
    u8 pad;
    u32 card;                       /* the card word (ID and owner) */
};

/* gModXyz (EWRAM 0x0202D500, symbols.ld). */
struct ModXyz {
    struct XyzMaterial materials[MATERIALS_MAX];
    struct DuelCard view[VIEW_MAX]; /* the detach list's cards (ModXyzMaterialView) */
    u8 viewCount;
    u8 step;                        /* enum XyzStep of the human's summon */
    u8 picked;                      /* zone mask of the picked materials */
    u8 pickedCount;
    u8 cancelled;
    u8 pad[3];
    u32 xyz;                        /* the card word of the Xyz Monster being summoned (the human's) */
    u16 summoningId;                /* its card ID until AddedCardsSummoned sees it arrive (0 none) */
    u16 usedTurn[2 * MONSTER_ZONES];        /* [player * 5 + zone]: gDuel.turnCount + 1 of its once-per-turn use */
    u16 usedSerial[2 * MONSTER_ZONES];      /* ... by the monster with this serial */
    char text[96];                  /* the material prompt */
};
extern struct ModXyz gModXyz;
STATIC_ASSERT(sizeof(struct ModXyz) <= 0x200, ModXyzFits);  /* the free EWRAM up to 0x0202D700 */

/* { number, count, or more, Level, type, attribute } */
static const u16 sRules[][6] = XYZ_RULES;

static inline struct DuelZone *ZoneOf(int player, int zone)
{
    return &gDuelPlayers[player & 1].zones[zone];
}

/* ---- Rules and materials ---- */

/* The rule row of Xyz Monster `id`, or NULL. */
static const u16 *RuleOf(u16 id)
{
    u16 number = gCardIdToNumber[id & CARD_ID_MASK];
    u32 i;

    if (!IS_XYZ_CARD(id))
        return NULL;
    for (i = 0; sRules[i][0] != 0; i++) {
        if (sRules[i][0] == number)
            return sRules[i];
    }
    return NULL;
}

/* 1 if the monster in (player, zone) can be an Xyz Material for `rule`. */
static int MaterialFits(int player, int zone, const u16 *rule)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);
    u32 stats;

    if (id == 0 || !z->isFaceUp)
        return 0;
    stats = gCardStats[id & CARD_ID_MASK];
    if (CARD_STATS_TYPE(stats) > CARD_TYPE_REPTILE || IS_XYZ_CARD(id) || IS_LINK_CARD(id)
        || IS_TOKEN_NUMBER(gCardIdToNumber[id & CARD_ID_MASK]))
        return 0;
    if (CARD_STATS_LEVEL(stats) != rule[3])
        return 0;
    if (rule[4] != 0 && GetZoneCardType(player, zone) != rule[4])
        return 0;
    if (rule[5] != 0 && GetZoneCardAttribute(player, zone) != rule[5])
        return 0;
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

int ModXyzCanSummon(int player, u16 xyzId)
{
    const u16 *rule = RuleOf(xyzId);

    if (rule == NULL || IsCannotSpecialSummon(xyzId & CARD_ID_MASK))
        return 0;
    return CountBits(FittingMask(player, rule)) >= rule[1];
}

/* A free entry, or NULL (the table is full: the material is lost to the Graveyard). */
static struct XyzMaterial *FreeEntry(void)
{
    int i;

    for (i = 0; i < MATERIALS_MAX; i++) {
        if (gModXyz.materials[i].state == MATERIAL_FREE)
            return &gModXyz.materials[i];
    }
    return NULL;
}

static void ToGraveyard(struct XyzMaterial *m)
{
    u32 card = m->card;

    AddCardToGraveyard((struct DuelCard *)&card);
    m->state = MATERIAL_FREE;
}

/* Pending materials of a summon that never placed its monster go to the Graveyard. */
static void FlushPending(void)
{
    int i, any = 0;

    for (i = 0; i < MATERIALS_MAX; i++) {
        if (gModXyz.materials[i].state == MATERIAL_PENDING) {
            ToGraveyard(&gModXyz.materials[i]);
            any = 1;
        }
    }
    if (any)
        DrawAllAreaTiles();
}

/* The serial of the Xyz Monster in (player, zone), or -1 for none. */
static int XyzSerial(int player, int zone)
{
    struct DuelZone *z;

    if (zone >= MONSTER_ZONES)
        return -1;
    z = ZoneOf(player, zone);
    if (DUEL_CARD_ID(&z->card) == 0 || !IS_XYZ_CARD(DUEL_CARD_ID(&z->card)))
        return -1;
    return z->serial;
}

int XyzMaterialCount(int player, int zone)
{
    int serial = XyzSerial(player, zone), i, n = 0;

    if (serial < 0)
        return 0;
    for (i = 0; i < MATERIALS_MAX; i++) {
        if (gModXyz.materials[i].state == MATERIAL_ATTACHED && gModXyz.materials[i].serial == serial)
            n++;
    }
    return n;
}

void ModXyzZoneCleared(int player, int zone)
{
    int serial = XyzSerial(player, zone), i;

    ModLinkZoneCleared(player, zone);       /* its Extra Monster Zone is free again */

    if (serial < 0)
        return;
    for (i = 0; i < MATERIALS_MAX; i++) {
        if (gModXyz.materials[i].state == MATERIAL_ATTACHED && gModXyz.materials[i].serial == serial)
            ToGraveyard(&gModXyz.materials[i]);
    }
}

void ModXyzReset(void)
{
    MemClear16(&gModXyz, sizeof(gModXyz));
}

int ModXyzArrived(int player, int zone, u16 id)
{
    int i;

    if (gModXyz.summoningId == 0 || (id & CARD_ID_MASK) != gModXyz.summoningId || zone >= MONSTER_ZONES)
        return 0;
    gModXyz.summoningId = 0;
    for (i = 0; i < MATERIALS_MAX; i++) {
        if (gModXyz.materials[i].state == MATERIAL_PENDING) {
            gModXyz.materials[i].state = MATERIAL_ATTACHED;
            gModXyz.materials[i].serial = ZoneOf(player, zone)->serial;
        }
    }
    return 1;
}

/* ---- Detaching ---- */

/* The attached entries of (player, zone) into the view (ModXyzMaterialView); their count. */
static int BuildView(int player, int zone)
{
    int serial = XyzSerial(player, zone), i;

    gModXyz.viewCount = 0;
    if (serial < 0)
        return 0;
    for (i = 0; i < MATERIALS_MAX && gModXyz.viewCount < VIEW_MAX; i++) {
        struct XyzMaterial *m = &gModXyz.materials[i];

        if (m->state == MATERIAL_ATTACHED && m->serial == serial)
            *(u32 *)&gModXyz.view[gModXyz.viewCount++] = m->card;
    }
    return gModXyz.viewCount;
}

const struct DuelCard *ModXyzMaterialView(int *count)
{
    *count = gModXyz.viewCount;
    return gModXyz.view;
}

/* Detach the first material of (player, zone) whose card ID is `id` (0: the first one). */
static void DetachId(int player, int zone, u16 id)
{
    int serial = XyzSerial(player, zone), i;

    if (serial < 0)
        return;
    for (i = 0; i < MATERIALS_MAX; i++) {
        struct XyzMaterial *m = &gModXyz.materials[i];

        if (m->state == MATERIAL_ATTACHED && m->serial == serial
            && (id == 0 || DUEL_CARD_ID((struct DuelCard *)&m->card) == id)) {
            ToGraveyard(m);
            DrawAllAreaTiles();
            return;
        }
    }
}

int XyzDetachBegin(int player, int zone)
{
    int n = BuildView(player, zone), i, best = 0, same = 1;

    if (n == 0)
        return 0;
    for (i = 1; i < n; i++) {
        if (DUEL_CARD_ID(&gModXyz.view[i]) != DUEL_CARD_ID(&gModXyz.view[0]))
            same = 0;
        if (CARD_STATS_ATK(gCardStats[DUEL_CARD_ID(&gModXyz.view[i])])
            < CARD_STATS_ATK(gCardStats[DUEL_CARD_ID(&gModXyz.view[best])]))
            best = i;
    }
    if (player != 0 || same) {      /* the CPU's weakest, or a choice that cannot matter */
        DetachId(player, zone, DUEL_CARD_ID(&gModXyz.view[player != 0 ? best : 0]));
        return 1;
    }
    if (CollectEffectTargets(0, CARD_LIST_KEY_XYZ_MATERIALS, 0) == 0)
        return 0;
    CardListView_Open(0, CARDLIST_AREA_EFFECT_TARGETS, CARD_LIST_KEY_XYZ_MATERIALS, 0);
    return 2;
}

void XyzDetachPicked(int player, int zone)
{
    DetachId(player, zone, gCardListView.cards[gCardListView.cursorRow + gCardListView.top] & CARD_ID_MASK);
}

/* ---- Once per turn ---- */

int ModXyzEffectUsed(int player, int zone)
{
    int i = (player & 1) * MONSTER_ZONES + zone;

    return zone < MONSTER_ZONES && gModXyz.usedTurn[i] == gDuel.turnCount + 1
           && gModXyz.usedSerial[i] == ZoneOf(player, zone)->serial;
}

void ModXyzMarkEffectUsed(int player, int zone)
{
    int i = (player & 1) * MONSTER_ZONES + zone;

    if (zone >= MONSTER_ZONES)
        return;
    gModXyz.usedTurn[i] = gDuel.turnCount + 1;
    gModXyz.usedSerial[i] = ZoneOf(player, zone)->serial;
}

/* ---- The summon ---- */

/* The materials in `mask` leave the field as pending materials (BanishFieldCard's clean-up of the cards linked to
 * them, without the banished pile), then the Xyz Monster `xyz` leaves the Extra Deck and is Special Summoned
 * (ModXyzArrived attaches the materials when it arrives). */
static void PerformXyzSummon(int player, u32 xyz, u32 mask)
{
    u16 cmd = player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK : DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK;
    int zone;

    FlushPending();
    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        struct DuelZone *z = ZoneOf(player, zone);
        struct XyzMaterial *m;

        if (!(mask & (1 << zone)))
            continue;
        m = FreeEntry();
        if (m != NULL) {
            m->card = *(u32 *)&z->card & CARD_WORD_KEEP;
            m->state = MATERIAL_PENDING;
            m->serial = 0;
        } else {
            u32 card = *(u32 *)&z->card & CARD_WORD_KEEP;

            AddCardToGraveyard((struct DuelCard *)&card);
        }
        BigBangShotLeaves(player, zone);
        DetachRevivingCards(player, zone);
        DestroyLinkedCards(player, zone, 1);
        RemoveLinksToZone(player, zone);
        ClearZone(player, zone);
    }
    DrawAllAreaTiles();
    DuelCmd_Push(cmd, ((u16 *)&xyz)[0], ((u16 *)&xyz)[1], 0);
    gModXyz.summoningId = DUEL_CARD_ID((struct DuelCard *)&xyz);
    gModXyz.xyz = xyz;
    QueueSpecialSummonChoosePosition(player, (struct DuelCard *)&gModXyz.xyz, 1, SUMMON_STATUS);
}

/* "Select 2 Level 4 monsters as Xyz Materials. (B: cancel)" */
static void BuildPrompt(const u16 *rule)
{
    char *t = gModXyz.text;

    StrCopy(t, "Select ");
    StrCatNumber(t, rule[1]);
    if (rule[2])
        StrCat(t, " or more");
    StrCat(t, " Level ");
    StrCatNumber(t, rule[3]);
    StrCat(t, " monsters as Xyz Materials. (B: cancel");
    if (rule[2])
        StrCat(t, ", START: done");
    StrCat(t, ")");
}

/* Text box input of the material picks. */
static u16 PickMaterialInput(void)
{
    const u16 *rule = RuleOf(DUEL_CARD_ID((struct DuelCard *)&gModXyz.xyz));
    u32 fitting;
    int zone;

    if (rule == NULL || (gMain.newKeys & B_BUTTON)) {
        gModXyz.cancelled = 1;
        PlaySE(SE_CANCEL);
        return 1;
    }
    fitting = FittingMask(0, rule);
    if ((gMain.newKeys & START_BUTTON) && rule[2] && gModXyz.pickedCount >= rule[1]) {
        PlaySE(SE_CONFIRM);
        return 1;
    }
    if (!DuelCursor_PickTarget(PICK_OWN_FACE_UP))
        return 0;
    zone = gDuelScreen.selIndex;
    if (gDuelScreen.selArea != DUEL_AREA_MONSTER || gDuelScreen.selPlayer != 0 || zone >= MONSTER_ZONES
        || !(fitting & (1 << zone)) || (gModXyz.picked & (1 << zone))) {
        PlaySE(SE_ERROR);
        return 0;
    }
    PlaySE(SE_CONFIRM);
    gModXyz.picked |= 1 << zone;
    gModXyz.pickedCount++;
    if (rule[2])
        return gModXyz.picked == fitting;           /* "or more": every fitting monster taken */
    return gModXyz.pickedCount >= rule[1];
}

int ModXyzMenuRun(u32 xyz, int start)
{
    struct ModXyz *s = &gModXyz;
    const u16 *rule;
    u32 fitting;

    if (gCardListView.active || gTextBox.active)
        return 0;
    rule = RuleOf(DUEL_CARD_ID((struct DuelCard *)&xyz));
    if (rule == NULL)
        return 1;
    if (start) {
        s->xyz = xyz;
        s->picked = 0;
        s->pickedCount = 0;
        s->cancelled = 0;
        fitting = FittingMask(0, rule);
        if (CountBits(fitting) < rule[1])
            return 1;
        if (!rule[2] && CountBits(fitting) == rule[1]) {
            s->picked = fitting;                    /* no choice to make */
        } else {
            BuildPrompt(rule);
            TextBoxOpen(0x206, 0x612, TEXTBOX_FLAGS_DEFAULT, (const u8 *)s->text);
            TextBoxSetMenu(TEXTBOX_MENU_CUSTOM, NULL, PickMaterialInput);
            s->step = XYZ_STEP_PICK;
            return 0;
        }
    }
    if (s->cancelled || CountBits(s->picked) < rule[1])
        return 1;
    PerformXyzSummon(0, s->xyz, s->picked);
    return 1;
}

/* ---- Look ---- */

const u8 *ModFusionKindFrame(u16 id)
{
    if (IS_SYNCHRO_CARD(id))
        return gCardFrameSynchroGfx;
    if (IS_XYZ_CARD(id))
        return gCardFrameXyzGfx;
    if (IS_LINK_CARD(id))
        return ModLinkFrame(id);            /* with its arrows (src/mod_link.c) */
    return gCardFrameFusionGfx;
}

/* ---- The CPU ---- */

/* The CPU's best Xyz Summon: the Extra Deck card whose ATK beats its strongest material by the most, with the
 * weakest fitting monsters as its materials. 1 if it summoned. */
int AiTryXyzSummon(void)
{
    struct DuelPlayer *p = &gDuelPlayers[1];
    int i, best = -1, bestGain = 0;
    u32 bestMask = 0;

    if (gDuel.turnPlayer != 1 || (gDuel.phase != PHASE_MAIN1 && gDuel.phase != PHASE_MAIN2) || !CanSpecialSummon(1))
        return 0;
    for (i = 0; i < p->fusionCount; i++) {
        u16 id = DUEL_CARD_ID(&p->fusionDeck[i]);
        const u16 *rule = RuleOf(id);
        u32 fitting, mask = 0;
        int n, strongest = 0, gain;

        if (rule == NULL || !ModXyzCanSummon(1, id))
            continue;
        fitting = FittingMask(1, rule);
        for (n = 0; n < rule[1]; n++) {             /* the weakest first */
            int zone, pick = -1;

            for (zone = 0; zone < MONSTER_ZONES; zone++) {
                if ((fitting & ~mask & (1 << zone))
                    && (pick < 0 || GetZoneCardAtk(1, zone) < GetZoneCardAtk(1, pick)))
                    pick = zone;
            }
            mask |= 1 << pick;
            if ((int)GetZoneCardAtk(1, pick) > strongest)
                strongest = GetZoneCardAtk(1, pick);
        }
        gain = (int)CARD_STATS_ATK(gCardStats[id & CARD_ID_MASK]) - strongest;
        if (gain > bestGain) {
            best = i;
            bestGain = gain;
            bestMask = mask;
        }
    }
    if (best < 0)
        return 0;
    PerformXyzSummon(1, *(u32 *)&p->fusionDeck[best], bestMask);
    return 1;
}
