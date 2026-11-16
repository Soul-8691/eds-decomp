-- Opening a pack (README_TODO.md, Cards, packs and DP): screenshots/gifs/pack-pull.gif, the Goat usage pack bought
-- in Booster Packs (a fresh save has 5000 DP; a format pack costs 500), its five cards, and the first one's details.
-- No scrolling: the list opens on Goat. The pack's own pool favours low-usage cards, so five Goat staples from that
-- pool (usage/goat.json) are written over the generated pack before it is shown (the user's choice, 2026-11-16).
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

local STAPLES = { "Sakuretsu Armor", "Nobleman of Crossout", "Gravekeeper's Spy", "Thunder Dragon", "Book of Moon" }

-- gPackOpenWork (include/booster.h): +0x000 openPackId, +0x102 cardNumbers[5]
local OPEN_PACK_ID, CARD_NUMBERS = 0x000, 0x102

M.run("pack_pull", function()
    M.toMainMenu()
    local packId = M.formatPackId("Goat")
    local numbers = {}
    for i, name in ipairs(STAPLES) do
        numbers[i] = M.read16(M.sym("gCardIdToNumber") + M.cardId(name) * 2)
    end
    M.menuTo(M.MENU.BOOSTER_PACKS)
    M.press(M.KEY.A)
    M.packShopCentred(packId)
    M.waitCallback("CB_PackShop", 60)
    M.record("pack-pull", "gif")
    M.wait(70)                      -- the list, on Goat
    local work = M.sym("gPackOpenWork")
    M.write16(work + OPEN_PACK_ID, 0)
    M.press(M.KEY.A)                -- open
    M.waitUntil(function() return M.read16(work + OPEN_PACK_ID) == packId end, 300, "the Goat pack's cards")
    for i, n in ipairs(numbers) do
        M.write16(work + CARD_NUMBERS + (i - 1) * 2, n)
    end
    M.wait(210)                     -- the five cards come in
    M.press(M.KEY.A)                -- the first card's details
    M.wait(120)
    M.stop()
end)
