-- AUTO-GENERATED from TESTING.md
-- Recording: Gravekeepers_Spy_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Gravekeeper's Spy",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Set **Gravekeeper's Spy** face-down. Opponent attacks with **Summoned Skull**; after FLIP, Special Summon a Gravekeeper from deck (another legal Gravekeeper from the pool) and confirm **Summoned Skull** cannot destroy **Gravekeeper's Spy** by battle alone.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
