---
title: Xyz Summoning (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-06
---
# Xyz Summoning (mod)

Xyz Monsters and Xyz Materials, added on 2026-11-06. None of this is in the original game. The code is
`src/mod_xyz.c` (far code), `include/xyz.h`, the shared Extra Deck summon in `src/mod_synchro.c` ([[synchro]]), and the
three test cards in `src/added_effects.c`. Built (`make demo`); **not tested in an emulator**.

## Card data
- **`gCardTypeEx` kind 2** (`CARD_KIND_EX_XYZ`; `IS_XYZ_CARD`). `tools/newcards.py` accepts YGOPRODeck's "XYZ Monster"
  (still not Pendulum or Link). The stats word says kind FUSION, as for Synchros, so the engine keeps Xyz Monsters
  in the Fusion Deck (the Extra Deck) with no extra code, and its Level field holds the **Rank**.
- `IS_EXTRA_NON_FUSION` (Synchro or Xyz): the added cards' Fusion-only effects skip both (Magical Scientist,
  The Dark - Hex-Sealed Fusion, Necro Fusion).
- **Material rules.** `build/include/xyz_rules.h` (`XYZ_RULES`), from the text's first line: `N Level R monsters`,
  `N+ Level R monsters`, with an optional Type or Attribute (`2 Level 4 Warrior monsters`). Each row: card number,
  count, "or more", Level, type, attribute (0: any). Anything else warns and reads as "2 Level <Rank> monsters".

## Look
- **Frame:** the Fusion frame with its purple made dark grey/black, `gCardFrameXyzGfx` (`tools/synchro_frame.py
  build/assets/xyz_frame.bin xyz`). The three frame picks in Card View (`card_detail.c`), the duel card image
  (`card_canvas.c`) and the battle scene (`duel_card_anim.c`) now call `ModFusionKindFrame` (far, in mod_xyz.c), which
  returns the Fusion, Synchro or Xyz frame.
- **Field icon:** black, from the template `card_icons/xyz_monster.png` (the silver Synchro icon with its two frame
  colours, indices 7 and 13, made dark). `tools/cardicons.py` appends its five frames after the Synchro ones
  (`DATA_XYZ` in `src/mod_field_icons.c`).
- **Deck Edit ring:** the frame drawn in the card's art palette as for Synchros, with black colours (`DE_XYZ_FRAME`).
- **Card View:** the type line reads "[Warrior/Xyz/Effect]". The star row shows the Rank (EDS has no Rank-star art).
- **White ATK/DEF (2026-11-06):** the label and digits are entry 8 of `gCardStatDigitsPal` (black), unreadable on the
  black frame. Card View sets OBJ palette 3's entry 8 to white each frame for an Xyz or Link card
  (`ModCardViewStatColor`, from `CardDetail_DrawSprites`); the duel's big card (`DrawCardInfo`, card_canvas.c) takes
  its palette from `ModCardStatPal` (a white-text copy in gModLink). Checked with Card View screenshots.
- **Info bar:** an Xyz Monster's bar shows "MAT" right of its name and the material count under it
  (`src/mod_info_bar.c`, cells 0x10 and 0x30). `TextCellsPutString` / `TextCellsPutNumber` set `textMapReset`, which
  resets the cell map after the upload; `TextCellsLoadIcon` clears it so the type and attribute icons keep their
  palettes, so the label clears it again after drawing (first build: Pearl's icons were miscoloured).
- **Deck Edit ring while scrolling (2026-11-06 fix):** up to 6 frames show but only OBJ palettes 11-15 are free for
  art, and the frame left without art fell back to the Fusion purple (a black frame flashing purple).
  `ModDeckRingBegin(slots)` (deck_edit_widgets.c now passes the ring) picks the frame that goes without art, a plain
  card's first and the one nearest an end of the ring; a Synchro or Xyz frame left without art borrows a drawn
  palette of its own kind (`gModDeckRing.kind`).

## Xyz Materials
- **Storage:** `gModXyz` (EWRAM 0x0202D500, symbols.ld, at most 0x200 bytes): 32 entries of {zone serial, state,
  card word (ID and owner)}.
- **Keyed by the Xyz Monster's `DuelZone.serial`.** `DuelCmd_MoveToZone` copies the whole zone record (serial
  included) and `SwapFieldCards` swaps records, so a control change keeps the materials.
