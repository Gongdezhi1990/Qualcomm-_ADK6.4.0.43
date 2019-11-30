/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    manages inquiry/pairing with devices based on RSSI
*/

/***************************************************************************
    Header files
*/
#include <config_store.h>

#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_inquiry.h"
#include "sink_slc.h"
#include "sink_statemanager.h"
#include "sink_devicemanager.h"
#include "sink_device_id.h"
#include "sink_peer.h"
#include "sink_config.h"
#include "sink_auto_power_off.h"
#include "sink_malloc_debug.h"
#include "sink_class_of_device.h"

#include "config_definition.h"
#include "sink_inquiry_config_def.h"
#include <config_store.h>
#ifdef ENABLE_SUBWOOFER
#include "sink_swat.h"
#endif

#ifdef ENABLE_FAST_PAIR
#include "sink_fast_pair.h"
#endif

#include <bdaddr.h>

#include <stdlib.h>

#define MAKE_INQ_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/* Runtime Data strcuture for Inq Module */
typedef struct __sink_inquiry_global_data_t
{
    inquiry_data_t  inquiry;
    uint16 paging_count;
    unsigned inquiry_tx:8;
    unsigned unused:6;
}sink_inquiry_global_data_t;

/* Global data strcuture for Inquiry */
static sink_inquiry_global_data_t gInquryData;
#define GINQDATA gInquryData


#ifdef DEBUG_INQ
    #define INQ_DEBUG(x) DEBUG(x)
    #define INQ_ERROR(x) TOLERATED_ERROR(x)
#else
    #define INQ_DEBUG(x)
    #define INQ_ERROR(x)
#endif

/*Default HCI values for scan */
#define HCI_PAGESCAN_INTERVAL_DEFAULT  (0x800)
#define HCI_PAGESCAN_WINDOW_DEFAULT   (0x12)
#define HCI_INQUIRYSCAN_INTERVAL_DEFAULT  (0x800)
#define HCI_INQUIRYSCAN_WINDOW_DEFAULT   (0x12)

/* RSSI configuration block */
#define NUM_INQ_DEVS        sinkInquiryGetNumberInquiryResults() 
#define NUM_INQ_RESULTS     (NUM_INQ_DEVS + 1)

#define RSSI_CONNECTING     (GINQDATA.inquiry.action == rssi_connecting)

/* Get thresholds dependant on whether we're connecting or pairing */
#define RSSI_DIFF_THRESHOLD (int)(RSSI_CONNECTING ? sinkInquiryGetRssiDiffConnThreshold() : sinkInquiryGetRssiConnThreshold())
#define RSSI_THRESHOLD      (RSSI_CONNECTING ? sinkInquiryGetRssiConnThreshold() : sinkInquiryGetRssiThreshold())
/* Check if device is in the PDL */
#define RSSI_CAN_CONNECT(x) (sinkInquiryConnectIfInPdl() ? deviceManagerGetAttributes(NULL, x) : TRUE)
#define RSSI_CAN_PAIR(x)    (sinkInquiryDonotPairIfInPdl() ? !deviceManagerGetAttributes(NULL, x) : TRUE)
#define RSSI_CHECK_PDL(x)   (RSSI_CONNECTING ? RSSI_CAN_CONNECT(x) : RSSI_CAN_PAIR(x))

#define LIAC 0x9E8B00
#define GIAC 0x9E8B33

/* Pack higher and lower uint16s to form Uint32 bitmask */
#define PACK_UINT16_TO_UINT32(pio_low, pio_high) (((uint32)pio_high << 16) | ((uint32) (pio_low) & 0x0000FFFFUL))

#ifdef ENABLE_PEER
/* Service search patterns */
/* DataElSeq(0x35), Length(0x03), 16-bit UUID(0x19), HSP AG(0x1112) */
static const uint8 hsp_service_search_pattern[] = {0x35, 0x03, 0x19, 0x11, 0x12};
/* DataElSeq(0x35), Length(0x03), 16-bit UUID(0x19), HFP AG(0x111F) */
static const uint8 hfp_service_search_pattern[] = {0x35, 0x03, 0x19, 0x11, 0x1F};
/* DataElSeq(0x35), Length(0x03), 16-bit UUID(0x19), Audio Source(0x110A) */
static const uint8 a2dp_service_search_pattern[] = {0x35, 0x03, 0x19, 0x11, 0x0A};
/* DataElSeq(0x35), Length(0x03), 16-bit UUID(0x19), AVRCP Target(0x110C) */
static const uint8 avrcp_service_search_pattern[] = {0x35, 0x03, 0x19, 0x11, 0x0C};

/* Service search list */
static const struct
{
    supported_profiles profile;
	uint16             size;
	const uint8 *const pattern;
} service_searches[] =
{
	{ profile_hsp  , sizeof(hsp_service_search_pattern)  , hsp_service_search_pattern   },
    { profile_hfp  , sizeof(hfp_service_search_pattern)  , hfp_service_search_pattern   },
	{ profile_a2dp , sizeof(a2dp_service_search_pattern) , a2dp_service_search_pattern  },
    { profile_avrcp, sizeof(avrcp_service_search_pattern), avrcp_service_search_pattern }
};

/* Prototype and task structure for RSSI Inquiry specific SDP searches */
static void handleInquiryMessage (Task task, MessageId id, Message message);
static const struct TaskData inquiry_task = { handleInquiryMessage };
#endif

static void inquiryConnect (uint8 index);
static uint8 sinkInquiryGetNumberInquiryResults(void);


/**********************************************************************
    Interfaces for getting rssi configuration 
*/
static void sinkInquiryGetRssiConfig(rssi_pairing_t *rssi)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    INQ_DEBUG(("INQ_DATA:sinkInquiryGetRssiConfig\n"));
    
    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
        memcpy(rssi,&read_config_data->rssi,sizeof(rssi_pairing_t));
        configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }
    return;
}

/**********************************************************************
    Interfaces for getting radio configuration 
*/
void sinkInquiryGetRadioConfig(radio_config_type *radio)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    INQ_DEBUG(("INQ_DATA:sinkDataGetRadioConfig\n"));
    
    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
        radio->page_scan_interval = read_config_data->radio.page_scan_interval;

#ifdef ENABLE_FAST_PAIR
          /*If device is pairing using fast pair procedure, use aggressive page scan interval. This is needed for reducing the delay
             in establishing BR/EDR ACL for pairing*/
         if(sinkFastPairGetFastPairState() == fast_pair_state_key_based_pairing_response)
        {
            radio->page_scan_interval = 0xA0; /*set for 100ms*/ 
            INQ_DEBUG(("INQ_DATA:page_scan_interval set to 100ms\n"));
        }
