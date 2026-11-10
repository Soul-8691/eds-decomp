--[[
lib_media.lua: the README's screenshots, GIFs and videos (README_TODO.md), captured from mGBA by script.

Each script in tools/media/ is one capture (or a small set of them):

    local M = dofile((script.dir or "tools/media") .. "/lib_media.lua")
    M.run("settings", function()
        M.toMainMenu()
        M.menuTo(M.MENU.SETTINGS)
        M.shot("Settings (Main Menu)")
        M.press(M.KEY.A)
        M.waitCallback("CB_Settings")
        M.shot("Settings")
    end)

M.run's function runs as a coroutine, one resume per frame: M.wait, M.press and the M.wait* helpers yield, so the
script reads as a sequence. Screen and RAM addresses come from the ROM's symbol file (eds_demo.sym next to the ROM,
from make demo), so a rebuild that moves code does not break the scripts.

Output (paths from the repo root, where run_tests.ps1 starts mGBA):
  M.shot(name)            screenshots/<name>.png, 240x160 like the README's other screenshots
  M.record(name, kind)    kind "gif": screenshots/gifs/<name>.gif, 2x nearest-neighbour, 30 fps (README_TODO.md)
  ... M.stop()            kind "mp4": screenshots/videos/<name>.mp4, 3x, 60 fps. No audio: the Lua API has no
                          sound capture (record the video with sound through mGBA's Tools > Record A/V instead)
The result goes to tools/testing_scenarios/recordings/_last_run.log as "PASS <script>" / "FAIL <script> <reason>",
which run_tests.ps1 -Media reads.

Duel captures (a set board, then steps) use the test harness instead: tools/testing_scenarios/lib_eds.lua EDS.run
with setupCards and steps, plus the `shot` step (lib_steps.lua). See tools/media/README.md.
]]

local M = {}

local here = (script and script.dir and script.dir ~= "." and script.dir:gsub("\\", "/")) or "tools/media"
local ROOT = here .. "/../.."
M.ROOT = ROOT

-- the test harness: its log, run log and ffmpeg lookup (tools/testing_scenarios/lib_eds.lua)
local paths = dofile(ROOT .. "/tools/testing_scenarios/eds_paths.lua")
local EDS = paths.loadEds()
M.EDS = EDS

M.KEY = { A = 0x01, B = 0x02, SELECT = 0x04, START = 0x08, RIGHT = 0x10, LEFT = 0x20, UP = 0x40, DOWN = 0x80,
          R = 0x100, L = 0x200 }
M.MENU = { CAMPAIGN = 0, LINK_BATTLE = 1, DECK_EDIT = 2, RECORD = 3, CALENDAR = 4, CARD_TRADING = 5, PASSWORD = 6,
           DECK_RECIPES = 7, BOOSTER_PACKS = 8, BANLISTS = 9, SETTINGS = 10, COUNT = 11 }

local G_MAIN = 0x03000040
local MAIN_CALLBACK = G_MAIN + 0x0410

M.PRESS_FRAMES = 3
M.GAP_FRAMES = 12

-- ---- symbols ----

