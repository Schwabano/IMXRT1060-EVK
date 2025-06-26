@echo off

set Title=Embedded Wizard Build Environment for %PlatformName% V14.00.00.00
title %Title%


echo *******************************************************************************
echo * Welcome to %Title%
echo *******************************************************************************
echo.

IF DEFINED BuildInitialized GOTO run

GOTO setpaths

:setpaths

:: set the path to make and its tools
set PATH=%~dp0\..\..\..\ThirdParty\Make;%PATH%

:: compiler path
set PATH=%~dp0\..\..\..\ThirdParty\gcc-arm-none-eabi\bin;%PATH%

set REDLINK_UTILLITY=..\..\..\ThirdParty\MCUXpressoIDE\bin\crt_emu_cm_redlink.exe

set BuildInitialized=1

:run

echo.
echo *******************************************************************************
echo *
echo * Getting started with Embedded Wizard and %PlatformName% board:
echo *
echo * https://doc.embedded-wizard.de/%GettingStartedLink%
echo *
echo *******************************************************************************
echo.
echo.
echo Entering Embedded Wizard template project
echo.

cd Application\Project\GCC

