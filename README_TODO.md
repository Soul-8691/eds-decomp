# README media plan (to do)

What each planned picture, GIF and video of [README.md](README.md) should show. [README_WIP.md](README_WIP.md) is
README.md with a placeholder in every slot listed here, in place. Each section below matches a README section. **Have**
lists the media the README already uses there. Each slot gives a suggested file name, where it goes, and what to
capture. When a slot is filled: replace its placeholder in README_WIP.md, copy it into README.md, and tick it here.

Slot types:

| Mark | Type | Recommended |
|---|---|---|
| 🖼️ | Image | mGBA screenshot (240 × 160), upscaled 2× with nearest-neighbour so it stays sharp; PNG in `screenshots/` |
| 🎞️ | GIF | 3-8 s, 2× nearest-neighbour, 15-30 fps, under 10 MB; `screenshots/gifs/` |
| 🎬 | Video | MP4, 15-60 s, 2× or 3×, game audio on; drag it into the README editor on GitHub (uploaded as a user attachment), under 10 MB |
| 🧰 | HMA | A Hex Maniac Advance window (not mGBA) with the build's `.toml` loaded, cropped to the table or image; PNG in `screenshots/hma/` |

Every slot has a placeholder picture in [`screenshots/placeholders/`](screenshots/placeholders/) (named after the slot;
the ones README.md used to show are marked "was"). The game's screenshots can be retaken with the mGBA scripts in
`tools/media/` ([wiki/tools/readme-media.md](wiki/tools/readme-media.md)).

---

## Header

- **Have:** `screenshots/Title Screen.png` and `screenshots/Duel.png` (centred, width 400).
- [ ] 🎬 **Trailer** · `trailer.mp4` · under the two header images. 30-60 s: title screen, picking a format duelist, the
  pre-duel screen, a duel with a Synchro or Link Summon, opening a pack.
- [x] 🖼️ **A duel in progress** · `screenshots/Duel.png` · beside the title screen. *(was `placeholders/duel-field.svg`)*

## Highlights

- [ ] 🎞️ **Highlights strip** · `screenshots/gifs/highlights.gif` · under the table. 4-6 one-second clips: a format
  duelist, a Synchro Summon, a pack opening, Deck Recipes, the CPU chaining a Trap.

## Play it

- [ ] 🖼️ **Rom Patcher JS, ready to patch** · `screenshots/Rom Patcher.png` · after step 4. Both files chosen, the
  Apply button showing (a browser screenshot, not mGBA).

## The Campaign

- **Have:** `Duelists (Screen 1-3).png` (a row of three).
- [ ] 🎞️ **Unlocking a format** · `screenshots/gifs/unlock.gif` · under the format table. The 20th win, then the next
  format duelist appearing on the select screen.

### Format duelists

- [ ] 🖼️ **Format check warning** · `screenshots/Format Check.png` · after the Portraits bullet. The message for a deck
  that isn't legal in the format. *(was `placeholders/format-check.svg`)*
- [ ] 🧰 **A format's deck chances** · `screenshots/hma/Deck Choices.png` · next to it. `data.decks.choices.goat` in
  HMA: the 10 deck slots and their chances out of 100.

### The pre-duel screen

- [ ] 🎞️ **Pre-duel screen** · `screenshots/gifs/pre-duel.gif` · under the paragraph. A bust-up scene with blinking
  eyes, the dialogue text and the QR code; then **A**.
- [ ] 🖼️ **Format picture pre-duel screen** · `screenshots/Pre-duel (Yugi-Kaiba).png` · next to the GIF. A format
  picture (Yugi-Kaiba through Android) with the QR code.

### Best-of-3 matches

- [ ] 🎞️ **Siding between games** · `screenshots/gifs/siding.gif` · under the paragraph. Swapping a card between the
  Main and Side Deck after game 1. *(was `placeholders/side-deck.svg`)*

### Event challengers

- [ ] 🖼️ **Deck Recipes before an event duel** · `screenshots/Event Challenger.png` · under the second paragraph.
  *(was `placeholders/event-challenger.svg`)*

## Practice opponents

- **Have:** `Duelists (Screen 4).png` (right-aligned, width 240).
- [ ] 🖼️ **Records: the practice page** · `screenshots/Records (Practice).png` · under the table. The Records screen's
  page with Solomon, Debug, Empty Deck and Test Duel.

### Solomon and Debug

- [ ] 🖼️ **Picking your deck against Debug** · `screenshots/Debug Picker.png` · after the Debug's rules bullet.
  *(was `placeholders/debug-picker.svg`)*
- [ ] 🖼️ **A duel with Debug** · `screenshots/Debug Duel.png` · next to it: Debug's hand face up.
  *(was `placeholders/debug-duel.svg`)*

### Empty Deck

- [ ] 🖼️ **A duel against Empty Deck** · `screenshots/Empty Deck.png` · under the paragraph. The CPU's empty field,
  hand and piles.

### Mulligan

