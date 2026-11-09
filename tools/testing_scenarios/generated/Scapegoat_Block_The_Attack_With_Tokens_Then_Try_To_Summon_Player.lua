-- AUTO-GENERATED from TESTING.md
-- Recording: Scapegoat_Block_The_Attack_With_Tokens_Then_Try_To_Summon_Player.mp4
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
    card = "Scapegoat",
    effect = "block_the_attack_with_tokens_then_try_to_summon",
    actor = "Player",
    format = "Joey-Pegasus",
    scenarioText = "Opponent attacks your **Bazoo the Soul-Eater** with **Summoned Skull**. With four open zones and no summon yet, activate **Scapegoat**, block the attack with tokens, then try to summon **Summoned Skull** the same turn (should fail).",
    play = { { wait = 90 } },
    recordFrames = nil,
})
