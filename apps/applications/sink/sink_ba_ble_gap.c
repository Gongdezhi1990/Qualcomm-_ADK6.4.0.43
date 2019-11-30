/****************************************************************************
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_ba_ble_gap.c

DESCRIPTION
    BLE GAP functionality
*/

#include "sink_ba_ble_gap.h"
#include "sink_ble.h"
#include "sink_main_task.h"
#include "sink_ba_common.h"
#include "sink_gatt_db.h"
#include "sink_gatt_manager.h"
#include "sink_ba.h"
#include "sink_ble_scanning.h"
#include "sink_ba_broadcaster.h"
#include "sink_ba_receiver.h"
#include "sink_gatt_common.h"

#include <gatt_manager.h>
#include <vm.h>

#include <stdlib.h>

#ifdef ENABLE_BROADCAST_AUDIO
#include "config_definition.h"
#include "sink_ble_config_def.h"
#include <config_store.h>
#include <gatt_broadcast_server_uuids.h>
#include <broadcast_context.h>

#ifdef DEBUG_BA_BLE_GAP
#define BA_BLE_GAP_INFO(x) DEBUG(x)
#define BA_BLE_GAP_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define BA_BLE_GAP_INFO(x)
#define BA_BLE_GAP_ERROR(x)
#endif /* DEBUG_BLE_GAP */

/* Length of 16 bit uuid services */
#define BA_RECEIVER_SERVICES_DATA_LENGTH (0x05)
#define BA_BROADCASTER_SERVICES_DATA_LENGTH (0x05)

/* Size of 16 bit variant IV */
#define SIZE_16_BIT_IV (sizeof(uint8)*2)

/******************************************************************************
  Utility function to set the scan filters for broadcaster to scan only for BA receivers 
*/
static void bleBaSetScanFilters(void)
{
    ble_ad_type ad_type = ble_ad_type_service_16bit_uuid;
    uint16 scan_interval = ADV_SCAN_INTERVAL_SLOW;
    uint16 scan_window = ADV_SCAN_WINDOW_SLOW;
    const uint8 broadcast_advert_filter[] = {UUID_BROADCAST_SERVICE & 0xFF,
                                             UUID_BROADCAST_SERVICE >> 8};
    const uint8 broadcast_2_advert_filter[] = {UUID_BROADCAST_SERVICE_2 & 0xFF,
                                     UUID_BROADCAST_SERVICE_2 >> 8};
    bool random_addr = FALSE;
#ifdef ENABLE_RANDOM_ADDR
    random_addr = TRUE;
#endif

    /* first clear the existing filter */
    ConnectionBleClearAdvertisingReportFilter();

    /* Just update the correct filter for recevier */
    if(BA_RECEIVER_MODE_ACTIVE)
    {
        ad_type = ble_ad_type_service_data;
        scan_interval = ADV_SCAN_INTERVAL_FAST;
        scan_window = ADV_SCAN_WINDOW_FAST;
        /* As receiver, we can also get adverts from different broadcast UUIDs */
        ConnectionBleAddAdvertisingReportFilter(ad_type, 
                                        sizeof(broadcast_2_advert_filter), 
                                        sizeof(broadcast_2_advert_filter), 
                                        broadcast_2_advert_filter);
    }
    
     /* setup advert filter, scan parameters and start scanning.
     * now waiting on a CL_DM_BLE_ADVERTISING_REPORT_IND message. */
    ConnectionBleAddAdvertisingReportFilter(ad_type, 
                                            sizeof(broadcast_advert_filter), 
                                            sizeof(broadcast_advert_filter), 
                                            broadcast_advert_filter);
    
    /* Set the Scan parameters */
    ConnectionDmBleSetScanParametersReq(TRUE, random_addr, FALSE,
                                        scan_interval,
                                        scan_window);
}

/******************************************************************************
  Utility function to set the advertising parameters for broadcaster & receiver 
*/
static void bleBaSetAdParams(uint16 adv_interval_min, uint16 adv_interval_max, ble_adv_type adv_type)
{
    ble_adv_params_t adv_params;
    bool random_addr = FALSE;

#ifdef ENABLE_RANDOM_ADDR
     random_addr = TRUE;
#endif

    adv_params.undirect_adv.adv_interval_min = adv_interval_min; 
    adv_params.undirect_adv.adv_interval_max = adv_interval_max;
    adv_params.undirect_adv.filter_policy = ble_filter_none;

    /* configure fast advertising rate */
    ConnectionDmBleSetAdvertisingParamsReq(adv_type, random_addr, BLE_ADV_CHANNEL_ALL, &adv_params);
}

