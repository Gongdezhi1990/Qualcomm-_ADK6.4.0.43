/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_phy_state.c
\brief      Manage physical state of an Earbud.
*/

#include "av_headset.h"
#include "av_headset_phy_state.h"
#include "av_headset_log.h"

#include <panic.h>

/*! Message creation macro for phyiscal state module. */
#define MAKE_PHYSTATE_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

//!@{ @name Bits in lock used to wait for all initial measurements to complete.
#define PHY_STATE_LOCK_EAR 1
#define PHY_STATE_LOCK_MOTION 2
#define PHY_STATE_LOCK_CASE 4
//!@}

/*! Clear bit in the lock */
#define appPhyStateClearLockBit(bit) \
do {                                 \
    appGetPhyState()->lock &= ~bit;  \
} while(0)

void appPhyStateSetState(phyStateTaskData* phy_state, phyState new_state);

/*! \brief Send a PHY_STATE_CHANGED_IND message to all registered client tasks.
 */
static void appPhyStateMsgSendStateChangedInd(phyState state)
{
    phyStateTaskData* phy_state = appGetPhyState();

    MAKE_PHYSTATE_MESSAGE(PHY_STATE_CHANGED_IND);
    message->new_state = state;
    appTaskListMessageSend(phy_state->client_tasks, PHY_STATE_CHANGED_IND, message);
}

/*! \brief Perform actions on entering PHY_STATE_IN_CASE state. */
static void appPhyStateEnterInCase(void)
{
    DEBUG_LOG("appPhyStateEnterInCase");
    appPhyStateMsgSendStateChangedInd(PHY_STATE_IN_CASE); 
}

/*! \brief Perform actions on entering PHY_STATE_OUT_OF_EAR state. */
static void appPhyStateEnterOutOfEar(void)
{
    DEBUG_LOG("appPhyStateEnterOutOfEar");
    appPhyStateMsgSendStateChangedInd(PHY_STATE_OUT_OF_EAR); 
}

/*! \brief Perform actions on entering PHY_STATE_IN_EAR state. */
static void appPhyStateEnterInEar(void)
{
    DEBUG_LOG("appPhyStateEnterInEar");
    appPhyStateMsgSendStateChangedInd(PHY_STATE_IN_EAR); 
}

/*! \brief Perform actions on entering PHY_STATE_OUT_OF_EAR_AT_REST state. */
static void appPhyStateEnterOutOfEarAtRest(void)
{
    DEBUG_LOG("appPhyStateEnterOutOfEarAtRest");
    appPhyStateMsgSendStateChangedInd(PHY_STATE_OUT_OF_EAR_AT_REST); 
}

/*! \brief Perform actions on exiting PHY_STATE_UNKNOWN state. */
static void appPhyStateExitUnknown(void)
{
}

/*! \brief Perform actions on exiting PHY_STATE_IN_CASE state. */
static void appPhyStateExitInCase(void)
{
}

/*! \brief Perform actions on exiting PHY_STATE_OUT_OF_EAR state. */
static void appPhyStateExitOutOfEar(void)
{
}

/*! \brief Perform actions on exiting PHY_STATE_IN_EAR state. */
static void appPhyStateExitInEar(void)
{
}

/*! \brief Perform actions on exiting PHY_STATE_OUT_OF_EAR_AT_REST state. */
static void appPhyStateExitOutOfEarAtRest(void)
{
}

static void appPhyStateHandleBadState(phyState phy_state)
{
    UNUSED(phy_state);
    DEBUG_LOGF("appPhyStateHandleBadState %d", phy_state);
    Panic();
}

/*! \brief Handle notification that Earbud is now in the case. */
static void appPhyStateHandleInternalInCaseEvent(void)
{
    phyStateTaskData *phy_state = appGetPhyState();

    switch (phy_state->state)
    {
        /* Already in the case, why are we getting told again? */
        case PHY_STATE_IN_CASE:
            break;

        /* Transition through PHY_STATE_OUT_OF_EAR before transitioning
           to PHY_STATE_IN_CASE */
        case PHY_STATE_OUT_OF_EAR_AT_REST:
        case PHY_STATE_IN_EAR:
            appPhyStateSetState(appGetPhyState(), PHY_STATE_OUT_OF_EAR);
            /* FALLTHROUGH */

        case PHY_STATE_UNKNOWN:
        case PHY_STATE_OUT_OF_EAR:
            appPhyStateSetState(appGetPhyState(), PHY_STATE_IN_CASE);
            break;

        default:
            appPhyStateHandleBadState(phy_state->state);
            break;
    }
}

