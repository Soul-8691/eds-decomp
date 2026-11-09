-- AUTO-GENERATED from TESTING.md
-- Recording: Bottomless_Trap_Hole_When_Your_Opponent_Summons_A_Monsters_With_1500_Or_More_Player.mp4
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
    card = "Bottomless Trap Hole",
    effect = "When_your_opponent_Summons_a_monsters_with_1500_or_more",
    actor = "Player",
    format = "Warrior",
    scenarioText = "Opponent Special Summons or Normal Summons **Summoned Skull** with 1500 or less ATK. Flip **Bottomless Trap Hole** in response: **Summoned Skull** is destroyed; opponent cannot chain a Normal Spell (e.g. **Change of Heart**)—only a Trap or Quick-Play Spell such as **Magic Jammer**.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
