/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

#include "gatt_ama_server_private.h"

#include "gatt_ama_server_msg_handler.h"

#include "gatt_ama_server_access.h"
#include "gatt_ama_server_db.h"

/****************************************************************************/
void GattAmaServerMsgHandler(Task task, MessageId id, Message payload)
{
    GAMASS *ama_server = (GAMASS *)task;

    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            /* Read/write access to characteristic */
            handleAmaServerAccess(ama_server, (GATT_MANAGER_SERVER_ACCESS_IND_T *)payload);
        }
        break;
        case GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM:
        {


            if(AmaTransportIsBle() == FALSE)
            {
                /* we send the last BLE packet and changed to RFCOMM, we can drop the link now */
                GattManagerDisconnectRequest(((GATT_NOTIFICATION_CFM_T *)payload)->cid);
            }

            #if 0
            GATT_AMA_SERVER_DEBUG_INFO(("AMA NOT CFM s %d, %04x, %d  ",
                        ((GATT_NOTIFICATION_CFM_T *)payload)->status,((GATT_NOTIFICATION_CFM_T *)payload)->cid,
                                                    ((GATT_NOTIFICATION_CFM_T *)payload)->handle   ));
            #endif

          /*ATT_NOTIFICATION_CFM_T *)payload)->handle));*/
          /* Library just absorbs confirmation messages */
          /*  handleAmaServerNotificationCfm(ama_server, (const GATT_NOTIFICATION_CFM_T *)msg);	*/
        }
        break;
        default:
        {
            /* Unrecognised GATT Manager message */
            GATT_AMA_SERVER_DEBUG_PANIC(("GATT AMA Manager Server Msg not handled\n"));
        }
        break;
    }
}


/***************************************************************************
NAME
    handleBleTestServerNotificationCfm

DESCRIPTION
    Handles the GATT_MANAGER_ACCESS_IND message that was sent to the library.
*/
#if 0
static void handleAmaServerNotificationCfm(GAMASS *const ama, const GATT_NOTIFICATION_CFM_T *cfm)
{
        MAKE_GATT_AMA_SERVER_MESSAGE(GATT_AMA_SERVER_NOTIFICATION_CFM); */

        UNUSED(ama);

    switch (cfm->handle)
    {
        case HANDLE_AMA_ALEXA_TX_CHAR:
        {

        }
        break;


        default:
        {
            /* message->cid = 0;  not suppose to be received cfm->cid; */

        }
        break;
    }

#if 0

    message->ama = ama;	  /* Pass the instance which can be returned in the response */
    message->cid = cfm->cid;				/* Pass the CID so the client can be identified */
    message->handle = cfm->handle;
    message->status = cfm->status;

    MessageSend(ama->app_task, GATT_AMA_SERVER_NOTIFICATION_CFM, message);

#endif
}
#endif

