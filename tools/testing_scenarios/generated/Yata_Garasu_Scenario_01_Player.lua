-- AUTO-GENERATED from TESTING.md
-- Recording: Yata_Garasu_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Yata-Garasu",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "You: empty field except **Yata-Garasu**; opponent has 6 cards including **Change of Heart**. Attack directly with **Yata-Garasu**, then pass turns until the opponent would draw: hand stays empty while **Yata-Garasu** is face-up.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
