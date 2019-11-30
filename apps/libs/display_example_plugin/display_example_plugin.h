/****************************************************************************
Copyright (c) 2010 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
   csr_common_example_plugin.h

DESCRIPTION
    
    
NOTES
   
*/
#ifndef _DISPLAY_EXAMPLE_PLUGIN_H_
#define _DISPLAY_EXAMPLE_PLUGIN_H_

#include <message.h> 



/*!  display example plugin
   This is an example display plugin that can be used with the display library.
*/

typedef struct
{
   TaskData   data;
   
		/*! The volume range, 0 indexed */
    unsigned         volume_range:4 ;	
		/*! lines of text on the display */
    unsigned         lines:4 ;	
		/*! length of lines of text on the display */
    uint8            length ;

}DisplayExamplePluginTaskdata;

extern const DisplayExamplePluginTaskdata display_example_plugin;

/* Plungin internal messages */
typedef enum DisplayExIntMessage
{
    DISP_EX_TEXTLN0_CLEAR_INT = 0x0001,
    DISP_EX_TEXTLN1_CLEAR_INT
#ifdef ENABLE_SCROLL
    ,
    DISP_EX_SCROLLLN0_TEXT_INT,
    DISP_EX_SCROLLLN1_TEXT_INT
#endif
} DISPLAYEX_INT_MESSAGE;


    /*the  message to scroll text*/
typedef struct 
{
    uint8 line ;
    uint16 scroll_update ;
    bool bounce;
        /*! Absolute value of text_pos is the position to draw the text next,
         *  if text_pos<0 text is scrolling backwards
         *  otherwise text is scrolling forwards */
    int8 text_pos;

        /*! length of the text to display*/
    uint8			text_length;
        /*! text to display */
    char			text[1];
} DispExScrollMessage_T;

    /*the  message to clear text line*/
typedef struct 
{
    uint8 line;
} DispExClearLineMessage_T;


#endif /* _DISPLAY_EXAMPLE_PLUGIN_H_ */
