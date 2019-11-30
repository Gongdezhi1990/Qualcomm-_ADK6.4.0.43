/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_charger.h
\brief      Header file for Charger monitoring
*/

#ifndef _AV_HEADSET_CHARGER_H_
#define _AV_HEADSET_CHARGER_H_

#include <charger.h>
#include <usb.h>
#include "av_headset.h"

/* Check whether charger detecion is currently happening */
#define appChargerDetectionIsPending() ( (UsbAttachedStatus() == UNKNOWN_STATUS)? TRUE:FALSE )

/*! \brief Messages which may be sent by the charger module. */
typedef enum 
{
    /*! Message to inform client the charger was attached */
    CHARGER_MESSAGE_ATTACHED = CHARGER_MESSAGE_BASE,
    /*! Message to inform client the charger was detached */
    CHARGER_MESSAGE_DETACHED
} chargerMessages;

/*! \brief Reasons the charger is disabled. */
typedef enum
{
    /*! No reason to disable */
    CHARGER_DISABLE_REASON_NONE = 0,
    /*! Temperature outside specified battery charge temperature */
    CHARGER_DISABLE_REASON_EXTREME_TEMPERATURE = 1,
    /*! Timed-out attempting to charge the battery */
    CHARGER_DISABLE_REASON_TIMEOUT = 2,
    /*! Requested by another module */
    CHARGER_DISABLE_REASON_REQUEST = 4,
    /*! No charger detected */
    CHARGER_DISABLE_REASON_NOT_DETECTED = 8,
} chargerDisableReason;

typedef enum
{
    CHARGER_DISCONNECTED,
    CHARGER_CONNECTED,
    CHARGER_CONNECTED_NO_ERROR,    
} chargerConnectionState;

/*! The charger module internal state */
typedef struct
{
    /*! Charger task */
    TaskData task;
    /*! List of client tasks */
    TaskList *client_tasks;
    /*! Set when charger is charging */
    unsigned is_charging:1;
    /*! Set when charger is connected */
    chargerConnectionState is_connected:2;
    /*! The current charger status */
    charger_status status;
    /*! Reasons the charger is disabled (bitfield). */
    chargerDisableReason disable_reason;
} chargerTaskData;

extern void appChargerInit(void);

extern void appChargerForceDisable(void);
extern void appChargerRestoreState(void);

extern bool appChargerCanPowerOff(void);
extern chargerConnectionState appChargerIsConnected(void);

/*! @brief Register a client to receive status messages from the charger module.
    @param client_task The task to register.
    @return TRUE if successfully registered.
            FALSE if registration not sucessfull of if the charger support is not
            compiled into the application.
*/
extern bool appChargerClientRegister(Task client_task);

/*! @brief Unregister a client.
    @param client_task The task to unregister. */
extern void appChargerClientUnregister(Task client_task);


#endif
