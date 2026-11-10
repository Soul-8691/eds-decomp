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
EDS.G_CHAIN_LINK_COUNT = EDS.G_CHAIN + 0x3C0
EDS.G_CHAIN_LINKS = EDS.G_CHAIN + 0x280
EDS.G_CHAIN_EFFECT_STEP = EDS.G_CHAIN + 0x3E0 -- gChain.effectStep (chain.h)
EDS.G_CHAIN_TARGET_STEP = EDS.G_CHAIN + 0x3E5 -- chainB step (effect_targets3.c TARGET_STEP)
EDS.G_TEXTBOX = 0x0201AE60
-- EffectTributeOpponentMonsterResolve (effect_resolve8.c) Soul Exchange hand / field tribute picks
EDS.EFFECT_TRIBUTE_OPP_PICK_SECOND = 0x76
EDS.EFFECT_TRIBUTE_OPP_PICK_HAND = 0x77
EDS.EFFECT_TRIBUTE_OPP_SUMMON_DONE = 0x6E
EDS.CARD_ID_SOUL_EXCHANGE = 1000 -- duel RAM id (engine key 1220)
EDS.CARD_ID_SUMMONED_SKULL = 675
EDS.DEFAULT_PLAYER_HAND = 5
-- Mulligan/Draw menu fills to 5, then DuelPhase_Draw calls DrawCards(0, 1) for the normal draw (battle_phase3.c).
EDS.FIRST_TURN_HAND = 6
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
EDS.G_SCENE_WORK = 0x02020310
EDS.G_TURN_ORDER_SCENE = EDS.G_SCENE_WORK + 0xAAC -- gSceneWork.u.turnOrder
-- RPS result at TurnOrderSceneWork.result (RPS_WIN=0, RPS_LOSE=1, RPS_DRAW=2)
EDS.OFF_TURN_ORDER_RESULT = 0x12
EDS.OFF_TURN_ORDER_OPP_ANSWERED = 0x11
EDS.OFF_TURN_ORDER_OPP_HAND = 0x10
EDS.OFF_TURN_ORDER_TURN_CHOICE = 0x13
EDS.OFF_TURN_ORDER_PHASE = 0x49
EDS.OFF_TURN_ORDER_CAROUSEL_STOP = 0x2 -- scrollers[SCROLLER_CAROUSEL].stop
EDS.OFF_TURN_ORDER_CAROUSEL_SPEED = 0x1
EDS.OFF_TURN_ORDER_OPP_SCROLLER_POS = 0x4 -- scrollers[SCROLLER_OPPONENT_CARD].pos
EDS.OFF_TURN_ORDER_OPP_SCROLLER_SPEED = 0x5
EDS.OFF_TURN_ORDER_CAROUSEL_SPREAD = 0x28
EDS.OFF_TURN_ORDER_CAROUSEL_SPREAD_SPEED = 0x29
EDS.G_MAIN_FIRST_PLAYER = EDS.G_MAIN + 0x4870
EDS.RPS_WIN = 0
EDS.RPS_LOSE = 1
EDS.RPS_DRAW = 2
EDS.TURN_CHOICE_SECOND = 1
EDS.TURN_ORDER_CHOOSE_HAND = 0
EDS.TURN_ORDER_SHOW_RESULT = 1
EDS.TURN_ORDER_CHOOSE_TURN = 2
-- Campaign first-duel turn order (lose RPS -> CPU goes first -> player draws second).
EDS.CB_CAMPAIGN_TURN_ORDER = 0x0801BE0D
-- LCG seed that tends to yield RPS lose after typical menu navigation (see opts.rngSeedGoSecond).
EDS.DEFAULT_RNG_SEED_GO_SECOND = 0x6D3A9C17
EDS.MAIN_CALLBACK = EDS.G_MAIN + 0x0410
EDS.MAIN_SEQ_INDEX1 = EDS.G_MAIN + 0x4859

EDS.CARD_NAMES = 0x0822C720
EDS.CARD_NAME_SIZE = 0x40
EDS.CARD_ID_LAST = 2400 -- added cards (e.g. Soul Exchange @ 1220) live above legacy scan cap

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
    -- also in recordings/_console.log (frame number first), to read a run afterwards
    local f = io.open(EDS.recordDir .. "/_console.log", "a")
    if f then
        f:write(string.format("%7d %s\n", emu and emu:currentFrame() or 0, msg))
        f:close()
    end
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

-- The ffmpeg command: "ffmpeg" when it is on mGBA's PATH, else ffmpeg.exe in a folder of the PATH saved in the
-- registry (machine, then user). A PATH changed after the terminal that launched mGBA started is not in mGBA's
-- environment; os.getenv("FFMPEG") overrides both.
local ffmpegCmd
function EDS.ffmpegCommand()
    if ffmpegCmd ~= nil then
        return ffmpegCmd or nil
    end
    ffmpegCmd = false
    local env = os.getenv("FFMPEG")
    if env and env ~= "" then
        ffmpegCmd = '"' .. env .. '"'
        return ffmpegCmd
    end
    local rc = os.execute("ffmpeg -version >nul 2>&1")
    if rc == 0 or rc == true then
        ffmpegCmd = "ffmpeg"
        return ffmpegCmd
    end
    for _, key in ipairs({
        [[HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment]],
        [[HKCU\Environment]],
    }) do
        local p = io.popen('reg query "' .. key .. '" /v Path 2>nul')
        local out = p and p:read("*a") or ""
        if p then
            p:close()
        end
        local value = out:match("Path%s+REG_%S+%s+([^\r\n]+)") or ""
        for dir in value:gmatch("[^;]+") do
            dir = dir:gsub("%%(%w+)%%", function(v) return os.getenv(v) or "" end):gsub("[\\/]+$", "")
            local exe = dir .. "\\ffmpeg.exe"
            local f = io.open(exe, "rb")
            if f then
                f:close()
                ffmpegCmd = '"' .. exe .. '"'
                EDS.log("MP4: ffmpeg found through the registry PATH: " .. exe)
                return ffmpegCmd
            end
        end
    end
    return nil
end

function EDS.ffmpegAvailable()
    return EDS.ffmpegCommand() ~= nil
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
    if opts.steps and not frameState.captureArmed then
        return          -- exact scenario: from the cursor on the card to activate (lib_steps.lua)
    end
    if frameState.cpuOnlyCapture and EDS.turnPlayer() ~= 1 then
        return          -- a CPU scenario with no action of yours: the CPU's turns only
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
        '%s -y -hide_banner -loglevel error -framerate %d -i "%s" %s -pix_fmt yuv420p -c:v libx264 "%s"',
        EDS.ffmpegCommand() or "ffmpeg", fps, pattern, vf, out:gsub("/", "\\"))
    EDS.log(string.format("MP4: encoding %d frames -> %s", frameState.captureIndex, out))
    -- (one more pair of quotes around the whole line: cmd.exe strips the outer pair of a line with several)
    local rc = os.execute('"' .. cmd .. '"')
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
    if holdLeft > 0 and EDS.debugKeys then
        EDS.log(string.format("dbg hold key 0x%03X left %d: game held 0x%04X new 0x%04X emu keys 0x%03X",
            heldKey, holdLeft, emu:read16(EDS.G_MAIN + 4), emu:read16(EDS.G_MAIN + 6), emu:getKeys()))
    end
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
-- Exact scenarios (opts.steps, once they run): the CPU's turn still passes through its Draw, Standby and End
-- Phases (gAiTurnPhases: TurnStart, Draw, Standby, AiRunStep, End, TurnEnd); only AiRunStep (index 3: its Main
-- and Battle Phase) is skipped.
EDS.AI_TURN_PHASE_RUN_STEP = 3
EDS.AI_TURN_PHASE_END = 4
function EDS.freezeOpponent(opts, frameState)
    if opts.freezeOpponent == false then
        emu:write8(EDS.G_MOD_TESTING_FREEZE, 0)
        return
    end
    emu:write8(EDS.G_MOD_TESTING_FREEZE, 1)
    if opts.steps and frameState and frameState.stepsRunning then
        if EDS.turnPlayer() == 1 and emu:read8(EDS.G_AI_STATE) == EDS.AI_TURN_PHASE_RUN_STEP then
            emu:write8(EDS.G_AI_STATE, EDS.AI_TURN_PHASE_END)
            emu:write8(EDS.G_AI_STATE + 1, 0)
        end
        return
    end
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
function EDS.chainEffectStep()
    return emu:read8(EDS.G_CHAIN_EFFECT_STEP)
