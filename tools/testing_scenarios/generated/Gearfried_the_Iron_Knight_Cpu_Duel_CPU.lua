-- AUTO-GENERATED from TESTING.md
-- Recording: Gearfried_the_Iron_Knight_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Gearfried the Iron Knight",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Yata** duel (or Test Duel with the format opponent deck): give the **opponent** `Gearfried the Iron Knight` in the main/extra deck at a realistic copy count from `usage/` decks. Verify the CPU plays **Gearfried the Iron Knight** when its generic spell/monster heuristics allow (same class as vanilla cards with that effect key).",
    play = nil,
    recordFrames = 3600,
})
