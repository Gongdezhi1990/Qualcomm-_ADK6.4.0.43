/****************************************************************************
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_hid_remote_control.c

DESCRIPTION
    Routines to handle the GATT HID Service.
*/

#include "sink_debug.h"
#include <stdlib.h>

#include "sink_configmanager.h"
#include "sink_utils.h"
#include "sink_ble.h"
#include "sink_gatt_client.h"
#include "sink_gatt_client_hid.h"

#include "sink_gatt_hid_remote_control.h"

#ifdef GATT_HID_REMOTE_CONTROL
#include "sink_input_manager.h"
#include "sink_gatt_hid_remote_control_config_def.h"

#ifdef DEBUG_GATT_HID_RC
#define GATT_HID_RC_DEBUG(x) DEBUG(x)
#else
#define GATT_HID_RC_DEBUG(x)
#endif

/*
    HID RC task global data
*/
typedef struct
{
    TaskData task; /* The task that is responsible for communicating with the BLE HID RC */

}gattHidRc_global_data_t;

/* Global data structure element for HID RC module */
static gattHidRc_global_data_t gattHidRc;
#define GATT_HID_RC_GDATA  gattHidRc


/* Data structure for the HID report map parser */
typedef struct
{
    uint16 usage_page;
    uint16 usage;
    uint16 size_fragment;
    uint8 fragment[USB_HID_MAP_ITEM_MAX_SIZE];
} map_parser_t;

static map_parser_t *parser;


/*******************************************************************************
NAME
    gattHidRcCreateButtonEventMessage

DESCRIPTION
    Create and send button related message events to Remote control message handler.

PARAMETERS
    mid             Messge ID that to be send
    timer           If any timer associated
    mask           Button Mask
    cid             Connection ID
    delay_time  delay

*/
static void gattHidRcCreateButtonEventMessage(gattHidRcMessageID_t mid,
            gattHidRcTimerID_t timer,
            uint16 mask,
            uint16 cid, uint32 delay_time)
{
    /* Create a BLE_HID_RC_BUTTON_EVENT_MSG_T,
     * this data type is generic for all "button event messages"
    * sent to the Input monitor
    */
    MAKE_GATT_HID_RC_MESSAGE(GATT_HID_RC_BUTTON_EVENT_MSG);
    message->timer = timer;
    message->mask = mask;
    message->cid = cid;

    /* Dispatch the message */
    MessageSendLater(&GATT_HID_RC_GDATA.task, mid, message, delay_time);
}

/*******************************************************************************
NAME
    gattHidRcGetConnectedRemote

DESCRIPTION
    Utility function for getting the connected active remotes for a CID.

PARAMETERS
    cid      Connection ID for the remote

RETURNS
    gattHidRcDevData_t Type remote control pointer if there is a remote assiciated with given CID

*/
static gattHidRcDevData_t * gattHidRcGetConnectedRemote(uint16 cid)
{
    gatt_client_services_t *client_services = NULL;
    /* Get the connection from cid */
    gatt_client_connection_t *connection = gattClientFindByCid(cid);
    client_services = gattClientGetServiceData(connection);
    /*if client service is available and there exists a HID client ,then return remote */
    if(client_services && client_services->remote)
        return client_services->remote;

    return NULL;
}

/*******************************************************************************
NAME
    gattHidRcIsConnected

DESCRIPTION
    Utility function checking if remote control is connected

PARAMETERS
    conn      Pointer to the client connection instance

RETURNS
    TRUE if connected else FALSE

*/
static  bool gattHidRcIsConnected(gatt_client_connection_t * conn)
{
    gatt_client_services_t *client_services = NULL;
    client_services = gattClientGetServiceData(conn);

    if(client_services && client_services->remote)
        return TRUE; /* The connected device is remote control */

    return FALSE; /* Looks like the connected device isn't a remote control */
}

