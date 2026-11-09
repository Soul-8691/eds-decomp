-- AUTO-GENERATED from TESTING.md
-- Recording: Chaos_Emperor_Dragon_Envoy_of_the_End_Once_Per_Turn_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Chaos Emperor Dragon - Envoy of the End",
    effect = "Once_per_turn",
    actor = "Player",
    scenarioText = "Your hand/field: enough LIGHT and DARK monsters from the pool (**Summoned Skull**, **Breaker the Magical Warrior**, etc.). Special Summon **Chaos Emperor Dragon - Envoy of the End**, pay 1000 LP, return all field cards and hands to the Deck, inflict 300 per returned card, and confirm **Change of Heart** cannot be played mid-resolution.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
