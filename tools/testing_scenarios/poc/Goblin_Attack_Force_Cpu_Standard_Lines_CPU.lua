-- POC — TESTING.md scenario
-- Recording: Goblin_Attack_Force_Cpu_Standard_Lines_CPU.mp4
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
local key = "Goblin_Attack_Force_Cpu_Standard_Lines_CPU"
local poc = POC[key]
local opts = {
    card = "Goblin Attack Force",
    effect = "cpu_standard_lines",
    actor = "CPU",
    format = "Imperial",
    scenarioText = "Campaign **Imperial** duel (or Test Duel with the format opponent deck): give the **opponent** `Goblin Attack Force` in the main/extra deck at a realistic copy count from `usage/` decks. CPU should Normal Summon/set and attack with **Goblin Attack Force** using ordinary lines; verify mandatory / passive text from **Coded behavior** without expecting a card-menu activation.",
    setupCards = poc and poc.setupCards,
    play = poc and poc.play,
    pocRequired = true,
}
opts.recordFrames = 3600
opts.play = nil
EDS.run(opts)