end

function EDS.chainIsResolving()
    return (emu:read8(EDS.G_CHAIN + 0x3D2) & 0x01) ~= 0
end

function EDS.chainResolvingCardId()
    local n = emu:read16(EDS.G_CHAIN_LINK_COUNT)
    if n == 0 or n > 16 then
        return 0
    end
    local ent = EDS.G_CHAIN_LINKS + (n - 1) * 0x14
    return emu:read16(ent) & 0x7FF
end

function EDS.chainBuilding()
    return (emu:read8(EDS.G_CHAIN + 0x3D0) & 0x01) ~= 0
end

function EDS.chainTargetStep()
    return emu:read8(EDS.G_CHAIN_TARGET_STEP)
end

function EDS.textBoxActive()
    return (emu:read8(EDS.G_TEXTBOX) & 0x01) ~= 0
end

function EDS.textBoxMenuKind()
    return emu:read16(EDS.G_TEXTBOX + 0x04)
end

function EDS.playerBattlePhaseDone(player)
    player = player or 0
    local base = player == 1 and EDS.PLAYER1 or EDS.PLAYER0
    return (emu:read8(base + 0x09) & 0x10) ~= 0
end

-- Card on the chain entry being built or resolved (pending[buildIndex] or last link).
function EDS.chainActivatingCardId()
    local n = emu:read16(EDS.G_CHAIN_LINK_COUNT)
    if n > 0 and n <= 16 then
        local ent = EDS.G_CHAIN_LINKS + (n - 1) * 0x14
        return emu:read16(ent) & 0x7FF
    end
    if EDS.chainBuilding() then
        local bi = emu:read8(EDS.G_CHAIN + 0x3D1)
        local ent = EDS.G_CHAIN + bi * 0x14
        return emu:read16(ent) & 0x7FF
    end
    if EDS.chainIsResolving() then
        return EDS.chainResolvingCardId()
    end
    return 0
end

function EDS.chainActivatingNumTargets()
    local n = emu:read16(EDS.G_CHAIN_LINK_COUNT)
    local ent
    if n > 0 and n <= 16 then
        ent = EDS.G_CHAIN_LINKS + (n - 1) * 0x14
    else
        local bi = emu:read8(EDS.G_CHAIN + 0x3D1)
        ent = EDS.G_CHAIN + bi * 0x14
    end
    return emu:read8(ent + 0x0A) & 0x07
end

-- 0x77 is also generic EFFECT_STEP_10; only Soul Exchange tribute resolve uses these steps here.
function EDS.tributeHandPickActive()
    if not EDS.chainIsResolving() then
        return false
    end
    if EDS.chainResolvingCardId() ~= EDS.CARD_ID_SOUL_EXCHANGE then
        return false
    end
    local s = EDS.chainEffectStep()
    return s == EDS.EFFECT_TRIBUTE_OPP_PICK_HAND or s == EDS.EFFECT_TRIBUTE_OPP_PICK_SECOND
end

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
    local opts = frameState.opts or {}
    if frameState.openingHandRamDone or frameState.openingRamDrew then
        return true
    end
    -- An exact scenario replaced both hands with its own (any size): the opening draw is behind it.
    if opts.steps and frameState.scenarioSetupDone then
        return true
    end
    EDS.syncDrawConfirmed(frameState)
    if frameState.drawConfirmed and EDS.openingHandReady(frameState, opts) then
        return true
    end
    return false
end

-- drawMenuCommit = player chose Draw in the deck menu; drawConfirmed = full first-turn hand in RAM.
function EDS.syncDrawConfirmed(frameState)
    if frameState.drawConfirmed or frameState.openingRamDrew then
        return
    end
    if not frameState.drawMenuCommit then
        return
    end
    local opts = frameState.opts or {}
    if EDS.openingHandReady(frameState, opts) then
        frameState.drawConfirmed = true
        EDS.log(string.format(
            "Opening draw complete (hand=%d deck=%d want=%d).",
            EDS.playerHandCount(), emu:read8(EDS.PLAYER0 + 3),
            EDS.openingHandWant(frameState, opts)))
    end
end

-- Route the free cursor toward your deck pile (DuelCursor_PickAny paths in duel_cursor.c).
function EDS.keyTowardOwnDeck(area)
    if area == EDS.DUEL_AREA_DECK then
        return EDS.KEY_A
    end
    if area == EDS.DUEL_AREA_FUSION_DECK or area == EDS.DUEL_AREA_SPELL_TRAP then
        return EDS.KEY_LEFT
    end
    if area == EDS.DUEL_AREA_HAND then
        return EDS.KEY_UP
    end
    if area == EDS.DUEL_AREA_MONSTER or area == EDS.DUEL_AREA_FIELD then
        return EDS.KEY_DOWN
    end
    return EDS.KEY_UP
end

-- Main Phase: move the free cursor onto your hand[slot] (one D-pad edge per call).
function EDS.keyTowardHandSlot(area, idx, slot)
    slot = slot or 0
    if area == EDS.DUEL_AREA_HAND then
        if idx < slot then
            return EDS.KEY_RIGHT
        end
        if idx > slot then
            return EDS.KEY_LEFT
        end
        return nil
    end
    -- duel_cursor.c PickAny (player 0): Deck --Left--> ST[4] --Down--> Hand[0]
    if area == EDS.DUEL_AREA_SPELL_TRAP then
        return EDS.KEY_DOWN
    end
    if area == EDS.DUEL_AREA_DECK then
        return EDS.KEY_LEFT
    end
    if area == EDS.DUEL_AREA_FUSION_DECK then
        return EDS.KEY_LEFT
    end
    if area == EDS.DUEL_AREA_MONSTER then
        return EDS.KEY_DOWN
    end
    if area == EDS.DUEL_AREA_FIELD or area == EDS.DUEL_AREA_GRAVEYARD
        or area == EDS.DUEL_AREA_BANISHED then
        return EDS.KEY_RIGHT
    end
    return EDS.KEY_DOWN
end

function EDS.syncDrawMenuCommit(frameState, wantHand)
    wantHand = wantHand or EDS.DEFAULT_PLAYER_HAND
    if frameState.drawMenuCommit or not frameState.drawPickedDraw then
        return
    end
    if EDS.playerHandCount() >= wantHand then
        frameState.drawMenuCommit = true
    end
end

-- After opening draw is confirmed, close the deck menu so the duel can leave Draw phase.
function EDS.tickPostDrawMenuClose(frameState)
    if not frameState.drawConfirmed then
        return
    end
    if not EDS.cardMenuOpen() then
        frameState.postDrawMenuClosedStreak = (frameState.postDrawMenuClosedStreak or 0) + 1
        return
    end
    frameState.postDrawMenuClosedStreak = 0
    if not EDS.busy() then
        EDS.press(EDS.KEY_B, 14)
    end
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

