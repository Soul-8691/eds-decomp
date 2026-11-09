-- AUTO-GENERATED from TESTING.md
-- Recording: Amazoness_Archer_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Amazoness Archer",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Vampire** duel (or Test Duel with the format opponent deck): give the **opponent** `Amazoness Archer` in the main/extra deck at a realistic copy count from `usage/` decks. Watch several turns; note any turn where a human could legally use **Amazoness Archer** but the CPU does not. File gaps against `src/added_effects.c` / `ai_deck.c`.",
    play = nil,
    recordFrames = 3600,
})
