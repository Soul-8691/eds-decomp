-- A refused banlist switch (README_TODO.md, Banlists): Banlist Refused.png. From the release ROM, where the check
-- runs (src/mod_banlists.c: make has none), on a fresh save shrunk (the trunk emptied, 20 cards off
-- the starter deck), which cannot make a 40-card Main Deck under the Goat banlist:
--   run_tests.ps1 -Media -Filter banlist_refused -Rom eds_release_demo.gba
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")

M.run("banlist_refused", function()
    M.toMainMenu()
    M.shrinkCollection(20)
    M.banlistSelect(M.BANLIST.GOAT)
    M.wait(30)
    M.shot("Banlist Refused")
end)
