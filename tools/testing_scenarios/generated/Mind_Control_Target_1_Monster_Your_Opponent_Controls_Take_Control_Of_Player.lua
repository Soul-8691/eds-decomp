-- AUTO-GENERATED from TESTING.md
-- Recording: Mind_Control_Target_1_Monster_Your_Opponent_Controls_Take_Control_Of_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Mind Control",
    effect = "Target_1_monster_your_opponent_controls_take_control_of",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Goat** opponent: put **Mind Control** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Mind Control** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
