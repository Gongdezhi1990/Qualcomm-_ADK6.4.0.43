/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Implementation of application testing functions for LE functionality.
*/


#include "av_headset.h"
#include "av_headset_log.h"

#include "av_headset_test_le.h"

void appTestSetAdvertEnable(bool enable)
{
    ConnectionDmBleSetAdvertiseEnable(enable);
}


void appTestSetAdvertisingParamsReq(void)
{
    ble_adv_params_t parm;
    parm.undirect_adv.adv_interval_min = 0x800;
    parm.undirect_adv.adv_interval_max = 0x2000;
    parm.undirect_adv.filter_policy    = ble_filter_none;

    ConnectionDmBleSetAdvertisingParamsReq(ble_adv_nonconn_ind,FALSE,BLE_ADV_CHANNEL_ALL,&parm);
}

