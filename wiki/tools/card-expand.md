---
title: Card expansion patch tools
type: tool
status: draft
confidence: high
sources: [rom-analysis]
updated: 2026-10-27
---
# Card expansion patch tools (`tools/card_expand/`)

Binary patch map for raising the card-ID limit past 820, derived and checked in [[card-capacity]]. The tools
patch a built `eds.gba`; they don't change the decomp sources.

| File | What it does |
|---|---|
| `card_expand.py LAST_ID` | Writes every ROM change (address, old bytes, new bytes, reason) to `build/card_expand/patches.csv`. With `--rom eds.gba --out FILE` it checks each site's old bytes and writes a patched copy. `--move-scratch` also moves the List Filter non-monster buffer; `--ram-only` applies just the Deck Edit list move. |
| `refs.py` | Rebuilds `card_table_refs.csv`: every literal-pool word in `build/eds.elf` that points into the per-card block, plus aligned words in `.rodata` and the game tables. It needs `arm-none-eabi-objdump` and a matching build. |
| `card_table_refs.csv` | The 1,275 pointers by table, index and function (committed: addresses only, no game data). |
| `write_tables.py [LAST_ID]` | Builds an expanded ROM from the original one, adding the cards named in `new_cards.txt` (repo root) as IDs 821, 822, … in list order. Each name is looked up on the [YGOPRODeck API](https://ygoprodeck.com/api-guide/) (`cardinfo.php` v7, 30 exact names per request), and the data and cropped art are cached in `assets/new_cards/` (git-ignored). It applies the code patches (or accepts a ROM that already has them), grows the ROM to the next power of two (16 MiB up to last ID 1640), and writes every card table at its new address. The existing cards come from `assets/cards/`, encoded by the converters in `tools/assets.py`. `LAST_ID` defaults to the smallest encodable value, with hidden placeholders (number 1999) in the gap, and `--move-scratch` turns on by itself past 310 non-monsters. The script reports whether existing cards are byte-identical to the original and warns about card numbers that clash, are reserved by code, or are hidden from Card Trading. |
| `test_tables.py` | A Unicorn test of a written ROM. For IDs 1–820 the game's own readers must give the same results as on the original: `LoadCardPicture` (VRAM and palette), `UnpackCardArt8bpp`, `IsEffect/Fusion/SpecialSummonOnly`, `FindCardByPassword`, `IsSameCardName`, and the Deck Edit lists. `--clone NEW=OLD` checks a new card made as a copy of OLD. |
| `test_relocation.py` | A Unicorn CPU test of the list code. Over 200 random valid saves, the original ROM and the relocated ROM give identical results. With the full patch for last ID 1020, Deck Edit lists 1018 cards. Needs `pip install unicorn`. |

```
python3 tools/card_expand/refs.py                          # after make compare
python3 tools/card_expand/card_expand.py 1020 --rom eds.gba --out build/card_expand/patched.gba --move-scratch
python3 tools/card_expand/test_relocation.py --seeds 200
# edit new_cards.txt, then (pip install pillow once; network needed the first time):
python3 tools/card_expand/write_tables.py --rom eds.gba --out build/card_expand/expanded.gba
python3 tools/card_expand/test_tables.py --new build/card_expand/expanded.gba
```

### Adding cards (`new_cards.txt` → `write_tables.py`)
One exact YGOPRODeck name per line. `Name | shown name` sets the name shown in the game (at most 41 characters,
the width of a Deck Edit row). After a second `|`, `key=value` settings override or supply API values: `atk`,
`def`, `level`, `attr`, `race`, `number`, and `password` (`none` for no password). Tokens need these settings,
because the API gives them no attribute, level, ATK, or DEF. The API data is converted as follows:
- **Stats.** ATK and DEF are divided by 10. `race` and `attribute` map to the `type` and `attr` enums on
  [[card-table]]. `kind` is 2 for Fusion, 3 for Ritual, 0 for Normal and Token, and 1 for everything else.
  Spells (type 22, attr 0) and Traps (type 21, attr 1) keep their subtype in bits 17–19. Pendulum, Synchro, XYZ,
  and Link monsters, and the races and attributes EDS lacks, are rejected.
- **Password.** The 8-digit passcode. A passcode that an existing card already has is an error, because it means
  the same card under its TCG name (for example, Anti-Spell Fragrance is EDS's Anti-Magic Fragrance, ID 25).
- **Text.** Reduced to one line of printable ASCII: curly quotes, dashes, and bullets are mapped, and accents are
  stripped. Text longer than 479 bytes is cut at a word, with a warning.
- **Art.** `image_url_cropped` is downloaded once to `img/PASSCODE.jpg`, centre-cropped to 72:80, resized
  (Lanczos), reduced to 15-bit colour, and median-cut to 64 colours in `art/PASSCODE.png`. An existing PNG is
  kept, so a hand-made replacement wins.
- **Card number.** The next free number in 0–1209, then 1210–1899. Numbers that code still tests (`/* no EDS card`
  in `include/constants/cards.h`) are skipped. Tokens get 1921–1998, which every list and pack hides, like Insect
  Monster Token. `number=` can pick one of the reserved effect keys on purpose.

`assets/new_cards/cardinfo.json` holds the API fields per listed name. Edit it and rerun to fix a value, or run
with `--offline` to make no requests and `--refresh` to fetch everything again. `cards.csv` lists the ID, number,
password, and stats written for each new card. New cards have no effect-table rows, so effect monsters, spells,
and traps do nothing in a duel.

Notes:
- `IsCardCollectionComplete` counts every ID up to `LAST_ID`, including `--pad` placeholders.
- `test_tables.py` compares the List Filter's ATK sort as a set, because the game's quicksort is unstable and
  extra cards can reorder ties.

Checked 2026-10-27:
- Unchanged assets at last ID 820: tables byte-identical, all readers identical. A ROM with only the code
  patches (tables not written) fails the test.
- 4 cloned cards (821–824): readers identical, clones match their sources, passwords find the new IDs.
- Last ID 1640 with 816 placeholders: 16 MiB ROM; only the 4 real cards appear in the lists.
- `new_cards.txt` (314 cards, last ID 1136, 2 placeholders, 361 non-monsters, so the buffer moves): the existing
  tables stay byte-identical and `test_tables.py` passes. Deck Edit, Trading, and Prohibition list 313 of the
  316 added IDs; the two placeholders and Sheep Token (number 1921) are hidden. A second Unicorn pass over the
  314 new IDs found every password with `FindCardByPassword`, gave the right fusion flag, and unpacked non-blank art.

## What the patch does and doesn't do
- **Does:** moves the Deck Edit trunk rows to `0x02028380` (2048 IDs per row), sets the card-count constants to
  `LAST_ID`, and repoints the stats, password, art and palette tables to the layout in `layout()`.
- **Doesn't:** write the grown tables or pad the ROM past 8 MiB, add effect-table rows, or assign card numbers.
- `LAST_ID` must be `imm8 << n`; otherwise pad with dummy IDs numbered 1920–1999. Because names grow in place,
  the largest value accepted is **1640** (820 new IDs, 16 MiB ROM). See [[card-capacity]] for the limits and the
  943 free card numbers.
- Sites are hard-coded addresses in the USA ROM. If a code unit changes size, re-derive them; `refs.py` covers the
  table pointers, but the list and count sites in `card_expand.py` are hand-derived.

Notes:
- The patched copies (`*.gba`) and CSVs are written under `build/`, which is git-ignored.
- This checkout's `tools/dr` has CRLF line endings, so `tools/dr` fails in WSL; call `python3` directly from
  the repo root.

Related: [[card-capacity]], [[ram-map]], [[card-table]], [[emulator]].
