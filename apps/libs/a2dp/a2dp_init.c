/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_init.c

DESCRIPTION
    This file contains the initialisation code for the A2DP profile library.

NOTES

*/



/****************************************************************************
    Header files
*/
#include "a2dp.h"
#include "a2dp_data_block.h"
#include "a2dp_init.h"
#include "a2dp_l2cap_handler.h"
#include "a2dp_private.h"
#include "a2dp_profile_handler.h"

#include <message.h>
#include <panic.h>
#include <print.h>
#include <stdlib.h>
#include <string.h>

/* The one, and only, A2DP instance */
A2DP *a2dp = 0;  /* Pointer located in global space */

#ifndef A2DP_1_2
/*
  SDP Service Record generated from a2dp_sink.sdp by sdpgen.pl
*/
static const uint8 a2dp_sink_service_record[] =
{
  0x09,     /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35,     /* DataElSeq 3 bytes */
  0x03,
    0x19,   /* uuid AudioSink(0x110b) */
    0x11,
    0x0b,
  0x09,     /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35,     /* DataElSeq 16 bytes */
  0x10,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0019 */
        0x00,
        0x19,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVDTP(0x0019) */
      0x00,
      0x19,
      0x09, /* uint16 0x0103 */
        0x01,
        0x03,
  0x09,     /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35,     /* DataElSeq 8 bytes */
  0x08,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AdvancedAudioDistribution(0x110d) */
      0x11,
      0x0d,
      0x09, /* uint16 0x0103 */
        0x01,
        0x03,
  0x09,     /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09,     /* uint16 0x0001 */
    0x00,
    0x01,
}; /* 48 bytes */


/*
  SDP Service Record generated from a2dp_source.sdp by sdpgen.pl
*/
static const uint8 a2dp_source_service_record[] =
{
  0x09,     /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35,     /* DataElSeq 3 bytes */
  0x03,
    0x19,   /* uuid AudioSource(0x110a) */
    0x11,
    0x0a,
  0x09,     /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35,     /* DataElSeq 16 bytes */
  0x10,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0019 */
        0x00,
        0x19,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVDTP(0x0019) */
      0x00,
      0x19,
      0x09, /* uint16 0x0103 */
        0x01,
        0x03,
  0x09,     /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35,     /* DataElSeq 8 bytes */
  0x08,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AdvancedAudioDistribution(0x110d) */
      0x11,
      0x0d,
      0x09, /* uint16 0x0103 */
        0x01,
        0x03,
  0x09,     /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09,     /* uint16 0x0001 */
    0x00,
    0x01,
}; /* 48 bytes */

#else

/* Define A2DP 1.2 */
/*
  SDP Service Record generated from a2dp_sink.sdp by sdpgen.pl
*/
static const uint8 a2dp_sink_service_record[] =
{
  0x09,     /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35,     /* DataElSeq 3 bytes */
  0x03,
    0x19,   /* uuid AudioSink(0x110b) */
    0x11,
    0x0b,
  0x09,     /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35,     /* DataElSeq 16 bytes */
  0x10,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0019 */
        0x00,
        0x19,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVDTP(0x0019) */
      0x00,
      0x19,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09,     /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35,     /* DataElSeq 8 bytes */
  0x08,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AdvancedAudioDistribution(0x110d) */
      0x11,
      0x0d,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09,     /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09,     /* uint16 0x0001 */
    0x00,
    0x01,
}; /* 48 bytes */


/*
  SDP Service Record generated from a2dp_source.sdp by sdpgen.pl
*/
static const uint8 a2dp_source_service_record[] =
{
  0x09,     /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35,     /* DataElSeq 3 bytes */
  0x03,
    0x19,   /* uuid AudioSource(0x110a) */
    0x11,
    0x0a,
  0x09,     /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35,     /* DataElSeq 16 bytes */
  0x10,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
      0x09, /* uint16 0x0019 */
        0x00,
        0x19,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AVDTP(0x0019) */
      0x00,
      0x19,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09,     /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35,     /* DataElSeq 8 bytes */
  0x08,
    0x35,   /* DataElSeq 6 bytes */
    0x06,
      0x19, /* uuid AdvancedAudioDistribution(0x110d) */
      0x11,
      0x0d,
      0x09, /* uint16 0x0102 */
        0x01,
        0x02,
  0x09,     /* SupportedFeatures(0x0311) = "0x0001" */
    0x03,
    0x11,
  0x09,     /* uint16 0x0001 */
    0x00,
    0x01,
}; /* 48 bytes */

#endif /*A2DP_1_2*/