#endif

        radio->page_scan_window = read_config_data->radio.page_scan_window;
        radio->inquiry_scan_interval = read_config_data->radio.inquiry_scan_interval;
        radio->inquiry_scan_window = read_config_data->radio.inquiry_scan_window;

        configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }
    return;
}

/****************************************************************************
NAME	
    sinkInquiryGetInquiryTimeout

DESCRIPTION
    Get InquiryTimeout_s.
RETURNS
    uint16
    
*/
uint16 sinkInquiryGetInquiryTimeout(void)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    uint16 inquiry_timeout = 0;

    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
        inquiry_timeout = read_config_data->InquiryTimeout_s;
        configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }

    return inquiry_timeout;
}

bool sinkInquirySetInquiryTimeout(uint16 timeout)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;

    if (configManagerGetWriteableConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (void **)&read_config_data, 0))
    {
        read_config_data->InquiryTimeout_s = timeout;
        configManagerUpdateWriteableConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME	
    sinkInquiryGetNumberInquiryResults

DESCRIPTION
    Get RSSI number of results.
RETURNS
    uint8
    
*/
static uint8 sinkInquiryGetNumberInquiryResults(void)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    uint8 num_inq_results = 0;

    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
      num_inq_results = read_config_data->rssi.num_results;
      configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }

    return num_inq_results;
}
/****************************************************************************
NAME	
    sinkInquiryGetRssiDiffConnThreshold

DESCRIPTION
    Get RSSI conn_diff_threshold.
RETURNS
    uint16
    
*/
static uint16 sinkInquiryGetRssiDiffConnThreshold(void)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    uint16 conn_diff_threshold = 0;

    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
      conn_diff_threshold = read_config_data->rssi.conn_diff_threshold;
      configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }

    return conn_diff_threshold;
}
/****************************************************************************
NAME	
    sinkInquiryGetRssiConnThreshold

DESCRIPTION
    Get RSSI diff_threshold.
RETURNS
    uint16
    
*/
static uint16 sinkInquiryGetRssiConnThreshold(void)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    uint16 diff_threshold = 0;

    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
      diff_threshold = read_config_data->rssi.diff_threshold ;
      configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }
    return diff_threshold;
}
/****************************************************************************
NAME	
    sinkInquiryGetRssiThreshold

DESCRIPTION
    Get RSSI threshold.
RETURNS
    uint16
    
*/
uint16 sinkInquiryGetRssiThreshold(void)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    uint16 threshold = 0;

    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
      threshold = read_config_data->rssi.threshold;
      configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }
    return threshold;
}
/****************************************************************************
NAME	
    sinkInquiryConnectIfInPdl

DESCRIPTION
    Check if don't connect to an AG if not already in the PDL.
RETURNS
    Success of Failure
    
*/
static bool sinkInquiryConnectIfInPdl(void)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    bool connect_if_in_pdl = FALSE;

    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
      connect_if_in_pdl = read_config_data->rssi.connect_if_in_pdl;
      configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }
    return (connect_if_in_pdl)?TRUE:FALSE;
}
/****************************************************************************
NAME	
    sinkInquiryDonotPairIfInPdl

DESCRIPTION
    Check if don't pair with an AG if already in the PDL.
RETURNS
    Success of Failure
    
*/
static bool sinkInquiryDonotPairIfInPdl(void)
{
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;
    bool dont_pair_if_in_pdl = FALSE;

    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
      dont_pair_if_in_pdl = read_config_data->rssi.dont_pair_if_in_pdl;
      configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);
    }
    return (dont_pair_if_in_pdl)?TRUE:FALSE;
}
/****************************************************************************
NAME	
    sinkInquiryIsPairOnPdlReset

DESCRIPTION
    Check if Start RSSI pairing on PDL reset event.
RETURNS
    Success of Failure
    
*/
bool sinkInquiryIsPairOnPdlReset(void)
{
    bool pair_on_pdl_reset = FALSE;
    sink_inquiry_readonly_config_def_t *read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
        pair_on_pdl_reset = read_config_data->rssi.pair_on_pdl_reset;
        configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);  
    }
    return (pair_on_pdl_reset) ? TRUE:FALSE;
}
/*************************************************************************************/
#ifdef ENABLE_PEER
static bool performSdpSearch (const bdaddr *bd_addr, uint16 search_idx)
{
    INQ_DEBUG(("INQ: performSdpSearch "));
    if (search_idx < SUPPORTED_PROFILE_COUNT)
    {
        INQ_DEBUG(("- profile %u\n", service_searches[search_idx].profile));
        ConnectionSdpServiceSearchRequest((Task)&inquiry_task, bd_addr, 1, service_searches[search_idx].size, service_searches[search_idx].pattern);
        return TRUE;
    }
    else
    {
        INQ_DEBUG(("- completed\n"));
        return FALSE;
    }
}
#endif

#ifdef ENABLE_PEER
static void initiateSdpSearch (const bdaddr *bd_addr)
{
    INQ_DEBUG(("INQ: initiateSdpSearch "));
	/*ConnectionSmSetSdpSecurityOut(TRUE, bd_addr);*/ /* Allow SDP searches to be performed without locally enforcing authentication */
	ConnectionSdpOpenSearchRequest((Task)&inquiry_task, bd_addr);  /* Generates a CL_SDP_OPEN_SEARCH_CFM */
    
    GINQDATA.inquiry.remote_profiles = 0;
}
#endif

#ifdef ENABLE_PEER
static sink_link_type getFirstConnectableProfile (supported_profiles remote_profiles)
{
    INQ_DEBUG(("INQ: First connectable profile "));
    if (remote_profiles & (profile_hsp | profile_hfp))
    {
        INQ_DEBUG(("= hfp/hsp\n"));
        return sink_hfp;
    }
    else if (remote_profiles & profile_a2dp)
    {
        INQ_DEBUG(("= a2dp\n"));
        return sink_a2dp;
    }
    else
    {
        INQ_DEBUG(("= none\n"));
        return sink_none;
    }
}
#endif

#ifdef ENABLE_PEER
static void handleSdpOpenCfm (void)
{
    uint8 index = GINQDATA.inquiry.attempting;
    inquiry_result_t* device = &GINQDATA.inquiry.results[index];
    performSdpSearch(&device->bd_addr, GINQDATA.inquiry.profile_search_idx);
}

