---
title: Banlists menu (mod)
type: game
status: draft
confidence: medium
sources: [rom-analysis]
updated: 2026-10-26
---
# Banlists menu (mod)

The main menu's tenth item, added by this build (`src/mod_banlists.c`, `@farcode`/`@rodata`). It picks the
Forbidden/Limited list that Deck Edit uses. Checked in the source and the build; not tested in an emulator.

## The banlists
`tools/banlist.py --all` writes `build/include/banlists.h` from `banlists/<name>_ban.json`. There are 15 lists, in
this order: Yugi-Kaiba, Critter, Treasure, Imperial, Android, Joey-Pegasus, Fiber, Yata, Scientist, Vampire, Chaos,
Warrior, Goat, Cyber, Reaper. Each has a card pool, `card_pools/<name>.txt`.

- **Goat and Cyber.** They share the same Forbidden/Limited list (April 2005, identical JSON files) but have
  different pools: Cyber's is Goat's plus 69 cards. They were merged for a while, then split again.
- **Which arrays count.** Only "forbidden", "limited" and "semiLimited" (0/1/2 copies). "unlimited" lists cards
  that came off the list.
- **Name matching.** Exact, then case/punctuation-insensitive, then `ALIASES` and the "TCG name | EDS name" lines of
  `config/new_cards.txt`. Ring of Destruction, which the game doesn't have, maps to Bell of Destruction (`ALIASES`). Other cards the game doesn't have are reported and skipped (Fiber Jar, Yata-Garasu, Chaos
  Emperor Dragon, ...); they're picked up when they're added.
- **Not offered:** the game's own list (`gCardCopyLimits`).

## Card pools
- **Generation.** `tools/banlist.py --all` turns each pool file (one TCG card name per line) into a sorted table of
  card numbers (`sBanlistPoolN`, the `pool`/`poolCount` of `struct Banlist`).
- **Effect.** A card outside the pool of the banlist in use gets limit 0 from `GetBanlistCopyLimit`, the same as a
  forbidden card. So Deck Edit refuses it, `AddCardToSaved*Deck` refuses it, a Deck Recipe load counts it as
  missing, and switching banlists moves it to the trunk.
- **Name matching** is the same as for the lists. A name maps to every number the game has for that card (both
  Polymerizations); the first version kept only the first number, which left the second Polymerization outside
  every pool. Pool aliases: Black Skull Dragon = B. Skull Dragon, Kuwagata α = Kuwagata a.
- **What's unmatched.** 1034 pool names have no EDS card (later TCG cards). 220 EDS cards are in no pool at all:
  OCG-only or game-only cards, e.g. Pendulum Machine and the Egyptian Gods. Checked with a script over every pool;
  the remaining names were looked up by hand for TCG spellings.

## Where the limits apply
- **Deck builder.** `GetBanlistCopyLimit` gives the limit of a card number (alternate art counts as the base card);
  unlisted cards stay unlimited (`CARD_COPY_LIMIT_NONE`, this build has no 3-copy limit). `src/mod_save.c` uses it
  in `IsBelowCardCopyLimit`, `AddCardToSaved*Deck` and `TrimSavedDecksToCopyLimits`, so it covers Deck Edit and
  [[deck-recipes]] switching.
- **CPU decks.** They keep the game's list: `RemoveOverLimitDeckCards` (CPU only) still calls `GetCardCopyLimit`.
- **Tristan.** His check (`mod_banlist.c`) is unchanged: Yugi-Kaiba pool and list. Its JSON now comes from
  `banlists/yugi-kaiba_ban.json`.

## Saving and switching
- **Saved choice.** `gSaveData.banlist` (+0x20CE) holds the index + 1. Verified unused by the original, by the same
  ROM literal scan as [[duelist-points]]. 0 (a New Game, or an older save) means `BANLIST_DEFAULT`, Goat.
- **Switching (A).** `SetActiveBanlist` trims the saved Deck and Side Deck (`TrimSavedDecksToCopyLimits`) and the
  Fusion Deck to the new limits, moving the extra copies to the trunk. It then saves, and the status line says how
  many cards moved.

## Scene
Laid out like [[deck-recipes]]:
- the "Banlists" label (built by `BuildMenuLabel`) as the title;
- 8 rows of `> * Name (date)` (`*` marks the list in use), with the forbidden/limited/semi-limited counts at x = 196;
- help lines and a status line.

## Main menu scrolling
With 10 items the menu shows 8 rows (`MAIN_MENU_VISIBLE`, from y = 16) and scrolls with the cursor
(`gMainMenuScroll`, EWRAM 0x0202F260, clamped on every draw).

OBJ tiles on the main menu:

| Tiles | Contents |
|---|---|
| 0x200 | Deck Recipes label |
| 0x240 | Booster Packs label |
| 0x280 | Banlists label |
| 0x2C0 | DP line |

Related: [[deck-recipes]], [[pack-shop]], [[duelist-points]], [[main-menu]].
