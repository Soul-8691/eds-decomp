/*
 * mod_opponents: the Campaign's opponents in this build. Not part of the original ROM.
 * The opponent select keeps its pages and the game's duelist IDs (the records, the decks), but each ID now stands
 * for one of these (sOpponents), and sPageDuelists places them (instead of gOpponentSelectDuelists):
 *   FORMAT  an opponent per format, named after it; the player's deck must be legal in it (BanlistBlocksDuel,
 *           src/mod_banlist.c, with the format's banlist and card pool from src/mod_banlists.c). Only the first is
 *           open at the start; beating format k sWinsToUnlockNext[k] times (10) opens format k + 1. That is
 *           make release; a plain make (testing) opens every format.
 *           They fill pages 0-2 in unlock order.
 *   DECK    page 3: an opponent per deck (src/mod_decks.c), always open: the LR-Chaos Turbo, sdlkilla-Warrior and
 *           geistD-Goat Control decks.
 *   HIDDEN  the duelist IDs not placed, and the empty slots (duelist 0: page 3's last two, page 4): no name, the
 *           stone cover over the portrait, the cursor skips them, never open. Page 4 cannot be reached.
 * Within a page the opponents follow the cursor's ring from the top left (slots 2, 3, 4, 0, 1: top left, top right,
 * right, bottom, left), so each one sits next to the one before it.
 *   ModIsOpponentUnlocked    IsOpponentUnlocked (booster_get_pack.c) asks it first
 *   ModSlotHidden            the opponent select (campaign_select.c): names, cursor
 *   ModPageDuelist           the duelist of a page's slot (campaign_select.c PAGE_DUELIST)
 *   ModOpponentSelectStart   OpponentSelect_Init: the select opens on the newest unlocked format
 *   ModNextPageOpen          the opponent select: pages 0-3 can be visited (locked opponents show covers)
 *   ModOpponentSelectName    the slot names
 *   ModOpponentSelectPortraits  card art drawn into the portraits that have one (DrawArtPortrait)
 *   ModSkipsDuelDialogue     no dialogue before or after a duel with these opponents (campaign_steps.c)
 *   ModOpponentFormat        the banlist index of a FORMAT opponent, or -1
 * A portrait: the card art (72 x 80, 6 bits a pixel, 9 x 10 tiles of 8 x 8, gCardArtGfx) is sampled into the
 * portrait's circle (centre slot position + (31, 33), radius 23.5, measured on the page bitmaps). Its 64 colours map
 * to the nearest colour the page uses; one with none close takes a palette entry the page bitmap leaves unused (page
 * 0 leaves 27, page 4 18; later portraits on a page share the colours of earlier ones). Both Mode 4 frames get it.
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "gba.h"
#include "card_data.h"
#include "save.h"
#include "campaign.h"
#include "constants/cards.h"
#include "constants/game.h"

/* The opponent select's page pictures (campaign_select.c). */
extern struct Mode4Bitmap gOpponentSelectPageBgs[5];

enum OpponentKind {
    KIND_HIDDEN,
    KIND_FORMAT,
    KIND_DECK,
};

/* Banlist indices: the order of BANLISTS in tools/banlist.py (build/include/banlists.h). */
enum {
    BANLIST_YUGI_KAIBA, BANLIST_CRITTER, BANLIST_TREASURE, BANLIST_IMPERIAL, BANLIST_ANDROID,
    BANLIST_JOEY_PEGASUS, BANLIST_FIBER, BANLIST_YATA, BANLIST_SCIENTIST, BANLIST_VAMPIRE, BANLIST_CHAOS,
    BANLIST_WARRIOR, BANLIST_GOAT, BANLIST_CYBER, BANLIST_REAPER,
};

#define FORMAT_COUNT    15

struct ModOpponent {
    u8 kind;                /* enum OpponentKind */
    u8 order;               /* KIND_FORMAT: its place in the unlock order (0 = open from the start) */
    u8 banlist;             /* KIND_FORMAT: its banlist (and card pool) */
    u8 pad;
    const char *name;
    u16 portrait;           /* card number of the portrait's art, 0 = the page's own portrait */
};

