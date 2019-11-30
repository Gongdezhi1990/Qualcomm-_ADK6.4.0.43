/*!
\copyright  Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_sm.h
\brief	    Header file for the application state machine
*/

#ifndef _AV_HEADSET_SM_H_
#define _AV_HEADSET_SM_H_

#include "av_headset_conn_rules.h"
#include "av_headset_phy_state.h"

/*!
    @startuml

    note "For clarity not all state transitions shown" as N1
    note "For clarity TERMINATING substate is not shown in all parent states" as N2
    note top of STARTUP
      _IDLE is any of the idle states
      * IN_CASE_IDLE
      * OUT_OF_CASE_IDLE
      * IN_EAR_IDLE
    end note

    [*] -down-> INITIALISING : Power On
    INITIALISING : App module and library init
    INITIALISING --> DFU_CHECK : Init Complete

    DFU_CHECK : Is DFU in progress?
    DFU_CHECK --> STARTUP : No DFU in progress

    STARTUP : Check for paired peer earbud
    STARTUP : Attempt peer synchronisation
    STARTUP : After attempt go to _IDLE
    STARTUP --> PEER_PAIRING : No paired peer

    FACTORY_RESET : Disconnect links, deleting all pairing, reboot
    FACTORY_RESET : Only entered from _IDLE
    FACTORY_RESET -r-> INITIALISING : Reboot

    PEER_PAIRING : Pair with peer earbud
    PEER_PAIRING --> STARTUP
    HANDSET_PAIRING : Pair with handset
    HANDSET_PAIRING : Return to _IDLE state

    state IN_CASE #LightBlue {
        IN_CASE : Charger Active
        IN_CASE : Buttons Disabled
        DFU : Device Upgrade
        DFU --> IN_CASE_IDLE #LightGreen : DFU Complete
        DFU_CHECK --> DFU : DFU in progress
        IN_CASE_IDLE : May have BT connection(s)
        IN_CASE_IDLE -up-> DFU : Start DFU
    }

    state OUT_OF_CASE #LightBlue {
        OUT_OF_CASE_IDLE : May have BT connection(s)
        OUT_OF_CASE_IDLE : Start dormant timer
        OUT_OF_CASE_SOPORIFIC : Allow sleep
        OUT_OF_CASE_SOPORIFIC_TERMINATING : Disconnect links
        OUT_OF_CASE_SOPORIFIC_TERMINATING : Inform power prepared to sleep
        OUT_OF_CASE_IDLE #LightGreen --> IN_CASE_IDLE : In Case
        IN_CASE_IDLE --> OUT_OF_CASE_IDLE : Out of Case
        OUT_OF_CASE_IDLE -u-> OUT_OF_CASE_SOPORIFIC : Idle timeout
        OUT_OF_CASE_SOPORIFIC -->  OUT_OF_CASE_SOPORIFIC_TERMINATING : POWER_SLEEP_PREPARE_IND
        OUT_OF_CASE_IDLE --> HANDSET_PAIRING : User or Auto pairing
        OUT_OF_CASE_BUSY : Earbud removed from ear
        OUT_OF_CASE_BUSY : Audio still playing
        OUT_OF_CASE_BUSY #LightGreen --> OUT_OF_CASE_IDLE : Out of ear audio timeout
        OUT_OF_CASE_BUSY --> OUT_OF_CASE_IDLE : Audio Inactive
    }

    state IN_EAR #LightBlue {
        IN_EAR_IDLE : May have BT connection(s)
        IN_EAR_IDLE #LightGreen -l-> OUT_OF_CASE_IDLE : Out of Ear
        IN_EAR_IDLE -u-> IN_EAR_BUSY : Audio Active
        OUT_OF_CASE_IDLE --> IN_EAR_IDLE : In Ear
        IN_EAR_BUSY : Streaming Audio Active (A2DP or SCO)
        IN_EAR_BUSY : Tones audio available in other states
        IN_EAR_BUSY #LightGreen -d-> IN_EAR_IDLE : Audio Inactive
        IN_EAR_BUSY -l-> OUT_OF_CASE_BUSY : Out of Ear
        OUT_OF_CASE_BUSY -l-> IN_EAR_BUSY : In Ear
        IN_EAR_IDLE --> HANDSET_PAIRING : User or Auto pairing
    }
    @enduml
*/

/*! \brief The state machine substates, note that not all parent states support
           all substates. */
