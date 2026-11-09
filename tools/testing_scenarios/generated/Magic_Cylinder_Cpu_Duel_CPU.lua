-- AUTO-GENERATED from TESTING.md
-- Recording: Magic_Cylinder_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Magic Cylinder",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Android** duel (or Test Duel with the format opponent deck): give the **opponent** `Magic Cylinder` in the main/extra deck at a realistic copy count from `usage/` decks. Verify the CPU plays **Magic Cylinder** when its generic spell/monster heuristics allow (same class as vanilla cards with that effect key).",
    play = nil,
    recordFrames = 3600,
})
