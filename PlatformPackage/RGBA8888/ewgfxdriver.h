/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This software and related documentation ("Software") are intellectual
* property owned by TARA Systems and are copyright of TARA Systems.
* Any modification, copying, reproduction or redistribution of the Software in
* whole or in part by any means not in accordance with the End-User License
* Agreement for Embedded Wizard is expressly prohibited. The removal of this
* preamble is expressly prohibited.
*
********************************************************************************
*
* DESCRIPTION:
*   This is an internal header of the Embedded Wizard Graphics Engine EWGFX.
*
*   The module 'ewgfxdriver' implements the platform independent layer to the
*   software pixel driver. Its functionality provides a kind of reserve in case
*   the underlying graphics subsystem can not handle the necessary operations.
*   For example, if there is no support for index8 surfaces, 'ewgfxdriver' will
*   recompense it by a software emulated index8 surface including all necessary
*   drawing operations.
*
*   This header file provides a generic and platform independent interface to
*   the software pixel driver functionality. The interface consists of two
*   categories of functions:
*
*   1. 'per row' worker functions. These functions build the low-level software
*      pixel driver. They are intended to perform drawing operations optimized
*      for the particular pixel format and the drawing mode. The declarations,
*      any way, are platform independent.
*
*   2. 'per area' drawing functions. These functions provide the top-level view
*      to the software pixel driver. They are intended to drive the 'per row'
*      worker functions for the affected fill/copy/warp operation. When called,
*      the top-level function will receive a pointer to one of the low-level
*      worker functions. The passed worker function can be used thereupon to
*      perform row-wise the operation without any dependency to the underlying
*      pixel format.
*
*   Additionally, the header file provides generic declarations to simplify
*   the integration of external graphics subsystem drivers into the Embedded
*   Wizard Graphics Engine.
*
*******************************************************************************/

#ifndef EWGFXDRIVER_H
#define EWGFXDRIVER_H


