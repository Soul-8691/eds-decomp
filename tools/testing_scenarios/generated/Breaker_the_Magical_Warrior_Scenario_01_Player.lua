-- AUTO-GENERATED from TESTING.md
-- Recording: Breaker_the_Magical_Warrior_Scenario_01_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Breaker the Magical Warrior",
    effect = "scenario_01",
    actor = "Player",
    scenarioText = "Normal Summon **Breaker the Magical Warrior**; opponent sets **Man-Eater Bug** and **Call of the Haunted**. Battle Phase: attack **Summoned Skull**, gain the Spell/Trap destruction trigger, then destroy the set **Call of the Haunted** and confirm once-per-turn tracking.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