/*! \brief Handle notification that Earbud is now out of the case. */
static void appPhyStateHandleInternalOutOfCaseEvent(void)
{
    phyStateTaskData *phy_state = appGetPhyState();

    switch (phy_state->state)
    {
        /* Already out of the case, why are we being told again? */
        case PHY_STATE_OUT_OF_EAR:
        case PHY_STATE_OUT_OF_EAR_AT_REST:
        case PHY_STATE_IN_EAR:
            break;

        /* Transition based on the stored sensor states */
        case PHY_STATE_IN_CASE:
        case PHY_STATE_UNKNOWN:
            appPhyStateSetState(appGetPhyState(), PHY_STATE_OUT_OF_EAR);
            if (phy_state->in_proximity)
            {
                appPhyStateSetState(appGetPhyState(), PHY_STATE_IN_EAR);
            }
            else if (!phy_state->in_motion)
            {
                appPhyStateSetState(appGetPhyState(), PHY_STATE_OUT_OF_EAR_AT_REST);
            }
            break;

        default:
            appPhyStateHandleBadState(phy_state->state);
            break;
    }
}

/*! \brief Handle notification that Earbud is now in ear. */
static void appPhyStateHandleInternalInEarEvent(void)
{
    phyStateTaskData *phy_state = appGetPhyState();

    /* Save state to use in determining state on further events */
    phy_state->in_proximity = TRUE;

    switch (phy_state->state)
    {
        /* Until there is an out of case event, the state should remain in-case */
        case PHY_STATE_IN_CASE:
            break;

        /* Already out of the case, why are we being told again? */
        case PHY_STATE_IN_EAR:
            break;

        /* Transition through PHY_STATE_OUT_OF_EAR before transitioning
           to PHY_STATE_IN_EAR */
        case PHY_STATE_OUT_OF_EAR_AT_REST:
            appPhyStateSetState(appGetPhyState(), PHY_STATE_OUT_OF_EAR);
            // FALLTHROUGH

        case PHY_STATE_UNKNOWN:
        case PHY_STATE_OUT_OF_EAR:
            appPhyStateSetState(appGetPhyState(), PHY_STATE_IN_EAR);
            break;

        default:
            appPhyStateHandleBadState(phy_state->state);
            break;
    }
}

/*! \brief Handle notification that Earbud is now out of the ear. */
static void appPhyStateHandleInternalOutOfEarEvent(void)
{
    phyStateTaskData *phy_state = appGetPhyState();

    /* Save state to use in determining state on further events */
    phy_state->in_proximity = FALSE;

    switch (phy_state->state)
    {
        /* Until there is an out of case event, the state should remain in-case */
        case PHY_STATE_IN_CASE:
            break;

        /* Already out of ear, why are we being told again? */
        case PHY_STATE_OUT_OF_EAR:
        case PHY_STATE_OUT_OF_EAR_AT_REST:
            break;

        case PHY_STATE_UNKNOWN:
        case PHY_STATE_IN_EAR:
            appPhyStateSetState(appGetPhyState(), PHY_STATE_OUT_OF_EAR);
            if (!phy_state->in_motion)
            {
                appPhyStateSetState(appGetPhyState(), PHY_STATE_OUT_OF_EAR_AT_REST);
            }
            break;

        default:
            appPhyStateHandleBadState(phy_state->state);
            break;
    }
}

/*! \brief Handle notification that Earbud is now moving */
static void appPhyStateHandleInternalMotionEvent(void)
{
    phyStateTaskData *phy_state = appGetPhyState();

    /* Save motion state to use in determining state on further events */
    phy_state->in_motion = TRUE;

    switch (phy_state->state)
    {
        /* Don't care */
        case PHY_STATE_OUT_OF_EAR:
        case PHY_STATE_IN_CASE:
        case PHY_STATE_IN_EAR:
            break;

        /* Motion event alone does not provide enough information to determine correct state */
        case PHY_STATE_UNKNOWN:
            break;

        /* Earbud was moved while not in ear, when previously not in motion */
        case PHY_STATE_OUT_OF_EAR_AT_REST:
            appPhyStateSetState(appGetPhyState(), PHY_STATE_OUT_OF_EAR);
            break;

        default:
            appPhyStateHandleBadState(phy_state->state);
            break;
    }
}

/*! \brief Handle notification that Earbud is now not moving. */
static void appPhyStateHandleInternalNotInMotionEvent(void)
{
    phyStateTaskData *phy_state = appGetPhyState();
    /* Save motion state to use in determining state on further events */
    phy_state->in_motion = FALSE;

    switch (phy_state->state)
    {
        /* Don't care */
        case PHY_STATE_OUT_OF_EAR_AT_REST:
        case PHY_STATE_IN_CASE:
        case PHY_STATE_IN_EAR:
            break;

        /* Motion event alone does not provide enough information to determine correct state */
        case PHY_STATE_UNKNOWN:
            break;

        /* Earbud out of ear and stopped moving */
        case PHY_STATE_OUT_OF_EAR:
            appPhyStateSetState(appGetPhyState(), PHY_STATE_OUT_OF_EAR_AT_REST);
            break;

        default:
            appPhyStateHandleBadState(phy_state->state);
            break;
    }
}

