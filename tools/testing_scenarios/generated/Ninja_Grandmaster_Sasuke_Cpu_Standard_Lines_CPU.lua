-- AUTO-GENERATED from TESTING.md
-- Recording: Ninja_Grandmaster_Sasuke_Cpu_Standard_Lines_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Ninja Grandmaster Sasuke",
    effect = "cpu_standard_lines",
    actor = "CPU",
    scenarioText = "Campaign **Warrior** duel (or Test Duel with the format opponent deck): give the **opponent** `Ninja Grandmaster Sasuke` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Ninja Grandmaster Sasuke** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
