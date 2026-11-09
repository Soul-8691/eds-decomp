-- AUTO-GENERATED from TESTING.md
-- Recording: Pikerus_Circle_of_Enchantment_Chain_Window_CPU.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Pikeru's Circle of Enchantment",
    effect = "chain_window",
    actor = "CPU",
    scenarioText = "Second pass: open a chain window the policy cares about (Normal/Special Summon, attack declaration, or your Spell activation) and confirm the opponent flips/sets/chains **Pikeru's Circle of Enchantment** when `CanActivateFieldCard` allows.",
    play = nil,
    recordFrames = 3600,
})
