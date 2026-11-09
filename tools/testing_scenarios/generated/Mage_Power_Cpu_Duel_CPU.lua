-- AUTO-GENERATED from TESTING.md
-- Recording: Mage_Power_Cpu_Duel_CPU.mp4
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
    card = "Mage Power",
    effect = "cpu_duel",
    actor = "CPU",
    format = "Android",
    scenarioText = "Campaign **Android** duel (or Test Duel with the format opponent deck): give the **opponent** `Mage Power` in the main/extra deck at a realistic copy count from `usage/` decks. Verify the CPU plays **Mage Power** when its generic spell/monster heuristics allow (same class as vanilla cards with that effect key).",
    play = nil,
    recordFrames = 3600,
})
