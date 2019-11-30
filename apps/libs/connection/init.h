/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    init.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_INIT_H_
#define    CONNECTION_INIT_H_


/* Connection Library components. Used to control initialisation */
typedef enum
{
    connectionInit          = 0,
    connectionInitDm,
    connectionInitRfc,
    connectionInitL2cap,
    connectionInitUdp,
    connectionInitTcp,
    connectionInitSdp,
	connectionInitVer,
    connectionInitSm,
    connectionInitComplete
}connectionInitState;

/* Initailisation timeout period */
#ifdef CONNECTION_DEBUG_LIB
#define INIT_TIMEOUT    (15000)
#else
#define INIT_TIMEOUT    (10000)
#endif /* CONNECTION_DEBUG_LIB */

/****************************************************************************

NAME    
    connectionHandleInternalInit    

DESCRIPTION
    This function is called to control the initialsation process.  To avoid race
    conditions at initialisation, the process is serialised.

RETURNS
    void
*/
void connectionHandleInternalInit(connectionInitState state);


/****************************************************************************

NAME    
    connectionSendInternalInitCfm    

DESCRIPTION
    This function is callled to send a CL_INTERNAL_INIT_CFM message to the 
    Connection Library task

RETURNS
    void
*/
void connectionSendInternalInitCfm(connectionInitState state);


/****************************************************************************

NAME    
    connectionSendInitCfm

DESCRIPTION
    This function is called from the main Connection Library task handler to 
    indicate to the Client application the result of the request to initialise
    the Connection Library.  The application task is passed in as the first
    parameter

RETURNS
    void
*/
void connectionSendInitCfm(Task task, connection_lib_status state, cl_dm_bt_version version);


#endif    /* CONNECTION_INIT_H_ */
