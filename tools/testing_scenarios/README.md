# TESTING.md mGBA scenario scripts

One Lua script per human (**Player**) or AI (**CPU**) scenario in repo-root [`TESTING.md`](../../TESTING.md). Each run targets **Test Duel** vs an empty opponent (`make demo`, `MOD_TEST_DECK`).

## Recording file names

Per [mGBA’s scripting API](https://mgba.io/docs/dev/scripting.html) (dev build docs mirror 0.10+), **`emu` has no method to start Tools → Record A/V**. The documented video-related calls are:

| API | Role |
|-----|------|
| `emu:screenshot(path)` | PNG of the current framebuffer |
| `emu:screenshotToImage()` | Same as an `Image` you can `save()` as PNG |

There is no Lua binding for MPEG-4/GIF/WebP recording (that path is Qt + ffmpeg inside the GUI). GitHub [#3066](https://github.com/mgba-emu/mgba/issues/3066) confirms no CLI flag either.

**Automated MP4 in this repo (default):** `lib_eds.lua` captures a PNG sequence with `emu:screenshot()` from **in_duel** onward, then runs **ffmpeg** on `Finished` to write the target `.mp4`. Requires `ffmpeg` on `PATH`. Disable with `captureMp4 = false` in script opts. Probe your build with:

```text
mGBA.exe --script tools/testing_scenarios/probe_mgba_recording.lua eds_demo.gba
```

**Manual MP4 (mGBA encoder):** still works if you prefer the built-in A/V dialog and do not use script capture.

Every scenario writes the expected MP4 basename to the scripting console and to `recordings/_next_recording.txt`:

```text
{card name}_{effect name}_{Player|CPU}.mp4
```

Examples:

- `Card_Destruction_Discard_And_Draw_Player.mp4`
- `Soul_Exchange_Tribute_Opponent_Monster_Player.mp4`
- `Torrential_Tribute_Chain_Window_CPU.mp4`

Spaces and punctuation are folded to underscores; each effect word is title-cased (`EDS.slugEffect` in `lib_eds.lua`, `effect_slug` in `gen_testing_lua.py`).

With **ffmpeg on PATH** (default), you only need to run the POC script; the `.mp4` should appear under `recordings/` when the console prints `MP4: wrote ...`. PNG staging dirs are `recordings/<basename>_frames/` (wiped at the start of each run, then kept after encode; `keepPngFrames = false` only skips retaining them—still cleared on the next run).

Without ffmpeg, use mGBA’s GUI: **Tools → Record A/V** → **MPEG-4**, set the path from `_next_recording.txt`, **Start** before the duel, **Stop** after **Finished**.

Format **main decks** (40 cards) load as soon as the duel starts (**before** your Draw step), from `usage/<format>.json` via `format_decks.lua`. When the usage deck is loaded into RAM, the script **clears both hands** and **shuffles each main deck once** (`p0ShuffledOnce` / `p1ShuffledOnce`). By default you confirm **Draw** on your deck (Mulligan → Draw → **A**). Optional `ramOpeningDraw = true` sets **`skipDrawPhase`** and **RAM-draws 5** instead (no deck menu). **Scenario hand/field RAM** (`setupCards`) runs only **after** you confirm **Draw** (or after `ramOpeningDraw` RAM-deal), still during Draw/Standby before Main 1. **Recording** waits until `scenarioSetupDone`, skips `draw_mulligan`, and holds **3 frames** so the first PNG matches the scenario hand. If **you go first**, load+shuffle wait until **Turn Start** (`duel_step` 2). Pass `format` from TESTING.md; disable with `usageDeck = false`.

Capture ends after the scripted `play` steps finish and the duel UI is idle (~**20** frames, `playSettleFrames`; no extra `recordTail` by default). Play steps may include `{ cursor = "opp_monster", zone = 0 }` or `{ cursor = "own_hand", slot = N }` before **A** presses.

Opponent **hand** cards are written in RAM for effects like **Card Destruction**, but Test Duel does **not** show the CPU’s hand on screen (unlike Debug duels).

By default, **Player** scripts set **`freezeOpponent`** (passive CPU): the opponent does not play its own turn and does not chain or quick-activate in response to your cards. Lua sets `gModTestingFreezeOpponent` (`0x0202FF03`) each frame; **`eds_demo.gba`** must be rebuilt after pulling this hook (`make -j$(nproc) demo`). Use `freezeOpponent = false` for **CPU** watch scenarios.

Use **Tools → Settings → Emulation → Fast forward** at max, or launch with:

```text
cd path\to\eds-decomp
mGBA.exe -6 -C audioSync=0 -C videoSync=0 -C fpsTarget=6000 -C scaleMultiplier=6 ^
    --script tools/testing_scenarios/poc/<script>.lua eds_demo.gba
```

`-6` sets the **initial window** to 6× (1440×960 for GBA). Script capture stores **native-sized PNGs** (usually 240×160); **ffmpeg** upscales to **1440×960** with nearest-neighbor (`captureScale`, default **6**). Lower with `captureScale = 2` in script opts if you want a smaller MP4. OpenGL **High-resolution scale** in mGBA can make PNGs larger already—the encoder still targets `240×scale` by `160×scale`.

Scripts resolve `lib_eds.lua` from the `.lua` file path (`debug.getinfo`), not from `script.dir` (mGBA often sets that to `.`).

Navigation matches `tools/card_screenshots.lua`: **A** through the title/attract until `CB_MainMenu`, **Campaign**, then **A** through Test Duel (needs `eds_demo.sav` in the repo root next to `eds_demo.gba`). If menu detection fails, check the scripting console for `callback=0x...` lines.

If the opponent wins the opening, the script waits in **`cpu_turn`** until **your** Draw/Main (`duel_step=5` and `turn_player=0` only). It does not treat `duel_step=8` (opponent turn) as Main Phase.

After your draw, **A** on the deck in Main Phase only opens **Surrender**—the script closes the menu with **B**, moves the field cursor **Left** (deck → spell/trap) then **Down** (→ hand, per `DuelCursor_PickAny`), and **blocks every A** until `sel_area=11` (hand). It will not run the play sequence on the deck pile.

In the first **Draw Phase**, the testing ROM’s deck menu lists **Mulligan** before **Draw** (`sDeckMenuCmdOrder` in `src/mod_opponents.c`). The script presses **A** on the deck, **Right** to **Draw**, then **A** again—do not mash **A** there or you will mulligan forever.

## Generate scripts

```bash
python3 tools/gen_testing_lua.py              # all scenarios → generated/
python3 tools/gen_testing_lua.py --poc-only     # subset for the 5 POC cards only
python3 tools/gen_testing_lua.py --list-only    # counts + sample basenames
```

Outputs:

| Path | Contents |
|------|----------|
| `generated/*.lua` | Stub navigators for every scenario |
| `poc/*.lua` | Same basenames for the 5 POC cards, with RAM setup + key sequences |
| `manifest.json` | Card, effect, actor, script path, recording path |
| `lib_eds.lua` | Duel RAM helpers, Test Duel navigation, naming |
| `poc_plays.lua` | Hand-tuned setup/play tables for POC **Player** scenarios |

## POC trial (5 cards)

Run these after `make demo` and with a save that reaches the main menu:

1. `poc/Card_Destruction_Discard_And_Draw_Player.lua`
2. `poc/Soul_Exchange_Tribute_Opponent_Monster_Player.lua`
3. `poc/Torrential_Tribute_Destroy_All_On_Summon_Player.lua`
4. `poc/Exiled_Force_Tribute_Destroy_Target_Player.lua`
5. `poc/Goblin_Attack_Force_Usage_Deck_Verify_Player.lua`

CPU scenarios for those cards use the same basename pattern with `_CPU` and let the empty-deck opponent idle while recording (~60 s).

## Regenerating after TESTING.md changes

```bash
python3 tools/gen_testing_md.py
python3 tools/gen_testing_lua.py
```

Effect slugs are derived from scenario wording and **Coded behavior** quotes; tweak `human_effect_name` / `ai_effect_name` in `gen_testing_lua.py` if a slug should be renamed for clarity.

## Headless

For automated RAM checks without MP4, use [`tools/emu.py`](../../tools/emu.py) `lua` with a savestate at Main Phase 1 (`tools/emu/states/duel_main1`) and set `skipNavigation = true` in a custom script.
