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
#include "bdaddr.h"
#include "dm_ble_advertising.h"

#include <vm.h>

#ifndef DISABLE_BLE

/****************************************************************************
NAME    
    ConnectionDmBleSetAdvertisingDataReq

DESCRIPTION
    Sets BLE Advertising data (0..31 octets).

RETURNS
   void
*/
void ConnectionDmBleSetAdvertisingDataReq(uint8 size_ad_data, const uint8 *ad_data)
{
    
#ifdef CONNECTION_DEBUG_LIB
        /* Check parameters. */
    if (size_ad_data == 0 || size_ad_data > BLE_AD_PDU_SIZE)
    {
        CL_DEBUG(("Pattern length is zero\n"));
    }
    if (ad_data == 0)
    {
        CL_DEBUG(("Pattern is null\n"));
    }
#endif
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_SET_ADVERTISING_DATA_REQ);
        prim->advertising_data_len = size_ad_data;
        memmove(prim->advertising_data, ad_data, size_ad_data);
        VmSendDmPrim(prim);
    }
}

/****************************************************************************
NAME    
    ConnectionDmBleSetAdvertiseEnable

DESCRIPTION
    Enable or Disable BLE Advertising.

RETURNS
   void
*/

void ConnectionDmBleSetAdvertiseEnable(bool enable)
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_SET_ADVERTISE_ENABLE_REQ);

        prim->advertising_enable = (enable) ? 1 : 0;

        VmSendDmPrim(prim);
    }
}

/****************************************************************************
NAME    
    ConnectionDmBleSetAdvertisingParametersReq

DESCRIPTION
    Sets BLE Advertising parameters

RETURNS
   void
*/
void ConnectionDmBleSetAdvertisingParamsReq( 
        ble_adv_type adv_type,
        bool random_own_address,
        uint8  channel_map,
        const ble_adv_params_t *adv_params 
        )
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_C(DM_HCI_ULP_SET_ADVERTISING_PARAMETERS_REQ);

        /* Set defaults to avoid HCI validation failures */
        prim->direct_address_type = HCI_ULP_ADDRESS_PUBLIC;
        prim->adv_interval_max = 0x0800; /* 1.28s */
        prim->adv_interval_min = 0x0800;
        prim->advertising_filter_policy = HCI_ULP_ADV_FP_ALLOW_ANY;

        switch(adv_type)
        {
            case ble_adv_ind:
                prim->advertising_type =
                    HCI_ULP_ADVERT_CONNECTABLE_UNDIRECTED;
                break;
            case ble_adv_direct_ind:
            case ble_adv_direct_ind_high_duty:
                prim->advertising_type = 
                    HCI_ULP_ADVERT_CONNECTABLE_DIRECTED_HIGH_DUTY;
                break;
            case ble_adv_scan_ind:
                prim->advertising_type = 
                    HCI_ULP_ADVERT_DISCOVERABLE;
                break;
            case ble_adv_nonconn_ind:
                prim->advertising_type = 
                    HCI_ULP_ADVERT_NON_CONNECTABLE;
                break;
            case ble_adv_direct_ind_low_duty:
                prim->advertising_type = 
                    HCI_ULP_ADVERT_CONNECTABLE_DIRECTED_LOW_DUTY;
                break;
        } 

        prim->own_address_type = 
            (random_own_address) ? 
                HCI_ULP_ADDRESS_RANDOM : HCI_ULP_ADDRESS_PUBLIC;
        
        channel_map &= BLE_ADV_CHANNEL_ALL;

        prim->advertising_channel_map = channel_map & BLE_ADV_CHANNEL_ALL; 

        if (adv_type ==  ble_adv_direct_ind ||
            adv_type ==  ble_adv_direct_ind_high_duty ||
            adv_type ==  ble_adv_direct_ind_low_duty )
        {
            /* Without an address, this cannot proceed. */
            if (
                    !adv_params || 
                    BdaddrIsZero(&adv_params->direct_adv.direct_addr)
               )
                Panic();

            prim->direct_address_type = 
                (adv_params->direct_adv.random_direct_address) ?
                    HCI_ULP_ADDRESS_RANDOM : HCI_ULP_ADDRESS_PUBLIC;

            BdaddrConvertVmToBluestack( 
                    &prim->direct_address,
                    &adv_params->direct_adv.direct_addr
                    ); 
        }
        else
        {
            if (adv_params)
            {
                /* These params are validated by HCI. */
                prim->adv_interval_min 
                    = adv_params->undirect_adv.adv_interval_min;
                prim->adv_interval_max
                    = adv_params->undirect_adv.adv_interval_max;

                switch (adv_params->undirect_adv.filter_policy)
                {
                    case ble_filter_none:
                        prim->advertising_filter_policy =
                            HCI_ULP_ADV_FP_ALLOW_ANY;
                        break;
                    case ble_filter_scan_only:
                        prim->advertising_filter_policy = 
                            HCI_ULP_ADV_FP_ALLOW_CONNECTIONS;
                        break;
                    case ble_filter_connect_only:
                        prim->advertising_filter_policy = 
                            HCI_ULP_ADV_FP_ALLOW_SCANNING;
                        break;
                    case ble_filter_both:
                        prim->advertising_filter_policy = 
                            HCI_ULP_ADV_FP_ALLOW_WHITELIST;
                        break;    
                }

                /* Set the direct address & type to 0, as they are not used. */
                prim->direct_address_type = 0;
                BdaddrSetZero(&prim->direct_address);
            }
            /* otherwise, if 'adv_params' is null, defaults are used. */
        }

        VmSendDmPrim(prim);
    }
}


