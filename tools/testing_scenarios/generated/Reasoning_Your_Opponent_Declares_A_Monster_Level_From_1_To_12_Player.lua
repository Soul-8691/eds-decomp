-- AUTO-GENERATED from TESTING.md
-- Recording: Reasoning_Your_Opponent_Declares_A_Monster_Level_From_1_To_12_Player.mp4
local EDS = dofile((script and script.dir or '.') .. '/../lib_eds.lua')
EDS.run({
    card = "Reasoning",
    effect = "Your_opponent_declares_a_monster_Level_from_1_to_12",
    actor = "Player",
    scenarioText = "Open with **Reasoning** while opponent controls only **Summoned Skull**. Count hand size before/after relative to **Card Destruction** in the opponent list; repeat when your Standby/Draw Phase trigger applies.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
