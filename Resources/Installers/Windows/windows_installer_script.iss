[Setup]
AppName=Open Ephys
AppVersion=1.0.0-apha.1
AppVerName=Open Ephys 1.0.0-alpha.1
AppCopyright=Copyright (C) 2010-2024, Open Ephys & Contributors
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
Name: install_usb1; Description: "Install FTDI D3XX driver (Open Ephys FPGA board)"; GroupDescription: "Acquisition Board drivers:";
Name: install_usb2; Description: "Install Opal Kelly Front Panel USB driver (Opal Kelly FPGA board)"; GroupDescription: "Acquisition Board drivers:"; Flags: unchecked;

[Files]
Source: "..\..\..\Build\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs; BeforeInstall: UpdateProgress(0);
Source: "..\..\DLLs\FTD3XXDriver_WHQLCertified_1.3.0.8_Installer.exe"; DestDir: {tmp}; Flags: deleteafterinstall; BeforeInstall: UpdateProgress(80);
Source: "..\..\DLLs\FrontPanelUSB-DriverOnly-4.5.5.exe"; DestDir: {tmp}; Flags: deleteafterinstall; BeforeInstall: UpdateProgress(90);

[Icons]
Name: "{group}\Open Ephys"; Filename: "{app}\open-ephys.exe"
Name: "{autodesktop}\Open Ephys"; Filename: "{app}\open-ephys.exe"; Tasks: desktopicon
Name: "{autoprograms}\Open Ephys"; Filename: "{app}\open-ephys.exe"

[Run]
Filename: "{tmp}\FTD3XXDriver_WHQLCertified_1.3.0.8_Installer.exe"; StatusMsg: "Installing FTDI D3XX driver..."; Tasks: install_usb1; Flags: skipifsilent
Filename: "{tmp}\FrontPanelUSB-DriverOnly-4.5.5.exe"; StatusMsg: "Installing Front Panel USB driver..."; Tasks: install_usb2; Flags: skipifsilent
Filename: "{app}\open-ephys.exe"; Description: "Launch Open Ephys GUI"; Flags: postinstall nowait skipifsilent

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

          if ResultCode = 0 then begin // ERROR_SUCCESS (0)
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
begin

  // https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist
  if not IsMsiProductInstalled('{36F68A90-239C-34DF-B58C-64B30153CE35}', PackVersionComponents(14, 30, 30704, 0)) then begin
    Dependency_Add('vcredist2022_x64.exe',
      '/passive /norestart',
      'Visual C++ 2015-2022 Redistributable (x64)',
      'https://aka.ms/vs/17/release/vc_redist.x64.exe',
      '');

  end;
  Result := True;
end;