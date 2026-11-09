-- AUTO-GENERATED from TESTING.md
-- Recording: Cyber_End_Dragon_Cpu_Standard_Lines_CPU.mp4
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
    card = "Cyber End Dragon",
    effect = "cpu_standard_lines",
    actor = "CPU",
    format = "Cyber",
    scenarioText = "Campaign **Cyber** duel (or Test Duel with the format opponent deck): give the **opponent** `Cyber End Dragon` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Cyber End Dragon** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    play = nil,
    recordFrames = 3600,
})