/*******************************************************************************
NAME
    handleHidRcButtonEventMessages

DESCRIPTION
    Handle Button related Remote Control Events .

PARAMETERS
    messageId     ID of the message that to be handled
    button_event  Button event associated with this handler

*/
static void handleHidRcButtonEventMessages(gattHidRcMessageID_t messageId,
                GATT_HID_RC_BUTTON_EVENT_MSG_T * button_event)
{
    /* Get the active remote data */
    gattHidRcDevData_t *remote = sinkGattHidRcGetConnectedRemote(button_event->cid);
    timerConfig_t InputTimers;

    inputManagerGetInputTimers(&InputTimers);

    if(!remote)
    {
        GATT_HID_RC_DEBUG(("GattHIDRc: Func()handleHidRcButtonEventMessages, No Device with CID = %x\n",button_event->cid));
        return;
    }

    GATT_HID_RC_DEBUG(("GattHIDRc: Func()handleHidRcButtonEventMessages Entry MsgId[%x] \n",messageId));
    GATT_HID_RC_DEBUG(("GattHIDRc: rc_button_state = %d currentmask[%d] remotemask[%d] \n",remote->rc_button_state,button_event->mask,remote->rc_button_mask));


    switch(messageId)
    {
        case GATT_HID_RC_BUTTON_DOWN_MSG:
        {
            if(remote->rc_button_state == inputNotDown)
            {
                /* Update the state for the input(s) and keep track of which button has been pressed down */
                remote->rc_buttons_down = bitCounter16(button_event->mask);
                remote->rc_button_state = inputMultipleDetect;
                remote->rc_button_mask = button_event->mask;
                /* Start the M_DOWN button timer to allow multiple buttons to be detected;
                 * once this timer fires, multiple buttons will not be detected
                 */
                gattHidRcCreateButtonEventMessage( GATT_HID_RC_BUTTON_TIMER_MSG,
                                                                        GATT_HID_RC_TIMER_MULTIPLE, 0, button_event->cid,
                                                                        InputTimers.multipleDetectTimer);
            }
            else if (remote->rc_button_state == inputMultipleDetect)
            {
                /* Add the newly pressed button to the mask to keep
                 *  track of which multiple buttons are currently pressed down
                 */
                remote->rc_button_mask |= button_event->mask;
                remote->rc_buttons_down += bitCounter16(button_event->mask);
            }
            else
            {
                /* Ignore the new button press */
            }

        }
        break;

        case GATT_HID_RC_BUTTON_TIMER_MSG:
        {
            GATT_HID_RC_DEBUG(("GattHIDRc: GATT_HID_RC_BUTTON_TIMER_MSG Timer[%x]\n",button_event->timer));

            switch(button_event->timer)
            {
                case GATT_HID_RC_TIMER_MULTIPLE:
                {
                    /* Update the state for the input(s) */
                    remote->rc_button_state = inputDownVShort;

                    /* Notify the input manager of the input button press (only if the DOWN event hasn't already been sent - which can happen in very corner cases of very very fast multiple button press/release) */
                    if (remote->rc_sent_down_notify == 0)
                    {
                        notifyInputManager( inputEventDown, remote->rc_button_mask, button_event->cid );
                    }

                    /* Start the SHORT and REPEAT button timers */
                    gattHidRcCreateButtonEventMessage( GATT_HID_RC_BUTTON_TIMER_MSG, GATT_HID_RC_TIMER_SHORT,
                                                                            remote->rc_button_mask, button_event->cid,
                                                                            (InputTimers.shortTimer -InputTimers.multipleDetectTimer) );
                    gattHidRcCreateButtonEventMessage( GATT_HID_RC_BUTTON_REPEAT_MSG, 0,
                                                                            remote->rc_button_mask, button_event->cid,
                                                                            (InputTimers.repeatTimer - InputTimers.multipleDetectTimer) );
                }
                break;
                case GATT_HID_RC_TIMER_SHORT:
                {
                    /* Update the button mask for the RC */
                    remote->rc_button_state = inputDownShort;

                    /* Notify the input manager of the timer event */
                    notifyInputManager(inputEventShortTimer, button_event->mask, button_event->cid );

                    /* Start the LONG timer */
                    gattHidRcCreateButtonEventMessage( GATT_HID_RC_BUTTON_TIMER_MSG,  GATT_HID_RC_TIMER_LONG,
                                                                            button_event->mask, button_event->cid,
                                                                            (InputTimers.longTimer - InputTimers.shortTimer) );
                }
                break;
                case GATT_HID_RC_TIMER_LONG:
                {
                    /* Update the button mask for the RC */
                    remote->rc_button_state = inputDownLong;

                    /* Notify the input manager of the timer event */
                    notifyInputManager( inputEventLongTimer, button_event->mask, button_event->cid );

                    /* Start the VLONG timer */
                    gattHidRcCreateButtonEventMessage( GATT_HID_RC_BUTTON_TIMER_MSG,  GATT_HID_RC_TIMER_VLONG,
                                                                            button_event->mask, button_event->cid,
                                                                            (InputTimers.vLongTimer - InputTimers.longTimer) );
                }
                break;
                case GATT_HID_RC_TIMER_VLONG:
                {
                    /* Update the button mask for the RC */
                    remote->rc_button_state = inputDownVLong;

                    /* Notify the input manager of the timer event */
                    notifyInputManager(inputEventVLongTimer, button_event->mask, button_event->cid );

                    /* Start the VVLONG timer */
                    gattHidRcCreateButtonEventMessage( GATT_HID_RC_BUTTON_TIMER_MSG,GATT_HID_RC_TIMER_VVLONG,
                                                                            button_event->mask, button_event->cid,
                                                                            (InputTimers.vvLongTimer - InputTimers.vLongTimer) );
                }
                break;
                case GATT_HID_RC_TIMER_VVLONG:
                {
                    /* Notify the input manager of the timer event */
                    notifyInputManager(inputEventVVLongTimer, button_event->mask, button_event->cid );

                    /* Update the button mask for the RC */
                    remote->rc_button_state = inputDownVVLong;
                }
                break;
            }
        }
        break;

        case GATT_HID_RC_BUTTON_REPEAT_MSG:
        {
            /* Notify the input manager of the timer event */
            notifyInputManager(inputEventRepeatTimer, button_event->mask, button_event->cid );

            /* Keep sending REPEAT messages until the button(s) is/are released */
            gattHidRcCreateButtonEventMessage(GATT_HID_RC_BUTTON_REPEAT_MSG, 0,
                                                                        button_event->mask, button_event->cid,
                                                                        InputTimers.repeatTimer );
        }
        break;

        case GATT_HID_RC_BUTTON_UP_MSG:
        {
            /* Is this a valid release? The input mask should not be identical to the current pressed button(s) */
            if (button_event->mask == remote->rc_button_mask)
            {
                /* Ignore the button release */
                return;
            }
            /* Are there any more buttons held down? */
            else if (button_event->mask == 0)
            {
                /* All buttons have been released */

                /* Cancel the (MULTIPLE,SHORT,LONG,VLONG or VVLONG) TIMER & REPEAT timers*/
                MessageCancelAll(&GATT_HID_RC_GDATA.task, GATT_HID_RC_BUTTON_TIMER_MSG);
                MessageCancelAll(&GATT_HID_RC_GDATA.task, GATT_HID_RC_BUTTON_REPEAT_MSG);

                /* Update the button state for the RC ready for the next button press */
                remote->rc_buttons_down = 0;
            }
            else
            {
                /* Update the exact number of buttons still held down */
                remote->rc_buttons_down -= bitCounter16(button_event->mask);
            }
             GATT_HID_RC_DEBUG(("GattHIDRc: Process Button Release ButtonDown[%x], ButtonState[%x] \n",remote->rc_buttons_down,remote->rc_button_state));

            /* Process the button(s) release, what state was the button when it was released? */
            switch(remote->rc_button_state)
            {
                case inputNotDown:
                {
                    /* Will get here when a multiple button press has been detected but one of the buttons was released early, when the other buttons are released will end up here */
                }
                break;
                case inputMultipleDetect:
                {
                    if (remote->rc_sent_down_notify == 0)
                    {
                        /* This was a very very quick button(s) press; need to send the input manager a DOWN event to notify there was a button pressed down (before sending the release event) */
                        notifyInputManager(inputEventDown, remote->rc_button_mask, button_event->cid );
                        remote->rc_sent_down_notify = 1;
                    }
                }
                case inputDownVShort:
                {
                    /* Notify the input manager of the "very short button press" event */
                    notifyInputManager( inputEventVShortRelease, button_event->mask, button_event->cid );
                }
                break;
                case inputDownShort:
                {
                    /* Notify the input manager of the timer event */
                    notifyInputManager(inputEventShortRelease, button_event->mask, button_event->cid );
                }
                break;
                case inputDownLong:
                {
                    /* Notify the input manager of the timer event */
                    notifyInputManager(inputEventLongRelease, button_event->mask, button_event->cid );
                }
                break;
                case inputDownVLong:
                {
                    /* Notify the input manager of the timer event */
                    notifyInputManager(inputEventVLongRelease, button_event->mask, button_event->cid );
                }
                break;
                case inputDownVVLong:
                {
                    /* Notify the input manager of the timer event */
                    notifyInputManager(inputEventVVLongRelease, button_event->mask, button_event->cid );
                }
                break;
            }

            /* Update the button mask for the device to keep track of the state of each of the buttons on the RC */
            remote->rc_button_mask = button_event->mask;

            /* If no more buttons are down, update the state ready to  further button presses */
            if (remote->rc_buttons_down == 0)
            {
                remote->rc_button_state = inputNotDown;
                remote->rc_sent_down_notify = 0;
            }
        }
        break;

        default:
            break;
    }
}

