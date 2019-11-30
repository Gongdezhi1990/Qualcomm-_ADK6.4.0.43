/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    display_midas_plugin.c
DESCRIPTION
    Interface file for example display plugin

NOTES
*/

#include <stdlib.h>
#include <print.h>

#include "display_plugin_if.h" /* messaging interface*/
#include "display_plugin_midas.h"
#include "display_midas.h"

#ifdef DEBUG_DISPLAY
#include <stdio.h>
#include <panic.h>
#define DISPLAY_DEBUG(X) printf X
#define DISPLAY_PANIC() Panic()
#else
#define DISPLAY_DEBUG(X)
#define DISPLAY_PANIC()
#endif

    /* task message handler*/
static void message_handler (Task task, MessageId id, Message message);

static void handleDisplayMessage ( Task task , MessageId id, Message message );
static void handleInternalDisplayMessage ( Task task , MessageId id, Message message );

/* initialise display plugin with device specific parameters */
const DisplayMidasPluginTaskdata display_plugin_midas = {{message_handler},
                                                                16};    /* length of lines on the display */

/****************************************************************************
DESCRIPTION
    The main display task message handler
*/
static void message_handler ( Task task, MessageId id, Message message ) 
{
    if ( (id >= DISPLAY_DOWNSTREAM_MESSAGE_BASE ) && (id < DISPLAY_DOWNSTREAM_MESSAGE_TOP) )
    {
        handleDisplayMessage (task , id, message ) ;
    }
    else
    {   
        handleInternalDisplayMessage (task , id , message ) ;
    }
}   

/****************************************************************************
DESCRIPTION

    messages from the display library are received here. 
    and converted into function calls to be implemented in the 
    plugin module
*/ 
static void handleDisplayMessage ( Task task , MessageId id, Message message )  
{
    switch (id)
    {
        case DISPLAY_PLUGIN_INIT_MSG:
        {
            DISPLAY_PLUGIN_INIT_MSG_T * init_message = (DISPLAY_PLUGIN_INIT_MSG_T *)message ;
            PRINT(("DISPLAY: DISPLAY_PLUGIN_INIT_MSG\n"));
        #ifdef ENABLE_SCROLL
            *init_message->max_length = 2*display_plugin_midas.length;
        #else
            *init_message->max_length = display_plugin_midas.length;
        #endif
            DisplayMidasPluginInit( (DisplayMidasPluginTaskdata*)task,
                                      init_message->app_task,
                                      init_message->reset_pio,
                                      init_message->backlight_pio); 
        }                       
        break;

        case DISPLAY_PLUGIN_SET_STATE_MSG:
        {
            DISPLAY_PLUGIN_SET_STATE_MSG_T * state_message = (DISPLAY_PLUGIN_SET_STATE_MSG_T *)message ;
            PRINT(("DISPLAY: DISPLAY_PLUGIN_SET_STATE\n"));     
            DisplayMidasPluginSetState((DisplayMidasPluginTaskdata*)task,
                                            state_message->state);
        }
        break;

        case DISPLAY_PLUGIN_SET_STATUS_LOCATION_MSG:
        {
            DISPLAY_PLUGIN_SET_STATUS_LOCATION_MSG_T * status_location_message = (DISPLAY_PLUGIN_SET_STATUS_LOCATION_MSG_T *)message ;
            PRINT(("DISPLAY: DISPLAY_PLUGIN_SET_STATUS_LOCATION_MSG\n"));
            DisplayMidasSetStatusLocation( (DisplayMidasPluginTaskdata*)task,
                                            status_location_message->line,
                                            status_location_message->start) ;
        }
        break;

        case DISPLAY_PLUGIN_SET_BATTERY_MSG:
        {
            DISPLAY_PLUGIN_SET_BATTERY_MSG_T * battery_message = (DISPLAY_PLUGIN_SET_BATTERY_MSG_T *)message ;
            PRINT(("DISPLAY: DISPLAY_PLUGIN_SET_BATTERY_MSG\n"));       
            DisplayMidasPluginSetBattery((DisplayMidasPluginTaskdata*)task,
                                            battery_message->battery_level);
        }
        break;

        case DISPLAY_PLUGIN_SET_TEXT_MSG:
        {
            DISPLAY_PLUGIN_SET_TEXT_MSG_T * text_message = (DISPLAY_PLUGIN_SET_TEXT_MSG_T *)message ;
            PRINT(("DISPLAY: DISPLAY_PLUGIN_SET_TEXT_MSG\n"));      
            DisplayMidasPluginSetText((DisplayMidasPluginTaskdata*)task,
                                        text_message->text,
                                        text_message->text_length,
                                        text_message->line,
                                        text_message->scroll,
                                        text_message->flash_enable,                                     
                                        text_message->scroll_update,
                                        text_message->scroll_pause,
                                        text_message->display_time);    
                                        
        }
        break;

        case DISPLAY_PLUGIN_SET_VOLUME_MSG:
        {
            DISPLAY_PLUGIN_SET_VOLUME_MSG_T * vol_message = (DISPLAY_PLUGIN_SET_VOLUME_MSG_T *)message ;
            PRINT(("DISPLAY: DISPLAY_PLUGIN_SET_VOLUME_MSG\n"));    
            DisplayMidasPluginSetVolume((DisplayMidasPluginTaskdata*)task,
                                           vol_message->volume);
                                          
        }
        break;

        case DISPLAY_PLUGIN_SET_ICON_MSG:
        {
            DISPLAY_PLUGIN_SET_ICON_MSG_T * icon_message = (DISPLAY_PLUGIN_SET_ICON_MSG_T *)message ;
            PRINT(("DISPLAY: DISPLAY_PLUGIN_SET_ICON_MSG\n"));      
            DisplayMidasPluginSetIcon((DisplayMidasPluginTaskdata*)task,
                                               icon_message->icon, icon_message->state);
        }                                           
        break;

        default:
        {
            PRINT(("DISPLAY: unhandled message %x\n", id));
            Panic();
        }
        break ;
    }
    

}


/****************************************************************************
DESCRIPTION
    Internal  messages to the task are handled here
*/ 
static void handleInternalDisplayMessage ( Task task , MessageId id, Message message )  
{
    switch(id)
    {
        case DISP_EX_TEXTLN0_CLEAR_INT:
        case DISP_EX_TEXTLN1_CLEAR_INT:
        {
            DisplayMidasPluginClearText((DisplayMidasPluginTaskdata*)task, ((DispExClearLineMessage_T *) message)->line);
        }
        break;

#ifdef ENABLE_SCROLL
        case DISP_EX_SCROLLLN0_TEXT_INT:
        case DISP_EX_SCROLLLN1_TEXT_INT:
        {
            DispExScrollMessage_T *scrlmsg = (DispExScrollMessage_T *) message;
            DisplayMidasPluginScrollText((DisplayMidasPluginTaskdata*)task, scrlmsg);
        }
        break;
#endif

        case DISP_EX_RESET_COMPLETE:
        {
            DisplayMidasResetComplete((DisplayMidasPluginTaskdata*)task);
        }
        break;

        case DISP_EX_SET_BACKLIGHT:
        {
            DispExBacklightMessage_T * backlight_message = (DispExBacklightMessage_T *)message;
            DisplayMidasPluginSetBacklight((DisplayMidasPluginTaskdata*)task,
                                               backlight_message->backlight_pio,
                                               backlight_message->backlight_status);
        }
        break;

        default:
        break;
    }
}
