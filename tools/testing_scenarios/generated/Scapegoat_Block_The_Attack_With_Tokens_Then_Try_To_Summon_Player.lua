-- AUTO-GENERATED from TESTING.md
-- Recording: Scapegoat_Block_The_Attack_With_Tokens_Then_Try_To_Summon_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Scapegoat",
    effect = "block_the_attack_with_tokens_then_try_to_summon",
    actor = "Player",
    scenarioText = "Opponent attacks your **Bazoo the Soul-Eater** with **Summoned Skull**. With four open zones and no summon yet, activate **Scapegoat**, block the attack with tokens, then try to summon **Summoned Skull** the same turn (should fail).",
    play = { { wait = 90 } },
    recordFrames = nil,
})
