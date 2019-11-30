/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       main.c
\brief      Main application task
*/

#include <hfp.h>
#include <a2dp.h>
#include <avrcp.h>
#include <connection.h>
#include <panic.h>
#include <pio.h>
#include <ps.h>
#include <string.h>
#include <boot.h>
#include <os.h>
#include <logging.h>
#include <app/message/system_message.h>

#include "av_headset.h"
#include "av_headset_auth.h"
#include "av_headset_ui.h"
#include "av_headset_pairing.h"
#include "av_headset_scan_manager.h"
#include "av_headset_anc_tuning.h"


/*! Application data structure */
appTaskData globalApp;

/*! \brief Handle subsystem event report. */
static void appHandleSubsystemEventReport(MessageSubsystemEventReport *evt)
{
    UNUSED(evt);
    DEBUG_LOGF("appHandleSubsystemEventReport, ss_id=%d, level=%d, id=%d, cpu=%d, occurrences=%d, time=%d",
        evt->ss_id, evt->level, evt->id, evt->cpu, evt->occurrences, evt->time);
}

/*! \brief System Message Handler

    This function is the message handler for system messages. They are 
    routed to existing handlers. If execution reaches the end of the 
    function then it is assumed that the message is unhandled.
*/
static void appHandleSystemMessage(Task task, MessageId id, Message message)
{
    bool handled = FALSE;

    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
        case MESSAGE_SUBSYSTEM_EVENT_REPORT:
            appHandleSubsystemEventReport((MessageSubsystemEventReport *)message);
            return;

        case MESSAGE_IMAGE_UPGRADE_ERASE_STATUS:
        case MESSAGE_IMAGE_UPGRADE_COPY_STATUS:
        case MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS:
        case MESSAGE_IMAGE_UPGRADE_HASH_ALL_SECTIONS_UPDATE_STATUS:
            handled = appUpgradeHandleSystemMessages(id, message, FALSE);
            break;

        case MESSAGE_USB_ENUMERATED:
            appAncTuningEnumerated();
            return;

        case MESSAGE_USB_SUSPENDED:
            appAncTuningSuspended();
            return;

        default:
            break;
    }

    if (!handled)
    {
        appHandleSysUnexpected(id);
    }
}

/*  Handler for the INIT_CFM message.

    Used to register the handler that decides whether to allow entry
    to low power mode, before passing the #INIT_CFM message to 
    the state machine handler.

    \param message The INIT_CFM message received (if any).
 */
static void appHandleInitCfm(Message message)
{
    appSmHandleMessage(appGetSmTask(), INIT_CFM, message);
}


/*! \brief Message Handler

    This function is the main message handler for the main application task, every
    message is handled in it's own seperate handler function.  The switch
    statement is broken into seperate blocks to reduce code size, if execution
    reaches the end of the function then it is assumed that the message is
    unhandled.
*/    
static void appHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);

    if (id == appInitWaitingForMessageId())
    {
        appInitHandleMessage(id, message);
        return;
    }

    /* Handle Connection Library messages that are not sent directly to
       the requestor */
    if (CL_MESSAGE_BASE <= id && id < CL_MESSAGE_TOP)
    {
        bool handled = FALSE;

        /* Pass connection library messages in turn to the modules that 
           are interested in them.
         */
        handled |= appPairingHandleConnectionLibraryMessages(id, message, handled);
        handled |= appConManagerHandleConnectionLibraryMessages(id, message, handled);
        handled |= appLinkPolicyHandleConnectionLibraryMessages(id, message, handled);
        handled |= appAuthHandleConnectionLibraryMessages(id, message, handled);
        handled |= appAdvManagerHandleConnectionLibraryMessages(id, message, handled);
        handled |= appGattHandleConnectionLibraryMessages(id, message, handled);
        handled |= appTestHandleConnectionLibraryMessages(id, message, handled);

        if (handled)
        {
            return;
        }
    }

    switch (id)
    {
            /* AV messages */
        case AV_CREATE_IND:
        case AV_DESTROY_IND:
            return;

        case INIT_CFM:
            appHandleInitCfm(message);
            return;
    }

    appHandleUnexpected(id);
}

extern void _init(void);
void _init(void)
{
    appAncTuningEarlyInit();
}

/*! \brief Application entry point

    This function is the entry point for the application, it performs basic
    initialisation of the application task state machine and then sets
    the state to 'initialising' which will start the initialisation procedure.

    \returns Nothing. Only exits by powering down.
*/
int main(void)
{
    OsInit();

    /* Set up task handlers */
    appGetApp()->task.handler = appHandleMessage;
    appGetApp()->systask.handler = appHandleSystemMessage;

    MessageSystemTask(appGetSysTask());

    /* Start the application module and library initialisation sequence */
    appInit();

    /* Start the message scheduler loop */
    MessageLoop();

    /* We should never get here, keep compiler happy */
    return 0;
}

/* Define pmalloc pools. This definition will be merged at run time with the
   base definitions from Hydra OS - see 'pmalloc_config_P1.h'. */

#include <pmalloc.h>

_Pragma ("unitsuppress Unused")

_Pragma ("datasection apppool")

static const pmalloc_pool_config app_pools[] =
{
    {   4, 14 },
    {   8, 25 },
    {  12, 17 },
    {  16,  7 },
    {  20, 22 },
    {  24, 15 },
    {  28, 55 },
    {  32, 21 },
    {  36, 19 },
    {  40, 10 },
    {  56,  9 },
    {  64,  7 },
    {  80,  9 },
    { 120, 16 },
    { 140,  4 },
    { 180,  6 },
    { 220,  3 },
    { 288,  1 }, 
    { 512,  1 },
    { 692,  3 }    
};
