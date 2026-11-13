--[[
Exact-scenario support for lib_eds.lua (tools/testing_scenarios/exact.py; README "Exact scenarios"):

  EDS.applyExactSetup(opts)       the whole starting state in RAM: both sides cleared, then the hands, every zone
                                  (with its position flags), the Deck top, Graveyard, banished cards, Fusion Deck,
                                  LP and gDuel.turnCount; for a scenario that starts on the CPU's turn, the CPU's
                                  Draw Phase is skipped (its hand stays as set) and the turn count is one less, the
                                  runner then ending your turn first.
  EDS.tickScenarioSteps(fs, opts) plays opts.steps with real key presses, one step after another (one call per
                                  frame; 1 when every step is done). The expectations are not checked.

Each step becomes a list of small operations (ops). An op waits for the screen it needs (the free or target cursor,
a card menu, a Yes/No box, a menu, the card list, the zone picker), presses one key at a time with the usual gap, and
reads the game's state back (gDuelScreen selection, gDuel.cardMenu, gTextBox, gCardListView, gModLink) to choose the
next key. The cursor is never moved by writing RAM: DuelCursor_Select also moves the sprite and scrolls the field.
]]

return function(EDS)

local G_CARD_LIST = 0x0201D810           -- gCardListView (card_list_view.h)
local G_MOD_LINK = 0x0202D700            -- gModLink (src/mod_link.c): the summon zone picker
local G_BATTLE = 0x02018450              -- gBattle (battle.h)
local G_SUMMON_ACTION = 0x0201CF90       -- gSummonAction (summon.h): +0x0E bit 0 = a summon is pending
local G_DUEL_TURN_COUNT = EDS.G_DUEL + 0x1B10
local G_DUEL_BATTLE_WORD = EDS.G_DUEL + 0x1B14

local CMD = {
    card_view = 0, def_pos = 1, atk_pos = 2, flip = 3, set = 4, summon = 5, activate = 6, attack = 7,
    draw = 8, surrender = 9, fusion = 10, sp_summon = 11, sp_summon_set = 12, mulligan = 13,
}
local TEXTBOX_MESSAGE, TEXTBOX_YES_NO, TEXTBOX_TWO_CHOICE, TEXTBOX_CUSTOM = 0, 1, 2, 5
-- gSaveData.options (0x02011C20 + 0x2152, include/save.h): the Settings bits the `options` step sets
local SAVE_OPTIONS = 0x02013D72
local OPTION_BITS = {
    speed = { mask = 0x0C, values = { default = 0x00, always = 0x04, super = 0x08 } },
    anims = { mask = 0x10, values = { show = 0x00, hide = 0x10 } },
    battle = { mask = 0x20, values = { show = 0x00, hide = 0x20 } },
}
local HOLD_KEYS = { R = 0x100, L = 0x200, B = 0x02 }
local MAIN_STEP_FIELD, MAIN_STEP_BATTLE = 1, 21
local BATTLE_STAGE_SELECT_ATTACKER, BATTLE_STAGE_SELECT_TARGET = 1, 2
local LIST_BUTTON_DECIDE = 3
local GAP = 16

local TYPES = {
    dragon = 1, zombie = 2, fiend = 3, pyro = 4, ["sea serpent"] = 5, rock = 6, machine = 7, fish = 8,
    dinosaur = 9, insect = 10, beast = 11, ["beast-warrior"] = 12, plant = 13, aqua = 14, warrior = 15,
    ["winged beast"] = 16, fairy = 17, spellcaster = 18, thunder = 19, reptile = 20,
}
-- The lines of the menus the scenarios choose from (src/added_effects.c), and Vampire Lord's labels.
local CHOICES = {
    ["gain 1200 lp"] = 0, ["inflict 800 damage"] = 1,                 -- Poison of the Old Man
    ["change battle position"] = 0, ["take control"] = 1,             -- Enemy Controller
    atk = 0, def = 1,                                                 -- Mystik Wok
    monster = 0, spell = 1, trap = 2,                                 -- Vampire Lord
}

-- ---- reading the game ----

local idCache = {}
local function cardId(name)
    if idCache[name] == nil then
        idCache[name] = EDS.findCardId(name) or false
        if not idCache[name] then
            EDS.log("steps: unknown card " .. tostring(name))
        end
    end
    return idCache[name] or nil
end

local function playerBase(p)
    return p == 1 and EDS.PLAYER1 or EDS.PLAYER0
end

local function zoneAddr(p, z)
    return playerBase(p) + EDS.ZONE_MON0 + z * EDS.ZONE_SIZE
end

local function zoneCardId(p, z)
    return emu:read32(zoneAddr(p, z)) & 0x7FF
end

local function textBox()
    return {
        active = (emu:read8(EDS.G_TEXTBOX) & 1) ~= 0,
        kind = emu:read16(EDS.G_TEXTBOX + 0x04),
        yesNo = emu:read16(EDS.G_TEXTBOX + 0x12),
        result = emu:read16(EDS.G_TEXTBOX + 0x14),
        step = emu:read8(EDS.G_TEXTBOX + 0x20),
    }
end

-- The box takes input (slid in, step 1 of TextBoxUpdate).
local function boxInput(kind)
    local b = textBox()
    return b.active and b.step == 1 and (kind == nil or b.kind == kind), b
end

local function listView()
    local b0 = emu:read8(G_CARD_LIST)
    local b5 = emu:read8(G_CARD_LIST + 5)
    local b8 = emu:read8(G_CARD_LIST + 8)
    return {
        active = (b0 & 1) ~= 0,
        step = emu:read8(G_CARD_LIST + 1),
        state = emu:read8(G_CARD_LIST + 2),
        row = b5 & 3,
        moving = ((b5 >> 2) & 7) ~= 0,
        top = emu:read16(G_CARD_LIST + 6),
        button = b8 & 3,
        buttons = (b8 >> 2) & 0xF,
        count = emu:read16(G_CARD_LIST + 0x30C),
    }
end

local function listCardId(i)
    return emu:read32(G_CARD_LIST + 0x0C + i * 4) & 0x7FF
end

local function listReady()
    local v = listView()
    return v.active and v.step == 1 and v.state == 0 and not v.moving, v
end

local function zonePicker()
    return {
        picking = emu:read8(G_MOD_LINK + 14) ~= 0,
        index = emu:read8(G_MOD_LINK + 15),
        count = emu:read8(G_MOD_LINK + 16),
        cand = function(k) return emu:read8(G_MOD_LINK + 17 + k) end,
    }
end

local function battle()
    local w = emu:read32(G_DUEL_BATTLE_WORD)
    local b = emu:read16(G_BATTLE)
    return {
        stage = (w >> 9) & 0xFF,
        step = (w >> 17) & 0xFF,
        attacker = b & 1,
        declared = (b & 0x08) ~= 0,
        atkSlot = (b >> 6) & 7,
    }
end

local function sel()
    return EDS.duelSelPlayer(), EDS.duelSelArea(), EDS.duelSelIndex()
end

local function summonActive()
    return (emu:read8(G_SUMMON_ACTION + 0x0E) & 1) ~= 0
end

-- The card a summon is pending for (gSummonAction: +0x0E bit 0 active; cardId bits 31-46), or 0.
local function summoningCardId()
    if not summonActive() then
        return 0
    end
    return ((emu:read32(G_SUMMON_ACTION) >> 31) | ((emu:read16(G_SUMMON_ACTION + 4) & 0x7FFF) << 1)) & 0x7FF
end

-- gDuelCmd (duel_cmd.h): queued commands (+0x808) or one running (+0x80D bit 5): animations still playing.
local G_DUEL_CMD = 0x020185C0
local function duelCommandsRunning()
    return emu:read16(G_DUEL_CMD + 0x808) ~= 0 or (emu:read8(G_DUEL_CMD + 0x80D) & 0x20) ~= 0
end

local inMainField                    -- (defined with the phase ops below)

-- An effect or a summon is running (its picks are target picks, not card menus).
local function effectRunning()
    return EDS.chainOrEffectBusy() or summonActive()
end

