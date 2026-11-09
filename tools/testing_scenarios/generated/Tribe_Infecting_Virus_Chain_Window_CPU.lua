-- AUTO-GENERATED from TESTING.md
-- Recording: Tribe_Infecting_Virus_Chain_Window_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Tribe-Infecting Virus",
    effect = "chain_window",
    actor = "CPU",
    scenarioText = "Second pass: open a chain window the policy cares about (Normal/Special Summon, attack declaration, or your Spell activation) and confirm the opponent flips/sets/chains **Tribe-Infecting Virus** when `CanActivateFieldCard` allows.",
    play = nil,
    recordFrames = 3600,
})
