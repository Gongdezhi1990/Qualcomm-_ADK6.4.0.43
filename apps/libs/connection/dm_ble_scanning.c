/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_scanning.c      

DESCRIPTION
    This file contains the implementation of Low Energy scan configuration.

NOTES

*/

#include "connection.h"
#include "connection_private.h"

#include <vm.h>

#ifndef DISABLE_BLE
/****************************************************************************
NAME    
    ConnectionDmBleSetScanEnable

DESCRIPTION
    Enables or disables BLE Scanning. The results come back in a 

RETURNS
    void
*/

void ConnectionDmBleSetScanEnable(bool enable)
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_SET_SCAN_ENABLE_REQ);

        prim->scan_enable = (enable) ? 1 : 0;

        /* Always filter duplicates */
        prim->filter_duplicates = 1;

        VmSendDmPrim(prim);
    }
}

/****************************************************************************
NAME    
    ConnectionBleAddAdvertisingReportFilter

DESCRIPTION
    Set a filter for advertising reports so that only those that match the
    filter are reported to the VM. Always an OR operation when adding a filter.

RETURNS
    TRUE if the filter is added, otherwise FALSE if it failed or there was not
    enough memory to add a new filter.
*/
bool ConnectionBleAddAdvertisingReportFilter(
    ble_ad_type     ad_type,
    uint16          interval,
    uint16          size_pattern,
    const uint8*    pattern    
    )
{
#ifdef CONNECTION_DEBUG_LIB
    /* Check parameters. */
    if (interval > BLE_AD_PDU_SIZE)
    {
        CL_DEBUG(("Interval greater than ad data length\n"));
    }
    if (size_pattern == 0 || size_pattern > BLE_AD_PDU_SIZE)
    {
        CL_DEBUG(("Pattern length is zero\n"));
    }
    if (pattern == 0)
    {
        CL_DEBUG(("Pattern is null\n"));
    }
#endif
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
        return FALSE;
    }
    else
    {
        /* Copy the data to a memory slot, which will be freed after
         * the function call.
         *
         * Data is uint8* but trap takes uint16 on xap
         */
        uint8 *pattern2 = (uint8*) PanicUnlessMalloc(size_pattern);
        memmove(pattern2, pattern, size_pattern);
        
        return VmAddAdvertisingReportFilter(
                    0,                          /* Operation is always OR */
                    ad_type,
                    interval,
                    size_pattern,
#ifdef HYDRACORE
                    pattern2
#else
                    (uint16)pattern2
#endif
                    );
    }
}

/****************************************************************************
NAME    
    ConnectionBleClearAdvertisingReportFilter

DESCRIPTION
    Clear any existing filters.

RETURNS
    TRUE if the filters were cleared.
*/
bool ConnectionBleClearAdvertisingReportFilter(void)
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
        return FALSE;
    }
    else
    {
        return VmClearAdvertisingReportFilter();
    }
}

/****************************************************************************
NAME    
    ConnectionDmBleSetScanParametersReq
    
DESCRIPTION
    Set up parameters to be used for BLE scanning. 

RETURNS
    None.
*/
void ConnectionDmBleSetScanParametersReq(
        bool    enable_active_scanning,
        bool    random_own_address,
        bool    white_list_only,
        uint16  scan_interval,
        uint16  scan_window
        )
{
#ifdef CONNECTION_DEBUG_LIB
    /* Check parameters. */
    if (scan_interval < 0x0004 || scan_interval > 0x4000  )
    {
        CL_DEBUG(("scan_interval outside range 0x0004..0x4000\n"));
    }
    if (scan_window < 0x0004 || scan_window > 0x4000  )
    {
        CL_DEBUG(("scan_window outside range 0x0004..0x4000\n"));
    }
    if (scan_window > scan_interval)
    {
        CL_DEBUG(("scan_window must be less than or equal to scan interval\n"));
    }
#endif
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_SET_SCAN_PARAMETERS_REQ);

        prim->scan_type                 = (enable_active_scanning) ? 1 : 0;
        prim->scan_interval             = scan_interval;
        prim->scan_window               = scan_window;
        prim->own_address_type          = (random_own_address) ? 1 : 0;
        prim->scanning_filter_policy    = (white_list_only) ? 1: 0;

        VmSendDmPrim(prim);
    }
}

/****************************************************************************
NAME    
    ConnectionDmBleSetScanResponseDataReq

DESCRIPTION
    Sets BLE Scan Response data (0..31 octets).

RETURNS
   void
*/
void ConnectionDmBleSetScanResponseDataReq(uint8 size_sr_data, const uint8 *sr_data)
{
    
#ifdef CONNECTION_DEBUG_LIB
        /* Check parameters. */
    if (size_sr_data == 0 || size_sr_data > BLE_SR_PDU_SIZE)
    {
        CL_DEBUG(("Data length is zero\n"));
    }
    if (sr_data == 0)
    {
        CL_DEBUG(("Data is null\n"));
    }
#endif
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_SET_SCAN_RESPONSE_DATA_REQ);
        prim->scan_response_data_len = size_sr_data;
        memmove(prim->scan_response_data, sr_data, size_sr_data);
        VmSendDmPrim(prim);
    }
}

#endif /* DISABLE_BLE */
