---
title: README media capture scripts
type: tool
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-10
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
| `*_Media_Player.lua` (from `gen_duel_media.py`) | Card Icons, Card Icons (Up Close), Activate-Set Order, Counters, Duel, Duel Board, Chain List | demo | **untested**: they need the media step verbs in `lib_steps.lua`, not applied yet |

Duel captures are written as test scenarios (setup and steps, the format of `tools/testing_scenarios/exact.py`) by
`tools/media/gen_duel_media.py`, with media steps: `point`, `open_menu`, `close`, `wait`, `shot`, `gif_start` /
`mp4_start` … `rec_stop`. The step code for these is drafted (scratchpad) but not yet in `lib_steps.lua`.

## Lessons

- Deck Recipes' `gDeckRecipes.cursor` is the absolute slot (+1 scroll, +2 the recipe last loaded); presses closer
  than about 16 frames apart can be missed, so loops re-check after each press.
- The Campaign's duelist select opens on the page last played and needs ~90 frames per page turn (L/R).
- Writing a list position instead of scrolling skips the rows the game draws on the way (wrong names and frames in
  Deck Edit): scroll with the D-pad.
- Do not rebuild a ROM while a test run uses it (mGBA keeps the file open).
