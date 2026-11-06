/*
 * mod_synchro: Synchro Monsters and Tuners. Not part of the original ROM (wiki/game/synchro.md, include/synchro.h).
 * The card data: gCardTypeEx (include/card_data.h; tools/newcards.py) gives every card ID its extended kind (Synchro),
 * a Tuner bit, an effect bit and the races EDS lacks. A Synchro Monster is kind FUSION in its stats word, so the
 * engine keeps it in the Fusion Deck (the Extra Deck); build/include/synchro_rules.h lists the Synchros and the Tuner
 * each needs (0: any).
 *
 * The Synchro Summon: 1 Tuner and 1 or more non-Tuner monsters, all face up on the summoner's field, whose Levels add
 * up exactly to the Synchro Monster's, go to the Graveyard; the Synchro Monster is Special Summoned from the Extra
 * Deck. It starts no chain: the usual Special Summon window follows (Horn of Heaven, Solemn Judgment, ...).
 *   the human   the Fusion Deck pile's Special Summon command (CardMenu_GetAvailableCommands, ModSynchroAvailable)
 *               runs ModSynchroMenuRun: the card list of the Synchros that can be summoned (CollectEffectTargets with
 *               CARD_LIST_KEY_SYNCHRO), then the Tuner and the non-Tuners on the field with the Ritual star gauge
 *               (DrawRitualStarGauge). Every pick must leave an exact total reachable, so the picks never dead-end.
 *   the CPU     AiTrySynchroSummon (AiPlayAddedCards): the summon that gains the most ATK over its materials.
 * The summoned monster is marked (ModWasSynchroSummoned) by AddedCardsSummoned (src/added_effects.c), which also
 * queues the "If this card is Synchro Summoned" triggers.
 *   ModCardTypeLine  CardDetail_DrawInfo (title_menu.c): the type line of an added monster
 * Like the other mod units, this one is @farcode and @rodata in units.txt and keeps its state in fixed EWRAM
 * (gModSynchro, symbols.ld), never in .bss.
 */

#include "global.h"
#include "card_data.h"              /* gCardStats, gCardTypeEx, CARD_STATS_*, IS_SYNCHRO_CARD, IS_TUNER_CARD */
#include "card_list_view.h"         /* CardListView_Open, gCardListView */
#include "chain.h"                  /* gChain (the star gauge's effectCount / effectSubStep) */
#include "constants/card_stats.h"   /* enum CardType, enum CardKind, enum RaceEx */
#include "constants/duel.h"         /* DUEL_AREA_*, ZONE_* */
#include "constants/duel_cmds.h"    /* DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK, DUEL_CMD_PLAYER */
#include "constants/sound.h"        /* SE_* */
#include "duel.h"                   /* gDuel, gDuelPlayers, struct DuelZone */
#include "duel_actions.h"           /* SendFieldCardToGrave */
#include "duel_cmd.h"               /* DuelCmd_Push */
#include "duel_screen.h"            /* DuelCursor_PickTarget, gDuelScreen */
#include "effect.h"                 /* CollectEffectTargets, DrawRitualStarGauge */
#include "sound.h"                  /* PlaySE */
#include "summon.h"                 /* QueueSpecialSummonChoosePosition, CanSpecialSummon */
#include "synchro.h"
#include "synchro_rules.h"          /* SYNCHRO_RULES (build/include, tools/newcards.py) */
#include "text_box.h"               /* TextBoxOpen, TextBoxSetMenu, gTextBox */
#include "util.h"                   /* StrCopy, StrCat, StrCatNumber, MemClear16 */
#include "gba.h"                    /* B_BUTTON */
#include "main.h"                   /* gMain.newKeys */

extern const char *const gCardTypeNames[];      /* 0x081988D0: by enum CardType */

#define MONSTER_ZONES       5
/* DuelCursor_PickTarget: player 0's face-up monsters. A monster needs a face bit and a position bit to match
 * (DuelCursor_IsValidTarget), so both positions are given. */
#define PICK_OWN_FACE_UP    (PICK_FACE_UP_MONSTER | PICK_ATTACK_POSITION | PICK_DEFENSE_POSITION)
#define SUMMON_STATUS       ZONE_STATUS_UNK14   /* as the Ritual Summon passes: summoned properly */

enum SynchroStep {
    SYNCHRO_STEP_START = 0,
    SYNCHRO_STEP_PICK_SYNCHRO,      /* the card list is open */
    SYNCHRO_STEP_PICK_TUNER,        /* the Tuner prompt is open */
    SYNCHRO_STEP_PICK_MATERIALS,    /* the non-Tuner prompt with the star gauge is open */
    SYNCHRO_STEP_SUMMON,
};

