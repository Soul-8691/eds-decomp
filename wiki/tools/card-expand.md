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
| `test_relocation.py` | A Unicorn CPU test of the list code. Over 200 random valid saves, the original ROM and the relocated ROM give identical results. With the full patch for last ID 1020, Deck Edit lists 1018 cards. Needs `pip install unicorn`. |

```
python3 tools/card_expand/refs.py                          # after make compare
python3 tools/card_expand/card_expand.py 1020 --rom eds.gba --out build/card_expand/patched.gba --move-scratch
python3 tools/card_expand/test_relocation.py --seeds 200
```

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
