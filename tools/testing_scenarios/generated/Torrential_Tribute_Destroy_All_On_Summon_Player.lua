-- AUTO-GENERATED from TESTING.md
-- Recording: Torrential_Tribute_Destroy_All_On_Summon_Player.mp4
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
    card = "Torrential Tribute",
    effect = "destroy_all_on_summon",
    actor = "Player",
    format = "Android",
    scenarioText = "You control **Mystic Tomato**; opponent Normal Summons **Summoned Skull** and sets **Man-Eater Bug**. Activate set **Torrential Tribute** when that summon resolves (Trap in response to Summon): every monster on the field is destroyed and any FLIP effects from **Man-Eater Bug** still resolve.",
    play = { { wait = 90 } },
    recordFrames = nil,
})