function EDS.forceHandCursor(player, slot)
    player = player or 0
    slot = slot or 0
    emu:write32(EDS.G_DUEL_SCREEN_SEL_PLAYER, player)
    emu:write32(EDS.G_DUEL_SCREEN_SEL_AREA, EDS.DUEL_AREA_HAND)
    emu:write32(EDS.G_DUEL_SCREEN_SEL_INDEX, slot)
end

function EDS.forceOpponentMonsterCursor(zone)
    zone = zone or 0
    emu:write32(EDS.G_DUEL_SCREEN_SEL_PLAYER, 1)
    emu:write32(EDS.G_DUEL_SCREEN_SEL_AREA, EDS.DUEL_AREA_MONSTER)
    emu:write32(EDS.G_DUEL_SCREEN_SEL_INDEX, zone)
end

function EDS.duelCursorReady()
    local flags = emu:read16(EDS.G_DUEL_SCREEN_FLAGS)
    return ((flags >> 3) & 1) == 1
end

function EDS.duelSelIndex()
    return emu:read32(EDS.G_DUEL_SCREEN_SEL_INDEX)
end

function EDS.isDeckPileMenu()
    if not EDS.cardMenuOpen() then
        return false
    end
    local cmd = EDS.cardMenuCommand()
    return cmd == EDS.CARDMENU_CMD_SURRENDER or cmd == EDS.CARDMENU_CMD_MULLIGAN
        or cmd == EDS.CARDMENU_CMD_DRAW
end

function EDS.isHandCommandMenu()
    if not EDS.cardMenuOpen() or EDS.duelSelArea() ~= EDS.DUEL_AREA_HAND then
        return false
    end
    return not EDS.isDeckPileMenu()
end

function EDS.dismissDeckPileMenuIfOpen()
    if not EDS.isDeckPileMenu() then
        return false
    end
    if not EDS.busy() then
        EDS.press(EDS.KEY_B, 18)
    end
    return true
end

-- Block A only on deck-pile menus (Surrender/Mulligan/Draw). Hand command menus need A to confirm Activate.
function EDS.safeToPressA()
    local area = EDS.duelSelArea()
    if EDS.cardMenuOpen() then
        if EDS.isDeckPileMenu() then
            return false
        end
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
    if area == EDS.DUEL_AREA_HAND or area == EDS.DUEL_AREA_MONSTER
        or area == EDS.DUEL_AREA_SPELL_TRAP then
        return true
    end
    return false
end

function EDS.handMenuTowardActivate()
    if EDS.dismissDeckPileMenuIfOpen() then
        return false
    end
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

function EDS.onHandSlot(player, slot)
    return EDS.duelSelPlayer() == player
        and EDS.duelSelArea() == EDS.DUEL_AREA_HAND
        and EDS.duelSelIndex() == slot
end

function EDS.handSlotCardId(player, slot)
    local base = player == 1 and EDS.PLAYER1 or EDS.PLAYER0
    local w = emu:read32(base + EDS.OFFSET_HAND + slot * 4)
    return w & 0x7FF
end

function EDS.findPlayerHandSlot(cardId, player)
    player = player or 0
    local base = player == 1 and EDS.PLAYER1 or EDS.PLAYER0
    local n = emu:read8(base + 2)
    for i = 0, n - 1 do
        if EDS.handSlotCardId(player, i) == (cardId & 0x7FF) then
            return i
        end
    end
    return nil
end

function EDS.selHandCardId(player)
    player = player or 0
    if EDS.duelSelPlayer() ~= player or EDS.duelSelArea() ~= EDS.DUEL_AREA_HAND then
        return nil
    end
    return EDS.handSlotCardId(player, EDS.duelSelIndex())
end

function EDS.onHandCard(player, cardId)
    return EDS.selHandCardId(player) == (cardId & 0x7FF)
end

function EDS.onOppMonsterSlot(zone)
    return EDS.duelSelPlayer() == 1
        and EDS.duelSelArea() == EDS.DUEL_AREA_MONSTER
        and EDS.duelSelIndex() == (zone or 0)
end

-- EffectTributeOpponentMonsterChainB: DuelCursor_PickTarget(PICK_PLAYER1(PICK_ANY_MONSTER)).
function EDS.keyTowardOppMonsterSlot(selPlayer, area, idx, zone)
    zone = zone or 0
    if selPlayer == 1 and area == EDS.DUEL_AREA_MONSTER then
        if idx < zone then
            return EDS.KEY_RIGHT
        end
        if idx > zone then
            return EDS.KEY_LEFT
        end
        return nil
    end
    if selPlayer == 0 and area == EDS.DUEL_AREA_MONSTER then
        return EDS.KEY_UP
    end
    if selPlayer == 0 and area == EDS.DUEL_AREA_SPELL_TRAP then
        return EDS.KEY_UP
    end
    if selPlayer == 0 and area == EDS.DUEL_AREA_HAND then
        return EDS.KEY_UP
    end
    if selPlayer == 0 and (area == EDS.DUEL_AREA_DECK or area == EDS.DUEL_AREA_FUSION_DECK) then
        return EDS.KEY_LEFT
    end
    if selPlayer == 1 and area ~= EDS.DUEL_AREA_MONSTER then
        return EDS.KEY_DOWN
    end
    return EDS.KEY_UP
end

-- Real D-pad routing (duel_cursor.c). RAM sel* writes desync from PickAny — use keys callback only.
function EDS.tickHandNavigateKeys(frameState)
    local nav = frameState.handNav
    if not nav or frameState.phase ~= "playing" then
        return
    end
    if EDS.chainOrEffectBusy() and nav.mode == "prep" then
        return
    end
    if EDS.busy() then
        return
    end
    if EDS.isDeckPileMenu() then
        EDS.press(EDS.KEY_B, 16)
        return
    end
    if EDS.cardMenuOpen() and not EDS.isHandCommandMenu() then
        EDS.press(EDS.KEY_B, 16)
        return
    end
    if nav.mode == "open" and EDS.isHandCommandMenu() then
        frameState.handNavDone = true
        frameState.handNav = nil
        EDS.log(string.format("Hand command menu open (slot %d).", nav.slot or 0))
        return
    end
    local slot = nav.slot or 0
    if EDS.duelSelPlayer() ~= 0 then
        EDS.press(EDS.KEY_UP, 16)
        return
    end
    if not EDS.duelCursorReady() then
        return
    end
    local area = EDS.duelSelArea()
    local idx = EDS.duelSelIndex()
    local toward = EDS.keyTowardHandSlot(area, idx, slot)
    if toward then
        EDS.press(toward, 16)
        return
    end
    if area == EDS.DUEL_AREA_HAND and idx == slot then
        if nav.mode == "open" and not EDS.cardMenuOpen() then
            if EDS.safeToPressA() then
                EDS.press(EDS.KEY_A, 20)
            end
        elseif nav.mode == "prep" then
            frameState.handReady = true
            frameState.handNav = nil
            EDS.log(string.format("Hand selected (index %d).", idx))
        end
    end
    if (frameState.timer or 0) % 90 == 0 then
        EDS.log(string.format(
            "hand_nav mode=%s want=%d area=%d idx=%d menu=%s",
            tostring(nav.mode), slot, area, idx, tostring(EDS.cardMenuOpen())))
    end
end

function EDS.prepHandStep(frameState)
    if frameState.handReady then
        return
    end
    if not frameState.handNav then
        frameState.handNav = { slot = 0, mode = "prep" }
        EDS.log("Hand prep: navigating with D-pad (no RAM cursor snap).")
    end
    if frameState.timer % 120 == 0 then
        EDS.log(string.format(
            "hand_prep area=%d index=%d player=%d",
            EDS.duelSelArea(), EDS.duelSelIndex(), EDS.duelSelPlayer()))
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
    else
        EDS.setPlayerSkipDraw(EDS.PLAYER0, false)
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

