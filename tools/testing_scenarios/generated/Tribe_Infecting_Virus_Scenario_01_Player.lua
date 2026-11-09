-- AUTO-GENERATED from TESTING.md
-- Recording: Tribe_Infecting_Virus_Scenario_01_Player.mp4
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
    card = "Tribe-Infecting Virus",
    effect = "scenario_01",
    actor = "Player",
    format = "Scientist",
    scenarioText = "Your field: **Tribe-Infecting Virus** plus discard fodder; opponent fields **Gearfried the Iron Knight**, **Summoned Skull**, and **Breaker the Magical Warrior**. Declare Machine, discard a Machine from the pool, and confirm only the opponent's Machines are destroyed.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
