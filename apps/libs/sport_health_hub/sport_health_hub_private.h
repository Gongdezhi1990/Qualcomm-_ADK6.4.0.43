/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016

\file
    This defines private functions for sport and health hub
*/
#ifndef SPORT_HEALTH_HUB_PRIVATE_H
#define SPORT_HEALTH_HUB_PRIVATE_H

/** Macros for creating messages */
#define MAKE_SH_HUB_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_SH_HUB_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN - 1);

#include <time.h>
#include "print.h"
#include "panic.h"
#include "sport_health_driver.h"
#include "sport_health_hub_sm.h"
#include "sport_health_hub.h"

#define TARGET_WINDOW_SIZE_DIV2_MS 20
#define TARGET_PERIOD_BT_ACTIVE 250

/** Macros defined in all variants of the HUB library */
#define SH_HUB_PANIC() Panic()
#define SH_HUB_ASSERT(x) {if (!(x)) Panic();}
#define SH_HUB_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}

/** Data structure which holds all the informations needed
 *  for processing funtions in Hub.
 */
typedef struct
{
    TaskData hub_task;
    Task     app_task;
    Task     driver_task;
    imu_mode_t imu_mode;
    ppg_mode_t ppg_mode;
    prox_mode_t prox_mode;
    motion_mode_t motion_detect_info;
    ppg_mode_t ppg_mode_shadow;
    hub_state_t state;
    sh_vm_bt_mode_t bt_mode;
    time_t   curr_wake;
    time_t   next_wake;
    uint32   wake_count;
    uint32   bt_interval_ms;
    uint16   algo_min_ms;
    uint16   algo_max_ms;
    uint16   fifo_min_ms;
    uint16   fifo_max_ms;
    uint16   min_interval_ms;
    uint16   max_interval_ms;
    uint16   target_early_ms; //reserved for future use
    uint16   target_late_ms;  //reserved for future use
    uint16   target_ms;
    bool     in_out_status;         //flag to store in ear or out of ear proximity status
    bool     v_motion_detect;
}sh_hub_data_t;

/**
 * @brief Sets the state for the hub
 * @param state (in) state to be set for the hub
 */
void sh_hub_set_state(hub_state_t state);
/**
 * @brief Gets the state for the hub
 * @return The current state of the hub
 */
hub_state_t sh_hub_get_state(void);
/**
 * @brief Event handler when the hub state is uninitialised
 * @param event
 * @return success TRUE/FALSE
 */
bool sh_hub_state_uninitialised_handle_event(hub_state_event_t event);
/**
 * @brief Event handler when the hub state is ready i.e.,
 *        hub is initialised and algorithms are enabled as per
 *        configured bitfield in customer app. Also, the required
 *        memory is allocated for each enabled algorithm.
 * @param event
 * @return success TRUE/FALSE
 */
bool sh_hub_state_ready_handle_event(hub_state_event_t event);
/**
 * @brief Event handler when the hub state is active i.e.,
 *        hub is woken up by the vm_timer and the sensor data
 *        is received for processing.
 * @param event
 * @return success TRUE/FALSE
 */
bool sh_hub_state_active_handle_event(hub_state_event_t event);
/**
 * @brief Delete the hub task as application has triggered
 *        teardown
 */
void sport_health_hub_delete(void);

#endif /**  SPORT_HEALTH_HUB_PRIVATE_H */
