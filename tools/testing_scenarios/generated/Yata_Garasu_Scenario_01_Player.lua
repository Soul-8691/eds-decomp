-- AUTO-GENERATED from TESTING.md
-- Recording: Yata_Garasu_Scenario_01_Player.mp4
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
    card = "Yata-Garasu",
    effect = "scenario_01",
    actor = "Player",
    format = "Fiber",
    scenarioText = "You: empty field except **Yata-Garasu**; opponent has 6 cards including **Change of Heart**. Attack directly with **Yata-Garasu**, then pass turns until the opponent would draw: hand stays empty while **Yata-Garasu** is face-up.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
