#ifndef GUARD_MOD_MULLIGAN_H
#define GUARD_MOD_MULLIGAN_H

/* Mulligan: shuffle the hand into the Main Deck and draw five again (unlimited). The Main Deck command menu offers
 * **Mulligan** next to **Draw** in your first Draw Phase until you choose **Draw**. */
int ModMulliganAllowedInDuel(void);

/* 1 during the human's first Draw Phase when mulligan is allowed for this duel. */
int ModMulliganAvailable(void);
void ModMulliganFromDeckMenu(void);       /* CARDMENU_CMD_MULLIGAN */
/* LoadDuelUiGfx: the Mulligan icon and label (gModMulliganMenuGfx) over the menu sheets' slot 13. */
void ModLoadMulliganMenuGfx(void);

#endif /* GUARD_MOD_MULLIGAN_H */
