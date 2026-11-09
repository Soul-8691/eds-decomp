-- AUTO-GENERATED from TESTING.md
-- Recording: Mage_Power_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Mage Power",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Equip **Mage Power** to **Mystic Tomato**. Add or remove monsters/spells (**Summoned Skull**, **Change of Heart**) on your field and confirm ATK/DEF changes match the coded counters.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