/****************************************************************************
NAME    
    connectionHandleDmBleAdvParamUpdateInd

DESCRIPTION
    Handle the DM_ULP_ADV_PARAM_UPDATE_IND message from Bluestack and pass it
    on to the appliction that initialised the CL.

RETURNS
    void
*/
void connectionHandleDmBleAdvParamUpdateInd( 
        const DM_ULP_ADV_PARAM_UPDATE_IND_T *ind
        ) 
{
    MAKE_CL_MESSAGE(CL_DM_BLE_ADVERTISING_PARAM_UPDATE_IND);
    
    message->adv_interval_min = ind->adv_interval_min;
    message->adv_interval_max = ind->adv_interval_max;

    switch(ind->advertising_type)
    {
        case HCI_ULP_ADVERT_CONNECTABLE_UNDIRECTED:
            message->advertising_type = ble_adv_ind;
            break;
        case HCI_ULP_ADVERT_CONNECTABLE_DIRECTED_HIGH_DUTY:
            message->advertising_type = ble_adv_direct_ind_high_duty;
            break;
        case HCI_ULP_ADVERT_DISCOVERABLE:
            message->advertising_type = ble_adv_scan_ind;
            break;
        case HCI_ULP_ADVERT_NON_CONNECTABLE:
            message->advertising_type = ble_adv_nonconn_ind;
            break;
        case HCI_ULP_ADVERT_CONNECTABLE_DIRECTED_LOW_DUTY:
            message->advertising_type = ble_adv_direct_ind_low_duty;
            break;
        default:
            CL_DEBUG(( 
                "Received unknown advertising type: %d\n",
                ind->advertising_type
                ));
            message->advertising_type = 0xFF;
            break;
    } 

    message->own_address_type = ind->own_address_type;
    message->direct_address_type = ind->direct_address_type;
    BdaddrConvertBluestackToVm(&message->direct_bd_addr, &ind->direct_address);
    message->advertising_channel_map = ind->advertising_channel_map;

    switch (ind->advertising_filter_policy)
    {
        case HCI_ULP_ADV_FP_ALLOW_ANY:
            message->advertising_filter_policy = ble_filter_none;
            break;
        case HCI_ULP_ADV_FP_ALLOW_CONNECTIONS:
            message->advertising_filter_policy = ble_filter_scan_only;
            break;
        case HCI_ULP_ADV_FP_ALLOW_SCANNING:
            message->advertising_filter_policy = ble_filter_connect_only;
            break;
        case HCI_ULP_ADV_FP_ALLOW_WHITELIST:
            message->advertising_filter_policy = ble_filter_both;
            break;    
        default:
            CL_DEBUG(( 
                "Received unknown advertising filter policy: %d\n", 
                ind->advertising_type 
                ));
            message->advertising_filter_policy = 0xFF;
            break;
    }

    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_ADVERTISING_PARAM_UPDATE_IND,
            message);
}
#endif /* DISABLE_BLE */

