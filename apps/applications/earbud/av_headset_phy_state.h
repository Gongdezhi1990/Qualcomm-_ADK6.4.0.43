/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_phy_state.h
\brief	    Definition of the physical state of an Earbud.
*/

#ifndef AV_HEADSET_PHY_STATE_H
#define AV_HEADSET_PHY_STATE_H

#include "av_headset_message.h"
#include "av_headset_tasklist.h"

/*! Clients will receive state update messages via #PHY_STATE_CHANGED_IND
    conforming to the following state sequence.
    @startuml

    note "Transitions/events not shown are\n\
not supported and have no effect\n\
Earbud will remain in the current state.\
\n\n\
Dashed arrows illustrate the start of a\n\
multiple state transition via intermediate state.\n\
All activity associated with the intermediate state\n\
will be performed during the transition." as N1

    IN_CASE : Earbud is in the case
    IN_CASE -right-> OUT_OF_EAR : Remove from case\n appPhyStateOutOfCaseEvent()

    OUT_OF_EAR : Earbud is not in the case nor in the ear.
    OUT_OF_EAR -right-> IN_EAR : Put in ear\n appPhyStateInEarEvent()
    OUT_OF_EAR -left-> IN_CASE : Put in case\n appPhyStateInCaseEvent()
    OUT_OF_EAR -down-> OUT_OF_EAR_AT_REST : No motion for N seconds\n appPhyStateNotInMotionEvent()

    OUT_OF_EAR_AT_REST : Earbud is not moving
    OUT_OF_EAR_AT_REST -up-> OUT_OF_EAR : Motion detected\n appPhyStateMotionEvent()
    OUT_OF_EAR_AT_REST .up.> OUT_OF_EAR : Put in case\n appPhyStateInCaseEvent()\n Final state will be IN_CASE\nOR\nPut in Ear\nappPhyStateInEarEvent()\n Final state will be IN_EAR

    IN_EAR : Earbud is in the ear
    IN_EAR : Usable as a microphone and speaker.
    IN_EAR -right-> OUT_OF_EAR : Remove from ear\n appPhyStateOutOfEarEvent()
    IN_EAR .left.> OUT_OF_EAR : Put in case\n appPhyStateInCaseEvent()\n Final state will be IN_CASE

    @enduml
*/

/*! \brief Enumeration of the physical states an Earbud can be in.
 */
typedef enum
{
    /*! The earbud physical state is unknown.
        This state value will not be reported to clients. */
    PHY_STATE_UNKNOWN,
    /*! The earbud is in the case. */
    PHY_STATE_IN_CASE,
    /*! The earbud is not in the case nor is it in the ear.
     *  It *may* be in motion or at rest. */
    PHY_STATE_OUT_OF_EAR,
    /*! The earbud is not in the case not is it in the ear, and no motion
     * has been detected for configurable period of time */
    PHY_STATE_OUT_OF_EAR_AT_REST,
    /*! The earbud is in the ear and usuable as a microphone and speaker. */
    PHY_STATE_IN_EAR
} phyState;

/*! \brief Physial State module state. */
typedef struct
{
    /*! Physical State module message task. */
    TaskData task;
    /*! Current physical state of the device. */
    phyState state;
    /*! List of tasks to receive #PHY_STATE_CHANGED_IND notifications. */
    TaskList* client_tasks;
    /*! Stores the motion state */
    bool in_motion;
    /*! Stores the proximity state */
    bool in_proximity;
    /*! Lock used to conditionalise sending of PHY_STATE_INIT_CFM. */
    uint16 lock;
} phyStateTaskData;

/*! \brief Messages which may be sent by the Physical State module. */
typedef enum av_headset_phy_state_messages
{
    /*! Initialisation of phy state is complete. */
    PHY_STATE_INIT_CFM = PHY_STATE_MESSAGE_BASE,
    /*! Indication of a changed physical state */
    PHY_STATE_CHANGED_IND,
};

/*! \brief Definition of #PHY_STATE_CHANGED_IND message. */
typedef struct
{
    /*! The physical state which the device is now in. */
    phyState new_state;
} PHY_STATE_CHANGED_IND_T;

/*! \brief Internal messages the physical state module can send itself. */
typedef enum
{
    PHY_STATE_INTERNAL_IN_CASE_EVENT,
    PHY_STATE_INTERNAL_OUT_OF_CASE_EVENT,
    PHY_STATE_INTERNAL_IN_EAR_EVENT,
    PHY_STATE_INTERNAL_OUT_OF_EAR_EVENT,
    PHY_STATE_INTERNAL_MOTION,
    PHY_STATE_INTERNAL_NOT_IN_MOTION,
};

/*! \brief Register a task for notification of changes in state.
    @param[in] client_task Task to receive PHY_STATE_CHANGED_IND messages.
 */
extern void appPhyStateRegisterClient(Task client_task);

/*! \brief Unregister a task for notification of changes in state.
    @param[in] client_task Task to unregister.
 */
extern void appPhyStateUnregisterClient(Task client_task);

/*! \brief Get the current physical state of the device.
    \return phyState Current physical state of the device.
*/
extern phyState appPhyStateGetState(void);

/*! \brief Handle notification that Earbud is now in the case. */
extern void appPhyStateInCaseEvent(void);

/*! \brief Handle notification that Earbud is now out of the case. */
extern void appPhyStateOutOfCaseEvent(void);

/*! \brief Handle notification that Earbud is now in ear. */
extern void appPhyStateInEarEvent(void);

/*! \brief Handle notification that Earbud is now out of the ear. */
extern void appPhyStateOutOfEarEvent(void);

/*! \brief Handle notification that Earbud is now moving */
extern void appPhyStateMotionEvent(void);

/*! \brief Handle notification that Earbud is now not moving. */
extern void appPhyStateNotInMotionEvent(void);

/*! \brief Tell the phy state module to prepare for entry to dormant.
           Phy state unregisters itself as a client of all sensors which (if
           phy state is the only remaining client), will cause the sensors to
           switch off or enter standby.
 */
extern void appPhyStatePrepareToEnterDormant(void);

/*! \brief Initialise the module.
    \note #PHY_STATE_INIT_CFM is sent when the phy state is known.
*/
extern void appPhyStateInit(void);

#endif /* AV_HEADSET_PHY_STATE_H */

