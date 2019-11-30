/*
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief
    This file contains the Authentication functionality for the Sink 
    Application
*/

/****************************************************************************
    Header files
*/
#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_audio_prompts.h"
#include "sink_statemanager.h"
#include "sink_auth.h"
#include "sink_ba.h"
#include "sink_malloc_debug.h"

#include "sink_devicemanager.h"
#include "sink_debug.h"
#include "sink_ble_gap.h"
#include "sink_ba_ble_gap.h"
#include "sink_audio_indication.h"
#include "sink_sc.h"
#include "sink_bredr.h"
#include "sink_scan.h"

#ifdef ENABLE_PARTYMODE
#include "sink_partymode.h"
#endif

#include "sink_nfc.h"

#ifdef ENABLE_SUBWOOFER
#include "sink_swat.h"
#endif

#ifdef ENABLE_FAST_PAIR
#include "sink_fast_pair.h"
#endif

#include <ps.h>
#include <bdaddr.h>
#include <stdlib.h>
#include <sink.h>
#include <vmtypes.h>

bool pairing_enabled = FALSE;

#ifdef DEBUG_AUTH
    #define AUTH_DEBUG(x) DEBUG(x)    
#else
    #define AUTH_DEBUG(x) 
#endif
/* 
    all key information except CSRK is requested as SM initiator and provided as responded during key distribution
*/
#define KEY_DISTRIBUTION (KEY_DIST_RESPONDER_ENC_CENTRAL | KEY_DIST_RESPONDER_ID | KEY_DIST_INITIATOR_ENC_CENTRAL | KEY_DIST_INITIATOR_ID)

#ifdef ENABLE_PEER

    /* PERSISTENT STORE KEY
     *
     * Persistent store keys are held in terms of uint16's
     *
     * sizeof() works on the "byte" type of the processor, which was
     * also a uint16 on the XAP processor.
     *
     * Other processors will have a different allocation size
     */
#define BD_ADDR_SIZE (PS_SIZE_ADJ(sizeof(bdaddr)))
#define LINK_KEY_SIZE 8
#define ATTRIBUTES_SIZE (PS_SIZE_ADJ(sizeof(sink_attributes)))
#define STATUS_LOC 0
#define BD_ADDR_LOC 1
#define LINK_KEY_LOC (BD_ADDR_LOC+BD_ADDR_SIZE)
#define ATTRIBUTES_LOC (LINK_KEY_LOC+LINK_KEY_SIZE)
#define PERMPAIR_PS_SIZE (BD_ADDR_SIZE + LINK_KEY_SIZE + ATTRIBUTES_SIZE + 1)

/****************************************************************************/
static void printBdAddrAndAttributes(uint16 *ps_key_buffer, const char *msg_text)
{
#ifdef DEBUG_DEV

    bdaddr perm_addr;
    sink_attributes perm_attributes;

    memcpy(&perm_addr, &ps_key_buffer[BD_ADDR_LOC], sizeof(perm_addr));
    memcpy(&perm_attributes, &ps_key_buffer[ATTRIBUTES_LOC], sizeof(perm_attributes));

    DEBUG(("DEV: perm %s %04X %02X %06lX prof:0x%02X route:%u,%u\n",
           msg_text,
           perm_addr.nap,
           perm_addr.uap,
           perm_addr.lap,
           perm_attributes.profiles,
           perm_attributes.master_routing_mode,
           perm_attributes.slave_routing_mode));
#else
    UNUSED(ps_key_buffer);
    UNUSED(msg_text);
#endif
}

