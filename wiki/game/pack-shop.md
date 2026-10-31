---
title: Booster Packs menu (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-31
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
  `gMain.rewardPack = 0`, so step 0 shows the pack list. While the shop is open, `ModPackList_AddUnlockedPacks`
  (`src/mod_format_packs.c`)   lists unlocked packs interleaved by format: each format's TCG sets (`packs/*_tcg.json`, release
  order within the format) then its usage-weighted pack (e.g. LOB, Yugi/Kaiba starters, Yugi-Kaiba, Metal Raiders, Critter, …).   TCG sets unlock when their [[banlists]] format opponent is unlocked (LOB/Yugi/Kaiba starters with Yugi-Kaiba,
  Metal Raiders with Critter, and so on). Usage packs need 10 wins in that format. The next format opponent
  needs 20 campaign wins. Event dates in `packs/formats.json` (`date`) are used at generation time for ordering
  and metadata (`sFormatEventDate[]`). Debug builds (`make`, not `make release`) show every
  pack. Campaign reward flows still use the vanilla list. Pack covers are rendered from card art at open
  time (`ModFormatPackLoadCoverGfx`, uniform scale to fill 56×112). Then come the usual opening, the reveal and card
  detail, and the cards go to the trunk.
- **Added around the steps:**
  - **B on the list** (no slide running) fades out (`FadeToBlack(4)`, as `PackList_FadeOut`) and returns to the
    main menu.
  - **A finished pack** saves the game (`SaveGame`) and restarts the steps at the list, with the pack just opened
    back in the centre (`firstIndex` is set before `PackList_Init` draws the covers).
- **Duelist Points** ([[duelist-points]]). `PackShop_Cost` is `GetPackCost` (1000 for every pack for now), and
  `PackShop_CanOpen` needs that many DP. A refused A plays `SE_ERROR` and shows "Not enough DP!". `PackShop_Pay`
  takes the DP. The list shows the centre pack's name and price at the top and the player's DP at the bottom.
- **RAM.** The state is `gPackShop` (EWRAM `0x0202F000`, symbols.ld).

Related: [[booster-packs]], [[booster-get-pack-c]], [[main-menu]], [[deck-recipes]].
