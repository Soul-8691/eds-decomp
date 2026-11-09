-- AUTO-GENERATED from TESTING.md
-- Recording: Pikerus_Circle_of_Enchantment_Damage_To_You_From_Card_Effects_Becomes_0_Until_The_End_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Pikeru's Circle of Enchantment",
    effect = "Damage_to_you_from_card_effects_becomes_0_until_the_end",
    actor = "Player",
    scenarioText = "Campaign duel vs the **Warrior** opponent: put **Pikeru's Circle of Enchantment** in your deck with pool backrow (**Call of the Haunted**) and monsters (**Breaker the Magical Warrior**, **Summoned Skull**). Opponent line: Normal Summon **Summoned Skull**, set **Man-Eater Bug**, pass. Play toward **Pikeru's Circle of Enchantment** as you would in the usage decks and verify effect once as human and once watching CPU.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