/*! \brief Physical State module message handler. */
static void appPhyStateHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
        case PHY_STATE_INTERNAL_IN_CASE_EVENT:
            appPhyStateHandleInternalInCaseEvent();
            break;
        case PHY_STATE_INTERNAL_OUT_OF_CASE_EVENT:
            appPhyStateHandleInternalOutOfCaseEvent();
            break;
        case PHY_STATE_INTERNAL_IN_EAR_EVENT:
            appPhyStateHandleInternalInEarEvent();
            break;
        case PHY_STATE_INTERNAL_OUT_OF_EAR_EVENT:
            appPhyStateHandleInternalOutOfEarEvent();
            break;
        case PHY_STATE_INTERNAL_MOTION:
            appPhyStateHandleInternalMotionEvent();
            break;
        case PHY_STATE_INTERNAL_NOT_IN_MOTION:
            appPhyStateHandleInternalNotInMotionEvent();
            break;

        case CHARGER_MESSAGE_ATTACHED:
            appPhyStateInCaseEvent();
            break;
        case CHARGER_MESSAGE_DETACHED:
            appPhyStateOutOfCaseEvent();
            break;

        case ACCELEROMETER_MESSAGE_IN_MOTION:
            appPhyStateMotionEvent();
            break;
        case ACCELEROMETER_MESSAGE_NOT_IN_MOTION:
            appPhyStateNotInMotionEvent();
            break;
        case PROXIMITY_MESSAGE_IN_PROXIMITY:
            appPhyStateInEarEvent();
            break;
        case PROXIMITY_MESSAGE_NOT_IN_PROXIMITY:
            appPhyStateOutOfEarEvent();
            break;

        default:
            DEBUG_LOGF("Unknown message received, id:%d", id);
            break;
    }
}

/*! \brief Register a task for notification of changes in state.
 */
void appPhyStateRegisterClient(Task client_task)
{
    phyStateTaskData* phy_state = appGetPhyState();

    DEBUG_LOGF("appPhyStateRegisterClient %p", client_task);

    appTaskListAddTask(phy_state->client_tasks, client_task);
}

void appPhyStateUnregisterClient(Task client_task)
{
    phyStateTaskData* phy_state = appGetPhyState();

    DEBUG_LOGF("appPhyStateUnregisterClient %p", client_task);

    appTaskListRemoveTask(phy_state->client_tasks, client_task);
}

void appPhyStateInit(void)
{
    phyStateTaskData* phy_state = appGetPhyState();

    DEBUG_LOG("appPhyStateInit");

    phy_state->task.handler = appPhyStateHandleMessage;
    phy_state->state = PHY_STATE_UNKNOWN;
    phy_state->client_tasks = appTaskListInit();
    phy_state->in_motion = FALSE;
    phy_state->in_proximity = FALSE;

/* Not registering as a client of the charger means no charger state messages
will be received. This means the in-case state can never be entered */
#ifndef DISABLE_IN_CASE_PHY_STATE
    appChargerClientRegister(&phy_state->task);
#else
    /* Without charger messages, remain out of case */
    MessageSend(&phy_state->task, CHARGER_MESSAGE_DETACHED, NULL);
#endif

    if (!appProximityClientRegister(&phy_state->task))
    {
        DEBUG_LOG("appPhyStateRegisterClient unable to register with proximity");
        /* Without proximity detection, assume the device is always in-ear */
        MessageSend(&phy_state->task, PROXIMITY_MESSAGE_IN_PROXIMITY, NULL);
    }
    if (!appAccelerometerClientRegister(&phy_state->task))
    {
        DEBUG_LOG("appPhyStateRegisterClient unable to register with accelerometer");
        /* Without accelerometer motion detection, assume the device is always moving */
        MessageSend(&phy_state->task, ACCELEROMETER_MESSAGE_IN_MOTION, NULL);
    }
    /* Supported sensors will send initial messages when the first measurement is made */
    phy_state->lock = PHY_STATE_LOCK_EAR | PHY_STATE_LOCK_MOTION | PHY_STATE_LOCK_CASE;
    MessageSendConditionally(appGetAppTask(), PHY_STATE_INIT_CFM, NULL, &phy_state->lock);
}

void appPhyStatePrepareToEnterDormant(void)
{
    phyStateTaskData* phy_state = appGetPhyState();

    DEBUG_LOG("appPhyStatePrepareToEnterDormant");
    appProximityClientUnregister(&phy_state->task);
    appAccelerometerClientUnregister(&phy_state->task);
}

