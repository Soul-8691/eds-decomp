-- AUTO-GENERATED from TESTING.md
-- Recording: Return_from_the_Different_Dimension_Pay_Half_Your_Life_Points_Special_Summon_As_Many_Of_Your_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Return from the Different Dimension",
    effect = "Pay_half_your_Life_Points_Special_Summon_as_many_of_your",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Warrior** opponent: put **Return from the Different Dimension** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Return from the Different Dimension** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
