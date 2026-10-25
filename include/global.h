#ifndef GUARD_GLOBAL_H
#define GUARD_GLOBAL_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef u8 bool8;
typedef u32 bool32;

#define TRUE 1
#define FALSE 0
#define NULL ((void *)0)

/* Number of elements of an array object (not of a pointer). */
#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* Byte offset of a struct member as a constant expression (agbcc has no <stddef.h> offsetof). */
#define OFFSET_OF(type, member) ((u32)&((type *)0)->member)

/* File-scope compile-time check: a false condition gives a negative array size, which does not compile.
 * Usage: STATIC_ASSERT(sizeof(struct Main) == 0x488C, MainSize);  (name: unique per translation unit) */
#define STATIC_ASSERT(cond, name) typedef char static_assert_##name[(cond) ? 1 : -1]

/* Pull a not-yet-decompiled function's assembly into a C translation unit, in place.
 * Usage (file scope):  INCLUDE_ASM("asm/nonmatching/bustup_scene", GetDuelistName);  */
#ifdef OBJDIFF_BASE
/* objdiff "base" build (make objdiff-report): leave not-yet-decompiled functions out entirely,
 * so they count as unmatched instead of trivially matching their own assembly. */
#define INCLUDE_ASM(DIR, NAME) extern int __objdiff_skipped_asm
#else
#define INCLUDE_ASM(DIR, NAME) asm(".include \"" DIR "/" #NAME ".s\"")
#endif

asm(".include \"asm/macros.inc\"");

/* Card IDs: 821 in the original game (0..820), plus the cards of config/new_cards.txt, whose count
 * tools/newcards.py writes to build/include/card_count.h. Tools that preprocess without that file (check.py, ...)
 * see the original game. */
#define CARD_ID_COUNT_ORIGINAL 821
#if defined(__has_include)
#if __has_include("card_count.h")
#include "card_count.h"
#endif
#endif
#ifndef CARD_ID_COUNT_ADDED
#define CARD_ID_COUNT_ADDED 0
#endif

/* Addresses of the per-card tables that moved to the end of the ROM to hold the added cards (data/
 * rodata_08087FD0.s). Units that read a table through an integer address (((const u32 *)0x08621DE0)[id])
 * now use these; the externs have their own C names so they never clash with a unit's local declaration. */
extern const u8 gCardDescriptionsAt[] asm("gCardDescriptions");
extern const u8 gCardArtGfxAt[] asm("gCardArtGfx");
extern const u8 gCardArtPalettesAt[] asm("gCardArtPalettes");
extern const u8 gCardStatsAt[] asm("gCardStats");
extern const u8 gCardIdToNumberAt[] asm("gCardIdToNumber");
extern const u8 gCardPasswordsAt[] asm("gCardPasswords");
#define CARD_DESCRIPTIONS_ADDR      ((u32)gCardDescriptionsAt)
#define CARD_ART_GFX_ADDR           ((u32)gCardArtGfxAt)
#define CARD_ART_PALETTES_ADDR      ((u32)gCardArtPalettesAt)
#define CARD_STATS_ADDR             ((u32)gCardStatsAt)
#define CARD_ID_TO_NUMBER_ADDR      ((u32)gCardIdToNumberAt)
#define CARD_PASSWORDS_ADDR         ((u32)gCardPasswordsAt)
/* Every card ID in name order, then 0 (card_data.h); the card-list builders walk it. */
extern const u16 gCardIdsByName[];

#endif /* GUARD_GLOBAL_H */
