--[[ POC play sequences keyed by recordingBasename(card, effect, actor) ]]

local function loadSibling(name)
    local src = debug.getinfo(1, "S").source or ""
    if src:sub(1, 1) == "@" then
        src = src:sub(2)
    end
    src = src:gsub("\\", "/")
    local dir = src:match("^(.*)/[^/]+$") or "tools/testing_scenarios"
    local ok, mod = pcall(dofile, dir .. "/" .. name)
    if ok then
        return mod
    end
    ok, mod = pcall(dofile, "tools/testing_scenarios/" .. name)
    if ok then
        return mod
    end
    error("[testing] cannot open " .. name)
end

local EDS = loadSibling("lib_eds.lua")

local K = {
    A = EDS.KEY_A,
    B = EDS.KEY_B,
    DOWN = EDS.KEY_DOWN,
    UP = EDS.KEY_UP,
    LEFT = EDS.KEY_LEFT,
    RIGHT = EDS.KEY_RIGHT,
    L = EDS.KEY_L,
    R = EDS.KEY_R,
}

local function id(name)
    return EDS.findCardId(name)
end

local P = {}

P.Card_Destruction_Discard_And_Draw_Player = {
    setupCards = {
        { player = 0, zone = "hand", slot = 0, name = "Card Destruction" },
        { player = 0, zone = "hand", slot = 1, name = "Change of Heart" },
        { player = 0, zone = "hand", slot = 2, name = "Summoned Skull" },
        { player = 1, zone = "monster", slot = 0, name = "Summoned Skull" },
        { player = 1, zone = "hand", slot = 0, name = "Summoned Skull" },
        { player = 1, zone = "hand", slot = 1, name = "Dark Hole" },
        { player = 1, zone = "hand", slot = 2, name = "Fissure" },
        { player = 1, zone = "hand", slot = 3, name = "Man-Eater Bug" },
    },
    play = {
        { key = K.A, gap = 40 },
        { wait = 75 },
        { key = K.A, gap = 35 },
        { wait = 90 },
        { key = K.A, gap = 35 },
        { wait = 120 },
        { key = K.A, gap = 35 },
        { wait = 90 },
        { key = K.A, gap = 35 },
        { wait = 240 },
        { key = K.A, gap = 35 },
        { wait = 360 },
    },
}

P.Soul_Exchange_Tribute_Opponent_Monster_Player = {
    setupCards = {
        { player = 0, zone = "hand", slot = 0, name = "Soul Exchange", id = 1000 },
        { player = 0, zone = "hand", slot = 1, name = "Summoned Skull", id = 675 },
        { player = 1, zone = "monster", slot = 0, name = "Summoned Skull", id = 675 },
    },
    -- Soul Exchange → target opponent Summoned Skull → Tribute Summon hand skull (see TESTING.md).
    play = {
        { handOpen = { slot = 0 } },
        { key = K.A, gap = 30 },
        { wait = 50 },
        { key = K.A, gap = 30 },
        { wait = 60 },
        { oppTarget = { zone = 0 } },
        { handPick = { cardId = 675, maxPresses = 64 } },
        { soulExchangeFinish = true },
    },
}

P.Torrential_Tribute_Destroy_All_On_Summon_Player = {
    setupCards = {
        { player = 0, zone = "monster", slot = 0, name = "Mystic Tomato" },
        { player = 0, zone = "st", slot = 0, name = "Torrential Tribute", set = true },
        { player = 1, zone = "monster", slot = 0, name = "Summoned Skull" },
        { player = 1, zone = "st", slot = 0, name = "Man-Eater Bug", set = true },
    },
    play = {
        { wait = 300 },
        { key = K.A, gap = 30 },
        { wait = 240 },
    },
}

P.Exiled_Force_Tribute_Destroy_Target_Player = {
    setupCards = {
        { player = 0, zone = "monster", slot = 0, name = "Exiled Force" },
        { player = 0, zone = "monster", slot = 1, name = "Mystic Tomato" },
        { player = 1, zone = "monster", slot = 0, name = "Summoned Skull" },
        { player = 1, zone = "st", slot = 0, name = "Man-Eater Bug", set = true },
    },
    play = {
        { key = K.A, gap = 25 },
        { wait = 40 },
        { key = K.A, gap = 25 },
        { wait = 120 },
    },
}

P.Goblin_Attack_Force_Usage_Deck_Verify_Player = {
    setupCards = {
        { player = 0, zone = "hand", slot = 0, name = "Goblin Attack Force" },
    },
    play = {
        { key = K.A, gap = 30 },
        { wait = 200 },
        { key = K.A, gap = 30 },
        { wait = 400 },
    },
}

return P
