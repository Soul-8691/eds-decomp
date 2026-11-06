---
title: Synchro Summoning (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-05
---
# Synchro Summoning (mod)

Synchro Monsters, Tuners and the races EDS lacks, added on 2026-11-05. None of this is in the original game. The code
is `src/mod_synchro.c` (far code), `include/synchro.h`, and the four test cards in `src/added_effects.c`. Built
(`make demo`); **not tested in an emulator**.

## Card data
- **`gCardTypeEx`.** One byte per card ID, written by `tools/newcards.py` and placed after `gCardIdsByName` in
  `data/rodata_08087FD0.s`. It's 0 for every original card.
  - Bits 0–1: the extended kind (`enum CardKindEx`, Synchro).
  - Bit 2: has an effect.
  - Bit 3: Tuner.
  - Bits 4–7: the extended race (`enum RaceEx`).
  - Macros in `include/card_data.h`: `IS_SYNCHRO_CARD`, `IS_TUNER_CARD`, `IS_EFFECT_CARD_EX`, `CARD_RACE_EX`.
- **Why a second table.** The stats word has no free bits. Kind is 2 bits and all four values are used. Race is 5
  bits, but the engine treats "type ≤ Reptile (20)" as "is a monster" in about 233 places across 48 files, so a new
  race number would make the card a non-monster.
- **Synchro Monsters** are kind FUSION in the stats word. The engine then keeps them in the Fusion Deck (the Extra
  Deck) in Deck Edit, on a return to hand and in the saved deck, with no extra code.
- **New races:** Psychic, Wyrm, Cyberse, Illusion, Creator God and Divine-Beast (for an added card).
  - The stats word holds the closest EDS type: Spellcaster, Dragon, Machine, Spellcaster, Fairy and Fairy. That's
    what the icons and the engine's checks see.
  - The real race shows in Card View (`ModCardRaceName`) and in the List Filter's Type list (`RaceKey`,
    `src/mod_list_filter.c`).
  - The race icons are placeholders until new art exists.
- **Material rules.** `build/include/synchro_rules.h` (`SYNCHRO_RULES`) is written from each Synchro's first line of
  text: `"<name>" + 1+ non-Tuner monsters` names the Tuner it needs, and `1 Tuner + 1+ non-Tuner monsters` takes any.
  Anything else is read as the generic rule, with a warning from the tool.

## Look
- **Frame:** the Fusion frame with its purple made white, `gCardFrameSynchroGfx` (`tools/synchro_frame.py`). It's used
  by Card View (`card_detail.c`), the duel card image (`card_canvas.c`) and the battle scene (`duel_card_anim.c`).
