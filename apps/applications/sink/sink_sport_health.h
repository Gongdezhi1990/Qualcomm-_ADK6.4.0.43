/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_sport_health.h
*/

#ifndef SINK_SPORT_HEALTH_H
#define SINK_SPORT_HEALTH_H
#ifdef ACTIVITY_MONITORING
/** Macros for creating messages */
#define MAKE_SHAPP_MESSAGE(TYPE) TYPE##_T *msg = PanicUnlessNew(TYPE##_T);

#ifdef TEST_TAP
#define SINGLE_TONE  84 /* tone_Q38 defined in sink_tones.c */
#define DOUBLE_TONE  23 /* tone_X defined in sink_tones.c */
#endif

/* macro for GATT Library debug lib printfs */
#ifdef ACTIVITY_MONITORING_DEBUG
#include <panic.h>
#include <stdio.h>
#define SINK_SPORT_HEALTH_DEBUG_INFO(x)  {printf x;}
#define SINK_SPORT_HEALTH_DEBUG(x)       {printf x;  Panic();}
#else
#define SINK_SPORT_HEALTH_DEBUG_INFO(x)
#define SINK_SPORT_HEALTH_DEBUG(x)
#endif

/** Helper macro for calculating the PIO bank */
#define PBANK(pio) ((uint16)((pio) / PIOS_PER_BANK))
/** Helper macro for calculating the PIO offset within the bank */
#define POFF(pio) ((pio) % PIOS_PER_BANK)
/** Helper macro for calculating the PIO offset mask */
#define POFFM(pio) (1U << POFF(pio))

/** Supported modes for sports and health app */
typedef enum
{
    SH_APP_UNINITIALISED,
    SH_APP_TIMING_SET,
    SH_APP_ACTIVE,
} sh_app_mode_t;

/** Data structure for sports and health app */
typedef struct
{
    TaskData sh_app_task;
    Task driver_task;
    Task hub_task;
    uint32 algo_enable;
    bool enabled;
    sh_app_mode_t mode;
}sport_health_app_t;

#endif
/**
 * @brief Message handler for sports and health app task
 * @param task Task ID
 * @param id Message ID
 * @param message Message content
 */
#ifdef ACTIVITY_MONITORING
Task sport_health_app_init(void);
#else
#define sport_health_app_init() ((void) (0))
#endif

/**
 * @brief This function is used to send the configuration required
 *        to enable the algorithms supported as part of sports and health.
 * @param algo_enable Bitfield which contains all the algorithms to be enabled
 */
#ifdef ACTIVITY_MONITORING
void sh_seq_enable(uint32 algo_enable);
#else
#define sh_seq_enable(algo_enable) UNUSED(algo_enable)
#endif

/**
 * @brief Function to enable the algorithms supported by default.
*/
#ifdef ACTIVITY_MONITORING
void sinkSportHealthEnableDefault(void);
#else
#define sinkSportHealthEnableDefault() ((void) (0))
#endif

/* Pass in pointer to buffer and buffer length. Returns length of data copied into buffer */
#ifdef ACTIVITY_MONITORING
uint16 getRSCMeasReading(uint8 *p_rsc_meas, uint8 max_length);
#else
#define getRSCMeasReading(p_rsc_meas, max_length) UNUSED(p_rsc_meas, max_length)
#endif

#ifdef ACTIVITY_MONITORING
uint16 getRSCExtensionReading(uint8 *p_rsc_meas, uint8 max_length);
#else
#define getRSCExtensionReading(p_rsc_meas, max_length) UNUSED(p_rsc_meas, max_length)
#endif

#ifdef ACTIVITY_MONITORING
/*!
 * @brief Handler for messages from the system about PIO changes
 *
 * This handler only handles the MESSAGE_PIO_CHANGED event from
 * the system.
 *
 * @param pTask Task data associated with events
 * @param pId The ID of the message we have received
 * @param pMessage Pointer to the message content (if any)
 */
void sport_health_interrupt_handler ( const void* pMessage );
#else
#define sport_health_interrupt_handler( pMessage ) UNUSED(pMessage)
#endif

/**
 * @brief This function is used to get the bitmask for logging debug configuration
 *        in the algorithms.
 * @param p_logging_debug_configuration Pointer to the debug configuration data.
 * @param max_length maximum length of this data
 * @return Lenght of the data
 */
#ifdef ACTIVITY_MONITORING
uint16 sinkSportHealthGetLoggingDebugConfiguration(uint8 *p_logging_debug_configuration, uint8 max_length);
#else
#define sinkSportHealthGetLoggingDebugConfiguration(p_logging_debug_configuration, max_length) UNUSED(p_logging_debug_configuration, max_length)
#endif

/**
 * @brief This function is used to set the bitmask of the debug configuration for
 * different logging levels in the algorithms.
 * @param p_logging_debug_configuration Pointer to the debug configuration data.
 * @return None
 */
#ifdef ACTIVITY_MONITORING
void sinkSportHealthSetLoggingDebugConfiguration(uint8 *p_logging_debug_configuration);
#else
#define sinkSportHealthSetLoggingDebugConfiguration(p_logging_debug_configuration) UNUSED(p_logging_debug_configuration)
#endif

#endif /* SINK_SPORT_HEALTH_H */