/*******************************************************************************
NAME
    gattHidRcMessageHandler

DESCRIPTION
    Remote Controler task message handler

PARAMETERS
    task        Task where the handler is registered
    ind          Message ID for this handler
    message  Any messge posted to associated task

*/
static void gattHidRcMessageHandler( Task task, MessageId id, Message message )
{
    UNUSED(task);
    if ( (id >= GATT_HID_RC_MSG_BASE) && (id < GATT_HID_RC_BUTTON_MSG_TOP) )
    {
        switch(id)
        {
            case GATT_HID_RC_BUTTON_DOWN_MSG:
            case GATT_HID_RC_BUTTON_TIMER_MSG:
            case GATT_HID_RC_BUTTON_REPEAT_MSG:
            case GATT_HID_RC_BUTTON_UP_MSG:
            {
                handleHidRcButtonEventMessages(id,(GATT_HID_RC_BUTTON_EVENT_MSG_T*)message);
            }
            break;

            default:
                GATT_HID_RC_DEBUG(("Unhandled RC msg[%x]\n", id));
                break;
        }
    }
}

/*******************************************************************************
NAME
    gattHidRcMapItemSize

DESCRIPTION
    Return the decoded item size from a HID report map item
*/
static uint8 gattHidRcMapItemSize(uint8 item)
{
    uint8 item_size = item & 0x03;
    return item_size == 3 ? 4 : item_size;
}


