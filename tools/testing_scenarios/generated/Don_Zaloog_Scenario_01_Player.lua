-- AUTO-GENERATED from TESTING.md
-- Recording: Don_Zaloog_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Don Zaloog",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Opponent attacks **Don Zaloog** with **Summoned Skull**; confirm destruction immunity and discards or battle trigger. Use **Spirit Reaper** or **Change of Heart** from the pool to test interactions with non-destruction removal.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
