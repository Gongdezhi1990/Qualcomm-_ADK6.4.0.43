#ifndef __LCD_H__
#define __LCD_H__
#include <app/lcd/lcd_if.h>



#if TRAPSET_LCD

/**
 *  \brief Configures the LCD hardware. 
 *  \param key Keys are defined in \#lcd_config_key. 
 *  \param value Depends on the key, and is defined in lcd_if.h 
 *  \return TRUE if successful, otherwise FALSE More detailed information on the keys and
 *  values can be found in lcd_if.h
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_lcd
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool LcdConfigure(uint16 key, uint16 value);

/**
 *  \brief Controls the activation of lcd segments. These segments must have been
 *  specified using VM trap PioSetLcdPins(). 
 *  \param mask Each bit in the mask corresponds to a PIO line, where b0=PIO0. Bits set to 1
 *  will be modified. Bits set to 0 will not be modified. 
 *  \param value Each bit specifies a PIO's LCD activation: 1=on, 0=off. 
 *  \return All zeros if sucessful, otherwise incorrect bits
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_lcd
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
uint32 LcdSet(uint32 mask, uint32 value);
#endif /* TRAPSET_LCD */
#endif
