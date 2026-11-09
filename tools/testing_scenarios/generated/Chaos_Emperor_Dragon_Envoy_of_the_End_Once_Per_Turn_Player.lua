-- AUTO-GENERATED from TESTING.md
-- Recording: Chaos_Emperor_Dragon_Envoy_of_the_End_Once_Per_Turn_Player.mp4
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
    card = "Chaos Emperor Dragon - Envoy of the End",
    effect = "Once_per_turn",
    actor = "Player",
    format = "Chaos",
    scenarioText = "Your hand/field: enough LIGHT and DARK monsters from the pool (**Summoned Skull**, **Breaker the Magical Warrior**, etc.). Special Summon **Chaos Emperor Dragon - Envoy of the End**, pay 1000 LP, return all field cards and hands to the Deck, inflict 300 per returned card, and confirm **Change of Heart** cannot be played mid-resolution.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
