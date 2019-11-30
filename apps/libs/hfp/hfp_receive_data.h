/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_receive_data.h
    
DESCRIPTION
    
*/

#ifndef HFP_RECEIVE_DATA_H_
#define HFP_RECEIVE_DATA_H_


/****************************************************************************
NAME    
    hfpHandleReceivedData

DESCRIPTION
    Called when we get an indication from the firmware that there's more data 
    received and waiting in the RFCOMM buffer. Parse it.

RETURNS
    void
*/
void hfpHandleReceivedData(Source source);


#endif /* HFP_RECEIVE_DATA_H_ */
