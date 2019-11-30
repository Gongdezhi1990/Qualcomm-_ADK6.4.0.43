/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_INQUIRY_H_
#define _SINK_INQUIRY_H_

#include <bdaddr.h>
#include <connection_no_ble.h>
#include "sink_inquiry_config_def.h"

typedef enum
{
    remote_device_unknown,
    remote_device_nonpeer,
    remote_device_peer
} remote_device;


typedef enum
{
    remote_features_none                        = 0x00,
    remote_features_shareme_a2dp_sink           = 0x01,
    remote_features_shareme_a2dp_source         = 0x02,
    remote_features_tws_a2dp_sink               = 0x04,
    remote_features_tws_a2dp_source             = 0x08,
    remote_features_peer_avrcp_target           = 0x10,
    remote_features_peer_avrcp_controller       = 0x20,
    remote_features_peer_mono_channel_supported = 0x40,
    remote_features_peer_sink                   = (remote_features_shareme_a2dp_sink | remote_features_tws_a2dp_sink),
    remote_features_peer_source                 = (remote_features_shareme_a2dp_source | remote_features_tws_a2dp_source),
    remote_features_peer_avrcp                  = (remote_features_peer_avrcp_target | remote_features_peer_avrcp_controller)
} remote_features;


/* Inquiry action */
typedef enum
{
    profile_none  = 0x00,
    profile_hsp   = 0x01,
    profile_hfp   = 0x02,
    profile_a2dp  = 0x04,
    profile_avrcp = 0x08
} supported_profiles;

#define SUPPORTED_PROFILE_COUNT 4


/* Inquiry session */
typedef enum
{
    inquiry_session_normal,   /* Normal inquiry session, for finding AGs etc */
    inquiry_session_peer      /* Peer inquiry session, for finding peer devices */
} inquiry_session;

/* Inquiry action */
typedef enum
{
    rssi_none,
    rssi_pairing,
    rssi_connecting,
    rssi_subwoofer
} inquiry_action;

/* Inquiry state */
typedef enum
{
    inquiry_idle,
    inquiry_searching,
    inquiry_connecting,
    inquiry_complete
} inquiry_state;

/* Radio configuration data */
typedef struct
{
    uint16  page_scan_interval;
    uint16  page_scan_window;
    uint16  inquiry_scan_interval;
    uint16  inquiry_scan_window;
}radio_config_type;

typedef struct
{
    inquiry_session session;
    bool req_disc;
} INQ_INQUIRY_PAIR_MSG_T;

/* Inquiry result */
typedef struct
{
    bdaddr bd_addr;                         /* Address of device */
    int16  rssi;                            /* Highest received signal strength indication from device */
#ifdef ENABLE_PEER
    unsigned           unused:3;            /* Unused */ 
    remote_device      peer_device:2;       /* Indicates that the remote device is of the same type as the local one */
    remote_features    peer_features:7;     /* Bitmask of Peer device features supported on remote device */
    supported_profiles remote_profiles:4;   /* Bitmask of profiles supported by a remote device */
#endif
}inquiry_result_t;

typedef struct
{
    inquiry_session    session:1;             /* Inquiry session (normal/peer)       */
    inquiry_action     action:2;              /* Inquiry Action (pairing/connecting)  */
    inquiry_state      state:2;               /* Inquiry State (searching/connecting) */
    unsigned           attempting:4;          /* Index of device being connected to   */
    supported_profiles remote_profiles:4;     /* Bitmask of profiles supported by a remote device */
    unsigned           profile_search_idx:3;  /* Index of current sdp service search */
    uint16             peer_version;          /* Indicates the Peer Device support version number of the device being connected to (as per 'attempting' field) */ 
    inquiry_result_t*  results;               /* Array of inquiry results             */
}inquiry_data_t;

/* Inquiry reminder timeout */
#define INQUIRY_REMINDER_TIMEOUT_SECS 5
#define INQUIRY_ON_PDL_RESET (sinkInquiryIsPairOnPdlReset())

/****************************************************************************
NAME    
    inquiryReset
    
DESCRIPTION
    Reset inquiry data to initial values
RETURNS
    void
*/
void inquiryReset( void );


/****************************************************************************
NAME    
    inquiryResume
    
DESCRIPTION
    Reset inquiry results and resume
RETURNS
    void
*/
void inquiryResume(void);


/****************************************************************************
NAME    
    inquiryPair
    
DESCRIPTION
    Kick off inquiry and pairing
RETURNS
    bool
*/
bool inquiryPair( inquiry_session session, bool req_disc );


/****************************************************************************
NAME    
    inquiryStart
    
DESCRIPTION
    Kick off Inquiry
RETURNS
    void
*/
void inquiryStart( bool req_disc );


/****************************************************************************
NAME    
    inquiryStop
    
DESCRIPTION
    Stop inquiry in progress.
RETURNS
    void
*/
void inquiryStop( void );


/****************************************************************************
NAME    
    inquiryTimeout
    
DESCRIPTION
    Inquiry process has timed out, wait for connections in progress to 
    complete and stop inquiring
RETURNS
    void
*/
void inquiryTimeout(void);


