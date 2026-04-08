; cFd Terminal — NSIS Installer Script
; Builds a classic Windows Setup.exe (no WiX needed)
; Install: makensis installer\windows\cfd.nsi
; Output:  build\cfd-setup-1.0.0.exe

;--- General -----------------------------------------------------------------
!define APP_NAME        "cFd Terminal"
!define APP_EXE         "cfd.exe"
!ifndef APP_VERSION
  !define APP_VERSION   "1.0.0"
!endif
!define APP_PUBLISHER   "saarors"
!define APP_URL         "https://github.com/saarors/cFd"
!define APP_DESCRIPTION "A custom terminal shell written in C"
!define INSTALL_DIR     "$PROGRAMFILES64\cFd"
!define REG_KEY         "Software\Microsoft\Windows\CurrentVersion\Uninstall\cFd"

;--- Compression & output ----------------------------------------------------
SetCompressor     /SOLID lzma
SetCompressorDictSize 32
!ifndef OUTFILE
  !define OUTFILE "..\..\build\cfd-setup-${APP_VERSION}.exe"
!endif
OutFile           "${OUTFILE}"
InstallDir        "${INSTALL_DIR}"
InstallDirRegKey  HKLM "${REG_KEY}" "InstallLocation"
RequestExecutionLevel admin

;--- Modern UI ---------------------------------------------------------------
!include "MUI2.nsh"
!include "EnvVarUpdate.nsh"   ; for PATH manipulation

!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TITLE  "cFd Terminal ${APP_VERSION}"
!define MUI_WELCOMEPAGE_TEXT   "cFd is a fully custom terminal shell written in C.$\r$\n$\r$\nNo bash. No PowerShell. No wrappers. Just C.$\r$\n$\r$\nThis wizard will install cFd on your computer."
!define MUI_FINISHPAGE_RUN     "$INSTDIR\cfd.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch cFd Terminal"
!define MUI_FINISHPAGE_LINK    "Visit the GitHub repository"
!define MUI_FINISHPAGE_LINK_LOCATION "${APP_URL}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--- Install -----------------------------------------------------------------
Section "cFd Terminal (required)" SEC_MAIN
  SectionIn RO   ; can't deselect

  SetOutPath "$INSTDIR"

  ; Copy the binary
  File "..\..\cfd.exe"

  ; Add to system PATH
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR"

  ; Start Menu shortcut
  CreateDirectory "$SMPROGRAMS\cFd Terminal"
  CreateShortcut  "$SMPROGRAMS\cFd Terminal\cFd Terminal.lnk" \
                  "$INSTDIR\cfd.exe"
  CreateShortcut  "$SMPROGRAMS\cFd Terminal\Uninstall cFd.lnk" \
                  "$INSTDIR\Uninstall.exe"

  ; Desktop shortcut
  CreateShortcut  "$DESKTOP\cFd Terminal.lnk" "$INSTDIR\cfd.exe"

  ; Write uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Registry — Programs and Features entry
  WriteRegStr   HKLM "${REG_KEY}" "DisplayName"     "${APP_NAME}"
  WriteRegStr   HKLM "${REG_KEY}" "DisplayVersion"  "${APP_VERSION}"
  WriteRegStr   HKLM "${REG_KEY}" "Publisher"        "${APP_PUBLISHER}"
  WriteRegStr   HKLM "${REG_KEY}" "URLInfoAbout"     "${APP_URL}"
  WriteRegStr   HKLM "${REG_KEY}" "InstallLocation"  "$INSTDIR"
  WriteRegStr   HKLM "${REG_KEY}" "UninstallString"  "$INSTDIR\Uninstall.exe"
  WriteRegDWORD HKLM "${REG_KEY}" "NoModify"         1
  WriteRegDWORD HKLM "${REG_KEY}" "NoRepair"         1

SectionEnd

;--- Uninstall ---------------------------------------------------------------
Section "Uninstall"

  ; Remove from PATH
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR"

  ; Remove files
  Delete "$INSTDIR\cfd.exe"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir  "$INSTDIR"

  ; Remove shortcuts
  Delete "$SMPROGRAMS\cFd Terminal\cFd Terminal.lnk"
  Delete "$SMPROGRAMS\cFd Terminal\Uninstall cFd.lnk"
  RMDir  "$SMPROGRAMS\cFd Terminal"
  Delete "$DESKTOP\cFd Terminal.lnk"

  ; Remove registry
  DeleteRegKey HKLM "${REG_KEY}"
  DeleteRegKey HKCU "Software\cFd"

SectionEnd
