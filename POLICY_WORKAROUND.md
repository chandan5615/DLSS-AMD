# Application Control Policy Workaround

## Issue
Your system has an Application Control policy that blocks unsigned executables. This prevents the installer from running.

## Workaround 1: Manual Installation (Portable)

### Step 1: Extract Files
```powershell
# Copy the executable to Program Files manually
$source = ".\build\bin\afglobal.exe"
$dest = "C:\Program Files\AMDGlobalFG"

# Create directory
New-Item -ItemType Directory -Path $dest -Force

# Copy files
Copy-Item $source -Destination $dest\
Copy-Item ".\afglobal_config.json" -Destination $dest\
Copy-Item ".\LICENSE" -Destination $dest\
```

### Step 2: Create Shortcuts
```powershell
# Create desktop shortcut
$WshShell = New-Object -ComObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("$env:USERPROFILE\Desktop\AFGlobalFG.lnk")
$Shortcut.TargetPath = "C:\Program Files\AMDGlobalFG\afglobal.exe"
$Shortcut.Save()
```

### Step 3: (Optional) Add to Startup
```powershell
# Add to Windows Startup
$source = "C:\Program Files\AMDGlobalFG\afglobal.exe"
$regPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run"
New-ItemProperty -Path $regPath -Name "AMDGlobalFG" -Value $source -Force
```

## Workaround 2: Request Administrator Override

1. Open Settings → Privacy & Security → App & browser control
2. Scroll to "Reputation-based protection" 
3. Toggle off "Check apps and files"
4. Then run the installer

## Workaround 3: Group Policy Editor (if available)

1. Press `Win+R`, type `gpedit.msc`
2. Navigate to: Computer Configuration → Windows Settings → Security Settings → Application Control Policies
3. Disable or adjust the policy
4. Restart computer
5. Run installer

## Alternative: Run from Batch as Admin

Double-click: `run_installer.bat`

Then approve the UAC prompt when it appears.

---

For production deployment, the executable should be code-signed with a valid certificate.
