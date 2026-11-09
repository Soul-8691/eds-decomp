-- AUTO-GENERATED from TESTING.md
-- Recording: Level_Limit_Area_B_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Level Limit - Area B",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Goat** duel (or Test Duel with the format opponent deck): give the **opponent** `Level Limit - Area B` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Level Limit - Area B** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
