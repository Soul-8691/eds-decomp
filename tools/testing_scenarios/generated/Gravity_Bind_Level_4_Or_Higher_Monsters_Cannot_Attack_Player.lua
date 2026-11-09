-- AUTO-GENERATED from TESTING.md
-- Recording: Gravity_Bind_Level_4_Or_Higher_Monsters_Cannot_Attack_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Gravity Bind",
    effect = "Level_4_or_higher_monsters_cannot_attack",
    actor = "Player",
    scenarioText = "Opponent attacks **Breaker the Magical Warrior** with **Summoned Skull**. Activate **Gravity Bind** during the attack step; opponent may chain **Magic Jammer** (or **Book of Moon**) on top—do not use Normal Spells like **Delinquent Duo** as chain links.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
