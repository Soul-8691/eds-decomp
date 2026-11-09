-- AUTO-GENERATED from TESTING.md
-- Recording: Magic_Cylinder_In_The_Attack_Step_Negate_The_Attack_And_Inflic_Player.mp4
local __edsScriptSrc = debug.getinfo(1, "S").source or ""
local function __edsTestingRoot()
    local src = __edsScriptSrc
    if src:sub(1, 1) == "@" then
        src = src:sub(2)
    end
    src = src:gsub("\\", "/")
    local dir = src:match("^(.*)/[^/]+$")
    if dir and (dir:match("/poc$") or dir:match("/generated$")) then
        return dir .. "/.."
    end
    if dir and dir:match("/testing_scenarios$") then
        return dir
    end
    return "tools/testing_scenarios"
end
local paths = dofile(__edsTestingRoot() .. "/eds_paths.lua")
local EDS = paths.loadEds()
EDS.run({
    card = "Magic Cylinder",
    effect = "in_the_attack_step_negate_the_attack_and_inflic",
    actor = "Player",
    format = "Android",
    scenarioText = "Opponent attacks your face-down **Man-Eater Bug** with **Summoned Skull** (note its ATK). Activate **Magic Cylinder** in the attack step: negate the attack and inflict that ATK as damage; **Man-Eater Bug** should not be destroyed by battle.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
