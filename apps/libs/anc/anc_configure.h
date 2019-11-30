/*******************************************************************************
Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_configure.h

DESCRIPTION
    Functions required to configure the ANC Sinks/Sources.
*/

#ifndef ANC_CONFIGURE_H_
#define ANC_CONFIGURE_H_

#include <csrtypes.h>

/******************************************************************************
NAME
    ancConfigureMicGains

DESCRIPTION
    Configure the ANC microphone gains.
*/
void ancConfigureFinetuneGains(void);

/******************************************************************************
NAME
    ancConfigureSidetoneGains

DESCRIPTION
    Configure the sidetone gains.
*/
void ancConfigureSidetoneGains(void);

/******************************************************************************
NAME
    ancConfigure

DESCRIPTION
    Configure the ANC hardware
*/
bool ancConfigure(bool enable);

/******************************************************************************
NAME
    ancConfigureAfterModeChange

DESCRIPTION
    (Re)Configure following an ANC mode change
*/
bool ancConfigureAfterModeChange(void);
        
#endif
