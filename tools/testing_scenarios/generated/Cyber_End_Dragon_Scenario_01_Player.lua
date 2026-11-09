-- AUTO-GENERATED from TESTING.md
-- Recording: Cyber_End_Dragon_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Cyber End Dragon",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Cyber** opponent: put **Cyber End Dragon** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Cyber End Dragon** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
