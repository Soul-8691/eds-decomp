-- AUTO-GENERATED from TESTING.md
-- Recording: Poison_of_the_Old_Man_Chain_Window_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Poison of the Old Man",
    effect = "chain_window",
    actor = "CPU",
    scenarioText = "Second pass: open a chain window the policy cares about (Normal/Special Summon, attack declaration, or your Spell activation) and confirm the opponent flips/sets/chains **Poison of the Old Man** when `CanActivateFieldCard` allows.",
    play = nil,
    recordFrames = 3600,
})
