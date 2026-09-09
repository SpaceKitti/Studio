# Fire Escape — Milestone 0 (Unreal 5.8)
$ErrorActionPreference = "Stop"
$UeRoot = "C:\Program Files\Epic Games\UE_5.8"
$Project = Join-Path $PSScriptRoot "FireEscape.uproject"
$Build = Join-Path $UeRoot "Engine\Build\BatchFiles\Build.bat"
$Editor = Join-Path $UeRoot "Engine\Binaries\Win64\UnrealEditor.exe"

if (-not (Test-Path $Project)) { throw "Missing $Project" }
if (-not (Test-Path $Build)) { throw "Unreal 5.8 not found at $UeRoot" }

Write-Host "Building FireEscapeEditor (Development Win64)..."
& $Build FireEscapeEditor Win64 Development -Project="$Project" -WaitMutex
if ($LASTEXITCODE -ne 0) { throw "Build failed with code $LASTEXITCODE" }

Write-Host "Launching M0 (game viewport)..."
& $Editor $Project "/Engine/Maps/Templates/Template_Default" -game -windowed -log
