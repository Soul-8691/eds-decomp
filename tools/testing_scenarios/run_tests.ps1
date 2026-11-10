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
  (PASS / FAIL / DONE); a script with no result after -TimeoutSeconds (60) is stopped and counted as TIMEOUT,
  unless its ffmpeg PNG -> MP4 encode is running (then up to -EncodeTimeoutSeconds more). The
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
    # a script with no result by then is stopped (TIMEOUT), unless its PNG -> MP4 encode (ffmpeg) is running:
    # that may take up to EncodeTimeoutSeconds more
    [int]$TimeoutSeconds = 60,
    [int]$EncodeTimeoutSeconds = 600,
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

$dir = Join-Path $scenarios $(if ($Poc) { "poc" } else { "generated" })
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
$scripts = @(Get-ChildItem -Path $dir -Filter "*.lua" | Where-Object { $_.BaseName -like $pattern } |
             Sort-Object @{ Expression = { if ($order.ContainsKey($_.BaseName)) { $order[$_.BaseName] } else { [int]::MaxValue } } },
                         Name)
# kept videos (review_videos.py moved them to recordings/keep/) are not recorded again
$keepDir = Join-Path $scenarios "recordings\keep"
if (-not $IncludeKept -and (Test-Path $keepDir)) {
    $kept = @($scripts | Where-Object { Test-Path (Join-Path $keepDir ($_.BaseName + ".mp4")) })
    if ($kept.Count -gt 0) {
        Write-Host ("Skipping {0} kept script(s) (recordings\keep; -IncludeKept runs them)" -f $kept.Count)
        $scripts = @($scripts | Where-Object { -not (Test-Path (Join-Path $keepDir ($_.BaseName + ".mp4"))) })
    }
}
if ($scripts.Count -eq 0) {
    Write-Host "No script matches '$Filter' in $dir"
    exit 1
}
if (-not (Test-Path $Mgba)) { throw "mGBA not found: $Mgba" }
if (-not (Test-Path (Join-Path $repo $Rom))) { throw "ROM not found: $Rom (build it with make demo)" }

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
    $args = @('-C', 'pauseOnMinimize=0', '-C', 'pauseOnFocusLost=0',
              '-C', 'audioSync=0', '-C', 'videoSync=0', '-C', 'fpsTarget=6000',
              '--script', $s.FullName, $Rom)
    if ($Visible) {
        $p = Start-Process -FilePath $Mgba -ArgumentList $args -WindowStyle Minimized -PassThru
    } else {
        $cmdLine = (@($Mgba) + $args | ForEach-Object { '"' + $_ + '"' }) -join ' '
        $p = Get-Process -Id ([EdsDesk]::Start("EdsTests", $Mgba, $cmdLine, $repo))
    }
    $result = "TIMEOUT"
    $detail = ""
    $tick = 0
    while ($true) {
        Start-Sleep -Milliseconds 100
        $tick++
        if ($tick % 10 -ne 0) { continue }      # (the result check once a second)
        $elapsed = ((Get-Date) - $start).TotalSeconds
        if ($elapsed -ge $TimeoutSeconds) {
            # past the timeout, only the PNG -> MP4 encode (the script's ffmpeg) may still finish
            $encoding = [bool](Get-Process ffmpeg -ErrorAction SilentlyContinue)
            if (-not $encoding -or $elapsed -ge $TimeoutSeconds + $EncodeTimeoutSeconds) { break }
        }
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
    $results += [pscustomobject]@{ Script = $name; Result = $result; Seconds = $secs; Detail = $detail }
}

$summary = Join-Path $rec "_summary.txt"
$results | Format-Table -AutoSize | Out-String -Width 200 | Set-Content -Encoding utf8 $summary
Write-Host ""
$results | Group-Object Result | ForEach-Object { Write-Host ("{0}: {1}" -f $_.Name, $_.Count) }
Write-Host "Summary: $summary (per-step log: recordings\_console.log)"
