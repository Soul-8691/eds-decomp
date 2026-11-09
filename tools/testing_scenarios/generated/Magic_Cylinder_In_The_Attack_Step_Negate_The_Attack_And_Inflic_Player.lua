-- AUTO-GENERATED from TESTING.md
-- Recording: Magic_Cylinder_In_The_Attack_Step_Negate_The_Attack_And_Inflic_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Magic Cylinder",
    effect = "in_the_attack_step_negate_the_attack_and_inflic",
    actor = "Player",
    scenarioText = "Opponent attacks your face-down **Man-Eater Bug** with **Summoned Skull** (note its ATK). Activate **Magic Cylinder** in the attack step: negate the attack and inflict that ATK as damage; **Man-Eater Bug** should not be destroyed by battle.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
