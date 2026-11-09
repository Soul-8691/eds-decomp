--[[ Resolve tools/testing_scenarios paths when mGBA sets script.dir to "." ]]

local function dirFromFile(source)
    if not source or source == "" then
        return nil
    end
    if source:sub(1, 1) == "@" then
        source = source:sub(2)
    end
    source = source:gsub("\\", "/")
    return source:match("^(.*)/[^/]+$")
end

local function scenariosDir()
    local fromScript = script and script.dir and script.dir:gsub("\\", "/") or ""
    if fromScript ~= "" and fromScript ~= "." then
        if fromScript:match("/poc$") or fromScript:match("/generated$") then
            return fromScript .. "/.."
        end
        if fromScript:match("/testing_scenarios$") then
            return fromScript
        end
    end
    local here = dirFromFile(debug.getinfo(1, "S").source)
    if here and here:match("testing_scenarios") then
        return here
    end
    return "tools/testing_scenarios"
end

local ROOT = scenariosDir():gsub("/+$", "")

local function loadFirst(candidates, label)
    for _, path in ipairs(candidates) do
        path = path:gsub("\\", "/")
        local ok, result = pcall(dofile, path)
        if ok then
            return result
        end
    end
    error("[testing] cannot open " .. label .. " (tried " .. table.concat(candidates, ", ") .. ")")
end

local M = {}
M.scenariosDir = ROOT
M.repoRoot = ROOT .. "/../.."

function M.loadEds()
    return loadFirst({
        ROOT .. "/lib_eds.lua",
        "tools/testing_scenarios/lib_eds.lua",
    }, "lib_eds.lua")
end

function M.loadPocPlays()
    return loadFirst({
        ROOT .. "/poc_plays.lua",
        "tools/testing_scenarios/poc_plays.lua",
    }, "poc_plays.lua")
end

--[[ Called from generated scripts before paths is available ]]
function M.bootstrapFromScript()
    local tries = {}
    if script and script.dir then
        local d = script.dir:gsub("\\", "/")
        if d ~= "." then
            tries[#tries + 1] = d .. "/../eds_paths.lua"
            tries[#tries + 1] = d .. "/eds_paths.lua"
        end
    end
    tries[#tries + 1] = "tools/testing_scenarios/eds_paths.lua"
    return loadFirst(tries, "eds_paths.lua")
end

return M
