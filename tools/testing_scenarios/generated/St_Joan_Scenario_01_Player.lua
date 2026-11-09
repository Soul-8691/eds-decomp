-- AUTO-GENERATED from TESTING.md
-- Recording: St_Joan_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "St. Joan",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Android** opponent: put **St. Joan** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Mystic Tomato**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **St. Joan** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
