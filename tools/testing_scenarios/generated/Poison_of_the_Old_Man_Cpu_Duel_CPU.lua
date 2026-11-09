-- AUTO-GENERATED from TESTING.md
-- Recording: Poison_of_the_Old_Man_Cpu_Duel_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Poison of the Old Man",
    effect = "cpu_duel",
    actor = "CPU",
    scenarioText = "Campaign **Scientist** duel (or Test Duel with the format opponent deck): give the **opponent** `Poison of the Old Man` in the main/extra deck at a realistic copy count from `usage/` decks. Let the CPU play the duel: it should activate/set/chain **Poison of the Old Man** when its `AiWants*` / `AiShouldActivateAddedCard` policy says so (see **AI coverage**). Repeat until you see at least one successful CPU use or confirm the policy never fires on this board.",
    play = nil,
    recordFrames = 3600,
})
