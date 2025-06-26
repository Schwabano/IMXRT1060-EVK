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
*   This is an internal header of the Embedded Wizard Graphics Engine EWGFX. It
*   declares the platform independent interface to an external bitmap and font
*   resource provider. In Embedded Wizard the access to resources is handled
*   separately by platform specific provider functionality. Depending on the
*   manner to store bitmap and font resources, the provider has the job to find,
*   open, decompress, load, etc. the desired bitmap or font.
*
*   The provider consists of two interfaces:
*
*   1. Bitmap loader. Its intention is to access and load bitmaps, images, etc.
*      Depending on the provider implementation, the bitmaps can be loaded from
*      the code memory or from an external storage medium.
*
*   2. Font loader. Its intention is to access and load fonts and glyphs.
*      Depending on the provider implementation, true type fonts can be opened
*      or pre-rasterized glyphs can be decompressed from the code memory.
*
*   Usually, a loader interface will consist of a header and an implementation
*   files, whose names are predetermined in order to allow the Graphics Engine
*   to access them independent of the particular target system:
*
*   > ewextbmp.h + ewextbmp.c -> for bitmap resource loader.
*
*   > ewextfnt.h + ewextfnt.c -> for font resource loader.
*
*******************************************************************************/

#ifndef EWGFXRES_H
#define EWGFXRES_H


