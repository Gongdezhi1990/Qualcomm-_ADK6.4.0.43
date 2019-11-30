/*
Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.

*/

/*!
@file
@ingroup sink_app
@brief   
    Implementation for handling PBAP library messages and functionality
*/

/****************************************************************************
    Header files
*/

#include <connection.h>
#include <hfp.h>
#include <print.h>
#include <panic.h>
#include <stdlib.h>
#include <bdaddr.h>
#include <stream.h>
#include <string.h>
#include <sink.h>
#include <source.h>

#include <display.h>

#ifdef ENABLE_PBAP

#include <pbapc.h>
#include <md5.h>

#include "sink_pbap.h"
#include "sink_main_task.h"
#include "sink_init.h"
#include "sink_slc.h"
#include "sink_statemanager.h"
#include "sink_callmanager.h"
#include "sink_link_policy.h"
#include "sink_display.h"
#include "sink_malloc_debug.h"
/* Include config store and definition headers */
#include "config_definition.h"
#include"sink_pbap_config_def.h" 
#include <config_store.h>

#ifdef DEBUG_PBAP
    #define PBAP_DEBUG(x) {printf x;}
#else
    #define PBAP_DEBUG(x) 
#endif

static const char gpbapbegin[] = "BEGIN:VCARD"; 
static const char gpbapname[]  = "\nN";
static const char gpbaptel[]   = "TEL";
static const char gpbapend[]   = "END:VCARD";

typedef struct 
{
    uint8 telLen;
    uint8 *pTel;
    uint8 nameLen;
    uint8 pName[1];    
}pbapMetaData;

/* Message Handler Prototypes */
static void handlePbapInitCfm(const PBAPC_INIT_CFM_T *pMsg);
static void handlePbapConnectCfm(const PBAPC_CONNECT_CFM_T *pMsg);
static void handleAuthRequestInd(const PBAPC_AUTH_REQUEST_IND_T *pMsg);
static void handleAuthResponseCfm(const PBAPC_AUTH_RESPONSE_CFM_T *pMsg);
static void handlePbapDisconnectInd(const PBAPC_DISCONNECT_IND_T *pMsg);
static void handlePbapSetPhonebookCfm(const PBAPC_SET_PHONEBOOK_CFM_T *pMsg);

static void handlePullVCardListCfm(const PBAPC_PULL_VCARD_LISTING_CFM_T *pMsg);
static void handlePullVCardEntryCfm(const PBAPC_PULL_VCARD_ENTRY_CFM_T *pMsg);
static void handlePullPhonebookCfm(const PBAPC_PULL_PHONEBOOK_CFM_T *pMsg);

/* App functions */
static void handleAppPullVcardEntry( void );
static void handleAppPullVcardList( void );
static void handleAppPullPhoneBook( void );
static void handleAppPhoneBookSize( void );

static uint8 VcardGetFirstTel(const uint8* pVcard, const uint16 vcardLen, pbapMetaData **pMetaData);
static bool handlePbapDialData(const uint8* pVcard, const uint16 vcardLen);
static void handlePbapRetrievedData(const uint8 *pVcard, const uint16 vcardLen);
static void handleVcardPhoneBookMessage(uint16 device_id, pbapc_lib_status status, const uint8 *lSource, const uint16 dataLen);
static void pbapDial(uint8 phonebook);

/* Sink PBAP global data */
typedef struct __sink_pbap_global_data_t
{    
    uint16         PbapBrowseEntryIndex;
    const uint8*   pbap_srch_val;
    unsigned       pbap_srch_attr:2;
    unsigned       pbap_active_pb:3;
    unsigned       pbap_ready:1;               /* pbapc library has been initialised */
    unsigned       pbap_command:3;
    unsigned       pbap_browsing_start_flag:1;
    unsigned       pbap_active_link:2;
    unsigned       pbap_phone_repository:2;
    unsigned       pbap_hfp_link:4;
    unsigned       pbap_access:1;
    unsigned       unused:13;
}sink_pbap_global_data_t;

/* PBAP global data structure instance */ 
static sink_pbap_global_data_t pbap_gdata;