static const char sNameYugiKaiba[] = "Yugi-Kaiba";
static const char sNameCritter[] = "Critter";
static const char sNameImperial[] = "Imperial";
static const char sNameTreasure[] = "Treasure";
static const char sNameAndroid[] = "Android";
static const char sNameJoeyPegasus[] = "Joey-Pegasus";
static const char sNameFiber[] = "Fiber";
static const char sNameYata[] = "Yata";
static const char sNameScientist[] = "Scientist";
static const char sNameVampire[] = "Vampire";
static const char sNameChaos[] = "Chaos";
static const char sNameWarrior[] = "Warrior";
static const char sNameGoat[] = "Goat";
static const char sNameCyber[] = "Cyber";
static const char sNameReaper[] = "Reaper";
static const char sNameChaosTurbo[] = "LR-Chaos Turbo";
static const char sNameWarriorDeck[] = "sdlkilla-Warrior";
static const char sNameGoatControl[] = "geistD-Goat Control";

/* Portrait cards that are added cards (config/new_cards.txt): 0 (the page's own portrait) when not in the build. */
#ifdef HAS_CARD_SCAPEGOAT
#define ART_SCAPEGOAT CARD_SCAPEGOAT
#else
#define ART_SCAPEGOAT 0
#endif
#ifdef HAS_CARD_SPIRIT_REAPER
#define ART_SPIRIT_REAPER CARD_SPIRIT_REAPER
#else
#define ART_SPIRIT_REAPER 0
#endif
#ifdef HAS_CARD_VAMPIRE_LORD
#define ART_VAMPIRE_LORD CARD_VAMPIRE_LORD
#else
#define ART_VAMPIRE_LORD 0
#endif
#ifdef HAS_CARD_D_D_WARRIOR_LADY
#define ART_D_D_WARRIOR_LADY CARD_D_D_WARRIOR_LADY
#else
#define ART_D_D_WARRIOR_LADY 0
#endif
#ifdef HAS_CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING
#define ART_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING CARD_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING
#else
#define ART_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING 0
#endif
#ifdef HAS_CARD_THOUSAND_EYES_RESTRICT
#define ART_THOUSAND_EYES_RESTRICT CARD_THOUSAND_EYES_RESTRICT
#else
#define ART_THOUSAND_EYES_RESTRICT 0
#endif
#ifdef HAS_CARD_FIBER_JAR
#define ART_FIBER_JAR CARD_FIBER_JAR
#else
#define ART_FIBER_JAR 0
#endif
#ifdef HAS_CARD_YATA_GARASU
#define ART_YATA_GARASU CARD_YATA_GARASU
#else
#define ART_YATA_GARASU 0
#endif
#ifdef HAS_CARD_MAGICAL_SCIENTIST
#define ART_MAGICAL_SCIENTIST CARD_MAGICAL_SCIENTIST
#else
#define ART_MAGICAL_SCIENTIST 0
#endif
#ifdef HAS_CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END
#define ART_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END CARD_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END
#else
#define ART_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END 0
#endif
#ifdef HAS_CARD_CYBER_DRAGON
#define ART_CYBER_DRAGON CARD_CYBER_DRAGON
#else
#define ART_CYBER_DRAGON 0
#endif

#define FORMAT(order, banlist, name, portrait) { KIND_FORMAT, order, banlist, 0, name, portrait }
#define DECK(name, portrait) { KIND_DECK, 0, 0, 0, name, portrait }
#define HIDDEN { KIND_HIDDEN, 0, 0, 0, NULL, 0 }

