-- AUTO-GENERATED from TESTING.md
-- Recording: Jar_of_Greed_Draw_1_Card_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Jar of Greed",
    effect = "Draw_1_card",
    actor = "Player",
    scenarioText = "Open with **Jar of Greed** while opponent controls only **Summoned Skull**. Count hand size before/after relative to **Change of Heart** in the opponent list; repeat when your Standby/Draw Phase trigger applies.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
