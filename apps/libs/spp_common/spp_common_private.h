/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    spp_common_private.h
    
DESCRIPTION
	Header file for the SPP profile library containing common private structures
	and functions.
	
*/


#ifndef SPP_COMMON_PRIVATE_H_
#define SPP_COMMON_PRIVATE_H_

#ifdef SPP_DEBUG_LIB
#include <stdio.h>
#define SPP_DEBUG(x) {printf x; Panic();}
#define SPP_PRINT(x) {printf x;}
#else
#define SPP_DEBUG(x)
#define SPP_PRINT(x)
#endif

/* Macro for creating messages */
#define MAKE_SPP_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/* Macro to add debug output to track state change. */

#ifdef SPP_DEBUG_LIB
#define SPP_STATE(new_state) {\
    printf("SPP State was %d, is now %d\n", spp->c.state, new_state);\
    spp->c.state=new_state;\
    }
#else
#define SPP_STATE(new_state) spp->c.state=new_state
#endif

/* Enum for internal messages */
typedef enum
{
    SPP_INTERNAL_TASK_DELETE_REQ = SPP_MESSAGE_TOP,
    SPP_INTERNAL_DISCONNECT_REQ,
    SPP_INTERNAL_CONNECT_REQ
} SppInternalMessageId;


/* SPP states */

typedef enum 
{
    sppIdle,                /* client only */
    sppInitialising,        /* client only */
    sppSearching,           /* client only */
	sppConnecting,			
	sppConnected,
	sppDisconnecting
} sppState;


/* Common SPP Client and Server task data */

typedef struct __SPP_COMMON
{
    TaskData        task; /* This element should always be in the first place of the structure*/
    Task            client_task;
    Sink            sink;
    bdaddr          bd_addr;
    uint16          max_payload_size;
    sppState        state;
} SPP_COMMON;


/****************************************************************************
NAME	
	connectionConvertRfcommConnectStatus

DESCRIPTION
	Convert the RFCOMM connection status to SPP connection status.

RETURNS
	spp_disconnect_status corresponding to the status passed in
*/
spp_connect_status sppConvertRfcommConnectStatus(rfcomm_connect_status status);



/****************************************************************************
NAME	
	connectionConvertRfcommDisconnectStatus

DESCRIPTION
	Convert the RFCOMM disconnection status to SPP disconnection status.

RETURNS
	spp_disconnect_status corresponding to the status passed in
*/
spp_disconnect_status sppConvertRfcommDisconnectStatus(rfcomm_disconnect_status status);


/****************************************************************************
NAME	
	sppSendConnectCfm

DESCRIPTION
	Send either SPP_CLIENT_CONNECT_CFM or SPP_CLIENT_CONNECT. Sends a 
	SPP__TASK_DELETE_REQ message if the connection was not successful.

RETURNS
	spp_connect_status corresponding to the status passed in.
*/
void sppSendConnectCfm(SPP *spp, MessageId id, spp_connect_status status);


/****************************************************************************
NAME	
	sppHandleUnexpectedMsg

DESCRIPTION
	Deals with an messages that are received by the SPP Client or Server 
	connection message handler in the wrong state.

RETURNS
    Nothing.
*/
void sppHandleUnexpectedMsg(sppState state, MessageId id);


/****************************************************************************
NAME	
	sppDeleteTask

DESCRIPTION
	Deletes the task indicated by the SPP instance pointer, after clearing down
	it's message queue.

RETURNS
    Nothing.
*/
void sppDeleteTask(SPP *spp);


/****************************************************************************
NAME	
	sppHandleRfcommDisconnectInd

DESCRIPTION
	Deals with the RFCOMM disconnect ind and puts the task into the 
	sppDisconnecting state to tidy up.

RETURNS
    Nothing.
*/
void sppHandleRfcommDisconnectInd(SPP *spp, CL_RFCOMM_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME	
	sppHandleRfcommDisconnectCfm

DESCRIPTION
	Deals with the RFCOMM disconnect cfm and starts the tidy up functions.

RETURNS
    Nothing.
*/
void sppHandleRfcommDisconnectCfm(SPP *spp, const CL_RFCOMM_DISCONNECT_CFM_T *cfm);


/****************************************************************************
NAME	
	sppForwardRfcommControlInd

DESCRIPTION
	Forward the CL_RFCOMM_CONTROL_IND message to the client task.

RETURNS
    Nothing.
*/
void sppForwardRfcommControlInd(SPP *spp, const CL_RFCOMM_CONTROL_IND_T *ind);


/****************************************************************************
NAME	
	sppForwardSmEncryptionChangeInd

DESCRIPTION
	Forward the CL_SM_ENCRYPTION_CHANGE_IND message to the client task.

RETURNS
    Nothing.
*/
void sppForwardSmEncryptionChangeInd(SPP *spp, const CL_SM_ENCRYPTION_CHANGE_IND_T *ind);
    
/****************************************************************************
NAME	
	sppHandleRfcommPortnegInd

DESCRIPTION
	Remote device has sent us a portneg request, respond

RETURNS
    Nothing.
*/
void sppHandleRfcommPortnegInd(SPP *spp, CL_RFCOMM_PORTNEG_IND_T *ind);

/****************************************************************************
NAME	
	sppForwardMessageMoreData

DESCRIPTION
	Takes a MESSAGE_MORE_DATA and copies it to the SPP_MESSAGE_MORE_DATA 
	structure and adds a reference to the SPP task instance.

RETURNS
    Nothing.
*/
void sppForwardMessageMoreData(SPP *spp, const MessageMoreData *msg);


/****************************************************************************
NAME	
	sppForwardMessageMoreSpace

DESCRIPTION
	Takes a MESSAGE_MORE_SPACE and copies it to the SPP_MESSAGE_MORE_SPACE
	structure and adds a reference to the SPP task instance.

RETURNS
    Nothing.
*/
void sppForwardMessageMoreSpace(SPP *spp, const MessageMoreSpace *msg);


#endif /* SPP_COMMON_PRIVATE_H_ */