function EDS.openingHandWant(frameState, opts)
    opts = opts or {}
    if frameState.minPlayerHand then
        return frameState.minPlayerHand
    end
    if opts.ramOpeningDraw == false then
        return EDS.FIRST_TURN_HAND
    end
    return EDS.DEFAULT_PLAYER_HAND
end

function EDS.openingHandReady(frameState, opts)
    return EDS.playerHandCount() >= EDS.openingHandWant(frameState, opts)
end

function EDS.openingMainPending(frameState, opts)
    opts = opts or {}
    if not EDS.openingDrawSatisfied(frameState) then
        return false
    end
    if opts.setupCards and not frameState.scenarioSetupDone then
        return false
    end
    return true
end

-- Apply scenario hand/field after the game draw, during Draw or Standby (before Main Phase / capture).
function EDS.canApplyScenarioSetup(frameState, opts)
    opts = opts or {}
    if EDS.turnPlayer() ~= 0 then
        return false
    end
    EDS.syncDrawConfirmed(frameState)
    local step = EDS.duelStep()
    if step < EDS.DUEL_STEP_DRAW or step > EDS.DUEL_STEP_MAIN then
        return false
    end
    if not EDS.openingHandReady(frameState, opts) then
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
    if opts.steps then
        if not EDS.canApplyExactSetup() then
            return
        end
        EDS.applyExactSetup(opts)
        frameState.scenarioSetupDone = true
        frameState.scenarioCaptureHold = 3
        return
    end
    EDS.defaultSetup(opts)
    frameState.scenarioSetupDone = true
    frameState.scenarioCaptureHold = 3
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
    -- DuelZone+0x06: isDefense bit0, isFaceUp bit1 (face-down attack is invalid).
    if defense then
        emu:write8(z + 6, 0x01) -- face-down Defense Position
    else
        emu:write8(z + 6, 0x02) -- face-up Attack Position
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

function EDS.inCampaignTurnOrder()
    return currentCallback() == EDS.CB_CAMPAIGN_TURN_ORDER
end

-- True when the Destiny Board turn-order scene is actually running (not a zeroed SceneWork stub).
function EDS.turnOrderRpsUiActive()
    if EDS.inCampaignTurnOrder() then
        return true
    end
    local to = EDS.G_TURN_ORDER_SCENE
    local phase = emu:read8(to + EDS.OFF_TURN_ORDER_PHASE)
    if phase > 6 then
        return false
    end
    if emu:read8(to + EDS.OFF_TURN_ORDER_CAROUSEL_SPREAD_SPEED) ~= 0
        or emu:read8(to + EDS.OFF_TURN_ORDER_CAROUSEL_SPREAD) ~= 0 then
        return true
    end
    if emu:read8(to + EDS.OFF_TURN_ORDER_OPP_ANSWERED) == 1 then
        return true
    end
    if phase > 0 then
        return true
    end
    if phase == 0 and emu:read8(to + EDS.OFF_TURN_ORDER_OPP_HAND) == 0xFF then
        return true
    end
    return false
end

-- Turn-order RPS / FIRST-SECOND screen (pre-duel); not the in-duel Draw Phase menu.
function EDS.turnOrderScreenActive(frameState)
    if frameState and frameState.drawConfirmed then
        return false
    end
    if not EDS.turnOrderRpsUiActive() then
        return false
    end
    local step = EDS.duelStep()
    if step >= EDS.DUEL_STEP_TURN_START then
        return false
    end
    if step >= EDS.DUEL_STEP_DRAW then
        return false
    end
    if step >= EDS.DUEL_STEP_OPENING
        and emu:read8(EDS.G_TURN_ORDER_SCENE + EDS.OFF_TURN_ORDER_PHASE) == EDS.TURN_ORDER_CHOOSE_HAND
        and emu:read8(EDS.G_TURN_ORDER_SCENE + EDS.OFF_TURN_ORDER_OPP_ANSWERED) == 0 then
        return false
    end
    return true
end

function EDS.maintainGoSecond(frameState, opts)
    opts = opts or {}
    if opts.goSecond == false then
        return
    end
    if frameState and frameState.drawConfirmed then
        return
    end
    local step = EDS.duelStep()
    if step >= EDS.DUEL_STEP_RESULT then
        return
    end
    local b = emu:read8(EDS.G_MAIN_FIRST_PLAYER)
    if (b & 1) == 0 then
        emu:write8(EDS.G_MAIN_FIRST_PLAYER, b | 0x01)
    end
    EDS.setPlayerSkipDraw(EDS.PLAYER0, false)
    EDS.setPlayerSkipDraw(EDS.PLAYER1, false)
end

-- Jump to RPS lose + opponent card shown (CPU picks first on A); avoids Random() on hand confirm.
function EDS.forceTurnOrderRpsLose(to)
    local stop = emu:read8(to + EDS.OFF_TURN_ORDER_CAROUSEL_STOP)
    if stop > 2 then
        return false
    end
    local opp = (stop + 2) % 3
    emu:write8(to + EDS.OFF_TURN_ORDER_OPP_HAND, opp)
    emu:write8(to + EDS.OFF_TURN_ORDER_OPP_ANSWERED, 1)
    emu:write8(to + EDS.OFF_TURN_ORDER_RESULT, EDS.RPS_LOSE)
    emu:write8(to + EDS.OFF_TURN_ORDER_PHASE, EDS.TURN_ORDER_SHOW_RESULT)
    emu:write8(to + EDS.OFF_TURN_ORDER_OPP_SCROLLER_POS, 0x30)
    emu:write8(to + EDS.OFF_TURN_ORDER_OPP_SCROLLER_SPEED, 0)
    return true
end

function EDS.pinRngSeedForGoSecond(frameState, opts)
    if frameState.turnOrderRngPinned == false then
        return
    end
    local seed = opts.rngSeedGoSecond
    if seed == nil then
        seed = EDS.DEFAULT_RNG_SEED_GO_SECOND
    end
    emu:write32(EDS.G_MAIN, seed)
end

