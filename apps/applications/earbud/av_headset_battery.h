/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_battery.h
\brief      Header file for Battery monitoring
*/


#ifndef _AV_HEADSET_BATTERY_H_
#define _AV_HEADSET_BATTERY_H_

#include "av_headset_message.h"

/*! The battery filter in this implementation is a power of 2 in length */
#define BATTERY_FILTER_POWER2 4
/*! The battery filter length. */
#define BATTERY_FILTER_LEN (1 << BATTERY_FILTER_POWER2)
/*! Bitmask for the number of bits in the battery filter length. */
#define BATTERY_FILTER_MASK (BATTERY_FILTER_LEN-1)

/*! Battery level updates messages. The message a client receives depends upon
    the batteryRegistrationForm::representation set when registering by calling
    #appBatteryRegister.  */
enum battery_messages
{
    /*! Message signalling the battery module initialisation is complete */
    MESSAGE_BATTERY_INIT_CFM = EB_BATTERY_MESSAGE_BASE,
    /*! Message signalling the battery voltage has changed. */
    MESSAGE_BATTERY_LEVEL_UPDATE_VOLTAGE,
    /*! Message signalling the battery percentage has changed. */
    MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT,
    /*! Message signalling the battery state has changed. */
    MESSAGE_BATTERY_LEVEL_UPDATE_STATE
};

/*! Highest level battery level states */
typedef enum 
{
    battery_level_unknown,
    battery_level_too_low,
    battery_level_critical,
    battery_level_low,
    battery_level_ok,
} battery_level_state;

/*! Options for representing the battery voltage */
enum battery_level_representation
{
    /*! As a voltage */
    battery_level_repres_voltage,
    /*! As a percent */
    battery_level_repres_percent,
    /*! As high-level states */
    battery_level_repres_state
};

/*! Message #MESSAGE_BATTERY_LEVEL_UPDATE_VOLTAGE content. */
typedef struct
{
    /*! The updated battery voltage in milli-volts. */
    uint16 voltage_mv;
} MESSAGE_BATTERY_LEVEL_UPDATE_VOLTAGE_T;

/*! Message #MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT content. */
typedef struct
{
    /*! The updated battery voltage as a percentage. */
    uint8 percent;
} MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT_T;

/*! Message #MESSAGE_BATTERY_LEVEL_UPDATE_STATE content. */
typedef struct
{
    /*! The updated battery state. */
    battery_level_state state;
} MESSAGE_BATTERY_LEVEL_UPDATE_STATE_T;

/*! Battery client registration form */
typedef struct
{
    /*! The task that will receive battery status messages */
    Task task;
    /*! The representation method requested by the client */
    enum battery_level_representation representation;
    /*! The reporting hysteresis:
          battery_level_repres_voltage: in millivolts
          battery_level_repres_percent: in percent
          battery_level_repres_state: in millivolts
    */
    uint16 hysteresis;

} batteryRegistrationForm;

/*! Structure used internally to the battery module to store per-client state */
typedef struct battery_registered_client_item
{
    /*! The next client in the list */
    struct battery_registered_client_item *next;
    /*! The client's registration information */
    batteryRegistrationForm form;
    /*! The last battery value sent to the client */
    union
    {
        /*! As a voltage */
        uint16 voltage;
        /*! As a percentage */
        uint8 percent;
        /*! As a state */
        battery_level_state state;
    } last;
} batteryRegisteredClient;

/*! Battery task structure */
typedef struct
{
    /*! Battery task */
    TaskData task;
    /*! The measurement period */
    uint16 period;
    /*! Store the vref measurement, which is required to calculate vbat */
    uint16 vref_raw;
    /*! A sub-struct to allow reset */
    struct
    {
        /*! Buffer of battery voltages in mv */
        uint16 buf[BATTERY_FILTER_LEN];
        /*! The current index into the filter */
        uint32 index;
        /*! Running sum of the filter buffer */
        uint32 accumulator;
    } filter;
    /*! A linked-list of clients */
    batteryRegisteredClient *client_list;
} batteryTaskData;

/*! Start monitoring the battery voltage */
extern void appBatteryInit(void);

/*! @brief Override the default measurement period.
    @param period The measurement period in milli-seconds.
    Setting to zero stops and resets the monitor, the monitor remains stopped
    until a non-zero value is set.
 */
extern void appBatterySetPeriod(uint16 period);

/*! @brief Register to receive battery change notifications.

    @note The first notification after registering will only be
    sent when sufficient battery readings have been taken after
    power on to ensure that the notification represents a stable
    value.

    @param form The client's registration form.

    @return TRUE on successful registration, otherwise FALSE.
*/
extern bool appBatteryRegister(batteryRegistrationForm *form);

/*! @brief Unregister a task from receiving battery change notifications.
    @param task The client task to unregister.
    Silently ignores unregister requests for a task not previously registered
*/
extern void appBatteryUnregister(Task task);

/*! @brief Read the filtered battery voltage in mV.
    @return The battery voltage. */
extern uint16 appBatteryGetVoltage(void);

/*! @brief Read the battery percent.
    @return The battery voltage as a percentage. */
extern uint8 appBatteryGetPercent(void);

/*! @brief Read the battery state.
    @return The battery state. */
extern battery_level_state appBatteryGetState(void);


/*! Convert a battery voltage in mv to a percentage

    \param  level_mv The battery level in milliVolts

    \return The battery percentage equivalent to supplied level */
extern uint8 appBatteryConvertLevelToPercentage(uint16 level_mv);

#endif
