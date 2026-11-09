-- AUTO-GENERATED from TESTING.md
-- Recording: Cold_Wave_Effect_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Cold Wave",
    effect = "effect",
    actor = "Player",
    scenarioText = "First turn of Main Phase 1, before any summon: activate **Cold Wave**. Both players try to Set/activate **Magic Jammer** or play Quick-Play Spells; Normal Spells such as **Change of Heart** must be blocked for two turns. Opponent later Normal Summons **Summoned Skull** under the lock-out.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
