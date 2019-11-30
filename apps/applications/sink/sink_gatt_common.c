/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_common.c

DESCRIPTION
    General routines to handle the GATT common for client and server.
*/

#include "sink_ble.h"
#include "sink_gatt_common.h"
#include "sink_gatt_server.h"
#include "sink_gatt_client.h"
#include "sink_gatt_hid_remote_control.h"
#include "sink_gatt_client_hid.h"
#include "sink_gatt_client_hrs.h"
#include "sink_ba.h"
#include "sink_ba_ble_gap.h"
#include "sink_debug.h"

#include <gatt_manager.h>
#include <vm.h>

#ifdef GATT_ENABLED

#ifdef DEBUG_GATT_COMMON
#define GATT_COMMON_DEBUG(x) DEBUG(x)
const char * const gatt_role[3] = {
    "UNKNOWN",
    "CENTRAL",
    "PERIPHERAL",
};
#else
#define GATT_COMMON_DEBUG(x) 
#endif

/* This macro defines the max connection is any role */
#define GATT_PER_ROLE_MAX_CONN  1
#define GATT_GET_CONN_PER_ROLE(role) ((role == ble_gap_role_central) ? ((GATT_CLIENT_MAX_CONN) ? GATT_PER_ROLE_MAX_CONN : 0) :\
                                                               ((GATT_SERVER_MAX_CONN) ? GATT_PER_ROLE_MAX_CONN : 0))
#define GATT_GET_CONN_CONFIG(role) ((role == ble_gap_role_central) ? GATT_CLIENT_MAX_CONN : GATT_SERVER_MAX_CONN)

static uint16 gattCommonGetConnConfig(ble_gap_role_t role)
{
    /* if max client conn + max server connection is greater than 2, then retrict the max in any role to 1 or 0
     * based on what was set for the role */
    if((GATT_CLIENT_MAX_CONN + GATT_SERVER_MAX_CONN) <= MAX_BLE_CONNECTIONS)
    {
        GATT_COMMON_DEBUG(("The Max Configuration for [%s] role is %d\n", gatt_role[role], GATT_GET_CONN_CONFIG(role)));
        return GATT_GET_CONN_CONFIG(role);
    }
    GATT_COMMON_DEBUG(("The Max Configuration for [%s] role is %d\n", gatt_role[role], GATT_GET_CONN_PER_ROLE(role)));
    return GATT_GET_CONN_PER_ROLE(role);
}

bool gattCommonIsMaxConnReached(ble_gap_role_t gap_role)
{
    uint16 max_conn = 0;
    uint16 central_conn = 0;
    uint16 index =0;

    /* If we are in BA receiver mode & association is in progress then
    always allow incoming connection */
    if((BA_RECEIVER_MODE_ACTIVE) && gapBaGetAssociationInProgress())
    {
        return FALSE;
    }

    if((max_conn = gattCommonGetConnConfig(gap_role)) != 0)
    {
        /* Find the number of device connected */
        for(index = 0; index < MAX_BLE_CONNECTIONS;index++)
        {
            if(GATT_CLIENT.connection.role == ble_gap_role_central)
                central_conn++;
        }
        /* for peripheral, the connection.role shall not be updated, till addClient is called
         * And this is called only after Server is added */
         if(gap_role == ble_gap_role_peripheral)
         {
            return ((BLE_DATA.number_connections - central_conn) >= max_conn);
         }
        /* else return based on central connection */
        return (central_conn >= max_conn);
    }
    return TRUE;
}

uint16 gattCommonGetNumOfConn(ble_gap_role_t gap_role)
{
    uint16 central_conn = 0;
    uint16 index =0;

    /* Find the number of device connected */
    for(index = 0; index < MAX_BLE_CONNECTIONS;index++)
    {
        if(GATT_CLIENT.connection.role == ble_gap_role_central)
            central_conn++;
    }
    /* for peripheral, the connection.role shall not be updated, till addClient is called
     * And this is called only after Server is added */
     if(gap_role == ble_gap_role_peripheral)
     {
        return (BLE_DATA.number_connections - central_conn);
     }
     return (central_conn);
}

/***************************************************************************
NAME    
    gattCommonHasMaxConnections
    
DESCRIPTION
    Utility function called to check if maximum gatt connection is reached.
    
PARAMETERS
    None
    
NOTE
    TRUE if reached else FALSE.
*/
bool gattCommonHasMaxConnections(void)
{
    return (BLE_DATA.number_connections >= MAX_BLE_CONNECTIONS);
}

