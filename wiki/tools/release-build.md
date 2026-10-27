---
title: Release build (make release)
type: tool
status: draft
confidence: high
sources: [rom-analysis]
updated: 2026-10-26
---
# Release build (`make release`)

`make release` (`$(MAKE) RELEASE=1 all`) builds `eds_release.gba`, the mod as players get it. Plain `make` builds
`eds.gba`, the testing build.

## How the mode reaches the code
- **The header.** The Makefile writes `build/include/mod_config.h` (`#define MOD_RELEASE 0|1`) on every run, but
  rewrites the file only when the value changes.
- **Includes.** `global.h` includes it (through `__has_include`; tools that preprocess alone see 0), and C objects
  wait for it (order-only).
- **Switching.** The `-MMD` dependency files then rebuild what includes it, which is about 40 s with `-j8`. Both
  builds share `build/` and `build/eds.elf`.

## What changes

| | `make` (testing) | `make release` |
|---|---|---|
| Deck out (`CanDeckOut`, duel_cmd_deck.c) | the human cannot deck out (except Link Battle) | anyone can |
| New game trunk (`BuildStarterDeck`, title_screen.c) | `GiveStartingTrunk`: 8 of every card | none (the starter deck only) |
| `DECK_MIN_CARDS` (save.h) | 1 | 40 |
| `CARD_COPY_LIMIT_NONE` (save.h) | 99 (no copy limit) | 3 |
| Format opponents (`ModIsOpponentUnlocked`, mod_opponents.c) | all open | Yugi-Kaiba open; each next one after 10 wins (`sWinsToUnlockNext`) |

## Checked
Both builds were checked in the preprocessed units:
- release `CanDeckOut` returns 1;
- `GetBanlistCopyLimit`'s default is 3;
- no `GiveStartingTrunk` call;
- `make` then switches back.

Related: [[duelist-points]], [[yugi-kaiba-opponent]], [[banlists-menu]].
