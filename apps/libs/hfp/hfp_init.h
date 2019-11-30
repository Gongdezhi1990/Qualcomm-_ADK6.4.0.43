/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_init.h
    
DESCRIPTION
    
*/

#ifndef HFP_INIT_H_
#define HFP_INIT_H_


/****************************************************************************
NAME    
    hfpInitRfcommRegisterCfm

DESCRIPTION
    Rfcomm channel has been allocated.

RETURNS
    void
*/
void hfpInitRfcommRegisterCfm(const CL_RFCOMM_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME    
    hfpInitSdpRegisterComplete

DESCRIPTION
    SDP registration has completed

RETURNS
    void
*/
void hfpInitSdpRegisterComplete(hfp_lib_status status);


/****************************************************************************
NAME    
    hfpInitSdpRegisterCfm

DESCRIPTION
    SDP register request from Connection library has returned

RETURNS
    void
*/
void hfpInitSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm);


#endif /* HFP_INIT_H_ */
