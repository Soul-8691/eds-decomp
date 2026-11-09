-- AUTO-GENERATED from TESTING.md
-- Recording: Offerings_to_the_Doomed_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Offerings to the Doomed",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Joey-Pegasus** duel (or Test Duel with the format opponent deck): give the **opponent** `Offerings to the Doomed` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Offerings to the Doomed** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
