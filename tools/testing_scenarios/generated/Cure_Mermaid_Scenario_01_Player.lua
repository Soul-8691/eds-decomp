-- AUTO-GENERATED from TESTING.md
-- Recording: Cure_Mermaid_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Cure Mermaid",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Yata** opponent: put **Cure Mermaid** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Mystic Tomato**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Cure Mermaid** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