/****************************************************************************/
static void readPsPermanentPairing (bdaddr *bd_addr, uint16 *link_key, uint16 *link_key_status, sink_attributes *attributes)
{
    uint16 * ps_key;

    /* Allocate and zero buffer to hold PS key */
    ps_key = (uint16*)callocDebugPanic(PERMPAIR_PS_SIZE,sizeof(uint16));

    /* Attempt to obtain current pairing data */
    PsRetrieve(CONFIG_PERMANENT_PAIRING, ps_key, PERMPAIR_PS_SIZE);

    /* Return any requested fields */
    if (link_key_status)
    {
        *link_key_status = ps_key[STATUS_LOC];
    }

    if (bd_addr)
    {
        memcpy(bd_addr, &ps_key[BD_ADDR_LOC], sizeof(*bd_addr));
    }

    if (link_key)
    {
        memcpy(link_key, &ps_key[LINK_KEY_LOC], LINK_KEY_SIZE * sizeof(uint16));
    }

    if (attributes)
    {
        memcpy(attributes, &ps_key[ATTRIBUTES_LOC], sizeof(*attributes));
    }
    
    printBdAddrAndAttributes(ps_key, "read");

    free(ps_key);
}

/****************************************************************************/
static void writePsPermanentPairing (const bdaddr *bd_addr, const uint16 *link_key, uint16 link_key_status, const sink_attributes *attributes)
{
    uint16 * ps_key;

    /* Allocate and zero buffer to hold PS key */
    ps_key = (uint16*)callocDebugPanic(PERMPAIR_PS_SIZE,sizeof(uint16));

    /* Attempt to obtain current pairing data */
    PsRetrieve(CONFIG_PERMANENT_PAIRING, ps_key, PERMPAIR_PS_SIZE);
    
    /* Update supplied fields */
    if (link_key_status)
    {
        ps_key[STATUS_LOC] = link_key_status;
    }

    if (bd_addr)
    {
        memcpy(&ps_key[BD_ADDR_LOC], bd_addr, sizeof(*bd_addr));
    }

    if (link_key)
    {
        memcpy(&ps_key[LINK_KEY_LOC], link_key, LINK_KEY_SIZE * sizeof(uint16));
    }

    if (attributes)
    {
        memcpy(&ps_key[ATTRIBUTES_LOC], attributes, sizeof(*attributes));
    }

    printBdAddrAndAttributes(ps_key, "write");

    /* Store updated pairing data */
    PsStore(CONFIG_PERMANENT_PAIRING, ps_key, PERMPAIR_PS_SIZE);

    free(ps_key);
}

/****************************************************************************/
void AuthInitPermanentPairing (void)
{
    bdaddr ps_bd_addr;
    
    AUTH_DEBUG(("AuthInitPermanentPairing\n"));
    
    /* Obtain just the bluetooth address of a permanently paired device */
    readPsPermanentPairing(&ps_bd_addr, 0, 0, 0);
    AUTH_DEBUG(("   ps bd_addr = [%x:%x:%lx]\n", ps_bd_addr.uap, ps_bd_addr.nap, ps_bd_addr.lap));
    
    if ( !BdaddrIsZero(&ps_bd_addr) )
    {   /* A valid address has been obtained, ask CL for info on it */
        ConnectionSmGetAuthDevice(&theSink.task, (const bdaddr *)&ps_bd_addr);
    }
}

/****************************************************************************/
void AuthRemovePermanentPairing (bool erase_ps_key)
{
    bdaddr ps_bd_addr;
    
    AUTH_DEBUG(("AuthRemovePermanentPairing  erase_ps_key = %u\n", erase_ps_key));
    
    readPsPermanentPairing(&ps_bd_addr, 0, 0, 0);
    AUTH_DEBUG(("   ps bd_addr = [%x:%x:%lx]\n", ps_bd_addr.uap, ps_bd_addr.nap, ps_bd_addr.lap));
    
    if ( !BdaddrIsZero(&ps_bd_addr) )
    {
        ConnectionSmDeleteAuthDeviceReq(TYPED_BDADDR_PUBLIC, (const bdaddr *)&ps_bd_addr);
    }
    
    if ( erase_ps_key )
    {
        PsStore(CONFIG_PERMANENT_PAIRING, 0, 0);
    }
}

