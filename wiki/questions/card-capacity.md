---
title: How many cards can be added?
type: question
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-27
---
# How many cards can be added without moving RAM?

**Answer (2026-10-27):** **2** new collectable cards with no RAM changes, or **8** if Deck Edit also hides the
unplayable Ticket and God cards (a one-constant code change). The limit is one fixed-size RAM list in
[[deck-edit-c]], not the save, the card-ID encoding or the ROM.

## Limits, from loosest to tightest
| Limit | Capacity | Used | Spare | Evidence |
|---|---|---|---|---|
| Card-ID encoding (`& 0x7FF` everywhere, 11–12-bit zone words) | 2047 IDs | 820 | 1227 | [[card-table]], [[card-id-map]] |
| Save trunk `gSaveData.trunk[0x800]` (+0x8..+0x2008) | 2047 IDs | 820 | 1227 | `include/save.h` (size asserts); the save already reserves every ID |
| Card numbers (`gCardNumberToId[2048]`) | 0..1999 | 811 + 162 reserved | 943 usable (see below) | [[card-id-map]] |
| Deck Edit list `gDeckEdit.lists[row].trunk[821]` | 821 entries | **819** (Deck Edit mode) | **2** | `include/deck_edit.h` `struct DeckEditListRow` |
| same list, Prohibition picker (mode 2) | 821 | 813 | 8 | `DeckEdit_BuildCardLists`, `src/deck_edit_cards.c` |

`DeckEdit_BuildCardLists` writes one entry per qualifying card ID into the list:
- mode 0 (Deck Edit): every owned ID whose card number is outside 1920–1999. With a full collection that is
  819 (all but the token);
- mode 1 (Card Trading): also hides numbers 1210–1899, so it holds fewer;
- mode 2 (Prohibition picker): every ID whose number is outside 1900–1999, owned or not: 813.

The struct has no slack. `trunk[821]` is followed directly by `deck[80]` and `side[15]`. Row 1 ends at `+0x1494`
(`listCount`), and `gDeckEdit` (`0x0201DB20` + 0x1C5C) ends 4 bytes before `gCardTrading` (`0x0201F780`). The
823rd listed card would overwrite `deck[0]` of the same row and corrupt the deck list built in the same loop.
`DeckEdit_FilterAndSortList` reaches the rows at fixed negative offsets from `gDeckEditSortScratch`, and
`src/deck_edit_cards.c` / `src/deck_edit_stats.c` keep local views with a hard-coded row stride (0x394 halfwords).
So growing the list means moving RAM offsets.

**Variant (+8):** if the mode-0 test excludes 1900–1999 (as mode 2 already does) instead of 1920–1999, the
3 Tickets and 3 Gods (unplayable, see [[cards]]) leave Deck Edit, and both modes then list 813 cards.

## Moving the list to free EWRAM
`0x02028380`–`0x0202FFFF` (0x7C80 bytes, 15,936 `u16` slots) has no references found by static analysis
([[ram-map]]). It lies between the end of `gSceneWork`'s largest view and `gScratchBuffer`. Two trunk-list rows
for every possible ID (2 × 2047 × 2 = 0x1FFC bytes) fit easily, and so would whole `DeckEditListRow`s.
Moving the list means rewriting the accessors (`DeckEdit_GetListCard` / `DeckEdit_SetListCard`), the local list
views in `src/deck_edit_cards.c` and `src/deck_edit_stats.c`, and the negative-offset pointers in
`DeckEdit_FilterAndSortList`. Those are code changes only, and no other RAM moves. The Prohibition and Card Trading
modes use the same rows.

### Binary patch map (2026-10-27, checked by CPU emulation)
Only the two trunk rows move: row 0 to `0x02028380`, row 1 to `0x02029380`, with 0x1000 bytes (2048 IDs) each.
The deck and side lists stay where they are. Eight functions address the trunk rows, and nothing else does
(every base+offset literal pair in the `gDeckEdit` range was checked):
- `GetListCard`, `SetListCard`, `GetActiveListRow`, `UpdateNameIndexLetters`, `GetSelectedCardCopies` and
  `DeckStats_CountCategory` use the literal `0x644` (becomes `0xA860`) and a `movs #0xE5; lsls #3` stride
  (becomes `movs #1; lsls #12`). Two trunk cases in `GetSelectedCardCopies` share the stride register `r6`, so
  there `adds r1,r3,#0; muls r1,r6` becomes `lsls r1,r3,#12; movs r1,r1`.
- `FilterAndSortList` uses two offsets from `gDeckEditSortScratch`.
- `BuildCardLists` uses an absolute row literal in modes 0/1, an offset from `listCount` in mode 2, and `r9` plus
  the stride in the re-filter loop. Mode 1 also derives `listCount` and the side-row pointer from the row
  literal; those are rebuilt with two offset literals and `subs r0,r3,#2`.
- Optional: the List Filter non-monster scratch (`sortScratch`, 310 entries; 226 used today) moves to
  `0x0202A380` through two literals.

The card count is 13 `movs #0xCD; lsls #2` pairs (= last ID), 3 `GeneratePackCards` literals (last ID + 1),
3 debug-browser literals and the `IsCardCollectionComplete` end pointer. The last ID must be `imm8 << n`. To get
another count, pad with dummy IDs numbered 1920–1999, which every list and pack hides.

Test: Unicorn ran the original and the relocated ROM from 200 random valid saves through `BuildCardLists`
(all modes), `GetSelectedCardCopies`, `DeckStats_CountCategory`, `FilterAndSortList`, `GetActiveListRow` and
`UpdateNameIndexLetters`. Results were identical, the old rows stayed untouched, and the deck and side lists
were unchanged. With the full patch for last ID 1020, Deck Edit listed 1018 IDs.

