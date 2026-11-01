#ifndef GUARD_MOD_DUEL_SPEED_H
#define GUARD_MOD_DUEL_SPEED_H

#include "global.h"

/* Duel fast-forward (hold B, gDuelScreen.fast, or Settings speed mode). */
int ModDuelFastForwardActive(void);
/* Extra timer skip when fast-forward is active (7 normally, 14 when Super). */
int ModDuelFastTimerSkip(int base);
/* Animation step jump when fast-forward is active (4 normally, 8 when Super). */
int ModDuelFastAnimAdvance(int oldStep);
void ModDuelApplySpeedSettings(void);
int ModGetSpeedMode(void);
void ModSetSpeedMode(int mode);
/* L on an idle duel frame: start the Settings screen (1 if it was started). */
int ModDuelSettingsHandleInput(void);
/* Runs the in-duel Settings screen; 1 while it owns the frame (DuelMainStep then does nothing else). */
int ModDuelSettingsRun(void);

#endif /* GUARD_MOD_DUEL_SPEED_H */
