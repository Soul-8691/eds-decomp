-- AUTO-GENERATED from TESTING.md
-- Recording: Return_from_the_Different_Dimension_Chain_Window_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Return from the Different Dimension",
    effect = "chain_window",
    actor = "CPU",
    scenarioText = "Second pass: open a chain window the policy cares about (Normal/Special Summon, attack declaration, or your Spell activation) and confirm the opponent flips/sets/chains **Return from the Different Dimension** when `CanActivateFieldCard` allows.",
    play = nil,
    recordFrames = 3600,
})
