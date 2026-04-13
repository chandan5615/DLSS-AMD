# PowerShell build script for Inno Setup installer
# Requires: Inno Setup 6.0 or later, afglobal.exe and afglobal_hook.dll built

param(
    [string]$InnoSetupPath = "",
    [switch]$SkipBuildCheck = $false
)

# Colors for output
function Write-Info { Write-Host $args -ForegroundColor Cyan }
function Write-Success { Write-Host $args -ForegroundColor Green }
function Write-Error { Write-Host $args -ForegroundColor Red }
function Write-Warning { Write-Host $args -ForegroundColor Yellow }

Write-Info "========================================"
Write-Info "AMD Global Frame Generator - Installer Builder"
Write-Info "========================================"
Write-Info ""

# Find Inno Setup installation
$iscc = $null

if ($InnoSetupPath -and (Test-Path $InnoSetupPath)) {
    $iscc = $InnoSetupPath
} else {
    # Try to find iscc.exe in system PATH
    $isccPath = Get-Command iscc.exe -ErrorAction SilentlyContinue
    if ($isccPath) {
        $iscc = $isccPath.Source
    }
    
    # Try common installation paths
    if (-not $iscc) {
        $commonPaths = @(
            "C:\Program Files (x86)\Inno Setup 6\iscc.exe",
            "C:\Program Files\Inno Setup 6\iscc.exe",
            "C:\Program Files (x86)\Inno Setup 5\iscc.exe",
            "C:\Program Files\Inno Setup 5\iscc.exe"
        )
        
        foreach ($path in $commonPaths) {
            if (Test-Path $path) {
                $iscc = $path
                break
            }
        }
    }
}

if (-not $iscc) {
    Write-Error "ERROR: Inno Setup not found!"
    Write-Info "Please install Inno Setup 6.0 or later from:"
    Write-Info "  https://jrsoftware.org/isdl.php"
    Write-Info ""
    Write-Info "Or specify installation path:"
    Write-Info "  build_installer.ps1 -InnoSetupPath 'C:\Program Files (x86)\Inno Setup 6\iscc.exe'"
    exit 1
}

Write-Success "Found Inno Setup at: $iscc"
Write-Info ""

# Verify build artifacts exist (unless skipped)
if (-not $SkipBuildCheck) {
    Write-Info "Verifying build artifacts..."
    
    $exe = "build\bin\afglobal.exe"
    $dll = "build\bin\afglobal_hook.dll"
    $config = "build\bin\afglobal_config.json"
    
    if (-not (Test-Path $exe)) {
        Write-Error "ERROR: afglobal.exe not found at $exe"
        Write-Info "Please run build.ps1 first to compile the project."
        exit 1
    }
    Write-Success "  [OK] afglobal.exe found"
    
    # DLL is optional (requires MinHook)
    if (-not (Test-Path $dll)) {
        Write-Warning "  [!] afglobal_hook.dll not found (requires MinHook installation)"
        Write-Info "      The installer will include exe only."
        Write-Info "      See MINHOOK_INSTALLATION.md for DLL setup."
    } else {
        Write-Success "  [OK] afglobal_hook.dll found"
    }
    
    if (Test-Path $config) {
        Write-Success "  [OK] afglobal_config.json found"
    }
    
    Write-Info ""
}

# Create output directory
$outputDir = "installer\output"
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
    Write-Info "Created output directory: $outputDir"
}

Write-Info ""
Write-Info "Building installer..."
Write-Info ""

# Run Inno Setup compiler
& "$iscc" /O"$outputDir" installer\afglobal_installer.iss

if ($LASTEXITCODE -ne 0) {
    Write-Error ""
    Write-Error "ERROR: Installer build failed!"
    Write-Info "Check the output above for details."
    exit 1
}

Write-Info ""
Write-Success "========================================"
Write-Success "Installer Build Complete!"
Write-Success "========================================"
Write-Info ""
Write-Info "Installer created at:"
Write-Info "  installer\output\AFGlobalFGSetup.exe"
Write-Info ""
Write-Info "To install AMD Global Frame Generator:"
Write-Info "  1. Run: installer\output\AFGlobalFGSetup.exe"
Write-Info "  2. Follow the installation wizard"
Write-Info "  3. Enable Windows Startup option if desired"
Write-Info ""