-- An effect that may be asking for a card now (a summon step's "effect pick": Soul Exchange, Metamorphosis): the chain
-- build's cost and target steps (gChain +0x3D0: bit 0 building, bits 1-7 the step; BUILD_RUN_CHAIN_B is 5), a link
-- resolving, or a summon under way. Not effectRunning: with the chain list shown (README media), the build byte stays
-- at BUILD_WAIT_LIST (9) after the chain is done, which made the next summon or Set look like an effect pick.
local function effectAsksForCard()
    local build = emu:read8(EDS.G_CHAIN + 0x3D0)
    if (build & 1) ~= 0 and (build >> 1) <= 5 then
        return true
    end
    if (emu:read8(EDS.G_CHAIN + 0x491) & 0x40) ~= 0 or (emu:read8(EDS.G_CHAIN + 0x3D2) & 1) ~= 0 then
        return true
    end
    return summonActive()
end

-- What changes when a press is taken: effect and target steps, picks so far, boxes, lists, menus, summons, battle.
local function progressSignature()
    local b = textBox()
    return table.concat({
        EDS.chainEffectStep(), EDS.chainTargetStep(), EDS.chainActivatingNumTargets(),
        tostring(b.active), b.step, tostring(listView().active), tostring(EDS.cardMenuOpen()),
        emu:read8(G_SUMMON_ACTION + 0x0E), emu:read8(G_MOD_LINK + 14), emu:read32(G_DUEL_BATTLE_WORD),
        emu:read16(EDS.G_CHAIN_LINK_COUNT), EDS.duelPhaseStep(),
    }, ",")
end

-- After an A that should move the game on: true once something changed; else A again after `wait` frames, at
-- most `tries` presses in all (the first one can come before the pick or menu listens).
local function pressTaken(op, wait, tries)
    if progressSignature() ~= op.sig then
        return true
    end
    op.since = (op.since or 0) + 1
    if op.since >= (wait or 25) then
        if (op.tries or 1) >= (tries or 3) then
            return true
        end
        op.tries = (op.tries or 1) + 1
        op.since = 0
        EDS.log("steps: " .. tostring(op.text) .. ": A again (not taken)")
        -- a longer hold: right after a box closes the game can skip reading the keys for a few frames
        local hold = EDS.PRESS_FRAMES
        EDS.PRESS_FRAMES = 8
        EDS.press(EDS.KEY_A, 16)        -- (EDS.press: the local press() is defined further down)
        EDS.PRESS_FRAMES = hold
    end
    return false
end

-- ---- places ----

-- "you.hand[Card Destruction]", "opp.m0", "you.gy[Sangan]", "opp.m[Summoned Skull]", "you.deck[0]", "m1"
local function parsePlace(s)
    local side, kind, rest = s:match("^(%a+)%.(%a+%d?)(.*)$")
    if not side then
        local z = s:match("^(m%d)$") or s:match("^(st%d)$") or s:match("^(fz)$")
        if z then
            side, kind, rest = "you", z, ""
        else
            return nil
        end
    end
    local key = rest:match("^%[(.*)%]$")
    local place = { player = side == "opp" and 1 or 0, kind = kind, key = key }
    local zk, zi = kind:match("^(%a+)(%d)$")
    if zk then
        place.kind, place.index = zk, tonumber(zi)
    elseif key and key:match("^%d+$") then
        place.index = tonumber(key)
        place.key = nil
    end
    return place
end

-- The cursor target of a field or hand place now: player, area, index (nil when the card is not there).
local function cursorOf(place)
    local p = place.player
    if place.kind == "hand" then
        if place.index then
            return p, EDS.DUEL_AREA_HAND, place.index
        end
        local id = cardId(place.key)
        local slot = id and EDS.findPlayerHandSlot(id, p)
        return slot and p, EDS.DUEL_AREA_HAND, slot
    end
    if place.kind == "m" or place.kind == "st" then
        local base = place.kind == "m" and 0 or 5
        local area = place.kind == "m" and EDS.DUEL_AREA_MONSTER or EDS.DUEL_AREA_SPELL_TRAP
        if place.index then
            return p, area, place.index
        end
        local id = cardId(place.key)
        for z = 0, 4 do
            if id and zoneCardId(p, base + z) == id then
                return p, area, z
            end
        end
        return nil
    end
    if place.kind == "fz" then
        return p, EDS.DUEL_AREA_FIELD, 0
    end
    if place.kind == "deck" then
        return p, EDS.DUEL_AREA_DECK, 0
    end
    if place.kind == "extra" and not place.key and not place.index then
        return p, EDS.DUEL_AREA_FUSION_DECK, 0      -- the Extra Deck pile itself (extra_summon)
    end
    return nil
end

local PILE_KINDS = { gy = true, deck = true, extra = true, banished = true }

-- ---- cursor movement (keys only) ----

-- Rows from the top of the screen; the piles sit at the ends of the rows beside them.
local function rowOf(p, area)
    local own = p == 0
    if area == EDS.DUEL_AREA_HAND then
        return own and 5 or 0
    end
    if area == EDS.DUEL_AREA_SPELL_TRAP or area == EDS.DUEL_AREA_FUSION_DECK or area == EDS.DUEL_AREA_DECK then
        return own and 4 or 1
    end
    return own and 3 or 2
end

local function colOf(area, idx)
    if area == EDS.DUEL_AREA_FIELD or area == EDS.DUEL_AREA_FUSION_DECK then
        return -1
    end
    if area == EDS.DUEL_AREA_DECK then
        return -2
    end
    if area == EDS.DUEL_AREA_GRAVEYARD then
        return 5
    end
    if area == EDS.DUEL_AREA_BANISHED then
        return 6
    end
    return idx
end

-- One key toward (tp, ta, ti), or nil when the cursor is there. `stuck` turns the horizontal choice around.
local function keyToward(tp, ta, ti, stuck)
    local p, a, i = sel()
    if p == tp and a == ta and (i == ti or a == EDS.DUEL_AREA_FIELD or a == EDS.DUEL_AREA_DECK) then
        return nil
    end
    local r, tr = rowOf(p, a), rowOf(tp, ta)
    if r ~= tr then
        return tr < r and EDS.KEY_UP or EDS.KEY_DOWN
    end
    local c, tc = colOf(a, i), colOf(ta, ti)
    local right = tc > c
    if p == 1 then
        right = not right      -- player 1's rows are mirrored
    end
    if stuck then
        right = not right
    end
    return right and EDS.KEY_RIGHT or EDS.KEY_LEFT
end

-- ---- ops ----
-- Each op: { kind = ..., ... }; runOp returns true when done. fs.op holds the op's own state.

local function press(key, gap)
    EDS.press(key, gap or GAP)
end

local function fieldCursorReady()
    return EDS.duelCursorReady() and not textBox().active and not listView().active and not EDS.cardMenuOpen()
end

local function opWaitFrames(op)
    op.n = (op.n or 0) + 1
    return op.n >= (op.frames or 30)
end

-- ---- README media (tools/media): `shot`, `gif_start` / `mp4_start` ... `rec_stop` ----

local function mediaRoot()
    return (EDS.recordDir:gsub("/recordings$", "")) .. "/../.."
end

local function winPath(p)
    return (p:gsub("/", "\\"))
end

local function opMediaShot(op)
    os.execute('mkdir "' .. winPath(mediaRoot() .. "/screenshots") .. '" 2>nul')
    emu:screenshot(mediaRoot() .. "/screenshots/" .. op.name .. ".png")
    EDS.log("media: shot " .. op.name .. ".png")
    return true
end

local function opMediaRec(op, fs)
    local dir = mediaRoot() .. "/tools/media/_frames/" .. op.name
    os.execute('if exist "' .. winPath(dir) .. '" rmdir /s /q "' .. winPath(dir) .. '" 2>nul')
    os.execute('mkdir "' .. winPath(dir) .. '" 2>nul')
    fs.mediaRec = { dir = dir, n = 0, name = op.name, format = op.format }
    EDS.log("media: recording " .. op.name .. " (" .. op.format .. ")")
    return true
end

-- Every frame while recording (EDS.tickScenarioSteps): the frame as a PNG. As the card-test videos
-- (EDS.captureFrameIfNeeded), the frames of an open text box (not drawn) and of a card list are left out, unless the
-- scenario shows them (video prompts='show' / lists='show').
local function mediaCaptureFrame(fs, opts)
    local r = fs.mediaRec
    local video = (opts and opts.video) or {}
    if r and video.prompts ~= "show" and (emu:read8(EDS.G_TEXTBOX) & 1) ~= 0 then
        return
    end
    if r and video.lists ~= "show" and (emu:read8(EDS.G_CARD_LIST) & 1) ~= 0 then
        return
    end
    if r then
        emu:screenshot(string.format("%s/%06d.png", r.dir, r.n))
        r.n = r.n + 1
    end
end

-- The recording encoded (ffmpeg): a GIF at 2x and 30 fps, or an MP4 at 3x (tools/media/lib_media.lua M.stop).
local MEDIA_TAIL_KEEP = 20          -- still frames kept at the end of a recording (a third of a second)

local function readFile(path)
    local f = io.open(path, "rb")
    if not f then
        return nil
    end
    local d = f:read("*a")
    f:close()
    return d
end

local function opMediaStop(op, fs)
    local r = fs.mediaRec
    fs.mediaRec = nil
    if not r or r.n < 2 then
        EDS.log("media: nothing recorded")
        return true
    end
    -- no still tail: the trailing frames identical to the last one go, but MEDIA_TAIL_KEEP
    local function frame(i) return string.format("%s/%06d.png", r.dir, i) end
    local last = readFile(frame(r.n - 1))
    local first = r.n - 1
    while first > 0 and readFile(frame(first - 1)) == last do
        first = first - 1
    end
    local keep = math.min(r.n - 1, first + MEDIA_TAIL_KEEP)
    for i = keep + 1, r.n - 1 do
        os.remove(frame(i))
    end
    if keep + 1 < r.n then
        EDS.log(string.format("media: %d still frames trimmed from the end", r.n - 1 - keep))
        r.n = keep + 1
    end
    local input = winPath(r.dir) .. "\\%06d.png"
    local out, args
    if r.format == "mp4" then
        os.execute('mkdir "' .. winPath(mediaRoot() .. "/screenshots/videos") .. '" 2>nul')
        out = winPath(mediaRoot() .. "/screenshots/videos/" .. r.name .. ".mp4")
        args = string.format('-framerate 60 -i "%s" -vf scale=720:480:flags=neighbor -pix_fmt yuv420p -c:v libx264 "%s"',
            input, out)
    else
        os.execute('mkdir "' .. winPath(mediaRoot() .. "/screenshots/gifs") .. '" 2>nul')
        out = winPath(mediaRoot() .. "/screenshots/gifs/" .. r.name .. ".gif")
        args = string.format('-framerate 60 -i "%s" -vf "fps=30,scale=480:320:flags=neighbor,split[a][b];'
            .. '[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=none" "%s"', input, out)
    end
    local cmd = string.format('%s -y -hide_banner -loglevel error %s', EDS.ffmpegCommand() or "ffmpeg", args)
    EDS.log(string.format("media: encoding %d frames -> %s", r.n, out))
    os.execute('"' .. cmd .. '"')
    os.execute('rmdir /s /q "' .. winPath(r.dir) .. '" 2>nul')
    return true
end

-- Dismiss a box (A) while waiting for something else; true if it pressed. Only MESSAGE boxes, except before a
-- cursor pick (beforePick): see below.
local function dismissMessage(beforePick)
    local ok, b = boxInput()
    -- before a cursor pick every box is a prompt: the duel runs no effect step while a box is open, and a box
    -- opened without TextBoxSetMenu keeps the previous box's kind (custom, two-line), so any kind gets A
    local kindOk = b.kind == TEXTBOX_MESSAGE or beforePick
    if ok and kindOk then
        press(EDS.KEY_A)
        return true
    end
    return false
end

-- Move the cursor onto a place (field, hand or deck pile) with the D-pad.
local function opCursor(op, fs)
    if op.effectPick and not op.started and effectAsksForCard() then
        -- the summon is the effect's own (Soul Exchange, Metamorphosis): the card is picked, no card menu
        op.kind = "target"
        local nxt = fs and fs.ops[fs.opIndex + 1]
        if nxt and nxt.kind == "menu" then
            nxt.kind = "log"
            nxt.msg = "card menu not needed: the effect asked for the card"
        end
        return false
    end
    if not op.started then
        if textBox().active then
            dismissMessage(true)
            return false
        end
        if listView().active or EDS.cardMenuOpen() and not op.allowMenu then
            return false
        end
        if not EDS.duelCursorReady() then
            return false
        end
        op.started = true
    end
    if dismissMessage(true) then
        return false                 -- (a message box takes the keys until A closes it)
    end
    local tp, ta, ti = cursorOf(op.place)
    if tp == nil then
        op.missing = (op.missing or 0) + 1
        if op.missing > 120 then
            EDS.log("steps: " .. op.text .. ": not on the field/in the hand; skipped")
            op.failed = true
            return true
        end
        return false
    end
    local p, a, i = sel()
    local here = string.format("%d/%d/%d", p, a, i)
    if op.last == here then
        op.same = (op.same or 0) + 1
    else
        op.same = 0
    end
    op.last = here
    local key = keyToward(tp, ta, ti, op.same >= 3 and op.same < 6)
    if key == nil then
        return true
    end
    op.presses = (op.presses or 0) + 1
    if op.presses > 80 then
        local b = textBox()
        EDS.log(string.format("steps: %s: cursor stuck at %s (want %d/%d/%d; box %s kind %d step %d; "
            .. "effect step 0x%02X; summon %s)", op.text, here, tp, ta, ti, tostring(b.active), b.kind, b.step,
            EDS.chainEffectStep(), tostring(summonActive())))
        op.failed = true
        return true
    end
    press(key)
    return false
end

-- A pick on the field while the summon's prompt stays open (Synchro, Xyz and Link materials, mod_synchro.c
-- ModSynchroMenuRun and the like): the cursor moves with the box shown (opTarget would press A on the box), then A.
local function opFieldPick(op)
    op.n = (op.n or 0) + 1
    if op.confirmed then
        return op.n >= op.at + 40
    end
    if op.n < 40 then
        return false                 -- the prompt of this pick comes up
    end
    -- the materials were taken without a pick (the only ones that fit: an Xyz's): the summon is under way (its zone
    -- or position comes next). (Not the box's kind: the Synchro prompt is a custom box, like the position menu.)
    if zonePicker().picking or summonActive() then
        EDS.log("steps: " .. op.text .. ": no material pick (taken by the game)")
        return true
    end
    local tp, ta, ti = cursorOf(op.place)
    if tp == nil then
        EDS.log("steps: " .. op.text .. ": material not on the field; skipped")
        return true
    end
    local p, a, i = sel()
    local here = string.format("%d/%d/%d", p, a, i)
    if op.last == here then
        op.same = (op.same or 0) + 1
    else
        op.same = 0
    end
    op.last = here
    local key = keyToward(tp, ta, ti, op.same >= 3 and op.same < 6)
    if key == nil then
        press(EDS.KEY_A)
        op.confirmed = true
        op.at = op.n
        return false
    end
    op.presses = (op.presses or 0) + 1
    if op.presses > 80 then
        EDS.log(string.format("steps: %s: cursor stuck at %s (want %d/%d/%d)", op.text, here, tp, ta, ti))
        return true
    end
    press(key)
    return false
end

-- Card menu open on the cursor's card: Right until the command is `cmd` (or one of `cmds`), then A.
local function opMenuCommand(op)
    local want = op.cmds or { op.cmd }
    local function isWanted(c)
        for _, w in ipairs(want) do
            if c == w then
                return true
            end
        end
        return false
    end
    if not EDS.cardMenuOpen() then
        if op.confirmed then
            return true
        end
        if op.onlyIfOpen then
            -- `menu <cmd>`: a menu the game opened; none within half a second (the step before did it all): done
            op.idle = (op.idle or 0) + 1
            return op.idle > 30
        end
        if textBox().active then
            return op.confirmed
        end
        if not op.opened then
            press(EDS.KEY_A)
            op.opened = true
            op.wait = 0
            return false
        end
        op.wait = (op.wait or 0) + 1
        if op.wait > 60 then
            op.opened = false       -- the menu did not open: A again
        end
        return false
    end
    if op.confirmed then
        return pressTaken(op)       -- (the menu still open: A again if it was not taken)
    end
    if isWanted(EDS.cardMenuCommand()) then
        op.sig = progressSignature()
        press(EDS.KEY_A)
        op.confirmed = true
        return false
    end
    op.cycles = (op.cycles or 0) + 1
    if op.cycles > 16 then
        EDS.log(string.format("steps: %s: command %s not in the menu; closed", op.text, table.concat(want, "/")))
        press(EDS.KEY_B)
        op.failed = true
        return true
    end
    press(EDS.KEY_RIGHT)
    return false
end

-- A menu box (two-line choice, phase menu, position, type, level or label menu): move gTextBox.result to
-- op.value (Down first; Right if Down does not move it), then A.
local function opMenuResult(op)
    local ok, b = boxInput()
    if not ok or b.kind == TEXTBOX_MESSAGE or b.kind == TEXTBOX_YES_NO then
        if ok and b.kind == TEXTBOX_MESSAGE then
            press(EDS.KEY_A)
        end
        if op.confirmed and not b.active then
            return true
        end
        op.idle = (op.idle or 0) + 1
        if op.optional and op.idle > (op.timeout or 240) then
            return true
        end
        return false
    end
    if op.confirmed then
        -- still choosing (menuState 0 = select) well after the A: the A was missed; again, held longer
        op.since = (op.since or 0) + 1
        if emu:read8(EDS.G_TEXTBOX + 0x22) == 0 and op.since >= 20 and (op.tries or 1) < 4 then
            op.tries = (op.tries or 1) + 1
            op.since = 0
            EDS.log("steps: " .. tostring(op.text) .. ": menu A again (not taken)")
            local hold = EDS.PRESS_FRAMES
            EDS.PRESS_FRAMES = 8
            press(EDS.KEY_A)
            EDS.PRESS_FRAMES = hold
        end
        return false
    end
    if b.result == op.value then
        press(EDS.KEY_A)
        op.confirmed = true
        return false
    end
    if op.lastResult == b.result then
        op.same = (op.same or 0) + 1
    else
        op.same = 0
    end
    op.lastResult = b.result
    if op.same >= 2 then
        op.useRight = not op.useRight
        op.same = 0
    end
    op.presses = (op.presses or 0) + 1
    if op.presses > 40 then
        EDS.log(string.format("steps: %s: menu value %d not reached (at %d)", op.text, op.value, b.result))
        press(EDS.KEY_A)
        op.confirmed = true
        return false
    end
    press(op.useRight and EDS.KEY_RIGHT or EDS.KEY_DOWN)
    return false
end

local function opYesNo(op)
    local ok, b = boxInput(TEXTBOX_YES_NO)
    if not ok then
        if op.answered and not b.active then
            return true
        end
        if ok == false and b.active and b.step == 1 and b.kind == TEXTBOX_MESSAGE then
            press(EDS.KEY_A)
        end
        op.idle = (op.idle or 0) + 1
        if op.optional and op.idle > (op.timeout or 240) then
            return true
        end
        return false
    end
    if op.answered then
        return false
    end
    if not op.yes then
        press(EDS.KEY_B)        -- B answers No
        op.answered = true
        return false
    end
    if b.yesNo ~= 0 then
        op.moves = (op.moves or 0) + 1
        press(op.moves % 4 < 2 and EDS.KEY_LEFT or EDS.KEY_RIGHT)
        return false
    end
    press(EDS.KEY_A)
    op.answered = true
    return false
end

-- Pick an entry of the card list (Graveyard, Deck, Fusion Deck, banished or effect targets), then Decide.
local function opListPick(op)
    local ok, v = listReady()
    if not ok then
        if op.picked and not v.active then
            return true
        end
        if not v.active then
            dismissMessage()
        end
        return false
    end
    if op.picked then
        return false
    end
    if op.target == nil then
        if op.place.index then
            op.target = op.place.index
        else
            local id = cardId(op.place.key)
            for i = 0, v.count - 1 do
                if listCardId(i) == id and not (op.used and op.used[i]) then
                    op.target = i
                    break
                end
            end
            if op.target == nil then
                EDS.log("steps: " .. op.text .. ": not in the list; picked the first entry")
                op.target = 0
            end
        end
    end
    local at = v.top + v.row
    if at < op.target then
        press(EDS.KEY_DOWN)
        return false
    end
    if at > op.target then
        press(EDS.KEY_UP)
        return false
    end
    local decide = ((v.buttons >> LIST_BUTTON_DECIDE) & 1) ~= 0
    if decide and v.button ~= LIST_BUTTON_DECIDE then
        press(EDS.KEY_RIGHT)
        return false
    end
    press(EDS.KEY_A)
    op.picked = true
    return false
end

-- The summon zone picker (mod_link.c): Right until the candidate is the zone, then A. Done at once when the
-- picker does not open (one fitting zone) and the summon has finished.
local function opZone(op)
    local zp = zonePicker()
    local ok = boxInput(TEXTBOX_CUSTOM)
    if zp.picking and ok then
        if op.confirmed then
            return false
        end
        local want = op.zone
        if zp.cand(zp.index) == want then
            press(EDS.KEY_A)
            op.confirmed = true
            return false
        end
        op.presses = (op.presses or 0) + 1
        if op.presses > 12 then
            EDS.log("steps: " .. op.text .. ": zone not offered; took the picker's choice")
            press(EDS.KEY_A)
            op.confirmed = true
            return false
        end
        press(EDS.KEY_RIGHT)
        return false
    end
    if op.confirmed then
        return true
    end
    if ok and not zp.picking then
        return true                  -- another menu (the position menu): no zone to pick (yet)
    end
    if dismissMessage() then
        return false
    end
    op.idle = (op.idle or 0) + 1
    return op.idle > (op.timeout or 150)
end

-- Main Phase field input ready (or the Battle Phase's attacker choice).
inMainField = function()
    return EDS.turnPlayer() == 0 and EDS.duelStep() == EDS.DUEL_STEP_MAIN
        and EDS.duelPhaseStep() == MAIN_STEP_FIELD and fieldCursorReady() and not effectRunning()
end

local function inAttackerChoice()
    local b = battle()
    return EDS.turnPlayer() == 0 and EDS.duelPhaseStep() == MAIN_STEP_BATTLE
        and b.stage == BATTLE_STAGE_SELECT_ATTACKER and b.step == 1 and fieldCursorReady()
end

-- B from the Main Phase field or the attacker choice, then the phase menu (result) or "End your turn?" (yes).
local function opPhaseMenu(op)
    if not op.opened then
        -- a message waiting for A keeps the field from being idle (Vampire Lord's "sent to the Graveyard" note)
        if dismissMessage() then
            return false
        end
        if inMainField() or inAttackerChoice() then
            press(EDS.KEY_B)
            op.opened = true
        end
        return false
    end
    local ok, b = boxInput()
    if not ok then
        if op.confirmed and not b.active then
            return true
        end
        op.wait = (op.wait or 0) + 1
        if op.wait > 120 and not b.active then
            op.opened = false       -- B was not taken: again
            op.wait = 0
        end
        return false
    end
    if op.confirmed then
        return false
    end
    if b.kind == TEXTBOX_YES_NO then
        -- "End your turn?" (no Battle Phase possible): yes to end the turn, no otherwise
        if op.value == 1 then
            if b.yesNo ~= 0 then
                press(EDS.KEY_LEFT)
                return false
            end
            press(EDS.KEY_A)
        else
            EDS.log("steps: " .. op.text .. ": no Battle Phase now (End your turn? No)")
            press(EDS.KEY_B)
        end
        op.confirmed = true
        return false
    end
    if b.result == op.value then
        press(EDS.KEY_A)
        op.confirmed = true
        return false
    end
    press(EDS.KEY_DOWN)
    return false
end

-- Pick a target with the target cursor (field or hand), after any message box. An optional one (a Tribute) is
-- skipped when the game is back at the Main Phase field, or shows the zone picker, without asking for it.
local function opTarget(op)
    if op.confirmed then
        -- (op.once: a pick the progress signature cannot see, such as a Fusion material: one press, then on)
        local done = pressTaken(op, op.once and 30 or 12, op.once and 1 or 12)
        if done and progressSignature() ~= op.sig then
            EDS.log(string.format("steps: %s: taken on press %d", op.text, op.tries or 1))
        end
        return done
    end
    if op.optional then
        local bx = textBox()
        if bx.active and (bx.kind == TEXTBOX_CUSTOM or bx.kind == TEXTBOX_TWO_CHOICE) then
            EDS.log("steps: " .. op.text .. ": a menu came instead; skipped")
            return true
        end
        if zonePicker().picking or inMainField() then
            op.idle = (op.idle or 0) + 1
            if op.idle > 45 then
                EDS.log("steps: " .. op.text .. ": not asked for; skipped")
                return true
            end
            if zonePicker().picking then
                return false
            end
        else
            op.idle = 0
        end
    end
    if dismissMessage(true) then
        return false
    end
    local b = textBox()
    if b.active or listView().active or EDS.cardMenuOpen() or not EDS.duelCursorReady() then
        if listReady() then          -- the pick is a list after all
            op.kind = "list"
        end
        return false
    end
    if op.optional and inMainField() then
        return false                 -- (counting to the skip above)
    end
    if not op.cursor then
        op.cursor = { kind = "cursor", place = op.place, text = op.text }
    end
    if not opCursor(op.cursor) then
        return false
    end
    if op.cursor.failed then
        return true
    end
    op.sig = progressSignature()
    press(EDS.KEY_A)
    op.confirmed = true
    return false
end

-- Draw in your Draw Phase (deck menu: Draw), or nothing when the Draw Phase is skipped.
local function opDraw(op)
    if EDS.turnPlayer() ~= 0 then
        return false
    end
    local step = EDS.duelStep()
    if not op.hand then
        op.hand = EDS.playerHandCount()
    end
    if step > EDS.DUEL_STEP_DRAW then
        return true                  -- drawn (or skipped)
    end
    if step < EDS.DUEL_STEP_DRAW then
        return false
    end
    if op.confirmed then
        return EDS.playerHandCount() > op.hand
    end
    if EDS.cardMenuOpen() then
        if EDS.cardMenuCommand() == CMD.draw then
            press(EDS.KEY_A)
            op.confirmed = true
        else
            press(EDS.KEY_RIGHT)
        end
        return false
    end
    if not EDS.duelCursorReady() or textBox().active then
        return false
    end
    if EDS.duelSelPlayer() == 0 and EDS.duelSelArea() == EDS.DUEL_AREA_DECK then
        press(EDS.KEY_A)
    else
        press(EDS.keyTowardOwnDeck(EDS.duelSelArea()))
    end
    return false
end

-- The other player's turn passes (op.cpu: the CPU plays it), until your next Draw Phase. Prompts the CPU
-- raises for you are declined (No / B) unless a `when` step is waiting.
local function declineHumanPrompts()
    local ok, b = boxInput()
    if ok then
        if b.kind == TEXTBOX_YES_NO then
            press(EDS.KEY_B)
        elseif b.kind == TEXTBOX_MESSAGE then
            press(EDS.KEY_A)
        end
        return true
    end
    if listReady() then
        press(EDS.KEY_B)
        return true
    end
    return false
end

local function opOtherTurn(op, fs)
    fs.cpuPlays = op.cpu and true or false
    if EDS.turnPlayer() == 1 then
        op.seen = true
    end
    if op.seen and EDS.turnPlayer() == 0 and EDS.duelStep() >= EDS.DUEL_STEP_DRAW then
        fs.cpuPlays = false
        return true
    end
    declineHumanPrompts()
    return false
end

-- `when <event>: <action>`: wait for the event (declining other prompts), then run the action's ops.
local function eventMatches(op)
    local ev = op.event
    local ok, b = boxInput()
    local listOk = listReady()
    if ev.kind == "attack" then
        local bt = battle()
        if not (ok and b.kind == TEXTBOX_YES_NO) then
            return false
        end
        if bt.stage < 3 or bt.stage > 6 or bt.attacker ~= 1 then
            return false
        end
        local _, _, ti = cursorOf(ev.place)
        return ti == nil or ti == bt.atkSlot or ev.place.kind ~= "m"
    end
    if ev.kind == "activate" then
        if not (ok and b.kind == TEXTBOX_YES_NO) then
            return false
        end
        local id = ev.place and ev.place.key and cardId(ev.place.key)
        return id == nil or EDS.chainActivatingCardId() == id
    end
    if ev.kind == "summon" then
        return ok and b.kind == TEXTBOX_YES_NO and battle().stage == 0
    end
    if ev.kind == "replay" then
        local bt = battle()
        return bt.stage == BATTLE_STAGE_SELECT_TARGET and bt.declared and EDS.turnPlayer() == 0
            and (fieldCursorReady() or (ok and b.kind == TEXTBOX_MESSAGE))
    end
    -- prompt <name>
    if ev.name == "declare_level" or ev.name == "position" then
        return ok and (b.kind == TEXTBOX_CUSTOM or b.kind == TEXTBOX_TWO_CHOICE)
    end
    if ev.name == "deck_pick" or ev.name == "list" then
        return listOk
    end
    return ok and b.kind == TEXTBOX_YES_NO
end

local buildOps                       -- (forward)

local function opWhen(op, fs)
    if op.matched then
        return true
    end
    if eventMatches(op) then
        op.matched = true
        local ops = buildOps(op.action, true, fs)
        if not fs.captureArmed and fs.armOnWhen then
            for _, o in ipairs(ops) do
                if o.activation then
                    o.arm = true            -- the capture starts with the cursor on this card
                    fs.armOnWhen = false
                    break
                end
            end
        end
        for k = #ops, 1, -1 do
            table.insert(fs.ops, fs.opIndex + 1, ops[k])
        end
        EDS.log("steps: event: " .. op.text)
        return true
    end
    fs.cpuPlays = true
    if not EDS.busy() and op.event.kind ~= "replay" then
        -- other prompts while waiting: No
        local ok, b = boxInput()
        if ok and b.kind == TEXTBOX_YES_NO then
            press(EDS.KEY_B)
        end
    end
    op.wait = (op.wait or 0) + 1
    if op.wait > 9000 then
        EDS.log("steps: " .. op.text .. ": the event did not happen")
        return true
    end
    return false
end

-- Wait until nothing is running (no box, chain, list or menu) for a few frames.
-- The scenario's last settle waits for the card's effect too (fs.effectReady, set by EDS.effectUnderTestDone): a
-- Spell's activation starts a few frames after the menu's A, after a short idle gap that 10 idle frames alone would
-- take for the end. A card that never starts a chain or a battle (a passive one) ends after END_EFFECT_WAIT frames.
local END_EFFECT_WAIT = 600

local function opSettle(op, fs)
    if op.text == "end" and fs and fs.cardUnderTest and not fs.effectDone and op.frames_in < END_EFFECT_WAIT then
        op.quiet = 0
        return false
    end
    if op.text ~= "end" then
        -- a question or a list waiting for an answer: the next step answers it (Kycoo's Yes/No after its attack)
        local ok, b = boxInput()
        if (ok and b.kind ~= TEXTBOX_MESSAGE) or listReady() then
            return true
        end
    end
    if textBox().active or EDS.chainOrEffectBusy() or listView().active or EDS.cardMenuOpen() or summonActive()
        or duelCommandsRunning() then
        op.quiet = 0
        dismissMessage()
        return false
    end
    op.quiet = (op.quiet or 0) + 1
    return op.quiet >= (op.frames or 20)
end

local RUN = {
    cursor = opCursor, menu = opMenuCommand, result = opMenuResult, yesno = opYesNo, list = opListPick,
    zone = opZone, phase = opPhaseMenu, target = opTarget, fieldpick = opFieldPick, draw = opDraw, other_turn = opOtherTurn,
    media_shot = opMediaShot, media_rec = opMediaRec, media_stop = opMediaStop,
    when = opWhen, settle = opSettle,
    wait = opWaitFrames,
    await = function(op)
        op.t = (op.t or 0) + 1
        if op.t > 60 * 60 then
            EDS.log("steps: await " .. op.name .. ": never came")
            return true
        end
        if not op.seen then
            op.seen = op.id ~= nil and (EDS.chainActivatingCardId() == op.id or EDS.chainResolvingCardId() == op.id)
            return false
        end
        if not op.resolved then
            -- (the chain list comes between the activation and the resolution: wait for the resolution itself)
            op.resolved = EDS.chainIsResolving()
            return op.resolved and op.startOnly
        end
        -- (not chainOrEffectBusy: with the chain list shown its build byte stays set after the chain)
        if EDS.chainIsResolving() or effectAsksForCard() or textBox().active or duelCommandsRunning() then
            op.quiet = 0
            return false
        end
        op.quiet = (op.quiet or 0) + 1
        return op.quiet >= 30
    end,
    unfreeze = function(op, fs)
        if fs.opts then
            fs.opts.freezeOpponent = false
        end
        -- (the frozen set-up also marked the CPU's next turn as skipped: EDS.freezeOpponent, skipTurn bit 0x08)
        emu:write8(EDS.PLAYER1 + 9, emu:read8(EDS.PLAYER1 + 9) & ~0x08)
        return true
    end,
    -- README media: a key held across the following steps (`hold R` ... `release R`: Hold R / Hold L)
    keyhold = function(op)
        if op.on then
            emu:addKeys(op.key)
        else
            emu:clearKeys(op.key)
        end
        return true
    end,
    -- README media: Settings bits in gSaveData.options (src/mod_settings.c), read live by the duel
    options = function(op)
        local v = (emu:read16(SAVE_OPTIONS) & ~op.clear) | op.set
        emu:write16(SAVE_OPTIONS, v)
        EDS.log(string.format("steps: options = 0x%04X", v))
        return true
    end,
    press = function(op)
        if not op.done then
            press(op.key)
            op.done = true
            return false
        end
        return true
    end,
    log = function(op)
        EDS.log("steps: " .. op.msg)
        return true
    end,
}

-- A place's "pick": a pile entry (list) or a card on the field / in a hand (target cursor).
local function pickOp(text, place)
    if PILE_KINDS[place.kind] and place.kind ~= "deck" or (place.kind == "deck" and (place.key or place.index)) then
        return { kind = "list", place = place, text = text }
    end
    return { kind = "target", place = place, text = text }
end

local function splitList(s)
    local out = {}
    for item in s:gmatch("[^,]+") do
        out[#out + 1] = item:match("^%s*(.-)%s*$")
    end
    return out
end

-- The ops of one step (in a `when` action, `activate` first answers the response prompt with Yes).
buildOps = function(step, inResponse, fs)
    local verb, args = step:match("^(%S+)%s*(.*)$")
    local ops = {}
    local function add(op)
        op.text = op.text or step
        ops[#ops + 1] = op
    end
    if step:match("^when ") then
        local event, action = step:match("^when (.-): (.+)$")
        local ev = {}
        local a, t = event:match("^attack (%S+) %-> (%S+)$")
        if a then
            ev.kind, ev.place = "attack", parsePlace(a)
        elseif event:match("^activate ") then
            ev.kind, ev.place = "activate", parsePlace(event:match("^activate (.+)$"))
        elseif event:match("^summon ") then
            ev.kind = "summon"
        elseif event == "replay" then
            ev.kind = "replay"
        else
            ev.kind, ev.name = "prompt", event:match("^prompt (.+)$")
        end
        add({ kind = "when", event = ev, action = action })
        return ops
    end
    if verb == "activate" then
        local place = parsePlace(args)
        if inResponse then
            add({ kind = "yesno", yes = true })
        end
        add({ kind = "cursor", place = place, allowMenu = false, activation = true })
        add({ kind = "menu", cmd = CMD.activate })
    elseif verb == "summon" or verb == "set" then
        local who, zone, pos = args:match("^(.-) %-> (%S+)%s*(%a*)$")
        add({ kind = "cursor", place = parsePlace(who), effectPick = true })
        add({ kind = "menu", cmd = verb == "summon" and CMD.summon or CMD.set })
        add({ kind = "zone", zone = parsePlace(zone).index, optional = true })
        if verb == "summon" then
            add({ kind = "result", value = pos == "def" and 1 or 0, optional = true, timeout = 60 })
        end
    elseif verb == "tribute_summon" then
        local who, zone, tpos, tributes = args:match("^(.-) %-> (%S+) (%a+) tributes (.+)$")
        add({ kind = "cursor", place = parsePlace(who), effectPick = true })
        add({ kind = "menu", cmd = CMD.summon })
        for _, t in ipairs(splitList(tributes)) do
            add({ kind = "target", place = parsePlace(t), optional = true })
        end
        add({ kind = "zone", zone = parsePlace(zone).index, optional = true })
        add({ kind = "result", value = tpos == "def" and 1 or 0, optional = true, timeout = 60 })
    elseif verb == "special_summon" then
        local who, zone, pos, banish = args:match("^(.-) %-> (%S+) (%a+)%s*(.*)$")
        add({ kind = "cursor", place = parsePlace(who), effectPick = true })
        add({ kind = "menu", cmd = CMD.sp_summon })
        local list = banish and banish:match("^banish (.+)$")
        if list then
            for _, b in ipairs(splitList(list)) do
                add(pickOp(step, parsePlace(b)))
            end
        end
        add({ kind = "zone", zone = parsePlace(zone).index, optional = true })
        add({ kind = "result", value = pos == "def" and 1 or 0, optional = true, timeout = 90 })
    elseif verb == "fusion_summon" then
        local fusion, mats, zone, pos = args:match("^(.-) from (.+) %-> (%S+) (%a+)$")
        add({ kind = "cursor", place = parsePlace("you.hand[Polymerization]") })
        add({ kind = "menu", cmd = CMD.activate })
        add({ kind = "list", place = { player = 0, kind = "extra", key = fusion } })
        for _, m in ipairs(splitList(mats)) do
            add({ kind = "target", place = parsePlace(m), once = true })
        end
        add({ kind = "zone", zone = parsePlace(zone).index, optional = true })
        add({ kind = "result", value = pos == "def" and 1 or 0, optional = true, timeout = 90 })
    elseif verb == "flip_summon" then
        add({ kind = "cursor", place = parsePlace(args) })
        add({ kind = "menu", cmd = CMD.flip })
    elseif verb == "change_position" then
        add({ kind = "cursor", place = parsePlace(args) })
        add({ kind = "menu", cmds = { CMD.def_pos, CMD.atk_pos } })
    elseif verb == "target" or verb == "discard" then
        add(pickOp(step, parsePlace(args)))
    elseif verb == "pick" then
        add(pickOp(step, parsePlace(args)))
    elseif verb == "yes" or verb == "no" then
        add({ kind = "yesno", yes = verb == "yes" })
    elseif verb == "choose" then
        local label = args:match('^"(.*)"$') or args
        local v = CHOICES[label:lower()] or tonumber(label:match("^#(%d+)$") or "")
        if v == nil then
            add({ kind = "log", msg = "unknown menu line " .. label .. "; first line taken" })
            v = 0
        end
        add({ kind = "result", value = v })
    elseif verb == "declare" then
        local lv = args:match("^level (%d+)$")
        local v
        if lv then
            v = tonumber(lv) - 1
        elseif TYPES[args:lower()] then
            v = TYPES[args:lower()] - 1
        else
            v = CHOICES[args:lower()] or 0
        end
        add({ kind = "result", value = v })
    elseif verb == "position" then
        add({ kind = "result", value = args == "def" and 1 or 0 })
    elseif verb == "zone" then
        add({ kind = "zone", zone = parsePlace(args).index })
    elseif verb == "battle" then
        add({ kind = "phase", value = 0 })
    elseif verb == "attack" then
        local a, t = args:match("^(%S+) %-> (%S+)$")
        add({ kind = "cursor", place = parsePlace(a) })
        add({ kind = "menu", cmd = CMD.attack })
        if t == "direct" then
            add({ kind = "yesno", yes = true, optional = true, timeout = 90 })
        else
            add({ kind = "target", place = parsePlace(t) })
        end
        add({ kind = "settle" })
    elseif verb == "cancel" then
        add({ kind = "wait", frames = 20 })
        add({ kind = "press", key = EDS.KEY_B })
    elseif verb == "main2" then
        add({ kind = "phase", value = 0 })
    elseif verb == "pass" then
        add({ kind = "phase", value = 1 })
    elseif verb == "opp_turn" then
        add({ kind = "other_turn", cpu = false })
    elseif verb == "cpu_turn" then
        add({ kind = "other_turn", cpu = true })
    elseif verb == "draw" then
        add({ kind = "draw" })
    elseif verb == "check" and args:match("^menu ") then
        -- the card's menu, opened for a moment so the video shows what it offers (not checked), then closed
        local where = args:match("^menu (.-) lacks ") or args:match("^menu (.-) has ") or args:match("^menu (.+)$")
        add({ kind = "cursor", place = parsePlace(where) })
        add({ kind = "press", key = EDS.KEY_A })
        add({ kind = "wait", frames = 90 })
        add({ kind = "press", key = EDS.KEY_B })
        add({ kind = "wait", frames = 20 })
        add({ kind = "log", msg = step .. " (shown, not checked)" })
    elseif verb == "check" or verb == "note" then
        add({ kind = "log", msg = step .. " (not checked)" })
    -- README media (tools/media): pictures and recordings of a set board
    elseif verb == "point" then
        add({ kind = "cursor", place = parsePlace(args) })      -- the cursor on a card (its info bar), no menu
    elseif verb == "shot" then
        add({ kind = "media_shot", name = args })               -- screenshots/<name>.png
    elseif verb == "open_menu" then
        add({ kind = "cursor", place = parsePlace(args) })      -- the card's menu, left open (a `shot` follows)
        add({ kind = "press", key = EDS.KEY_A })
        add({ kind = "wait", frames = 40 })
    elseif verb == "close" then
        add({ kind = "press", key = EDS.KEY_B })
        add({ kind = "wait", frames = 20 })
    elseif verb == "wait" then
        add({ kind = "wait", frames = tonumber(args) or 30 })
    elseif verb == "gif_start" or verb == "mp4_start" then
        add({ kind = "media_rec", name = args, format = verb == "gif_start" and "gif" or "mp4" })
    elseif verb == "extra_summon" then
        -- `extra_summon <Card> from <material>, <material> -> <m0..m4|emz0|emz1> [atk|def]`: a Synchro, Xyz or Link
        -- Summon through the Extra Deck pile's Special Summon list (mod_synchro.c, mod_xyz.c, mod_link.c)
        local card, mats, zone, pos = args:match("^(.-) from (.+) %-> (%S+)%s*(%a*)$")
        add({ kind = "cursor", place = { player = 0, kind = "extra" } })
        add({ kind = "menu", cmd = CMD.sp_summon })
        add({ kind = "list", place = { player = 0, kind = "extra", key = card } })
        for _, m in ipairs(splitList(mats)) do
            add({ kind = "fieldpick", place = parsePlace(m) })
        end
        local emz = zone:match("^emz(%d)$")
        -- (the materials leave first: the picker can take several seconds to open)
        add({ kind = "zone", zone = emz and 16 + tonumber(emz) or parsePlace(zone).index, optional = true,
              timeout = 600 })
        if pos ~= "" then
            add({ kind = "result", value = pos == "def" and 1 or 0, optional = true, timeout = 90 })
        end
        add({ kind = "settle" })
    elseif verb == "await" or verb == "await_resolve" then
        -- `await <card>`: until that card's chain link has come and resolved (a clip that ends on the CPU's play);
        -- `await_resolve <card>`: only until it starts resolving (Hold R: let go of R there, or the chain list waits)
        add({ kind = "await", id = cardId(args), name = args, startOnly = verb == "await_resolve",
              limit = 3600 })
    elseif verb == "unfreeze" then
        -- `unfreeze`: the CPU plays from here on (a clip whose CPU turn matters, after your own set-up turn)
        add({ kind = "unfreeze" })
    elseif verb == "settle" then
        -- `settle`: until the duel is idle (no box, chain, effect, list, menu, summon or queued command) for 20
        -- frames: a recording ends there, with no still tail
        add({ kind = "settle" })
    elseif verb == "menu" then
        -- `menu set`: a command of a card menu the game opened by itself (Graverobber's stolen card)
        add({ kind = "menu", cmd = CMD[args] or CMD.activate, onlyIfOpen = true })
    elseif verb == "options" then
        -- `options speed=super anims=hide battle=show`: the Settings bits (gSaveData.options)
        local set, clear = 0, 0
        for name, value in args:gmatch("(%a+)=(%a+)") do
            local o = OPTION_BITS[name]
            if o and o.values[value] then
                clear = clear | o.mask
                set = set | o.values[value]
            else
                add({ kind = "log", msg = "unknown option " .. name .. "=" .. value })
            end
        end
        add({ kind = "options", set = set, clear = clear })
    elseif verb == "hold" or verb == "release" then
        add({ kind = "keyhold", key = HOLD_KEYS[args] or 0, on = verb == "hold" })
    elseif verb == "rec_stop" then
        add({ kind = "media_stop" })
    else
        add({ kind = "log", msg = "unknown step " .. step })
    end
    if verb ~= "when" and verb ~= "check" and verb ~= "note" and verb ~= "pass" and verb ~= "opp_turn"
        and verb ~= "cpu_turn" and verb ~= "draw" and verb ~= "cancel" then
        add({ kind = "wait", frames = 6 })
    end
    return ops
end

-- ---- the end of a CPU scenario ----

-- True on the first frame after the card under test did its thing (a chain link of that card resolved, or a
-- battle it fought in, as attacker or attack target, ended) when the duel has nothing left to run: no chain or
-- effect step, box, list, menu or summon, and an empty, idle duel command queue (the queued animations: cards to
-- the Graveyard, LP counting down, draws). Cards with neither (a passive one) never end here: the
-- scenario then runs to the end of its steps (the CPU's whole turn).
-- The game showing the card's effect (DUEL_CMD_SHOW_CARD_EFFECT 0x73 running at the head of gDuelCmd, arg2 = the
-- card ID): what a Continuous Spell/Trap does when its effect applies (Solemn Wishes on a draw). Its activation
-- resolves with no effect, so for one (opts.effectShown, from gen_testing_lua.py) the effect starts here.
local DUEL_CMD_SHOW_CARD_EFFECT = 0x73
local function cardEffectShown(id)
    -- (with the animations skipped, gModTestingLastShown holds the card the game would have shown)
    local last = emu:read16(EDS.G_MOD_TESTING_LAST_SHOWN)
    if last ~= 0 then
        emu:write16(EDS.G_MOD_TESTING_LAST_SHOWN, 0)
        if (last & 0x7FF) == id then
            return true
        end
    end
    return (emu:read8(G_DUEL_CMD + 0x80D) & 0x20) ~= 0
        and (emu:read16(G_DUEL_CMD) & 0xFFF) == DUEL_CMD_SHOW_CARD_EFFECT
        and (emu:read16(G_DUEL_CMD + 2) & 0x7FF) == id
end

-- The duel phase (gDuel.phase, +0x1B12 bits 2-4: Draw 0, Standby 1, Main 1 2, Battle 3, Main 2 4, End 5).
local function duelPhase()
    return (emu:read8(EDS.G_DUEL_TURN_FLAGS) >> 2) & 7
end

local VIDEO_SHOW_FRAMES = 90

-- The card `id` face up in Defense Position in a monster zone of either player.
local function cardInDefense(id)
    for p = 0, 1 do
        for z = 0, 4 do
            if zoneCardId(p, z) == id and (emu:read8(zoneAddr(p, z) + 6) & 1) ~= 0 then
                return true
            end
        end
    end
    return false
end

function EDS.effectUnderTestDone(fs)
    local id = fs.cardUnderTest
    if not id then
        return false
    end
    if fs.effectDone then
        return true
    end
    if fs.showFrames then
        -- (showing: counted whatever the duel does meanwhile; the CPU goes on playing, so the shot also ends, and
        -- the capture stops, when its next phase begins: the phase changes or the field fades to white for it,
        -- REG_BLDCNT effect 2)
        fs.showFrames = fs.showFrames + 1
        local fading = ((emu:read16(0x04000050) >> 6) & 3) == 2
        if fs.showFrames < VIDEO_SHOW_FRAMES and duelPhase() == fs.showPhase and not fading then
            return false
        end
        if fs.showFrames < VIDEO_SHOW_FRAMES then
            fs.captureStop = true
        end
        fs.effectDone = true
        return true
    end
    if fs.needShown and not fs.shownSeen and cardEffectShown(id) then
        fs.shownSeen = true
    end
    if not fs.effectSeen then
        if fs.video.start == "summon" and summoningCardId() == id then
            fs.effectSeen = "summon"        -- (its own summon is the effect: The Fiend Megacyber)
        elseif EDS.chainResolvingCardId() == id or (EDS.chainIsResolving() and EDS.chainActivatingCardId() == id) then
            fs.effectSeen = "chain"
        else
            local bt = battle()
            if bt.stage >= 7 and bt.stage <= 11 then         -- damage calculation to the end of the attack
                local atkPlayer = bt.attacker
                local b = emu:read16(G_BATTLE)
                local defSlot = (b >> 9) & 7
                if zoneCardId(atkPlayer, bt.atkSlot) == id
                    or (defSlot < 5 and zoneCardId(1 - atkPlayer, defSlot) == id) then
                    fs.effectSeen = "battle"
                end
            end
        end
        return false
    end
    if fs.needShown and not fs.shownSeen then
        -- (activated; its effect has not applied yet: shown, or the card chained again once its activation ended,
        -- as Skull Lair or Wave Motion Cannon do)
        local resolving = EDS.chainResolvingCardId() == id
            or (EDS.chainIsResolving() and EDS.chainActivatingCardId() == id)
        if not resolving and not EDS.chainOrEffectBusy() then
            fs.activationOver = true
        elseif resolving and fs.activationOver then
            fs.shownSeen = true
        end
        if not fs.shownSeen then
            return false
        end
    end
    fs.effectReady = true               -- (the effect started: the end waits for it; the video start is fixed)
    local busy = textBox().active or listView().active or EDS.cardMenuOpen() or EDS.chainOrEffectBusy()
        or summonActive() or duelCommandsRunning()
    if fs.effectSeen == "battle" then
        local st = battle().stage
        busy = busy or (st >= 2 and st <= 11)
    elseif fs.effectSeen == "summon" then
        -- (done when the summon is: the CPU goes on playing other cards at once, so the duel need not be idle)
        busy = summonActive()
    end
    if busy then
        return false
    end
    -- video until='defense': the card in Defense Position (Goblin Attack Force at the end of the Battle Phase)
    if fs.video["until"] == "defense" and not cardInDefense(id) then
        return false
    end
    -- video show=<place>: the cursor on that zone for VIDEO_SHOW_FRAMES, so the info bar shows its ATK
    if fs.video.show then
        if not fs.showFrames then
            local place = parsePlace(fs.video.show)
            local zone = place.kind == "st" and 5 + (place.index or 0) or (place.index or 0)
            emu:write16(EDS.G_MOD_TESTING_SELECT, 0x8000 | ((place.player & 1) << 8) | (zone & 0xF))
            fs.showFrames = 0
            fs.showPhase = duelPhase()
            EDS.log("steps: showing " .. fs.video.show)
        end
        fs.showFrames = fs.showFrames + 1
        if fs.showFrames < VIDEO_SHOW_FRAMES then
            return false
        end
    end
    fs.effectDone = true
    return true
end

-- ---- where a CPU scenario's video starts ----

-- The card under the duel cursor (gDuelScreen's selection: the CPU moves it with DuelCursor_Select when it points
-- at a card it plays, DUEL_CMD_POINT_AT_CARD), or 0.
local function selectedCardId()
    local p, area, index = EDS.duelSelPlayer(), EDS.duelSelArea(), EDS.duelSelIndex()
    if p > 1 then
        return 0
    end
    if area == EDS.DUEL_AREA_HAND then
        return EDS.handSlotCardId(p, index) or 0
    end
    if (area == EDS.DUEL_AREA_MONSTER or area == EDS.DUEL_AREA_SPELL_TRAP) and index <= 4 then
        return zoneCardId(p, area + index)
    end
    if area == EDS.DUEL_AREA_FIELD then
        return zoneCardId(p, 10)
    end
    return 0
end

-- The card a summon is pending for (gSummonAction: +0x0E bit 0 active; cardId bits 31-46), or 0.

-- Every frame of a CPU-only capture until the card's effect starts (fs.effectReady): the frames are all saved
-- from the CPU's turn on, and the video starts at fs.captureStartIndex (EDS.finalizeMp4Capture: ffmpeg
-- -start_number). That is the first frame, in the phase the effect starts in, at which the CPU starts using the
-- card under test: the cursor on it (the CPU pointing at it to play it), a summon of it (a Flip Summon moves no
-- cursor), or its attack declaration; or the start of that phase when none of these happened.
local VIDEO_HAND_LEAD = 12

local function trackCpuVideoStart(fs)
    if not fs.cpuOnlyCapture or not fs.captureArmed or fs.effectReady or not fs.cardUnderTest then
        return
    end
    if EDS.turnPlayer() ~= 1 then
        return
    end
    local idx = math.max((fs.captureIndex or 1) - 1, 0)       -- the frame captured this frame
    local phase = duelPhase()
    if phase ~= fs.videoPhase then
        fs.videoPhase = phase
        fs.videoPhaseStart = idx
        fs.captureStartIndex = idx
        fs.videoOnCard = false
    end
    -- A Spell/Trap the CPU plays from its hand: its cursor first reaches it on the field, so the video starts
    -- VIDEO_HAND_LEAD frames before it left the hand (no earlier than the phase start), with it still in the hand.
    local inHand = EDS.findPlayerHandSlot(fs.cardUnderTest, 1) ~= nil
    if fs.videoCardInHand and not inHand and not fs.videoOnCard then
        fs.captureStartIndex = math.max(fs.videoPhaseStart or 0, idx - VIDEO_HAND_LEAD)
        fs.videoOnCard = true
    end
    fs.videoCardInHand = inHand
    -- the first time in this phase (the CPU pointing at it to play it: the flip, the activation, the attack); the
    -- cursor comes back onto it later for the effect's activation, after the flip
    if fs.videoOnCard then
        return
    end
    local bt = battle()
    if selectedCardId() == fs.cardUnderTest or summoningCardId() == fs.cardUnderTest
        or (bt.declared and bt.attacker == 1 and zoneCardId(1, bt.atkSlot) == fs.cardUnderTest) then
        fs.captureStartIndex = idx
        fs.videoOnCard = true
    end
end

-- ---- the runner ----

function EDS.tickScenarioSteps(fs, opts)
    fs.opts = opts                  -- (the `unfreeze` step changes opts.freezeOpponent)
    if not fs.ops then
        fs.ops = {}
        local steps = {}
        if opts.start and opts.start.turn == "opp" then
            steps[#steps + 1] = "pass"          -- the setup is on your Main Phase 1: the CPU's turn comes next
        end
        for _, s in ipairs(opts.steps or {}) do
            steps[#steps + 1] = s
        end
        for _, s in ipairs(steps) do
            if s == "pass" and opts.actor == "CPU" then
                -- The CPU's Standby Phase re-applies the cursor's stored selection (DuelCursor_Refresh), which
                -- scrolls the field to it: left on your hand, the camera would pan down to your field before the
                -- CPU acts. So the cursor goes to the CPU's Spell/Trap Zone 0 first: its scroll target (20) is the
                -- one the CPU's turn starts at (its hand), so the refresh does not move the camera at all.
                fs.ops[#fs.ops + 1] = { kind = "cursor", place = parsePlace("opp.st0"), camera = true,
                                        text = "cursor to the CPU's side (camera)", step = s }
            end
            local ok, ops = pcall(buildOps, s, false, fs)
            if ok then
                for _, op in ipairs(ops) do
                    op.step = s
                    fs.ops[#fs.ops + 1] = op
                end
            else
                EDS.log("steps: cannot read step " .. s .. ": " .. tostring(ops))
            end
        end
        -- the end: the duel idle (no chain, box, summon or queued animation) for 10 frames; the margin covers the
        -- idle gaps between phases after a final `pass` (End Phase effects come after them)
        fs.ops[#fs.ops + 1] = { kind = "settle", text = "end", frames = 10, limit = END_EFFECT_WAIT + 900 }
        fs.opIndex = 1
        -- The PNG capture starts once the cursor is on the first card to activate: that cursor op is marked
        -- `arm`. With the activation in a `when` step, its cursor op is marked when inserted; with none, the
        -- first cursor op; with no cursor op at all, the capture starts now.
        local arm
        local video = opts.video or {}
        for _, op in ipairs(fs.ops) do
            if op.kind == "cursor" and not op.camera and (op.activation or video.start == "first") then
                arm = op
                break
            end
        end
        if not arm then
            for _, s in ipairs(steps) do
                if s:match("^when .-: activate ") then
                    fs.armOnWhen = true
                    break
                end
            end
        end
        if not arm and not fs.armOnWhen then
            for _, op in ipairs(fs.ops) do
                if op.kind == "cursor" and not op.camera then
                    arm = op
                    break
                end
            end
        end
        -- A CPU scenario with no action of yours (only passing, drawing, answering): the capture starts with
        -- the CPU's turn and leaves your turns out (fs.cpuOnlyCapture, read by EDS.captureFrameIfNeeded).
        fs.cpuScenario = opts.actor == "CPU"
        fs.cardUnderTest = opts.card and cardId(opts.card)
        fs.needShown = opts.effectShown and true or false
        fs.video = opts.video or {}
        if fs.video.effect == "none" then
            fs.effectReady, fs.effectDone = true, true      -- (nothing is meant to happen)
        end
        if fs.cpuScenario and fs.cardUnderTest then
            opts.onlyCardId = fs.cardUnderTest      -- the CPU activates no other card (EDS.writeTestingFlags)
        end
        if fs.cpuScenario and not arm and not fs.armOnWhen then
            fs.armOnCpuTurn = true
            fs.cpuOnlyCapture = true
        elseif arm then
            arm.arm = true
        elseif not fs.armOnWhen then
            fs.captureArmed = true
        end
        EDS.log(string.format("steps: %d steps, %d ops", #steps, #fs.ops))
    end
    mediaCaptureFrame(fs, opts)
    if fs.armOnCpuTurn and not fs.captureArmed and EDS.turnPlayer() == 1 then
        fs.captureArmed = true
        EDS.log("steps: capture starts (the CPU's turn)")
    end
    local wasReady = fs.effectReady
    trackCpuVideoStart(fs)
    -- (every scenario tracks the card's effect, for the last settle; a CPU scenario also ends on it)
    local done = EDS.effectUnderTestDone(fs) and fs.cpuScenario
    if fs.effectReady and not wasReady and fs.captureStartIndex then
        EDS.log(string.format("steps: the effect started (%s); the video starts at frame %d (phase %d%s)",
            fs.shownSeen and "shown" or fs.effectSeen, fs.captureStartIndex, fs.videoPhase or -1,
            fs.videoOnCard and ", cursor on the card" or ""))
    end
    if done then
        EDS.log("steps: the card's effect is done; ending the scenario")
        fs.opIndex = #fs.ops + 1
        fs.endNow = true                -- (no settle wait in EDS.run: the duel is idle already)
        return true
    end
    if fs.opIndex > #fs.ops then
        return true
    end
    if EDS.busy() then
        return false
    end
    local op = fs.ops[fs.opIndex]
    if op.step ~= fs.lastLoggedStep and op.step then
        fs.lastLoggedStep = op.step
        EDS.log("step: " .. op.step)
    end
    op.frames_in = (op.frames_in or 0) + 1
    local fn = RUN[op.kind]
    if not fn then
        fs.opIndex = fs.opIndex + 1
        return false
    end
    if fn(op, fs) then
        if op.arm and not fs.captureArmed then
            fs.captureArmed = true
            EDS.log("steps: capture starts (cursor on " .. tostring(op.text) .. ")")
        end
        fs.opIndex = fs.opIndex + 1
    elseif op.frames_in > (op.limit or 900) and op.kind ~= "other_turn" and op.kind ~= "when" then
        local tb, lv = textBox(), listView()
        EDS.log(string.format("steps: %s timed out (%s) [box active=%s kind=%d step=%d; list=%s; chain=%s; summon=%s; cmds=%s; prompt=%d]",
            op.kind, tostring(op.text or op.step), tostring(tb.active), tb.kind, tb.step, tostring(lv.active),
            tostring(EDS.chainOrEffectBusy()), tostring(summonActive()), tostring(duelCommandsRunning()),
            emu:read8(EDS.G_DUEL + 0x1B50)))
        fs.opIndex = fs.opIndex + 1
    end
    return false
end

-- ---- the starting state ----

local function writeCard(addr, id, owner, special)
    emu:write32(addr, (id & 0xFFF) | ((owner & 1) << 12) | (special and 0x10000 or 0))
end

local function clearZone(p, z)
    local a = zoneAddr(p, z)
    for off = 0, EDS.ZONE_SIZE - 4, 4 do
        emu:write32(a + off, 0)
    end
end

local PILE_OFFSET = { deck = EDS.OFFSET_DECK, gy = 0x904, extra = EDS.OFFSET_FUSION, banished = 0xB84 }
local PILE_COUNT = { deck = 3, gy = 4, extra = 5, banished = 6 }

function EDS.applyExactSetup(opts)
    -- A CPU scenario keeps the CPU's board from its first turn (its zones and piles stay as they are, so no card
    -- vanishes before it acts); only its hand, and the zones and piles the setup lists, are replaced.
    local keepCpuBoard = opts.actor == "CPU"
    local listed = { [0] = {}, [1] = {} }
    for _, row in ipairs(opts.setupCards or {}) do
        if row.zone ~= "hand" then
            local z = row.zone == "monster" and (row.slot or 0) or row.zone == "st" and 5 + (row.slot or 0) or 10
            listed[row.player == 1 and 1 or 0][z] = true
        end
    end
    local listedPiles = { [0] = {}, [1] = {} }
    for _, pile in ipairs(opts.setupPiles or {}) do
        listedPiles[pile.player == 1 and 1 or 0][pile.pile] = true
    end
    for p = 0, 1 do
        local base = playerBase(p)
        local keep = keepCpuBoard and p == 1
        local clearMonsters = keep and (opts.video or {}).clear == "cpu_monsters"
        for z = 0, 10 do
            if not keep or listed[p][z] or (clearMonsters and z <= 4) then
                clearZone(p, z)
            end
        end
        emu:write8(base + 2, 0)                   -- hand
        if not keep or listedPiles[p].gy then
            emu:write8(base + 4, 0)               -- Graveyard
        end
        if not keep or listedPiles[p].extra then
            emu:write8(base + 5, 0)               -- Fusion Deck
        end
        if not keep or listedPiles[p].banished then
            emu:write8(base + 6, 0)               -- banished
        end
        if not keep then
            emu:write16(base + 0x26, 0)           -- attackedMask
            emu:write8(base + 8, emu:read8(base + 8) & ~0x30)  -- normalSummonUsed, summonedThisTurn
        end
    end
    for _, row in ipairs(opts.setupCards or {}) do
        local p = row.player == 1 and 1 or 0
        local base = playerBase(p)
        local id = row.id or cardId(row.name)
        if id then
            if row.zone == "hand" then
                writeCard(base + EDS.OFFSET_HAND + (row.slot or 0) * 4, id, p)
                emu:write8(base + 2, math.max(emu:read8(base + 2), (row.slot or 0) + 1))
            elseif row.zone == "monster" or row.zone == "st" or row.zone == "field" then
                local z = row.zone == "monster" and (row.slot or 0) or row.zone == "st" and 5 + (row.slot or 0) or 10
                local a = zoneAddr(p, z)
                writeCard(a, id, p, row.specialSummoned)
                local flags
                if row.zone == "monster" then
                    if row.set or row.defense then
                        flags = 0x01                      -- face-down Defense
                    elseif row.position == "def" then
                        flags = 0x03                      -- face-up Defense
                    else
                        flags = 0x02                      -- face-up Attack
                    end
                    emu:write8(a + 7, 0x20)               -- effectUnused: its once-per-turn effect is ready
                else
                    flags = row.set and 0x00 or 0x02
                    if row.set then
                        -- canActivate (+0x91 bit 2): Set on an earlier turn, so it may be activated; not negated (bit 3)
                        emu:write8(a + 0x91, (emu:read8(a + 0x91) & ~0x08) | 0x04)
                    end
                end
                emu:write8(a + 6, flags)
            end
        end
    end
    for _, pile in ipairs(opts.setupPiles or {}) do
        local p = pile.player == 1 and 1 or 0
        local base = playerBase(p)
        local ids = {}
        for _, n in ipairs(pile.names or {}) do
            local id = cardId(n)
            if id then
                ids[#ids + 1] = id
            end
        end
        if pile.pile == "deck" then
            -- the listed cards go on top, the rest of the Deck stays below them
            local rest = {}
            for i = 0, emu:read8(base + 3) - 1 do
                rest[#rest + 1] = emu:read32(base + EDS.OFFSET_DECK + i * 4)
            end
            local n = 0
            for _, id in ipairs(ids) do
                writeCard(base + EDS.OFFSET_DECK + n * 4, id, p)
                n = n + 1
            end
            for _, w in ipairs(rest) do
                if n >= 80 then
                    break
                end
                emu:write32(base + EDS.OFFSET_DECK + n * 4, w)
                n = n + 1
            end
            emu:write8(base + 3, n)
        else
            for i, id in ipairs(ids) do
                writeCard(base + PILE_OFFSET[pile.pile] + (i - 1) * 4, id, p)
                if pile.pile == "banished" then
                    emu:write16(base + 0xCC4 + (i - 1) * 2, 0)    -- BANISH_NORMAL
                end
            end
            emu:write8(base + PILE_COUNT[pile.pile], #ids)
        end
    end
    local st = opts.start or {}
    if st.lp then
        emu:write16(EDS.PLAYER0, st.lp[1])
        emu:write16(EDS.PLAYER1, st.lp[2])
    end
    if st.turnCount then
        if st.turn == "opp" then
            emu:write16(G_DUEL_TURN_COUNT, math.max(st.turnCount - 1, 0))
            EDS.setPlayerSkipDraw(EDS.PLAYER1, true)    -- the CPU's hand stays as set
        else
            emu:write16(G_DUEL_TURN_COUNT, st.turnCount)
        end
    end
    EDS.requestRedraw()            -- the field shows the cards written here at once (ModTestingTick)
    EDS.log(string.format(
        "Exact setup: P0 hand=%d deck=%d gy=%d, P1 hand=%d deck=%d gy=%d, LP %d/%d, turnCount %d.",
        emu:read8(EDS.PLAYER0 + 2), emu:read8(EDS.PLAYER0 + 3), emu:read8(EDS.PLAYER0 + 4),
        emu:read8(EDS.PLAYER1 + 2), emu:read8(EDS.PLAYER1 + 3), emu:read8(EDS.PLAYER1 + 4),
        emu:read16(EDS.PLAYER0), emu:read16(EDS.PLAYER1), emu:read16(G_DUEL_TURN_COUNT)))
end

-- The exact setup is applied in Main Phase 1, once its banner is done and nothing runs (so no Standby Phase
-- effect sees it early).
function EDS.canApplyExactSetup()
    return EDS.turnPlayer() == 0 and EDS.duelStep() == EDS.DUEL_STEP_MAIN
        and EDS.duelPhaseStep() == MAIN_STEP_FIELD and not EDS.chainOrEffectBusy()
        and not textBox().active and not EDS.cardMenuOpen()
end

end
