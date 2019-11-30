/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_temperature.h
\brief      Clients of the temperature module should use this API. If a client just
            wants to read the current temperature, appTemperatureGet() should be used.
            If a client wants to be informed about changes in temperature with respect
            to limits, the client should register using #appTemperatureClientRegister.
            The module will then send status #temperatureMessages to the client.
*/

#ifndef AV_HEADSET_TEMPERATURE_H
#define AV_HEADSET_TEMPERATURE_H

#include "av_headset_tasklist.h"
#include "av_headset_message.h"

/*! Enumeration of messages the temperature module can send to its clients */
enum temperatureMessages
{
    /*! Temperature initialisation complete. */
    TEMPERATURE_INIT_CFM = TEMPERATURE_MESSAGE_BASE,
    /*! The temperature state has changed. */
    TEMPERATURE_STATE_CHANGED_IND,
};

/*! Client temperature states */
typedef enum temperature_states
{
    TEMPERATURE_STATE_WITHIN_LIMITS,
    TEMPERATURE_STATE_ABOVE_UPPER_LIMIT,
    TEMPERATURE_STATE_BELOW_LOWER_LIMIT,
    TEMPERATURE_STATE_UNKNOWN,
} temperatureState;

/*! Message content for #TEMPERATURE_STATE_CHANGED_IND */
typedef struct
{
    /*! The new state */
    temperatureState state;
} TEMPERATURE_STATE_CHANGED_IND_T;

/*! \brief Temperature module state. */
typedef struct
{
    /*! Temperature module message task. */
    TaskData task;
    /*! List of registered client tasks */
    TaskList *clients;
    /*! The latest thermistor temperature measurement */
    int8 temperature;
    /*! Temperature module's message lock */
    uint16 lock;
} temperatureTaskData;

/*! \brief Initialise the temperature module */
extern void appTemperatureInit(void);

/*! \brief Destroy the temperature module */
extern void appTemperatureDestroy(void);

/*! \brief Register with temperature to receive notifications.
    \param task The task to register.
    \param lower_limit The lower temperature limit in degrees Celsius.
    \param upper_limit The upper temperature limit in degrees Celsius.
    \return TRUE if registration was successful, FALSE if temperature
    support is not included in the built image.
    \note The temperature measurement range depends on the capabilities of the
    temperature sensor used. If a limit is selected outside the sensor measurement
    range, that limit will never be exceeded. The limit is settable in the range
    of a signed 8-bit integer (-128 to +127 degrees Celsius).
*/
#if defined(INCLUDE_TEMPERATURE)
extern bool appTemperatureClientRegister(Task task, int8 lower_limit, int8 upper_limit);
#else
#define appTemperatureClientRegister(task, lower, upper) FALSE
#endif

/*! \brief Unregister with temperature.
    \param task The task to unregister.
*/
#if defined(INCLUDE_TEMPERATURE)
extern void appTemperatureClientUnregister(Task task);
#else
#define appTemperatureClientUnregister(task) ((void)0)
#endif

/*! \brief Get the client's state.
    \param task The client's task.
    \return The client's state, or TEMPERATURE_STATE_WITHIN_LIMITS if the platform does not
            support temperature measurement.
*/
#if defined(INCLUDE_TEMPERATURE)
extern temperatureState appTemperatureClientGetState(Task task);
#else
#define appTemperatureClientGetState(task) TEMPERATURE_STATE_WITHIN_LIMITS
#endif

/*! \brief Get the last measured temperature.
    \return The last temperature measurement.
*/
#if defined(INCLUDE_TEMPERATURE)
extern int8 appTemperatureGet(void);
#endif

#endif // AV_HEADSET_TEMPERATURE_H
