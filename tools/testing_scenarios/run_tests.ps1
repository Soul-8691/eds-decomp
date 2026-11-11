<#
.SYNOPSIS
  Runs TESTING.md scenario scripts in mGBA, out of sight, one after another, and prints a PASS/FAIL summary.

.DESCRIPTION
  Each script runs in its own mGBA at full speed, started on a separate Win32 desktop ("EdsTests", made with
  CreateDesktop; not a Task View virtual desktop): its windows are shown there, so it runs at full speed (a hidden
  mGBA runs at real time only), but they never appear on your screen or taskbar, not even briefly.
  pauseOnMinimize=0 and pauseOnFocusLost=0 keep it running unfocused. The scripts' screenshots come from the emulator, not the window, so capture still
  works. -Visible runs it on your desktop instead (minimized).
  The result is the script's line in recordings/_last_run.log
  (PASS / FAIL / DONE); a script that makes no progress for -TimeoutSeconds (60: its log does not grow and no
  ffmpeg encode runs), or runs longer than -MaxSeconds (60), is stopped and counted as TIMEOUT, and marked
  recordings/throwaway/<script>.toolong so it is not run again. -RerecordKept records the kept and highlighted
  scripts again, replacing their videos. The
  summary also goes to recordings/_summary.txt.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1 -Filter DD_Assailant
  powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1 -Filter "*_CPU"
  powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1            # every generated script
  powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1 -Poc       # the poc/ scripts
  Scripts whose video is in recordings\keep (marked with review_videos.py) are skipped unless -IncludeKept.
#>
param(
    # wildcard on the script names (without .lua); "*" runs them all
    [string]$Filter = "*",
    # run the poc/ scripts instead of generated/
    [switch]$Poc,
    # run the README media capture scripts (tools/media) instead of the test scenarios
    [switch]$Media,
    # -Media: the save to copy for the captures (default: the ROM's own .sav), e.g. eds_demo.sav with a release ROM
    [string]$Save = "",
    # seconds without progress (no new log line, no ffmpeg encode) before a script is stopped as TIMEOUT
    [int]$TimeoutSeconds = 60,
    # the most a script may run in all: a video that takes longer is not wanted (a TIMEOUT leaves the marker
    # recordings/throwaway/<script>.toolong, so the script is skipped from then on)
    [int]$MaxSeconds = 60,
    # record the kept and highlighted scripts again (recordings/keep/, keep/highlights/): each new video replaces the
    # old one in its folder
    [switch]$RerecordKept,
    # only these script names (e.g. the ones that failed last time); empty: every script matching -Filter
    [string[]]$Names = @(),
    [string]$Mgba = "C:\Program Files\mGBA\mGBA.exe",
    [string]$Rom = "eds_demo.gba",
    # run mGBA on your own desktop (minimized) instead of the separate one
    [switch]$Visible,
    # also run the scripts whose video was kept (recordings/keep/, review_videos.py); they are skipped otherwise
    [switch]$IncludeKept
)

# Win32: start mGBA on a desktop of its own (CreateDesktop, then CreateProcess with lpDesktop): windows there are
# real, shown windows (so mGBA runs at full speed; hidden, it ran at real time), but Explorer and the taskbar live on
# the Default desktop and never see them, not even for the moment a window takes to open.
Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public static class EdsDesk {
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    struct STARTUPINFO {
        public int cb; public string lpReserved; public string lpDesktop; public string lpTitle;
        public int dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
        public short wShowWindow, cbReserved2; public IntPtr lpReserved2, hStdInput, hStdOutput, hStdError;
    }
    [StructLayout(LayoutKind.Sequential)]
    struct PROCESS_INFORMATION { public IntPtr hProcess, hThread; public int dwProcessId, dwThreadId; }
    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern IntPtr CreateDesktop(string name, IntPtr device, IntPtr devmode, int flags, uint access, IntPtr sa);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern bool CreateProcess(string app, string cmdLine, IntPtr pa, IntPtr ta, bool inherit, uint flags,
                                     IntPtr env, string dir, ref STARTUPINFO si, out PROCESS_INFORMATION pi);
    [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr h);
    static IntPtr desk = IntPtr.Zero;
    public static int Start(string desktopName, string app, string cmdLine, string dir) {
        if (desk == IntPtr.Zero) {
            desk = CreateDesktop(desktopName, IntPtr.Zero, IntPtr.Zero, 0, 0x10000000 /* GENERIC_ALL */, IntPtr.Zero);
            if (desk == IntPtr.Zero)
                throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
        }
        STARTUPINFO si = new STARTUPINFO();
        si.cb = Marshal.SizeOf(typeof(STARTUPINFO));
        si.lpDesktop = desktopName;
        PROCESS_INFORMATION pi;
        if (!CreateProcess(app, cmdLine, IntPtr.Zero, IntPtr.Zero, false, 0, IntPtr.Zero, dir, ref si, out pi))
            throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return pi.dwProcessId;
    }
}
"@


