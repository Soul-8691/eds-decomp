-- Cards a recipe left out (README_TODO.md, Decks and banlists): Left Out.png. Three of the Chaos Turbo recipe's
-- cards are taken out of the trunk (a collection that lacks them), so loading the recipe leaves them out and Deck
-- Recipes lists them with the reason (src/mod_recipes.c MODE_LEFT_OUT). (make ignores the banlist when loading, so
-- the trunk is the reason here.)
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")
local MODE_LEFT_OUT = 3

M.run("left_out", function()
    M.toMainMenu()
    for _, name in ipairs({ "Black Luster Soldier - Envoy of Beginning", "Chaos Sorcerer", "Jar of Greed" }) do
        M.takeFromTrunk(name)
    end
    M.openMenu(M.MENU.DECK_RECIPES, "CB_DeckRecipes", 75)
    M.loadRecipe("Chaos Turbo")
    M.waitUntil(function() return M.read8(M.sym("gDeckRecipes") + 3) == MODE_LEFT_OUT end, 300, "the left-out list")
    M.wait(40)
    M.shot("Left Out")
end)
