-- AUTO-GENERATED from TESTING.md
-- Recording: Thousand_Eyes_Restrict_And_List_Every_Monster_Destroyed_Check_Whether_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Thousand-Eyes Restrict",
    effect = "and_list_every_monster_destroyed_check_whether",
    actor = "Player",
    scenarioText = "You: **Mystic Tomato** + a stall monster from the pool; opponent: **Summoned Skull** + set **Man-Eater Bug**. Activate **Thousand-Eyes Restrict** and list every monster destroyed; check whether **Man-Eater Bug** FLIP triggers when applicable.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
