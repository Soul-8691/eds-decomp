-- AUTO-GENERATED from TESTING.md
-- Recording: Night_Assailant_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Night Assailant",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Goat** duel (or Test Duel with the format opponent deck): give the **opponent** `Night Assailant` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Night Assailant** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