/*! \brief Handle state transitions.

    \param  phy_state   Pointer to the task data for this module.
    \param  new_state   State to set.
 */
void appPhyStateSetState(phyStateTaskData* phy_state, phyState new_state)
{
    switch (phy_state->state)
    {
        case PHY_STATE_UNKNOWN:
            appPhyStateExitUnknown();
            break;
        case PHY_STATE_IN_CASE:
            appPhyStateExitInCase();
            break;
        case PHY_STATE_OUT_OF_EAR:
            appPhyStateExitOutOfEar();
            break;
        case PHY_STATE_OUT_OF_EAR_AT_REST:
            appPhyStateExitOutOfEarAtRest();
            break;
        case PHY_STATE_IN_EAR:
            appPhyStateExitInEar();
            break;
        default:
            appPhyStateHandleBadState(phy_state->state);
            break;
    }

    phy_state->state = new_state;

    switch (phy_state->state)
    {
        case PHY_STATE_IN_CASE:
            appPhyStateEnterInCase();
            break;
        case PHY_STATE_OUT_OF_EAR:
            appPhyStateEnterOutOfEar();
            break;
        case PHY_STATE_OUT_OF_EAR_AT_REST:
            appPhyStateEnterOutOfEarAtRest();
            break;
        case PHY_STATE_IN_EAR:
            appPhyStateEnterInEar();
            break;
        default:
            appPhyStateHandleBadState(phy_state->state);
            break;
    }

}

/*! \brief Get the current physical state of the device.
    \return phyState Current physical state of the device.
*/
phyState appPhyStateGetState(void)
{
    phyStateTaskData* phy_state = appGetPhyState();
    return phy_state->state;
}

/*! \brief Handle notification that Earbud is now in the case. */
void appPhyStateInCaseEvent(void)
{
    phyStateTaskData* phy_state = appGetPhyState();
    MessageCancelAll(&phy_state->task, PHY_STATE_INTERNAL_IN_CASE_EVENT);
    MessageSend(&phy_state->task, PHY_STATE_INTERNAL_IN_CASE_EVENT, NULL);
    appPhyStateClearLockBit(PHY_STATE_LOCK_CASE);
}

/*! \brief Handle notification that Earbud is now out of the case. */
void appPhyStateOutOfCaseEvent(void)
{
    phyStateTaskData* phy_state = appGetPhyState();
    MessageCancelAll(&phy_state->task, PHY_STATE_INTERNAL_OUT_OF_CASE_EVENT);
    MessageSend(&phy_state->task, PHY_STATE_INTERNAL_OUT_OF_CASE_EVENT, NULL);
    appPhyStateClearLockBit(PHY_STATE_LOCK_CASE);
}

/*! \brief Handle notification that Earbud is now in ear. */
void appPhyStateInEarEvent(void)
{
    phyStateTaskData* phy_state = appGetPhyState();
    MessageCancelAll(&phy_state->task, PHY_STATE_INTERNAL_IN_EAR_EVENT);
    MessageSend(&phy_state->task, PHY_STATE_INTERNAL_IN_EAR_EVENT, NULL);
    appPhyStateClearLockBit(PHY_STATE_LOCK_EAR);
}

/*! \brief Handle notification that Earbud is now out of the ear. */
void appPhyStateOutOfEarEvent(void)
{
    phyStateTaskData* phy_state = appGetPhyState();
    MessageCancelAll(&phy_state->task, PHY_STATE_INTERNAL_OUT_OF_EAR_EVENT);
    MessageSend(&phy_state->task, PHY_STATE_INTERNAL_OUT_OF_EAR_EVENT, NULL);
    appPhyStateClearLockBit(PHY_STATE_LOCK_EAR);
}

/*! \brief Handle notification that Earbud is now moving */
void appPhyStateMotionEvent(void)
{
    phyStateTaskData* phy_state = appGetPhyState();
    MessageCancelAll(&phy_state->task, PHY_STATE_INTERNAL_MOTION);
    MessageSend(&phy_state->task, PHY_STATE_INTERNAL_MOTION, NULL);
    appPhyStateClearLockBit(PHY_STATE_LOCK_MOTION);
}

/*! \brief Handle notification that Earbud is now not moving. */
void appPhyStateNotInMotionEvent(void)
{
    phyStateTaskData* phy_state = appGetPhyState();
    MessageCancelAll(&phy_state->task, PHY_STATE_INTERNAL_NOT_IN_MOTION);
    MessageSend(&phy_state->task, PHY_STATE_INTERNAL_NOT_IN_MOTION, NULL);
    appPhyStateClearLockBit(PHY_STATE_LOCK_MOTION);
}
