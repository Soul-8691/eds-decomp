-- AUTO-GENERATED from TESTING.md
-- Recording: Brain_Control_Chain_Rules_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Brain Control",
    effect = "chain_rules",
    actor = "Player",
    scenarioText = "Same board: when you activate **Brain Control**, opponent may chain **Magic Jammer** (or **Book of Moon**). Confirm stack order. Normal Spells such as **Pot of Greed** cannot be chained—only Traps and Quick-Play Spells.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