/******************************************************************************
  Utility function to start BLE advertising
*/
static void bleStartAdvertising(void)
{
    sinkBleSetGapState(ble_gap_state_scanning_advertising);
    /* Change advertising params */
    gapSetAdvertisingParamsDefault();
    /* Start sending advertisments */
    sinkGattManagerStartAdvertising();
}

/*******************************************************************************/
uint8* setupBroadcasterIvServiceData(uint8* ad_data, uint8* space)
{
    broadcast_encr_config_t *config = BroadcastContextGetEncryptionConfig();

    if(config)
       ad_data = bleAddHeaderToAdData(ad_data, space, BA_BROADCASTER_SERVICES_DATA_LENGTH, 
                                                  ble_ad_type_service_data);
    else
       ad_data = bleAddHeaderToAdData(ad_data, space, 0x03, ble_ad_type_service_data);

    ad_data = bleAddServiceUuidToAdData(ad_data, space, UUID_BROADCAST_SERVICE);

    if(config)
    {
        *ad_data = (config->variant_iv & 0xFF);
        *(ad_data + 1) = ((config->variant_iv >> 8) & 0xFF);
        BA_BLE_GAP_INFO(("0x%02x%02x ", ad_data[1], ad_data[0]));
        ad_data += SIZE_16_BIT_IV;
        *space -= SIZE_16_BIT_IV;
    }
    return ad_data;
}

/*******************************************************************************/
uint8* setupReceiverAssociationServiceData(uint8* ad_data, uint8* space)
{
    ad_data = bleAddHeaderToAdData(ad_data, space, BA_RECEIVER_SERVICES_DATA_LENGTH, 
                                      ble_ad_type_service_16bit_uuid);
    ad_data = bleAddServiceUuidToAdData(ad_data, space, UUID_BROADCAST_SERVICE);
    ad_data = bleAddServiceUuidToAdData(ad_data, space, UUID_BROADCAST_SERVICE_2);

    return ad_data;
}

/********************************************************************************/
void gapBaStartAssociationTimer(void)
{
    MessageCancelFirst(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT);
    MessageSendLater(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT, 0, D_SEC(sinkBroadcastAudioGetAssociationTimeOut()));
}

/********************************************************************************/
void gapBaStopAssociationTimer(void)
{
    MessageCancelFirst(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT);
    gapBaSetAssociationInProgress(FALSE);
}

/*******************************************************************************/
void gapBaRestartAssociationTimer(void)
{
    /* Should be already in association to restart it */
    if(gapBaGetAssociationInProgress())
    {
        MessageSendLater(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT, 0, D_SEC(sinkBroadcastAudioGetAssociationTimeOut()));
    }
}

/*******************************************************************************/
void gapBaSetAssociationInProgress(bool assoc_in_progress)
{
    GAP.ba_association_in_progress = assoc_in_progress;
}

/*******************************************************************************/
bool gapBaGetAssociationInProgress(void)
{
    return GAP.ba_association_in_progress;
}

/*******************************************************************************/
uint16* gapBaGetReferenceOfAssociationInProgress(void)
{
    return (uint16*) &GAP.ba_association_in_progress;
}

/********************************************************************************/
void gapBaSetBroadcastToAdvert(bool requires_advertising)
{
    GAP.adv.ba_advert_active = requires_advertising;
}

/********************************************************************************/
bool gapBaRequiresBroadcastToAdvert(void)
{
    return (GAP.adv.ba_advert_active) ? TRUE : FALSE;
}

/********************************************************************************/
void gapBaSetBroadcastToScan(bool requires_scanning)
{
    GAP.scan.ba_scan_active = requires_scanning;
}

/********************************************************************************/
bool gapBaRequiresBroadcastToScan(void)
{
    return (GAP.scan.ba_scan_active) ? TRUE : FALSE;
}


