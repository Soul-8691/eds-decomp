-- AUTO-GENERATED from TESTING.md
-- Recording: Tribe_Infecting_Virus_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Tribe-Infecting Virus",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Your field: **Tribe-Infecting Virus** plus discard fodder; opponent fields **Gearfried the Iron Knight**, **Summoned Skull**, and **Breaker the Magical Warrior**. Declare Machine, discard a Machine from the pool, and confirm only the opponent's Machines are destroyed.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
