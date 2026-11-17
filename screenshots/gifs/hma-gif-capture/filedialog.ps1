param([string]$Path, [int]$TimeoutSec = 40, [int]$HoldMs = 1800)
# Waits for HexManiacAdvance's Open file dialog, types $Path into "File name" (UI Automation), waits, presses Open.
Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes
$A = [System.Windows.Automation.AutomationElement]
$pidHma = (Get-Process HexManiacAdvance | Select-Object -First 1).Id
$end = (Get-Date).AddSeconds($TimeoutSec)
$dlg = $null
while (-not $dlg -and (Get-Date) -lt $end) {
   $cond = New-Object System.Windows.Automation.PropertyCondition($A::ProcessIdProperty, $pidHma)
   foreach ($w in $A::RootElement.FindAll([System.Windows.Automation.TreeScope]::Children, $cond)) {
      if ($w.Current.ClassName -eq '#32770') { $dlg = $w; break }
   }
   if (-not $dlg) { Start-Sleep -Milliseconds 200 }
}
if (-not $dlg) { "no dialog"; exit 1 }
Start-Sleep -Milliseconds 900
$editCond = New-Object System.Windows.Automation.AndCondition(
   (New-Object System.Windows.Automation.PropertyCondition($A::ControlTypeProperty, [System.Windows.Automation.ControlType]::Edit)),
   (New-Object System.Windows.Automation.PropertyCondition($A::NameProperty, 'File name:')))
$edit = $dlg.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $editCond)
if (-not $edit) { "no file name box"; exit 1 }
$edit.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue($Path)
"typed path"
Start-Sleep -Milliseconds $HoldMs
$btnCond = New-Object System.Windows.Automation.AndCondition(
   (New-Object System.Windows.Automation.PropertyCondition($A::ControlTypeProperty, [System.Windows.Automation.ControlType]::Button)),
   (New-Object System.Windows.Automation.PropertyCondition($A::AutomationIdProperty, '1')))
$open = $dlg.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $btnCond)
if (-not $open) { "no Open button"; exit 1 }
$open.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
"pressed Open"
