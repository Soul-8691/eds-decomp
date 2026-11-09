-- AUTO-GENERATED from TESTING.md
-- Recording: Threatening_Roar_Your_Opponent_Cannot_Declare_An_Attack_This_Turn_Player.mp4
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
    card = "Threatening Roar",
    effect = "Your_opponent_cannot_declare_an_attack_this_turn",
    actor = "Player",
    format = "Cyber",
    scenarioText = "Set **Threatening Roar**; opponent attacks with **Summoned Skull** or flips with **Delinquent Duo**. Resolve the FLIP effect against a legal target still in the pool (**Summoned Skull** or **Breaker the Magical Warrior**).",
    play = { { wait = 90 } },
    recordFrames = nil,
})
