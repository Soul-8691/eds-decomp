"""Point HMA.md's placeholder images at the finished GIFs in screenshots/gifs/ (run from anywhere).

Each <img src="screenshots/placeholders/hma-X.svg" alt="Placeholder: ..."> whose screenshots/gifs/hma-X.gif exists
becomes <img src="screenshots/gifs/hma-X.gif" alt="<description>">. Placeholders without a GIF yet are left alone.
"""
import os
import re

DECOMP = r'C:\Users\pswan\Downloads\eds-decomp'
ALT = {
    'hma': 'Hex Maniac Advance CE with eds_demo.toml loaded, stepping through added cards in data.cards.stats',
    'hma-goto': "HMA's Goto panel opening the data, graphics and text folders and the data.cards and data.decks anchors",
    'hma-deck-choices': 'data.decks.choices.goat in HMA: the format\'s deck slots and their chances out of 100',
    'hma-recipes': 'data.decks.recipes in HMA (name, DeckList, Side Deck, side count), then recipe names in text.decks.recipes',
    'hma-unlocks': 'The unlock win constants in HMA: data.constants.formats.unlock.* in Goto, their bytes, and '
                   'data.constants.packs.format.usagewins',
    'hma-prices': "data.prices.cards in HMA: a format's row, then its cards and DP prices",
    'hma-pack-pools': "data.packs.format.pools in HMA: a usage pack's cards and pull weights",
    'hma-pack-covers': 'Format pack covers (graphics.packs.format.*) in the Image tool, then data.packs.format.names',
    'hma-card-stats': 'data.cards.stats on added cards in HMA, with each card\'s name, text, password, Link Arrows and '
                      'extended type alongside',
    'hma-card-text': 'data.cards.names and data.cards.descriptions in HMA\'s grid view, filtered by name and by text',
    'hma-linkmarkers': 'data.cards.linkmarkers and data.cards.typeex in HMA on Stardust Dragon, Utopia, Link Spider '
                       'and Proxy Dragon',
    'hma-import-ygoprodeck': 'HMA-CE\'s Import from YGOProDeck importing Red Gadget over an added card (ID 1127): the prompt, '
                             'the result, its stats, art and Deck Edit icon',
    'hma-opponents': 'data.opponents in HMA: each opponent\'s kind, unlock order, format, name and portrait card',
    'hma-portrait': 'Opponent portraits (graphics.opponents.<name>.face) in HMA\'s Image tool',
    'hma-opponent-deck': "An opponent's deck in HMA: data.decks.mod.goatchaoscontrol's DeckList and card list",
    'hma-import-ydk': 'HMA-CE\'s Import .ydk loading a .ydk file into an opponent\'s deck table',
    'hma-preduel-text': "data.preduel.text in HMA's grid view: each opponent's five pre-duel lines",
    'hma-preduel-picture': 'Pre-duel pictures (graphics.preduel.<format>.picture) in HMA\'s Image tool',
    'hma-title': 'The title screen graphics (graphics.title.*, the medal) and text.title in HMA',
    'hma-miniatures': 'In-duel card art (graphics.cards.fieldicons.miniatures) in HMA\'s Image tool',
    'hma-ring-art': "Deck Edit's ring art (graphics.cards.icons.images with its per-card palettes) in HMA's Image tool",
}

path = os.path.join(DECOMP, 'HMA.md')
text = open(path, encoding='utf-8').read()
done = []

def swap(m):
    name = m.group(2)
    if not os.path.exists(os.path.join(DECOMP, 'screenshots', 'gifs', name + '.gif')):
        return m.group(0)
    done.append(name)
    alt = ALT.get(name, m.group(4).replace('Placeholder: ', ''))
    return f'{m.group(1)}screenshots/gifs/{name}.gif{m.group(3)}{alt}{m.group(5)}'

text = re.sub(r'(<img src=")screenshots/placeholders/(hma[\w-]*)\.svg("[^>]*?alt=")([^"]*)(")', swap, text)
open(path, 'w', encoding='utf-8', newline='\n').write(text)
print(f'{len(done)} placeholders now GIFs: {", ".join(done)}')
left = re.findall(r'screenshots/placeholders/(hma[\w-]*)\.svg', text)
print(f'still placeholders: {", ".join(left) or "none"}')
