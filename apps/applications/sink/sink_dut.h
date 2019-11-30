/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_dut.h
    
DESCRIPTION
	Place the device into Device Under Test (DUT) mode
    
*/

#ifndef _SINK_DUT_H_
#define _SINK_DUT_H_


typedef enum
{
    dut_test_invalid,    
    dut_test_audio,
    dut_test_keys,
    dut_test_service,
    dut_test_tx,
    dut_test_dut
} dut_test_mode;

/****************************************************************************
DESCRIPTION
  	This function is called to place the device into DUT mode
*/
void enterDutMode(void);

/****************************************************************************
DESCRIPTION
  	This function is called to place the device into TX continuous test mode
*/

void enterTxContinuousTestMode ( void ) ;



/****************************************************************************
DESCRIPTION
    Gets the currently active DUT mode
*/
dut_test_mode getDUTMode(void);
/*************************************************************************
DESCRIPTION
    Initialise DUT mode
*/
void dutInit(void);

void dutDisconnect(void);

#endif /* _SINK_DUT_H_ */