#ifdef __cplusplus
  extern "C"
  {
#endif


/*******************************************************************************
* MACRO:
*   EW_RED
*   EW_GREEN
*   EW_BLUE
*   EW_ALPHA
*
* DESCRIPTION:
*   The following macros provide a generic way to extract the RGBA color
*   channels from the RGBA8888 aColor value as it is passed to all driver
*   functions.
*
*   Note, the RGBA8888 color format is the internal, platform independent way
*   to store color values.
*
*******************************************************************************/
#define EW_RED( aColor )   ((( aColor ) >> 0  ) & 0xFF )
#define EW_GREEN( aColor ) ((( aColor ) >> 8  ) & 0xFF )
#define EW_BLUE( aColor )  ((( aColor ) >> 16 ) & 0xFF )
#define EW_ALPHA( aColor ) ((( aColor ) >> 24 ) & 0xFF )


/*******************************************************************************
* MACRO:
*   EW_PIXEL_FORMAT_XXX
*
* DESCRIPTION:
*   The following enumeration defines the 3 fundamental surface formats. They
*   are needed amongst other things to specify the desired pixel format, when
*   creating new surfaces or when accessing surface memory directly.
*
* ELEMENTS:
*   EW_PIXEL_FORMAT_SCREEN - Represents the pixel format of the frame buffer.
*     Usually this format corresponds to the EW_PIXEL_FORMAT_NATIVE. In special
*     cases, however, it is necessary to distinguish between operations to run
*     on the frame buffer or on the normal (native) destination when these use
*     different color formats.
*     The pixel format does not make any assumption regarding the respective
*     color space, color channels, etc. In this manner the Graphics Engine
*     remains platform and color independent.
*     Please note, surfaces with this color format can serve as a destination
*     only in all drawing operations.
*   EW_PIXEL_FORMAT_NATIVE - Represents the main pixel format valid within the
*     particular graphics subsystem without any assumptions about the respective
*     color space, color channels, etc. This is the pixel format, the subsystem
*     is working with internally. In this manner the Graphics Engine remains
*     platform and color independent.
*     Please note, surfaces with this color format can serve as a destination
*     and source in all drawing operations.
*   EW_PIXEL_FORMAT_INDEX8 - Universal 8 bit per pixel CLUT format. This format
*     expects an additional color look-up table for translation between an 8 bit
*     index and the native color value.
*     Please note, surfaces with this color format can serve as a source only
*     in all drawing operations.
*   EW_PIXEL_FORMAT_ALPHA8 - Universal 8 bit per pixel alpha only format. Useful
*     as storage for glyphs, masks, etc.
*     Please note, surfaces with this color format can serve as a source only
*     in all drawing operations.
*   EW_PIXEL_FORMAT_RGB565 - Represents a color-only pixel format with 16-bit
*     per pixel. The support for this format is optional depending on the used
*     target system and the Native color format in the target system.
*     Please note, surfaces with this color format can serve as a source only
*     in all drawing operations.
*
*******************************************************************************/
#define EW_PIXEL_FORMAT_SCREEN -1
#define EW_PIXEL_FORMAT_NATIVE  0
#define EW_PIXEL_FORMAT_INDEX8  1
#define EW_PIXEL_FORMAT_ALPHA8  2
#define EW_PIXEL_FORMAT_RGB565  3


/*******************************************************************************
* MACRO:
*   EW_DRIVER_VARIANT_XXX
*
* DESCRIPTION:
*   Following macros specify the basic color formats supported by the Graphics
*   Engine. These are used for verification purpose and plausibility tests.
*
*******************************************************************************/
#define EW_DRIVER_VARIANT_RGBA8888  1
#define EW_DRIVER_VARIANT_RGBA4444  2
#define EW_DRIVER_VARIANT_RGB565A8  3
#define EW_DRIVER_VARIANT_RGB555A8  4
#define EW_DRIVER_VARIANT_YUVA8888  5
#define EW_DRIVER_VARIANT_LUMA44    6
#define EW_DRIVER_VARIANT_RGB565    7
#define EW_DRIVER_VARIANT_RGB888    8
#define EW_DRIVER_VARIANT_INDEX8    11
#define EW_DRIVER_VARIANT_ALPHA8    12
#define EW_DRIVER_VARIANT_RGB565_RGBA8888                                      \
  (( EW_DRIVER_VARIANT_RGB565 << 8 ) | EW_DRIVER_VARIANT_RGBA8888 )
#define EW_DRIVER_VARIANT_RGB888_RGBA8888                                      \
  (( EW_DRIVER_VARIANT_RGB888 << 8 ) | EW_DRIVER_VARIANT_RGBA8888 )


/*******************************************************************************
* VARIABLE:
*   EwPixelDriverVariant
*
* DESCRIPTION:
*   This global variable stores the target basic color format supported by this
*   pixel driver. It is used at the runtime to verify, whether correct pixel
*   driver are linked together with the application.
*
*   This variable can take one of EW_DRIVER_VARIANT_XXX values.
*
*******************************************************************************/
extern const int EwPixelDriverVariant;


/*******************************************************************************
* TYPE:
*   XGraphicsEngineInfo
*
* DESCRIPTION:
*   The structure XGraphicsEngineInfo stores all essential parameters of the
*   Graphics Engine.
*
* ELEMENTS:
*   Version                          - The version of the Graphics Engine as
*     32-bit value with the major number in upper 16-bit and minor number in
*     lower 16-bit indicating the recent version of the GFX library.
*
*   DoesSupportGradients,
*   DoesSupportPaths,
*   DoesSupportWarp,
*   DoesSupportBlur,
*   DoesSupportMask,
*   DoesSupportTint,
*   DoesSupportBIDI                  - != 0 if the Graphics Engine does include
*     the support for the corresponding feature. If the feature is removed from
*     the Graphics Engine, the respective value is 0. The features are removed
*     by explicitly defining one of the following macros during the compilation
*     of the Graphics Engine: EW_DONT_USE_GRADIENTS, EW_DONT_USE_PATH_FUNCTIONS,
*     EW_DONT_USE_WARP_FUNCTIONS, EW_DONT_USE_BLUR_FUNCTIONS,
*     EW_DONT_USE_MASK_FUNCTIONS, EW_DONT_USE_TINT_FUNCTIONS,
*     EW_DONT_USE_BIDI_FUNCTIONS.
*
*   DoesSupportCompression           - != 0 if the Graphics Engine does support
*     compressed bitmap resources. If the feature is removed from the Graphics
*     Engine, the value is 0. The feature is removed by explicitly defining the
*     macro EW_DONT_USE_COMPRESSION during the compilation of the Graphics
*     Engine.
*
*   DoesSupportNativeSurfaces
*   DoesSupportIndex8Surfaces
*   DoesSupportRGB565Surfaces
*   DoesSupportScreenSurfaces        - != 0 if the Graphics Engine does include
*     support for surfaces (bitmaps) stored in the format Native, Index8, RGB565
*     or Screen. If the support is not included, the respective value is 0.
*     Whether particular surface type is supported does depend primarily on the
*     version of the Graphics Engine. By defining one of the following macros
*     during the compilation of the Graphics Engine, the support can be removed
*     or added explicitly: EW_DONT_USE_NATIVE_SURFACES,
*     EW_DONT_USE_INDEX8_SURFACES, EW_DONT_USE_RGB565_SURFACES,
*     EW_USE_PIXEL_FORMAT_SCREEN.
*
*   FormatOfNativeSurfaces           - Stores the pixel format used by native
*     surface or 0 if there is no support for native surface format. Possible
*     values are:
*       EW_DRIVER_VARIANT_RGBA8888 
*       EW_DRIVER_VARIANT_RGBA4444
*       EW_DRIVER_VARIANT_LUMA44
*       EW_DRIVER_VARIANT_INDEX8
*
*   FormatOfScreenSurfaces           - Stores the pixel format used by screen
*     surface or 0 if there is no support for screen surface format. Possible
*     values are:
*       EW_DRIVER_VARIANT_RGB565
*       EW_DRIVER_VARIANT_RGB888
*
*   ColorChannelBitOffsetLum
*   ColorChannelBitOffsetRed
*   ColorChannelBitOffsetGreen
*   ColorChannelBitOffsetBlue
*   ColorChannelBitOffsetAlpha       - Determine the bit-positions of color
*     channels how these are arranged in pixel of a native surface. The values
*     Lum and (Red,Green,Blue) are used exclusively depending on the format of
*     the native surface. The values found here do correspond to definitions
*     of the macros EW_COLOR_CHANNEL_BIT_OFFSET_XXX. If there is no support for
*     native surface available, the values are 0. 
*
*   ScreenColorChannelBitOffsetRed
*   ScreenColorChannelBitOffsetGreen
*   ScreenColorChannelBitOffsetBlue  -  Determine the bit-positions of color
*     channels how these are arranged in pixel of a screen surface. The values
*     found here does correspond to definitions of the macros
*     EW_SCREEN_COLOR_CHANNEL_BIT_OFFSET_XXX. If there is no support for screen
*     surface available, the values are 0. 
*
*   RGB565ColorChannelBitOffsetRed
*   RGB565ColorChannelBitOffsetGreen
*   RGB565ColorChannelBitOffsetBlue  - Determine the bit-positions of color
*     channels how these are arranged in pixel of an RGB565 surface. The values
*     found here does correspond to definitions of the macros
*     EW_RGB565_COLOR_CHANNEL_BIT_OFFSET_XXX. If there is no support for RGB565
*     surface available, the values are 0. 
*
*   PremultiplyColorChannels         - != 0 if the Graphics Engine calculates
*     with color channels (Red, Green, Blue or Lum) being premultiplied by the
*     corresponding Alpha value. If there is no support for native surface
*     available, the value is 0. The value corresponds to the definition of the
*     macro EW_PREMULTIPLY_COLOR_CHANNELS.
*
*******************************************************************************/
typedef struct
{
  unsigned int      Version;
  unsigned char     DoesSupportGradients;
  unsigned char     DoesSupportPaths;
  unsigned char     DoesSupportWarp;
  unsigned char     DoesSupportBlur;
  unsigned char     DoesSupportMask;
  unsigned char     DoesSupportTint;
  unsigned char     DoesSupportBIDI;
  unsigned char     DoesSupportCompression;
  unsigned char     Reserved1[8];
  unsigned char     DoesSupportNativeSurfaces;
  unsigned char     DoesSupportIndex8Surfaces;
  unsigned char     DoesSupportRGB565Surfaces;
  unsigned char     DoesSupportScreenSurfaces;
  unsigned char     Reserved2[4];
  unsigned int      FormatOfNativeSurfaces;
  unsigned int      FormatOfScreenSurfaces;
  unsigned char     ColorChannelBitOffsetLum;
  unsigned char     ColorChannelBitOffsetRed;
  unsigned char     ColorChannelBitOffsetGreen;
  unsigned char     ColorChannelBitOffsetBlue;
  unsigned char     ColorChannelBitOffsetAlpha;
  unsigned char     ScreenColorChannelBitOffsetRed;
  unsigned char     ScreenColorChannelBitOffsetGreen;
  unsigned char     ScreenColorChannelBitOffsetBlue; 
  unsigned char     RGB565ColorChannelBitOffsetRed;  
  unsigned char     RGB565ColorChannelBitOffsetGreen;
  unsigned char     RGB565ColorChannelBitOffsetBlue; 
  unsigned char     PremultiplyColorChannels;
}
XGraphicsEngineInfo;


/*******************************************************************************
* TYPE:
*   XSurfaceMemory
*
* DESCRIPTION:
*   The XSurfaceMemory structure provides a lightweight storage for adresses to
*   color planes and (if any) to the color table of a previously locked surface.
*   It is used by the software pixel driver to read and modify surface content.
*
* ELEMENTS:
*   Pixel1  - Pointer to the first surface plane.
*   Pitch1X - Distance between two pixel columns of the first surface plane.
*   Pitch1Y - Distance between two pixel rows of the first surface plane.
*   Pixel2  - Pointer to the second (optional) surface plane.
*   Pitch2X - Distance between two pixel columns of the second surface plane.
*   Pitch2Y - Distance between two pixel rows of the second surface plane.
*   Clut    - Pointer to the (optional) color table of the surface. In order
*     to cover all color formats, a single entry of the clut is always 32 bit
*     wide - even when the target color format is 16 or 8 bit wide.
*   Reader  - This value is optional and exists on target systems configured to
*     store constant surfaces (resources) in a not directly accessible memory
*     area. In this case the Pixel Driver will invoke the specified function
*     Reader to ensure that the data referenced by the pointer is loaded in a
*     RAM area and return the pointer to the loaded data entry within this area.
*
*******************************************************************************/
typedef struct
{
  void*             Pixel1;
  int               Pitch1X;
  int               Pitch1Y;
  void*             Pixel2;
  int               Pitch2X;
  int               Pitch2Y;
  unsigned int*     Clut;

#ifdef EW_USE_READER_FOR_CONST_SURFACES 
  XFlashAreaReaderProc Reader;
#endif
} XSurfaceMemory;


/*******************************************************************************
* TYPE:
*   XGradient
*
* DESCRIPTION:
*   The XGradient structure provides a storage for the components of a 2D color
*   or 2D opacity gradient. It defines a start value and slopes. For precision
*   purpose, the values are codes as 12.20 fixed point numbers.
*
* ELEMENTS:
*   R0, R1, R2, R3 - Startvalues and slopes for the red channel.
*   G0, G1, G2, G3 - Startvalues and slopes for the green channel.
*   B0, B1, B2, B3 - Startvalues and slopes for the blue channel.
*   A0, A1, A2, A3 - Startvalues and slopes for the alpha channel.
*   Width, Height  - Size of the gradient area.
*   InvWidth,
*   InvHeight      - Reciprocal values of the gradient area width and height.
*   IsVertical     - != 0 if the gradient has a vertical trend.
*   IsHorizontal   - != 0 if the gradient has a horizontal trend.
*
*******************************************************************************/
typedef struct
{
  int               R0, R1, R2, R3;
  int               G0, G1, G2, G3;
  int               B0, B1, B2, B3;
  int               A0, A1, A2, A3;
  int               Width;
  int               Height;
  int               InvWidth;
  int               InvHeight;
  int               IsVertical;
  int               IsHorizontal;
} XGradient;


/*******************************************************************************
* TYPE:
*   XGradientStop
*
* DESCRIPTION:
*   The XGradientStop structures defines a single color stop within a color
*   gradient.
*
* ELEMENTS:
*   Offset - Offset in range 0.0 .. 1.0 defining the position of the color stop
*     within the gradient.
*   Color  - Color value associated to the color stop.
*******************************************************************************/
typedef struct
{
  float             Offset;
  unsigned int      Color;
} XGradientStop;


/*******************************************************************************
* TYPE:
*   XDeleteSurfaceProc
*
* DESCRIPTION:
*  The following type declares a prototype of a user defined function, which
*  can be passed as callback during the creation of a surface with wrapped 
*  image content.
*
*  This callback function will then be called when the surface is deleted. In
*  this manner the original wrapped image content can be disposed correctly.
*
* ARGUMENTS:
*   aHandle - Value identifying the image content wrapped within the surface.
*     This can be, e.g. a texture-id in case of an OpenGL target system or a
*     pointer to memory containing the pixel data, etc. This value is specified
*     at the creation time of the surface intended to 'wrap' the foreign image
*     content.
*
*   aArg    - User defined argument specified at the creation time of the
*    surface. This value is simply passed during the callback invocation.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XDeleteSurfaceProc)
(
  void*             aHandle,
  void*             aArg
);


/*******************************************************************************
* PROTOTYPE:
*   XLineWorker
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'draw line pixel' operation.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination surface.
*   aX, aY    - Position of the pixel to draw.
*   aGradient - Color information to draw the pixel.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XLineWorker)
(
  XSurfaceMemory*   aDst,
  int               aX,
  int               aY,
  XGradient*        aGradient
);


/*******************************************************************************
* PROTOTYPE:
*   XFillWorker
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'fill pixel row' operation.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the affected row.
*   aWidth    - Length of the row in pixel.
*   aGradient - Color information to fill the row.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XFillWorker)
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* PROTOTYPE:
*   XCopyWorker
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'copy pixel row' operation.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Length of the row in pixel.
*   aGradient - Color or opacity information to modulate the copied pixel.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XCopyWorker)
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* PROTOTYPE:
*   XWarpWorker
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'warp pixel row' operation.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination row.
*   aSrc          - Pointer to the first pixel of the source surface area to
*     project on the destination row.
*   aWidth        - Length of the row in pixel.
*   aS, aT        - Source surface coordinates corresponding to the first pixel
*     of the destination row.
*   aSS, aTS      - Factors to interpolate the s and t coefficients for next
*     following pixel of the destination row.
*   aSrcWidth,
*   aSrcHeight    - Size of the source area in pixel to limit the operation.
*   aGradient     - Color or opacity information to modulate the copied pixel.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XWarpWorker)
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* MACRO:
*   EW_TINT_MODE_XXX
*
* DESCRIPTION:
*   The following enumerations define the possible filter modes for the function
*   EwFilterBitmap().
*
* ELEMENTS:
*   EW_TINT_MODE_NONE              - No tint mode specified. Original colors
*     resulting from the source image are used without any modification.
*
*   EW_TINT_MODE_GRAYSCALE         - Converting RGB colors to grayscale values.
*     The original RGB colors resulting from the source image are converted to
*     grayscale values according to their luminance. The opacity information is
*     taken over without any modifications. The new color value replaces the 
*     original color information. Following are the corresponding equations:
*
*        new_color.rgb = luminance( org_color.rgb )
*        new_color.a   = old_color.a
*
*   EW_TINT_MODE_TINT_BY_OPACITY   - Color modulation by opacity. The opacity
*     resulting from the source image is used to modulate a configured color
*     value. This new color value replaces the original color information.
*     Following is the corresponding equations:
*
*        new_color = org_color.alpha * config_color
*
*   EW_TINT_MODE_TINT_BY_LUMINANCE - Color modulation by luminance. Luminance
*     resulting from the source image is used to modulate the RGB components of
*     a configured color value. The opacity resulting from the source image is
*     used to modulate the opacity of this configured color value. This new
*     color value replaces the original color information. Following are the
*     corresponding equations:
*
*        new_color.rgb = luminance( org_color.rgb ) * config_color.rgb
*        new_color.a   = org_color.a                * config_color.a
*
*******************************************************************************/
#define EW_TINT_MODE_NONE              0 
#define EW_TINT_MODE_GRAYSCALE         1
#define EW_TINT_MODE_TINT_BY_OPACITY   2
#define EW_TINT_MODE_TINT_BY_LUMINANCE 3


/*******************************************************************************
* PROTOTYPE:
*   XMaskTintWorker
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'mask and/or tint pixel row' operation.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination row.
*   aSrc          - Pointer to the first pixel of the source row.
*   aMask         - Pointer to the first pixel of the mask row or NULL if no
*     mask is used.
*   aWidth        - Length of the row in pixel.
*   aColor        - A color value to be used by the tint filter stored in a
*     XGradient data structure.
*   aIntensity    - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0 .. 255. If the value is 255, the original
*     colors are completely replaced by values resulting from the filter. If
*     the value is equal zero, the original colors are used.
*   aOpacity      - Modulates the opacity of the pixel data resulting from the
*     filter operation. This parameter can take values in range of 0 .. 255.
*   aBlend        - If != 0, the drawn pixel will be alpha-blended with the
*     pixel in the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XMaskTintWorker)
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* PROTOTYPE:
*   XBlurWorker
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'blur pixel sequence' operation. Please note,
*   unlike other worker functions, this version is not limitted to perform the
*   operation row-wise. The pixel can also be read/written column-wise.
*
*   The funcion calculates the blur effect for the given number of pixel within
*   the destination surface. Due to the blur effect, the source read operations
*   may involve more source pixel before and after the sequence. The larger the
*   blur radius, the more source pixel are involved.
*
* ARGUMENTS:
*   aDst     - Pointer to the first pixel of the destination sequence.
*   aSrc     - Pointer to the first pixel of the source sequence.
*   aCount   - Length of the sequence in pixel.
*   aLimit1,
*   aLimit2  - Offsets to the first and to the last pixel within the current row
*     calculated relative to the address specified in aSrc. The blur algorithm
*     considers only source pixel lying within this area as valid. Any further
*     pixel beyond this limit is considered as being transparent.
*   aClamp1  - Number of source pixel preceding this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aClamp2  - Number of source pixel following this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aRadius  - Blur radius in pixel.
*   aOpacity - Modulates the opacity of the pixel data resulting from the blur
*     operation. This parameter can take values in range of 0 .. 255.
*   aBlend   - If != 0, the drawn pixel will be alpha-blended with the pixel in
*     the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XBlurWorker)
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aCount,
  int               aLimit1,
  int               aLimit2,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* PROTOTYPE:
*   XFillLinearGradientWorker
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'fill linear gradient' operation.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aMask      - Pointer to the first pixel of the mask ALPHA8 row.
*   aWidth     - Width of the row.
*   aDeltaX,
*   aDeltaY    - Distance between the first pixel of the row and the start
*     position of the gradient.
*   aUnitX,
*   aUnitY     - Unit vector describing the direction of the line. Expressed in
*     0.16 bit fix-point integer.
*   aNoOfStops - No of color stops belonging to the gradient.
*   aOffsets   - An array of offsets corresponding to the color stops.
*   aColors    - An array of colors corresponding to the color stops.
*     The colors are stored in packed format (see EwPackColor()).
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XFillLinearGradientWorker)
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aDeltaX,
  int               aDeltaY,
  int               aUnitX,
  int               aUnitY,
  int               aNoOfStops,
  int*              aOffsets,
  unsigned int*     aColors
);


/*******************************************************************************
* PROTOTYPE:
*   XFillRadialGradientWorker
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'fill radial gradient' operation.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aMask      - Pointer to the first pixel of the mask ALPHA8 row.
*   aWidth     - Width of the row.
*   aDeltaX,
*   aDeltaY    - Distance between the first pixel of the row and the start
*     position of the gradient.
*   aMatrix    - 2x2 component matrix describing evtl. scaling/rotation of
*     gradient coordinates. The components of the matrix are stored in 16.16
*     precision. If NULL no transformation is applied.
*   aNoOfStops - No of color stops belonging to the gradient.
*   aOffsets   - An array of offsets corresponding to the color stops.
*   aColors    - An array of colors corresponding to the color stops.
*     The colors are stored in packed format (see EwPackColor()).
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XFillRadialGradientWorker)
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aDeltaX,
  int               aDeltaY,
  int*              aMatrix,
  int               aNoOfStops,
  int*              aOffsets,
  unsigned int*     aColors
);


/*******************************************************************************
* PROTOTYPE:
*   XLineDriver
*
* DESCRIPTION:
*   The following type declares a prototype for an external graphics subsystem
*   function, which should perform the 'draw line' operation by using solid or
*   gradient color values.
*
* ARGUMENTS:
*   aDstHandle  - Handle to the screen/native destination surface.
*   aDstX1,
*   aDstY1      - Start position to draw the line (relative to the top-left
*      corner of the destination surface).
*   aDstX2,
*   aDstY2      - End position to draw the line (relative to the top-left
*      corner of the destination surface). The pixel at the end position does
*      not belong to the line - it is invisible. In this manner polylines can
*      be drawn.
*   aClipX,
*   aClipY,
*   aClipWidth,
*   aClipHeight - Optional clipping area relative to the top-left corner of the
*     destination surface. No pixel outside this area may be drawn.
*   aBlend      - != 0 if the operation should be performed with alpha blending.
*   aColors     - Array with 2 RGBA8888 color values. The both color values do
*     correspond to the start and to the end pixel of the drawn line.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XLineDriver)
(
  void*             aDstHandle,
  int               aDstX1,
  int               aDstY1,
  int               aDstX2,
  int               aDstY2,
  int               aClipX,
  int               aClipY,
  int               aClipWidth,
  int               aClipHeight,
  int               aBlend,
  unsigned long*    aColors
);


/*******************************************************************************
* PROTOTYPE:
*   XFillDriver
*
* DESCRIPTION:
*   The following type declares a prototype for an external graphics subsystem
*   function, which should perform the 'fill rectangular area' operation by
*   using solid or gradient color values.
*
* ARGUMENTS:
*   aDstHandle  - Handle to the screen/native destination surface.
*   aDstX,
*   aDstY       - Origin of the area to fill (relative to the top-left corner
*      of the destination surface).
*   aWidth,
*   aHeight     - Size of the area to fill.
*   aBlend      - != 0 if the operation should be performed with alpha blending.
*   aColors     - Array with 4 RGBA8888 color values. The four color values do
*     correspond to the four corners of the area: top-left, top-right, bottom-
*     right and bottom-left.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XFillDriver)
(
  void*             aDstHandle,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aBlend,
  unsigned long*    aColors
);


/*******************************************************************************
* PROTOTYPE:
*   XCopyDriver
*
* DESCRIPTION:
*   The following type declares a prototype for an external graphics subsystem
*   function, which should perform the 'copy rectangular area' operation from a
*   native, index8, alpha8 or rgb565 surface to a screen or native surface. The
*   copied pixel can optionally be modulated by solid or gradient color/opacity
*   values.
*
* ARGUMENTS:
*   aDstHandle  - Handle to the screen/native destination surface.
*   aSrcHandle  - Handle to the native/index8/alpha8/rgb565 source surface.
*   aDstX,
*   aDstY       - Origin of the area to fill with the copied source surface
*     pixel (relative to the top-left corner of the destination surface).
*   aWidth,
*   aHeight     - Size of the area to fill with the copied source surface pixel.
*   aSrcX,
*   aSrcY       - Origin of the area to copy from the source surface.
*   aBlend      - != 0 if the operation should be performed with alpha blending.
*   aColors     - Array with 4 color values. These four values do correspond
*     to the four corners of the area: top-left, top-right, bottom-right and
*     bottom-left.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XCopyDriver)
(
  void*             aDstHandle,
  void*             aSrcHandle,
  int               aDstX,
  int               aDstY,
  int               aSrcX,
  int               aSrcY,
  int               aWidth,
  int               aHeight,
  int               aBlend,
  unsigned long*    aColors
);


/*******************************************************************************
* PROTOTYPE:
*   XTileDriver
*
* DESCRIPTION:
*   The following type declares a prototype for an external graphics subsystem
*   function, which will perform the 'multiple copy rectangular area' operation
*   from a native, index8, alpha8 or rgb565 surface to a screen or native surface.
*   The copied pixel can be modulated by solid or gradient color/opacity values.
*
* ARGUMENTS:
*   aDstHandle  - Handle to the screen/native destination surface.
*   aSrcHandle  - Handle to the native/index8/alpha8/rgb565 source surface.
*   aDstX,
*   aDstY       - Origin of the area to fill with the copied source surface
*     pixel (relative to the top-left corner of the destination surface).
*   aDstWidth,
*   aDstHeight  - Size of the area to fill with the copied source surface pixel.
*   aSrcX,
*   aSrcY       - Origin of the area to copy from the source surface.
*   aSrcWidth,
*   aSrcHeight  - Size of the source area to copy.
*   aOfsX,
*   aOfsY       - Offset to the first visible source pixel at the origin of the
*     destination area.
*   aBlend      - != 0 if the operation should be performed with alpha blending.
*   aColors     - Array with 4 color values. These four values do correspond
*     to the four corners of the area: top-left, top-right, bottom-right and
*     bottom-left.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XTileDriver)
(
  void*             aDstHandle,
  void*             aSrcHandle,
  int               aDstX,
  int               aDstY,
  int               aDstWidth,
  int               aDstHeight,
  int               aSrcX,
  int               aSrcY,
  int               aSrcWidth,
  int               aSrcHeight,
  int               aOfsX,
  int               aOfsY,
  int               aBlend,
  unsigned long*    aColors
);


/*******************************************************************************
* PROTOTYPE:
*   XWarpDriver
*
* DESCRIPTION:
*   The following type declares a prototype for an external graphics subsystem
*   function, which can perform the 'copy and warp rectangular area' operation
*   from native, index8, alpha8 or rgb565 surface to a screen or native surface.
*   The copied pixel can optionally be modulated by solid or gradient color/
*   opacity values.
*
*   The warp operation abstracts the scale, rotate and perspective projection
*   operations. The operation is limited to convex polygons only. In case of a
*   non convex polygon, the operation may fail.
*
* ARGUMENTS:
*   aDstHandle  - Handle to the screen/native destination surface.
*   aSrcHandle  - Handle to the native/index8/alpha8/rgb565 source surface.
*   DstX1,
*   DstY1,
*   DstW1,
*   ...
*   DstX4,
*   DstY4,
*   DstW4       - Coordinates of the polygon to fill with the source bitmap as
*     floating point values for sub-pixel precision (Relative to the top-left
*     corner of the destination bitmap). The 'W' coefficients do control the
*     perspective distortion as it is realized in 3D graphics hardware (see
*     OpenGL homogeneous coordinates for more information).
*   aSrcX,
*   aSrcY       - Origin of the area to copy from the source surface.
*   aSrcWidth,
*   aSrcHeight  - Size of the source area to copy.
*   aClipX,
*   aClipY,
*   aClipWidth,
*   aClipHeight - Optional clipping area relative to the top-left corner of the
*     destination surface. No pixel outside this area may be drawn.
*   aBlend      - != 0 if the operation should be performed with alpha blending.
*   aFilter     - != 0 if the bi-linear filter should be applied to the copied
*      pixel. This argument is optional and controls the output quality only.
*   aColors     - Array with 4 color values. These four values do correspond to
*     the respective corners of the polygon.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XWarpDriver)
(
  void*             aDstHandle,
  void*             aSrcHandle,
  float             aDstX1,
  float             aDstY1,
  float             aDstW1,
  float             aDstX2,
  float             aDstY2,
  float             aDstW2,
  float             aDstX3,
  float             aDstY3,
  float             aDstW3,
  float             aDstX4,
  float             aDstY4,
  float             aDstW4,
  int               aSrcX,
  int               aSrcY,
  int               aSrcWidth,
  int               aSrcHeight,
  int               aClipX,
  int               aClipY,
  int               aClipWidth,
  int               aClipHeight,
  int               aBlend,
  int               aFilter,
  unsigned long*    aColors
);


/*******************************************************************************
* PROTOTYPE:
*   XFilterDriver
*
* DESCRIPTION:
*   The following type declares a prototype for an external graphics subsystem
*   function, which should perform the 'filter' operation on a native, index8,
*   alpha8 or rgb565 surface to a screen or native surface. The operation may
*   include a 'mask', 'blur' and 'tint' filter effects. These can be combined
*   or used individually. For more details concerning the filter operaions see
*   the top-level function EwFilterBitmap().
*
* ARGUMENTS:
*   aDstHandle      - Handle to the screen/native destination surface.
*   aSrcHandle      - Handle to the native/index8/alpha8/rgb565 source surface.
*   aMaskHandle     - Handle to the alpha8 mask surface or NULL.
*   aDstX,
*   aDstY,
*   aDstWidth,
*   aDstHeight      - Area affected by the drawing operation (relative to the
*     top-left corner of the destination surface).
*   aSrcX,
*   aSrcY,
*   aSrcWidth,
*   aSrcHeight      - Area within the source surface relative to its top-left
*     corner to use as source.
*   aSrcPosX,
*   aSrcPosY        - Position within the source surface to map to the origin
*    (top-left corner) of the destination area (aDstX,aDstY).
*   aMaskPosX,
*   aMaskPosY       - Position within the mask surface to map to the origin
*     (top-left corner) of the destination area (aDstX,aDstY).
*   aClipX,
*   aClipY,
*   aClipWidth,
*   aClipHeight     - Area to limit the drawing operation (Relative to the top-
*     left corner of the destination surface).
*   aMaskInverted   - This parameter controls how pixel data from the surface
*     specified in the parameter aMaskHandle are evaluated. If aMaskInverted is
*     'true', the data from the mask are inverted resulting in opaque pixel
*     wherever the mask does contain a transparent pixel and transparent pixel
*     wherever an opaque pixel is found in the mask surface. Areas outside the
*     mask surface are conseqently considered as opaque when aMaskInverted is
*     set 'true'.
*   aMaskBeforeBlur - The parameter aMaskBeforeBlur controls the order in which
*     mask and blur operations are performed. If aMaskBeforeBlur is 'false',
*     the mask is applied after the blur effect. This allows the blur effect to
*     be clipped by the mask. If aMaskBeforeBlur is 'true', the blur effect is
*     applied on previously masked image content. The blur effect considers the
*     masked out areas as being transparent which allows the edges between the
*     regular and masked out areas to appear blurred.
*   aBlurRadius     - Determines the intensity of the blur filter effect. The
*     value is expressed in pixel as radius to blur the corresponding source
*     image content. If the value is <= 0, no blur filter is applied.
*   aBlurClampToEdges - Controls the blur effect at the edges of the source
*     surface. Providing 'true' in this parameter prevents the edges from being
*     transparent when the surface is aligned at edges of the destination area.
*   aMode           - This parameter controls the tint filter. Depending on
*     the selected value, the original colors found in the source surface are
*     modified or replaced by the value aColor. The intensity of the effect is
*     controlled by the parameter aIntensity. See EW_TINT_MODE_XXX for more
*     details.
*   aIntensity      - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0.0 .. 1.0. If the value is 1.0, the original
*     colors are completely replaced by values resulting from the filter effect.
*     In turn, the value equal to zero results in the original color values
*     without any tinting effect. Otherwise the colors are interpolated between
*     the original and the color value resulting from the filter effect.
*   aColor          - A color value to be used by the tint filter. See aMode
*     parameter.
*   aBlend          - != 0 if the operation should be performed with alpha 
*      blending.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XFilterDriver)
(
  void*             aDstHandle,
  void*             aSrcHandle,
  void*             aMaskHandle,
  int               aDstX,
  int               aDstY,
  int               aDstWidth,
  int               aDstHeight,
  int               aSrcX,
  int               aSrcY,
  int               aSrcWidth,
  int               aSrcHeight,
  int               aSrcPosX,
  int               aSrcPosY,
  int               aMaskPosX,
  int               aMaskPosY,
  int               aClipX,
  int               aClipY,
  int               aClipWidth,
  int               aClipHeight,
  int               aMaskInverted,
  int               aMaskBeforeBlur,
  int               aBlurRadius,
  int               aBlurClampToEdges,
  int               aMode,
  float             aIntensity,
  unsigned long     aColor,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* PROTOTYPE:
*   XPolygonDriver
*
* DESCRIPTION:
*   The following type declares a prototype for an external graphics subsystem
*   function, which should perform the 'fill polygon' operation by using solid
*   or gradient color values. The polygon is described by values in the aPaths
*   parameter.
*
*  aPaths stores the edges as a serie of X,Y pairs starting always with a value
*  specifying the number of existing edges. With this approach one path can
*  consist of several sub-paths. The end of the list is determined by a sub-
*  path with 0 edges.
*
*  +-------+------+------+------+------+------+------+-------+     +-----+
*  | edges |  X0  |  Y0  |  X1  |  Y1  |  X2  |  Y2  | edges | ... |  0  |
*  +-------+------+------+------+------+------+------+-------+     +-----+
*
* ARGUMENTS:
*   aDstHandle      - Handle to the screen/native destination surface.
*   aPaths          - An array containing the path data. The array starts with
*     the number of edges a path is composed of. Then follow the coordinates of
*     all path corners as X,Y pairs. After the last coordinate pair next path
*     can follow starting again with the number of edges. The end of the path
*     data is signed with 0. The X,Y coordinates are stored as signed integer
*     with 4-bit fixpoint precision.
*   aDstX,
*   aDstY           - Origin of the area to fill (relative to the top-left
*     corner of the destination surface).
*   aWidth,
*   aHeight         - Size of the area to fill.
*   aBlend          - If != 0, the operation should be performed with alpha
*     blending.
*   aAntialiased    - If != 0, the antialiasing should be applied to every
*     pixel.
*   aNonZeroWinding - Controls the fill rule to be used by the algorithm. If
*    this parameter is == 0, the even-odd fill rule is used. If this parameter
*    is != 0, the non-zero winding rule is used.
*   aColors         - Array with 4 RGBA8888 color values. The four color values
*     do correspond to the four corners of the area: top-left, top-right,
*     bottom-right and bottom-left.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XPolygonDriver)
(
  void*             aDstHandle,
  int*              aPaths,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aBlend,
  int               aAntialiased,
  int               aNonZeroWinding,
  unsigned long*    aColors
);


/*******************************************************************************
* PROTOTYPE:
*   XGradientDriver
*
* DESCRIPTION:
*   The following type declares a prototype for an external graphics subsystem
*   function, which should perform the 'fill gradient' operation with a polygon
*   described by values in the aPaths parameter.
*
*  aPaths stores the edges as a serie of X,Y pairs starting always with a value
*  specifying the number of existing edges. With this approach one path can
*  consist of several sub-paths. The end of the list is determined by a sub-
*  path with 0 edges.
*
*  +-------+------+------+------+------+------+------+-------+     +-----+
*  | edges |  X0  |  Y0  |  X1  |  Y1  |  X2  |  Y2  | edges | ... |  0  |
*  +-------+------+------+------+------+------+------+-------+     +-----+
*
* ARGUMENTS:
*   aDstHandle      - Handle to the screen/native destination surface.
*   aPaths          - An array containing the path data. The array starts with
*     the number of edges a path is composed of. Then follow the coordinates of
*     all path corners as X,Y pairs. After the last coordinate pair next path
*     can follow starting again with the number of edges. The end of the path
*     data is signed with 0. The X,Y coordinates are stored as signed integer
*     with 4-bit fixpoint precision.
*   aDstX,
*   aDstY           - Origin of the area to fill (relative to the top-left
*     corner of the destination surface).
*   aWidth,
*   aHeight         - Size of the area to fill.
*   aBlend          - If != 0, the operation should be performed with alpha
*     blending.
*   aAntialiased    - If != 0, the antialiasing should be applied to every
*     pixel.
*   aNonZeroWinding - Controls the fill rule to be used by the algorithm. If
*    this parameter is == 0, the even-odd fill rule is used. If this parameter
*    is != 0, the non-zero winding rule is used.
*   aIsRadial       - != 0 if the path should be filled with a radial gradient.
*     In such case aStartX, aStartY determine the center position of a circle
*     with given aRadius. If this value is == 0, the path is filled with linear
*     gradient interpolated along a straight line between position aStart and
*     aEnd.
*   aStartX,
*   aStartY         - Determines the start position for the gradient.
*   aEndX, aEndY    - Determines the end position of a linear gradient.
*   aRadius         - Determines the radius of a circle in case of a radial
*     gradient.
*   aMatrix          - 2x2 component matrix describing evtl. scaling/rotation of
*     gradient coordinates in case of the radial gradient only.
*   aNoOfStops      - Number of color stops in the array aStops.
*   aStops          - Array with color stops. The entries in the array are
*     sorted according to the stop offset values.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
typedef void (*XGradientDriver)
(
  void*             aDstHandle,
  int*              aPaths,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aBlend,
  int               aAntialiased,
  int               aNonZeroWinding,
  int               aIsRadial,
  int               aStartX,
  int               aStartY,
  int               aEndX,
  int               aEndY,
  int               aRadius,
  float*            aMatrix,
  int               aNoOfStops,
  XGradientStop*    aStops
);


/*******************************************************************************
* FUNCTION:
*   EwCreateScreenSurface
*
* DESCRIPTION:
*   The function EwCreateScreenSurface() has the job to create and return a new
*   screen surface with the given size. If no surface could be created due to
*   memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function creates a pure software surface
*   within the CPU address space only.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateScreenSurface
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwDestroyScreenSurface
*
* DESCRIPTION:
*   The function EwDestroyScreenSurface() has the job to release resources of
*   a previously created screen surface.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function destroys pure software surfaces
*   previously created by the function EwCreateScreenSurface() only.
*
* ARGUMENTS:
*   aSurface - Handle to the surface to destroy.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwDestroyScreenSurface
(
  void*             aSurface
);


/*******************************************************************************
* FUNCTION:
*   EwGetScreenSurfaceMemory
*
* DESCRIPTION:
*   The function EwGetScreenSurfaceMemory() has the job to obtain pointers to
*   the memory planes of the given surface.
*
*   The returned pointers are calculated for the given pixel position aX, aY.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function can handle with the pure software
*   surfaces previously created by the function EwCreateScreenSurface() only.
*
* ARGUMENTS:
*   aSurface     - Handle to the surface to obtain the addresses.
*   aX, aY       - Position within the surface to calculate the pixel address.
*   aReserved    - Should be 0.
*   aWriteAccess - If != 0 the caller has the intention to modify the returned
*     memory content.
*   aMemory      - Structure to receive the desired address information.
*
* RETURN VALUE:
*   If successful, the function fills the given aMemory structure with the
*   address information and returns != 0. Otherwise 0 is returned.
*
*******************************************************************************/
int EwGetScreenSurfaceMemory
(
  void*             aSurface,
  int               aX,
  int               aY,
  int               aReserved,
  int               aWriteAccess,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwGetScreenSurfaceMemSize
*
* DESCRIPTION:
*   The function EwGetScreenSurfaceMemSize() has the job to determine the number
*   of memory bytes occupied by a screen surface with the given size.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface in pixel.
*
* RETURN VALUE:
*   If successful, the function return the surface size in bytes.
*
*******************************************************************************/
int EwGetScreenSurfaceMemSize
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwCreateNativeSurface
*
* DESCRIPTION:
*   The function EwCreateNativeSurface() has the job to create and return a new
*   native surface with the given size. If no surface could be created due to
*   memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function creates a pure software surface
*   within the CPU address space only.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateNativeSurface
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwCreateConstNativeSurface
*
* DESCRIPTION:
*   The function EwCreateConstNativeSurface() has the job to create and return
*   a new native surface with the given size and pixel data content. Important
*   here is the fact that the function doesn't reserve any memory to store the
*   pixel information. Instead it, the surface associates the pixel data passed
*   in the parameter aMemory.
*
*   This function is thus intended to create surfaces from the pixel data stored
*   directly within a ROM area. Accordingly it is essential that the ROM content
*   does exactly correspond to the native surface pixel format. It includes the
*   order of color channels, premultiplication with alpha value, etc..
*
*   If no surface could be created due to memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality.
*
*   Please note, the surfaces created with this function are signed internally
*   as constant. Trying to obtrain write-access to such surface will result in
*   a runtime error.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*   aMemory - Structure to pass the ROM address information.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateConstNativeSurface
(
  int               aWidth,
  int               aHeight,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwDestroyNativeSurface
*
* DESCRIPTION:
*   The function EwDestroyNativeSurface() has the job to release resources of
*   a previously created native surface.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function destroys pure software surfaces
*   previously created by the function EwCreateNativeSurface() only.
*
* ARGUMENTS:
*   aSurface - Handle to the surface to destroy.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwDestroyNativeSurface
(
  void*             aSurface
);


/*******************************************************************************
* FUNCTION:
*   EwGetNativeSurfaceMemory
*
* DESCRIPTION:
*   The function EwGetNativeSurfaceMemory() has the job to obtain pointers to
*   the memory planes and if existing to the color table of the given surface.
*
*   The returned pointers are calculated for the given pixel position aX, aY and
*   in case of an existing color table, to the entry at the position aIndex.
*
*   Please note, if the affected surface has been created with constant pixel
*   information by using EwCreateConstNativeSurface(), you can't obtain the
*   write access to such surface. In such case the function will fail with the
*   return value 0.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function can handle with the pure software
*   surfaces previously created by the function EwCreateNativeSurface() and
*   EwCreateConstNativeSurface() only.
*
* ARGUMENTS:
*   aSurface     - Handle to the surface to obtain the addresses.
*   aX, aY       - Position within the surface to calculate the pixel address.
*   aIndex       - Index within the color table to calculate the CLUT address.
*   aWriteAccess - If != 0 the caller has the intention to modify the returned
*     memory content.
*   aMemory      - Structure to receive the desired address information.
*
* RETURN VALUE:
*   If successful, the function fills the given aMemory structure with the
*   address information and returns != 0. Otherwise 0 is returned.
*
*******************************************************************************/
int EwGetNativeSurfaceMemory
(
  void*             aSurface,
  int               aX,
  int               aY,
  int               aIndex,
  int               aWriteAccess,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwGetNativeSurfaceMemSize
*
* DESCRIPTION:
*   The function EwGetNativeSurfaceMemSize() has the job to determine the number
*   of memory bytes occupied by a native surface with the given size.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface in pixel.
*
* RETURN VALUE:
*   If successful, the function return the surface size in bytes.
*
*******************************************************************************/
int EwGetNativeSurfaceMemSize
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwCreateConstIndex8Surface
*
* DESCRIPTION:
*   The function EwCreateConstIndex8Surface() has the job to create and return
*   a new index8 surface with the given size and pixel data content. Important
*   here is the fact that the function doesn't reserve any memory to store the
*   pixel information. Instead it, the surface associates the pixel data passed
*   in the parameter aMemory.
*
*   This function is thus intended to create surfaces from the pixel data stored
*   directly within a ROM area. Accordingly it is essential that the ROM content
*   does exactly correspond to the index8 surface pixel format. It includes the
*   order of CLUT color channels, premultiplication with alpha value, etc..
*
*   If no surface could be created due to memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality.
*
*   Please note, the surfaces created with this function are signed internally
*   as constant. Trying to obtrain write-access to such surface will result in
*   a runtime error.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*   aMemory - Structure to pass the ROM address information.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateConstIndex8Surface
(
  int               aWidth,
  int               aHeight,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwCreateIndex8Surface
*
* DESCRIPTION:
*   The function EwCreateIndex8Surface() has the job to create and return a new
*   index8 surface with the given size. If no surface could be created due to
*   memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function creates a pure software surface
*   within the CPU address space only.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateIndex8Surface
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwDestroyIndex8Surface
*
* DESCRIPTION:
*   The function EwDestroyIndex8Surface() has the job to release resources of
*   a previously created index8 surface.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function destroys pure software surfaces
*   previously created by the function EwCreateIndex8Surface() only.
*
* ARGUMENTS:
*   aSurface - Handle to the surface to destroy.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwDestroyIndex8Surface
(
  void*             aSurface
);


/*******************************************************************************
* FUNCTION:
*   EwGetIndex8SurfaceMemory
*
* DESCRIPTION:
*   The function EwGetIndex8SurfaceMemory() has the job to obtain the pointer to
*   the index8 memory plane and to the color table of the given surface.
*
*   The returned pointer is calculated for the given pixel position aX, aY and
*   in case of the color table, to the entry at the position aIndex.
*
*   Please note, if the affected surface has been created with constant pixel
*   information by using EwCreateConstIndex8Surface(), you can't obtain the
*   write access to such surface. In such case the function will fail with the
*   return value 0.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function can handle with the pure software
*   surfaces previously created by the function EwCreateIndex8Surface() and
*   EwCreateConstIndex8Surface() only.
*
* ARGUMENTS:
*   aSurface     - Handle to the surface to obtain the addresses.
*   aX, aY       - Position within the surface to calculate the pixel address.
*   aIndex       - Index within the color table to calculate the CLUT address.
*   aWriteAccess - If != 0 the caller has the intention to modify the returned
*     memory content.
*   aMemory      - Structure to receive the desired address information.
*
* RETURN VALUE:
*   If successful, the function fills the given aMemory structure with the
*   address information and returns != 0. Otherwise 0 is returned.
*
*******************************************************************************/
int EwGetIndex8SurfaceMemory
(
  void*             aSurface,
  int               aX,
  int               aY,
  int               aIndex,
  int               aWriteAccess,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwGetIndex8SurfaceMemSize
*
* DESCRIPTION:
*   The function EwGetIndex8SurfaceMemSize() has the job to determine the number
*   of memory bytes occupied by an index8 surface with the given size.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface in pixel.
*
* RETURN VALUE:
*   If successful, the function return the surface size in bytes.
*
*******************************************************************************/
int EwGetIndex8SurfaceMemSize
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwCreateAlpha8Surface
*
* DESCRIPTION:
*   The function EwCreateAlpha8Surface() has the job to create and return a new
*   alpha8 surface with the given size. If no surface could be created due to
*   memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function creates a pure software surface
*   within the CPU address space only.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateAlpha8Surface
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwCreateConstAlpha8Surface
*
* DESCRIPTION:
*   The function EwCreateConstAlpha8Surface() has the job to create and return
*   a new alpha8 surface with the given size and pixel data content. Important
*   here is the fact that the function doesn't reserve any memory to store the
*   pixel information. Instead it, the surface associates the pixel data passed
*   in the parameter aMemory.
*
*   This function is thus intended to create surfaces from the pixel data stored
*   directly within a ROM area. Accordingly it is essential that the ROM content
*   does exactly correspond to the alpha8 surface pixel format.
*
*   If no surface could be created due to memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality.
*
*   Please note, the surfaces created with this function are signed internally
*   as constant. Trying to obtrain write-access to such surface will result in
*   a runtime error.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*   aMemory - Structure to pass the ROM address information.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateConstAlpha8Surface
(
  int               aWidth,
  int               aHeight,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwDestroyAlpha8Surface
*
* DESCRIPTION:
*   The function EwDestroyAlphaSurface() has the job to release resources of
*   a previously created alpha8 surface.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function destroys pure software surfaces
*   previously created by the function EwCreateAlpha8Surface() only.
*
* ARGUMENTS:
*   aSurface - Handle to the surface to destroy.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwDestroyAlpha8Surface
(
  void*             aSurface
);


/*******************************************************************************
* FUNCTION:
*   EwGetAlpha8SurfaceMemory
*
* DESCRIPTION:
*   The function EwGetAlpha8SurfaceMemory() has the job to obtain pointers to
*   the memory planes of the given surface.
*
*   The returned pointer is calculated for the given pixel position aX, aY.
*
*   Please note, if the affected surface has been created with constant pixel
*   information by using EwCreateConstAlpha8Surface(), you can't obtain the
*   write access to such surface. In such case the function will fail with the
*   return value 0.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function can handle with the pure software
*   surfaces previously created by the function EwCreateIAlpha8Surface() and
*   EwCreateConstAlpha8Surface() only.
*
* ARGUMENTS:
*   aSurface     - Handle to the surface to obtain the addresses.
*   aX, aY       - Position within the surface to calculate the pixel address.
*   aReserved    - Should be 0.
*   aWriteAccess - If != 0 the caller has the intention to modify the returned
*     memory content.
*   aMemory      - Structure to receive the desired address information.
*
* RETURN VALUE:
*   If successful, the function fills the given aMemory structure with the
*   address information and returns != 0. Otherwise 0 is returned.
*
*******************************************************************************/
int EwGetAlpha8SurfaceMemory
(
  void*             aSurface,
  int               aX,
  int               aY,
  int               aReserved,
  int               aWriteAccess,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwGetAlpha8SurfaceMemSize
*
* DESCRIPTION:
*   The function EwGetAlpha8SurfaceMemSize() has the job to determine the number
*   of memory bytes occupied by an alpha8 surface with the given size.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface in pixel.
*
* RETURN VALUE:
*   If successful, the function return the surface size in bytes.
*
*******************************************************************************/
int EwGetAlpha8SurfaceMemSize
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwCreateConstRGB565Surface
*
* DESCRIPTION:
*   The function EwCreateConstRGB565Surface() has the job to create and return
*   a new rgb565 surface with the given size and pixel data content. Important
*   here is the fact that the function doesn't reserve any memory to store the
*   pixel information. Instead it, the surface associates the pixel data passed
*   in the parameter aMemory.
*
*   This function is thus intended to create surfaces from the pixel data stored
*   directly within a ROM area. Accordingly it is essential that the ROM content
*   does exactly correspond to the rgb565 surface pixel format. It includes the
*   order of CLUT color channels, premultiplication with alpha value, etc..
*
*   If no surface could be created due to memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality.
*
*   Please note, the surfaces created with this function are signed internally
*   as constant. Trying to obtrain write-access to such surface will result in
*   a runtime error.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*   aMemory - Structure to pass the ROM address information.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateConstRGB565Surface
(
  int               aWidth,
  int               aHeight,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwCreateRGB565Surface
*
* DESCRIPTION:
*   The function EwCreateRGB565Surface() has the job to create and return a new
*   rgb565 surface with the given size. If no surface could be created due to
*   memory deficit, 0 is returned.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function creates a pure software surface
*   within the CPU address space only.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface to create.
*
* RETURN VALUE:
*   If successful, returns a handle to the new surface. Otherwise 0 is returned.
*
*******************************************************************************/
void* EwCreateRGB565Surface
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwDestroyRGB565Surface
*
* DESCRIPTION:
*   The function EwDestroyRGB565Surface() has the job to release resources of
*   a previously created rgb565 surface.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function destroys pure software surfaces
*   previously created by the function EwCreateRGB565Surface() only.
*
* ARGUMENTS:
*   aSurface - Handle to the surface to destroy.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwDestroyRGB565Surface
(
  void*             aSurface
);


/*******************************************************************************
* FUNCTION:
*   EwGetRGB565SurfaceMemory
*
* DESCRIPTION:
*   The function EwGetRGB565SurfaceMemory() has the job to obtain the pointer to
*   the rgb565 memory plane and to the color table of the given surface.
*
*   The returned pointer is calculated for the given pixel position aX, aY and
*   in case of the color table, to the entry at the position aIndex.
*
*   Please note, if the affected surface has been created with constant pixel
*   information by using EwCreateConstRGB565Surface(), you can't obtain the
*   write access to such surface. In such case the function will fail with the
*   return value 0.
*
*   The function is used, if the underlying graphics subsystem doesn't provide
*   any adequate functionality. The function can handle with the pure software
*   surfaces previously created by the function EwCreateRGB565Surface() and
*   EwCreateConstRGB565Surface() only.
*
* ARGUMENTS:
*   aSurface     - Handle to the surface to obtain the addresses.
*   aX, aY       - Position within the surface to calculate the pixel address.
*   aIndex       - Index within the color table to calculate the CLUT address.
*   aWriteAccess - If != 0 the caller has the intention to modify the returned
*     memory content.
*   aMemory      - Structure to receive the desired address information.
*
* RETURN VALUE:
*   If successful, the function fills the given aMemory structure with the
*   address information and returns != 0. Otherwise 0 is returned.
*
*******************************************************************************/
int EwGetRGB565SurfaceMemory
(
  void*             aSurface,
  int               aX,
  int               aY,
  int               aIndex,
  int               aWriteAccess,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwGetRGB565SurfaceMemSize
*
* DESCRIPTION:
*   The function EwGetRGB565SurfaceMemSize() has the job to determine the number
*   of memory bytes occupied by an rgb565 surface with the given size.
*
* ARGUMENTS:
*   aWidth,
*   aHeight - Size of the surface in pixel.
*
* RETURN VALUE:
*   If successful, the function return the surface size in bytes.
*
*******************************************************************************/
int EwGetRGB565SurfaceMemSize
(
  int               aWidth,
  int               aHeight
);


/*******************************************************************************
* FUNCTION:
*   EwInitSingleColorGradient
*
* DESCRIPTION:
*   The function EwInitSingleColorGradient() initializes the gradient structure
*   with the given single color value.
*
* ARGUMENTS:
*   aColor    - Color value in the universal RGBA8888 color format.
*   aGradient - Destination gradient structure to initialize.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwInitSingleColorGradient
(
  unsigned int      aColor,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwInitColorGradient
*
* DESCRIPTION:
*   The function EwInitColorGradient() has the job to initialize a rectangular,
*   linear gradient from 4 color values. The color values do correspond to the
*   four corners of the rectangular gradient: top-left, top-right, bottom-right
*   and bottom-left.
*
* ARGUMENTS:
*   aWidth,
*   aHeight   - Size of the gradient area in pixel.
*   aColors   - Array of 4 color values in the universal RGBA8888 color format.
*   aGradient - Destination gradient structure to initialize.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwInitColorGradient
(
  int               aWidth,
  int               aHeight,
  unsigned int*     aColors,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwInitOpacityGradient
*
* DESCRIPTION:
*   The function EwInitOpacityGradient() has the job to initialize a new linear,
*   rectangular gradient from 4 color values. The color values do correspond to
*   the four corners of the gradient area: top-left, top-right, bottom-right and
*   bottom-left.
*
* ARGUMENTS:
*   aWidth,
*   aHeight   - Size of the gradient area in pixel.
*   aColors   - Array of 4 color values in the universal RGBA8888 color format.
*   aGradient - Destination gradient structure to initialize.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwInitOpacityGradient
(
  int               aWidth,
  int               aHeight,
  unsigned int*     aColors,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwGetColorFromGradient
*
* DESCRIPTION:
*   The function EwGetColorFromGradient() has the job to interpolate the color
*   value for the given position within a color gradient.
*
* ARGUMENTS:
*   aGradient  - Color gradient to get the color.
*   aX, aY     - Position relative to the upper-left corner of the gradient to
*     get the color value.
*
* RETURN VALUE:
*   Interpolated color value in the universal RGBA8888 color format.
*
*******************************************************************************/
unsigned int EwGetColorFromGradient
(
  XGradient*        aGradient,
  int               aX,
  int               aY
);


/*******************************************************************************
* FUNCTION:
*   EwGetOpacityFromGradient
*
* DESCRIPTION:
*   The function EwGetOpacityFromGradient() has the job to interpolate the
*   opacity value for the given position within a color gradient.
*
* ARGUMENTS:
*   aGradient  - Color gradient to get the value.
*   aX, aY     - Position relative to the upper-left corner of the gradient to
*     get the opacity value.
*
* RETURN VALUE:
*   Interpolated color value in the universal RGBA8888 color format. Only the
*   alpha channel is valid.
*
*******************************************************************************/
unsigned int EwGetOpacityFromGradient
(
  XGradient*        aGradient,
  int               aX,
  int               aY
);


/*******************************************************************************
* FUNCTION:
*   EwRasterAlpha8Polygon
*
* DESCRIPTION:
*  The function EwRasterAlpha8Polygon() implements an algorithm to estimate the
*  content of an ALPHA8 bitmap from polygon data determined by edges in the
*  array aPaths. aPaths stores the edges as a serie of X,Y pairs starting always
*  with a value specifying the number of existing edges. With this approach one
*  path can consist of several sub-paths. The end of the list is determined by a
*  sub-path with 0 edges.
*
*  +-------+------+------+------+------+------+------+-------+     +-----+
*  | edges |  X0  |  Y0  |  X1  |  Y1  |  X2  |  Y2  | edges | ... |  0  |
*  +-------+------+------+------+------+------+------+-------+     +-----+
*
*  The function evaluates the path data for intersections between the edges and
*  the pixel within the destination area aDstX, aDstY, aWidth and aHeight. Then
*  the affected pixel are set or cleared according to whether they lie inside
*  or outside the polygon.
*
* ARGUMENTS:
*   aDst            - Pointer to the first pixel of the destination ALPHA8
*     surface. The caller is responsable for clearing the surface before.
*   aPaths          - An array containing the path data. The array starts with
*     the number of edges a path is composed of. Then follow the coordinates of
*     all path corners as X,Y pairs. After the last coordinate pair next path
*     can follow starting again with the number of edges. The end of the path
*     data is signed with 0. The X,Y coordinates are stored as signed integer
*     with 4-bit fixpoint precision. The coordinates are valid relative to the
*     top-left corner of the destination area aDstX, aDstY.
*   aDstX,
*   aDstY           - Origin of the area to fill (relative to the top-left
*     corner of the destination surface).
*   aWidth,
*   aHeight         - Size of the area to fill.
*   aX, aY          - Path coordinate at the top-left corner of aDstX, aDstY
*     area. This value is expressed in in 4-bit fixpoint precision.
*   aAntialiased    - If != 0, the function applies antialiasing to the pixel.
*     The antialiasing is based on supersampling with 4 samples in X and Y
*     direction.
*   aNonZeroWinding - Controls the fill rule to be used by the algorithm. If
*    this parameter is == 0, the even-odd fill rule is used. If this parameter
*    is != 0, the non-zero winding rule is used.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwRasterAlpha8Polygon
(
  XSurfaceMemory*   aDst,
  int*              aPaths,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aX,
  int               aY,
  int               aAntialiased,
  int               aNonZeroWinding
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateLine
*
* DESCRIPTION:
*   The function EwEmulateLine() drives a draw line operation by using solid or
*   gradient color values.
*
*   The function provides the top-level interface to the software pixel driver.
*   The real drawing operation will be driven by the passed worker function.
*
* ARGUMENTS:
*   aDst        - Pointer to the first pixel of the destination surface.
*   aDstX1,
*   aDstY1      - Start position to draw the line (relative to the top-left
*     corner of the destination surface).
*   aDstX2,
*   aDstY2      - End position to draw the line (relative to the top-left
*      corner of the destination surface). The pixel at the end position does
*      not belong to the line - it is invisible. In this manner polylines can
*      be drawn.
*   aClipX1,
*   aClipY1,
*   aClipX2,
*   aClipY2     - Optional clipping area relative to the top-left corner of the
*     destination surface. No pixel outside this area may be drawn.
*   aGradient   - Color information to modulate the drawn pixel.
*   aWorker     - Low-level worker function to perform the operation.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateLine
(
  XSurfaceMemory*   aDst,
  int               aDstX1,
  int               aDstY1,
  int               aDstX2,
  int               aDstY2,
  int               aClipX1,
  int               aClipY1,
  int               aClipX2,
  int               aClipY2,
  XGradient*        aGradient,
  XLineWorker       aWorker
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateFill
*
* DESCRIPTION:
*   The function EwEmulateFill() drives a fill operation for a rectangular area
*   by using solid or gradient color values.
*
*   The function provides the top-level interface to the software pixel driver.
*   The real drawing operation will be driven by the passed worker function.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination surface.
*   aDstX,
*   aDstY     - Origin of the area to fill (relative to the top-left corner of
*     the destination surface).
*   aWidth,
*   aHeight   - Size of the area to fill.
*   aGradient - Color information to modulate the filled pixel.
*   aGrdX,
*   aGrdY     - Origin of the affected area in the gradient coordinate space.
*   aWorker   - Low-level worker function to perform the operation row-wise.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateFill
(
  XSurfaceMemory*   aDst,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  XGradient*        aGradient,
  int               aGrdX,
  int               aGrdY,
  XFillWorker       aWorker
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateCopy
*
* DESCRIPTION:
*   The function EwEmulateCopy() drives a copy operation for a rectangular area
*   from a native, index8, alpha8 or rgb565 surface to a screen or native surface.
*   The function modulates the copied pixel by solid or gradient opacity values.
*
*   The function provides the top-level interface to the software pixel driver.
*   The real drawing operation will be driven by the passed worker function.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination surface.
*   aSrc      - Pointer to the first pixel of the source surface.
*   aDstX,
*   aDstY     - Origin of the area to fill (relative to the top-left corner of
*     the destination surface).
*   aWidth,
*   aHeight   - Size of the area to fill.
*   aSrcX,
*   aSrcY     - Origin of the source area to copy (relative to the top-left
*     corner of the source surface).
*   aGradient - Color information to modulate the copied pixel.
*   aGrdX,
*   aGrdY     - Origin of the affected area in the gradient coordinate space.
*   aWorker   - Low-level worker function to perform the operation row-wise.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateCopy
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aSrcX,
  int               aSrcY,
  XGradient*        aGradient,
  int               aGrdX,
  int               aGrdY,
  XCopyWorker       aWorker
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateWarp
*
* DESCRIPTION:
*   The function EwEmulateWarp() drives a warp operation for a rectangular area
*   of a native, index8, alpha8 or rgb565 surface to a polygon within a screen 
*   or native surface. The function modulates the copied pixel by solid or 
*   gradient opacity values.
*
*   The function provides the top-level interface to the software pixel driver.
*   The real drawing operation will be driven by the passed worker function.
*
* ARGUMENTS:
*   aDst        - Pointer to the first pixel of the destination surface.
*   aSrc        - Pointer to the first pixel of the source surface.
*   DstX1,
*   DstY1,
*   DstW1,
*   ...
*   DstX4,
*   DstY4,
*   DstW4       - Coordinates of the polygon to fill with the source bitmap
*     (Relative to the top-left corner of the destination bitmap). The 'X', 'Y'
*     coefficients are specified in 29.3 fixed point format due to the
*     sub-pixel precision. The 'W' coefficients in 16.16.
*   aSrcX,
*   aSrcY       - Origin of the source area to copy (relative to the top-left
*     corner of the source surface).
*   aSrcWidth,
*   aSrcHeight  - Size of the source area to copy and warp.
*   aClipX1,
*   aClipY1,
*   aClipX2,
*   aClipY2     - Optional clipping area relative to the top-left corner of the
*     destination surface. No pixel outside this area may be drawn.
*   aGradient   - Color information to modulate the copied pixel.
*   aWorker     - Low-level worker function to perform the operation row-wise.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateWarp
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aDstX1,
  int               aDstY1,
  int               aDstW1,
  int               aDstX2,
  int               aDstY2,
  int               aDstW2,
  int               aDstX3,
  int               aDstY3,
  int               aDstW3,
  int               aDstX4,
  int               aDstY4,
  int               aDstW4,
  int               aSrcX,
  int               aSrcY,
  int               aSrcWidth,
  int               aSrcHeight,
  int               aClipX1,
  int               aClipY1,
  int               aClipX2,
  int               aClipY2,
  XGradient*        aGradient,
  XWarpWorker       aWorker
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateMaskTintFilter
*
* DESCRIPTION:
*   The function EwEmulateMaskTintFilter() drives a copy operation with mask or
*   tint filter between a native, index8, alpha8 or rgb565 surface as source and
*   a screen or native surface as destination.
*
*   The function provides the top-level interface to the software pixel driver.
*   The real drawing operation will be driven by the passed worker functions.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination surface.
*   aSrc          - Pointer to the first pixel of the source surface.
*   aMask         - Pointer to the first pixel of the mask surface or NULL if
*     no mask is used.
*   aDstX,
*   aDstY         - Origin of the area to fill (relative to the top-left corner
*     of the destination surface).
*   aWidth,
*   aHeight       - Size of the area to fill.
*   aSrcX,
*   aSrcY         - Position within the source surface to map to the origin of
*     the destination area (relative to the top-left corner of the source
*     surface).
*   aMaskX,
*   aMaskY        - Position within the mask surface to map to the origin of
*     the destination area (relative to the top-left corner of the mask
*     surface).
*   aMaskX1..
*   aMaskY2       - Boundary within the mask surface indicating the area to use
*     as mask. The area outside the mask boundary is considered as transparent.
*   aMaskInverted - If aMaskInverted != 0, the data from the mask surface is
*     inverted and the area outside the mask boundary is considered as opaque.
*     Please note: if mask is used and the mask is not inverted, the resulting
*     drawing area may not be larger than the mask area!
*   aColor        - A color value to be used by the tint filter stored in a
*     XGradient data structure.
*   aIntensity    - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0 .. 255. If the value is 255, the original
*     colors are completely replaced by values resulting from the filter. If
*     the value is equal zero, the original colors are used.
*   aOpacity      - Modulates the opacity of the pixel data resulting from the
*     filter operation. This parameter can take values in range of 0 .. 255.
*   aBlend        - If != 0, the drawn pixel will be alpha-blended with the
*     pixel in the background.
*   aWorker1      - Low-level worker functions to perform the operation row-
*     wise. 
*   aWorker2      - Second worker optimized to perform the operation without
*     the mask. This worker will be used for areas lying outside the mask when
*     the mask is inverted.
*     
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateMaskTintFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aSrcX,
  int               aSrcY,
  int               aMaskX,
  int               aMaskY,
  int               aMaskX1,
  int               aMaskY1,
  int               aMaskX2,
  int               aMaskY2,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend,
  XMaskTintWorker   aWorker1,
  XMaskTintWorker   aWorker2
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateBlurFilter
*
* DESCRIPTION:
*   The function EwEmulateBlurFilter() drives a copy operation with blur filter
*   between a native, alpha8, index8, rgb565 surface as source and a screen,
*   native or alpha8 surface as destination. Alpha8 as destination is possibly
*   only when the source is stored in the alpha8 format too.
*
*   The function provides the top-level interface to the software pixel driver.
*   The real drawing operation will be driven by the passed worker function.
*
*   Please note, the function performs a single blur pass by reading the pixel
*   row-wise from the source surface, bluring them and storing the results as
*   columns in the destination surface. The rows and columns are thus exchanged
*   after the operation is done. Repeating the operation again restores the
*   original order.
*
* ARGUMENTS:
*   aDst     - Pointer to the first pixel of the destination surface.
*   aSrc     - Pointer to the first pixel of the source surface.
*   aDstX,
*   aDstY    - Origin of the area to fill (relative to the top-left corner of
*     the destination surface).
*   aWidth,
*   aHeight  - Size of the area to fill.
*   aSrcX,
*   aSrcY    - Position within the source surface to map to the origin of the
*     destination area (relative to the top-left corner of the source surface).
*   aSrcX1..
*   aSrcX2   - Horizontal boundary within the source surface indicating the
*     area to use as source. Source pixel outside this boundary are considered
*     as transparent.
*   aClamp1..
*   aClamp2  - Vertical boundary relative to the first row in the destination
*     area from where the algorithm starts to clamp the blur effect.
*   aRadius  - Blur radius in pixel.
*   aOpacity - Modulates the opacity of the pixel data resulting from the blur
*     operation. This parameter can take values in range of 0 .. 255.
*   aBlend   - If != 0, the drawn pixel will be alpha-blended with the pixel in
*     the background.
*   aWorker  - Low-level worker function to perform the operation pixel-wise.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateBlurFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aSrcX,
  int               aSrcY,
  int               aSrcX1,
  int               aSrcX2,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  int               aOpacity,
  int               aBlend,
  XBlurWorker       aWorker
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateMaskTintBlurFilter
*
* DESCRIPTION:
*   The function EwEmulateMaskTintBlurFilter() drives a copy operation with mask
*   or tint filter followed by a blur filter between a native, index8, alpha8 or
*   rgb565 surface as source and a screen or native surface as destination.
*
*   The function provides the top-level interface to the software pixel driver.
*   The real drawing operation will be driven by the passed worker functions.
*
*   Please note, the function performs a single blur pass by reading the pixel
*   row-wise from the source surface, bluring them and storing the results as
*   columns in the destination surface. The rows and columns are thus exchanged
*   after the operation is done. Repeating the operation again restores the
*   original order.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination surface.
*   aSrc          - Pointer to the first pixel of the source surface.
*   aMask         - Pointer to the first pixel of the mask surface or NULL if
*     no mask is used.
*   aAux          - Pointer to the first pixel of an auxiliary surface used to
*     mask tint pixel rows before the rows are blured. Important, the surface
*     has to be as wide as the source surface and contain one row only.
*   aDstX,
*   aDstY         - Origin of the area to fill (relative to the top-left corner
*     of the destination surface).
*   aWidth,
*   aHeight       - Size of the area to fill.
*   aSrcX,
*   aSrcY         - Position within the source surface to map to the origin of
*     the destination area (relative to the top-left corner of the source
*     surface).
*   aSrcX1..
*   aSrcX2        - Horizontal boundary within the source surface indicating
*     the area to use as source. Source pixel outside this boundary are 
*     considered as transparent. If the mask filter is used, the mask also may
*     limit this area so no pixel on the left or on the right of the mask are
*     evaluated
*   aMaskX,
*   aMaskY        - Position within the mask surface to map to the origin of
*     the destination area (relative to the top-left corner of the mask
*     surface).
*   aMaskX1..
*   aMaskY2       - Boundary within the mask surface indicating the area to use
*     as mask. The area outside the mask boundary is considered as transparent.
*   aMaskInverted - If aMaskInverted != 0, the data from the mask surface is
*     inverted and the area outside the mask boundary is considered as opaque.
*     Please note: if mask is used and the mask is not inverted, the resulting
*     drawing area may not be larger than the mask area!
*   aColor        - A color value to be used by the tint filter stored in a
*     XGradient data structure.
*   aIntensity    - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0 .. 255. If the value is 255, the original
*     colors are completely replaced by values resulting from the filter. If
*     the value is equal zero, the original colors are used.
*   aClamp1..
*   aClamp2       - Vertical boundary relative to the first row in the 
*     destination area from where the algorithm starts to clamp the blur effect.
*   aRadius       - Blur radius in pixel.
*   aWorker0      - Worker function to perform the copy operation pixel-wise
*     between the source surface and a NATIVE surface.
*   aWorker1      - Low-level worker functions to perform the mask/tint filter
*     operation row-wise between the source and a NATIVE surface.
*   aWorker2      - Worker optimized to perform only the tint operation between
*     a source and a NATIVE surface. This worker will be used for areas lying
*     outside the mask when the mask is inverted.
*   aWorker3      - Worker function to perform the blur filter operation pixel-
*     wise between a NATIVE and the destination surfaces.
*   aWorker4      - Worker function to perform the blur filter operation pixel-
*     wise between the source surface and a destination surface.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateMaskTintBlurFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  XSurfaceMemory*   aAux,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aSrcX,
  int               aSrcY,
  int               aSrcX1,
  int               aSrcX2,
  int               aMaskX,
  int               aMaskY,
  int               aMaskX1,
  int               aMaskY1,
  int               aMaskX2,
  int               aMaskY2,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  XCopyWorker       aWorker0,
  XMaskTintWorker   aWorker1,
  XMaskTintWorker   aWorker2,
  XBlurWorker       aWorker3,
  XBlurWorker       aWorker4
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateBlurMaskTintFilter
*
* DESCRIPTION:
*   The function EwEmulateBlurMaskTintFilter() drives a copy operation with blur
*   filter followed by a mask or tint filter between a native or alpha8 surface
*   as source and a screen or native surface as destination.
*
*   The function provides the top-level interface to the software pixel driver.
*   The real drawing operation will be driven by the passed worker functions.
*
*   Please note, the function performs a single blur pass by reading the pixel
*   row-wise from the source surface, bluring them and storing the results as
*   columns in the destination surface. The rows and columns are thus exchanged
*   after the operation is done. Repeating the operation again restores the
*   original order.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination surface.
*   aSrc          - Pointer to the first pixel of the source surface.
*   aMask         - Pointer to the first pixel of the mask surface or NULL if
*     no mask is used.
*   aAux          - Pointer to the first pixel of an auxiliary surface used to
*     store the results of a blurred column before it is processed by the mask/
*     tint filter. The layout of the auxiliary surface has to be as wide as the
*     height of the destination surface and contain one row only. The format of
*     the surface has to be native or alpha8 if the source is alpha8 too.
*   aDstX,
*   aDstY         - Origin of the area to fill (relative to the top-left corner
*     of the destination surface).
*   aWidth,
*   aHeight       - Size of the area to fill.
*   aSrcX,
*   aSrcY         - Position within the source surface to map to the origin of
*     the destination area (relative to the top-left corner of the source
*     surface).
*   aSrcX1..
*   aSrcX2        - Horizontal boundary within the source surface indicating
*     the area to use as source. Source pixel outside this boundary are 
*     considered as transparent.
*   aMaskX,
*   aMaskY        - Position within the mask surface to map to the origin of
*     the destination area (relative to the top-left corner of the mask
*     surface).
*   aMaskX1..
*   aMaskY2       - Boundary within the mask surface indicating the area to use
*     as mask. The area outside the mask boundary is considered as transparent.
*   aMaskInverted - If aMaskInverted != 0, the data from the mask surface is
*     inverted and the area outside the mask boundary is considered as opaque.
*     Please note: if mask is used and the mask is not inverted, the resulting
*     drawing area may not be larger than the mask area!
*   aColor        - A color value to be used by the tint filter stored in a
*     XGradient data structure.
*   aIntensity    - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0 .. 255. If the value is 255, the original
*     colors are completely replaced by values resulting from the filter. If
*     the value is equal zero, the original colors are used.
*   aClamp1..
*   aClamp2       - Vertical boundary relative to the first row in the 
*     destination area from where the algorithm starts to clamp the blur effect.
*   aRadius       - Blur radius in pixel.
*   aOpacity      - Modulates the opacity of the pixel data resulting from the
*     blur/mask/tint operation. This parameter can take values in range of 0 ..
*     255.
*   aBlend        - If != 0, the drawn pixel will be alpha-blended with the
*     pixel in the background.
*   aWorker1      - Worker used to blur between the source and the auxiliary
*     surface. The source and the auxiliary surfaces are both either NATIVE or
*     ALPHA8.
*   aWorker2      - Worker used to blur between the source and the destination
*     surface. The source surface is NATIVE. Destination surface is either
*     NATIVE or SCREEN.
*   aWorker3      - Low-level worker functions to perform the mask/tint filter
*     operation column-wise between the auxiliary and the destination surfaces.
*   aWorker4      - Worker optimized to perform only the tint operation between
*     the auxiliary and the destination surfaces. This worker will be used for
*     areas lying outside the mask when the mask is inverted.
*   aWorker5      - Worker optimized to perform only a copy operation between
*     the auxiliary and the destination surfaces. This worker will be used for
*     areas lying outside the mask when the mask is inverted and no tint filter
*     is used.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateBlurMaskTintFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  XSurfaceMemory*   aAux,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aSrcX,
  int               aSrcY,
  int               aSrcX1,
  int               aSrcX2,
  int               aMaskX,
  int               aMaskY,
  int               aMaskX1,
  int               aMaskY1,
  int               aMaskX2,
  int               aMaskY2,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  int               aOpacity,
  int               aBlend,
  XBlurWorker       aWorker1,
  XBlurWorker       aWorker2,
  XMaskTintWorker   aWorker3,
  XMaskTintWorker   aWorker4,
  XMaskTintWorker   aWorker5
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateFillPolygon
*
* DESCRIPTION:
*  The function EwEmulateFillPolygon() drives a fill polygon operation with
*  polygon data provided in the array aPaths. aPaths stores polygon edges as a
*  serie of X,Y coordinate pairs starting always with a value specifying the
*  number of edges the path is composed of. With this approach one path can
*  consist of several sub-paths. The end of the list is determined by a sub-
*  path with 0 edges:
*
*  +-------+------+------+------+------+------+------+-------+     +-----+
*  | edges |  X0  |  Y0  |  X1  |  Y1  |  X2  |  Y2  | edges | ... |  0  |
*  +-------+------+------+------+------+------+------+-------+     +-----+
*
*  The function evaluates the path data for intersections between the edges and
*  the pixel within the destination area aDstX, aDstY, aWidth and aHeight. The
*  function modulates the pixel by solid or gradient color values.
*
*  The function provides the top-level interface to the software pixel driver.
*  The real drawing operation will be driven by the passed worker function.
*
* ARGUMENTS:
*   aDst            - Pointer to the first pixel of the destination surface.
*   aPaths          - An array containing the path data. The array starts with
*     the number of edges a path is composed of. Then follow the coordinates of
*     all path corners as X,Y pairs. After the last coordinate pair next path
*     can follow starting again with the number of edges. The end of the path
*     data is signed with 0. The X,Y coordinates are stored as signed integer
*     with 4-bit fixpoint precision. The coordinates are valid relative to the
*     top-left corner of the destination bitmap.
*   aDstX,
*   aDstY           - Origin of the area to fill (relative to the top-left
*     corner of the destination surface).
*   aWidth,
*   aHeight         - Size of the area to fill.
*   aAntialiased    - If != 0, the function applies antialiasing to the pixel.
*     The antialiasing is based on supersampling with 4 samples in X and Y
*     direction.
*   aNonZeroWinding - Controls the fill rule to be used by the algorithm. If
*    this parameter is == 0, the even-odd fill rule is used. If this parameter
*    is != 0, the non-zero winding rule is used.
*   aGradient       - Color information to modulate the copied pixel.
*   aGrdX,
*   aGrdY           - Origin of the affected area in the gradient coordinate
*     space.
*   aWorker         - Low-level worker function to perform the final copy
*     operation of the rasterized pixel row-wise.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateFillPolygon
(
  XSurfaceMemory*   aDst,
  int*              aPaths,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aAntialiased,
  int               aNonZeroWinding,
  XGradient*        aGradient,
  int               aGrdX,
  int               aGrdY,
  XCopyWorker       aWorker
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateFillLinearGradient
*
* DESCRIPTION:
*  The function EwEmulateFillLinearGradient() drives a fill polygon operation
*  with polygon data provided in the array aPaths and a linear color gradient.
*  aPaths stores polygon edges as a serie of X,Y coordinate pairs starting
*  always with a value specifying the number of edges the path is composed of.
*  With this approach one path can consist of several sub-paths. The end of the
*  list is determined by a sub-path with 0 edges:
*
*  +-------+------+------+------+------+------+------+-------+     +-----+
*  | edges |  X0  |  Y0  |  X1  |  Y1  |  X2  |  Y2  | edges | ... |  0  |
*  +-------+------+------+------+------+------+------+-------+     +-----+
*
*  The function evaluates the path data for intersections between the edges and
*  the pixel within the destination area aDstX, aDstY, aWidth and aHeight. The
*  function modulates the pixel by color values resulting from the linear
*  gradient.
*
*  The function provides the top-level interface to the software pixel driver.
*  The real drawing operation will be driven by the passed worker functions.
*
* ARGUMENTS:
*   aDst             - Pointer to the first pixel of the destination surface.
*   aPaths           - An array containing the path data. The array starts with
*     the number of edges a path is composed of. Then follow the coordinates of
*     all path corners as X,Y pairs. After the last coordinate pair next path
*     can follow starting again with the number of edges. The end of the path
*     data is signed with 0. The X,Y coordinates are stored as signed integer
*     with 4-bit fixpoint precision. The coordinates are valid relative to the
*     top-left corner of the destination bitmap.
*   aDstX,
*   aDstY            - Origin of the area to fill (relative to the top-left
*     corner of the destination surface).
*   aWidth,
*   aHeight          - Size of the area to fill.
*   aAntialiased     - If != 0, the function applies antialiasing to the pixel.
*     The antialiasing is based on supersampling with 4 samples in X and Y
*     direction.
*   aNonZeroWinding  - Controls the fill rule to be used by the algorithm. If
*    this parameter is == 0, the even-odd fill rule is used. If this parameter
*    is != 0, the non-zero winding rule is used.
*   aStartX,
*   aStartY          - Determines the start position for the gradient.
*   aEndX, aEndY     - Determines the end position of the gradient.
*   aNoOfStops       - Number of color stops in the array aStops.
*   aStops           - Array with color stops. The entries in the array are
*     sorted according to the stop offset values.
*   aWorker          - Low-level worker function to perform the fill gradient
*     operation.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateFillLinearGradient
(
  XSurfaceMemory*   aDst,
  int*              aPaths,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aAntialiased,
  int               aNonZeroWinding,
  int               aStartX,
  int               aStartY,
  int               aEndX,
  int               aEndY,
  int               aNoOfStops,
  XGradientStop*    aStops,
  XFillLinearGradientWorker aWorker
);


/*******************************************************************************
* FUNCTION:
*   EwEmulateFillRadialGradient
*
* DESCRIPTION:
*  The function EwEmulateFillGradient() drives a fill polygon operation with
*  polygon data provided in the array aPaths and a radial color gradient.
*  aPaths stores polygon edges as a serie of X,Y coordinate pairs starting
*  always with a value specifying the number of edges the path is composed of.
*  With this approach one path can consist of several sub-paths. The end of the
*  list is determined by a sub-path with 0 edges:
*
*  +-------+------+------+------+------+------+------+-------+     +-----+
*  | edges |  X0  |  Y0  |  X1  |  Y1  |  X2  |  Y2  | edges | ... |  0  |
*  +-------+------+------+------+------+------+------+-------+     +-----+
*
*  The function evaluates the path data for intersections between the edges and
*  the pixel within the destination area aDstX, aDstY, aWidth and aHeight. The
*  function modulates the pixel by color values resulting from the radial
*  gradient.
*
*  The function provides the top-level interface to the software pixel driver.
*  The real drawing operation will be driven by the passed worker functions.
*
* ARGUMENTS:
*   aDst             - Pointer to the first pixel of the destination surface.
*   aPaths           - An array containing the path data. The array starts with
*     the number of edges a path is composed of. Then follow the coordinates of
*     all path corners as X,Y pairs. After the last coordinate pair next path
*     can follow starting again with the number of edges. The end of the path
*     data is signed with 0. The X,Y coordinates are stored as signed integer
*     with 4-bit fixpoint precision. The coordinates are valid relative to the
*     top-left corner of the destination bitmap.
*   aDstX,
*   aDstY            - Origin of the area to fill (relative to the top-left
*     corner of the destination surface).
*   aWidth,
*   aHeight          - Size of the area to fill.
*   aAntialiased     - If != 0, the function applies antialiasing to the pixel.
*     The antialiasing is based on supersampling with 4 samples in X and Y
*     direction.
*   aNonZeroWinding  - Controls the fill rule to be used by the algorithm. If
*    this parameter is == 0, the even-odd fill rule is used. If this parameter
*    is != 0, the non-zero winding rule is used.
*   aStartX,
*   aStartY          - Determines the start position for the gradient.
*   aRadius          - Determines the radius of a circle.
*   aMatrix          - 2x2 component matrix describing evtl. scaling/rotation of
*     gradient coordinates.
*   aNoOfStops       - Number of color stops in the array aStops.
*   aStops           - Array with color stops. The entries in the array are
*     sorted according to the stop offset values.
*   aWorker          - Low-level worker function to perform the fill gradient
*     operation.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwEmulateFillRadialGradient
(
  XSurfaceMemory*   aDst,
  int*              aPaths,
  int               aDstX,
  int               aDstY,
  int               aWidth,
  int               aHeight,
  int               aAntialiased,
  int               aNonZeroWinding,
  int               aStartX,
  int               aStartY,
  int               aRadius,
  float*            aMatrix,
  int               aNoOfStops,
  XGradientStop*    aStops,
  XFillRadialGradientWorker aWorker
);


/*******************************************************************************
* FUNCTION:
*   EwPackColor
*
* DESCRIPTION:
*   The following EwPackColor() function has the job to convert the given RGBA
*   color channels into a generic packed 32 bit format as it is used internally
*   by the Graphics Engine.
*
*   The generic packed 32 bit color format provides an abstraction of a 32 bit
*   color value consisting of three color channels and one alpha channel. The
*   content of the three color chanels is a subject of the particular target
*   system - Graphics Engine doesn't need to know anything about the meaning of
*   these channels - they are abstract.
*
*   The generic packed 32 bit color format stores the channels in the following
*   order:
*
*              31           24           16             8            0
*               +------------+---------------------------------------+
*               |   alpha    |  channel 3 |  channel 2  |  channel 1 |
*               +------------+---------------------------------------+
*
*   Depending on the target color format, the function can modify and convert
*   the values of the channels. For example, the RGB -> YUV conversion can be
*   executed or the color channels can be pre-multiplied by the alpha value.
*
*   The resulting 32 bit value is particular for the respective target color
*   format. It is predestined to be used more effectively durring all drawing
*   operations.
*
* ARGUMENTS:
*   aRed,
*   aGreen,
*   aBlue,
*   aAlpha       - Non-premultiplied color channel values in the range 0 .. 255.
*   aPixelFormat - Hint identifying the format of the destination where drawings
*     with the color are performed. Based on this information the function can
*     eventually perform color space optimizations.
*
* RETURN VALUE:
*   Packed 32 bit color value. Particular for the target color format.
*
*******************************************************************************/
unsigned int EwPackColor
(
  int               aRed,
  int               aGreen,
  int               aBlue,
  int               aAlpha,
  int               aPixelFormat
);


/*******************************************************************************
* FUNCTION:
*   EwPackClutEntry
*
* DESCRIPTION:
*   The following EwPackClutEntry() function has the job to convert the given
*   RGBA color channels into a 32 bit wide clut entry as it is used by Index8
*   surfaces internally.
*
*   Like the EwPackColor() function, the resulting value depends on the used
*   native color format. Therefore, the order of color channels, color spaces,
*   etc. are not predetermined here. The result is just a 32 bit value without
*   any assumptions about its structure.
*
*   The resulting 32 bit value is particular for the respective target color
*   format. It is predestined to be used more effectively durring all drawing
*   Index8 bitmaps.
*
* ARGUMENTS:
*   aRed,
*   aGreen,
*   aBlue,
*   aAlpha  - Non-premultiplied color channel values in the range 0 .. 255.
*
* RETURN VALUE:
*   32 bit clut entry. Particular for the target color format.
*
*******************************************************************************/
unsigned int EwPackClutEntry
(
  int               aRed,
  int               aGreen,
  int               aBlue,
  int               aAlpha
);


/*******************************************************************************
* FUNCTION:
*   EwSetPixelSolid
*   EwScreenSetPixelSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to draw a single pixel with a solid color. The new pixel overwrites the
*   existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination surface.
*   aX, aY    - Position to draw the pixel.
*   aGradient - Solid color information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwSetPixelSolid
(
  XSurfaceMemory*   aDst,
  int               aX,
  int               aY,
  XGradient*        aGradient
);

void EwScreenSetPixelSolid
(
  XSurfaceMemory*   aDst,
  int               aX,
  int               aY,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwSetPixelSolidBlend
*   EwScreenSetPixelSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to draw a single pixel with a solid color. The new pixel will be alpha
*   blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination surface.
*   aX, aY    - Position to draw the pixel.
*   aGradient - Solid color information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwSetPixelSolidBlend
(
  XSurfaceMemory*   aDst,
  int               aX,
  int               aY,
  XGradient*        aGradient
);

void EwScreenSetPixelSolidBlend
(
  XSurfaceMemory*   aDst,
  int               aX,
  int               aY,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwFillRowSolid
*   EwScreenFillRowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a solid color. The new pixel overwrites
*   the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aWidth    - Width of the row.
*   aGradient - Solid color information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFillRowSolid
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenFillRowSolid
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwFillRowSolidBlend
*   EwScreenFillRowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a solid color. The new pixel will be
*   alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aWidth    - Width of the row.
*   aGradient - Solid color information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFillRowSolidBlend
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenFillRowSolidBlend
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwFillRowGradient
*   EwScreenFillRowGradient
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a color gradient. The new pixel will
*   overwrite the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aWidth    - Width of the row.
*   aGradient - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFillRowGradient
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenFillRowGradient
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwFillRowGradientBlend
*   EwScreenFillRowGradientBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a color gradient. The new pixel will be
*   alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aWidth    - Width of the row.
*   aGradient - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFillRowGradientBlend
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenFillRowGradientBlend
(
  XSurfaceMemory*   aDst,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyNativeRow
*   EwScreenCopyNativeRow
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row between two native surfaces. The operation
*   is executed without any additional opacity values. The new pixel overwrites
*   the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyNativeRow
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyNativeRow
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyNativeRowBlend
*   EwScreenCopyNativeRowBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row between two native surfaces. The operation
*   is executed without any additional opacity values. The new pixel will be
*   alpha blended withe the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyNativeRowBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyNativeRowBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyNativeRowSolid
*   EwScreenCopyNativeRowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row between two native surfaces. The operation
*   is executed with an additional solid opacity value. The new pixel will
*   overwrite the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Solid opacity information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyNativeRowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyNativeRowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyNativeRowSolidBlend
*   EwScreenCopyNativeRowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row between two native surfaces. The operation
*   is executed with an additional solid opacity value. The new pixel will be
*   alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Solid opacity information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyNativeRowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyNativeRowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyNativeRowGradient
*   EwScreenCopyNativeRowGradient
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row between two native surfaces. The operation
*   is executed with an additional opacity gradient. The new pixel overwrites
*   the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Opacity gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyNativeRowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyNativeRowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyNativeRowGradientBlend
*   EwScreenCopyNativeRowGradientBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row between two native surfaces. The operation
*   is executed with an additional opacity gradient. The new pixel will be alpha
*   blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Opacity gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyNativeRowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyNativeRowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyIndex8Row
*   EwScreenCopyIndex8Row
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an index8 to a native surface. The
*   operation is executed without any additional opacity values. The new pixel
*   overwrites the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyIndex8Row
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyIndex8Row
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyIndex8RowBlend
*   EwScreenCopyIndex8RowBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an index8 to a native surface. The
*   operation is executed without any additional opacity values. The new pixel
*   will be alpha blended withe the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyIndex8RowBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyIndex8RowBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyIndex8RowSolid
*   EwScreenCopyIndex8RowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an index8 to a native surface. The
*   operation is executed with an additional solid opacity value. The new pixel
*   will overwrite the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Solid opacity information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyIndex8RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyIndex8RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyIndex8RowSolidBlend
*   EwScreenCopyIndex8RowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an index8 to a native surface. The
*   operation is executed with an additional solid opacity value. The new pixel
*   will be alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Solid opacity information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyIndex8RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyIndex8RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyIndex8RowGradient
*   EwScreenCopyIndex8RowGradient
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an index8 to a native surface. The
*   operation is executed with an additional opacity gradient. The new pixel
*   overwrites the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Opacity gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyIndex8RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyIndex8RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyIndex8RowGradientBlend
*   EwScreenCopyIndex8RowGradientBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an index8 to a native surface. The
*   operation is executed with an additional opacity gradient. The new pixel
*   will be alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Opacity gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyIndex8RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyIndex8RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyAlpha8RowSolid
*   EwScreenCopyAlpha8RowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a solid color additionally modulated by
*   the opacity values from the source alpha8 surface. The new pixel overwrite
*   the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Solid color information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyAlpha8RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyAlpha8RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyAlpha8RowSolidBlend
*   EwScreenCopyAlpha8RowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a solid color additionally modulated by
*   the opacity values from the source alpha8 surface. The new pixel will be
*   alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Solid color information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyAlpha8RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyAlpha8RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyAlpha8RowGradient
*   EwScreenCopyAlpha8RowGradient
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a color gradient additionally modulated
*   by the opacity values from the source alpha8 surface. The new pixel will
*   overwrite the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyAlpha8RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyAlpha8RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyAlpha8RowGradientBlend
*   EwScreenCopyAlpha8RowGradientBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a color gradient additionally modulated
*   by the opacity values from the source alpha8 surface. The new pixel will be
*   alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyAlpha8RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyAlpha8RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyRGB565Row
*   EwScreenCopyRGB565Row
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an rgb565 to a native surface. The
*   operation is executed without any additional opacity values. The new pixel
*   overwrites the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyRGB565Row
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyRGB565Row
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyRGB565RowSolid
*   EwScreenCopyRGB565RowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an rgb565 to a native surface. The
*   operation is executed with an additional solid opacity value. The new pixel
*   will overwrite the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Solid opacity information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyRGB565RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyRGB565RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyRGB565RowSolidBlend
*   EwScreenCopyRGB565RowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an rgb565 to a native surface. The
*   operation is executed with an additional solid opacity value. The new pixel
*   will be alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Solid opacity information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyRGB565RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyRGB565RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyRGB565RowGradient
*   EwScreenCopyRGB565RowGradient
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an rgb565 to a native surface. The
*   operation is executed with an additional opacity gradient. The new pixel
*   overwrites the existing pixel within the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Opacity gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyRGB565RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyRGB565RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwCopyRGB565RowGradientBlend
*   EwScreenCopyRGB565RowGradientBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to copy a horizontal pixel row from an rgb565 to a native surface. The
*   operation is executed with an additional opacity gradient. The new pixel
*   will be alpha blended with the existing pixel of the destination.
*
* ARGUMENTS:
*   aDst      - Pointer to the first pixel of the destination row.
*   aSrc      - Pointer to the first pixel of the source row.
*   aWidth    - Width of the row.
*   aGradient - Opacity gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwCopyRGB565RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);

void EwScreenCopyRGB565RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpNativeRow
*   EwScreenWarpNativeRow
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface. The new pixel will overwrite the existing pixel in the destination
*   memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpNativeRow
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRow
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpNativeRowFilter
*   EwScreenWarpNativeRowFilter
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface.  For better output quality the source pixel are interpolated by a
*   bi-linear filter. The resulting pixel will overwrite the existing pixel in
*   the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpNativeRowFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpNativeRowBlend
*   EwScreenWarpNativeRowBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface. The new pixel will be alpha-blended with the existing pixel in the
*   destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpNativeRowBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpNativeRowFilterBlend
*   EwScreenWarpNativeRowFilterBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface.  For better output quality the source pixel are interpolated by a
*   bi-linear filter. The new pixel will be alpha-blended with the existing
*   pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpNativeRowFilterBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowFilterBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpNativeRowGradient
*   EwWarpNativeRowSolid
*   EwScreenWarpNativeRowGradient
*   EwScreenWarpNativeRowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. The new
*   pixel will overwrite the existing pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpNativeRowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpNativeRowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpNativeRowFilterGradient
*   EwWarpNativeRowFilterSolid
*   EwScreenWarpNativeRowFilterGradient
*   EwScreenWarpNativeRowFilterSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. For
*   better output quality the pixel are interpolated by a bi-linear filter. The
*   new pixel will overwrite the existing pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpNativeRowFilterGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpNativeRowFilterSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowFilterGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowFilterSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpNativeRowGradientBlend
*   EwWarpNativeRowSolidBlend
*   EwScreenWarpNativeRowGradientBlend
*   EwScreenWarpNativeRowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. The
*   resulting pixel will be alpha-blended with the pixel in the destination
*   memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpNativeRowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpNativeRowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpNativeRowFilterGradientBlend
*   EwWarpNativeRowFilterSolidBlend
*   EwScreenWarpNativeRowFilterGradientBlend
*   EwScreenWarpNativeRowFilterSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. For
*   better output quality the pixel are interpolated by a bi-linear filter.
*   The resulting pixel will be alpha-blended with the existing pixel in the
*   destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpNativeRowFilterGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpNativeRowFilterSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowFilterGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpNativeRowFilterSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpIndex8Row
*   EwScreenWarpIndex8Row
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface. The new pixel will overwrite the existing pixel in the destination
*   memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpIndex8Row
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8Row
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpIndex8RowFilter
*   EwScreenWarpIndex8RowFilter
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface.  For better output quality the source pixel are interpolated by a
*   bi-linear filter. The resulting pixel will overwrite the existing pixel in
*   the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpIndex8RowFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpIndex8RowBlend
*   EwScreenWarpIndex8RowBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface. The new pixel will be alpha-blended with the existing pixel in the
*   destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpIndex8RowBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpIndex8RowFilterBlend
*   EwScreenWarpIndex8RowFilterBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface.  For better output quality the source pixel are interpolated by a
*   bi-linear filter. The new pixel will be alpha-blended with the existing
*   pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpIndex8RowFilterBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowFilterBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpIndex8RowGradient
*   EwWarpIndex8RowSolid
*   EwScreenWarpIndex8RowGradient
*   EwScreenWarpIndex8RowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. The new
*   pixel will overwrite the existing pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpIndex8RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpIndex8RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpIndex8RowFilterGradient
*   EwWarpIndex8RowFilterSolid
*   EwScreenWarpIndex8RowFilterGradient
*   EwScreenWarpIndex8RowFilterSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. For
*   better output quality the pixel are interpolated by a bi-linear filter.
*   The new pixel will overwrite the existing pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpIndex8RowFilterGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpIndex8RowFilterSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowFilterGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowFilterSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpIndex8RowGradientBlend
*   EwWarpIndex8RowSolidBlend
*   EwScreenWarpIndex8RowGradientBlend
*   EwScreenWarpIndex8RowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. The
*   resulting pixel will be alpha-blended with the pixel in the destination
*   memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpIndex8RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpIndex8RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpIndex8RowFilterGradientBlend
*   EwWarpIndex8RowFilterSolidBlend
*   EwScreenWarpIndex8RowFilterGradientBlend
*   EwScreenWarpIndex8RowFilterSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. For
*   better output quality the pixel are interpolated by a bi-linear filter.
*   The resulting pixel will be alpha-blended with the existing pixel in the
*   destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpIndex8RowFilterGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpIndex8RowFilterSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowFilterGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpIndex8RowFilterSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpAlpha8RowGradient
*   EwWarpAlpha8RowSolid
*   EwScreenWarpAlpha8RowGradient
*   EwScreenWarpAlpha8RowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. The new
*   pixel will overwrite the existing pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpAlpha8RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpAlpha8RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpAlpha8RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpAlpha8RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpAlpha8RowFilterGradient
*   EwWarpAlpha8RowFilterSolid
*   EwScreenWarpAlpha8RowFilterGradient
*   EwScreenWarpAlpha8RowFilterSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. For
*   better output quality the pixel are interpolated by a bi-linear filter. The
*   new pixel will overwrite the existing pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpAlpha8RowFilterGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpAlpha8RowFilterSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpAlpha8RowFilterGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpAlpha8RowFilterSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpAlpha8RowGradientBlend
*   EwWarpAlpha8RowSolidBlend
*   EwScreenWarpAlpha8RowGradientBlend
*   EwScreenWarpAlpha8RowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. The
*   resulting pixel will be alpha-blended with the pixel in the destination
*   memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpAlpha8RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpAlpha8RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpAlpha8RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpAlpha8RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpAlpha8RowFilterGradientBlend
*   EwWarpAlpha8RowFilterSolidBlend
*   EwScreenWarpAlpha8RowFilterGradientBlend
*   EwScreenWarpAlpha8RowFilterSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. For
*   better output quality the pixel are interpolated by a bi-linear filter.
*   The resulting pixel will be alpha-blended with the existing pixel in the
*   destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpAlpha8RowFilterGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpAlpha8RowFilterSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpAlpha8RowFilterGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpAlpha8RowFilterSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpRGB565Row
*   EwScreenWarpRGB565Row
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface. The new pixel will overwrite the existing pixel in the destination
*   memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpRGB565Row
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565Row
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpRGB565RowFilter
*   EwScreenWarpRGB565RowFilter
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface.  For better output quality the source pixel are interpolated by a
*   bi-linear filter. The resulting pixel will overwrite the existing pixel in
*   the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - <Unused in this function>
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpRGB565RowFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowFilter
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwWarpRGB565RowGradient
*   EwWarpRGB565RowSolid
*   EwScreenWarpRGB565RowGradient
*   EwScreenWarpRGB565RowSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. The new
*   pixel will overwrite the existing pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpRGB565RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpRGB565RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpRGB565RowFilterGradient
*   EwWarpRGB565RowFilterSolid
*   EwScreenWarpRGB565RowFilterGradient
*   EwScreenWarpRGB565RowFilterSolid
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. For
*   better output quality the pixel are interpolated by a bi-linear filter.
*   The new pixel will overwrite the existing pixel in the destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpRGB565RowFilterGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpRGB565RowFilterSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowFilterGradient
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowFilterSolid
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpRGB565RowGradientBlend
*   EwWarpRGB565RowSolidBlend
*   EwScreenWarpRGB565RowGradientBlend
*   EwScreenWarpRGB565RowSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. The
*   resulting pixel will be alpha-blended with the pixel in the destination
*   memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpRGB565RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpRGB565RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

/*******************************************************************************
* FUNCTION:
*   EwWarpRGB565RowFilterGradientBlend
*   EwWarpRGB565RowFilterSolidBlend
*   EwScreenWarpRGB565RowFilterGradientBlend
*   EwScreenWarpRGB565RowFilterSolidBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise drawing operation
*   to fill a horizontal pixel row with a transformed content of the source
*   surface and modulate the pixel with opacity values from a gradient. For
*   better output quality the pixel are interpolated by a bi-linear filter.
*   The resulting pixel will be alpha-blended with the existing pixel in the
*   destination memory.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aSrc       - Pointer to the first pixel of the source surface.
*   aWidth     - Width of the row.
*   aS, aT     - Start values for the S and T texture mapping coordinates.
*   aSS, aTS   - Per pixel increment for the interpolation of S and T.
*   aSrcWidth,
*   aSrcHeight - Size of the source surface to warp.
*   aGradient  - Color gradient information.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwWarpRGB565RowFilterGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwWarpRGB565RowFilterSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowFilterGradientBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);

void EwScreenWarpRGB565RowFilterSolidBlend
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aWidth,
  int               aS,
  int               aT,
  int               aSS,
  int               aTS,
  int               aSrcWidth,
  int               aSrcHeight,
  XGradient*        aGradient
);


/*******************************************************************************
* FUNCTION:
*   EwFilterNative
*   EwScreenFilterNative
*
* DESCRIPTION:
*   The following functions define the low-level, pixel-wise drawing operation
*   to copy the contents from the native source surface, modulate the opacity
*   of the resulting pixel by the value aOpacity and finally write the results
*   into the destination surface or blend the results with the contents found
*   in the destination surface if aBlend is != 0.
*
*   This operation is in fact not a filter. It is like a copy transfer. However
*   the particularity of this version is the addresing within the destination
*   surface. Depending on the parameters stored in aDst, the worker can write
*   the pixel column-wise into the destination while reading the pixel row-wise
*   from the source. This mode is obligatory for all mask/tint filter worker
*   when these follow the blur filter. The blur filter changes the row/column
*   layout of the processed pixel.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination row.
*   aSrc          - Pointer to the first pixel of the source row.
*   aMask         - Always NULL.
*   aWidth        - Length of the row in pixel.
*   aColor        - Always NULL.
*   aIntensity    - Always 0.
*   aOpacity      - Modulates the opacity of the read source pixel data. This
*     parameter can take values in range of 0 .. 255.
*   aBlend        - If != 0, the drawn pixel will be alpha-blended with the
*     pixel in the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFilterNative
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterNative
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* FUNCTION:
*   EwFilterNativeMask
*   EwFilterNativeTintByOpacity
*   EwFilterNativeTintByLuminance
*   EwFilterNativeMaskTintByOpacity
*   EwFilterNativeMaskTintByLuminance
*   EwScreenFilterNativeMask
*   EwScreenFilterNativeTintByOpacity
*   EwScreenFilterNativeTintByLuminance
*   EwScreenFilterNativeMaskTintByOpacity
*   EwScreenFilterNativeMaskTintByLuminance
*
* DESCRIPTION:
*   The following functions define the low-level, pixel-wise drawing operation
*   to tint the contents from the native source surface by a color value found
*   in aGradient, mask the contents using a mask surface, modulate the opacity
*   of the resulting pixel by the value aOpacity and finally write the results
*   into the destination surface or blend the results with the contents found
*   in the destination surface if aBlend is != 0.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination row.
*   aSrc          - Pointer to the first pixel of the source row.
*   aMask         - Pointer to the first pixel of the mask row or NULL if no
*     mask is used.
*   aWidth        - Length of the row in pixel.
*   aColor        - A color value to be used by the tint filter stored in a
*     XGradient data structure.
*   aIntensity    - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0 .. 255. If the value is 255, the original
*     colors are completely replaced by values resulting from the filter. If
*     the value is equal zero, the original colors are used.
*   aOpacity      - Modulates the opacity of the pixel data resulting from the
*     filter operation. This parameter can take values in range of 0 .. 255.
*   aBlend        - If != 0, the drawn pixel will be alpha-blended with the
*     pixel in the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFilterNativeMask
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterNativeTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterNativeTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterNativeMaskTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterNativeMaskTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterNativeMask
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterNativeTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterNativeTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterNativeMaskTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterNativeMaskTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* FUNCTION:
*   EwFilterIndex8Mask
*   EwFilterIndex8TintByOpacity
*   EwFilterIndex8TintByLuminance
*   EwFilterIndex8MaskTintByOpacity
*   EwFilterIndex8MaskTintByLuminance
*   EwScreenFilterIndex8Mask
*   EwScreenFilterIndex8TintByOpacity
*   EwScreenFilterIndex8TintByLuminance
*   EwScreenFilterIndex8MaskTintByOpacity
*   EwScreenFilterIndex8MaskTintByLuminance
*
* DESCRIPTION:
*   The following functions define the low-level, pixel-wise drawing operation
*   to tint the contents from the index8 source surface by a color value found
*   in aGradient, mask the contents using a mask surface, modulate the opacity
*   of the resulting pixel by the value aOpacity and finally write the results
*   into the destination surface or blend the results with the contents found
*   in the destination surface if aBlend is != 0.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination row.
*   aSrc          - Pointer to the first pixel of the source row.
*   aMask         - Pointer to the first pixel of the mask row or NULL if no
*     mask is used.
*   aWidth        - Length of the row in pixel.
*   aColor        - A color value to be used by the tint filter stored in a
*     XGradient data structure.
*   aIntensity    - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0 .. 255. If the value is 255, the original
*     colors are completely replaced by values resulting from the filter. If
*     the value is equal zero, the original colors are used.
*   aOpacity      - Modulates the opacity of the pixel data resulting from the
*     filter operation. This parameter can take values in range of 0 .. 255.
*   aBlend        - If != 0, the drawn pixel will be alpha-blended with the
*     pixel in the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFilterIndex8Mask
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterIndex8TintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterIndex8TintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterIndex8MaskTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterIndex8MaskTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterIndex8Mask
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterIndex8TintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterIndex8TintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterIndex8MaskTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterIndex8MaskTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* FUNCTION:
*   EwFilterAlpha8Mask
*   EwFilterAlpha8TintByOpacity
*   EwFilterAlpha8TintByLuminance
*   EwFilterAlpha8MaskTintByOpacity
*   EwFilterAlpha8MaskTintByLuminance
*   EwScreenFilterAlpha8Mask
*   EwScreenFilterAlpha8TintByOpacity
*   EwScreenFilterAlpha8TintByLuminance
*   EwScreenFilterAlpha8MaskTintByOpacity
*   EwScreenFilterAlpha8MaskTintByLuminance
*
* DESCRIPTION:
*   The following functions define the low-level, pixel-wise drawing operation
*   to tint the contents from the alpha8 source surface by a color value found
*   in aGradient, mask the contents using a mask surface, modulate the opacity
*   of the resulting pixel by the value aOpacity and finally write the results
*   into the destination surface or blend the results with the contents found
*   in the destination surface if aBlend is != 0.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination row.
*   aSrc          - Pointer to the first pixel of the source row.
*   aMask         - Pointer to the first pixel of the mask row or NULL if no
*     mask is used.
*   aWidth        - Length of the row in pixel.
*   aColor        - A color value to be used by the tint filter stored in a
*     XGradient data structure.
*   aIntensity    - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0 .. 255. If the value is 255, the original
*     colors are completely replaced by values resulting from the filter. If
*     the value is equal zero, the original colors are used.
*   aOpacity      - Modulates the opacity of the pixel data resulting from the
*     filter operation. This parameter can take values in range of 0 .. 255.
*   aBlend        - If != 0, the drawn pixel will be alpha-blended with the
*     pixel in the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFilterAlpha8Mask
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterAlpha8TintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterAlpha8TintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterAlpha8MaskTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterAlpha8MaskTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterAlpha8Mask
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterAlpha8TintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterAlpha8TintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterAlpha8MaskTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterAlpha8MaskTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

/*******************************************************************************
* FUNCTION:
*   EwFilterRGB565Mask
*   EwFilterRGB565TintByOpacity
*   EwFilterRGB565TintByLuminance
*   EwFilterRGB565MaskTintByOpacity
*   EwFilterRGB565MaskTintByLuminance
*   EwScreenFilterRGB565Mask
*   EwScreenFilterRGB565TintByOpacity
*   EwScreenFilterRGB565TintByLuminance
*   EwScreenFilterRGB565MaskTintByOpacity
*   EwScreenFilterRGB565MaskTintByLuminance
*
* DESCRIPTION:
*   The following functions define the low-level, pixel-wise drawing operation
*   to tint the contents from the rgb565 source surface by a color value found
*   in aGradient, mask the contents using a mask surface, modulate the opacity
*   of the resulting pixel by the value aOpacity and finally write the results
*   into the destination surface or blend the results with the contents found
*   in the destination surface if aBlend is != 0.
*
* ARGUMENTS:
*   aDst          - Pointer to the first pixel of the destination row.
*   aSrc          - Pointer to the first pixel of the source row.
*   aMask         - Pointer to the first pixel of the mask row or NULL if no
*     mask is used.
*   aWidth        - Length of the row in pixel.
*   aColor        - A color value to be used by the tint filter stored in a
*     XGradient data structure.
*   aIntensity    - This parameter controls the intensity of the tint filter.
*     This value lies in range of 0 .. 255. If the value is 255, the original
*     colors are completely replaced by values resulting from the filter. If
*     the value is equal zero, the original colors are used.
*   aOpacity      - Modulates the opacity of the pixel data resulting from the
*     filter operation. This parameter can take values in range of 0 .. 255.
*   aBlend        - If != 0, the drawn pixel will be alpha-blended with the
*     pixel in the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFilterRGB565Mask
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterRGB565TintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterRGB565TintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterRGB565MaskTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwFilterRGB565MaskTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterRGB565Mask
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterRGB565TintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterRGB565TintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterRGB565MaskTintByOpacity
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterRGB565MaskTintByLuminance
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  XSurfaceMemory*   aMask,
  int               aWidth,
  int               aMaskInverted,
  XGradient*        aColor,
  int               aIntensity,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* FUNCTION:
*   EwFilterNativeBlur
*   EwScreenFilterNativeBlur
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'blur pixel sequence' operation from a native
*   to a native or screen surface. Please note, unlike other worker functions,
*   this version is not limitted to perform the operation row-wise. The pixel
*   can also be read/written column-wise.
*
*   The funcion calculates the blur effect for the given number of pixel within
*   the destination surface. Due to the blur effect, the source read operations
*   may involve more source pixel before and after the sequence. The larger the
*   blur radius, the more source pixel are involved.
*
*   Please note, for performance reasons, the Graphics Engine applies the so-
*   called box-blur algorithm. Here, the pixel enclosed within the the blur
*   area are simply accumulated and the resulting sum is divided by the number
*   of accumulated pixel.
*
* ARGUMENTS:
*   aDst     - Pointer to the first pixel of the destination sequence.
*   aSrc     - Pointer to the first pixel of the source sequence.
*   aCount   - Length of the sequence in pixel.
*   aLimit1,
*   aLimit2  - Offsets to the first and to the last pixel within the current row
*     calculated relative to the address specified in aSrc. The blur algorithm
*     considers only source pixel lying within this area as valid. Any further
*     pixel beyond this limit is considered as being transparent.
*   aClamp1  - Number of source pixel preceding this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aClamp2  - Number of source pixel following this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aRadius  - Blur radius in pixel.
*   aOpacity - Modulates the opacity of the pixel data resulting from the blur
*     operation. This parameter can take values in range of 0 .. 255.
*   aBlend   - If != 0, the drawn pixel will be alpha-blended with the pixel in
*     the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFilterNativeBlur
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aCount,
  int               aLimit1,
  int               aLimit2,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  int               aOpacity,
  int               aBlend
);

void EwScreenFilterNativeBlur
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aCount,
  int               aLimit1,
  int               aLimit2,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* FUNCTION:
*   EwFilterIndex8Blur
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'blur pixel sequence' operation from index8
*   to a native surface. Please note, unlike other worker functions, this
*   version is not limitted to perform the operation row-wise. The pixel can
*   also be read/written column-wise.
*
*   The funcion calculates the blur effect for the given number of pixel within
*   the destination surface. Due to the blur effect, the source read operations
*   may involve more source pixel before and after the sequence. The larger the
*   blur radius, the more source pixel are involved.
*
*   Please note, for performance reasons, the Graphics Engine applies the so-
*   called box-blur algorithm. Here, the pixel enclosed within the the blur
*   area are simply accumulated and the resulting sum is divided by the number
*   of accumulated pixel.
*
* ARGUMENTS:
*   aDst     - Pointer to the first pixel of the destination sequence.
*   aSrc     - Pointer to the first pixel of the source sequence.
*   aCount   - Length of the sequence in pixel.
*   aLimit1,
*   aLimit2  - Offsets to the first and to the last pixel within the current row
*     calculated relative to the address specified in aSrc. The blur algorithm
*     considers only source pixel lying within this area as valid. Any further
*     pixel beyond this limit is considered as being transparent.
*   aClamp1  - Number of source pixel preceding this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aClamp2  - Number of source pixel following this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aRadius  - Blur radius in pixel.
*   aOpacity - Modulates the opacity of the pixel data resulting from the blur
*     operation. This parameter can take values in range of 0 .. 255.
*   aBlend   - If != 0, the drawn pixel will be alpha-blended with the pixel in
*     the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFilterIndex8Blur
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aCount,
  int               aLimit1,
  int               aLimit2,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* FUNCTION:
*   EwAlpha8FilterAlpha8Blur
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'blur pixel sequence' operation from alpha8
*   to an alpha8 surface. Please note, unlike other worker functions, this
*   version is not limitted to perform the operation row-wise. The pixel can
*   also be read/written column-wise.
*
*   The funcion calculates the blur effect for the given number of pixel within
*   the destination surface. Due to the blur effect, the source read operations
*   may involve more source pixel before and after the sequence. The larger the
*   blur radius, the more source pixel are involved.
*
*   Please note, for performance reasons, the Graphics Engine applies the so-
*   called box-blur algorithm. Here, the pixel enclosed within the the blur
*   area are simply accumulated and the resulting sum is divided by the number
*   of accumulated pixel.
*
* ARGUMENTS:
*   aDst     - Pointer to the first pixel of the destination sequence.
*   aSrc     - Pointer to the first pixel of the source sequence.
*   aCount   - Length of the sequence in pixel.
*   aLimit1,
*   aLimit2  - Offsets to the first and to the last pixel within the current row
*     calculated relative to the address specified in aSrc. The blur algorithm
*     considers only source pixel lying within this area as valid. Any further
*     pixel beyond this limit is considered as being transparent.
*   aClamp1  - Number of source pixel preceding this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aClamp2  - Number of source pixel following this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aRadius  - Blur radius in pixel.
*   aOpacity - Modulates the opacity of the pixel data resulting from the blur
*     operation. This parameter can take values in range of 0 .. 255.
*   aBlend   - If != 0, the drawn pixel will be alpha-blended with the pixel in
*     the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwAlpha8FilterAlpha8Blur
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aCount,
  int               aLimit1,
  int               aLimit2,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* FUNCTION:
*   EwFilterRGB565Blur
*
* DESCRIPTION:
*   The following type declares a prototype for a worker function, which should
*   perform the low level software 'blur pixel sequence' operation from RGB565
*   to a native surface. Please note, unlike other worker functions, this
*   version is not limitted to perform the operation row-wise. The pixel can
*   also be read/written column-wise.
*
*   The funcion calculates the blur effect for the given number of pixel within
*   the destination surface. Due to the blur effect, the source read operations
*   may involve more source pixel before and after the sequence. The larger the
*   blur radius, the more source pixel are involved.
*
*   Please note, for performance reasons, the Graphics Engine applies the so-
*   called box-blur algorithm. Here, the pixel enclosed within the the blur
*   area are simply accumulated and the resulting sum is divided by the number
*   of accumulated pixel.
*
* ARGUMENTS:
*   aDst     - Pointer to the first pixel of the destination sequence.
*   aSrc     - Pointer to the first pixel of the source sequence.
*   aCount   - Length of the sequence in pixel.
*   aLimit1,
*   aLimit2  - Offsets to the first and to the last pixel within the current row
*     calculated relative to the address specified in aSrc. The blur algorithm
*     considers only source pixel lying within this area as valid. Any further
*     pixel beyond this limit is considered as being transparent.
*   aClamp1  - Number of source pixel preceding this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aClamp2  - Number of source pixel following this first pixel of the sequence
*     from where the algorithm starts to clamp the blur effect. With the clamp
*     the blur effect ends abruptly.
*   aRadius  - Blur radius in pixel.
*   aOpacity - Modulates the opacity of the pixel data resulting from the blur
*     operation. This parameter can take values in range of 0 .. 255.
*   aBlend   - If != 0, the drawn pixel will be alpha-blended with the pixel in
*     the background.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFilterRGB565Blur
(
  XSurfaceMemory*   aDst,
  XSurfaceMemory*   aSrc,
  int               aCount,
  int               aLimit1,
  int               aLimit2,
  int               aClamp1,
  int               aClamp2,
  int               aRadius,
  int               aOpacity,
  int               aBlend
);


/*******************************************************************************
* FUNCTION:
*   EwFillLinearGradientRow
*   EwScreenFillLinearGradientRow
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise function to fill
*   a horizontal pixel row with a complex linear color gradient masked by given
*   ALPHA8 pixel row. The new pixel overwrites the existing pixel within the
*   destination.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aMask      - Pointer to the first pixel of the mask ALPHA8 row.
*   aWidth     - Width of the row.
*   aDeltaX,
*   aDeltaY    - Distance between the first pixel of the row and the start
*     position of the gradient.
*   aUnitX,
*   aUnitY     - Unit vector describing the direction of the line. Expressed in
*     0.16 bit fix-point integer.
*   aNoOfStops - No of color stops belonging to the gradient.
*   aOffsets   - An array of offsets corresponding to the color stops.
*   aColors    - An array of colors corresponding to the color stops.
*     The colors are stored in packed format (see EwPackColor()).
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFillLinearGradientRow
(
  XSurfaceMemory*    aDst,
  XSurfaceMemory*    aMask,
  int                aWidth,
  int                aDeltaX,
  int                aDeltaY,
  int                aUnitX,
  int                aUnitY,
  int                aNoOfStops,
  int*               aOffsets,
  unsigned int*      aColors
);

void EwScreenFillLinearGradientRow
(
  XSurfaceMemory*    aDst,
  XSurfaceMemory*    aMask,
  int                aWidth,
  int                aDeltaX,
  int                aDeltaY,
  int                aUnitX,
  int                aUnitY,
  int                aNoOfStops,
  int*               aOffsets,
  unsigned int*      aColors
);


/*******************************************************************************
* FUNCTION:
*   EwFillLinearGradientRowBlend
*   EwScreenFillLinearGradientRowBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise function to fill
*   a horizontal pixel row with a complex linear color gradient masked by given
*   ALPHA8 pixel row. The new pixel will be alpha blended with the existing
*   pixel of the destination.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aMask      - Pointer to the first pixel of the mask ALPHA8 row.
*   aWidth     - Width of the row.
*   aDeltaX,
*   aDeltaY    - Distance between the first pixel of the row and the start
*     position of the gradient.
*   aUnitX,
*   aUnitY     - Unit vector describing the direction of the line. Expressed in
*     0.16 bit fix-point integer.
*   aNoOfStops - No of color stops belonging to the gradient.
*   aOffsets   - An array of offsets corresponding to the color stops.
*   aColors    - An array of colors corresponding to the color stops.
*     The colors are stored in packed format (see EwPackColor()).
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFillLinearGradientRowBlend
(
  XSurfaceMemory*    aDst,
  XSurfaceMemory*    aMask,
  int                aWidth,
  int                aDeltaX,
  int                aDeltaY,
  int                aUnitX,
  int                aUnitY,
  int                aNoOfStops,
  int*               aOffsets,
  unsigned int*      aColors
);

void EwScreenFillLinearGradientRowBlend
(
  XSurfaceMemory*    aDst,
  XSurfaceMemory*    aMask,
  int                aWidth,
  int                aDeltaX,
  int                aDeltaY,
  int                aUnitX,
  int                aUnitY,
  int                aNoOfStops,
  int*               aOffsets,
  unsigned int*      aColors
);


/*******************************************************************************
* FUNCTION:
*   EwFillRadialGradientRow
*   EwScreenFillRadialGradientRow
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise function to fill
*   a horizontal pixel row with a complex radial color gradient masked by given
*   ALPHA8 pixel row. The new pixel overwrites the existing pixel within the
*   destination.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aMask      - Pointer to the first pixel of the mask ALPHA8 row.
*   aWidth     - Width of the row.
*   aDeltaX,
*   aDeltaY    - Distance between the first pixel of the row and the start
*     position of the gradient.
*   aMatrix    - 2x2 component matrix describing evtl. scaling/rotation of
*     gradient coordinates. The components of the matrix are stored in 16.16
*     precision. If NULL no transformation is applied.
*   aNoOfStops - No of color stops belonging to the gradient.
*   aOffsets   - An array of offsets corresponding to the color stops.
*   aColors    - An array of colors corresponding to the color stops.
*     The colors are stored in packed format (see EwPackColor()).
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFillRadialGradientRow
(
  XSurfaceMemory*    aDst,
  XSurfaceMemory*    aMask,
  int                aWidth,
  int                aDeltaX,
  int                aDeltaY,
  int*               aMatrix,
  int                aNoOfStops,
  int*               aOffsets,
  unsigned int*      aColors
);

void EwScreenFillRadialGradientRow
(
  XSurfaceMemory*    aDst,
  XSurfaceMemory*    aMask,
  int                aWidth,
  int                aDeltaX,
  int                aDeltaY,
  int*               aMatrix,
  int                aNoOfStops,
  int*               aOffsets,
  unsigned int*      aColors
);


/*******************************************************************************
* FUNCTION:
*   EwFillRadialGradientRowBlend
*   EwScreenFillRadialGradientRowBlend
*
* DESCRIPTION:
*   The following function defines the low-level, pixel-wise function to fill
*   a horizontal pixel row with a complex radial color gradient masked by given
*   ALPHA8 pixel row. The new pixel will be alpha blended with the existing
*   pixel of the destination.
*
* ARGUMENTS:
*   aDst       - Pointer to the first pixel of the destination row.
*   aMask      - Pointer to the first pixel of the mask ALPHA8 row.
*   aWidth     - Width of the row.
*   aDeltaX,
*   aDeltaY    - Distance between the first pixel of the row and the start
*     position of the gradient.
*   aMatrix    - 2x2 component matrix describing evtl. scaling/rotation of
*     gradient coordinates. The components of the matrix are stored in 16.16
*     precision. If NULL no transformation is applied.
*   aNoOfStops - No of color stops belonging to the gradient.
*   aOffsets   - An array of offsets corresponding to the color stops.
*   aColors    - An array of colors corresponding to the color stops.
*     The colors are stored in packed format (see EwPackColor()).
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFillRadialGradientRowBlend
(
  XSurfaceMemory*    aDst,
  XSurfaceMemory*    aMask,
  int                aWidth,
  int                aDeltaX,
  int                aDeltaY,
  int*               aMatrix,
  int                aNoOfStops,
  int*               aOffsets,
  unsigned int*      aColors
);

void EwScreenFillRadialGradientRowBlend
(
  XSurfaceMemory*    aDst,
  XSurfaceMemory*    aMask,
  int                aWidth,
  int                aDeltaX,
  int                aDeltaY,
  int*               aMatrix,
  int                aNoOfStops,
  int*               aOffsets,
  unsigned int*      aColors
);


/*******************************************************************************
* MACRO:
*   EW_SVG_XXX
*
* DESCRIPTION:
*   The following enumerations define op-codes for elements and attributes in an
*   SVG document. While parsing an SVG file all relevant elements and attributes
*   found there are stored as corresponding commands with op-codes defined below.
*   At display time, the commands are executed one by one.
*
*******************************************************************************/

/* Op-Code to terminate a sequence. */
#define EW_SVG_NoOp                                         0

/* Op-Codes to start an element. No operands are involved. */
#define EW_SVG_EnterG                                       2
#define EW_SVG_EnterRect                                    3
#define EW_SVG_EnterCircle                                  4
#define EW_SVG_EnterEllipse                                 5
#define EW_SVG_EnterLine                                    6
#define EW_SVG_EnterPolyline                                7
#define EW_SVG_EnterPolygon                                 8
#define EW_SVG_EnterPath                                    9
#define EW_SVG_EnterUse                                    10
#define EW_SVG_EnterLinearGradient                         11
#define EW_SVG_EnterRadialGradient                         12
#define EW_SVG_EnterStop                                   13
#define EW_SVG_EnterSVG                                    14
#define EW_SVG_EnterDefs                                   15
#define EW_SVG_EnterStyle                                  16

/* Op-Codes to end an element. No operands are involved. */
#define EW_SVG_LeaveG                                      19
#define EW_SVG_LeaveRect                                   20
#define EW_SVG_LeaveCircle                                 21
#define EW_SVG_LeaveEllipse                                22
#define EW_SVG_LeaveLine                                   23
#define EW_SVG_LeavePolyline                               24
#define EW_SVG_LeavePolygon                                25
#define EW_SVG_LeavePath                                   26
#define EW_SVG_LeaveUse                                    27
#define EW_SVG_LeaveLinearGradient                         28
#define EW_SVG_LeaveRadialGradient                         29
#define EW_SVG_LeaveStop                                   30
#define EW_SVG_LeaveSVG                                    31
#define EW_SVG_LeaveDefs                                   32
#define EW_SVG_LeaveStyle                                  33

/* Op-Code for the corresponding 'id' and 'class' attributes. The Op-Codes
   are also used to define a CSS selector. Each opcode expects one 'uint32'
   operand identifying uniquely the id, class or element. */
#define EW_SVG_SetId                                       36
#define EW_SVG_SetClass                                    37
#define EW_SVG_SetElement                                  38

/* Op-Code to search for matching CSS and process enclosed attributes. */
#define EW_SVG_ProcessCSS                                  42
#define EW_SVG_SelectCSS                                   43

/* Op-Codes to set absolute coordinates and length values. Each opcode expect
   one 'float' operand. */
#define EW_SVG_SetX                                        46
#define EW_SVG_SetY                                        47
#define EW_SVG_SetWidth                                    48
#define EW_SVG_SetHeight                                   49
#define EW_SVG_SetCX                                       50
#define EW_SVG_SetCY                                       51
#define EW_SVG_SetX1                                       52
#define EW_SVG_SetY1                                       53
#define EW_SVG_SetX2                                       54
#define EW_SVG_SetY2                                       55
#define EW_SVG_SetR                                        56
#define EW_SVG_SetRX                                       57
#define EW_SVG_SetRY                                       58
#define EW_SVG_SetStrokeWidth                              59
#define EW_SVG_SetStrokeMiter                              60

/* Op-Codes to prefix unit for the following command. No operands are
   involved. */
#define EW_SVG_SetUnitPercentage                           66

/* Op-Codes to set drawing modes. No operands are involved. */
#define EW_SVG_SetGradientUnits_UserSpaceOnUse             69
#define EW_SVG_SetGradientUnits_ObjectBoundingBox          70
#define EW_SVG_SetStopCurrentColor                         71
#define EW_SVG_SetFillNone                                 72
#define EW_SVG_SetFillCurrentColor                         73
#define EW_SVG_SetStrokeNone                               74
#define EW_SVG_SetStrokeCurrentColor                       75
#define EW_SVG_SetFillRule_NonZero                         76
#define EW_SVG_SetFillRule_EvenOdd                         77
#define EW_SVG_SetLineCap_Butt                             78
#define EW_SVG_SetLineCap_Round                            79
#define EW_SVG_SetLineCap_Square                           80
#define EW_SVG_SetLineJoin_Miter                           81
#define EW_SVG_SetLineJoin_Round                           82
#define EW_SVG_SetLineJoin_Bevel                           83
#define EW_SVG_SetVectorEffect_None                        85
#define EW_SVG_SetVectorEffect_NonScaling                  86

/* Op-Codes to select gradients. Each op-code involves a 32-bit id identifying
   the desired gradient element. */
#define EW_SVG_SetFillGradient                             88
#define EW_SVG_SetStrokeGradient                           89

/* Op-Codes to set color values. Each op-code involves a 32-bit color stored
   in the ARGB order. */
#define EW_SVG_SetFillColor                                92
#define EW_SVG_SetStrokeColor                              93
#define EW_SVG_SetStopColor                                94

/* Op-Codes to set opacity values. Each op-code involves one 'float' operand. */
#define EW_SVG_SetOpacity                                  96
#define EW_SVG_SetFillOpacity                              97
#define EW_SVG_SetStrokeOpacity                            98
#define EW_SVG_SetStopOpacity                              99

/* Op-Codes to set offset values. Each op-code involves one 'float' operand. */
#define EW_SVG_SetOffset                                  102

/* Op-Codes to set view box values. Each op-code involves four 'float'
   operands. */
#define EW_SVG_SetViewBox                                 105

/* Op-Code to set a 'href' value. Each op-code involves one 'uint32' operand
   identifying the referenced element */
#define EW_SVG_SetRef                                     107

/* Op-Codes to perform a transformation with one operand. Each op-code involves
   one 'float' operand. */
#define EW_SVG_Translate1                                 108
#define EW_SVG_Scale1                                     109
#define EW_SVG_Rotate1                                    110
#define EW_SVG_SkewX                                      111
#define EW_SVG_SkewY                                      112

/* Op-Codes to perform a transformation with two operand. Each op-code involves
   two 'float' operand. */
#define EW_SVG_Translate2                                 113
#define EW_SVG_Scale2                                     114

/* Op-Codes to perform a transformation with three operand. Each op-code
   involves three 'float' operand. */
#define EW_SVG_Rotate3                                    115

/* Op-Codes to perform a transformation with six operand. Each op-code involves
   six 'float' operands a, b, c, d, e, f. */
#define EW_SVG_Matrix                                     116

/* Op-Code to start a list of point coordinates. Each op-code involves one
   'uint32' operand defining the number of following 'float' operands. */
#define EW_SVG_SetPoints                                  118

/* Op-Code to start a path command sequence. Each op-code involves one
   'uint32' operand defining the length of the sequence within the buffer. 
   Then follow the corresponding path commands. */
#define EW_SVG_SetPath                                    119

/* Op-Codes to describe the content of a path. Each op-code expects two 'float'
   operands x, y. */
#define EW_SVG_BeginPath                                  122
#define EW_SVG_AddLine                                    123

/* Op-Codes to describe the content of a path. Each op-code expects four 'float'
   operands x1, y1, x2, y2. */
#define EW_SVG_AddBezier2                                 124

/* Op-Codes to describe the content of a path. Each op-code expects six 'float'
   operands x1, y1, x2, y2, x3, y3. */
#define EW_SVG_AddBezier3                                 125

/* Op-Codes to describe the content of a path. Each op-code expects seven 'float'
   operands cx, cy, rx, ry, angle, start_angle, end_angle. */
#define EW_SVG_AddArc                                     126

/* Op-Codes to describe the content of a path. No operands are involved. */
#define EW_SVG_ClosePath                                  127


/*******************************************************************************
* TYPE:
*   XSVGCommands
*
* DESCRIPTION:
*   The structure XSVGCommands is used to store vector graphic commands (see
*   EW_SVG_XXX). The commands describe the content of an image. By processing
*   the commands the SVG content can be rendered.
*
*   Please note, command op-codes are stored in quads inside the same DWORD.
*   The operands belonging to the commands follow the quad. If necessary, a
*   no operation commands (EW_SVG_NoOp) can be used to align next following
*   commands to start at a new DWORD. Operands are either 32-bit floating-
*   point or 32-bit unsigned integer values, depending on the corresponding
*   op-code:
*
*  Bit 31...................0
*       +----+----+----+----+
*       | I4 | I3 | I2 | I1 |
*       +----+----+----+----+
*       |     Operand 1     |
*       |     Operand 2     |
*       |       ...         |
*       |     Operand N     |
*       +----+----+----+----+
*       | NO | NO | NO | I5 |
*       +----+----+----+----+
*       |     Operand 1     |
*       |     Operand 2     |
*       |       ...         |
*       |     Operand N     |
*       +----+----+----+----+
*       | NO | I8 | I7 | I6 |
*       +----+----+----+----+
*       |     Operand 1     |
*       |     Operand 2     |
*       |       ...         |
*       |     Operand N     |
*       +----+----+----+----+
*
* ELEMENTS:
*   Count - Number of DWORDs in the Data array.
*   Data  - Stores the op-codes and their operands.
*
*******************************************************************************/
typedef struct
{
  int               Count;
  unsigned int      Data[1];
}
XSVGCommands;


/*******************************************************************************
* FUNCTION:
*   EwAllocVideo
*
* DESCRIPTION:
*   The function EwAllocVideo() has the job to reserve a block of memory which
*   is exclusively intended for pure software surfaces.
*
*   The large size and the persistent character of surfaces may expect special
*   heap management and algorithms. The intension of this function is to handle
*   surface memory allocations in a separate way of any other allocations.
*
* ARGUMENTS:
*   aSize - Desired size of the new memory block in bytes.
*
* RETURN VALUE:
*   If successful, returns the pointer to the reserved memory block, otherwise
*   null is returned.
*
*******************************************************************************/
void* EwAllocVideo
(
  int               aSize
);


/*******************************************************************************
* FUNCTION:
*   EwFreeVideo
*
* DESCRIPTION:
*   The function EwFreeVideo() is the counterpart of EwAllocVideo() function.
*   Its job is to release the given memory block.
*
*   The large size and the persistent character of surfaces may expect special
*   heap management and algorithms. The intension of this function is to handle
*   surface memory allocations in a separate way of any other allocations.
*
* ARGUMENTS:
*   aMemory - Address of the memory block to release.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFreeVideo
(
  void*             aMemory
);


/*******************************************************************************
* MACRO:
*   EW_REDIRECT_WARP_FUNC
*
* DESCRIPTION:
*   The macro EW_REDIRECT_WARP_FUNC defines a new function as wrapper to an
*   already existing warp operations implemented in another function.
*
*   This macro is used in Software Pixel Driver modules implementing limited
*   set of possible warp operations. The wrapper function created by the macro
*   takes care of the right redirection of the operation to the corresponding
*   worker function.
*
* ARGUMENTS:
*   aFunc1 - Function to serve as the entry point for the operation.
*   aFunc2 - Function, which should be called by the wrapper.
*
*******************************************************************************/
#define EW_REDIRECT_WARP_FUNC( aFunc1, aFunc2 )                                \
  void aFunc1( XSurfaceMemory* aDst, XSurfaceMemory* aSrc, int aWidth,         \
    int aS, int aT, int aSS, int aTS, int aSrcWidth, int aSrcHeight,           \
    XGradient* aGradient )                                                     \
  {                                                                            \
    aFunc2( aDst, aSrc, aWidth, aS, aT, aSS, aTS, aSrcWidth, aSrcHeight,       \
            aGradient );                                                       \
  }


#ifdef __cplusplus
  }
#endif

#endif /* EWGFXDRIVER_H */

/* pba */