/* gModSynchro (EWRAM 0x0202FE00, symbols.ld). */
struct ModSynchro {
    u8 step;                        /* enum SynchroStep of the human's summon */
    u8 tunerZone;
    u8 materials;                   /* zone mask of the picked non-Tuners */
    u8 needed;                      /* Levels the non-Tuners must add up to */
    u32 synchro;                    /* the card word of the Synchro Monster being summoned */
    u16 summoningId;                /* its card ID until AddedCardsSummoned sees it arrive (0 none) */
    u8 cancelled;                   /* B during the Tuner or non-Tuner pick */
    u8 pad[3];
    char text[128];                 /* the prompt: the Tuner and the Levels still needed */
    u16 summonedIds[2 * MONSTER_ZONES];     /* [player * 5 + zone]: the Synchro Summoned monster there */
};
extern struct ModSynchro gModSynchro;
STATIC_ASSERT(sizeof(struct ModSynchro) <= 0xB0, ModSynchroFits);    /* gModDuelExtra follows at 0x0202FEB0 */

static const u16 sRules[][2] = SYNCHRO_RULES;
#define SYNCHRO_TUNER_TYPE 0x8000      /* a rule's Tuner: 0x8000 | enum CardType ("1 Fiend-Type Tuner") */

static inline struct DuelZone *ZoneOf(int player, int zone)
{
    return &gDuelPlayers[player & 1].zones[zone];
}

static const u8 sStrPickTuner[] = "Select the Tuner monster. (B: cancel)";

/* ---- Levels and materials ---- */

static u32 LevelOf(u16 id)
{
    return CARD_STATS_LEVEL(gCardStats[id & CARD_ID_MASK]);
}

/* The face-up monster in (player, zone), or 0. */
static u16 FaceUpMonster(int player, int zone)
{
    struct DuelZone *z = ZoneOf(player, zone);
    u16 id = DUEL_CARD_ID(&z->card);

    if (id == 0 || !z->isFaceUp || CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK]) > CARD_TYPE_REPTILE)
        return 0;
    return id;
}

/* Zone mask of player's face-up non-Tuner monsters, without `except` (a zone or -1). */
static u32 NonTunerMask(int player, int except)
{
    u32 mask = 0;
    int zone;

    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        u16 id = FaceUpMonster(player, zone);

        if (id != 0 && zone != except && !IS_TUNER_CARD(id))
            mask |= 1 << zone;
    }
    return mask;
}

/* 1 if some non-empty subset of the zones in `mask` has Levels adding up to exactly `target`. */
static int LevelsReachable(int player, u32 mask, int target)
{
    u32 sub;

    if (target <= 0)
        return 0;
    for (sub = mask; sub != 0; sub = (sub - 1) & mask) {
        int zone, sum = 0;

        for (zone = 0; zone < MONSTER_ZONES; zone++) {
            if (sub & (1 << zone))
                sum += LevelOf(FaceUpMonster(player, zone));
        }
        if (sum == target)
            return 1;
    }
    return 0;
}

/* The Tuner card number Synchro `id` needs (0: any), or -1 if it is not a listed Synchro Monster. */
static int RequiredTuner(u16 id)
{
    u16 number = gCardIdToNumber[id & CARD_ID_MASK];
    u32 i;

    for (i = 0; sRules[i][0] != 0; i++) {
        if (sRules[i][0] == number)
            return sRules[i][1];
    }
    return -1;
}

/* 1 if the face-up Tuner in (player, zone) can be the Tuner of Synchro `synchroId` with some non-Tuners. */
static int TunerFits(int player, int zone, u16 synchroId)
{
    u16 tuner = FaceUpMonster(player, zone);
    int rule = RequiredTuner(synchroId);

    if (tuner == 0 || !IS_TUNER_CARD(tuner) || rule < 0)
        return 0;
    if (rule & SYNCHRO_TUNER_TYPE) {
        if (GetZoneCardType(player, zone) != (u32)(rule & ~SYNCHRO_TUNER_TYPE))
            return 0;   /* "1 <Type> Tuner": the Tuner's current type */
    } else if (rule != 0 && gCardIdToNumber[tuner & CARD_ID_MASK] != rule) {
        return 0;
    }
    return LevelsReachable(player, NonTunerMask(player, zone), (int)LevelOf(synchroId) - (int)LevelOf(tuner));
}

