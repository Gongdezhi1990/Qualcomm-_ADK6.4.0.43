/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Header file defining the temperature sensor API. Temperature sensors
            (e.g. thermistor) should provide an implementation for this API.
            The API is called by the av_headset_temperature module.
*/

#ifndef AV_HEADSET_TEMPERATURE_SENSOR_H
#define AV_HEADSET_TEMPERATURE_SENSOR_H

#include "message.h"
#include "types.h"

#if defined(INCLUDE_TEMPERATURE)

/*! \brief Initialise the sensor for taking measurements */
void appTemperatureSensorInit(void);

/*! \brief Request a new temperature measurement is taken.
    \param task The task to which any measurement related messages should be delivered.
    \note The caller will call #appTemperatureSensorHandleMessage for any messages
    delivered to task. */
void appTemperatureSensorRequestMeasurement(Task task);

/*! \brief Handle a temperature measurement related message and get the latest
    temperature measurement.

    \param[in] task The recipient task.
    \param id The message ID.
    \param message The message content.
    \param[in,out] temperature Input: the last temperature measurement, which may
    assist the sensor in measuring the current temperature. Output: The new
    temperature measurement result. Unmodified if the function returns FALSE.
    \note Temperatures are in degrees Celsius.

    \return TRUE if a new temperature measurement is available in temperature,
    otherwise FALSE. */
bool appTemperatureSensorHandleMessage(Task task, MessageId id, Message message, int8 *temperature);

#endif /* INCLUDE_TEMPERATURE */

#endif /* AV_HEADSET_TEMPERATURE_SENSOR_H */
