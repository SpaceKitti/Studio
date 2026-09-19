# Bake M1_RaidStores on TrinityOrb. Does NOT open or modify M0_FireEscape.
$ErrorActionPreference = "Stop"
$UE = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
$Proj = "C:\Users\Akitt\Games\Studio\unreal\FireEscape.uproject"
$Script = "C:\Users\Akitt\Games\Studio\unreal\Tools\BuildM1RaidStores.py"
$LogDir = "C:\Users\Akitt\Games\Studio\unreal\Saved\Logs"
$StudioRoot = "C:\Users\Akitt\Games\Studio"
$m0 = "C:\Users\Akitt\Games\Studio\unreal\Content\Maps\M0_FireEscape.umap"
$m1 = "C:\Users\Akitt\Games\Studio\unreal\Content\Maps\M1_RaidStores.umap"

if (-not (Test-Path $UE)) { throw "Missing UE: $UE" }
if (-not (Test-Path $Proj)) { throw "Missing project: $Proj" }

# Sync latest bake script from origin/main (layout-only Tools path)
if (Test-Path (Join-Path $StudioRoot ".git")) {
  Push-Location $StudioRoot
  try {
    git fetch origin main 2>&1 | Out-Host
    git checkout origin/main -- unreal/Tools/BuildM1RaidStores.py unreal/Tools/RunM1RaidStores.ps1 2>&1 | Out-Host
    Write-Host "Synced Tools bake scripts from origin/main"
  } catch {
    Write-Host "WARN: git sync skipped: $_"
  } finally {
    Pop-Location
  }
}

if (-not (Test-Path $Script)) { throw "Missing script: $Script — copy BuildM1RaidStores.py to Tools\ first" }

$m0Before = $null
if (Test-Path $m0) {
  $m0Before = (Get-Item $m0).LastWriteTime
  Write-Host "M0 LastWriteTime BEFORE bake: $m0Before"
}
$bakeStart = Get-Date
Write-Host "Bake start: $bakeStart"

Get-Process UnrealEditor*,UnrealEditor-Win64* -ErrorAction SilentlyContinue | ForEach-Object {
  Write-Host "Stopping $($_.ProcessName) pid=$($_.Id)"
  Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue
}
Start-Sleep -Seconds 2

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$arg = "-ExecutePythonScript=`"$Script`""
Write-Host "Launching UE bake for M1_RaidStores (viewmode lit required in script)..."
& $UE $Proj $arg -unattended -nosplash -log
Write-Host "ExitCode=$LASTEXITCODE"

if (Test-Path "$LogDir\BuildM1RaidStores.txt") {
  Write-Host "---- BuildM1RaidStores.txt (tail) ----"
  Get-Content "$LogDir\BuildM1RaidStores.txt" -Tail 80
}
if (Test-Path "$LogDir\M1_RaidStores_manifest.json") {
  Write-Host "---- manifest ----"
  Get-Content "$LogDir\M1_RaidStores_manifest.json"
}

if (Test-Path $m0) {
  $m0After = (Get-Item $m0).LastWriteTime
  Write-Host "M0 LastWriteTime AFTER bake: $m0After"
  if ($m0Before -ne $null -and $m0After -eq $m0Before) {
    Write-Host "M0 UNCHANGED: yes"
  } else {
    Write-Host "M0 UNCHANGED: NO (investigate)"
  }
  Get-Item $m0 | Format-List FullName, Length, LastWriteTime
} else {
  Write-Host "M0 umap missing"
}

if (Test-Path $m1) {
  $m1Item = Get-Item $m1
  Write-Host "M1 LastWriteTime: $($m1Item.LastWriteTime) Length=$($m1Item.Length)"
  if ($m1Item.LastWriteTime -gt $bakeStart) {
    Write-Host "M1 UPDATED after bake start: yes"
  } else {
    Write-Host "M1 UPDATED after bake start: NO"
  }
  $m1Item | Format-List FullName, Length, LastWriteTime
} else {
  Write-Host "M1 umap NOT FOUND"
}
