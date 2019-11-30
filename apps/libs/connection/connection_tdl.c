/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    connection_tdl.c

DESCRIPTION
    This file contains the functionality to manage a list of trusted devices.
    A list of devices is maintained in the persistent store.

    DM_SM_KEY_ID, used for BLE Privacy, is not supported yet.
    DM_SM_KEY_SIGN, used for BLE Data Signing, is not supported yet.

NOTES

*/

/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "connection_tdl.h"
#include "../gatt/gatt_caching.h"

#include <bdaddr.h>
#include <message.h>
#include <panic.h>
#include <ps.h>
#include <stdlib.h>
#include <string.h>
#include <vm.h>


/*lint -e525 -e725 -e830 */

/*
   The trusted device list is stored in the user area of Persistent store.

   TRUSTED_DEVICE_INDEX         base for the TDL, contains the index
   TRUSTED_DEVICE_LIST + 0      first device in the list
   TRUSTED_DEVICE_LIST + 1      second device in the list

*/
#define TRUSTED_DEVICE_INDEX    PSKEY_TRUSTED_DEVICE_LIST

#define TRUSTED_DEVICE_LIST     (TRUSTED_DEVICE_INDEX + 1)

/* The number of devices to be manage */
#define MAX_TRUSTED_DEVICES  \
    (((connectionState *)connectionGetCmTask())->smState.TdlNumberOfDevices)

/* Size of the Trusted Device Index in words */
#define TDI_SIZE                ((MAX_NO_DEVICES_TO_MANAGE - 1) / 4 + 1)

/* Unused index */
#define TDI_ORDER_UNUSED              (0xf)



#define MASK_SECURITY_BONDING_BIT             (1<<0)
#define MASK_SECURITY_MITM_PROTECTION_BIT     (1<<2)

#define MASK_SECURITY_SC_BIT_TO_BIT1          (1<<1)

/*
    The Trusted Device index is used to maintain a list of the Most Recently
    Used trusted device.  This index is reordered each time the bonding
    procedure is successful with a remote device.  Each item in this list
    refers to an entry in the Trusted Device List.  Adopting this approach
    results in Persistent Store write operations being kept to a minimum.

    Each TDI element contains the index in the PS of the Trusted Device
    Information (i.e. TRUSTED_DEVICE_LIST + .order
    And also a Hash of the Bdaddr of the device that is used to reduce
    PS reads when searching the list.  This is calculated with
    TDI_HASH macro.
*/
typedef struct
{
    unsigned                    hash:12;
    unsigned                    order:4;
}td_index_element_t;

/* Invalidate hash - indicates the hash for the indexed device is unknown */
#define TDI_HASH_UNUSED            (0x0fff)

/* Macro to calculate the hash of a TDI entry to facilitate faster searches
 * This is simply the first 12 least significant bits of the passed in
 * bdaddr LAP
 */
#define TDI_HASH(bd_address) ((bd_address).lap & 0x0fff)

typedef struct
{
    td_index_element_t          element[MAX_NO_DEVICES_TO_MANAGE];
}td_index_t;

/* Used to convert from DM_SM_KEY_* type, in case the ordering in dm_prim.h
 * should change or be added to. Also, the function calc_td_data_offset()
 * relies on the specific ordering of this enum.
 */
typedef enum
{
    TDL_KEY_ENC_BREDR,
    TDL_KEY_ENC_CENTRAL,
    TDL_KEY_DIV,
    TDL_KEY_SIGN,
    TDL_KEY_ID,
    TDL_KEY_UNKNOWN
} TDL_DATA_TYPE_T;

#define MASK_ENC_BREDR      (1<<TDL_KEY_ENC_BREDR)
#define MASK_ENC_CENTRAL    (1<<TDL_KEY_ENC_CENTRAL)
#define MASK_DIV            (1<<TDL_KEY_DIV)
#define MASK_SIGN           (1<<TDL_KEY_SIGN)
#define MASK_ID             (1<<TDL_KEY_ID)

#define BLE_ONLY_LINK_KEYS  (MASK_ENC_CENTRAL | MASK_DIV | MASK_SIGN | MASK_ID)

#define SIZE_ENC_BREDR      (sizeof(DM_SM_KEY_ENC_BREDR_T))
#define SIZE_ENC_CENTRAL    (sizeof(DM_SM_KEY_ENC_CENTRAL_T))
#define SIZE_DIV            (sizeof(uint16))
#define SIZE_ID             (sizeof(DM_SM_KEY_ID_T))

/* Indicates the contents of the packed link key store. */
/*
    A device can be set as "priority device",
    therefore we need an indicator for this.
    Structure below had a 'spare' bit we can
    now use as 'priority_device' flag.
*/
typedef struct _content_t
{
    unsigned    addr_type:2;
    unsigned    trusted:1;
    unsigned    key_size:4;
    unsigned    security_req:3;
    unsigned    priority_device:1;
    unsigned    data_flags:5; /* TDL_DATA_TYPE_T */
} content_t;

/* Encryption key size is in range 7 to 16 octets. Pack it into 4 bits by
 * subtracting 6.
 */
#define PACK_TD_ENC_KEY_SIZE(TDP, EKS) \
     ((TDP)->content.key_size = ((EKS) - 6) & 0xF)

#define UNPACK_TD_ENC_KEY_SIZE(EKS, TDP) \
     ((EKS) = (uint16)(TDP)->content.key_size + 6)

/* Trusted Device data - in packed format. The 'content' field dictates the
 * size and content of the data field.
 */
typedef struct _td_data_t
{
    uint16              bdaddr[3];
    content_t           content;
    uint16              data[1];
} td_data_t;

/* Size of a single entry of data: sizeof(td_data_t::data[0])
 */
#define SIZE_DATA_ELEMENT sizeof(uint16)

/* Size of the td_data_t type without the data e.g. just the packed bdaddr
 * and content fields.
 */
#define SIZE_TD         (sizeof(td_data_t)-SIZE_DATA_ELEMENT)

#define SIZE_TD_DATA_T (SIZE_ENC_BREDR + \
                        SIZE_ENC_CENTRAL + \
                        SIZE_DIV + \
                        SIZE_ID + \
                        SIZE_TD)

/* Ram copy of the TD Index (tdi) to reduce the number of PS store accesses. */
typedef struct
{
    bool valid;
    td_index_t tdi;
} tdi_cache_t;

static tdi_cache_t tdi_cache = {0};

#define GET_TDI_CACHE (tdi_cache.tdi)


/****************************************************************************

DESCRIPTION
    Reads the Trusted Device List Index from PS and unpacks it into the
    td_index_t structure.
*/
static void read_trusted_device_index(td_index_t *tdi)
{
    uint16  ps_tdi[TDI_SIZE];

    if (!tdi_cache.valid)
    {
        if (PsRetrieve(TRUSTED_DEVICE_INDEX, ps_tdi, TDI_SIZE) != TDI_SIZE)
        {
            uint16 i;
            for(i = 0; i < MAX_NO_DEVICES_TO_MANAGE; i++)
            {
                tdi_cache.tdi.element[i].order = TDI_ORDER_UNUSED;
                tdi_cache.tdi.element[i].hash  = TDI_HASH_UNUSED;
            }
        }
        else
        {
            uint16 i;

            /* Unpack TDI */
            for (i = 0; i < MAX_NO_DEVICES_TO_MANAGE; i++)
            {
                tdi_cache.tdi.element[i].order = (ps_tdi[i / 4] >> (4 * (i % 4))) & 0x0F;
                /* hash value for this device as yet unknown */
                tdi_cache.tdi.element[i].hash  = TDI_HASH_UNUSED;
            }
        }

        tdi_cache.valid = TRUE;
    }

    /* copy TDI list if buffer provided */
    if(tdi)
    {
        memmove(tdi, &tdi_cache.tdi, sizeof(*tdi));
    }
}




