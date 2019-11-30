/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2018

\file

    Functions that provide access to the audio hardware on the
    development board.

*/

#ifndef DEV_BOARD_AUDIO_H
#define DEV_BOARD_AUDIO_H

#include "types.h"

/*!
 * \brief Initialise audio hardware
 *
 * This function is used to perform any additional
 * board-level audio hardware initialisation.
 */
extern void dev_board_audio_initialise(void);


#endif // DEV_BOARD_AUDIO_H
