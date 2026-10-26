---
title: Yugi-Kaiba opponent (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-26
---
# Yugi-Kaiba opponent (mod)

This build turns Tristan (`DUELIST_TRISTAN`, 4) into the "Yugi-Kaiba" format opponent: his decks are the three
Yugi-Kaiba .ydk decks (src/mod_decks.c), and his duels require a deck legal in that format (`BanlistBlocksDuel`).
The changes below are in `src/mod_banlist.c`. Checked in the source and the build (and the portrait in a Python
prototype of the same algorithm); not tested in an emulator.

## Name
`OpponentSelect_LoadPage` asks `ModOpponentSelectName`, which gives "Yugi-Kaiba". That covers the slot name and
the selected-duelist banner, which reuses those tiles.

Not changed: the Record screen, whose name plates and portraits are BG images (`Record_DrawPage`). The duel screen
shows no opponent name.

## Portrait (verified against the ROM)
- **Where.** The opponent-select pages are 240x160 Mode 4 bitmaps (`gOpponentSelectPageBgs`, 8bpp, 256-colour
  palette) with the portraits drawn in. Tristan is page 0, slot 2 (`gOpponentSelectDuelists` = 1 2 4 5 3 ...),
  top left. The ring's inside is a circle centred at slot position + (31, 33) with radius about 23.5, measured on
  the bitmap.
- **When.** `ModOpponentSelectPortraits`, at the end of `OpponentSelect_LoadPage`, draws La Jinn the Mystical Genie
  of the Lamp's card art into that circle in both Mode 4 frames. The art is scaled 72/49 and sampled nearest.
- **Art decoding.** `gCardArtGfx` in the *built* ROM: 6 bits per pixel, a little-endian bit stream, 9 x 10 tiles
  of 8 x 8, as `LoadCardPicture` and `UnpackCardArt8bpp` read it. The baserom's art at 0x082A6500 doesn't decode
  this way; the build re-packs the art (newcards).
- **Colours.** The art's 64 colours map to the nearest colour the page bitmap uses. A colour with none within
  about 24 (8-bit RGB distance) gets a palette entry the bitmap doesn't use. Page 0 uses 228 entries and leaves 27
  free besides 0; La Jinn needs 27.

## No dialogue
`ModSkipsDuelDialogue`:
- **Before the duel.** `Campaign_SelectOpponent` returns before `Campaign_StartPreDuelDialogue`.
- **After the duel.** `Campaign_ShowDuelResult` returns without the win/lose/draw text; the result music still plays.
- **In a match.** Between duels it goes straight to the Side Deck swap, then back to the turn order, without the
  between-duels or next-duel texts.

Related: [[banlists-menu]], [[campaign-select-c]], [[campaign-steps-c]].
