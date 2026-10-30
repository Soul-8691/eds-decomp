---
title: Campaign opponents (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-30
---
# Campaign opponents (mod)

This build re-labels the Campaign's 24 duelist IDs as format and deck opponents (`src/mod_opponents.c`). The opponent
select keeps its five pages, its bitmaps and the IDs (`gOpponentSelectDuelists`; records, decks and unlock checks
stay keyed by ID). Checked in the source and the build (portraits also in a Python prototype of the same algorithm);
not tested in an emulator.

## The opponents
`sPageDuelists` (mod_opponents.c) places them; `PAGE_DUELIST` in campaign_select.c asks `ModPageDuelist` instead of
the ROM's `gOpponentSelectDuelists`. The formats fill pages 0-2 in unlock order. Within a page they follow the
cursor's ring from the top left (slots 2, 3, 4, 0, 1: top left, top right, right, bottom, left), so each opponent sits
next to the one before it.

| Page | Top left | Top right | Right | Bottom | Left |
|---|---|---|---|---|---|
| 0 | Yugi-Kaiba (Tristan) | Critter (Rex) | Treasure (Espa Roba) | Imperial (Mako) | Android (Mai) |
| 1 | Joey-Pegasus (Weevil) | Fiber (Rare Hunter) | Yata (Arkana) | Scientist (Umbra & Lumis) | Vampire (Marik) |
| 2 | Chaos (Strings) | Warrior (Kaiba) | Goat (Ishizu) | Cyber (Yami Bakura) | Reaper (Yami Yugi) |
| 3 | LR-Chaos Turbo (Simon) | sdlkilla-Warrior (Pegasus) | geistD-Goat Control (Grandpa) | (empty) | (empty) |

- **Names in brackets** are the duelist IDs, which key the records, decks and unlocks.
- **Hidden.** The IDs not placed (Yugi, Tea, Joey, Bakura, Shadi, Duel Computer) are hidden. Page 4 is empty and
  can't be reached (`ModNextPageOpen`: pages 0-3).
- **Opening position.** The select opens on the newest unlocked format (`ModOpponentSelectStart`, in
  `OpponentSelect_Init`).
- **History.** The first version kept the ROM's placement (formats from page 1, slot order), which put Critter at
  page 1's bottom centre instead of next to Yugi-Kaiba.

## Format opponents
- **Unlocks.** They unlock in that order (Yugi-Kaiba open from the start). Beating format k
  `sWinsToUnlockNext[k]` times opens format k + 1 (all 1, each its own constant), counted from
  `gSaveData.duelRecords[id].wins`.
- **Format check.** Picking one runs `BanlistBlocksDuel` (src/mod_banlist.c) with the format's banlist and card pool
  (`FormatCardLimit`, src/mod_banlists.c: 0xFF outside the pool, else the list's limit, else 3). If the deck is
  illegal, a warning lists the offending cards ("Deck not legal for <format> (<date>)").
- **Decks.**
  - Yugi-Kaiba plays the three Yugi-Kaiba .ydk decks (Soul Control 50%, Vanilla Beat 30%, Stall Mill 20%).
  - Critter plays Jazz-Masked Control 60% and Gyomberman-Muka Muka Turbo 40% (`CritterDeck`, main + extra decks).
  - Treasure plays dump truck-Tomato Control 80%, SolEmerald69-Recruiter Control 10% and bsocks-Germ Burn 10%
    (`TreasureDeck`).
  - Imperial plays Aretos-Tomato Control 80% and bsocks-Relinquished 20% (`ImperialDeck`).
  - Android plays Jazz-Tomato Control 80% and paoloNS-Vanilla Beat 20% (`AndroidDeck`). Fairy Box was added to
    `config/new_cards.txt` for Tomato Control (engine key 1424, see [[added-cards]]).
  - Joey-Pegasus plays AJTBLS-Tomato Control 80% and ellietincan-Vanilla Beat (JP) 20% (`JoeyPegasusDeck`).
  - Fiber plays Aretos-Tomato Control (Fiber) 80% and GBO-Recruiter Control 20% (`FiberDeck`). Super Robolady was
    added to `config/new_cards.txt` for Tomato Control. The Rare Hunter's Exodia AI flag (ai_deck.c) is now set only
    with his original deck.
  - Yata plays Shigeki Kitamura-Tomato Control 50%, ellietincan-Warrior Tomato Control 40% and AlephYa2-Gearfried
    10% (`YataDeck`, Arkana's slot). Mirage of Nightmare (all three Main Decks) was added to `config/new_cards.txt`
    for them and is coded ([[added-cards]]). Dice Jar, in Warrior Tomato Control's Side Deck, was added and removed
    again, so that Side Deck has 14 cards.
  - Scientist plays Aretos-Tomato Control (Scientist) 60% and DK-ZERO-Scientist FTK 40% (`ScientistDeck`, Umbra &
    Lumis' slot). Every card in both was already in the game. Magical Scientist and Gilasaurus are coded (2026-10-30,
    [[added-cards]]); no coded effect yet: Super Robolady, Super Roboyarou (and Dark Balter, Fiend Skull Dragon, Ryu Senshi
    below).
  - Vampire plays Woawa-Warrior Control 80%, ellietincan-Scientist FTK 10% and AJTBLS-Warrior Tomato Control 10%
    (`VampireDeck`, Marik's slot). Sasuke Samurai (Warrior Control's Side Deck) was added to `config/new_cards.txt`
    for them (ID 1160); its effect is not coded yet.
  - The other 5 keep their original duelists' decks for now.
  - Cards in these decks with no coded effect yet: Dark Balter the Terrible, Fiend Skull Dragon, Ryu Senshi (Joey-
    Pegasus Tomato Control's Fusion Deck), Skull Lair and The Last Warrior from Another Planet (Joey-Pegasus Vanilla
    Beat and both Android decks).
  - The .ydk files are in `decks/`. Their passcodes resolve through the built password table, which stores each
    passcode as 4 BCD bytes in digit order.
- **Portraits.** Yugi-Kaiba La Jinn, Critter Sangan, Imperial Imperial Order, Treasure Confiscation, Android Jinzo,
  Joey-Pegasus Graceful Charity, Fiber Fiber Jar, Yata Yata-Garasu, Scientist Magical Scientist, Vampire Vampire
  Lord, Chaos Chaos Emperor Dragon - Envoy of the End, Warrior D.D. Warrior Lady, Goat Scapegoat, Cyber Cyber Dragon,
  Reaper Spirit Reaper. Fiber Jar, Yata-Garasu, Magical Scientist, Chaos Emperor Dragon and Cyber Dragon were added
  to `config/new_cards.txt` for these (IDs 1130-1134; their effects were not coded then. Fiber Jar, Yata-Garasu, Cyber
  Dragon and Magical Scientist have been coded since, see [[added-cards]]). An added portrait card that is
  not in the build leaves the page's own portrait (the `ART_*` guards).
- **Free palette entries.** Page 1 has 26, page 2 94 and page 3 23. A Python preview of the three pages looked
  clean.

## Deck opponents
Always open, with no format check:
- Simon plays the LR-Chaos Turbo deck, Pegasus the sdlkilla-Warrior deck and Grandpa the geistD-Goat Control deck
  (`GetOpponentDeckOverride`).
- Portraits: Black Luster Soldier - Envoy of the Beginning, D.D. Warrior Lady and Thousand-Eyes Restrict.

## Hidden slots
- **On screen.** No name, and the stone cover (`OpponentSelect_DrawLockedCovers`) over the portrait.
- **Cursor.** It skips them (`ModSlotHidden`, also used when a page loads), and they never unlock.
- **Pages.** Every page can be visited (`ModNextPageOpen`); locked opponents show the cover and "[ Unknown ]".

## Names
`ModOpponentSelectName` gives the table's name for the slot name and the banner. Not changed: the Record screen,
whose name plates and portraits are BG images (`Record_DrawPage`).

## Portraits (verified against the ROM)
- **Where.** `ModOpponentSelectPortraits`, at the end of `OpponentSelect_LoadPage`, draws the card art of each open
  opponent that has a portrait card (La Jinn for Yugi-Kaiba, the three deck cards) into its circle in both Mode 4
  frames: centre slot position + (31, 33), radius about 23.5, measured on the bitmaps; the art is scaled 72/49.
- **Art decoding.** `gCardArtGfx` of the *built* ROM: 6 bits per pixel, a little-endian bit stream, 9 x 10 tiles of
  8 x 8, as `LoadCardPicture` and `UnpackCardArt8bpp` read it. The baserom's art at 0x082A6500 doesn't decode this
  way; the build re-packs the art.
- **Colours.** The art's colours map to the nearest colour the page bitmap uses; colours with none close take
  palette entries the bitmap doesn't use. Page 0 has 27 free, page 4 18; later portraits share earlier ones'
  colours.

## No dialogue
`ModSkipsDuelDialogue` (every non-hidden opponent):
- **Before the duel.** `Campaign_SelectOpponent` returns before `Campaign_StartPreDuelDialogue`. That dialogue also
  holds Simon's first-meeting gift (every missing card), which therefore no longer happens.
- **After the duel.** `Campaign_ShowDuelResult` returns without the result text.
- **In a match.** Between duels it goes straight to the Side Deck swap.

## Rewards
- **Event days.** `Campaign_GiveRewards`'s choose step ignores the calendar event for these opponents. A win always
  goes to the regular pick-a-pack list (no fixed holiday/event packs, no tournament tickets), and a loss has no
  penalty (no Rare Hunter card loss).
- **Stale reward pack.** The "new packs" step now clears `gMain.rewardPack` in both branches. The original
  cleared it only without a notice, so a magazine day's pack left there opened at once and skipped the list.

## Calendar event challengers
- **Who.** `Campaign_StartDay` (link_battle.c) draws every event challenger (Championship rounds, Duel Ceremony,
  Grandpa Cup qualifier and final, Rare Hunter day) with `ModEventChallenger`: a random unlocked format, in place of
  the event's own table (`gTournamentOpponents`, `gGrandpaCup*Opponents`, `gRareHunterOpponents`). In make release
  only formats whose banlist lets the collection make a 40-card Main Deck (`BanlistMainDeckCapacity`, the Banlists
  menu's check); if none can, any unlocked one.
- **Dialogue.** The event intro plays as before, and `Campaign_SelectOpponent` keeps the challenger's pre-duel text
  (its original bustup portrait). On a Rare Hunter day only duelists 11-15 have a text; another challenger skips it.
- **Banlist and deck.** Sub-step 3 runs `CB_ChallengerRecipes(format)` (mod_recipes.c), the Deck Recipes scene with
  "Challenger's format: X" under the list. On entry it puts the format in use as the Deck Edit banlist
  (`SwitchToBanlist`, as the Banlists menu: over-limit copies go back to the trunk). SELECT opens Deck Edit
  (sub-step 4 runs `CB_DeckEdit`, saves, and comes back to the recipes). B starts the duel; in make release it is
  refused while the deck has under 40 cards or a card the format does not allow (`CountIllegalCards`, mod_banlist.c),
  unless the collection cannot make a legal deck at all: then a second B duels anyway. The event's BGM keeps
  playing.
- **Rewards** are still those of the format opponents (see Rewards above).

Related: [[banlists-menu]], [[campaign-select-c]], [[campaign-steps-c]].
