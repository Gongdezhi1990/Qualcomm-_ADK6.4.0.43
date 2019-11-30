/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * This is a standard part of the P0/P1 binary interface.  It declares a set of
 * IDs for the IPC App msg primitive, and allows P0 to look up P1's set of
 * registered message handlers to see whether to pass a message across or not.
*/

#ifndef IPC_MSG_HANDLERS_H_
#define IPC_MSG_HANDLERS_H_

/**
 * Enumeration of the various message streams presented to the App.  For each
 * of these there is an API function for registering a "listener" task.  Note
 * that separate message streams for individual sinks are multiplexed on the
 * Sink stream.
 */
typedef enum
{
    IPC_MSG_TYPE_SYSTEM                = 0,
    IPC_MSG_TYPE_PIO                   = 1,
    IPC_MSG_TYPE_VOLTSENSE             = 2,
    IPC_MSG_TYPE_STATUS                = 3,
    IPC_MSG_TYPE_HOST_COMMS            = 4,
    IPC_MSG_TYPE_BLUESTACK             = 5,
    IPC_MSG_TYPE_KALIMBA               = 6,
    IPC_MSG_TYPE_CHARGER               = 7,
    IPC_MSG_TYPE_CAPACITIVE_SENSOR     = 8,
    IPC_MSG_TYPE_ATT                   = 9,
    IPC_MSG_TYPE_INFRARED              = 10,
    IPC_MSG_TYPE_ADC                   = 11,
    IPC_MSG_TYPE_NFC                   = 12,
    IPC_MSG_TYPE_AUDIO                 = 13,
    IPC_MSG_TYPE_BITSERIAL             = 14,
    NUM_IPC_MSG_TYPES
} IPC_MSG_TYPE;


#endif /* IPC_MSG_HANDLERS_H_ */
