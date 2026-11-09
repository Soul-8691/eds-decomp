-- AUTO-GENERATED from TESTING.md
-- Recording: Cyber_Dragon_Scenario_01_Player.mp4
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
    card = "Cyber Dragon",
    effect = "scenario_01",
    actor = "Player",
    format = "Cyber",
    scenarioText = "Opponent controls **Summoned Skull** and you hold **Cyber Dragon** with an empty field. Special Summon **Cyber Dragon** from hand, attack into **Summoned Skull**, then overlay with format-legal Fusion/Synchro pieces if present in the pool.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
