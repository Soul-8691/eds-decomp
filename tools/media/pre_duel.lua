-- The pre-duel screen (README_TODO.md, The Campaign): screenshots/gifs/pre-duel.gif, Weevil's bust-up (the second
-- page's top left: the Joey-Pegasus format) with its blinking eyes, text and QR code, then A to rock-paper-scissors.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("pre_duel", function()
    M.toMainMenu()
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    M.campaignPick(1, M.SLOT.TOP_LEFT)
    M.press(M.KEY.A)
    M.wait(70)                      -- (the fade from the select screen)
    M.record("pre-duel", "gif")
    M.wait(240)
    M.press(M.KEY.A)
    M.wait(40)
    M.stop()
end)
