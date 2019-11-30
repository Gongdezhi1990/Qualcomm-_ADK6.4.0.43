/****************************************************************************
Copyright (c) 2010 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    display_example.h

DESCRIPTION
    
    
NOTES
   
*/

#ifndef _DISPLAY_EXAMPLE_H_
#define _DISPLAY_EXAMPLE_H_

/* display plugin functions*/
void DisplayExamplePluginInit( DisplayExamplePluginTaskdata *task, Task app_task ) ;
void DisplayExamplePluginSetState( DisplayExamplePluginTaskdata *task, bool state ) ;
void DisplayExamplePluginSetText( DisplayExamplePluginTaskdata *task, char* text, uint8 line, uint8 text_length, uint8 scroll, bool flash, uint16 scroll_update, uint16 scroll_pause, uint16 display_time ) ;
void DisplayExamplePluginSetVolume( DisplayExamplePluginTaskdata *task, uint16 volume ) ;
void DisplayExamplePluginSetIcon( DisplayExamplePluginTaskdata *task, uint8 icon, bool state ) ;
void DisplayExamplePluginSetBattery( DisplayExamplePluginTaskdata *task, uint8 battery_level ) ;

/*internal plugin message functions*/
#ifdef ENABLE_SCROLL
void DisplayExamplePluginScrollText( DisplayExamplePluginTaskdata *task, DispExScrollMessage_T * dispscrmsg ) ;
#endif
void DisplayExamplePluginClearText( DisplayExamplePluginTaskdata *task, uint8 line ) ;

#endif /*_DISPLAY_EXAMPLE_H_*/
