-- AUTO-GENERATED from TESTING.md
-- Recording: Cure_Mermaid_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Cure Mermaid",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Yata** duel (or Test Duel with the format opponent deck): give the **opponent** `Cure Mermaid` in the main/extra deck at a realistic copy count from `usage/` decks. Watch several turns; note any turn where a human could legally use **Cure Mermaid** but the CPU does not. File gaps against `src/added_effects.c` / `ai_deck.c`.",
    play = nil,
    recordFrames = 3600,
})
