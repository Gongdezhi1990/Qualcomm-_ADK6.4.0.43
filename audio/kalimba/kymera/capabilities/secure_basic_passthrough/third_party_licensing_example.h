/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup secure_basic_passthrough
 * \file  secure_basic_passthrough_struct.h
 * \ingroup capabilities
 *
 * Secure basic passthrough operator header file containing type definitions
 * shared between C and asm. <br>
 *
 */

#ifndef _THIRD_PARTY_LICENSING_EXAMPLE_H_
#define _THIRD_PARTY_LICENSING_EXAMPLE_H_
/*****************************************************************************
Include Files
*/
#include "buffer/cbuffer_c.h"
#include "audio_fadeout.h"
#include "stream/stream_audio_data_format.h"
#include "secure_basic_passthrough_shared_const.h"
#include "ttp/ttp.h"
#include "op_msg_utilities.h"
#include "ps/ps.h"

#include "third_party_security/third_party_security.h"

/****************************************************************************
Public Constant Definitions
*/


/****************************************************************************
Public Type Declarations
*/

typedef enum 
{
    license_state_ps_key_requested = 0,
    license_state_ps_key_received  = 1,
    license_state_ps_key_stored    = 2,
    license_state_btaddr_requested = 3,
    license_state_btaddr_received  = 4,
    license_state_btaddr_stored    = 5,
    license_state_passed           = 6,
    license_state_ok               = (1<<license_state_ps_key_requested) | (1<<license_state_ps_key_received)  |
                                     (1<<license_state_ps_key_stored)    | (1<<license_state_btaddr_requested) |
                                     (1<<license_state_btaddr_received)  | (1<<license_state_btaddr_stored)    |
                                     (1<<license_state_passed),
} LICENSE_STATE;

typedef struct SECURE_BASIC_PASSTHROUGH_LICENSING
{
    unsigned         state;
    OPERATOR_DATA   *op_data;

    /* Licensed BT Address Range */
    BD_ADDR_T        btaddr_start;
    BD_ADDR_T        btaddr_end;

    /* Customer license */
    unsigned        *license_key;
    unsigned         license_key_length;
    PS_KEY_TYPE      key;
    PERSISTENCE_RANK rank;

    /* BT Address received via operator message from apps. */
    BD_ADDR_T        device_btaddr;

} SECURE_BASIC_PASSTHROUGH_LICENSING;

typedef struct
{
    OPMSG_HEADER hdr;
    unsigned     data[];

} OPMSG_BT_ADDR;

#ifdef LEGACY_LICENSING
/* Setup and start license check */
extern void basic_passthrough_retrieve_license_req(OPERATOR_DATA *op_data);
/* Full license check */
extern bool secure_basic_passthrough_license_verified(OPERATOR_DATA *op_data);
/* Quick license check, use bit set by previous full license check */
extern bool secure_basic_passthrough_license_ok(OPERATOR_DATA *op_data);
#else
/* Setup and start license check */
extern bool basic_passthrough_retrieve_license_req(OPERATOR_DATA *op_data, unsigned capid);
/* Full license check */
extern bool secure_basic_passthrough_license_verified(SECURE_BASIC_PASSTHROUGH_LICENSING *lic);
/* Quick license check, use bit set by previous full license check */
extern bool secure_basic_passthrough_license_ok(OPERATOR_DATA *op_data);
/* Release and free license key */
void basic_passthrough_release_license(void);
#endif

#endif /* _THIRD_PARTY_LICENSING_EXAMPLE_ */

