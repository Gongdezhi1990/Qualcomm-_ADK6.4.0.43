/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    capacitive_sensor_if.h

CONTAINS
    Definitions for the capacitive touch sensor subsystem.

DESCRIPTION
    This file is seen by the stack/CDA devices, and customer applications, and
    contains things that are common between them.
*/

/*!
 @file capacitive_sensor_if.h
 @brief Parameters CapacitiveSensorPadQuery(),
 CapacitiveSensorConfigureMultiPad(), CapacitiveSensorEnablePads() and
 CapacitiveSensorConfigureTouchOffset().

 See also #MessageCapacitiveSensorChanged and MessageCapacitiveSensorTask() for documentation
 on receiving messages on CapacitiveSensor events.
 For more information and recommended usage instructions, please read the
 Capacitive Touch Sense System Guide included in the CSR's Bluetooth Developers
 SDK.
*/

#ifndef __CAPACITIVE_SENSOR_IF_H__
#define __CAPACITIVE_SENSOR_IF_H__

/*! @brief General configuration functions for capacitive touch sensor.
 *
 * \note This configuration interface is deprecated.  Use CapacitiveSensorConfigMultipad
 * instead  */
typedef enum
{
    /*! Sets bulk capacitance, chip specific value */
    CAPACITIVE_SENSOR_SET_BULK_CAPACITANCE,
    /*! Sets Cint update divider */
    CAPACITIVE_SENSOR_SET_CINT_UPDATE_DIVIDER,
    /*! Sets number of ADC averages per measurement (0-7 = 1-128) */
    CAPACITIVE_SENSOR_SET_NUM_ADC_AVERAGES,
    /*! Sets gap between measurements in milliseconds (0-16ms) */
    CAPACITIVE_SENSOR_SET_MEASUREMENT_GAP,
    /*! Sets fast update divider for fast noise removal */
    CAPACITIVE_SENSOR_SET_FAST_UPDATE_DIVIDER,
    /*! Set number of successive events to cause a trigger */
    CAPACITIVE_SENSOR_SET_NUM_TRIGGER_EVENTS
} capacitive_sensor_config_key;


/*! @brief Pad specific configuration functions for capacitive touch sensor.
 *
 * \note This configuration interface is deprecated.  Use CapacitiveSensorConfigMultipad
 * instead  */
typedef enum
{
    /*! Sets trigger level of the pad specified. The level change necessary to
     *  trigger an event.  This event will be delivered to the task registered
     *  with MessageCapacitiveSensorTask(). See #MessageCapacitiveSensorChanged.
     *  A non-zero threshold enables a pad.  A zero threshold disables a pad.
     *  If all pads are disabled, the capacitive_sensor block will be shut down
     *  to conserve power.  This is the initial (power-on) state.
     *  Positive and negative thresholds are set equal.  The minimum threshold
     *  setting is 65fF and the maximum is 1023fF.  Enabling an uncalibrated
     *  pad or pad 0 when it is used as the analogue shield will result in an
     *  error.  Changing the threshold level on an enabled pad is permitted.
     *  The change will be effected the next time that pad is scanned.
     *  Any queued messages will still be delivered after a pad is disabled.
     */
    CAPACITIVE_SENSOR_SET_TRIGGER_LEVEL,

    /*! Sets the value to preload the background capacitance integrator with.
     * On legacy BlueCore chips this is provided via PSKEY_CAP_SENSE_PRELOAD.
     */
    CAPACITIVE_SENSOR_PRELOAD_BACKGROUND_CAP
} capacitive_sensor_config_pad_key;

/**
 * Oversample rate configuration options
 */
typedef enum
{
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_0 = 0, /**< No oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_1 = 1, /**< 2 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_2 = 2, /**< 4 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_3 = 3, /**< 8 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_4 = 4, /**< 16 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_5 = 5, /**< 32 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_6 = 6, /**< 64 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_7 = 7, /**< 128 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_8 = 8, /**< 256 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_9 = 9, /**< 512 oversampling */
    CAPACITIVE_SENSOR_OVER_SMPL_RATE_10 = 10, /**< 1024 oversampling */
    CAPACITIVE_SENSOR_MAX_OVER_SMPL_RATE = CAPACITIVE_SENSOR_OVER_SMPL_RATE_10
} capacitive_sensor_config_over_smpl_rate;
/**
 * Helper macro to convert a VM capacitive_sensor oversample rate into a 
 * VM capacitive_sensor oversample rate index.
 */
#define vm_capacitive_sensor_over_smpl_rate_src_to_idx(x) \
    ((uint16)(x) - (uint16)CAPACITIVE_SENSOR_OVER_SMPL_RATE_0)
/**
 * Helper macro to convert a VM capacitive_sensor oversample rate index into a 
 * VM capacitive_sensor oversample rate.
 */
#define vm_capacitive_sensor_over_smpl_rate_idx_to_src(x) \
    ((capacitive_sensor_config_over_smpl_rate)((x) + (uint16)CAPACITIVE_SENSOR_OVER_SMPL_RATE_0))

/**
 * Structure for configuring a single CapacitiveSensor channel (pad).  These
 * are the parameters most likely to be common to all pads.
 */
