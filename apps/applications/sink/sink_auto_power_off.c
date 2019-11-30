/*
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

*/
/**
\file
\ingroup sink_app
\brief
    This file handles all auto power off funcationality.
*/

#include "sink_auto_power_off.h"
#include "sink_private_data.h"
#include "sink_main_task.h"

#include "sink_statemanager.h"
#include "sink_gatt_client.h"
#include "sink_gatt_server.h"
#include "sink_ba.h"
#include "sink_a2dp.h"
#include "sink_gaia.h"


#ifdef DEBUG_AUTO_POWER_OFF
#define AUTO_POWER_OFF_DEBUG(x) DEBUG(x)
#else
#define AUTO_POWER_OFF_DEBUG(x)
#endif

/*************************************************************************
NAME
    sinkHandleAutoPowerOff

DESCRIPTION
    This function determines whether to send power off immediately or need to reset the 
    auto power off timer again.
    Depends the device state it clear/updates the auto_power_off bit mask value based on 
    eithr BLE connection status or silence detection during media streaming.

RETURNS
    TRUE for postponed the auto power off, FALSE otherwise

**************************************************************************/
bool sinkHandleAutoPowerOff(Task task, sinkState lState)
{
    /* Intialise for not do shutdown operation. */
    bool shutdown = FALSE; 

    AUTO_POWER_OFF_DEBUG(("APO: sinkHandleAutoPowerOff UE state [%d]\n", lState));

    AUTO_POWER_OFF_DEBUG(("APO: Auto S Off[%d] sec elapsed\n" , sinkDataGetAutoSwitchOffTimeout())) ;

    switch ( lState )
    {
        case deviceLimbo:
        case deviceConnectable:
        case deviceConnDiscoverable:
        case deviceConnected:
            /* Allow auto power off in all the above device states. since theSink.routed_audio NULL and this will set the 
              * shutdown to TRUE and therefore there is no BREAK here and allow to FALL TRHOUGH */
        case deviceA2DPStreaming:
            /* Is connected to any Audio Source. */
            if ( sinkAudioIsAudioRouted() )
            {
                if(sinkAudioIsSilenceDetected())
                {
                    /* Media is connected to source but while streaming silence detected.*/
                    shutdown = TRUE;
                }
             }
            else
            {
                /* Media is not streaming through any of the audio source so allow auto power off. */
                 shutdown = TRUE;
            }
            break;

         /* Don't allow auto power off while during call. */
        case deviceOutgoingCallEstablish:
        case deviceIncomingCallEstablish:
        case deviceActiveCallSCO:
        case deviceActiveCallNoSCO:
        case deviceTestMode:
        case deviceThreeWayCallWaiting:
        case deviceThreeWayCallOnHold:
        case deviceThreeWayMulticall:
        case deviceIncomingCallOnHold:
            break ;

        default:
            break ;
    }

    /* Before power down check is any BLE connection exist. */
    if(shutdown)
    {
    }

#ifdef ENABLE_GAIA
    /* Look for GAIA connection exist or not. */
    if(shutdown)
    {
        /* Don't allow shutdown if it is connected to any GAIA application. */
        if((gaia_get_transport() != NULL) && (!gaia_get_remain_connected()))
        {
            shutdown = FALSE;
        }
    }
#endif

    /* If application is not in the process of link loss recovery, 
     * then process automatic power-down request else postponed 
     */
#ifdef ENABLE_PEER
    if(getA2dpPeerLinkLossReconnect())
    {
        /*TWS linkloss recovery is in progress,hence postpone
          the automatic powerdown*/      
        shutdown = FALSE;        
    }
#endif
    if (shutdown)
    {
        AUTO_POWER_OFF_DEBUG(("APO: Sending EventUsrPowerOff\n"));

        MessageSend(task, EventUsrPowerOff , 0);
        return FALSE;
    }
    return TRUE;      /* Postponed the Auto Power Off. */
}

/*************************************************************************
NAME
    sinkStartAutoPowerOffTimer

DESCRIPTION
    This function starts the auto power OFF timer. This cancels all the previous 
    EventSysAutoSwitchOff messages and sends a new one after AutoSwitchOffTime_s
    delay.

RETURNS
    VOID

**************************************************************************/

void sinkStartAutoPowerOffTimer(void)
{
    AUTO_POWER_OFF_DEBUG(("APO: Starting auto power OFF timer \n"));
    sinkCancelAndIfDelaySendLater(EventSysAutoSwitchOff, D_SEC(sinkDataGetAutoSwitchOffTimeout()));
}

/*************************************************************************
NAME
    sinkStopAutoPowerOffTimer

DESCRIPTION
    This function stops the auto power OFF timer. This cancels all the  
    EventSysAutoSwitchOff messages sent.

RETURNS
    VOID

**************************************************************************/

void sinkStopAutoPowerOffTimer(void)
{
    AUTO_POWER_OFF_DEBUG(("APO: Stopping auto power OFF timer \n"));
    MessageCancelAll(&theSink.task, EventSysAutoSwitchOff);
}

