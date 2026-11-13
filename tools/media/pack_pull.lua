-- Opening a pack (README_TODO.md, Cards, packs and DP): screenshots/gifs/pack-pull.gif, the Goat usage pack bought
-- in Booster Packs (a fresh save has 5000 DP; a format pack costs 500), its five cards, and the first one's details.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("pack_pull", function()
    M.toMainMenu()
    M.menuTo(M.MENU.BOOSTER_PACKS)
    M.press(M.KEY.A)
    M.packShopCentred(M.formatPackId("Goat"))
    M.waitCallback("CB_PackShop", 60)
    M.record("pack-pull", "gif")
    M.wait(90)                      -- the list: a look at the packs either side, back to Goat
    M.press(M.KEY.RIGHT, nil, 70)
    M.press(M.KEY.LEFT, nil, 70)
    M.press(M.KEY.LEFT, nil, 70)
    M.press(M.KEY.RIGHT, nil, 70)
    M.press(M.KEY.A)                -- open: the five cards come in
    M.wait(210)
    M.press(M.KEY.A)                -- the first card's details
    M.wait(120)
    M.stop()
end)
