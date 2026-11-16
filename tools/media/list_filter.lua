-- Deck Edit's List Filter (README_TODO.md): screenshots/gifs/list-filter.gif. The trunk (every card once: the
-- captures' fresh save holds a few), the command bar's List Filter: Effect Monsters, sorted by ATK, then the third
-- page (src/mod_list_filter.c): Attribute DARK, ATK 1000 to 2000, Level 1 to 4; A filters and the list narrows.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

local FILTER_EFFECT = 2             -- enum ListFilter (include/deck_edit.h)
local SORT_ATK = 1                  -- enum ListSort
local ATTR_DARK = 1                 -- sAttributes: Any 0, Dark 1 .. Wind 7
local ATK_1000, ATK_2000 = 10, 20   -- steps of 100
local LEVEL_4 = 4                   -- ROW_LEVEL_TO (ROW_LEVEL_FROM starts at 1)

-- A List Filter page cursor (gDeckEdit +0x1C49 filter, +0x1C4A sort) moved onto `want`.
local function cursorTo(offset, want, what)
    local addr = M.sym("gDeckEdit") + offset
    local keys = { M.KEY.DOWN, M.KEY.RIGHT }
    for k = 1, #keys do
        for _ = 1, 8 do
            if M.read8(addr) == want then
                M.wait(20)
                return
            end
            M.press(keys[k], nil, 20)
        end
    end
    if M.read8(addr) ~= want then
        error(what .. ": cursor " .. want .. " not reached (at " .. M.read8(addr) .. ")")
    end
end

M.run("list_filter", function()
    M.toMainMenu()
    M.giveAllCards()
    M.openMenu(M.MENU.DECK_EDIT, "CB_DeckEdit", 120)
    M.record("list-filter", "gif")
    M.wait(40)
    M.deckEditCommand(M.DECK_CMD.LIST_FILTER)
    M.wait(70)
    cursorTo(0x1C49, FILTER_EFFECT, "the filter page")
    M.press(M.KEY.A, nil, 40)       -- the filter page (Effect Monsters) -> the sort page
    cursorTo(0x1C4A, SORT_ATK, "the sort page")
    M.press(M.KEY.A, nil, 50)       -- the sort page (ATK) -> the third page
    M.refineSet(M.REFINE.ATTRIBUTE, ATTR_DARK, false, 8)  -- (Any and 7 Attributes)
    M.refineSet(M.REFINE.ATK_FROM, ATK_1000, true)
    M.refineSet(M.REFINE.ATK_TO, ATK_2000, true)
    M.refineSet(M.REFINE.LEVEL_TO, LEVEL_4, true)
    M.wait(30)
    M.press(M.KEY.A, nil, 20)       -- filter
    M.wait(120)
    M.pressN(M.KEY.DOWN, 6, 14)     -- down the narrowed list
    M.wait(40)
    M.stop()
end)
