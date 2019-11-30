/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_common.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_COMMON_H_
#define AVRCP_COMMON_H_


#include "avrcp_private.h"
#include "avrcp_signal_handler.h"
#include "avrcp_send_response.h"

/************************************************************************
 * Macros that changes the value of one or more of its paramaters
 ************************************************************************/
/* Use the START PKT Header to set Continuation packet header */
#define AVCTP_SET_CONT_PKT_HEADER(new,old)  do {\
          new = (uint8)(((old) & (~AVCTP0_PACKET_TYPE_MASK))|AVCTP0_PACKET_TYPE_CONTINUE);\
          } while (0)

/* Use Start packet header to set End packet header */
#define AVCTP_SET_END_PKT_HEADER(new, old)  do { \
            new =(uint8)(((old) & (~AVCTP0_PACKET_TYPE_MASK))| AVCTP0_PACKET_TYPE_END);\
            } while(0)

/* Generate the next transaction ID at CT Side */
#define AVRCP_NEXT_TRANSACTION(transaction) \
            (uint8)(((transaction)==0x0F)? 1 :(transaction)+1)

/* Check whether the incoming data is an allocated one */
#define AVRCP_UINT16_TO_UINT8(src,dest,offset) do {\
            (dest)[offset] = (uint8)((src) >> 8); \
            (dest)[(offset)+1] = (uint8)((src) & 0xFF); \
            } while (0)


#define AVRCP_UINT8_TO_UINT16(src,dest,offset) do {\
            (dest)= (uint16)(((src)[offset] << 8) |  ((src)[offset+1] & 0xFF)); \
            } while (0)

/************************************************************************
 * Macros that sets the AVRCP task data.
 ************************************************************************/
/*  Update the avrcp state. */
#define avrcpSetState(avrcp,cur_state)  (avrcp->bitfields.state = cur_state)

/* update the avbp state */
#define avrcpSetAvbpState(avbp, cur_state) (avbp->bitfields.avbp_state = cur_state)

/*************************************************************************
 * Macros to check the expected avrcp task data settings.
 *************************************************************************/
/* TRUE if Player application settings are enabled in the application */
#define isAvrcpPlayerSettingsEnabled(avrcp) \
          ((avrcp->bitfields.local_target_features & AVRCP_PLAYER_APPLICATION_SETTINGS)\
                                        == AVRCP_PLAYER_APPLICATION_SETTINGS)

/* TRUE if the library is initialize as version 1.3 */
#define isAvrcpMetadataEnabled(avrcp) \
          (avrcp->bitfields.local_extensions & AVRCP_VERSION_1_3)

/* A successful SDP query sets the remote features */
#define isAvrcpSdpDone(avrcp) (avrcp->bitfields.remote_features)

/* TRUE if the task state is expected */
#define isAvrcpConnected(avrcp) (avrcp->bitfields.state == avrcpConnected) 
#define isAvbpConnecting(avbp)  (avbp->bitfields.avbp_state == avbpConnecting)
#define isAvbpConnected(avbp)   (avbp->bitfields.avbp_state == avbpConnected)

/* Accessing browsing channel from APIs require NULL check */
#define isAvbpCheckConnected(avbp) (avbp && (avbp->bitfields.avbp_state == avbpConnected))

/* Get Remote Browsing Support */
#define isAvrcpRemoteBrowseSupport(avrcp) \
            ((avrcp->bitfields.remote_extensions & AVRCP_BROWSING_SUPPORTED)\
                                    == AVRCP_BROWSING_SUPPORTED)

/* Get Local browsing Support */
#define isAvrcpBrowsingEnabled(avrcp) \
             ((avrcp->bitfields.local_extensions & AVRCP_BROWSING_SUPPORTED)\
                                   == AVRCP_BROWSING_SUPPORTED)

/* Get Local Target Support */
#define isAvrcpTargetCat1Supported(avrcp) \
             ((avrcp->bitfields.local_target_features & AVRCP_CATEGORY_1)\
                                   == AVRCP_CATEGORY_1)

#define isAvrcpTargetCat3Supported(avrcp) \
             ((avrcp->bitfields.local_target_features & AVRCP_CATEGORY_3)\
                                   == AVRCP_CATEGORY_3)

/* Get the Local Target Database aware support */
#define isAvrcpDatabaseEnabled(avrcp) \
         ((avrcp->bitfields.local_extensions & AVRCP_DATABASE_AWARE_PLAYER_SUPPORTED)\
                == AVRCP_DATABASE_AWARE_PLAYER_SUPPORTED)

/* is local device supported Search? */
#define isAvrcpSearchEnabled(avrcp) \
           ((avrcp->bitfields.local_extensions & AVRCP_SEARCH_SUPPORTED)\
                == AVRCP_SEARCH_SUPPORTED)

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

/****************************************************************************
NAME    
    avrcpGetNextTransactionLabel

DESCRIPTION
    Increments the transaction label and returns the new value. Note that
    a transaction label of zero has special meaning as will therefore not be
    returned from this function.
*/
uint8 avrcpGetNextTransactionLabel(AVRCP *avrcp);


