-- POC — TESTING.md scenario
-- Recording: Card_Destruction_Discard_And_Draw_Player.mp4
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
local POC = paths.loadPocPlays()
local key = "Card_Destruction_Discard_And_Draw_Player"
local poc = POC[key]
local opts = {
    card = "Card Destruction",
    effect = "discard_and_draw",
    actor = "Player",
    format = "Yugi-Kaiba",
    scenarioText = "Your hand: **Card Destruction**, **Change of Heart**, and another pool card; opponent has four cards including **Summoned Skull**. Activate **Card Destruction**: both hands discard, both draw the same count.",
    setupCards = poc and poc.setupCards,
    play = poc and poc.play,
    pocRequired = true,
}
EDS.run(opts)