/* By enum DuelistId. */
static const struct ModOpponent sOpponents[] = {
    [0] = HIDDEN,
    [DUELIST_YUGI] = HIDDEN,
    [DUELIST_TEA] = HIDDEN,
    [DUELIST_JOEY] = HIDDEN,
    [DUELIST_TRISTAN] = FORMAT(0, BANLIST_YUGI_KAIBA, sNameYugiKaiba, CARD_LA_JINN_THE_MYSTICAL_GENIE_OF_THE_LAMP),
    [DUELIST_BAKURA] = HIDDEN,
    [DUELIST_REX] = FORMAT(1, BANLIST_CRITTER, sNameCritter, CARD_SANGAN),
    [DUELIST_MAKO] = FORMAT(2, BANLIST_IMPERIAL, sNameImperial, CARD_IMPERIAL_ORDER),
    [DUELIST_ESPA_ROBA] = FORMAT(3, BANLIST_TREASURE, sNameTreasure, CARD_CONFISCATION),
    [DUELIST_MAI] = FORMAT(4, BANLIST_ANDROID, sNameAndroid, CARD_JINZO),
    [DUELIST_WEEVIL] = FORMAT(5, BANLIST_JOEY_PEGASUS, sNameJoeyPegasus, CARD_GRACEFUL_CHARITY),
    [DUELIST_RARE_HUNTER] = FORMAT(6, BANLIST_FIBER, sNameFiber, ART_FIBER_JAR),
    [DUELIST_ARKANA] = FORMAT(7, BANLIST_YATA, sNameYata, ART_YATA_GARASU),
    [DUELIST_UMBRA_LUMIS] = FORMAT(8, BANLIST_SCIENTIST, sNameScientist, ART_MAGICAL_SCIENTIST),
    [DUELIST_MARIK] = FORMAT(9, BANLIST_VAMPIRE, sNameVampire, ART_VAMPIRE_LORD),
    [DUELIST_STRINGS] = FORMAT(10, BANLIST_CHAOS, sNameChaos, ART_CHAOS_EMPEROR_DRAGON_ENVOY_OF_THE_END),
    [DUELIST_KAIBA] = FORMAT(11, BANLIST_WARRIOR, sNameWarrior, ART_D_D_WARRIOR_LADY),
    [DUELIST_ISHIZU] = FORMAT(12, BANLIST_GOAT, sNameGoat, ART_SCAPEGOAT),
    [DUELIST_YAMI_BAKURA] = FORMAT(13, BANLIST_CYBER, sNameCyber, ART_CYBER_DRAGON),
    [DUELIST_YAMI_YUGI] = FORMAT(14, BANLIST_REAPER, sNameReaper, ART_SPIRIT_REAPER),
    [DUELIST_SHADI] = HIDDEN,
    [DUELIST_SIMON] = DECK(sNameChaosTurbo, ART_BLACK_LUSTER_SOLDIER_ENVOY_OF_THE_BEGINNING),
    [DUELIST_PEGASUS] = DECK(sNameWarriorDeck, ART_D_D_WARRIOR_LADY),
    [DUELIST_GRANDPA] = DECK(sNameGoatControl, ART_THOUSAND_EYES_RESTRICT),
    [DUELIST_DUEL_COMPUTER] = HIDDEN,
};

/* Wins against format k (in unlock order) that open format k + 1 (make release; with make every format is open). */
static const u8 sWinsToUnlockNext[FORMAT_COUNT - 1] = {
    10,     /* Yugi-Kaiba   -> Critter */
    10,     /* Critter      -> Imperial */
    10,     /* Imperial     -> Treasure */
    10,     /* Treasure     -> Android */
    10,     /* Android      -> Joey-Pegasus */
    10,     /* Joey-Pegasus -> Fiber */
    10,     /* Fiber        -> Yata */
    10,     /* Yata         -> Scientist */
    10,     /* Scientist    -> Vampire */
    10,     /* Vampire      -> Chaos */
    10,     /* Chaos        -> Warrior */
    10,     /* Warrior      -> Goat */
    10,     /* Goat         -> Cyber */
    10,     /* Cyber        -> Reaper */
};

/* The duelist of each [page][slot] (slots: 0 bottom, 1 left, 2 top left, 3 top right, 4 right); 0 = empty. */
#define PAGE_COUNT      4               /* pages that can be visited */
static const u8 sPageDuelists[5][5] = {
    /*  bottom                left                top left               top right              right */
    { DUELIST_ESPA_ROBA,    DUELIST_MAI,        DUELIST_TRISTAN,       DUELIST_REX,           DUELIST_MAKO },
    { DUELIST_UMBRA_LUMIS,  DUELIST_MARIK,      DUELIST_WEEVIL,        DUELIST_RARE_HUNTER,   DUELIST_ARKANA },
    { DUELIST_YAMI_BAKURA,  DUELIST_YAMI_YUGI,  DUELIST_STRINGS,       DUELIST_KAIBA,         DUELIST_ISHIZU },
    { 0,                    0,                  DUELIST_SIMON,         DUELIST_PEGASUS,       DUELIST_GRANDPA },
    { 0,                    0,                  0,                     0,                     0 },
};

