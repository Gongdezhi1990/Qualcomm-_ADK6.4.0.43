/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sport_health_hub_sm.h

DESCRIPTION
    Entry point to the HUB VM Library State Machine.
*/

#ifndef SPORT_HEALTH_HUB_SM_H
#define SPORT_HEALTH_HUB_SM_H

/** Hub library states */
typedef enum
{
    HUB_STATE_UNINITIALISE,
    HUB_STATE_READY,
    HUB_STATE_ACTIVE
}hub_state_t;

/** Hub library state events */
typedef enum
{
    HUB_STATE_EVENT_INITIALISE,    /**< HUB_STATE_UNINITIALISE */
    HUB_STATE_EVENT_ALGO_ENABLE,   /**< HUB_STATE_READY or HUB_STATE_ACTIVE */
    HUB_STATE_EVENT_IMU_ACTIVATE,  /**< HUB_STATE_READY */
    HUB_STATE_EVENT_PPG_ACTIVATE,  /**< HUB_STATE_READY */
    HUB_STATE_EVENT_PROX_ACTIVATE,  /**< HUB_STATE_READY */
    HUB_STATE_EVENT_ACTIVATE,
    HUB_STATE_EVENT_IMU_DATA_RCVD,
    HUB_STATE_EVENT_PPG_DATA_RCVD,
    HUB_STATE_EVENT_UPDATE_TIMER,   /**< HUB_STATE_READY or HUB_STATE_ACTIVE */
    HUB_STATE_EVENT_ALGO_DISABLE
}hub_state_event_id_t;

/** Event structure that is used to inject an event and any corresponding
   arguments into the hub state machine. */
typedef struct
{
    hub_state_event_id_t id;
    void *args;
} hub_state_event_t;

/**
 * @brief Main entry point to the ANC VM library State Machine. All events will be
 *        injected using this function that will then determine which state specific
 *        handler should process the event.
 *
 * @param event
 *
 * @return TRUE/FALSE
 */
bool sh_hub_sm_handle_event(hub_state_event_t event);

#endif // SPORT_HEALTH_HUB_SM_H