/*******************************************************************************
NAME
    gattHidRcReadMapItem

DESCRIPTION
    Read a HID report map item
    Enable notifications for any characteristic representing consumer input
*/
static void gattHidRcReadMapItem(uint16 cid, uint8 *item)
{
    uint8 item_tag;
    uint16 item_value;
    uint8 item_size;

    GATT_HID_RC_DEBUG(("GattHIDRc:Entry gattHidRcReadMapItem\n"));

    item_size = gattHidRcMapItemSize(item[0]);
    item_tag = item[0] >> 2;

    switch (item_size)
    {
        case 1:
        {
            item_value = (uint16) (item[1] & 0x00ff);
        }
        break;
        case 2:
        {
            item_value = item[1] | ((uint16) item[2] << 8);
        }
        break;
        default:
            item_value = 0;
            break;
    }

    switch (item_tag)
    {
        case USB_HID_TAG_USAGE_PAGE:
        {
            parser->usage_page = item_value;
            parser->usage = USB_CONSUMER_USAGE_UNKNOWN;
        }
        break;
        case USB_HID_TAG_USAGE:
        {
            if (parser->usage_page == USB_USAGE_PAGE_CONSUMER)
            {
                parser->usage = item_value;
            }
        }
        break;
        case USB_HID_TAG_REPORT_ID:
        {
            if (parser->usage == USB_CONSUMER_USAGE_CONTROL)
            {
                /* Register for notification */
                GATT_HID_RC_DEBUG(("GattHIDRc:USB_HID_TAG_REPORT_ID [%d] Register for notification  \n",item_value));
                sinkGattHidClientNotificationReqForReportId(cid,item_value,TRUE);
            }
        }
        break;
    }
}