/****************************************************************************
NAME
    ConnectionTrustedDeviceListSize()

FUNCTION
    Returns how many devices are in the trusted device list

RETURNS
    uint8
*/
uint16 ConnectionTrustedDeviceListSize(void)
{
    uint16      count;

    for (
            count = 0;
            count < MAX_TRUSTED_DEVICES && GET_TDI_CACHE.element[count].order != TDI_ORDER_UNUSED;
            count++
         ) ;
    return count;
}

/****************************************************************************

DESCRIPTION
    Store the Trusted Device List Index in to PS and pack it, using 4-bits
    per index value.
    Also updates the cached copy (both order and hash)
*/
static void store_trusted_device_index(const td_index_t *tdi)
{
    uint16 ps_tdi[TDI_SIZE];
    uint16 i;

    /* Only store the TDI cache if it has changed */
    if (memcmp(tdi, &tdi_cache.tdi, sizeof(*tdi)))
    {
        /* Update cached copy */
        memmove(&tdi_cache.tdi, tdi, sizeof(*tdi));

        memset(ps_tdi, 0, TDI_SIZE * sizeof(uint16));
        /* Pack TDI */
        for (i = 0; i < MAX_NO_DEVICES_TO_MANAGE; i++)
            ps_tdi[i / 4] |= (tdi_cache.tdi.element[i].order & 0x0F) << (4 * (i % 4));

        PsStore(TRUSTED_DEVICE_INDEX, ps_tdi, TDI_SIZE);
    }
}


/****************************************************************************
NAME
    convert_dm_sm_to_tdl_key_type

FUNCTION
    Converts DM_SM_KEY_* type to the associated CL TDL_KEY_* type, in case the
    DM_SM_KEY_TYPE_T ever changes in content or order. Also, dictates the bit
    order in the content_t 'data_flags' field.

RETURNS
    TDL_DATA_TYPE_T

*/
static TDL_DATA_TYPE_T convert_dm_sm_to_tdl_key_type(DM_SM_KEY_TYPE_T key_type)
{
    switch (key_type)
    {
        case DM_SM_KEY_ENC_BREDR:   return TDL_KEY_ENC_BREDR;
        case DM_SM_KEY_ENC_CENTRAL: return TDL_KEY_ENC_CENTRAL;
        case DM_SM_KEY_DIV:         return TDL_KEY_DIV;
        case DM_SM_KEY_ID:          return TDL_KEY_ID;
        default:
            break;
    }
    return TDL_KEY_UNKNOWN;
}

/****************************************************************************
NAME
    get_data_type_size

FUNCTION
    Returns the size of the associated key data.

RETURNS
    uint16

*/
static uint16 get_data_type_size(TDL_DATA_TYPE_T key_type)
{
    switch (key_type)
    {
        case TDL_KEY_ENC_BREDR:     return SIZE_ENC_BREDR;
        case TDL_KEY_ENC_CENTRAL:   return SIZE_ENC_CENTRAL;
        case TDL_KEY_DIV:           return SIZE_DIV;
        case TDL_KEY_ID:            return SIZE_ID;
        default:
            break;
    }
    return 0;
}

/****************************************************************************
NAME
    pack_td_bdaddr

FUNCTION
    Pack a type and bdaddr address into the td_data_t type.

RETURNS

*/
static void pack_td_bdaddr(td_data_t *td, uint8 type, const bdaddr *addr)
{
    td->content.addr_type = type;
    td->bdaddr[0] = addr->nap;
    td->bdaddr[1] = (uint16)addr->uap << 8 |
                    (uint16)((addr->lap & 0x00FF0000) >> 16);
    td->bdaddr[2] = (uint16)addr->lap;
}

/****************************************************************************
NAME
    unpack_td_typed_bdaddr

FUNCTION
    Unpack a typed_bdaddr address from the td_data_t type.

RETURNS
    bdaddr type (TYPED_BDADDR_*).
*/
static uint8 unpack_td_bdaddr(bdaddr *addr, const td_data_t *td)
{
    addr->nap = (uint16)td->bdaddr[0];
    addr->uap = (uint8)(td->bdaddr[1] >> 8);
    addr->lap = (uint32)(td->bdaddr[1] & 0xFF) << 16 | td->bdaddr[2];
    return (uint8)td->content.addr_type;
}


/****************************************************************************
NAME
    unpack_td_TYPED_BDADDR_T

FUNCTION
    Unpack a TYPED_BD_ADDR_T address from the td_data_t type.

RETURNS

*/
static void unpack_td_TYPED_BDADDR_T(
                TYPED_BD_ADDR_T *addrt,
                const td_data_t *td
                )
{
    addrt->type     = (uint8_t)td->content.addr_type;
    addrt->addr.nap = (uint16_t)td->bdaddr[0];
    addrt->addr.uap = (uint8_t)(td->bdaddr[1] >> 8);
    addrt->addr.lap = (uint24_t)(td->bdaddr[1] & 0xFF) << 16 | td->bdaddr[2];
}


/****************************************************************************
NAME
    pack_td_security_requirements

FUNCTION
    Pack a key security requirements into the td_data_t type. The
    dm_prim.h defines DM_SM_SECURITY_ only use 3-bits (that VM is interested
    in), only the SC enable bit needs to be relocated.

RETURNS

*/
static void pack_td_security_requirements(
                    td_data_t   *td,
                    uint16      security_requirements
                    )
{
    /* copy bits 3 (DM_SM_SECURITY_MITM_PROTECTION)and 1(DM_SM_SECURITY_BONDING).
    * to  content.security_req
    */
    td->content.security_req = security_requirements &
           (MASK_SECURITY_BONDING_BIT | MASK_SECURITY_MITM_PROTECTION_BIT);

    /* relocate bit 4(DM_SM_SECURITY_SECURE_CONNECTIONS) to bit 2 of
    * content.security_req as connection library only uses  3 bits
    */
    td->content.security_req |= (security_requirements >> 2) &
                                      MASK_SECURITY_SC_BIT_TO_BIT1;
}

/****************************************************************************
NAME
    unpack_td_security_requirements

FUNCTION
    Unpack the key security requirements from the td_data_t type.

RETURNS

*/
static uint16 unpack_td_security_requirements(const td_data_t *td)
{
    /* copy bits 3 (DM_SM_SECURITY_MITM_PROTECTION)and bits 1
    * (DM_SM_SECURITY_BONDING) from content.security_req back to sec_req
    */
    uint16 sec_req = td->content.security_req &
           (MASK_SECURITY_BONDING_BIT | MASK_SECURITY_MITM_PROTECTION_BIT);

    /* Relocate bit 2(used for SC bit) back to bit 4
    * (DM_SM_SECURITY_SECURE_CONNECTIONS) of sec_req
    */
    sec_req |= (td->content.security_req & MASK_SECURITY_SC_BIT_TO_BIT1) << 2;

    return sec_req;
}

/****************************************************************************
NAME
    calc_td_data_size

FUNCTION
    Calculates the size of a td_data_t data field, based on the data flags set
    in the content field.

RETURNS

*/
static uint16 calc_td_data_size(content_t content)
{
    uint16 size = 0;
    uint16 dt;

    for (dt = TDL_KEY_ENC_BREDR; dt < TDL_KEY_UNKNOWN; dt++)
        if (content.data_flags & (1<<dt))
            size += get_data_type_size((TDL_DATA_TYPE_T)dt);

    return size;
}

/* Calculate the size of td_data_t var passed as a PTR to the macro */
#define CALC_TD_SIZE(TDP)  (SIZE_TD + calc_td_data_size((TDP)->content))

/****************************************************************************
NAME
    calc_td_data_offset

FUNCTION
    Calculates the index offset to the data related to a specific link key,
    as indicated by the flag settings int the content_t type.

    The order of keys is specific to the TDL_DATA_TYPE_T, if they are changed
    in that type the changes must also be reflected here.

RETURNS
    uint16
*/