$ErrorActionPreference = "Stop"
$scenarios = $PSScriptRoot
$repo = (Resolve-Path (Join-Path $scenarios "..\..")).Path
Set-Location $repo

$dir = if ($Media) { Join-Path $scenarios "..\media" } else { Join-Path $scenarios $(if ($Poc) { "poc" } else { "generated" }) }
$pattern = if ($Filter.Contains("*") -or $Filter.Contains("?")) { $Filter } else { "*$Filter*" }
# TESTING.md's order (top to bottom: each scenario's line starts with its script name in backticks); scripts it does
# not name come last, by name
$order = @{}
$testingMd = Join-Path $repo "TESTING.md"
if (Test-Path $testingMd) {
    $i = 0
    foreach ($line in Get-Content $testingMd -Encoding UTF8) {
        if ($line -match '^`([A-Za-z0-9_]+)`') {
            if (-not $order.ContainsKey($Matches[1])) { $order[$Matches[1]] = $i++ }
        }
    }
}
$scripts = @(Get-ChildItem -Path $dir -Filter "*.lua" | Where-Object { $_.BaseName -like $pattern -and $_.BaseName -notlike "lib_*" } |
             Where-Object { $Names.Count -eq 0 -or $Names -contains $_.BaseName } |
             Sort-Object @{ Expression = { if ($order.ContainsKey($_.BaseName)) { $order[$_.BaseName] } else { [int]::MaxValue } } },
                         Name)
# kept, highlighted and thrown-away videos (review_videos.py moved them to recordings/keep/, recordings/keep/highlights/
# or recordings/throwaway/) are not recorded again
$doneDirs = @((Join-Path $scenarios "recordings\keep"), (Join-Path $scenarios "recordings\keep\highlights"),
              (Join-Path $scenarios "recordings\throwaway"))
$throwDir = Join-Path $scenarios "recordings\throwaway"
# the folder of a script's kept or highlighted video, or $null
function Get-KeptDir($script) {
    foreach ($d in $doneDirs[0..1]) { if (Test-Path (Join-Path $d ($script.BaseName + ".mp4"))) { return $d } }
    return $null
}
# kept, highlighted, thrown away (a video), or too long (a .toolong marker in throwaway/)
function Test-Reviewed($script) {
    if (Get-KeptDir $script) { return $true }
    return [bool](Get-ChildItem -Path $throwDir -Filter ($script.BaseName + ".*") -ErrorAction SilentlyContinue)
}
if ($RerecordKept) {
    $scripts = @($scripts | Where-Object { Get-KeptDir $_ })
    Write-Host ("Re-recording {0} kept or highlighted script(s)" -f $scripts.Count)
} elseif (-not $IncludeKept -and -not $Media) {
    $reviewed = @($scripts | Where-Object { Test-Reviewed $_ })
    if ($reviewed.Count -gt 0) {
        Write-Host ("Skipping {0} kept, highlighted or thrown-away script(s) (recordings\keep, keep\highlights, throwaway; -IncludeKept runs them)" -f $reviewed.Count)
        $scripts = @($scripts | Where-Object { -not (Test-Reviewed $_) })
    }
}
if ($scripts.Count -eq 0) {
    Write-Host "No script matches '$Filter' in $dir"
    exit 1
}
if (-not (Test-Path $Mgba)) { throw "mGBA not found: $Mgba" }
if (-not (Test-Path (Join-Path $repo $Rom))) { throw "ROM not found: $Rom (build it with make demo)" }

if ($Media) {
    # lib_media.lua reads the ROM name for its symbol file (<rom>.sym)
    Set-Content -Encoding ascii -Path (Join-Path $dir "_rom.txt") -Value ([IO.Path]::GetFileNameWithoutExtension($Rom))
}
$rec = Join-Path $scenarios "recordings"
New-Item -ItemType Directory -Force $rec | Out-Null
$runLog = Join-Path $rec "_last_run.log"

