-- AUTO-GENERATED from TESTING.md
-- Recording: Reinforcement_of_the_Army_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Reinforcement of the Army",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Yata** duel (or Test Duel with the format opponent deck): give the **opponent** `Reinforcement of the Army` in the main/extra deck at a realistic copy count from `usage/` decks. Let the CPU play the duel: it should activate/set/chain **Reinforcement of the Army** when its `AiWants*` / `AiShouldActivateAddedCard` policy says so (see **AI coverage**). Repeat until you see at least one successful CPU use or confirm the policy never fires on this board.",
    play = nil,
    recordFrames = 3600,
})
