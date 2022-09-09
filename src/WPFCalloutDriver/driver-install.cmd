@echo off

SET SIGNTOOL_PATH="C:\Program Files (x86)\Windows Kits\10\App Certification Kit"
SET DRIVER_BIN=WFPCalloutDriver.sys
SET DRIVER_INF=WFPCalloutDriver.inf
SET DRIVER_CAT=WFPCalloutDriver.cat
SET DRIVER_CERT=WFPCalloutDriver.cer

:: Removal
IF /I "%1" EQU "r"  ( GOTO :Uninstall ) ELSE (
IF /I "%1" EQU "-r" ( GOTO :Uninstall ) ELSE (
IF /I "%1" EQU "/r" ( GOTO :Uninstall )))

Net Stop WFPCalloutDriver

ECHO Attempting to sign %DRIVER_BIN%
%SIGNTOOL_PATH%\SignTool.exe Sign /a /v /fd SHA256 %DRIVER_BIN%
if %ERRORLEVEL% NEQ 0 (
	echo Signing failed: %ERRORLEVEL%
	EXIT /B %ERRORLEVEL%
)

ECHO Copying files
COPY /Y %DRIVER_BIN% %WinDir%\System32\Drivers\
COPY /Y %DRIVER_INF% %WinDir%\System32\Drivers\
COPY /Y %DRIVER_CAT% %WinDir%\System32\Drivers\
COPY /Y %DRIVER_CERT% %WinDir%\System32\Drivers\

ECHO Registering the WFPSampler Callout Driver
RunDLL32.Exe syssetup,SetupInfObjectInstallAction DefaultInstall 132 %WinDir%\System32\Drivers\%DRIVER_INF%
if %ERRORLEVEL% NEQ 0 (
	echo RunDLL failed: %ERRORLEVEL%
	EXIT /B %ERRORLEVEL%
)

ECHO Starting WFPCalloutDriver
Net Start WFPCalloutDriver

GOTO :EoF

:Uninstall

ECHO Stopping WFPCalloutDriver
Net Stop WFPCalloutDriver

ECHO Unregistering WFPCalloutDriver
RunDLL32.Exe SETUPAPI.DLL,InstallHinfSection DefaultUninstall 132 %WinDir%\System32\Drivers\%DRIVER_INF%

ECHO Deleting WFPCalloutDriver files
ERASE /F /Q %WinDir%\System32\Drivers\%DRIVER_CAT%
ERASE /F /Q %WinDir%\System32\Drivers\%DRIVER_INF%
ERASE /F /Q %WinDir%\System32\Drivers\%DRIVER_BIN%
ERASE /F /Q %WinDir%\System32\Drivers\%DRIVER_CERT%

:EoF