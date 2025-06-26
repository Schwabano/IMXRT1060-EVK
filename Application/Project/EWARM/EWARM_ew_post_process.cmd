@echo off
SETLOCAL EnableDelayedExpansion

set out_file="%EmWi_OutputDirectory%\ewfiles.ipcf"

if not exist "%EmWi_OutputDirectory%/../../PlatformPackage/%EmWi_ColorFormat%" (
echo "ERROR: The requested color format %EmWi_ColorFormat% is not supported for this platform!"
exit /B 1
)

if exist "%EmWi_OutputDirectory%/../../PlatformPackage/%EmWi_ColorFormat%/ewgfx.c" (
  set EmWi_RTESrcFiles=ewcolor.c ewdebug.c ewextrte.c ewobject.c ewpoint.c ewrect.c ewref.c ewresource.c ewscalars.c ewslot.c ewstring.c ewtimer.c ewheap.c ewinvocations.c

  if %EmWi_ColorFormat% == Index8 (
    set EmWi_GFX_SrcFiles=ewextbmp_Index8.c ewextfnt.c ewextgfx.c ewextpxl_Index8.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c ewgfxbidi.c ewgfxsvg.c ewextsvg.c
  )
  if %EmWi_ColorFormat% == LumA44 (
    set EmWi_GFX_SrcFiles=ewextbmp_LumA44.c ewextfnt.c ewextgfx.c ewextpxl_LumA44.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c ewgfxbidi.c ewgfxsvg.c ewextsvg.c
  )
  if %EmWi_ColorFormat% == RGB565 (
    set EmWi_GFX_SrcFiles=ewextbmp_RGB565.c ewextfnt.c ewextgfx.c ewextpxl_RGB565.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c ewgfxbidi.c ewgfxsvg.c ewextsvg.c
  )
  if %EmWi_ColorFormat% == RGB888 (
    set EmWi_GFX_SrcFiles=ewextbmp_RGB888.c ewextfnt.c ewextgfx.c ewextpxl_RGB888.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c ewgfxbidi.c ewgfxsvg.c ewextsvg.c
  )
  if %EmWi_ColorFormat% == RGBA4444 (
    set EmWi_GFX_SrcFiles=ewextbmp_RGBA4444.c ewextfnt.c ewextgfx.c ewextpxl_RGBA4444.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c ewgfxbidi.c ewgfxsvg.c ewextsvg.c
  )
  if %EmWi_ColorFormat% == RGBA8888 (
    set EmWi_GFX_SrcFiles=ewextbmp_RGBA8888.c ewextfnt.c ewextgfx.c ewextpxl_RGBA8888.c ewgfx.c ewgfxattrtext.c ewgfxcore.c ewgfxdriver.c ewgfxtasks.c ewgfxbidi.c ewgfxsvg.c ewextsvg.c
  )
) ELSE (
  set EmWi_RTESrcFiles=ewextrte.c libewrte-m7-iar.a
  if %EmWi_SurfaceRotation% == 0 (
    set EmWi_GFX_SrcFiles=ewextgfx.c libewgfx-m7-iar.a
  ) else (
    set EmWi_GFX_SrcFiles=ewextgfx.c libewgfx-m7-r%EmWi_SurfaceRotation%-iar.a
  )
)


@echo ^<?xml version="1.0" encoding="UTF-8"?^> > %out_file%
@echo ^<^^!-- IAR Project Connection File --^> >>  %out_file%
@echo ^<iarProjectConnection version="1.6" name="EmbeddedWizard" oneShot="false" >>  %out_file%
@echo   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" >>  %out_file%
@echo   xsi:noNamespaceSchemaLocation="IarProjectConnection_1.6.xsd"^> >>  %out_file%

@echo   ^<includePath^> >>  %out_file%
@echo      ^<path^>$PROJ_DIR$\..\..\..\PlatformPackage\RTE^</path^>  >>  %out_file%

:: The GFX include path has to be set by IAR Project Configuration, since
:: the previous set include path of another color format can not be removed by project connection
:: @echo      ^<path^>$PROJ_DIR$\..\..\..\PlatformPackage\%EmWi_ColorFormat%^</path^>  >>  %out_file%
::

@echo   ^</includePath^> >>  %out_file%

@echo   ^<defines^> >>  %out_file%
@echo     ^<define^>EW_SURFACE_ROTATION=%EmWi_SurfaceRotation%^</define^> >>  %out_file%
@echo     ^<define^>EW_FRAME_BUFFER_COLOR_FORMAT=EW_FRAME_BUFFER_COLOR_FORMAT_%EmWi_ColorFormat%^</define^> >>  %out_file%
@echo   ^</defines^> >>  %out_file%
@echo   ^<files^> >>  %out_file%

@echo     ^<group name="GeneratedCode"^> >>  %out_file%
for %%f in (%EmWi_OutputSrcFiles%) do ( @echo       ^<path^>%%f^</path^> >>  %out_file% )
@echo     ^</group^> >>  %out_file%

@echo     ^<group name="RTE"^> >>  %out_file%
for %%f in (%EmWi_RTESrcFiles%) do ( @echo       ^<path^>../../PlatformPackage/RTE/%%f^</path^> >>  %out_file% )
@echo     ^</group^> >>  %out_file%

@echo     ^<group name="%EmWi_ColorFormat%"^> >>  %out_file%
for %%f in (%EmWi_GFX_SrcFiles%) do ( @echo       ^<path^>../../PlatformPackage/%EmWi_ColorFormat%/%%f^</path^> >>  %out_file% )
@echo     ^</group^> >>  %out_file%

@echo     ^<group name="Header Files"^> >>  %out_file%
@echo     ^</group^> >>  %out_file%
@echo     ^<group name="Library Files"^> >>  %out_file%
@echo     ^</group^> >>  %out_file%
@echo     ^<group name="Documentation" build="false"^> >>  %out_file%
@echo     ^</group^> >>  %out_file%
@echo   ^</files^> >>  %out_file%
@echo ^</iarProjectConnection^> >>  %out_file%

:: IAR workaround GFX include path
if exist "%EmWi_OutputDirectory%\EmWiGfxInclude" (
  rmdir /S /Q "%EmWi_OutputDirectory%\EmWiGfxInclude"
)

mkdir "%EmWi_OutputDirectory%\EmWiGfxInclude"
xcopy /Q "%EmWi_OutputDirectory%\..\..\PlatformPackage\%EmWi_ColorFormat%\*.h" "%EmWi_OutputDirectory%\EmWiGfxInclude"