/****************************************************************************
NAME
      gattHidRcGetNumOfLookup

DESCRIPTION
    Returns number of lookup table present

RETURNS
      void
*/
static uint16 gattHidRcGetNumOfLookup(uint16 readonly_size)
{
    uint16 num_lookup = 0;

    uint16 size_lookup_table = (readonly_size * sizeof(uint16)) - (sizeof(sink_gatt_hidrc_readonly_config_def_t) - sizeof(gattHidRcLookupTable_t));

    num_lookup = size_lookup_table/sizeof(gattHidRcLookupTable_t);

    GATT_HID_RC_DEBUG(("GattHIDRc: RC Config : Lookup Size[%d]:\n", num_lookup));

    return num_lookup;
}

/*******************************************************************************
NAME
    gattHidRcCleanupParser

DESCRIPTION
    Clean Up report Map Parser
*/
static void gattHidRcCleanupParser(void)
{
   if (parser != NULL)
    {
        free(parser);
        parser = NULL;
    }
}

/****************************************************************************/
gattHidRcDevData_t * sinkGattHidRcGetConnectedRemote(uint16 cid)
{
    gatt_client_services_t *client_services = NULL;
    /* Get the connection from cid */
    gatt_client_connection_t *connection = gattClientFindByCid(cid);
    client_services = gattClientGetServiceData(connection);
    /*if client service is available and there exists a HID client ,then return remote */
    if(client_services && client_services->remote)
        return client_services->remote;

    return NULL;
}


/****************************************************************************
                                       Interface Functions
****************************************************************************/

/****************************************************************************/

void sinkGattHidRcAddRemote(uint16 cid)
{
    gatt_client_services_t *client_services = NULL;
    gatt_client_connection_t *connection;
    uint16 *hid_rc;

    GATT_HID_RC_DEBUG(("GattHIDRc: Func:sinkGattHidRcAddRemote() Entry \n"));

    connection = gattClientFindByCid(cid);
    hid_rc = gattClientGetServicePtr(connection, sizeof(gattHidRcDevData_t));

    if (hid_rc)
    {
        client_services = gattClientGetServiceData(connection);
        client_services->remote = (gattHidRcDevData_t *)hid_rc;
        memset(client_services->remote,0,sizeof(gattHidRcDevData_t));
        GATT_HID_RC_DEBUG(("GattHIDRc: Remote Added \n"));
    }
}

