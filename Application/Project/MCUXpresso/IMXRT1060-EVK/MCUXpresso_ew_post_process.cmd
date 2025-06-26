::*******************************************************************************
::
:: E M B E D D E D   W I Z A R D   P R O J E C T
::
::                                                Copyright (c) TARA Systems GmbH
::                                    written by Paul Banach and Manfred Schweyer
::
::
:: Embedded Wizard Post Process script for automatic updating of the MCUXpresso
:: project (color format, screen orientation, generated source files,
:: Embedded Wizard runtime environment libraries and source files.
:: Call this script from Embedded Wizard Studio. A Post Process can be set as
:: Profil parameter.
::
::*******************************************************************************

@echo off

SET mypath=%~dp0
SET realativepath=%mypath:~0,-1%
echo %realativepath%

PowerShell.exe -ExecutionPolicy Bypass -Command "& '%realativepath%\MCUXpresso_ew_post_process_power_shell_script.ps1'"

if ERRORLEVEL == 10 (
 echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 echo This Embedded Wizard Post Process needs at least V4.0 of Windows Powershell
 echo Powershell update can be found at:
 echo https://docs.microsoft.com/de-de/powershell/scripting/setup/installing-windows-powershell
 echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 exit /B 1
)

if ERRORLEVEL == 8 (
 echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 echo The selected color format '%EmWi_ColorFormat%' is not supported!
 echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 exit /B 1
)

if ERRORLEVEL == 6 (
 echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 echo The MCUXpresso project was not found!
 echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 exit /B 1
)

if ERRORLEVEL == 1 (
 echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 echo Unknown error occured during Embedded Wizard Post Process!
 echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 exit /B 1
)