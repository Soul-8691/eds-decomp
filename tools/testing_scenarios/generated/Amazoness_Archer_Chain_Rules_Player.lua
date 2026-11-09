-- AUTO-GENERATED from TESTING.md
-- Recording: Amazoness_Archer_Chain_Rules_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Amazoness Archer",
    effect = "chain_rules",
    actor = "Player",
    scenarioText = "Same board: use **Amazoness Archer** per **Coded behavior**. If the ROM offers a chain window to the opponent, it should accept only **Magic Jammer**-type responses (Trap or Quick-Play Spell), not Normal Spells.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
