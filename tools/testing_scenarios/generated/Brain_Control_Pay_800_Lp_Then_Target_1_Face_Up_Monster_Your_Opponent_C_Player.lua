-- AUTO-GENERATED from TESTING.md
-- Recording: Brain_Control_Pay_800_Lp_Then_Target_1_Face_Up_Monster_Your_Opponent_C_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Brain Control",
    effect = "Pay_800_LP_then_target_1_face_up_monster_your_opponent_c",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Goat** opponent: put **Brain Control** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Brain Control** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
