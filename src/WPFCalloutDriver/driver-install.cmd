SET SIGNTOOL_PATH="C:\Program Files (x86)\Windows Kits\10\App Certification Kit"
SET DRIVER_BIN="WFPCalloutDriver.sys"
SET DRIVER_INF="WFPCalloutDriver.inf"
SET DRIVER_CAT="WFPCalloutDriver.cat"

:: Removal
IF /I "%1" EQU "r"  ( GOTO :Uninstall ) ELSE (
IF /I "%1" EQU "-r" ( GOTO :Uninstall ) ELSE (
IF /I "%1" EQU "/r" ( GOTO :Uninstall )))

ECHO Attempting to sign WFPSampler.Exe
%SIGNTOOL_PATH%\SignTool.exe Sign -A -V %DRIVER_BIN%

ECHO Copying WFPCalloutDriver.sys binary
COPY /Y %DRIVER_BIN% %WinDir%\System32\Drivers\

ECHO Registering the WFPSampler Callout Driver
RunDLL32.Exe syssetup,SetupInfObjectInstallAction DefaultInstall 132 %WinDir%\System32\Drivers\%DRIVER_INF%

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

:EoF