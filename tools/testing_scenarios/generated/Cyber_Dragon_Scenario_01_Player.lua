-- AUTO-GENERATED from TESTING.md
-- Recording: Cyber_Dragon_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Cyber Dragon",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Opponent controls **Summoned Skull** and you hold **Cyber Dragon** with an empty field. Special Summon **Cyber Dragon** from hand, attack into **Summoned Skull**, then overlay with format-legal Fusion/Synchro pieces if present in the pool.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