/********************************************************************************/
bool gapBaStartScanning(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Start scanning:  \n"));
    /* In broadcast mode, check which role we are in and scan accordingly */
    if((BA_BROADCASTER_MODE_ACTIVE) && gapBaGetAssociationInProgress())
    {
        BA_BLE_GAP_INFO((" Connectable BA Association data\n"));
        /* Set the scan filter for scanning only BA receivers */
        bleBaSetScanFilters();
        /* Enable Scanning */
        ConnectionDmBleSetScanEnable(TRUE);
    }
    else if((BA_RECEIVER_MODE_ACTIVE) && gapBaRequiresBroadcastToScan())
    {
        BA_BLE_GAP_INFO((" Non-Conntable Variant IV data\n"));
        /* Start scanning for variant IV */
        bleBaSetScanFilters();
        /* Enable Scanning */
        ConnectionDmBleSetScanEnable(TRUE);
    }
    else
    {
        /* Ignore this scan request, as we are in broadcast mode, but don't require to scan */
        BA_BLE_GAP_INFO(("GAP: IGNORING the scanning, since BA is active & neither are we assocaiting or broadcasting\n"));
        return FALSE;
    }

    return TRUE;
}

/********************************************************************************/
void gapBaStopScanning(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Stop scanning \n"));

    /* stop scanning for adverts from CSB receivers. */
    ConnectionDmBleSetScanEnable(FALSE);
    /* cancel any pending CL_DM_BLE_ADVERTISING_REPORT_IND message */
    MessageCancelAll(sinkGetMainTask(), CL_DM_BLE_ADVERTISING_REPORT_IND);
    /* If we stopped scanning, just because we connected to receiver on association,
        then we just need to disable scanning. However we can keep the filters as 
        scanning for new receiver can trigger after we are done with current receiver */
    if(!gapBaGetAssociationInProgress())
    {
        /* clear the advertising report filter */
        ConnectionBleClearAdvertisingReportFilter();
        /* Add the standalone filters back */
        bleAddScanFilters();
    }
}

/********************************************************************************/
void gapBaTriggerAdvertising(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Trigger Advertising after setting the advert params: "));
    if((BA_BROADCASTER_MODE_ACTIVE) && gapBaRequiresBroadcastToAdvert())
    {
        if(!((ConnectionDmBleCheckTdlDeviceAvailable() && !gattCommonIsMaxConnReached(ble_gap_role_peripheral))
                   ||(sinkBleGapIsBondable())))
        {
            BA_BLE_GAP_INFO((" Non-Conntable Variant IV data\n"));
            /* start the adverts */
            ConnectionDmBleSetAdvertiseEnable(TRUE);
        }
    }
    else if((BA_RECEIVER_MODE_ACTIVE) && gapBaGetAssociationInProgress())
    {
        BA_BLE_GAP_INFO((" Connectable BA Association data\n"));
        /* Start sending advertisments */
        sinkGattManagerStartAdvertising();
    }
    else
    {
        BA_BLE_GAP_INFO((" Ignore this\n"));
        /* Do nothing */
    }
}

/********************************************************************************/
bool gapBaStartAdvertising(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Start Advertising: "));
    if((BA_BROADCASTER_MODE_ACTIVE) && gapBaRequiresBroadcastToAdvert())
    {
        if((ConnectionDmBleCheckTdlDeviceAvailable() && !gattCommonIsMaxConnReached(ble_gap_role_peripheral))
                       ||(sinkBleGapIsBondable()))
        {
            BA_BLE_GAP_INFO((" Connectable standalone bondable data with BA variant IV advert\n"));
            /* Allow standalone advertisements if in bondable state of broadcast mode or if there exists BLE device
            in PDL but it's not connected */
            bleStartAdvertising();
        }
        else
        {
            BA_BLE_GAP_INFO((" Non-Conntable Variant IV data\n"));
            bleBaSetAdParams(ADV_INTERVAL_MIN_SLOW, ADV_INTERVAL_MAX_SLOW, ble_adv_nonconn_ind);
            /* Some platform requires delay before triggering advertisement. So wait for 
               CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM to trigger advertisement */
        }
    }
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        if(gapBaGetAssociationInProgress())
        {
            BA_BLE_GAP_INFO((" Connectable BA Association data\n"));
            /* Update the advertisement data for BA */
            bleBaSetAdParams(ADV_INTERVAL_MIN_FAST, ADV_INTERVAL_MAX_FAST, ble_adv_ind);
           /* Some platform requires delay before triggering advertisement. So wait for 
               CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM to trigger advertisement */
        }
        else if((ConnectionDmBleCheckTdlDeviceAvailable() && !gattCommonIsMaxConnReached(ble_gap_role_peripheral))
                       ||(sinkBleGapIsBondable()))
        {
            BA_BLE_GAP_INFO((" Connectable standalone bondable data\n"));
            /* Allow standalone advertisements if in bondable state of broadcast mode or if there exists BLE device
            in PDL but it's not connected */
            bleStartAdvertising();
        }
    }
    else if((ConnectionDmBleCheckTdlDeviceAvailable() && !gattCommonIsMaxConnReached(ble_gap_role_peripheral))
                       ||(sinkBleGapIsBondable()))
    {
        BA_BLE_GAP_INFO((" Connectable standalone bondable data\n"));
        /* Allow standalone advertisements if in bondable state of broadcast mode or if there exists BLE device
        in PDL but it's not connected */
        bleStartAdvertising();
    }
    else if (gattCommonIsMaxConnReached(ble_gap_role_peripheral))
    {
        /* A central deivce is connected. so stop the advertisement */
        sinkBleSetGapState(ble_gap_state_scanning_advertising);
        return FALSE;
    }
    else
    {
        /* Just ignore */
        BA_BLE_GAP_INFO(("GAP: IGNORING the advertsising, since BA is active & we are not neither are we broadcasting, associating or bonding\n"));
        return FALSE;
    }
    return TRUE;
}

