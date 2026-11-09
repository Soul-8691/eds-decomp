-- AUTO-GENERATED from TESTING.md
-- Recording: Threatening_Roar_Your_Opponent_Cannot_Declare_An_Attack_This_Turn_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Threatening Roar",
    effect = "Your_opponent_cannot_declare_an_attack_this_turn",
    actor = "Player",
    scenarioText = "Set **Threatening Roar**; opponent attacks with **Summoned Skull** or flips with **Delinquent Duo**. Resolve the FLIP effect against a legal target still in the pool (**Summoned Skull** or **Breaker the Magical Warrior**).",
    play = { { wait = 90 } },
    recordFrames = nil,
})