/****************************************************************************
NAME    
    convertResponseToStatus

DESCRIPTION
    Convert response type to a status that the library uses.
*/
avrcp_status_code convertResponseToStatus(avrcp_response_type resp);


/****************************************************************************
NAME    
   avrcpSendCommonStatusCfm

DESCRIPTION
    Send a Common Status Message. 
******************************************************************************/
void avrcpSendCommonStatusCfm(AVRCP*            avrcp,
                              avrcp_status_code status,
                              uint16            message_id);

/****************************************************************************
NAME
    avrcpSendCommonFragmentedMetadataCfm

DESCRIPTION
    
*/
void avrcpSendCommonFragmentedMetadataCfm(AVRCP        *avrcp, 
                                       avrcp_status_code status,
                                       uint16            id, 
                                       uint16            metadata_packet_type,
                                       uint16            data_length, 
                                       const uint8*      data);

#ifdef AVRCP_ENABLE_DEPRECATED
/****************************************************************************
NAME
    avrcpSendCommonMetadataCfm

DESCRIPTION
  Send a common Metadata Cfm message. This is a deprecated fuction and will be 
  replaced by avrcpSendCommonStatusCfm.  
*/
void avrcpSendCommonMetadataCfm(AVRCP *avrcp, 
                                avrcp_status_code status, 
                                uint16 id);
#else
#define avrcpSendCommonMetadataCfm avrcpSendCommonStatusCfm
#endif


#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
* NAME    
*    avrcpGetErrorStatusCode
*
* DESCRIPTION
*   Responses to AVRCP Specific Browsing Commands and AVRCP Specific AV/C
*   Commands. contain 1 byte Error Status Code. This function extracts the
*   Error Status code from the application response
*   
******************************************************************************/
uint8 avrcpGetErrorStatusCode(avrcp_response_type *response,
                              uint8 command_type);

/****************************************************************************
NAME
    avrcpSendCommonMetadataInd

DESCRIPTION
    
*/
void avrcpSendCommonMetadataInd(AVRCP *avrcp, uint16 id);


/****************************************************************************
NAME
    avrcpSendCommonFragmentedMetadataInd

DESCRIPTION
    
*/
void avrcpSendCommonFragmentedMetadataInd(  AVRCP     *avrcp, 
                                            uint16     id, 
                                            uint16     number_of_data_items, 
                                            uint16     data_length, 
                                            Source     source);


#endif /* !AVRCP_CT_ONLY_LIB*/

/****************************************************************************
NAME    
    avrcpSendCommonCfmMessageToApp

DESCRIPTION
    Create a common cfm message (many messages sent to the app
    have the form of the message below and a common function can be used to
    allocate them). Send the message not forgetting to set the correct 
    message id.

RETURNS
    void
*/
void avrcpSendCommonCfmMessageToApp(uint16              message_id, 
                                    avrcp_status_code   status, 
                                    Sink                sink, 
                                    AVRCP               *avrcp);


/****************************************************************************
NAME
    avrcpGetCompanyId

DESCRIPTION
    Extract the company id from a data buffer.
*/
uint32 avrcpGetCompanyId(const uint8 *ptr, uint16 offset);


/****************************************************************************
NAME
    avrcpGrabSink

DESCRIPTION
    Get access to the Sink.
*/
uint8 *avrcpGrabSink(Sink sink, uint16 size);

/****************************************************************************
NAME
    avrcpSourceProcessed

DESCRIPTION
    Called when the retrieved data source has been proccessed. So more data
    that has arrived over the air will be handled by the library until this
    is called.
*/
void avrcpSourceProcessed(AVRCP *avrcp, bool intern);

/****************************************************************************
NAME
   convertUint8ValuesToUint32 

DESCRIPTION
    Combine 4 consecutive uint8 values into a uint32.
*/
uint32 convertUint8ValuesToUint32(const uint8 *ptr);


/****************************************************************************
NAME
    convertUint32ToUint8Values

DESCRIPTION
    Convert uint32 into 4 consecutive uint8 values.
*/
void convertUint32ToUint8Values(uint8 *ptr, uint32 value);


/****************************************************************************
NAME
   avrcpSourceFromConstData 

DESCRIPTION
    This function allocates a data block, copies the const data  and 
    returns a source. It also stored the ptr to the data block so it 
    can be freed when the source empties.
*/
Source avrcpSourceFromConstData(AVRCP *avrcp, const uint8 *data, uint16 length);


/****************************************************************************
NAME
    avrcpSourceFromData

DESCRIPTION
    This function takes an allocated data block and returns a source. It 
    also stored the ptr to the data block so it can freed when the
    source empties.
*/
Source avrcpSourceFromData(AVRCP *avrcp, uint8 *data, uint16 length);

/**************************************************************************
 * NAME
 *  avrcpSourceDrop
 *
 * DESCRIPTION 
 *  Drop the AVRCP Source 
 */
void avrcpSourceDrop(AVRCP* avrcp);

#endif /* AVRCP_COMMON_H_ */
