-- The format check (README_TODO.md, Format duelists): Format Check.png. From the release ROM, where the check runs
-- (src/mod_banlist.c: make has none): the saved Deck gets cards Yugi-Kaiba does not allow, then the Yugi-Kaiba
-- duelist (the first page's top left) is picked, and the warning lists them (BanlistBlocksDuel).
--   run_tests.ps1 -Media -Filter format_check -Rom eds_release_demo.gba
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("format_check", function()
    M.toMainMenu()
    M.addToDeck("Breaker the Magical Warrior", 1)
    M.addToDeck("Chaos Sorcerer", 1)
    M.addToDeck("Cyber Dragon", 2)
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    M.campaignPick(0, M.SLOT.TOP_LEFT)
    M.press(M.KEY.A)
    M.wait(90)
    M.shot("Format Check")
end)