static void handleSdpCloseCfm (const CL_SDP_CLOSE_SEARCH_CFM_T *cfm)
{
    uint8 index = GINQDATA.inquiry.attempting;
    inquiry_result_t* device = &GINQDATA.inquiry.results[index];
    sink_link_type first_profile = getFirstConnectableProfile(GINQDATA.inquiry.remote_profiles);

    UNUSED(cfm);

    if (first_profile)
    {   /* Attempt connection */
        INQ_DEBUG(("INQ: slcConnectDevice\n"));
        slcConnectDevice(&device->bd_addr, first_profile);
    }
    else
    {   /* Device does not support a profile we can connect to */
        INQ_DEBUG(("INQ: inquiryConnectNext\n"));
        inquiryConnectNext();
    }
}

static void handleSdpServiceSearchAttributeCfm (const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    if(GINQDATA.inquiry.results)
    {
        /* We are inquiring, so no A2DP connect request has been issued yet to this device, thus there is no entry in a2dp_link_data */
        inquiry_result_t *inquiry_results = &GINQDATA.inquiry.results[GINQDATA.inquiry.attempting];
        
        
         /* Response is due to a request for the Peer Device service record */
        if ( (cfm->status==sdp_response_success) && !cfm->error_code )
        {   /* Response obtained, examine it */
            uint16 remote_peer_version = peerGetRemoteVersionFromServiceRecord(cfm->attributes, cfm->size_attributes);
            remote_features remote_peer_features = peerGetRemoteSupportedFeaturesFromServiceRecord(cfm->attributes, cfm->size_attributes);
            
            peerSetRemoteSupportedCodecsFromServiceRecord(cfm->attributes, cfm->size_attributes);
            peerSetRemoteRenderingMode(peerCheckRemoteRenderingMode(remote_peer_features));
            inquiry_results->peer_features = peerDetermineCompatibleRemoteFeatures(remote_peer_features, remote_peer_version);
            inquiry_results->peer_device = remote_device_peer;
            GINQDATA.inquiry.peer_version = remote_peer_version; /* Stored here to save RAM usage in theSink.inquiry.results[] */
        }
        else
        {
            inquiry_results->peer_features = remote_features_none;
            inquiry_results->peer_device = remote_device_nonpeer;
            GINQDATA.inquiry.peer_version = 0;   /* Stored here to save RAM usage in theSink.inquiry.results[] */
        }

        if (((inquiry_results->peer_device != remote_device_peer)&&(GINQDATA.inquiry.session != inquiry_session_peer)) ||                                                                 /* A non-Peer device */
            ((inquiry_results->peer_features != remote_features_none) && peerIsCompatibleDevice(GINQDATA.inquiry.peer_version) ) )   /* A Peer device     */
        {   /* Always allow non peer devices to connect.  Peer devices are restricted by version number and feature set */
            INQ_DEBUG(("INQ: Device compatible - inquiryConnect\n"));
            inquiryConnect(GINQDATA.inquiry.attempting);
        }
        else
        {   /* Device not suitable for connection */
            INQ_DEBUG(("INQ: Device NOT compatible - inquiryConnectNext\n"));
            inquiryConnectNext();
        }
    }
}

static void handleSdpServiceSearchCfm (const CL_SDP_SERVICE_SEARCH_CFM_T *cfm)
{
    if (cfm->status != sdp_search_busy)
    {
#ifdef DEBUG_PRINT_ENABLED
        uint16 n=cfm->size_records;
        uint16 i;
        INQ_DEBUG(("INQ: Sdp service search result [ "));
        for (i=0; i<n; i++)
        {
            INQ_DEBUG(("0x%X ",cfm->records[i]));
        }
        INQ_DEBUG(("]\n"));
#endif

        if (cfm->status == sdp_response_success)
        {
            INQ_DEBUG(("INQ: Device supports profile %u\n", service_searches[GINQDATA.inquiry.profile_search_idx].profile));
            GINQDATA.inquiry.remote_profiles |= service_searches[GINQDATA.inquiry.profile_search_idx].profile;
        }
        else
        {
            INQ_DEBUG(("INQ: Device does NOT support %u\n", service_searches[GINQDATA.inquiry.profile_search_idx].profile));
            GINQDATA.inquiry.remote_profiles &= ~service_searches[GINQDATA.inquiry.profile_search_idx].profile;
        }
        
        if ( !performSdpSearch(&cfm->bd_addr, ++GINQDATA.inquiry.profile_search_idx) )
        {
            ConnectionSdpCloseSearchRequest((Task)&inquiry_task);  /* Generates a CL_SDP_CLOSE_SEARCH_CFM */
        }
    }
}




/****************************************************************************
NAME	
	handleInquiryMessage
    
DESCRIPTION
    Task function to handle responses to SDP search requests made during RSSI inquiry
    
RETURNS
	void
*/
static void handleInquiryMessage (Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch (id)
    {
    case CL_SDP_OPEN_SEARCH_CFM:
        INQ_DEBUG(("INQ: CL_SDP_OPEN_SEARCH_CFM\n"));
        handleSdpOpenCfm();
        break;
    
    case CL_SDP_CLOSE_SEARCH_CFM:
        INQ_DEBUG(("INQ: CL_SDP_CLOSE_SEARCH_CFM\n"));
        handleSdpCloseCfm((const CL_SDP_CLOSE_SEARCH_CFM_T *) message);
        break;
    
    case CL_SDP_SERVICE_SEARCH_CFM:
        INQ_DEBUG(("INQ: CL_SDP_SERVICE_SEARCH_CFM\n"));
        handleSdpServiceSearchCfm ((const CL_SDP_SERVICE_SEARCH_CFM_T*) message);
        break;
    
    case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
        INQ_DEBUG(("INQ: CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM\n"));
        handleSdpServiceSearchAttributeCfm ((const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T*) message);
        break;
    
    default:
        INQ_DEBUG(("INQ: Unexpected message, id=0x%X\n", id));
        break;
    }
}
#endif  /* ENABLE_PEER */


/****************************************************************************
NAME    
    inquiryResetEntry
    
DESCRIPTION
    Reset inquiry data to initial values
RETURNS
    void
*/
static void inquiryResetEntry(uint8 index)
{
    BdaddrSetZero(&GINQDATA.inquiry.results[index].bd_addr);
    GINQDATA.inquiry.results[index].rssi = RSSI_THRESHOLD;
}


