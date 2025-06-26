@echo off
setlocal
:loop
set file=%1
if "%file%" == "" goto endloop
set file=%file:/=\%
if not exist "%file%" mkdir "%file%"
shift
goto loop
:endloop
endlocal