local SYM, SYMSIZE = {}, {}
local function loadSymbols()
    -- the ROM's own symbol file: run_tests.ps1 -Media writes the ROM's name to tools/media/_rom.txt
    local candidates = {}
    local rf = io.open(here .. "/_rom.txt", "r")
    if rf then
        local rom = (rf:read("*l") or ""):gsub("%s+$", "")
        rf:close()
        if rom ~= "" then
            candidates[#candidates + 1] = ROOT .. "/" .. rom .. ".sym"
        end
    end
    for _, c in ipairs({ ROOT .. "/eds_demo.sym", ROOT .. "/eds.sym", "eds_demo.sym" }) do
        candidates[#candidates + 1] = c
    end
    for _, p in ipairs(candidates) do
        local f = io.open(p, "r")
        if f then
            for line in f:lines() do
                local addr, size, name = line:match("^(%x+) %a+ (%x+) (%S+)$")
                if addr then
                    SYM[name] = tonumber(addr, 16)
                    SYMSIZE[name] = tonumber(size, 16)
                end
            end
            f:close()
            return p
        end
    end
    return nil
end
M.symFile = loadSymbols()

function M.sym(name)
    local a = SYM[name]
    if not a then
        error("symbol not found: " .. name .. " (" .. tostring(M.symFile) .. ")")
    end
    return a
end

function M.symSize(name)
    M.sym(name)
    return SYMSIZE[name]
end

-- The card ID whose name (gCardNames, 0x40 bytes per ID) is `name`; the first one (an alternate artwork has a
-- higher ID). An error when no card has it.
local CARD_NAME_SIZE = 0x40
local nameCache = {}
function M.cardId(name)
    if nameCache[name] then
        return nameCache[name]
    end
    local base = M.sym("gCardNames")
    local count = M.symSize("gCardNames") // CARD_NAME_SIZE
    for id = 1, count - 1 do
        local raw = emu:readRange(base + id * CARD_NAME_SIZE, CARD_NAME_SIZE)
        local n = raw:match("^[^%z]*")
        if n == name then
            nameCache[name] = id
            return id
        end
    end
    error("no card named " .. name)
end

-- ---- logging and results ----

local scriptName = "media"

function M.log(msg)
    EDS.log("[" .. scriptName .. "] " .. msg)
end

-- ---- the frame loop ----

local co
local holdKey, holdLeft, gapLeft = 0, 0, 0
local rec                                   -- the recording in progress: { dir, n, name, kind, every }
local finished = false

local function tickKeys()
    if holdLeft > 0 then
        holdLeft = holdLeft - 1
        if holdLeft == 0 then
            emu:clearKeys(holdKey)
        end
    elseif gapLeft > 0 then
        gapLeft = gapLeft - 1
    end
end

local function captureFrame()
    if rec then
        emu:screenshot(string.format("%s/%06d.png", rec.dir, rec.n))
        rec.n = rec.n + 1
    end
end

function M.wait(frames)
    for _ = 1, (frames or 1) do
        coroutine.yield()
    end
end

-- Holds `key` (a mask, M.KEY.*) for `hold` frames, then waits `gap` frames.
function M.press(key, hold, gap)
    holdKey = key
    holdLeft = hold or M.PRESS_FRAMES
    gapLeft = gap or M.GAP_FRAMES
    emu:addKeys(key)
    while holdLeft > 0 or gapLeft > 0 do
        coroutine.yield()
    end
end

-- Holds `key` (the game's key repeat moves on) until fn() is true, at most `timeout` frames; then lets go.
function M.holdUntil(key, fn, timeout, label)
    local t = 0
    emu:addKeys(key)
    while not fn() do
        t = t + 1
        if t > (timeout or 3600) then
            emu:clearKeys(key)
            error("timeout holding a key for " .. (label or "a condition"))
        end
        coroutine.yield()
    end
    emu:clearKeys(key)
    M.wait(10)
end

-- Presses `key` `n` times.
function M.pressN(key, n, gap)
    for _ = 1, n do
        M.press(key, nil, gap)
    end
end

-- Waits until fn() is true (checked every frame), at most `timeout` frames; an error ends the script as FAIL.
function M.waitUntil(fn, timeout, label)
    local t = 0
    while not fn() do
        t = t + 1
        if t > (timeout or 1800) then
            error("timeout waiting for " .. (label or "a condition"))
        end
        coroutine.yield()
    end
end

function M.callback()
    return emu:read32(MAIN_CALLBACK)
end

-- Waits until the main callback is the symbol `name` (a scene's CB_*), then `settle` frames for its fade-in.
function M.waitCallback(name, settle, timeout)
    local target = M.sym(name) | 1
    M.waitUntil(function() return (M.callback() | 1) == target end, timeout, name)
    M.wait(settle or 60)
end

function M.inCallback(name)
    return (M.callback() | 1) == (M.sym(name) | 1)
end

-- ---- the title screen and the main menu ----

-- From power-on: waits on the title screen (CB_Title) for `settle` frames, so the flame has drawn.
function M.toTitle(settle)
    M.waitUntil(function()
        if M.inCallback("CB_Title") then
            return true
        end
        return false
    end, 3600, "the title screen")
    M.wait(settle or 120)
end

-- From power-on (or the title screen): A (and START) until the main menu, then `settle` frames.
function M.toMainMenu(settle)
    local t = 0
    while not M.inCallback("CB_MainMenu") do
        t = t + 1
        if t % 30 == 1 then
            M.press(M.KEY.A, 3, 0)
        elseif t == 90 then
            M.press(M.KEY.START, 3, 0)
        else
            coroutine.yield()
        end
        if t > 3600 then
            error("timeout waiting for the main menu")
        end
    end
    M.wait(settle or 45)
end

-- Moves the main menu cursor onto `item` (M.MENU.*) with Up/Down; the menu wraps, so the shorter way.
function M.menuTo(item, settle)
    local cur = M.sym("gMainMenuCursor")
    for _ = 1, M.MENU.COUNT do
        local c = emu:read16(cur)
        if c == item then
            break
        end
        local down = (item - c) % M.MENU.COUNT
        M.press(down <= M.MENU.COUNT // 2 and M.KEY.DOWN or M.KEY.UP, nil, 14)
    end
    M.wait(settle or 30)
end

-- From the main menu: the item's scene (menuTo, A), waiting for its callback when `cb` is given.
function M.openMenu(item, cb, settle)
    M.menuTo(item)
    M.press(M.KEY.A)
    if cb then
        M.waitCallback(cb, settle)
    else
        M.wait(settle or 90)
    end
end

-- ---- captures ----

local function winPath(p)
    return (p:gsub("/", "\\"))
end

local function mkdir(p)
    os.execute('mkdir "' .. winPath(p) .. '" 2>nul')
end

-- A screenshot (240x160) to screenshots/<name>.png.
function M.shot(name)
    mkdir(ROOT .. "/screenshots")
    local path = ROOT .. "/screenshots/" .. name .. ".png"
    emu:screenshot(path)
    M.log("shot " .. name .. ".png")
    coroutine.yield()
end

-- Starts recording every frame (PNG) for M.stop to encode: kind "gif" or "mp4".
function M.record(name, kind)
    local dir = ROOT .. "/tools/media/_frames/" .. name
    os.execute('if exist "' .. winPath(dir) .. '" rmdir /s /q "' .. winPath(dir) .. '" 2>nul')
    mkdir(dir)
    rec = { dir = dir, n = 0, name = name, kind = kind or "gif" }
    M.log("recording " .. name .. " (" .. rec.kind .. ")")
end

-- Ends the recording and encodes it (ffmpeg); the PNG frames are deleted afterwards.
function M.stop()
    local r = rec
    rec = nil
    if not r or r.n < 2 then
        error("nothing recorded")
    end
    local ffmpeg = EDS.ffmpegCommand() or "ffmpeg"
    local input = winPath(r.dir) .. "\\%06d.png"
    local out, args
    if r.kind == "mp4" then
        mkdir(ROOT .. "/screenshots/videos")
        out = winPath(ROOT .. "/screenshots/videos/" .. r.name .. ".mp4")
        args = string.format('-framerate 60 -i "%s" -vf scale=720:480:flags=neighbor -pix_fmt yuv420p -c:v libx264 "%s"',
            input, out)
    else
        mkdir(ROOT .. "/screenshots/gifs")
        out = winPath(ROOT .. "/screenshots/gifs/" .. r.name .. ".gif")
        args = string.format('-framerate 60 -i "%s" -vf "fps=30,scale=480:320:flags=neighbor,split[a][b];'
            .. '[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=none" "%s"', input, out)
    end
    local cmd = string.format('%s -y -hide_banner -loglevel error %s', ffmpeg, args)
    M.log(string.format("encoding %d frames -> %s", r.n, out))
    os.execute('"' .. cmd .. '"')
    os.execute('rmdir /s /q "' .. winPath(r.dir) .. '" 2>nul')
    local f = io.open(out, "rb")
    if not f then
        error("ffmpeg wrote nothing: " .. out)
    end
    f:close()
end

-- ---- RAM helpers ----

function M.read8(a) return emu:read8(a) end
function M.read16(a) return emu:read16(a) end
function M.read32(a) return emu:read32(a) end
function M.write8(a, v) emu:write8(a, v) end
function M.write16(a, v) emu:write16(a, v) end
function M.write32(a, v) emu:write32(a, v) end

-- ---- the runner ----

-- Runs `fn` as the capture script `name`; logs PASS / FAIL to the run log run_tests.ps1 reads.
function M.run(name, fn)
    scriptName = name
    EDS.appendRunLog("START " .. name)
    M.log("start (symbols: " .. tostring(M.symFile) .. ")")
    co = coroutine.create(fn)
    local id
    id = callbacks:add("frame", function()
        tickKeys()
        captureFrame()
        if finished then
            return
        end
        local ok, err = coroutine.resume(co)
        if not ok then
            finished = true
            M.log("FAIL: " .. tostring(err))
            EDS.appendRunLog("FAIL " .. name .. " " .. tostring(err):gsub("%s+", " "))
            callbacks:remove(id)
        elseif coroutine.status(co) == "dead" then
            finished = true
            M.log("done")
            EDS.appendRunLog("PASS " .. name)
            callbacks:remove(id)
        end
    end)
end

return M
