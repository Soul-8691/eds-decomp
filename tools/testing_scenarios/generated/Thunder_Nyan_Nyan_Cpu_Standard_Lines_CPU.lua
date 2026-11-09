-- AUTO-GENERATED from TESTING.md
-- Recording: Thunder_Nyan_Nyan_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Thunder Nyan Nyan",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Chaos** duel (or Test Duel with the format opponent deck): give the **opponent** `Thunder Nyan Nyan` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Thunder Nyan Nyan** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
