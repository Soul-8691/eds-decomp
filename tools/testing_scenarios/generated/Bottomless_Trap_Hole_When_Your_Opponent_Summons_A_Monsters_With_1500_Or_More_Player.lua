-- AUTO-GENERATED from TESTING.md
-- Recording: Bottomless_Trap_Hole_When_Your_Opponent_Summons_A_Monsters_With_1500_Or_More_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Bottomless Trap Hole",
    effect = "When_your_opponent_Summons_a_monsters_with_1500_or_more",
    actor = "Player",
    scenarioText = "Opponent Special Summons or Normal Summons **Summoned Skull** with 1500 or less ATK. Flip **Bottomless Trap Hole** in response: **Summoned Skull** is destroyed; opponent cannot chain a Normal Spell (e.g. **Change of Heart**)—only a Trap or Quick-Play Spell such as **Magic Jammer**.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
