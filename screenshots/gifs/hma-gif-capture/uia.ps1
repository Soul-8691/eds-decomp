param(
   [string]$Action = "list",      # list | invoke | expand | settext | focus
   [string]$Name = "",            # element name to find (exact) for invoke/expand/settext
   [string]$Text = "",            # text for settext
   [string]$Type = "",            # optional ControlType filter, e.g. Button, Edit
   [int]$Depth = 6,
   [string]$Window = "Hex Maniac Advance"  # top-level window title prefix
)
# Drives the HMA GUI through Windows UI Automation (no mouse/keyboard involved).
Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes
$root = [System.Windows.Automation.AutomationElement]::RootElement
$windows = $root.FindAll([System.Windows.Automation.TreeScope]::Children, [System.Windows.Automation.Condition]::TrueCondition)
$win = $null
foreach ($w in $windows) { if ($w.Current.Name -like "$Window*") { $win = $w; break } }
if (-not $win) { throw "window '$Window' not found" }

function Find-All($el) {
   $cond = [System.Windows.Automation.Condition]::TrueCondition
   $el.FindAll([System.Windows.Automation.TreeScope]::Descendants, $cond)
}

if ($Action -eq "list") {
   $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
   function Walk($el, $d) {
      if ($d -gt $Depth) { return }
      $c = $walker.GetFirstChild($el)
      while ($c) {
         $n = $c.Current.Name; $t = $c.Current.ControlType.ProgrammaticName -replace 'ControlType\.', ''
         if (-not $Type -or $t -eq $Type) { if ($n -or $t -eq $Type) { ("  " * $d) + "$t '$n' [$($c.Current.AutomationId)]" } }
         Walk $c ($d + 1)
         $c = $walker.GetNextSibling($c)
      }
   }
   Walk $win 0
   return
}

$matches = @(Find-All $win | Where-Object { $_.Current.Name -eq $Name -and (-not $Type -or ($_.Current.ControlType.ProgrammaticName -replace 'ControlType\.', '') -eq $Type) })
if ($matches.Count -eq 0) { throw "no element named '$Name'" }
$el = $matches[0]
switch ($Action) {
   "invoke" {
      $p = $null
      if ($el.TryGetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern, [ref]$p)) { $p.Invoke(); "invoked '$Name'" }
      elseif ($el.TryGetCurrentPattern([System.Windows.Automation.TogglePattern]::Pattern, [ref]$p)) { $p.Toggle(); "toggled '$Name'" }
      elseif ($el.TryGetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern, [ref]$p)) { $p.Expand(); "expanded '$Name'" }
      else { throw "'$Name' has no invoke/toggle/expand pattern" }
   }
   "expand" { $p = $el.GetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern); $p.Expand(); "expanded '$Name'" }
   "settext" { $p = $el.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern); $p.SetValue($Text); "set '$Name' = '$Text'" }
   "focus" { $el.SetFocus(); "focused '$Name'" }
}
