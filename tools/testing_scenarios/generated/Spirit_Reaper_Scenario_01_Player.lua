-- AUTO-GENERATED from TESTING.md
-- Recording: Spirit_Reaper_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Spirit Reaper",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Opponent attacks **Spirit Reaper** with **Summoned Skull**; confirm destruction immunity and discards or battle trigger. Use **Spirit Reaper** or **Change of Heart** from the pool to test interactions with non-destruction removal.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
