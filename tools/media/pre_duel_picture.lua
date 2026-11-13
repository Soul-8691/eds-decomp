-- The pre-duel screen of a format picture (README_TODO.md): Pre-duel (Yugi-Kaiba).png, the first page's top left.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("pre_duel_picture", function()
    M.toMainMenu()
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    M.campaignPick(0, M.SLOT.TOP_LEFT)
    M.press(M.KEY.A)
    M.wait(150)
    M.shot("Pre-duel (Yugi-Kaiba)")
end)
