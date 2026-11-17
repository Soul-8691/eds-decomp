# Editing with HMA-CE: screenshots

What the mod's data looks like in Hex Maniac Advance CE with the build's `.toml`. Setting it up, and the list of
anchors, is in the README's [Editing with Hex Maniac Advance](README.md#editing-with-hex-maniac-advance) (use HMA CE
built from source, `upcoming` branch). [README_TODO.md](README_TODO.md) says what each picture should show.

<p align="center">
  <img src="screenshots/HMA%20TOML.png" width="480" alt="A build's generated .toml open in VS Code: its header lists each anchor and what it holds"><br>
  <sub>The build's <code>.toml</code>: every anchor, and what it holds</sub>
</p>

## The patched ROM in HMA

<table>
<tr>
<td align="center"><img src="screenshots/gifs/hma.gif" width="240" alt="Hex Maniac Advance CE with eds_demo.toml loaded, stepping through added cards in data.cards.stats"><br><sub>HMA with the TOML</sub></td>
<td align="center"><img src="screenshots/gifs/hma-goto.gif" width="240" alt="HMA's Goto panel opening the data, graphics and text folders and the data.cards and data.decks anchors"><br><sub>HMA: the anchors in Goto</sub></td>
</tr>
</table>

## Formats, decks and recipes

<table>
<tr>
<td align="center"><img src="screenshots/gifs/hma-deck-choices.gif" width="240" alt="data.decks.choices.goat in HMA: the format's deck slots and their chances out of 100"><br><sub>A format's deck chances</sub></td>
<td align="center"><img src="screenshots/gifs/hma-recipes.gif" width="240" alt="data.decks.recipes in HMA (name, DeckList, Side Deck, side count), then recipe names in text.decks.recipes"><br><sub>The premade Deck Recipes</sub></td>
<td align="center"><img src="screenshots/gifs/hma-unlocks.gif" width="240" alt="The unlock win constants in HMA: data.constants.formats.unlock.* in Goto, their bytes, and data.constants.packs.format.usagewins"><br><sub>The unlock wins</sub></td>
</tr>
</table>

## Prices and packs

<table>
<tr>
<td align="center"><img src="screenshots/gifs/hma-prices.gif" width="240" alt="data.prices.cards in HMA: a format's row, then its cards and DP prices"><br><sub>A format's card prices</sub></td>
<td align="center"><img src="screenshots/gifs/hma-pack-pools.gif" width="240" alt="data.packs.format.pools in HMA: a usage pack's cards and pull weights"><br><sub>A pack's cards and weights</sub></td>
<td align="center"><img src="screenshots/gifs/hma-pack-covers.gif" width="240" alt="Format pack covers (graphics.packs.format.*) in the Image tool, then data.packs.format.names"><br><sub>Pack names and covers</sub></td>
</tr>
</table>

## The added cards

<table>
<tr>
<td align="center"><img src="screenshots/gifs/hma-card-stats.gif" width="240" alt="data.cards.stats on added cards in HMA, with each card's name, text, password, Link Arrows and extended type alongside"><br><sub>The added cards' stats</sub></td>
<td align="center"><img src="screenshots/gifs/hma-card-text.gif" width="240" alt="data.cards.names and data.cards.descriptions in HMA's grid view, filtered by name and by text"><br><sub>Card names and text</sub></td>
<td align="center"><img src="screenshots/gifs/hma-linkmarkers.gif" width="240" alt="data.cards.linkmarkers and data.cards.typeex in HMA on Stardust Dragon, Utopia, Link Spider and Proxy Dragon"><br><sub>Link Arrows</sub></td>
</tr>
</table>

HMA-CE can also fill in a card's stats and art from YGOProDeck: **Import from YGOProDeck**.

<p align="center">
  <img src="screenshots/gifs/hma-import-ygoprodeck.gif" width="240" alt="HMA-CE's Import from YGOProDeck importing Red Gadget over an added card (ID 1127): the prompt, the result, its stats, art and Deck Edit icon"><br>
  <sub>Import from YGOProDeck (GIF)</sub>
</p>

## Opponents and their decks

<table>
<tr>
<td align="center"><img src="screenshots/gifs/hma-opponents.gif" width="240" alt="data.opponents in HMA: each opponent's kind, unlock order, format, name and portrait card"><br><sub>The opponents table</sub></td>
<td align="center"><img src="screenshots/gifs/hma-portrait.gif" width="240" alt="Opponent portraits (graphics.opponents.<name>.face) in HMA's Image tool"><br><sub>An opponent's portrait</sub></td>
<td align="center"><img src="screenshots/gifs/hma-opponent-deck.gif" width="240" alt="An opponent's deck in HMA: data.decks.mod.goatchaoscontrol's DeckList and card list"><br><sub>An opponent's deck</sub></td>
</tr>
</table>

HMA-CE can also fill a deck table from a `.ydk` deck file: **Import .YDK**.

<p align="center">
  <img src="screenshots/gifs/hma-import-ydk.gif" width="240" alt="HMA-CE's Import .ydk loading a .ydk file into an opponent's deck table"><br>
  <sub>Import .YDK (GIF)</sub>
</p>

## Screens and art

<table>
<tr>
<td align="center"><img src="screenshots/gifs/hma-preduel-text.gif" width="240" alt="data.preduel.text in HMA's grid view: each opponent's five pre-duel lines"><br><sub>Pre-duel text</sub></td>
<td align="center"><img src="screenshots/gifs/hma-preduel-picture.gif" width="240" alt="Pre-duel pictures (graphics.preduel.<format>.picture) in HMA's Image tool"><br><sub>A pre-duel picture</sub></td>
<td align="center"><img src="screenshots/gifs/hma-title.gif" width="240" alt="The title screen graphics (graphics.title.*, the medal) and text.title in HMA"><br><sub>The title screen</sub></td>
</tr>
<tr>
<td align="center"><img src="screenshots/gifs/hma-miniatures.gif" width="240" alt="In-duel card art (graphics.cards.fieldicons.miniatures) in HMA's Image tool"><br><sub>In-duel card art</sub></td>
<td align="center"><img src="screenshots/gifs/hma-ring-art.gif" width="240" alt="Deck Edit's ring art (graphics.cards.icons.images with its per-card palettes) in HMA's Image tool"><br><sub>Deck Edit ring art</sub></td>
</tr>
</table>
