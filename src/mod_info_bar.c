/*
 * mod_info_bar: the duel's info bar, moved out of duel_info_bar.c. Not part of the original ROM.
 *   DuelScreen_DrawCursorInfo  redraws the two text rows at the bottom of the duel screen for what the field cursor
 *                              is on (campaign.c, card_list_viewer.c, duel_cmd_deck.c and duel_field_view.c call it)
 * It is the original function with one change: the opponent's face-down cards are described too in make (testing),
 * and in a duel with Debug (ModDebugDuel, src/mod_opponents.c). It lives here, as @farcode, because the size-limited
 * code before the pinned data at 0x08080A20 had no room for that check; moving it out also frees that space.
 * Like the other mod units, this one is @farcode and @rodata in units.txt.
 */

#include "global.h"
#include "util.h"                   /* StrLen */
#include "constants/duel.h"         /* enum DuelArea */
#include "duel.h"                   /* gDuelPlayers, gDuelZones, IsHandRevealed */
#include "duel_screen.h"            /* gDuelScreen, DuelInfo_*, TextCellsClear, DuelCursor_GetCardId */
#include "card_data.h"              /* IS_XYZ_CARD */
#include "xyz.h"                    /* XyzMaterialCount */

/* An Xyz Monster's material count: "MAT" right of its name (cells 0x10-0x12: the name ends by 0x0E), the count
 * under it (row 1), clear of the type icons (0x13-0x16). */
#define XYZ_LABEL_CELL          0x10
#define XYZ_COUNT_CELL          0x30
static const u8 sStrXyzMaterials[] = "MAT";
#define LINK_DEF_CELL           0x39
static const u8 sStrNoDef[] = "    -";

/* Info bar labels (rodata_08080A20.s) */
extern const u8 gStrMyFusionDeck[];         /* "My Fusion Deck:" */
extern const u8 gStrOpponentFusionDeck[];   /* "Opponent Fusion Deck" */
extern const u8 gStrMyDeck[];               /* "My Deck:" */
extern const u8 gStrOpponentDeck[];         /* "Opponent Deck" */
extern const u8 gStrMyGraveyard[];          /* "My Graveyard:" */
extern const u8 gStrOpponentGraveyard[];    /* "Opponent Graveyard:" */
extern const u8 gStrMyRemovedCards[];       /* "My Removed Cards:" */
extern const u8 gStrOpponentRemovedCards[]; /* "Opponent Removed Cards:" */

#define INFO_LABEL_GLYPH_WIDTH  10      /* glyph width argument of DuelInfo_DrawLabelNumber */

/* 1 if card zone `zone` of `player` may be described: the human's own, a face-up one, or (make, Debug) any. */
static int ZoneCardShown(u32 player, int zone)
{
    if (player == 0 || !MOD_RELEASE || ModDebugDuel())
        return 1;
    return gDuelZones[player & 1].zones[zone].isFaceUp;
}

/* The pile labels with their counts: the human's pile, or the opponent's (count shown when `opponentCount`). */
static void DrawPileLabel(u32 player, const u8 *mine, const u8 *theirs, u8 count, int opponentCount)
{
    if (player == 0)
        DuelInfo_DrawLabelNumber(INFO_LABEL_GLYPH_WIDTH, mine, count, StrLen((const char *)mine));
    else if (opponentCount)
        DuelInfo_DrawLabelNumber(INFO_LABEL_GLYPH_WIDTH, theirs, count, StrLen((const char *)theirs));
    else
        DuelInfo_DrawLabelNumber(INFO_LABEL_GLYPH_WIDTH, theirs, 0, 0);
}

/*
 * Redraws the info bar for what the field cursor is on: (selPlayer, selArea, selIndex) in gDuelScreen, with selArea an
 * enum DuelArea. A card is described only if it is the human player's own, or the opponent's and face up (in the
 * hand: IsHandRevealed returns 1); otherwise the bar stays blank. The four piles print their label and card count
 * ("My Deck: 35"); the opponent's deck and fusion deck show the label alone, so their sizes stay hidden.
 */
void DuelScreen_DrawCursorInfo(void)
{
    u16 cardId = DuelCursor_GetCardId();
    u32 player = gDuelScreen.selPlayer;
    u32 area = gDuelScreen.selArea;
    u32 index = gDuelScreen.selIndex;
    struct DuelPlayer *p = &gDuelPlayers[player & 1];

    TextCellsClear();
    switch (area) {
    case DUEL_AREA_MONSTER:
        if (cardId && ZoneCardShown(player, index)) {
            DuelInfo_DrawMonsterZone(player, area + index);
            if (IS_LINK_CARD(cardId) && gDuelZones[player & 1].zones[index].isFaceUp) {
                /* a Link Monster has no DEF: "-" over its number (DuelInfo_DrawMonsterZone's cells 0x39-0x3D) */
                TextCellsPutString(LINK_DEF_CELL, sStrNoDef, 7);
                gDuelScreen.textMapReset = 0;
            }
            if (IS_XYZ_CARD(cardId) && gDuelZones[player & 1].zones[index].isFaceUp) {
                TextCellsPutString(XYZ_LABEL_CELL, sStrXyzMaterials, 7);
                TextCellsPutNumber(XYZ_COUNT_CELL, XyzMaterialCount(player, index), 7, 2);
                /* Both set textMapReset, which would reset the cell map after the upload and lose the palettes
                 * the type and attribute icons patched in (TextCellsLoadIcon clears it for that reason). */
                gDuelScreen.textMapReset = 0;
            }
        }
        break;
    case DUEL_AREA_SPELL_TRAP:
        /* selIndex counts within the spell/trap row (zone 5 + selIndex) */
        if (cardId && ZoneCardShown(player, ZONE_SPELL_0 + index))
            DuelInfo_DrawSpellZone(cardId, 1, player, area + index);
        break;
    case DUEL_AREA_FIELD:
        if (cardId && ZoneCardShown(player, ZONE_FIELD))
            DuelInfo_DrawSpellZone(cardId, 1, player, area + index);
        break;
    case DUEL_AREA_HAND:
        if (cardId && (player == 0 || (u16)IsHandRevealed(player)))
            DuelInfo_DrawCard(cardId, 1);
        break;
    case DUEL_AREA_FUSION_DECK:
        DrawPileLabel(player, gStrMyFusionDeck, gStrOpponentFusionDeck, p->fusionCount, 0);
        break;
    case DUEL_AREA_DECK:
        DrawPileLabel(player, gStrMyDeck, gStrOpponentDeck, p->deckCount, 0);
        break;
    case DUEL_AREA_GRAVEYARD:
        DrawPileLabel(player, gStrMyGraveyard, gStrOpponentGraveyard, p->graveCount, 1);
        break;
    case DUEL_AREA_BANISHED:
        DrawPileLabel(player, gStrMyRemovedCards, gStrOpponentRemovedCards, p->banishedCount, 1);
        break;
    }
}
