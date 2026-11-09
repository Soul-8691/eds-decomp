#ifndef GUARD_MOD_TESTING_AUTOMATION_H
#define GUARD_MOD_TESTING_AUTOMATION_H

#include "global.h"

/* EWRAM byte (symbols.ld): mGBA scenario scripts set this to 1 each frame to freeze the Test Duel CPU. */
extern u8 gModTestingFreezeOpponent;

/* 1 when scripts request a passive opponent in a Test Duel (make demo, not release). */
int ModTestingFreezeOpponentActive(void);

#endif /* GUARD_MOD_TESTING_AUTOMATION_H */
