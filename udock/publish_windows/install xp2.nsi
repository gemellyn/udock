Function .onInit
 
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\UDock" \
  "UninstallString"
  StrCmp $R0 "" done
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "UDock is already installed. $\n$\nClick `OK` to remove the \
  previous version or `Cancel` to cancel this upgrade." \
  IDOK uninst
  Abort
 
;Run the uninstaller
uninst:
  ClearErrors
  ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller done
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
  no_remove_uninstaller:
 
done:
 
FunctionEnd

; The name of the installer
Name "UDock Installer"

; The file to write
OutFile "udock_xp2.exe"

; The default installation directory
InstallDir $PROGRAMFILES\UDock

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\UDock" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "UDock"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\Release\udock.exe"
  File "..\run\myShip.bin"
  File "..\run\*.dll"
  File /r "..\run\sound"
  File /r "..\run\shaders"
  SetOutPath $INSTDIR\molecules
  File "..\run\molecules\9.mol2"
  File "..\run\molecules\10.mol2"
  File "..\run\molecules\11.mol2"
  File "..\run\molecules\12.mol2"
  File "..\run\molecules\13.mol2"
  File "..\run\molecules\14.mol2"
  SetOutPath $INSTDIR\export
	SetOutPath $INSTDIR
  File /r "..\run\textures"
  SetOutPath $INSTDIR\logs
  SetOutPath $INSTDIR
  File "..\run\redist\vcredist_x86.exe"
  File "..\run\redist\oalinst.exe"
  

  ExecWait '"$INSTDIR\vcredist_x86.exe" /passive /norestart'
  ExecWait '"$INSTDIR\oalinst.exe"'
  
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\UDock "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UDock" "DisplayName" "UDock"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UDock" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UDock" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UDock" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\UDock"
  CreateShortCut "$SMPROGRAMS\UDock\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\UDock\UDock.lnk" "$INSTDIR\udock.exe" "l v" "$INSTDIR\udock.exe" 0
  CreateShortCut "$SMPROGRAMS\UDock\UDock_inverse.lnk" "$INSTDIR\udock.exe" "l v i" "$INSTDIR\udock.exe" 0
  CreateShortCut "$DESKTOP\udock.lnk" "$INSTDIR\udock.exe" "l v" "$INSTDIR\udock.exe" 0
  CreateShortCut "$DESKTOP\udock_inverse.lnk" "$INSTDIR\udock.exe" "l v i" "$INSTDIR\udock.exe" 0
  ;CreateShortCut "$SMPROGRAMS\UDock\UDockWindowed.lnk" "$INSTDIR\udock.exe" "w" "$INSTDIR\udock.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UDock"
  DeleteRegKey HKLM SOFTWARE\UDock

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*
  Delete $INSTDIR\molecules\*.*
  Delete $INSTDIR\shaders\*.*
  Delete $INSTDIR\sound\*.*
	Delete $INSTDIR\textures\*.*
	Delete $INSTDIR\export\*.*
	Delete $INSTDIR\logs\*.*
  
  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\UDock\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\UDock"
  RMDir "$INSTDIR"
  RMDir "$INSTDIR\molecules"
  RMDir "$INSTDIR\shaders"
  RMDir /r "$INSTDIR\sound"
	RMDir /r "$INSTDIR\textures"
	RMDir /r "$INSTDIR\export"
	RMDir /r "$INSTDIR\logs"

SectionEnd


