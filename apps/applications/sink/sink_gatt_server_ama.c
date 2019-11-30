/****************************************************************************
Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_server_ama.c

DESCRIPTION
    Routines to handle messages sent from the GATT AMA Server Task.
*/
#include "stdlib.h"
#include "stdio.h"

#include "vm.h"
#include "sink_ble.h"
#include "sink_gatt.h"
#include "sink_gatt_db.h"
#include "sink_gatt_server_ama.h"
#include "sink_gatt_common.h"
#include "sink_debug.h"
#include "sink_development.h"
#include "sink_private_data.h"
#include "sink_gatt_server.h"
#include "sink_powermanager.h"

#include <csrtypes.h>
#include <message.h>

#ifdef GATT_AMA_SERVER
#include <gatt_ama_server.h>
#endif

#ifdef ENABLE_AMA
#include <ama.h>
#endif

#ifdef GATT_AMA_SERVER

#define DEBUG_GATT_AMA_SERVER

#ifdef DEBUG_GATT_AMA_SERVER
#define GATT_AMA_SERVER_INFO(x) DEBUG(x)
#define GATT_AMA_SERVER_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define GATT_AMA_SERVER_INFO(x)
#define GATT_AMA_SERVER_ERROR(x)
#endif

#define AMA_TX_MTU_SIZE    (178)

#define BAD_SINK ((Sink) 0xFFFF)
#define BAD_SINK_CLAIM (0xFFFF)

/* Assigned numbers for GATT Bluetooth Namespace Descriptors */
#define NAMESPACE_BLUETOOTH_SIG             0x01        /* Bluetooth SIG Namespace */
#define DESCRIPTION_BATTERY_UNKNOWN         0x0000      /* Bluetooth SIG description "unknown" */
#define DESCRIPTION_BATTERY_LOCAL           0x010F      /* Bluetooth SIG description "internal" for local battery*/
#define DESCRIPTION_BATTERY_SECOND          0x0002      /* Bluetooth SIG description "Second" for remote battery*/
#define DESCRIPTION_BATTERY_THIRD           0x0003      /* Bluetooth SIG description "third" for peer battery */

#define ENABLE_NOTIFICATIONS                            /* Enable notifications on the battery server */

#ifdef ENABLE_NOTIFICATIONS
#define AMA_SERVER_ENABLE_NOTIFICATIONS TRUE
#else
#define AMA_SERVER_ENABLE_NOTIFICATIONS FALSE
#endif

#define GATT_AMA_SERVER_NOT_TIME    30
#define GATT_AMA_SERVER_NOT_COUNT   10
#define GATT_AMA_SERVER_NOT_CREDIT_COUNT   10

#define GATT_AMA_SERVER_NOT_LENGTH  20

#define AMA_SERVER_SEND_ONE_NOTIF     0xA4
#define AMA_SERVER_NOT_TEST_START     0xA5
#define AMA_SERVER_UPDATE_PARAM_FAST  0xA6
#define AMA_SERVER_UPDATE_PARAM_SLOW  0xA7
#define AMA_SERVER_BURST_SIZE         0xA8
#define AMA_SERVER_CREATE_ATT_SINK    0xA9

#define AMA_SERVER_GEN_1              0xAA

#define CONNECTION_PARAMTER_UPDATE_LATENCY      0
#define CONNECTION_PARAMTER_INTERVAL_MAX        20
#define CONNECTION_PARAMTER_INTERVAL_MIN        12

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

uint16 ama_burst_size =  GATT_AMA_SERVER_NOT_COUNT;

GAMASS*  sink_gatt_ama_server_task;

static TaskData local_ama_task;

/*******************************************************************************/
uint16 sinkGattAmaServerCalculateSize(void)
{
    /* Local battery server size */
    uint16 size = sizeof(GAMASS);

    GATT_AMA_SERVER_INFO(("AMA[%d]\n", size));
    return size;
}

/*******************************************************************************/
bool sinkGattAmaServerInitialise(uint16 **ptr)
{
    sinkGattUpdateMinimumTxMtu(AMA_TX_MTU_SIZE);

	if (GattAmaServerInit(  (GAMASS *)*ptr, sinkGetBleTask(), NULL , HANDLE_AMA_ALEXA_SERVICE,
	                                 HANDLE_AMA_ALEXA_SERVICE_END))
	{
	    GATT_AMA_SERVER_INFO(("GATT AMA Server initialised\n"));
	    gattServerSetServicePtr(ptr, gatt_server_service_ama);

	    sink_gatt_ama_server_task = (GAMASS *)*ptr;

		/* The size of HRS is also calculated and memory is alocated.
		 * So advance the ptr so that the next Server while initializing.
		 * shall not over write the same memory */
	    *ptr += sizeof(GAMASS);

		/* this is just because the MORE_SPACE and other messages cannot be processed becasue of the GATT MANAger */
		local_ama_task.handler = sinkGattAmaServerMsgHandler;
		return TRUE;
	}

	/* Heart Rate Service library initialization failed */
	GATT_AMA_SERVER_INFO(("GATT AMA Sensor init failed \n"));
	return FALSE;
}

