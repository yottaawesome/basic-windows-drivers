:: Based on Dusty Harper's WFPSampler installer batch file
SET SIGNTOOL_PATH="C:\Program Files (x86)\Windows Kits\10\App Certification Kit"
SET DRIVER_BIN=EmptyKMDFDriver.sys
SET DRIVER_INF=EmptyKMDFDriver.inf
SET DRIVER_CAT=EmptyKMDFDriver.cat
SET DRIVER_CERT=EmptyKMDFDriver.cer

:: Removal
IF /I "%1" EQU "r"  ( GOTO :Uninstall ) ELSE (
IF /I "%1" EQU "-r" ( GOTO :Uninstall ) ELSE (
IF /I "%1" EQU "/r" ( GOTO :Uninstall )))

ECHO Attempting to sign driver
%SIGNTOOL_PATH%\SignTool.exe Sign -A -V %DRIVER_BIN%

ECHO Copying WFPCalloutDriver.sys binary
COPY /Y %DRIVER_BIN% %WinDir%\System32\Drivers\
COPY /Y %DRIVER_INF% %WinDir%\System32\Drivers\
COPY /Y %DRIVER_CAT% %WinDir%\System32\Drivers\
COPY /Y %DRIVER_CERT% %WinDir%\System32\Drivers\

ECHO Registering the driver
RunDLL32.Exe syssetup,SetupInfObjectInstallAction DefaultInstall 132 %WinDir%\System32\Drivers\%DRIVER_INF%

ECHO Starting driver
Net Start EmptyKMDFDriver

GOTO :EoF

:Uninstall

ECHO Stopping driver
Net Stop EmptyKMDFDriver

ECHO Unregistering driver
RunDLL32.Exe SETUPAPI.DLL,InstallHinfSection DefaultUninstall 132 %WinDir%\System32\Drivers\%DRIVER_INF%

ECHO Deleting driver files
ERASE /F /Q %WinDir%\System32\Drivers\%DRIVER_CAT%
ERASE /F /Q %WinDir%\System32\Drivers\%DRIVER_INF%
ERASE /F /Q %WinDir%\System32\Drivers\%DRIVER_BIN%
ERASE /F /Q %WinDir%\System32\Drivers\%DRIVER_CERT%

:EoF