/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    handles the csr to csr features 
*/

#include "sink_debug.h"
#include "sink_events.h"
#include "sink_csr_features.h"
#include "sink_main_task.h"

#include <hfp.h>

#ifdef DEBUG_CSR2CSR
    #define CSR2CSR_DEBUG(x) DEBUG(x)
#else
    #define CSR2CSR_DEBUG(x)
#endif     

void csr2csrHandleTxtInd (void ) 
{
    CSR2CSR_DEBUG(("CSR2CSR TXT IND\n")) ;
}

void csr2csrHandleSmsInd (void ) 
{
    CSR2CSR_DEBUG(("CSR2CSR SMS IND\n")) ;
}

void csr2csrHandleSmsCfm(void ) 
{
    CSR2CSR_DEBUG(("CSR2CSR SMS CFM\n")) ;
}

void csr2csrHandleAgBatteryRequestInd ( void ) 
{
    CSR2CSR_DEBUG(("CSR2CSR BATTERY REQUEST IND\n")) ;
	MessageSend (&theSink.task , EventUsrBatteryLevelRequest , 0 );
}

void csr2csrHandleAgBatteryRequestRes(uint8 idx)
{
    const unsigned char csr2csr_battery_level_indications[4] = {2,4,7,9};
    
    CSR2CSR_DEBUG(("CSR2CSR BATTERY REQUEST RES %d", idx)) ;
    if(idx < sizeof(csr2csr_battery_level_indications))
    {
        uint16 batt_level = csr2csr_battery_level_indications[idx];
        CSR2CSR_DEBUG((" [%d]", batt_level)) ;
        /* Attempt to send indication to both AGs (HFP will block if unsupported) */
        HfpCsrFeaturesBatteryLevelRequest(hfp_primary_link, batt_level);
        HfpCsrFeaturesBatteryLevelRequest(hfp_secondary_link, batt_level);
    }
    CSR2CSR_DEBUG(("\n")) ;
}
