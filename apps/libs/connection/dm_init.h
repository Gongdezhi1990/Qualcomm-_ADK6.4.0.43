/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_init.h
    
DESCRIPTION

*/

#ifndef	CONNECTION_DM_INIT_H_
#define	CONNECTION_DM_INIT_H_


/****************************************************************************
NAME
connectionVersionInit

DESCRIPTION
This Function is called on receipt of a DM_AM_REGISTER_CFM and forms the part 
of the DM initiation procedure which sets up the bluetooth version of a dev

RETURNS

*/
void connectionVersionInit(void);

		
/****************************************************************************
NAME
    connectionDmInit

DESCRIPTION
    This Function is called to register the Connection Manager with Bluestack
*/
void connectionDmInit(void);


/****************************************************************************
NAME
    connectionDmInfoInit

DESCRIPTION
    This function is called to complete the DM init. It mainly enables
    informational messages. All the primitives it sends must be called after 
    the DM has initialized (so cannot be part of DM init function) and we
    either don't get a return message from BlueStack or we don't care about it.
*/
void connectionDmInfoInit(void);


/****************************************************************************
NAME
connectionAuthInit

DESCRIPTION
This Function is called to initialise the Trusted Device List (if there is one).

RETURNS

*/
uint16 connectionAuthInit(void);


#endif	/* CONNECTION_DM_INIT_H_ */
