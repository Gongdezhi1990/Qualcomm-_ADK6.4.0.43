/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016 - 2018

\file

    Functions that provide access to the buttons on the
    development board.

    The support is hard coded to support SW1 to SW8, calling the
    function supplied by the application for each button that is
    pressed.
    - Multiple button presses are ignored.
    - There is no facility for handling types of press such as
    long, double

    An application wishing to use this should call
    dev_board_initialise_buttons() once during initialisation.

*/

#ifndef DEV_BOARD_BUTTONS_H
#define DEV_BOARD_BUTTONS_H

#include "types.h"

typedef void (*dev_board_button_callback)(uint8 button);

/*!
 * \brief Initialise button handling for the board.
 *
 * This function should be called once during the application
 * initialisation of the system. It overrides the PioTask handler,
 * so if the application requires additional PIO handling the
 * application will also need to handle these messages.
 *
 * The function will error with a Panic() if it is called
 * multiple times or if no callback function, cb, is supplied.
 *
 * \param cb  The application call back function
 */
extern void dev_board_initialise_buttons(dev_board_button_callback cb);


#endif // DEV_BOARD_BUTTONS_H
