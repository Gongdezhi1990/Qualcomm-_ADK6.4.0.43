/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_accelerometer.h
\brief      Header file for headset accelerometer support
*/

#ifndef AV_HEADSET_ACCELEROMETER_H
#define AV_HEADSET_ACCELEROMETER_H

#include "av_headset_tasklist.h"
#include "av_headset_message.h"
#include "dormant.h"

/*! Enumeration of messages the proximity sensor can send to its clients */
enum accelerometer_messages
{
    /*! The sensor is in motion. */
    ACCELEROMETER_MESSAGE_IN_MOTION = ACCELEROMETER_MESSAGE_BASE,
    /*! The sensor is not in motion. */
    ACCELEROMETER_MESSAGE_NOT_IN_MOTION,
};

/*! Forward declaration of a config structure (type dependent) */
struct __accelerometer_config;
/*! Accelerometer config incomplete type */
typedef struct __accelerometer_config accelerometerConfig;

/*! @brief Accelerometer module state. */
typedef struct
{
    /*! Accelerometer module message task. */
    TaskData task;
    /*! Handle to the bitserial instance. */
    bitserial_handle handle;
    /*! List of registered client tasks */
    TaskList *clients;
    /*! The config */
    const accelerometerConfig *config;    
} accelTaskData;

/*! \brief Register with accelerometer to receive notifications.
    \param task The task to register.
    \return TRUE if the client was successfully registered.
            FALSE if registration was unsuccessful or if the platform does not
            have a accelerometer sensor.
    The sensor will be enabled the first time a client registers.
*/
#if defined(INCLUDE_ACCELEROMETER)
extern bool appAccelerometerClientRegister(Task task);
#else
#define appAccelerometerClientRegister(task) FALSE
#endif

/*! \brief Unregister with accelerometer.
    \param task The task to unregister.
    The sensor will be disabled when the final client unregisters. */
#if defined(INCLUDE_ACCELEROMETER)
extern void appAccelerometerClientUnregister(Task task);
#else
#define appAccelerometerClientUnregister(task) ((void)task)
#endif

/*! \brief Obtain the key/value to pass to DormantConfigure to allow the
    accelerometer interrupt to wake the device from dormant mode. The caller may
    just pass the returned values into the DormantConfigure trap, or modify them
    to enable other wake sources on the same key.

    \param[out] key     The key to be passed to DormantConfigure
    \param[OUT] value   The value to be passed to DormantConfigure

    \return TRUE if the accelerometer is able to wake the chip from dormant and
            the key/value are valid, otherwise FALSE.
*/
extern bool appAccelerometerGetDormantConfigureKeyValue(dormant_config_key *key, uint32* value);

#endif // AV_HEADSET_ACCELEROMETER_H