- **Field card icon:** silver, from the template `card_icons/synchro_monster.png`. It's the Fusion icon with its two
  frame colours (indices 7 and 13) in silver, and you can repaint it freely.
  - The icons share gCardIconPal's 16 colours, and all 16 are used, so `tools/cardicons.py` draws any pixel whose
    colour isn't gCardIconPal's at its index with the nearest colour of the field icons' shared art palette. The
    silver colours get heavy weight so that palette keeps them.
  - The frames go after Deck Edit's ring art in gModFieldIconData: BG upright and sideways, then sprite poses 0-2.
    `mod_field_icons.c` picks them by `IS_SYNCHRO_CARD` (`DATA_SYNCHRO`).
  - Poses 1-2 (the flip's narrower frames) are the game's Fusion poses with the same colours swapped.
- **Not yet silver:** Deck Edit's frame thumbnails, which still use the Fusion purple.
- **Card View's type line:** for every added monster it's rebuilt by `ModCardTypeLine`: "Race/Synchro/Fusion/Ritual,
  /Tuner, /Effect". That also fixes added Fusion Monsters with an effect, which used to read "/Fusion" only.

## The Synchro Summon
1 Tuner and 1 or more non-Tuner monsters, all face up on the summoner's field, whose Levels add up exactly to the
Synchro Monster's Level, go to the Graveyard. The Synchro Monster is then Special Summoned from the Extra Deck. It
starts no chain, and the usual Special Summon window follows, so Horn of Heaven, Royal Oppression and Solemn Judgment
can answer it.
- **The human.** In your Main Phase, A on your Fusion Deck pile offers Special Summon when a Synchro Summon is possible
  (`ModSynchroAvailable`, `card_menu_input.c`). Then `ModSynchroMenuRun` (`duel_cmd_queue.c`) runs:
  1. the card list of the Synchros you can summon (`CollectEffectTargets` with `CARD_LIST_KEY_SYNCHRO`);
  2. the Tuner, with the cursor, skipped when only one fits;
  3. the non-Tuners, under the Ritual star gauge (`DrawRitualStarGauge`). A pick is refused unless the rest can
     still add up exactly, so the picks never dead-end. B cancels the Tuner or non-Tuner pick; nothing leaves the field before the last pick. The non-Tuner prompt names the Tuner, its Level and the Levels still needed.
- **The CPU.** `AiTrySynchroSummon` (`AiPlayAddedCards`) picks the Synchro, Tuner and non-Tuners that gain the most
  ATK over the strongest material, and only a gain counts.
- **Marks.** `ModSynchroArrived` (`AddedCardsSummoned`) marks the arriving monster as Synchro Summoned
  (`ModWasSynchroSummoned`), and "If this card is Synchro Summoned" triggers use that mark.
- **Excluded from Fusion effects.** Magical Scientist skips Synchros (`IsScientistFusion`). Polymerization only lists
  recipe results, so it never offers them.
- **Not done:** the "properly summoned" revival rule. A Synchro in the Graveyard can be revived like any monster.

## The four test cards
- **Junk Synchron.** Level 3 DARK Warrior Tuner, Normal Summon trigger. You choose whether to use it. It revives a
  Level 2 or lower monster from your Graveyard in Defense Position, and that monster's zone is marked disabled
  (`isDisabled`). The CPU always uses it and takes the highest-ATK target.
- **Quillbolt Hedgehog.** From your Graveyard while you control a face-up Tuner.
  - **The human:** A on your Graveyard pile opens a menu with Special Summon. While it's available, that pile's A no
    longer opens the Graveyard list directly.
  - **The CPU:** it uses it before trying a Synchro Summon.
  - **The activation:** it's an off-field activation, so it can be answered.
  - **The banish:** a Quillbolt summoned this way is banished when it leaves the field, including as Synchro
    Material, through Dark Magician of Chaos's "banished instead" check (`QuillboltBanishedOnLeave`).
- **Junk Warrior.** "Junk Synchron" + 1 or more non-Tuners. When it's Synchro Summoned, it gains the current ATK of
  your face-up Level 2 or lower monsters, as a `ZONE_LINK_ATK_BONUS` link that lasts while it's on the field.
- **Stardust Dragon.** "1 Tuner + 1+ non-Tuner monsters".
  - **Answering:** a face-up Stardust answers a link that would destroy a card on the field. "Would destroy" means
    My Body as a Shield's monster destroyers plus `sCardDestroyers`, the Spell/Trap ones. It works through the
    engine's monster-response path for key 1525: `CanPlayerChain`, `Chain_GetResponseCommands`. The human can now
    pick face-up monsters in chain responses on their own turn too.
  - **Effect:** Tribute it as the cost, then the link is negated and destroyed (My Body's resolve).
  - **Revival:** in that turn's End Phase, Stardust comes back from its owner's Graveyard
    (`EndPhase_StardustDragon`). It always comes back; the human isn't asked.
  - **CPU:** `AiChainStardustDragon` answers the human's links that would destroy one of its cards.

## Size
These add small hooks to original units in the size-limited area: `card_canvas.c`, `card_detail.c`,
`duel_card_anim.c`, `title_menu.c`, `card_menu_input.c`, `duel_cmd_queue.c`, `duel_setup.c`, `effect_activation.c`,
`battle_phase3.c` and `summon_action.c`. The demo's `.text` grew from 0x7f1ac to about 0x7f300.

Related: [[added-cards]], [[deck-edit-filter-c]], [[xyz]].

## Xyz (2026-11-06)
- The Fusion Deck pile's Special Summon now lists the Xyz Monsters too (`ModSynchroListAccepts`,
  `ModSynchroAvailable`); an Xyz pick runs `ModXyzMenuRun` (`SYNCHRO_STEP_XYZ`). An Xyz Monster is never Synchro
  Material (`FaceUpMonster`). Card View's type line gets "/Xyz". See [[xyz]].

## Typed Tuner rules (2026-11-05)
- "1 <Type>-Type Tuner + 1 or more non-Tuner monsters" (Chaos King Archfiend): `tools/newcards.py` writes the
  rule's Tuner as `0x8000 | type`, and `TunerFits` checks the Tuner's current type (`GetZoneCardType`).
  See [[added-cards]].

## Deck Edit ring frame (2026-11-05)
- Deck Edit's ring draws the Fusion frame for a Synchro (`GetCardFrameIndex`: kind FUSION), a 32x32 sprite of
  `gDeckEditCardFrameObjTiles` in OBJ palette 6, whose tiles use entries 1, 4, 10, 12 and 15. No 4x4 tile block is
  free for a frame of its own, and palette 6 has no spare entries, so the frame takes the card's art palette instead:
  `tools/cardicons.py` (`deck_edit_entry`, `DE_SYNCHRO_FRAME`) quantises a Synchro card's ring art to the other ten
  entries and puts the frame colours in silver at those five; `ModDeckRingArt` (`src/mod_field_icons.c`) gives the
  frame sprite the art's palette bank (11 + k). With no art drawn (all five art slots used, or another screen's
  sheets) the frame stays purple. Checked with a render of the built data; not tested in an emulator.
