-- AUTO-GENERATED from TESTING.md
-- Recording: Don_Zaloog_Scenario_01_Player.mp4
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
    card = "Don Zaloog",
    effect = "scenario_01",
    actor = "Player",
    format = "Yata",
    scenarioText = "Opponent attacks **Don Zaloog** with **Summoned Skull**; confirm destruction immunity and discards or battle trigger. Use **Spirit Reaper** or **Change of Heart** from the pool to test interactions with non-destruction removal.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
