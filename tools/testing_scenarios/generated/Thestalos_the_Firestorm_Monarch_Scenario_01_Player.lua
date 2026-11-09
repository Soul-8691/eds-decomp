-- AUTO-GENERATED from TESTING.md
-- Recording: Thestalos_the_Firestorm_Monarch_Scenario_01_Player.mp4
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
    card = "Thestalos the Firestorm Monarch",
    effect = "scenario_01",
    actor = "Player",
    format = "Reaper",
    scenarioText = "Tribute **Breaker the Magical Warrior** and **Summoned Skull** (or one Level 4 from the pool) for **Thestalos the Firestorm Monarch**. On summon, hit the effect against opponent card(s) **Summoned Skull** / **Book of Moon** / hand, and test on your next Standby if a Monarch trigger repeats.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
