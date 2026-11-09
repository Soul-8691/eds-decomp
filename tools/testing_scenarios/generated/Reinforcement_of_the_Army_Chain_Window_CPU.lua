-- AUTO-GENERATED from TESTING.md
-- Recording: Reinforcement_of_the_Army_Chain_Window_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Reinforcement of the Army",
    effect = "chain_window",
    actor = "CPU",
    scenarioText = "Second pass: simplify the board until the hook conditions in **AI coverage** are met (extra hand size, weaker monsters, or an open monster zone) and re-check CPU behavior.",
    play = nil,
    recordFrames = 3600,
})
