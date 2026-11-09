-- AUTO-GENERATED from TESTING.md
-- Recording: Sakuretsu_Armor_On_It_Or_On_Their_Set_Player.mp4
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
    card = "Sakuretsu Armor",
    effect = "on_it_or_on_their_set",
    actor = "Player",
    format = "Vampire",
    scenarioText = "Opponent controls **Summoned Skull**; you activate **Sakuretsu Armor** on it (or on their set **Call of the Haunted**). If opponent chains **Magic Jammer**, confirm stack order and resolution. **Change of Heart** is a Normal Spell and cannot be chained on that stack—only Traps and Quick-Play Spells can.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
