---
title: Link Monsters, Extra Monster Zones and the zone pick (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-11-06
---
# Link Monsters, Extra Monster Zones and the zone pick (mod)

Added on 2026-11-06. None of this is in the original game. The code is `src/mod_link.c` (far code),
`include/link_monster.h` (not `include/link.h`, which is the link cable's), the shared Extra Deck summon in
`src/mod_synchro.c`, and the two test cards in `src/added_effects.c`. Built (`make demo`); **not tested in an
emulator**.

## Decisions
- **Storage:** the engine keeps 5 monster records per player; one of them can *stand in* an Extra Monster Zone.
  Every loop over the 5 records (battle, Raigeki, targeting, the CPU) covers it unchanged. Rule difference: at most
  5 monsters per player in all (e.g. 4 Main + 1 EMZ).
- **Place:** each EMZ counts as the Main Monster Zone at its screen edge: the left one uses the record of Main zone 1,
  the right one that of zone 5 (mirrored for the opponent), so that zone must be free and stays unusable while a
  monster stands in the EMZ (`EmzRecordOf`). Link Arrows use the real columns: the left EMZ is above Main column 2,
  the right one above column 4. (First build: any free record, and Link Spider in the left EMZ could point down at
  its own blocked column.)
- **Rules:** Master Rule 5: a Link Monster goes to an EMZ or a zone one of its controller's Link Monsters points to;
  other Extra Deck monsters may go to a Main zone or a free EMZ.

## Card data
- `tools/newcards.py` accepts "Link Monster"; it caches YGOPRODeck's `linkval` and `linkmarkers` (a Link card cached
  without them is fetched again). Stats word: kind FUSION (the Extra Deck), Level = Link Rating, DEF 0.
  `gCardTypeEx` kind 3 (`CARD_KIND_EX_LINK`, `IS_LINK_CARD`); `gCardLinkMarkers` (u8 per card ID, bits TL T TR L R
  BL B BR, `LINK_MARKERS`); `LINK_RATING`.
- Material rules: `build/include/link_rules.h` (`LINK_RULES`): count, "or more", kind (any, Normal, Effect, Type,
  Attribute, Level) and value, from the text's first line; anything else warns and reads as "2 monsters".
- Also fixed: `stats_word` looked up the card's own race instead of its EDS stand-in (a Cyberse card failed).

## Look
- Frame: the Fusion frame made dark blue (`tools/synchro_frame.py link`, `gCardFrameLinkGfx`, through
  `ModFusionKindFrame`); field icon `card_icons/link_monster.png` (blue); Deck Edit ring `DE_LINK_FRAME`.
- Card View: "[Cyberse/Link-2/Effect]" (`ModCardTypeLine`), no stars and no DEF (`CardDetail_DrawSprites`,
  title_menu.c).
- Card previews (2026-11-06): the duel preview (`LoadCardFrame`), Card View and the battle scene all take the frame
  from `ModFusionKindFrame`, which for a Link Monster returns `ModLinkFrame` (src/mod_link.c): a copy of
  `gCardFrameLinkGfx` in `gModLinkFrame` (EWRAM 0x02034000, inside `gBustupBitmapBuffer`, used only by the dialogue
  scenes and the pre-duel picture) with its 8 arrows painted around the art window (x 16-87, y 32-111 of the
  104x144 frame), red (palette entry 1, unused by the frame) where it points, the frame's dark blue (entry 3)
  where it doesn't. Rebuilt on every call. Checked in Card View screenshots. It also paints over the frame's
  ":" (pixels 62,129 and 62,131) that the DEF label hides on other frames.
- White ATK text on the blue frame, and no stars or DEF on the duel's big card (`DrawCardInfo`): see [[xyz]].
- Field: orange arrow sprites on the edges of every face-up Link Monster for its markers (turned around for the
  opponent's), OBJ tiles 0x3EA-0x3EC (up, left, up-left, flipped for the rest), OBJ palette 12 entry 4.
- Info bar: a Link Monster's DEF reads "-".

## Extra Monster Zones
- `gModLink.emzSerial[2]`: the zone serial of the monster standing in each EMZ (screen left, right). The serial
  travels with the zone record, so control changes keep it; `ModLinkZoneCleared` (from `ModXyzZoneCleared`, the
  `ClearZone` hook) frees it.
- Drawing: `ModZoneFieldX/Y` give a record's field position (its EMZ when it stands in one), asked by `GetAreaX` /
  `GetAreaY` (card_canvas.c) and `DrawZoneTiles` / `ClearZoneTiles` (duel_field_screen.c). The record's Main column
  then shows empty.
- The cursor reaches an EMZ monster by its record's index (it jumps to the middle row).
- A player has at most one monster in an EMZ; a used EMZ is unavailable to both.

## The zone pick (every summon)
- `ModSummonZoneStep`, called by `SummonAction_Update` (summon_action.c) before the kind's step machine:
  the human picks the Monster Zone of every summon but a Flip Summon, with the Control Pad and A over a marker
  (the "can attack" marker sprite), in a text box. Places: free Main zones (and the Tributes' zones of a Tribute
  Summon); a free EMZ for an Extra Deck monster; for a Link Monster only an EMZ or a pointed zone.
- No prompt with one place; the CPU and Link Battles keep the engine's zone (a CPU Link Monster: an EMZ first).
- `ModSummonSetZone` (the Link Summon) and `ModSummonRestrictZones` (Link Spider) steer it.
- A Link Monster is always face-up Attack Position: forced in the step, and the position question of
  `SummonStep_SpecialChoosePosition` is skipped (`ModSummonDefenseChoice`).

## Link Monsters on the field
- `ModLinkPointsTo` / `ModLinkPointedMonsters` / `ModLinkPointedFreeZones`: from the grid (row 0 player 0's Main
  zones, row 1 the EMZs at columns 2 and 4, row 2 the opponent's Main zones mirrored).
- Never changes position: `DuelCmd_ChangePosition` and `DuelCmd_FlipCard` (duel_cmd_field.c) refuse a Link Monster;
  the card menu drops its position commands (card_command_menu.c).
- Never Synchro or Xyz Material (`FaceUpMonster`, `MaterialFits`).
  > [!warning] Limitation
  > The original cards' Level checks read the stats Level, which is the Link Rating.

## The Link Summon
- The Fusion Deck pile's Special Summon list now also offers the Link Monsters you can summon
  (`ModLinkCanSummon`, `SYNCHRO_STEP_LINK`, `ModLinkMenuRun`):
  - pick the materials with the cursor; a Link Monster counts as its rating when that still fits, else 1;
  - they must add up to the Link Rating, at least the rule's count of monsters, and leave a place;
  - then pick the place (an EMZ, or a pointed Main zone);
  - the materials go to the Graveyard and the Link Monster is Special Summoned there.
- CPU: `AiTryLinkSummon` (`AiPlayAddedCards`, after the Xyz try): the Link Monster with the most ATK over its
  strongest material, an EMZ first.
- Not offered in Link Battles (the EMZ placement is not sent to the partner).

## The test cards
- **Link Spider** (Link-1, Bottom, "1 Normal Monster"):
  - ACTIVATE on its card menu, once per turn (`ModXyzEffectUsed`).
  - Special Summon a Level 4 or lower Normal Monster from the hand (the human picks from a list, the CPU its
    strongest) in face-up Attack Position to a free zone it points to.
- **Proxy Dragon** (Link-2, Left and Right, "2 monsters"):
  - **By card effect:** `DestroyFieldCard` asks `ModProxyDestroyIntercept` first. The CPU destroys its weakest
    pointed monster instead. The human gets `PROMPT_MOD_REPLACE_DESTROY` (duel_prompts.c): Yes/No, then a cursor
    pick when it points to more than one; No destroys the card as usual.
  - **By battle:** `BattleStage_DestroyMonsters` asks `ModProxyBattle` before the monsters leave; a replacement
    clears that side's destroyed flag (the battle damage stays).
  - Limitation: only one prompt waits at a time; while one does, other destructions go ahead as usual.

## Size
Size-limited hooks: card_canvas.c (GetAreaX/Y), duel_field_screen.c (2), summon_action.c (2), duel_cmd_field.c (2),
card_command_menu.c, duel_field_moves.c (DestroyFieldCard), battle_phase2.c, duel_prompts.c, title_menu.c (2).
Demo `.text` ends at 0x0807F440.

Related: [[duel-hud]], [[xyz]], [[synchro]], [[added-cards]].
