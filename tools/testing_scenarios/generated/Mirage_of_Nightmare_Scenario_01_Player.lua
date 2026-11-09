-- AUTO-GENERATED from TESTING.md
-- Recording: Mirage_of_Nightmare_Scenario_01_Player.mp4
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
    card = "Mirage of Nightmare",
    effect = "scenario_01",
    actor = "Player",
    format = "Yata",
    scenarioText = "You hold **Mirage of Nightmare** and three pool cards; opponent holds **Change of Heart**, **Summoned Skull**, and two others. Resolve **Mirage of Nightmare** and verify each side's hand size after equal discard and draw.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
