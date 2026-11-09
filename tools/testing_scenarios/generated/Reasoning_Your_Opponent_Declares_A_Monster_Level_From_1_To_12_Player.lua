-- AUTO-GENERATED from TESTING.md
-- Recording: Reasoning_Your_Opponent_Declares_A_Monster_Level_From_1_To_12_Player.mp4
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
    card = "Reasoning",
    effect = "Your_opponent_declares_a_monster_Level_from_1_to_12",
    actor = "Player",
    format = "Scientist",
    scenarioText = "Open with **Reasoning** while opponent controls only **Summoned Skull**. Count hand size before/after relative to **Card Destruction** in the opponent list; repeat when your Standby/Draw Phase trigger applies.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
