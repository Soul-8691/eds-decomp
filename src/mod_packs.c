/*
 * mod_packs: Booster Packs, the main menu's ninth item. Not part of the original ROM.
 *   PackShop_BuildLabel   the "Booster Packs" menu label (MainMenu_Init, tiles 0x240-0x27F), put together by
 *                         BuildMenuLabel (src/mod_recipes.c) from the letters of the other labels
 *   PackShop_Start        MainMenu_Launch, before it installs CB_PackShop
 *   CB_PackShop           the scene: the game's own Get-a-pack steps (gGetPackSteps through CB_GetPack, as the
 *                         Campaign's rewards and the debug menu run them): the list of the unlocked packs
 *                         (PackList_AddUnlockedPacks), then the opening and the five-card reveal, the cards going
 *                         to the trunk. Added around them: B on the list goes back to the main menu, a finished
 *                         pack comes back to the list (on the pack just opened), and the pack price check.
 * Duelist Points (DP) are not implemented yet, so every pack is free: PackShop_CanOpen always allows it and
 * PackShop_Pay takes nothing. DP will go there (PackShop_Cost gives the price).
 * Like the other mod units, this one is @farcode and @rodata in units.txt and keeps its state at a fixed EWRAM
 * address (gPackShop, symbols.ld), never in .bss.
 */

#include "global.h"
#include "gba.h"
#include "main.h"
#include "palette.h"
#include "sound.h"
#include "booster.h"
#include "save.h"
#include "constants/sound.h"

/* src/mod_recipes.c */
struct LabelChunk {
    u8 item;
    u8 x0;
    u8 x1;
};
void BuildMenuLabel(const struct LabelChunk *chunks, int count, u16 *dst);

/* The pack list's work area (booster_pack.c views gSceneWork the same way). */
extern struct PackListWork gSceneWork;

/* The scene's state: gPackShop (EWRAM 0x0202F000, symbols.ld). */
struct PackShop {
    u8 started;
    u8 leaving;             /* B on the list: fading out to the main menu */
    u8 restore;             /* put lastPackId back in the centre when the list comes back */
    u8 pad;
    u16 lastPackId;
};
extern struct PackShop gPackShop;

/* "Booster Packs": B of Link Battle, o of Record (twice), s of Password, t of Battle, e of Deck, r of Record, a
 * space of Deck Edit, Pa of Password, ck of Deck, s of Password. Each letter keeps a 1-pixel gap after it (the
 * sheet has 1-2) to stay within the 96 pixels the icons allow. */
static const struct LabelChunk sLabelChunks[] = {
    { 1, 59, 67 },
    { 3, 66, 73 },
    { 3, 66, 73 },
    { 6, 50, 56 },
    { 1, 77, 83 },
    { 2, 40, 47 },
    { 3, 75, 80 },
    { 2, 63, 65 },
    { 6, 33, 49 },
    { 2, 48, 63 },
    { 6, 50, 55 },
};

void PackShop_BuildLabel(u16 *dst)
{
    BuildMenuLabel(sLabelChunks, ARRAY_COUNT(sLabelChunks), dst);
}

/* ---- Duelist Points (not implemented yet: every pack is free) ---- */

/* The DP price of a pack. */
static u32 PackShop_Cost(u16 packId)
{
    return 0;
}

/* 1 if the player may open the pack (DP: can pay PackShop_Cost). */
static int PackShop_CanOpen(u16 packId)
{
    return PackShop_Cost(packId) == 0;
}

/* Pay for the pack the player opens (DP: take PackShop_Cost). */
static void PackShop_Pay(u16 packId)
{
}

/* ---- The scene ---- */

void PackShop_Start(void)
{
    gPackShop.started = 0;
}

/* The pack in the list's centre slot. */
static u16 CentrePackId(void)
{
    return gPackInfo[gSceneWork.packRows[(gSceneWork.firstIndex + gSceneWork.centerSlot) % gSceneWork.packCount]].id;
}

/*
 * Runs CB_GetPack, with these steps of its own:
 *   - the first frame: no reward pack (the list is shown), the menu music;
 *   - the list's first frame after its set-up (PackList_Init state 1, before the covers are drawn): the pack opened
 *     last goes back in the centre;
 *   - the list waiting for input (step GETPACK_STEP_SELECT, sub-state 2, no slide running): B fades out and goes
 *     back to the main menu; A on a pack the player cannot open is refused, else it is paid for;
 *   - CB_GetPack done (the cards are in the trunk): SaveGame, and the steps start over at the list.
 * Returns 1 when the player leaves (MainLoop goes back to the main menu, which saves the game).
 */
u16 CB_PackShop(void)
{
    struct PackShop *s = &gPackShop;

    if (!s->started) {
        s->started = 1;
        s->leaving = 0;
        s->restore = 0;
        gMain.rewardPack = 0;
        PlayBGMNoTrack(SONG_MAIN_MENU);
    }
    if (s->leaving) {
        if (FadeToBlack(4)) {
            REG_DISPCNT &= ~(DISPCNT_BG0_ON | DISPCNT_OBJ_ON);
            FadeOutBGM();
            return 1;
        }
        return 0;
    }
    if (gMain.seqIndex1 == GETPACK_STEP_SELECT) {
        if (gMain.seqState1 == 1 && gSceneWork.state == 1 && s->restore) {
            int r;

            s->restore = 0;
            for (r = 0; r < gSceneWork.packCount; r++) {
                if (gPackInfo[gSceneWork.packRows[r]].id == s->lastPackId) {
                    gSceneWork.firstIndex = (r + gSceneWork.packCount - gSceneWork.centerSlot) % gSceneWork.packCount;
                    break;
                }
            }
        }
        if (gMain.seqState1 == 2 && gSceneWork.slideFrame == 0 && !(gSceneWork.flags & 1)) {
            if (gMain.newKeys & B_BUTTON) {
                PlaySE(SE_CANCEL);
                s->leaving = 1;
                return 0;
            }
            if (gMain.newKeys & A_BUTTON) {
                u16 packId = CentrePackId();

                if (!PackShop_CanOpen(packId)) {
                    PlaySE(SE_ERROR);
                    gMain.newKeys &= ~A_BUTTON;
                } else {
                    PackShop_Pay(packId);
                    s->lastPackId = packId;
                    s->restore = 1;
                }
            }
        }
    }
    if (CB_GetPack()) {
        SaveGame();         /* the cards are in the trunk; the scene does not change, so nothing else saves */
        gMain.seqIndex1 = GETPACK_STEP_SELECT;
        gMain.seqState1 = 0;
        gMain.seqState2 = 0;
    }
    return 0;
}
