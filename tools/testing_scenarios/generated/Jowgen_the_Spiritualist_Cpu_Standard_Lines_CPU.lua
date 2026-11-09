-- AUTO-GENERATED from TESTING.md
-- Recording: Jowgen_the_Spiritualist_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Jowgen the Spiritualist",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Scientist** duel (or Test Duel with the format opponent deck): give the **opponent** `Jowgen the Spiritualist` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Jowgen the Spiritualist** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
