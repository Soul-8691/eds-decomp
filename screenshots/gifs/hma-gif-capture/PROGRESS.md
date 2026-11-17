# HMA.md GIF capture: progress

GIFs of `eds_demo.gba` (eds-decomp) in HexManiacAdvance CE, for the slots in `eds-decomp/HMA.md`.
Finished GIFs are in `eds-decomp/screenshots/gifs/` (named after each placeholder: `hma-<slot>.gif`).

## How they're made
1. HMA CE GUI open on `eds_demo.gba` (with `eds_demo.toml`), MCP server connected in live mode.
2. `capture.ps1 -Dir <frames> -Seconds N` (run in the background first) grabs the HMA window every ~0.3 s with
   PrintWindow (works even when the window is covered).
3. One MCP `run_python` call schedules the steps inside HMA with a WPF DispatcherTimer (`builtins._gif_run`), one
   step every ~1.4 s: select a table row, go to an anchor, switch the left tool (0 Table, 1 Text, 2 Image),
   set the Image tool's SpriteScale / SpritePage / PalettePage, open a grid (Table tool > Open as Grid) and filter it.
4. `makegif.py <frames> <out.gif> --pattern "b*.png" --crop X Y W H --max-width 1280 --min-change 0.0003`
   (env `STABLE=1` drops mid-animation frames) crops, removes repeated frames and writes the GIF.

## Done: all 21
Every placeholder in HMA.md now points at its GIF (use_gifs.py does the swap and the alt text). The two demo imports
(Red Gadget over card 1127, DK-ZERO-Stall Mill into goatchaoscontrol) were undone in HMA afterwards; nothing saved.
The import GIFs composite HMA's dialogs into the frames (capture.ps1); the file dialog is typed into from inside HMA.
The .ydk used is a passwords-only copy (DK-ZERO-Stall Mill.ydk here): HMA's parser skips the decomp's annotated
lines ("31122090 -- Gyakutenno Megami").

## Notes
- Title: graphics.titlescreen.background no longer exists; used graphics.title.text/flame/coin, the medal and
  text.title.version/release. The unused original game logo (garbled with its palette) is left out.
- Ring art: set SpritePage and PalettePage to the card ID (each icon has its own palette row).
- Matched words (unlock wins) aren't in Goto's autocomplete; shown via Goto's folders instead.
