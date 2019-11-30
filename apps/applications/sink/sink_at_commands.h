/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app

*/

#ifndef _SINK_AT_H_
#define _SINK_AT_H_

#include "sink_events.h"
#include <hfp.h>



/*these values have special meanings in the AT commands and will be replaced in place*/

typedef enum at_command_variables { 
    at_cmd_battery_0_to_5 = 0x81,
    at_cmd_battery_0_to_9_scaling_1 = 0x82,
    at_cmd_battery_0_to_9_scaling_2 = 0x83,
    at_cmd_charger_docked = 0x84
}at_cmd_vars_t ;


/****************************************************************************
DESCRIPTION
    handles an unrecognised AT command from the HFP library
        
    Attempts to match an AT command to those configured. If configured, a response
    will be sent.
    
    If not configured, a response will not be sent.
    
    If the +MICTEST is configured, the mic test switching will take place
    
    
*/
void sinkHandleUnrecognisedATCmd( HFP_UNRECOGNISED_AT_CMD_IND_T *ind ) ;


/****************************************************************************
DESCRIPTION
    Sends a given AT command from the configuration 
        
    works through the configuration looking for '\0' signifying end of string, 
    moves to next string in list until string == '\0'
    
    If a string matching the id passed in, then the command is sent to the AG.
    If not, no command is sent
    
*/
void sinkSendATCmd( uint16 at_id) ;


/****************************************************************************
DESCRIPTION
    Checks for a user event -> AT command match configured in CONFIG_USR_3
    If configued, AT command sent to valid HFP connections using 
    sinkSendATCommand
    
    Up to MAX_AT_COMMANDS_TO_SEND AT commands can be stored in the PSKEY 
    for Sending to the  AG
    
    An EventSysGasGauge0 or an EventSysChargerGasGauge0 configured will result in the 
    same message being sent for any of the gas gauge events.
    
*/
void ATCommandPlayEvent ( sinkEvents_t id ) ;

#endif /* _SINK_AT_H_ */
