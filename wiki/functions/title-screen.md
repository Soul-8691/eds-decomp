---
title: CB_Title (proposed) and its steps
type: function
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-01
---
# CB_Title `CB_Title`

| Function | Address | Size | Proposed name | Unit |
|---|---|---|---|---|
| runner | `0x080057BC` | 0x5C | `CB_Title` | `title_menu` |
| step 0 | `0x0800527C` | 0x94 | `Title_Init` | `title_screen` |
| step 1 | `0x08005310` | 0x54 | `Title_Setup` | `title_screen` |
| step 2 | `0x08005368` | 0xF4 | `Title_Intro` (hypothesis) | `title_screen` |
| step 3 | `0x0800548C` | 0x74 | `Title_HandleInput` | `title_screen` |
| step 4 | `0x0800545C` | 0x26 | `Title_FadeOut` | `title_screen` |
| step 5 | `0x0800553C` | 0x1D4 | (none; 13-case state machine, jump table `0x08005564`) | `title_menu` |
| step 6 | `0x08005718` | 0xA2 | `Title_StartGame` | `title_menu` |
| helper | `0x08004FD8` | 0x232 | `Title_Draw` (hypothesis) | `title_screen` |

Mode: Thumb. Match: nonmatching. Runner table `0x081988B0`, index `gMain+0x4878`, sub-state `+0x4858`. The debug table calls it "TITLE".

## Purpose
Verified from the code:
- **Init:** clears `gTitleState` (`0x0201527C`, 4 bytes). `bit0 = IsSaveChecksumValid()` (save present), and `bit1 = bit0`, so the cursor starts on **Continue** when a save exists. Then `DISPCNT = 0`, `SetBrightnessBlack`, `ResetVideo`, `ResetBgScroll`, default BGxCNT (`Title_InitBgCnt`), `vblankFlags = 3`.
- **Setup:** repeats the video reset, then `Title_Draw` (`0x08004FD8`, which has the strings "New Game"/"Continue", installs the HBlank handler `0x08004ABD` and the VBlank callback `0x08004F09`), then `PlayBGMNoTrack(0)`. **The title BGM is song 0.**
- **Intro** (step 2): uses `FadeFromBlack`, `FadeToWhite`, `LoadBgImage(…, 0x087C056C)` and `FadeFromWhite`. It sets and clears the HBlank IRQ (hypothesis: the title logo flash/reveal).
- **HandleInput:** Left/Right (`newKeys & 0x30`) toggles between New Game and Continue, but only when a save exists (`PlaySE(0)`); otherwise it plays `PlaySE(3)` (error). A plays `PlaySE(1)`, calls `FadeOutBGM()` and advances to the next step.
- **FadeOut:** `FadeToBlack(1)`, clears the VBlank callback.
> [!note] Mod change (2026-11-04)
> New Game no longer plays Tea's intro: `Title_StartGame` starts BGM 1 and runs `StarterDeckSelect_Run` only (events 100 and 101 are skipped). The screen sets up its own video (`PackList_InitVideo`, `StarterDeckSelect_Init`), and it still calls `InitSaveData`, `BuildStarterDeck` and `SaveGame`. The next bullet describes the original.

- **StartGame:** if **Continue** (bit1), it returns 1 immediately, so `CB_Title` returns 1 and `MainLoop` enters `CB_MainMenu`. For **New Game**: `StartDialogue(100)` (`0x08001C10`), `PlayBGM(1)`, runs the text-box runner `0x08001AE4` until it finishes, then `StarterDeckSelect_Run` (new-game setup: it calls `InitSaveData` and `SaveGame`), then `StartDialogue(101)` and runs it. Dialogue events 100/101 are Tea's intro ("Nice to meet you! I am $q02, a friend of …"); see [[text-system]]. Only then does it return 1, and the game goes to the main menu.

Hypothesis: step 5 (`0x0800553C`) is the attract/intro animation that plays between the title fade-out and the game start (it loads OBJ graphics `0x087CC1D4`/`0x087CBFD4` and BG `0x087C29D4`). It hasn't been traced case by case.

## Notes
- `0x08001AE4` is the entry the debug menu calls "Bustup" (a bust-up portrait dialogue), and `0x08001B34` is "Auto Bustup". They step through the text-box module's function table at `0x0813ADD4` ([[text-system]]) and also work as stand-alone scenes.

Related: [[program-flow]], [[license-sequence]], [[main-menu]], [[save-game]].

> [!note] Mod change: version and build lines (2026-11-04)
> `src/mod_title.c` draws two lines in the 8-pixel font: the demo version top left (`gModTitleVersion`, only with `MOD_DEMO`: `make demo`, `make release-demo`) and `Release` or `Debug` top right (`gModTitleRelease` / `gModTitleDebug`, by `MOD_RELEASE`; all three strings are data, 16 bytes each, HMA `text.title.*`). OBJ tiles 64-79, OBJ palette 15. `MOD_DEMO` comes from the Makefile through `build/include/mod_config.h`. Built; not tested in an emulator.

## The mod's title images (2026-11-08)