static uint16 calc_td_data_offset(
                    content_t       content,
                    TDL_DATA_TYPE_T data_type
                    )
{
    uint16_t offset = 0;

    if (data_type == TDL_KEY_ENC_BREDR) return offset;
    if (content.data_flags & MASK_ENC_BREDR) offset += SIZE_ENC_BREDR;

    if (data_type == TDL_KEY_ENC_CENTRAL) return offset;
    if (content.data_flags & MASK_ENC_CENTRAL) offset += SIZE_ENC_CENTRAL;

    if (data_type == TDL_KEY_DIV) return offset;
    if (content.data_flags & MASK_DIV) offset += SIZE_DIV;

    /* Assume that the data type is TDL_KEY_ID, offset will be to that key. */
    return offset;
}

/****************************************************************************
NAME
    pack_td_key

FUNCTION
    Pack a DM_SM_UKEY_T key, indicated by the DM_SM_KEY_TYPE_T.

RETURNS

*/
static td_data_t* pack_td_key(
            td_data_t               **tdp,
            const uint16            *key,
            DM_SM_KEY_TYPE_T        dm_sm_key_type)
{
    td_data_t *td = *tdp;
    TDL_DATA_TYPE_T key_type = convert_dm_sm_to_tdl_key_type(dm_sm_key_type);
    uint16_t offset = calc_td_data_offset(td->content, key_type);
    uint16_t key_size = get_data_type_size(key_type);
    uint16_t key_mask = (1 << key_type);

    /* If not already set, then realloc the td->data to make space.*/
    if (!(td->content.data_flags & key_mask))
    {
        uint16_t size_prev_data = calc_td_data_size(td->content);

        /* Set the content data flag bit to 1 for this key type so that
         * the new size of td_data can be calculated.
         */
        td->content.data_flags |= key_mask;

        *tdp =
            (td_data_t *) PanicNull(
                    realloc(td,  SIZE_TD + calc_td_data_size(td->content) )
                    );

        td = *tdp;

        /* Copy data to make space for new data, if it is below it. */
        if (size_prev_data - offset > 0)
            memmove(
                    &td->data[ (offset + key_size) / SIZE_DATA_ELEMENT ],
                    &td->data[ offset / SIZE_DATA_ELEMENT ],
                    size_prev_data - offset
                    );
    }

    memmove(&td->data[ offset / SIZE_DATA_ELEMENT ], key, key_size);

    return td;
}


/****************************************************************************
NAME
    unpack_td_key

FUNCTION
    Unpack a DM_SM_UKEY_T key, indicated by the DM_SM_KEY_TYPE_T.

RETURNS

*/
static void unpack_td_key(
                uint16                  *key,
                const td_data_t         *td,
                const TDL_DATA_TYPE_T   key_type
                )
{
    uint16_t key_size = get_data_type_size(key_type);
    uint16_t offset = calc_td_data_offset(td->content, key_type);

    memmove(key, &td->data[ offset / SIZE_DATA_ELEMENT ], key_size);
}




/****************************************************************************
NAME
    dm_sm_add_device_req

FUNCTION
    This function creates and populates a DM_SM_ADD_DEVICE_REQ primitive
    and sends it to Bluestack to register a device with the Bluestack
    Security Manager

RETURNS

*/
static void dm_sm_add_device_req( const td_data_t *td )
{
    uint16 key_present[DM_SM_MAX_NUM_KEYS] = {
        DM_SM_KEY_NONE,
        DM_SM_KEY_NONE,
        DM_SM_KEY_NONE,
        DM_SM_KEY_NONE,
        DM_SM_KEY_NONE
    };
    uint16  key_idx = 0;
    TDL_DATA_TYPE_T  dt;

    MAKE_PRIM_T(DM_SM_ADD_DEVICE_REQ);

    unpack_td_TYPED_BDADDR_T(&prim->addrt, td);

    /* The Privacy and Data Signing are not currently supported and
     * so the following fields are default.
     */
    prim->privacy =  DM_SM_PRIVACY_DISABLED;
    prim->reconnection_bd_addr.lap = 0;
    prim->reconnection_bd_addr.uap = 0;
    prim->reconnection_bd_addr.nap = 0;

    prim->trust =
        td->content.trusted ? DM_SM_TRUST_ENABLED : DM_SM_TRUST_DISABLED;

    prim->keys.security_requirements = unpack_td_security_requirements(td);

    for (dt = TDL_KEY_ENC_BREDR; dt < TDL_KEY_UNKNOWN; dt++)
    {
        uint16 *key = NULL;

        if (td->content.data_flags & (1<<dt))
        {
            switch(dt)
            {
                case TDL_KEY_ENC_BREDR:
                    key_present[key_idx] = DM_SM_KEY_ENC_BREDR;
                    key = (uint16 *) PanicUnlessMalloc(
                            sizeof(DM_SM_KEY_ENC_BREDR_T)
                            );
                    break;

                case TDL_KEY_ENC_CENTRAL:
                    key_present[key_idx] = DM_SM_KEY_ENC_CENTRAL;
                    key = (uint16 *) PanicUnlessMalloc(
                            sizeof(DM_SM_KEY_ENC_CENTRAL_T)
                            );
                    break;

                case TDL_KEY_DIV: /* This is a special case copying a value rather
                                     than a pointer */
                    key_present[key_idx] = DM_SM_KEY_DIV;
                    unpack_td_key(&prim->keys.u[key_idx++].div, td, dt);
                    continue;

                case TDL_KEY_ID:
                    key_present[key_idx] = DM_SM_KEY_ID;
                    key = (uint16 *) PanicUnlessMalloc(
                            sizeof(DM_SM_KEY_ID_T)
                            );
                    break;

                default:
                    /* To please the compiler. */
                    break;
            }

            unpack_td_key(key, td, dt);
            key = VmGetHandleFromPointer(key);
            prim->keys.u[key_idx++].none = (void *)key;
        }
    }

    UNPACK_TD_ENC_KEY_SIZE(prim->keys.encryption_key_size,td);

    prim->keys.present = DM_SM_KEYS_PRESENT(
            key_present[0],
            key_present[1],
            key_present[2],
            key_present[3],
            key_present[4],
            DM_SM_KEYS_UPDATE_EXISTING
            );
    VmSendDmPrim(prim);
}

/*************************************************************************
NAME
    tdl_find_device

DESCRIPTION
    Find a device from the trusted device list.

RETURNS
    pointer to the device data (must be freed by the caller), or NULL
    *pp position of the record, or empty slot pos in case of device not found
    *pi index of the current position, e.g. tdi.element[idx]
*/
static td_data_t *tdl_find_device(
                        uint8                   addr_type,
                        const bdaddr            *addr,
                        uint16                  *pp,
                        uint16                  *pi
                        )
{
    td_data_t *td = (td_data_t *) PanicUnlessMalloc(SIZE_TD_DATA_T);
    typed_bdaddr *dev_taddr = PanicUnlessNew(typed_bdaddr);
    uint16 used = 0;
    uint16 pos;
    uint16 idx;
    uint16 max_trusted_devices = MAX_TRUSTED_DEVICES;

    for (idx = 0;
         idx < max_trusted_devices && (pos = GET_TDI_CACHE.element[idx].order) != TDI_ORDER_UNUSED;
         idx++)
    {
        /* Check the stored hash of the device, if it matches only then read from the PS */
        if( GET_TDI_CACHE.element[idx].hash == TDI_HASH(*addr) ||
            GET_TDI_CACHE.element[idx].hash == TDI_HASH_UNUSED )
        {
            if (PsRetrieve(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(SIZE_TD_DATA_T)))
            {
                dev_taddr->type = unpack_td_bdaddr(&dev_taddr->addr, td);

                if (
                    dev_taddr->type == addr_type &&
                    BdaddrIsSame(&dev_taddr->addr, addr)
                    )
                {
                    /* found our device */
                    goto out;
                }

                /* mark slot as used */
                used |= 1 << pos;
            }
        }
        else if (GET_TDI_CACHE.element[idx].hash != TDI_HASH_UNUSED )
        {
            used |= 1 << pos;
        }
    }

    if (idx < max_trusted_devices)
    {
        /* find first free slot */
        /* we do know that there is a free slot so no need for checking
         * idx < max_trusted_devices */
        for (pos = 0; used & (1 << pos); pos++);
    }
    else
    {
        /* set pos to the last/oldest position.
         * expect this pos value to be updated below */
        idx = max_trusted_devices - 1;
        pos = GET_TDI_CACHE.element[idx].order;
        
        /* tdl is full, return the last position with priority device flag unset */        
        for(idx = max_trusted_devices - 1; idx != 0; idx--)
        {
            pos = GET_TDI_CACHE.element[idx].order;
            if(PsRetrieve(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(SIZE_TD_DATA_T)))
            {
                if(!(td->content.priority_device))
                    break;
            }
        }
    }

    /* device data not needed anymore */
    free(td);
    td = NULL;

out:
    free(dev_taddr);

    if (pp)
    {
        *pp = pos;
    }

    if (pi)
    {
        *pi = idx;
    }

    return td;
}

