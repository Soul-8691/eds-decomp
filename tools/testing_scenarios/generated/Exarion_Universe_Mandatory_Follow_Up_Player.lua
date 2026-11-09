-- AUTO-GENERATED from TESTING.md
-- Recording: Exarion_Universe_Mandatory_Follow_Up_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Exarion Universe",
    effect = "mandatory_follow_up",
    actor = "Player",
    scenarioText = "Same duel: after **Exarion Universe**'s mandatory effect runs (see **Coded behavior**—not an optional Trigger you activate), confirm the ROM applies it automatically. Opponent cannot chain **Magic Jammer** to that mandatory step; they may still use it in a later open window.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
