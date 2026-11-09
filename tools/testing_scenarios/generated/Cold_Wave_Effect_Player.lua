-- AUTO-GENERATED from TESTING.md
-- Recording: Cold_Wave_Effect_Player.mp4
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
    card = "Cold Wave",
    effect = "effect",
    actor = "Player",
    format = "Chaos",
    scenarioText = "First turn of Main Phase 1, before any summon: activate **Cold Wave**. Both players try to Set/activate **Magic Jammer** or play Quick-Play Spells; Normal Spells such as **Change of Heart** must be blocked for two turns. Opponent later Normal Summons **Summoned Skull** under the lock-out.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