- [ ] 🎞️ **Mulligan** · `screenshots/gifs/mulligan.gif` · under the paragraph. Opening the Main Deck in the first Draw
  Phase, Mulligan twice (the hand changes), then Draw.

## Decks and banlists

- **Have:** `Deck Recipes (Main Menu).png`, `Deck Recipes.png`, `Deck Edit.png`, `Banlists (Main Menu).png`,
  `Banlists.png` (a 3 + 2 table).
- [ ] 🖼️ **Cards a recipe left out** · `screenshots/Left Out.png` · the empty sixth cell of the table.
  *(was `placeholders/left-out.svg`)*

### Deck Recipes

- [ ] 🧰 **The premade Deck Recipes** · `screenshots/hma/Deck Recipes.png` · after the bullets. `data.decks.recipes`
  with `text.decks.recipes` (the names).

### Banlists

- [ ] 🖼️ **A refused switch** · `screenshots/Banlist Refused.png` · under the paragraph. The message when your cards
  can't make 40 under the new banlist.

### Deck Edit

- [ ] 🎞️ **List Filter** · `screenshots/gifs/list-filter.gif` · after the bullets. Choosing a letter, a Type and an ATK
  range; the list narrowing.
- [ ] 🎞️ **Empty Main/Side** · `screenshots/gifs/empty-main-side.gif` · next to it. A full deck emptied into the trunk
  in one press.

## Cards, packs and DP

- **Have:** `Booster Packs (Main Menu).png`, `Pack Opener.png`, `Password.png` (a row of three).
- [ ] 🎞️ **Opening a pack** · `screenshots/gifs/pack-pull.gif` · a second row of the table. Buying a usage-weighted pack
  and the pulls. *(was `placeholders/pack-pull.svg`)*
- [ ] 🖼️ **The free pack reward** · `screenshots/New Packs.png` · beside the GIF. The "new packs are available" message
  after a win. *(was `placeholders/new-packs.svg`)*

### Duelist Points (DP)

- [ ] 🧰 **A format's card prices** · `screenshots/hma/Prices.png` · after the bullets. `data.prices.cards` for one
  format.

### Booster Packs

- [ ] 🧰 **A pack's cards and weights** · `screenshots/hma/Pack Pools.png` · after the bullets.
  `data.packs.format.pools` with one usage pack open.
- [ ] 🧰 **Pack names and covers** · `screenshots/hma/Pack Covers.png` · next to it. `graphics.packs.format` and
  `data.packs.format.names`.
- [ ] 🧰 **The unlock wins** · `screenshots/hma/Unlocks.png` · next to it. `data.constants.formats.unlock` and
  `data.constants.packs.format.usagewins`.

### Usage stats and Mobius

- [ ] 🖼️ **Mobius** · `screenshots/Mobius.png` · after the paragraph, before the details block. The usage table for one
  format. *(was `placeholders/mobius.svg`)*
- [ ] 🖼️ **A usage chart** · `screenshots/Mobius Chart.png` · inside "Running Mobius", after the Charts bullet. The
  output of `graph_card_usage.py "Monster Reborn"`.

## In the duel

- **Have:** `Card Icons.png`, `Card Icons (Up Close) - Summon-Set Order.png`, `Activate-Set Order.png`, `Counters.png`,
  `Duel Board.png`, `Chain List.png` (two rows of three).
- [x] 🖼️ **The chain list** · `screenshots/Chain List.png`. *(was `placeholders/chain-list.svg`)*
- [x] 🖼️ **The full board** · `screenshots/Duel Board.png`. *(was `placeholders/duel-board.svg`)*

### Settings

- **Have:** `Settings (Main Menu).png`, `Settings.png` (with the two Hide switches).
- [ ] 🎬 **Duel speeds** · `duel-speed.mp4` · after the Settings paragraph. The same turn at DEFAULT, ALWAYS and SUPER,
  side by side or one after another.
- [ ] 🎬 **Hide card animations** · `hide-anims.mp4` · after it. The same activation with the setting off, then on.
- [ ] 🎞️ **Hide battle scenes** · `screenshots/gifs/hide-battle.gif` · after it. An attack played out on the field.

### Controls

- [ ] 🎞️ **Hold R** · `screenshots/gifs/hold-r.gif` · after the table. An opponent's chain going by with no prompt.

### Smarter CPU

- [ ] 🎬 **The CPU at work** · `cpu-play.mp4` · under the paragraph. A clever CPU turn: a well-timed Trap, a chain, or a
  combo from one of the format decks. *(was `placeholders/cpu-play.svg`)*

### Fixes to the original game

- [ ] 🎞️ **Call of the Haunted and Giant Trunade** · `screenshots/gifs/call-trunade.gif` · after the note. Trunade
  returns Call to the hand; its monster goes to the Graveyard, the Call only to the hand.
- [ ] 🎞️ **Graverobber's cost** · `screenshots/gifs/graverobber.gif` · next to it. A stolen Spell, Set, then activated:
  2000 LP paid.

