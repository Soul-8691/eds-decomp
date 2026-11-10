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
  (PASS / FAIL / DONE); a script with no result after -TimeoutSeconds is stopped and counted as TIMEOUT. The
  summary also goes to recordings/_summary.txt.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1 -Filter DD_Assailant
  powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1 -Filter "*_CPU"
  powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1            # every generated script
  powershell -ExecutionPolicy Bypass -File tools\testing_scenarios\run_tests.ps1 -Poc       # the poc/ scripts
#>
param(
    # wildcard on the script names (without .lua); "*" runs them all
    [string]$Filter = "*",
    # run the poc/ scripts instead of generated/
    [switch]$Poc,
    [int]$TimeoutSeconds = 300,
    [string]$Mgba = "C:\Program Files\mGBA\mGBA.exe",
    [string]$Rom = "eds_demo.gba",
    # run mGBA on your own desktop (minimized) instead of the separate one
    [switch]$Visible
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
$scripts = @(Get-ChildItem -Path $dir -Filter "*.lua" | Where-Object { $_.BaseName -like $pattern } | Sort-Object Name)
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
    while (((Get-Date) - $start).TotalSeconds -lt $TimeoutSeconds) {
        Start-Sleep -Milliseconds 100
        $tick++
        if ($tick % 10 -ne 0) { continue }      # (the result check once a second)
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
