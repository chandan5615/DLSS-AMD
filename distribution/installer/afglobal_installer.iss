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
AppId={{DCDB4B6E-F1A7-4A2F-9D3E-8C2B5E7A9F1D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppPublisherURL}
DefaultDirName={autopf}\AMDGlobalFG
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
PrivilegesRequired=admin
OutputBaseFilename=AFGlobalFGSetup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIconTask}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "autostart"; Description: "Run on Windows Startup"; GroupDescription: "Options:"; Flags: unchecked

[Files]
Source: "bin\afglobal.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "bin\afglobal_hook.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "afglobal_config.json"; DestDir: "{app}"; Flags: ignoreversion createallsubdirs
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion isreadme

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

[Code]
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  // Check for running instance and ask user to close it
  if IsProcessRunning('afglobal.exe') then
  begin
    if MsgBox('AMD Global Frame Generator is currently running. Please close it before continuing.' + #13#10#13#10 + 'Do you want to continue after closing the application?', mbConfirmation, MB_YESNO) = IDNO then
    begin
      Result := 'User cancelled installation because afglobal.exe was still running';
    end
    else
    begin
      // Try to close the application gracefully
      KillProcess('afglobal.exe');
      Sleep(1000);
    end;
  end;
end;

function IsProcessRunning(const FileName: string): Boolean;
var
  FWMIService: Variant;
  FSWbemLocator: Variant;
  FWMIObject: Variant;
begin
  Result := False;
  try
    FSWbemLocator := CreateOleObject('WbemScripting.SWbemLocator');
    FWMIService := FSWbemLocator.ConnectServer('localhost', 'root\cimv2', '', '');
    FWMIObject := FWMIService.ExecQuery(Format('select name from Win32_Process where name=''%s''', [FileName]));
    Result := (FWMIObject.Count > 0);
  except
    Result := False;
  end;
end;

procedure KillProcess(const FileName: string);
var
  FWMIService: Variant;
  FSWbemLocator: Variant;
  FWMIObject: Variant;
  FObjItem: Variant;
  i: Integer;
begin
  try
    FSWbemLocator := CreateOleObject('WbemScripting.SWbemLocator');
    FWMIService := FSWbemLocator.ConnectServer('localhost', 'root\cimv2', '', '');
    FWMIObject := FWMIService.ExecQuery(Format('select name from Win32_Process where name=''%s''', [FileName]));
    
    for i := 1 to FWMIObject.Count do
    begin
      FObjItem := FWMIObject.ItemIndex(i - 1);
      FObjItem.Terminate();
    end;
  except
  end;
end;

[UninstallDelete]
Type: files; Name: "{app}\afglobal.log"
Type: files; Name: "{app}\afglobal_config.json"
Type: dirifempty; Name: "{app}"
