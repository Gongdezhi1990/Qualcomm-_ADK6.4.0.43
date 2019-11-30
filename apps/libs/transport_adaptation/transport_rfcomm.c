/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Routines for managing a transport channel over RFCOMM
 */


#include<connection_no_ble.h>
#include<panic.h>
#include<print.h>
#include "transport_rfcomm.h"
#include "transport_adaptation.h"
#include "transport_adaptation_common.h"

#define RFCOMM_DEFAULT_CONFIG   (0)

/******************************************************************************
Utility function to convert bdaddr received from rfcomm connection ind
to tp_bdaddr format using trusted devices list.
*/
static void transportRfcommConvBdaddrToTpaddr(bdaddr *bd_addr,
                                              tp_bdaddr *tpaddr)
{
    bool validPdlIndex = TRUE;
    uint16 pdlIndex = 0;
    typed_bdaddr dev_addr;

    /* Scan through the PDL to find a matching device profile.
    * ConnectionSmGetIndexedAttributeNowReq() will fail when we reach the end of the PDL.*/
    while (validPdlIndex)
    {
        validPdlIndex = ConnectionSmGetIndexedAttributeNowReq(0, pdlIndex, 0,
                                                              (uint8*)NULL,
                                                              &dev_addr);
        if(BdaddrIsSame(&dev_addr.addr, bd_addr))
        {
            tpaddr->taddr = dev_addr;
            break;
        }
        pdlIndex++;
    }
    tpaddr->transport  = TRANSPORT_BREDR_ACL;
}

/******************************************************************************/
void transportRfcommRegister(Task task, uint8 server_channel)
{
    PRINT(("TRFCOMM:transportRfcommRegister\n"));
    ConnectionRfcommAllocateChannel(task, server_channel);
}

/******************************************************************************/
void transportRfcommDeregister(Task task, uint8 server_channel)
{
    PRINT(("TRFCOMM:transportRfcommDeregister\n"));
    ConnectionRfcommDeallocateChannel(task, server_channel);
}

/******************************************************************************/
void transportRfcommConnect(Task task,
                            const bdaddr* bd_addr,
                            uint16 local_transport_id,
                            uint8 remote_transport_id)
{
    PRINT(("TRFCOMM:transportRfcommConnect\n"));
    ConnectionRfcommConnectRequest(task, bd_addr,
                                   local_transport_id,
                                   remote_transport_id,
                                   RFCOMM_DEFAULT_CONFIG);
}

/******************************************************************************/
void transportRfcommDisconnect(Task task,Sink sink)
{
    PRINT(("TRFCOMM:transportRfcommDisconnect\n"));
    ConnectionRfcommDisconnectRequest(task, sink);
}

