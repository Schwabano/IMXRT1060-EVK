:: *****************************************************************************
:: *
:: * E M B E D D E D   W I Z A R D   P R O J E C T
:: *
:: *****************************************************************************
:: * FlashDownload.cmd
:: *
:: * This script uses the MCUXpresso Redlink Programmer to programm the flash
:: * memory of a NXP i.MX RT10xx device.
:: *
:: * Usage:    FlashDownload.cmd <elf-file or bin-file> [options]
:: *
:: * Options:
:: *   separateConsole          -  open a separate console window
:: *   eraseFlash               -  erase the complete flash memory before programming
:: *   imageStartAddress=<addr> -  defines the start address a binary file has
:: *                               to be programmed to
:: *   reset                    -  reset target after flashing
:: *
:: *****************************************************************************


@ECHO OFF
setlocal ENABLEDELAYEDEXPANSION

:: Load commandline options
call :getCommadlineOptions "%*"

:: change size of console
if [%separateConsole%] == [set] (
  title Programming taget using MCUXpresso Redlink Programmer ...
  mode con:cols=140 lines=50
)

:: load configuration
for /F "delims= eol=:" %%A in (%~dps0FlashDownload.cfg) do set "%%A"


set pathOfThisScript=%~dp0
set RedlinkProgrammer=%pathOfThisScript%..\..\ThirdParty\MCUXpressoIDE\bin\crt_emu_cm_redlink.exe


:: do checks
if not exist "%RedlinkProgrammer%" (
  echo.
  echo Redlink Programmer check failed:
  echo Redlink Programmer not found on path:
  echo "%RedlinkProgrammer%",
  echo that should be part of the Embedded Wizard Build Environment.
  echo.
  pause
  exit
)

set RedlinkProgrammerCall=%RedlinkProgrammer% --vendor NXP -x %pathOfThisScript% -p %PackageName% --flash-dir %pathOfThisScript% -g %AdditionalProgrammerParams%

echo Programming file '%~nx1' to target using MCUXpresso IDE Redlink Programmer ...

if [%eraseFlash%] == [set] (
  echo erasing flash ...
  %RedlinkProgrammerCall% --flash-mass-erase > NUL
)

if not [%imageStartAddress%] == [] (
  set flashStartAddress=--base %imageStartAddress%
)

echo flashing target ...
%RedlinkProgrammerCall% --flash-load %1 %flashStartAddress%

:: output result
echo.
if %ERRORLEVEL% == 0 (
  echo '%~nx1' successfully flashed !!
) ELSE (
  echo ERROR - flashing failed!
  echo.
  call :doPause
)

if [%reset%] == [set] (
  echo.
  echo reseting target ...
  %RedlinkProgrammerCall% --reset hard  > NUL
)

if [%separateConsole%] == [set] (
  timeout /t 3 > NUL
)
exit /B 0



:getCommadlineOptions
  :: replace equal signs by ':'
  for /f "tokens=1-10 delims==" %%a in (%1) do set list3=%%a:%%b:%%c:%%d:%%e:%%f:%%g:%%h:%%i:%%j

  for %%i in (%list3%) do (
    set var=%%i
    call :setvar !var::= !
  )
  exit /B 0

:setvar
  if x%2==x (
    set %1=set
  ) else (
    set %1=%2
  )
  exit /B 0

:doPause
  if [%separateConsole%] == [set] (
    pause
  )
  exit /B 0