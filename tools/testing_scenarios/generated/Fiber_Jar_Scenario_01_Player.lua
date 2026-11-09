-- AUTO-GENERATED from TESTING.md
-- Recording: Fiber_Jar_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Fiber Jar",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Both players have damaged fields (**Mystic Tomato** vs **Summoned Skull**) and uneven graveyards. Flip **Fiber Jar** and resolve the FLIP effect; opponent may only respond with **Magic Jammer** (Trap or Quick-Play Spell), not a Normal Spell like **Change of Heart**. Confirm timing matches the ROM.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
