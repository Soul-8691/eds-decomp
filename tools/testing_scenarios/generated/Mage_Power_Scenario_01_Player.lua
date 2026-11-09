-- AUTO-GENERATED from TESTING.md
-- Recording: Mage_Power_Scenario_01_Player.mp4
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
    card = "Mage Power",
    effect = "scenario_01",
    actor = "Player",
    format = "Android",
    scenarioText = "Equip **Mage Power** to **Mystic Tomato**. Add or remove monsters/spells (**Summoned Skull**, **Change of Heart**) on your field and confirm ATK/DEF changes match the coded counters.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