u16 ModPageDuelist(int page, int slot)
{
    return sPageDuelists[page][slot];
}

static const struct ModOpponent *Opponent(u16 duelist)
{
    return &sOpponents[duelist < ARRAY_COUNT(sOpponents) ? duelist : 0];
}

/* 1 if opponent `duelist` is open (0 for a hidden one). */
int ModIsOpponentUnlocked(u16 duelist)
{
    const struct ModOpponent *o = Opponent(duelist);
    u32 d;

    if (o->kind == KIND_DECK)
        return 1;
    if (o->kind != KIND_FORMAT)
        return 0;
#if !MOD_RELEASE
    return 1;           /* make (testing): every format open */
#endif
    if (o->order == 0)
        return 1;
    for (d = 0; d < ARRAY_COUNT(sOpponents); d++) {
        if (sOpponents[d].kind == KIND_FORMAT && sOpponents[d].order == o->order - 1)
            return gSaveData.duelRecords[d].wins >= sWinsToUnlockNext[o->order - 1];
    }
    return 0;
}

/* The banlist of the format opponent at `order` in the unlock order, -1 for none. */
int ModFormatBanlist(int order)
{
    u32 d;

    for (d = 0; d < ARRAY_COUNT(sOpponents); d++) {
        if (sOpponents[d].kind == KIND_FORMAT && sOpponents[d].order == order)
            return sOpponents[d].banlist;
    }
    return -1;
}

/* How many formats are unlocked (they unlock in order, so these are orders 0 to the count - 1). */
int ModUnlockedFormatCount(void)
{
    u32 d;
    int k;

    for (k = 0; k < FORMAT_COUNT; k++) {
        for (d = 0; d < ARRAY_COUNT(sOpponents); d++) {
            if (sOpponents[d].kind == KIND_FORMAT && sOpponents[d].order == k)
                break;
        }
        if (d == ARRAY_COUNT(sOpponents) || !ModIsOpponentUnlocked(d))
            break;
    }
    return k;
}

int ModSlotHidden(int page, int slot)
{
    return Opponent(ModPageDuelist(page, slot))->kind == KIND_HIDDEN;
}

int ModNextPageOpen(int page)
{
    return page < PAGE_COUNT - 1;
}

/* Where the opponent select opens: page * 5 + slot of the newest unlocked format. */
int ModOpponentSelectStart(void)
{
    int newest = ModUnlockedFormatCount() - 1;
    int page, slot;

    for (page = 0; page < PAGE_COUNT; page++) {
        for (slot = 0; slot < 5; slot++) {
            const struct ModOpponent *o = Opponent(ModPageDuelist(page, slot));

            if (o->kind == KIND_FORMAT && o->order == newest)
                return page * 5 + slot;
        }
    }
    return 2;           /* page 0, top left */
}

const u8 *ModOpponentSelectName(u16 duelist, const u8 *name)
{
    const struct ModOpponent *o = Opponent(duelist);

    return o->name != NULL ? (const u8 *)o->name : name;
}

int ModSkipsDuelDialogue(u16 duelist)
{
    return Opponent(duelist)->kind != KIND_HIDDEN;
}

int ModOpponentFormat(u16 duelist)
{
    const struct ModOpponent *o = Opponent(duelist);

    return o->kind == KIND_FORMAT ? o->banlist : -1;
}

/* ---- Portraits ---- */

#define PORTRAIT_DX         31
#define PORTRAIT_DY         33
#define PORTRAIT_D2         (47 * 47)       /* (2 * radius)^2, in half pixels */
#define PORTRAIT_SCALE      376             /* art pixels per half screen pixel, / 512 (72 / 49 * 256) */
#define COLOR_NEAR          27              /* squared distance in 5-bit units (about 24 in 8-bit) */

