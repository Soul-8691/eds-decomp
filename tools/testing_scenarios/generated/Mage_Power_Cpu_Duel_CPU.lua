-- AUTO-GENERATED from TESTING.md
-- Recording: Mage_Power_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Mage Power",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Android** duel (or Test Duel with the format opponent deck): give the **opponent** `Mage Power` in the main/extra deck at a realistic copy count from `usage/` decks. Verify the CPU plays **Mage Power** when its generic spell/monster heuristics allow (same class as vanilla cards with that effect key).",
    play = nil,
    recordFrames = 3600,
})