typedef enum sm_application_sub_states
{
    APP_SUBSTATE_TERMINATING           = 0x0001, /*!< Preparing to shutdown (e.g. disconnecting links) */
    APP_SUBSTATE_SOPORIFIC             = 0x0002, /*!< Allowing sleep */
    APP_SUBSTATE_SOPORIFIC_TERMINATING = 0x0004, /*!< Preparing to sleep (e.g. disconnecting links) */
    APP_SUBSTATE_IDLE                  = 0x0008, /*!< Audio is inactive */
    APP_SUBSTATE_BUSY                  = 0x0010, /*!< Audio is active */
    APP_SUBSTATE_DFU                   = 0x0020, /*!< Upgrading firmware */

    APP_SUBSTATE_INITIALISING          = 0x0040, /*!< App module and library initialisation in progress. */
    APP_SUBSTATE_DFU_CHECK             = 0x0080, /*!< Interim state, to see if DFU is in progress. */
    APP_SUBSTATE_FACTORY_RESET         = 0x0100, /*!< Resetting the earbud to factory defaults. */
    APP_SUBSTATE_STARTUP               = 0x0200, /*!< Startup, syncing with peer. */

    APP_SUBSTATE_PEER_PAIRING          = 0x0400, /*!< Pairing with peer earbud */
    APP_SUBSTATE_HANDSET_PAIRING       = 0x0800, /*!< Pairing with a handset */
    APP_SUBSTATE_DISCONNECTING         = 0x1000, /*!< Handing over handset connection */

    APP_END_OF_SUBSTATES = APP_SUBSTATE_DISCONNECTING, /*!< The last substate */
    APP_SUBSTATE_MASK    = ((APP_END_OF_SUBSTATES << 1) - 1), /*!< Bitmask to retrieve substate from full state */
} appSubState;

/*! \brief Application states.
 */
typedef enum sm_application_states
{
    /*!< Initial state before state machine is running. */
    APP_STATE_NULL                = 0x0000,
    APP_STATE_INITIALISING        = APP_SUBSTATE_INITIALISING,
    APP_STATE_DFU_CHECK           = APP_SUBSTATE_DFU_CHECK,
    APP_STATE_FACTORY_RESET       = APP_SUBSTATE_FACTORY_RESET,
    APP_STATE_STARTUP             = APP_SUBSTATE_STARTUP,
    APP_STATE_PEER_PAIRING        = APP_SUBSTATE_PEER_PAIRING,
    APP_STATE_HANDSET_PAIRING     = APP_SUBSTATE_HANDSET_PAIRING,
    APP_STATE_TERMINATING         = APP_SUBSTATE_TERMINATING,

    /*! Earbud is in the case, parent state */
    APP_STATE_IN_CASE                   = APP_END_OF_SUBSTATES<<1,
        APP_STATE_IN_CASE_TERMINATING   = APP_STATE_IN_CASE + APP_SUBSTATE_TERMINATING,
        APP_STATE_IN_CASE_IDLE          = APP_STATE_IN_CASE + APP_SUBSTATE_IDLE,
        APP_STATE_IN_CASE_DFU           = APP_STATE_IN_CASE + APP_SUBSTATE_DFU,
        APP_STATE_IN_CASE_DISCONNECTING = APP_STATE_IN_CASE + APP_SUBSTATE_DISCONNECTING,

    /*!< Earbud is out of the case, parent state */
    APP_STATE_OUT_OF_CASE                           = APP_STATE_IN_CASE<<1,
        APP_STATE_OUT_OF_CASE_TERMINATING           = APP_STATE_OUT_OF_CASE + APP_SUBSTATE_TERMINATING,
        APP_STATE_OUT_OF_CASE_SOPORIFIC             = APP_STATE_OUT_OF_CASE + APP_SUBSTATE_SOPORIFIC,
        APP_STATE_OUT_OF_CASE_SOPORIFIC_TERMINATING = APP_STATE_OUT_OF_CASE + APP_SUBSTATE_SOPORIFIC_TERMINATING,
        APP_STATE_OUT_OF_CASE_IDLE                  = APP_STATE_OUT_OF_CASE + APP_SUBSTATE_IDLE,
        APP_STATE_OUT_OF_CASE_BUSY                  = APP_STATE_OUT_OF_CASE + APP_SUBSTATE_BUSY,
        APP_STATE_OUT_OF_CASE_DISCONNECTING         = APP_STATE_OUT_OF_CASE + APP_SUBSTATE_DISCONNECTING,

    /*!< Earbud in in ear, parent state */
    APP_STATE_IN_EAR                   = APP_STATE_OUT_OF_CASE<<1,
        APP_STATE_IN_EAR_TERMINATING   = APP_STATE_IN_EAR + APP_SUBSTATE_TERMINATING,
        APP_STATE_IN_EAR_IDLE          = APP_STATE_IN_EAR + APP_SUBSTATE_IDLE,
        APP_STATE_IN_EAR_BUSY          = APP_STATE_IN_EAR + APP_SUBSTATE_BUSY,
        APP_STATE_IN_EAR_DISCONNECTING = APP_STATE_IN_EAR + APP_SUBSTATE_DISCONNECTING,

} appState;

