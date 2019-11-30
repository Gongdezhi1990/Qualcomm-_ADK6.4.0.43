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

#ifndef IPC_STATUS_H
#define IPC_STATUS_H

/****************************************************************************
Public Type Declarations
*/

typedef enum IPC_STATUS
{
    IPC_SUCCESS               = 0,
    IPC_ERROR_BUFFER_FULL     = 1,
    IPC_ERROR_WRITE_ERROR     = 2,
    IPC_ERROR_READ_ERROR      = 3,
    IPC_ERROR_VERSION_DIFF    = 4,
    IPC_ERROR_INVALID_PROCID  = 5,
    IPC_ERROR_KEYNOTFOUND     = 6,
    IPC_ERROR_LUTNOTFOUND     = 7,
    IPC_ERROR_MSGBLKNOTFOUND  = 8,
    IPC_ERROR_SIGBLKNOTFOUND  = 9,
    IPC_ERROR_NOACCESSP0ONLY  = 10,
    IPC_ERROR_NOTSUPPORTED    = 11,
    IPC_ERROR_FAILED          = 12,
    IPC_ERROR_MSG_TOO_LARGE   = 13,
    IPC_ERROR_MSGREADFAILED   = 14,
    IPC_ERROR_PARAM_NULL      = 15,
    IPC_ERROR_INVALID_PARAM   = 16,
    IPC_ERROR_SIGBLOCK_NULL   = 17,
    IPC_ERROR_NOFREECHANNEL   = 18,
    IPC_ERROR_NOMEMORY        = 19,
    IPC_SIGNAL_BUSY           = 20,
    IPC_ERROR_PXNOTSTARTED    = 21,
    IPC_ERROR_TIMEOUT         = 22,

} IPC_STATUS;

/****************************************************************************
Public Constant and macros
*/

/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Declarations
*/

#endif /* IPC_STATUS_H */

