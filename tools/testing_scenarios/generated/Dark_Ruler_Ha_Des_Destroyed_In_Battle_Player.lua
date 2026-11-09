-- AUTO-GENERATED from TESTING.md
-- Recording: Dark_Ruler_Ha_Des_Destroyed_In_Battle_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Dark Ruler Ha Des",
    effect = "destroyed_in_battle",
    actor = "Player",
    scenarioText = "You: **Mystic Tomato** + a stall monster from the pool; opponent: **Summoned Skull** + set **Man-Eater Bug**. Activate **Dark Ruler Ha Des** and list every monster destroyed; check whether **Man-Eater Bug** FLIP triggers when applicable.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
