-- AUTO-GENERATED from TESTING.md
-- Recording: Berserk_Gorilla_If_This_Card_Is_In_Defense_Position_Destroy_It_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Berserk Gorilla",
    effect = "If_this_card_is_in_Defense_Position_Destroy_it",
    actor = "Player",
    scenarioText = "Opponent controls **Summoned Skull**; use **Berserk Gorilla**'s destruction effect per **Coded behavior**. Confirm targets; use **Magic Jammer** in a separate response window if the ROM allows, not a Normal Spell on the same chain.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
