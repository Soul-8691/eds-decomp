-- README header: screenshots/Title Screen.png (the title screen once its flame has drawn).
local M = dofile((script and script.dir or "tools/media") .. "/lib_media.lua")
M.run("title_screen", function()
    M.toTitle(240)
    M.shot("Title Screen")
end)