/****************************************************************************
DESCRIPTION
    This function updates the Trusted Device Index

    Moves the specified device to the top of the list
*/
static void update_trusted_device_index(
    const uint16 pos,
    const uint16 idx
    )
{
    td_index_t      tdi;
    uint16          index;
    td_index_element_t tdevice;

    /* Read the TDI from persistent store */
    read_trusted_device_index(&tdi);

    /* Store device info */
    tdevice =  tdi.element[idx];

    /* Re-order TDI */
    for(index = 0; index < idx; index++)
    {
        tdi.element[idx - index]  = tdi.element[idx - index - 1];
    }

    /* Update with the position of the new record in the TDL as MRU
     * device. */
    tdi.element[0].order= pos;
    tdi.element[0].hash = tdevice.hash;

    /* Store persistently */
    store_trusted_device_index(&tdi);
}

/****************************************************************************
DESCRIPTION:
    This functions deletes the device in the TDL at PS Key index POS from the 
    Security Manager in Bluestack.
*/
static void delete_from_security_manager(uint16 pos)
{
    td_data_t td;

    /* Verify there is a TDL entry in this 'pos'. */
    if (PsRetrieve(TRUSTED_DEVICE_LIST + pos,  NULL, 0))
    {
        typed_bdaddr addrt;
        MAKE_PRIM_T(DM_SM_REMOVE_DEVICE_REQ);

        /* Don't have to read all the TDL entry data. */
        PsRetrieve(TRUSTED_DEVICE_LIST + pos, &td, PS_SIZE_ADJ(SIZE_TD));
        
        addrt.type = unpack_td_bdaddr(&addrt.addr, &td);
        BdaddrConvertTypedVmToBluestack(&prim->addrt, &addrt);
        
        VmSendDmPrim(prim);
    } 
}

/****************************************************************************

DESCRIPTION
    This function will delete an entry from the trusted device index and
    then delete the list entry itself.
*/
static uint16 delete_from_trusted_device_list(uint16 pos, uint16 idx)
{
    td_index_t tdi;
    uint16 i;
    uint16 max_trusted_devices = MAX_TRUSTED_DEVICES;

    /* Read the TDI from persistent store */
    read_trusted_device_index(&tdi);

    /* Delete index from TDI and reorder TDI */
    for(i = idx; i < max_trusted_devices - 1; i++)
        tdi.element[i] = tdi.element[i + 1];

    /* LRU index is now invalid */
    tdi.element[max_trusted_devices - 1].order = TDI_ORDER_UNUSED;
    tdi.element[max_trusted_devices - 1].hash  = TDI_HASH_UNUSED;

    store_trusted_device_index(&tdi);

    /* Delete the list entry */
    PsStore(TRUSTED_DEVICE_LIST + pos, NULL, 0);

    /* Delete any associated attribute data, if it exists. */
    if(PsRetrieve(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0))
    {
        PsStore(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0);
    }

    /* Delete any associated GATT attribute data */
    PsStore(PSKEY_TDL_GATT_ATTRIBUTE_BASE + pos, NULL, 0);

    return TRUE;
}

/****************************************************************************
NAME
    connectionCheckSecurityRequirement

FUNCTION
    This function is called to check if SC bit needs to be set as part of
    security request sent to bluestack.

RETURNS
    TRUE if SC bit needs to be set as part of security request.
*/
bool connectionCheckSecurityRequirement(const typed_bdaddr      *taddr)
{
    td_data_t       *td;
    uint16          is_sec_req      = TRUE;
    uint16          sec_req;

    td = tdl_find_device(TBDADDR_PUBLIC, &taddr->addr, NULL, NULL);
    if (td)
    {
        sec_req = unpack_td_security_requirements(td);
        is_sec_req = (sec_req & DM_SM_SECURITY_SECURE_CONNECTIONS) ? TRUE : FALSE;
        free(td);
    }
    return is_sec_req;
}

/****************************************************************************
NAME
    connectionInitTrustedDeviceList

FUNCTION
    This function is called to initialise the Trusted Device List.  All
    devices in the list are registered with the Bluestack Security Manager.

RETURNS
    The number of devices registered with Bluestack
*/
uint16 connectionInitTrustedDeviceList(void)
{
    uint16 i;
    uint16 pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t *td;
    uint16 max_trusted_devices = MAX_TRUSTED_DEVICES;

    /* Set the TDI RAM cache as invalid so that
     * it will be cached from the PS store on first read. */
    memset(&tdi_cache, 0, sizeof(tdi_cache));

    /* read the device index from PS */

    /* Read TDI from PS */
    read_trusted_device_index(NULL);

    td = PanicUnlessMalloc(SIZE_TD_DATA_T);

    for (i = 0;
         i < max_trusted_devices && (pos = GET_TDI_CACHE.element[i].order) != TDI_ORDER_UNUSED;
         i++)
    {
        if (PsRetrieve(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(SIZE_TD_DATA_T)))
        {
            bdaddr tempAddr;
            dm_sm_add_device_req(td);

            /* Store hash of the trusted device so we can improve search efficiency
               later */
            unpack_td_bdaddr(&tempAddr, td);
            GET_TDI_CACHE.element[i].hash = TDI_HASH(tempAddr);
        }

        /* If we fail to read a Link Key then the link keys and index are out
         * of step - this can lead to an infinite reset cycle. To avoid this,
         * delete the TDL and Index, then Panic to cause a reset.
         */
        else
        {
            uint16 j;

            /* Delete the index */
            PsStore(TRUSTED_DEVICE_INDEX, NULL, 0);

            /* Delete all the List Entry PS Keys */
            for (j = 0; j < max_trusted_devices; j++)
            {
                PsStore(TRUSTED_DEVICE_LIST + j, NULL, 0);
            }

            CL_DEBUG_INFO((
                "ERROR: Could not read TDL Entry at index %d, position %d\n",
                i,
                pos
                ));

            Panic();
        }
    }

    free(td);

    return i;
}

