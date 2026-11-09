--[[
Shared mGBA helpers for TESTING.md scenario scripts.

Recording basename (MP4 file name without extension):
  {card name}_{effect name}_{Player|CPU}

Use eds_demo.gba (make demo) with MOD_TEST_DECK and an empty Test Duel opponent deck.
Run from repo root, e.g.:

  mGBA.exe -C audioSync=0 -C videoSync=0 -C fpsTarget=6000 ^
      --script tools/testing_scenarios/poc/Card_Destruction_discard_and_draw_Player.lua eds_demo.gba

Start mGBA Tools -> Record A/V with output path set to the basename this script prints (see README), or use
tools/emu.py lua when headless (no MP4 from Lua API).
]]

local EDS = {}

EDS.G_DUEL = 0x020192E0
EDS.PLAYER0 = EDS.G_DUEL + 4
EDS.PLAYER1 = EDS.PLAYER0 + 0xD64
EDS.G_AI_STATE = 0x02015EF0
EDS.G_MOD_TESTING_FREEZE = 0x0202FF03 -- gModTestingFreezeOpponent (demo ROM; see mod_testing_automation.c)
EDS.AI_TURN_PHASE_PAST_END = 6 -- gAiTurnPhases NULL terminator index (AiRunTurn returns 1)
EDS.DUEL_CTRL_PHASE = 0x02015EE8
EDS.DUEL_STEP_DRAW = 3
EDS.DUEL_STEP_STANDBY = 4
EDS.DUEL_STEP_OPENING = 1
EDS.DUEL_STEP_TURN_START = 2
EDS.DUEL_STEP_MAIN = 5
EDS.G_CHAIN = 0x02017A40
EDS.DEFAULT_PLAYER_HAND = 5
EDS.DUEL_STEP_OPPONENT_TURN = 8
EDS.DUEL_STEP_RESULT = 9
EDS.G_DUEL_TURN_FLAGS = EDS.G_DUEL + 0x1B12
EDS.G_DUEL_CARDMENU = EDS.G_DUEL + 0x1B2C
EDS.G_DUEL_PHASE_STEP = EDS.G_DUEL + 0x1B20
EDS.G_DUEL_SCREEN = 0x0201CFB0
EDS.G_DUEL_SCREEN_SEL_PLAYER = EDS.G_DUEL_SCREEN + 0x824
EDS.G_DUEL_SCREEN_SEL_AREA = EDS.G_DUEL_SCREEN + 0x828
EDS.G_DUEL_SCREEN_FLAGS = EDS.G_DUEL_SCREEN + 0x808
EDS.DUEL_AREA_MONSTER = 0
EDS.DUEL_AREA_SPELL_TRAP = 5
EDS.DUEL_AREA_FUSION_DECK = 12
EDS.DUEL_AREA_DECK = 13
EDS.DUEL_AREA_HAND = 11
EDS.G_DUEL_SCREEN_SEL_INDEX = EDS.G_DUEL_SCREEN + 0x82C
EDS.CARDMENU_CMD_SET = 4
EDS.CARDMENU_CMD_ACTIVATE = 6
EDS.CARDMENU_CMD_DRAW = 8
EDS.CARDMENU_CMD_SURRENDER = 9
EDS.CARDMENU_CMD_MULLIGAN = 13

EDS.G_MAIN = 0x03000040
EDS.MAIN_CALLBACK = EDS.G_MAIN + 0x0410
EDS.MAIN_SEQ_INDEX1 = EDS.G_MAIN + 0x4859

EDS.CARD_NAMES = 0x0822C720
EDS.CARD_NAME_SIZE = 0x40
EDS.CARD_ID_LAST = 900 -- safe upper bound for name scan on demo builds

EDS.KEY_A, EDS.KEY_B = 0x01, 0x02
EDS.KEY_START, EDS.KEY_SELECT = 0x08, 0x04
EDS.KEY_RIGHT, EDS.KEY_LEFT = 0x10, 0x20
EDS.KEY_UP, EDS.KEY_DOWN = 0x40, 0x80
EDS.KEY_R, EDS.KEY_L = 0x100, 0x200

EDS.PRESS_FRAMES = 3
EDS.GAP_FRAMES = 12
EDS.TIMEOUT_FRAMES = 24000
-- GBA LCD 240x160; emu:screenshot() is usually native unless OpenGL high-res is on.
EDS.GBA_SCREEN_W = 240
EDS.GBA_SCREEN_H = 160
EDS.DEFAULT_CAPTURE_SCALE = 6 -- match mGBA -6 / scaleMultiplier; output MP4 via ffmpeg neighbor upscale

EDS.ZONE_SIZE = 0x94
EDS.ZONE_MON0 = 0x28
EDS.OFFSET_HAND = 0x684
EDS.OFFSET_DECK = 0x7C4
EDS.OFFSET_FUSION = 0xA44

local function libDir()
    local src = debug.getinfo(1, "S").source or ""
    if src:sub(1, 1) == "@" then
        src = src:sub(2)
    end
    src = src:gsub("\\", "/")
    return src:match("^(.*)/[^/]+$") or "tools/testing_scenarios"
end

local scenariosDir = libDir():gsub("/+$", "")
EDS.scenariosDir = scenariosDir
EDS.recordDir = scenariosDir .. "/recordings"
EDS.repoRoot = scenariosDir .. "/../.."

local formatDecksCache

local heldKey, holdLeft, gapLeft = 0, 0, 0

function EDS.log(msg)
    console:log("[testing] " .. msg)
end

function EDS.slug(s)
    s = s:gsub("[%c%z]", " ")
    -- mGBA's Lua: %w is letters/digits only (no underscore); keep _ explicitly
    s = s:gsub("[^%a%d%s_%-]", "")
    s = s:gsub("[%s%-]+", "_")
    s = s:gsub("^_+", ""):gsub("_+$", "")
    if s == "" then
        return "effect"
    end
    if #s > 56 then
        s = s:sub(1, 56)
    end
    return s
end