/********************************************************************************/
static void gapBaStopAdvertising(void)
{
    BA_BLE_GAP_INFO(("BA BLE: Stop Advertising\n"));
    ConnectionDmBleSetAdvertiseEnable(FALSE);
}

/******************************************************************************/
void gapBaStartAssociation(void)
{
    bool is_asso_started = FALSE;
    BA_BLE_GAP_INFO(("GAP sinkBaBleGapStartAssociation\n"));

    /* Now in assoicating state */
    sinkBleSetGapBaAssociationState(ble_gap_ba_association_state_associating);
    sinkBleSetGapBondState(ble_gap_bond_state_bondable);
    gapBaSetAssociationInProgress(TRUE);
    
    if(BA_BROADCASTER_MODE_ACTIVE)
    {
        is_asso_started = gapStartScanning(TRUE);
    }
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        /* Stop scanning for BA and non BA adverts */
        gapStopScanning();
        sinkBleGapStartReadLocalName(ble_gap_read_name_associating);
        is_asso_started = TRUE;
    }
    else
        /* Just igore, we can't be in this standalone mode to do association */
        return;

    /* If scanning started, start the timer */
    if (is_asso_started)
    {
        /* Indicate on LED that association is started */
        MessageSend(sinkGetMainTask(), EventSysBAAssociationStart, 0);
        /* Start association timer */
        gapBaStartAssociationTimer();
    }
}

/******************************************************************************/
void gapBaStopAssociation(void)
{
    BA_BLE_GAP_INFO(("GAP gapBaStopAssociation\n"));

    /* Reset the association flag */
    gapBaSetAssociationInProgress(FALSE);
        
    if(BA_BROADCASTER_MODE_ACTIVE)
        gapStopScanning();
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        gapStopAdvertising(gapGetAdvSpeed());
        /* In case we are in receiver role, then we need to indicate that Receiver is not associated */
        MessageSend(sinkGetMainTask() , EventSysBAReceiverNotAssociated, NULL ) ;
    }

    /* Now in normal state */
    sinkBleSetGapBaAssociationState(ble_gap_ba_association_state_non_associating);
    
    sinkBleSetGapBondState(ble_gap_bond_state_non_bondable);

    /* Indicate on LED that association is ended */
    MessageSend(sinkGetMainTask(), EventSysBAAssociationEnd, 0);
}

