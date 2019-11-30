/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_send_data.h
    
DESCRIPTION
    
*/

#ifndef HFP_SEND_DATA_H_
#define HFP_SEND_DATA_H_


/****************************************************************************
NAME    
    hfpSendAtCmd

DESCRIPTION
    Send an AT command by putting it into the RFCOMM buffer.

RETURNS
    void
*/
void hfpSendAtCmd(hfp_link_data* link, uint16 length, const char *at_cmd, hfp_at_cmd pending_cmd);


/****************************************************************************
NAME    
    hfpSendNextAtCmd

DESCRIPTION
    Attempt to send the next AT cmd (if any) pending in the RFCOMM buffer.

RETURNS
    void
*/
void hfpSendNextAtCmd(hfp_link_data* link, uint16 offset, uint8 *data_out);



#endif /* HFP_SEND_DATA_H_ */
