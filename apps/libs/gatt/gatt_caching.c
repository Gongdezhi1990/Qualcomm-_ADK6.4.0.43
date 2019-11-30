/****************************************************************************
Copyright (c) 2018 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_caching.c        

DESCRIPTION
    GATT functions related to GATT Caching.
*/

/***************************************************************************
    Header Files
*/
#include "gatt.h"
#include "gatt_private.h"
#include "gatt_caching.h"
#include "gatt_server_uuids.h"


#include <bdaddr.h>
#include <vm.h>
#include <stdlib.h>

/***************************************************************************
    Macros
*/
#define SIZE_GATT_TD ((sizeof(gatt_td_data_t)-sizeof(uint16)))

/*************************************************************************
NAME    
    GattHandleCsfWrite
    
DESCRIPTION
    This function handles a write by a client to the Client Supported Features
    characteristic.
    
RETURNS
    True if the value is valid, False otherwise
*/
bool GattHandleCsfWrite(uint8 feat, uint16 cid)
{
    gatt_td_data_t *gd;
    bool trusted = FALSE;
    bool successful = FALSE;
    tp_bdaddr *client = PanicUnlessMalloc(sizeof(tp_bdaddr));
    cid_map_t *conn = gattFindConnOrPanic(cid); /* never NULL */
    
    VmGetBdAddrtFromCid(cid, client);
    
    /* If client is a trusted device, any changes need to be stored in PS.
     * Load the struct to make any necessary changes.
     */
    gd = (gatt_td_data_t *) PanicUnlessMalloc(SIZE_GATT_TD);
    
    if (ConnectionGetGattAttribute(&client->taddr, SIZE_GATT_TD, (uint8 *)gd))
    {
        trusted = TRUE;
    }
    
    /* Compare to gatt_client_supported_features enum. */
    if (feat & csf_robust_caching_enable)
    {
        /* Notify Bluestack that client supports robust caching. */
        MAKE_ATT_PRIM(ATT_ADD_ROBUST_CACHING_REQ)
    
        prim->change_aware = ATT_CHANGE_AWARE_CLIENT;
        
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, client);

        VmSendAttPrim(prim);
        
        /* Store in non-persistent memory for non-trusted devices. */
        conn->robust_caching_enabled = 1;
        
        if (trusted) 
        {
            gd->content.robust_caching = 1;
            
            /* Store trusted device data back into PS. */
            ConnectionPutGattAttribute(
                &client->taddr, 
                SIZE_GATT_TD, 
                (uint8 *)gd);
        }
        
        successful = TRUE;
    }
    
    
    free(gd);
    free(client);

    return successful;
}

/****************************************************************************
NAME    
    GattHandleCsfRead
    
DESCRIPTION
    This function handles a read request from a client to the Client Supported
    Features characteristic.
    
RETURNS

*/
void GattHandleCsfRead(uint16 cid, uint8 *val)
{
    cid_map_t *conn = gattFindConnOrPanic(cid); /* never NULL */
    
    /* This will need to be bitwise OR'ed if more CSF are added in the
     * future, or it will need its own bitfield.
     */
    val[0] = conn->robust_caching_enabled;
}

/****************************************************************************
NAME
    gattNewConnectionGattCaching

DESCRIPTION
    This function is called when a new GATT connection is completed, to check whether the
    device is a trusted client, and if so whether it is change aware and to update any
    relevant conn-related data.

RETURNS

*/
void gattNewConnectionGattCaching(gattState *theGatt, typed_bdaddr taddr, cid_map_t *conn)
{
    uint8 service_changed_vals[4];
    gatt_td_data_t *gd;

    gd = (gatt_td_data_t *) PanicUnlessMalloc(SIZE_GATT_TD);
    
    if (ConnectionGetGattAttribute(&taddr, SIZE_GATT_TD, (uint8 *)gd))
    {
        /* Load GATT Caching information into the conn struct. */
        conn->robust_caching_enabled = gd->content.robust_caching;
        conn->change_aware = gd->content.change_aware;
        conn->serv_chg_cccd_enable = gd->content.indications_enabled;
        
        /* Only send the indication if the client is change unaware and has
         * enabled indications.
         */ 
        if (!(conn->change_aware) && (conn->serv_chg_cccd_enable))
        {
            /* DB hash has changed since this trusted client disconnected,
             * send service changed indication.
             */
            service_changed_vals[0] = 0x00;
            service_changed_vals[1] = 0x01;
            service_changed_vals[2] = 0xff;
            service_changed_vals[3] = 0xff;
      
            GattIndicationRequest(conn->task, conn->cid, theGatt->service_changed_local_handle, 4*sizeof(uint16), service_changed_vals);
        }
    }
    
    free(gd);
}


/****************************************************************************
NAME
    gattHandleAttChangeAwareInd

DESCRIPTION
    Handle the ATT Change Aware Indication message. A client has become change
    aware, and as such the GATT attributes stored in PS must be updated, if the
    client is a trusted client.

RETURNS

*/
void gattHandleAttChangeAwareInd(const ATT_CHANGE_AWARE_IND_T *ind)
{
    gatt_td_data_t *gd;
    tp_bdaddr *client;
    
    if (ind->flags == ATT_BONDED_CLIENT)
    {
        client = (tp_bdaddr *) PanicUnlessMalloc(sizeof(tp_bdaddr));
        BdaddrConvertTpBluestackToVm(client, &ind->tp_addrt);
        
        gd = (gatt_td_data_t *) PanicUnlessMalloc(SIZE_GATT_TD);
        
        if (ConnectionGetGattAttribute(&client->taddr, SIZE_GATT_TD, (uint8 *)gd))
        {
            gd->content.change_aware = 1;
            
            ConnectionPutGattAttribute(&client->taddr, SIZE_GATT_TD, (uint8 *)gd);
        }
        
        free(client);
        free(gd);
    }
}

/****************************************************************************
NAME
    GattHandleServChangedIndRead

DESCRIPTION
    Handles the request to read the Service Changed Client Characteristic Configuration
    Descriptor.

RETURNS

*/
uint8 GattHandleServChangedIndRead(uint16 cid)
{
    cid_map_t *conn;
    
    conn = gattFindConnOrPanic(cid);
    
    return (conn->serv_chg_cccd_enable? 2 : 0);
}

/*************************************************************************
NAME    
    GattHandleServChangedIndWrite
    
DESCRIPTION
    This function handles a write by a client to the Service Changed
    Client Characteristic Configuration Descriptor.
    
RETURNS

*/

void GattHandleServChangedIndWrite(uint8 value, uint16 cid)
{
    gatt_td_data_t *gd;
    tp_bdaddr *client = PanicUnlessMalloc(sizeof(tp_bdaddr));
    cid_map_t *conn = gattFindConnOrPanic(cid); /* never NULL */
    uint8 ind_val = (value == 2 ? 1 : 0);
    
    VmGetBdAddrtFromCid(cid, client);
    
    /* Store in non-persistent memory for non-trusted devices. */
    conn->serv_chg_cccd_enable = ind_val;
    
    /* If client is a trusted device, any changes need to be stored in PS.
     * Load the struct to make any necessary changes.
     */
    gd = (gatt_td_data_t *) PanicUnlessMalloc(SIZE_GATT_TD);
    
    if (ConnectionGetGattAttribute(&client->taddr, SIZE_GATT_TD, (uint8 *)gd))
    {
        /* Store trusted device data back into PS. */
        gd->content.indications_enabled = ind_val;
        ConnectionPutGattAttribute(&client->taddr, SIZE_GATT_TD, (uint8 *)gd);
    }

    free(gd);
    free(client);
}