/****************************************************************************
NAME    
    inquiryConnectNext
    
DESCRIPTION
    Helper function to connect to next inquiry result
RETURNS
    void
*/
void inquiryConnectNext(void);


/****************************************************************************
NAME    
    inquiryHandleResult
    
DESCRIPTION
    Inquiry result received
RETURNS
    void
*/
void inquiryHandleResult( CL_DM_INQUIRE_RESULT_T* result );


/****************************************************************************
NAME    
    inquiryGetConnectingDevice
    
DESCRIPTION
    Returns the details of the device being attempted to connect following the inquiry procedure.
RETURNS
    void
*/
inquiry_result_t* inquiryGetConnectingDevice (void);

/****************************************************************************
NAME
    sinkInquirySetInquirySession

DESCRIPTION
    Set Inquiry Session.
RETURNS
    void
****************************************************************************/
void sinkInquirySetInquirySession(inquiry_session session);
/****************************************************************************
NAME
    sinkInquirySetInquiryAction

DESCRIPTION
    Set Inquiry Action.
RETURNS
    void
****************************************************************************/
void sinkInquirySetInquiryAction(inquiry_action action);
/****************************************************************************
NAME
    sinkInquirySetInquiryState

DESCRIPTION
    Set Inquiry State.
RETURNS
    void
****************************************************************************/
void sinkInquirySetInquiryState(inquiry_state state);
/****************************************************************************
NAME
    sinkInquiryGetInquiryState

DESCRIPTION
    Get Inquiry State.
    
RETURNS
    Inquiry state
    
****************************************************************************/
inquiry_state sinkInquiryGetInquiryState(void);
/****************************************************************************
NAME
    sinkInquiryGetProfileSearchIdx

DESCRIPTION
    Get Profile Search Index.
RETURNS
    uint8
****************************************************************************/
uint8 sinkInquiryGetProfileSearchIdx(void);
/****************************************************************************
NAME
    sinkInquiryResetInquiryResults

DESCRIPTION
    Reset Inquiry results.
RETURNS
    void
****************************************************************************/
void sinkInquiryResetInquiryResults(void);
/****************************************************************************
NAME
    sinkInquiryAllocInquiryResults

DESCRIPTION
    Allocate memory for inquiry result.
RETURNS
    void
****************************************************************************/
void sinkInquiryAllocInquiryResults(uint8 size_of_data);
/****************************************************************************
NAME
    sinkInquiryAddDeviceBdaddr

DESCRIPTION
     Add the device Bdaddr to inquiry results.
****************************************************************************/
void sinkInquiryAddDeviceBdaddr(uint8 counter,  bdaddr bd_addr );
/****************************************************************************
NAME
    sinkInquiryAddDeviceRssi

DESCRIPTION
     Add the device RSSI to inquiry results.
****************************************************************************/
void sinkInquiryAddDeviceRssi(uint8 counter, int16 rssi );
/****************************************************************************
NAME
    sinkInquirySetInquiryTx

DESCRIPTION
     Set Inquiry TX power
****************************************************************************/
void sinkInquirySetInquiryTx(int8 tx_power);
/****************************************************************************
NAME
    sinkInquiryGetInquiryTxPower

DESCRIPTION
     Get Inquiry TX power
RETURNS
     int8
****************************************************************************/
int8 sinkInquiryGetInquiryTxPower(void);
/****************************************************************************
NAME
    sinkInquiryReleaseAndResetInquiryResults

DESCRIPTION
    Release and reset inquiry results.
****************************************************************************/
void sinkInquiryReleaseAndResetInquiryResults(void);
/****************************************************************************
NAME
    sinkInquiryResetInquiryAttempt

DESCRIPTION
    Reset inquiry attempt.
****************************************************************************/
void sinkInquiryResetInquiryAttempt(void);
/****************************************************************************
NAME
    sinkInquirySetInquiryAttempt

DESCRIPTION
    Set inquiry attempt.
****************************************************************************/
void sinkInquirySetInquiryAttempt(uint8 attempting);
/****************************************************************************
NAME
    sinkinquiryGetInquiryResults

DESCRIPTION
    Get inquiry results.
RETURNS
    Pointer to inquiry_result_t.
****************************************************************************/
inquiry_result_t* sinkinquiryGetInquiryResults(void);
/****************************************************************************
NAME	
    sinkInquiryGetInquiryTimeout

DESCRIPTION
    Get InquiryTimeout_s.
RETURNS
    uint16
****************************************************************************/
uint16 sinkInquiryGetInquiryTimeout(void);
/****************************************************************************
NAME	
    sinkInquirySetInquiryTimeout

DESCRIPTION
    Set InquiryTimeout_s.
RETURNS
    bool
****************************************************************************/
bool sinkInquirySetInquiryTimeout(uint16 timeout);
/****************************************************************************
NAME	
    sinkInquiryGetRssiThreshold

DESCRIPTION
    Get RSSI threshold.
RETURNS
    uint16
****************************************************************************/
uint16 sinkInquiryGetRssiThreshold(void);
/****************************************************************************
NAME	
    sinkInquiryIsPairOnPdlReset

DESCRIPTION
    Check if Start RSSI pairing on PDL reset event.
RETURNS
    Success of Failure
****************************************************************************/
bool sinkInquiryIsPairOnPdlReset(void);
/****************************************************************************
NAME	
    sinkInquiryGetRadioConfig

DESCRIPTION
    This interfaces could be used to get the radio_config_type 
    
RETURNS
    void
****************************************************************************/
void sinkInquiryGetRadioConfig(radio_config_type *radio);

