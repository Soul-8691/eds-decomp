-- The main menu's added screens, each with its main menu picture first (Pack Opener on Reaper's pack, Deck Edit on
-- Tribe-Infecting Virus in the Chaos Turbo recipe's Main Deck):
--   Settings (Main Menu).png, Settings.png, Deck Recipes (Main Menu).png, Deck Recipes.png,
--   Banlists (Main Menu).png, Banlists.png, Booster Packs (Main Menu).png, Pack Opener.png, Deck Edit.png
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

local PACK = "Reaper"                       -- the pack in the centre of Pack Opener.png
local DECK_EDIT_RECIPE = "Chaos Turbo"            -- Deck Edit.png: this recipe loaded, its Main Deck shown
local DECK_EDIT_CARD = "Tribe-Infecting Virus"   -- with the cursor on this card

-- A line of a build header (build/include/...), or nil.
local function buildFile(name)
    local f = io.open(M.ROOT .. "/build/include/" .. name, "r")
    if not f then
        return nil
    end
    local t = f:read("*a")
    f:close()
    return t
end

-- The pack ID of the format pack named `name` (FORMAT_PACK_ID_FIRST 601 + its sFormatPackNames index).
local function formatPackId(name)
    local t = buildFile("format_pack_data.h") or ""
    local block = t:match("sFormatPackNames%b[]%s*=%s*(%b{})") or ""
    local i = 0
    for n in block:gmatch('"(.-)",') do
        if n == name then
            return 601 + i
        end
        i = i + 1
    end
    error("no format pack " .. name)
end

-- Booster Packs with `packId` in the centre: CB_PackShop's own restore (lastPackId, restore: it puts the last
-- opened pack back in the centre) is asked for while the list sets up.
local function packShopCentred(packId)
    local shop = M.sym("gPackShop")
    for _ = 1, 150 do
        M.write16(shop + 4, packId)         -- lastPackId
        M.write8(shop + 2, 1)               -- restore
        coroutine.yield()
    end
end

-- gDeckRecipes (src/mod_recipes.c struct DeckRecipes): cursor +0 (the slot), scroll +1 (the top row's slot),
-- active +2 (the recipe last loaded), mode +3 (0 list, 1 the actions, 3 the cards
-- a load left out), action +4; the slots' summaries from +0x2C, 36 bytes each (sizes, then the name at +4).
local RECIPE_COUNT, RECIPE_SUMMARY = 60, 36

-- Deck Recipes: the recipe whose name holds `pattern` loaded (cursor onto it with Down, A, Load, A), then back to
-- the main menu.
local function loadRecipe(pattern)
    local dr = M.sym("gDeckRecipes")
    local slot
    for n = 0, RECIPE_COUNT - 1 do
        local name = emu:readRange(dr + 0x2C + n * RECIPE_SUMMARY + 4, 32):match("^[^%z]*")
        if name:find(pattern, 1, true) then
            slot = n
            break
        end
    end
    if not slot then
        error("no recipe named like " .. pattern)
    end
    local function at() return M.read8(dr) end
    for _ = 1, RECIPE_COUNT * 3 do       -- (a press can be missed: checked after each one)
        if at() == slot then
            break
        end
        M.press(at() < slot and M.KEY.DOWN or M.KEY.UP, nil, 16)
    end
    if at() ~= slot then
        error("Deck Recipes cursor did not reach slot " .. slot)
    end
    M.press(M.KEY.A, nil, 20)                          -- the actions
    for _ = 1, 5 do
        if M.read8(dr + 4) == 0 then                   -- ACTION_LOAD
            break
        end
        M.press(M.KEY.LEFT, nil, 10)
    end
    for _ = 1, 3 do                                    -- (a load over a deck asks twice)
        if M.read8(dr + 3) ~= 1 then
            break
        end
        M.press(M.KEY.A, nil, 40)
    end
    M.wait(60)
    if M.read8(dr + 2) ~= slot then
        error("recipe " .. slot .. " was not loaded (active " .. M.read8(dr + 2) .. ")")
    end
    M.log("loaded recipe " .. slot .. " (" .. pattern .. ")")
end

-- Deck Edit: the Main Deck list (R selects the next list: deck_edit_list.c), its cursor scrolled onto
-- `name` with Down, so the rows on the way are drawn as in play.
local function deckEditMainDeckOn(name)
    local de = M.sym("gDeckEdit")
    local lists = M.sym("gDeckEditCardLists")
    local added = tonumber((buildFile("card_count.h") or ""):match("CARD_ID_COUNT_ADDED (%d+)") or "0")
    local trunkSize = 821 + added
    local rowSize = (trunkSize + 80 + 15) * 2
    for _ = 1, 3 do
        if M.read8(de + 0x1C1C) == 1 then              -- curList: the Main Deck
            break
        end
        M.press(M.KEY.R, nil, 30)
    end
    if M.read8(de + 0x1C1C) ~= 1 then
        error("Deck Edit: could not reach the Main Deck list")
    end
    local id = M.cardId(name)
    local base = lists + M.read8(de + 0x14A0 + 1) * rowSize + trunkSize * 2
    local target
    for i = 0, 79 do
        if (M.read16(base + i * 2) & 0x7FF) == id then
            target = i
            break
        end
    end
    if not target then
        error(name .. " is not in the Main Deck")
    end
    local function pos() return M.read16(de + 0x0620 + 2) end   -- listPos[1]
    for _ = 1, 90 do
        if pos() == target then
            break
        end
        M.press(pos() < target and M.KEY.DOWN or M.KEY.UP, nil, 10)
    end
    if pos() ~= target then
        error("Deck Edit cursor did not reach " .. name)
    end
end

local function back()
    -- B until the main menu again
    for _ = 1, 6 do
        if M.inCallback("CB_MainMenu") then
            break
        end
        M.press(M.KEY.B, nil, 40)
    end
    M.waitCallback("CB_MainMenu", 45)
end

M.run("main_menu_screens", function()
    M.toMainMenu()
    for _, s in ipairs({
        { M.MENU.SETTINGS, "CB_Settings", "Settings" },
        { M.MENU.DECK_RECIPES, "CB_DeckRecipes", "Deck Recipes" },
        { M.MENU.BANLISTS, "CB_Banlists", "Banlists" },
        { M.MENU.BOOSTER_PACKS, "CB_PackShop", "Booster Packs", "Pack Opener" },
    }) do
        M.menuTo(s[1])
        M.shot(s[3] .. " (Main Menu)")
        M.press(M.KEY.A)
        if s[2] == "CB_PackShop" then
            M.waitCallback(s[2], 0)
            packShopCentred(formatPackId(PACK))
            M.wait(60)
        else
            M.waitCallback(s[2], 75)
        end
        if s[2] == "CB_DeckRecipes" then
            -- the list from the top (it opens on the recipe last loaded or saved)
            local dr = M.sym("gDeckRecipes")
            for _ = 1, 180 do
                if M.read8(dr) == 0 then
                    break
                end
                M.press(M.KEY.UP, nil, 16)
            end
            M.wait(30)
        end
        M.shot(s[4] or s[3])
        back()
    end
    M.openMenu(M.MENU.DECK_RECIPES, "CB_DeckRecipes", 75)
    loadRecipe(DECK_EDIT_RECIPE)
    back()
    M.openMenu(M.MENU.DECK_EDIT, "CB_DeckEdit", 120)
    deckEditMainDeckOn(DECK_EDIT_CARD)
    M.wait(40)
    M.shot("Deck Edit")
end)
