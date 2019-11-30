/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */
/*!
    @file display_screens.c
    @brief Simple multiple screen manager for MIDAS displays.
*/

#include "display_screens.h"

#include <display.h>
#include <display_plugin_if.h>
#include <display_plugin_midas.h>

/*! Upper line of the display index. */
#define LINE_TOP_INDEX          1
/*! Lower line of the display index. */
#define LINE_BOTTOM_INDEX       2

/*! Internal message definitions. */
enum
{
    /*! Timer message to reset display to 0th lcd_screen. */
    APP_DISPLAY_MSG_RESET_DISPLAY,
};

/*! Forward declarations. */
void handleDisplayPluginMessage( Task task, MessageId id, Message message );

/*! @brief Initialise the display hardware and message handling.
 */
void DisplayScreensInit(display_state* display, uint8 reset_pio, uint8 backlight_pio)
{
    display->display_task.handler = handleDisplayPluginMessage;
    DisplayInit((TaskData*)&display_plugin_midas,
                &display->display_task, reset_pio, backlight_pio);
}

/*! @brief Start a timer to reset the display to the 0th lcd_screen.
 */
void DisplayScreensStartResetToMainTimer(display_state* display)
{
    MessageCancelAll(&display->display_task, APP_DISPLAY_MSG_RESET_DISPLAY);
    MessageSendLater(&display->display_task, APP_DISPLAY_MSG_RESET_DISPLAY,
                     NULL, D_SEC(display->reset_to_main_time));
}

/*! @brief Set the display back to the 0th lcd_screen. */
static void DisplayScreensResetToMain(display_state* display)
{
    display->current_screen = 0;
    DisplayScreensUpdate(display);
}

/*! @brief Refresh the display.
 */
void DisplayScreensUpdate(display_state* display)
{
    DisplaySetText(display->display[display->current_screen].line_top,
                   strlen(display->display[display->current_screen].line_top),
                   LINE_TOP_INDEX,
                   DISPLAY_TEXT_SCROLL_SCROLL, 1000, 2000, FALSE, 0);
    DisplaySetText(display->display[display->current_screen].line_bottom,
                   strlen(display->display[display->current_screen].line_bottom),
                   LINE_BOTTOM_INDEX,
                   DISPLAY_TEXT_SCROLL_SCROLL, 1000, 2000, FALSE, 0);
}

/*! @brief Cycle to next screen in the display.
 */
void DisplayScreensRotate(display_state* display)
{
    /* explicit rotate command, cancel any pending reset to main timer */
    MessageCancelAll(&display->display_task, APP_DISPLAY_MSG_RESET_DISPLAY);

    display->current_screen = (display->current_screen += 1) % display->num_screens;
    DisplayScreensUpdate(display);
}

/* Handle any display plugin messages */
void handleDisplayPluginMessage( Task task, MessageId id, Message message )
{
    display_state* display = (display_state*)task;

    switch (id)
    {
        case DISPLAY_PLUGIN_INIT_IND:
        {
            DISPLAY_PLUGIN_INIT_IND_T *m = (DISPLAY_PLUGIN_INIT_IND_T *) message;
            if (m->result)
            {
                DisplaySetState(TRUE);
                DisplayScreensUpdate(display);
            }
        }
        break;

        case APP_DISPLAY_MSG_RESET_DISPLAY:
        {
            DisplayScreensResetToMain(display);
        }
        break;

        default:
        break;
    }
}

