-- Password.png: Breaker the Magical Warrior's password (71413901) typed on the keypad, then GET CARD: the
-- "Buy ...?" question. The keys are reached with the D-pad over gPasswordKeypad's links (read from the ROM).
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")
local PASSWORD = "71413901"
local GET_CARD = 10

M.run("password", function()
    M.toMainMenu()
    M.openMenu(M.MENU.PASSWORD, "CB_Password", 120)
    local pw = M.sym("gPassword")
    local pad = M.sym("gPasswordKeypad")
    local function links(k)
        local b6, b7 = M.read8(pad + k * 8 + 6), M.read8(pad + k * 8 + 7)
        return { { M.KEY.UP, b6 & 15 }, { M.KEY.DOWN, b6 >> 4 }, { M.KEY.LEFT, b7 & 15 }, { M.KEY.RIGHT, b7 >> 4 } }
    end
    local function selected()
        return (M.read8(pw + 0x10) >> 4) & 15
    end
    -- shortest D-pad path from the selected key to `target` (breadth-first over the links)
    local function goTo(target)
        for _ = 1, 4 do
            local from = selected()
            if from == target then
                return
            end
            local prev, queue, seen = {}, { from }, { [from] = true }
            while #queue > 0 do
                local k = table.remove(queue, 1)
                for _, l in ipairs(links(k)) do
                    if not seen[l[2]] then
                        seen[l[2]] = true
                        prev[l[2]] = { k, l[1] }
                        queue[#queue + 1] = l[2]
                    end
                end
            end
            local keys, k = {}, target
            while k ~= from and prev[k] do
                table.insert(keys, 1, prev[k][2])
                k = prev[k][1]
            end
            for _, key in ipairs(keys) do
                M.press(key, nil, 10)
            end
        end
        if selected() ~= target then
            error("keypad: cannot reach key " .. target)
        end
    end
    for i = 1, #PASSWORD do
        goTo(tonumber(PASSWORD:sub(i, i)))
        M.press(M.KEY.A, nil, 14)
    end
    goTo(GET_CARD)
    M.press(M.KEY.A)
    -- the roll, then the reveal (PasswordState.timer, +0x12 bits 4-15, counts to 0x12C), then the question
    M.waitUntil(function() return M.read16(pw + 0x14) ~= 0 end, 900, "the card lookup")
    M.waitUntil(function() return (M.read16(pw + 0x12) >> 4) >= 0x12C end, 900, "the card reveal")
    M.wait(30)
    M.shot("Password")
end)