/****************************************************************************/
static bool validateSeps(sep_data_type *seps, uint8 size_seps)
{
    uint8 i, j;

    for (i = 0; i < size_seps; i++)
    {
        /* if no config is supplied then invalid params */
        if (!seps[i].sep_config)
        {
            return FALSE;
        }
        
        /* Valid SEID values are from 0x01..0x3E inclusive */
        if ( !((seps[i].sep_config->seid > 0x00) && (seps[i].sep_config->seid < 0x3F)) )
        {
            return FALSE;
        }
        
        /* Ensure unavailable bit set if SEP initialised as in_use */
        if (seps[i].in_use)
        {
            seps[i].in_use = A2DP_SEP_UNAVAILABLE;
        }
            
        for (j = 0; j < i; ++j)
        {
            /* if any SEIDs are equal then invalid params */
            if (seps[i].sep_config->seid == seps[j].sep_config->seid)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}


/* TODO: Move to a2dp_api.c and rename to bring inline with new API */
/****************************************************************************/
void a2dpSendInitCfmToClient(a2dp_status_code status)
{
    MAKE_A2DP_MESSAGE(A2DP_INIT_CFM);
    message->status = status;
    MessageSend(a2dp->clientTask, A2DP_INIT_CFM, message);

    /* If the initialisation failed, free the allocated task */
    if (status != a2dp_success)
    {
        free(a2dp);
        a2dp = 0;
    }
}


/****************************************************************************/
void A2dpInit(Task clientTask, uint16 role, service_record_type *service_records, uint8 size_seps, sep_data_type *seps, uint16 linkloss_timeout)
{
    /* Initialise the task data */
    if ( !a2dp )
    {
        uint8 device_id;
        
        a2dp = PanicUnlessNew(A2DP);
        memset( a2dp, 0, sizeof(A2DP) );
        for (device_id=0; device_id<A2DP_MAX_REMOTE_DEVICES_DEFAULT; device_id++)
        {
            a2dpInitialiseRemoteDevice(&a2dp->remote_conn[device_id],device_id);
        }
        
        PRINT(("sizeof(A2DP)=0x%u\n", sizeof(A2DP)));
        
        /* Set the handler function */
        a2dp->task.handler = a2dpProfileHandler;
        /* Set up the lib client */
        a2dp->clientTask = clientTask;
        a2dp->linkloss_timeout = linkloss_timeout;
        a2dp->bitfields.max_remote_devs = A2DP_MAX_REMOTE_DEVICES_DEFAULT;
        a2dp->bitfields.profile_role = role;
        
        blockInit();
        
        if ( seps && size_seps && validateSeps(seps, size_seps) )
        {
            for (device_id=0; device_id<A2DP_MAX_REMOTE_DEVICES; device_id++)
            {
                sep_data_type *sep_list = (sep_data_type *)PanicNull( blockAdd( device_id, data_block_sep_list, size_seps, sizeof(sep_data_type) ) );
                memmove( sep_list, (sep_data_type *)seps, size_seps*sizeof(sep_data_type) );
            }
        }
        else
        {
            a2dpSendInitCfmToClient(a2dp_invalid_parameters);
            return;
        }
    
    
        /* Used to count the number of SDP records registered.  Decremented again by a2dpHandleSdpRegisterCfm() and will 
           kick off a call to a2dpRegisterL2cap() when it hits zero - i.e. all CFM messages for SDP regsitering process
           have been received.                                                                                           */
        a2dp->bitfields.sdp_register_outstanding = 0;
        
        if (service_records)
        {
            if (service_records->size_service_record_a && service_records->service_record_a)
            {
                /* Client has supplied their own record so register it without checking */
                ConnectionRegisterServiceRecord(&a2dp->task, service_records->size_service_record_a, service_records->service_record_a);
                a2dp->bitfields.sdp_register_outstanding++;
            }
            if (service_records->size_service_record_b && service_records->service_record_b)
            {
                /* Client has supplied their own record so register it without checking */
                ConnectionRegisterServiceRecord(&a2dp->task, service_records->size_service_record_b, service_records->service_record_b);
                a2dp->bitfields.sdp_register_outstanding++;
            }
        }
        else
        {
            /* Client using default library record */
            if (role & A2DP_INIT_ROLE_SINK)
            {
                ConnectionRegisterServiceRecord(&a2dp->task, sizeof(a2dp_sink_service_record), a2dp_sink_service_record);
                PRINT(("Register Sink Service Rec\n"));
                a2dp->bitfields.sdp_register_outstanding++;
            }
            if (role & A2DP_INIT_ROLE_SOURCE)
            {
                ConnectionRegisterServiceRecord(&a2dp->task, sizeof(a2dp_source_service_record), a2dp_source_service_record);
                PRINT(("Register Source Service Rec\n"));
                a2dp->bitfields.sdp_register_outstanding++;
            }
        }
    
        if ( a2dp->bitfields.sdp_register_outstanding==0 )
        {
            /* Skip the service record registering if the user doesn't require any at this point. */
            a2dpRegisterL2cap();
        }
    }
}

void a2dpInitialiseRemoteDevice(remote_device *device, uint8 device_id)
{
    memset( device, 0, sizeof(remote_device) );  /* Enumerated type defaults
                                                    are zero in value */
    /*
     * device_id must be initialised as it is used to index into the device's
     * data_blocks before a link is instantiated.
     */
    device->bitfields.device_id = device_id;
}
