# Bake M1_RaidStores on TrinityOrb. Does NOT open or modify M0_FireEscape.
$ErrorActionPreference = "Stop"
$UE = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
$Proj = "C:\Users\Akitt\Games\Studio\unreal\FireEscape.uproject"
$Script = "C:\Users\Akitt\Games\Studio\unreal\Tools\BuildM1RaidStores.py"
$LogDir = "C:\Users\Akitt\Games\Studio\unreal\Saved\Logs"

if (-not (Test-Path $UE)) { throw "Missing UE: $UE" }
if (-not (Test-Path $Proj)) { throw "Missing project: $Proj" }
if (-not (Test-Path $Script)) { throw "Missing script: $Script — copy BuildM1RaidStores.py to Tools\ first" }

# Kill any existing editor so we don't attach to M0
Get-Process UnrealEditor*,UnrealEditor-Win64* -ErrorAction SilentlyContinue | ForEach-Object {
  Write-Host "Stopping $($_.ProcessName) pid=$($_.Id)"
  Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue
}
Start-Sleep -Seconds 2

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$arg = "-ExecutePythonScript=`"$Script`""
Write-Host "Launching UE bake for M1_RaidStores..."
& $UE $Proj $arg -unattended -nosplash -log
Write-Host "ExitCode=$LASTEXITCODE"
if (Test-Path "$LogDir\BuildM1RaidStores.txt") {
  Write-Host "---- BuildM1RaidStores.txt (tail) ----"
  Get-Content "$LogDir\BuildM1RaidStores.txt" -Tail 40
}
if (Test-Path "$LogDir\M1_RaidStores_manifest.json") {
  Write-Host "---- manifest ----"
  Get-Content "$LogDir\M1_RaidStores_manifest.json"
}
# Confirm M0 map file timestamp untouched by reporting hashes/sizes
$m0 = "C:\Users\Akitt\Games\Studio\unreal\Content\Maps\M0_FireEscape.umap"
$m1 = "C:\Users\Akitt\Games\Studio\unreal\Content\Maps\M1_RaidStores.umap"
if (Test-Path $m0) { Get-Item $m0 | Select-Object FullName, Length, LastWriteTime }
if (Test-Path $m1) { Get-Item $m1 | Select-Object FullName, Length, LastWriteTime } else { Write-Host "M1 umap NOT FOUND" }