Write-Host ("{0} script(s) from {1}" -f $scripts.Count, $dir)
$results = @()
$n = 0
foreach ($s in $scripts) {
    $n++
    $name = $s.BaseName
    $before = if (Test-Path $runLog) { @(Get-Content $runLog).Count } else { 0 }
    $start = Get-Date
    Write-Host -NoNewline ("[{0}/{1}] {2} ... " -f $n, $scripts.Count, $name)
    # -Media: the captures load recipes, buy packs and so on, so each runs a copy of the ROM and of its save
    # (recordings/_media_rom/): your save is read, never written
    $romArg = $Rom
    if ($Media) {
        $mediaRom = Join-Path $rec "_media_rom"
        New-Item -ItemType Directory -Force $mediaRom | Out-Null
        Copy-Item -Force (Join-Path $repo $Rom) (Join-Path $mediaRom (Split-Path $Rom -Leaf))
        $save = if ($Save) { Join-Path $repo $Save } else { Join-Path $repo ([IO.Path]::ChangeExtension($Rom, ".sav")) }
        $saveCopy = Join-Path $mediaRom ([IO.Path]::ChangeExtension((Split-Path $Rom -Leaf), ".sav"))
        if (Test-Path $save) { Copy-Item -Force $save $saveCopy } elseif (Test-Path $saveCopy) { Remove-Item -Force $saveCopy }
        $romArg = Join-Path $mediaRom (Split-Path $Rom -Leaf)
    }
    $args = @('-C', 'pauseOnMinimize=0', '-C', 'pauseOnFocusLost=0',
              '-C', 'audioSync=0', '-C', 'videoSync=0', '-C', 'fpsTarget=6000',
              '--script', $s.FullName, $romArg)
    if ($Visible) {
        $p = Start-Process -FilePath $Mgba -ArgumentList $args -WindowStyle Minimized -PassThru
    } else {
        $cmdLine = (@($Mgba) + $args | ForEach-Object { '"' + $_ + '"' }) -join ' '
        $p = Get-Process -Id ([EdsDesk]::Start("EdsTests", $Mgba, $cmdLine, $repo))
    }
    $result = "TIMEOUT"
    $detail = ""
    $tick = 0
    $consoleLog = Join-Path $rec "_console.log"
    $lastSize = if (Test-Path $consoleLog) { (Get-Item $consoleLog).Length } else { 0 }
    $lastProgress = Get-Date
    while ($true) {
        Start-Sleep -Milliseconds 100
        $tick++
        if ($tick % 10 -ne 0) { continue }      # (the result check once a second)
        # Progress: the script's log grows (each step, and a stuck step's own time-out, logs a line) or its
        # PNG -> MP4 encode (ffmpeg) runs. TimeoutSeconds without any is a TIMEOUT; MaxSeconds caps the whole run.
        $size = if (Test-Path $consoleLog) { (Get-Item $consoleLog).Length } else { 0 }
        $encoding = [bool](Get-Process ffmpeg -ErrorAction SilentlyContinue)
        if ($size -ne $lastSize -or $encoding) {
            $lastSize = $size
            $lastProgress = Get-Date
        }
        $elapsed = ((Get-Date) - $start).TotalSeconds
        if (((Get-Date) - $lastProgress).TotalSeconds -ge $TimeoutSeconds -or ($elapsed -ge $MaxSeconds -and -not $encoding)) { break }
        if (Test-Path $runLog) {
            $new = @(Get-Content $runLog | Select-Object -Skip $before)
            $line = $new | Where-Object { $_ -match "^(PASS|FAIL|DONE) $([regex]::Escape($name))\b" } | Select-Object -Last 1
            if ($line) {
                $result = $Matches[1]
                $detail = ($line -replace "^\S+ \S+\s*", "")
                break
            }
        }
        if ($p.HasExited) {
            $result = "EXITED"
            break
        }
    }
    if (-not $p.HasExited) {
        Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
    }

    if ($result -eq "TIMEOUT") {
        Get-Process ffmpeg -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
    }
    $secs = [int]((Get-Date) - $start).TotalSeconds
    $color = switch ($result) { "PASS" { "Green" } "DONE" { "Yellow" } default { "Red" } }
    Write-Host ("{0} ({1}s)" -f $result, $secs) -ForegroundColor $color
    if (-not $Media) {
        $video = Join-Path $rec ($name + ".mp4")
        if ($RerecordKept -and $result -eq "PASS" -and (Test-Path $video)) {
            # the new video replaces the kept one, in its folder; if that one is open (the review player), the new
            # video stays in recordings/ and the run goes on
            $keptVideo = Join-Path (Get-KeptDir $s) ($name + ".mp4")
            try {
                Copy-Item -Force $video $keptVideo -ErrorAction Stop
                Remove-Item -Force $video -ErrorAction Stop
            } catch {
                Write-Host ("  (could not replace {0}: {1}; the new video is in recordings\)" -f $keptVideo, $_.Exception.Message) -ForegroundColor Yellow
            }
        } elseif ($result -eq "TIMEOUT" -and -not $RerecordKept) {
            # too long to be wanted: never recorded again (review_videos.py lists only videos, not the marker)
            New-Item -ItemType Directory -Force $throwDir | Out-Null
            Set-Content -Path (Join-Path $throwDir ($name + ".toolong")) -Value ("TIMEOUT after {0}s" -f $secs)
        }
    }
    $results += [pscustomobject]@{ Script = $name; Result = $result; Seconds = $secs; Detail = $detail }
}

$summary = Join-Path $rec "_summary.txt"
$results | Format-Table -AutoSize | Out-String -Width 200 | Set-Content -Encoding utf8 $summary
Write-Host ""
$results | Group-Object Result | ForEach-Object { Write-Host ("{0}: {1}" -f $_.Name, $_.Count) }
Write-Host "Summary: $summary (per-step log: recordings\_console.log)"
