/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    spps_shim.h
    
DESCRIPTION
	SPPS Server Shim Layer
	
*/

#ifndef SPPS_SHIM_LAYER_H
#define SPPS_SHIM_LAYER_H

#include <spps.h>

typedef struct __SPP SPPS;

#define SPPS_SHIM_MESSAGE_BASE SPP_MESSAGE_TOP
typedef enum
{
    SPPS_MESSAGE_MORE_DATA_TEST_EXTRA = SPPS_SHIM_MESSAGE_BASE,
    SPPS_START_SERVICE_CFM_TEST_EXTRA,
    SPPS_STOP_SERVICE_CFM_TEST_EXTRA,
    SPPS_CONNECT_IND_TEST_EXTRA,
    SPPS_SERVER_CONNECT_CFM_TEST_EXTRA,
    SPPS_DISCONNECT_IND_TEST_EXTRA,
    SPPS_DISCONNECT_CFM_TEST_EXTRA,

    /* Library message limit */
    SPPS_SHIM_MESSAGE_TOP
} SppsShimMessageId;
    
typedef struct
{
    Sink    sink;
    
    uint16  size_data;
    uint8   data[1];
} SPPS_MESSAGE_MORE_DATA_TEST_EXTRA_T;

typedef struct
{
    spp_start_status    status; 
} SPPS_START_SERVICE_CFM_TEST_EXTRA_T;

typedef struct
{
    spp_stop_status     status;
} SPPS_STOP_SERVICE_CFM_TEST_EXTRA_T;

typedef struct
{
    bdaddr  addr;   
    uint8   server_channel;
    Sink    sink;
} SPPS_CONNECT_IND_TEST_EXTRA_T;

typedef struct 
{
    SPP                         *spp;
    spp_connect_status          status;
    Sink                        sink;
    uint16                      payload_size;
} SPPS_SERVER_CONNECT_CFM_TEST_EXTRA_T;

typedef struct 
{
    SPP                     *spp;
    spp_disconnect_status   status; 
    Sink                    sink;
} SPPS_DISCONNECT_IND_TEST_EXTRA_T;

typedef struct 
{
    SPP                     *spp;
    spp_disconnect_status   status; 
    Sink                    sink;
} SPPS_DISCONNECT_CFM_TEST_EXTRA_T;


/*****************************************************************************/
void SppsHandleComplexMessage(Task task, MessageId id, Message message);

/*****************************************************************************/
void SppsStartServiceTestExtra(Task theAppTask);

/*****************************************************************************/
void SppsStopServiceTestExtra(Task theAppTask);

/*****************************************************************************/
void SppsConnectResponseTestExtra(Task theAppTask, const bdaddr *bd_addr, const bool response, const Sink sink, const uint8 local_server_channel, const uint16 max_payload_size);

/*****************************************************************************/
void SppsDisconnectResponseTestExtra(SPPS *spps);

/*****************************************************************************/
void SppsDisconnectRequestTestExtra(SPPS *spps);

#endif

