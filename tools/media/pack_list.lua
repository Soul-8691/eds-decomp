-- Scrolling through the packs (README_TODO.md, Booster Packs): screenshots/gifs/pack-list.gif. Booster Packs opened
-- on the first pack (Legend of Blue Eyes White Dragon), then Right through the list: each TCG set and usage pack with
-- its cover and price.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

local STEPS = 14                    -- packs passed
local HOLD = 45                     -- frames on each

M.run("pack_list", function()
    M.toMainMenu()
    M.menuTo(M.MENU.BOOSTER_PACKS)
    M.press(M.KEY.A)
    M.packShopCentred(601)          -- FORMAT_PACK_ID_FIRST: the list's first pack
    M.waitCallback("CB_PackShop", 60)
    M.wait(30)
    M.record("pack-list", "gif")
    M.wait(50)
    for _ = 1, STEPS do
        M.press(M.KEY.RIGHT, nil, HOLD)
    end
    M.wait(30)
    M.stop()
end)