/*! \brief Return TRUE if the state is in the case */
#define appSmStateInCase(state) (0 != ((state) & APP_STATE_IN_CASE))

/*! \brief Return TRUE if the state is out of case 
    \note this means in one of the "out of case" states, and does not
    include "in ear".
*/
#define appSmStateOutOfCase(state) (0 != ((state) & APP_STATE_OUT_OF_CASE))

/*! \brief Return TRUE if the state is in the ear */
#define appSmStateInEar(state) (0 != ((state) & APP_STATE_IN_EAR))

/*! \brief Return TRUE if the sub-state is terminating */
#define appSmSubStateIsTerminating(state) \
    (0 != ((state) & (APP_SUBSTATE_TERMINATING | APP_SUBSTATE_SOPORIFIC_TERMINATING)))

/*! \brief Return TRUE if the state is idle */
#define appSmStateIsIdle(state) (0 != ((state) & (APP_SUBSTATE_IDLE | APP_SUBSTATE_SOPORIFIC)))

/*! \brief Check if the application is in a core state.
    \note Warning, ensure this macro is updated if enum #sm_application_states
    or #sm_application_sub_states is changed.
*/
#define appSmIsCoreState() (0 != (appGetSubState() & (APP_SUBSTATE_IDLE | \
                                                      APP_SUBSTATE_BUSY | \
                                                      APP_SUBSTATE_SOPORIFIC)))

/*! \brief Main application state machine task data. */
typedef struct
{
    TaskData task;                      /*!< SM task */
    appState state;                     /*!< Application state */
    phyState phy_state;                 /*!< Cache the current physical state */
    uint16 disconnect_lock;             /*!< Disconnect message lock */
    bool user_pairing:1;                /*!< User initiated pairing */
                                        /*! Flag to allow a specific DFU mode, entered when entering the case.
                                            This flag is set when using the UI to request DFU. The device will 
                                            need to be placed into the case (attached to a charger) before DFU
                                            will be allowed */
    bool enter_dfu_in_case:1;
} smTaskData;

/*! \brief Used to specify which link needs to be disconnected */
typedef enum
{
    SM_DISCONNECT_PEER     = 1 << 0,
    SM_DISCONNECT_HANDSET  = 1 << 1,
    SM_DISCONNECT_ALL      = SM_DISCONNECT_PEER | SM_DISCONNECT_HANDSET,
} smDisconnectBits;

/*! \brief Get the state machine disconnect lock. */
#define appSmDisconnectLockGet() (appGetSm()->disconnect_lock)
/*! \brief Clear all bits in the disconnect lock */
#define appSmDisconnectLockClearAll() appSmDisconnectLockGet() = 0;
/*! \brief Set connected links (that will be disconnected) bits in the disconnect lock */
#define appSmDisconnectLockSetLinks(sm_disconnect_bits) appSmDisconnectLockGet() |= sm_disconnect_bits
/*! \brief Clear connected links (that will be disconnected) bits in the disconnect lock */
#define appSmDisconnectLockClearLinks(sm_disconnect_bits) appSmDisconnectLockGet() &= ~sm_disconnect_bits
/*! \brief Query if the peer is disconnecting */
#define appSmDisconnectLockPeerIsDisconnecting() (0 != (appSmDisconnectLockGet() & SM_DISCONNECT_PEER))
/*! \brief Query if the handset is disconnecting */
#define appSmDisconnectLockHandsetIsDisconnecting() (0 != (appSmDisconnectLockGet() & SM_DISCONNECT_HANDSET))

/*! \brief Change application state.

    \param new_state State to move to.
 */
extern void appSetState(appState new_state);

/*! \brief Get current application state.

    \return appState Current application state.
 */
extern appState appGetState(void);

/*! \brief Initialise the main application state machine.
 */
extern void appSmInit(void);

/*! \brief Application state machine message handler.
    \param task The SM task.
    \param id The message ID to handle.
    \param message The message content (if any).
*/
extern void appSmHandleMessage(Task task, MessageId id, Message message);

/* FUNCTIONS THAT CHECK THE STATE OF THE SM
 *******************************************/

/*! @brief Query if this earbud is in-ear.
    @return TRUE if in-ear, otherwise FALSE. */
extern bool appSmIsInEar(void);
/*! @brief Query if this earbud is out-of-ear.
    @return TRUE if out-of-ear, otherwise FALSE. */
extern bool appSmIsOutOfEar(void);
/*! @brief Query if this earbud is in-case.
    @return TRUE if in-case, otherwise FALSE. */
