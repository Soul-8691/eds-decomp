-- AUTO-GENERATED from TESTING.md
-- Recording: Breaker_the_Magical_Warrior_Scenario_01_Player.mp4
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
    card = "Breaker the Magical Warrior",
    effect = "scenario_01",
    actor = "Player",
    format = "Scientist",
    scenarioText = "Normal Summon **Breaker the Magical Warrior**; opponent sets **Man-Eater Bug** and **Call of the Haunted**. Battle Phase: attack **Summoned Skull**, gain the Spell/Trap destruction trigger, then destroy the set **Call of the Haunted** and confirm once-per-turn tracking.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
