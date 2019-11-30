/****************************************************************************
Copyright (c) 2010 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    display_example.h

DESCRIPTION
    
    
NOTES
   
*/

#include <stdlib.h>
#include <panic.h>
#include <print.h>
#include <stream.h>
#include <app/vm/vm_if.h>
#include <message.h> 
#include <string.h>




#include "display_plugin_if.h"  
#include "display_example_if.h"  
#include "display_example_plugin.h"
#include "display_example.h"



typedef struct display_Tag
{

   uint16 volume_range;
   
   Task app_task;

}DISPLAY_EXAMPLE_t ;

/* The task instance pointer*/
static DISPLAY_EXAMPLE_t * DISPLAY_EXAMPLE = NULL;

/* display plugin functions*/
void DisplayExamplePluginInit(  DisplayExamplePluginTaskdata *task, 
                                Task app_task)
{
    UNUSED(task);

    if (DISPLAY_EXAMPLE)
          Panic();
   
    DISPLAY_EXAMPLE = PanicUnlessNew ( DISPLAY_EXAMPLE_t ); 
   
   /* initalise the display */
   DISPLAY_EXAMPLE->app_task = app_task;
   
   /* indicate success */
   {
        MAKE_DISPLAY_MESSAGE(DISPLAY_PLUGIN_INIT_IND);
        PRINT(("DISPLAYEX: Send DISPLAY_PLUGIN_INIT_IND\n"));
        message->result = TRUE;
        MessageSend(DISPLAY_EXAMPLE->app_task, DISPLAY_PLUGIN_INIT_IND, message);
    }
   
}

void DisplayExamplePluginSetState( DisplayExamplePluginTaskdata *task, bool state ) 
{
    UNUSED(task);

    PRINT(("DISP: State %u:\n", state));


}

void DisplayExamplePluginSetText( DisplayExamplePluginTaskdata *task, char* text, uint8 text_length, uint8 line, uint8 scroll, bool flash, uint16 scroll_update, uint16 scroll_pause, uint16 display_time ) 
{
    /* configure clear messages so they're active on the correct line number */
    /* NB. The application numbers lines as 1 and 2, but the plugin numbers them as 0 and 1. */
    uint8   active_line_clear_msg = (line==1)?DISP_EX_TEXTLN0_CLEAR_INT:DISP_EX_TEXTLN1_CLEAR_INT;
    uint8   active_line_scroll_msg;
    UNUSED(flash);

    PRINT(("DISP: (L%u)(L%u)(F%u)(S%u,%u)(D:%u): ", line,text_length, flash,scroll_update,scroll_pause, display_time));
    PRINT(("%-.*s", text_length, text));
    PRINT(("\n"));

    /* cancel any clear messages */
    MessageCancelAll( (TaskData*)task , active_line_clear_msg);

#ifdef ENABLE_SCROLL
    /* configure scroll messages so they're active on the correct line number */
    active_line_scroll_msg = (line==1)?DISP_EX_SCROLLLN0_TEXT_INT:DISP_EX_SCROLLLN1_TEXT_INT;
    /* cancel any scrolling messages */
    MessageCancelAll( (TaskData*)task , active_line_scroll_msg);

    switch(scroll)
    {
        case DISPLAY_TEXT_SCROLL_SCROLL:
        case DISPLAY_TEXT_SCROLL_BOUNCE:
        {
            if (text_length > display_example_plugin.length)
            {
                /* send message to update display for scroll */
                MAKE_DISPLAY_MESSAGE_WITH_LEN(DispExScrollMessage, text_length);

                message->text_length = text_length;
                message->text_pos = 1;
                message->line = line;
                message->bounce = (scroll == DISPLAY_TEXT_SCROLL_BOUNCE);
                message->scroll_update = scroll_update;
                memmove(message->text,text,text_length) ;

                /* if scrolling, check if a longer pause is required to start scrolling */
                MessageSendLater((TaskData*)task, active_line_scroll_msg, message, scroll_pause?scroll_pause:scroll_update);
            }
        }
        break; 
        case DISPLAY_TEXT_SCROLL_STATIC:
        default:
            ; /* do nothing */
    }
#else
    UNUSED(scroll);
    UNUSED(scroll_pause);
    UNUSED(scroll_update);
    UNUSED(active_line_scroll_msg);
#endif

    /* Draw text here (Line:line, Text Length: text_length, First Character: text[0]) */

    /* check if this display is for a limited time only */
    if(display_time)
    {
        MAKE_DISPLAY_MESSAGE(DispExClearLineMessage);
        message->line = line;
        MessageSendLater((TaskData*)task, active_line_clear_msg, message, D_SEC(display_time));
    }
}


