-- AUTO-GENERATED from TESTING.md
-- Recording: Reinforcement_of_the_Army_Add_1_Level_4_Or_Lower_Warrior_Monster_From_Your_Deck_To_Player.mp4
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
    card = "Reinforcement of the Army",
    effect = "Add_1_Level_4_or_lower_Warrior_monster_from_your_Deck_to",
    actor = "Player",
    format = "Yata",
    scenarioText = "Campaign duel vs the **Yata** opponent: put **Reinforcement of the Army** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Mystic Tomato**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Reinforcement of the Army** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
