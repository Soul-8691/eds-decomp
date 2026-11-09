-- AUTO-GENERATED from TESTING.md
-- Recording: Exarion_Universe_Mandatory_Follow_Up_Player.mp4
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
    card = "Exarion Universe",
    effect = "mandatory_follow_up",
    actor = "Player",
    format = "Cyber",
    scenarioText = "Same duel: after **Exarion Universe**'s mandatory effect runs (see **Coded behavior**—not an optional Trigger you activate), confirm the ROM applies it automatically. Opponent cannot chain **Magic Jammer** to that mandatory step; they may still use it in a later open window.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
