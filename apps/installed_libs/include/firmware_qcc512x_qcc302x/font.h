#ifndef __FONT_H__
#define __FONT_H__

/*! file  @brief Font subsystem support.
** 
** 
Functions to control the font subsystem.
*/
/*!
@brief The index of a font. Should be less than the value returned from FontInit().
*/
typedef uint16 FontId;
/*!
@brief A character in UCS-2.
*/
typedef uint16 ucs2char;

#if TRAPSET_FONT

/**
 *  \brief Initalise the font subsystem.
 *   
 *  \return The number of fonts found, or zero if no fonts were found (for example, if the
 *  fonts.app file wasn't found in the filesystem).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_font
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
FontId FontInit(void );

/**
 *  \brief Retrieve one character glyph from a font.
 *   
 *  \param font The index of the font to get the glyph from 
 *  \param c The character to get from that font 
 *  \param glyph A buffer to write the glyph data into 
 *  \param max_glyph_wid The number of words which can be written to glyph (must be zero if glyph is
 *  NULL)
 *  \return The width of the glyph if it was found and fits in the buffer provided (or no
 *  buffer was provided), otherwise returns (size_t) -1.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_font
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
size_t FontGetGlyph(FontId font, ucs2char c, uint16 * glyph, size_t max_glyph_wid);
#endif /* TRAPSET_FONT */
#endif
