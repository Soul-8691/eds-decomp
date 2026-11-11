/*
 * mod_deck_edit: Deck Edit command-bar tweaks. Not part of the original ROM.
 *   ModDeckEditEmptyMainAndSide  return every main-deck, fusion-deck and side-deck copy to the trunk
 *   gModDeckEditEmptyMainSideLabelGfx  command-bar tiles (build/assets/mod_deck_edit_empty_label.bin)
 */

#include "global.h"
#include "card_data.h"
#include "constants/sound.h"
#include "deck_edit.h"
#include "gba.h"
#include "save.h"
#include "sound.h"

void RemoveCardFromSavedDeck(u16 id);
void RemoveCardFromSavedSideDeck(u16 id);
void RemoveCardFromSavedFusionDeck(u16 id);
void AddCardToTrunk(u16 id);
void DeckEdit_BuildCardLists(void);
void DeckEdit_CountSideDeckMonsters(void);
void ListRefine_Reset(void);

static void ReturnDeckCopiesToTrunk(u16 id, u32 (*getCopies)(u16), void (*remove)(u16))
{
    while (getCopies(id) != 0) {
        remove(id);
        AddCardToTrunk(id);
    }
}

void ModDeckEditEmptyMainAndSide(void)
{
    const u16 *name;
    u8 list;

    for (name = gCardIdsByName; *name != 0; name++) {
        u16 id = *name;

        ReturnDeckCopiesToTrunk(id, GetTrunkDeckCopies, RemoveCardFromSavedDeck);
        ReturnDeckCopiesToTrunk(id, GetTrunkFusionCopies, RemoveCardFromSavedFusionDeck);
        ReturnDeckCopiesToTrunk(id, GetTrunkSideCopies, RemoveCardFromSavedSideDeck);
    }
    for (list = 0; list <= 2; list++) {
        gDeckEdit.listRow[list] = 0;
        gDeckEdit.listPos[list] = 0;
        gDeckEdit.commandMenu.filter[list] = 0;
        gDeckEdit.commandMenu.sort[list] = 0;
    }
    ListRefine_Reset();
    DeckEdit_BuildCardLists();
    DeckEdit_CountSideDeckMonsters();
    if (gDeckEdit.listCount[0][gDeckEdit.curList] == 0)
        gDeckEdit.curList = DECKEDIT_LIST_TRUNK;
    gDeckEdit.commandMenu.anim = CMDMENU_REFRESH;
    /* Redraw the list as switching lists does (R): the big card art, the card frames beside the list (else they
     * kept showing the emptied cards), the rows and the scroll bar; then the list's label (it may now be the
     * trunk). */
    DeckEdit_StartListSlide(2);
    DeckEdit_DrawStatementLabels(gDeckEdit.curList);
    PlaySE(SE_CONFIRM);
}
