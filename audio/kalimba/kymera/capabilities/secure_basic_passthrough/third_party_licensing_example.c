/****************************************************************************
 * Copyright (c) 2013 - 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  secure_basic_passthrough.c
 * \ingroup  operators
 *
 *  Basic passthrough operator
 *
 */
/****************************************************************************
Include Files
*/
#include "secure_basic_passthrough_private.h"
#include "op_msg_helpers.h"
#include "adaptor/adaptor.h"
#include "ps/ps.h"
#include "obpm_prim.h"
#include "common/interface/util.h"

#include "third_party_licensing_example.h"

/****************************************************************************
Private Constant Definitions
*/

/****************************************************************************
Private Type Definitions
*/

/****************************************************************************
Private Macro Definitions
*/
/*
 * For in-house testing of the example, enable QDEBUG_VERIFY.
 * Else disable or remove QDEBUG_VERIFY.
 */
#define QDEBUG_VERIFY      1

/*****************************************************************************
Private Constant Declarations
*/
#ifdef LEGACY_LICENSING

static void ask_btaddr_via_unsolicited_message(OPERATOR_DATA *op_data);
static bool secure_basic_passthrough_retrieve_license_callback(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length,  unsigned* data, STATUS_KYMERA status, uint16 extra_status_info);

static inline SECURE_BASIC_PASSTHROUGH_OP_DATA *get_instance_data(OPERATOR_DATA *op_data)
{
    return (SECURE_BASIC_PASSTHROUGH_OP_DATA *) base_op_get_instance_data(op_data);
}

/* [1] Request license key --------------------------------------------------------- */

/* First, request the license key. This is stored in a ps key. The example license
 * should be stored in 'subsys3_psflash.htf' in the ADK. The example key has a range
 * of BT Addresses for which the example license key is valid. A customer may or may
 * not limit its license to a BT Address range. The first 6 octets in the example
 * license are the BT Address range, octets 6,7,8 are the example license. Everything
 * is in plain text, a customer application is expected to add a cryptographically
 * secure method. The example license is:-
 *
 * # 0xC000   BT ADDR range from:   to:                 license key:
 * 0x600000 = [ 02 00 5b 00 01 af   02 00 5b 00 ff af   22 11 44 33 66 55 ]
 *
 * assuming a device pskey_bdaddr={0x00af01,0x5b,0x0002} in 'subsys1_config2.htf'.
 */
