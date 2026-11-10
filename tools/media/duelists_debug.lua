-- Duelists (Screen 4).png: the Campaign's last duelist page with the cursor on Debug (top right), from the release
-- ROM (make demo adds the Test Duel there):
--   run_tests.ps1 -Media -Filter duelists_debug -Rom eds_release_demo_extra.gba
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")
local LAST_PAGE = 4
local TOP_LEFT = 2            -- gOpponentSelectSlotPos: slot 2 is the top-left portrait
local TOP_RIGHT = 3           -- slot 3, top right: Debug on the last page (src/mod_opponents.c)

M.run("duelists_debug", function()
    M.toMainMenu()
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    local sel = M.sym("gOpponentSelect")
    -- (the screen opens on the page last played: L back to the first)
    for i = 1, 8 do
        if (M.read8(sel) & 7) == 0 then
            break
        end
        M.press(M.KEY.L, nil, 90)
    end
    for page = 1, 4 do
        local capture = page == LAST_PAGE
        M.waitUntil(function() return (M.read8(sel) & 7) == page - 1 end, 600, "duelist page " .. page)
        M.wait(60)
        -- the cursor on the top-left portrait (slot 2: the page's first format); on the last page, Debug (top right)
        local want = page == 4 and TOP_RIGHT or TOP_LEFT
        for _ = 1, 5 do
            if (M.read16(sel) >> 3) & 7 == want then
                break
            end
            M.press(M.KEY.LEFT, nil, 20)
        end
        M.wait(40)
        if capture then
            M.shot(string.format("Duelists (Screen %d)", page))
        end
        if page < 4 then
            M.press(M.KEY.R, nil, 20)
        end
    end
end)
