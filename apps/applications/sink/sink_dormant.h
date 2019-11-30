/*
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

*/
/** 
\file 
\ingroup sink_app
\brief Support for Dormant Mode. 

*/

#ifndef _SINK_DORMANT_H_
#define _SINK_DORMANT_H_

/****************************************************************************
NAME
    sinkDormantIsWakeUpFromNfcConfigured

DESCRIPTION
    Reads config to determine if the device can be woken from dormant with NFC.

RETURNS
    bool
*/

#ifdef ENABLE_DORMANT_SUPPORT
bool sinkDormantIsWakeUpFromNfcConfigured(void);
#else
#define sinkDormantIsWakeUpFromNfcConfigured() FALSE
#endif

/****************************************************************************
NAME
    sinkDormantEnable

DESCRIPTION
    Go into dormant mode.

    SYS_CTRL and VCHG will always wake up the device from Dormant and Off.

    Additional wake up sources for Dormant such as PIO, timer or NFC can
    also be configured.

RETURNS
    void
*/


#ifdef ENABLE_DORMANT_SUPPORT
void sinkDormantEnable(bool periodic_wakeup_required, bool match_power_off_wakeup_sources);
#else
#define sinkDormantEnable(periodic_wakeup_required, match_power_off_wakeup_sources) ((void)(0))
#endif

/****************************************************************************
NAME
    sinkDormantIsSupported

DESCRIPTION
    API to check whether the dormant mode is enabled or not
    
****************************************************************************/
#ifdef ENABLE_DORMANT_SUPPORT
#define sinkDormantIsSupported() TRUE
#else
#define sinkDormantIsSupported() FALSE
#endif

#endif /* _SINK_DORMANT_H_ */
