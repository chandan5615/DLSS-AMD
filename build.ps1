# PowerShell build script for AMD Global Frame Generator
# Requires: Visual Studio 2019/2022, CMake, PowerShell 5.1+

param(
    [string]$CMakePath = "cmake",
    [string]$Configuration = "Release",
    [switch]$UseVcpkg = $false,
    [string]$VcpkgRoot = $env:VCPKG_ROOT
)

# Colors for output
function Write-Info { Write-Host $args -ForegroundColor Cyan }
function Write-Success { Write-Host $args -ForegroundColor Green }
function Write-Error { Write-Host $args -ForegroundColor Red }
function Write-Warning { Write-Host $args -ForegroundColor Yellow }

Write-Info "========================================"
Write-Info "AMD Global Frame Generator - Build Script"
Write-Info "========================================"
Write-Info ""

# Check if CMake is available
$cmakeVersion = & $CMakePath --version 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Error "ERROR: CMake not found at '$CMakePath'"
    Write-Info "Please install CMake and add it to PATH"
    exit 1
}

Write-Success "Found CMake: $(($cmakeVersion[0]))"

# Create build directory
if (-not (Test-Path "build")) {
    Write-Info "Creating build directory..."
    New-Item -ItemType Directory -Path "build" -Force | Out-Null
}

# Change to build directory
Push-Location "build"

# Configure with CMake - try to find best generator
Write-Info ""
Write-Info "Configuring project with CMake..."
Write-Info ""

# Try Visual Studio 2022 first, then 2019, then Ninja, then default
$generator = $null
$generators = @("Visual Studio 17 2022", "Visual Studio 16 2019", "Ninja", "")

foreach ($gen in $generators) {
    if ([string]::IsNullOrEmpty($gen)) {
        Write-Warning "Using CMake default generator (auto-detected)"
        $generator = ""
        break
    }
    
    # Try this generator
    if ($UseVcpkg -and $VcpkgRoot) {
        & $CMakePath -G $gen -A x64 `
            -DCMAKE_TOOLCHAIN_FILE="$VcpkgRoot\scripts\buildsystems\vcpkg.cmake" `
            .. *> $null
    } else {
        & $CMakePath -G $gen -A x64 .. *> $null
    }
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Using generator: $gen"
        $generator = $gen
        break
    }
}

# If all generators failed, show error
if ($LASTEXITCODE -ne 0 -and -not [string]::IsNullOrEmpty($generator)) {
    # Re-run with default to show actual error
    if ($UseVcpkg -and $VcpkgRoot) {
        Write-Info "Using vcpkg from: $VcpkgRoot"
        & $CMakePath -A x64 `
            -DCMAKE_TOOLCHAIN_FILE="$VcpkgRoot\scripts\buildsystems\vcpkg.cmake" `
            ..
    } else {
        & $CMakePath -A x64 ..
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Error ""
    Write-Error "ERROR: CMake configuration failed!"
    Write-Info "Please ensure you have:"
    Write-Info "  - Visual Studio 2019 or 2022 installed"
    Write-Info "  - CMake 3.20 or later"
    Write-Info "  - DirectX 12 SDK"
    Pop-Location
    exit 1
}

# Build the project
Write-Info ""
Write-Info "Building project..."
Write-Info ""

& $CMakePath --build . --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Error ""
    Write-Error "ERROR: Build failed!"
    Pop-Location
    exit 1
}

# Create output directory
if (-not (Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" -Force | Out-Null
}

# Copy binaries to bin directory
Write-Info ""
Write-Info "Copying binaries to bin directory..."

$files = @(
    "$Configuration\afglobal.exe",
    "$Configuration\afglobal_hook.dll"
)

foreach ($file in $files) {
    if (Test-Path $file) {
        Copy-Item -Path $file -Destination "bin\" -Force
        Write-Success "Copied $(Split-Path -Leaf $file)"
    }
}

# Copy config file
if (Test-Path "..\afglobal_config.json") {
    Copy-Item -Path "..\afglobal_config.json" -Destination "bin\" -Force
    Write-Success "Copied afglobal_config.json"
}

# Return to original directory
Pop-Location

Write-Info ""
Write-Success "========================================"
Write-Success "Build Complete!"
Write-Success "========================================"
Write-Info ""
Write-Info "Output files located in: build\bin\"
Write-Info "  - afglobal.exe (Main executable)"
Write-Info "  - afglobal_hook.dll (Injection DLL)"
Write-Info "  - afglobal_config.json (Configuration)"
Write-Info ""
Write-Info "Next steps:"
Write-Info "  1. Test the application: build\bin\afglobal.exe"
Write-Info "  2. Edit configuration: build\bin\afglobal_config.json"
Write-Info "  3. Create installer: .\build_installer.ps1"
Write-Info ""
Write-Info "For manual installation:"
Write-Info "  Copy build\bin\* to C:\Program Files\AMDGlobalFG"
Write-Info ""
