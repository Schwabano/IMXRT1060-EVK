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
*   This header belongs to the low level software driver for target platforms
*   based on the LUMA44 native pixel format. This implementation includes
*   following special options:
*
*   > Pre-multiplied or non pre-multiplied pixel luminance channel. Depending
*     on the graphics hardware, it may be expected that luminance channel is
*     stored and calculated as already alpha pre-multiplied value.
*
*     This option is controlled by the definition EW_PREMULTIPLY_COLOR_CHANNELS.
*     If this macro is defined, the pixel driver handles the luminance channel
*     as already pre-multiplied by the corresponding pixel alpha value.
*
*     To select the pre-multiplied color format take following line into your
*     'ewextgfx.h' file:
*
*       #define EW_PREMULTIPLY_COLOR_CHANNELS 1
*
*     If your graphics hardware doesn't work with pre-multiplied color channels
*     set the following line into your 'ewextgfx.h' file:
*
*       #define EW_PREMULTIPLY_COLOR_CHANNELS 0
*
*   > Custom specific color channel order. Depending on the graphics hardware,
*     it may be necessary to adapt the order in which the color channels are
*     stored in the video memory.
*
*     The format name 'LUMA44' refers to the generic 8 bit color format where
*     the luminance and alpha channels are stored within a 8 bit value in the
*     following manner:
*
*               8             4             0
*               +---------------------------+
*               |    alpha    |     lum     |
*               +---------------------------+
*
*
*     To control the order of channels, specify the bit start position of each
*     channel. For this purpose the macros EW_COLOR_CHANNEL_BIT_OFFSET_XXX are
*     available (XXX stands for LUM and ALPHA). For example to adapt this pixel
*     driver to graphics hardware using ALUM44 color format, add following lines
*     to your 'ewextgfx.h' file:
*
*       #define EW_COLOR_CHANNEL_BIT_OFFSET_LUM     4
*       #define EW_COLOR_CHANNEL_BIT_OFFSET_ALPHA   0
*
*******************************************************************************/


/* If not explicitly defined, assume default bit positions for the lum and alpha
   channels within a pixel memory. */
#ifndef EW_COLOR_CHANNEL_BIT_OFFSET_LUM
  #define EW_COLOR_CHANNEL_BIT_OFFSET_LUM    0
#endif

#ifndef EW_COLOR_CHANNEL_BIT_OFFSET_ALPHA
  #define EW_COLOR_CHANNEL_BIT_OFFSET_ALPHA  4
#endif


/* Verify the position of the lum and alpha channel */
#if (( EW_COLOR_CHANNEL_BIT_OFFSET_LUM != 0  ) &&                              \
     ( EW_COLOR_CHANNEL_BIT_OFFSET_LUM != 4 ))
  #error Wrong 'luminance' channel bit position. Only '0' or '4' allowed.
#endif

#if (( EW_COLOR_CHANNEL_BIT_OFFSET_ALPHA != 0  ) &&                            \
     ( EW_COLOR_CHANNEL_BIT_OFFSET_ALPHA != 4 ))
  #error Wrong 'alpha' channel bit position. Only '0' or '4' allowed.
#endif

#if ( EW_COLOR_CHANNEL_BIT_OFFSET_LUM == EW_COLOR_CHANNEL_BIT_OFFSET_ALPHA )
  #error Bit offset conflict in the color channel definition.
#endif


/* The following mode can be enabled for Index8 target systems only. LUMA44
   doesn't support the global CLUT/pallete. */
#ifdef EW_ENABLE_COLOR_TABLE
  #error Please remove the macro 'EW_ENABLE_COLOR_TABLE' from your make file.
#endif


/* pba */
