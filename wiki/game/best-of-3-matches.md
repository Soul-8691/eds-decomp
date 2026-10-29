---
title: Best-of-3 Matches
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-29
---

# Best-of-3 Matches

Every Campaign duel against the CPU is a best-of-3 match in this build. The original played single duels, except
on match days (the Championship, Duel Ceremony, Grandpa Cup and the holiday special duels).

## The original match flow (kept)

- **Format.** `gMain.duelFormat` is `DUEL_FORMAT_SINGLE` or `DUEL_FORMAT_MATCH`. `Campaign_StartDay` (link_battle.c)
  sets SINGLE and clears `matchDuelCount` / `matchScore` each day. Match days and the special duels set MATCH.
- **Scoring.** `Campaign_ShowDuelResult` ([[campaign-steps-c]]) adds +1 for a win and -1 for a loss to `matchScore`.
  After two duels ±2 decides; after three the sign does (0 is a draw). `gDuel.result` then becomes the match's
  result, so the record, the DP and the pack are for the match.
- **Between duels.** An open match runs the side-deck swap (`SideDeckSwap_Run`: Deck Edit with `sideSwapMode`, which
  exchanges cards between the Main Deck and the Side Deck, `SideDeckSwap_ExchangeCards`). It then goes back to
  `CAMPAIGN_STEP_TURN_ORDER`, so the opponent select and pre-duel dialogue don't run again. The loser of the last
  duel picks first or second (`Campaign_DecideTurnOrder`).

## Changes

- **Every duel a match.** `Campaign_ShowDuelResult` sets `DUEL_FORMAT_MATCH` when it scores a duel. The day keeps
  SINGLE until then, so the pre-duel dialogue is unchanged: the first meeting, rematch and greeting texts, and
  Simon's missing cards. Opponents 0, 25 and 31, which the result step skips, would stay single; they aren't
  duelists (1-24 are used).
- **The CPU keeps its deck.** `GetOpponentDeckOverride` (src/mod_decks.c) picks the deck on the match's first duel,
  through `PickOpponentDeckOverride` (a weighted random pick for Tristan and the format opponents). It returns the
  same deck for the later duels, cached in `gModMatch` (EWRAM 0x0202D400, symbols.ld).
- **Siding is undone after the match.** `Campaign_SetupDuel` saves the Deck and Side Deck lists on the first duel
  (`ModMatchSavePlayerDeck`). Once the match is decided, `ModMatchRestorePlayerDeck` puts them back: it takes the
  current lists off the trunk's per-card Deck / Side Deck counters, copies the saved lists in, and adds them back.
  The swap only exchanges cards between the two, so the trunk counts don't change.
- `gModMatch.valid`-style magic words (`MOD_MATCH_VALID`) guard the cache and the saved decks, since that RAM is not
  cleared at power-on.

> [!question] Not checked
> Whether the game saves between the duels of a match. If it does and the console is turned off mid-match, the saved
> deck keeps that match's side-deck swaps.

Checked in the source and the build; not tested in an emulator.