-- Work around RPS-win opening-draw bug: lose RPS so CPU takes first turn (you draw on your turn).
function EDS.tickTurnOrderGoSecond(frameState, opts)
    opts = opts or {}
    if opts.goSecond == false then
        return
    end
    if frameState.turnOrderRngPinned == false then
        return
    end
    local step = EDS.duelStep()
    if step >= EDS.DUEL_STEP_TURN_START and step < EDS.DUEL_STEP_RESULT then
        frameState.turnOrderRngPinned = false
        return
    end
    EDS.pinRngSeedForGoSecond(frameState, opts)
    if not EDS.turnOrderScreenActive(frameState) then
        return
    end
    if EDS.busy() then
        return
    end
    local to = EDS.G_TURN_ORDER_SCENE
    local phase = emu:read8(to + EDS.OFF_TURN_ORDER_PHASE)
    frameState.turnOrderTick = (frameState.turnOrderTick or 0) + 1

    if phase == EDS.TURN_ORDER_CHOOSE_HAND then
        if emu:read8(to + EDS.OFF_TURN_ORDER_CAROUSEL_SPEED) ~= 0 then
            return
        end
        frameState.turnOrderChooseWait = (frameState.turnOrderChooseWait or 0) + 1
        if frameState.turnOrderChooseWait < 8 then
            return
        end
        if not frameState.turnOrderRpsCommitted and EDS.forceTurnOrderRpsLose(to) then
            frameState.turnOrderRpsCommitted = true
            EDS.log("Turn order: forced RPS lose (CPU goes first).")
        end
        return
    end

    local result = emu:read8(to + EDS.OFF_TURN_ORDER_RESULT)
    if result == EDS.RPS_WIN then
        emu:write8(to + EDS.OFF_TURN_ORDER_RESULT, EDS.RPS_LOSE)
        result = EDS.RPS_LOSE
        if phase == EDS.TURN_ORDER_CHOOSE_TURN then
            emu:write8(to + EDS.OFF_TURN_ORDER_PHASE, EDS.TURN_ORDER_SHOW_RESULT)
            phase = EDS.TURN_ORDER_SHOW_RESULT
        end
        EDS.log("Turn order: coerced RPS_WIN -> RPS_LOSE.")
    end

    if phase == EDS.TURN_ORDER_CHOOSE_TURN then
        emu:write8(to + EDS.OFF_TURN_ORDER_TURN_CHOICE, EDS.TURN_CHOICE_SECOND)
        if frameState.turnOrderTick % 18 == 0 then
            EDS.press(EDS.KEY_A, 18)
            EDS.log("Turn order: SECOND + A (won RPS fallback).")
        end
        return
    end

    if phase == EDS.TURN_ORDER_SHOW_RESULT then
        local oppPos = emu:read8(to + EDS.OFF_TURN_ORDER_OPP_SCROLLER_POS)
        if oppPos < 0x30 and frameState.turnOrderRpsCommitted then
            emu:write8(to + EDS.OFF_TURN_ORDER_OPP_SCROLLER_POS, 0x30)
            emu:write8(to + EDS.OFF_TURN_ORDER_OPP_SCROLLER_SPEED, 0)
            oppPos = 0x30
        end
        local answered = emu:read8(to + EDS.OFF_TURN_ORDER_OPP_ANSWERED)
        if answered ~= 1 or oppPos < 0x30 then
            return
        end
        if result == EDS.RPS_LOSE and frameState.turnOrderTick % 18 == 0 then
            EDS.press(EDS.KEY_A, 18)
            EDS.log("Turn order: A (lost RPS; CPU chooses first).")
        elseif result == EDS.RPS_DRAW and frameState.turnOrderTick % 18 == 0 then
            frameState.turnOrderRpsCommitted = false
            frameState.turnOrderChooseWait = 0
            EDS.press(EDS.KEY_A, 18)
            EDS.log("Turn order: A (RPS draw; rematch).")
        end
    end
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
        -- Do not mash A during turn-order RPS (random win breaks the opening draw).
        if not EDS.turnOrderScreenActive(frameState) then
            if frameState.timer % 32 == 1 then
                EDS.press(EDS.KEY_A, 20)
            end
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
                EDS.log(string.format(
                    "draw_mulligan: need opening draw (hand=%d step=%d commit=%s).",
                    EDS.playerHandCount(), step, tostring(frameState.drawMenuCommit)))
            elseif opts.setupCards and not frameState.scenarioSetupDone then
                EDS.syncDrawConfirmed(frameState)
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
        EDS.tickPostDrawMenuClose(frameState)
        if EDS.busy() then
            return
        end
        local wantHand = EDS.openingHandWant(frameState, opts)
        EDS.syncDrawMenuCommit(frameState, wantHand)
        EDS.syncDrawConfirmed(frameState)
        local scenarioOk = not opts.setupCards or frameState.scenarioSetupDone
        if scenarioOk and EDS.openingDrawSatisfied(frameState) then
            if EDS.inHumanMainPhase1() or step >= EDS.DUEL_STEP_STANDBY then
                frameState.phase = "in_duel"
                frameState.timer = 0
                EDS.log(string.format(
                    "Entering in_duel after opening draw + scenario (step=%d).", step))
                return
            elseif frameState.drawConfirmed and not EDS.cardMenuOpen()
                and (frameState.postDrawMenuClosedStreak or 0) >= 45
                and not frameState.openingAwaitMain then
                frameState.openingAwaitMain = true
                frameState.phase = "in_duel"
                frameState.timer = 0
                EDS.log("Entering in_duel (draw menu closed; waiting for Main 1).")
                return
            end
        end
        if step == EDS.DUEL_STEP_DRAW and EDS.turnPlayer() == 0 then
            if frameState.drawConfirmed then
                EDS.tickPostDrawMenuClose(frameState)
            elseif opts.ramOpeningDraw ~= false then
                if EDS.cardMenuOpen() and not EDS.busy() and frameState.timer % 28 == 5 then
                    EDS.press(EDS.KEY_B, 22)
                end
            elseif not frameState.drawConfirmed or EDS.playerHandCount() < wantHand then
                local menuOpen = EDS.cardMenuOpen()
                local cmd = EDS.cardMenuCommand()
                local area = EDS.duelSelArea()
                if not menuOpen then
                    if frameState.timer >= 12 and frameState.timer % 20 == 0 and not EDS.busy() then
                        EDS.press(EDS.keyTowardOwnDeck(area), 26)
                    end
                else
                    if cmd == EDS.CARDMENU_CMD_MULLIGAN and not frameState.drawPickedDraw then
                        EDS.press(EDS.KEY_RIGHT, 28)
                        frameState.drawPickedDraw = true
                    elseif cmd == EDS.CARDMENU_CMD_DRAW then
                        if EDS.playerHandCount() < wantHand and not EDS.busy()
                            and frameState.timer % 22 == 0 then
                            EDS.press(EDS.KEY_A, 28)
                        end
                        EDS.syncDrawMenuCommit(frameState, wantHand)
                    elseif frameState.timer % 35 == 10 then
                        EDS.press(EDS.KEY_RIGHT, 28)
                    end
                end
                EDS.syncDrawMenuCommit(frameState, wantHand)
                EDS.syncDrawConfirmed(frameState)
                if frameState.drawPickedDraw and not menuOpen
                    and EDS.playerHandCount() >= EDS.DEFAULT_PLAYER_HAND
                    and EDS.playerHandCount() < wantHand
                    and not EDS.busy() and frameState.timer % 20 == 0 then
                    EDS.press(EDS.keyTowardOwnDeck(area), 26)
                end
                if frameState.drawConfirmed and EDS.cardMenuOpen()
                    and not EDS.busy() and frameState.timer % 20 == 0 then
                    EDS.press(EDS.KEY_B, 18)
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

-- Soul Exchange chainB: designate opponent's monster (PickTarget, not RAM cursor).
function EDS.tickOppMonsterTargetKeys(frameState)
    if frameState.phase ~= "playing" or frameState.playDone then
        return
    end
    local pick = frameState.pendingOppTarget
    if not pick then
        return
    end
    if EDS.tributeHandPickActive() then
        frameState.pendingOppTarget = nil
        EDS.log("Opponent target assist: resolve hand pick active (target done).")
        return
    end
    if EDS.chainActivatingCardId() == EDS.CARD_ID_SOUL_EXCHANGE
        and EDS.chainActivatingNumTargets() >= 1 then
        frameState.pendingOppTarget = nil
        EDS.log("Opponent target assist: target registered on chain.")
        return
    end
    if EDS.busy() then
        return
    end
    pick.tick = (pick.tick or 0) + 1
    local zone = pick.zone or 0
    if EDS.textBoxActive() then
        if pick.tick % 12 == 0 then
            EDS.press(EDS.KEY_A, 18)
            EDS.log("Opponent target assist: A (dismiss target prompt).")
        end
        return
    end
    if not EDS.duelCursorReady() then
        return
    end
    local selP = EDS.duelSelPlayer()
    local area = EDS.duelSelArea()
    local idx = EDS.duelSelIndex()
    if EDS.onOppMonsterSlot(zone) then
        if pick.tick % 10 == 0 then
            EDS.press(EDS.KEY_A, 18)
            pick.presses = (pick.presses or 0) + 1
            EDS.log(string.format(
                "Opponent target assist: A on opp M%d (press %d).",
                zone, pick.presses))
        end
        if (pick.presses or 0) >= 12 and pick.tick > 200 then
            frameState.pendingOppTarget = nil
            EDS.log("Opponent target assist: timeout (still on target?).")
        end
        return
    end
    local toward = EDS.keyTowardOppMonsterSlot(selP, area, idx, zone)
    if toward and pick.tick % 2 == 0 then
        EDS.press(toward, 16)
    end
    if pick.tick % 90 == 0 then
        EDS.log(string.format(
            "opp_target want=M%d sel p=%d a=%d i=%d targets=%d chainB=%d",
            zone, selP, area, idx, EDS.chainActivatingNumTargets(), EDS.chainTargetStep()))
    end
    if pick.tick > 900 then
        frameState.pendingOppTarget = nil
        EDS.log("Opponent target assist: navigation timeout.")
    end