void DisplayExamplePluginSetVolume( DisplayExamplePluginTaskdata *task, uint16 volume ) 
{
    UNUSED(task);

    PRINT(("DISP: Volume %u:\n", volume));

    /* Store volume and update display */
}

void DisplayExamplePluginSetIcon( DisplayExamplePluginTaskdata *task, uint8 icon, bool state ) 
{
    UNUSED(task);

    PRINT(("DISP: Icon %u(%u):\n", icon, state));

    /* Store icon and state and update display */
}

void DisplayExamplePluginSetBattery( DisplayExamplePluginTaskdata *task, uint8 battery_level ) 
{
    UNUSED(task);

    PRINT(("DISP: Batt %u:\n", battery_level));

    /* Store batery_level and update display */
}

#ifdef ENABLE_SCROLL
void DisplayExamplePluginScrollText( DisplayExamplePluginTaskdata *task, DispExScrollMessage_T * dispscrmsg)
{
    /* configure scroll messages so they're active on the correct line number */
    const uint8 active_line_scroll_msg = (dispscrmsg->line==1)?DISP_EX_SCROLLLN0_TEXT_INT:DISP_EX_SCROLLLN1_TEXT_INT;
    uint8 text_pos;

    PRINT(("DISP: Scroll line  %u, %u: ", dispscrmsg->line , dispscrmsg->scroll_update));
    PRINT((dispscrmsg->text));
    PRINT(("\n"));

    {
            /* send message to update display for scroll */
        MAKE_DISPLAY_MESSAGE_WITH_LEN(DispExScrollMessage, dispscrmsg->text_length);

        message->text_length = dispscrmsg->text_length;
        message->text_pos = dispscrmsg->text_pos+1;
        if (dispscrmsg->text_pos<0)
            text_pos = -dispscrmsg->text_pos;
        else
        {
            text_pos = dispscrmsg->text_pos;
            if ((uint8)message->text_pos > message->text_length - display_example_plugin.length)
            {
                if (dispscrmsg->bounce)
                    message->text_pos = 1-dispscrmsg->text_pos;    /* Go back one character, but set negative as now text is going in other direction */
                else
                    message->text_pos = 0;    /* Go back to the beginning */
            }
        }
        message->line = dispscrmsg->line;
        message->bounce = dispscrmsg->bounce;
        message->scroll_update = dispscrmsg->scroll_update;
        memmove(message->text,dispscrmsg->text,dispscrmsg->text_length) ;

        /* if scrolling, check if a longer pause is required to start scrolling */
        MessageSendLater((TaskData*)task, active_line_scroll_msg, message, message->scroll_update);
    }
    /* Draw text here (Line:dispscrmsg->line, Text Length: display_example_plugin.length, First Character: dispscrmsg->text[text_pos]) */
}
#endif

void DisplayExamplePluginClearText( DisplayExamplePluginTaskdata *task, uint8 line ) 
{
#ifdef ENABLE_SCROLL
    /* configure scroll messages so they're active on the correct line number */
    const uint8 active_line_scroll_msg = (line==1)?DISP_EX_SCROLLLN0_TEXT_INT:DISP_EX_SCROLLLN1_TEXT_INT;
    /* Cancel any scrolling on this line */
    MessageCancelAll( (TaskData*)task , active_line_scroll_msg);
#endif
    UNUSED(task);

    PRINT(("DISP: Clear line (%u)\n", line ));

    /* Clear Line of Display(Line: line) */
}