/******************************************************************************/
void sinkGattAmaServerMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

	if(id == GATT_AMA_SERVER_WRITE_IND)
	{
		GATT_AMA_SERVER_INFO((" GATT_AMA_SERVER_WRITE_IND	\n",id ));
	}

	else if(id == GATT_AMA_SERVER_CLIENT_C_CFG)
	{
		uint16 client_config = ((GATT_AMA_SERVER_CLIENT_C_CFG_T*)message)->client_config;

		GATT_AMA_SERVER_INFO((" GATT_AMA_SERVER_CLIENT_C_CFG %d\n",client_config ));

		sinkAmaServerConnectionParameterUpdate(TRUE);
	}
}

/******************************************************************************/
bool sinkAmaServerNotify(uint16 cid, uint16 handle, uint16 size, uint8* p_data)
{
	static uint8 count = 0;
    uint16 index = gattCommonConnectionFindByCid(cid);
	bool ret;
	uint8*  data =  (uint8*)PanicUnlessMalloc(size);

    UNUSED(p_data);

	count++;
	memset(data, count, size);
    handle = HANDLE_AMA_ALEXA_RX_CHAR;
    cid = sink_gatt_ama_server_task->cid;

	/* Send the AMA Server notification. */
    ret = GattAmaServerSendNotification( GATT_SERVER.ama_server,
                          GATT_SERVER.ama_server->cid, handle, size, &data[0]);
	GATT_AMA_SERVER_INFO(("Send AMA Not len %d ret %d index %d \n",size, ret , index ));

	free(data);
	return ret;
}

/******************************************************************************/
void sinkAmaServerConnectEvent(GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T * cfm)
{
    uint16 index = gattCommonConnectionFindByCid(cfm->cid);

	sink_gatt_ama_server_task->cid = cfm->cid;
    GATT_SERVER.ama_server->cid = cfm->cid;

	sink_gatt_ama_server_task->taddr = cfm->taddr;
	GATT_SERVER.ama_server->taddr = cfm->taddr;

	sink_gatt_ama_server_task->mtu = cfm->mtu;
	GATT_SERVER.ama_server->mtu = cfm->mtu;

    GATT_AMA_SERVER_INFO(("BLE AMA Connect Ev taddr ype %d cid  %04x\n", cfm->taddr.type,  cfm->cid ));

	if(cfm->taddr.type == TYPED_BDADDR_RANDOM)
	{
		tp_bdaddr	random_addr;

		random_addr.taddr = cfm->taddr;
		random_addr.transport = cfm->taddr.type;

		VmGetPublicAddress(&random_addr, &sink_gatt_ama_server_task->peer_public_addr);
        AmaTransportStorePeerAddr(&sink_gatt_ama_server_task->peer_public_addr.taddr.addr );
	}
	else
	{
        AmaTransportStorePeerAddr(&cfm->taddr.addr);
	}

}

/******************************************************************************/
void sinkAmaServerDisconnectEvent(uint16 cid, uint16 reason)
{
   /*  uint16 index = gattCommonConnectionFindByCid(cid); */
	GATT_AMA_SERVER_ERROR(("BLE AMA Discon Ev cid  %04x res %04x  \n",cid, reason ));
}

/******************************************************************************/
void sinkAmaServerConnectionParameterUpdate(bool fast)
{
    GATT_AMA_SERVER_INFO(("BLE AMA cannction update fast = %04x  \n",fast ));

    if(fast == FALSE)
    {
        sinkBleSetSlaveConnectionParamsUpdate(&sink_gatt_ama_server_task->taddr);
    }
    else
    {

        GATT_AMA_SERVER_INFO(("Set BLE Updated FAST Slave Connection Params for AMA \n"));

        ConnectionDmBleConnectionParametersUpdateReq(
                    sinkGetBleTask(),
                    &sink_gatt_ama_server_task->taddr,
                    SINK_BLE_AMA_CONN_PARAM_SLAVE_CONN_INTERVAL_MIN,
                    SINK_BLE_AMA_CONN_PARAM_SLAVE_CONN_INTERVAL_MAX,
                    SINK_BLE_AMA_PARAM_SLAVE_CONN_LATENCY,
                    SINK_BLE_AMA_PARAM_SUPERVISION_TIMEOUT,
                    SINK_BLE_AMA_PARAM_CE_LENGTH_MIN,
                    SINK_BLE_AMA_PARAM_CE_LENGTH_MAX
                    );
    }
}

#endif /* GATT_AMA_SERVER */
