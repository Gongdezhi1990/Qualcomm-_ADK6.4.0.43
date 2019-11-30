/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.
 

 DESCRIPTION:
   Helper functions for sending AT commands to the remote device. There is
   also a way to send raw data with no formatting. The raw method is useful
   for user defined protocols and for testing badly formatted AT commands.
*/

#ifndef AGHFP_SEND_DATA_H_
#define AGHFP_SEND_DATA_H_


#include <aghfp.h>


/* Send an AT command by putting it into the RFCOMM buffer. */
void aghfpSendAtCmd(AGHFP *aghfp, const char *at_cmd);

void aghfpAtCmdBegin(AGHFP *aghfp);
void aghfpAtCmdString(AGHFP *aghfp, const char *string);
void aghfpAtCmdData(AGHFP *aghfp, const uint8 *data, uint16 len);
void aghfpAtCmdEnd(AGHFP *aghfp);

/* Send raw data to the remote end, do not add any formatting to it. */
void aghfpSendRawData(AGHFP *aghfp, uint16 length, const uint8 *data);


#endif /* AGHFP_SEND_DATA_H_ */
