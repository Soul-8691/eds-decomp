-- AUTO-GENERATED from TESTING.md
-- Recording: Magical_Merchant_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Magical Merchant",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Set **Magical Merchant**; opponent attacks with **Summoned Skull** or flips with **Book of Moon**. Resolve the FLIP effect against a legal target still in the pool (**Summoned Skull** or **Breaker the Magical Warrior**).",
    play = { { wait = 90 } },
    recordFrames = nil,
})
