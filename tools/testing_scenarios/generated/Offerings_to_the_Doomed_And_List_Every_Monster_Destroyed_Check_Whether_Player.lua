-- AUTO-GENERATED from TESTING.md
-- Recording: Offerings_to_the_Doomed_And_List_Every_Monster_Destroyed_Check_Whether_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Offerings to the Doomed",
    effect = "and_list_every_monster_destroyed_check_whether",
    actor = "Player",
    scenarioText = "You: **Bazoo the Soul-Eater** + **Bazoo the Soul-Eater**; opponent: **Summoned Skull** + set **Man-Eater Bug**. Activate **Offerings to the Doomed** and list every monster destroyed; check whether **Man-Eater Bug** FLIP triggers when applicable.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
