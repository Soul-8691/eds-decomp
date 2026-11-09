-- AUTO-GENERATED from TESTING.md
-- Recording: Bazoo_the_Soul_Eater_Scenario_01_Player.mp4
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
    card = "Bazoo the Soul-Eater",
    effect = "scenario_01",
    actor = "Player",
    format = "Android",
    scenarioText = "Opponent Graveyard: two high-ATK monsters from the pool (**Summoned Skull**, **Summoned Skull**). Battle with **Bazoo the Soul-Eater**: after battle damage, banish up to two opponent Graveyard cards (or activate **Call of the Haunted** first to refill targets).",
    play = { { wait = 90 } },
    recordFrames = nil,
})
