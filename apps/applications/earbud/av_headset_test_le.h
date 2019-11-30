/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Interface for application testing functions for Bluetooth Low Energy
            functionality
*/

#ifndef AV_HEADSET_TEST_LE_H
#define AV_HEADSET_TEST_LE_H

#include "av_headset.h"


/*! Prototype test function to enable/disable advertising

    \note This is just a wrapper onto CL and enables any pre-set advertising
    messages.

    \todo Replace with more relevant test (if we go down this route)

    \param enable   Enable/Disable advertising
*/
void appTestSetAdvertEnable(bool enable);


/*! Prototype test function to set hard coded advertising params 

    \note This is just a wrapper onto CL

    \todo Replace with more relevant test (if we go down this route)
*/
void appTestSetAdvertisingParamsReq(void);


#endif // AV_HEADSET_TEST_LE_H