function EDS.slugEffect(s)
    s = EDS.slug(s):lower()
    local parts = {}
    for w in s:gmatch("[^_]+") do
        if #w > 0 then
            parts[#parts + 1] = w:sub(1, 1):upper() .. w:sub(2)
        end
    end
    if #parts == 0 then
        return "Effect"
    end
    local out = table.concat(parts, "_")
    if #out > 56 then
        out = out:sub(1, 56)
    end
    return out
end

function EDS.recordingBasename(card, effect, actor)
    return EDS.slug(card) .. "_" .. EDS.slugEffect(effect) .. "_" .. actor
end

function EDS.recordingPath(card, effect, actor)
    return EDS.recordDir .. "/" .. EDS.recordingBasename(card, effect, actor) .. ".mp4"
end

function EDS.preferredRecordingPath(card, effect, actor)
    local file = EDS.recordingBasename(card, effect, actor) .. ".mp4"
    local lib = libDir():gsub("\\", "/")
    if lib:match("^%a:") or lib:match("^/") then
        return (lib .. "/recordings/" .. file):gsub("/", "\\")
    end
    local rel = EDS.recordingPath(card, effect, actor)
    local root = EDS.repoRoot:gsub("\\", "/")
    if root:match("^%a:") or root:match("^/") then
        return (root .. "/tools/testing_scenarios/recordings/" .. file):gsub("/", "\\")
    end
    return rel:gsub("/", "\\")
end

function EDS.writeNextRecordingHint(paths)
    os.execute('mkdir "' .. EDS.recordDir:gsub("/", "\\") .. '" 2>nul')
    local f = io.open(EDS.recordDir .. "/_next_recording.txt", "w")
    if f then
        for _, p in ipairs(paths) do
            f:write(p .. "\n")
        end
        f:close()
    end
end

--[[ mGBA scripting (https://mgba.io/docs/dev/scripting.html) exposes emu:screenshot() only.
    Tools -> Record A/V is Qt-only and is not bound to Lua. Optional captureMp4 uses PNG frames + ffmpeg. ]]
function EDS.probeEmuRecordingApi()
    local found = {}
    if not emu then
        return found
    end
    for _, name in ipairs({
        "screenshot", "screenshotToImage", "startRecording", "stopRecording",
        "beginVideoLog", "endVideoLog", "videoRecord",
    }) do
        if emu[name] then
            found[#found + 1] = name
        end
    end
    return found
end

function EDS.ffmpegAvailable()
    local rc = os.execute("ffmpeg -version >nul 2>&1")
    return rc == 0 or rc == true
end

function EDS.startRecording(card, effect, actor, opts)
    opts = opts or {}
    local rel = EDS.recordingPath(card, effect, actor)
    local abs = EDS.preferredRecordingPath(card, effect, actor)
    EDS.writeNextRecordingHint({ abs, rel })
    EDS.log("MP4 target: " .. abs)
    local api = EDS.probeEmuRecordingApi()
    if #api > 0 then
        EDS.log("emu recording API: " .. table.concat(api, ", "))
    end
    local useCapture = opts.captureMp4
    if useCapture == nil then
        useCapture = EDS.ffmpegAvailable()
    end
    if useCapture and EDS.ffmpegAvailable() then
        EDS.log("MP4: scripted capture (emu:screenshot PNG sequence + ffmpeg). See captureMp4 in README.")
    else
        EDS.log("MP4: no ffmpeg on PATH — use mGBA Tools -> Record A/V -> MPEG-4 (Lua has no encoder API).")
        if useCapture then
            EDS.log("MP4: install ffmpeg and ensure it is on PATH, or pass captureMp4=false.")
        end
    end
    return abs, useCapture and EDS.ffmpegAvailable()
end

function EDS.initMp4Capture(frameState, mp4Path, opts)
    local dir = (EDS.recordDir .. "/" .. frameState.basename .. "_frames"):gsub("/", "\\")
    os.execute('if exist "' .. dir .. '" rmdir /s /q "' .. dir .. '" 2>nul')
    os.execute('mkdir "' .. dir .. '" 2>nul')
    frameState.captureDir = dir
    frameState.mp4Out = mp4Path
    frameState.captureIndex = 0
    frameState.captureFrom = opts.captureFrom or "in_duel"
    frameState.captureFps = opts.captureFps or 60
    frameState.captureScale = opts.captureScale or EDS.DEFAULT_CAPTURE_SCALE
    frameState.keepPngFrames = opts.keepPngFrames ~= false
    local outW = EDS.GBA_SCREEN_W * frameState.captureScale
    local outH = EDS.GBA_SCREEN_H * frameState.captureScale
    EDS.log(string.format(
        "Capture dir: %s (PNG usually %dx%d; MP4 encoded %dx%d, %dx integer scale)",
        dir, EDS.GBA_SCREEN_W, EDS.GBA_SCREEN_H, outW, outH, frameState.captureScale))
    EDS.log("Launch mGBA with -6 (or maximize window) so on-screen size matches capture scale.")
end

function EDS.captureFrameIfNeeded(frameState, opts)
    if not frameState.captureDir then
        return
    end
    local from = frameState.captureFrom
    if from == "in_duel" then
        if opts.setupCards and not frameState.scenarioSetupDone then
            return
        end
        if frameState.scenarioCaptureHold and frameState.scenarioCaptureHold > 0 then
            frameState.scenarioCaptureHold = frameState.scenarioCaptureHold - 1
            return
        end
        local p = frameState.phase
        if p == "draw_mulligan" then
            return
        end
        if p ~= "in_duel" and p ~= "playing" and p ~= "watch" then
            return
        end
    elseif from == "playing" and frameState.phase ~= "playing" and frameState.phase ~= "watch" then
        return
    end
    local n = frameState.captureIndex
    local path = string.format("%s/%06d.png", frameState.captureDir, n)
    if n == 0 and emu.screenshotToImage then
        local img = emu:screenshotToImage()
        if img and img.width and img.height then
            frameState.srcW = img.width
            frameState.srcH = img.height
            EDS.log(string.format("screenshotToImage: %dx%d", img.width, img.height))
        end
    end
    emu:screenshot(path)
    frameState.captureIndex = n + 1
end

function EDS.ffmpegScaleVf(frameState)
    local scale = frameState.captureScale or EDS.DEFAULT_CAPTURE_SCALE
    local wantW = EDS.GBA_SCREEN_W * scale
    local wantH = EDS.GBA_SCREEN_H * scale
    local sw = frameState.srcW or EDS.GBA_SCREEN_W
    local sh = frameState.srcH or EDS.GBA_SCREEN_H
    if sw == wantW and sh == wantH then
        return ""
    end
    return string.format("-vf scale=%d:%d:flags=neighbor", wantW, wantH)
end

function EDS.finalizeMp4Capture(frameState)
    if not frameState.captureDir or (frameState.captureIndex or 0) < 2 then
        EDS.log("MP4: not enough captured frames; no encode.")
        return false
    end
    local out = frameState.mp4Out
    local fps = frameState.captureFps or 60
    local pattern = frameState.captureDir:gsub("/", "\\") .. "\\%06d.png"
    local vf = EDS.ffmpegScaleVf(frameState)
    local cmd = string.format(
        'ffmpeg -y -hide_banner -loglevel error -framerate %d -i "%s" %s -pix_fmt yuv420p -c:v libx264 "%s"',
        fps, pattern, vf, out:gsub("/", "\\"))
    EDS.log(string.format("MP4: encoding %d frames -> %s", frameState.captureIndex, out))
    local rc = os.execute(cmd)
    local ok = rc == 0 or rc == true
    local probe = io.open(out, "rb")
    if probe then
        probe:close()
        EDS.log("MP4: wrote " .. out)
        ok = true
    elseif not ok then
        EDS.log("MP4: ffmpeg failed (is ffmpeg on PATH?).")
    end
    if ok and not frameState.keepPngFrames then
        os.execute('rmdir /s /q "' .. frameState.captureDir .. '" 2>nul')
    end
    return ok
end

function EDS.appendRunLog(line)
    os.execute('mkdir "' .. EDS.recordDir:gsub("/", "\\") .. '" 2>nul')
    local f = io.open(EDS.recordDir .. "/_last_run.log", "a")
    if f then
        f:write(line .. "\n")
        f:close()
    end
end

function EDS.busy()
    return holdLeft > 0 or gapLeft > 0
end

function EDS.press(key, gap)
    heldKey = key
    holdLeft = EDS.PRESS_FRAMES
    gapLeft = gap or EDS.GAP_FRAMES
    emu:addKeys(key)
end

function EDS.tickKeys()
    if holdLeft > 0 then
        holdLeft = holdLeft - 1
        if holdLeft == 0 then
            emu:clearKeys(heldKey)
        end
    elseif gapLeft > 0 then
        gapLeft = gapLeft - 1
    end
end

function EDS.duelStep()
    return emu:read8(EDS.DUEL_CTRL_PHASE)
end

function EDS.turnPlayer()
    return (emu:read8(EDS.G_DUEL_TURN_FLAGS) >> 1) & 1
end

-- Passive Test Duel opponent: no CPU turns and no chains / quick-play answers to your cards.
function EDS.freezeOpponent(opts)
    if opts.freezeOpponent == false then
        emu:write8(EDS.G_MOD_TESTING_FREEZE, 0)
        return
    end
    emu:write8(EDS.G_MOD_TESTING_FREEZE, 1)
    local p1 = emu:read8(EDS.PLAYER1 + 9)
    emu:write8(EDS.PLAYER1 + 9, p1 | 0x08) -- skipTurn (consumed at TurnStart)
    if EDS.duelStep() == EDS.DUEL_STEP_OPPONENT_TURN or EDS.turnPlayer() == 1 then
        emu:write8(EDS.G_AI_STATE, EDS.AI_TURN_PHASE_PAST_END)
        emu:write8(EDS.G_AI_STATE + 1, 0)
    end
end

function EDS.playerHandCount()
    return emu:read8(EDS.PLAYER0 + 2)
end

function EDS.opponentHandCount()
    return emu:read8(EDS.PLAYER1 + 2)
end

-- Chain list, response windows, or effect resolve still running on screen.
function EDS.chainOrEffectBusy()
    local flags491 = emu:read8(EDS.G_CHAIN + 0x491)
    if (flags491 & 0x40) ~= 0 then
        return true
    end
    local b3d0 = emu:read8(EDS.G_CHAIN + 0x3D0)
    if (b3d0 & 0x01) ~= 0 then
        return true
    end
    local b3d2 = emu:read8(EDS.G_CHAIN + 0x3D2)
    if (b3d2 & 0x01) ~= 0 then
        return true
    end
    return false
end

function EDS.openingDrawSatisfied(frameState)
    if frameState.openingHandRamDone or frameState.openingRamDrew then
        return true
    end
    if frameState.drawConfirmed and EDS.openingHandReady(frameState) then
        return true
    end
    if EDS.duelStep() >= EDS.DUEL_STEP_MAIN and (frameState.timer or 0) > 90 then
        return true
    end
    return false
end

function EDS.fillPlayerHandToCount(wantCount, opts)
    wantCount = wantCount or EDS.DEFAULT_PLAYER_HAND
    local playerBase = EDS.PLAYER0
    local count = emu:read8(playerBase + 2)
    if count >= wantCount then
        return
    end
    local fillerIds = {}
    local slug = EDS.formatSlug(opts and opts.format)
    local spec = EDS.loadFormatDecks()[slug]
    if spec and spec.main then
        for _, name in ipairs(spec.main) do
            local id = EDS.findCardId(name)
            if id then
                fillerIds[#fillerIds + 1] = id
            end
        end
    end
    for slot = count, wantCount - 1 do
        local id
        if #fillerIds > 0 then
            id = fillerIds[((slot - count) % #fillerIds) + 1]
        else
            id = EDS.findCardId("Dark Magician") or 1
        end
        EDS.setHandCard(playerBase, slot, id)
    end
    emu:write8(playerBase + 2, wantCount)
    EDS.log(string.format("Padded player hand to %d (was %d).", wantCount, count))
end

function EDS.inTestDuelFlow(frameState)
    local ph = frameState.phase
    return ph == "pre_duel" or ph == "cpu_turn" or ph == "draw_mulligan"
        or ph == "in_duel" or ph == "playing" or ph == "watch"
end

function EDS.inHumanMainPhase1()
    return EDS.duelStep() == EDS.DUEL_STEP_MAIN and EDS.turnPlayer() == 0
end

function EDS.inDuelMainPhase()
    return EDS.inHumanMainPhase1()
end

function EDS.cardMenuOpen()
    return (emu:read32(EDS.G_DUEL_CARDMENU) & 1) == 1
end

function EDS.cardMenuCommand()
    return (emu:read32(EDS.G_DUEL_CARDMENU) >> 2) & 0xF
end

function EDS.duelPhaseStep()
    return emu:read8(EDS.G_DUEL_PHASE_STEP)
end

function EDS.duelSelPlayer()
    return emu:read32(EDS.G_DUEL_SCREEN_SEL_PLAYER)
end

function EDS.duelSelArea()
    return emu:read32(EDS.G_DUEL_SCREEN_SEL_AREA)
end

-- Test Duel can leave the cursor on the visible CPU hand; snap to your hand for automation.
function EDS.forceOwnHandCursor()
    if EDS.duelSelPlayer() ~= 0 or EDS.duelSelArea() ~= EDS.DUEL_AREA_HAND then
        emu:write32(EDS.G_DUEL_SCREEN_SEL_PLAYER, 0)
        emu:write32(EDS.G_DUEL_SCREEN_SEL_AREA, EDS.DUEL_AREA_HAND)
        emu:write32(EDS.G_DUEL_SCREEN_SEL_INDEX, 0)
    end
end

function EDS.forceOwnDeckCursor()
    if EDS.duelSelPlayer() ~= 0 or EDS.duelSelArea() ~= EDS.DUEL_AREA_DECK then
        emu:write32(EDS.G_DUEL_SCREEN_SEL_PLAYER, 0)
        emu:write32(EDS.G_DUEL_SCREEN_SEL_AREA, EDS.DUEL_AREA_DECK)
        emu:write32(EDS.G_DUEL_SCREEN_SEL_INDEX, 0)
    end
end

function EDS.duelCursorReady()
    local flags = emu:read16(EDS.G_DUEL_SCREEN_FLAGS)
    return ((flags >> 3) & 1) == 1
end

function EDS.duelSelIndex()
    return emu:read32(EDS.G_DUEL_SCREEN_SEL_INDEX)
end

-- Block A only on deck-pile menus (Surrender/Mulligan/Draw). Hand command menus need A to confirm Activate.
function EDS.safeToPressA()
    local area = EDS.duelSelArea()
    if EDS.cardMenuOpen() then
        local cmd = EDS.cardMenuCommand()
        if cmd == EDS.CARDMENU_CMD_SURRENDER or cmd == EDS.CARDMENU_CMD_MULLIGAN
            or cmd == EDS.CARDMENU_CMD_DRAW then
            return false
        end
        if area == EDS.DUEL_AREA_DECK or area == EDS.DUEL_AREA_FUSION_DECK then
            return false
        end
        return true
    end
    if area == EDS.DUEL_AREA_DECK or area == EDS.DUEL_AREA_FUSION_DECK
        or area == EDS.DUEL_AREA_GRAVEYARD or area == EDS.DUEL_AREA_BANISHED then
        return false
    end
    return area == EDS.DUEL_AREA_HAND
end

function EDS.handMenuTowardActivate()
    if not EDS.cardMenuOpen() or EDS.duelSelArea() ~= EDS.DUEL_AREA_HAND then
        return false
    end
    if EDS.cardMenuCommand() == EDS.CARDMENU_CMD_ACTIVATE then
        return true
    end
    if not EDS.busy() then
        EDS.press(EDS.KEY_RIGHT, 14)
    end
    return false
end

function EDS.prepHandStep(frameState)
    EDS.forceOwnHandCursor()
    if EDS.cardMenuOpen() then
        if not EDS.busy() then
            EDS.press(EDS.KEY_B, 20)
        end
        return
    end
    local area = EDS.duelSelArea()
    if area == EDS.DUEL_AREA_HAND then
        if EDS.duelSelIndex() ~= 0 then
            EDS.press(EDS.KEY_LEFT, 18)
            return
        end
        frameState.handReady = true
        EDS.log(string.format("Hand selected (index %d).", EDS.duelSelIndex()))
        return
    end
    if EDS.busy() then
        return
    end
    -- duel_cursor.c (player 0): Deck -> Left -> Spell/Trap; Spell/Trap -> Down -> Hand.
    if area == EDS.DUEL_AREA_DECK then
        EDS.press(EDS.KEY_LEFT, 22)
    elseif area == EDS.DUEL_AREA_SPELL_TRAP then
        EDS.press(EDS.KEY_DOWN, 22)
    elseif area == EDS.DUEL_AREA_FUSION_DECK then
        EDS.press(EDS.KEY_LEFT, 22)
    else
        if (frameState.timer % 60) < 30 then
            EDS.press(EDS.KEY_DOWN, 22)
        else
            EDS.press(EDS.KEY_LEFT, 22)
        end
    end
    if frameState.timer % 90 == 0 then
        EDS.log(string.format("hand_prep area=%d index=%d", area, EDS.duelSelIndex()))
    end
end

function EDS.romCardName(id)
    local raw = emu:readRange(EDS.CARD_NAMES + id * EDS.CARD_NAME_SIZE, EDS.CARD_NAME_SIZE)
    local nul = raw:find("\0", 1, true)
    return nul and raw:sub(1, nul - 1) or raw
end

function EDS.findCardId(name)
    local want = name:lower()
    for id = 1, EDS.CARD_ID_LAST do
        local n = EDS.romCardName(id)
        if n ~= "" and n:lower() == want then
            return id
        end
    end
    return nil
end

function EDS.playerIndexFromBase(playerBase)
    return playerBase == EDS.PLAYER1 and 1 or 0
end

-- struct DuelCard: id bits 0-11, owner bit 12 (whose GY/deck/hand on discard).
function EDS.writeDuelCard(addr, cardId, ownerPlayer)
    local id = cardId & 0xFFF
    local owner = (ownerPlayer or 0) & 1
    emu:write32(addr, id | (owner << 12))
end

function EDS.writeCardWord(addr, cardId, ownerPlayer)
    EDS.writeDuelCard(addr, cardId, ownerPlayer)
end

function EDS.formatSlug(displayName)
    if not displayName or displayName == "" then
        return "yugi-kaiba"
    end
    local s = displayName:lower():gsub("[^%a%d]+", "-"):gsub("-+", "-")
    return (s:gsub("^-", ""):gsub("-$", ""))
end

function EDS.loadFormatDecks()
    if formatDecksCache then
        return formatDecksCache
    end
    local path = EDS.scenariosDir .. "/format_decks.lua"
    local ok, tbl = pcall(dofile, path)
    if ok and type(tbl) == "table" then
        formatDecksCache = tbl
    else
        formatDecksCache = {}
        EDS.log("Could not load format_decks.lua")
    end
    return formatDecksCache
end

-- DuelPlayer +0x009 bit 2: skipDrawPhase (consumed when the Draw Phase runs).
function EDS.setPlayerSkipDraw(playerBase, on)
    local b = emu:read8(playerBase + 9)
    if on then
        emu:write8(playerBase + 9, b | 0x04)
    else
        emu:write8(playerBase + 9, b & 0xFB)
    end
end

-- deck[0] is the next card to draw (ModDeckListVisible / duel pile order).
-- Move every hand card to the bottom of the main deck (for redoing a scripted opening draw).
function EDS.returnHandToMainDeck(playerBase)
    local handCount = emu:read8(playerBase + 2)
    if handCount == 0 then
        return
    end
    local deckCount = emu:read8(playerBase + 3)
    local owner = EDS.playerIndexFromBase(playerBase)
    for i = 0, handCount - 1 do
        local card = emu:read32(playerBase + EDS.OFFSET_HAND + i * 4)
        EDS.writeDuelCard(playerBase + EDS.OFFSET_DECK + deckCount * 4, card, owner)
        deckCount = deckCount + 1
    end
    emu:write8(playerBase + 2, 0)
    emu:write8(playerBase + 3, deckCount)
end

function EDS.drawFromMainDeck(playerBase, want)
    want = want or EDS.DEFAULT_PLAYER_HAND
    local handCount = emu:read8(playerBase + 2)
    local deckCount = emu:read8(playerBase + 3)
    local need = want - handCount
    if need <= 0 or deckCount <= 0 then
        return 0
    end
    local drawn = 0
    local owner = EDS.playerIndexFromBase(playerBase)
    while drawn < need and deckCount > 0 do
        local card = emu:read32(playerBase + EDS.OFFSET_DECK)
        EDS.writeDuelCard(playerBase + EDS.OFFSET_HAND + handCount * 4, card, owner)
        handCount = handCount + 1
        for i = 0, deckCount - 2 do
            emu:write32(
                playerBase + EDS.OFFSET_DECK + i * 4,
                emu:read32(playerBase + EDS.OFFSET_DECK + (i + 1) * 4))
        end
        deckCount = deckCount - 1
        drawn = drawn + 1
    end
    emu:write8(playerBase + 2, handCount)
    emu:write8(playerBase + 3, deckCount)
    return drawn
end

function EDS.deckTopCardId(playerBase)
    return emu:read32(playerBase + EDS.OFFSET_DECK) & 0xFFF
end

-- Main deck: at most one shuffle per player per script run.
function EDS.shufflePlayerMainDeckOnce(frameState)
    if frameState and frameState.p0ShuffledOnce then
        return false
    end
    if EDS.shuffleMainDeck(EDS.PLAYER0) then
        if frameState then
            frameState.p0ShuffledOnce = true
        end
        return true
    end
    return false
end

function EDS.shuffleCpuMainDeckOnce(frameState)
    if frameState and frameState.p1ShuffledOnce then
        return false
    end
    if EDS.shuffleMainDeck(EDS.PLAYER1) then
        if frameState then
            frameState.p1ShuffledOnce = true
        end
        return true
    end
    return false
end

function EDS.shuffleMainDeck(playerBase)
    local n = emu:read8(playerBase + 3)
    if n < 2 then
        EDS.log(string.format("shuffleMainDeck: deckCount=%d (skip shuffle)", n))
        return false
    end
    local cards = {}
    for i = 0, n - 1 do
        cards[i + 1] = emu:read32(playerBase + EDS.OFFSET_DECK + i * 4)
    end
    math.randomseed((os.time() % 0x7fffffff) + emu:currentFrame() + playerBase)
    for i = n, 2, -1 do
        local j = math.random(i)
        cards[i], cards[j] = cards[j], cards[i]
    end
    for i = 1, n do
        emu:write32(playerBase + EDS.OFFSET_DECK + (i - 1) * 4, cards[i])
    end
    return true
end

function EDS.setDeckPile(playerBase, offset, cardIds, ownerPlayer)
    ownerPlayer = ownerPlayer or EDS.playerIndexFromBase(playerBase)
    for i, id in ipairs(cardIds) do
        if i > 80 then
            break
        end
        EDS.writeDuelCard(playerBase + offset + (i - 1) * 4, id, ownerPlayer)
    end
    if offset == EDS.OFFSET_DECK then
        emu:write8(playerBase + 3, math.min(#cardIds, 80))
    elseif offset == EDS.OFFSET_FUSION then
        emu:write8(playerBase + 5, math.min(#cardIds, 80))
    end
end

function EDS.ensureUsageDecks(opts, frameState)
    if frameState.usageDecksDone or opts.usageDeck == false then
        return
    end
    local lp0 = emu:read16(EDS.PLAYER0)
    if lp0 < 2000 or lp0 > 32000 then
        return
    end
    local phase = frameState.phase
    local step = EDS.duelStep()
    local inDuel = lp0 >= 2000 and lp0 <= 32000 and step >= EDS.DUEL_STEP_OPENING and step < EDS.DUEL_STEP_RESULT
    if not inDuel and phase ~= "cpu_turn" and phase ~= "draw_mulligan" and phase ~= "pre_duel" then
        return
    end
    if phase == "pre_duel" and (frameState.timer or 0) < 2 then
        return
    end
    -- Going first: do not load/shuffle until Turn Start (Opening setup would clobber the deck).
    if EDS.turnPlayer() == 0 and step < EDS.DUEL_STEP_TURN_START then
        return
    end
    EDS.applyUsageDecks(opts, frameState)
    frameState.usageDecksDone = true
    frameState.deckShuffledAtLoad = frameState.p0ShuffledOnce == true
    frameState.requireOpeningDraw = true
end

function EDS.applyUsageDecks(opts, frameState)
    if opts.usageDeck == false then
        return
    end
    frameState = frameState or {}
    local slug = EDS.formatSlug(opts.format)
    local decks = EDS.loadFormatDecks()
    local spec = decks[slug]
    if not spec then
        EDS.log("No usage deck for format " .. tostring(opts.format) .. " (" .. slug .. ")")
        return
    end
    local mainIds, extraIds = {}, {}
    for _, name in ipairs(spec.main or {}) do
        local id = EDS.findCardId(name)
        if id then
            mainIds[#mainIds + 1] = id
        end
    end
    for _, name in ipairs(spec.extra or {}) do
        local id = EDS.findCardId(name)
        if id then
            extraIds[#extraIds + 1] = id
        end
    end
    if #mainIds < 10 then
        EDS.log("Usage deck resolved too few cards; skipping pile setup.")
        return
    end
    EDS.setDeckPile(EDS.PLAYER0, EDS.OFFSET_DECK, mainIds, 0)
    EDS.setDeckPile(EDS.PLAYER1, EDS.OFFSET_DECK, mainIds, 1)
    if #extraIds > 0 then
        EDS.setDeckPile(EDS.PLAYER0, EDS.OFFSET_FUSION, extraIds, 0)
        EDS.setDeckPile(EDS.PLAYER1, EDS.OFFSET_FUSION, extraIds, 1)
    end
    emu:write8(EDS.PLAYER0 + 2, 0)
    emu:write8(EDS.PLAYER1 + 2, 0)
    local p0TopBefore = EDS.deckTopCardId(EDS.PLAYER0)
    local p1TopBefore = EDS.deckTopCardId(EDS.PLAYER1)
    local p0Shuffled = EDS.shufflePlayerMainDeckOnce(frameState)
    local p1Shuffled = EDS.shuffleCpuMainDeckOnce(frameState)
    local p0TopAfter = EDS.deckTopCardId(EDS.PLAYER0)
    local p1TopAfter = EDS.deckTopCardId(EDS.PLAYER1)
    EDS.log(string.format(
        "Usage deck %s / %s: main=%d extra=%d; P0 shuffled=%s (top %d->%d) P1 shuffled=%s (top %d->%d), hands cleared.",
        slug, spec.deckType or "?", #mainIds, #extraIds,
        tostring(p0Shuffled), p0TopBefore, p0TopAfter,
        tostring(p1Shuffled), p1TopBefore, p1TopAfter))
    if opts.ramOpeningDraw ~= false then
        EDS.setPlayerSkipDraw(EDS.PLAYER0, true)
    end
end

-- RAM-draw 5 from the deck shuffled once at usage-deck load (ramOpeningDraw default path).
function EDS.tickPlayerOpeningDraw(frameState, opts)
    opts = opts or {}
    if opts.ramOpeningDraw == false then
        return
    end
    if frameState.openingRamDrew or not frameState.usageDecksDone or not frameState.deckShuffledAtLoad then
        return
    end
    if EDS.turnPlayer() ~= 0 then
        return
    end
    local step = EDS.duelStep()
    if step < EDS.DUEL_STEP_TURN_START or step > EDS.DUEL_STEP_STANDBY then
        return
    end
    local want = frameState.minPlayerHand or EDS.DEFAULT_PLAYER_HAND
    frameState.requireOpeningDraw = true
    EDS.setPlayerSkipDraw(EDS.PLAYER0, true)
    if EDS.playerHandCount() > 0 and not frameState.openingRamDrew then
        EDS.returnHandToMainDeck(EDS.PLAYER0)
        EDS.log("Returned early hand to deck (no re-shuffle; RAM-draw 5 next).")
    end
    local deckN = emu:read8(EDS.PLAYER0 + 3)
    if deckN < want then
        return
    end
    if EDS.playerHandCount() < want then
        local drawn = EDS.drawFromMainDeck(EDS.PLAYER0, want)
        if EDS.playerHandCount() >= want then
            frameState.openingRamDrew = true
            frameState.openingHandRamDone = true
            frameState.drawConfirmed = true
            EDS.log(string.format(
                "Opening hand: RAM drew %d (hand=%d deck=%d); skip in-game Draw.",
                drawn, EDS.playerHandCount(), emu:read8(EDS.PLAYER0 + 3)))
        end
    end
end

function EDS.openingHandReady(frameState)
    local want = frameState.minPlayerHand or EDS.DEFAULT_PLAYER_HAND
    return EDS.playerHandCount() >= want
end

-- Apply scenario hand/field after the game draw, during Draw or Standby (before Main Phase / capture).
function EDS.canApplyScenarioSetup(frameState, opts)
    opts = opts or {}
    if EDS.turnPlayer() ~= 0 then
        return false
    end
    local step = EDS.duelStep()
    if step < EDS.DUEL_STEP_DRAW or step > EDS.DUEL_STEP_STANDBY then
        return false
    end
    if not EDS.openingHandReady(frameState) then
        return false
    end
    if opts.ramOpeningDraw ~= false then
        return frameState.openingRamDrew == true
    end
    return frameState.drawConfirmed == true
end

function EDS.tickScenarioSetup(opts, frameState)
    if frameState.scenarioSetupDone or not opts.setupCards then
        return
    end
    if not EDS.canApplyScenarioSetup(frameState, opts) then
        return
    end
    EDS.defaultSetup(opts)
    frameState.scenarioSetupDone = true
    frameState.scenarioCaptureHold = 3
    EDS.forceOwnHandCursor()
    EDS.log(string.format(
        "Scenario RAM applied at duel_step=%d (after opening draw).", EDS.duelStep()))
end

function EDS.setHandCard(playerBase, slot, cardId)
    EDS.writeDuelCard(playerBase + EDS.OFFSET_HAND + slot * 4, cardId, EDS.playerIndexFromBase(playerBase))
    local count = emu:read8(playerBase + 2)
    if count < slot + 1 then
        emu:write8(playerBase + 2, slot + 1)
    end
end

function EDS.setMonster(playerBase, zone, cardId, defense)
    local z = playerBase + EDS.ZONE_MON0 + zone * EDS.ZONE_SIZE
    EDS.writeDuelCard(z, cardId, EDS.playerIndexFromBase(playerBase))
    if defense then
        emu:write8(z + 6, 1) -- isDefense at DuelZone+6 bit 0 (approximation for test harness)
    end
end

function EDS.setSpellTrap(playerBase, zone, cardId, setCard)
    local z = playerBase + EDS.ZONE_MON0 + (5 + zone) * EDS.ZONE_SIZE
    EDS.writeDuelCard(z, cardId, EDS.playerIndexFromBase(playerBase))
    if setCard then
        emu:write8(z + 7, 0) -- face-down hint; exact layout may need savestate load
    end
end

-- Minimal state setup hooks (POC scripts extend via opts.setup).
function EDS.defaultSetup(opts)
    local setup = opts.setupCards
    if not setup then
        return
    end
    local handHigh = { 0, 0 }
    for _, row in ipairs(setup) do
        local pidx = row.player == 1 and 2 or 1
        local who = row.player == 1 and EDS.PLAYER1 or EDS.PLAYER0
        local id = row.id or EDS.findCardId(row.name)
        if not id then
            EDS.log("Unknown card: " .. tostring(row.name))
        elseif row.zone == "hand" then
            local slot = row.slot or 0
            EDS.setHandCard(who, slot, id)
            if slot + 1 > handHigh[pidx] then
                handHigh[pidx] = slot + 1
            end
        elseif row.zone == "monster" then
            EDS.setMonster(who, row.slot or 0, id, row.defense)
        elseif row.zone == "st" then
            EDS.setSpellTrap(who, row.slot or 0, id, row.set)
        end
    end
    if handHigh[1] > 0 then
        emu:write8(EDS.PLAYER0 + 2, math.max(emu:read8(EDS.PLAYER0 + 2), handHigh[1]))
    end
    if handHigh[2] > 0 then
        emu:write8(EDS.PLAYER1 + 2, math.max(emu:read8(EDS.PLAYER1 + 2), handHigh[2]))
    end
    EDS.log(string.format(
        "RAM setup: P0 hand=%d deck=%d, P1 hand=%d deck=%d (opponent hand hidden in Test Duel UI).",
        emu:read8(EDS.PLAYER0 + 2), emu:read8(EDS.PLAYER0 + 3),
        emu:read8(EDS.PLAYER1 + 2), emu:read8(EDS.PLAYER1 + 3)))
end

local mainMenuCallbacks = {}
for _, symName in ipairs({ "eds_demo.sym", "eds_release_demo.sym" }) do
    local f = io.open(EDS.repoRoot .. "/" .. symName, "r")
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
    mainMenuCallbacks[0x08003B14 + 1] = true
    EDS.log("CB_MainMenu: using fallback 0x08003B15 (rebuild eds_demo.sym if menu detect fails)")
end

local function currentCallback()
    return emu:read32(EDS.MAIN_CALLBACK)
end

local function inMainMenu()
    return mainMenuCallbacks[currentCallback()] == true
end

-- Campaign -> Test Duel (MOD_TEST_DECK cursor) -> confirm -> duel. Fragile; prefer savestate at Main 1 when available.
function EDS.navigateTestDuel(frameState, opts)
    opts = opts or {}
    if frameState.phase == "done" then
        return
    end
    if frameState.phase == "wait_duel" and EDS.inDuelMainPhase() then
        frameState.phase = "in_duel"
        frameState.timer = 0
        EDS.log("Reached Main Phase.")
        return
    end
    if EDS.busy() then
        return
    end
    local limit = frameState.timeout or EDS.TIMEOUT_FRAMES
    if frameState.phase == "boot" then
        frameState.timer = frameState.timer + 1
        if inMainMenu() then
            frameState.phase = "main_menu"
            frameState.timer = 0
            EDS.log("Main menu (callback 0x" .. string.format("%08X", currentCallback()) .. ").")
        elseif EDS.inDuelMainPhase() then
            frameState.phase = "in_duel"
            frameState.timer = 0
        else
            -- Title / publisher / intro: mash A (and START once) like tools/card_screenshots.lua
            if frameState.timer % 30 == 1 then
                EDS.press(EDS.KEY_A, 28)
            elseif frameState.timer == 90 then
                EDS.press(EDS.KEY_START, 28)
            end
            if frameState.timer % 600 == 0 then
                EDS.log(string.format("title_wait f=%d cb=0x%08X", frameState.timer, currentCallback()))
            end
            if frameState.timer > limit then
                EDS.log("Timeout waiting for main menu.")
                frameState.phase = "done"
                frameState.failReason = "timeout_boot"
            end
        end
        return
    end
    if frameState.phase == "main_menu" then
        frameState.timer = frameState.timer + 1
        -- First menu row is Campaign (tools/emu/states/duel_start.txt).
        if frameState.timer == 25 or frameState.timer == 70 then
            EDS.press(EDS.KEY_A, 24)
        end
        if frameState.timer > 45 and not inMainMenu() then
            frameState.phase = "opponent_pick"
            frameState.timer = 0
            EDS.log("Campaign selected (callback 0x" .. string.format("%08X", currentCallback()) .. ").")
        end
        if frameState.timer > limit then
            EDS.log("Timeout leaving main menu.")
            frameState.phase = "done"
            frameState.failReason = "timeout_main_menu"
        end
        return
    end
    if frameState.phase == "opponent_pick" or frameState.phase == "pre_duel" then
        frameState.timer = frameState.timer + 1
        local step = EDS.duelStep()
        if step == EDS.DUEL_STEP_DRAW and EDS.turnPlayer() == 0 then
            frameState.phase = "draw_mulligan"
            frameState.timer = 0
            frameState.drawSub = 0
            frameState.requireOpeningDraw = true
            EDS.log("Draw Phase: deck menu defaults to Mulligan; use Right then Draw.")
            return
        end
        if EDS.inHumanMainPhase1() then
            if frameState.requireOpeningDraw and not EDS.openingDrawSatisfied(frameState) then
                frameState.phase = "draw_mulligan"
                frameState.timer = 0
                EDS.log(string.format(
                    "pre_duel: Main 1 but opening draw incomplete (hand=%d).",
                    EDS.playerHandCount()))
                return
            end
            frameState.phase = "in_duel"
            frameState.timer = 0
            EDS.log("Player Main Phase 1.")
            return
        end
        if step == EDS.DUEL_STEP_OPPONENT_TURN or EDS.turnPlayer() == 1 then
            frameState.phase = "cpu_turn"
            frameState.timer = 0
            EDS.log("Opponent's turn (waiting for your Main Phase 1).")
            return
        end
        -- MOD_TEST_DECK: cursor should start on Test Duel; confirm with A through deck flow.
        if frameState.timer % 32 == 1 then
            EDS.press(EDS.KEY_A, 20)
        end
        if frameState.timer == 200 then
            EDS.press(EDS.KEY_START, 20)
        end
        if frameState.timer % 900 == 0 then
            EDS.log(string.format("pre_duel f=%d phase_byte=%d cb=0x%08X",
                frameState.timer, step, currentCallback()))
        end
        if frameState.timer > limit then
            EDS.log("Timeout entering duel.")
            frameState.phase = "done"
            frameState.failReason = "timeout_pre_duel"
        end
        return
    end
    if frameState.phase == "cpu_turn" then
        frameState.timer = frameState.timer + 1
        if EDS.duelStep() == EDS.DUEL_STEP_DRAW and EDS.turnPlayer() == 0 then
            frameState.phase = "draw_mulligan"
            frameState.timer = 0
            frameState.requireOpeningDraw = true
            EDS.log("Player Draw Phase.")
            return
        end
        if EDS.inHumanMainPhase1() then
            if frameState.requireOpeningDraw and not EDS.openingDrawSatisfied(frameState) then
                frameState.phase = "draw_mulligan"
                frameState.timer = 0
                EDS.log(string.format(
                    "cpu_turn: Main 1 but opening draw incomplete (hand=%d).",
                    EDS.playerHandCount()))
                return
            end
            frameState.phase = "in_duel"
            frameState.timer = 0
            EDS.log("Player Main Phase 1.")
            return
        end
        if frameState.timer % 900 == 0 then
            EDS.log(string.format("cpu_turn f=%d duel_step=%d turn_player=%d",
                frameState.timer, EDS.duelStep(), EDS.turnPlayer()))
        end
        if frameState.timer > limit then
            frameState.phase = "done"
            frameState.failReason = "timeout_cpu_turn"
        end
        return
    end
    if frameState.phase == "draw_mulligan" then
        frameState.timer = frameState.timer + 1
        local step = EDS.duelStep()
        if EDS.inHumanMainPhase1() then
            if frameState.requireOpeningDraw and not EDS.openingDrawSatisfied(frameState) then
                if step == EDS.DUEL_STEP_DRAW or EDS.playerHandCount() < (frameState.minPlayerHand or EDS.DEFAULT_PLAYER_HAND) then
                    EDS.log(string.format(
                        "draw_mulligan: need opening draw (hand=%d step=%d).",
                        EDS.playerHandCount(), step))
                else
                    frameState.drawConfirmed = true
                end
            else
                frameState.phase = "in_duel"
                frameState.timer = 0
                EDS.log("Player Main Phase 1.")
                return
            end
        end
        if EDS.turnPlayer() == 1 then
            frameState.phase = "cpu_turn"
            frameState.timer = 0
            return
        end
        if EDS.busy() then
            return
        end
        local wantHand = frameState.minPlayerHand or EDS.DEFAULT_PLAYER_HAND
        local scenarioOk = not opts.setupCards or frameState.scenarioSetupDone
        if scenarioOk and EDS.openingDrawSatisfied(frameState) then
            if EDS.inHumanMainPhase1() or step >= EDS.DUEL_STEP_STANDBY then
                frameState.phase = "in_duel"
                frameState.timer = 0
                EDS.log("Entering in_duel after opening draw + scenario RAM.")
                return
            end
        end
        if step == EDS.DUEL_STEP_DRAW and EDS.turnPlayer() == 0 then
            if opts.ramOpeningDraw ~= false then
                if EDS.cardMenuOpen() and not EDS.busy() and frameState.timer % 28 == 5 then
                    EDS.press(EDS.KEY_B, 22)
                end
            elseif not frameState.drawConfirmed or EDS.playerHandCount() < wantHand then
                local menuOpen = EDS.cardMenuOpen()
                local cmd = EDS.cardMenuCommand()
                if not menuOpen and not EDS.busy() then
                    EDS.forceOwnDeckCursor()
                end
                local ready = EDS.duelCursorReady()
                    and EDS.duelSelArea() == EDS.DUEL_AREA_DECK
                if not menuOpen then
                    frameState.drawPickedDraw = false
                    if ready and frameState.timer >= 20 and frameState.timer % 30 == 5 then
                        EDS.press(EDS.KEY_A, 28)
                    elseif frameState.timer >= 45 and frameState.timer % 35 == 5 then
                        EDS.forceOwnDeckCursor()
                        EDS.press(EDS.KEY_A, 28)
                    end
                else
                    if cmd == EDS.CARDMENU_CMD_MULLIGAN and not frameState.drawPickedDraw then
                        EDS.press(EDS.KEY_RIGHT, 28)
                        frameState.drawPickedDraw = true
                    elseif cmd == EDS.CARDMENU_CMD_DRAW then
                        if not frameState.drawConfirmed then
                            EDS.press(EDS.KEY_A, 28)
                            frameState.drawConfirmed = true
                        end
                    elseif frameState.timer % 35 == 10 then
                        EDS.press(EDS.KEY_RIGHT, 28)
                    end
                end
                if frameState.drawConfirmed and EDS.cardMenuOpen()
                    and EDS.playerHandCount() >= wantHand
                    and frameState.timer % 45 == 20 then
                    EDS.press(EDS.KEY_B, 22)
                end
            end
        elseif step == EDS.DUEL_STEP_STANDBY then
            -- Standby advances on its own; A on the deck opens Surrender (no Mulligan/Draw).
            if EDS.cardMenuOpen() and not EDS.busy() and frameState.timer % 30 == 5 then
                EDS.press(EDS.KEY_B, 20)
            end
        end
        if frameState.timer % 600 == 0 then
            EDS.log(string.format(
                "draw_mulligan f=%d duel_step=%d phaseStep=%d area=%d cursor=%s menu=%s cmd=%d",
                frameState.timer, step, EDS.duelPhaseStep(), EDS.duelSelArea(),
                tostring(EDS.duelCursorReady()), tostring(EDS.cardMenuOpen()), EDS.cardMenuCommand()))
        end
        if frameState.timer > limit then
            EDS.log("Timeout in Draw / Standby (mulligan menu).")
            frameState.phase = "done"
            frameState.failReason = "timeout_draw"
        end
        return
    end
end

function EDS.runPlaySequence(seq, frameState)
    if not seq or #seq == 0 then
        return false
    end
    local step = frameState.playIndex
    if step > #seq then
        return true
    end
    if EDS.busy() then
        return false
    end
    local item = seq[step]
    if item.wait then
        frameState.playWait = (frameState.playWait or 0) + 1
        if frameState.playWait >= item.wait then
            frameState.playWait = 0
            frameState.playIndex = step + 1
        end
        return false
    end
    if item.key then
        if item.key == EDS.KEY_A then
            if EDS.cardMenuOpen() and EDS.duelSelArea() == EDS.DUEL_AREA_HAND then
                if not EDS.handMenuTowardActivate() then
                    return false
                end
            elseif not EDS.safeToPressA() then
                return false
            end
        end
        EDS.press(item.key, item.gap or EDS.GAP_FRAMES)
        EDS.log(string.format("Play step %d/%d (key).", step, #seq))
        frameState.playIndex = step + 1
    end
    return false
end

--[[
opts fields:
  card, effect, actor ("Player" | "CPU")
  scenarioText (string)
  skipNavigation (bool) — already in duel at Main 1
  setupCards — list for defaultSetup
  play — array of {key=, gap=, wait=} or function(frameState) returning done bool
  recordFrames — auto-stop after N frames once play finishes (CPU watch)
  freezeOpponent — default true for Player actor: passive CPU; false for CPU-watch POCs
  captureMp4 — PNG frames via emu:screenshot + ffmpeg (default: on when ffmpeg on PATH)
  captureFrom — "boot" | "in_duel" | "playing" (default in_duel)
  captureFps — ffmpeg -framerate (default 60)
  captureScale — integer upscale to 240×160 (default 6 → 1440×960 MP4)
]]
function EDS.run(opts)
    opts = opts or {}
    local actor = opts.actor or "Player"
    if opts.freezeOpponent == nil then
        opts.freezeOpponent = actor ~= "CPU"
    end
    -- false (default): one Lua shuffle + Mulligan/Draw menu (A on deck). true: skipDraw + RAM-draw 5.
    if opts.ramOpeningDraw == nil then
        opts.ramOpeningDraw = false
    end
    if opts.recordTail == nil then
        opts.recordTail = 0
    end
    local timeout = opts.maxFrames or EDS.TIMEOUT_FRAMES
    local basename = EDS.recordingBasename(opts.card or "Card", opts.effect or "effect", actor)
    EDS.appendRunLog("START " .. basename)
    if opts.pocRequired and (not opts.setupCards or not opts.play) then
        EDS.appendRunLog("FAIL " .. basename .. " missing_poc_table")
        EDS.log("ERROR: POC table not found for " .. basename)
        return
    end
    local mp4Path, doCapture = EDS.startRecording(opts.card or "Card", opts.effect or "effect", actor, opts)
    EDS.log(string.format("%s / %s / %s", opts.card, opts.effect, actor))
    if opts.scenarioText then
        EDS.log(opts.scenarioText:sub(1, 120))
    end

    local frameState = {
        phase = opts.skipNavigation and "in_duel" or "boot",
        timer = 0,
        playIndex = 1,
        playDone = false,
        recordStopAt = nil,
        timeout = timeout,
        basename = basename,
        minPlayerHand = opts.minPlayerHand or EDS.DEFAULT_PLAYER_HAND,
    }
    if doCapture then
        EDS.initMp4Capture(frameState, mp4Path, opts)
    end

    local cbid
    local keysId
    keysId = callbacks:add("keys", function()
        if opts.ramOpeningDraw ~= false then
            return
        end
        if frameState.phase ~= "draw_mulligan" then
            return
        end
        if EDS.turnPlayer() ~= 0 or EDS.duelStep() ~= EDS.DUEL_STEP_DRAW then
            return
        end
        if EDS.cardMenuOpen() or frameState.timer < 20 then
            return
        end
        if frameState.drawConfirmed then
            return
        end
        EDS.forceOwnDeckCursor()
        emu:addKeys(EDS.KEY_A)
    end)
    cbid = callbacks:add("frame", function()
        EDS.tickKeys()
        if EDS.inTestDuelFlow(frameState) then
            EDS.freezeOpponent(opts)
        end
        EDS.ensureUsageDecks(opts, frameState)
        if EDS.inTestDuelFlow(frameState) then
            EDS.tickPlayerOpeningDraw(frameState, opts)
        end
        EDS.tickScenarioSetup(opts, frameState)
        EDS.captureFrameIfNeeded(frameState, opts)
        if frameState.phase ~= "in_duel" and frameState.phase ~= "playing" and frameState.phase ~= "watch" then
            EDS.navigateTestDuel(frameState, opts)
        end
        if frameState.phase == "in_duel" and not frameState.setupDone then
            if not EDS.inHumanMainPhase1() then
                frameState.phase = EDS.turnPlayer() == 1 and "cpu_turn" or "draw_mulligan"
                frameState.timer = 0
                return
            end
            frameState.setupDone = true
            frameState.phase = actor == "CPU" and "watch" or "playing"
            frameState.timer = 0
        end
        if frameState.phase == "playing" then
            frameState.timer = frameState.timer + 1
            if frameState.timer == 1 or frameState.timer % 45 == 0 then
                EDS.forceOwnHandCursor()
            end
            if not frameState.scenarioSetupDone then
                EDS.tickScenarioSetup(opts, frameState)
            end
            if not frameState.scenarioSetupDone then
                if frameState.timer % 120 == 0 then
                    EDS.log(string.format(
                        "playing: waiting scenario setup (hand=%d step=%d).",
                        EDS.playerHandCount(), EDS.duelStep()))
                end
            elseif not frameState.handReady then
                EDS.prepHandStep(frameState)
                if frameState.timer > 1200 then
                    EDS.log("Hand prep failed (still not on hand / menu stuck).")
                    frameState.phase = "done"
                    frameState.failReason = "hand_prep_failed"
                end
            elseif type(opts.play) == "function" then
                if opts.play(frameState) then
                    frameState.playDone = true
                end
            else
                if EDS.runPlaySequence(opts.play, frameState) then
                    frameState.playDone = true
                end
            end
            local tailNeed = opts.recordTail
            if frameState.playDone and tailNeed > 0 then
                if EDS.chainOrEffectBusy() or EDS.cardMenuOpen() then
                    frameState.tailFrames = 0
                    frameState.postPlayStable = 0
                else
                    frameState.postPlayStable = (frameState.postPlayStable or 0) + 1
                    if frameState.postPlayStable >= 45 then
                        frameState.tailFrames = (frameState.tailFrames or 0) + 1
                    end
                end
            end
            if frameState.timer > timeout then
                frameState.phase = "done"
                frameState.failReason = "timeout_play"
            elseif frameState.playDone
                and (tailNeed <= 0 or (frameState.tailFrames or 0) >= tailNeed) then
                frameState.phase = "done"
                if tailNeed > 0 then
                    EDS.log(string.format("Play sequence done; held %d frames for recording tail.", tailNeed))
                else
                    EDS.log("Play sequence done; ending capture (no recording tail).")
                end
            end
        end
        if frameState.phase == "watch" then
            frameState.timer = frameState.timer + 1
            if frameState.timer > (opts.recordFrames or 3600) then
                frameState.phase = "done"
            end
        end
        if frameState.phase == "done" then
            if frameState.captureDir then
                EDS.finalizeMp4Capture(frameState)
            end
            callbacks:remove(cbid)
            callbacks:remove(keysId)
            emu:clearKeys(0xFFFFFFFF)
            local tag = "DONE"
            if frameState.playDone then
                tag = "PASS"
            elseif frameState.failReason then
                tag = "FAIL"
            end
            EDS.appendRunLog(string.format(
                "%s %s duel_main=%s reason=%s play_done=%s",
                tag,
                basename,
                tostring(EDS.inDuelMainPhase()),
                tostring(frameState.failReason or ""),
                tostring(frameState.playDone)
            ))
            if frameState.playDone then
                if frameState.captureDir then
                    EDS.log(string.format("Finished — %s (play sequence + tail; MP4 from ffmpeg if encode succeeded).", tag))
                else
                    EDS.log(string.format("Finished — %s (play sequence + tail). Stop mGBA Record A/V to finalize MP4.", tag))
                end
            elseif frameState.failReason then
                EDS.log(string.format(
                    "Finished — %s: %s (play step %d).",
                    tag, frameState.failReason, frameState.playIndex or 0))
            else
                EDS.log("Finished — " .. tag .. ".")
            end
        end
    end)
end

return EDS
