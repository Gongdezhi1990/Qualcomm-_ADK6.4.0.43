/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_power.h
\brief	    Header file for the Power Management.
*/

#ifndef __AV_HEADSET_POWER_H
#define __AV_HEADSET_POWER_H

#include "message.h"
#include "av_headset_tasklist.h"

/*
@startuml
    POWER_STATE_INIT : Initialising power, shutdown if battery is too low
    POWER_STATE_OK : No reason to sleep / shutdown
    POWER_STATE_TERMINATING_CLIENTS_NOTIFIED : POWER_SHUTDOWN_PREPARE_IND sent to clients on entry
    POWER_STATE_TERMINATING_CLIENTS_RESPONDED : Powering off
    POWER_STATE_SOPORIFIC_CLIENTS_NOTIFIED : POWER_SLEEP_PREPARE_IND sent to clients on entry
    POWER_STATE_SOPORIFIC_CLIENTS_RESPONDED : Going to sleep

    POWER_STATE_INIT #Pink --> POWER_STATE_OK : Battery ok or charging
    POWER_STATE_OK #LightGreen --> POWER_STATE_TERMINATING_CLIENTS_NOTIFIED : Power event (e.g. low battery)
    POWER_STATE_TERMINATING_CLIENTS_NOTIFIED --> POWER_STATE_TERMINATING_CLIENTS_RESPONDED : All clients responded appPowerShutdownPrepareResponse()
    POWER_STATE_TERMINATING_CLIENTS_RESPONDED --> POWER_STATE_OK : Power event whilst terminating (e.g. charging)

    POWER_STATE_OK --> POWER_STATE_SOPORIFIC_CLIENTS_NOTIFIED : All clients allowing sleep
    POWER_STATE_SOPORIFIC_CLIENTS_NOTIFIED --> POWER_STATE_SOPORIFIC_CLIENTS_RESPONDED : All clients responded appPowerSleepPrepareResponse()
    POWER_STATE_SOPORIFIC_CLIENTS_RESPONDED --> POWER_STATE_OK : Power event whilst soporific (e.g. charging)

@enduml
*/
typedef enum
{
    POWER_STATE_INIT,
    POWER_STATE_OK,
    POWER_STATE_TERMINATING_CLIENTS_NOTIFIED,
    POWER_STATE_TERMINATING_CLIENTS_RESPONDED,
    POWER_STATE_SOPORIFIC_CLIENTS_NOTIFIED,
    POWER_STATE_SOPORIFIC_CLIENTS_RESPONDED,
} powerState;

/*! @brief Power control module state. */
typedef struct
{
    TaskData task;                /*!< Task for Power control messages */
    TaskList *clients;            /*!< List of client tasks */
    bool allow_dormant;           /*!< Flag that can be modified during testing to disable dormant */
    uint32 performance_req_count; /*!< Counts the number of requestors for VM_PERFORMANCE profile */
    powerState state;             /*!< The state */
    uint16 lock;                  /*!< Module's message lock */
    bool user_initiated_shutdown; /*!< Flag that a shutdown was user initiated */
} powerTaskData;

/*! Power messages. */
enum powerMessages
{
    /*! Message indicating power module initialisation is complete. */
    APP_POWER_INIT_CFM = EB_POWER_MESSAGE_BASE,
    /*! Message indicating power module clients should prepare to sleep.
        In response, clients should call #appPowerSleepPrepareResponse() when
        prepared to sleep. */
    APP_POWER_SLEEP_PREPARE_IND,
    /*! Message indicating that an external event caused power to cancel sleep.
        Will be sent to clients after all clients have responded to
        #POWER_SLEEP_PREPARE_IND. */
    APP_POWER_SLEEP_CANCELLED_IND,
    /*! Message indicating power module clients shoud prepare to shutdown.
        In response, clients should all #appPowerShutdownPrepareResponse() when
        prepared to shutdown. No response required. */
    APP_POWER_SHUTDOWN_PREPARE_IND,
    /*! Message indicating that an external event caused power to cancel shutdown.
        Will be sent to clients after all clients have responded to
        #POWER_SHUTDOWN_PREPARE_IND. No response required. */
    APP_POWER_SHUTDOWN_CANCELLED_IND,
};

/*! \brief The client's response to #POWER_SHUTDOWN_PREPARE_IND.
           This informs power that the client is prepared to shutdown.
    \param task The client task.
*/
extern void appPowerShutdownPrepareResponse(Task task);

/*! \brief The client's response to #POWER_SLEEP_PREPARE_IND.
           This informs power that the client is prepared to sleep.
    \param task The client task.
*/
extern void appPowerSleepPrepareResponse(Task task);

/*! \brief Request to power off device
    \return TRUE if the device will now attempt to power off or if the device is
            currently powering off. FALSE if the device cannot currently power
            off (for instance, because the charger is attached, or because
            the device is currently entering sleep).

    If the device can power off, it will inform its clients and wait for a
    response from each (e.g. to allow them to perform tasks related to shutting
    down). After all responses are received, the device will shut down.

    It is possible, whilst waiting for the clients to respond, that the device
    state will change and it will be unable to shutdown (e.g. if a charger is
    attached). In this case after receiving all reponses, the client will send
    a #POWER_SHUTDOWN_CANCELLED_IND to clients informing them that shutdown was
    cancelled.
*/
extern bool appPowerOffRequest(void);

/*! \brief Power on device.

    This function is called to request powering on the device.  Turning on the
    power supply is handled by the state machine.
*/
extern void appPowerOn(void);

/*! \brief Reboot device.

    This function is called when the power-off watchdog has expired, this
    means we have failed to shutdown after 10 seconds.

    We should now force a reboot.
*/
extern void appPowerReboot(void);

/*! \brief Initialise power control task

    Called at start up to initialise the data for power control.
*/
extern void appPowerInit(void);

/*! @brief Register to receive power notifications.
    @param task The client's task.
*/
extern void appPowerClientRegister(Task task);

/*! @brief Unregister a task from receiving power notifications.
    @param task The client task to unregister.
    Silently ignores unregister requests for a task not previously registered
*/
extern void appPowerClientUnregister(Task task);

/*! @brief Inform power that sleep is allowed.
    @param task The client's task, previously registered with #appPowerClientRegister.
    If all power's clients allow sleep, power will sleep.
*/
extern void appPowerClientAllowSleep(Task task);

/*! @brief Inform power that sleep is prohibited.
    @param task The client's task, previously registered with #appPowerClientRegister.
    If any of power's client prohibit sleep, power will not sleep.
*/
extern void appPowerClientProhibitSleep(Task task);

/*! \brief Request the VM runs in VM_PERFORMANCE mode.
    \note The module counts the number of requests to run in VM_PERFORMANCE and
          only reverts to run in VM_BALANCED mode when no VM_PERFORMANCE
          requestors remain.
*/
extern void appPowerPerformanceProfileRequest(void);

/*! \brief Relinquish request for the VM to run in VM_PERFORMANCE mode.
    \note The module counts the number of requests to run in VM_PERFORMANCE and
          only reverts to run in VM_BALANCED mode when no VM_PERFORMANCE
          requestors remain.
*/
extern void appPowerPerformanceProfileRelinquish(void);

#endif /* __AV_HEADSET_POWER_H */
