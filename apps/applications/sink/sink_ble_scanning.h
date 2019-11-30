/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ble_advertising.h

DESCRIPTION
    BLE Advertising functionality
*/


#ifndef _SINK_BLE_SCANNING_H_
#define _SINK_BLE_SCANNING_H_


#include <connection.h>

#include <csrtypes.h>

/****************************************************************************
NAME    
    bleClearScanData
    
DESCRIPTION
    Function to clear all scan filters
*/
#ifdef GATT_ENABLED
void bleClearScanData(void);
#else
#define bleClearScanData() ((void)(0))
#endif
    

/****************************************************************************
NAME    
    bleStartScanning
    
DESCRIPTION
    Function to enable BLE scanning
    
PARAMETERS
    white_list  Set to TRUE to only scan for white-list devices. FALSE otherwise.
    fast        Flag for speed
*/
#ifdef GATT_ENABLED
void bleStartScanning(bool white_list, bool fast);
#else
#define bleStartScanning(white_list, fast) ((void)(0))
#endif


/****************************************************************************
NAME    
    bleStopScanning
    
DESCRIPTION
    Function to disable BLE scanning
*/
#ifdef GATT_ENABLED
void bleStopScanning(void);
#else
#define bleStopScanning() ((void)(0))
#endif


/****************************************************************************
NAME    
    bleHandleScanResponse
    
DESCRIPTION
    Handles advertising packets received during scanning
*/
#ifdef GATT_ENABLED
void bleHandleScanResponse(const CL_DM_BLE_ADVERTISING_REPORT_IND_T *ind);
#else
#define bleHandleScanResponse(ind) ((void)(0))
#endif

/****************************************************************************
NAME    
    bleAddScanFilters
    
DESCRIPTION
    Utility function to add the scanning filters
*/
#ifdef GATT_ENABLED
void bleAddScanFilters(void);
#else
#define bleAddScanFilters() ((void)(0))
#endif


#endif /* _SINK_BLE_SCANNING_H_ */

