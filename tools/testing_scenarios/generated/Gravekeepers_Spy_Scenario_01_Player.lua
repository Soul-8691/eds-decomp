-- AUTO-GENERATED from TESTING.md
-- Recording: Gravekeepers_Spy_Scenario_01_Player.mp4
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
    card = "Gravekeeper's Spy",
    effect = "scenario_01",
    actor = "Player",
    format = "Yata",
    scenarioText = "Set **Gravekeeper's Spy** face-down. Opponent attacks with **Summoned Skull**; after FLIP, Special Summon a Gravekeeper from deck (another legal Gravekeeper from the pool) and confirm **Summoned Skull** cannot destroy **Gravekeeper's Spy** by battle alone.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
