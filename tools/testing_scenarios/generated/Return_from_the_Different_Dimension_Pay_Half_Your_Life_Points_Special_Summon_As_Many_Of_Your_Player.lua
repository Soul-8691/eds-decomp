-- AUTO-GENERATED from TESTING.md
-- Recording: Return_from_the_Different_Dimension_Pay_Half_Your_Life_Points_Special_Summon_As_Many_Of_Your_Player.mp4
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
    card = "Return from the Different Dimension",
    effect = "Pay_half_your_Life_Points_Special_Summon_as_many_of_your",
    actor = "Player",
    format = "Warrior",
    scenarioText = "Campaign duel vs the **Warrior** opponent: put **Return from the Different Dimension** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Return from the Different Dimension** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
