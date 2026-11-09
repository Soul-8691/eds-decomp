-- AUTO-GENERATED from TESTING.md
-- Recording: Night_Assailant_Flip_Monster_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Night Assailant",
    effect = "Flip_monster",
    actor = "Player",
    scenarioText = "You: **Breaker the Magical Warrior** + **Breaker the Magical Warrior**; opponent: **Summoned Skull** + set **Man-Eater Bug**. Activate **Night Assailant** and list every monster destroyed; check whether **Man-Eater Bug** FLIP triggers when applicable.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
