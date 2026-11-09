-- AUTO-GENERATED from TESTING.md
-- Recording: Breaker_the_Magical_Warrior_Chain_Window_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Breaker the Magical Warrior",
    effect = "chain_window",
    actor = "CPU",
    scenarioText = "Second pass: open a chain window the policy cares about (Normal/Special Summon, attack declaration, or your Spell activation) and confirm the opponent flips/sets/chains **Breaker the Magical Warrior** when `CanActivateFieldCard` allows.",
    play = nil,
    recordFrames = 3600,
})
