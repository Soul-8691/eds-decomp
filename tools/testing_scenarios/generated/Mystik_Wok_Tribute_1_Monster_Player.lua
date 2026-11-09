-- AUTO-GENERATED from TESTING.md
-- Recording: Mystik_Wok_Tribute_1_Monster_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Mystik Wok",
    effect = "Tribute_1_monster",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Warrior** opponent: put **Mystik Wok** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Mystik Wok** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
