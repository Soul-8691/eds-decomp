-- Deck Recipes before an event duel (README_TODO.md, Event challengers): Event Challenger.png. The day set to a Duel
-- Ceremony (the 2nd Saturday of a month: gSaveData.days +0x2150, day 0 being Monday 1 January 2001, so day 12 is
-- Saturday 13 January), a deck loaded, then the Campaign: the event's dialogue, then the challenger's Deck Recipes
-- screen (src/mod_recipes.c CB_ChallengerRecipes).
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("event_challenger", function()
    local save = M.sym("gSaveData")
    M.toMainMenu()
    M.write16(save + 0x2150, 12)
    M.openMenu(M.MENU.DECK_RECIPES, "CB_DeckRecipes", 75)
    M.loadRecipe("Chaos Turbo")
    M.back()
    M.menuTo(M.MENU.CAMPAIGN)
    M.press(M.KEY.A)
    -- A through the event's dialogue until the Campaign reaches the challenger's recipes (gMain.seqIndexCampaign
    -- +0x4857 is 1, the select step; gMain.subStep, bits 4-11 of +0x488A, is SELECT_STEP_CHALLENGER_RECIPES 3)
    local G_MAIN = 0x03000040
    local function atRecipes()
        return M.read8(G_MAIN + 0x4857) == 1 and ((M.read16(G_MAIN + 0x488A) >> 4) & 0xFF) == 3
    end
    local t = 0
    while not atRecipes() do
        t = t + 1
        if t > 6000 then
            error("timeout waiting for the challenger's recipes")
        end
        if t % 60 == 0 then
            M.press(M.KEY.A, nil, 10)
        else
            coroutine.yield()
        end
    end
    M.wait(120)                     -- the challenger's Deck Recipes fades in
    M.shot("Event Challenger")
end)