typedef struct
{
    uint8 charge_time;  /**< Time (us) for the analogue macro to charge sensor
                              for [analogue] */
    uint8 sample_time;  /**< Time (us) for the analogue macro to sample the
                              sensor for [analogue] */
    uint8 fast_update_period;  /**< Binary exponent of the inverse weighting
                                    of new ADC values for fast accumulator
                                    update (small => faster response)
                                    range: 0-15. [digital]*/
    uint8 slow_update_period;  /**< Binary exponent of the inverse weighting
                                    of new ADC values for slow accumulator
                                    update (large => slower response)
                                    range: 0-21. [digital]*/
    uint16 acquisition_delay;  /**< Time to insert before obtaining a reading,
                                   in units of 16 us. Range 0-0xfff. [digital]*/
    uint8 event_count;  /**< Number of consecutive trigger events that must be
                             observed before a trigger is considered to have
                             fired [digital] */
    capacitive_sensor_config_over_smpl_rate oversampling_rate; /**< Number of
                     readings over which to average the ADC reading [digital] */
} CAPACITIVE_SENSOR_PAD_CONFIG;

/**
 * Structure for configuring a single CapacitiveSensor channel (pad).  These are
 * parameters that are most likely to change between pads.
 */
typedef struct
{
    int32 trigger[2];   /**< On and off trigger levels relative to the
                             background capacitance.  Due to new hysteresis
                             compensation support, these values should typically
                             both be positive (second smaller than first) */
    uint32 preload;     /**< Value to preload into the accumulators as the
                             initial condition for the background level of
                             capacitance */
} CAPACITIVE_SENSOR_PAD_LEVELS;

#define CAPACITIVE_SENSOR_DONT_PRELOAD (HAL_DATA_CONV_ADC_RANGE+1)

typedef enum
{
    CAPACITIVE_SENSOR_PAD_DEFAULT_CHARGE_TIME          =  1,
    CAPACITIVE_SENSOR_PAD_DEFAULT_SAMPLE_TIME          =  7,
    CAPACITIVE_SENSOR_PAD_DEFAULT_FAST_UPDATE_PERIOD   =  2,
    CAPACITIVE_SENSOR_PAD_DEFAULT_SLOW_UPDATE_PERIOD   =  19,
    CAPACITIVE_SENSOR_PAD_DEFAULT_ACQUISITION_DELAY    =  0,
    CAPACITIVE_SENSOR_PAD_DEFAULT_EVENT_COUNT          =  0,
    CAPACITIVE_SENSOR_PAD_DEFAULT_OVERSAMPLING_RATE    =  CAPACITIVE_SENSOR_OVER_SMPL_RATE_0
} CAPACITIVE_SENSOR_PAD_DEFAULT;

/** Convenience macro for setting defaults in a CAPACITIVE_SENSOR_PAD_CONFIG */
#define CAPACITIVE_SENSOR_PAD_CONFIG_SET_DEFAULTS(cfg) \
    (cfg)->charge_time          = CAPACITIVE_SENSOR_PAD_DEFAULT_CHARGE_TIME;\
    (cfg)->sample_time          = CAPACITIVE_SENSOR_PAD_DEFAULT_SAMPLE_TIME;\
    (cfg)->fast_update_period   = CAPACITIVE_SENSOR_PAD_DEFAULT_FAST_UPDATE_PERIOD;\
    (cfg)->slow_update_period   = CAPACITIVE_SENSOR_PAD_DEFAULT_SLOW_UPDATE_PERIOD;\
    (cfg)->acquisition_delay    = CAPACITIVE_SENSOR_PAD_DEFAULT_ACQUISITION_DELAY;\
    (cfg)->event_count          = CAPACITIVE_SENSOR_PAD_DEFAULT_EVENT_COUNT;\
    (cfg)->oversampling_rate    = (capacitive_sensor_config_over_smpl_rate) \
                                  CAPACITIVE_SENSOR_PAD_DEFAULT_OVERSAMPLING_RATE;


/*! @brief The action which caused the event */
typedef enum
{
    /*! Pad delta value has crossed from above negative threshold to below it.
     */
    CAPACITIVE_SENSOR_EVENT_NEG = 0,
    /*! Pad delta value has crossed from below positive threshold to above it.
     */
    CAPACITIVE_SENSOR_EVENT_POS = 1
} capacitive_sensor_event_type;


/*! @brief Returned values from CapacitiveSensorPadQuery() */
typedef struct
{
    /*! Pad absolute capacitance. This is the sum of both internal (Cbulk and 
     *  Cint) and external (touch) capacitances.
     */
    uint32  Cint_fF;

    /*! Pad capacitance change compared to the slow moving average value.
     */
    int32   Cint_delta_fF;  
} capacitive_sensor_pad_state;


/*! @brief Description of single event registered */
typedef struct
{
    unsigned int  pad :8;        /*!< Pad number */
    unsigned int  direction :8;  /*!< Positive or negative excursion,
                                      see \c capacitive_sensor_event_type */
    uint16        time_ms;       /*!< Timestamp of event */
} capacitive_sensor_event;
#endif /* __CAPACITIVE_SENSOR_IF_H__ */