/* 1 if `player` can Synchro Summon the Extra Deck card `synchroId` now. */
static int CanSynchroSummon(int player, u16 synchroId)
{
    int zone;

    if (!IS_SYNCHRO_CARD(synchroId))
        return 0;
    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        if (TunerFits(player, zone, synchroId))
            return 1;
    }
    return 0;
}

int ModSynchroListAccepts(u16 id, int player)
{
    return CanSynchroSummon(player, id);
}

int ModSynchroAvailable(int player)
{
    struct DuelPlayer *p = &gDuelPlayers[player & 1];
    int i;

    if (gDuel.turnPlayer != player || (gDuel.phase != PHASE_MAIN1 && gDuel.phase != PHASE_MAIN2)
        || !CanSpecialSummon(player))
        return 0;
    for (i = 0; i < p->fusionCount; i++) {
        if (CanSynchroSummon(player, DUEL_CARD_ID(&p->fusionDeck[i])))
            return 1;
    }
    return 0;
}

/* ---- The summon itself ---- */

/* Send the Tuner and the non-Tuners to the Graveyard, take the Synchro Monster out of the Extra Deck and queue its
 * Special Summon (AddedCardsSummoned marks it when it arrives). */
static void PerformSynchroSummon(int player, int tunerZone, u32 materials)
{
    u16 cmd = player ? DUEL_CMD_PLAYER | DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK : DUEL_CMD_REMOVE_CARD_FROM_FUSION_DECK;
    int zone;

    SendFieldCardToGrave(player, tunerZone, 0, 1);
    for (zone = 0; zone < MONSTER_ZONES; zone++) {
        if (materials & (1 << zone))
            SendFieldCardToGrave(player, zone, 0, 1);
    }
    DuelCmd_Push(cmd, ((u16 *)&gModSynchro.synchro)[0], ((u16 *)&gModSynchro.synchro)[1], 0);
    gModSynchro.summoningId = DUEL_CARD_ID((struct DuelCard *)&gModSynchro.synchro);
    QueueSpecialSummonChoosePosition(player, (struct DuelCard *)&gModSynchro.synchro, 1, SUMMON_STATUS);
}

/* Text box input of the Tuner pick: a face-up Tuner that fits the chosen Synchro Monster. */
static u16 PickTunerInput(void)
{
    u16 synchroId = DUEL_CARD_ID((struct DuelCard *)&gModSynchro.synchro);

    if (gMain.newKeys & B_BUTTON) {
        gModSynchro.cancelled = 1;
        PlaySE(SE_CANCEL);
        return 1;
    }
    if (!DuelCursor_PickTarget(PICK_OWN_FACE_UP))
        return 0;
    if (gDuelScreen.selArea != DUEL_AREA_MONSTER || gDuelScreen.selPlayer != 0
        || !TunerFits(0, gDuelScreen.selIndex, synchroId)) {
        PlaySE(SE_ERROR);
        return 0;
    }
    PlaySE(SE_CONFIRM);
    gModSynchro.tunerZone = gDuelScreen.selIndex;
    return 1;
}

/* Text box input of the non-Tuner picks: each must leave an exact total reachable with the ones still free. The
 * star gauge (DrawRitualStarGauge) shows gChain.effectCount stars, gChain.effectSubStep still to pay. */
static u16 PickMaterialInput(void)
{
    int zone, left;
    u32 free;
    u16 id;

    if (gMain.newKeys & B_BUTTON) {
        gModSynchro.cancelled = 1;
        PlaySE(SE_CANCEL);
        return 1;
    }
    if (!DuelCursor_PickTarget(PICK_OWN_FACE_UP))
        return 0;
    zone = gDuelScreen.selIndex;
    free = NonTunerMask(0, gModSynchro.tunerZone) & ~gModSynchro.materials;
    id = FaceUpMonster(0, zone);
    left = gChain.effectSubStep - (int)LevelOf(id);
    if (gDuelScreen.selArea != DUEL_AREA_MONSTER || gDuelScreen.selPlayer != 0 || !(free & (1 << zone)) || left < 0
        || (left > 0 && !LevelsReachable(0, free & ~(1 << zone), left))) {
        PlaySE(SE_ERROR);
        return 0;
    }
    PlaySE(SE_CONFIRM);
    gModSynchro.materials |= 1 << zone;
    gChain.effectSubStep = left;
    return left == 0;
}