/****************************************************************************
NAME    
    inquiryReset
    
DESCRIPTION
    Reset inquiry data to initial values
RETURNS
    void
*/
void inquiryReset(void)
{
    uint8 index;
    INQ_DEBUG(("INQ: Reset\n"));
    if(GINQDATA.inquiry.results)
        for(index=0; index < NUM_INQ_RESULTS; index++)
            inquiryResetEntry(index);

    GINQDATA.inquiry.attempting = 0;
}


/****************************************************************************
NAME    
    inquiryResume
    
DESCRIPTION
    Reset inquiry results and resume
RETURNS
    void
*/
void inquiryResume(void)
{

    if(GINQDATA.inquiry.state == inquiry_idle)
    {
        inquiryReset();
        GINQDATA.inquiry.state = inquiry_searching;
        
        if (sinkInquiryIsInqSessionNormal())
        {
            sink_inquiry_readonly_config_def_t *inq_config_data;
            if (configManagerGetReadOnlyConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID, (const void **)&inq_config_data))
            {
                uint32 class_of_device = 0;
                class_of_device = PACK_UINT16_TO_UINT32(inq_config_data->rssi.cod_filter_lo,inq_config_data->rssi.cod_filter_hi);
                INQ_DEBUG(("INQ: Resume GIAC %d, %d, 0x%lX\n", inq_config_data->rssi.max_responses, inq_config_data->rssi.timeout, class_of_device));
                /* Using GIAC */
                ConnectionInquire(&theSink.task, GIAC, inq_config_data->rssi.max_responses, inq_config_data->rssi.timeout, class_of_device);
                configManagerReleaseConfig(SINK_INQUIRY_READONLY_CONFIG_BLK_ID);  
            }
        }
        else
        {
            if (peerUseLiacForPairing())
            {
                INQ_DEBUG(("INQ: Resume LIAC %d, %d, 0x%lX\n", 4, 46, AUDIO_MAJOR_SERV_CLASS | AV_MAJOR_DEVICE_CLASS ));
                /* Obtain upto 4 responses, using LIAC with 46*1.28sec=58.88sec timeout */
                ConnectionInquire(&theSink.task, LIAC, 4, 46, AUDIO_MAJOR_SERV_CLASS | AV_MAJOR_DEVICE_CLASS );
            }
            else
            {
                INQ_DEBUG(("INQ: Resume GIAC %d, %d, 0x%lX\n", 4, 46, AUDIO_MAJOR_SERV_CLASS | AV_MAJOR_DEVICE_CLASS ));
                /* Obtain upto 4 responses, using GIAC with 46*1.28sec=58.88sec timeout */
                ConnectionInquire(&theSink.task, GIAC, 4, 46, AUDIO_MAJOR_SERV_CLASS | AV_MAJOR_DEVICE_CLASS );
            }
        }
    }
}


/****************************************************************************
NAME    
    inquiryPair
    
DESCRIPTION
    Kick off inquiry and pairing
RETURNS
    bool
*/
bool inquiryPair (inquiry_session session, bool req_disc)
{
    INQ_DEBUG(("INQ: Pair %d\n", GINQDATA.inquiry.action));
    
    if(GINQDATA.inquiry.action == rssi_none)
    {
        if (sinkInquiryGetPagingCount())
        {
            /* Wait until pairing completes before starting inquiry */
            MAKE_INQ_MESSAGE(INQ_INQUIRY_PAIR_MSG);
            message->session = session;
            message->req_disc = req_disc;         
            MessageSendConditionally(&theSink.task, EventSysInquiryDelayedStart, message, sinkInquiryGetPagingCountPtr());       
    
            INQ_DEBUG(("INQ: Pair delayed\n"));
        }
        else
        {
            GINQDATA.inquiry.action = rssi_pairing;
            GINQDATA.inquiry.session = session;
            inquiryStart(req_disc);
        }
        
        return TRUE;
    }
    
    return FALSE;
}


/****************************************************************************
NAME    
    inquiryStart
    
DESCRIPTION
    Kick off inquiry
RETURNS
    void
*/
void inquiryStart (bool req_disc)
{
    uint16 InquiryTimeout_s = 0;
    InquiryTimeout_s = sinkInquiryGetInquiryTimeout();

    if(!GINQDATA.inquiry.results)
    {
        INQ_DEBUG(("INQ: Start\n" )) ;

        /*Restart APD timer to avoid APD timeout during inquiry*/
        sinkStartAutoPowerOffTimer();
        
        /* Go discoverable (and disconnect any active SLC) */
        if(GINQDATA.inquiry.action == rssi_pairing)
        {
           INQ_DEBUG(("INQ: RSSI Pairing\n" ));
           MessageCancelAll(&theSink.task, EventUsrEstablishSLC);
           stateManagerEnterConnDiscoverableState(req_disc);
        }
        slcReset();
        
        /* Allocate space to store inquiry results */
        GINQDATA.inquiry.results = (inquiry_result_t*)mallocPanic(NUM_INQ_RESULTS * sizeof(inquiry_result_t));
        GINQDATA.inquiry.state = inquiry_idle;

        /* Increase page timeout */
        ConnectionSetPageTimeout(16384);

        /* Start a periodic inquiry, this will keep going until we cancel */
        inquiryResume();

        /* Send a reminder event */
        MessageCancelAll(&theSink.task, EventSysRssiPairReminder);
        MessageSendLater(&theSink.task, EventSysRssiPairReminder, 0, D_SEC(INQUIRY_REMINDER_TIMEOUT_SECS));

        /* Send timeout if enabled */
        if(InquiryTimeout_s)
        {
            MessageCancelAll(&theSink.task, EventSysRssiPairTimeout);
            MessageSendLater(&theSink.task, EventSysRssiPairTimeout, 0, D_SEC(InquiryTimeout_s));
        }
    }
}


