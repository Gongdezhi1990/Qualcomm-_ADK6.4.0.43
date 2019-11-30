/****************************************************************************
Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    display.h

DESCRIPTION
    header file for the display library
*/

/*!
@file   display.h
@brief  Header file for the display library.
    This defines the Application Programming interface to the display library.
    
    i.e. the interface between the VM application and the display library.
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <message.h>
#include "display_plugin_if.h"

/*!
    @brief Connects initilaises the display plugin params.  Underlying plugin should
		also use this message to do any specific initialisation required.
    
    @param display_plugin The display plugin to use
    @param app_task the application task
    @param reset_pio the reset PIO of the display
    @param backlight_pio backlight PIO of the display
	
      
*/
bool DisplayInit(Task display_plugin,
                 Task app_task,
                 uint8 reset_pio,
                 uint8 backlight_pio);


/*!
    @brief Gets the DISPLAY_BUSY flag

    @return The current state of the displaybusy flag
*/
bool IsDisplayBusy(void);


/*!
    @brief Update the current DISPLAY_BUSY pointer

    @param task The current display task
*/
void SetDisplayBusy(TaskData* task);


/*!
    @brief 	update the state of the display
    @param 	state The on/off state of display

*/
void DisplaySetState( bool state );

/*!
    @brief 	set the location of the status information
    @param 	line The line on the display where the status information should be shown,
        specify 0 if no status information should be displayed
    @param 	start If true the information is put at the start of the specified line,
        if false then it is put at the end

*/
void DisplaySetStatusLocation( uint8 line, bool start );

/*!
    @brief 	update the text line on the display

*/
void DisplaySetText( const char* text,
                     uint8 txtlen,
                     uint8 line,
                     bool  scroll,
                     uint16 scroll_update,
                     uint16 scroll_pause,
                     bool  flash,
                     uint16  display_time );

/*!
    @brief 	update the volume level on the display
    @param 	volume

*/
void DisplaySetVolume( uint16 volume  );

/*!
	@brief 	update an icon status on the display
    @param 	icon The icon id
    @param 	state state
	
*/
void DisplaySetIcon( uint8 icon, bool state );

/*!
	@brief 	update the battery level on the display
    @param 	battery_level The battery level
	
*/
void DisplaySetBatteryLevel( uint8 battery_level );


#endif /*_DISPLAY_H_*/