/****************************************************************************/
void AuthUpdatePermanentPairing (const bdaddr *bd_addr, const sink_attributes *attributes)
{
    bdaddr ps_bdaddr;
    AUTH_DEBUG(("AuthUpdatePermanentPairing\n"));
    
    readPsPermanentPairing(&ps_bdaddr, 0, 0, 0);

    if(!BdaddrIsZero(&ps_bdaddr) && !BdaddrIsSame(&ps_bdaddr, bd_addr))
    {
        AuthRemovePermanentPairing(FALSE);
    }
    
    /* Update permanent pairing info */
    writePsPermanentPairing(0, 0, 0, attributes);
    
    ConnectionSmGetAuthDevice(&theSink.task, bd_addr);
}

/****************************************************************************/
void handleGetAuthDeviceCfm (const CL_SM_GET_AUTH_DEVICE_CFM_T *cfm)
{
    AUTH_DEBUG(("handleGetAuthDeviceCfm\n"));
    AUTH_DEBUG(("   status = %u\n",cfm->status));
    AUTH_DEBUG(("   ps bd_addr = [%x:%x:%lx]\n", cfm->bd_addr.uap, cfm->bd_addr.nap, cfm->bd_addr.lap));
    AUTH_DEBUG(("   trusted = %u\n",cfm->trusted));
    AUTH_DEBUG(("   link key type = %u",cfm->link_key_type));
    AUTH_DEBUG(("   link key size = %u\n",cfm->size_link_key));
    
    if ( cfm->status == success )
    {   /* Device exists in CL PDL */
        sink_attributes attributes;
        uint16 link_key_status = ((cfm->trusted & 0xF)<<8) | ((cfm->link_key_type & 0xF)<<4) | (cfm->size_link_key & 0xF);

        if (!deviceManagerGetAttributes(&attributes, &cfm->bd_addr))
        {
            bdaddr perm_bdaddr;

            /* No attributes in PDL, so check if attributes for this bdaddr
               are stored in the permanent pairing data.
               If not, revert to defaults. */
            readPsPermanentPairing(&perm_bdaddr, 0, 0, &attributes);
            if (BdaddrIsZero(&perm_bdaddr) || !BdaddrIsSame(&perm_bdaddr, &cfm->bd_addr))
            {
                deviceManagerGetDefaultAttributes(&attributes, dev_type_none);
            }
        }

        /* Update permanent pairing info */
        writePsPermanentPairing(&cfm->bd_addr, cfm->link_key, link_key_status, &attributes);
        
        /* Update attributes */
        deviceManagerStoreAttributes(&attributes, (const bdaddr *)&cfm->bd_addr);
        
        /* Mark the device as trusted and push it to the top of the PDL */
        ConnectionSmUpdateMruDevice((const bdaddr *)&cfm->bd_addr); 
        
        deviceManagerUpdatePriorityDevices();
    }
    else
    {   /* Device *does not* exist in CL PDL */ 
        bdaddr ps_bd_addr;
        uint16 ps_link_key_status;
        uint16 ps_link_key[LINK_KEY_SIZE];
    
        readPsPermanentPairing(&ps_bd_addr, ps_link_key, &ps_link_key_status, 0);
    
        if ( !BdaddrIsZero(&ps_bd_addr) )
        {   /* We have permanently paired device, add it to CL PDL */
            uint16 trusted = ((ps_link_key_status>>8) & 0xF);
            cl_sm_link_key_type key_type = (cl_sm_link_key_type)((ps_link_key_status>>4) & 0xF);
            uint16 size_link_key = ps_link_key_status & 0xF;
        
            ConnectionSmAddAuthDevice(&theSink.task, (const bdaddr *)&ps_bd_addr, trusted, TRUE, key_type, size_link_key, (const uint16 *)ps_link_key);
        }
    }
 }

/****************************************************************************/
void handleAddAuthDeviceCfm (const CL_SM_ADD_AUTH_DEVICE_CFM_T *cfm)
{
    if ( cfm->status == success )
    {   /* Ask for device info again to allow write of attribute data */
        ConnectionSmGetAuthDevice(&theSink.task, &cfm->bd_addr);  
    }
}
#endif  /* ENABLE_PEER */