/****************************************************************************
NAME	
    sinkInquiryCheckInqActionSub

DESCRIPTION
    This interfaces could be used to check inquiry action is RSSI sub or not 
    
RETURNS
    bool: TRUE if action is RSSI sub else FALSE
****************************************************************************/
bool sinkInquiryCheckInqActionSub(void);

/****************************************************************************
NAME	
    sinkInquiryCheckInqActionRssi

DESCRIPTION
    This interfaces could be used to check inquiry action is RSSI 
    
RETURNS
    bool: TRUE if action is RSSI  else FALSE
****************************************************************************/
bool sinkInquiryCheckInqActionRssi(void);

/****************************************************************************
NAME	
    sinkInquiryCheckInqActionRssiNone

DESCRIPTION
    This interfaces could be used to check inquiry action is RSSI none
    
RETURNS
    bool: TRUE if action is RSSI none else FALSE
****************************************************************************/
bool sinkInquiryCheckInqActionRssiNone(void);

/****************************************************************************
NAME	
    sinkInquiryIsInqSessionNormal

DESCRIPTION
    This interfaces could be used to check Inquiry session is normal or not
    
RETURNS
    bool: TRUE if session is normal  else FALSE
****************************************************************************/
bool sinkInquiryIsInqSessionNormal(void);

/****************************************************************************
NAME	
    sinkInquiryIsInqSessionPeer

DESCRIPTION
    This interfaces could be used to check Inquiry session is peer or not
    
RETURNS
    bool: TRUE if session is peer else FALSE
****************************************************************************/
bool sinkInquiryIsInqSessionPeer(void);

/****************************************************************************
NAME	
    sinkInquiryGetConnAttemptingIndex

DESCRIPTION
    This interfaces could be used to get the connection attempting index
    
RETURNS
    uint8: connection attempting index
****************************************************************************/
uint8 sinkInquiryGetConnAttemptingIndex(void);

/****************************************************************************
NAME
    sinkInquiryIsstateConnecting
DESCRIPTION
    This interface could be used to check if an inquiring result is currently 
    attempting connection to one of the results
RETURNS
    bool: TRUE if state is connecting else FALSE
****************************************************************************/
bool sinkInquiryIsStateConnecting(void);

/****************************************************************************
NAME
    sinkInquiryClearPagingCount

DESCRIPTION
    Zero the count of outstanding paging attempts
    
RETURNS
    void
    
****************************************************************************/
void sinkInquiryClearPagingCount(void);

/****************************************************************************
NAME
    sinkInquiryDecrementPagingCount

DESCRIPTION
    Decrement the count of outstanding paging attempts
    
RETURNS
    void
    
****************************************************************************/
void sinkInquiryDecrementPagingCount(void);

/****************************************************************************
NAME
    sinkInquiryIncrementPagingCount

DESCRIPTION
    Increment the count of outstanding paging attempts
    
RETURNS
    void
    
****************************************************************************/
void sinkInquiryIncrementPagingCount(void);

/****************************************************************************
NAME
    sinkInquiryGetPagingCount

DESCRIPTION
    Gets the count of outstanding paging attempts
    
RETURNS
    Number of outstanding paging attempts
    
****************************************************************************/
uint16 sinkInquiryGetPagingCount(void);

/****************************************************************************
NAME
    sinkInquiryGetPagingCountPtr

DESCRIPTION
    Gets a pointer to the count of outstanding paging attempts
    The pointer is suitable to be passed to MessageSendConditionally()
    
RETURNS
    Pointer to uint16 number of outstanding paging attempts
    
****************************************************************************/
uint16 *sinkInquiryGetPagingCountPtr(void);

#ifdef ENABLE_PEER
/****************************************************************************
NAME
    sinkInquiryGetPeerInqResult

DESCRIPTION
    This interfaces could be used to get peer devcie inquiry result
    
RETURNS
    uint8: Inq result
****************************************************************************/
uint8 sinkInquiryGetPeerInqResult(uint8 index);

/****************************************************************************
NAME	
    sinkInquiryGetPeerFeatures

DESCRIPTION
    This interfaces could be used to get peer devcie features in inquiry result
    
RETURNS
    uint8: Features
****************************************************************************/
uint8 sinkInquiryGetPeerFeatures(uint8 index);

/****************************************************************************
NAME	
    sinkInquiryGetPeerVersion

DESCRIPTION
    This interfaces could be used to get peer version
    
RETURNS
    uint16: Peer version
****************************************************************************/
uint16 sinkInquiryGetPeerVersion(void);

#endif /* ENABLE_PEER*/

#endif /* _SINK_INQUIRY_H_ */