/****************************************************************************
NAME    
    inquiryStop
    
DESCRIPTION
    Stop inquiry in progress.
RETURNS
    void
*/
void inquiryStop(void)
{
    if (GINQDATA.inquiry.state != inquiry_connecting)
    {
    /* Free space used for inquiry results */
        if(GINQDATA.inquiry.results)
        {
            INQ_DEBUG(("INQ: Stopped\n"));
    
            freePanic(GINQDATA.inquiry.results);
            GINQDATA.inquiry.results = NULL;
            GINQDATA.inquiry.state = inquiry_idle;
    
            /* Continue standard connection procedure */
            if(RSSI_CONNECTING)
                MessageSend(&theSink.task, EventSysContinueSlcConnectRequest, 0);
            
            GINQDATA.inquiry.action = rssi_none;
        }
        
        /* Cancel current inquiry search */
        ConnectionInquireCancel(&theSink.task);
        MessageCancelFirst(&theSink.task, EventSysRssiResume);
        MessageCancelFirst(&theSink.task, EventSysRssiPairReminder);
        MessageCancelFirst(&theSink.task, EventSysRssiPairTimeout);
        MessageCancelFirst(&theSink.task, EventSysContinueSlcConnectRequest);
    
        /* Restore Page Timeout */
        ConnectionSetPageTimeout(0);
    }
        
    /* Only change state if we are not in Limbo */
    if (stateManagerGetState() != deviceLimbo)
    {
        if(deviceManagerNumConnectedDevs())
        {
            /* We connected, enter correct state */
            GINQDATA.inquiry.action = rssi_none;
            /* change to connected state if not in a call state */
            if (stateManagerGetState() < deviceConnected)
                stateManagerEnterConnectedState();

        }
        else
        {
            /*No connected devices, enter Connectable/Discoverable appropriately*/
            uint16 lNumDevices = ConnectionTrustedDeviceListSize();

            /* Check if we want to go discoverable */
            if ( lNumDevices < sinkDataGetPDLLimitForDiscovery())
            {
                GINQDATA.inquiry.session = inquiry_session_normal;
                stateManagerEnterConnDiscoverableState ( TRUE );
            }
        }
    }
}


/****************************************************************************
NAME    
    inquiryTimeout
    
DESCRIPTION
    Inquiry process has timed out, wait for connections in progress to 
    complete and stop inquiring
RETURNS
    void
*/
void inquiryTimeout(void)
{
    INQ_DEBUG(("INQ: Timeout - "));
    if(GINQDATA.inquiry.results)
    {
        /* Wait for connect attempts to complete */
        if(GINQDATA.inquiry.state == inquiry_connecting)
        {
            INQ_DEBUG(("Wait for SLC Complete\n"));
            GINQDATA.inquiry.state = inquiry_complete;
        }
        else
        {
            INQ_DEBUG(("Stop\n"));
            inquiryStop();
        }
    }
    else
    {
        INQ_DEBUG(("Stop\n"));
        inquiryStop();
    }
}


/****************************************************************************
NAME    
    inquiryCheckBdaddr
    
DESCRIPTION
    Helper function to check if an address is in inquiry results
*/
static bool inquiryCheckBdaddr(const bdaddr *bd_addr, uint8* index)
{
    /* Check all devices for a previous entry */
    for((*index) = 0; (*index) < NUM_INQ_RESULTS; (*index)++)
        if(BdaddrIsSame(bd_addr, &GINQDATA.inquiry.results[*index].bd_addr))
            return TRUE;
    return FALSE;
}


/****************************************************************************
NAME    
    inquiryGetIndex
    
DESCRIPTION
    Work out the correct index in inquiry data for a given RSSI value
*/
static void inquiryGetIndex(int16 rssi, uint8* index)
{
    for((*index) = 0; (*index) < NUM_INQ_RESULTS; (*index)++)
        if(rssi > GINQDATA.inquiry.results[*index].rssi)
            return;
    (*index)++;
}


/****************************************************************************
NAME    
    inquiryConnect
    
DESCRIPTION
    Connect to the specified inquiry result
    
RETURNS
    void
*/
static void inquiryConnect (uint8 index)
{
    rssi_pairing_t rssiConfig;
    sinkInquiryGetRssiConfig(&rssiConfig);
    
    INQ_DEBUG(("INQ: State [%d\n]", GINQDATA.inquiry.state));
    INQ_DEBUG(("INQ: Connecting [%d]\n", index));

    if(GINQDATA.inquiry.results)
    {
        if(index < rssiConfig.num_results || (rssiConfig.try_all_discovered && index < NUM_INQ_RESULTS))
        {
            inquiry_result_t* device = &GINQDATA.inquiry.results[index];
            INQ_DEBUG(("INQ: Address %04x,%02x,%06lx\n", device->bd_addr.nap, device->bd_addr.uap, device->bd_addr.lap));

            /* Check there's a valid result at position idx */
            if(!BdaddrIsZero(&device->bd_addr))
            {
                /* Allow 2 close devices if multipoint enabled, otherwise just one */
                int16 rssi =  GINQDATA.inquiry.results[sinkDataIsMultipointEnabled() ? 2 : 1].rssi;
                INQ_DEBUG(("INQ: RSSI %d Difference %d (%d)\n", device->rssi, (device->rssi - rssi), RSSI_DIFF_THRESHOLD));

                /* Check that difference threshold criteria are met */
                if(rssiConfig.try_all_discovered || (device->rssi - rssi) >= (int16)RSSI_DIFF_THRESHOLD)
                {
                    /* Don't continue connecting after timeout if try_all_discovered enabled */
                    if(!rssiConfig.try_all_discovered || (GINQDATA.inquiry.state != inquiry_complete))
                    {
                        INQ_DEBUG(("INQ: Connect\n"));
    
                        /* Ensure the link key for this device is deleted before the connection attempt,
                        prevents reconnection problems with BT2.1 devices.*/
                        if(GINQDATA.inquiry.action == rssi_pairing)
                            deviceManagerRemoveDevice(&device->bd_addr);
    
                        GINQDATA.inquiry.state = inquiry_connecting;
#ifdef ENABLE_PEER
                        if ((device->peer_device != remote_device_nonpeer) && 
                            (device->peer_features == remote_features_none) && 
                            (GINQDATA.inquiry.session == inquiry_session_peer))
                        {   
                            /* A Peer device with unknown features or an unknown device (also with unknown features) */
                            RequestRemotePeerServiceRecord((Task)&inquiry_task, &device->bd_addr);
                        }
                        else if (!device->remote_profiles)
                        {   /* Remote device supported profiles not contained in EIR data - search using SDP */
                            initiateSdpSearch(&device->bd_addr);
                        }
                        else
                        {
                            sink_link_type first_profile = getFirstConnectableProfile(device->remote_profiles);
                            
                            GINQDATA.inquiry.remote_profiles = device->remote_profiles;   /* TODO: Refactor inquiry.remote_profiles out */
                                
                            if (first_profile)
                            {   /* Device supports a profile we can connect to - attempt connection */
                                INQ_DEBUG(("INQ: slcConnectDevice\n"));
                                slcConnectDevice(&device->bd_addr, first_profile);
                            }
                            else
                            {   /* TODO: Temporary mechanism until this routine is modified to allow for repetition */
                                /* Issue a connect request for HFP */
                                slcConnectDevice(&device->bd_addr, sink_hfp);
                            }
                        }
#else          
                        /* Issue a connect request for HFP */
                        slcConnectDevice(&device->bd_addr, sink_hfp);
#endif     
                        return;
                    }
                }
            }
        }

        /* We're no longer attempting a connection */
        GINQDATA.inquiry.attempting = 0;

        /* If complete, connecting or pairing with second AG failed - stop, otherwise - resume inquiry */
        if (RSSI_CONNECTING ||(GINQDATA.inquiry.state == inquiry_complete) ||
            (deviceManagerNumConnectedDevs() && sinkInquiryIsInqSessionNormal()))
        {
            inquiryStop();
        }
        else
        {
            GINQDATA.inquiry.state = inquiry_idle;
            MessageSendLater(&theSink.task, EventSysRssiResume, 0, D_SEC(rssiConfig.resume_timeout));
        }
    }    
}

