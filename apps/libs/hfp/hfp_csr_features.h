/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_csr_features.h
    
DESCRIPTION
    
*/

#ifndef HFP_CSR_FEATURES_H_
#define HFP_CSR_FEATURES_H_

#include "hfp_private.h"

/*
    Send AT+CSRSF to the AG indicating our supported features
*/
void hfpCsrFeaturesReq(hfp_link_data* link);


/*
    Have received response to AT+CSRSF
*/
void hfpCsrFeaturesHandleCsrSfAtAck(hfp_link_data* link, hfp_lib_status status);


/*
    Handle CSR Power Level Request
*/
void hfpCsrFeaturesHandleBatteryLevelReq(const HFP_INTERNAL_CSR_BATTERY_LEVEL_REQ_T *req);


/*
    Handle CSR Power Source Request
*/
void hfpCsrFeaturesHandlePowerSourceReq(const HFP_INTERNAL_CSR_POWER_SOURCE_REQ_T *req);


/*
    Handle internal Get SMS request.
*/
bool hfpHandleCsrGetSmsReq(const HFP_INTERNAL_CSR_GET_SMS_REQ_T *req);


/*
    Handle Get SMS ack.
*/
void hfpCsrFeaturesHandleGetSmsAtAck(hfp_link_data* link, hfp_lib_status status);

#endif /* HFP_CSR_FEATURES_H_ */

