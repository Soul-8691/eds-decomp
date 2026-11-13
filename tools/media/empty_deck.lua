-- Practice opponents, Empty Deck (README_TODO.md): Empty Deck.png, a duel against it in your Main Phase 1 with the
-- cursor on its side (its empty field, hand and piles). Empty Deck is the last page's right slot. The captures run on
-- a fresh save, which has no deck: the Chaos Turbo recipe is loaded first.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("empty_deck", function()
    M.toMainMenu()
    M.openMenu(M.MENU.DECK_RECIPES, "CB_DeckRecipes", 75)
    M.loadRecipe("Chaos Turbo")
    M.back()
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    M.campaignPick(3, M.SLOT.RIGHT)
    -- (A through what comes before the duel: your deck from the recipes, if asked)
    local t = 0
    while not M.rpsChoosing() do
        if t % 60 == 0 then
            M.press(M.KEY.A, nil, 10)
        end
        t = t + 1
        if t > 1800 then
            error("timeout before rock-paper-scissors")
        end
        coroutine.yield()
    end
    M.rpsWin(true)
    M.toMainPhase()
    M.pointOpponentHand()
    M.shot("Empty Deck")
end)
