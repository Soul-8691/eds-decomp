-- POC — TESTING.md scenario
-- Recording: Card_Destruction_Cpu_Duel_CPU.mp4
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
local key = "Card_Destruction_Cpu_Duel_CPU"
local poc = POC[key]
local opts = {
    card = "Card Destruction",
    effect = "cpu_duel",
    actor = "CPU",
    format = "Yugi-Kaiba",
    scenarioText = "Campaign **Yugi-Kaiba** duel (or Test Duel with the format opponent deck): give the **opponent** `Card Destruction` in the main/extra deck at a realistic copy count from `usage/` decks. Let the CPU play the duel: it should activate/set/chain **Card Destruction** when its `AiWants*` / `AiShouldActivateAddedCard` policy says so (see **AI coverage**). Repeat until you see at least one successful CPU use or confirm the policy never fires on this board.",
    setupCards = poc and poc.setupCards,
    play = poc and poc.play,
    pocRequired = true,
}
opts.recordFrames = 3600
opts.play = nil
EDS.run(opts)
