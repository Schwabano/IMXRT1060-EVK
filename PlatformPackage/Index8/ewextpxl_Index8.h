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
*   based on the Index8 native pixel format.
*
*   The format name 'Index8' refers to the generic 8 bit color format working
*   with a global color look-up table (CLUT also called palette). Instead of
*   color information, pixel store index numbers inside a color table. At the
*   runtime the table is used to convert the index into the real color.
*
*   Usually, this index/color transcription will be performed by the graphics
*   hardware. In order to support alpha-blending, Embedded Wizard also manages
*   its own copy of the color table and provides special algorithms to evaluate
*   the table very fast. Within its own color table, Embedded Wizard stores all
*   colors as 32 bit RGBA8888 values. Due to the 8 bit per index, the number of
*   entries within the color table is limited to 256:
*                 
*            7                   0
*            +-------------------+
*   pixel    |       index       |                   CLUT / Palette
*            +-------------------+
*                      |                      31   24     16      8     0
*                      |                      +-----+------+------+-----+
*                      |                   #0 |  A  |   B  |  G   |  R  |
*                      |                      +-----+------+------+-----+
*                      +---------------->  #1 |  A  |   B  |  G   |  R  |
*                                             +-----+------+------+-----+
*
*                                                        .....
*
*                                             +-----+------+------+-----+
*                                        #254 |  A  |   B  |  G   |  R  |
*                                             +-----+------+------+-----+
*                                        #255 |  A  |   B  |  G   |  R  |
*                                             +-----+------+------+-----+
*
*   If using this target format, ensure the following macro is defined in your
*   project or make file. The entire set of Embedded Wizard files has to be
*   compiled with this macro defined:
*
*     EW_ENABLE_COLOR_TABLE
*
*******************************************************************************/


/* Index8 expects a special mode enabled. Ensure, the entire project is 
   compiled with this macro set. */
#ifndef EW_ENABLE_COLOR_TABLE
  #error Please define the macro 'EW_ENABLE_COLOR_TABLE' in your make file.
#endif


/* Index8 works with non pre-multiplied colors only */
#ifdef EW_PREMULTIPLY_COLOR_CHANNELS
  #error Index8 color format does not provide pre-multiplied colors.
#endif


/* pba */
