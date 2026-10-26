---
title: Booster Packs menu (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-26
---
# Booster Packs menu (mod)

The main menu's ninth item, added by this build (`src/mod_packs.c`, `@farcode`/`@rodata`). It opens booster packs
on the game's own Get-a-pack screens. Checked in the source and the build; not tested in an emulator.

- **Menu.** `MAIN_MENU_BOOSTER_PACKS = 8` (`MAIN_MENU_COUNT = 9`; the rows are centred, starting at y = 8).
  - `MainMenu_Launch` calls `PackShop_Start`, then installs `CB_PackShop`.
  - The label "Booster Packs" is built by `BuildMenuLabel` (src/mod_recipes.c; see [[deck-recipes]]) from letters
    of the other labels, with 1-pixel gaps so the 95-pixel text leaves room for the icons (at most 96). It goes to
    OBJ tiles 0x240-0x27F.
- **Scene.** `CB_PackShop` runs `CB_GetPack` (`gGetPackSteps`, as the Campaign rewards and the debug menu do) with
  `gMain.rewardPack = 0`, so step 0 shows the pack list. The list holds only the unlocked packs
  (`PackList_AddUnlockedPacks`, `IsPackUnlocked`). Then come the usual opening, the reveal and card detail, and the
  cards go to the trunk.
- **Added around the steps:**
  - **B on the list** (no slide running) fades out (`FadeToBlack(4)`, as `PackList_FadeOut`) and returns to the
    main menu.
  - **A finished pack** saves the game (`SaveGame`) and restarts the steps at the list, with the pack just opened
    back in the centre (`firstIndex` is set before `PackList_Init` draws the covers).
- **Duelist Points.** Not implemented yet, so every pack is free. The hooks are `PackShop_Cost` (0),
  `PackShop_CanOpen` (refuses A with `SE_ERROR` when it returns 0) and `PackShop_Pay` (empty).
- **RAM.** The state is `gPackShop` (EWRAM `0x0202F000`, symbols.ld).

Related: [[booster-packs]], [[booster-get-pack-c]], [[main-menu]], [[deck-recipes]].