/******************************************************************************/
void gapBaStartBroadcast(void)
{
    BA_BLE_GAP_INFO(("GAP gapBaStartBroadcast\n"));

    if(BA_BROADCASTER_MODE_ACTIVE)
    {
        if(ConnectionDmBleCheckTdlDeviceAvailable())
        {
            if(gattCommonIsMaxConnReached(ble_gap_role_peripheral))
            {
                /* A central device is already connected so start reading local name to do non-connectable IV adverts */
                sinkBleGapStartReadLocalName(ble_gap_read_name_broadcasting);
            }
            else
            {
                BA_BLE_GAP_INFO(("GAP start reading local name to do standalone & non-connectable IV adverts\n"));
                sinkBleGapStartReadLocalName(ble_gap_read_name_advertising_broadcasting);
            }
        }
        else
        {
            /* There is no BLE deivce is PDL so start reading local name to do non-connectable IV adverts */
            BA_BLE_GAP_INFO(("GAP start reading local name to do non-connectable IV adverts\n"));
            sinkBleGapStartReadLocalName(ble_gap_read_name_broadcasting);
        }
        gapBaSetBroadcastToAdvert(TRUE);
    }
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        if(ConnectionDmBleCheckTdlDeviceAvailable())
        {
            if(!(gattCommonIsMaxConnReached(ble_gap_role_peripheral)))
            {
                BA_BLE_GAP_INFO(("GAP start reading local name to do standalone adverts\n"));
                sinkBleGapStartReadLocalName(ble_gap_read_name_advertising);
            }
            /* We need to be in scanning/advertising state */
            sinkBleSetGapState(ble_gap_state_scanning_advertising);
        }

        /* First check if we already have an associated broadcaster if yes then start scanning 
            else remain in same state */
        if(!sinkReceiverIsAssociated())
            return;

        /* Going to scan for fresh Variant IV */
        sinkReceiverResetVariantIv();
        gapBaSetBroadcastToScan(TRUE);
        gapStartScanning(TRUE);
    }

    /* We need to be in scanning/advertising state */
    sinkBleSetGapState(ble_gap_state_scanning_advertising);
}

/******************************************************************************/
void gapBaStopBroadcast(void)
{
    BA_BLE_GAP_INFO(("GAP gapBaStopBroadcast\n"));

    if(BA_BROADCASTER_MODE_ACTIVE)
    {
        /* Looks like user pressed bondable button. Stop non-conn IV adverts */
        gapBaSetBroadcastToAdvert(FALSE);
        gapBaStopAdvertising();
    }
    else if(BA_RECEIVER_MODE_ACTIVE)
    {
        /* Stop scanning for non-conn IV */
        gapBaSetBroadcastToScan(FALSE);
        gapBaStopScanning();
    }
    else
    {
        BA_BLE_GAP_INFO(("GAP Switch to standalone mode, start normal advertising/scanning\n"));
        /* If there is mode switch from BA mode to standalone
             then we need to stop broadcasting and start standalone 
             advertising/scanning */
        /* We might be broadcasting IV */
        gapBaSetBroadcastToAdvert(FALSE);
        gapBaStopAdvertising();

        /* Or scanning for non-conn IV */
        gapBaSetBroadcastToScan(FALSE);
        gapBaStopScanning();

        /* Now start standalone adverts */
        sinkBleSetGapState(ble_gap_state_scanning_advertising);
        gapStartScanning(TRUE);
        sinkBleGapStartReadLocalName(ble_gap_read_name_advertising);

        return;
    }

    /* In case we are stopping broadcasting, then we need to move to following state:
        1. Idle -> If there is no peripheral connection
        2. Fully Connected (1 link reserved for association) -> If there is one peripheral link
        3. Scanning Advertising -> If there exists a BLE device in PDL but peripheral link does not exist.
           This is required otherwise peripheral link connection would not go through.(BLE state machine would be in IDLE state)
    */
    sinkBleSetGapState(ble_gap_state_idle);
    if(gattCommonGetNumOfConn(ble_gap_role_peripheral) == (MAX_BLE_CONNECTIONS - 1))
        sinkBleSetGapState(ble_gap_state_fully_connected);
    else if(ConnectionDmBleCheckTdlDeviceAvailable() && !(gattCommonIsMaxConnReached(ble_gap_role_peripheral)))
        sinkBleSetGapState(ble_gap_state_scanning_advertising);
}

/******************************************************************************/
void gapBaActionCancelAssociation(void)
{
    /* First stop association */
    gapBaStopAssociation();

    /* If receiver role, then based on which state association we called, move to that state
        back */
    if(BA_RECEIVER_MODE_ACTIVE)
    {
        if(gattCommonGetNumberOfConn() == (MAX_BLE_CONNECTIONS-1))
            sinkBleSetGapState(ble_gap_state_fully_connected);
        else if(!ConnectionDmBleCheckTdlDeviceAvailable())
            sinkBleSetGapState(ble_gap_state_idle);
        else
            /* We need to be in scanning/advertising state */
            sinkBleSetGapState(ble_gap_state_scanning_advertising);
    }
    /* Nothing happened after association, lets move to scan_advert state */
    gapBaStartBroadcast();
}

#endif /* ENABLE_BROADCAST_AUDIO */

