/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_gain.h

DESCRIPTION
    Functions required to update the ANC Sidetone gain.
*/

#ifndef ANC_GAIN_H_
#define ANC_GAIN_H_

#include <stdlib.h>

/******************************************************************************
NAME
    ancGainIncrementFineTuneGain

DESCRIPTION
    Increment the fine tune gain associated with the configured ANC microphones.

RETURNS
    Bool indicating if the fine tune gain was successfully updated.
*/
bool ancGainIncrementFineTuneGain(void);


#endif
