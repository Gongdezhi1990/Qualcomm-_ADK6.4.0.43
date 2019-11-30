/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup third_party_security
 * \file  third_party_security.h
 * \ingroup security
 *
 * Third party security header file. <br>
 *
 */

#ifndef _THIRD_PARTY_SECURITY_H_
#define _THIRD_PARTY_SECURITY_H_
/*****************************************************************************
Include Files
*/
#include "opmgr/opmgr_for_ops.h"

/****************************************************************************
Public Constant Definitions
*/
#define INSTALL_LICENSE_CHECK                                  1

/****************************************************************************
Public Constant Definitions
*/

/****************************************************************************
Public Type Declarations
*/

/*****************************************************************************
Private Function Definitions
*/

#ifdef INSTALL_LICENSE_CHECK
typedef struct 
{
    uint24   lap;   /* Lower address   00..23 (0x5a5a) */
    uint8    uap;   /* Upper address   24..31 (  0x5b) */
    uint16   nap;   /* Non-significant 32..47 (0x0002) */
} BD_ADDR_T;

typedef void (*TPSECLIB_CALLBACK)(bool status, BD_ADDR_T btaddr, void *data);

/**
 * \brief   Request the BT Address from apps by using a (operator) message.
 *
 *          This function is intended to be used in a customer application
 *          (or example) that uses/requires third party licencing.
 *
 * \param   op_id        - Operator ID (may be 0).
 * \param   callback     - Caller's callback function
 * \param   data         - Caller's data pointer to call callback with
 *
 * \return  FALSE if no message was sent
 *          TRUE if message was sent successfully
 */
extern bool get_bt_address(unsigned ext_op_id, TPSECLIB_CALLBACK callback, void *data);

/**
 * \brief   Store pointer to (third party licensing) object with
 *          status information for (in-house) testing purposes.
 *
 * \param   ptr          - Pointer to store in TEST_REG_3
 */
extern void set_lic_debug(void *ptr);
#endif /* INSTALL_LICENSE_CHECK */

#endif /* _THIRD_PARTY_SECURITY_H_ */