/****************************************************************************
    Get pbap enabled or not 
*/
bool pbapIsEnabled( void )
{
    pbap_read_only_config_def_t* c_config_data = NULL;
    bool enabled = FALSE;
    if (configManagerGetReadOnlyConfig(PBAP_READ_ONLY_CONFIG_BLK_ID, (const void **)&c_config_data))
    {
       enabled = c_config_data->pbap_enabled;
       configManagerReleaseConfig(PBAP_READ_ONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

/****************************************************************************
NAME	
	pbapSetBrowseEntryIndex

DESCRIPTION
    Set pbap browse entry index

*/
void pbapSetBrowseEntryIndex(uint16 index)
{
    pbap_gdata.PbapBrowseEntryIndex = index;
}

/****************************************************************************
NAME	
	pbapGetBrowseEntryIndex

DESCRIPTION
    Get pbap browse entry index
    
*/
uint16 pbapGetBrowseEntryIndex(void)
{
    return pbap_gdata.PbapBrowseEntryIndex;
}

/****************************************************************************
NAME	
	pbapSetSearchValue

DESCRIPTION
    Set the pbap search value
    
*/
void pbapSetSearchValue(const uint8* value)
{
    pbap_gdata.pbap_srch_val = value;
}

/****************************************************************************
NAME	
	pbapGetSearchValue

DESCRIPTION
    Get the pbap search value
    
*/
const uint8* pbapGetSearchValue(void)
{
    return (const uint8*) pbap_gdata.pbap_srch_val;
}

/****************************************************************************
NAME	
	pbapSetSearchAttribute

DESCRIPTION
    Set the pbap search attribute
    
*/
void pbapSetSearchAttribute(uint8 attribute)
{
    pbap_gdata.pbap_srch_attr = attribute;
}

/****************************************************************************
NAME	
	pbapGetSearchAttribute

DESCRIPTION
    Get the pbap search attribute
    
*/
uint8 pbapGetSearchAttribute(void)
{
    return pbap_gdata.pbap_srch_attr;
}

/****************************************************************************
NAME	
	pbapSetActivePhonebook

DESCRIPTION
    Set the pbap active phonebook
    
*/
void pbapSetActivePhonebook(uint8 phonebook)
{
    pbap_gdata.pbap_active_pb = phonebook;
}

/****************************************************************************
NAME	
	pbapGetActivePhonebook

DESCRIPTION
    Get the pbap active phonebook
    
*/
uint8 pbapGetActivePhonebook(void)
{
    return pbap_gdata.pbap_active_pb;
}

/****************************************************************************
NAME	
	pbapSetReady

DESCRIPTION
    Set the pbap ready
    
*/
void pbapSetReady(uint8 ready)
{
    pbap_gdata.pbap_ready = ready;
}

/****************************************************************************
NAME	
	pbapGetReady

DESCRIPTION
    Get the pbap ready
    
*/
uint8 pbapGetReady(void)
{
    return pbap_gdata.pbap_ready;
}

/****************************************************************************
NAME	
	pbapSetCommand

DESCRIPTION
    Set the pbap command
    
*/
void pbapSetCommand(uint8 command)
{
    pbap_gdata.pbap_command = command;
}

/****************************************************************************
NAME	
	pbapGetCommand

DESCRIPTION
    Get the pbap command
    
*/
uint8 pbapGetCommand(void)
{
    return pbap_gdata.pbap_command;
}

/****************************************************************************
NAME	
	pbapSetBrowsingStartFlag

DESCRIPTION
    Set the pbap browsing start flag
    
PARAMS
    uint16 value 
    
RETURNS
	void
*/
void pbapSetBrowsingStartFlag(uint8 flag)
{
    pbap_gdata.pbap_browsing_start_flag = flag;
}

/****************************************************************************
NAME	
	pbapGetBrowsingStartFlag

DESCRIPTION
    Get the pbap browsing start flag
    
*/
uint8 pbapGetBrowsingStartFlag(void)
{
    return pbap_gdata.pbap_browsing_start_flag;
}

/****************************************************************************
NAME	
	pbapSetActiveLink

DESCRIPTION
    Set the pbap active link
    
*/
void pbapSetActiveLink(uint8 link)
{
    pbap_gdata.pbap_active_link = link;
}

/****************************************************************************
NAME	
	pbapSetActiveLink

DESCRIPTION
    Get the pbap active link
    
*/
uint8 pbapGetActiveLink(void)
{
    return pbap_gdata.pbap_active_link;
}

/****************************************************************************
NAME	
	pbapSetPhoneRepository

DESCRIPTION
    Set the pbap phone repository
    
*/
void pbapSetPhoneRepository(uint8 repository)
{
    pbap_gdata.pbap_phone_repository = repository;
}

/****************************************************************************
NAME	
	pbapGetPhoneRepository

DESCRIPTION
    Get the pbap phone repository
    
*/
uint8 pbapGetPhoneRepository(void)
{
    return pbap_gdata.pbap_phone_repository;
}

/****************************************************************************
NAME	
	pbapSetHfpLink

DESCRIPTION
    Set the pbap hfp link
    
*/
void pbapSetHfpLink(uint8 link)
{
    pbap_gdata.pbap_hfp_link = link;
}

/****************************************************************************
NAME	
	pbapGetHfpLink

DESCRIPTION
    Get the pbap hfp link
    
*/
uint8 pbapGetHfpLink(void)
{
    return pbap_gdata.pbap_hfp_link;
}

/****************************************************************************
NAME	
	pbapSetAccess

DESCRIPTION
    Set the pbap access
    
*/
void pbapSetAccess(uint8 access)
{
    pbap_gdata.pbap_access = access;
}

/****************************************************************************
NAME	
	pbapGetAccess

DESCRIPTION
    Get the pbap access
    
*/
uint8 pbapGetAccess(void)
{
    return pbap_gdata.pbap_access;
}

/*
  Initialise the PBAP System
*/
void initPbap(void)
{      
    PBAP_DEBUG(("initPbap\n"));
    
    /* initialise defaults */
    pbapSetReady(FALSE);
    pbapSetCommand(pbapc_action_idle);
    pbapSetActiveLink(pbapc_invalid_link);
    pbapSetActivePhonebook(pbap_pb);
    pbapSetBrowseEntryIndex(1);
    pbapSetHfpLink(0);
    
    /* Initialise the PBAP library */
	PbapcInit(&theSink.task);
}

/*
	Connect to PBAP Server of the HFP AGs
*/
bool pbapConnect( hfp_link_priority pbapc_hfp_link )
{
    tp_bdaddr tpaddr;
    Sink sink;
    PBAP_DEBUG(("PBAP Connect\n"));
    if(pbapIsEnabled())
    {    
        if( HfpLinkGetSlcSink(pbapc_hfp_link, &sink) && SinkGetBdAddr(sink, &tpaddr) )
        {
            /* Set the link to active state */
            linkPolicySetLinkinActiveMode(sink);
                            
            pbapSetCommand(pbapc_action_idle);
            
            PBAP_DEBUG(("Connecting Pbap profile, Addr %x,%x,%lx\n", tpaddr.taddr.addr.nap, tpaddr.taddr.addr.uap, tpaddr.taddr.addr.lap ));
            PbapcConnectRequest(&theSink.task, &tpaddr.taddr.addr);
            
            pbapSetHfpLink(pbapc_hfp_link);
            
            return TRUE;
        }
        else
            PBAP_DEBUG(("Connecting Pbap, failed to get bdaddr\n" ));
    }
    return FALSE;
}

/*
	Disconnect from all PBAP Servers
*/
void pbapDisconnect( void )
{
    uint16 device_id = 0;
    
    PBAP_DEBUG(("Disconnect all Pbap connections\n"));
    
    for(device_id = 0; device_id < MAX_PBAPC_CONNECTIONS; device_id++)
    {
        PbapcDisconnectRequest( device_id );
    }
    
    pbapSetCommand(pbapc_action_idle);
}


/*
    Disconnect PBAP Server with the provided device.
*/
void pbapDisconnectDevice(const bdaddr *bd_addr)
{    
	uint8 device_id = pbapc_invalid_link;
	
    PBAP_DEBUG(("Disconnect Pbap connection based on device ID\n"));
    
	device_id = PbapcGetLinkFrmAddr(bd_addr);

	if(device_id < pbapc_invalid_link)
	{
        PbapcDisconnectRequest(device_id);
	}
    else
	{
        PBAP_DEBUG(("Invalid Link\n"));
	}
       
    pbapSetCommand(pbapc_action_idle);    
}


/*
	Dial the first entry in the specified phonebook
*/
static void pbapDial(uint8 phonebook)
{  
    /* attempt to dial the first entry in the AG call history entries */
    if(pbapGetReady())
    {
        pbapSetActivePhonebook(phonebook);

        if(pbapGetActiveLink() != pbapc_invalid_link)
        {
            /* Set the link to active state */
            linkPolicySetLinkinActiveMode(PbapcGetSink(pbapGetActiveLink()));
            
            /* the Pbap profile of the primary HFP device has been connected, set the phonebook and dial */
            PBAP_DEBUG(("Pbap dial, set the phonebook first\n"));
            PbapcSetPhonebookRequest(pbapGetActiveLink(), pbapGetPhoneRepository(), pbapGetActivePhonebook());
        }
        else
        {
            /* Otherwise, try to connect Pbap profile of the primary HFP device first before dialling */
            /* If primary HFP device fails due to no supported PBAP, try secondary HFP devices        */
            
            PBAP_DEBUG(("Pbap dial, connect the Pbap profile first\n"));
            
            if( !pbapConnect( hfp_primary_link ) )
            {
                MessageSend ( &theSink.task , EventSysPbapDialFail , 0 ) ;
                pbapSetCommand(pbapc_action_idle);
                pbapSetActivePhonebook(0);
            }
        }        
    }
    else
    {
        PBAP_DEBUG(("PBAPC profile was not initialised\n"));
        MessageSend ( &theSink.task , EventSysPbapDialFail , 0 ) ;
        pbapSetCommand(pbapc_action_idle);
    }
}

/*
	Dial the first entry in the phonebook
*/
void pbapDialPhoneBook( uint8 phonebook )
{
    if(pbapGetCommand() == pbapc_action_idle)
    {
        if (!stateManagerIsConnected() )
        {	
#ifdef ENABLE_AVRCP
            sinkAvrcpCheckManualConnectReset(NULL);
#endif            
            PBAP_DEBUG(("Pbap dial, Connect the HFP profile first\n"));
            MessageSend ( &theSink.task , EventUsrEstablishSLC , 0 ) ;
            
            switch(phonebook)
            {
                case pbap_ich:
                    sinkQueueEvent( EventUsrPbapDialIch ) ;
                break;
                case pbap_mch:
                    sinkQueueEvent( EventUsrPbapDialMch ) ;
                break;
                default:
                break;
            }
        }
        else
        {   
            PBAP_DEBUG(("PBAP DialPhoneBook\n"));
            pbapDial(phonebook);
        }    
        
        pbapSetCommand(pbapc_dialling);
    }
}


/*********************************************************************************
    PBAP Message Handler:
	    Process the Pbapc profile message from library
*********************************************************************************/
void handlePbapMessages(Task task, MessageId pId, Message pMessage)
{
   	PBAP_DEBUG(("handlePbapMessages, ID: [%x]\n",pId));

    UNUSED(task);
	switch (pId)
	{
	case PBAPC_INIT_CFM:
        handlePbapInitCfm((const PBAPC_INIT_CFM_T *)pMessage);
		break;
    case PBAPC_AUTH_REQUEST_IND :
        handleAuthRequestInd((const PBAPC_AUTH_REQUEST_IND_T *)pMessage);
        break;
    case PBAPC_AUTH_RESPONSE_CFM:
        handleAuthResponseCfm((const PBAPC_AUTH_RESPONSE_CFM_T *)pMessage);
        break;
        
	case PBAPC_CONNECT_CFM:
        handlePbapConnectCfm((const PBAPC_CONNECT_CFM_T *)pMessage);
		break;
	case PBAPC_DISCONNECT_IND:
        handlePbapDisconnectInd((const PBAPC_DISCONNECT_IND_T *)pMessage);
		break;
	case PBAPC_SET_PHONEBOOK_CFM:
        handlePbapSetPhonebookCfm((const PBAPC_SET_PHONEBOOK_CFM_T *)pMessage);
		break;
        
	case PBAPC_PULL_VCARD_LISTING_CFM:
        handlePullVCardListCfm((const PBAPC_PULL_VCARD_LISTING_CFM_T *)pMessage);
		break;
    case PBAPC_PULL_VCARD_ENTRY_CFM:
        handlePullVCardEntryCfm((const PBAPC_PULL_VCARD_ENTRY_CFM_T *)pMessage);
		break;
	case PBAPC_PULL_PHONEBOOK_CFM:
        handlePullPhonebookCfm((const PBAPC_PULL_PHONEBOOK_CFM_T *)pMessage);
		break;
    
    /* Local PBAPC App message */
    case PBAPC_APP_PULL_VCARD_ENTRY:
        handleAppPullVcardEntry();
        break;
    case PBAPC_APP_PULL_VCARD_LIST:
        handleAppPullVcardList();
        break;
    case PBAPC_APP_PULL_PHONE_BOOK:
        handleAppPullPhoneBook();
        break;
    case PBAPC_APP_PHONE_BOOK_SIZE:
        handleAppPhoneBookSize();
        break;
        
    default:
        PBAP_DEBUG(("PBAPC Unhandled message : 0x%X\n",pId));
        break;
	}
}

/* Message Handlers */
static void handlePbapInitCfm( const PBAPC_INIT_CFM_T *pMsg)
{
	PBAP_DEBUG(("PBAPC_INIT_CFM, status: [%x]\n", pMsg->status));
    
	if (pMsg->status == pbapc_success)
	{
		PBAP_DEBUG(("success\n"));
        pbapSetReady(TRUE);

        /* start initialising the configurable parameters*/
    	InitUserFeatures() ; 
	}
	else
	{ 
        /* Failed to initialise PBAPC */
		PBAP_DEBUG(("PBAP init failed   Status : %d\n", pMsg->status));	
		Panic();
	}
}

static void handlePbapConnectCfm(const PBAPC_CONNECT_CFM_T *pMsg)
{	
    PBAP_DEBUG(("PBAPC_CONNECT_CFM, device_id : %d,Status : %d, packet size:[%d], repositories:[%d]\n", pMsg->device_id, pMsg->status, pMsg->packetSize, pMsg->repositories));
	
    if(pMsg->status == pbapc_success)
    {
        if(stateManagerGetState() == deviceLimbo)
        {
            pbapSetCommand(pbapc_action_idle);
            PbapcDisconnectRequest(pMsg->device_id);
            return;
        }

        /* If the Pbap of primary HFP device has been connected, save its device_id as the active link */
        if( pbapGetHfpLink() == hfp_primary_link )
        {
            pbapSetActiveLink(pMsg->device_id);
            pbapSetPhoneRepository(pMsg->repositories);
                    
            PBAP_DEBUG(("PBAPC_CONNECT_CFM, Set the active Pbap link as [%d]\n", pbapGetActiveLink()));
        }
        
        /* if we are making Pbapc dialing now if pbap dial is ongoing.*/
        if( (pbapGetActiveLink() != pbapc_invalid_link) )
        {
            switch(pbapGetCommand())
            {
                case pbapc_dialling:
                case pbapc_browsing_entry:
                case pbapc_browsing_list:
                case pbapc_setting_phonebook:
                    PbapcSetPhonebookRequest(pbapGetActiveLink(), 
                                             pbapGetPhoneRepository(), 
                                             pbapGetActivePhonebook());
                break;
                case pbapc_downloading:
                    MessageSend(&theSink.task , PBAPC_APP_PULL_PHONE_BOOK , 0 ) ;
                break;
                case pbapc_action_idle:
                    /* Set the link policy based on the HFP or A2DP state */
                    linkPolicyPhonebookAccessComplete(PbapcGetSink(pbapGetActiveLink()));    
                default:
                break;
            }
        }
    }
    else if(pMsg->status != pbapc_pending)    
    {
        Sink sink;
        
        /* pbapc profile connection failure */
        if(pbapGetCommand() == pbapc_dialling)
        {
            MessageSend ( &theSink.task , EventSysPbapDialFail , 0 ) ;
        } 
        
        /* Set the link policy based on the HFP or A2DP state */
        HfpLinkGetSlcSink(pbapGetHfpLink(), &sink);
        linkPolicyPhonebookAccessComplete( sink ); 
        
        pbapSetCommand(pbapc_action_idle);
    }

    pbapSetBrowsingStartFlag(0);
}

static void handlePbapDisconnectInd(const PBAPC_DISCONNECT_IND_T *pMsg)
{
    PBAP_DEBUG(("PBAPC_DISCONNECT_IND, "));
    
    /* Reset the active pbapc link */
    if(pbapGetActiveLink() == pMsg->device_id)
    {
        if(PbapcGetNoOfConnection() == 1)
        {
            pbapSetActiveLink(pbapc_secondary_link - pMsg->device_id);
            pbapSetPhoneRepository(PbapcGetServerProperties( pbapGetActiveLink()));
        }
        else
        {
            pbapSetActiveLink(pbapc_invalid_link);
        }
        PBAP_DEBUG(("change the active pbap link id to [%d]\n", pbapGetActiveLink()));
    }

    pbapSetCommand(pbapc_action_idle);
    
    pbapSetBrowsingStartFlag(0);
    
    pbapSetAccess(FALSE);
}

static void handlePbapSetPhonebookCfm(const PBAPC_SET_PHONEBOOK_CFM_T *pMsg)
{
	PBAP_DEBUG(("PBAPC_SET_PHONEBOOK_CFM, Status : %d\n", pMsg->status));
        
    switch(pMsg->status)
    {
        case pbapc_success:
            /* Successfully set the phonebook, pull first entry from the phone book */
            switch(pbapGetCommand())
            {
                case pbapc_dialling:
                    MessageSend ( &theSink.task , PBAPC_APP_PULL_VCARD_ENTRY, 0 ) ;
                break;
                case pbapc_browsing_entry:
                    MessageSend ( &theSink.task , PBAPC_APP_PULL_VCARD_ENTRY, 0 ) ;
                break;
                case pbapc_browsing_list:
                    MessageSend ( &theSink.task , PBAPC_APP_PULL_VCARD_LIST, 0 ) ;
                break;
                case pbapc_action_idle:
                case pbapc_downloading:
                case pbapc_setting_phonebook:
                    /* Set the link policy based on the HFP or A2DP state */
                    linkPolicyPhonebookAccessComplete(PbapcGetSink(pbapGetActiveLink())); 
                default:
                break;
            }
        break;
        
        case pbapc_spb_unauthorised:
            /* access to this phonebook denied by PBAP server */
            PBAP_DEBUG(("PBAP access to phonebook unauthorised\n")); 
        default:
            /* other error */
            PBAP_DEBUG(("PBAP failed to set phonebook\n")); 
            if(pbapGetCommand() == pbapc_dialling)
            {
                MessageSend ( &theSink.task , EventSysPbapDialFail , 0 ) ; 
                pbapSetCommand(pbapc_action_idle);
            }
            /* Set the link policy based on the HFP or A2DP state */
            linkPolicyPhonebookAccessComplete(PbapcGetSink(pbapGetActiveLink())); 
        break;
    }
}

static void handleAuthResponseCfm(const PBAPC_AUTH_RESPONSE_CFM_T *pMsg)
{
    PBAP_DEBUG(("PBAPC_AUTH_RESPONSE_CFM"));
    /* Client is currently not bothered about authenticating the server */
    UNUSED(pMsg);
    return; 
}  

static void handleAuthRequestInd(const PBAPC_AUTH_REQUEST_IND_T *pMsg)
{
    uint8 digest[PBAPC_OBEX_SIZE_DIGEST];
    PRINT(("PBAPC_AUTH_REQUEST_IND. options = %d\n", pMsg->options ));

    {
        MD5_CTX context;
        /* Digest blocks */
        MD5Init(&context);
        MD5Update(&context, pMsg->nonce, PBAPC_OBEX_SIZE_DIGEST);
        MD5Update(&context, (const uint8 *)":",1);
        MD5Update(&context, (const uint8 *)"8888",4);
        MD5Final(digest,&context);
    }
  
    /* Client is not bothered about authenticating the Server, So just
       echo back the nonce. If the client wants to authenticate , it must 
       send its own nonce and authenticate the server on receiving 
       PBAPC_AUTH_RESPONSE_CFM message.
    */ 
    PbapcConnectAuthResponse(pMsg->device_id, &digest[0], 0, NULL, NULL);
}

static void handlePullVCardEntryCfm(const PBAPC_PULL_VCARD_ENTRY_CFM_T *pMsg)
{
    const uint8 *lSource = SourceMap(pMsg->src);
    
    PBAP_DEBUG(("PBAPC_PULL_VCARD_ENTRY_CFM, source:[%p], size:[%d]\n", (const void*)lSource, pMsg->dataLen));
    
    handleVcardPhoneBookMessage(pMsg->device_id, pMsg->status, lSource, pMsg->dataLen);
    
    if(pMsg->status != pbapc_pending && pbapGetCommand() == pbapc_dialling)
    {
        MessageSend ( &theSink.task , EventSysPbapDialFail , 0 ) ; 
    }
    
    if(pMsg->status != pbapc_pending)
    { 
        /* Reset the flag */
        pbapSetCommand(pbapc_action_idle);
    }
}

static void handlePullVCardListCfm(const PBAPC_PULL_VCARD_LISTING_CFM_T *pMsg)
{  
    
#ifdef DEBUG_PBAP    
    {
        const uint8 *lSource = SourceMap(pMsg->src);          
        uint16 i;
        PBAP_DEBUG(("PBAPC_PULL_VCARD_LIST_CFM, source:[%p], size:[%d]\n", (const void*)lSource, pMsg->dataLen));
        PBAP_DEBUG(("The pb data is: "));

        if (lSource == NULL)
        {
            PBAP_DEBUG(("NULL"));
        }
        else
        {
            for(i = 0; i < pMsg->dataLen; i++)
                PBAP_DEBUG(("%c", *(lSource + i))); 
        }
        PBAP_DEBUG(("\n"));    
    }
#endif

      /* Read more data for pbap dial fail or other pbap features */
    if (pMsg->status == pbapc_pending)
    {
        PBAP_DEBUG(("    Requesting next Packet\n"));
        PbapcPullContinue(pMsg->device_id);
    }
    else
    { 
        PBAP_DEBUG(("    Requesting complete.\n"));
        /* Send Complete to Server */
        PbapcPullComplete(pMsg->device_id);

        /* Set the link policy based on the HFP or A2DP state */
        linkPolicyPhonebookAccessComplete(PbapcGetSink(pbapGetActiveLink()));     

        pbapSetCommand(pbapc_action_idle);
    }
}

static void handlePullPhonebookCfm(const PBAPC_PULL_PHONEBOOK_CFM_T *pMsg)
{

#ifdef DEBUG_PBAP    
    {
        const uint8 *lSource = SourceMap(pMsg->src);
        uint16 i;
       
        PBAP_DEBUG(("PBAPC_PULL_PHONEBOOK_CFM, source:[%p], pbsize:[%d], datalen:[%d]\n", (const void*)lSource,  pMsg->pbookSize, pMsg->dataLen));

        PBAP_DEBUG(("The pb data is: "));

        if (lSource == NULL)
        {
            PBAP_DEBUG(("NULL"));
        }
        else
        {
            for(i = 0; i < pMsg->dataLen; i++)
                PBAP_DEBUG(("%c", *(lSource + i))); 
        }
        PBAP_DEBUG(("\n"));    
    }
#endif    

      /* Read more data for pbap dial fail or other pbap features */
    if (pMsg->status == pbapc_pending)
    {
        PBAP_DEBUG(("    Requesting next Packet\n"));
        PbapcPullContinue(pMsg->device_id);
    }
    else
    { 
        PBAP_DEBUG(("    Requesting complete.\n"));
        /* Send Complete to Server */
        PbapcPullComplete(pMsg->device_id);

        /* Set the link policy based on the HFP or A2DP state */
        linkPolicyPhonebookAccessComplete(PbapcGetSink(pbapGetActiveLink()));     

        pbapSetCommand(pbapc_action_idle);
    }
}

/****************************************************************************
    Internal PBAP App Message
****************************************************************************/
static void handleAppPullVcardEntry(void)
{
    PBAP_DEBUG(("PBAPC_APP_PULL_VCARD_ENTRY %d\n", pbapGetBrowseEntryIndex()));
    if(pbapGetActiveLink() != pbapc_invalid_link)
    {
        PbapcPullvCardEntryParams *pParams = newZDebugPanic(PbapcPullvCardEntryParams);

        if(pParams)
        {
            pParams->filter.filterHigh = PBAPC_FILTER_HIGH;
            pParams->filter.filterLow  = PBAPC_FILTER_LOW;
            pParams->format = pbap_format_21;
                
            PbapcPullVcardEntryRequest(pbapGetActiveLink(), (uint32)(pbapGetBrowseEntryIndex()), pParams );
                
            PBAP_DEBUG(("PBAPC_APP_PULL_VCARD_ENTRY free %p %p\n",(void*)pParams,(void*)&pParams));
            
            freePanic(pParams);
        }
    }     
    else
    { 
    	PBAP_DEBUG((" Pbap in incorrect state\n"));
    }
}

static void handleAppPullVcardList(void)
{
    PBAP_DEBUG(("PBAPC_APP_PULL_VCARD_LIST, "));
    if(pbapGetActiveLink() != pbapc_invalid_link)
    {
        PbapcPullvCardListParams *pParams = newZDebugPanic(PbapcPullvCardListParams);

        if(pParams)
        {
            pParams->order    = pbap_order_idx;
            pParams->srchAttr = pbapGetSearchAttribute();
            pParams->srchVal  = pbapGetSearchValue();
            pParams->srchValLen = (pParams->srchAttr == pbap_search_name) ? (uint8)(strlen((const char *)pParams->srchVal) + 1) : 0;
            pParams->maxList    = PBAPC_MAX_LIST;
            pParams->listStart  = PBAPC_LIST_START;
    
	   	    PbapcPullVcardListingRequest( pbapGetActiveLink(), pbap_root, pParams ); 
                
            freePanic(pParams);
        }
    }
    else
	{ 
	    PBAP_DEBUG(("    Pbap in incorrect state\n"));
	}
}

static void handleAppPullPhoneBook(void)
{
    PBAP_DEBUG(("PBAPC_APP_PULL_PHONE_BOOK, "));
    if(pbapGetActiveLink() != pbapc_invalid_link)
    {
        PbapcPullPhonebookParams *pParams = newZDebugPanic(PbapcPullPhonebookParams);

        if(pParams)
        {
            pParams->filter.filterHigh = PBAPC_FILTER_HIGH;
            pParams->filter.filterLow  = PBAPC_FILTER_LOW;
            pParams->format    = pbap_format_21;
            pParams->maxList   = PBAPC_MAX_LIST;
            pParams->listStart = PBAPC_LIST_START;
            	
            if(pbapGetCommand() != pbapc_dialling)
            {
                pbapSetActivePhonebook(pbap_pb);
            }
                
            PbapcPullPhonebookRequest(pbapGetActiveLink(), pbapGetPhoneRepository(), pbapGetActivePhonebook(), pParams);
                
            freePanic(pParams);
        }
	}
	else
	{ 
	    PBAP_DEBUG(("    Pbap in incorrect state\n"));
    }
   
}

static void handleAppPhoneBookSize(void)
{
    PBAP_DEBUG(("PBAPC_APP_PHONE_BOOK_SIZE, "));
    if(pbapGetActiveLink() != pbapc_invalid_link)
    {
        PbapcPullPhonebookParams *pParams = newZDebugPanic(PbapcPullPhonebookParams);

        if(pParams)
        {
            pParams->filter.filterHigh = PBAPC_FILTER_HIGH;
            pParams->filter.filterLow  = PBAPC_FILTER_LOW;
            pParams->format    = pbap_format_21;
            pParams->maxList   = 0;
            pParams->listStart = 0;
            	
            if(pbapGetCommand() != pbapc_dialling)
            {
                pbapSetActivePhonebook(pbap_pb);
            }
                
            PbapcPullPhonebookRequest(pbapGetActiveLink(), pbapGetPhoneRepository(), pbapGetActivePhonebook(), pParams);
                
            freePanic(pParams);
        }
	}
	else
	{ 
	    PBAP_DEBUG(("    Pbap in incorrect state\n"));
    }
   
}

/****************************************************************************
NAME	
	VcardGetFirstTel

DESCRIPTION
    Find the first telephone number from the supplied VCARD data
    
PARAMS
    pVcard   pointer to supplied VCARD data
    pTel     pointer to section of pVcard where the telephone number begins
    
RETURNS
	uint8    length of the found telephone number, 0 if not found
*/
static uint8 *memstr( const uint8 *buffer, const uint16 buffer_size, const uint8 *str, const uint16 count )
{

    uint8 *p = (uint8 *)memchr(buffer, str[0], buffer_size);
    PBAP_DEBUG(("PBAP memstr\n"));
    
    while (p && p < buffer + buffer_size)
    {
        if(memcmp((const char *)p, (const char *)str, count) == 0)
        {
            return p;
        }
        p += 1;
        p = (uint8 *)memchr(p, str[0], (uint16)(buffer+buffer_size - p));
    }
    
    return 0;
}

static uint16 VcardFindMetaData( const uint8 *start, const uint8 *end, uint8 **metaData, const char *str, const uint16 count)
{
    uint16 len;
    const uint8 *p = start;
    uint8 *endstring = NULL;
    UNUSED(count);
       
    PBAP_DEBUG(("PBAP VcardFindMetaData\n"));

    /* find the MetaData */
    len         = (uint16)(end - p);
    
    if((((*metaData) = (uint8 *)memstr(p, len, (const uint8 *)str, (uint16)strlen(str))) != NULL) &&
       (((*metaData) = (uint8 *)memchr((uint8 *)(*metaData), ':',  end - (*metaData))) != NULL))
    {
        (*metaData) += 1;
        endstring    = (uint8 *)memchr((uint8 *)(*metaData), '\n', end - (*metaData)) - 1;
    }
    else
    {
        /* There are some errors about the format of phonebook. */
        return 0;
    }
    
    return(uint16)(endstring - (*metaData));
}

static uint8 VcardGetFirstTel(const uint8* pVcard, const uint16 vcardLen, pbapMetaData **pMetaData)
{ 
    uint16 len    = 0;
    uint16 telLen, nameLen = 0;
    uint8 *pTel   = NULL;
    uint8 *pName  = NULL;
    
    /* Find the start and end position of the first Vcard Entry */
    uint8 *start = memstr(pVcard, vcardLen, (const uint8 *)gpbapbegin, (uint16)strlen(gpbapbegin));
    const uint8 *end = memstr(pVcard, vcardLen, (const uint8 *)gpbapend, (uint16)strlen(gpbapend));
    end           = (end == NULL) ? (pVcard + vcardLen - 1) : end;

#ifdef DEBUG_PBAP    
    {
        uint16 i;
        PBAP_DEBUG(("The pVcard is: "));

        for(i = 0; i < vcardLen; i++)
            PBAP_DEBUG(("%c", *(pVcard + i))); 
        
        PBAP_DEBUG(("\n"));    
    }
#endif
    
    PBAP_DEBUG(("First entry start:[%p], end:[%p]\n", (void*)start, (const void*)end));
    
    while(start && start < end)
    {
        start = start + strlen(gpbapbegin);

        /* find the Tel */
        telLen = VcardFindMetaData(start, end, &pTel, gpbaptel, (const uint16)strlen(gpbaptel));
        
        if( telLen )
        {
            PBAP_DEBUG(("VcardGetFirstTel:telephone number found ok\n"));
            
            /* find the Name */
            nameLen = VcardFindMetaData(start, end, &pName, gpbapname, (const uint16)strlen(gpbapname));
            
            /* allocate the memory for pMetaData structure */
            *pMetaData = (pbapMetaData *)mallocPanic(sizeof(pbapMetaData) + nameLen);

            if(pMetaData)
            {
                (*pMetaData)->pTel    = pTel;
                (*pMetaData)->telLen  = (uint8)telLen;
                (*pMetaData)->nameLen = (uint8)nameLen;
            
                PBAP_DEBUG(("CallerID pos:[%p], len:[%d]\n", (void*)pName, nameLen));
           
                if(nameLen)
                {
                    /* This memory should be freed after pbap dial command or Audio Prompt has completed */
                    memmove(&((*pMetaData)->pName), pName, nameLen);
                    (*pMetaData)->pName[nameLen] = '\0';
                
                    /* Remove the ';' between names */
                    /* Based on PBAP spec., the name format is: 
                      LastName;FirstName;MiddleName;Prefix;Suffix
                    */
                    len = nameLen;
                    pName = (*pMetaData)->pName;
                    while(pName < (*pMetaData)->pName + nameLen)
                    {
                        pName    = (uint8 *)memchr(pName, ';', len) ;
                        /*if no ; is found exit */                            
                        if(!pName)
                            break;
                        *pName++ = ' ';
                        /* determine how many characters are left */
                        len = nameLen - (uint16)(pName - (*pMetaData)->pName);
                    }
                
                    PBAP_DEBUG(("VcardGetFirstTel:CallerID found ok\n"));
                }

                return (uint8)telLen;
            }
            else
            {
                PBAP_DEBUG(("VcardGetFirstTel:No memory slot to store MetaData\n"));
                return 0;
            }
        }
        
        /* If the first Vcard Entry the Tel is enmty, try next Entry. */
        /* First find the next Vcard Entry start and end positions    */
        end = end + strlen(gpbapend);
        len = (uint16)(pVcard + vcardLen - end);
        start = memstr(end, len , (const uint8 *)gpbapbegin, (uint16)strlen(gpbapbegin));
        end   = memstr(end, len,  (const uint8 *)gpbapend,   (uint16)strlen(gpbapend));
        end   = (end == NULL) ? (const uint8 *)(pVcard + vcardLen - 1) : (const uint8 *)end;
        
        PBAP_DEBUG(("next start:[%p], end:[%p]\n", (void*)start, (const void*)end));
    }
     
    PBAP_DEBUG(("VcardGetFirstTel:telephone number not found\n"));
   
    return 0;
}

static bool handlePbapDialData(const uint8 *pVcard, const uint16 vcardLen)
{
    pbapMetaData *pMetaData = NULL;
    bool l_success = FALSE;
    
    PBAP_DEBUG(("handlePbapDial:The length of data is [%d]\n", vcardLen));
    
  	/* Process Data to find telephone number*/
    if(VcardGetFirstTel(pVcard, vcardLen, &pMetaData))
    {
        PBAP_DEBUG(("handlePbapDial:dialling from PBAP Phonebook\n"));
        
        /* Display the name of tel of pbap dial entry.*/
        /* Audio Prompts can be used to play the caller ID */
#ifdef DEBUG_PBAP
        {
            uint8 i = 0;
            PBAP_DEBUG(("The Name is: "));
            for(i = 0; i < pMetaData->nameLen; i++)
                PBAP_DEBUG(("%c ", *(pMetaData->pName + i)));

            PBAP_DEBUG(("\nThe Tel is: "));
            for(i = 0; i < pMetaData->telLen; i++)
                PBAP_DEBUG(("%c ", *(pMetaData->pTel + i)));
            PBAP_DEBUG(("\n"));
        }
#endif

        displayShowText((char*)pMetaData->pName, pMetaData->nameLen, DISPLAY_TEXT_SCROLL_SCROLL, 1000, 2000, FALSE, 0, SINK_TEXT_TYPE_CALLER_INFO);

        HfpDialNumberRequest(hfp_primary_link, pMetaData->telLen, (uint8 *)(pMetaData->pTel));

        /* Task of Pbapc profile has completed and Hfp profile starts to work */
        pbapSetCommand(pbapc_action_idle);

        l_success = TRUE;

    }
    else
    {
         /* error, number not found */   
         PBAP_DEBUG(("handlePbapDial:no number found to dial.\n"));
    }
    
    if(pMetaData)
    {
        freePanic(pMetaData);
    }

    return(l_success);
}

static void handlePbapRetrievedData(const uint8 *pVcard, const uint16 vcardLen)
{
    /* Display the content of the retrieved data.*/
    #ifdef DEBUG_PBAP
    {
        uint8 i = 0;
        for(i = 0; i < vcardLen; i++)
            PBAP_DEBUG(("%c ", *(pVcard + i)));
            
        PBAP_DEBUG(("\n"));
    }
    #else
    UNUSED(vcardLen);
    UNUSED(pVcard);
    #endif
}

static void handleVcardPhoneBookMessage(uint16 device_id, pbapc_lib_status status, const uint8 *lSource, const uint16 dataLen)
{
    PBAP_DEBUG(("PBAP vcardPhoneBookMessage\n"));

    if(pbapGetCommand() == pbapc_dialling)
    {
	    /* Process the data and dial the number */
        if(handlePbapDialData(lSource, dataLen))
        {
            PbapcPullComplete(device_id);
            return;
        }
    }
    else
    {
        /* Other pbap features, for example, downloading or browsing phonebook object. */
        /* As no external memory avaible, just display the data: Name, Tel */
        handlePbapRetrievedData(lSource, dataLen);
    }
    
    /* Read more data for pbap dial fail or other pbap features */
    if (status == pbapc_pending)
    {
        PBAP_DEBUG(("    Requesting next Packet\n"));
        PbapcPullContinue(device_id);
    }
    else
    { 
        PBAP_DEBUG(("    Requesting complete.\n"));
	    /* Send Complete to Server */
        PbapcPullComplete(device_id);
    }
}

#endif /*ENABLE_PBAP*/
