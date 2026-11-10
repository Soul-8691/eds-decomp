# TESTING.md mGBA scenario scripts

One Lua script per human (**Player**) or AI (**CPU**) scenario in repo-root [`TESTING.md`](../../TESTING.md). Each run targets **Test Duel** vs an empty opponent (`make demo`, `MOD_TEST_DECK`).

## Exact scenarios

The scenarios are exact, so a script can reproduce each one: the whole starting state, every input, and the state to
check after. They are data in `exact_cards_1.py` ... `exact_cards_5.py` (one `SCENARIOS` dict each, by TESTING.md
format), written in the small format that [`exact.py`](exact.py) defines and documents:

| Field | What it holds |
|---|---|
| `start` | Where the setup is applied (the start of that turn's Main Phase 1, the Draw Phase done): `turn` (`you` / `opp`), `turn_count` (`gDuel.turnCount`; 0 has no Battle Phase), `lp` |
| `setup` | `<side>.<place>: <cards>` lines: `hand` (slot 0 first), `m0`-`m4` and `st0`-`st4` (`gDuelPlayers[p]` zones 0-4 / 5-9) with `atk`, `def`, `set` or `up` (and `ss` for Special Summoned), `fz`, `deck` (the top, top card first: `deck[0]` is the next draw), `gy`, `banished`, `extra`. Both hands are exactly as listed; anything else not listed is empty (the Deck excepted) |
| `steps` | Verbs (`activate`, `summon`, `tribute_summon`, `special_summon`, `fusion_summon`, `set`, `flip_summon`, `change_position`, `target`, `pick`, `discard`, `yes`, `no`, `choose`, `declare`, `zone`, `position`, `battle`, `attack`, `cancel`, `main2`, `pass`, `opp_turn`, `cpu_turn`, `draw`, `check`) on places such as `you.hand[Card Destruction]`, `you.m0`, `opp.m[Summoned Skull]`, `you.gy[Sangan]`; `when <event>: <step>` waits for an attack, activation, summon, prompt or replay first |
| `expect` | `you.m0 = Summoned Skull def`, `opp.m0 = empty`, `you.gy = A, B` (exact), `opp.gy has A` / `lacks A`, `you.hand count 2`, `opp.lp = 7200` (or `<=`, `in 5500\|6300`), `you.m0.atk = 2500`, `you.deck[0] = A`, `menu you.hand[A] has\|lacks Special Summon`, `cannot attack you.m0 -> direct`, `battle_phase blocked` |

`tools/gen_testing_md.py` renders them into TESTING.md (a setup table, numbered steps and the expected state, each
as code and in words) and fails when a card has no scenario, a line does not parse, or a scenario names a card that
is not legal in the card's format. `tools/gen_testing_lua.py` writes one script per scenario with that data:
`setupCards` (the hand and field rows, in the form `lib_eds.lua` already places; `position = "def"` and
`specialSummoned = true` are new fields it does not read yet), `setupPiles` (deck top, Graveyard, banished, Fusion
Deck), `start`, `steps`, `expect` and `freezeOpponent`. The key presses that play the `steps`, the RAM writes for
`setupPiles` and `start` (LP, starting on the CPU's turn), and the `expect` checks are still to be written in
`lib_eds.lua`; until then a generated script places the hand and field and waits (`play = { { wait = 90 } }`).

### CPU scenarios (`_CPU` scripts)

- **Only the card under test.** The script writes that card's ID to `gModTestingOnlyCard` (0x0202D800) every frame.
  In a testing build's Test Duel the CPU then activates no other card: `CanActivateEffect` and
  `ModCanActivateMonsterEffect` refuse them (`ModTestingCpuCardBlocked`, src/mod_testing_automation.c). It still
  summons, Sets and attacks as usual.
- **Where the video starts.** Frames are saved from the CPU's turn on; the MP4 starts (ffmpeg `-start_number`) in the
  phase the card's effect starts in, at the first frame the CPU starts using the card: its cursor on the card, a
  summon of it (a Flip Summon moves no cursor), its attack declaration, or 12 frames before it leaves the CPU's hand
  (a Spell/Trap played from the hand: the cursor first reaches it on the field). With none of these, it starts when
  that phase begins. It ends right after the effect (`EDS.effectUnderTestDone`).

### Reviewing the videos

`python tools/testing_scenarios/review_videos.py` (Windows Python with Pillow; ffmpeg on PATH) lists the recordings in
TESTING.md order and plays them (frame steps, speed, zoom, loop; the next videos decode ahead). **Keep** (K) moves a
video to `recordings/keep/`; `run_tests.ps1` then skips its script (`-IncludeKept` runs it anyway). Keep on a kept
video moves it back.

### The end of every scenario

The last step waits for the card's effect to start (a chain link of the card, or a battle it fights in) and for the
duel to go idle (10 frames). A Spell's activation starts a few frames after the menu's A, after an idle gap, which
10 idle frames alone took as the end. A card that never starts a chain or a battle ends after 600 frames.

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
- `Torrential_Tribute_Cpu_Duel_CPU.mp4`

Spaces and punctuation are folded to underscores; each effect word is title-cased (`EDS.slugEffect` in `lib_eds.lua`, `effect_slug` in `gen_testing_lua.py`).

With **ffmpeg on PATH** (default), you only need to run the POC script; the `.mp4` should appear under `recordings/` when the console prints `MP4: wrote ...`. PNG staging dirs are `recordings/<basename>_frames/` (wiped at the start of each run, then kept after encode; `keepPngFrames = false` only skips retaining them—still cleared on the next run).

Without ffmpeg, use mGBA’s GUI: **Tools → Record A/V** → **MPEG-4**, set the path from `_next_recording.txt`, **Start** before the duel, **Stop** after **Finished**.

Format **main decks** (40 cards) load as soon as the duel starts (**before** your Draw step), from `usage/<format>.json` via `format_decks.lua`. When the usage deck is loaded into RAM, the script **clears both hands** and **shuffles each main deck once** (`p0ShuffledOnce` / `p1ShuffledOnce`). By default you confirm **Draw** on your deck (the deck menu opens on **Draw**: **A**). Optional `ramOpeningDraw = true` sets **`skipDrawPhase`** and **RAM-draws 5** instead (no deck menu). **Scenario hand/field RAM** (`setupCards`) runs only **after** you confirm **Draw** (or after `ramOpeningDraw` RAM-deal), still during Draw/Standby before Main 1. **Recording** waits until `scenarioSetupDone`, skips `draw_mulligan`, and holds **3 frames** so the first PNG matches the scenario hand. If **you go first**, load+shuffle wait until **Turn Start** (`duel_step` 2). Pass `format` from TESTING.md; disable with `usageDeck = false`.

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

In the first **Draw Phase**, the testing ROM’s deck menu lists **Draw** before **Mulligan** (`sDeckMenuCmdOrder` in `src/mod_opponents.c`), so it opens on Draw. The script presses **A** on the deck, then **A** on **Draw** (Left back to Draw if the menu shows Mulligan).

## Generate scripts

```bash
python3 tools/gen_testing_lua.py              # all scenarios → generated/
python3 tools/gen_testing_lua.py --poc-only     # subset for the 5 POC cards only
python3 tools/gen_testing_lua.py --list-only    # counts + sample basenames
```

Outputs:

| Path | Contents |
|------|----------|
| `generated/*.lua` | One script per scenario, with its exact data (the old ones are deleted on each run) |
| `poc/*.lua` | The 5 POC cards' scenarios again, with `poc_plays.lua`'s setup + key sequences where it has them |
| `manifest.json` | Card, effect, actor, format, starting turn, script path, recording path |
| `lib_eds.lua` | Duel RAM helpers, Test Duel navigation, naming |
| `poc_plays.lua` | Hand-tuned setup/play tables for POC **Player** scenarios |

## POC trial (5 cards)

Run these after `make demo` and with a save that reaches the main menu:

1. `poc/Card_Destruction_Discard_And_Draw_Player.lua`
2. `poc/Soul_Exchange_Tribute_Opponent_Monster_Player.lua`
3. `poc/Torrential_Tribute_Destroy_All_On_Summon_Player.lua`
4. `poc/Exiled_Force_Tribute_Destroy_Target_Player.lua`
5. `poc/Goblin_Attack_Force_Mandatory_Follow_Up_Player.lua` (`poc_plays.lua` has no table for it yet; the older
   `Goblin_Attack_Force_Usage_Deck_Verify_Player.lua` is no longer a TESTING.md scenario)

CPU scenarios for those cards use the same basename pattern with `_CPU` and let the empty-deck opponent idle while recording (~60 s).

## Regenerating after TESTING.md changes

```bash
python3 tools/gen_testing_md.py
python3 tools/gen_testing_lua.py
```

Script and recording names come from each scenario's `effect` (`<Card>_<Effect>_<Player|CPU>`); rename one by
changing its `effect` in `exact_cards_*.py`. The POC basenames above match `poc_plays.lua`'s keys; a POC script uses
that table's setup and key presses when it has the scenario, else the scenario's own setup.

## Headless

For automated RAM checks without MP4, use [`tools/emu.py`](../../tools/emu.py) `lua` with a savestate at Main Phase 1 (`tools/emu/states/duel_main1`) and set `skipNavigation = true` in a custom script.
