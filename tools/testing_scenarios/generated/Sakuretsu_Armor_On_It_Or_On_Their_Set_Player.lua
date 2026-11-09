-- AUTO-GENERATED from TESTING.md
-- Recording: Sakuretsu_Armor_On_It_Or_On_Their_Set_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Sakuretsu Armor",
    effect = "on_it_or_on_their_set",
    actor = "Player",
    scenarioText = "Opponent controls **Summoned Skull**; you activate **Sakuretsu Armor** on it (or on their set **Call of the Haunted**). If opponent chains **Magic Jammer**, confirm stack order and resolution. **Change of Heart** is a Normal Spell and cannot be chained on that stack—only Traps and Quick-Play Spells can.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
