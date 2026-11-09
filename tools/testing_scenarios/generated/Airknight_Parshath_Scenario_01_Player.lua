-- AUTO-GENERATED from TESTING.md
-- Recording: Airknight_Parshath_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Airknight Parshath",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Open with **Airknight Parshath** while opponent controls only **Summoned Skull**. Count hand size before/after relative to **Change of Heart** in the opponent list; repeat when your Standby/Draw Phase trigger applies.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
