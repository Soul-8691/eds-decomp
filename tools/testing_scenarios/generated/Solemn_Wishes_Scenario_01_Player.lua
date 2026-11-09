-- AUTO-GENERATED from TESTING.md
-- Recording: Solemn_Wishes_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Solemn Wishes",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Set **Solemn Wishes**; opponent attacks with **Summoned Skull** or flips with **Change of Heart**. Resolve the FLIP effect against a legal target still in the pool (**Summoned Skull** or **Mystic Tomato**).",
    play = { { wait = 90 } },
    recordFrames = nil,
})
