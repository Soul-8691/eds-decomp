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

Every slot has a placeholder picture in [`screenshots/placeholders/`](screenshots/placeholders/) (named after the slot;
the ones README.md used to show are marked "was").

---

## Header

- **Have:** `screenshots/Title Screen.png` (centred, width 400).
- [ ] 🎬 **Trailer** · `trailer.mp4` · above the nav links, replacing the title screenshot. 30-60 s: title screen,
  picking a format duelist, the pre-duel screen, a duel with a Synchro or Link Summon, opening a pack.
- [ ] 🖼️ **A duel in progress** · `screenshots/Duel.png` · beside the title screen (two images of width 400). A
  mid-game board with card art on the field, both life points and the phase panel. *(was `placeholders/duel-field.svg`)*

## Highlights

No media: the table stays text. Optional:

- [ ] 🎞️ **Highlights strip** · `screenshots/gifs/highlights.gif` · under the table. 4-6 one-second clips: a format
  duelist, a Synchro Summon, a pack opening, Deck Recipes, the CPU chaining a Trap.

## Play it

- [ ] 🖼️ **Rom Patcher JS, ready to patch** · `screenshots/Rom Patcher.png` · after step 4. Both files chosen, the
  Apply button showing (a browser screenshot, not mGBA).

## The Campaign

- **Have:** `Duelists (Screen 1-3).png` (a row of three).
- [ ] 🎞️ **Unlocking a format** · `screenshots/gifs/unlock.gif` · under the unlock paragraph. The 20th win, then the next
  format duelist appearing on the select screen.

### Format duelists

- [ ] 🎞️ **Pre-duel screen** · `screenshots/gifs/pre-duel.gif` · beside the Pre-duel screen bullet. A bust-up scene
  with blinking eyes, the dialogue text and the QR code; then **A**.
- [ ] 🖼️ **Format picture pre-duel screen** · `screenshots/Pre-duel (Yugi-Kaiba).png` · next to the GIF. A format
  picture (Yugi-Kaiba through Android) with the QR code.
- [ ] 🖼️ **Format check warning** · `screenshots/Format Check.png` · after the Your deck bullet. The message for a deck
  that isn't legal in the format. *(was `placeholders/format-check.svg`)*

### Best-of-3 matches

- [ ] 🎞️ **Siding between games** · `screenshots/gifs/siding.gif` · under the paragraph. Swapping a card between the
  Main and Side Deck after game 1. *(was `placeholders/side-deck.svg`)*

### Event challengers

- [ ] 🖼️ **Deck Recipes before an event duel** · `screenshots/Event Challenger.png` · under the second paragraph.
  *(was `placeholders/event-challenger.svg`)*

### Practice opponents

- **Have:** `Duelists (Screen 4).png` (right-aligned, width 240).
- [ ] 🖼️ **Picking your deck against Debug** · `screenshots/Debug Picker.png` · after the Your deck against Debug
  bullet. *(was `placeholders/debug-picker.svg`)*
- [ ] 🖼️ **A duel with Debug** · `screenshots/Debug Duel.png` · next to it: Debug's hand face up.
  *(was `placeholders/debug-duel.svg`)*
- [ ] 🎞️ **Mulligan** · `screenshots/gifs/mulligan.gif` · after the Mulligan bullet. Opening the Main Deck in the first
  Draw Phase, Mulligan twice (the hand changes), then Draw.

## Decks and banlists

- **Have:** `Deck Recipes (Main Menu).png`, `Deck Recipes.png`, `Deck Edit.png`, `Banlists (Main Menu).png`,
  `Banlists.png` (a 3 + 2 table).
- [ ] 🖼️ **Cards a recipe left out** · `screenshots/Left Out.png` · the empty sixth cell of the table.
  *(was `placeholders/left-out.svg`)*

### Deck Edit

- [ ] 🎞️ **List Filter** · `screenshots/gifs/list-filter.gif` · after the List Filter bullet. Choosing a letter, a
  Type and an ATK range; the list narrowing.
- [ ] 🎞️ **Empty Main/Side** · `screenshots/gifs/empty-main-side.gif` · after its bullet. A full deck emptied into the
  trunk in one press.

### Banlists

