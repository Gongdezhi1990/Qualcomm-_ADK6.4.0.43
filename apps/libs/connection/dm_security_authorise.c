/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_authorise.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "connection_tdl.h"

#include <message.h>
#include <string.h>
#include <vm.h>


/*****************************************************************************/
void ConnectionSmAuthoriseResponse(
        const bdaddr*   bd_addr,
        dm_protocol_id  protocol_id,
        uint32          channel,
        bool            incoming,
        bool            authorised
        )
{
#ifdef CONNECTION_DEBUG_LIB
    if ((protocol_id != protocol_l2cap) && (protocol_id != protocol_rfcomm))
    {
        CL_DEBUG(("ConnectionSmAuthoriseResponse - Out of range protocol id 0x%x\n", 
                    protocol_id));
    }

    if ( (protocol_id == protocol_rfcomm) &&
            /* If RFCOMM */
            incoming &&
            /* server channel */
            ( (channel < RFCOMM_SERVER_CHANNEL_MIN) || 
              (channel > RFCOMM_SERVER_CHANNEL_MAX) )
            /* and not in Valid range */
       )
    {
        /* Panic with a meaningful error message */
        CL_DEBUG(("cd ..Out of range RFCOMM server channel 0x%lx\n", channel));
    }

    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr));
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_AUTHORISE_RES);
        message->bd_addr = *bd_addr;
        message->protocol_id = protocol_id;
        message->channel = channel;
        message->incoming = incoming;
        message->authorised = authorised;
        MessageSend(
                connectionGetCmTask(),
                CL_INTERNAL_SM_AUTHORISE_RES,
                message
                );
    }
}


/*****************************************************************************/
void ConnectionSmPinCodeResponse(
        const typed_bdaddr* taddr,
        uint8               length,
        const uint8*        pin_code
        )
{   
#ifdef CONNECTION_DEBUG_LIB
    if(taddr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)taddr)); 
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_PIN_REQUEST_RES);
        message->taddr = *taddr;
        message->pin_length = length;
        if (length > 0)
            memmove(message->pin, pin_code, length);
        MessageSend(
                connectionGetCmTask(), 
                CL_INTERNAL_SM_PIN_REQUEST_RES, 
                message
                );
    }
}


/*****************************************************************************/
void ConnectionSmIoCapabilityResponse(
        const tp_bdaddr*    tpaddr,
        cl_sm_io_capability io_capability,
        mitm_setting        mitm,
        bool                bonding,
        uint16              key_distribution,
        uint16              oob_data,
        uint8*              oob_hash_c,
        uint8*              oob_rand_r
        )
{
#ifdef CONNECTION_DEBUG_LIB
    if (tpaddr == NULL)
    {
        CL_DEBUG(("Bluetooth address is NULL\n"));
    }
    else if (tpaddr->transport == TRANSPORT_NONE)
    {
        CL_DEBUG(("Bluetooth address transport type is NONE\n"));
    }
    else if (tpaddr->taddr.type == TYPED_BDADDR_INVALID)
    {
        CL_DEBUG(("Bluetooth address type is INVALID\n")); 
    }
#endif
    {
        uint16 rand_r_len = 0;
        uint16 hash_c_len = 0;
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES);

        message->tpaddr = *tpaddr;
        message->io_capability = io_capability;
        message->bonding = bonding;
        message->mitm = mitm;

        message->oob_data = DM_SM_OOB_DATA_NONE;    /* Bitfield default of 0 */
        message->oob_hash_c = NULL;
        message->oob_rand_r = NULL;        

        if (oob_data & oob_data_p192)
        {
            /* For legacy BLE pairing, only HASH_C data is required. */
            if (tpaddr->transport == TRANSPORT_BREDR_ACL)
            {
                rand_r_len += CL_SIZE_OOB_DATA;
            }
            hash_c_len += CL_SIZE_OOB_DATA;
            message->oob_data |= DM_SM_OOB_DATA_P192;
        }
        if (oob_data & oob_data_p256)
        {
            rand_r_len += CL_SIZE_OOB_DATA;
            hash_c_len += CL_SIZE_OOB_DATA;
            message->oob_data |= DM_SM_OOB_DATA_P256;
        }

        if (rand_r_len)
        {
#ifdef CONNECTION_DEBUG_LIB
            if (!oob_rand_r)
                CL_DEBUG(("OOB oob_data = 0x%0X but no expected oob_rand_r data", 
                            oob_data));
#endif
            message->oob_rand_r = PanicUnlessMalloc(rand_r_len);
            memmove(message->oob_rand_r, oob_rand_r, rand_r_len);
        }

        if(hash_c_len)
        {
#ifdef CONNECTION_DEBUG_LIB
            if (!oob_hash_c)
                CL_DEBUG(("OOB oob_data = 0x%0X but no expected oob_hash_c data", 
                            oob_data));
#endif
            message->oob_hash_c = PanicUnlessMalloc(hash_c_len);
            memmove(message->oob_hash_c, oob_hash_c, hash_c_len);
        }

        /* Not relevant for BR/EDR connections */
        if (tpaddr->transport == TRANSPORT_BREDR_ACL) 
        {
            message->key_distribution = 0;
        }
        else 
        {
            message->key_distribution = key_distribution;
        }
        
        MessageSend(
                connectionGetCmTask(),
                CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES,
                message
                );
    }
}

/*****************************************************************************/
void ConnectionSmUserConfirmationResponse(
        const tp_bdaddr*        tpaddr,
        bool                    confirm
        )
{
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES);
    message->tpaddr = *tpaddr;
    message->confirm = confirm;
    MessageSend(
            connectionGetCmTask(),
            CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES,
            message
            );
}


/*****************************************************************************/
void ConnectionSmUserPasskeyResponse(
        const tp_bdaddr* tpaddr,
        bool cancelled,
        uint32 passkey
        )
{
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES);
    message->tpaddr = *tpaddr;
    message->cancelled = cancelled;
    message->numeric_value = passkey;
    MessageSend(
            connectionGetCmTask(),
            CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES,
            message
            );
}


/*****************************************************************************/
void ConnectionSmDeleteAuthDeviceReq(uint8 type, const bdaddr* bd_addr)
{
    /* Remove the device from the trusted device list if not priority device */
    connectionAuthDeleteDevice(type, bd_addr);
}



/*****************************************************************************/
void ConnectionSmDeleteAllAuthDevices(uint16 ps_base)
{
    /* Remove all but priority devices from the trusted device list */
    connectionAuthDeleteAllDevice(ps_base);
}