/****************************************************************************
NAME
    connectionAuthAddDevice

FUNCTION
    This function is called to add a trusted device to the persistent trusted
    device list.  A flag indicating if the device was successfully added is
    returned.
*/
bool connectionAuthAddDevice(const CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T *req)
{
    uint16          pos; /* TRUSTED_DEVICE_LIST + pos */
    uint16          idx; /* tdi.order[idx] */
    td_data_t       *td     = NULL;
    bool            ok      = TRUE;
                    /* If there is a link key, there is encryption. */
    uint16          sec_req = DM_SM_SECURITY_ENCRYPTION;

    td = tdl_find_device(TBDADDR_PUBLIC, &req->bd_addr, &pos, &idx);

    if(td)
    {
        /* If not bonded BUT there is an existing entry in the TDL, then this
         * device did have a bonded connection but now it does not. Therefore,
         * delete the previously bonded link key BUT only from the TDL. The 
         * SM will be updated with the non-bonded link key.
         */
        if (!req->bonded)
        {
            delete_from_trusted_device_list(pos, idx);

            /* Delete any associated attribute data, if it exists. */
            if(PsRetrieve(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0))
            {
                PsStore(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0);
            }
        }
    }
    else
    {
        /* Delete any existing link key at 'pos' from the SM as it is 
         * going to be over-written by this new one.
         */
        delete_from_security_manager(pos);

        /* If the device is not already in the TDL then we may have to add it, 
         * Possibly overwriting the oldest connection in the TDL.
         */
        td = (td_data_t *)PanicNull( calloc(1,  SIZE_TD_DATA_T ) );
        pack_td_bdaddr(td, TYPED_BDADDR_PUBLIC, &req->bd_addr);
        
        /* Delete any associated attribute data, if it exists. */
        if(PsRetrieve(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0))
        {
            PsStore(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0);
        }
    }

    /* Update the stored link key */
    td = pack_td_key(&td, (uint16 *)&req->enc_bredr ,DM_SM_KEY_ENC_BREDR);
    td->content.trusted = req->trusted ? TRUE : FALSE;

    /*
        When adding, the device is always a non-priority device.
        This is catered for already by 'td' struct being calloc'd.
    */

    if(req->bonded && req->enc_bredr.link_key_type != DM_SM_LINK_KEY_DEBUG)
    {
        sec_req |= DM_SM_SECURITY_BONDING;

        if (
            req->enc_bredr.link_key_type == DM_SM_LINK_KEY_AUTHENTICATED_P192 ||
            req->enc_bredr.link_key_type == DM_SM_LINK_KEY_AUTHENTICATED_P256
           )
        {
            sec_req |= DM_SM_SECURITY_MITM_PROTECTION;
        }

        pack_td_security_requirements(td, sec_req);

        /* Store trusted device persistently in the list */
        if  ( !PsStore(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(CALC_TD_SIZE(td))) )
        {
            ok = FALSE;
        }
        else
        {
            /* Set hash of new device for more efficient list searches */
            GET_TDI_CACHE.element[idx].hash = TDI_HASH(req->bd_addr);

            /* Keep a track of the most recently used device by updating the
             * TDI. Update the index AFTER the list entry, to help avoid
             * mismatch errors.
             */
            update_trusted_device_index(pos, idx);

            /*
                To keep TDL properly sorted we need to explicitly
                call the SetPriority function setting it as a non-priority
                device. Otherwise the sorting wouldn't happen.
            */
            ok = ConnectionAuthSetPriorityDevice(&req->bd_addr, FALSE);
        }
    }
    else
    {
        pack_td_security_requirements(td, sec_req);
    }

    /* Update the device in the Bluestack Security Manager database */
    dm_sm_add_device_req(td);
    free(td);

    return ok;
}

/****************************************************************************
NAME
    connectionAuthGetDevice

FUNCTION
    This function is called to get a trusted device from the persistent trusted
    device list.  A flag indicating if the device was found is returned.
*/
bool connectionAuthGetDevice(
            const bdaddr *peer_bd_addr,
            cl_sm_link_key_type *link_key_type,
            uint16 *link_key,
            uint16 *trusted
            )
{
    td_data_t *td = tdl_find_device(TYPED_BDADDR_PUBLIC, peer_bd_addr, NULL, NULL);

    if (td != NULL)
    {
        DM_SM_KEY_ENC_BREDR_T enc_bredr;

        unpack_td_key((uint16 *)&enc_bredr, td, TDL_KEY_ENC_BREDR);

        *link_key_type = connectionConvertLinkKeyType(enc_bredr.link_key_type);
        memmove(link_key, &enc_bredr.link_key, BREDR_KEY_SIZE * sizeof(uint16));

        *trusted = td->content.trusted;

        free(td);

        return TRUE;
    }

    return FALSE;
}



/****************************************************************************
NAME
    connectionAuthDeleteDevice

FUNCTION
    This function is called to remove a trusted device from the persistent
    trusted device list.  A flag indicating if the device was successfully
    removed is returned. Priority devices are not deleted.
*/
bool connectionAuthDeleteDevice(
        uint8           type,
        const bdaddr*   peer_bd_addr
        )
{
    uint16 pos; /* TRUSTED_DEVICE_LIST + pos */
    uint16 idx; /* tdi.order[idx] */
    bool result = FALSE;
    td_data_t *td = tdl_find_device(type, peer_bd_addr, &pos, &idx);

    /* Search the trusted device list for the specified device */
    if (td != NULL)
    {
        /* Only remove the device if it is not a priority device. */
        if (!td->content.priority_device)
        {
            delete_from_security_manager(pos);

            /* Delete it and re-order TDI */
            delete_from_trusted_device_list(pos, idx);

            /* Delete any associated attribute data, if it exists. */
            if(PsRetrieve(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0))
            {
                PsStore(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0);
            }

            result = TRUE;
        }
        free(td);
    }

    return result;
}


/****************************************************************************
NAME
    connectionAuthDeleteAllDevice

FUNCTION
    This function is called to remove all trusted devices from the persistent
    trusted device list.  A flag indicating if all the devices were
    successfully removed is returned. Priority devices are not deleted.

RETURNS
    TRUE if even one link key is deleted, returns FALSE if the TDL is empty.
*/
bool connectionAuthDeleteAllDevice(uint16 ps_base)
{
    /* Flag to indicate if the devices were deleted */
    bool            deleted = FALSE;

    /* Trusted device list record index */
    uint16          rec = 0;

    /* Trusted device record */
    td_data_t       *td =  (td_data_t *) PanicUnlessMalloc(SIZE_TD_DATA_T);

    /* trusted device index */
    td_index_t      tdi;
    uint16 max_trusted_devices = MAX_TRUSTED_DEVICES;

    UNUSED(ps_base);

    /* Read the TDI from persistent store in case we end up deleting devices */
    read_trusted_device_index(&tdi);

    /* Loop through list of trusted devices */
    for(rec = 0; rec < max_trusted_devices; rec++)
    {
        if  ((tdi.element[rec].order != TDI_ORDER_UNUSED)
              &&
              PsRetrieve(
                TRUSTED_DEVICE_LIST + tdi.element[rec].order,
                td,
                PS_SIZE_ADJ(SIZE_TD_DATA_T)
                )
            )
        {
            /* only if the device is not a priority device */
            if (!td->content.priority_device)
            {
                /* Unregister with Bluestack security manager */
                MAKE_PRIM_T(DM_SM_REMOVE_DEVICE_REQ);
                unpack_td_TYPED_BDADDR_T(&prim->addrt, td);

                /* Delete entry from TDL */
                (void)PsStore(TRUSTED_DEVICE_LIST + tdi.element[rec].order, NULL, 0);
                deleted = TRUE;

                VmSendDmPrim(prim);

                /* Delete any associated attribute data */
                (void)PsStore(PSKEY_TDL_ATTRIBUTE_BASE + tdi.element[rec].order, NULL, 0);

                /* Delete any associated GATT attribute data */
                (void)PsStore(PSKEY_TDL_GATT_ATTRIBUTE_BASE + tdi.element[rec].order, NULL, 0);

                /* set index value to unused after device is deleted */
                tdi.element[rec].order = TDI_ORDER_UNUSED;
                tdi.element[rec].hash  = TDI_HASH_UNUSED;
            }
        }
    }

    /* if any deletions were made store index persistently */
    if (deleted)
    {
        store_trusted_device_index(&tdi);
    }

    free(td);

    return deleted;
}


