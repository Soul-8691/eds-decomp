-- AUTO-GENERATED from TESTING.md
-- Recording: Soul_Exchange_Tribute_Opponent_Monster_Player.mp4
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
    card = "Soul Exchange",
    effect = "tribute_opponent_monster",
    actor = "Player",
    format = "Yugi-Kaiba",
    scenarioText = "You: **Soul Exchange** plus a Level 5+ Tribute from the pool (e.g. **Summoned Skull**). Opponent fields **Summoned Skull** in Attack Position. Main Phase 1: activate **Soul Exchange** on **Summoned Skull**, Tribute Summon the high-Level card, confirm you skip the Battle Phase, and confirm the tribute removed **Summoned Skull** even though they controlled it.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
