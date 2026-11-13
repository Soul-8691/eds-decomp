-- Deck Edit's List Filter (README_TODO.md): screenshots/gifs/list-filter.gif. The trunk (every card once: the
-- captures' fresh save holds a few), the command bar's List Filter, then the third page (src/mod_list_filter.c):
-- name starting with D, Type Warrior, ATK from 1000 (12 cards); A filters and the list narrows.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

local LETTER_D = 4                  -- ROW_NAME: 1..26 are A..Z
local TYPE_WARRIOR = 23             -- sTypes: Aqua 1 .. Warrior 23 .. Zombie 26
local ATK_1000 = 10                 -- steps of 100

M.run("list_filter", function()
    M.toMainMenu()
    M.giveAllCards()
    M.openMenu(M.MENU.DECK_EDIT, "CB_DeckEdit", 120)
    M.record("list-filter", "gif")
    M.wait(40)
    M.deckEditCommand(M.DECK_CMD.LIST_FILTER)
    M.wait(70)
    M.press(M.KEY.A, nil, 40)       -- the filter page (All Cards) -> the sort page
    M.press(M.KEY.A, nil, 50)       -- the sort page -> the third page
    M.refineSet(M.REFINE.NAME, LETTER_D)
    M.refineSet(M.REFINE.TYPE, TYPE_WARRIOR, false, 27)   -- (Any and 26 Types)
    M.refineSet(M.REFINE.ATK_FROM, ATK_1000, true)
    M.wait(30)
    M.press(M.KEY.A, nil, 20)       -- filter
    M.wait(120)
    M.pressN(M.KEY.DOWN, 6, 14)     -- down the narrowed list
    M.wait(40)
    M.stop()
end)