#ifdef __cplusplus
  extern "C" 
  {
#endif


/* Predeklaration of platform dependent resource structures */
struct XBmpRes;
struct XFntRes;
struct XSVGRes;


/*******************************************************************************
* FUNCTION:
*   EwBmpInit
*
* DESCRIPTION:
*   The function EwBmpInit() is called during the initialization of the Graphics
*   Engine. Its intention is to give the external loader a chance to initialize
*   its private variables and to start the bitmap decoder functionality (if any)
*
* ARGUMENTS:
*   aArgs - User argument passed in the EwInitGraphicsEngine().
*
* RETURN VALUE:
*   If successful, returns != 0.
*
*******************************************************************************/
int EwBmpInit
(
  void*             aArgs
);


/*******************************************************************************
* FUNCTION:
*   EwBmpDone
*
* DESCRIPTION:
*   The function EwBmpDone() will be called during the shutdown of the Graphics
*   Engine. It gives the loader a chance to deinitialize itself just before the
*   Graphics Engine terminates.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwBmpDone
(
  void
);


/*******************************************************************************
* FUNCTION:
*   EwBmpOpen
*
* DESCRIPTION:
*   The function EwBmpOpen() will be called by the Graphics Engine in order to
*   initiate the access to the given bitmap resource. Depending on the manner,
*   the resources are handled in the particular platform system, the function
*   can open an image file or it can access bitmap information available in the
*   code memory.
*
*   The function should return a handle to the opened resource. By using this
*   handle, the bitmap content and its metrics can be requested by the Graphics
*   Engine without taking in account any platform specific aspects.
*
* ARGUMENTS:
*   aResource - Descriptor of the bitmap resource to open. The content of this
*     descriptor depends on the particular platform system.
*
* RETURN VALUE:
*   If sucessful, the function returns the handle to the opened bitmap resource.
*   If failed, the function returns null.
*
*******************************************************************************/
void* EwBmpOpen
(
  const struct XBmpRes* aResource
);


/*******************************************************************************
* FUNCTION:
*   EwBmpClose
*
* DESCRIPTION:
*   The function EwBmpClose() will be called by the Graphics Engine to finish
*   the processing of a bitmap resource. Depending on the manner, the resources
*   are handled in the particular platform system, the function can close the
*   previously opened file and free any temporarily reserved resources.
*
* ARGUMENTS:
*   aHandle - Handle to the bitmap resource to terminate the access.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwBmpClose
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwBmpGetCacheSearchTag
*
* DESCRIPTION:
*   The function EwBmpGetCacheSearchTag() will be called by the Graphics Engine
*   in order to query a tag identifying uniquely the bitmap within the surface
*   cache.
*
* ARGUMENTS:
*   aResource - Descriptor of the bitmap resource to query the search tag.
*
* RETURN VALUE:
*   If sucessful, the function returns the search tag associated to the bitmap.
*
*******************************************************************************/
const void* EwBmpGetCacheSearchTag
(
  const struct XBmpRes*  aResource
);


/*******************************************************************************
* FUNCTION:
*   EwBmpGetMetrics
*
* DESCRIPTION:
*   The function EwBmpGetMetrics() will be called by the Graphics Engine to get
*   the general attributes of the bitmap resource. By using of these attributes,
*   Graphics Engine can create and initialize video surfaces.
*
* ARGUMENTS:
*   aHandle         - Handle to the bitmap resource to determine its attributes.
*   aFormat         - Pointer to a variable, where the pixel format of the bitmap
*     resource will returned (See EW_PIXEL_FORMAT_XXX).
*   aNoOfFrames     - Pointer to a variable, where the number of frames included
*     within this bitmap resource will returned.
*   aNoOfVirtFrames - Pointer to a variable, where the number of all frames incl.
*     duplicates will be returned. This value can be greater than aNoOfFrames if
*     some of the frames do repeat in the bitmap.
*   aFrameWidth,
*   aFrameHeight    - Pointers to variables, where the size in pixel of a single
*     frame will be returned.
*   aFrameDelay     - Pointer to a variable, where the delay in milliseconds for
*     animated bitmap resources will be returned.
*
* RETURN VALUE:
*   If sucessful, the function returns != 0.
*
*******************************************************************************/
int EwBmpGetMetrics
(
  void*             aHandle,
  int*              aFormat,
  int*              aNoOfFrames,
  int*              aNoOfVirtFrames,
  int*              aFrameWidth,
  int*              aFrameHeight,
  int*              aFrameDelay
);


/*******************************************************************************
* FUNCTION:
*   EwBmpGetFrameMetrics
*
* DESCRIPTION:
*   The function EwBmpGetFrameMetrics() will be called by the Graphics Engine in
*   order to obtain attributes for a single bitmap resource frame.
*
* ARGUMENTS:
*   aHandle      - Handle to the bitmap resource to determine its attributes.
*   aFrameNo     - Number of the desired frame to get its attributes.
*   aOpqX, aOpqY,
*   aOpqWidth,
*   aOpqHeight   - Pointers to variables, where an optional full-opaque area of
*     the frame will be returned. By using this area, drawing operation can be
*     optimized. The returned origin lies relative to the top-left corner of the
*     frame.
*
* RETURN VALUE:
*   If sucessful, the function returns != 0.
*
*******************************************************************************/
int EwBmpGetFrameMetrics
(
  void*             aHandle,
  int               aFrameNo,
  int*              aOpqX,
  int*              aOpqY,
  int*              aOpqWidth,
  int*              aOpqHeight
);


/*******************************************************************************
* FUNCTION:
*   EwBmpGetFrameMemory
*
* DESCRIPTION:
*   The function EwBmpGetFrameMemory() will be called by the Graphics Engine in
*   order to verify whether it is possible to directly access the frame pixel
*   data of the bitmap resource represented by aHandle and in the positive case
*   to obtain the affected memory address pointers.
*
*   This function belongs to the interface separating the Graphics Engine from
*   the platform specific bitmap resource loader functionality. In the case the
*   target system stores the bitmap resources already in a usable format, the
*   function allows the Graphics Engine to use it directly without to need any
*   additional surface memory to be allocated and pixel contents to be copied.
*
*   This approach will not work when the bitmap resources are stored compressed
*   or loaded dynamically from e.g. file system. To work it is essential, that
*   the delivered pixel memory address is persistently valid (e.g. resides in
*   ROM code) and the pixel in the memory are stored in the format used by the
*   surfaces in the target system.
*
*   The function should return 0 if the direct access is not supported.
*
* ARGUMENTS:
*   aHandle  - Handle to the bitmap resource to obtain direct pixel memory
*     access.
*   aFrameNo - Number of the desired frame to get its pixel data.
*   aMemory  - Structure to receive the desired address information.
*
* RETURN VALUE:
*   If successful, the function fills the given aMemory structure with the
*   address information and returns != 0. Otherwise 0 is returned.
*
*******************************************************************************/
int EwBmpGetFrameMemory
(
  void*             aHandle,
  int               aFrameNo,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwBmpLoadFrame
*
* DESCRIPTION:
*   The function EwBmpLoadFrame() will be called by the Graphics Engine in order
*   to load the frame pixel data from the openend bitmap resource aHandle into
*   the given memory area aMemory. Depending on the manner, the resources are
*   handled in the particular platform, the pixel data may be loaded from a file
*   or they can be decompressed from the code memory.
*
* ARGUMENTS:
*   aHandle  - Handle to the bitmap resource to load the pixel data.
*   aFrameNo - Number of the desired frame to get its pixel data.
*   aMemory  - Memory descriptor containing pointers and pitch information of
*     the destination to write the pixel data.
*
* RETURN VALUE:
*   If sucessful, the function returns != 0.
*
*******************************************************************************/
int EwBmpLoadFrame
(
  void*             aHandle,
  int               aFrameNo,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwBmpLoadClut
*
* DESCRIPTION:
*   The function EwBmpLoadClut() will be called by the Graphics Engine in order
*   to load the Clut data from the openend bitmap resource aHandle into the 
*   given memory area aMemory.
*
* ARGUMENTS:
*   aHandle  - Handle to the bitmap resource to load the CLUT data.
*   aMemory  - Memory descriptor containing pointers to the CLUT where to copy
*     the CLUT data.
*
* RETURN VALUE:
*   If sucessful, the function returns != 0.
*
*******************************************************************************/
int EwBmpLoadClut
(
  void*             aHandle,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwBmpLoadMappingTable
*
* DESCRIPTION:
*   The function EwBmpLoadMappingTable() will be called by the Graphics Engine
*   in order to load a table used to map between virtual and real frame numbers.
*   If a bitmap contains repetitions of a frame, the corresponding frames are
*   stored only once (as real frame). The numbers of original frames need to be
*   mapped to the numbers of the real (existing) frames.
*
* ARGUMENTS:
*   aHandle  - Handle to the bitmap resource to load the table.
*   aMapping - Pointer to memory where to copy the table. The memory area has
*     to be large enough.
*
* RETURN VALUE:
*   If sucessful, the function returns the number of virtual frames (the number
*   of copied entries).
*
*******************************************************************************/
int EwBmpLoadMappingTable
(
  void*             aHandle,
  unsigned short*   aMapping
);


/*******************************************************************************
* FUNCTION:
*   EwFntInit
*
* DESCRIPTION:
*   The function EwFntInit() is called during the initialization of the Graphics
*   Engine. Its intention is to give the external loader a chance to initialize
*   its private variables and to start the TrueType font engine functionality
*   (if any).
*
* ARGUMENTS:
*   aArgs - User argument passed in the EwInitGraphicsEngine().
*
* RETURN VALUE:
*   If successful, returns != 0.
*
*******************************************************************************/
int EwFntInit
(
  void*             aArgs
);


/*******************************************************************************
* FUNCTION:
*   EwFntDone
*
* DESCRIPTION:
*   The function EwFntDone() will be called during the shutdown of the Graphics
*   Engine. It gives the loader a chance to deinitialize itself just before the
*   Graphics Engine terminates.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFntDone
(
  void
);


/*******************************************************************************
* FUNCTION:
*   EwFntGetResource
*
* DESCRIPTION:
*   The function EwFntGetResource() has the job to find or create a new resource
*   descriptor matching the attributes specified in the function parameters. The
*   returned resource can thereupon be passed to EwLoadFont() function in order
*   to use the font.
*
*   Please note: this function allows the caller to flexibly create fonts on
*   target systems supporting a TrueType font engine. In the case, you are using
*   the version without TrueType font engine integration, all fonts are already
*   stored as prerasterized glyphs. Dynamic creation of a font with different
*   attributes is in such case not possible and the function will return null.
*
* ARGUMENTS:
*   aName        - A unique name of the TrueType font stored as zero terminated
*     ASCII string (e.g. "Arial"). The function limits the evaluation to the
*     first 31 characters from the string.
*   aHeight      - The desired height of the font in pixel.
*   aBold        - This parameter determines, whether a bold or a normal style
*     of the font should be used at the runtime. A bold font will be used if
*     this parameter contains a value != 0.
*   aItalic      - This parameter determines, whether an italic or a normal
*     style of the font should be used at the runtime. An italic font is used
*     if this parameter contains a value != 0.
*   aAspectRatio - The AspectRatio parameter defines the desired aspect ratio
*     of the font in the range from 0.25 to 4.0. The default value is 1.0 - in
*     this case the aspect ratio of the font corresponds to the origin design
*     of the font.
*   aNoOfColors  - Desired quality of the font glyphs to raster. Only the 
*     values 2, 4 or 16 are valid.
*   aKerning     - Determines whether kerning data should be used for this
*     font. In such case the value has to be != 0.
*   aRowDistance - The desired distance between two consecutive text rows. If
*     the value is == 0, the distance is calculated on the base of the font
*     metrics.
*   aPopularMode - If != 0, calculate the font size similarly to how other
*     application it does. If == 0, the mode compatible to older Embedded
*     Wizard version is used.
*
* RETURN VALUE:
*   Returns a pointer to a data structure representing the font resource. If
*   the target system does not support the dynamic font creation, the function
*   will return null.
*
*******************************************************************************/
const struct XFntRes* EwFntGetResource
(
  const char*       aName,
  int               aHeight,
  int               aBold,
  int               aItalic,
  float             aAspectRatio,
  int               aNoOfColors,
  int               aKerning,
  int               aRowDistance,
  int               aPopularMode
);


/*******************************************************************************
* FUNCTION:
*   EwFntOpen
*
* DESCRIPTION:
*   The function EwFntOpen() will be called by the Graphics Engine in order to
*   initiate the access to the given font resource. Depending on the manner,
*   the resources are handled in the particular platform system, the function
*   can open a TrueType file or it can access font information available in the
*   code memory.
*
*   The function should return a handle to the opened resource. By using this
*   handle, the font content and its metrics can be requested by the Graphics
*   Engine without taking in account any platform specific aspects.
*
* ARGUMENTS:
*   aResource - Descriptor of the font resource to open. The content of this
*     descriptor depends on the particular platform system.
*
* RETURN VALUE:
*   If sucessful, the function returns the handle to the opened font resource.
*   If failed, the function returns null.
*
*******************************************************************************/
void* EwFntOpen
(
  const struct XFntRes*  aResource
);


/*******************************************************************************
* FUNCTION:
*   EwFntClose
*
* DESCRIPTION:
*   The function EwFntClose() will be called by the Graphics Engine to finish
*   the processing of a font resource. Depending on the manner, the resources
*   are handled in the particular platform system, the function can close the
*   previously opened file and free any temporarily reserved resources.
*
* ARGUMENTS:
*   aHandle - Handle to the font resource to terminate the access.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFntClose
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwFntGetResourceName
*
* DESCRIPTION:
*   The function EwFntGetResourceName() will be called by the Graphics Engine in
*   order to query the name of the corresponding font resource member. This name
*   is used to display error messages and warnings.
*
*   The function should return a pointer to a zero terminated char string.
*
* ARGUMENTS:
*   aHandle - Handle of the font resource to query its name.
*
* RETURN VALUE:
*   If sucessful, the function returns the pointer to the char string.
*
*******************************************************************************/
const char* EwFntGetResourceName
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwFntGetCacheSearchTag
*
* DESCRIPTION:
*   The function EwFntGetCacheSearchTag() will be called by the Graphics Engine
*   in order to query a tag identifying uniquely the font within the font glyph
*   cache.
*
*   In case of multiple variants of a font resource differing only in character
*   code mapping, the function should return a tag common for all variants of
*   the font. This will promote the reusage of the glyphs in the glyph cache.
*
* ARGUMENTS:
*   aHandle - Handle of the font resource to query the search tag.
*
* RETURN VALUE:
*   If sucessful, the function returns the search tag associated to the font.
*
*******************************************************************************/
const void* EwFntGetCacheSearchTag
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwFntGetMetrics
*
* DESCRIPTION:
*   The function EwFntGetMetrics() will be called by the Graphics Engine to get
*   the general attributes of the font resource. By using of these attributes,
*   Graphics Engine can create and initialize new font instances.
*
* ARGUMENTS:
*   aHandle   - Handle to the font resource to determine its attributes.
*   aAscent,
*   aDescent,
*   aLeading  - Pointers to variables, where the font metrics will be returned.
*
* RETURN VALUE:
*   If sucessful, the function returns != 0.
*
*******************************************************************************/
int EwFntGetMetrics
(
  void*             aHandle,
  int*              aAscent,
  int*              aDescent,
  int*              aLeading
);


/*******************************************************************************
* FUNCTION:
*   EwFntGetGlyphMetrics
*
* DESCRIPTION:
*   The function EwFntGetGlyphMetrics() will be called by the Graphics Engine in
*   order to obtain the metrics of a single glyph from the given font.
*
* ARGUMENTS:
*   aHandle   - Handle to the font resource to determine its attributes.
*   aGlyphId  - ID of the glyph to be inspected.
*   aOriginX,
*   aOriginY  - Pointers to variables, where the origin offset in pixel of the
*     desired glyph will be returned.
*   aWidth,
*   aHeight   - Pointers to variables, where the size in pixel of the glyph will
*     be returned.
*   aAdvance  - Pointer to a variable, where the horizontal advance in pixel for
*     the glyph will be returned.
*
* RETURN VALUE:
*   If sucessful, the function returns != 0.
*
*******************************************************************************/
int EwFntGetGlyphMetrics
(
  void*             aHandle,
  unsigned int      aGlyphId,
  int*              aOriginX,
  int*              aOriginY,
  int*              aWidth,
  int*              aHeight,
  int*              aAdvance
);


/*******************************************************************************
* FUNCTION:
*   EwFntGetKerning
*
* DESCRIPTION:
*   The function EwFntGetKerning() will be called by the Graphics Engine in
*   order to obtain the kerning value for the pair of two glyphs from the given
*   font.
*
* ARGUMENTS:
*   aHandle   - Handle to the font resource to determine the kerning.
*   aGlyphId1 - ID of the first glyph.
*   aGlyphId2 - ID of the second glyph.
*
* RETURN VALUE:
*   The function returns the kerning value for the given pair of glyphs or 0
*   if the glyphs are not affected by kerning or at least one of the glyphs is
*   not existing in the font.
*
*******************************************************************************/
int EwFntGetKerning
(
  void*             aHandle,
  unsigned int      aGlyphId1,
  unsigned int      aGlyphId2
);


/*******************************************************************************
* FUNCTION:
*   EwFntIsGlyphAvailable
*
* DESCRIPTION:
*   The function EwFntIsGlyphAvailable() will be called by the Graphics Engine
*   in order to verify whether the given font contains a glyph for a character.
*
* ARGUMENTS:
*   aHandle   - Handle to the font resource to determine the glyph existence.
*   aCharCode - Character code of the glyph.
*
* RETURN VALUE:
*   The function returns != 0 if the glyph exists. Otherwise 0 is returned.
*
*******************************************************************************/
int EwFntIsGlyphAvailable
(
  void*             aHandle,
  unsigned short    aCharCode
);


/*******************************************************************************
* FUNCTION:
*   EwFntGetGlyphId
*
* DESCRIPTION:
*   The function EwFntGetGlyphId() will be called by the Graphics Engine in
*   order to query the glyph id for a given character.
*
* ARGUMENTS:
*   aHandle   - Handle to the font resource to determine the glyph existence.
*   aCharCode - Character code of the glyph.
*
* RETURN VALUE:
*   The function returns != 0 if the glyph exists. Otherwise 0 is returned.
*   The value 0 corresponds in this case to the '.notdef' glyph.
*
*******************************************************************************/
unsigned int EwFntGetGlyphId
(
  void*             aHandle,
  unsigned short    aCharCode
);


/*******************************************************************************
* FUNCTION:
*   EwFntInitShaper
*
* DESCRIPTION:
*   The function EwFntInitShaper() is called by the Graphics Engine in order
*   to prepare a context object for the following cluster shaping process. The
*   context is used by the shaper to store state information for the already
*   processed text and depending on it to decide how to shape the following
*   clusters.
*
*   If not used anymore, the context should be freed by using EwFntDoneShaper()
*   function.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   The function returns a handle to the new created shaper context object.
*
*******************************************************************************/
void* EwFntInitShaper
(
  void
);


/*******************************************************************************
* FUNCTION:
*   EwFntDoneShaper
*
* DESCRIPTION:
*   The function EwFntDoneShaper() is called by the Graphics Engine in order
*   to release a context object used previously during shaping process. The
*   context is ceated by the function EwFntInitShaper().
*
* ARGUMENTS:
*   aHandle - Handle to the shaper context.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFntDoneShaper
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwFntResetShaper
*
* DESCRIPTION:
*   The function EwFntResetShaper() is called by the Graphics Engine in order
*   to restore the default state of the shaper context object.
*
* ARGUMENTS:
*   aHandle - Handle to the shaper context.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFntResetShaper
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwFntSaveShaper
*
* DESCRIPTION:
*   The function EwFntSaveShaper() is called by the Graphics Engine in order to
*   store the actual state of the shaper context object temporarily. To restore
*   the state again, the function EwFntRestoreShaper() is used.
*
* ARGUMENTS:
*   aHandle - Handle to the shaper context.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFntSaveShaper
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwFntRestoreShaper
*
* DESCRIPTION:
*   The function EwFntRestoreShaper() is called by the Graphics Engine in order
*   to restore the state of the shaper context object stored previously by the
*   invocation of the function EwFntSaveShaper().
*
*   Please note: When calling the function, the actual state of the shaper is
*   saved too. It is as if the saved and actual states were exchanged.
*
* ARGUMENTS:
*   aHandle - Handle to the shaper context.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwFntRestoreShaper
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwFntShapeCluster
*
* DESCRIPTION:
*   The function EwFntShapeCluster() is called by the Graphics Engine in order
*   to evaluate the text (aCharCodes) looking for the next indivisible unit
*   (cluster) and convert the character codes in corresponding glyph ids. The
*   glyph ids are then returned in the array aGlyphIds.
*
*   Depending on the text found in aCharCodes and features available in the
*   font, the function may apply diverse reordering and substitution steps so
*   the returned glyphs represent the original text formatted correctly. This
*   functionality is known as 'layout shaping' or 'complex text layout'. For
*   example, the function may convert a sequence of character codes to a unique
*   glyph id representing a ligature of those characters.
*   
* ARGUMENTS:
*   aHandle        - Handle to the font resource to process the characters and
*     estimate the right glyph ids.
*   aCharCodes     - An array containing the original text as Unicode code
*     points. The maximum number of characters in this array, the function may
*     process, is determined by the parameter aNoOfCharCodes.
*     Usually, the storage order of character codes corresponds to the order
*     in which the characters are rendered on the screen from left to right.
*     This can be modified by the parameter aDirection.
*   aNoOfCharCodes - Number of characters provided in the array aCharCodes.
*   aDirection     - Provides information about the order of how characters
*     in the aCharCodes array should be evaluated. If aDirection is 0 (zero),
*     the characters are stored in the order corresponding to how the glyphs
*     are rendered from left to right direction. If aDirection is != 0, the
*     codes are stored in reversed order. This means the first character is
*     displayed on the right of the second character.
*   aGlyphIds      - An array where resulting glyph ids are stored. The
*     parameter aNoOfGlyphIds determines the capacity of this array.
*   aDeltasOriginX - An array where horizontal displacement of the resulting
*     glyphs is stored. The parameter aNoOfGlyphIds determines the capacity
*     of this array.
*   aDeltasOriginY - An array where vertical displacement of the resulting
*     glyphs is stored. The parameter aNoOfGlyphIds determines the capacity
*     of this array.
*   aDeltasAdvance - An array where horizontal displacement of the following
*     glyph position is stored. The parameter aNoOfGlyphIds determines the
*     capacity of this array.
*   aNoOfGlyphIds  - As input parameter determines the capacity of the arrays
*     aGlyphIds, aGlyphXOffsets and aGlyphYOffsets. When the function has
*     finished the processing, the parameter will store how many glyphs and
*     offsets are available in the arrays aGlyphIds, aDeltasOriginX,
*     aDeltasOriginY and aDeltasAdvance.
*   aLineEnd       - If there is a potential line end after the just shaped
*     cluster, the variable referred by aLineEnd is initialized with a value
*     != 0. If there is no line break candidate between the processed cluster
*     and next following cluster, value 0 is assigned to the variable.
*   aContext       - Handle to the shaper context containing the information
*     collected during the process of the preceding cluster. This information
*     can be used when processing a new cluster. EwFntShapeCluster() should
*     thereupon update the context.
*
* RETURN VALUE:
*   The function returns how many character codes from the array aCharCodes
*   haven been processed. If the function couldn't even process a single
*   character, the returned value is 0.
*
*******************************************************************************/
int EwFntShapeCluster
(
  void*                 aHandle,
  const unsigned short* aCharCodes,
  int                   aNoOfCharCodes,
  int                   aDirection,
  unsigned int*         aGlyphIds,
  short*                aDeltasOriginX,
  short*                aDeltasOriginY,
  short*                aDeltasAdvance,
  int*                  aNoOfGlyphIds,
  int*                  aLineEnd,
  void*                 aContext
);


/*******************************************************************************
* FUNCTION:
*   EwFntLoadGlyph
*
* DESCRIPTION:
*   The function EwFntLoadGlyph() will be called by the Graphics Engine in order
*   to load the glyph pixel data from the openend font resource aHandle into the
*   given memory area aMemory. Depending on the manner, a resource is handled in
*   the particular platform, the pixel data may be loaded from a TrueType file
*   or they can be decompressed from the code memory.
*
* ARGUMENTS:
*   aHandle  - Handle to the font resource to load its glyph pixel data.
*   aGlyphId - ID of the glyph to be loaded.
*   aMemory  - Memory descriptor containing pointers and pitch information of
*     the destination to write the glyph pixel data.
*
* RETURN VALUE:
*   If sucessful, the function returns != 0.
*
*******************************************************************************/
int EwFntLoadGlyph
(
  void*             aHandle,
  unsigned int      aGlyphId,
  XSurfaceMemory*   aMemory
);


/*******************************************************************************
* FUNCTION:
*   EwSVGInit
*
* DESCRIPTION:
*   The function EwSVGInit() is called during the initialization of the Graphics
*   Engine. Its intention is to give the external loader a chance to initialize
*   its private variables.
*
* ARGUMENTS:
*   aArgs - User argument passed in the EwInitGraphicsEngine().
*
* RETURN VALUE:
*   If successful, returns != 0.
*
*******************************************************************************/
int EwSVGInit
(
  void*             aArgs
);


/*******************************************************************************
* FUNCTION:
*   EwSVGDone
*
* DESCRIPTION:
*   The function EwSVGDone() will be called during the shutdown of the Graphics
*   Engine. It gives the loader a chance to deinitialize itself just before the
*   Graphics Engine terminates.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwSVGDone
(
  void
);


/*******************************************************************************
* FUNCTION:
*   EwSVGOpen
*
* DESCRIPTION:
*   The function EwSVGOpen() will be called by the Graphics Engine in order to
*   initiate the access to the given SVG resource. Depending on the manner, the
*   resources are handled in the particular platform system, the function can
*   open an image file or it can access bitmap information available in the code
*   memory.
*
*   The function should return a handle to the opened resource. By using this
*   handle, the SVG content can be requested by the Graphics Engine without
*   taking in account any platform specific aspects.
*
* ARGUMENTS:
*   aResource - Descriptor of the SVG resource to open. The content of this
*     descriptor depends on the particular platform system.
*
* RETURN VALUE:
*   If sucessful, the function returns the handle to the opened SVG resource.
*   If failed, the function returns null.
*
*******************************************************************************/
void* EwSVGOpen
(
  const struct XSVGRes* aResource
);


/*******************************************************************************
* FUNCTION:
*   EwSVGClose
*
* DESCRIPTION:
*   The function EwSVGClose() will be called by the Graphics Engine to finish
*   the processing of an SVG resource. Depending on the manner, the resources
*   are handled in the particular platform system, the function can close the
*   previously opened file and free any temporarily reserved resources.
*
* ARGUMENTS:
*   aHandle - Handle to the SVG resource to terminate the access.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwSVGClose
(
  void*             aHandle
);


/*******************************************************************************
* FUNCTION:
*   EwSVGGetConstData
*
* DESCRIPTION:
*   The function EwSVGGetConstData() will be called by the Graphics Engine to
*   access a command list describing the SVG elements and their attributes. By
*   evaluating the command list, the SVG content can be rendered. This function
*   assumes, the commands list is stored as const data. 
*
* ARGUMENTS:
*   aHandle   - Handle to the bitmap resource to determine its attributes.
*   aCommands - Pointer to a variable to receive the address of SVG commands.

* RETURN VALUE:
*   If sucessful, the function returns != 0.
*
*******************************************************************************/
int EwSVGGetConstData
(
  void*                aHandle,
  const XSVGCommands** aCommands
);


#ifdef __cplusplus
  }
#endif

#endif /* EWGFXRES_H */


/* msy, pba */
