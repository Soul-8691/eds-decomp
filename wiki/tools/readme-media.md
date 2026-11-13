---
title: README media capture scripts
type: tool
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-13
---

# README media capture scripts

mGBA Lua scripts that take the README's screenshots, GIFs and videos (README_TODO.md lists the slots), so they can be
redone whenever the game changes. They live in `tools/media/` and run through the test runner.

```
powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1 -Media [-Filter name] [-Rom x.gba] [-Save y.sav]
```

## The library: `tools/media/lib_media.lua`

- `M.run(name, fn)` runs `fn` as a coroutine, one resume per frame, so a script reads as a sequence of steps.
- Navigation: `M.toTitle`, `M.toMainMenu`, `M.menuTo(M.MENU.*)` (by `gMainMenuCursor`), `M.openMenu`, `M.press`,
  `M.holdUntil` (holds a key for the game's key repeat), `M.waitCallback("CB_…")`.
- Addresses come from the ROM's own symbol file (`<rom>.sym`, named in `tools/media/_rom.txt` by the runner), so a
  rebuild that moves code does not break the scripts. `M.cardId(name)` reads `gCardNames`.
- Output: `M.shot(name)` writes `screenshots/<name>.png` at 240×160; `M.record(name, "gif"|"mp4")` …
  `M.stop()` writes `screenshots/gifs/<name>.gif` (2×, 30 fps) or `screenshots/videos/<name>.mp4` (3×) with ffmpeg. The
  Lua API has no sound capture: videos with sound need mGBA's Tools > Record A/V.
- Results go to `recordings/_last_run.log` as `PASS`/`FAIL <script>`, which `run_tests.ps1` reads.

## The runner (`run_tests.ps1 -Media`)

- Each capture runs on a **copy** of the ROM and of its save in `tools/testing_scenarios/recordings/_media_rom/`, so
  recipe loads and the like never touch the real save. `-Save` picks the save to copy (e.g. the demo save with the
  release ROM).
- On the separate Win32 desktop like the tests (no windows, full speed).

## Scripts (2026-11-10)

| Script | Output | ROM | Status |
|---|---|---|---|
| `title_screen.lua` | Title Screen.png | `eds_release_demo_extra.gba` (shows the version and Release label) | verified |
| `main_menu_screens.lua` | Settings, Deck Recipes, Banlists, Booster Packs (each with its main menu picture), Pack Opener (Reaper's pack, through `gPackShop`'s restore), Deck Edit (loads the Chaos Turbo recipe, then R to the Main Deck and Down onto Tribe-Infecting Virus) | demo | verified |
| `duelists.lua` | Duelists (Screen 1-3), cursor on each page's first format | demo | verified |
| `duelists_debug.lua` | Duelists (Screen 4), cursor on Debug (top right) | release | verified |
| `password.lua` | Password.png: Breaker's password typed on the keypad (a path over `gPasswordKeypad`'s links), then the "Buy …?" question | demo | verified |
| `*_Media_Player.lua` (from `gen_duel_media.py`) | Card Icons, Card Icons (Up Close), Activate-Set Order, Counters, Duel, Duel Board (cards side by side), Chain List | demo | Duel Board verified 2026-11-13; the rest run before |
| `*_Media_Player.lua` (2026-11-13) | GIFs: synchro, xyz, link, zone-pick, call-trunade, graverobber, hide-battle, hold-r, hold-l. Videos: new-cards, cpu-play, hide-anims, duel-speed-default/always/super | demo | verified (contact sheets) |
| `pre_duel.lua` / `pre_duel_picture.lua` | pre-duel.gif (Weevil's bust-up, page 2 top left, then A) / Pre-duel (Yugi-Kaiba).png (page 1 top left) | demo | verified |
| `debug_duel.lua` | Debug Picker.png, rps.gif (a forced win: the CPU's card set once it answers), mulligan.gif (two Mulligans, then Draw), Debug Duel.png (cursor on Debug's face-up hand) | demo | verified |
| `empty_deck.lua` | Empty Deck.png (loads Chaos Turbo first: a fresh save has no deck) | demo | verified |
| `records_practice.lua` | Records (Practice).png (R three times) | demo | verified |
| `pack_pull.lua` | pack-pull.gif (the list browsed, the Goat pack opened, a card's details) | demo | verified |
| `empty_main_side.lua` / `list_filter.lua` | empty-main-side.gif / list-filter.gif (every card in the trunk; D, Warrior, ATK from 1000: 12 cards) | demo | verified |
| `left_out.lua` | Left Out.png (three Chaos Turbo cards taken out of the trunk, then the recipe loaded: make ignores the banlist when loading) | demo | verified |
| `format_check.lua` | Format Check.png (cards Yugi-Kaiba lacks added to the saved Deck, then its duelist picked) | **release** (make has no format check) | verified |
| `banlist_refused.lua` | Banlist Refused.png (the collection shrunk, then Goat: "only 34 (40 needed)") | **release** (make has no check) | verified |
| `siding.lua` | siding.gif (game 1 won at once, then Bell of Destruction out, Kycoo in) | demo | verified |
| `new_packs.lua` | New Packs.png (one duel counted played and won, the new-packs notice bit set, the match won) | demo | verified |
| `unlock.lua` | unlock.gif (Tristan at 19 wins, the 20th, then Critter's portrait open on the select screen; the reward screens paused out) | **release** (make opens every format) | verified |
| `event_challenger.lua` | Event Challenger.png (a Duel Ceremony day: `gSaveData.days` 12, Saturday 13 January 2001) | demo | verified |
| `stitch_media.py` | duel-speed.mp4 (the three speeds side by side), trailer.mp4 (git-ignored), highlights.gif | ffmpeg | verified |

Duel captures are written as test scenarios (setup and steps, the format of `tools/testing_scenarios/exact.py`) by
`tools/media/gen_duel_media.py`, with media steps: `point`, `open_menu`, `close`, `wait`, `shot`, `gif_start` /
`mp4_start` … `rec_stop`, all in `lib_steps.lua`. Added 2026-11-13:
- `extra_summon <Card> from <materials> -> <m0..m4|emz0|emz1> [atk|def]`: the Extra Deck pile's Special Summon list, the
  material picks (`fieldpick`: the cursor moves with the summon's prompt open), the zone and the position.
- `options speed=… anims=… battle=…` (gSaveData.options, 0x02013D72), `hold R|L|B` / `release …`, `menu <cmd>` (a
  menu the game opened itself), `settle` (until the duel is idle), `unfreeze` (the CPU plays from here; clears its
  skip-turn bit), `await <card>` / `await_resolve <card>` (that card's chain link resolved / started resolving).
- A recording drops the frames of hidden prompts and lists, as the card-test videos do, and its still tail (20
  identical frames kept).

### Capture rules (the user's, 2026-11-13)
- Card effects are recorded like the card-test videos: no chain list, card-show animations, prompts or battle
  scenes (`CLEAN`); a capture shows one only when that is its subject (Synchro/Xyz/Link: lists, prompts and the
  summoned card; Hide Anims: card animations; Hide Battle and the speeds: battle; Hold R: chain and prompts).
- Plays are competitive: attack only weaker monsters; no needless loss of your own cards (the CPU's Giant Trunade,
  not yours; Proxy Dragon from Sangan and Witch of the Black Forest, which search); no monster under 900 ATK in
  Attack Position.
- Cards on a board sit next to each other; a recording starts with the cursor already on the first card used
  (else it crosses empty hand slots that still name the hand the setup replaced); no still tail.

### Harness fixes made for these
- `effectAsksForCard` (lib_steps.lua) replaces `effectRunning` for a summon/Set step's effect-pick test: with the
  chain list shown, `gChain` +0x3D0 stays at `BUILD_WAIT_LIST` after the chain, which made the next summon look
  like an effect pick. Fusion material picks press A once (`once`): the progress signature can't see them.
- `run_tests.ps1 -Media` retries copying the ROM/save copy (the previous mGBA holds it for a moment).

## Lessons
- The captures run on a fresh save (no `eds_demo.sav`): no deck, a trunk of a few cards, 5000 DP. `M.loadRecipe`,
  `M.giveAllCards` set up what a capture needs. The release-demo ROM's recipe list differs (no Chaos Turbo).
- `lib_media.lua` helpers (2026-11-13): `campaignPick(page, slot)` (steered by `gOpponentSelectSlotPos`), `rpsWin`,
  `mulligan`, `toMainPhase`, `pointOpponentHand`, `deckEditCommand`, `refineSet` (List Filter values from
  `gDeckEdit` +0x644; key-count presses get dropped), `giveAllCards`, `callbackName`; recordings trim their still tail.
- Faked progress (2026-11-13): `M.winDuel` (the CPU's LP to 0; Duel_CheckWin ends the duel), a match decided by one
  duel (gMain +0x4888 duel count 1, +0x4889 score 1), `M.giveAllCards` / `takeFromTrunk` / `addToDeck` /
  `shrinkCollection`, notices (`gSaveData` +0x2164), wins (`duelRecords`, +0x20D0), the day (+0x2150). The
  Campaign's steps run inside `CB_Campaign` (gMain +0x4857 step, +0x488A bits 4-11 sub-step); the side swap ends
  with B only once its animation is over (not used: new_packs.lua decides the match in one duel instead).
- The pre-duel screen and rock-paper-scissors are steps inside `CB_Campaign`; Debug asks for both decks (Deck
  Recipes) first.

- Deck Recipes' `gDeckRecipes.cursor` is the absolute slot (+1 scroll, +2 the recipe last loaded); presses closer
  than about 16 frames apart can be missed, so loops re-check after each press.
- The Campaign's duelist select opens on the page last played and needs ~90 frames per page turn (L/R).
- Writing a list position instead of scrolling skips the rows the game draws on the way (wrong names and frames in
  Deck Edit): scroll with the D-pad.
- Do not rebuild a ROM while a test run uses it (mGBA keeps the file open).
