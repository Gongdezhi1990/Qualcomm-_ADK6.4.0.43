/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc_mas_handler.c
    
DESCRIPTION
   This file implements all incoming and Outgoing MNS Server messages
*/

#include <obex.h>
#include <string.h>
#include <source.h>
#include "mapc_private.h"
#include "mapc_extern.h"


const uint8 event_report[] = "x-bt/MAP-event-report";


/**************************************************************************
 * NAME:
 *  mapcMnsHandlePutInd
 *
 * DESCRIPTION
 * MNS Server function to handle PUT Indication Message.
 ************************************************************************/
void mapcMnsHandlePutInd( Mapc mnsTask, OBEX_PUT_IND_T* putInd )
{
    ObexResponse response= obex_srv_unavailable;
    Source src = 0;
    uint16 srcLen = putInd->sourceLen;


    if( IsMapcConnected( mnsTask ) )
    {
        uint16 len; 
        
        if( srcLen ) src = putInd->source;
        if( IsMapcFuncFree( mnsTask ) )
        {
            const uint8* type;

            /* Get the type header to get the Message Type */
            type = ObexObjMapHdrValue( obex_type_hdr, srcLen, src, &len );

            if( ( len == sizeof(event_report) ) &&
                ( memcmp( type, event_report, len ) ==  0 ))
            {
                const uint8 *appParam;
 
                /* Extract the MasInstace ID */
                appParam = ObexObjMapHdrValue( obex_app_params, srcLen,
                                               src, &len );
                if( appParam && (len >= MAPC_BYTE_APP_HDR_SIZE) &&
                    (appParam[0] ==  mapc_app_mas_instance_id) )
                {
                    response = obex_remote_success;
                    mnsTask->lastAppParam = appParam[MAPC_2BYTE_SIZE];
                }
                else
                {
                    response = obex_bad_request;
                }
            }
            else
            {
                response = obex_bad_request;
            }
        }
        else /* session->function must be mapc_send_event  */
        {
            response = obex_remote_success;            
        }
    }

    if( response == obex_remote_success )
    {
        ObexSeqHeaderId id = (putInd->final)? obex_ebody_hdr: obex_body_hdr;
       
        src =  ObexObjFilterHdrValue( mnsTask->session, id, &srcLen );
        if( SourceSize( src ) < srcLen )
        {
            MAPC_INFO(("Invalid BODY\n"));
            srcLen = SourceSize(src);
        }
        
        MAPC_SET_FUNCTION( mnsTask, mapc_send_event );
        mnsTask->function = mapc_reg_notification;
        
        mapcMnsSendEventInd( mnsTask, putInd->final, srcLen, src );
    }
    else
    {
        mapcPostMnsSendEventResp( mnsTask, response );
    }
}

/**************************************************************************
 * NAME:
 *  mapcMnsIntSendEventResp
 *
 * DESCRIPTION
 *  Send Event Response
 ************************************************************************/
void mapcMnsIntSendEventResp( MAPC_INT_SEND_EVENT_RESP_T* rsp)
{
    Mns mnsTask = rsp->session;
    
    /* send the Obex response */
    if( rsp->respCode != obex_continue )  MAPC_RESET_FUNCTION( mnsTask );

    ObexPutResponse( mnsTask->session, rsp->respCode );
}

