param([string]$Out, [switch]$Info, [string]$Dir = "", [double]$Seconds = 0, [int]$IntervalMs = 300)
# -Dir/-Seconds: burst mode, saving Dir\b0000.png, b0001.png, ... every IntervalMs for Seconds.
# Renders the Hex Maniac Advance main window into a PNG with PrintWindow (works when the window is covered).
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class HmaWin {
   [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
   [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
   [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
   [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
   [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
   [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
   [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
   [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
   public delegate bool EnumProc(IntPtr h, IntPtr lParam);
   [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr lParam);
   // the process's visible top-level windows, top of the z-order first
   public static System.Collections.Generic.List<IntPtr> ProcessWindows(uint pid) {
      var list = new System.Collections.Generic.List<IntPtr>();
      EnumWindows((h, l) => { uint p; GetWindowThreadProcessId(h, out p); if (p == pid && IsWindowVisible(h)) list.Add(h); return true; }, IntPtr.Zero);
      return list;
   }
}
"@
[HmaWin]::SetProcessDPIAware() | Out-Null
$p = Get-Process HexManiacAdvance | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $p) { throw "HexManiacAdvance window not found" }
$h = $p.MainWindowHandle
if ([HmaWin]::IsIconic($h)) { [HmaWin]::ShowWindow($h, 9) | Out-Null; Start-Sleep -Milliseconds 500 }
$r = New-Object HmaWin+RECT
[HmaWin]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.Right - $r.Left; $hgt = $r.Bottom - $r.Top
if ($Info) { "window $($r.Left),$($r.Top) ${w}x${hgt}"; return }
function Shot($hwnd, $width, $height) {
   $b = New-Object System.Drawing.Bitmap $width, $height
   $gg = [System.Drawing.Graphics]::FromImage($b)
   $dc = $gg.GetHdc()
   [HmaWin]::PrintWindow($hwnd, $dc, 2) | Out-Null
   $gg.ReleaseHdc($dc); $gg.Dispose()
   return $b
}
function Grab($path) {
   $bmp = Shot $h $w $hgt
   # dialogs and other windows of HMA, drawn over the main window where they sit on screen (bottom of the z-order first)
   $others = [HmaWin]::ProcessWindows([uint32]$p.Id) | Where-Object { $_ -ne $h }
   if ($others) {
      $g = [System.Drawing.Graphics]::FromImage($bmp)
      [array]::Reverse($others)
      foreach ($o in @($others)) {
         $orc = New-Object HmaWin+RECT
         [HmaWin]::GetWindowRect($o, [ref]$orc) | Out-Null
         $ow = $orc.Right - $orc.Left; $oh = $orc.Bottom - $orc.Top
         if ($ow -le 0 -or $oh -le 0) { continue }
         $ob = Shot $o $ow $oh
         $g.DrawImage($ob, $orc.Left - $r.Left, $orc.Top - $r.Top, $ow, $oh)
         $ob.Dispose()
      }
      $g.Dispose()
   }
   $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
   $bmp.Dispose()
}
if ($Dir) {
   New-Item -ItemType Directory -Force $Dir | Out-Null
   Get-ChildItem $Dir -Filter "b*.png" | Remove-Item -Force
   $end = (Get-Date).AddSeconds($Seconds); $i = 0
   while ((Get-Date) -lt $end) {
      Grab (Join-Path $Dir ("b{0:D4}.png" -f $i)); $i++
      Start-Sleep -Milliseconds $IntervalMs
   }
   "burst: $i frames in $Dir"
   return
}
Grab $Out
"saved $Out ${w}x${hgt}"
