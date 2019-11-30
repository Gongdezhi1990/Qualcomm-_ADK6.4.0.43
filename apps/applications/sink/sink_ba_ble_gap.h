/****************************************************************************
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ba_ble_gap.h

DESCRIPTION
    BA BLE GAP functionality
*/

#ifndef _SINK_BA_BLE_GAP_H_
#define _SINK_BA_BLE_GAP_H_

#include "sink_ble_gap.h"
#include "sink_ble_advertising.h"

#include <connection.h>
#include <csrtypes.h>

/* Advertising TX parameters for fast and slow advertising. */
#define ADV_INTERVAL_MIN_FAST   0x0020
#define ADV_INTERVAL_MAX_FAST   0x0020
#define ADV_INTERVAL_MIN_SLOW   0x0800
#define ADV_INTERVAL_MAX_SLOW   0x0800

/* Length of Advertisement Flags */
#define FLAGS_DATA_LENGTH (0x02)

/*******************************************************************************
NAME
    gapBaStartScanning
    
DESCRIPTION
    Start scanning for BA devices.
    
PARAMETERS
    None
    
RETURNS
    TRUE if the scanning was started. FALSE otherwise.
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool gapBaStartScanning(void);
#else
#define gapBaStartScanning() (FALSE)
#endif

/*******************************************************************************
NAME
    gapBaStopScanning
    
DESCRIPTION
    Stop scanning for BA devices.
    
PARAMETERS
    None
    
RETURNS
    TRUE if the scanning was started. FALSE otherwise.
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaStopScanning(void);
#else
#define gapBaStopScanning() ((void)(0))
#endif

/*******************************************************************************
NAME
    gapBaTriggerAdvertising
    
DESCRIPTION
    Trigger actual advertising of data
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaTriggerAdvertising(void);
#else
#define gapBaTriggerAdvertising() ((void)(0))
#endif

/*******************************************************************************
NAME
    gapBaStartAdvertising
    
DESCRIPTION
    Start advertising BA adverts.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool gapBaStartAdvertising(void);
#else
#define gapBaStartAdvertising() (FALSE)
#endif

/*******************************************************************************
NAME
    gapBaStartAssociation
    
DESCRIPTION
    Interface to start broadcast association
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaStartAssociation(void);
#else
#define gapBaStartAssociation() ((void)(0))
#endif

/*******************************************************************************
NAME
    gapBaStopAssociation
    
DESCRIPTION
    Interface to stop broadcast association
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaStopAssociation(void);
#else
#define gapBaStopAssociation() ((void)(0))
#endif

/*******************************************************************************
NAME
    gapBaStartAssociationTimer
    
DESCRIPTION
    Starts the timer for association.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaStartAssociationTimer(void);
#else
#define gapBaStartAssociationTimer() ((void)(0))
#endif

/*******************************************************************************
NAME
    gapBaStopAssociationTimer
    
DESCRIPTION
    Stops the timer for association.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaStopAssociationTimer(void);
#else
#define gapBaStopAssociationTimer() ((void)(0))
#endif

/*******************************************************************************
NAME
    gapBaRestartAssociationTimer
    
DESCRIPTION
    restart the timer for association.
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaRestartAssociationTimer(void);
#else
#define gapBaRestartAssociationTimer() ((void)(0))
#endif

/*******************************************************************************
NAME
    gapBaSetAssociationInProgress
    
DESCRIPTION
    Utility function to set the association progress flag
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaSetAssociationInProgress(bool assoc_in_progress);
#else
#define gapBaSetAssociationInProgress(assoc_in_progress) ((void)(0))
#endif

/*******************************************************************************
NAME
    gapBaGetAssociationInProgress
    
DESCRIPTION
    Utility function to set the association progress flag
    
PARAMETERS
    None
    
RETURNS
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool gapBaGetAssociationInProgress(void);
#else
#define gapBaGetAssociationInProgress() (FALSE)
#endif

/*******************************************************************************
NAME
    gapBaGetReferenceOfAssociationInProgress
    
DESCRIPTION
    Utility function to get the reference of association progress flag
    
PARAMETERS
    None
    
RETURNS
    uint16* reference of association progress flag
*/
#ifdef ENABLE_BROADCAST_AUDIO
uint16* gapBaGetReferenceOfAssociationInProgress(void);
#else
#define gapBaGetReferenceOfAssociationInProgress() (NULL)
#endif

