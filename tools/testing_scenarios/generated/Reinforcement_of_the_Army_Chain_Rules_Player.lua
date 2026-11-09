-- AUTO-GENERATED from TESTING.md
-- Recording: Reinforcement_of_the_Army_Chain_Rules_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Reinforcement of the Army",
    effect = "chain_rules",
    actor = "Player",
    scenarioText = "Same board: when you activate **Reinforcement of the Army**, opponent may chain **Magic Jammer** (or **Book of Moon**). Confirm stack order. Normal Spells such as **Change of Heart** cannot be chained—only Traps and Quick-Play Spells.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
