/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    lcd_if.h

CONTAINS
    Definitions for the LCD subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __LCD_IF_H__
#define __LCD_IF_H__

typedef enum
{
 /*!
     Sets lcd block clock divider using value1 in the VM trap
     LcdConfigure(LCD_CLK_DIV, value)
     The frequency of segment oscillation is
       Seg Freq = (XTAL-FREQ / 16 ) / 2^(value1 + 1).

     Value1 is a 4-bit value, this gives the following range of possible
     segment frequencies with 26MHz Xtal:
      value = 0x0,  Seg Freq = 812.5 kHz
      value = 0xF,  Seg Freq = 24.8 Hz
     No range-checking is performed on value and the trap always returns zero.
     */
    LCD_CLK_DIV
} lcd_config_key;

#endif /* __LCD_IF_H__ */
