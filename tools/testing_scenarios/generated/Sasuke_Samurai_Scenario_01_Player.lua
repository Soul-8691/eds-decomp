-- AUTO-GENERATED from TESTING.md
-- Recording: Sasuke_Samurai_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Sasuke Samurai",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Opponent controls **Summoned Skull**; use **Sasuke Samurai**'s destruction effect per **Coded behavior**. Confirm targets; use **Magic Jammer** in a separate response window if the ROM allows, not a Normal Spell on the same chain.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