void ModSynchroMenuRun(void)
{
    struct ModSynchro *s = &gModSynchro;
    int zone, tuners = 0, onlyTuner = 0;
    u16 synchroId;

    if (gCardListView.active || gTextBox.active)
        return;
    switch (s->step) {
    case SYNCHRO_STEP_START:
        s->materials = 0;
        s->cancelled = 0;
        if (CollectEffectTargets(0, CARD_LIST_KEY_SYNCHRO, 0) == 0)
            break;
        CardListView_Open(0, CARDLIST_AREA_EFFECT_TARGETS, CARD_LIST_KEY_SYNCHRO, 0);
        s->step = SYNCHRO_STEP_PICK_SYNCHRO;
        return;
    case SYNCHRO_STEP_PICK_SYNCHRO:
        s->synchro = gCardListView.cards[gCardListView.cursorRow + gCardListView.top];
        synchroId = DUEL_CARD_ID((struct DuelCard *)&s->synchro);
        for (zone = 0; zone < MONSTER_ZONES; zone++) {
            if (TunerFits(0, zone, synchroId)) {
                tuners++;
                onlyTuner = zone;
            }
        }
        if (tuners == 0)
            break;
        s->step = SYNCHRO_STEP_PICK_TUNER;
        if (tuners == 1) {
            s->tunerZone = onlyTuner;
        } else {
            TextBoxOpen(0x206, 0x612, TEXTBOX_FLAGS_DEFAULT, sStrPickTuner);
            TextBoxSetMenu(TEXTBOX_MENU_CUSTOM, NULL, PickTunerInput);
            return;
        }
        /* fall through */
    case SYNCHRO_STEP_PICK_TUNER:
        if (s->cancelled)
            break;
        synchroId = DUEL_CARD_ID((struct DuelCard *)&s->synchro);
        s->needed = LevelOf(synchroId) - LevelOf(FaceUpMonster(0, s->tunerZone));
        gChain.effectCount = s->needed;
        gChain.effectSubStep = s->needed;
        /* "Tuner: <name> (Level n). Select non-Tuners with m more Levels. (B: cancel)" */
        StrCopy(s->text, "Tuner: ");
        StrCat(s->text, (const char *)&gCardNames[(FaceUpMonster(0, s->tunerZone) & CARD_ID_MASK) * CARD_NAME_SIZE]);
        StrCat(s->text, " (Level ");
        StrCatNumber(s->text, LevelOf(FaceUpMonster(0, s->tunerZone)));
        StrCat(s->text, "). Select non-Tuners with ");
        StrCatNumber(s->text, s->needed);
        StrCat(s->text, " Levels. (B: cancel)");
        TextBoxOpen(0x206, 0x612, TEXTBOX_FLAGS_DEFAULT, (const u8 *)s->text);
        TextBoxSetMenu(TEXTBOX_MENU_CUSTOM, DrawRitualStarGauge, PickMaterialInput);
        s->step = SYNCHRO_STEP_PICK_MATERIALS;
        return;
    case SYNCHRO_STEP_PICK_MATERIALS:
        if (s->cancelled)
            break;
        PerformSynchroSummon(0, s->tunerZone, s->materials);
        s->step = SYNCHRO_STEP_SUMMON;
        return;
    }
    s->step = SYNCHRO_STEP_START;
    gDuel.cardMenu.confirmed = 0;
}

/* ---- The CPU ---- */

/* The CPU's best Synchro Summon: the Extra Deck card, Tuner and non-Tuners whose summon gains the most ATK over the
 * strongest material (only a gain counts). 1 if it summoned. */
int AiTrySynchroSummon(void)
{
    const int player = 1;
    struct DuelPlayer *p = &gDuelPlayers[player];
    int i, zone, best = -1, bestTuner = 0, bestGain = 0;
    u32 bestMaterials = 0;

    if (!ModSynchroAvailable(player))
        return 0;
    for (i = 0; i < p->fusionCount; i++) {
        u16 synchroId = DUEL_CARD_ID(&p->fusionDeck[i]);
        int atk = CARD_STATS_ATK(gCardStats[synchroId & CARD_ID_MASK]) * CARD_STATS_POINTS_SCALE;

        if (!IS_SYNCHRO_CARD(synchroId))
            continue;
        for (zone = 0; zone < MONSTER_ZONES; zone++) {
            u32 mask, sub;
            int target;

            if (!TunerFits(player, zone, synchroId))
                continue;
            mask = NonTunerMask(player, zone);
            target = (int)LevelOf(synchroId) - (int)LevelOf(FaceUpMonster(player, zone));
            for (sub = mask; sub != 0; sub = (sub - 1) & mask) {
                int z, sum = 0, strongest = CARD_STATS_ATK(gCardStats[FaceUpMonster(player, zone) & CARD_ID_MASK]);

                for (z = 0; z < MONSTER_ZONES; z++) {
                    if (sub & (1 << z)) {
                        u16 m = FaceUpMonster(player, z);

                        sum += LevelOf(m);
                        if ((int)CARD_STATS_ATK(gCardStats[m & CARD_ID_MASK]) > strongest)
                            strongest = CARD_STATS_ATK(gCardStats[m & CARD_ID_MASK]);
                    }
                }
                if (sum == target && atk - strongest * CARD_STATS_POINTS_SCALE > bestGain) {
                    bestGain = atk - strongest * CARD_STATS_POINTS_SCALE;
                    best = i;
                    bestTuner = zone;
                    bestMaterials = sub;
                }
            }
        }
    }
    if (best < 0)
        return 0;
    gModSynchro.synchro = *(u32 *)&p->fusionDeck[best];
    PerformSynchroSummon(player, bestTuner, bestMaterials);
    return 1;
}

