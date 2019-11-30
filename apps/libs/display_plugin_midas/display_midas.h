/****************************************************************************
Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    display_midas.h

DESCRIPTION


NOTES

*/

#ifndef _DISPLAY_MIDAS_H_
#define _DISPLAY_MIDAS_H_

/* display plugin functions*/
void DisplayMidasPluginInit( DisplayMidasPluginTaskdata *task, Task app_task, uint8 reset_pio, uint8 backlight_pio ) ;
void DisplayMidasPluginSetState( DisplayMidasPluginTaskdata *task, bool state ) ;
void DisplayMidasSetStatusLocation( DisplayMidasPluginTaskdata *task, uint8 line, bool start) ;
void DisplayMidasPluginSetText( DisplayMidasPluginTaskdata *task, char* text, uint8 line, uint8 text_length, uint8 scroll, bool flash, uint16 scroll_update, uint16 scroll_pause, uint16 display_time ) ;
void DisplayMidasPluginSetVolume( DisplayMidasPluginTaskdata *task, uint16 volume ) ;
void DisplayMidasPluginSetIcon( DisplayMidasPluginTaskdata *task, uint8 icon, bool state ) ;
void DisplayMidasPluginSetBattery( DisplayMidasPluginTaskdata *task, uint8 battery_level ) ;

/*internal plugin message functions*/
#ifdef ENABLE_SCROLL
void DisplayMidasPluginScrollText( DisplayMidasPluginTaskdata *task, DispExScrollMessage_T * dispscrmsg ) ;
#endif
void DisplayMidasPluginClearText( DisplayMidasPluginTaskdata *task, uint8 line ) ;
void DisplayMidasResetComplete( DisplayMidasPluginTaskdata *task ) ;
void DisplayMidasPluginSetBacklight( DisplayMidasPluginTaskdata *task, uint8 backlight_pio, bool backlight_status );

#endif /*_DISPLAY_MIDAS_H_*/
