/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    handles the csr to csr features 
*/

#ifndef _CSR2CSRFEATURES_

void csr2csrHandleTxtInd(void);
void csr2csrHandleSmsInd(void);   
void csr2csrHandleSmsCfm(void);
void csr2csrHandleAgBatteryRequestInd(void);
void csr2csrHandleAgBatteryRequestRes(uint8 idx);
    
#endif
