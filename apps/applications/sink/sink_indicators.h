/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_INDICATORS_H_
#define _SINK_INDICATORS_H_

#include <hfp.h>

/****************************************************************************
NAME    
    indicatorsHandleServiceInd
    
DESCRIPTION
    Interprets the service Indicator messages and sends the appropriate message 

RETURNS
    void
*/
void indicatorsHandleServiceInd ( const HFP_SERVICE_IND_T *pInd );


#endif /* _SINK_INDICATORS_H_ */

