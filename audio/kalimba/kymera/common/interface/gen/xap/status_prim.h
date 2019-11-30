/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2018
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef STATUS_PRIM_H
#define STATUS_PRIM_H


/*******************************************************************************

  NAME
    STATUS_KYMERA

  DESCRIPTION
    Status IDs of the response message.

 VALUES
    STATUS_OK                    - Everything is OK
    STATUS_CMD_FAILED            - Execution of the command failed for some
                                   reason
    STATUS_UNKNOWN_COMMAND       - Unknown signal ID
    STATUS_UNSUPPORTED_COMMAND   - Command is not supported
    STATUS_INVALID_CMD_PARAMS    - Invalid parameters in the command
    STATUS_INVALID_CMD_LENGTH    - Length of the message is wrong
    STATUS_INVALID_CONNECTION_ID - Something bonkers in the connection IDe

*******************************************************************************/
typedef enum
{
    STATUS_OK  = 0x0000,
    STATUS_CMD_FAILED  = 0x1000,
    STATUS_UNKNOWN_COMMAND = 0x1001,
    STATUS_UNSUPPORTED_COMMAND = 0x1002,
    STATUS_INVALID_CMD_PARAMS = 0x1003,
    STATUS_INVALID_CMD_LENGTH = 0x1004,
    STATUS_INVALID_CONNECTION_ID = 0x1005
} STATUS_KYMERA;


#define STATUS_PRIM_ANY_SIZE 1


#endif /* STATUS_PRIM_H */

