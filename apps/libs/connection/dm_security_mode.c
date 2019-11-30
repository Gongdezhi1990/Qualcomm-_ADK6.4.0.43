/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_mode.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include    <message.h>
#include    <string.h>
#include    <vm.h>


/*****************************************************************************/
void ConnectionSmSetSecurityMode(Task theAppTask, dm_security_mode sec_mode, encryption_mode enc_mode)
{
    /* Check params are within allowed values - debug build only */
#ifdef CONNECTION_DEBUG_LIB
    if (sec_mode > sec_mode4_ssp)
    {
        CL_DEBUG(("Out of range security mode 0x%x\n", sec_mode));
    }

    if (enc_mode > hci_enc_mode_pt_to_pt_and_bcast)
    {
        CL_DEBUG(("Out of range encyption mode 0x%x\n", enc_mode));
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_SET_SC_MODE_REQ);
        message->theAppTask = theAppTask;
        message->mode = sec_mode;
        message->mode3_enc = enc_mode;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_SET_SC_MODE_REQ, message);
    }
}


/*****************************************************************************/
void ConnectionSmSecModeConfig(Task theAppTask, cl_sm_wae write_auth_enable, bool debug_keys, bool legacy_auto_pair_key_missing)
{
#ifdef CONNECTION_DEBUG_LIB
    if (write_auth_enable > cl_sm_wae_always)
    {
        CL_DEBUG(("Out of range wae 0x%x\n", write_auth_enable));
    }
#endif
    {
    MAKE_CL_MESSAGE(CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ);
    message->theAppTask = theAppTask;
    message->write_auth_enable = write_auth_enable;
    message->debug_keys = debug_keys;
        message->legacy_auto_pair_key_missing = legacy_auto_pair_key_missing;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ, message);
    }
}


