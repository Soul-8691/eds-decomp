-- AUTO-GENERATED from TESTING.md
-- Recording: Mask_of_Restrict_Neither_Player_Can_Tribute_Cards_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Mask of Restrict",
    effect = "Neither_player_can_Tribute_cards",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Reaper** opponent: put **Mask of Restrict** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Mask of Restrict** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
