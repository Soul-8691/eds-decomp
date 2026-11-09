-- AUTO-GENERATED from TESTING.md
-- Recording: Soul_Exchange_Chain_Window_CPU.mp4
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
    card = "Soul Exchange",
    effect = "chain_window",
    actor = "CPU",
    format = "Yugi-Kaiba",
    scenarioText = "Second pass: simplify the board until the hook conditions in **AI coverage** are met (extra hand size, weaker monsters, or an open monster zone) and re-check CPU behavior.",
    play = nil,
    recordFrames = 3600,
})
