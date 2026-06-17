[Setup]
AppId=Open Ephys
AppName=Open Ephys GUI
AppVersion=1.0.2
AppVerName=Open Ephys GUI 1.0.2
AppCopyright=Copyright (C) 2010-2026, Open Ephys & Contributors
AppPublisher=open-ephys.org
AppPublisherURL=https://open-ephys.org/gui
DefaultDirName={autopf}\Open Ephys
DefaultGroupName=Open Ephys GUI
DisableProgramGroupPage=yes
UsePreviousGroup=no
OutputBaseFilename=Open-Ephys_Installer
OutputDir=.
LicenseFile=..\..\..\LICENSE
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
ChangesAssociations=yes
SetupIconFile="..\..\Build-files\icon.ico"
UninstallDisplayIcon={app}\open-ephys.exe
AllowNoIcons=yes
WizardStyle=modern dynamic
ArchiveExtraction=enhanced/nopassword
SetupLogging=yes

[Tasks]
Name: desktopicon; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: install_ftdi; Description: "Install FTDI D3XX driver"; GroupDescription: "Drivers:";
Name: install_ftdi\14001; Description: "v1.4.0.1 for Open Ephys FPGA Acquisition board"; GroupDescription: "Drivers:"; Flags: exclusive;
Name: install_ftdi\13010; Description: "v1.3.0.10 for Neuropixels OneBox"; GroupDescription: "Drivers:"; Flags: exclusive unchecked;
Name: install_ok_usb; Description: "Install Opal Kelly Front Panel USB driver (Opal Kelly FPGA board)"; GroupDescription: "Drivers:"; Flags: unchecked;

[Files]
Source: "..\..\..\Build\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs; BeforeInstall: UpdateProgress(0);
Source: "..\..\DLLs\FTD3XXDriver_WHQLCertified_1.3.0.10_Installer.exe"; DestDir: {tmp}; Flags: deleteafterinstall; Tasks: install_ftdi\13010; BeforeInstall: UpdateProgress(80);
Source: "https://github.com/open-ephys-plugins/ftdi-drivers/raw/refs/heads/main/installers/FTDI_DriverInstaller_1.4.0.1_x64.exe"; DestDir: "{tmp}"; DestName: "FTDI_DriverInstaller_1.4.0.1_x64.exe"; ExternalSize: 71_818_824; Flags: external download ignoreversion; Tasks: install_ftdi\14001; BeforeInstall: UpdateProgress(50);
Source: "..\..\DLLs\FrontPanelUSB-DriverOnly-4.5.5.exe"; DestDir: {tmp}; Flags: deleteafterinstall; BeforeInstall: UpdateProgress(90);

[Icons]
Name: "{autodesktop}\Open Ephys GUI"; Filename: "{app}\open-ephys.exe"; Tasks: desktopicon
Name: "{autoprograms}\Open Ephys GUI"; Filename: "{app}\open-ephys.exe"

[Run]
Filename: "{tmp}\FTD3XXDriver_WHQLCertified_1.3.0.10_Installer.exe"; StatusMsg: "Installing FTDI D3XX driver v1.3.0.10..."; Tasks: install_ftdi\13010; Flags: skipifsilent
Filename: "{tmp}\FTDI_DriverInstaller_1.4.0.1_x64.exe"; StatusMsg: "Installing FTDI D3XX driver v1.4.0.1..."; Tasks: install_ftdi\14001; Flags: skipifsilent
Filename: "{tmp}\FrontPanelUSB-DriverOnly-4.5.5.exe"; StatusMsg: "Installing Front Panel USB driver..."; Tasks: install_ok_usb; Flags: skipifsilent
Filename: "{app}\open-ephys.exe"; Description: "Launch Open Ephys GUI"; Flags: postinstall nowait skipifsilent

[InstallDelete]
Type: files; Name: "{app}\plugins\*.dll";
Type: files; Name: "{autodesktop}\Open Ephys.lnk";
Type: files; Name: "{autoprograms}\Open Ephys.lnk";
Type: filesandordirs; Name: "{autoprograms}\Open Ephys";

[Code]
// types and variables
type
  TDependency_Entry = record
    Filename: String;
    Parameters: String;
    Title: String;
    URL: String;
    Checksum: String;
  end;

var
  Dependency_Memo: String;
  Dependency_Entry: TDependency_Entry;
  Dependency_Added: Boolean;
  Dependency_NeedToRestart: Boolean;
  Dependency_DownloadPage: TDownloadWizardPage;

procedure Dependency_Add(const Filename, Parameters, Title, URL, Checksum: String);
var
  Dependency: TDependency_Entry;
begin
  Dependency_Memo := Dependency_Memo + #13#10 + '%1' + Title;

  Dependency.Filename := Filename;
  Dependency.Parameters := Parameters;
  Dependency.Title := Title;

  if FileExists(ExpandConstant('{tmp}{\}') + Filename) then begin
    Dependency.URL := '';
  end else begin
    Dependency.URL := URL;
  end;

  Dependency.Checksum := Checksum;

  Dependency_Entry := Dependency;
  Dependency_Added := True;
