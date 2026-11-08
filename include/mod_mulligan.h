#ifndef GUARD_MOD_MULLIGAN_H
#define GUARD_MOD_MULLIGAN_H

/* Mulligan: shuffle the hand into the Main Deck and draw five again (unlimited). In duels: the Deck's menu offers
 * Mulligan next to Draw in your first Draw Phase, until you draw; on deck-select screens a reshuffle shows a sample
 * hand in the status line. */
int ModMulliganAllowedInDuel(void);
int ModMulliganAllowedOnDeckSelect(void);
void ModMulliganWorkClear(void);

/* 1 while the human may mulligan: ModMulliganAllowedInDuel and the duel's first or second turn (the human's first
 * Draw Phase, whoever starts; the Deck's menu offers it only in the Draw Phase, so drawing ends it). */
int ModMulliganAvailable(void);
void ModMulliganFromDeckMenu(void);       /* CARDMENU_CMD_MULLIGAN */
/* LoadDuelUiGfx: the Mulligan icon and label (gModMulliganMenuGfx) over the menu sheets' slot 13. */
void ModLoadMulliganMenuGfx(void);

/* Deck-select: simulate a five-card opening hand into gModMulligan.text (no text box). */
void ModMulliganDeckSelectRoll(const u16 *deckIds, int deckCount);
void ModMulliganDeckSelectRollStarter(int starterChoice);
const u8 *ModMulliganDeckSelectStatus(void);

#endif /* GUARD_MOD_MULLIGAN_H */
