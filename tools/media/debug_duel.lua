-- Practice opponents, Debug (README_TODO.md): Debug Picker.png (Deck Recipes: "First pick your own deck"),
-- Debug Duel.png (Main Phase 1, Debug's hand face up; rock-paper-scissors won on the way, no longer recorded), and screenshots/gifs/mulligan.gif (two Mulligans, then Draw: against Debug they are unlimited). Debug is
-- the last page's top right (src/mod_opponents.c sPageDuelists).
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("debug_duel", function()
    M.toMainMenu()
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    M.campaignPick(3, M.SLOT.TOP_RIGHT)
    M.press(M.KEY.A)
    M.wait(150)
    M.shot("Debug Picker")
    M.press(M.KEY.A)                -- your deck: the first recipe
    M.wait(120)
    M.press(M.KEY.A)                -- Debug's deck: the first recipe; then Debug's pre-duel screen
    M.wait(150)
    M.press(M.KEY.A)
    M.waitUntil(M.rpsChoosing, 1800, "rock-paper-scissors")
    M.rpsWin(true)
    M.wait(45)
    M.mulligan(2, "mulligan")
    M.toMainPhase()
    M.pointOpponentHand()
    M.shot("Debug Duel")
end)
