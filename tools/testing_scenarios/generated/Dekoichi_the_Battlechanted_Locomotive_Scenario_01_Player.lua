-- AUTO-GENERATED from TESTING.md
-- Recording: Dekoichi_the_Battlechanted_Locomotive_Scenario_01_Player.mp4
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
    card = "Dekoichi the Battlechanted Locomotive",
    effect = "scenario_01",
    actor = "Player",
    format = "Goat",
    scenarioText = "Set **Dekoichi the Battlechanted Locomotive**; opponent attacks with **Summoned Skull** or flips with **Card Destruction**. Resolve the FLIP effect against a legal target still in the pool (**Summoned Skull** or **Breaker the Magical Warrior**).",
    play = { { wait = 90 } },
    recordFrames = nil,
})
