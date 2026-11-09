-- AUTO-GENERATED from TESTING.md
-- Recording: Goblin_Attack_Force_Usage_Deck_Verify_Player.mp4
local __edsScriptSrc = debug.getinfo(1, "S").source or ""
local function __edsTestingRoot()
    local src = __edsScriptSrc
    if src:sub(1, 1) == "@" then
        src = src:sub(2)
    end
    src = src:gsub("\\", "/")
    local dir = src:match("^(.*)/[^/]+$")
    if dir and (dir:match("/poc$") or dir:match("/generated$")) then
        return dir .. "/.."
    end
    if dir and dir:match("/testing_scenarios$") then
        return dir
    end
    return "tools/testing_scenarios"
end
local paths = dofile(__edsTestingRoot() .. "/eds_paths.lua")
local EDS = paths.loadEds()
EDS.run({
    card = "Goblin Attack Force",
    effect = "usage_deck_verify",
    actor = "Player",
    format = "Imperial",
    scenarioText = "Campaign duel vs the **Imperial** opponent: put **Goblin Attack Force** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Mystic Tomato**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Goblin Attack Force** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
