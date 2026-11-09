-- AUTO-GENERATED from TESTING.md
-- Recording: Book_of_Moon_And_List_Every_Monster_Destroyed_Check_Whether_Player.mp4
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
    card = "Book of Moon",
    effect = "and_list_every_monster_destroyed_check_whether",
    actor = "Player",
    format = "Yata",
    scenarioText = "You: **Mystic Tomato** + **Spirit Reaper**; opponent: **Summoned Skull** + set **Man-Eater Bug**. Activate **Book of Moon** and list every monster destroyed; check whether **Man-Eater Bug** FLIP triggers when applicable.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
