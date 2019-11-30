/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd.h

DESCRIPTION
    A utility library to handle hardware specific dependency of SCM library.
*/


/*!
@file    broadcast_cmd.h
@brief  A utility library to handle hardware specific dependency of SCM library
*/


#ifndef BROADCAST_CMD_
#define BROADCAST_CMD_

#include <sink.h>
#include <library.h>

/****************************************************************************
 * Typedefs and Enumerations
 ****************************************************************************/
 
/*! library status codes.
 */
typedef enum
{
    /*! The requested operation was successful. */
    broadcast_cmd_success,

    /*! Failed */
    broadcast_cmd_failed
} broadcast_cmd_status;

/*! broadcast roles
 */
typedef enum
{
    /*! Default unknown role */
    broadcast_cmd_role_unknown,
    /*! broadcaster role */
    broadcast_cmd_role_broadcaster,
    /*! receiver role */
    broadcast_cmd_role_receiver
} broadcast_cmd_role;


/****************************************************************************
 * Message Interface
 ****************************************************************************/
 
/*! Messages sent by the Broadcast Command library to the task registered with
    #BroadcastCmdScmInit(), typically a VM application. 
*/
typedef enum
{
    /*! Library initialisation confirmation message */
    BROADCAST_CMD_INIT_CFM = BROADCAST_CMD_MESSAGE_BASE,
    BROADCAST_CMD_ENABLE_CFM,
    BROADCAST_CMD_DISABLE_CFM,
    BROADCAST_CMD_AFH_UPDATE_CFM,
    BROADCAST_CMD_SHUTDOWN_CFM,
    BROADCAST_CMD_AFH_FULL_UPDATE_IND,

    BROADCAST_CMD_MESSAGE_TOP
} BroadcastCmdMessageId;

/*! Message sent in response to #BroadcastCmdScmInit() to confirm status
    of SCM library initialisation.
 
    Possible BROADCAST_CMD_INIT_CFM_T::status values are:-
        - #broadcast_cmd_success
        - #broadcast_cmd_failed
 */
typedef struct
{
    /*! Role with which SCM was initialized */
    broadcast_cmd_role role;
    /*! Status of the library initialisation. */
    broadcast_cmd_status status;
} BROADCAST_CMD_CFM_T;

typedef BROADCAST_CMD_CFM_T BROADCAST_CMD_INIT_CFM_T;
typedef BROADCAST_CMD_CFM_T BROADCAST_CMD_ENABLE_CFM_T;
typedef BROADCAST_CMD_CFM_T BROADCAST_CMD_DISABLE_CFM_T;

/*! 
    Message sent when AFH Update indication is reeceived from broadcaster.
  */
typedef struct
{
    /*! Size of data in octets */
    uint16 size_data;
    /*! Actual received data */
    uint8 *data;
} BROADCAST_CMD_AFH_FULL_UPDATE_IND_T;


/****************************************************************************
 * Functions
 ****************************************************************************/
/*!
    @brief Create an instance of the SCM library based on broadcast role.

    The task specified in the app_task parameter will receive a 
    #SCM_XXX_INIT_CFM message indicating the result of the library instance 
    initialisation. Where XXX stands for BROADCAST or RECEIVER based on the role which initialized SCM
    
    If successful, the message will contain a reference to the SCM library
    intance, which must be passed in further API calls.

    @param app_task [IN] Application task to which messages are sent.
    @param role [IN] Broadcast role with which SCM needs to be initialized.
*/
void BroadcastCmdScmInit(Task app_task, broadcast_cmd_role role);

/*!
    @brief Enable SCM and register the appropirate transport task

    The task specified in the transport_task parameter will receive all SCM transport related messages
    like SCM_XXX_TRANSPORT_REGISTER_REQ, SCM_XXX_SEGMENT_REQ etc.Where XXX stands 
    for BROADCAST or RECEIVER based on the role which initialized SCM
    
    @param transport_task [IN] SCM transport task which needs to be registered.
*/
void BroadcastCmdScmEnable(Task transport_task);

/*!
    @brief Disable SCM and de-register the transport task

    SCM is disabled and so the transport task is de-registered.
    
    @param None
*/
void BroadcastCmdScmDisable(void);

/*!
    @brief Send a SCM AFH update message

    @param data [IN] Message contents as an array of octets.  Caller retains ownership of memory.
    @param size_data [IN] Size of data in octets.
    @param num_transmissions [IN] Number of transmissions for this segment.

    @return bool TRUE if successfully sent else FALSE.
*/
bool BroadcastCmdScmSendAfhUpdate(const uint8* data, unsigned size_data, unsigned num_transmissions);

/****************************************************************************
 * Hardware Specific interface functions
 ****************************************************************************/
/*!
@brief Interface for hardware specific module to register its task as SCM transport task.

@param transport_task [IN] hardware specific module task

@return None
*/
void BroadcastCmdScmTransportRegisterReq(Task transport_task);

  /*!
@brief Interface for hardware specific module to de-register its task as SCM transport task.

@param None

@return None
*/
void BroadcastCmdScmTransportUnRegisterReq(void);

/*!
@brief Interface for hardware specific module to inform message send comfirmation

@param header [IN] Segment header.
@param num_transmissions [IN] Number of transmissions remaining for this segment, 0 if segment transmission complete.

@return None
*/
void BroadcastCmdScmSeqmentSendCfm(unsigned header, unsigned num_transmissions);

 /*!
@brief Interface for hardware specific module to inform message indication

@param header [IN] Segment header.
@param data [IN] Segment data, #SCM_SEGMENT_DATA_SIZE octets.

@return None
*/
void BroadcastCmdScmSeqmentInd(unsigned header, const uint8 *data);

/*!
@brief Interface for hardware specific module to inform message Seqment Expired

@param header [IN] Segment header.

@return None
*/
void BroadcastCmdScmSeqmentExpiredInd(unsigned header);

/*!
@brief Register a different Task to receive BROADCAST_CMD_DISABLE_CFM.

This function allows a different Task to the one set in #BroadcastCmdScmInit
to receive BROADCAST_CMD_DISABLE_CFM.

It is intended for use by audio plugins so they can be notified when
BROADCAST_CMD_DISABLE_CFM has happened and react if necessary, for example
releasing any resource that were using the scm instance.

@param task [IN] Task to send BROADCAST_CMD_DISABLE_CFM to.

@return None
*/
void BroadcastCmdScmRegisterDisableHandler(Task task);

/*!
@brief Destroy the scm instance contained in this library.

This function will free the scm instance and remove it from
the broadcast_context.

@return None
*/
void BroadcastCmdScmDestroy(void);

#endif /* BROADCAST_CMD_ */
