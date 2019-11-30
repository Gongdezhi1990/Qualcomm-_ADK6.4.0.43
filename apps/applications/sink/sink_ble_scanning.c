/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_ble_scanning.c

DESCRIPTION
    BLE Scanning functionality
*/

#include "sink_ble_scanning.h"

#include "sink_ble_advertising.h"
#include "sink_gatt_manager.h"
#include "sink_debug.h"
#include "sink_ble.h"
#include "sink_ba_receiver.h"

#ifdef GATT_AMS_CLIENT
#include "sink_gatt_client_ams.h"
#endif
#include "sink_gatt_client_battery.h"
#include "sink_gatt_client_hid.h"
#include "sink_gatt_client_dis.h"
#include "sink_gatt_client_ias.h"
#include "sink_gatt_client_ancs.h"
#include "sink_gatt_client_spc.h"
#include "sink_gatt_client_hrs.h"

#include <gatt.h>
#include <csrtypes.h>

#ifdef GATT_ENABLED


/* Macro for BLE AD Data Debug */
#ifdef DEBUG_BLE
#include <stdio.h>
#define BLE_SCAN_DEBUG(x) DEBUG(x)
#else
#define BLE_SCAN_DEBUG(x) 
#endif


/****************************************************************************/
void bleClearScanData(void)
{
    BLE_SCAN_DEBUG(("BLE: Clear scan filters\n"));
    ConnectionBleClearAdvertisingReportFilter();
}


/****************************************************************************/
void bleStartScanning(bool white_list, bool fast)
{
    ble_scanning_parameters_t scan_params;
    bool random_addr = FALSE;
#ifdef ENABLE_RANDOM_ADDR
    random_addr = TRUE;
#endif 
    BLE_SCAN_DEBUG(("BLE: Start scanning fast=[%u] white_list=[%u]\n", fast, white_list));
    
    /* Get the scan parameters for the current mode */
    sinkBleGetScanningParameters(fast, &scan_params);
    
    /* Set the BLE Scan parameters */
    ConnectionDmBleSetScanParametersReq(FALSE, random_addr, white_list, scan_params.interval, scan_params.window);

    /* Enable scanning */
    ConnectionDmBleSetScanEnable(TRUE);
}


/****************************************************************************/
void bleStopScanning(void)
{
    /* Stop scanning, this will stop advertisements from BLE devices to be recieved by the application */
    BLE_SCAN_DEBUG(("BLE: Stop scanning\n"));
    ConnectionDmBleSetScanEnable(FALSE);
}


/****************************************************************************/
void bleHandleScanResponse(const CL_DM_BLE_ADVERTISING_REPORT_IND_T *ind)
{
    /* BA receiver handles all reponses when active. */
    if(BA_RECEIVER_MODE_ACTIVE)
    {
        sinkReceiverHandleIVAdvert(ind);
    }
    else
    {
		if((ind->event_type == ble_adv_event_connectable_undirected) ||
                (ind->event_type == ble_adv_event_connectable_directed))
        {
			ble_gap_event_t event; 
			ble_gap_event_args_t args;
		
			/* Send GAP event to attempt connection to advertising device */
			event.id = ble_gap_event_central_conn_attempt;
			args.central_conn_attempt.current_taddr = ind->current_taddr;
			args.central_conn_attempt.permanent_taddr = ind->permanent_taddr;
			event.args = &args;
			sinkBleGapEvent(event);
		}
    }
}

/****************************************************************************/
void bleAddScanFilters(void)
{
    uint8  advertising_filter = sinkBleGapGetAdvertisingFilter();

    /* Setup advertising filters for supported services */
    if(advertising_filter & (1<<ANCS_AD_BIT))
    {
        sinkGattAncsClientSetupAdvertisingFilter();
    }
#ifdef GATT_AMS_CLIENT
    if(advertising_filter & (1<<AMS_AD_BIT))
    {
        sinkGattAmsClientSetupAdvertisingFilter();
    }
#endif
    if(advertising_filter & (1<<BATTERY_AD_BIT))
    {
        gattBatteryClientSetupAdvertisingFilter();
    }
    if(advertising_filter & (1<<DIS_AD_BIT))
    {
        sinkGattDisClientSetupAdvertisingFilter();
    }
    if(advertising_filter & (1<<HID_AD_BIT))
    {
        sinkGattHidClientSetupAdvertisingFilter();
    }
    if(advertising_filter & (1<<IAS_AD_BIT))
    {
        sinkGattIasClientSetupAdvertisingFilter();
    }
    if(advertising_filter & (1<<SPC_AD_BIT))
    {
        sinkGattSpClientSetupAdvertisingFilter();
    } 
    if(advertising_filter & (1<<HRS_AD_BIT))
    {
        sinkGattHrsClientSetupAdvertisingFilter();
    }
}


#endif /* GATT_ENABLED */
