-- AUTO-GENERATED from TESTING.md
-- Recording: St_Joan_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "St. Joan",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Android** duel (or Test Duel with the format opponent deck): give the **opponent** `St. Joan` in the main/extra deck at a realistic copy count from `usage/` decks. Watch several turns; note any turn where a human could legally use **St. Joan** but the CPU does not. File gaps against `src/added_effects.c` / `ai_deck.c`.",
    play = nil,
    recordFrames = 3600,
})