extern bool appSmIsInCase(void);
/*! @brief Query if this earbud is out-of-case.
    @return TRUE if out-of-case, otherwise FALSE. */
extern bool appSmIsOutOfCase(void);
/*! @brief Query if the earbud is connectable.
    @return TRUE if connectable, otherwise FALSE. */
extern bool appSmIsConnectable(void);
/*! @brief Query if the earbud is in the special DFU mode.
    @return TRUE if in DFU mode, otherwise FALSE. */
#define appSmIsInDfuMode() (appGetState() == APP_STATE_IN_CASE_DFU)

/*! @brief Query if this earbud is intentionally disconnecting links. */
#define appSmIsDisconnectingLinks() \
    (0 != (appGetSubState() & (APP_SUBSTATE_SOPORIFIC_TERMINATING | \
                               APP_SUBSTATE_TERMINATING | \
                               APP_SUBSTATE_FACTORY_RESET | \
                               APP_SUBSTATE_DISCONNECTING)))

/*! @brief Query if this earbud is pairing. */
#define appSmIsPairing() \
    (appGetState() == APP_STATE_HANDSET_PAIRING)
/*! @brief Query if this state is a sleepy state. */
#define appSmStateIsSleepy(state) \
    (0 != ((state) & (APP_SUBSTATE_SOPORIFIC | \
                      APP_SUBSTATE_SOPORIFIC_TERMINATING)))
/*! @brief Query if this state is a connectable state. */
#define appSmStateIsConnectable(state) \
    (0 == ((state) & (APP_SUBSTATE_SOPORIFIC_TERMINATING | \
                      APP_SUBSTATE_TERMINATING | \
                      APP_SUBSTATE_FACTORY_RESET | \
                      APP_SUBSTATE_DISCONNECTING)))

/*! Check whether we are allowed to create new BLE connections.

    New connections are allowed in the case and specifically when
    in a special DFU mode (Download Firmware Upgrade).

    Configuration options allow BLE to be used
    \li Out of the case

    And to block NEW BLE connections
    \li if in a call / playing music (not idle)
*/
#define appSmStateAreNewBleConnectionsAllowed(state) \
    (   appSmStateInCase(state) \
     || (   (appSmStateOutOfCase(state) || appSmStateInEar(state))\
         && (((state) & APP_SUBSTATE_IDLE) == APP_SUBSTATE_IDLE \
             || appConfigBleNewConnectionsWhenBusy()) \
         && appConfigBleAllowedOutOfCase()) \
     || (state) == APP_STATE_IN_CASE_DFU)

/*! Query if DFU is pending, or if we are in DFU mode 

    \return FALSE if no DFU is expected, TRUE otherwise */
extern bool appSmIsDfuPending(void);


/*! @brief Get the physical state as received from last update message. */
#define appSmGetPhyState() (appGetSm()->phy_state)

/*! @brief Query if pairing has been initiated by the user. */
#define appSmIsUserPairing() (appGetSm()->user_pairing)
/*! @brief Set user initiated pairing flag. */
#define appSmSetUserPairing()  (appGetSm()->user_pairing = TRUE)
/*! @brief Clear user initiated pairing flag. */
#define appSmClearUserPairing()  (appGetSm()->user_pairing = FALSE)


/* FUNCTIONS THAT CHECK THE STATE OF BLE FUNCTIONALITY */
/* Defined here as this is application-level knowledge */

/*! Does the application think that BLE advertising is taking place ?
    May not have yet started, or may have already ended */
#define appSmIsBleAdvertising()     (appGattAttemptingBleConnection())

/*! Do we have a BLE connection */
#define appSmHasBleConnection()     (appGattHasBleConnection())



/* FUNCTIONS TO INITIATE AN ACTION IN THE SM
 ********************************************/
/*! \brief Initiate pairing with a handset. */
extern void appSmPairHandset(void);
/*! \brief Delete paired handsets. */
extern void appSmDeleteHandsets(void);
/*! \brief Connect to paired handset. */
extern void appSmConnectHandset(void);
/*! \brief Request a factory reset. */
extern void appSmFactoryReset(void);
/*! \brief Reboot the earbud. */
extern void appSmReboot(void);
/*! \brief Initiate handover. */
extern void appSmInitiateHandover(void);

/*! \brief Enter the special upgrade state.

    This should be called to enter the upgrade (or DFU) mode immediately.

    \note The upgrade mode may be time limited. This is based on the values
          in the configuration (for instance \ref appConfigDfuTimeoutToPlaceInCaseMs).
 */
extern void appSmEnterDfuMode(void);

/*! \brief Enable disable entry to upgarde mode when entering the case 

    \param enable Enable or disable this mode
 */
extern void appSmEnterDfuModeInCase(bool enable);

#endif