- **Leaving the field.** Every way off the field ends in `ClearZone` (`src/duel_card_lists.c`), which now calls
  `ModXyzZoneCleared` first: the materials under that serial go to their owners' Graveyards (`AddCardToGraveyard`, no
  animation). This is one call in the size-limited `.text`.
- **Detaching:** `XyzDetachBegin`: the CPU's lowest-ATK material; the human's only choice when all the materials are
  the same card; otherwise a card list of that monster's materials (`CARD_LIST_KEY_XYZ_MATERIALS`,
  `ModXyzMaterialView`), then `XyzDetachPicked`.
- **Once per turn:** `ModXyzEffectUsed` / `ModXyzMarkEffectUsed`, by turn and zone serial, in gModXyz (no change to the
  size-limited turn-start code).

## The Xyz Summon
- **The human.** The Fusion Deck pile's Special Summon command (Main Phase) now lists the Synchro and the Xyz
  Monsters you can summon together (`ModSynchroListAccepts`, `CARD_LIST_KEY_SYNCHRO`). Picking an Xyz Monster hands
  over to `ModXyzMenuRun`: you pick the materials with the cursor (B cancels before anything moves; START ends an
  "or more" pick). With exactly the needed number on the field, they're taken without a prompt.
- **Materials:** face-up monsters with the rule's Level (and Type/Attribute), not Xyz Monsters and not tokens.
- **Doing it** (`PerformXyzSummon`): each material's card word is stored as pending, then the zone gets
  BanishFieldCard's clean-up without the banished pile (`BigBangShotLeaves`, `DetachRevivingCards`,
  `DestroyLinkedCards`, `RemoveLinksToZone`, `ClearZone`). The Xyz Monster leaves the Extra Deck and is Special
  Summoned (position chosen, the proper-summon flag). `ModXyzArrived` (`AddedCardsSummoned`) attaches the pending
  materials under its new zone's serial. Pending materials of a summon that never placed its monster go to the
  Graveyard at the next Xyz Summon.
- It starts no chain; the usual Special Summon window follows. A summon negated after placement (Solemn Judgment)
  sends the monster away through ClearZone, so its materials go to the Graveyard.
- **The CPU** (`AiTryXyzSummon`, in `AiPlayAddedCards` after the Synchro try): the Xyz Monster whose ATK beats its
  strongest material by the most, with its weakest fitting monsters as the materials.
- **Rank is not Level** in the mod code: Xyz Monsters are never Synchro Material (`FaceUpMonster` in mod_synchro.c)
  or Xyz Material.
  > [!warning] Limitation
  > The original cards' Level checks (Ritual Tributes, Gravity Bind, "Level 4 or lower" effects, the info bar's
  > star) read the stats word, so they see the Rank as a Level.

## The three test cards
- **Gem-Knight Pearl.** "2 Level 4 monsters"; no effect.
- **Number 17: Leviathan Dragon.** "2 Level 3 monsters". ACTIVATE on its card menu (`ModCanActivateMonsterEffect`),
  once per turn (marked in chainA): the resolve detaches 1, then +500 ATK (`ZONE_LINK_ATK_BONUS`). With no material
  it can't attack directly (`ModCannotAttackDirectly`, which `CanMonsterAttack` asks for the CPU too). The CPU uses
  the effect whenever it can.
- **Number 39: Utopia.** "2 Level 4 monsters". `ModAttackDeclaredTriggers` (battle_phase1.c; its `#ifdef` now also
  takes Utopia) queues a trigger for each face-up Utopia with a material when any monster declares an attack; the
  CPU's only against the human's attacks. The resolve asks the human (Yes/No), detaches 1 and negates the attack
  (`DUEL_CMD_NEGATE_ATTACK`). A Utopia with no material that is an attack's target is destroyed there.
- The detach of both effects happens in the resolve, not as a cost, so a negated activation costs no material.

## Size
- Size-limited hooks: `ClearZone` (one call), and the three frame picks, now one call each instead of the inline
  Synchro test. Everything else is far code.

Related: [[synchro]], [[added-cards]].