/****************************************************************************/
void sinkGattHidRcProcessButtonPress(uint16 size,uint8* value,uint16 cid)
{
    uint16 count = 0;
    uint16 index = 0;
    uint16 num_valid_hid_codes = 0;
    uint16 current_button_mask = 0;
    /* Message ID to be send */
    gattHidRcMessageID_t message_id = GATT_HID_RC_MSG_BASE;
    /* Get the connected Remote */
    gattHidRcDevData_t *remote = gattHidRcGetConnectedRemote(cid);
    sink_gatt_hidrc_readonly_config_def_t *r_config_data = NULL;
    uint16 read_size = configManagerGetReadOnlyConfig(SINK_GATT_HIDRC_READONLY_CONFIG_BLK_ID, (const void **)&r_config_data);

    GATT_HID_RC_DEBUG(("GattHIDRc:Func() sinkGattHidRcProcessButtonPress() Entry: cid = %x\n", cid));

    if(remote == NULL)
    {
        GATT_HID_RC_DEBUG(("GattHIDRc:No Remotes Found for cid[%x] = \n", cid));
        return;
    }

    /* Validate HID code received */
    for(count = 0; count < size; count++)
    {
        if(value[count] != 0)
        {
            /*Retrieve read only data*/
            if (read_size)
            {
                uint16 lookup_size = gattHidRcGetNumOfLookup(read_size);
                num_valid_hid_codes++;

                /* A valid HID code has been obtained, check it is supported, else ignore the code */
                for(index = 0; index < lookup_size; index++)
                {
                    GATT_HID_RC_DEBUG(("GattHIDRc: HID Received = %x HID Configured= %x \n",
                                        value[count],
                                        r_config_data->lookup_table[index].hid_code));

                    if(r_config_data->lookup_table[index].hid_code == value[count])
                    {
                        current_button_mask |=  (1 << r_config_data->lookup_table[index].input_id);
                        GATT_HID_RC_DEBUG(("GattHIDRc: Found a Known HID = %x \n",value[count]));
                        break;
                    }
                }
            }
        }
    }

    configManagerReleaseConfig(SINK_GATT_HIDRC_READONLY_CONFIG_BLK_ID);

    GATT_HID_RC_DEBUG(("GattHIDRc: Num Valid Hid Codes [%d] current mask[%x] \n",num_valid_hid_codes,current_button_mask));

    if(!num_valid_hid_codes)
    {
        /* No HID codes found, all buttons has been released */
        MAKE_GATT_HID_RC_MESSAGE(GATT_HID_RC_BUTTON_EVENT_MSG);
        GATT_HID_RC_DEBUG(("GattHIDRc: Button released RcButtonMask[%d]\n", remote->rc_button_mask));
        message_id = GATT_HID_RC_BUTTON_UP_MSG;
        message->cid = cid;
        message->mask = 0;
        /* Send the button event off to be processed */
        MessageSend( &GATT_HID_RC_GDATA.task, message_id, message );
    }
    /*Check the mask is valid and the code was available in look up , then only send to process the same */
    else if(current_button_mask)
    {
        /* Single Button press? */
        if(remote->rc_button_mask == 0)
        {
            MAKE_GATT_HID_RC_MESSAGE(GATT_HID_RC_BUTTON_EVENT_MSG);
            GATT_HID_RC_DEBUG(("GattHIDRc:Single Button Press CurrMask[%x] RCMask[%x]  \n",current_button_mask,remote->rc_button_mask));
            message_id = GATT_HID_RC_BUTTON_DOWN_MSG;
            message->mask = current_button_mask;
            message->cid = cid;
            MessageSend( &GATT_HID_RC_GDATA.task, message_id, message );
        }
        /* Button (s) Released ?*/
        else if(current_button_mask & remote->rc_button_mask)
        {
            MAKE_GATT_HID_RC_MESSAGE(GATT_HID_RC_BUTTON_EVENT_MSG);
            GATT_HID_RC_DEBUG(("GattHIDRc:Buttons Released CurrMask[%x] RCMask[%x]  \n",current_button_mask,remote->rc_button_mask));
            message_id = GATT_HID_RC_BUTTON_UP_MSG;
            message->cid = cid;
            message->mask = current_button_mask;
            MessageSend( &GATT_HID_RC_GDATA.task, message_id, message );
        }
        /* Other Buttons have been pressed */
        else
        {
            /* Inform the input manager which new button(s) have been pressed */
            MAKE_GATT_HID_RC_MESSAGE(GATT_HID_RC_BUTTON_EVENT_MSG);
            GATT_HID_RC_DEBUG(("GattHIDRc:Other Buttons Pressed CurrMask[%x] RCMask[%x]  \n",current_button_mask,remote->rc_button_mask));
            message_id = GATT_HID_RC_BUTTON_DOWN_MSG;
            message->cid = cid;
            message->mask = current_button_mask;
            MessageSend( &GATT_HID_RC_GDATA.task, message_id, message );
        }
    }
}