/****************************************************************************
NAME
    connectionAuthSetTrustLevel

FUNCTION
    This function is called to set the trust level of a device stored in the
    trusted device list.  The Bluestack Security Manager is updated with the
    change.

RETURNS
    TRUE is record updated, otherwise FALSE
*/
bool connectionAuthSetTrustLevel(const bdaddr* peer_bd_addr, uint16 trusted)
{
    uint16 pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t *td = tdl_find_device(TYPED_BDADDR_PUBLIC, peer_bd_addr, &pos, NULL);

    if (td != NULL)
    {
        /* Update the trust level */
        td->content.trusted = trusted ? TRUE : FALSE;

        /* Store the record */
        PsStore(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(CALC_TD_SIZE(td)));

        /* Update Bluestack Security Manager Database */
        dm_sm_add_device_req(td);

        free(td);

        /* Record updated */
        return TRUE;
    }

    /* Record for this device does not exist */
    return FALSE;
}

/****************************************************************************
NAME
    ConnectionAuthSetPriorityDevice

FUNCTION
    This function is called to set the priority flag of a device
    stored in the trusted device list.  The Blustack Security Manager
    is updated with the change.

RETURNS
    TRUE if record updated, otherwise FALSE
*/
bool ConnectionAuthSetPriorityDevice(
        const bdaddr* bd_addr,
        bool is_priority_device
        )
{
    uint16      pos;
    uint16      idx;
    td_data_t   *td = tdl_find_device(TYPED_BDADDR_PUBLIC, bd_addr, &pos, &idx);

    if (td != NULL)
    {
        /* Update the priority flag */
        td->content.priority_device = is_priority_device ? TRUE : FALSE;

        /* Store the record */
        PsStore(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(CALC_TD_SIZE(td)));

        /* update the MRU status of the device */
        update_trusted_device_index(pos, idx);

        /*
           If we are removing the priority status we need to
           move the device down the TDL to be below any still
           existing priority devices.
        */
        if (!is_priority_device)
        {
            td_index_element_t    temp;
            uint16      index;
            td_index_t  tdi;

            /* Read the TDI from persistent store */
            read_trusted_device_index(&tdi);

            /* check all devices to the right to see if they're priority */
            for (index = 1; index < MAX_NO_DEVICES_TO_MANAGE; index++)
            {
                /* if we're at the end of (a non-full) list we're done*/
                if (tdi.element[index].order == TDI_ORDER_UNUSED)
                {
                    break;
                }

                /* otherwise get device's priority */
                PsRetrieve(
                        TRUSTED_DEVICE_LIST + tdi.element[index].order,
                        td,
                        PS_SIZE_ADJ(SIZE_TD_DATA_T)
                        );

                /* if it is a priority device, swap places */
                if (td->content.priority_device)
                {
                    temp = tdi.element[index];
                    tdi.element[index] = tdi.element[index-1];
                    tdi.element[index-1] = temp;
                }
                else
                {
                    /* if it's non-priority device, we're done */
                    break;
                }
            }

            /* we have to store modified TDL Index */
            store_trusted_device_index(&tdi);
        }

        free(td);

        /* Record updated */
        return TRUE;
    }

    /* Record for this device does not exist */
    return FALSE;
}


