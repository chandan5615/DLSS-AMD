# AMD Global Frame Generator - Manual Installation Script
# Use this if the installer is blocked by Application Control policy

param(
    [string]$InstallPath = "C:\Program Files\AMDGlobalFG",
    [switch]$CreateDesktopShortcut = $true,
    [switch]$AddToStartup = $true
)

Write-Host "AMD Global Frame Generator - Installation Script" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

# Check if running as administrator
$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent()
$principal = New-Object System.Security.Principal.WindowsPrincipal($currentUser)
$isAdmin = $principal.IsInRole([System.Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "⚠ This script should be run as Administrator for best results." -ForegroundColor Yellow
    Write-Host "   Right-click PowerShell and select 'Run as Administrator'" -ForegroundColor Yellow
    Write-Host ""
}

# Verify source files exist
$sourceExe = ".\build\bin\afglobal.exe"
$sourceConfig = ".\afglobal_config.json"
$sourceLicense = ".\LICENSE"

if (-not (Test-Path $sourceExe)) {
    Write-Host "ERROR: afglobal.exe not found at $sourceExe" -ForegroundColor Red
    exit 1
}

Write-Host "Step 1: Creating installation directory..." -ForegroundColor Green
try {
    New-Item -ItemType Directory -Path $InstallPath -Force -ErrorAction Stop | Out-Null
    Write-Host "✓ Directory created: $InstallPath" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Failed to create directory: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Step 2: Copying application files..." -ForegroundColor Green
try {
    Copy-Item $sourceExe -Destination "$InstallPath\afglobal.exe" -Force -ErrorAction Stop
    Write-Host "✓ Copied afglobal.exe" -ForegroundColor Green
    
    if (Test-Path $sourceConfig) {
        Copy-Item $sourceConfig -Destination "$InstallPath\afglobal_config.json" -Force -ErrorAction Stop
        Write-Host "✓ Copied afglobal_config.json" -ForegroundColor Green
    }
    
    if (Test-Path $sourceLicense) {
        Copy-Item $sourceLicense -Destination "$InstallPath\LICENSE" -Force -ErrorAction Stop
        Write-Host "✓ Copied LICENSE" -ForegroundColor Green
    }
} catch {
    Write-Host "ERROR: Failed to copy files: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Step 3: Creating shortcuts..." -ForegroundColor Green

if ($CreateDesktopShortcut) {
    try {
        $WshShell = New-Object -ComObject WScript.Shell
        $DesktopPath = [System.IO.Path]::Combine($env:USERPROFILE, "Desktop")
        $ShortcutPath = [System.IO.Path]::Combine($DesktopPath, "AMD Global Frame Generator.lnk")
        
        $Shortcut = $WshShell.CreateShortcut($ShortcutPath)
        $Shortcut.TargetPath = "$InstallPath\afglobal.exe"
        $Shortcut.WorkingDirectory = $InstallPath
        $Shortcut.Description = "AMD Global Frame Generator"
        $Shortcut.Save()
        
        Write-Host "✓ Created desktop shortcut" -ForegroundColor Green
    } catch {
        Write-Host "⚠ Warning: Could not create desktop shortcut: $_" -ForegroundColor Yellow
    }
}

if ($AddToStartup) {
    try {
        $regPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run"
        $exePath = "$InstallPath\afglobal.exe"
        New-ItemProperty -Path $regPath -Name "AMDGlobalFG" -Value $exePath -Force -ErrorAction Stop | Out-Null
        Write-Host "✓ Added to Windows Startup" -ForegroundColor Green
    } catch {
        Write-Host "⚠ Warning: Could not add to startup registry: $_" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Installation Complete!" -ForegroundColor Green
Write-Host ""
Write-Host "Application installed at: $InstallPath" -ForegroundColor Cyan
