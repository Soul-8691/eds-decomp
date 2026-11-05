--[[
mGBA script: a Card View screenshot of every coded new card, saved as cards/<card name>.png.

  "C:\Program Files\mGBA\mGBA.exe" -C audioSync=0 -C videoSync=0 -C fpsTarget=6000 ^
      --script tools/card_screenshots.lua eds_demo.gba
  (the -C overrides switch off frame pacing so the run is not held to 60 fps; the Lua API has no speed control)

Use a ROM built from the repo-root new_cards.txt (make demo, make release-demo, or a Demo patch) with a save that
can open Deck Edit: the cards are the lines above "------", and they get card IDs 821, 822, ... in that order
(tools/newcards.py). Each ID's name in the ROM (gCardNames) is checked against the list first, so another card
pool stops the script. After reordering new_cards.txt, rebuild the ROM before capturing.

Optional cards/_capture_only.txt (one display name per line, before "|"): capture only those entries, then run
tools/gen_new_cards_md.py to refresh NEW_CARDS.md.

The script plays the game with key presses: A until the main menu, then Down, Down, A into Deck Edit. For each
card it writes the card's ID into the list entry under the cursor (DeckEdit_GetListCard's cell), opens the
command bar with A and picks Card View, waits for Card Detail to finish its fade-in, takes a screenshot (240x160),
closes it with B and puts the list entry back. Progress goes to the scripting console and to cards/_log.txt.
]]

local G_MAIN = 0x03000040
local MAIN_CALLBACK = G_MAIN + 0x0410
local MAIN_SEQ_INDEX1 = G_MAIN + 0x4859
-- CB_MainMenu moves between builds too: its address comes from the demo builds' symbol files (read below).
local CB_MAIN_MENU_FALLBACK = 0x08003B14 + 1    -- Thumb; eds_demo.sym of 2026-11-05
local CARD_DETAIL = 0x02013D90                  -- gCardDetail; bits 1-15 of the first u16 are the state
local DECK_EDIT = 0x0201DB20                    -- gDeckEdit
local DE_LIST_POS = DECK_EDIT + 0x0620          -- u16 listPos[3]
local DE_LIST_ROW = DECK_EDIT + 0x14A0          -- u8 listRow[3]
local DE_CUR_LIST = DECK_EDIT + 0x1C1C          -- u8 curList
local DE_COMMAND_MENU = DECK_EDIT + 0x1C3C      -- word bits 15-17: command bar choice
local DECK_EDIT_CARD_LISTS = 0x0202A000         -- gDeckEditCardLists[2]: u16 trunk[CARD_ID_COUNT], deck[80], side[15]
local CARD_NAMES = 0x0822C720                   -- gCardNames, 0x40 bytes per card ID
local CARD_NAME_SIZE = 0x40
local FIRST_ADDED_ID = 821

local STEP_UPDATE = 2                           -- enum DeckEditStep
local STEP_CARD_VIEW = 13
local DETAIL_INPUT = 2                          -- enum CardDetailState
local CMD_CARD_VIEW = 0                         -- enum DeckEditCommand

local KEY_A, KEY_B, KEY_LEFT, KEY_DOWN = 0x01, 0x02, 0x20, 0x80

local PRESS_FRAMES = 3      -- frames a key is held
local GAP_FRAMES = 12       -- frames between presses in menus
local SETTLE_FRAMES = 8     -- frames after Card Detail's fade-in before the screenshot
local TIMEOUT_FRAMES = 1800

local root = script.dir .. "/.."
local outDir = root .. "/cards"
local logFile = io.open(outDir .. "/_log.txt", "w")

local function log(msg)
    console:log("[card_screenshots] " .. msg)
    if logFile then
        logFile:write(msg .. "\n")
        logFile:flush()
    end
end

local function trim(s)
    return (s:gsub("^%s+", ""):gsub("%s+$", ""))
end

