/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016 - 2018

\file
    This defines functions for controlling the LCD display on the
    development board.

    An application wishing to use this should call dev_board_lcd_initialise()
    once during initialisation and can then follow with calls to
    dev_board_lcd_write().

    The PIOs required for I2C of the board need to be configured in the
    filesystem.
*/

#ifndef DEV_BOARD_LCD_H
#define DEV_BOARD_LCD_H

#include <types.h>

/*!
 * Initialise the LCD display. This should be called early in the application
 * initialisation.
 */
extern void dev_board_lcd_initialise(void);

/*!
 * Write text to the LCD display on the display at the line requested.
 * The two lines are numbered 0 and 1.
 *
 * \param line Index of line to write to. Valid values 0 or 1.
 * \text  text Text to write to the display
 */
extern void dev_board_lcd_write(uint8 line, char * text);


#endif // DEV_BOARD_LCD_H
