-- AUTO-GENERATED from TESTING.md
-- Recording: Gearfried_the_Iron_Knight_Scenario_01_Player.mp4
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
    card = "Gearfried the Iron Knight",
    effect = "scenario_01",
    actor = "Player",
    format = "Yata",
    scenarioText = "Opponent controls **Summoned Skull**; use **Gearfried the Iron Knight**'s destruction effect per **Coded behavior**. Confirm targets; use **Magic Jammer** in a separate response window if the ROM allows, not a Normal Spell on the same chain.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
