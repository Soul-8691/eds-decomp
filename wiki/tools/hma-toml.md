---
title: Hex Maniac Advance metadata for the demo (tools/hma_toml.py)
type: tool
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-06
---
# Hex Maniac Advance metadata for the demo

`tools/hma_toml.py` writes `eds_demo.toml` next to `eds_demo.gba` on every `make demo` (not `make release-demo`),
from `baserom.toml`, the [Hex Maniac Advance](https://github.com/haven1433/HexManiacAdvance) metadata of the
original ROM (repo root). Without `baserom.toml` it does nothing. `eds_demo.toml` is git-ignored.

## How each entry moves
- **NamedAnchors.** The original address is placed in the source: a data label of `data/*.s` (`Name: @ 0x08xxxxxx`,
  its extent up to the next such label), or one of the card tables that moved to the end of the ROM
  (`config/assets.tsv` gives their original ranges; `MOVED_TABLES` names `gCardDescriptions`, `gCardStats`,
  `gCardIdToNumber`, `gCardPasswords`, and `cards/art` splits into `gCardArtGfx` / `gCardArtPalettes`). The new
  address is that symbol in `eds_demo.sym` plus the same offset.
  - Counts: `data.cards.numbers` (by card ID) gets CARD_ID_COUNT; `data.cards.internalnumbers` (by card number) the
    highest card number in use + 1 (from `newcards__number_to_id.bin`). Tables sized by a list (`]cardnumbers`)
    follow the list.
- **MatchedWords.** The original address is placed in a function (`config/functions.tsv`), found again in the
  `.sym`; the original instruction is read from `baserom.gba` (read only).
  - An unchanged function: the same instruction at the same offset.
  - A changed one: the same opcode bits and immediate (any register), the closest to the old offset.
  - The card count limits (`data.constants.cards.limit`, `.limit2`): the original forms (`movs #0xCD; lsls #2` =
    820, or a literal 821) can't hold the demo's values, so the script finds the literal holding CARD_ID_COUNT - 1 or
    CARD_ID_COUNT (Length 2). Functions rewritten without the constant (Deck Edit's and Card Trading's list
    builders now walk `gCardIdsByName`) are left out and listed in a comment at the top of the file.
  - Constants in data (the pack rarity weights) move like anchors.
- **Lists.** `cardnumbers` (by card ID) is cut or grown to CARD_ID_COUNT and `cardinternalnumbers` (by card number)
  gets the added cards, both with their in-game names from `build/new_cards.tsv`. The other lists are kept.
- **DefaultHash** is recomputed for every changed format and list with HMA's `StoredList.GenerateHash` (SHA-256 of
  each item, folded into a rotating seed 0xCAFEBABE; checked against three of `baserom.toml`'s hashes). HMA keeps an
  entry whose hash doesn't match its own reference as a user edit.

## Checked (2026-11-06)
- Five anchors moved (the card images, palettes, stats, ID-to-number table, passwords); every other anchor kept its
  address, and the data there is the same as the original's but for tables the mod changes (the effect table's
  function pointers, two starter-deck stats).
- Every kept MatchedWord reads the expected value in the demo ROM (the limits 1006 / 1007, the pack and Fusion
  constants unchanged); 7 card-limit sites were left out.
- HMA's metadata parser cuts every line at `#`, so the header comments are fine (no card name holds a `#`).
- Not opened in HMA yet.
