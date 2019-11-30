/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_INIT_H_
#define _SINK_INIT_H_

#include <hfp.h>

/*************************************************************************
NAME    
    InitUserFeatures
    
DESCRIPTION
    This function initialises all of the user features - this will result in a
    poweron message if a user event is configured correctly and the device will 
    complete the power on

RETURNS

*/
void InitUserFeatures ( void );

/*************************************************************************
NAME    
    InitEarlyUserFeatures
    
DESCRIPTION
    This function initialises the configureation that is required early 
    on in the start-up sequence. 

RETURNS

*/
void InitEarlyUserFeatures ( void );

/****************************************************************************
NAME	
	SetupPowerTable

DESCRIPTION
	Attempts to obtain a low power table from the Ps Key store.  If no table 
	(or an incomplete one) is found	in Ps Keys then the default is used.
	
RETURNS
	void
*/
void SetupPowerTable( void );


/****************************************************************************
NAME    
    sinkHfpInit
    
DESCRIPTION
    Initialise the HFP library
	
RETURNS
    void
*/
void sinkHfpInit( void );

void sinkInitConfigureDeviceClass(void);

/*!
    @brief return the device class configuration from PS keys or return a
    default configuration

    @return device class bitmap
*/
uint32 sinkInitGetDeviceClassConfig(void);

/****************************************************************************
NAME    
    sinkAppInit
    
DESCRIPTION
    Initialization specific to Sink App 

RETURNS
    void
*/
void sinkAppInit(const HFP_INIT_CFM_T *cfm);

#endif /* _SINK_INIT_H_ */
