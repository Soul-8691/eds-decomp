-- Deck Edit's Empty Main/Side (README_TODO.md): screenshots/gifs/empty-main-side.gif, the Chaos Turbo recipe's Main
-- Deck emptied into the trunk in one press (src/mod_deck_edit.c ModDeckEditEmptyMainAndSide).
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("empty_main_side", function()
    M.toMainMenu()
    M.openMenu(M.MENU.DECK_RECIPES, "CB_DeckRecipes", 75)
    M.loadRecipe("Chaos Turbo")
    M.back()
    M.openMenu(M.MENU.DECK_EDIT, "CB_DeckEdit", 120)
    M.deckEditMainDeckOn("Tribe-Infecting Virus")
    M.wait(40)
    M.record("empty-main-side", "gif")
    M.wait(30)
    M.deckEditCommand(M.DECK_CMD.EMPTY)
    M.wait(150)
    M.stop()
end)