- [ ] 🖼️ **A refused switch** · `screenshots/Banlist Refused.png` · under the paragraph. The message when your cards
  can't make 40 under the new banlist.

## Cards, packs and DP

- **Have:** `Booster Packs (Main Menu).png`, `Pack Opener.png`, `Password.png` (a row of three).
- [ ] 🎞️ **Opening a pack** · `screenshots/gifs/pack-pull.gif` · a second row of the table. Buying a usage-weighted pack
  and the pulls. *(was `placeholders/pack-pull.svg`)*
- [ ] 🖼️ **The free pack reward** · `screenshots/New Packs.png` · beside the GIF. The "new packs are available" message
  after a win. *(was `placeholders/new-packs.svg`)*

### Usage stats and Mobius

- [ ] 🖼️ **Mobius** · `screenshots/Mobius.png` · after the paragraph, before the details block. The usage table for one
  format. *(was `placeholders/mobius.svg`)*
- [ ] 🖼️ **A usage chart** · `screenshots/Mobius Chart.png` · inside "Running Mobius", after the Charts bullet. The
  output of `graph_card_usage.py "Monster Reborn"`.

## In the duel

- **Have:** `Card Icons.png`, `Card Icons (Up Close) - Summon-Set Order.png`, `Activate-Set Order.png`, `Counters.png`,
  `Settings (Main Menu).png`, `Settings.png` (two rows of three).
- [ ] 🖼️ **The chain list** · `screenshots/Chain List.png` · a third row of the table, with card art in the list.
  *(was `placeholders/chain-list.svg`)*

### The duel screen

- [ ] 🖼️ **The full board** · `screenshots/Duel Board.png` · beside the Layout bullet: the phase panel, both Extra
  Monster Zones, both life points.

### Menus and controls

- [ ] 🎬 **Duel speeds** · `duel-speed.mp4` · after the Settings table. The same turn at DEFAULT, ALWAYS and SUPER, side by
  side or one after another.
- [ ] 🎞️ **Hold R** · `screenshots/gifs/hold-r.gif` · after the Hold R bullet. An opponent's chain going by with no
  prompt.

### Smarter CPU

- [ ] 🎬 **The CPU at work** · `cpu-play.mp4` · under the paragraph. A clever CPU turn: a well-timed Trap, a chain, or a
  combo from one of the format decks. *(was `placeholders/cpu-play.svg`)*

## Synchro, Xyz and Link

- **Have:** `cards/Brionac, Dragon of the Ice Barrier.png`, `cards/Number 39_ Utopia.png`, `cards/Proxy Dragon.png`.
- [ ] 🎞️ **A Synchro Summon** · `screenshots/gifs/synchro.gif` · in the table's Synchro row, or a row of three GIFs
  above the card images. The Special Summon list, the Tuner and non-Tuner picks, Stardust Dragon arriving.
- [ ] 🎞️ **An Xyz Summon** · `screenshots/gifs/xyz.gif` · next to it. Two Level 4s becoming Utopia; "MAT 2" in the info
  bar.
- [ ] 🎞️ **A Link Summon** · `screenshots/gifs/link.gif` · next to it. Picking the materials, then an Extra Monster Zone;
  the arrows on the field.
- [ ] 🎞️ **Choosing a zone** · `screenshots/gifs/zone-pick.gif` · after the Choosing zones bullet. A Normal Summon with
  the zone cursor.

## The new cards

- **Have:** the six card images and the six more in "More cards".
- [ ] 🎬 **New cards in action** · `new-cards.mp4` · after the gallery. A few effects resolving: Breaker destroying a
  Spell/Trap, Cyber Dragon's Special Summon, Thousand-Eyes Restrict absorbing a monster.

## Building from source

### Hex Maniac Advance TOML

- [ ] 🖼️ **HMA with the TOML** · `screenshots/HMA.png` · after the paragraph. Hex Maniac Advance showing a labelled
  table from the TOML (a format pack pool or the title screen images).

---

## Layout notes

- Keep image rows to **three across** (width 240 each) and the header to **two** (width 400), as now.
- Captions go under each image as `<br><sub>…</sub>`, matching the existing tables.
- Every image gets real alt text describing what it shows.
- A GIF in a table cell works like an image (`<img src="….gif" width="240">`). A video can't go in a table: give it its
  own line, centred, with a one-sentence caption under it.
