-- AUTO-GENERATED from TESTING.md
-- Recording: Poison_of_the_Old_Man_Activate_1_Of_These_Effects_Gain_1200_Lp_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Poison of the Old Man",
    effect = "Activate_1_of_these_effects_Gain_1200_LP",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Scientist** opponent: put **Poison of the Old Man** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Poison of the Old Man** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
