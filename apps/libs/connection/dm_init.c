/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_init.c        

DESCRIPTION		
    This file contains the functions to initialise the Device Manager 
    component of the connection library

NOTES

*/

/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "dm_init.h"
#include "connection_tdl.h"

#include <vm.h>
#include <stream.h>

/****************************************************************************
NAME
setDefaultLinkPolicy

DESCRIPTION
This Function is called to set the default link policy.  This saves having
to set the link policy for every ACL connection

RETURNS

*/
static void setDefaultLinkPolicy(uint16_t in, uint16 out)
{
	MAKE_PRIM_T(DM_SET_DEFAULT_LINK_POLICY_REQ);
	prim->link_policy_settings_in = in;
	prim->link_policy_settings_out = out;
	VmSendDmPrim(prim);
}

/****************************************************************************
NAME
connectionVersionInit

DESCRIPTION
This Function is called on receipt of a DM_AM_REGISTER_CFM and forms the part 
of the DM initiation procedure which sets up the bluetooth version of a dev

RETURNS

*/
void connectionVersionInit (void)
{
	/* Check the HCI version */
	ConnectionReadLocalVersion(connectionGetCmTask());
}

/****************************************************************************
NAME
connectionDmInit

DESCRIPTION
This Function is called to register the Connection Manager with Bluestack.  
The Bluestack message DM_AM_REGISTER_REQ is sent to the Bluestack Device
Manager (DM)

RETURNS

*/
void connectionDmInit(void)
{
     MAKE_PRIM_T(DM_AM_REGISTER_REQ);
     prim->phandle = 0;
     VmSendDmPrim(prim);
	 
	 /* Configure all SCOs to be streams */
	 StreamConfigure(VM_STREAM_SCO_ENABLED, 1);
}


/*****************************************************************************/
void connectionDmInfoInit(void)
{
    if (connectionGetBtVersion() < bluetooth2_1)
    {
        /* Set default link policy */
        setDefaultLinkPolicy(ENABLE_MS_SWITCH, ENABLE_MS_SWITCH);
    }
    else
    {
        /* Set default link policy */
        setDefaultLinkPolicy(ENABLE_MS_SWITCH | ENABLE_SNIFF, ENABLE_MS_SWITCH | ENABLE_SNIFF);
    }
}
    
/****************************************************************************
NAME
connectionAuthInit

DESCRIPTION
This Function is called to initialise the Trusted Device List (if there is one).

RETURNS

*/
uint16 connectionAuthInit(void)
{
    uint16 noDevices = 0;

    /* Initialise the Trusted Device List.  This involves adding all devices
       in the Trusted Device List to Bluestack Security Managers' database */
    noDevices = connectionInitTrustedDeviceList();

    if(noDevices == 0)
    {
        /* There are no valid records in the Trusted Device Index therefore
           no devices were added to the Bluestack Security Managers' 
           database, initialisation is complete */
        connectionSendInternalInitCfm(connectionInitComplete);
    }

    return noDevices;
}
