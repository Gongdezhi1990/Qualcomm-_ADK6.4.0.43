/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_SCAN_H_
#define _SINK_SCAN_H_

#include <connection.h>

/****************************************************************************
NAME
    sinkScanInit
    
DESCRIPTION
    Initialise sink_scan module
*/
void sinkScanInit(void);

/****************************************************************************
NAME    
    sinkWriteEirData
    
DESCRIPTION
    Writes the local name and device UUIDs into device EIR data, local name 
    is shortened to fit into a DH1 packet if necessary

RETURNS
    void
*/
void sinkWriteEirData( const CL_DM_LOCAL_NAME_COMPLETE_T *message );

/****************************************************************************
NAME    
    sinkEnableConnectable
    
DESCRIPTION
    Make the device connectable 

RETURNS
    void
*/
void sinkEnableConnectable( void );

/****************************************************************************
NAME    
    sinkDisableConnectable
    
DESCRIPTION
    Take device out of connectable mode.

RETURNS
    void
*/
void sinkDisableConnectable( void );

/****************************************************************************
NAME
    sinkIsConnectable
    
DESCRIPTION
    Check whether connectable scanning has been enabled

RETURNS
    TRUE if connectable, otherwise FALSE
*/
bool sinkIsConnectable(void);

/****************************************************************************
NAME    
    sinkEnableDiscoverable
    
DESCRIPTION
    Make the device discoverable. 
*/
void sinkEnableDiscoverable( void );


/****************************************************************************
NAME    
    sinkDisableDiscoverable
    
DESCRIPTION
    Make the device non-discoverable. 
*/
void sinkDisableDiscoverable( void );


/****************************************************************************
NAME    
    sinkEnableMultipointConnectable
    
DESCRIPTION
    when in multi point mode check to see if device can be made connectable,
    this will be when only one AG is currently connected. this function will
    be called upon certain button presses which will reset the 60 second timer
    and allow a second AG to connect should the device have become non discoverable
*/
void sinkEnableMultipointConnectable( void );

/****************************************************************************
NAME
    sinkEnableGattConnectable
    
DESCRIPTION
    Enable page scanning to facilitate connection of GATT over BR/EDR. Calling
    sinkDisableConnectable will not disable page scan if page scanning for GATT
    is also disabled.
*/
void sinkEnableGattConnectable(void);

/****************************************************************************
NAME
    sinkDisableGattConnectable
    
DESCRIPTION
    Disable page scanning to facilitate connection of GATT over BR/EDR. Calling
    sinkDisableConnectable will not disable page scan if page scanning for GATT
    is also disabled.
*/
void sinkDisableGattConnectable(void);

/****************************************************************************
NAME
    sinkDisableAllConnectable
    
DESCRIPTION
    Disable page scanning for any reason
*/
void sinkDisableAllConnectable(void);

#endif /* _SINK_SCAN_H_ */
