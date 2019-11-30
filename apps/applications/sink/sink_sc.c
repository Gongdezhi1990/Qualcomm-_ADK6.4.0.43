/****************************************************************************
Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_sc.c

DESCRIPTION
    BR/EDR SC functionality

NOTES

*/

#include "sink_sc.h"
#include "sink_main_task.h"
#include "sink_private_data.h"

#include <bdaddr.h>
#include <sink.h>
#include <config_store.h>

#ifdef ENABLE_BREDR_SC
#include "sink_secure_connection_config_def.h"
#ifdef DEBUG_SC
#define SC_DEBUG(x) DEBUG(x)
#else
#define SC_DEBUG(x) 
#endif

/* As per HFP1.7 Spec Section 6.2 If Secure Connections is used, the 
 * Authenticated Payload Timeout should be less than or equal to 10s.
 * APT timer = N* 10 msec. N is 1000 for  APT = 10000 msec or 10 seconds
 */
#define AUTHENTICATED_PAYLOAD_TIMEOUT_HFP_SC_MAX       1000

/* Secure Connections (Host Support) feature is Bit 3 of 
 * Table 3.3: Extended feature mask definition (page 1) as per Core Spec v4.2
 */
#define SECURE_CONNECTION_HOST_SUPPORT_FEATURE        (1<<3)


/*******************************************************************************
NAME
    sinkSCIsBREdrGetSecureConnectionMode
    
DESCRIPTION
    Check if BR/EDR Secure Connection Mode is Enabled 
    
RETURNS
    TRUE if BR/EDR Secure Connection Mode is Enabled, FALSE if not
*/
uint8 sinkSCIsBREdrGetSecureConnectionMode(void)
{   
    uint8 result = FALSE;
    sink_sec_readonly_config_def_t *ro_config = NULL;

    /* Read the SC configuration */
    if (configManagerGetReadOnlyConfig(SINK_SEC_READONLY_CONFIG_BLK_ID, (const void**)&ro_config))
    {
        result = ro_config->SecureConnectionMode;
        configManagerReleaseConfig(SINK_SEC_READONLY_CONFIG_BLK_ID);
    }

    return result;
}

/*******************************************************************************
NAME
    sinkSCGetBrEdrAuthenticationPaylodTO
    
DESCRIPTION
    Get BR/EDR Secure Connection Authentication Payload Time Out
    
RETURNS
    uint16
*/
uint16 sinkSCGetBrEdrAuthenticationPaylodTO(void)
{
    uint16 timeout = 0;
    sink_sec_readonly_config_def_t *ro_config = NULL;

    /* Read the SC configuration */
    if (configManagerGetReadOnlyConfig(SINK_SEC_READONLY_CONFIG_BLK_ID, (const void**)&ro_config))
    {
        timeout = ro_config->BrEdrAuthenticatedPayloadTO_s;
        configManagerReleaseConfig(SINK_SEC_READONLY_CONFIG_BLK_ID);
    }

    return timeout;
}

