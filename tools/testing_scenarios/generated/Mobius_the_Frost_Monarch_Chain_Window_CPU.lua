-- AUTO-GENERATED from TESTING.md
-- Recording: Mobius_the_Frost_Monarch_Chain_Window_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Mobius the Frost Monarch",
    effect = "chain_window",
    actor = "CPU",
    scenarioText = "Second pass: simplify the board until the hook conditions in **AI coverage** are met (extra hand size, weaker monsters, or an open monster zone) and re-check CPU behavior.",
    play = nil,
    recordFrames = 3600,
})
