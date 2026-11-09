-- AUTO-GENERATED from TESTING.md
-- Recording: Enemy_Controller_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Enemy Controller",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Warrior** opponent: put **Enemy Controller** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Enemy Controller** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