static void inquiryConnectFirst(void)
{
    GINQDATA.inquiry.attempting = 0;
    inquiryConnect(GINQDATA.inquiry.attempting);
}

inquiry_result_t* inquiryGetConnectingDevice (void)
{
    inquiry_result_t* connecting_device = NULL;

    if(GINQDATA.inquiry.results != NULL)
    {
        connecting_device = &GINQDATA.inquiry.results[GINQDATA.inquiry.attempting];
    }

    return connecting_device;
}
void inquiryConnectNext(void)
{
    inquiryConnect(++GINQDATA.inquiry.attempting);
}

/****************************************************************************
NAME
    sinkInquirySetInquirySession

DESCRIPTION
    Set Inquiry Session.
RETURNS
    void
****************************************************************************/
void sinkInquirySetInquirySession(inquiry_session session)
{
     GINQDATA.inquiry.session = session;
}
/****************************************************************************
NAME
    sinkInquirySetInquiryAction

DESCRIPTION
    Set Inquiry Action.
RETURNS
    void
****************************************************************************/
void sinkInquirySetInquiryAction(inquiry_action action)
{
     GINQDATA.inquiry.action= action;
}
/****************************************************************************
NAME
    sinkInquirySetInquiryState

DESCRIPTION
    Set Inquiry State.
RETURNS
    void
****************************************************************************/
void sinkInquirySetInquiryState(inquiry_state state)
{
     GINQDATA.inquiry.state = state;
}
/****************************************************************************
NAME
    sinkInquiryGetInquiryState

DESCRIPTION
    Get Inquiry State.
    
RETURNS
    Inquiry state
    
****************************************************************************/
inquiry_state sinkInquiryGetInquiryState(void)
{
    return GINQDATA.inquiry.state;
}
/****************************************************************************
NAME
    sinkInquiryGetProfileSearchIdx

DESCRIPTION
    Get Profile Search Index.
RETURNS
    uint8
****************************************************************************/
uint8 sinkInquiryGetProfileSearchIdx(void)
{
      return GINQDATA.inquiry.profile_search_idx;
}
/****************************************************************************
NAME
    sinkInquiryResetInquiryResults

DESCRIPTION
    Reset Inquiry results.
RETURNS
    void
****************************************************************************/
void sinkInquiryResetInquiryResults(void)
{
     GINQDATA.inquiry.results = NULL; 
}
/****************************************************************************
NAME
    sinkInquiryAllocInquiryResults

DESCRIPTION
    Allocate memory for inquiry result.
RETURNS
    void
****************************************************************************/
void sinkInquiryAllocInquiryResults(uint8 size_of_data)
{
    GINQDATA.inquiry.results = (inquiry_result_t *)PanicNull(malloc(size_of_data * sizeof(inquiry_result_t)));
    memset(GINQDATA.inquiry.results, 0, (size_of_data * sizeof(inquiry_result_t)));
}

/****************************************************************************
NAME
    sinkInquiryAddDeviceBdaddr

DESCRIPTION
     Add the device Bdaddr to inquiry results.
****************************************************************************/
void sinkInquiryAddDeviceBdaddr(uint8 counter,  bdaddr bd_addr )
{
     GINQDATA.inquiry.results[counter].bd_addr = bd_addr;
}
/****************************************************************************
NAME
    sinkInquiryAddDeviceRssi

DESCRIPTION
     Add the device RSSI to inquiry results.
****************************************************************************/
void sinkInquiryAddDeviceRssi(uint8 counter, int16 rssi )
{
     GINQDATA.inquiry.results[counter].rssi = rssi;
}
/****************************************************************************
NAME
    sinkInquirySetInquiryTx

DESCRIPTION
     Set Inquiry TX power
****************************************************************************/
void sinkInquirySetInquiryTx(int8 tx_power)
{
     GINQDATA.inquiry_tx = tx_power;
}
/****************************************************************************
NAME
    sinkInquiryGetInquiryTxPower

DESCRIPTION
     Get Inquiry TX power
RETURNS
     int8
****************************************************************************/
int8 sinkInquiryGetInquiryTxPower(void)
{
    return GINQDATA.inquiry_tx;
}
/****************************************************************************
NAME
    sinkInquiryReleaseAndResetInquiryResults

DESCRIPTION
    Release and reset inquiry results.
****************************************************************************/
void sinkInquiryReleaseAndResetInquiryResults(void)
{
     free(GINQDATA.inquiry.results);
     GINQDATA.inquiry.results = NULL;
}
/****************************************************************************
NAME
    sinkInquiryResetInquiryAttempt

DESCRIPTION
    Reset inquiry attempt.
****************************************************************************/
void sinkInquiryResetInquiryAttempt(void)
{
     GINQDATA.inquiry.attempting = 0;
}
/****************************************************************************
NAME
    sinkInquirySetInquiryAttempt

DESCRIPTION
    Set inquiry attempt.
****************************************************************************/
void sinkInquirySetInquiryAttempt(uint8 attempting)
{
     GINQDATA.inquiry.attempting = attempting; 
}
/****************************************************************************
NAME
    sinkinquiryGetInquiryResults

DESCRIPTION
    Get inquiry results.
RETURNS
    Pointer to inquiry_result_t.
****************************************************************************/
inquiry_result_t* sinkinquiryGetInquiryResults(void)
{
     return GINQDATA.inquiry.results;
}

