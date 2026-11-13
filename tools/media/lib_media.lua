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
    if rec and not rec.paused then
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

-- The symbol name of the main callback now (logs), or its address.
function M.callbackName()
    local cb = M.callback() & ~1
    for name, a in pairs(SYM) do
        if (a & ~1) == cb and name:match("^CB_") then
            return name
        end
    end
    return string.format("0x%08X", cb)
end

-- ---- the Campaign's opponent select (gOpponentSelect, include/campaign.h) ----

-- Slots: 0 bottom, 1 left, 2 top left, 3 top right, 4 right (gOpponentSelectSlotPos: the portraits' top-left).
M.SLOT = { BOTTOM = 0, LEFT = 1, TOP_LEFT = 2, TOP_RIGHT = 3, RIGHT = 4 }
local SLOT_POS = { [0] = { 89, 91 }, { 36, 58 }, { 1, 5 }, { 177, 5 }, { 142, 58 } }

function M.selectPage()
    return M.read8(M.sym("gOpponentSelect")) & 7
end

function M.selectSlot()
    return (M.read16(M.sym("gOpponentSelect")) >> 3) & 7
end

-- On the select screen: L/R to `page` (about 90 frames a turn), then the cursor onto `slot` with the D-pad, steered by
-- the portraits' positions (the longer axis first; the other when a press does not move it).
function M.campaignPick(page, slot)
    for _ = 1, 8 do
        if M.selectPage() == page then
            break
        end
        M.press(M.selectPage() < page and M.KEY.R or M.KEY.L, nil, 90)
    end
    if M.selectPage() ~= page then
        error("duelist page " .. page .. " not reached")
    end
    M.wait(30)
    local flip = false
    for _ = 1, 12 do
        local cur = M.selectSlot()
        if cur == slot then
            M.wait(30)
            return
        end
        local dx = SLOT_POS[slot][1] - SLOT_POS[cur][1]
        local dy = SLOT_POS[slot][2] - SLOT_POS[cur][2]
        local horizontal = math.abs(dx) >= math.abs(dy)
        if flip then
            horizontal = not horizontal
        end
        local key
        if horizontal then
            key = dx > 0 and M.KEY.RIGHT or M.KEY.LEFT
        else
            key = dy > 0 and M.KEY.DOWN or M.KEY.UP
        end
        M.press(key, nil, 24)
        flip = M.selectSlot() == cur and not flip
    end
    error("duelist slot " .. slot .. " not reached (at " .. M.selectSlot() .. ")")
end

-- ---- a duel's opening: rock-paper-scissors, the first Draw Phase (tools/testing_scenarios/lib_eds.lua) ----

local function turnOrder(off)
    return M.read8(EDS.G_TURN_ORDER_SCENE + off)
end

-- True while the rock-paper-scissors screen waits for your card.
function M.rpsChoosing()
    return EDS.turnOrderRpsUiActive() and turnOrder(EDS.OFF_TURN_ORDER_PHASE) == EDS.TURN_ORDER_CHOOSE_HAND
        and EDS.duelStep() < EDS.DUEL_STEP_DRAW
end

-- Rock-paper-scissors, won: A on a card, then the CPU's card (once it has answered) is the one yours beats (the
-- harness's forced loss, EDS.forceTurnOrderRpsLose, the other way round); then `first` (go first) or second, A.
function M.rpsWin(first)
    M.waitUntil(M.rpsChoosing, 1800, "rock-paper-scissors")
    M.wait(60)
    M.press(M.KEY.A)
    local to = EDS.G_TURN_ORDER_SCENE
    M.waitUntil(function() return turnOrder(EDS.OFF_TURN_ORDER_OPP_ANSWERED) == 1 end, 600, "the CPU's card")
    local stop = turnOrder(EDS.OFF_TURN_ORDER_CAROUSEL_STOP)
    M.write8(to + EDS.OFF_TURN_ORDER_OPP_HAND, (stop + 1) % 3)
    M.write8(to + EDS.OFF_TURN_ORDER_RESULT, EDS.RPS_WIN)
    M.wait(100)                     -- "WIN !!" shown; A goes on to the first / second choice
    for _ = 1, 10 do
        if turnOrder(EDS.OFF_TURN_ORDER_PHASE) == EDS.TURN_ORDER_CHOOSE_TURN then
            break
        end
        M.press(M.KEY.A, nil, 40)
    end
    M.waitUntil(function() return turnOrder(EDS.OFF_TURN_ORDER_PHASE) == EDS.TURN_ORDER_CHOOSE_TURN end, 300,
        "the first / second choice")
    M.wait(45)
    M.write8(to + EDS.OFF_TURN_ORDER_TURN_CHOICE, first and 0 or EDS.TURN_CHOICE_SECOND)
    M.press(M.KEY.A)
end

-- Your first Draw Phase (the testing ROM, or against Debug: src/mod_mulligan.c): the cursor onto your Deck, then
-- `n` Mulligans from its menu (each redraws the hand), then Draw. `record` names a GIF of it.
function M.mulligan(n, record)
    M.waitUntil(function()
        return EDS.duelStep() == EDS.DUEL_STEP_DRAW and EDS.turnPlayer() == 0 and not EDS.busy()
    end, 3600, "your first Draw Phase")
    for _ = 1, 20 do
        if EDS.duelSelArea() == EDS.DUEL_AREA_DECK and EDS.duelSelPlayer() == 0 then
            break
        end
        M.press(EDS.keyTowardOwnDeck(EDS.duelSelArea()), nil, 10)
    end
    M.wait(30)
    if record then
        M.record(record, "gif")
    end
    local function choose(cmd)
        for _ = 1, 3 do
            if EDS.cardMenuOpen() then
                break
            end
            M.press(M.KEY.A, nil, 30)
        end
        for _ = 1, 6 do
            if EDS.cardMenuCommand() == cmd then
                break
            end
            M.press(cmd == EDS.CARDMENU_CMD_MULLIGAN and M.KEY.RIGHT or M.KEY.LEFT, nil, 20)
        end
        M.wait(20)
        M.press(M.KEY.A, nil, 20)
    end
    for _ = 1, n do
        choose(EDS.CARDMENU_CMD_MULLIGAN)
        M.wait(150)                 -- the hand goes back, the deck is shuffled, five are drawn
    end
    choose(EDS.CARDMENU_CMD_DRAW)
    M.wait(90)
    if record then
        M.stop()
    end
end

-- The cursor onto the opponent's hand (the screen scrolls to their side; the info bar names the card).
function M.pointOpponentHand()
    for _ = 1, 12 do
        if EDS.duelSelPlayer() == 1 and EDS.duelSelArea() == EDS.DUEL_AREA_HAND then
            break
        end
        M.press(M.KEY.UP, nil, 14)
    end
    M.wait(60)
end

-- From the opening to your Main Phase 1: in your first Draw Phase the deck menu's Draw (not Mulligan); A through the
-- rest (messages, the CPU's turn when it goes first).
function M.toMainPhase(timeout)
    local t = 0
    while not EDS.inHumanMainPhase1() do
        t = t + 1
        if t > (timeout or 3600) then
            error("timeout waiting for Main Phase 1")
        end
        if EDS.busy() then
            coroutine.yield()
        elseif EDS.duelStep() == EDS.DUEL_STEP_DRAW and EDS.turnPlayer() == 0 then
            if EDS.cardMenuOpen() then
                M.press(EDS.cardMenuCommand() == EDS.CARDMENU_CMD_DRAW and M.KEY.A or M.KEY.LEFT, nil, 16)
            else
                M.press(EDS.keyTowardOwnDeck(EDS.duelSelArea()), nil, 10)
            end
        elseif t % 40 == 0 then
            M.press(M.KEY.A, nil, 10)
        else
            coroutine.yield()
        end
    end
    M.wait(60)
end

-- ---- shared screens (moved from main_menu_screens.lua) ----

-- A line of a build header (build/include/...), or nil.
function M.buildFile(name)
    local f = io.open(M.ROOT .. "/build/include/" .. name, "r")
    if not f then
        return nil
    end
    local t = f:read("*a")
    f:close()
    return t
end

-- gDeckRecipes (src/mod_recipes.c struct DeckRecipes): cursor +0 (the slot), scroll +1 (the top row's slot),
-- active +2 (the recipe last loaded), mode +3 (0 list, 1 the actions, 3 the cards
-- a load left out), action +4; the slots' summaries from +0x2C, 36 bytes each (sizes, then the name at +4).
local RECIPE_COUNT, RECIPE_SUMMARY = 60, 36

-- Deck Recipes: the recipe whose name holds `pattern` loaded (cursor onto it with Down, A, Load, A), then back to
-- the main menu.
function M.loadRecipe(pattern)
    local dr = M.sym("gDeckRecipes")
    local slot
    for n = 0, RECIPE_COUNT - 1 do
        local name = emu:readRange(dr + 0x2C + n * RECIPE_SUMMARY + 4, 32):match("^[^%z]*")
        if name:find(pattern, 1, true) then
            slot = n
            break
        end
    end
    if not slot then
        error("no recipe named like " .. pattern)
    end
    local function at() return M.read8(dr) end
    for _ = 1, RECIPE_COUNT * 3 do       -- (a press can be missed: checked after each one)
        if at() == slot then
            break
        end
        M.press(at() < slot and M.KEY.DOWN or M.KEY.UP, nil, 16)
    end
    if at() ~= slot then
        error("Deck Recipes cursor did not reach slot " .. slot)
    end
    M.press(M.KEY.A, nil, 20)                          -- the actions
    for _ = 1, 5 do
        if M.read8(dr + 4) == 0 then                   -- ACTION_LOAD
            break
        end
        M.press(M.KEY.LEFT, nil, 10)
    end
    for _ = 1, 3 do                                    -- (a load over a deck asks twice)
        if M.read8(dr + 3) ~= 1 then
            break
        end
        M.press(M.KEY.A, nil, 40)
    end
    M.wait(60)
    if M.read8(dr + 2) ~= slot then
        error("recipe " .. slot .. " was not loaded (active " .. M.read8(dr + 2) .. ")")
    end
    M.log("loaded recipe " .. slot .. " (" .. pattern .. ")")
end

-- Deck Edit: the Main Deck list (R selects the next list: deck_edit_list.c), its cursor scrolled onto
-- `name` with Down, so the rows on the way are drawn as in play.
function M.deckEditMainDeckOn(name)
    local de = M.sym("gDeckEdit")
    local lists = M.sym("gDeckEditCardLists")
    local added = tonumber((M.buildFile("card_count.h") or ""):match("CARD_ID_COUNT_ADDED (%d+)") or "0")
    local trunkSize = 821 + added
    local rowSize = (trunkSize + 80 + 15) * 2
    for _ = 1, 3 do
        if M.read8(de + 0x1C1C) == 1 then              -- curList: the Main Deck
            break
        end
        M.press(M.KEY.R, nil, 30)
    end
    if M.read8(de + 0x1C1C) ~= 1 then
        error("Deck Edit: could not reach the Main Deck list")
    end
    local id = M.cardId(name)
    local base = lists + M.read8(de + 0x14A0 + 1) * rowSize + trunkSize * 2
    local target
    for i = 0, 79 do
        if (M.read16(base + i * 2) & 0x7FF) == id then
            target = i
            break
        end
    end
    if not target then
        error(name .. " is not in the Main Deck")
    end
    local function pos() return M.read16(de + 0x0620 + 2) end   -- listPos[1]
    for _ = 1, 90 do
        if pos() == target then
            break
        end
        M.press(pos() < target and M.KEY.DOWN or M.KEY.UP, nil, 10)
    end
    if pos() ~= target then
        error("Deck Edit cursor did not reach " .. name)
    end
end

-- B until the main menu again
function M.back()
    -- B until the main menu again
    for _ = 1, 6 do
        if M.inCallback("CB_MainMenu") then
            break
        end
        M.press(M.KEY.B, nil, 40)
    end
    M.waitCallback("CB_MainMenu", 45)
end


-- The pack ID of the format pack named `name` (FORMAT_PACK_ID_FIRST 601 + its sFormatPackNames index).
function M.formatPackId(name)
    local t = M.buildFile("format_pack_data.h") or ""
    local block = t:match("sFormatPackNames%b[]%s*=%s*(%b{})") or ""
    local i = 0
    for n in block:gmatch('"(.-)",') do
        if n == name then
            return 601 + i
        end
        i = i + 1
    end
    error("no format pack " .. name)
end

-- Booster Packs with `packId` in the centre: CB_PackShop's own restore (lastPackId, restore: it puts the last
-- opened pack back in the centre) is asked for while the list sets up.
function M.packShopCentred(packId)
    local shop = M.sym("gPackShop")
    for _ = 1, 150 do
        M.write16(shop + 4, packId)         -- lastPackId
        M.write8(shop + 2, 1)               -- restore
        coroutine.yield()
    end
end

-- Deck Edit's command bar (gDeckEdit.commandMenu +0x1C3C, include/deck_edit.h): A opens it; Left/Right onto command
-- `cmd` (0 Card View, 1 To Trunk, 2 To Main, 3 To Side, 4 List Filter, 5 Statistics, 6 Empty Main/Side); A.
M.DECK_CMD = { CARD_VIEW = 0, TO_TRUNK = 1, TO_MAIN = 2, TO_SIDE = 3, LIST_FILTER = 4, STATISTICS = 5, EMPTY = 6 }

function M.deckEditCommand(cmd)
    local de = M.sym("gDeckEdit")
    local function choice() return (M.read32(de + 0x1C3C) >> 15) & 7 end
    local function open() return (M.read8(de + 0x1C48) & 1) ~= 0 end
    M.press(M.KEY.A, nil, 20)
    M.waitUntil(open, 120, "the command bar")
    M.wait(10)
    for _ = 1, 10 do
        if choice() == cmd then
            break
        end
        M.press(choice() < cmd and M.KEY.RIGHT or M.KEY.LEFT, nil, 14)
    end
    if choice() ~= cmd then
        error("command bar: choice " .. cmd .. " not reached (at " .. choice() .. ")")
    end
    M.wait(20)
    M.press(M.KEY.A, nil, 20)
end

-- Every named card at least once in the trunk (gSaveData.trunk[id].count, the low 10 bits; trunkSize +0x20C6): a
-- fresh save holds a few cards only. The captures run on a copy of the save.
function M.giveAllCards()
    local save = M.sym("gSaveData")
    local names = M.sym("gCardNames")
    local ids = math.min(M.symSize("gCardNames") // 0x40, 0x800)
    local total = 0
    for id = 1, ids - 1 do
        local a = save + 8 + id * 4
        local w = M.read16(a)
        if (w & 0x3FF) == 0 and M.read8(names + id * 0x40) ~= 0 then
            w = (w & ~0x3FF) | 1
            M.write16(a, w)
        end
        total = total + (w & 0x3FF)
    end
    M.write16(save + 0x20C6, total)
end

-- The List Filter's third page (src/mod_list_filter.c struct ListRefine at gDeckEdit +0x644: the row, then a value
-- per row): Up/Down onto `row`, then Left/Right (one step; with `count`, the row's values, the shorter way round as
-- one step wraps) or L/R (5, `big`) until its value is `value`.
M.REFINE = { NAME = 0, TYPE = 1, ATTRIBUTE = 2, ATK_FROM = 3, ATK_TO = 4, DEF_FROM = 5, DEF_TO = 6, LEVEL_FROM = 7,
             LEVEL_TO = 8 }

function M.refineSet(row, value, big, count)
    local base = M.sym("gDeckEdit") + 0x644
    for _ = 1, 12 do
        local r = M.read8(base)
        if r == row then
            break
        end
        M.press(r < row and M.KEY.DOWN or M.KEY.UP, nil, 24)
    end
    for _ = 1, 40 do
        local v = M.read8(base + 1 + row)
        if v == value then
            return
        end
        local up = value > v
        if count and not big then
            up = (value - v) % count <= count // 2     -- (one step wraps: the shorter way round)
        end
        if big and math.abs(value - v) >= 5 then
            M.press(value > v and M.KEY.R or M.KEY.L, nil, 24)
        else
            M.press(up and M.KEY.RIGHT or M.KEY.LEFT, nil, 24)
        end
    end
    error("List Filter: row " .. row .. " did not reach " .. value)
end

-- Banlists (src/mod_banlists.c): from the main menu, the cursor (gBanlistsMenu +0) onto banlist `index` (the order of
-- build/include/banlists.h: 0 Yugi-Kaiba .. 12 Goat .. 14 Reaper), then A puts it in use (in make release a switch
-- the collection cannot follow is refused). Stays on the screen.
M.BANLIST = { YUGI_KAIBA = 0, GOAT = 12 }

function M.banlistSelect(index)
    M.openMenu(M.MENU.BANLISTS, "CB_Banlists", 90)
    local menu = M.sym("gBanlistsMenu")
    for _ = 1, 40 do                -- (presses closer than about 20 frames are dropped)
        if M.read8(menu) == index then
            break
        end
        M.press(M.read8(menu) < index and M.KEY.DOWN or M.KEY.UP, nil, 30)
    end
    if M.read8(menu) ~= index then
        error("Banlists: cursor did not reach " .. index)
    end
    M.wait(20)
    M.press(M.KEY.A, nil, 60)
end

-- Card `name` (all its IDs) out of the trunk: count 0 (gSaveData.trunk[id], the low 10 bits; trunkSize follows).
function M.takeFromTrunk(name)
    local save = M.sym("gSaveData")
    local base = M.sym("gCardNames")
    local ids = math.min(M.symSize("gCardNames") // 0x40, 0x800)
    for id = 1, ids - 1 do
        if emu:readRange(base + id * 0x40, 0x40):match("^[^%z]*") == name then
            local a = save + 8 + id * 4
            local w = M.read16(a)
            M.write16(save + 0x20C6, M.read16(save + 0x20C6) - (w & 0x3FF))
            M.write16(a, w & ~0x3FF)
        end
    end
end

-- `copies` (1-3) of card `name` into the saved Deck (gSaveData.trunk[id].deckCopies, bits 10-11 of the first u16):
-- a deck for the release ROM's format check, whose recipes start empty.
function M.addToDeck(name, copies)
    local save = M.sym("gSaveData")
    local id = M.cardId(name)
    local a = save + 8 + id * 4
    M.write16(a, (M.read16(a) & ~0x0C00) | ((copies & 3) << 10))
end

-- A small collection: the trunk emptied (counts 0), then `fewer` cards taken off the saved Deck (one copy each of the
-- first cards found in it). With the 40-card starter deck, `fewer` 3 leaves 37 cards owned.
function M.shrinkCollection(fewer)
    local save = M.sym("gSaveData")
    local ids = math.min(M.symSize("gCardNames") // 0x40, 0x800)
    for id = 1, ids - 1 do
        local a = save + 8 + id * 4
        local w = M.read16(a)
        if fewer > 0 and ((w >> 10) & 3) > 0 then
            w = w - 0x0400          -- one copy fewer in the Deck
            fewer = fewer - 1
        end
        M.write16(a, w & ~0x3FF)
    end
    M.write16(save + 0x20C6, 0)
end

-- Win the duel at once: the CPU's LP to 0 (gDuel.players[1].lifePoints, EDS.PLAYER1 +0); Duel_CheckWin (duel_main.c)
-- ends the duel at its next check, as a real win would.
function M.winDuel()
    M.write16(EDS.PLAYER1, 0)
    M.waitUntil(function() return not EDS.inDuelMainPhase() end, 600, "the duel's end")
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

-- A recording's frames held back (M.pause) and taken again (M.resume): one GIF of two moments (unlock.lua).
function M.pause()
    if rec then
        rec.paused = true
    end
end

function M.resume()
    if rec then
        rec.paused = false
    end
end

-- Ends the recording and encodes it (ffmpeg); the PNG frames are deleted afterwards.
function M.stop()
    local r = rec
    rec = nil
    if not r or r.n < 2 then
        error("nothing recorded")
    end
    -- no still tail: the trailing frames identical to the last one go, but 20 (as lib_steps.lua's recordings)
    local function frame(i) return string.format("%s/%06d.png", r.dir, i) end
    local function readAll(p)
        local f = io.open(p, "rb")
        if not f then
            return nil
        end
        local d = f:read("*a")
        f:close()
        return d
    end
    local last = readAll(frame(r.n - 1))
    local first = r.n - 1
    while first > 0 and readAll(frame(first - 1)) == last do
        first = first - 1
    end
    local keep = math.min(r.n - 1, first + 20)
    for i = keep + 1, r.n - 1 do
        os.remove(frame(i))
    end
    r.n = keep + 1
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
