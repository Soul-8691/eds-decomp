-- AUTO-GENERATED from TESTING.md
-- Recording: Creature_Swap_Chain_Rules_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Creature Swap",
    effect = "chain_rules",
    actor = "Player",
    scenarioText = "Same board: when you activate **Creature Swap**, opponent may chain **Magic Jammer** (or **Scapegoat**). Confirm stack order. Normal Spells such as **Change of Heart** cannot be chained—only Traps and Quick-Play Spells.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