/****************************************************************************
NAME	
    sinkInquiryCheckInqActionSub
DESCRIPTION
    This interfaces could be used to check inquiry action is RSSI sub or not 
RETURNS
    bool: TRUE if action is RSSI sub else FALSE
****************************************************************************/
bool sinkInquiryCheckInqActionSub(void)
{
    return (GINQDATA.inquiry.action  == rssi_subwoofer) ? TRUE:FALSE;
}

/****************************************************************************
NAME	
    sinkInquiryCheckInqActionRssi
DESCRIPTION
    This interfaces could be used to check inquiry action is RSSI 
RETURNS
    bool: TRUE if action is RSSI  else FALSE
****************************************************************************/
bool sinkInquiryCheckInqActionRssi(void)
{
    return  (GINQDATA.inquiry.action == rssi_pairing)?TRUE:FALSE;
}

/****************************************************************************
NAME	
    sinkInquiryCheckInqActionRssiNone
DESCRIPTION
    This interfaces could be used to check inquiry action is RSSI none
RETURNS
    bool: TRUE if action is RSSI none else FALSE
****************************************************************************/
bool sinkInquiryCheckInqActionRssiNone(void)
{
    return (GINQDATA.inquiry.action == rssi_none) ? TRUE:FALSE;
}

/****************************************************************************
NAME	
    sinkInquiryIsInqSessionNormal
DESCRIPTION
    This interfaces could be used to check Inquiry session is normal or not
RETURNS
    bool: TRUE if session is normal  else FALSE
****************************************************************************/
bool sinkInquiryIsInqSessionNormal(void)
{
    return (GINQDATA.inquiry.session == inquiry_session_normal) ? TRUE:FALSE;
}

/****************************************************************************
NAME	
    sinkInquiryIsInqSessionPeer
DESCRIPTION
    This interfaces could be used to check Inquiry session is peer or not
RETURNS
    bool: TRUE if session is peer else FALSE
****************************************************************************/
bool sinkInquiryIsInqSessionPeer(void)
{
    return (GINQDATA.inquiry.session == inquiry_session_peer) ? TRUE:FALSE;
}

/****************************************************************************
NAME	
    sinkInquiryGetConnAttemptingIndex
DESCRIPTION
    This interfaces could be used to get the connection attempting index
RETURNS
    uint8: connection attempting index
****************************************************************************/
uint8 sinkInquiryGetConnAttemptingIndex(void)
{
    return GINQDATA.inquiry.attempting;
}

/****************************************************************************
NAME
    sinkInquiryClearPagingCount

DESCRIPTION
    Zero the count of outstanding paging attempts
    
RETURNS
    void
    
****************************************************************************/
void sinkInquiryClearPagingCount(void)
{
    GINQDATA.paging_count = 0;
    INQ_DEBUG(("INQ: clr pc\n"));
}

/****************************************************************************
NAME
    sinkInquiryDecrementPagingCount

DESCRIPTION
    Decrement the count of outstanding paging attempts if non-zero
    
RETURNS
    void
    
****************************************************************************/
void sinkInquiryDecrementPagingCount(void)
{
    if (GINQDATA.paging_count)
    {
        --GINQDATA.paging_count;
    }
    
    INQ_DEBUG(("INQ: dec pc %u\n", GINQDATA.paging_count));
}

/****************************************************************************
NAME
    sinkInquiryIncrementPagingCount

DESCRIPTION
    Increment the count of outstanding paging attempts
    
RETURNS
    void
    
****************************************************************************/
void sinkInquiryIncrementPagingCount(void)
{
    ++GINQDATA.paging_count;
    INQ_DEBUG(("INQ: inc pc %u\n", GINQDATA.paging_count));
}

/****************************************************************************
NAME
    sinkInquiryGetPagingCount

DESCRIPTION
    Gets the count of outstanding paging attempts
    
RETURNS
    Number of outstanding paging attempts
    
****************************************************************************/
uint16 sinkInquiryGetPagingCount(void)
{
    return GINQDATA.paging_count;
}

/****************************************************************************
NAME
    sinkInquiryGetPagingCountPtr

DESCRIPTION
    Gets a pointer to the count of outstanding paging attempts
    The pointer is suitable to be passed to MessageSendConditionally()
    
RETURNS
    Pointer to uint16 number of outstanding paging attempts
    
****************************************************************************/
uint16 *sinkInquiryGetPagingCountPtr(void)
{
    return &GINQDATA.paging_count;
}

/****************************************************************************
NAME
    sinkInquiryIsstateConnecting
DESCRIPTION
    This interfaces could be used to check if an inquiring result is currently 
    attempting connection to one of the results
RETURNS
    bool: TRUE if state is connecting else FALSE
****************************************************************************/
bool sinkInquiryIsStateConnecting(void)
{
    return (GINQDATA.inquiry.state == inquiry_connecting) ? TRUE:FALSE;
}

#ifdef ENABLE_PEER

/****************************************************************************
NAME
    sinkInquiryGetPeerInqResult
DESCRIPTION
    This interfaces could be used to get peer devcie inquiry result
RETURNS
    uint8: Inq result
****************************************************************************/
uint8 sinkInquiryGetPeerInqResult(uint8 index)
{
    return GINQDATA.inquiry.results[index].peer_device;
}

/****************************************************************************
NAME	
    sinkInquiryGetPeerFeatures
DESCRIPTION
    This interfaces could be used to get peer devcie features in inquiry result
RETURNS
    uint8: Features
****************************************************************************/
uint8 sinkInquiryGetPeerFeatures(uint8 index) 
{
    return GINQDATA.inquiry.results[index].peer_features;
}

/****************************************************************************
NAME	
    sinkInquiryGetPeerVersion
DESCRIPTION
    This interfaces could be used to get peer version
RETURNS
    uint16: Peer version
****************************************************************************/
uint16 sinkInquiryGetPeerVersion(void)
{
    return GINQDATA.inquiry.peer_version;
}
#endif /* ENABLE_PEER*/

