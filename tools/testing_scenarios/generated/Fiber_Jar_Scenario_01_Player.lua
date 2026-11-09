-- AUTO-GENERATED from TESTING.md
-- Recording: Fiber_Jar_Scenario_01_Player.mp4
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
    card = "Fiber Jar",
    effect = "scenario_01",
    actor = "Player",
    format = "Fiber",
    scenarioText = "Both players have damaged fields (**Mystic Tomato** vs **Summoned Skull**) and uneven graveyards. Flip **Fiber Jar** and resolve the FLIP effect; opponent may only respond with **Magic Jammer** (Trap or Quick-Play Spell), not a Normal Spell like **Change of Heart**. Confirm timing matches the ROM.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
