-- Records (README_TODO.md, Practice opponents): Records (Practice).png, the Records screen's last page (Solomon,
-- Debug, Empty Deck and Test Duel). R turns the page; each page is kept in the scratch log as it passes.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("records_practice", function()
    M.toMainMenu()
    M.openMenu(M.MENU.RECORD, "CB_Record", 120)
    for _ = 1, 3 do
        M.press(M.KEY.R, nil, 60)
    end
    M.wait(30)
    M.shot("Records (Practice)")
end)
