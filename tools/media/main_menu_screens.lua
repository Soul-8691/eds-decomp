-- The main menu's added screens, each with its main menu picture first (Pack Opener on Reaper's pack, Deck Edit on
-- Tribe-Infecting Virus in the Chaos Turbo recipe's Main Deck):
--   Settings (Main Menu).png, Settings.png, Deck Recipes (Main Menu).png, Deck Recipes.png,
--   Banlists (Main Menu).png, Banlists.png, Booster Packs (Main Menu).png, Pack Opener.png, Deck Edit.png
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

local PACK = "Reaper"                       -- the pack in the centre of Pack Opener.png
local DECK_EDIT_RECIPE = "Chaos Turbo"            -- Deck Edit.png: this recipe loaded, its Main Deck shown
local DECK_EDIT_CARD = "Tribe-Infecting Virus"   -- with the cursor on this card
local buildFile, loadRecipe, deckEditMainDeckOn, back = M.buildFile, M.loadRecipe, M.deckEditMainDeckOn, M.back
local formatPackId, packShopCentred = M.formatPackId, M.packShopCentred







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