/****************************************************************************/
static bool AuthCanSinkConnect ( const bdaddr * bd_addr );

/****************************************************************************/
static bool AuthCanSinkPair(TRANSPORT_T transport, bool sm_over_bredr) ;

/****************************************************************************/
void sinkHandlePinCodeInd(const CL_SM_PIN_CODE_IND_T* ind)
{
    uint16  pin_length = 0;
    uint16  pin[16];
    void   *pin_to_play = pin;

    if ( AuthCanSinkPair(TRANSPORT_BREDR_ACL, FALSE) )
    {

        AUTH_DEBUG(("auth: Can Pin\n")) ;
        
#ifdef HYDRACORE
        /* The PIN is specified here as there is currently no PSKEY_FIXED_PIN equivalent */
        uint8 packed_pin[16] = "0000";
        
        pin_length = 4;
        
        pin_to_play = packed_pin;
#else
        /* Do we have a fixed pin in PS, if not reject pairing */
        if ((pin_length = PsFullRetrieve(PSKEY_FIXED_PIN, pin, 16)) == 0 || pin_length > 16)
        {
            /* Set length to 0 indicating we're rejecting the PIN request */
            AUTH_DEBUG(("auth : failed to get pin\n")) ;
            pin_length = 0; 
        }
        else 
#endif
        if(SinkAudioPromptsGetVoicePromptPairing())
        {
            sinkAudioIndicationPlayEvent(EventSysPinCodeRequest);
            AudioPromptPlayNumString(pin_length, (uint8 *)pin_to_play);
        }
    } 

    /* Respond to the PIN code request */
    ConnectionSmPinCodeResponse(&ind->taddr, (uint8)pin_length, (uint8*)pin_to_play); 
}

/****************************************************************************/
void sinkHandleUserConfirmationInd(const CL_SM_USER_CONFIRMATION_REQ_IND_T* ind)
{
    tp_bdaddr  *confirmation_addr;

    bool can_pair = AuthCanSinkPair(ind->tpaddr.transport, FALSE);

    /* Can we pair? */
#ifdef ENABLE_FAST_PAIR
    if ( can_pair && (sinkDataIsMITMRequired() || sinkFastPairIsInProgress()))
#else
    if ( can_pair && sinkDataIsMITMRequired())
#endif
    {
        sinkDataSetAuthConfirmationFlag(TRUE);
        AUTH_DEBUG(("auth: Can Confirm %ld\n",ind->numeric_value)) ;
        /* Should use text to speech here */
        confirmation_addr  = newDebugPanic(tp_bdaddr);
        *confirmation_addr = ind->tpaddr;
        sinkDataSetSMConfirmationAddress(confirmation_addr);
#ifdef ENABLE_FAST_PAIR
        if (sinkFastPairIsInProgress())
        {
            sinkFastPairHandleProviderPasskey(ind->numeric_value);
        }
#endif
        if(SinkAudioPromptsGetVoicePromptPairing())
        {
            sinkAudioIndicationPlayEvent(EventSysConfirmationRequest);
            AudioPromptPlayNumber(ind->numeric_value);
        }
    }
    else
    {
        /* Reject the Confirmation request */
        AUTH_DEBUG(("auth: Rejected Confirmation Req\n")) ;
        ConnectionSmUserConfirmationResponse(&ind->tpaddr, FALSE);
    }
}

/****************************************************************************/
void sinkHandleUserPasskeyInd(const CL_SM_USER_PASSKEY_REQ_IND_T* ind)
{
    /* Reject the Passkey request */
    AUTH_DEBUG(("auth: Rejected Passkey Req\n")) ;
    ConnectionSmUserPasskeyResponse(&ind->tpaddr, TRUE, 0);
}

