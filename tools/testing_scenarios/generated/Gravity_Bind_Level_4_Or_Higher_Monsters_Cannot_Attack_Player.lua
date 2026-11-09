-- AUTO-GENERATED from TESTING.md
-- Recording: Gravity_Bind_Level_4_Or_Higher_Monsters_Cannot_Attack_Player.mp4
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
    card = "Gravity Bind",
    effect = "Level_4_or_higher_monsters_cannot_attack",
    actor = "Player",
    format = "Cyber",
    scenarioText = "Opponent attacks **Breaker the Magical Warrior** with **Summoned Skull**. Activate **Gravity Bind** during the attack step; opponent may chain **Magic Jammer** (or **Book of Moon**) on top—do not use Normal Spells like **Delinquent Duo** as chain links.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
