/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/** \file
 *
 *  This is the main public project header for the \c ipc LUT library.
 *
 */
/****************************************************************************
Include Files
*/

#ifndef IPC_CONSTANTS_H
#define IPC_CONSTANTS_H

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Public Constant and macros
*/

/*
 * IPC Signal priorities and signal fifo size
 */

/* Four priorities defined in the architecture (CS-336170-DD) */
#define IPC_NUM_PRIORITIES           4

/*
 * The signal fifo should be able to take one of each possible signal.
 * The number of signals is variable, specified as a parameter in
 * 'ipc_setup_comms'. Signals relate to data channels, the number of
 * ipc signals required equals at most the max number of data channels.
 */
#define IPC_NUM_FIFO_ELEMENTS        IPC_MAX_DATA_CHANNELS

/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Declarations
*/

#endif /* IPC_CONSTANTS_H */

