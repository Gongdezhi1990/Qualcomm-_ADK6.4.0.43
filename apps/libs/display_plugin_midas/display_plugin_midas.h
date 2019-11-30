/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
   display_plugin_midas.h

DESCRIPTION


NOTES

*/
#ifndef _DISPLAY_MIDAS_PLUGIN_H_
#define _DISPLAY_MIDAS_PLUGIN_H_

#include <message.h>

/*!  Midas Display Plugin
    This is the display plugin for the Midas MCCOG21605C6W-BNMLWI display
    that should be used with the display library for Crescendo.
*/

typedef struct
{
    TaskData   data;

    /*! length of lines of text on the display */
    uint8            length ;

}DisplayMidasPluginTaskdata;

extern const DisplayMidasPluginTaskdata display_plugin_midas;

/* Plugin internal messages */
typedef enum DisplayExIntMessage
{
    DISP_EX_TEXTLN0_CLEAR_INT = 0x0001,
    DISP_EX_TEXTLN1_CLEAR_INT,
#ifdef ENABLE_SCROLL
    DISP_EX_SCROLLLN0_TEXT_INT,
    DISP_EX_SCROLLLN1_TEXT_INT,
#endif
    DISP_EX_RESET_COMPLETE,
    DISP_EX_SET_BACKLIGHT
} DISPLAYEX_INT_MESSAGE;

    /*the  message to scroll text*/
typedef struct 
{
    uint8 line ;
    uint16 scroll_pause ;
    uint16 scroll_update ;
    bool bounce;
    int8 text_pos;

        /*!length of the text to display*/
    uint8 text_length;
        /*! text to display */
    char text[1];

} DispExScrollMessage_T;

    /*the  message to clear text line*/
typedef struct 
{
    uint8 line;
} DispExClearLineMessage_T;

    /*the  message to set the Backlight ON/OFF*/
typedef struct 
{
    uint8 backlight_pio;
    bool  backlight_status;
} DispExBacklightMessage_T;

#endif /* _DISPLAY_MIDAS_PLUGIN_H_ */