/****************************************************************************/
void sinkHandleUserPasskeyNotificationInd(const CL_SM_USER_PASSKEY_NOTIFICATION_IND_T* ind)
{
    AUTH_DEBUG(("Passkey: %ld \n", ind->passkey));
#ifdef ENABLE_FAST_PAIR
    if((sinkDataIsMITMRequired() || sinkFastPairIsInProgress())&& SinkAudioPromptsGetVoicePromptPairing())
#else
    if(sinkDataIsMITMRequired() && SinkAudioPromptsGetVoicePromptPairing())
#endif
    {
        sinkAudioIndicationPlayEvent(EventSysPasskeyDisplay);
        AudioPromptPlayNumber(ind->passkey);
    }
    /* Should use text to speech here */
}
/****************************************************************************/
void sinkHandleIoCapabilityInd(const CL_SM_IO_CAPABILITY_REQ_IND_T* ind)
{
    bool can_pair = AuthCanSinkPair(ind->tpaddr.transport, ind->sm_over_bredr);
    bool bonding = TRUE; /* By default we want to bond */
    uint16 key_dist_type = KEY_DISTRIBUTION;

#ifdef ENABLE_FAST_PAIR
    /* End pairing if remote device has set i/o capabilities to No Input No Output when Fast pairing is in progress */
    if(sinkFastPairIsInProgress())
    {
        /* Need to stop the FastPair timer as we now know that the pairing procedure 
            has been started and FP is in progress*/
        sinkFastPairStopTimer();
        if(!sinkFastPairGetStatus())
        {
            can_pair = FALSE;
        }
    }
#endif
    /* If not pairable should reject */
    if(can_pair)
    {
#ifdef ENABLE_FAST_PAIR
        cl_sm_io_capability local_io_capability = (sinkDataIsMITMRequired() || sinkFastPairIsInProgress()) ? cl_sm_io_cap_display_yes_no : cl_sm_io_cap_no_input_no_output;
#else
        cl_sm_io_capability local_io_capability = sinkDataIsMITMRequired() ? cl_sm_io_cap_display_yes_no : cl_sm_io_cap_no_input_no_output;
#endif
#ifdef ENABLE_FAST_PAIR
        mitm_setting sink_mitm_setting = (sinkDataIsMITMRequired() || sinkFastPairIsInProgress()) ? mitm_required : mitm_not_required;
#else
        mitm_setting sink_mitm_setting = sinkDataIsMITMRequired() ? mitm_required : mitm_not_required;
#endif

        /* Bonding only if in BA association state and remote BD address is random resolavable or BR-EDR device wants to pair BA device*/
        if(sinkBroadcastAudioIsActive() && gapBaGetAssociationInProgress())
        {
            AUTH_DEBUG(("auth: Sending IO Capability in case of BA Association \n"));
#ifndef ENABLE_RANDOM_ADDR
            if(ind->tpaddr.taddr.type == TYPED_BDADDR_PUBLIC)
            {
                if(ind->tpaddr.transport == TRANSPORT_BLE_ACL )
				{
                    /* This could happen if both the parties involved are public address holders*/
                    bonding = FALSE;
				}
                /* When both the parties involved in association using public address, no need of any key distribution */
                key_dist_type = KEY_DIST_NONE;
            }
            
#endif
       }
        AUTH_DEBUG(("auth: Sending IO Capability \n"));
        /* Send Response and request to bond with device */
        ConnectionSmIoCapabilityResponse(&ind->tpaddr, local_io_capability, sink_mitm_setting, bonding, key_dist_type, oob_data_none, NULL, NULL);
    }
    else
    {
        AUTH_DEBUG(("auth: Rejecting IO Capability Req \n"));
        ConnectionSmIoCapabilityResponse(&ind->tpaddr, cl_sm_reject_request, mitm_not_required, FALSE, KEY_DIST_NONE, oob_data_none, NULL, NULL);
    }
}

/****************************************************************************/
void sinkHandleRemoteIoCapabilityInd(const CL_SM_REMOTE_IO_CAPABILITY_IND_T* ind)
{
#ifdef ENABLE_FAST_PAIR
    if(sinkFastPairIsInProgress())
    {
        /* Pairing procedure should not proceed if remote device has set io cap
           as no input/no output to avoid using just works */
        if(ind->io_capability == cl_sm_io_cap_no_input_no_output)
        {
            sinkFastPairSetStatus(fast_pair_io_cap_failure);
        }
    }
#else
    UNUSED(ind);
#endif
    AUTH_DEBUG(("auth: Incoming Authentication Request\n"));
}

