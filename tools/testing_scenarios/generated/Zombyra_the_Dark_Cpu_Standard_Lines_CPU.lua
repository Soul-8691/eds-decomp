-- AUTO-GENERATED from TESTING.md
-- Recording: Zombyra_the_Dark_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Zombyra the Dark",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Android** duel (or Test Duel with the format opponent deck): give the **opponent** `Zombyra the Dark` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Zombyra the Dark** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
