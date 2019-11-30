/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */
/*!
    @file display_screens.h
    @brief Interface to simple multiple screen manager for MIDAS displays.

    ========
    Overview
    ========
    An application can define a series of 'screens' to be displayed on the LCD
    display. The library provides a simple API to update and display any screen,
    cycle through screens and display a screen temporarily.
    
    Content for the screens is local to the application and may be modified
    easily in application specific functions, in application context. 
    
    No messaging or message handling required.

    =====
    Usage
    =====
    A display (#display_state) is composed of an ordered list of screens
    (#lcd_screen), each screen is 2 lines, each line 16 characters.

    Define some char arrays to hold the contents of a line on a screen,
    they can be const if they won't change, e.g.

    static const char screen_1_line_1[] = "screen 1 line 1 ";
    static const char screen_1_line_2[] = "screen 1 line 2 ";
    static const char screen_2_line_1[] = "screen 2 line 1 ";
    static char screen_2_line_2[] = "screen 2 line 2 ";

    Define a display e.g.

    lcd_screen example_display[NUM_SCREENS] = {
                                                {screen_1_line_1,
                                                 screen_1_line_2},

                                                {screen_2_line_1,
                                                 screen_2_line_2)
                                              };

    Setup a #display_state structure, which references the screens array,
    along with some simple parameters:
        display_state example_ds;
        example_ds.num_screens = NUM_SCREENS;
        example_ds.current_screen = 0;
        example_ds.reset_to_main_time = RESET_TIME_SECS;
        example_ds.display = example_display;

    Initialise the display with #DisplayScreensInit(&example_ds, RESET_PIO, BACKLIGHT_PIO).

    Switch to any screen:-
        example_ds.current_screen = 1;
        DisplayScreensUpdate(&example_ds);
        
    Temporarily show a different screen (will revert after RESET_TIME_SECS)
    to show transient events:-
        example_ds.current_screen = 1;
        DisplayScreensUpdate(&example_ds);
        DisplayScreensResetToMain(&example_ds);
        
    Rotate through screens in the display (will loop back through first),
    can be tied to button/PIO or any other event:-
        DisplayScreensRotate(&example_ds);

    Modify and update a screen line:-
        sprintf(screen_2_line_1, "Error Num:%d", errno);
        example_ds.current_screen = 1;
        DisplayScreensUpdate(&example_ds);

*/
#ifndef _DISPLAY_SCREENS_H
#define _DISPLAY_SCREENS_H

/*! @brief Definition of a single screen on the display. */
typedef struct 
{
    /*! Top line of the screen. */
    const char* line_top;

    /*! Botton line of the screen. */
    const char* line_bottom;
} lcd_screen;

/*! @brief State of the display. */
typedef struct
{
    /*! Task for handling messages from core display library and
        internal timer messages. */
    TaskData    display_task;

    /*! Number of lcd_screen structures in the display. */
    uint16      num_screens;

    /*! Current lcd_screen being displayed. */
    uint16      current_screen;

    /*! Time in seconds after which to switch screen back to main. */
    uint16      reset_to_main_time;

    /*! Ordered array of lcd_screen's which can be displayed. */
    lcd_screen* display;
} display_state;

/*! @brief Initialise the display hardware and message handling.

    @param display [IN] Pointer to the display state.
    @param reset_pio PIO to reset the display hardware.
    @param backlight_pio PIO to control the display backlight.
 */
void DisplayScreensInit(display_state* display, uint8 reset_pio, uint8 backlight_pio);

/*! @brief Refresh the display.

    @param display [IN] Pointer to the display state.
 */
void DisplayScreensUpdate(display_state* display);

/*! @brief Cycle to next screen in the display.

    @param display [IN] Pointer to the display state.
 */
void DisplayScreensRotate(display_state* display);

/*! @brief Start a timer to reset the display to the 0th lcd_screen.

    @param display [IN] Pointer to the display state.
 */
void DisplayScreensStartResetToMainTimer(display_state* display);

#endif /* _DISPLAY_SCREENS_H */
