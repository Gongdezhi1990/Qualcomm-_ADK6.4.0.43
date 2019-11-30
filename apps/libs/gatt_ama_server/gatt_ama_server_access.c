/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

#include "gatt_ama_server_private.h"

#include "gatt_ama_server_access.h"
#include "gatt_ama_server_db.h"

#include "stdio.h"

#include <ama.h>

/* Required octets for values sent to Client Configuration Descriptor */
#define GATT_CLIENT_CONFIG_OCTET_SIZE sizeof(uint8) * 2
/* Required octets for values sent to Presentation Descriptor */
#define GATT_PRESENTATION_OCTET_SIZE sizeof(uint8) * 7



#define AMA_BUFFER_SIZE 4

//static uint8 amaData[AMA_BUFFER_SIZE];


/* AMA TODO : Revisit for future improvement */
GAMASS *AmaServer;




/* Required octets for values sent to Client Configuration Descriptor */
#define GATT_CLIENT_CONFIG_NUM_OCTETS   2



/***************************************************************************
NAME
    sendAmaAccessRsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
static void sendAmaAccessRsp(Task task,
                                    uint16 cid,
                                    uint16 handle,
                                    uint16 result,
                                    uint16 size_value,
                                    const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        GATT_AMA_SERVER_DEBUG_PANIC(("Couldn't send GATT access response\n"));
    }
}

/***************************************************************************
NAME
    sendAmaAccessErrorRsp

DESCRIPTION
    Send an error access response to the GATT Manager library.
*/
static void sendAmaAccessErrorRsp(const GAMASS *ama_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind, uint16 error)
{
    sendAmaAccessRsp((Task)&ama_server->lib_task, access_ind->cid, access_ind->handle, error, 0, NULL);
}

/***************************************************************************
NAME
    amaServiceAccess

DESCRIPTION
    Deals with access of the HANDLE_AMA_SERVICE handle.
*/
static void amaServiceAccess(GAMASS *ama_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        /* there is no char with read access */
        sendAmaAccessErrorRsp(ama_server, access_ind, gatt_status_read_not_permitted);
    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        sendAmaAccessRsp((Task)&ama_server->lib_task, access_ind->cid, HANDLE_AMA_ALEXA_TX_CHAR, gatt_status_success, 0, NULL);
    }
    else
    {
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendAmaAccessErrorRsp(ama_server, access_ind, gatt_status_request_not_supported);
    }
}







/***************************************************************************
NAME
    amaCharecteristicAccess

DESCRIPTION
    Deals with access of the HANDLE_AMA_SERVICE handle.
*/
static void amaCharecteristicAccess(GAMASS *ama_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    uint16 result = gatt_status_success;

    uint16 client_config = 0;
    uint8* data_ptr = NULL;
    uint16 size_value = 0;

    /* ...  just for notification */
    AmaServer = ama_server;
    AmaServer->cid = access_ind->cid;



    GATT_AMA_SERVER_DEBUG_INFO(("AMA Acs hnd = %d, len %d\n", access_ind->handle, access_ind->size_value));

#if 0
    if (access_ind->handle == HANDLE_AMA_ALEXA_RX_CHAR && access_ind->flags & ATT_ACCESS_READ)
    {
        amaTbdReadSomething(data, len);

        if(len > AMA_BUFFER_SIZE)
        {
            result = gatt_status_invalid_length;
        }


    GattManagerServerAccessResponse((Task)&ama_server->lib_task, access_ind->cid, HANDLE_AMA_ALEXA_RX_CHAR, result, AMA_BUFFER_SIZE, amaData);
    }
    else
#endif

    if ( access_ind->flags & ATT_ACCESS_WRITE)
    {
        if(access_ind->handle == HANDLE_AMA_ALEXA_TX_CHAR)
        {
            AmaParseData(&access_ind->value[0], access_ind->size_value);
        }
        else if (access_ind->handle == HANDLE_AMA_ALEXA_RX_CLIENT_C_CFG)
        {

            MAKE_GATT_AMA_SERVER_MESSAGE(GATT_AMA_SERVER_CLIENT_C_CFG);

            // GATT_AMA_SERVER_DEBUG_INFO(("Gatt_AMA Access CLIENT_C_CFG  handle\n"));

            client_config =  /* access_ind->value[0]<<8 | access_ind->value[1]; */
            access_ind->value[1]<<8 | access_ind->value[0];

            message->ama = ama_server;     /* Pass the instance which can be returned in the response */
            message->cid = access_ind->cid;                 /* Pass the CID so the client can be identified */
            message->handle = access_ind->handle;
            message->client_config = client_config;

            MessageSend(ama_server->app_task, GATT_AMA_SERVER_CLIENT_C_CFG, message);

            size_value = GATT_CLIENT_CONFIG_NUM_OCTETS;

            data_ptr = (uint8*)&client_config;
#if 0
            /* AMA TODO Revisit for future improvement */
#endif
        }
        else
        {
            result = gatt_status_write_not_permitted;
            // GATT_AMA_SERVER_DEBUG_INFO(("Gatt_AMA Access  unknown handle \n"));
        }

        sendAmaAccessRsp((Task)&ama_server->lib_task, access_ind->cid, access_ind->handle, result, size_value, data_ptr);

    }

    else if (access_ind->handle == HANDLE_AMA_ALEXA_RX_CHAR && access_ind->flags & ATT_ACCESS_READ)
    {
         //uint8 data_to_read[] = {0x00, 0x01, 0x02};

      //  GATT_AMA_SERVER_DEBUG_INFO(("Gatt_AMA Access  READ %d \n" ,access_ind->handle));

        sendAmaAccessRsp((Task)&ama_server->lib_task, access_ind->cid, HANDLE_AMA_ALEXA_RX_CHAR, result, 0, NULL);
	}
    else
    {
        GATT_AMA_SERVER_DEBUG_INFO(("Gatt_AMA Access  REJECT handle = %d \n", access_ind->handle));
        /* Reject access requests that aren't read/write, which shouldn't happen. */
        sendAmaAccessErrorRsp(ama_server, access_ind, gatt_status_request_not_supported);
    }
}



/***************************************************************************/
void handleAmaServerAccess(GAMASS *ama_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    switch (access_ind->handle)
    {
        case HANDLE_AMA_ALEXA_SERVICE:
        {
            amaServiceAccess(ama_server, access_ind);
        }
        break;

        case HANDLE_AMA_ALEXA_RX_CLIENT_C_CFG:
        case HANDLE_AMA_ALEXA_TX_CHAR:
        case HANDLE_AMA_ALEXA_RX_CHAR:
        {
            amaCharecteristicAccess(ama_server, access_ind);
        }
        break;



        default:
        {
            /* Respond to invalid handles */
            sendAmaAccessErrorRsp(ama_server, access_ind, gatt_status_invalid_handle);
        }
        break;
    }
}





/***************************************************************************/