/****************************************************************************/
void sinkHandleAuthoriseInd(const CL_SM_AUTHORISE_IND_T *ind)
{
    
    bool lAuthorised = FALSE ;
    
    if ( AuthCanSinkConnect(&ind->bd_addr) )
    {
        lAuthorised = TRUE ;
    }
    
    AUTH_DEBUG(("auth: Authorised [%d]\n" , lAuthorised)) ;
        
    /*complete the authentication with the authorised or not flag*/
    ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, lAuthorised);
}

/****************************************************************************/
void sinkHandleAuthenticateCfm(const CL_SM_AUTHENTICATE_CFM_T *cfm)
{
#ifdef ENABLE_SUBWOOFER
    if (sinkInquiryCheckInqActionSub())
    {
        if ((cfm->status == auth_status_success) && (cfm->bonded))
        {           
            /* Mark the subwoofer as a trusted device */
            deviceManagerMarkTrusted(&cfm->bd_addr);

            /* Store the subwoofers BDADDR to PS */
            sinkSwatWriteSubwooferBdaddr(&cfm->bd_addr);

            /* Setup some default attributes for the subwoofer */
            deviceManagerStoreDefaultAttributes(&cfm->bd_addr, dev_type_sub);

            /* mark the subwoofer device as DO NOT DELETE in PDL */
            ConnectionAuthSetPriorityDevice(&cfm->bd_addr, TRUE);
        }
        return;
    }
#endif
    /* Leave bondable mode if successful unless we got a debug key */
    if (cfm->status == auth_status_success && cfm->key_type != cl_sm_link_key_debug)
    {
        if ((!sinkInquiryCheckInqActionRssi()) || (!sinkInquiryIsInqSessionNormal()))
        {
            /* Mark the device as trusted */
            deviceManagerMarkTrusted(&cfm->bd_addr);
            MessageSend (&theSink.task , EventSysPairingSuccessful , 0 );
            sinkNfcAuthenticationSuccessInd(&theSink.task);
        }
    }
    
    /* Set up some default params and shuffle PDL */
    if(cfm->bonded)
    {
        sink_attributes attributes;
        
        deviceManagerClearAttributes(&attributes);
        if(!deviceManagerGetAttributes(&attributes, &cfm->bd_addr))
        {
            deviceManagerGetDefaultAttributes(&attributes, dev_type_ag);
        }
        else
        {
            deviceManagerUpdateAttributesWithDeviceDefaults(&attributes, dev_type_ag);
        }

        /* Check if the key_type generated is p256. If yes then set the
        * attribute.mode to sink_mode_unknown. Once the encryption type is known in
        * CL_SM_ENCRYPTION_CHANGE_IND or  CL_SM_ENCRYPT_CFM message,device
        * attributes will be updated accordingly with proper mode.
        * Update the device attributes with this information to be reused later.
        */
        if((sinkSCIsBREdrGetSecureConnectionMode() > 0)  && 
               ((cfm->key_type == cl_sm_link_key_unauthenticated_p256) ||
               (cfm->key_type == cl_sm_link_key_authenticated_p256)))
        {
            attributes.mode = sink_mode_unknown;
        }
        else
        {
            attributes.mode = sink_no_secure_connection;
        }

        deviceManagerStoreAttributes(&attributes, &cfm->bd_addr);

        ConnectionAuthSetPriorityDevice((const bdaddr *)&cfm->bd_addr, FALSE);
    }
    
    /* Reset pairing info if we timed out on confirmation */
    AuthResetConfirmationFlags();
}

