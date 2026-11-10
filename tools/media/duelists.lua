-- Duelists (Screen 1-3).png: the Campaign's format duelist pages (R turns the page), the cursor on each page's first
-- format (top left). Screen 4 (Debug's page) is duelists_debug.lua, from the release ROM.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")
local TOP_LEFT = 2            -- gOpponentSelectSlotPos: slot 2 is the top-left portrait
local TOP_RIGHT = 3           -- slot 3, top right: Debug on the last page (src/mod_opponents.c)

M.run("duelists", function()
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
    for page = 1, 3 do
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
        M.shot(string.format("Duelists (Screen %d)", page))
        if page < 3 then
            M.press(M.KEY.R, nil, 20)
        end
    end
end)
