-- AUTO-GENERATED from TESTING.md
-- Recording: Return_from_the_Different_Dimension_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Return from the Different Dimension",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Warrior** duel (or Test Duel with the format opponent deck): give the **opponent** `Return from the Different Dimension` in the main/extra deck at a realistic copy count from `usage/` decks. Let the CPU play the duel: it should activate/set/chain **Return from the Different Dimension** when its `AiWants*` / `AiShouldActivateAddedCard` policy says so (see **AI coverage**). Repeat until you see at least one successful CPU use or confirm the policy never fires on this board.",
    play = nil,
    recordFrames = 3600,
})