- **Plain images instead of packs.** The original title loads image packs (deduplicated tiles and a cell list),
  which Hex Maniac Advance can't show. `tools/title_images.py` writes plain copies (`gModTitleImages`,
  `data/mod_title_data.s`: u32 offsets[4], then each image's colours and its tiles in row-major order) and
  `src/mod_title.c` draws them: `ModTitle_Load` (in `Title_LoadGraphics`, in place of the flame and coin packs'
  `LoadBgImage4bpp` calls) and `ModTitle_LoadCopyright` (at the white flash, in place of the copyright pack). The
  grid (BG3) stays the original's, already a plain 4 x 4-tile pack.
- **"Yak's YGO Hack"** is one 240 x 32 image, `title/yaks_ygo_hack.png`, at its final size: the 2x scaling is in the
  image, not the code. It was made once by running `BuildMenuLabelEx` (src/mod_recipes.c: the main menu sheet's
  highlighted letters and the drawn Y, G, O, H, ') in Python on the ROM's `gMainMenuObjGfx` / `gMainMenuObjPal`,
  doubled and cropped to the 240 pixels the sprites showed. It is drawn on BG0 (the original logo's 8bpp layer, map
  rows 5-8, tiles 0x10-0x87, palette entries 0-15) instead of four affine 32 x 16 sprites; the label is no longer
  built at run time. (A GBA sprite is at most 64 x 64, so one image on a background is the single-picture form.)
- **The flames** are 8bpp now: BG1 is set to 256-colour in `ModTitle_Load` (the title only; the main menu sets its
  own BG1CNT), tiles 0x90-0x11F (8bpp), colours in palette entries 16-143. The 15 original colours are 16-30; up
  to 128 colours can be used. A full 256 is not possible: the coin, grid and copyright layers need their 4bpp
  palettes (banks 9, 11, 12) and BG0 the text's 0-15.
- **The coin** moved to 4bpp tiles 0x2C0-0x34F (12 x 12 with its blank tiles; the copyright line is 0x284-0x2BB).
- Hex Maniac Advance ([[hma-toml]]): `graphics.title.text`, `.flame`, `.coin`, `.copyright` (each `.image` and
  `.palette`), `text.title.version` / `.release` / `.debug`, and the original grid `graphics.titlescreen.background`
  (its baserom.toml address and size corrected: tiles at 0x0867DFFC, 4 x 4).
- Checked in mGBA (screenshots without input): the title with the text, coin, copyright, grid and the two lines;
  the flames waving over the coin in the fade-in. `.text` 0x10 bytes smaller (0x0807F33C).
- **Flame wave toggle:** `gModTitleFlameWave` (src/mod_title.c; HMA MatchedWord `data.constants.title.flamewave`,
  1 byte). `Title_LoadGraphics` now calls `ModTitle_LoadWave` instead of copying `gTitleLogoWave` to
  `gMain.hblankScroll`: 1 (default) copies the wave, 0 zeros the table, so `Title_HBlank` leaves the flames still.
  Checked in mGBA with the byte set to 0 (straight flames in the fade-in). `.text` -0x14 (0x0807F328).
- **Full-screen text and flame images (2026-11-08):** both are now 240 x 160 (the whole of BG0 and of BG1), so
  anything can be drawn anywhere: `title/yaks_ygo_hack.png` has the text at y 40, the flame image the flames at
  x 72, y 0. Two full-screen 8bpp images (600 tiles each, 75 KB) don't fit in the title's background VRAM, so
  `LoadScreenImage` loads only their distinct tiles (`PoolAdd`: a blank tile is tile 0, a tile equal to one
  loaded before shares it; a hash then a word compare) into 8bpp tiles 0x10-0x15F and 0x1CC-0x3FF, 644 in all
  for both images (past that a tile shows blank). The defaults use 158 (text 74, flames 84). The copyright line
  and the coin moved to 4bpp tiles 0x2C0-0x2F7 and 0x2F8-0x387, next to the grid (0x388-0x397). The pool's
  bookkeeping is in `gBustupBitmapBuffer` (free on the title). HMA: `graphics.title.text.image` and
  `graphics.title.flame.image` are `ucs8x30x20`. Checked in mGBA (the same title and fade-in as before).
- **Palette order and 4bpp text (2026-11-08):** the flame image now holds BG palette entries 0-127 (128 colours;
  entry 0 is the screen's backdrop, and the flames' 15 original colours keep their indices 1-15, so a pixel's value
  is its palette entry). The text moved behind them: BG0 is set to 16-colour in `ModTitle_Load`
  (`REG_BG0CNT &= ~BGCNT_256COLOR`), the image is 4bpp (`ucs4x30x20`) in BG palette 8 (entries 128-143). Each image
  has its own tile pool (`PoolStart` / `PoolAdd`, bookkeeping still in `gBustupBitmapBuffer`). Charblock 1 bytes:
  0x0000-0x003F tile 0 (blank); the text's 4bpp tiles from 0x0040 (4bpp tile numbers reach only 0x8000: room for
  the whole text, 600 tiles, as distinct tiles); 0x5800-0x72FF the copyright, coin and grid; the flames' 8bpp tiles
  from 0x7300 to 0xBFFF (0x06010000, the end of BG VRAM), then down from 0x5800 to the text's end. So the flames get
  at least 308 distinct tiles (more when the text uses fewer). HMA: `graphics.title.flame.palette` is
  `` `ucp4:01234567` `` (128 colours). Checked in mGBA (screenshots at frames 660 and 1200: the same as before).
  `.text` unchanged (0x0807F328).
