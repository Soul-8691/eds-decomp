-- Unlocking a format (README_TODO.md, The Campaign): screenshots/gifs/unlock.gif. From the release ROM (make opens every
-- format): Tristan (the Yugi-Kaiba duelist, page 1 top left) at 19 wins (gSaveData.duelRecords[4], wins bits 0-10),
-- then a match won at once (one duel counted as played and won, as new_packs.lua), the 20th win; then the next format
-- duelist appears on the select screen.
--   run_tests.ps1 -Media -Filter unlock -Rom eds_release_demo.gba
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")
local G_MAIN = 0x03000040
local TRISTAN = 4

M.run("unlock", function()
    local save = M.sym("gSaveData")
    M.toMainMenu()
    local rec = save + 0x20D0 + TRISTAN * 4
    M.write32(rec, (M.read32(rec) & ~0x7FF) | 19)
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    M.campaignPick(0, M.SLOT.TOP_LEFT)
    M.press(M.KEY.A)
    M.wait(150)
    M.press(M.KEY.A)
    M.rpsWin(true)
    M.toMainPhase()
    M.write8(G_MAIN + 0x4888, (M.read8(G_MAIN + 0x4888) & ~0x30) | 0x10)
    M.write8(G_MAIN + 0x4889, 1)
    M.record("unlock", "gif")
    M.wait(30)
    M.winDuel()
    M.wait(200)                     -- "YOU WIN"
    M.pause()                       -- (the reward dialogue and the free pack are left out)
    local n = 0
    while not M.inCallback("CB_MainMenu") do
        n = n + 1
        if n > 60 then
            error("timeout back to the main menu")
        end
        M.press(n < 9 and M.KEY.A or M.KEY.B, nil, 60)
    end
    M.wait(45)
    M.menuTo(M.MENU.CAMPAIGN)
    M.resume()
    M.press(M.KEY.A)
    M.waitCallback("CB_Campaign", 200)  -- the page last played: the new duelist's portrait is open
    M.stop()
end)
