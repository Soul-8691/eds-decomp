-- AUTO-GENERATED from TESTING.md
-- Recording: Dark_Magician_of_Chaos_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Dark Magician of Chaos",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Chaos** duel (or Test Duel with the format opponent deck): give the **opponent** `Dark Magician of Chaos` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Dark Magician of Chaos** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
