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
    for them (ID 1160) and coded the same day ([[added-cards]]).
  - Chaos plays ellietincan-Chaos Control 80% and indoors-Chaos Warrior 20% (`ChaosDeck`, Strings' slot). Every card
    was already in the game. Two passcodes do not match the built table: Black Skull Dragon (11901678) is the original
    B. Skull Dragon, which has no passcode in EDS, and Dark Paladin (98502113) was added under 98502115 (an
    alternative artwork's ID on YGOPRODeck); a .ydk lookup needs those two aliases.
  - Warrior plays Gyomberman-Goat Control 40%, ellietincan-Warrior 30%, Patrick Smith-Warrior Control 20% and
    paoloNS-Chaos Return 10% (`WarriorDeck`, Kaiba's slot). Every card was already in the game; the uncoded ones are
    listed in DECKS.md.
  - Goat plays LR-Chaos Turbo 40%, sdlkilla-Warrior 20%, geistD-Goat Control 15% (the deck opponents' decks),
    N♡va-Chaos Control 10% (recipe "Nova-Chaos Control"), wolfe_KMH-Panda Burn, Dingosig-Chaos Return and
    paede17-Reasoning Gate Turbo 5% each (`GoatDeck`, Ishizu's slot, in twentieths: 8/4/3/2/1/1/1). Every card was
    already in the game; the uncoded ones are listed in DECKS.md.
  - The other 2 keep their original duelists' decks for now.
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
  Dragon, Magical Scientist and Chaos Emperor Dragon have been coded since, see [[added-cards]]). An added portrait card that is
  not in the build leaves the page's own portrait (the `ART_*` guards).
- **Free palette entries.** Page 1 has 26, page 2 94 and page 3 23. A Python preview of the three pages looked
  clean.

## Deck opponents
Always open, with no format check:
- Simon plays the LR-Chaos Turbo deck, Pegasus the sdlkilla-Warrior deck and Shadi's ID the geistD-Goat Control deck
  (Grandpa's ID until 2026-10-30) (`GetOpponentDeckOverride`).
- **Solomon (2026-10-30).** Grandpa's ID (`DUELIST_GRANDPA`), named "Solomon", first on the page (top left; the
  layout is Solomon, Chaos Turbo, Warrior, Goat Control, Test Duel in the cursor's ring order) with his own portrait
  (`PORTRAIT_FROM_SLOT(4, 3)`, the original page 4 top right; a preview from the built ROM's page bitmaps checked it).
  He is `DECK_ALWAYS`, so `make release` has him too (the only open slot of page 3, now visited in both builds:
  `PAGE_COUNT` 4), always open (`ModIsOpponentUnlocked`).
  - **His deck.** `Campaign_SelectOpponent` (campaign_steps.c) goes to the added sub-step 5 for him before a match's
    first duel (`ModOpponentPlaysRecipe`): `CB_OpponentRecipes` (mod_recipes.c) is the Deck Recipes scene with
    `opponentPick` set. A on a used recipe with a Main Deck calls `ModSetOpponentRecipe` (mod_decks.c), which keeps
    its Main Deck then Fusion Deck as card numbers in `gModOpponentRecipe` (EWRAM 0x0202F400, symbols.ld) and its
    `DeckList`; nothing of the player's is loaded or changed. B returns 2 and the step goes back to the opponent
    select. `GetOpponentDeckOverride` returns that list for him; the match's later duels keep it (gModMatch).
    `LoadOpponentDeck` (ai_deck.c) skips `RemoveOverLimitDeckCards` for him, as for Test Duel.
  - **Debug (2026-10-30).** Pegasus' ID (`DUELIST_DEBUG` = `DUELIST_PEGASUS` in mod_opponents.c), named "Debug",
    top right on the page with Cyber Twin Dragon's art, `DECK_ALWAYS`. The page now holds only Solomon (top left),
    Debug (top right) and Test Duel (right, make only; it was on the left). The LR-Chaos Turbo, sdlkilla-Warrior and
    geistD-Goat Control deck opponents (Simon's, Pegasus' and Shadi's IDs) are gone (`HIDDEN`); the Goat format
    opponent plays those decks, and they stay recipe presets 31-33.
    - Debug works like Solomon (`ModOpponentPlaysRecipe`; `GetOpponentDeckOverride` gives `gModOpponentRecipe`;
      `LoadOpponentDeck` keeps its copy counts; `AwardDuelDP` gives no DP). Its built decks start from 8 of every card
      with a copy limit of 99, Solomon's from 3 and 3 (`ModRecipeBuildCopies`, passed to `ModRecipeBuildBegin`;
      `GetBanlistCopyLimit` returns `ModRecipeBuildLimit`). The recipe scene's lines use the opponent's name.
    - `ModDebugDuel` (gMain.opponent is Debug, not a Link Battle) turns on make's duel rules in make release too:
      `IsHandRevealed` returns 1 (duel_piles.c), `CanDeckOut` is 0 for the player (duel_cmd_deck.c), Card View opens
      on the opponent's face-down cards (card_menu_input.c), and the info bar describes them.
    - **The info bar moved.** Three checks in `DuelScreen_DrawCursorInfo` grew duel_info_bar.c by 0x90 bytes, which
      the size-limited `.text` (to 0x08080A20) could not take. The function moved to a new `@farcode` unit,
      src/mod_info_bar.c, written more compactly (one `ZoneCardShown` check and one `DrawPileLabel` helper). Release
      `.text` went from 0x0808098C to 0x080807D0. duel_info_bar already matched none of its 21 functions.
  - **Building his deck (2026-10-30).** SELECT in the picker calls `ModRecipeBuildBegin` (mod_decks.c). It writes the save
    (`SaveGame`), then gives `gSaveData` a temporary collection: 3 of every card ID in the trunk (the cards of
    `GiveStartingTrunk`: no tokens, no unused IDs), the deck copy counters at 0, and empty decks. It also sets
    `gModOpponentRecipe.building` (the struct's old pad). While that is set, `GetBanlistCopyLimit` returns
    `CARD_COPY_LIMIT_NONE` (3 in make release) and `IsInActiveBanlistPool` returns 1 (mod_banlists.c), so no banlist
    applies. `CB_OpponentRecipes` then runs `CB_DeckEdit` itself (`solomonDeckEdit` in `gDeckRecipes`), not through a
    Campaign sub-step, so campaign_steps.c (size-limited `.text`) is unchanged. After that it runs the scene again.
    While he builds, the scene shows "Solomon's deck: Dn Sn Fn" and the keys work as follows:
    - A opens the usual actions. Load puts a recipe into his deck (cards over the copy limit are listed as left out,
      "banlist: none"). Save here writes his deck to a slot (the name entry starts at "Solomon"). Neither marks the
      slot (`SetActive`) nor saves the game.
    - SELECT opens Deck Edit again.
    - B, if his Main Deck has cards, calls `ModSetOpponentRecipe` with it, then `ModRecipeBuildEnd`.
    - START calls `ModRecipeBuildEnd` only.
    - The status lines name whose deck it is (`AppendOwner`): "Recipe 05 is now Debug's deck.", "Solomon's deck was saved
      as recipe 12.", and so on. They said "your deck" until 2026-10-30, so a Load looked as if it went to the player's
      deck (reported). It always went to the opponent's.

    `ModRecipeBuildEnd` reads `gSaveData` back from SRAM, so the player's collection, decks and banlist return as
    they were. Nothing between the two saves the game: Deck Edit never calls `SaveGame`. A reset reloads the save
    from SRAM, and the boot clears EWRAM, flag included.
  - **No DP.** `AwardDuelDP` (mod_dp.c) returns early for him.
  - Original ties to Grandpa's ID: his unlock (a complete collection) is overridden by `ModIsOpponentUnlocked`.
  - Release `.text` 0x080807D0 (the Campaign step and the copy-limit test).
- Portraits: Black Luster Soldier - Envoy of the Beginning, D.D. Warrior Lady and Thousand-Eyes Restrict.
- **Test Duel (2026-10-30).** The Duel Computer's slot (`DUELIST_DUEL_COMPUTER`, page 3 bottom: the fourth in the
  cursor's ring) is "Test Duel" in `make` builds, hidden in `make release`. Its deck is the .ydk in `TEST_YDK` (the
  Makefile; default `/mnt/c/ProjectIgnis/deck/test.ydk`), which `tools/testdeck.py` turns into
  `build/include/test_deck.h` on every build (rewritten only when it changes): `MOD_TEST_DECK`,
  `TEST_DECK_PORTRAIT` (the first Main Deck card's number; no longer used for the portrait) and `sTestDeck` (Main then Extra Deck card
  numbers; passcodes through the built password table, with the B. Skull Dragon / Dark Paladin aliases; unknown ones
  left out with a warning). `GetOpponentDeckOverride` returns it, and `LoadOpponentDeck` (ai_deck.c) skips
  `RemoveOverLimitDeckCards` for that duelist, so a test deck keeps any number of copies. Without the file (or with
  no known Main Deck card) the header defines nothing and the slot stays hidden.
  - **Portrait: the Duel Computer's own** (same day). `PORTRAIT_FROM_SLOT(page, slot)` (bit 15 set in
    `ModOpponent.portrait`) copies the portrait circle of an original page's slot: `DrawSlotPortrait` reads that
    page's bitmap and palette (`gOpponentSelectPageBgs`, raw 8bpp) pixel for pixel over the same circle, and maps
    each colour to the current page with `PortraitColor` (the nearest entry in use, or a free one: the card-art
    portraits' rule, now shared with `DrawArtPortrait`). The Duel Computer is the original page 4, right slot
    (`gOpponentSelectDuelists` in the ROM: page 4 = 0, 22, 23, 24, 21). A preview rendered from the built ROM's page
    bitmaps showed it cleanly in page 3's bottom circle.

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
