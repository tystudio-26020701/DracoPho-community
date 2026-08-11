; DracoPho Windows 安装器（Inno Setup）
;
; 构建命令（CI 或本地）：
;   ISCC.exe /DMyAppVersion=26.8.5.0 ^
;            /DMyAppTag=v26.8.5.0 ^
;            /DMyExeName=dracoPho.exe ^
;            /DMyProductName=dracoPho ^
;            /DMySourceDir=C:\path\to\app ^
;            /DMyIconFile=C:\path\to\icon.ico ^
;            dracoPho-setup.iss
;
; 产物：<MyProductName>-<MyAppTag>-windows-x86_64-setup.exe

#ifndef MyAppVersion
  #error "MyAppVersion is not defined"
#endif
#ifndef MyAppTag
  #define MyAppTag "{#MyAppVersion}"
#endif
#ifndef MyExeName
  #error "MyExeName is not defined"
#endif
#ifndef MyProductName
  #error "MyProductName is not defined"
#endif
#ifndef MySourceDir
  #error "MySourceDir is not defined"
#endif
#ifndef MyIconFile
  #define MyIconFile ""
#endif

[Setup]
AppId={{4D3C9B0E-6A2F-4B8E-9D1A-5C7E2F3A4B5C}
AppName=DracoPho
AppVersion={#MyAppVersion}
AppVerName=DracoPho {#MyAppVersion}
AppPublisher=tystudio-26020701
AppPublisherURL=https://github.com/tystudio-26020701/DracoPho-community
DefaultDirName={autopf}\DracoPho
DefaultGroupName=DracoPho
DisableProgramGroupPage=yes
OutputDir=.
OutputBaseFilename={#MyProductName}-{#MyAppTag}-windows-x86_64-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
SetupIconFile={#MyIconFile}
UninstallDisplayIcon={app}\bin\{#MyExeName}
PrivilegesRequired=admin
MinVersion=10.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#MySourceDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\DracoPho"; Filename: "{app}\bin\{#MyExeName}"
Name: "{autodesktop}\DracoPho"; Filename: "{app}\bin\{#MyExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\bin\{#MyExeName}"; Description: "{cm:LaunchProgram,DracoPho}"; Flags: nowait postinstall skipifsilent
