-- AUTO-GENERATED from TESTING.md
-- Recording: Kycoo_the_Ghost_Destroyer_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Kycoo the Ghost Destroyer",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Android** duel (or Test Duel with the format opponent deck): give the **opponent** `Kycoo the Ghost Destroyer` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Kycoo the Ghost Destroyer** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
