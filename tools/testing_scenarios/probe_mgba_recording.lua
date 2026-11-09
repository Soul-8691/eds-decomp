--[[ One-shot: print which recording-related APIs exist in this mGBA build.
    mGBA.exe --script tools/testing_scenarios/probe_mgba_recording.lua eds_demo.gba ]]
local function log(msg)
    console:log("[probe] " .. msg)
end

if not emu then
    log("No emu yet — load a ROM first, then re-run or use --script with a .gba path.")
    return
end

local names = {
    "screenshot", "screenshotToImage", "startRecording", "stopRecording",
    "beginVideoLog", "endVideoLog", "videoRecord",
}
for _, n in ipairs(names) do
  if emu[n] then
    log("emu." .. n .. " exists")
  else
    log("emu." .. n .. " missing")
  end
end

for _, top in ipairs({ "callbacks", "console", "canvas", "storage", "input" }) do
    if _G[top] then
        log(top .. " present")
    end
end

log("Official docs: https://mgba.io/docs/dev/scripting.html — only screenshot is defined for video output.")
log("EDS testing uses emu:screenshot() + ffmpeg when captureMp4 is enabled (default if ffmpeg on PATH).")
