-- Siding between games (README_TODO.md, Best-of-3 matches): screenshots/gifs/siding.gif. Game 1 against the
-- Yugi-Kaiba duelist is won at once (M.winDuel); then the side swap: Bell of Destruction out, Kycoo the Ghost
-- Destroyer in.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("siding", function()
    M.toMainMenu()
    M.openMenu(M.MENU.DECK_RECIPES, "CB_DeckRecipes", 75)
    M.loadRecipe("Chaos Turbo")
    M.back()
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    M.campaignPick(0, M.SLOT.TOP_LEFT)
    M.press(M.KEY.A)
    M.wait(150)
    M.press(M.KEY.A)
    M.rpsWin(true)
    M.toMainPhase()
    M.winDuel()
    M.wait(450)                     -- "YOU WIN", then the side swap (Deck Edit on the Main Deck)
    M.record("siding", "gif")
    M.wait(40)
    M.press(M.KEY.A, nil, 60)       -- the bar: Select A Swapping Card
    M.press(M.KEY.A, nil, 60)       -- the Side Deck
    M.press(M.KEY.A, nil, 120)      -- its first card (Kycoo) for the Main Deck's Bell of Destruction
    M.stop()
end)