end;

<event('InitializeWizard')>
procedure Dependency_Internal1;
begin
  Dependency_DownloadPage := CreateDownloadPage(SetupMessage(msgWizardPreparing), SetupMessage(msgPreparingDesc), nil);
end;

<event('PrepareToInstall')>
function Dependency_Internal2(var NeedsRestart: Boolean): String;
var
  ResultCode: Integer;
  Retry: Boolean;
begin
  if Dependency_Added then begin
    Dependency_DownloadPage.Show;

    if Dependency_Entry.URL <> '' then begin
      Dependency_DownloadPage.Clear;
      Dependency_DownloadPage.Add(Dependency_Entry.URL, Dependency_Entry.Filename, Dependency_Entry.Checksum);

      Retry := True;
      while Retry do begin
        Retry := False;

        try
          Dependency_DownloadPage.Download;
        except
          if Dependency_DownloadPage.AbortedByUser then begin
            Result := Dependency_Entry.Title;
          end else begin
            case SuppressibleMsgBox(AddPeriod(GetExceptionMessage), mbError, MB_ABORTRETRYIGNORE, IDIGNORE) of
              IDABORT: begin
                Result := Dependency_Entry.Title;
              end;
              IDRETRY: begin
                Retry := True;
              end;
            end;
          end;
        end;
      end;
    end;

    if Result = '' then begin
      Dependency_DownloadPage.SetText(Dependency_Entry.Title, '');

      while True do begin
        ResultCode := 0;
        if ShellExec('', ExpandConstant('{tmp}{\}') + Dependency_Entry.Filename, Dependency_Entry.Parameters, '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode) then begin

          if (ResultCode = 0) then begin // ERROR_SUCCESS (0)
             break;
          end else if ResultCode = 3010 then begin // ERROR_SUCCESS_REBOOT_REQUIRED (3010)
            Dependency_NeedToRestart := True;
            break;
          end;

          case SuppressibleMsgBox(FmtMessage(SetupMessage(msgErrorFunctionFailed), [Dependency_Entry.Title, IntToStr(ResultCode)]), mbError, MB_ABORTRETRYIGNORE, IDIGNORE) of
            IDABORT: begin
              Result := Dependency_Entry.Title;
              break;
            end;
            IDIGNORE: begin
              break;
            end;
          end;
        end;
      end;
    end;

    Dependency_DownloadPage.Hide;
  end;
end;

<event('UpdateReadyMemo')>
function Dependency_Internal3(const Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
begin
  Result := '';
  if MemoUserInfoInfo <> '' then begin
    Result := Result + MemoUserInfoInfo + Newline + NewLine;
  end;
  if MemoDirInfo <> '' then begin
    Result := Result + MemoDirInfo + Newline + NewLine;
  end;
  if MemoTypeInfo <> '' then begin
    Result := Result + MemoTypeInfo + Newline + NewLine;
  end;
  if MemoComponentsInfo <> '' then begin
    Result := Result + MemoComponentsInfo + Newline + NewLine;
  end;
  if MemoGroupInfo <> '' then begin
    Result := Result + MemoGroupInfo + Newline + NewLine;
  end;
  if MemoTasksInfo <> '' then begin
    Result := Result + MemoTasksInfo;
  end;

  if Dependency_Memo <> '' then begin
    if MemoTasksInfo = '' then begin
      Result := Result + SetupMessage(msgReadyMemoTasks);
    end;
    Result := Result + FmtMessage(Dependency_Memo, [Space]);
  end;
end;

<event('NeedRestart')>
function Dependency_NeedRestart: Boolean;
begin
  Result := Dependency_NeedToRestart;
end;


function Dependency_IsX64: Boolean;
begin
  Result := Is64BitInstallMode;
end;

function Dependency_String(const x86, x64: String): String;
begin
  if Dependency_IsX64 then begin
    Result := x64;
  end else begin
    Result := x86;
  end;
end;

function Dependency_ArchSuffix: String;
begin
  Result := Dependency_String('', '_x64');
end;

function Dependency_ArchTitle: String;
begin
  Result := Dependency_String(' (x86)', ' (x64)');
end;

procedure UpdateProgress(Position: Integer);
begin
  WizardForm.ProgressGauge.Position := Position * WizardForm.ProgressGauge.Max div 100;
end;

function InitializeSetup: Boolean;
var
  Version: String;
  PackedVersion: Int64;
begin
  // https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist
  if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', Version) and (Copy(Version, 1, 1) = 'v') then begin
    Delete(Version, 1, 1);
  end;
  if not StrToVersion(Version, PackedVersion) or (ComparePackedVersion(PackedVersion, PackVersionComponents(14, 51, 36247, 0)) < 0) then begin
    Dependency_Add('vcredist14_x64.exe',
      '/passive /norestart',
      'Visual C++ v14 Redistributable (x64)',
      'https://aka.ms/vc14/vc_redist.x64.exe',
      '');
  end;
  Result := True;
end;