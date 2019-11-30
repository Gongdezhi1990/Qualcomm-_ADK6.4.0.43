/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2017
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef SYSTEM_KEYS_PRIM_H
#define SYSTEM_KEYS_PRIM_H


/*******************************************************************************

  NAME
    SYSTEM_KEYS_MSG

  DESCRIPTION
    Describes a common set of keys for system messages.

 VALUES
    SHUTDOWN_PREPARE - Used for the acknowledgement of an imminent shutdown.
    BUILD_ID         - Used for getting the build ID integer.
    BUILD_ID_STRING  - Used for getting the build ID string.
    STREAM_RATE      - Used for getting and setting the stream rate.
    PANIC_ON_FAULT   - When set to TRUE, the firmware calls panic() rather than
                       fault(). See PanicOnFault MIB entry.
    PROFILER_ENABLE  - Enables/Disables the built-in profiler.
    KICK_PERIOD      - Used for getting and setting system kick period. This
                       should only be set on system startup. Changing the kick
                       period with a running processing graph may have
                       unpredictable results

*******************************************************************************/
typedef enum
{
    SYSTEM_KEYS_MSG_SHUTDOWN_PREPARE = 0x0001,
    SYSTEM_KEYS_MSG_BUILD_ID = 0x0002,
    SYSTEM_KEYS_MSG_BUILD_ID_STRING = 0x0003,
    SYSTEM_KEYS_MSG_STREAM_RATE = 0x0004,
    SYSTEM_KEYS_MSG_PANIC_ON_FAULT = 0x0005,
    SYSTEM_KEYS_MSG_PROFILER_ENABLE = 0x0006,
    SYSTEM_KEYS_MSG_KICK_PERIOD = 0x0007
} SYSTEM_KEYS_MSG;
/*******************************************************************************

  NAME
    SYSTEM_KEYS_UNSOL_MSG

  DESCRIPTION
    Describes a common set of keys for unsolicited system messages.

 VALUES
    UNACHIEVABLE_LATENCY - Unsolicited message sent by AudioSS using
                           time-to-play when a packet is late for a number of
                           consecutive kicks. Note. the value here was hardcoded
                           so that it corresponds to the above SYSTEM_KEYS_MSG
                           that will never be used as unsolicited.
    SHUTDOWN_COMPLETE    - Unsolicited message sent by PS in the case of system
                           shutdown.

*******************************************************************************/
typedef enum
{
    SYSTEM_KEYS_UNSOL_MSG_UNACHIEVABLE_LATENCY = 0x0002,
    SYSTEM_KEYS_UNSOL_MSG_SHUTDOWN_COMPLETE = SYSTEM_KEYS_MSG_SHUTDOWN_PREPARE
} SYSTEM_KEYS_UNSOL_MSG;


#define SYSTEM_KEYS_PRIM_ANY_SIZE 1


#endif /* SYSTEM_KEYS_PRIM_H */

