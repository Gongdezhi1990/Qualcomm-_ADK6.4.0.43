/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_init.h
DESCRIPTION
    Header file for Initialization routines. 
*/
#ifndef __IAP2_INIT_H__
#define __IAP2_INIT_H__

#include <connection_no_ble.h>
#include "iap2_private.h"

void iap2HandleInternalCpInitCfm(const IAP2_INTERNAL_CP_INIT_CFM_T *cfm);
void iap2HandleRfcommRegisterCfm(const CL_RFCOMM_REGISTER_CFM_T *cfm);
void iap2HandleClSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm);

#endif /* __IAP2_INIT_H__ */
