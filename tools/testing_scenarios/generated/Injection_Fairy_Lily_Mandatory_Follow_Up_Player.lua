-- AUTO-GENERATED from TESTING.md
-- Recording: Injection_Fairy_Lily_Mandatory_Follow_Up_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Injection Fairy Lily",
    effect = "mandatory_follow_up",
    actor = "Player",
    scenarioText = "Same duel: after **Injection Fairy Lily**'s mandatory effect runs (see **Coded behavior**—not an optional Trigger you activate), confirm the ROM applies it automatically. Opponent cannot chain **Magic Jammer** to that mandatory step; they may still use it in a later open window.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
