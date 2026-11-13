-- The free pack reward's "new packs are available" text (README_TODO.md, Cards, packs and DP): New Packs.png. A match
-- against the Yugi-Kaiba duelist, decided by this duel: one duel counted as played and won (gMain +0x4888/+0x4889),
-- so the win makes 2-0;
-- gSaveData.unlockNotices bit 1 (new packs) is set, as a win that opens a usage pack sets it. The rewards step shows
-- the text and clears the bit (src/campaign_steps.c REWARD_STEP_NEW_PACKS_TEXT); the shot follows.
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")
local G_MAIN = 0x03000040

M.run("new_packs", function()
    local save = M.sym("gSaveData")
    M.toMainMenu()
    M.openMenu(M.MENU.DECK_RECIPES, "CB_DeckRecipes", 75)
    M.loadRecipe("Chaos Turbo")
    M.back()
    M.openMenu(M.MENU.CAMPAIGN, "CB_Campaign", 150)
    M.campaignPick(0, M.SLOT.TOP_LEFT)
    M.press(M.KEY.A)
    M.wait(150)
    M.press(M.KEY.A)
    M.rpsWin(true)
    M.toMainPhase()
    -- a match is decided after two duels: one counted as played (matchDuelCount, +0x4888 bits 4-5) and won
    -- (matchScore +0x4889), so this win makes 2-0
    M.write8(G_MAIN + 0x4888, (M.read8(G_MAIN + 0x4888) & ~0x30) | 0x10)
    M.write8(G_MAIN + 0x4889, 1)
    M.write16(save + 0x2164, M.read16(save + 0x2164) | 2)       -- unlockNotices: new packs
    M.winDuel()
    local t = 0
    while (M.read16(save + 0x2164) & 2) ~= 0 do                 -- A through the results to the rewards
        t = t + 1
        if t > 3000 then
            error("timeout waiting for the new packs text")
        end
        if t % 60 == 0 then
            M.press(M.KEY.A, nil, 10)
        else
            coroutine.yield()
        end
    end
    M.wait(150)                     -- "Have you managed to reinforce your Deck?"
    M.press(M.KEY.A)                -- then the new packs line, which types out slowly
    M.wait(330)
    M.shot("New Packs")
end)
