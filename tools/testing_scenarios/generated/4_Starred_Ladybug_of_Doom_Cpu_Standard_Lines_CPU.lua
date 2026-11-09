-- AUTO-GENERATED from TESTING.md
-- Recording: 4_Starred_Ladybug_of_Doom_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "4-Starred Ladybug of Doom",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Imperial** duel (or Test Duel with the format opponent deck): give the **opponent** `4-Starred Ladybug of Doom` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **4-Starred Ladybug of Doom** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
