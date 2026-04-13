; Inno Setup Script for AMD Global Frame Generator
; This script creates an installer for the AMD Global Frame Generator application

#define MyAppName "AMD Global Frame Generator"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Advanced Micro Devices"
#define MyAppPublisherURL "https://www.amd.com"
#define MyAppExeName "afglobal.exe"
#define MyAppDLLName "afglobal_hook.dll"
#define MyAppConfigName "afglobal_config.json"

[Setup]
AppId={{DCDB4B6E-F1A7-4A2F-9D3E-8C2B5E7A9F1D}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppPublisherURL}
DefaultDirName={autopf}\AMDGlobalFG
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
PrivilegesRequired=admin
OutputDir=.\output
OutputBaseFilename=AFGlobalFGSetup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Shortcuts:"; Flags: unchecked
Name: "autostart"; Description: "Run on Windows Startup"; GroupDescription: "Options:"; Flags: unchecked

[Files]
Source: "..\build\bin\afglobal.exe"; DestDir: "{app}"; Flags: ignoreversion
; DLL is optional - only included if MinHook is installed
; Source: "..\build\bin\afglobal_hook.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\afglobal_config.json"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion isreadme

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "https://aka.ms/vs/17/release/vc_redist.x64.exe"; Description: "Install VC++ Redistributable"; Flags: shellexec skipifdoesntexist

[Registry]
; Add to startup if task is selected
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "AMDGlobalFG"; ValueData: "{app}\{#MyAppExeName}"; Tasks: autostart; Flags: uninsdeletevalue

; Add to uninstall registry
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Uninstall\{{DCDB4B6E-F1A7-4A2F-9D3E-8C2B5E7A9F1D}"; ValueType: string; ValueName: "DisplayName"; ValueData: "{#MyAppName}"
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Uninstall\{{DCDB4B6E-F1A7-4A2F-9D3E-8C2B5E7A9F1D}"; ValueType: string; ValueName: "DisplayVersion"; ValueData: "{#MyAppVersion}"
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Uninstall\{{DCDB4B6E-F1A7-4A2F-9D3E-8C2B5E7A9F1D}"; ValueType: string; ValueName: "UninstallString"; ValueData: "{uninstallexe}"


; Code section removed - basic installer only
; For production, consider adding process detection

[UninstallDelete]
Type: files; Name: "{app}\afglobal.log"
Type: files; Name: "{app}\afglobal_config.json"
Type: dirifempty; Name: "{app}"