## Maximum card count with the patch (2026-10-27)
**Answer: last ID 1640 (820 new IDs), keeping the name table in place. The hard ceiling is last ID 2040.**

| Limit on the last card ID | Value | Why |
|---|---|---|
| ID encoding | 2047 | 11-bit `& 0x7FF`; ID 0 = none, and 2048 wraps to 0 |
| Loop-bound encoding | **2040** | the 13 bounds are `movs #imm8; lsls #n` pairs, so the last ID must be `imm8 << n`: any value up to 255, even values up to 510, multiples of 4 up to 1020, multiples of 8 up to 2040. Reaching 2047 would also mean changing each loop's branch condition |
| RAM (relocated rows, save trunk, moved sort scratch) | 2047 | 2048 entries each |
| Names kept in place (grow into the unused name bank `0x08239460`) | **1641 → 1640** | `(0x082461A0 − 0x0822C720) / 0x40` = 1642 entries; 1640 is the largest encodable value below it. `card_expand.py` refuses anything higher. Going past it means moving `gCardNames` (58 pointers), which was decided against |
| ROM size | 16 MiB up to about 1,880 cards; 32 MiB for 2040 | art and palettes are 0x1160 bytes per card, from `0x08800000` |

At last ID 1640 the art and palettes end at about `0x08EF6060`, so a 16 MiB ROM is enough.

### Card-number budget
Each card also needs a card number (decks, packs, effects and same-name checks use it; see [[card-id-map]]).
Every number below is a value the original game neither uses nor tests. Counted from the ID→number table, the
effect-table keys (`0x0819A9D4`) and every `CARD_*` constant in `include/constants/cards.h`. That header
names 162 numbers with no EDS card that code still reacts to: 136 effect keys, battle and summon checks, and
the same-name pairs 1034 (Polymerization), 1210 (Dark Magician) and 1249 (Harpie Lady).

| Numbers | Free | Use |
|---|---|---|
| 0–1209 | **405** | Ordinary cards. |
| 1210–1899 | **538** | Ordinary, except that Card Trading (link) hides this range (`DeckEdit_BuildCardLists` mode 1). |
| 1900–1919 | 14 | Avoid: Tickets (1901–1903) and Gods (1910–1912) live here, and the Prohibition picker hides the range. |
| 1920–1999 | 79 | Hidden from every list and pack: padding IDs only. |
| base + 2000 | one per card | Alternate art (below). |

So there are **943 numbers for distinct new cards**, which covers the 820 new IDs available at last ID 1640.
Only past about 1763 (820 + 943) would the extra IDs have to be alternate-art copies.

### Alternate art
- `CardNumberToId` maps a number of 2000 or more to `gCardNumberToId[number − 2000] + 1`. The 2048-entry
  number→ID table has no entry for alternates; they borrow the base card's entry.
- So an alternate must have the card ID directly after its base: base at ID k, alternate at ID k + 1 with number
  base + 2000. Each base can have only one alternate.
- `IsSameCardName` (`src/card_detail.c`) folds number − 2000 into the base number, so effects that act on every
  copy of a card treat the two as one card. An alternate slot therefore can't hold an unrelated card.
- Existing IDs are packed in alphabetical order, so an existing card can't get a new alternate without
  renumbering every later ID, which would break saves. In practice, alternates are pairs of new IDs.

> [!question] Confirm at runtime
> Run a write watchpoint on the range with the emulator harness ([[emulator]]) through Deck Edit, packs, the
> Exodia/Destiny Board scenes, a campaign duel and a link screen.

## What adding cards needs anyway (no RAM involved)
- **ROM tables.** The per-card block (`0x0822C720`–`0x08625460`, see [[card-table]]) is packed with no gaps, and
  data is not relocatable yet: edited items must fit their original slots ([[assets]]). Several tables can still
  grow in place:
  - names, into the unused all-zero name bank (up to ID 1641);
  - descriptions, into the old art once the art has moved;
  - ID→number, into the old passwords (up to ID 2463);
  - number→ID, which is fixed at 2048 entries.

  Stats (499 pointers), passwords (2), art (5) and palettes (5) have to move. [[card-expand]] puts stats and
  passwords in the old palette area and art and palettes at `0x08800000`, past the 8 MiB end. A card costs about
  5.3 KiB, mostly art (0x10E0 + 0x80 palette + 0x40 name + 0x1E0 text + 12 bytes).
- **Loop bounds.** Every loop over card IDs uses the ROM constant `0x334`: `DeckEdit_BuildCardLists`,
  deck-edit stats and prohibit code, `IsCardCollectionComplete`, `FindCardByPassword` (`src/link_sio.c`),
  `DebugGetAllCards` (`src/sprite.c`), the [[card-detail-c]] browsers and the random packs in `GeneratePackCards`.
- **Effects.** `FindCardEffect` binary-searches a hard-coded 426-row table ([[cards]]). A new effect card needs
  a new row and a new search bound; a normal monster needs none.

> [!question] Not checked
> Only loops whose bound is a literal 0x334/820/821 were searched. A duel or AI buffer indexed by card ID with
> another size would not show up. None is known.

Tools: [[card-expand]] (`tools/card_expand/`) generates and tests the patch.

Method: read `include/save.h`, `include/deck_edit.h`, `DeckEdit_BuildCardLists` and `DeckEdit_FilterAndSortList`;
grepped `src/` and `asm/` for 0x334/0x335/820/821 bounds.

Related: [[cards]], [[card-table]], [[card-id-map]], [[deck-edit-c]], [[save-game]], [[ram-map]].
