/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_proximity.h
\brief      Header file for proximity sensor support
*/

#ifndef AV_HEADSET_PROXIMITY_H
#define AV_HEADSET_PROXIMITY_H

#include "av_headset_tasklist.h"
#include "av_headset_message.h"

/*! Enumeration of messages the proximity sensor can send to its clients */
enum proximity_messages
{
    /*! The sensor has detected an object in proximity. */
    PROXIMITY_MESSAGE_IN_PROXIMITY = PROXIMITY_MESSAGE_BASE,
    /*! The sensor has detected an object not in proximity. */
    PROXIMITY_MESSAGE_NOT_IN_PROXIMITY,
};

/*! Forward declaration of a config structure (type dependent) */
struct __proximity_config;
/*! Proximity config incomplete type */
typedef struct __proximity_config proximityConfig;

/*! Forward declaration of a state structure (type dependent) */
struct __proximity_state;
/*! Proximity state incomplete type */
typedef struct __proximity_state proximityState;


/*! @brief Proximity module state. */
typedef struct
{
    /*! Proximity State module message task. */
    TaskData task;
    /*! Handle to the bitserial instance. */
    bitserial_handle handle;
    /*! List of registered client tasks */
    TaskList *clients;
    /*! The type specific proximity sensor state */
    proximityState *state;
    /*! The config */
    const proximityConfig *config;
} proximityTaskData;

/*! \brief Register with proximity to receive notifications.
    \param task The task to register.
    \return TRUE if the client was successfully registered.
            FALSE if registration was unsuccessful or if the platform does not
            have a proximity sensor.
    The sensor will be enabled the first time a client registers.
*/
#if defined(INCLUDE_PROXIMITY)
extern bool appProximityClientRegister(Task task);
#else
#define appProximityClientRegister(task) FALSE
#endif

/*! \brief Unregister with proximity.
    \param task The task to unregister.
    The sensor will be disabled when the final client unregisters. */
#if defined(INCLUDE_PROXIMITY)
extern void appProximityClientUnregister(Task task);
#else
#define appProximityClientUnregister(task) ((void)task)
#endif

#endif // AV_HEADSET_PROXIMITY_H