end

function EDS.keyTowardOwnMonsterSlot(selPlayer, area, idx, zone)
    zone = zone or 0
    if selPlayer == 0 and area == EDS.DUEL_AREA_MONSTER then
        if idx < zone then
            return EDS.KEY_RIGHT
        end
        if idx > zone then
            return EDS.KEY_LEFT
        end
        return nil
    end
    if selPlayer == 0 and area == EDS.DUEL_AREA_SPELL_TRAP then
        return EDS.KEY_UP
    end
    if selPlayer == 0 and area == EDS.DUEL_AREA_HAND then
        return EDS.KEY_UP
    end
    if selPlayer == 0 and (area == EDS.DUEL_AREA_DECK or area == EDS.DUEL_AREA_FUSION_DECK) then
        return EDS.KEY_LEFT
    end
    if selPlayer ~= 0 then
        return EDS.KEY_DOWN
    end
    return EDS.KEY_DOWN
end

-- Soul Exchange resolve: text box "Select a high-level monster", then DuelCursor_PickTarget(PICK_HAND)
-- at effectStep 0x77 (no RAM cursor — PickTarget only).
function EDS.tickHandMonsterPickKeys(frameState)
    if frameState.phase ~= "playing" or frameState.playDone then
        return
    end
    local romPick = EDS.tributeHandPickActive()
    local pick = frameState.pendingHandPick
    if romPick and not pick then
        pick = {
            slot = 1,
            cardId = EDS.CARD_ID_SUMMONED_SKULL,
            secondZone = 0,
            maxPresses = 64,
            fromRom = true,
        }
        frameState.pendingHandPick = pick
        frameState.handPickTick = 0
        frameState.handPickPresses = 0
        frameState.handPickSawRom = true
        EDS.log(string.format("Hand pick assist: ROM effectStep=0x%02X.", EDS.chainEffectStep()))
    end
    if not pick and not romPick then
        return
    end
    if not pick then
        return
    end
    if EDS.busy() then
        return
    end
    frameState.handPickTick = (frameState.handPickTick or 0) + 1
    local step = EDS.chainEffectStep()
    if frameState.handPickSawRom and not romPick then
        if (frameState.handPickPresses or 0) > 0
            or step == EDS.EFFECT_TRIBUTE_OPP_SUMMON_DONE then
            frameState.pendingHandPick = nil
            EDS.log(string.format("Hand pick assist: done (effectStep=0x%02X).", step))
            return
        end
    end
    local wantId = pick.cardId
    local wantSlot = pick.slot or 1
    if wantId then
        local found = EDS.findPlayerHandSlot(wantId, 0)
        if found ~= nil then
            wantSlot = found
            if pick.resolvedSlot ~= found then
                pick.resolvedSlot = found
                EDS.log(string.format(
                    "Hand pick assist: cardId %d at hand[%d] (handCount=%d).",
                    wantId, found, EDS.playerHandCount()))
            end
        end
    end
    local wantZone = pick.secondZone or 0

    if EDS.textBoxActive()
        and (step == EDS.EFFECT_TRIBUTE_OPP_PICK_HAND
            or step == EDS.EFFECT_TRIBUTE_OPP_PICK_SECOND) then
        if frameState.handPickTick % 12 == 0 then
            EDS.press(EDS.KEY_A, 18)
            EDS.log(string.format(
                "Hand pick assist: A (dismiss prompt, effectStep=0x%02X).", step))
        end
        return
    end
    if not EDS.duelCursorReady() then
        return
    end

    local selP = EDS.duelSelPlayer()
    local area = EDS.duelSelArea()
    local idx = EDS.duelSelIndex()

    if step == EDS.EFFECT_TRIBUTE_OPP_PICK_SECOND then
        if selP == 0 and area == EDS.DUEL_AREA_MONSTER and idx == wantZone then
            if frameState.handPickTick % 10 == 0 then
                EDS.press(EDS.KEY_A, 18)
                frameState.handPickPresses = (frameState.handPickPresses or 0) + 1
                EDS.log(string.format(
                    "Hand pick assist: A second tribute M%d (press %d).",
                    wantZone, frameState.handPickPresses))
            end
            return
        end
        local toward = EDS.keyTowardOwnMonsterSlot(selP, area, idx, wantZone)
        if toward and frameState.handPickTick % 2 == 0 then
            EDS.press(toward, 16)
        end
        return
    end

    if step == EDS.EFFECT_TRIBUTE_OPP_PICK_HAND then
        local onTarget = wantId and EDS.onHandCard(0, wantId) or EDS.onHandSlot(0, wantSlot)
        if onTarget then
            if frameState.handPickTick % 10 == 0 then
                EDS.press(EDS.KEY_A, 18)
                frameState.handPickPresses = (frameState.handPickPresses or 0) + 1
                EDS.log(string.format(
                    "Hand pick assist: A on hand[%d] id=%s (effectStep=0x%02X press %d).",
                    EDS.duelSelIndex(), wantId and tostring(wantId) or "?",
                    step, frameState.handPickPresses))
            end
        else
            if selP ~= 0 then
                if frameState.handPickTick % 2 == 0 then
                    EDS.press(EDS.KEY_UP, 16)
                end
            else
                local toward = EDS.keyTowardHandSlot(area, idx, wantSlot)
                if toward and frameState.handPickTick % 2 == 0 then
                    EDS.press(toward, 16)
                end
            end
            if frameState.handPickTick % 90 == 0 then
                EDS.log(string.format(
                    "hand_pick wantSlot=%d wantId=%s sel p=%d a=%d i=%d selId=%s",
                    wantSlot, wantId and tostring(wantId) or "nil",
                    selP, area, idx, tostring(EDS.selHandCardId(0))))
            end
        end
    elseif romPick and frameState.handPickTick % 10 == 0 then
        EDS.press(EDS.KEY_A, 18)
        frameState.handPickPresses = (frameState.handPickPresses or 0) + 1
        EDS.log(string.format(
            "Hand pick assist: A fallback (effectStep=0x%02X press %d).",
            step, frameState.handPickPresses))
    end

    if not pick.fromRom and not romPick and (frameState.handPickPresses or 0) >= (pick.maxPresses or 8) then
        frameState.pendingHandPick = nil
        EDS.log("Hand pick assist: play-step timeout (effect step never 0x77?).")
    end
    if (frameState.handPickPresses or 0) >= (pick.maxPresses or 64) then
        frameState.pendingHandPick = nil
        EDS.log("Hand pick assist: max presses.")
    end
end