static u32 ColorDist(u16 a, u16 b)
{
    int dr = (a & 31) - (b & 31), dg = (a >> 5 & 31) - (b >> 5 & 31), db = (a >> 10 & 31) - (b >> 10 & 31);

    return dr * dr + dg * dg + db * db;
}

/* Pixel (x, y) of card art id: a 6-bit colour index. */
static u32 ArtPixel(u16 id, int x, int y)
{
    const u8 *src = (const u8 *)(CARD_ART_GFX_ADDR + id * CARD_ART_SIZE);
    u32 i = ((y >> 3) * 9 + (x >> 3)) * 64 + (y & 7) * 8 + (x & 7);
    u32 bit = i * 6;

    return ((src[bit >> 3] | src[(bit >> 3) + 1] << 8) >> (bit & 7)) & 63;
}

/* Draw card art id into the portrait of `slot`; used[] marks the palette entries in use (updated). */
static void DrawArtPortrait(int slot, u16 id, u8 *used)
{
    const u16 *artPal = (const u16 *)(CARD_ART_PALETTES_ADDR + id * CARD_ART_PALETTE_SIZE);
    u16 *pal = (u16 *)PLTT;
    u8 map[64];
    int i, j, x, y, freeIndex = 1;
    int cx = gOpponentSelectSlotPos[slot].x + PORTRAIT_DX, cy = gOpponentSelectSlotPos[slot].y + PORTRAIT_DY;

    for (i = 0; i < 64; i++) {
        u32 best = 0xFFFFFFFF;

        map[i] = 0;
        for (j = 1; j < 256; j++) {
            if (used[j] && ColorDist(artPal[i], pal[j]) < best) {
                best = ColorDist(artPal[i], pal[j]);
                map[i] = j;
            }
        }
        if (best > COLOR_NEAR) {
            while (freeIndex < 256 && used[freeIndex])
                freeIndex++;
            if (freeIndex < 256) {
                pal[freeIndex] = artPal[i];
                used[freeIndex] = 1;
                map[i] = freeIndex;
            }
        }
    }
    for (y = cy - 24; y <= cy + 24; y++) {
        for (x = cx - 24; x <= cx + 24; x++) {
            int dx = 2 * (x - cx) + 1, dy = 2 * (y - cy) + 1;
            int ax, ay;
            u32 offset, c;
            u16 *p0, *p1;

            if (dx * dx + dy * dy > PORTRAIT_D2 || x < 0 || x >= 240 || y < 0 || y >= 160)
                continue;
            ax = 36 + dx * PORTRAIT_SCALE / 512;
            ay = 40 + dy * PORTRAIT_SCALE / 512;
            ax = ax < 0 ? 0 : ax > 71 ? 71 : ax;
            ay = ay < 0 ? 0 : ay > 79 ? 79 : ay;
            c = map[ArtPixel(id, ax, ay)];
            offset = y * 240 + x;
            p0 = (u16 *)(MODE4_FRAME0 + (offset & ~1));
            p1 = (u16 *)(MODE4_FRAME1 + (offset & ~1));
            if (offset & 1) {
                *p0 = (*p0 & 0x00FF) | c << 8;
                *p1 = (*p1 & 0x00FF) | c << 8;
            } else {
                *p0 = (*p0 & 0xFF00) | c;
                *p1 = (*p1 & 0xFF00) | c;
            }
        }
    }
}

/* OpponentSelect_LoadPage, after the page's bitmap and palette: the portraits with card art (open opponents). */
void ModOpponentSelectPortraits(int page)
{
    const u8 *bitmap = gOpponentSelectPageBgs[page].bitmap;
    u8 used[256];
    int slot, i, scanned = 0;

    for (slot = 0; slot < 5; slot++) {
        u16 duelist = ModPageDuelist(page, slot);
        const struct ModOpponent *o = Opponent(duelist);

        if (o->portrait == 0 || !ModIsOpponentUnlocked(duelist))
            continue;
        if (!scanned) {
            for (i = 0; i < 256; i++)
                used[i] = 0;
            for (i = 0; i < 240 * 160; i++)
                used[bitmap[i]] = 1;
            scanned = 1;
        }
        DrawArtPortrait(slot, gCardNumberToId[o->portrait], used);
    }
}