void basic_passthrough_retrieve_license_req(OPERATOR_DATA *op_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *op_extra_data = get_instance_data(op_data);
    PS_KEY_TYPE key;

    L2_DBG_MSG("[1] Request license key");

    op_extra_data->lic.state  = 0;
#ifdef QDEBUG_VERIFY
    set_lic_debug(&op_extra_data->lic);
#endif

    /* The ucid is 'free' to use. This is usually used to distinguish different instances
     * of the same capability (operator). See CS-00304075-SP. If such functionality is not
     *required for a customer, the ucid can be used for something else.
     */
    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(base_op_get_cap_id(op_data), op_extra_data->ucid, OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read(op_data, key, PERSIST_ANY, secure_basic_passthrough_retrieve_license_callback);
    op_extra_data->lic.state |= (1<<license_state_ps_key_requested);
}

/* [2] Callback -------------------------------------------------------------------- */

/* This callback is called as a reply to the ps key request in [1] above.
 * It contains the ps key with the user/customer license. In this example
 * this is just a plain-text license for demonstration purposes.
 *
 */
static bool secure_basic_passthrough_retrieve_license_callback(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status, uint16 extra_status_info)
{
    OPERATOR_DATA *op_data = (OPERATOR_DATA*) instance_data;
    SECURE_BASIC_PASSTHROUGH_OP_DATA *op_extra_data = get_instance_data(op_data);

    L2_DBG_MSG1("[2a] License key response. Status 0x%x", status);

    op_extra_data->lic.state |= (1<<license_state_ps_key_received);

    op_extra_data->lic.key    = key;
    op_extra_data->lic.rank   = rank;

    if (status == STATUS_OK)
    {
        /* In this example, the example license key uses a BT Address range in the first 6 octets */
        if (length > 5)
        {
            /* If customer has chosen to use license BT ADDR range, then read in BT ADDR range */
            op_extra_data->lic.btaddr_start.nap = (data[0]&0xffff);               /* Non-significant 32..47 (0x0002) */
            op_extra_data->lic.btaddr_start.uap = (data[1]&0xff);                 /* Upper address   24..31 (  0x5b) */
            op_extra_data->lic.btaddr_start.lap =  data[2]|((data[1]&0xff00)<<8); /* Lower address   00..23 (0xaf01) */
            op_extra_data->lic.btaddr_end.nap   = (data[3]&0xffff);               /* Non-significant 32..47 (0x0002) */
            op_extra_data->lic.btaddr_end.uap   = (data[4]&0xff);                 /* Upper address   24..31 (  0x5b) */
            op_extra_data->lic.btaddr_end.lap   =  data[5]|((data[4]&0xff00)<<8); /* Lower address   00..23 (0xafff) */

            L2_DBG_MSG4("[2b] License key response. BT address range %04x %02x %06x - %06x",
                        op_extra_data->lic.btaddr_start.nap, op_extra_data->lic.btaddr_start.uap,
                        op_extra_data->lic.btaddr_start.lap, op_extra_data->lic.btaddr_end.lap );
        }

        if (length > 6)
        {
            /* The remainder is the actual license key in this example */
            op_extra_data->lic.license_key_length = length - 6;          /* 6 octets for the BT ADDR range */
            op_extra_data->lic.license_key        = xzpmalloc(op_extra_data->lic.license_key_length);
            if (op_extra_data->lic.license_key != NULL)
            {
                memcpy(op_extra_data->lic.license_key, &data[6], op_extra_data->lic.license_key_length * sizeof(data[0]));
                op_extra_data->lic.state |= (1<<license_state_ps_key_stored);

                L2_DBG_MSG1("[2c] License key response. License [0]=%04x",
                            op_extra_data->lic.license_key[0]);

                /* OK, got the license. Now ask apps for the device's BT Address */
                ask_btaddr_via_unsolicited_message(op_data);

                return(TRUE);
            }
            L2_DBG_MSG("[2d] License key response. Out of memory trying to allocate for license key.");
        }
        else
        {
            L2_DBG_MSG("[2e] License key response. Expected more than 6 octets for License key.");
        }
    }

    return FALSE;
}

/* [3] Ask BT ADDR using message-from-operator ------------------------------------- */

/* Send a message-from-operator to apps/host. This must use the BT ADDR operator
 * message ID, OPMSG_FRAMEWORK_GET_BDADDR.
 */
static void ask_btaddr_via_unsolicited_message(OPERATOR_DATA *op_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA *op_extra_data = get_instance_data(op_data);
    unsigned num_params = 1;
    unsigned params[1];

    params[0] = base_op_get_cap_id(op_data);
    op_extra_data->lic.state |= (1<<license_state_btaddr_requested);

    common_send_unsolicited_message(op_data, OPMSG_FRAMEWORK_GET_BDADDR, num_params, params);

    L2_DBG_MSG("[3] Request device BT Address");
}

/* [4] Receive reply from apps0 via operator message ------------------------------- */

/* This function is a callback for operator messages (host/apps -> audio) for
 * operator message ID OPMSG_FRAMEWORK_SET_BDADDR. The message will contain
 * the BT ADDR as determined by host/apps. If 'pskey_bdaddr' is used in
 * 'subsys1_config2.htf', the BT ADDR will be that, else it will be the fw
 * default. Use this to check the device's address with the license's range
 * of permitted BT addresses.
 *
 * This function must be added/listed in 'secure_basic_passthrough_opmsg_handler_table'
 * which is a list of callbacks for operator messages (from host/apps) for a given
 * operator message ID.
 */
bool secure_basic_passthrough_opmsg_receive_bt_addr(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA  *op_extra_data = get_instance_data(op_data);

    op_extra_data->lic.state |= (1<<license_state_btaddr_received);

    L2_DBG_MSG1("[4a] Device BT address response (%p)", message_data);

    if (message_data != NULL)
    {
        op_extra_data->lic.device_btaddr.nap =   OPMSG_FIELD_GET(message_data, OPMSG_BTADDR_OPERATOR_MESSAGE, BT_ADDR_NAP);
        op_extra_data->lic.device_btaddr.uap = ((OPMSG_FIELD_GET(message_data, OPMSG_BTADDR_OPERATOR_MESSAGE, BT_ADDR_UAP_LAP))>>8)&0xff;
        op_extra_data->lic.device_btaddr.lap =  (OPMSG_FIELD_GET(message_data, OPMSG_BTADDR_OPERATOR_MESSAGE, BT_ADDR_LAP))|
                                              (((OPMSG_FIELD_GET(message_data, OPMSG_BTADDR_OPERATOR_MESSAGE, BT_ADDR_UAP_LAP))&0xff)<<16);
        op_extra_data->lic.state |= (1<<license_state_btaddr_stored);
        L2_DBG_MSG3("[4b] Device BT address  %04x %02x %06x",
                    op_extra_data->lic.device_btaddr.nap,
                    op_extra_data->lic.device_btaddr.uap,
                    op_extra_data->lic.device_btaddr.lap );
    }

    /* Run the licese check once, and set a single bit if success.
     * This then allows a bit-check to verify license, if a full
     * license check is not wanted (e.g. for performance reasons).
     */
    if (secure_basic_passthrough_license_verified(op_data))
    {
        op_extra_data->lic.state |= (1<<license_state_passed);
        L2_DBG_MSG("[4c] License OK!");
    }
    else
    {
        L2_DBG_MSG("[4c] License check failed.");
    }

    return(TRUE);
}

/* [5] The license check ----------------------------------------------------------- */

/* This is an example License Key Checker/Verifier. It is a
 * simple string compare for demonstration purposes. Customers
 * are expected to replace this with a proper mathematically
 * secure License Key verification algorithm.
 */
static bool LicenseKeyVerified(SECURE_BASIC_PASSTHROUGH_OP_DATA *op_extra_data)
{
    /* This must match the ps key (0x600000 (==0xC000<<7)) in 'subsys3_psflash.htf'
     * where 0xC000 is this capability's ID. Adjust the ps key in 'subsys3_psflash.htf'
     * if the capability ID is different from 0xC000.
     */
    if ((op_extra_data->lic.license_key[0] == 0x1122) &&
        (op_extra_data->lic.license_key[1] == 0x3344) &&
        (op_extra_data->lic.license_key[2] == 0x5566)   )
    {
        return TRUE;
    }
    return FALSE;
}

/* Check whether BT ADDR in in BT ADDR range of license key */
static bool BTAddrInRange(SECURE_BASIC_PASSTHROUGH_OP_DATA *op_extra_data)
{
    /* Check that nap and uap are as required; in our case in this example we
     * require a single value for 'nap' and 'uap'. For 'lap' this example supports
     * a range of values. With the example values at [1], the licensed 'lap' range
     * would be 0xAF01...0xAFFF.
     */
    if ((op_extra_data->lic.btaddr_start.nap == op_extra_data->lic.device_btaddr.nap) &&
        (op_extra_data->lic.btaddr_start.uap == op_extra_data->lic.device_btaddr.uap) &&
        /* Now check that the range is right */
        (op_extra_data->lic.btaddr_start.lap <= op_extra_data->lic.device_btaddr.lap) &&
        (op_extra_data->lic.btaddr_end.lap   >  op_extra_data->lic.device_btaddr.lap)    )
    {
        return TRUE;
    }
    return FALSE;
}

/* Example 'full' license key verifier, where the license key is checked and
 * whether the device's BT Address is in the range of licensed addresses.
 */
bool secure_basic_passthrough_license_verified(OPERATOR_DATA *op_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA  *op_extra_data = get_instance_data(op_data);

    /* Verify first if we have received both the License Key (via ps read
     * key) and the BT Address (via an operator message from apps0).
     */
    if ((op_extra_data->lic.state & (1<<license_state_btaddr_stored)) &&
        (op_extra_data->lic.state & (1<<license_state_ps_key_stored))  )
    {
        /* Yes, both License Key and BT Address are present. */
        if ((LicenseKeyVerified(op_extra_data)) &&
            (BTAddrInRange(op_extra_data))         )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Example 'short' license key verifier. The 'full' example license verifier
 * is run once in 'secure_basic_passthrough_opmsg_receive_bt_addr' and a bit
 * is set if ok. This shortened license check just looks at this bit, or
 * rather, the complete license state including this bit.
 */
bool secure_basic_passthrough_license_ok(OPERATOR_DATA *op_data)
{
    SECURE_BASIC_PASSTHROUGH_OP_DATA  *op_extra_data = get_instance_data(op_data);

    if (op_extra_data->lic.state == license_state_ok)
    {
        return TRUE;
    }
    return FALSE;
}
#else /* LEGACY_LICENSING */

SECURE_BASIC_PASSTHROUGH_LICENSING *lic = NULL;

/****************************************************************************
Private Function Declarations
*/
static bool ask_btaddr_via_unsolicited_message(OPERATOR_DATA *op_data);
static bool secure_basic_passthrough_retrieve_license_callback(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length,  unsigned* data, STATUS_KYMERA status, uint16 extra_status_info);


/* [1] Request license key --------------------------------------------------------- */

/* First, request the license key. This is stored in a ps key. The example license
 * should be stored in 'subsys3_psflash.htf' in the ADK. The example key has a range
 * of BT Addresses for which the example license key is valid. A customer may or may
 * not limit its license to a BT Address range. The first 6 octets in the example
 * license are the BT Address range, octets 6,7,8 are the example license. Everything
 * is in plain text, a customer application is expected to add a cryptographically
 * secure method. The example license is:-
 *
 * # 0xC000   BT ADDR range from:   to:                 license key:
 * 0x600000 = [ 02 00 5b 00 01 af   02 00 5b 00 ff af   22 11 44 33 66 55 ]
 *
 * assuming a device pskey_bdaddr={0x00af01,0x5b,0x0002} in 'subsys1_config2.htf'.
 */
bool basic_passthrough_retrieve_license_req(OPERATOR_DATA *op_data, unsigned capid)
{
    PS_KEY_TYPE key;

    L2_DBG_MSG("[1] Request license key");

    lic = xzpmalloc(sizeof(SECURE_BASIC_PASSTHROUGH_LICENSING));
    if (NULL == lic)
    {
        return FALSE;
    }
    lic->state   = 0;
    lic->op_data = op_data;
#ifdef QDEBUG_VERIFY
    set_lic_debug(lic);
#endif

    /* The ucid is 'free' to use. This is usually used to distinguish different instances
     * of the same capability (operator). See CS-00304075-SP. If such functionality is not
     *required for a customer, the ucid can be used for something else.
     */
    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(capid&(~0x3F), capid&0x3F, OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read(op_data, key, PERSIST_ANY, secure_basic_passthrough_retrieve_license_callback);
    lic->state |= (1<<license_state_ps_key_requested);

    return TRUE;
}

/*
 * Release the allocated space for the license when destroying the operator.
 */
void basic_passthrough_release_license(void)
{
    if (lic->license_key != NULL)
    {
        pfree(lic->license_key);
        lic->license_key = NULL;
    }
    pfree(lic);
    lic = NULL;
}

/* [2] Callback -------------------------------------------------------------------- */

/* This callback is called as a reply to the ps key request in [1] above.
 * It contains the ps key with the user/customer license. In this example
 * this is just a plain-text license for demonstration purposes.
 *
 */
static bool secure_basic_passthrough_retrieve_license_callback(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status, uint16 extra_status_info)
{
    OPERATOR_DATA *op_data = (OPERATOR_DATA*) instance_data;

    L2_DBG_MSG1("[2a] License key response. Status 0x%x", status);

    lic->state |= (1<<license_state_ps_key_received);

    lic->key    = key;
    lic->rank   = rank;

    if (status == STATUS_OK)
    {
        /* In this example, the example license key uses a BT Address range in the first 6 octets */
        if (length > 5)
        {
            /* If customer has chosen to use license BT ADDR range, then read in BT ADDR range */
            lic->btaddr_start.nap = (data[0]&0xffff);               /* Non-significant 32..47 (0x0002) */
            lic->btaddr_start.uap = (data[1]&0xff);                 /* Upper address   24..31 (  0x5b) */
            lic->btaddr_start.lap =  data[2]|((data[1]&0xff00)<<8); /* Lower address   00..23 (0xaf01) */
            lic->btaddr_end.nap   = (data[3]&0xffff);               /* Non-significant 32..47 (0x0002) */
            lic->btaddr_end.uap   = (data[4]&0xff);                 /* Upper address   24..31 (  0x5b) */
            lic->btaddr_end.lap   =  data[5]|((data[4]&0xff00)<<8); /* Lower address   00..23 (0xafff) */

            L2_DBG_MSG4("[2b] License key response. BT address range %04x %02x %06x - %06x",
                        lic->btaddr_start.nap, lic->btaddr_start.uap,
                        lic->btaddr_start.lap, lic->btaddr_end.lap );
        }

        if (length > 6)
        {
            /* The remainder is the actual license key in this example */
            lic->license_key_length = length - 6;          /* 6 octets for the BT ADDR range */
            lic->license_key        = xzpmalloc(lic->license_key_length);
            if (lic->license_key != NULL)
            {
                memcpy(lic->license_key, &data[6], lic->license_key_length * sizeof(data[0]));
                lic->state |= (1<<license_state_ps_key_stored);

                L2_DBG_MSG1("[2c] License key response. License [0]=%04x",
                            lic->license_key[0]);

                /* OK, got the license. Now ask apps for the device's BT Address */
                ask_btaddr_via_unsolicited_message(op_data);

                return(TRUE);
            }
            L2_DBG_MSG("[2d] License key response. Out of memory trying to allocate for license key.");
        }
        else
        {
            L2_DBG_MSG("[2e] License key response. Expected more than 6 octets for License key.");
        }
    }

    return FALSE;
}

/* [3] Ask BT ADDR using message-from-operator ------------------------------------- */

static void secure_basic_passthrough_opmsg_receive_bt_addr(bool result, BD_ADDR_T btaddr, void *data);

//#define USE_SECURITY_FOR_OP    1

/* Send a message-from-operator to apps/host. This must use the BT ADDR operator
 * message ID, OPMSG_FRAMEWORK_GET_BDADDR.
 */
static bool ask_btaddr_via_unsolicited_message(OPERATOR_DATA *op_data)
{
    lic->state |= (1<<license_state_btaddr_requested);

#ifdef USE_SECURITY_FOR_OP
    /* When specifying a valid external operator ID, the BT Address operator 
     * message handler in the table (secure_basic_passthrough_opmsg_handler_table
     * in this example) will be called, but after the callback was called. */
    bool result = get_bt_address(INT_TO_EXT_OPID(op_data->id), secure_basic_passthrough_opmsg_receive_bt_addr, op_data);
#else
    /* When specifying an invalid external operator ID, just the callback will
     * be called upon receiving the BT Address. */
    bool result = get_bt_address(0, secure_basic_passthrough_opmsg_receive_bt_addr, op_data);
#endif
    return result;
}

/* [4] Receive reply from apps0 via operator message ------------------------------- */

/* This function is a callback for operator messages (host/apps -> audio) for
 * operator message ID OPMSG_FRAMEWORK_SET_BDADDR. The message will contain
 * the BT ADDR as determined by host/apps. If 'pskey_bdaddr' is used in
 * 'subsys1_config2.htf', the BT ADDR will be that, else it will be the fw
 * default. Use this to check the device's address with the license's range
 * of permitted BT addresses.
 *
 * This function must be added/listed in 'secure_basic_passthrough_opmsg_handler_table'
 * which is a list of callbacks for operator messages (from host/apps) for a given
 * operator message ID.
 */
static void secure_basic_passthrough_opmsg_receive_bt_addr(bool result, BD_ADDR_T btaddr, void *data)
{
    lic->state |= (1<<license_state_btaddr_received);

    if (result)
    {
        lic->device_btaddr.nap = btaddr.nap;
        lic->device_btaddr.uap = btaddr.uap;
        lic->device_btaddr.lap = btaddr.lap;
        lic->state |= (1<<license_state_btaddr_stored);

        /* Run the license check once, and set a single bit if success.
         * This then allows a bit-check to verify license, if a full
         * license check is not wanted (e.g. for performance reasons).
         */
        if (secure_basic_passthrough_license_verified(lic))
        {
            lic->state |= (1<<license_state_passed);
        }
    }
}

/* [5] The license check ----------------------------------------------------------- */

/* This is an example License Key Checker/Verifier. It is a
 * simple string compare for demonstration purposes. Customers
 * are expected to replace this with a proper mathematically
 * secure License Key verification algorithm.
 */
static bool LicenseKeyVerified(SECURE_BASIC_PASSTHROUGH_LICENSING *lic)
{
    /* This must match the ps key (0x600000 (==0xC000<<7)) in 'subsys3_psflash.htf'
     * where 0xC000 is this capability's ID. Adjust the ps key in 'subsys3_psflash.htf'
     * if the capability ID is different from 0xC000.
     */
    if ((lic->license_key[0] == 0x1122) &&
        (lic->license_key[1] == 0x3344) &&
        (lic->license_key[2] == 0x5566)   )
    {
        return TRUE;
    }
    return FALSE;
}

/* Check whether BT ADDR in in BT ADDR range of license key */
static bool BTAddrInRange(SECURE_BASIC_PASSTHROUGH_LICENSING *lic)
{
    /* Check that nap and uap are as required; in our case in this example we
     * require a single value for 'nap' and 'uap'. For 'lap' this example supports
     * a range of values. With the example values at [1], the licensed 'lap' range
     * would be 0xAF01...0xAFFF.
     */
    if ((lic->btaddr_start.nap == lic->device_btaddr.nap) &&
        (lic->btaddr_start.uap == lic->device_btaddr.uap) &&
        /* Now check that the range is right */
        (lic->btaddr_start.lap <= lic->device_btaddr.lap) &&
        (lic->btaddr_end.lap   >  lic->device_btaddr.lap)    )
    {
        return TRUE;
    }
    return FALSE;
}

/* Example 'full' license key verifier, where the license key is checked and
 * whether the device's BT Address is in the range of licensed addresses.
 */
bool secure_basic_passthrough_license_verified(SECURE_BASIC_PASSTHROUGH_LICENSING *lic)
{
    /* Verify first if we have received both the License Key (via ps read
     * key) and the BT Address (via an operator message from apps0).
     */
    if ((lic->state & (1<<license_state_btaddr_stored)) &&
        (lic->state & (1<<license_state_ps_key_stored))  )
    {
        /* Yes, both License Key and BT Address are present. */
        if ((LicenseKeyVerified(lic)) &&
            (BTAddrInRange(lic))         )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Example 'short' license key verifier. The 'full' example license verifier
 * is run once in 'secure_basic_passthrough_opmsg_receive_bt_addr' and a bit
 * is set if ok. This shortened license check just looks at this bit, or
 * rather, the complete license state including this bit.
 */
bool secure_basic_passthrough_license_ok(OPERATOR_DATA *op_data)
{
    /* Keep parameter to remain compatible with with LEGACY_LICENSING */
    /* so that the call to it in the .asm needs no change. */
    UNUSED(op_data);

    if (lic->state == license_state_ok)
    {
        return TRUE;
    }
    return FALSE;
}
#endif /* LEGACY_LICENSING */