bool sinkSCSetBrEdrAuthenticationPaylodTO(uint16 timeout)
{
    sink_sec_readonly_config_def_t *ro_config = NULL;

    if (configManagerGetWriteableConfig(SINK_SEC_READONLY_CONFIG_BLK_ID, (void**)&ro_config, 0))
    {
        ro_config->BrEdrAuthenticatedPayloadTO_s = timeout;
        configManagerUpdateWriteableConfig(SINK_SEC_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME
    scWriteApt

DESCRIPTION
    Write the APT value for the BR/EDR link. The application writes the minimum 
    of the (configurable)default value or AUTHENTICATED_PAYLOAD_TIMEOUT_HFP_SC_MAX
    if APT value is more than AUTHENTICATED_PAYLOAD_TIMEOUT_HFP_SC_MAX in case
    of HFP. Different profile may have different APT requirement, currently 
    HFP1.7 Spec Section6.2 defines APT be less than or equal to 10s

PARAMETERS
    bd_addr     The Bluetooth address of the remote device.

RETURNS
    None
*/
void scWriteApt(const bdaddr* bd_addr)
{
    sink_attributes attributes;
    tp_bdaddr ag_addr;
    uint16 bredr_apt = sinkSCGetBrEdrAuthenticationPaylodTO();
    SC_DEBUG (("BREDR_SC : scWriteApt\n"));

    ag_addr.transport = TRANSPORT_BREDR_ACL;
    ag_addr.taddr.type = TYPED_BDADDR_PUBLIC;
    ag_addr.taddr.addr = *bd_addr;

    deviceManagerClearAttributes(&attributes);
    if(deviceManagerGetAttributes(&attributes, bd_addr))
    {
        if(attributes.mode == sink_secure_connection_mode)
        {
            if((attributes.profiles & (sink_hfp)) &&
                   bredr_apt > AUTHENTICATED_PAYLOAD_TIMEOUT_HFP_SC_MAX)
            {
                bredr_apt = AUTHENTICATED_PAYLOAD_TIMEOUT_HFP_SC_MAX;
            }
            /* Write the APT value to the controller for the link */
            ConnectionWriteAPT(&theSink.task, &ag_addr, bredr_apt, cl_apt_bluestack);
        }
    }
}

/****************************************************************************
NAME
    scSetHfpLinkMode

DESCRIPTION
    Informs hfp library about the link is secure.
    Notifies the app the HFP slc is secure.

PARAMETERS
    bd_addr     The Bluetooth address to match.
    mode         Secure connection mode.

RETURNS
    None
*/
void scSetHfpLinkMode(const bdaddr* bd_addr, sink_link_mode mode)
{
    hfp_link_priority priority;
    SC_DEBUG (("BREDR_SC : scSetHfpLinkMode\n"));

    if(mode == sink_secure_connection_mode)
    {
       priority = HfpLinkPriorityFromBdaddr(bd_addr);

       /* Inform the hfp library that the link is secure */
       HfpLinkSetLinkMode(priority, TRUE);

        /* Notify the app that hfp link is secure */
        MessageSend(&theSink.task, EventSysHfpSecureLink, 0);
    }
}

#ifdef TEST_SCOM
/****************************************************************************
NAME
     test_sinkHandleUserConfirmation

DESCRIPTION
     This function is test code used for user confirmation "Yes/No" using user 
     event when MITM is enabled to test SCOM. This is for Test purpose only.

PARAMETERS
    accept     User confirmation (accept or reject ) for SCOM pairing.

RETURNS
    None
*/
void test_sinkHandleUserConfirmation(bool accept)
{
    SC_DEBUG (("BREDR_SC : test_sinkHandleUserConfirmation is %d\n", accept));

    if(sinkDataGetAuthConfirmationFlag())
    {
        ConnectionSmUserConfirmationResponse(sinkDataGetSMConfirmationAddress(), accept);
    }
}
#endif /* TEST_SCOM*/

/****************************************************************************
NAME
     isLinkSecure

DESCRIPTION
     This function is called to check if the given hfp link is secure or not.

PARAMETERS
    priority    Priority of the remote device.

RETURNS
     TRUE if BR/EDR link is Secure, FALSE if not
*/
bool  isLinkSecure(hfp_link_priority priority)
{
    bdaddr ag_addr;
    sink_attributes attributes;
    SC_DEBUG (("BREDR_SC : isLinkSecure for HFP link %d\n", priority));

    deviceManagerClearAttributes(&attributes);

    if(HfpLinkGetBdaddr(priority, &ag_addr))
    {
       if(deviceManagerGetAttributes(&attributes, &ag_addr))
       {
          return ((attributes.mode == sink_secure_connection_mode)? TRUE: FALSE);
       }
    }
    return FALSE;
}

/*******************************************************************************
NAME
    sinkHandleBrEdrEncryptionChange

DESCRIPTION
    Handles encryption change Indication on BR/EDR Link when encryption is 
    initiated by local device and confirm is received or when encryption change
    indication is received as a result of encryption initiated by remote device

PARAMETERS
    tp_addr     The Bluetooth address of the remote device.
    encrypt_type Encryption key type

RETURNS
    None
*/
static void sinkHandleBrEdrEncryptionChange(const typed_bdaddr *tp_addr, cl_sm_encryption_key_type encrypt_type)
{
    bool local_sc_supported = sinkSCIsBREdrGetSecureConnectionMode() > 0? TRUE: FALSE;
    SC_DEBUG (("BREDR_SC : sinkHandleBrEdrEncryptionChange\n"));

    if(local_sc_supported)
    {
        sink_attributes attributes;
        deviceManagerClearAttributes(&attributes);
        deviceManagerGetAttributes(&attributes,&tp_addr->addr);

        if(attributes.mode == sink_mode_unknown)
        {
            /* Get the encryption type of BR/EDR link
            * if cl_sm_encryption_e0_brdedr_aes_ccm_le  then it is  non-SC  pairing
            * if cl_sm_encryption_aes_ccm_bredr then it is SC pairing. Store this
            * information to trigger APT write after HFP SLC for subsequent reconnection
            */
            attributes.mode = (encrypt_type == cl_sm_encryption_aes_ccm_bredr) ?
                             sink_secure_connection_mode: sink_no_secure_connection;

            /* Update the device attributes */
            deviceManagerStoreAttributes(&attributes, &tp_addr->addr);
        }
        /*  Write the APT on SC link */
        if(attributes.mode == sink_secure_connection_mode)
        {
            /* Write the APT value for the BR/EDR link */
            scWriteApt(&tp_addr->addr);
        }
    }
}

/*******************************************************************************
NAME
    sinkHandleBrEdrEncryptionCfm

DESCRIPTION
    Handles encryption change Indication on BR/EDR Link when encryption is 
    initiated by local device and confirm is received.

PARAMETERS
    cfm     Pointer to encrypt confirmation message.

RETURNS
    None
*/
void sinkHandleBrEdrEncryptionCfm(const CL_SM_ENCRYPT_CFM_T* cfm)
{
    tp_bdaddr tpaddr;
    SC_DEBUG (("BREDR_SC : sinkHandleBrEdrEncryptionCfm status -%d encrypted - %d\n", cfm->status, cfm->encrypted));

    if(cfm->status == success && cfm->encrypted)
    {
        /* Get the remote bluetooth address from the sink */
        SinkGetBdAddr(cfm->sink, &tpaddr);

        sinkHandleBrEdrEncryptionChange(&tpaddr.taddr, cfm->encrypt_type);
    }
}

/*******************************************************************************
NAME
    sinkHandleBrEdrEncryptionChangeInd

DESCRIPTION
    Handles encryption change Indication on BR/EDR Link when encryption change
    indication is received as a result of encryption initiated by remote device

PARAMETERS
    ind     Pointer to encryption change indication message.

RETURNS
    None
*/
void sinkHandleBrEdrEncryptionChangeInd(const CL_SM_ENCRYPTION_CHANGE_IND_T* ind)
{
    SC_DEBUG (("BREDR_SC : sinkHandleBrEdrEncryptionChangeInd  encrypted - %d transport - %d\n", ind->encrypted, ind->tpaddr.transport));

    if(ind->encrypted && ind->tpaddr.transport == TRANSPORT_BREDR_ACL)
    {
        /* if the link is encrypted and transport is TRANSPORT_BREDR_ACL,
        * then check  encryption type to know if the BR/EDR link is SC or not.
        */
        sinkHandleBrEdrEncryptionChange(&ind->tpaddr.taddr, ind->encrypt_type);
    }
}
#endif /* ENABLE_BREDR_SC */