/* ---- The Synchro Summoned mark ---- */

void ModSynchroReset(void)
{
    MemClear16(&gModSynchro, sizeof(gModSynchro));
}

u16 ModSynchroSummoningId(void)
{
    return gModSynchro.summoningId;
}

/* AddedCardsSummoned: `id` arrived in (player, zone); 1 if it is the Synchro Monster being Synchro Summoned (now
 * marked). */
int ModSynchroArrived(int player, int zone, u16 id)
{
    if (gModSynchro.summoningId == 0 || (id & CARD_ID_MASK) != gModSynchro.summoningId || zone >= MONSTER_ZONES)
        return 0;
    gModSynchro.summoningId = 0;
    gModSynchro.summonedIds[(player & 1) * MONSTER_ZONES + zone] = id & CARD_ID_MASK;
    return 1;
}

int ModWasSynchroSummonedId(int player, int zone, u16 id)
{
    if (zone >= MONSTER_ZONES || (id & CARD_ID_MASK) == 0)
        return 0;
    return gModSynchro.summonedIds[(player & 1) * MONSTER_ZONES + zone] == (id & CARD_ID_MASK);
}

int ModWasSynchroSummoned(int player, int zone)
{
    u16 id;

    if (zone >= MONSTER_ZONES)
        return 0;
    id = gModSynchro.summonedIds[(player & 1) * MONSTER_ZONES + zone];
    return id != 0 && DUEL_CARD_ID(&ZoneOf(player, zone)->card) == id;
}

/* ---- Card View ---- */

/* The names of enum RaceEx, as YGOPRODeck writes them. */
static const char *const sRaceExNames[RACE_EX_COUNT] = {
    [RACE_EX_PSYCHIC] = "Psychic",
    [RACE_EX_WYRM] = "Wyrm",
    [RACE_EX_CYBERSE] = "Cyberse",
    [RACE_EX_ILLUSION] = "Illusion",
    [RACE_EX_CREATOR_GOD] = "Creator God",
    [RACE_EX_DIVINE_BEAST] = "Divine-Beast",
};

const char *ModCardRaceName(u16 id)
{
    u32 race = CARD_RACE_EX(id);

    if (race != RACE_EX_NONE && race < RACE_EX_COUNT)
        return sRaceExNames[race];
    return gCardTypeNames[CARD_STATS_TYPE(gCardStats[id & CARD_ID_MASK])];
}

/* Card View's type line between the brackets, for an added monster: "Race/Synchro/Tuner/Effect" (the parts that
 * apply). The original cards keep the line CardDetail_DrawInfo built, whose Fusion and Ritual effect checks know
 * only them. `buf` holds "[" and the original's line. */
void ModCardTypeLine(u16 cardId, char *buf)
{
    u32 stats = gCardStats[cardId & CARD_ID_MASK];
    u32 type = CARD_STATS_TYPE(stats);
    u32 kind = CARD_STATS_KIND(stats);

    if ((cardId & CARD_ID_MASK) < CARD_ID_COUNT_ORIGINAL || type > CARD_TYPE_REPTILE)
        return;
    buf[1] = '\0';
    StrCat(buf, ModCardRaceName(cardId));
    if (IS_SYNCHRO_CARD(cardId))
        StrCat(buf, "/Synchro");
    else if (kind == CARD_KIND_FUSION)
        StrCat(buf, "/Fusion");
    else if (kind == CARD_KIND_RITUAL)
        StrCat(buf, "/Ritual");
    if (IS_TUNER_CARD(cardId))
        StrCat(buf, "/Tuner");
    if (kind == CARD_KIND_EFFECT || IS_EFFECT_CARD_EX(cardId))
        StrCat(buf, "/Effect");
}
