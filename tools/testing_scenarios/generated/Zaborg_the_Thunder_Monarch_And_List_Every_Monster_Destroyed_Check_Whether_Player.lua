-- AUTO-GENERATED from TESTING.md
-- Recording: Zaborg_the_Thunder_Monarch_And_List_Every_Monster_Destroyed_Check_Whether_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Zaborg the Thunder Monarch",
    effect = "and_list_every_monster_destroyed_check_whether",
    actor = "Player",
    scenarioText = "You: **Breaker the Magical Warrior** + **Breaker the Magical Warrior**; opponent: **Summoned Skull** + set **Man-Eater Bug**. Activate **Zaborg the Thunder Monarch** and list every monster destroyed; check whether **Man-Eater Bug** FLIP triggers when applicable.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