/****************************************************************************
NAME
    ConnectionAuthGetPriorityDeviceStatus

FUNCTION
    This function is called to get the priority flag of a device
    stored in the trusted device list. Return value indicates whether
    the device is in the TDL in the first place
    (to mimic ConnectionAuthGetPriorityDevice).

RETURNS
    TRUE if device exists, otherwise FALSE.
    is_priority_device gets updated if device exists in TDL.
*/
bool ConnectionAuthGetPriorityDeviceStatus(
        const bdaddr* bd_addr,
        bool *is_priority_device
        )
{
    td_data_t   *td = tdl_find_device(TYPED_BDADDR_PUBLIC, bd_addr, NULL, NULL);

    if (td != NULL)
    {
        /* Get the priority flag */
        *is_priority_device = td->content.priority_device;
        free(td);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME
    ConnectionAuthIsPriorityDevice

FUNCTION
    This function is called to test the priority flag status of a device
    stored in the trusted device list. It assumes the device is already
    in the TDL.

RETURNS
    TRUE if device is protected, otherwise FALSE (also if device does not exist).
*/
bool ConnectionAuthIsPriorityDevice(
        const bdaddr* bd_addr
        )
{
    bool return_value = FALSE;

    td_data_t   *td = tdl_find_device(TYPED_BDADDR_PUBLIC, bd_addr, NULL, NULL);

    if (td != NULL)
    {
        /* Return the priority flag */
        return_value = td->content.priority_device;
        free(td);
    }

    return return_value;
}

/****************************************************************************
NAME
    connectionAuthUpdateMru

FUNCTION
    This function is called to keep a track of the most recently used device.
    The TDI index is updated provided that the device specified is currently
    stored in the TDL.
*/
uint16 connectionAuthUpdateMru(const bdaddr* peer_bd_addr)
{
    uint16 pos; /* TRUSTED_DEVICE_LIST + pos */
    uint16 idx; /* tdi.order[idx] */
    td_data_t *td = tdl_find_device(TYPED_BDADDR_PUBLIC, peer_bd_addr, &pos, &idx);

    if (td != NULL)
    {
        /* Keep a track of the most recently used device by updating the TDI. */
        update_trusted_device_index(pos, idx);

        free(td);

        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME
    connectionAuthPutAttribute

FUNCTION
    This function is called to store the specified data in the specified
    persistent  store key.  The persistent store key is calculated from
    the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthPutAttribute(
        uint16          ps_base,
        uint8           addr_type,
        const bdaddr*   bd_addr,
        uint16          size_psdata,
        const uint8*    psdata
        )
{
    uint16 pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t *td = tdl_find_device(addr_type, bd_addr, &pos, NULL);

    UNUSED(ps_base);

    if (td != NULL)
    {
        PsStore(PSKEY_TDL_ATTRIBUTE_BASE + pos, psdata, PS_SIZE_ADJ(size_psdata));
        free(td);
    }
}


/****************************************************************************
NAME
    connectionAuthGetAttribute

FUNCTION
    This function is called to read the specified data from the specified
    persistent store key.  The persistent store key is calculated from
    the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthGetAttribute(
        Task appTask,
        uint16 ps_base,
        uint8 addr_type,
        const bdaddr* bd_addr,
        uint16 size_psdata
        )
{
    if (appTask)
    {
        /* Send a message back to the application task */
        MAKE_CL_MESSAGE_WITH_LEN(CL_SM_GET_ATTRIBUTE_CFM, size_psdata);
        message->size_psdata = size_psdata;
        message->psdata[0] = 0;

        message->bd_addr = *bd_addr;

        if (
                connectionAuthGetAttributeNow(
                    ps_base,
                    addr_type,
                    bd_addr,
                    size_psdata,
                    message->psdata)
           )
        {
            message->status = success;
        }
        else
        {
            message->status = fail;
        }

        MessageSend(appTask, CL_SM_GET_ATTRIBUTE_CFM, message);
    }
}


/****************************************************************************
NAME
    connectionAuthGetAttributeNow

FUNCTION
    This function is called to read the specified data from the specified
    persistent store key.  The persistent store key is calculated from
    the specified base + the index of the specified device in TDL.

RETURNS
*/
bool connectionAuthGetAttributeNow(
        uint16 ps_base,
        uint8 addr_type,
        const bdaddr* bd_addr,
        uint16 size_psdata,
        uint8* psdata
        )
{
    uint16 pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t *td = tdl_find_device(addr_type, bd_addr, &pos, NULL);

    UNUSED(ps_base);

    if (td != NULL)
    {
        free(td);

        if(size_psdata)
        {
            /* Read attribute data */
            if(PsRetrieve(PSKEY_TDL_ATTRIBUTE_BASE + pos, psdata, PS_SIZE_ADJ(size_psdata)))
            {
                return TRUE;
            }
        }
        else
        {
            /* No attribute data required, so just indicate success */
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************
NAME
    connectionAuthGetIndexedAttribute

FUNCTION
    This function is called to read the specified data from the specified
    persistent store key.  The persistent store key is calculated from
    the specified base + the index of the specified device in TDL.

RETURNS
*/
void connectionAuthGetIndexedAttribute(
    Task appTask,
    uint16 ps_base,
    uint16 mru_index,
    uint16 size_psdata
    )
{
    /* Send a message back to the application task */
    MAKE_CL_MESSAGE_WITH_LEN(CL_SM_GET_INDEXED_ATTRIBUTE_CFM, size_psdata);
    message->size_psdata = size_psdata;
    message->psdata[0] = 0;

    if (
            connectionAuthGetIndexedAttributeNow(
                ps_base,
                mru_index,
                size_psdata,
                message->psdata,
                &message->taddr)
       )
    {
        message->status = success;
    }
    else
    {
        message->status = fail;
    }

    /* Send confirmation back to application */
    MessageSend(appTask, CL_SM_GET_INDEXED_ATTRIBUTE_CFM, message);
}


/****************************************************************************
NAME
    connectionAuthGetIndexedAttributeNow

FUNCTION
    This function is called to read the specified data from the specified
    persistent store key.  The persistent store key is calulated from
    the specified base + the index of the specified device in TDL.

RETURNS
*/
bool connectionAuthGetIndexedAttributeNow(
        uint16          ps_base,
        uint16          mru_index,
        uint16          size_psdata,
        uint8           *psdata,
        typed_bdaddr    *taddr
        )
{
    bool            successful = FALSE;
    uint16          max_trusted_devices = MAX_TRUSTED_DEVICES;

    UNUSED(ps_base);

    if (mru_index < max_trusted_devices)
    {
        /* Only want the address but need to read the maximum key size to
         * ensure PsRetrieve returns a positive (>0) response.
         */
        td_data_t  *td =  (td_data_t *) PanicUnlessMalloc(SIZE_TD_DATA_T);

        /* Read the device record from the Trusted Device List */
        if  (
            GET_TDI_CACHE.element[mru_index].order != TDI_ORDER_UNUSED &&
            PsRetrieve(
                (TRUSTED_DEVICE_LIST + GET_TDI_CACHE.element[mru_index].order),
                td,
                PS_SIZE_ADJ(SIZE_TD_DATA_T)
                )
            )
        {
            /* Get Bluetooth address of device. */
            taddr->type = unpack_td_bdaddr(&taddr->addr, td);

            /* Check if application wants attribute data */
            if (size_psdata != 0)
            {
                /* Read attribute data */
                if  (
                    PsRetrieve(
                        PSKEY_TDL_ATTRIBUTE_BASE + GET_TDI_CACHE.element[mru_index].order,
                        psdata,
                        PS_SIZE_ADJ(size_psdata))
                    )
                    successful = TRUE;
            }
            else
            {
                /* No attribute data required, so just indicate success */
                successful = TRUE;
            }
        }

        free(td);

    }
    return successful;
}

/****************************************************************************
NAME
    ConnectionPutGattAttribute

FUNCTION
    This function is called to store the specified GATT-related data in the
    specified persistent  store key.  The persistent store key is calculated
    from the specified base + the index of the specified device in TDL.

RETURNS
*/
void ConnectionPutGattAttribute(
        typed_bdaddr*   taddr,
        uint16          size_psdata,
        const uint8*    psdata
        )
{
    uint16 pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t *td = tdl_find_device(taddr->type, &taddr->addr, &pos, NULL);

    if (td != NULL)
    {
        PsStore(PSKEY_TDL_GATT_ATTRIBUTE_BASE + pos, psdata, PS_SIZE_ADJ(size_psdata));
        free(td);
    }
}

/****************************************************************************
NAME
    ConnectionGetGattAttribute

FUNCTION
    This function is called to read the specified GATT-related data from the
    specified persistent store key.  The persistent store key is calculated from
    the specified base + the index of the specified device in TDL.

RETURNS
*/
bool ConnectionGetGattAttribute(
        typed_bdaddr* taddr,
        uint16 size_psdata,
        uint8* psdata
        )
{
    uint16 pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t *td = tdl_find_device(taddr->type, &taddr->addr, &pos, NULL);

    if (td != NULL)
    {
        free(td);

        /* Read attribute data */
        if(PsRetrieve(PSKEY_TDL_GATT_ATTRIBUTE_BASE + pos, psdata, PS_SIZE_ADJ(size_psdata)))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************
NAME
    connectionGetDatabaseHash

FUNCTION
    This function is called to read the stored database hash value.

RETURNS
*/
bool ConnectionGetDatabaseHash(uint16 size_psdata, uint8 *psdata)
{
    if (size_psdata != 0)
    {
        return PsRetrieve(PSKEY_GATT_DB_HASH, psdata, PS_SIZE_ADJ(size_psdata));
    }
    
    return 0;
}

/****************************************************************************
NAME
    connectionPutDatabaseHash

FUNCTION
    This function is called to store the generated database hash value.

RETURNS
*/
void ConnectionPutDatabaseHash(uint16 size_psdata, uint8 *psdata)
{
    if (size_psdata != 0)
    {
        PsStore(PSKEY_GATT_DB_HASH, psdata, PS_SIZE_ADJ(size_psdata));
    }
}

/****************************************************************************
NAME
    ConnectionGattTdlBootSequence

FUNCTION
    This function is called at boot to check if any trusted clients exist in
    the TDL, update their related data to change-unaware if a DB change has
    occured, and return the details of any trusted clients that have enabled
    robust caching to the GATT library so that Bluestack can be notified.

RETURNS
    A count of how many trusted clients support robust caching.
*/
uint8 ConnectionGattTdlBootSequence(bool hashes_matched, tp_bdaddr *device_list, bool *status_list)
{
    uint16 pos;     /* TDL_GATT_ATTRIBUTE_BASE + pos*/
    uint8 i;
    uint8 count;
    gatt_td_data_t *gd;
    
    uint16 max_trusted_devices = MAX_TRUSTED_DEVICES;
    gd = PanicUnlessMalloc(SIZE_TD_DATA_T);
    count = 0;
    
    for (i = 0;
         i < max_trusted_devices && (pos = GET_TDI_CACHE.element[i].order) != TDI_ORDER_UNUSED;
         i++)
    {
        if(PsRetrieve(PSKEY_TDL_GATT_ATTRIBUTE_BASE + pos, gd, PS_SIZE_ADJ(SIZE_TD)))
        {
            /* If the DB hashes didn't match, a DB change has occured since the last boot, thus 
             * every trusted client is now change-unaware. Update their status accordingly.
             */
            if (!hashes_matched)
            {
                gd->content.change_aware = 0;
                PsStore(PSKEY_TDL_GATT_ATTRIBUTE_BASE + pos, gd, PS_SIZE_ADJ(SIZE_TD));
            }
            
            if (gd->content.robust_caching)
            {
                device_list[count].taddr.type = unpack_td_bdaddr(&device_list[count].taddr.addr, (td_data_t *) gd);
                status_list[count] = gd->content.change_aware;
                count++;
            }
        }
    }
    
    free(gd);
    
    return count;
}

/****************************************************************************
NAME
    connectionAuthUpdateTdl

FUNCTION
    Update the TDL for the device with keys indicated. Keys are packed
    for storage in PS, as much as possible.

RETURNS

*/
void connectionAuthUpdateTdl(
    const TYPED_BD_ADDR_T   *addrt,
    const DM_SM_KEYS_T      *keys
    )
{
    uint16          pos; /* TRUSTED_DEVICE_LIST + pos */
    uint16          idx; /* tdi.element[idx].order */
    td_data_t       *td;
    gatt_td_data_t  *gd;
    uint16          key_idx;
    uint16          key_type;
    bool            ble_key_present = FALSE;

    typed_bdaddr *taddr = PanicUnlessNew(typed_bdaddr);
    BdaddrConvertTypedBluestackToVm(taddr, addrt);
    td = tdl_find_device(taddr->type, &taddr->addr, &pos, &idx);

    /* Device not found - create new entry, or remove the oldest link key
     * at that position in the Trusted Device List, and replace it with the
     * new one. */
    if (!td)
    {
        /* Delete any existing link key at 'pos' from the SM as it is 
         * going to be over-written by this new one.
         */
        delete_from_security_manager(pos);
        
        td = (td_data_t *)PanicNull( calloc(1, SIZE_TD_DATA_T));
        pack_td_bdaddr(td, taddr->type, &taddr->addr);

        if(PsRetrieve(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0))
        {
            PsStore(PSKEY_TDL_ATTRIBUTE_BASE + pos, NULL, 0);
        }

        /* Since this is a new trusted device, set up and store a
         * GATT attributes entry. No need to delete any existing data,
         * simply overwrite it. Deletion functions should have ensured
         * nothing was here anyway.
         */
        gd = (gatt_td_data_t *)PanicNull( calloc(1,  SIZE_TD ) );
    
        pack_td_bdaddr((td_data_t *)gd, taddr->type, &taddr->addr);
        gd->content.change_aware = 1;
        gd->content.robust_caching = 0;
        PsStore(PSKEY_TDL_GATT_ATTRIBUTE_BASE + pos, gd, PS_SIZE_ADJ(SIZE_TD));
        
        free(gd);

        /* The location in the TDL to add the new entry is in 
           idx. Set the HASH. */
        GET_TDI_CACHE.element[idx].hash = TDI_HASH(taddr->addr);
    }

    free(taddr);

    for (key_idx=0; key_idx<DM_SM_MAX_NUM_KEYS; key_idx++)
    {
        key_type =
            keys->present >> (DM_SM_NUM_KEY_BITS * key_idx) & DM_SM_KEY_MASK;

        switch(key_type)
        {
        case DM_SM_KEY_SIGN:
            /* SIGN/CSRK is not currently supported (persistently stored). */
            /* Drop through. */
        case DM_SM_KEY_NONE:
            /* No action required */
            break;
        /* The DIV key is accessed as a uint16 value, not through a pointer */
        case DM_SM_KEY_DIV:
            ble_key_present = TRUE;
            td = pack_td_key(&td, &keys->u[key_idx].div, DM_SM_KEY_DIV);
            break;
        /* Other link keys are accessed as uint16 arrays. */
        case DM_SM_KEY_ENC_CENTRAL:
            /* Drop through */
        case DM_SM_KEY_ID:
            ble_key_present = TRUE;
            /* Drop through */
        case DM_SM_KEY_ENC_BREDR:
            td = pack_td_key(&td, (uint16 *)keys->u[key_idx].none, key_type);
            break;
        default:
            break;
        }
    }

    /* The DM_SM_KEYS_T security_requirements field is only relevant for BLE link keys.
     * DO NOT UPDATE IT in PS Store if only the BR/EDR link key is present.
     */
    if (ble_key_present)
    {
        pack_td_security_requirements(td, keys->security_requirements);

        PACK_TD_ENC_KEY_SIZE(td, keys->encryption_key_size);
    }

    /* Update the trusted device list to indicate this was the most recent. */
    if ( PsStore(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(CALC_TD_SIZE(td))) )
    {
        update_trusted_device_index(pos, idx);
    }

    free(td);

}


/****************************************************************************
NAME
    connectionAuthDeleteDeviceFromTdl

FUNCTION
    Search the TDL for the device indicated and remove it from the index
    (effectively deleting it from the TDL).

RETURNS

*/

void connectionAuthDeleteDeviceFromTdl(const TYPED_BD_ADDR_T *addrt)
{
    uint16          idx;
    uint16          pos;
    td_data_t       *td;
    typed_bdaddr    taddr;

    BdaddrConvertTypedBluestackToVm(&taddr, addrt);

    td = tdl_find_device(taddr.type, &taddr.addr, &pos, &idx);

    if (td)
    {
        delete_from_trusted_device_list(pos, idx);
        free(td);
    }
}

#ifndef DISABLE_BLE
/****************************************************************************
NAME
    ConnectionDmBleAddTdlDevicesToWhiteList

FUNCTION
    Add devices in the TDL to the BLE White List

RETURNS

*/
void ConnectionDmBleAddTdlDevicesToWhiteListReq(bool ble_only_devices)
{
    uint16      i;
    uint16      pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t   *td;
    bdaddr      addr;
    uint8       addr_type;
    uint16      max_trusted_devices = MAX_TRUSTED_DEVICES;

    td = PanicUnlessMalloc(SIZE_TD_DATA_T);

    for (i = 0;
         i < max_trusted_devices && (pos = GET_TDI_CACHE.element[i].order) != TDI_ORDER_UNUSED;
         i++)
    {
        if (PsRetrieve(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(SIZE_TD_DATA_T)))
        {
            /* If only adding BLE devices and there are no BLE link keys
             * for this device, then continue to the next in the list.
             */
            if (
                ble_only_devices &&
                !(td->content.data_flags & BLE_ONLY_LINK_KEYS)
               )
            {
                continue;
            }

            addr_type = unpack_td_bdaddr(&addr, td);
            ConnectionDmBleAddDeviceToWhiteListReq(addr_type, &addr);
        }
    }

    free(td);
}

/****************************************************************************
NAME
    ConnectionBondedToPrivacyEnabledDevice

FUNCTION
    Determine if this BlueCore is bonded with a privacy enabled device.

RETURNS
    TRUE if it is bonded to a privacy enabled device, otherwise FALSE.

*/
bool ConnectionBondedToPrivacyEnabledDevice(void)
{
    uint16      i;
    uint16      pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t*  td;
    bool        result = FALSE;
    uint16      max_trusted_devices = MAX_TRUSTED_DEVICES;

    td = PanicUnlessMalloc(SIZE_TD_DATA_T);

    for (i = 0;
         i < max_trusted_devices && (pos = GET_TDI_CACHE.element[i].order) != TDI_ORDER_UNUSED;
         i++)
    {
        if (PsRetrieve(TRUSTED_DEVICE_LIST + pos, td, PS_SIZE_ADJ(SIZE_TD_DATA_T)))
        {
            /* Check for the IRK (ID link key).
             * This indicates that we have bonded with a device using privacy.
             */
            if (td->content.data_flags & MASK_ID)
            {
                result = TRUE;
                break;
            }
        }
    }

    free(td);

    return result;
}
/****************************************************************************
NAME
    ConnectionDmBleCheckTdlDeviceAvailable

FUNCTION
    Check in the TDL if any LE device is paired

RETURNS

*/
bool ConnectionDmBleCheckTdlDeviceAvailable(void)
{
    uint16      i;
    uint16      pos; /* TRUSTED_DEVICE_LIST + pos */
    td_data_t   *td;
    uint16     max_trusted_devices = MAX_TRUSTED_DEVICES;
    bool        ret = FALSE;

    td = PanicUnlessMalloc(SIZE_TD_DATA_T);

    for (i = 0;
         i < max_trusted_devices && (pos = GET_TDI_CACHE.element[i].order) != TDI_ORDER_UNUSED;
         i++)
    {
        if (PsRetrieve(TRUSTED_DEVICE_LIST + pos, td, SIZE_TD_DATA_T))
        {
            /* check if there are BLE link keys for this device.
             */
            if (td->content.data_flags & BLE_ONLY_LINK_KEYS)
            {
                ret =  TRUE; /*there is LE device pairing information available*/
                break;
            }
        }
    }

    free(td);
    return ret;
}

#endif /* DISABLE_BLE */
/*lint +e525 +e725 +e830 */