-- After tribute summon: display position (Attack), then B → end Main (no Battle Phase).
function EDS.tickSoulExchangeFinishKeys(frameState)
    if frameState.phase ~= "playing" or frameState.playDone then
        return
    end
    if not frameState.pendingPostSoulExchange then
        return
    end
    if EDS.busy() then
        return
    end
    local tick = (frameState.postSoulExchangeTick or 0) + 1
    frameState.postSoulExchangeTick = tick
    local phase = frameState.postSoulExchangePhase or "prompts"

    if phase == "prompts" then
        if EDS.textBoxActive() then
            if tick % 12 == 0 then
                EDS.press(EDS.KEY_A, 18)
                EDS.log("Soul Exchange finish: A (text box / summon position).")
            end
            return
        end
        if EDS.cardMenuOpen() then
            if tick % 12 == 0 then
                EDS.press(EDS.KEY_B, 16)
                EDS.log("Soul Exchange finish: B (close card menu).")
            end
            return
        end
        if EDS.chainOrEffectBusy() then
            return
        end
        frameState.postSoulExchangePhase = "end_main"
        frameState.postSoulExchangeTick = 0
        EDS.log("Soul Exchange finish: prompts done; ending Main Phase.")
        return
    end

    if phase == "end_main" then
        if EDS.textBoxActive() then
            frameState.postSoulExchangePhase = "end_confirm"
            frameState.postSoulExchangeTick = 0
            return
        end
        if EDS.chainOrEffectBusy() then
            return
        end
        if EDS.cardMenuOpen() then
            if tick % 12 == 0 then
                EDS.press(EDS.KEY_B, 16)
                EDS.log("Soul Exchange finish: B (close menu before end phase).")
            end
            return
        end
        if not frameState.postEndMainBPushed and tick % 20 == 0 then
            EDS.press(EDS.KEY_B, 18)
            frameState.postEndMainBPushed = true
            EDS.log("Soul Exchange finish: B (request end Main Phase).")
        end
        return
    end

    if phase == "end_confirm" then
        if EDS.textBoxActive() then
            local kind = EDS.textBoxMenuKind()
            if kind == 5 and not frameState.postPhaseMenuPickedEndTurn then
                if tick % 18 == 0 then
                    EDS.press(EDS.KEY_DOWN, 16)
                    frameState.postPhaseMenuPickedEndTurn = true
                    EDS.log("Soul Exchange finish: Down (select End Turn, skip Battle Phase).")
                end
            elseif tick % 12 == 0 then
                EDS.press(EDS.KEY_A, 18)
                EDS.log(string.format(
                    "Soul Exchange finish: A (end turn; menuKind=%d battlePhaseDone=%s).",
                    kind, tostring(EDS.playerBattlePhaseDone(0))))
            end
            if tick > 180 and frameState.postEndMainConfirmed then
                frameState.pendingPostSoulExchange = nil
                frameState.postSoulExchangePhase = nil
                EDS.log("Soul Exchange finish: done (battle phase skipped).")
            end
            return
        end
        frameState.postEndMainConfirmed = true
        if not EDS.chainOrEffectBusy() and tick > 20 then
            frameState.pendingPostSoulExchange = nil
            frameState.postSoulExchangePhase = nil
            EDS.log("Soul Exchange finish: Main Phase ended.")
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
    local item = seq[step]
    if item.handOpen then
        local slot = item.handOpen.slot or 0
        frameState.playHandSlot = slot
        if frameState.playHandOpenStep ~= step then
            frameState.playHandOpenStep = step
            frameState.handNavDone = false
            frameState.handNav = { slot = slot, mode = "open" }
            EDS.log(string.format("Play: navigate to hand[%d] and open menu.", slot))
        end
        if frameState.handNavDone or EDS.isHandCommandMenu() then
            frameState.handNav = nil
            frameState.playIndex = step + 1
        elseif (frameState.handOpenWait or 0) > 900 then
            EDS.log("Play: handOpen timed out.")
            frameState.handNav = nil
            frameState.playIndex = step + 1
        else
            frameState.handOpenWait = (frameState.handOpenWait or 0) + 1
        end
        return false
    end
    if item.oppTarget then
        if frameState.playOppTargetStep ~= step then
            frameState.playOppTargetStep = step
            frameState.pendingOppTarget = {
                zone = item.oppTarget.zone or 0,
                tick = 0,
                presses = 0,
            }
            EDS.log(string.format(
                "Play: opponent monster target assist (zone %d).",
                item.oppTarget.zone or 0))
        end
        if not frameState.pendingOppTarget then
            frameState.playIndex = step + 1
        end
        return false
    end
    if item.handPick then
        if frameState.pendingOppTarget then
            return false
        end
        if frameState.playHandPickStep ~= step then
            frameState.playHandPickStep = step
            frameState.pendingHandPick = item.handPick
            frameState.handPickTick = 0
            frameState.handPickPresses = 0
            frameState.handPickSawRom = true
            EDS.log("Play: waiting for hand monster pick (assist).")
        end
        if not frameState.pendingHandPick then
            frameState.playIndex = step + 1
        end
        return false
    end
    if item.soulExchangeFinish then
        if frameState.playSoulExchangeFinishDone then
            frameState.playIndex = step + 1
            return false
        end
        if frameState.playSoulExchangeFinishStep ~= step then
            frameState.playSoulExchangeFinishStep = step
            frameState.pendingPostSoulExchange = true
            frameState.postSoulExchangePhase = "prompts"
            frameState.postSoulExchangeTick = 0
            frameState.postEndMainBPushed = nil
            frameState.postPhaseMenuPickedEndTurn = nil
            frameState.postEndMainConfirmed = nil
            EDS.log("Play: Soul Exchange finish (summon position + skip Battle Phase).")
        end
        if not frameState.pendingPostSoulExchange then
            frameState.playSoulExchangeFinishDone = true
            frameState.playIndex = step + 1
            EDS.log("Play: Soul Exchange finish step complete.")
        end
        return false
    end
    if EDS.busy() then
        return false
    end
    if item.wait then
        frameState.playWait = (frameState.playWait or 0) + 1
        if frameState.playWait >= item.wait then
            frameState.playWait = 0
            frameState.playIndex = step + 1
        end
        return false
    end
    if item.cursor then
        if item.cursor == "opp_monster" then
            EDS.forceOpponentMonsterCursor(item.zone or 0)
        elseif item.cursor == "own_hand" or item.cursor == "own_hand_menu" then
            local slot = item.slot or 0
            frameState.playHandSlot = slot
            frameState.handNav = { slot = slot, mode = "prep" }
            frameState.handReady = false
        end
        EDS.log(string.format("Play step %d/%d (cursor %s).", step, #seq, item.cursor))
        frameState.playIndex = step + 1
        return false
    end
    if item.key then
        if item.key == EDS.KEY_A then
            if EDS.dismissDeckPileMenuIfOpen() then
                return false
            end
            local inEffect = EDS.chainOrEffectBusy()
            if not inEffect and not EDS.cardMenuOpen() then
                local slot = frameState.playHandSlot or 0
                if not EDS.onHandSlot(0, slot) then
                    frameState.handNav = { slot = slot, mode = "prep" }
                    return false
                end
            end
            if not inEffect and EDS.cardMenuOpen() and EDS.duelSelArea() == EDS.DUEL_AREA_HAND then
                if not EDS.handMenuTowardActivate() then
                    return false
                end
            elseif not inEffect and not EDS.safeToPressA() then
                if (frameState.timer or 0) % 90 == 0 then
                    EDS.log(string.format(
                        "Play A blocked (area=%d menu=%s chain=%s).",
                        EDS.duelSelArea(), tostring(EDS.cardMenuOpen()), tostring(inEffect)))
                end
                return false
            elseif inEffect and EDS.cardMenuOpen() then
                local cmd = EDS.cardMenuCommand()
                if cmd == EDS.CARDMENU_CMD_SURRENDER or cmd == EDS.CARDMENU_CMD_MULLIGAN
                    or cmd == EDS.CARDMENU_CMD_DRAW then
                    return false
                end
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
    if opts.goSecond == nil then
        opts.goSecond = true
    end
    if opts.recordTail == nil then
        opts.recordTail = 0
    end
    if opts.steps and opts.play == nil then
        -- exact scenario: lib_steps.lua plays opts.steps (the expectations are not checked)
        opts.play = function(fs)
            return EDS.tickScenarioSteps(fs, opts)
        end
        opts.playSettleFrames = opts.playSettleFrames or 0     -- (the steps end on an idle duel already)
        if opts.maxFrames == nil then
            opts.maxFrames = 60 * 60 * 15
        end
    end
    local timeout = opts.maxFrames or EDS.TIMEOUT_FRAMES
    local basename = EDS.recordingBasename(opts.card or "Card", opts.effect or "effect", actor)
    EDS.appendRunLog("START " .. basename)
    if opts.pocRequired and actor ~= "CPU" and (not opts.setupCards or not opts.play) then
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
        minPlayerHand = opts.minPlayerHand,
        opts = opts,
        turnOrderRngPinned = opts.goSecond ~= false,
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
        local duelStep = EDS.duelStep()
        if EDS.turnPlayer() ~= 0 then
            return
        end
        if frameState.drawConfirmed then
            if EDS.cardMenuOpen() then
                emu:addKeys(EDS.KEY_B)
            end
            return
        end
        if duelStep ~= EDS.DUEL_STEP_DRAW then
            return
        end
        local wantHand = EDS.openingHandWant(frameState, opts)
        if EDS.cardMenuOpen() then
            local cmd = EDS.cardMenuCommand()
            if cmd == EDS.CARDMENU_CMD_MULLIGAN and not frameState.drawPickedDraw then
                emu:addKeys(EDS.KEY_RIGHT)
            elseif cmd == EDS.CARDMENU_CMD_DRAW and EDS.playerHandCount() < wantHand then
                emu:addKeys(EDS.KEY_A)
            end
            return
        end
        if frameState.drawPickedDraw and EDS.playerHandCount() >= EDS.DEFAULT_PLAYER_HAND
            and EDS.playerHandCount() < wantHand then
            emu:addKeys(EDS.keyTowardOwnDeck(EDS.duelSelArea()))
            return
        end
        if frameState.timer < 6 then
            return
        end
        emu:addKeys(EDS.keyTowardOwnDeck(EDS.duelSelArea()))
    end)
    cbid = callbacks:add("frame", function()
        EDS.tickKeys()
        if opts.goSecond ~= false and not frameState.drawConfirmed then
            EDS.maintainGoSecond(frameState, opts)
        end
        if EDS.inTestDuelFlow(frameState) then
            EDS.freezeOpponent(opts, frameState)
        end
        EDS.ensureUsageDecks(opts, frameState)
        if EDS.inTestDuelFlow(frameState) then
            EDS.tickPlayerOpeningDraw(frameState, opts)
        end
        EDS.tickScenarioSetup(opts, frameState)
        EDS.captureFrameIfNeeded(frameState, opts)
        if frameState.phase ~= "in_duel" and frameState.phase ~= "playing" and frameState.phase ~= "watch" then
            EDS.tickTurnOrderGoSecond(frameState, opts)
            EDS.navigateTestDuel(frameState, opts)
        end
        if frameState.phase == "in_duel" and not frameState.setupDone then
            frameState.timer = (frameState.timer or 0) + 1
            if not EDS.inHumanMainPhase1() then
                if EDS.openingMainPending(frameState, opts) then
                    EDS.tickPostDrawMenuClose(frameState)
                    if frameState.timer % 300 == 0 then
                        EDS.log(string.format(
                            "Waiting for Main Phase 1 (duel_step=%d menu=%s).",
                            EDS.duelStep(), tostring(EDS.cardMenuOpen())))
                    end
                    return
                end
                frameState.phase = EDS.turnPlayer() == 1 and "cpu_turn" or "draw_mulligan"
                frameState.timer = 0
                return
            end
            if opts.setupCards and not frameState.scenarioSetupDone then
                return
            end
            frameState.setupDone = true
            frameState.phase = (actor == "CPU" and not opts.steps) and "watch" or "playing"
            frameState.timer = 0
            frameState.playStartedAt = 0
            frameState.handReady = opts.steps ~= nil    -- (steps move the cursor themselves)
            frameState.stepsRunning = opts.steps ~= nil
            if type(opts.play) == "table" then
                local first = opts.play[frameState.playIndex or 1]
                if first and first.handOpen then
                    local slot = first.handOpen.slot or 0
                    frameState.handReady = true
                    frameState.handNav = { slot = slot, mode = "open" }
                    frameState.playHandSlot = slot
                    EDS.log(string.format(
                        "Main Phase: navigating to hand[%d] for Soul Exchange.", slot))
                end
            end
        end
        if frameState.phase == "playing" then
            frameState.timer = frameState.timer + 1
            if not opts.steps then      -- (the POC assists; exact scenarios press every key in lib_steps.lua)
                EDS.tickOppMonsterTargetKeys(frameState)
                EDS.tickHandMonsterPickKeys(frameState)
                EDS.tickSoulExchangeFinishKeys(frameState)
            end
            if frameState.playStartedAt == 0 then
                frameState.playStartedAt = frameState.timer
            end
            if opts.steps then
                -- (no hand prep or menu clean-up: the steps press every key)
            elseif frameState.timer <= 8 and not frameState.handNav
                and (EDS.isDeckPileMenu()
                    or (EDS.cardMenuOpen() and not EDS.isHandCommandMenu())) then
                if frameState.timer % 2 == 0 and not EDS.busy() then
                    EDS.press(EDS.KEY_B, 16)
                end
            elseif not frameState.handNav and not EDS.chainOrEffectBusy()
                and frameState.timer % 90 == 0 then
                local area = EDS.duelSelArea()
                if area == EDS.DUEL_AREA_DECK or area == EDS.DUEL_AREA_FUSION_DECK then
                    EDS.prepHandStep(frameState)
                end
            end
            if not opts.steps then
                EDS.tickHandNavigateKeys(frameState)
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
            elseif not frameState.handReady and not frameState.handNav then
                EDS.prepHandStep(frameState)
                if frameState.timer > 1200 then
                    EDS.log("Hand prep failed (still not on hand / menu stuck).")
                    frameState.phase = "done"
                    frameState.failReason = "hand_prep_failed"
                end
            elseif type(opts.play) == "function" then
                if opts.play(frameState) then
                    frameState.playSequenceDone = true
                end
            else
                if not frameState.playSequenceDone and EDS.runPlaySequence(opts.play, frameState) then
                    frameState.playSequenceDone = true
                end
            end
            if frameState.playSequenceDone and not frameState.playDone then
                if EDS.chainOrEffectBusy() or EDS.cardMenuOpen() then
                    frameState.playSettleStable = 0
                else
                    frameState.playSettleStable = (frameState.playSettleStable or 0) + 1
                end
                local settleNeed = opts.playSettleFrames
                if settleNeed == nil then
                    settleNeed = 20
                end
                if frameState.endNow then
                    settleNeed = 0          -- (lib_steps.lua found the duel idle after the effect)
                end
                if (frameState.playSettleStable or 0) >= settleNeed then
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

-- Exact scenarios: the starting state and the step runner (lib_steps.lua).
do
    local ok, install = pcall(dofile, scenariosDir .. "/lib_steps.lua")
    if ok and type(install) == "function" then
        install(EDS)
    else
        EDS.log("Could not load lib_steps.lua: " .. tostring(install))
    end
end

return EDS