/****************************************************************************/
#ifdef ENABLE_PEER
static supported_profiles getEirRemoteProfiles (uint16 size_eir_data, const uint8 *eir_data)
{
    uint16 i;
    INQ_DEBUG(("\n"));
    for (i=0; i<size_eir_data; i++) { INQ_DEBUG(("0x%X ",eir_data[i])); }
    INQ_DEBUG(("\n"));
    
    INQ_DEBUG(("INQ: EIR Data Size = %u\n",size_eir_data));
    
    while (size_eir_data >= 4)  /* Min possible size for a 16-bit service class UUID list EIR record */
    {
        uint16 eir_record_size = eir_data[0] + 1;   /* Record size in eir_data[0] does not include length byte, just tag and data size */
    
        INQ_DEBUG(("INQ: EIR Record Size = %u, Tag = 0x%X\n",eir_record_size, eir_data[1]));
        if ((eir_data[1] == 0x02) || (eir_data[1] == 0x03))     /* Partial or complete list of 16-bit service class UUIDs */
        {
            supported_profiles remote_profiles = profile_none;
            
            do
            {
                eir_record_size -= 2;
                eir_data += 2;

                INQ_DEBUG(("INQ: EIR Service UUID:0x%X\n",((eir_data[1]<<8) + eir_data[0])));

                /* Attempt to match next service uuid in EIR record */
                switch ((eir_data[1]<<8) + eir_data[0])
                {
                case 0x110A:    /* A2DP Audio Source */
                    remote_profiles |= profile_a2dp;
                    break;
                case 0x110C:    /* AVRCP Target */
                    remote_profiles |= profile_avrcp;
                    break;
                case 0x1112:    /* HSP AG */
                    remote_profiles |= profile_hsp;
                    break;
                case 0x111F:    /* HFP AG */
                    remote_profiles |= profile_hfp;
                    break;
                }
            }
            while (eir_record_size);
            
            return remote_profiles;
        }
        
        if (size_eir_data > eir_record_size)  
        {
            size_eir_data -= eir_record_size;
            eir_data += eir_record_size;
        }
        else
        {
            size_eir_data = 0;
        }
    }
    
    return profile_none;
}
#endif

/****************************************************************************
NAME    
    inquiryHandleResult
    
DESCRIPTION
    Inquiry result received
RETURNS
    void
*/
void inquiryHandleResult( CL_DM_INQUIRE_RESULT_T* result )
{
#ifdef ENABLE_SUBWOOFER
    /* Is the inquiry action searching for a subwoofer device? */
    if (sinkInquiryCheckInqActionSub())
    {
        handleSubwooferInquiryResult(result);
        return; /* Nothing more to do here as the inquiry result has been handled */
    }
#endif
    
    /* Check inquiry data is valid (if not we must have cancelled) */
    if(GINQDATA.inquiry.results)
    {
#ifdef DEBUG_INQ
        uint8 debug_idx;
        INQ_DEBUG(("INQ: Inquiry Result %x Addr %04x,%02x,%06lx RSSI: %d\n", result->status,
                                                                       result->bd_addr.nap,
                                                                       result->bd_addr.uap,
                                                                       result->bd_addr.lap, 
                                                                       result->rssi )) ;

        for(debug_idx=0; debug_idx<NUM_INQ_RESULTS; debug_idx++)
            INQ_DEBUG(("INQ: [Addr %04x,%02x,%06lx RSSI: %d]\n", GINQDATA.inquiry.results[debug_idx].bd_addr.nap,
                                                                 GINQDATA.inquiry.results[debug_idx].bd_addr.uap,
                                                                 GINQDATA.inquiry.results[debug_idx].bd_addr.lap, 
                                                                 GINQDATA.inquiry.results[debug_idx].rssi )) ;
#endif
        if(result->status == inquiry_status_result)
        {
#ifdef ENABLE_PEER
            remote_device peer_device = remote_device_unknown;
            
            if (peerUseDeviceIdRecord())
            {   /* Check for a peer device by matching device id records */
                if (CheckEirDeviceIdData(result->size_eir_data, result->eir_data))
                {   /* Mark device as a peer */
                    INQ_DEBUG(("INQ: Matched device id record on remote device\n"));
                    peer_device = remote_device_peer;
                    result->rssi += 0x100; /* Bump rssi value by maxiumum possible range so peer devices will be at top of sorted list */
                }
                else
                {
                    INQ_DEBUG(("INQ: Did NOT match device id record on remote device\n"));
                    peer_device = remote_device_nonpeer;
                }
            }
#endif
            
#ifdef ENABLE_PEER
            /* Filter out peer/non-peer devices depending on inquiry session */
            INQ_DEBUG(("INQ:session=%u device=%u\n", GINQDATA.inquiry.session, peer_device));
            if (((sinkInquiryIsInqSessionPeer()) && (peer_device != remote_device_nonpeer)) || 
                ((sinkInquiryIsInqSessionNormal()) && (peer_device != remote_device_peer)))
#endif
            {
                /* Check if device is in PDL */
                INQ_DEBUG(("RSSI_CHECK_PDL = %u\n",RSSI_CHECK_PDL(&result->bd_addr)));
                if(RSSI_CHECK_PDL(&result->bd_addr))
                {
                    uint8 old_index;
                    uint8 new_index;
                    inquiry_result_t res;
                    res.bd_addr = result->bd_addr;
                    res.rssi = result->rssi;
#ifdef ENABLE_PEER
                    res.peer_device = peer_device;
                    res.peer_features = remote_features_none;
                    res.remote_profiles = getEirRemoteProfiles(result->size_eir_data, result->eir_data);
                    INQ_DEBUG(("INQ: EIR Remote Profiles = %u\n",res.remote_profiles));
#endif
                    inquiryGetIndex(res.rssi, &new_index);
                    INQ_DEBUG(("INQ: new_index = %u\n",new_index));
                    
                    /* Check if an entry exists for this device */
                    if(inquiryCheckBdaddr(&res.bd_addr, &old_index))
                    {
                        /* Don't update if new entry further down the list */
                        if(new_index > old_index)
                            return;
                        /* Reset the old entry (it should fall out the bottom) */
                        inquiryResetEntry(old_index);
                    }

                    /* While new index is valid  */
                    while(new_index < NUM_INQ_RESULTS)
                    {
                        inquiry_result_t prev;
                        /* Remember the previous result for this index */
                        prev = GINQDATA.inquiry.results[new_index];
                        /* Put this result in its place */
                        GINQDATA.inquiry.results[new_index] = res;
                        res = prev;
                        /* Get the new index of previous result */
                        inquiryGetIndex(res.rssi, &new_index);
                    }
                }
            }
        }
        else
        {
            INQ_DEBUG(("INQ: Inquiry Complete\n"));
            /* Attempt to connect to device */
            inquiryConnectFirst();
        }
    }
}
