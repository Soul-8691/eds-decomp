-- AUTO-GENERATED from TESTING.md
-- Recording: Reinforcement_of_the_Army_Add_1_Level_4_Or_Lower_Warrior_Monster_From_Your_Deck_To_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Reinforcement of the Army",
    effect = "Add_1_Level_4_or_lower_Warrior_monster_from_your_Deck_to",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Yata** opponent: put **Reinforcement of the Army** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Mystic Tomato**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Reinforcement of the Army** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