## Synchro, Xyz and Link

- **Have:** `cards/Brionac, Dragon of the Ice Barrier.png`, `cards/Number 39_ Utopia.png`, `cards/Proxy Dragon.png`.
- [ ] 🎞️ **A Synchro Summon** · `screenshots/gifs/synchro.gif` · a row of three GIFs under the card images. The Special
  Summon list, the Tuner and non-Tuner picks, Stardust Dragon arriving.
- [ ] 🎞️ **An Xyz Summon** · `screenshots/gifs/xyz.gif` · next to it. Two Level 4s becoming Utopia; "MAT 2" in the info
  bar.
- [ ] 🎞️ **A Link Summon** · `screenshots/gifs/link.gif` · next to it. Picking the materials, then an Extra Monster Zone;
  the arrows on the field.

### Extra Monster Zones and choosing zones

- [ ] 🎞️ **Choosing a zone** · `screenshots/gifs/zone-pick.gif` · after the Choosing zones bullet. A Normal Summon with
  the zone cursor.

## The new cards

- **Have:** the six card images and the six more in "More cards".
- [ ] 🎬 **New cards in action** · `new-cards.mp4` · after the gallery. A few effects resolving: Breaker destroying a
  Spell/Trap, Cyber Dragon's Special Summon, Thousand-Eyes Restrict absorbing a monster.

### How they're added

- [ ] 🧰 **The added cards' stats** · `screenshots/hma/Card Stats.png` · after the bullets. `data.cards.stats` on an
  added card.
- [ ] 🧰 **Card names and text** · `screenshots/hma/Card Text.png` · next to it. `data.cards.names` and
  `data.cards.descriptions`.
- [ ] 🧰 **Link Arrows** · `screenshots/hma/Link Markers.png` · next to it. `data.cards.linkmarkers` and
  `data.cards.typeex` on Proxy Dragon.

## Editing with Hex Maniac Advance

- [ ] 🧰 **HMA with the TOML** · `screenshots/hma/HMA.png` · after the first paragraph. The patched ROM open in HMA with
  a labelled table.
- [ ] 🧰 **The anchors in Goto** · `screenshots/hma/Goto.png` · next to it. Goto showing the `data`, `graphics` and
  `text` folders.

### Opponents and their decks (after the table)

- [ ] 🧰 **The opponents table** · `screenshots/hma/Opponents.png`. `data.opponents`: kind, format, portrait.
- [ ] 🧰 **An opponent's portrait** · `screenshots/hma/Portrait.png`. `graphics.opponents.<name>` in the image editor.
- [ ] 🧰 **An opponent's deck** · `screenshots/hma/Opponent Deck.png`. `data.decks.mod.<deck>`: its card list.

### Screens and art (after the table)

- [ ] 🧰 **Pre-duel text** · `screenshots/hma/Pre-duel Text.png`. `data.preduel.text`: one opponent's five lines.
- [ ] 🧰 **A pre-duel picture** · `screenshots/hma/Pre-duel Picture.png`. `graphics.preduel.<format>` in the image
  editor.
- [ ] 🧰 **The title screen** · `screenshots/hma/Title.png`. `graphics.titlescreen.background` and `text.title`.
- [ ] 🧰 **In-duel card art** · `screenshots/hma/Miniatures.png`. `graphics.cards.fieldicons.miniatures`.
- [ ] 🧰 **Deck Edit ring art** · `screenshots/hma/Ring Art.png`. `graphics.cards.icons.images` and `.palettes`.

## Debug and testing builds

### Debug controls

- [ ] 🎞️ **Hold L** · `screenshots/gifs/hold-l.gif` · after the bullets. The CPU's turn passing with nothing played.
- [ ] 🎞️ **Rock-paper-scissors** · `screenshots/gifs/rps.gif` · next to it. Winning the rock-paper-scissors against
  Debug.

### Card test scripts and videos

- [ ] 🖼️ **The test runner** · `screenshots/Test Runner.png` · after the bullets. `run_tests.ps1` output: PASS /
  TIMEOUT per script (a terminal screenshot).
- [ ] 🖼️ **The video reviewer** · `screenshots/Video Reviewer.png` · next to it. `review_videos.py` with a video open
  and the Keep / Highlight / Throwaway buttons.
- [ ] 🎬 **The card test reel** · `highlights_4x.mp4` · under them. The highlighted test videos at 4x speed
  (`tools/testing_scenarios/highlights_4x.mp4`, about a minute).

---

## Layout notes

- Keep image rows to **three across** (width 240 each) and the header to **two** (width 400), as now.
- Captions go under each image as `<br><sub>…</sub>`, matching the existing tables.
- Every image gets real alt text describing what it shows.
- A GIF in a table cell works like an image (`<img src="….gif" width="240">`). A video can't go in a table: give it its
  own line, centred, with a one-sentence caption under it.
- HMA screenshots: crop to the table or image being shown, with the anchor name visible in the address bar.
