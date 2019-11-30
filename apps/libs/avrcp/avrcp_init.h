/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_init.h
    
DESCRIPTION
    Header file for Initialization routines. 
    
*/

#ifndef AVRCP_INIT_H_
#define AVRCP_INIT_H_

#include "avrcp_common.h"

typedef struct
{
    AVRCP   avrcp;
    AVBP    avbp;
} AVRCP_AVBP_INIT;


/****************************************************************************
 *NAME    
 *   avrcpGetDeviceTask
 *
 *DESCRIPTION
 *  Get the device Task
 */
AvrcpDeviceTask* avrcpGetDeviceTask(void);

/****************************************************************************
 *NAME    
 *   avrcpGetInitTask
 *
 *DESCRIPTION
 *  Get the Initailization Task; 
 */
Task avrcpGetInitTask( void );


/****************************************************************************
*NAME    
*  avrcpInitDefaults
*
*DESCRIPTION
*  Initialize the Default AVRCP task values.
*/
void avrcpInitDefaults( AVRCP       *avrcp,
                        avrcpState   state);


/****************************************************************************
*NAME
*  avrcpFindTask
*
*DESCRIPTION
*  Find the AVRCP Task from BT address.
*/
AVRCP *avrcpFindTask(const bdaddr *bd_addr);


/****************************************************************************
*NAME    
*  avrcpCreateTask
*
*DESCRIPTION
*  Create the AVRCP Task and initiaze the default values.
*/
AVRCP *avrcpCreateTask( Task         client, 
                        avrcpState   state,
                        const bdaddr *bd_addr);

/****************************************************************************
*NAME    
*   avrcpResetValues
*
*DESCRIPTION
*  Reset the local state values to their initial states. 
*/
void avrcpResetValues(AVRCP* avrcp);
 
/****************************************************************************
*NAME    
*   avrcpInitTaskData
*
*DESCRIPTION
*    Initialise all members of the AVRCP task data structure.
*/
void avrcpInitTaskData( AVRCP               *avrcp,
                        avrcp_device_type   dev, 
                        uint8               controller_features, 
                        uint8               target_features, 
                        uint8               extensions);

/****************************************************************************
*NAME    
*   avrcpInitBrowsing
*
*DESCRIPTION
*    Initialise all members of the AVBP task data structure and register 
*    AVCTP_Browsing PSM with L2CAP. 
*/
void avrcpInitBrowsing(AVRCP_AVBP_INIT*  init_block); 

/****************************************************************************
*NAME    
*    avrcpHandleInternalInitReq
*
*DESCRIPTION
*    Init request messages are sent internally withing the profile instance
*    until its initialisation has completed. These messages are handled in
*    this function.
*/
void avrcpHandleInternalInitReq(const AVRCP_INTERNAL_INIT_REQ_T   *req);

/****************************************************************************
*NAME    
*    avrcpSendInitCfmToClient
*
*DESCRIPTION
*    Send an AVRCP_INIT_CFM message to the client task.
*/
void avrcpSendInitCfmToClient( avrcp_unreg_status   state,
                               uint32             sdp_handle, 
                               avrcp_status_code  status);

/****************************************************************************
*NAME    
*   avrcpHandleDeleteTask
*
*DESCRIPTION
*   Delete a dynamically allocated AVRCP and AVBP task instance. 
*   Before deleting make  sure all messages for that task are flushed 
*   from the message queue.
*/
void avrcpHandleDeleteTask(AVRCP *avrcp);

/****************************************************************************
*NAME    
*    avrcpResetAvbpValues
*
*DESCRIPTION
*   Reset the browsing channel Task 
*/
void avrcpResetAvbpValues(AVBP* avbp);

/****************************************************************************
*NAME    
*   avrcpDeleteAllTasks
*
* DESCRIPTION
*   This will be called to delete all tasks. 
*/
void avrcpDeleteAllTasks(AVRCP *avrcp);

#endif /* AVRCP_INIT_H_ */
