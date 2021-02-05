[Setup]
AppName=Open Ephys
AppVersion=0.5.3.1
AppVerName=Open Ephys 0.5.3.1
AppPublisher=open-ephys.org
AppPublisherURL=https://open-ephys.org/gui
DefaultDirName={autopf}\Open Ephys
DefaultGroupName=Open Ephys
OutputBaseFilename=Open-Ephys_Installer
OutputDir=.
LicenseFile=..\..\..\LICENSE
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
ChangesAssociations=yes
SetupIconFile="..\..\Build-files\icon.ico"
UninstallDisplayIcon={app}\open-ephys.exe
AllowNoIcons=yes
WizardStyle=modern

[Tasks]
Name: desktopicon; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: install_usb; Description: "Install Opal Kelly Front Panel USB driver for Open Ephys Acquisition Board"; GroupDescription: "External drivers:";

[Files]
Source: "..\..\..\Build\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs; BeforeInstall: UpdateProgress(0);
Source: "..\..\..\Build\Release\shared\*"; DestDir: "{commonappdata}\Open Ephys\shared"; Flags: ignoreversion recursesubdirs; BeforeInstall: UpdateProgress(55);
Source: "..\..\DataFiles\*"; DestDir: "{userdocs}\Open Ephys\DataFiles"; Flags: ignoreversion recursesubdirs; BeforeInstall: UpdateProgress(60);
Source: "vcredist_x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall; BeforeInstall: UpdateProgress(70);
Source: "..\..\DLLs\FrontPanelUSB-DriverOnly-4.5.5.exe"; DestDir: {tmp}; Flags: deleteafterinstall; BeforeInstall: UpdateProgress(90);

[Icons]
Name: "{group}\Open Ephys"; Filename: "{app}\open-ephys.exe"
Name: "{autodesktop}\Open Ephys"; Filename: "{app}\open-ephys.exe"; Tasks: desktopicon
Name: "{autoprograms}\Open Ephys"; Filename: "{app}\open-ephys.exe"

[Run]
Filename: "{tmp}\vcredist_x64.exe"; Parameters: "/q /norestart /q:a /c:""VCREDI~3.EXE /q:a /c:""""msiexec /i vcredist.msi /qn"""" """; Check: VCRedistNeedsInstall; WorkingDir: {app}; StatusMsg: "Installing VC++ 2019 Redistributables...";
Filename: "{tmp}\FrontPanelUSB-DriverOnly-4.5.5.exe"; StatusMsg: "Installing Front Panel USB driver..."; Tasks: install_usb; Flags: skipifsilent

[Code]
procedure UpdateProgress(Position: Integer);
begin
  WizardForm.ProgressGauge.Position := Position * WizardForm.ProgressGauge.Max div 100;
end;

#IFDEF UNICODE
  #DEFINE AW "W"
#ELSE
  #DEFINE AW "A"
#ENDIF
type
  INSTALLSTATE = Longint;
const
  INSTALLSTATE_INVALIDARG = -2;  // An invalid parameter was passed to the function.
  INSTALLSTATE_UNKNOWN = -1;     // The product is neither advertised or installed.
  INSTALLSTATE_ADVERTISED = 1;   // The product is advertised but not installed.
  INSTALLSTATE_ABSENT = 2;       // The product is installed for a different user.
  INSTALLSTATE_DEFAULT = 5;      // The product is installed for the current user.

  // Visual C++ 2019 Redistributable 14.25.28508
  VC_2019_REDIST_X86_MIN = '{2BC3BD4D-FABA-4394-93C7-9AC82A263FE2}';
  VC_2019_REDIST_X64_MIN = '{EEA66967-97E2-4561-A999-5C22E3CDE428}';

  VC_2019_REDIST_X86_ADD = '{0FA68574-690B-4B00-89AA-B28946231449}';
  VC_2019_REDIST_X64_ADD = '{7D0B74C2-C3F8-4AF1-940F-CD79AB4B2DCE}';

function MsiQueryProductState(szProduct: string): INSTALLSTATE; 
  external 'MsiQueryProductState{#AW}@msi.dll stdcall';

function VCVersionInstalled(const ProductID: string): Boolean;
begin
  Result := MsiQueryProductState(ProductID) = INSTALLSTATE_DEFAULT;
end;

function VCRedistNeedsInstall: Boolean;
begin
  Result := not (VCVersionInstalled(VC_2019_REDIST_X64_MIN));
end;