/******************************************************************************/
void handleRfcommMessage(Task task,MessageId id,Message message)
{

    PRINT(("handleRfcommMessage\n"));

    switch (id)
    {
        case CL_RFCOMM_REGISTER_CFM:
            {
                CL_RFCOMM_REGISTER_CFM_T *m   = (CL_RFCOMM_REGISTER_CFM_T*) message;
                MAKE_TA_CLIENT_MESSAGE(TRANSPORT_REGISTER_CFM,msg);
                
                PRINT(("TRFCOMM:CL_RFCOMM_REGISTER_CFM\n"));                

                if(m->status == success)
                    msg->status  = SUCCESS;
                else
                    msg->status  = FAIL;

                msg->transport      = TRANSPORT_RFCOMM;
                msg->transport_id   = (uint16)(m->server_channel);
                TA_SEND_REGISTER_CFM(msg);
            }
            break;

        case CL_RFCOMM_CONNECT_IND:
            {
                CL_RFCOMM_CONNECT_IND_T *m = (CL_RFCOMM_CONNECT_IND_T*) message;
                
                PRINT(("TRFCOMM:CL_RFCOMM_CONNECT_IND\n"));                
                
                ConnectionRfcommConnectResponse(task, TRUE,
                                                m->sink, m->server_channel,
                                                RFCOMM_DEFAULT_CONFIG);
            }
            break;

        case CL_RFCOMM_SERVER_CONNECT_CFM:
            {
                tp_bdaddr tpaddr;
                CL_RFCOMM_SERVER_CONNECT_CFM_T *m = (CL_RFCOMM_SERVER_CONNECT_CFM_T*) message;              
                MAKE_TA_CLIENT_MESSAGE(TRANSPORT_CONNECT_CFM,msg);
                
                PRINT(("TRFCOMM:CL_RFCOMM_SERVER_CONNECT_CFM\n"));                

                transportRfcommConvBdaddrToTpaddr(&m->addr, &tpaddr);

                msg->transport    = TRANSPORT_RFCOMM;
                msg->transport_id = (uint16)(m->server_channel);
                msg->sink         = m->sink;
                msg->addr         = tpaddr;

                if (m->status == rfcomm_connect_success)
                {
                    msg->status  = SUCCESS;
                }
                else
                {
                    msg->status  = FAIL;
                }
                TA_SEND_CONNECT_CFM(msg);
            }
            break;

        case CL_RFCOMM_DISCONNECT_IND:
            {
                CL_RFCOMM_DISCONNECT_IND_T *m = (CL_RFCOMM_DISCONNECT_IND_T*) message;
                
                MAKE_TA_CLIENT_MESSAGE(TRANSPORT_DISCONNECT_CFM,msg);
                
                PRINT(("TRFCOMM:CL_RFCOMM_DISCONNECT_IND\n"));                
                
                ConnectionRfcommDisconnectResponse(m->sink);

                if(m->status == rfcomm_disconnect_normal_disconnect ||
				   m->status == rfcomm_disconnect_l2cap_link_loss)
                    msg->status = SUCCESS;
                else
                    msg->status = FAIL;

                msg->transport = TRANSPORT_RFCOMM;
                msg->sink = m->sink;
                TA_SEND_DISCONNECT_CFM(msg);
            }
		break;

		case CL_RFCOMM_DISCONNECT_CFM:
            {
                CL_RFCOMM_DISCONNECT_CFM_T *m   = (CL_RFCOMM_DISCONNECT_CFM_T*) message;
                MAKE_TA_CLIENT_MESSAGE(TRANSPORT_DISCONNECT_CFM,msg);
                
                PRINT(("TRFCOMM:CL_RFCOMM_DISCONNECT_CFM\n"));

                if(m->status == rfcomm_disconnect_success)
                    msg->status = SUCCESS;
                else
                    msg->status = FAIL;

                msg->transport = TRANSPORT_RFCOMM;
                msg->sink = m->sink;
                TA_SEND_DISCONNECT_CFM(msg);
            }
            break;

        case CL_RFCOMM_UNREGISTER_CFM:
            {
                CL_RFCOMM_UNREGISTER_CFM_T *m   = (CL_RFCOMM_UNREGISTER_CFM_T*) message;
                MAKE_TA_CLIENT_MESSAGE(TRANSPORT_DEREGISTER_CFM,msg);
                
                PRINT(("TRFCOMM:CL_RFCOMM_UNREGISTER_CFM\n"));

                if(m->status == success)
                    msg->status  = SUCCESS;
                else
                    msg->status  = FAIL;

                msg->transport    = TRANSPORT_RFCOMM;
                msg->transport_id = (uint16)(m->server_channel);
                TA_SEND_DEREGISTER_CFM(msg);
            }
            break;
            
        case CL_RFCOMM_PORTNEG_IND:
            {
                CL_RFCOMM_PORTNEG_IND_T *m = (CL_RFCOMM_PORTNEG_IND_T*)message;
                PRINT(("TRFCOMM:CL_RFCOMM_PORTNEG_IND\n"));

                /* If this was a request send our default port params, otherwise accept any requested changes */
                ConnectionRfcommPortNegResponse(task, m->sink, m->request ? NULL : &m->port_params);
            }
            break;
            
        /*  Things to ignore  */
        case CL_RFCOMM_CONTROL_IND:
        case CL_RFCOMM_LINE_STATUS_IND:
            break;  
            
        default:
            {
                PRINT(("TRFCOMM:INVALID MESSAGE\n"));
                /* indicate we couldn't handle the message */
            }
            break;
    }
}

