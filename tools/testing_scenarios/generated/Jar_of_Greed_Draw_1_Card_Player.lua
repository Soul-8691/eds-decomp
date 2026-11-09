-- AUTO-GENERATED from TESTING.md
-- Recording: Jar_of_Greed_Draw_1_Card_Player.mp4
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
    card = "Jar of Greed",
    effect = "Draw_1_card",
    actor = "Player",
    format = "Android",
    scenarioText = "Open with **Jar of Greed** while opponent controls only **Summoned Skull**. Count hand size before/after relative to **Change of Heart** in the opponent list; repeat when your Standby/Draw Phase trigger applies.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