/****************************************************************************/
static bool AuthCanSinkPair(TRANSPORT_T transport, bool sm_over_bredr)
{
    if(transport == TRANSPORT_BREDR_ACL)
    {
        if(!sinkDataIsSecureParingEnabled())
            return TRUE;
        
        if(stateManagerGetState() == deviceConnDiscoverable)
            return TRUE;
        
        if(sinkInquiryCheckInqActionSub())
            return TRUE;
        
        if(sinkNfcIsSecondAGPairingAllowed())
            return TRUE;
        
#ifdef ENABLE_FAST_PAIR        
        if(sinkFastPairIsInProgress())
            return TRUE;
#endif

#ifdef ENABLE_PARTYMODE
        if(sinkPartymodeIsEnabled())
            return TRUE;
#endif
    }
    else if(transport == TRANSPORT_BLE_ACL)
    {
        if(sinkBleGapIsBondable())
            return TRUE;
        
        if(sm_over_bredr)
            return TRUE;
    }
    
    return FALSE;
}

/****************************************************************************/
static bool AuthCanSinkConnect ( const bdaddr * bd_addr )
{
    bool lCanConnect = FALSE ;
    uint8 NoOfDevices = deviceManagerNumConnectedDevs();
    
    /* if device is already connected via a different profile allow this next profile to connect */
    if(deviceManagerProfilesConnected(bd_addr))
    {
        AUTH_DEBUG(("auth: already connected, CanConnect = TRUE\n")) ;
        lCanConnect = TRUE;
    }
    /* this bdaddr is not already connected, therefore this is a new device, ensure it is allowed 
       to connect, if not reject it */
    else
    {
        /* when multipoint is turned off, only allow one device to connect */
        if(((!sinkDataIsMultipointEnabled())&&(!NoOfDevices))||
           ((sinkDataIsMultipointEnabled())&&(NoOfDevices < MAX_MULTIPOINT_CONNECTIONS)))
        {
            /* is secure pairing enabled? */
            if (sinkDataIsSecureParingEnabled())
            {
                /* If page scan is enabled (i.e. we are either connectable/discoverable or 
                connected in multi point) */
                if ( sinkIsConnectable() )
                {
                    lCanConnect = TRUE ;
                    AUTH_DEBUG(("auth: is connectable\n")) ;
                }        
            }
            /* no secure pairing */
            else
            {
                AUTH_DEBUG(("auth: MP CanConnect = TRUE\n")) ;
                lCanConnect = TRUE ;
            }
        }
    }
  
    AUTH_DEBUG(("auth:  CanConnect = %d\n",lCanConnect)) ;
  
    return lCanConnect ;
}

/****************************************************************************/
void sinkPairingAcceptRes( void )
{
    tp_bdaddr* tpaddr = sinkDataGetSMConfirmationAddress();
    
    if(AuthCanSinkPair(tpaddr->transport, FALSE) && sinkDataGetAuthConfirmationFlag())
    {
        AUTH_DEBUG(("auth: Accepted Confirmation Req\n")) ;
        ConnectionSmUserConfirmationResponse(tpaddr, TRUE);
     }
    else
     {
        AUTH_DEBUG(("auth: Invalid state for confirmation\n"));
     }
}

/****************************************************************************/
void sinkPairingRejectRes( void )
{
    tp_bdaddr* tpaddr = sinkDataGetSMConfirmationAddress();
    
    if(AuthCanSinkPair(tpaddr->transport, FALSE) && sinkDataGetAuthConfirmationFlag())
    {    
        AUTH_DEBUG(("auth: Rejected Confirmation Req\n")) ;
        ConnectionSmUserConfirmationResponse(tpaddr, FALSE);
    }
    else
    {
        AUTH_DEBUG(("auth: Invalid state for confirmation\n"));
    }
}

/****************************************************************************/
void AuthResetConfirmationFlags ( void )
{
    AUTH_DEBUG(("auth: Reset Confirmation Flags\n"));
    if(sinkDataGetAuthConfirmationFlag())
    {
        AUTH_DEBUG(("auth: Free Confirmation Addr\n"));
        freePanic(sinkDataGetSMConfirmationAddress());
    }
    sinkDataSetSMConfirmationAddress(NULL);
    sinkDataSetAuthConfirmationFlag(FALSE);
}