/*******************************************************************************
NAME    
    gapBaSetBroadcastToAdvert
    
DESCRIPTION
    Utility function to set the broadcaster advertising (non-conn) IV
    
PARAMETERS
    requires_advertising Boolean to set it TRUE or FALSE
    
RETURN
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaSetBroadcastToAdvert(bool requires_advertising);
#else
#define gapBaSetBroadcastToAdvert(requires_advertising) ((void)(0))
#endif

/*******************************************************************************
NAME    
    gapBaRequiresBroadcastToAdvert
    
DESCRIPTION
    Utility function to check if broadcaster needs to advert (non-conn) IV
    
PARAMETERS
    None
    
RETURN
    TRUE if broadcaster needs to advert, else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool gapBaRequiresBroadcastToAdvert(void);
#else
#define gapBaRequiresBroadcastToAdvert() (FALSE)
#endif

/*******************************************************************************
NAME    
    gapBaSetBroadcastToScan
    
DESCRIPTION
    Utility function to set the broadcast system to scan
    
PARAMETERS
    requires_scanning Boolean to set it TRUE or FALSE
    
RETURN
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaSetBroadcastToScan(bool requires_scanning);
#else
#define gapBaSetBroadcastToScan(requires_scanning) ((void)(0))
#endif

/*******************************************************************************
NAME    
    gapBaRequiresBroadcastToScan
    
DESCRIPTION
    Utility function to check if broadcaster needs to scan (non-conn) IV
    
PARAMETERS
    None
    
RETURN
    TRUE if broadcaster needs to scan, else FALSE
*/
#ifdef ENABLE_BROADCAST_AUDIO
bool gapBaRequiresBroadcastToScan(void);
#else
#define gapBaRequiresBroadcastToScan() (FALSE)
#endif

/*******************************************************************************
NAME    
    gapBaStartBroadcast
    
DESCRIPTION
    This function triggers either advertising variant IV or scanning for one
    
PARAMETERS
    None
    
RETURN
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaStartBroadcast(void);
#else
#define gapBaStartBroadcast() ((void)(0))
#endif

/*******************************************************************************
NAME    
    gapBaStopBroadcast
    
DESCRIPTION
    This function stops either advertising variant IV or scanning for one
    
PARAMETERS
    None
    
RETURN
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaStopBroadcast(void);
#else
#define gapBaStopBroadcast() ((void)(0))
#endif

/*******************************************************************************
NAME    
    gapBaActionCancelAssociation
    
DESCRIPTION
    This function acts on association timeout/cancellation, which state to move and what actions to take
    
PARAMETERS
    None
    
RETURN
    None
*/
#ifdef ENABLE_BROADCAST_AUDIO
void gapBaActionCancelAssociation(void);
#else
#define gapBaActionCancelAssociation() ((void)(0))
#endif

/*******************************************************************************
NAME    
    setupBroadcasterIvServiceData
    
DESCRIPTION
    This function can be used to set the service data for broadcasting variant IV
    
PARAMETERS
    uint8* ad_data   pointer to the advertisement data buffer
    uint8* space space left in advertisement data buffer
    
RETURN
    uint8* updated ad_data pointer
*/
#ifdef ENABLE_BROADCAST_AUDIO
uint8* setupBroadcasterIvServiceData(uint8* ad_data, uint8* space);
#else
#define setupBroadcasterIvServiceData(ad_data, space) (ad_data)
#endif

/*******************************************************************************
NAME    
    setupReceiverAssociationServiceData
    
DESCRIPTION
    This function can be used to set the service data for advertising broadcast service UUID 
     Used by BA receiver during association process
PARAMETERS
    uint8* ad_data   pointer to the advertisement data buffer
    uint8* space space left in advertisement data buffer
    
RETURN
    uint8* updated ad_data pointer
*/
#ifdef ENABLE_BROADCAST_AUDIO
uint8* setupReceiverAssociationServiceData(uint8* ad_data, uint8* space);
#else
#define setupReceiverAssociationServiceData(ad_data, space) (ad_data)
#endif

#endif /* _SINK_BLE_GAP_H_ */
