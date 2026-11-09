-- AUTO-GENERATED from TESTING.md
-- Recording: Pot_of_Avarice_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Pot of Avarice",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Open with **Pot of Avarice** while opponent controls only **Summoned Skull**. Count hand size before/after relative to **Book of Moon** in the opponent list; repeat when your Standby/Draw Phase trigger applies.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
