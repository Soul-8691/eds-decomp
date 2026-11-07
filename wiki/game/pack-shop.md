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
  (`src/mod_format_packs.c`) lists unlocked packs interleaved by format: each format's TCG sets
  (`packs/*_tcg.json`, release order within the format) then its usage-weighted pack (e.g. LOB, Yugi/Kaiba
  starters, Yugi-Kaiba, Metal Raiders, Critter, …). Usage packs include only cards in that format's
  `usage/<stem>.json` (same universe as [[duelist-points]] / `tools/cardprices.py`), not the full legal pool.
  Pull weights still favor lower-usage cards, but `tools/format_packs.py` flattens the curve (`USAGE_PACK_SCORE_POWER`,
  uniform baseline) so staples are easier to hit and the same low-tier cards repeat less.
  TCG sets unlock when their [[banlists]] format opponent is unlocked (LOB/Yugi/Kaiba starters with Yugi-Kaiba,
  Metal Raiders with Critter, and so on). Usage packs need 10 wins in that format. The next format opponent
  needs 20 campaign wins. Event dates in `packs/formats.json` (`date`) are used at generation time for ordering
  and metadata (`sFormatEventDate[]`). Debug builds (`make`, not `make release`) show every
  pack. Pack covers are rendered from card art at open
  time (`ModFormatPackLoadCoverGfx`, uniform scale to fill 56×112). Then come the usual opening, the reveal and card
  detail, and the cards go to the trunk.
- **Added around the steps:**
  - **B on the list** (no slide running) fades out (`FadeToBlack(4)`, as `PackList_FadeOut`) and returns to the
    main menu.
  - **A finished pack** saves the game (`SaveGame`) and restarts the steps at the list, with the pack just opened
    back in the centre (`firstIndex` is set before `PackList_Init` draws the covers).
- **Duelist Points** ([[duelist-points]]). `GetPackCost` charges **250** DP for TCG set packs and **500** for
  usage-weighted format packs (`FORMAT_PACK_COST_*` in `include/format_packs.h`); vanilla campaign packs stay at
  1000. `PackShop_Cost` uses `GetPackCost`, and
  `PackShop_CanOpen` needs that many DP. A refused A plays `SE_ERROR` and shows "Not enough DP!". `PackShop_Pay`
  takes the DP. The list shows the centre pack's name and price at the top and the player's DP at the bottom.
- **RAM.** The state is `gPackShop` (EWRAM `0x0202F000`, symbols.ld). The list's rows (up to 64, more than
  `gSceneWork.packRows`' 32) and the `PackInfo` that `ModPackInfoAtRow` fills for a format pack are `gModPackList`
  (`0x0202F010`, 0xCC bytes). Until 2026-10-31 they were `static` variables in `.bss`, which the linker places at
  `0x02000000`, on top of `gTextCanvas`. Drawing the list's text then overwrote them, and on the Campaign win list
  (no shop fallback) every pack after the 32nd (Ancient Sanctuary in `make`) showed as Vol.1. A mod unit must not
  have `.bss`; check the map for `.bss` with a size other than `0x0` in `build/src/mod_*.o`.

## The pack of a Campaign win
`ModPackList_AddUnlockedPacks` builds the format-pack list every time a pick-a-pack list opens, not only in the shop.
The vanilla `PackList_AddUnlockedPacks` list is no longer shown. The only lists with `gMain.rewardPack = 0` are the shop's
and the Campaign win's (magazines and event prizes pass a fixed pack and skip the list), so after a won match
`Campaign_GiveRewards` offers the same packs as the shop. Its pick step runs `CB_RewardPack`, which wraps
`CB_GetPack`. The pack is free: there is no price check, and B does nothing. `PackListReward_Draw` (src/mod_dp.c) shows the centre pack's name at the top and
"Free pack   A: open" at the bottom. A win against Solomon or Debug skips the pick (`ModWinGivesPack`). Test Duel
(make builds only) gives one. Built from a clean tree with `make`, `make release`, `make demo` and
`make release-demo` (2026-10-31), not tested in an emulator.

**"New packs" notice (text 351).** After a campaign win, `Campaign_RecordDuelResult` compares
`ModCountUnlockedFormatPacks()` before and after the duel; if more format rows are unlocked (TCG sets when a
format opponent opens at 20 wins against the previous format; usage-weighted packs at 10 wins in that format),
`unlockNotices` bit 1 is set and `Campaign_GiveRewards` shows dialogue 351 before the free pick-a-pack list.

Related: [[booster-packs]], [[booster-get-pack-c]], [[main-menu]], [[deck-recipes]].

## Format pack pulls (2026-11-06)
- `ModGenerateFormatPackCards` (src/mod_format_packs.c) makes five weighted picks straight from the pack's pool
  (`sFormatPackPools`: card number and weight, `tools/format_packs.py`): `RandomBelow(total of the weights)`,
  then `WeightedPick` walks the pool to the card where the running sum passes it, so a card's chance is
  exactly weight / total (as PACKS.md lists). A card already in the pack is re-rolled (up to 64 times).
- It used to read a precomputed 8192-entry roulette per pack (`gFormatPackRouletteBlob`, 800 KB), filled
  by 8192 seeded random draws: 58 of the 3557 pool cards (11 packs, the lowest weights) never landed in
  theirs and could not be pulled, and a pool edited in Hex Maniac Advance changed nothing. Removed with
  `data/mod_format_pack_roulettes.s` and `tools/check_roulette.py`; the pool is now all a pull reads
  ([[hma-toml]]: `data.packs.format.pools`).

## Campaign reward packs (2026-11-06)
- Every pack a Campaign reward opens goes through `GetRewardPack(packId)` (booster_get_pack.c): the event
  rewards (`gMain.rewardPack`: Duelist Pack, Grandpa Cup prize, Rare Selections, Limited Collection, Vol.
  4/5, ...) and the Weekly Yu-Gi-Oh! A/B and magazine deliveries (`Campaign_DeliverMagazines`). It now
  stores `ModRewardPack(packId)` (src/mod_format_packs.c): for an original pack, a random TCG format pack
  (not a usage pack) the player has unlocked (`ModIsFormatPackUnlocked`: its format open; every pack in
  debug builds), picked on the reward's first frame (`seqIndex1` and `seqState1` 0) and kept in
  `gMain.rewardPack` for the frames after. 0 (the free pick from the shop list) and format packs pass
  unchanged; with no TCG pack open the original pack stays (cannot happen in practice: Yugi-Kaiba's
  sets are open from the start). `GeneratePackCards` already sends a format pack ID to
  `ModGenerateFormatPackCards`. The hook is one call in a size-limited unit (`.text` +4 bytes, ends at
  0x0807F494). The original boosters, special packs, their covers and rarity odds are now unused.
