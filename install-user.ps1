# AMD Global Frame Generator - User Directory Installation
# Installs to user AppData (no admin required)

$InstallPath = "$env:APPDATA\AMDGlobalFG"
$SourceExe = ".\build\bin\afglobal.exe"
$SourceConfig = ".\afglobal_config.json"  
$SourceLicense = ".\LICENSE"

Write-Host "AMD Global Frame Generator - Installation" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Path $SourceExe)) {
    Write-Host "ERROR: afglobal.exe not found" -ForegroundColor Red
    exit 1
}

Write-Host "Installing to: $InstallPath" -ForegroundColor Yellow

New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
Copy-Item $SourceExe -Destination "$InstallPath\afglobal.exe" -Force
Copy-Item $SourceConfig -Destination "$InstallPath\afglobal_config.json" -Force -ErrorAction SilentlyContinue
Copy-Item $SourceLicense -Destination "$InstallPath\LICENSE" -Force -ErrorAction SilentlyContinue

Write-Host "Files installed successfully" -ForegroundColor Green
Write-Host ""
Write-Host "To run:"
Write-Host "$InstallPath\afglobal.exe" -ForegroundColor Yellow
Write-Host ""
Write-Host "Creating desktop shortcut..." -ForegroundColor Cyan

$WshShell = New-Object -ComObject WScript.Shell
$DesktopPath = [System.Environment]::GetFolderPath("Desktop")
$ShortcutPath = Join-Path $DesktopPath "AMDGlobalFG.lnk"

$Shortcut = $WshShell.CreateShortcut($ShortcutPath)
$Shortcut.TargetPath = "$InstallPath\afglobal.exe"
$Shortcut.WorkingDirectory = $InstallPath
$Shortcut.Save()

Write-Host "Desktop shortcut created" -ForegroundColor Green
Write-Host ""
Write-Host "Installation Complete!" -ForegroundColor Green
