-- AUTO-GENERATED from TESTING.md
-- Recording: Creature_Swap_Each_Player_Chooses_1_Monster_They_Control_And_Switches_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Creature Swap",
    effect = "Each_player_chooses_1_monster_they_control_and_switches",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Fiber** opponent: put **Creature Swap** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Mystic Tomato**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Creature Swap** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
