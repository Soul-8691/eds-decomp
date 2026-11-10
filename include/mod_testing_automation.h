#ifndef GUARD_MOD_TESTING_AUTOMATION_H
#define GUARD_MOD_TESTING_AUTOMATION_H

#include "global.h"

/* EWRAM byte (symbols.ld): mGBA scenario scripts set this to 1 each frame to freeze the Test Duel CPU. */
extern u8 gModTestingFreezeOpponent;

/* 1 when scripts request a passive opponent in a Test Duel (make demo, not release): bit 0 of the byte. */
int ModTestingFreezeOpponentActive(void);
/* 1 when the CPU holds as many Set Spell/Traps as scripts allow (bits 4-7 of the byte; 0 = no limit). */
int ModTestingCpuSetLimitReached(void);
/* Every duel frame: a script's redraw request (bit 1) redraws the field once (DrawAllAreaTiles). */
void ModTestingTick(void);

#endif /* GUARD_MOD_TESTING_AUTOMATION_H */
