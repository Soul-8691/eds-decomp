-- AUTO-GENERATED from TESTING.md
-- Recording: Monster_Gate_Tribute_1_Monster_Excavate_Cards_From_The_Top_Of_Your_De_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Monster Gate",
    effect = "Tribute_1_monster_excavate_cards_from_the_top_of_your_De",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Warrior** opponent: put **Monster Gate** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Monster Gate** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
