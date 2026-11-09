-- AUTO-GENERATED from TESTING.md
-- Recording: United_We_Stand_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "United We Stand",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Equip **United We Stand** to **Mystic Tomato**. Add or remove monsters/spells (**Summoned Skull**, **Change of Heart**) on your field and confirm ATK/DEF changes match the coded counters.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
