@echo off

:: *************************************************************************************************
:: Please do NOT change the following settings
:: *************************************************************************************************

title Flashing MasterDemo
mode con:cols=100 lines=50
set PlatformName=IMXRT1060-EVK

echo.
echo *******************************************************************************
echo *
echo * Flashing Embedded Wizard Master Demo for target %PlatformName%
echo *
echo *******************************************************************************
echo.

..\Application\FlashDownload\FlashDownload.cmd EmbeddedWizard-%PlatformName%.elf separateConsole reset
