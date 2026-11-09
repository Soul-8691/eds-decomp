-- AUTO-GENERATED from TESTING.md
-- Recording: Threatening_Roar_Chain_Window_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Threatening Roar",
    effect = "chain_window",
    actor = "CPU",
    scenarioText = "Second pass: open a chain window the policy cares about (Normal/Special Summon, attack declaration, or your Spell activation) and confirm the opponent flips/sets/chains **Threatening Roar** when `CanActivateFieldCard` allows.",
    play = nil,
    recordFrames = 3600,
})