/***************************************************************************
NAME    
    gattCommonAddConnections
    
DESCRIPTION
    Function to add connection.
    
PARAMETERS
    None
    
NOTE
    TRUE if reached else FALSE.
*/
bool gattCommonAddConnections(uint16 cid, ble_gap_role_t gap_role)
{
    uint8 conn_index;
    if(!gattCommonHasMaxConnections() && !gattCommonIsMaxConnReached(gap_role))
    {
         for(conn_index=0;conn_index < MAX_BLE_CONNECTIONS;conn_index++)
        {
            /* if either there is an empty slot available */
            if(GATT[conn_index].cid == 0) 
            {
                GATT[conn_index].cid = cid;
                BLE_DATA.number_connections++;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/***************************************************************************
NAME    
    gattCommonRemoveConnections
    
DESCRIPTION
    Function to remove connection.
    
PARAMETERS
    cid
    
NOTE
    TRUE if reached else FALSE.
*/
bool gattCommonRemoveConnections(uint16 cid)
{
    uint8 conn_index;

    for(conn_index=0;conn_index < MAX_BLE_CONNECTIONS;conn_index++)
    {
        /* Check if the connection exists */
        if(GATT[conn_index].cid == cid) 
        {
            GATT[conn_index].cid = 0;
            GATT[conn_index].is_bonding = FALSE;
            BLE_DATA.number_connections--;
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************/
uint16 gattCommonConnectionFindByCid(uint16 cid)
{
    uint16 conn_index = GATT_INVALID_INDEX;
    uint16 index;
    for(index=0;index < MAX_BLE_CONNECTIONS ;index++)
    {
        if(GATT[index].cid == cid)
        {
            conn_index = index;
            break;
        }
    }
    return conn_index;
}



/****************************************************************************/
uint16 gattCommonFindCidBondingDevice(void)
{
    uint16 conn_index = 0;
    for(conn_index=0;conn_index < MAX_BLE_CONNECTIONS ;conn_index++)
    {
        if(GATT[conn_index].is_bonding == TRUE)
        {
            return GATT[conn_index].cid;
        }
    }
    return 0;
}

/****************************************************************************
NAME    
    gattCommonSetDeviceBonding
    
DESCRIPTION
    Sets the bonding flag of the device as expected.
    
PARAMETERS
    none
    
RETURNS    
    returns the connection ID
*/
bool gattCommonSetDeviceBonding(uint16 cid, bool req_set)
{
    uint16 conn_index = 0;
    for(conn_index=0;conn_index < MAX_BLE_CONNECTIONS ;conn_index++)
    {
        if(GATT[conn_index].cid == cid)
        {
            GATT[conn_index].is_bonding =  req_set;
            return TRUE;
        }
    }
    return FALSE;
}

/****************************************************************************/
bool gattCommonConnectionRemove(uint16 cid)
{
    uint16 conn_index = 0;
    bool ret = FALSE;
    for(conn_index=0;conn_index < MAX_BLE_CONNECTIONS ;conn_index++)
    {
        if(GATT[conn_index].cid == cid)
        {
            GATT[conn_index].cid = 0;
            ret = TRUE;
            break;
        }
    }
    return ret;
}


/****************************************************************************/
bool gattCommonDisconnectAll(bool central)
{
    /* Call into gatt_manager to remove connections */
    uint16 index = 0;
    bool ret = FALSE;
    bool allow_disc;
    for(index=0;index < MAX_BLE_CONNECTIONS;index++)
    {
        allow_disc = FALSE;
        if(GATT[index].cid)
        {
            if((central && GATT_CLIENT.connection.role == ble_gap_role_central) ||
               (!central && GATT_CLIENT.connection.role != ble_gap_role_central))
                allow_disc = TRUE;

            if(allow_disc)
            {
                GattManagerDisconnectRequest(GATT[index].cid);
                ret = TRUE;
            }
        }
    }
    return ret;
}


/***************************************************************************/
bool gattCommonIsFullyDisconnected(void)
{
    uint16 conn_index = 0;

    for(conn_index=0;conn_index < MAX_BLE_CONNECTIONS ;conn_index++)
    {
        if(GATT[conn_index].cid != 0 )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/***************************************************************************/
uint16 gattCommonGetNumberOfConn(void)
{
    return BLE_DATA.number_connections;
}

/***************************************************************************/
uint16 gattCommonGetCentralConnCid(void)
{
    uint16 cid = GATT_CLIENT_INVALID_CID;
    uint16 index = 0;

    /* Find the number of device connected */
    for(index = 0; index < MAX_BLE_CONNECTIONS;index++)
    {
        if(GATT_CLIENT.connection.role == ble_gap_role_central)
        {
            /*Found the central connection, return it's cid */
            cid = GATT[index].cid;
            break;
        }
    }
    return (cid);
}

/***************************************************************************/
uint16 gattCommonGetCidForAddr(bdaddr *bd_addr)
{
    uint8 index;
    tp_bdaddr tp_bd_addr;
    tp_bdaddr per_bd_addr;

    if(bd_addr)
    {
        for(index=0;index < MAX_BLE_CONNECTIONS;index++)
        {
            if(VmGetBdAddrtFromCid(GATT[index].cid, &tp_bd_addr))
            {
                per_bd_addr = tp_bd_addr;
                if(tp_bd_addr.taddr.type == TYPED_BDADDR_RANDOM) 
                     VmGetPublicAddress(&tp_bd_addr, &per_bd_addr);
                
                /* Found a valid bd_address, check if it matches the one sent */
                if(BdaddrIsSame(bd_addr, &per_bd_addr.taddr.addr))
                    return GATT[index].cid;
            }
        }
    }
    return GATT_INVALID_INDEX;
}

#endif /* GATT_ENABLED */