/****************************************************************************/
void  sinkGattHidRcResetParser(void)
{
    GATT_HID_RC_DEBUG(("GattHIDRc:Entry sinkGattHidRcResetParser\n"));

    if (parser == NULL)
    {
        GATT_HID_RC_DEBUG(("GattHIDRc:Size of RC Parser [%d]\n", sizeof(map_parser_t)));
        parser = PanicUnlessMalloc(sizeof (map_parser_t));
    }

    parser->size_fragment = 0;
    parser->usage_page = USB_USAGE_PAGE_UNKNOWN;
}

/****************************************************************************/
void sinkGattHidRcProcessReportMapDescriptor(uint16 cid,uint16 size_data,uint8 *data,uint16 more_to_come)
{
    uint8 *item;
 #ifdef DEBUG_GATT_HID_RC
    uint16 count = 0;
 #endif
    gattHidRcDevData_t *remote = sinkGattHidRcGetConnectedRemote(cid);

    GATT_HID_RC_DEBUG(("\n GattHIDRc:Entry sinkGattHidRcProcessReportMapDescriptor\n"));

    /* If there is no connected remote for CID provided or parser Map is not initialised , no need to proceed further */
    if(remote == NULL || parser == NULL)
    {
           GATT_HID_RC_DEBUG(("GattHIDRc:No Remotes Found for cid[%x] or map parser in not initialised\n", cid));
           gattHidRcCleanupParser();
           return;
    }

#ifdef DEBUG_GATT_HID_RC
    for (count=0;count<size_data; count++)
    {
        GATT_HID_RC_DEBUG(("%02x ", data[count]));
    }
#endif
    GATT_HID_RC_DEBUG(("\n GattHIDRc: BLE REPORT_MAP : Start parsing: More to come[%d]\n",more_to_come));

    if (parser->size_fragment)
    {
        /* Complete the outstanding fragment and parse it  */
        uint8 missing = gattHidRcMapItemSize(parser->fragment[0]) + 1 - parser->size_fragment;
        memmove(parser->fragment + parser->size_fragment, data, missing);
        GATT_HID_RC_DEBUG(("GattHIDRc: get frag %u for 0x%02X\n",
                          parser->size_fragment, parser->fragment[0]));

        data += missing;
        size_data -= missing;

        gattHidRcReadMapItem(cid, parser->fragment);
        parser->size_fragment = 0;
    }

    item = data;

    while (item < data + size_data)
    {
        uint8 item_size = gattHidRcMapItemSize(item[0]);

        if (item + item_size + 1 > data + size_data)
        {
        /*  Data item is incomplete; save fragment for next pass  */
            parser->size_fragment = data + size_data - item;
            GATT_HID_RC_DEBUG(("GattHIDRc: put frag %u from 0x%02X\n",
                          parser->size_fragment, item[0]));

            memmove(parser->fragment, item, parser->size_fragment);
        }
        else
        {
            /*  Process the next item  */
            gattHidRcReadMapItem(cid, item);
        }

        item += item_size + 1;
    }

    if (!more_to_come)
    {
        GATT_HID_RC_DEBUG(("GattHIDRc: map parsing done\n"));
        gattHidRcCleanupParser();
    }

    GATT_HID_RC_DEBUG(("GattHIDRc: sinkGattHidRcProcessReportMapDescriptor : Parsing Done \n"));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void sinkGattHidRcInit(void)
{
   /* Init Remote Control Internal Message handler */
   GATT_HID_RC_GDATA.task.handler = gattHidRcMessageHandler;
}

/*******************************************************************************/
uint16 sinkGattHidRcGetRemoteControlCid(void)
{
    uint16 index;
    for(index = 0; index < MAX_BLE_CONNECTIONS; index ++)
    {
        /* if at all the remote control is connected then send its CID.*/
        if(gattHidRcIsConnected(&GATT_CLIENT.connection))
        {
            return GATT[index].cid;
        }
    }
    return GATT_CLIENT_INVALID_CID;
}
#endif /* GATT_HID_REMOTE_CONTROL */

