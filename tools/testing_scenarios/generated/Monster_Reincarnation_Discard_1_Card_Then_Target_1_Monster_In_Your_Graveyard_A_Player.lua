-- AUTO-GENERATED from TESTING.md
-- Recording: Monster_Reincarnation_Discard_1_Card_Then_Target_1_Monster_In_Your_Graveyard_A_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Monster Reincarnation",
    effect = "Discard_1_card_then_target_1_monster_in_your_Graveyard_a",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Warrior** opponent: put **Monster Reincarnation** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Monster Reincarnation** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