-- The main menu's callback in any of the demo ROMs: CB_MainMenu (+1, Thumb) from eds_demo.sym and
-- eds_release_demo.sym next to the script's repo, else the fallback. Pressing A until some other value showed up
-- started a Campaign duel when the build had moved it.
local mainMenuCallbacks = {}
for _, symName in ipairs({ "eds_demo.sym", "eds_release_demo.sym" }) do
    local f = io.open(root .. "/" .. symName, "r")
    if f then
        for line in f:lines() do
            local addr = line:match("^(%x+)%s.*%sCB_MainMenu%s*$")
            if addr then
                mainMenuCallbacks[tonumber(addr, 16) + 1] = true
            end
        end
        f:close()
    end
end
if next(mainMenuCallbacks) == nil then
    mainMenuCallbacks[CB_MAIN_MENU_FALLBACK] = true
end
local function inMainMenu(cb) return mainMenuCallbacks[cb] == true end

-- [(name, game name)] above the ------ line, as tools/newcards.py load_list reads them.
local function loadCards()
    local f = io.open(root .. "/new_cards.txt", "r")
    if not f then
        return nil
    end
    local cards = {}
    for line in f:lines() do
        line = trim(line:gsub("\r", ""))
        if line:sub(1, 6) == "------" then
            break
        end
        if line ~= "" and line:sub(1, 1) ~= "#" then
            local name, gameName = line:match("^(.-)%s*|%s*(.-)$")
            if not name then
                name, gameName = line, line
            end
            if gameName == "" then
                gameName = name
            end
            cards[#cards + 1] = { name = name, gameName = gameName, id = FIRST_ADDED_ID + #cards }
        end
    end
    f:close()
    return cards
end

local function romCardName(id)
    local raw = emu:readRange(CARD_NAMES + id * CARD_NAME_SIZE, CARD_NAME_SIZE)
    local nul = raw:find("\0", 1, true)
    return nul and raw:sub(1, nul - 1) or raw
end

local function fileName(name)
    return (name:gsub('[<>:"/\\|%?%*]', "_")) .. ".png"
end

local allDemoCards = loadCards()
if not allDemoCards then
    log("cannot read " .. root .. "/new_cards.txt")
    return
end
local demoCardCount = #allDemoCards
local cards = allDemoCards

-- Optional cards/_capture_only.txt: one display name per line (from new_cards.txt, before |).
-- When present, only those cards are captured (for new entries without re-shooting the whole list).
local function loadCaptureOnly()
    local f = io.open(outDir .. "/_capture_only.txt", "r")
    if not f then
        return nil
    end
    local set = {}
    for line in f:lines() do
        line = trim(line:gsub("\r", ""))
        if line ~= "" and line:sub(1, 1) ~= "#" then
            set[line] = true
        end
    end
    f:close()
    return set
end

local captureOnly = loadCaptureOnly()
if captureOnly then
    local filtered = {}
    for _, c in ipairs(cards) do
        if captureOnly[c.name] then
            filtered[#filtered + 1] = c
        end
    end
    log(string.format("capture filter: %d of %d cards", #filtered, demoCardCount))
    cards = filtered
end

log(#cards .. " cards to capture")
local cardIdCount = FIRST_ADDED_ID + demoCardCount
local listRowSize = (cardIdCount + 80 + 15) * 2

local function callback() return emu:read32(MAIN_CALLBACK) end
local function step() return emu:read8(MAIN_SEQ_INDEX1) end
local function detailState() return emu:read16(CARD_DETAIL) >> 1 end
local function commandChoice() return (emu:read32(DE_COMMAND_MENU) >> 15) & 7 end
-- Left/Right switch lists while the bar is closed, so only steer the choice once it is fully open (rows == 2).
local function commandBarOpen() return (emu:read8(DE_COMMAND_MENU + 1) >> 5) & 3 == 2 end
local deckEditCallback
local function inListView() return callback() == deckEditCallback and step() == STEP_UPDATE end

-- Address of the list entry under the Deck Edit cursor.
local function cursorEntry()
    local list = emu:read8(DE_CUR_LIST)
    local row = emu:read8(DE_LIST_ROW + list)
    local pos = emu:read16(DE_LIST_POS + list * 2)
    local offsets = { [0] = 0, [1] = cardIdCount * 2, [2] = (cardIdCount + 80) * 2 }
    return DECK_EDIT_CARD_LISTS + row * listRowSize + offsets[list] + pos * 2
end

-- Key presses: press(k) holds k for PRESS_FRAMES, then nothing is held for `gap` frames.
local heldKey, holdLeft, gapLeft = 0, 0, 0
local function busy() return holdLeft > 0 or gapLeft > 0 end
local function press(k, gap)
    heldKey, holdLeft, gapLeft = k, PRESS_FRAMES, gap or GAP_FRAMES
    emu:addKeys(k)
end
local function tickKeys()
    if holdLeft > 0 then
        holdLeft = holdLeft - 1
        if holdLeft == 0 then
            emu:clearKeys(heldKey)
        end
    elseif gapLeft > 0 then
        gapLeft = gapLeft - 1
    end
end

local state, index, timer, menuPresses = "check", 1, 0, 0
local entryAddr, entryOld
local startClock = os.clock()
local cbid

local function stop(msg)
    log(msg)
    state = "done"
    callbacks:remove(cbid)
    if logFile then
        logFile:close()
        logFile = nil
    end
end

local function goTo(s)
    state, timer = s, 0
end

local function onFrame()
    tickKeys()
    if busy() then
        return
    end
    timer = timer + 1
    if timer > TIMEOUT_FRAMES and state ~= "check" then
        stop(string.format("stuck in state %s (callback 0x%08X, step %d, Card Detail state %d)", state,
            callback(), step(), detailState()))
        return
    end

    if state == "check" then
        for k, c in ipairs(cards) do
            local inRom = romCardName(c.id)
            if inRom ~= c.gameName then
                stop(string.format("card ID %d is \"%s\" in this ROM, but new_cards.txt entry \"%s\" expects \"%s\": "
                    .. "load a ROM built from new_cards.txt (make demo)", c.id, inRom, c.name, c.gameName))
                return
            end
        end
        goTo("title")
    elseif state == "title" then
        -- A skips the intro and picks the title screen's entry, until the main menu runs
        if inMainMenu(callback()) then
            goTo("mainMenu")
            gapLeft = 60
        else
            press(KEY_A, 30)
        end
    elseif state == "mainMenu" then
        menuPresses = menuPresses + 1
        if menuPresses <= 2 then
            press(KEY_DOWN)
        elseif menuPresses == 3 then
            press(KEY_A)
        elseif not inMainMenu(callback()) then
            deckEditCallback = callback()
            log(string.format("in Deck Edit (callback 0x%08X)", deckEditCallback))
            goTo("list")
        end
    elseif state == "list" then
        if index > #cards then
            stop(string.format("done: %d screenshots in %s (%.1f s)", #cards, outDir, os.clock() - startClock))
            return
        end
        if inListView() and timer > 20 then
            entryAddr = cursorEntry()
            entryOld = emu:read16(entryAddr)
            emu:write16(entryAddr, cards[index].id)
            press(KEY_A)
            goTo("menu")
        end
    elseif state == "menu" then
        if step() ~= STEP_UPDATE then
            goTo("view")
        elseif not commandBarOpen() then
            if timer % 90 == 0 then
                press(KEY_A)
            end
        elseif commandChoice() ~= CMD_CARD_VIEW then
            press(KEY_LEFT)
        else
            press(KEY_A)
        end
    elseif state == "view" then
        if step() == STEP_CARD_VIEW and detailState() == DETAIL_INPUT then
            gapLeft = SETTLE_FRAMES
            goTo("shot")
        end
    elseif state == "shot" then
        local c = cards[index]
        emu:screenshot(outDir .. "/" .. fileName(c.name))
        log(string.format("%3d/%d %s", index, #cards, c.name))
        press(KEY_B)
        goTo("close")
    elseif state == "close" then
        if inListView() then
            emu:write16(entryAddr, entryOld)
            index = index + 1
            goTo("list")
        end
    end
end

cbid = callbacks:add("frame", onFrame)
