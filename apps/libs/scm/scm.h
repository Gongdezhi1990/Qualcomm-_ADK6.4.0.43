/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file scm.h
  @brief Interface to the SCM library.

  This library supports both the broadcaster and receiver roles in a broadcast
  environment.

  A single instance of the SCM library is created for either broadcast or
  receiver role by calling the appropriate initialisation function.

  Typical usage for a broadcast role would be:-

  App                                 Library                                        Transport
   | --> #ScmBroadcastInit()        --> |                                                 |
   | <-- #SCM_BROADCAST_INIT_CFM    <-- |                                                 |
   | --> #ScmBroadcastEnable()      --> |                                                 |
   |                                    | --> #SCM_BROADCAST_TRANSPORT_REGISTER_REQ   --> |
   |                                    | <-- #ScmBroadcastTransportRegisterCfm()     <-- |
   | <-- #SCM_BROADCAST_ENABLE_CFM  <-- |                                                 |
   | --> #ScmBroadcastMessageReq()  --> |                                                 |
   |                                    | --> #SCM_BROADCAST_SEGMENT_REQ              --> |
   |                                    | <-- #ScmBroadcastSegmentCfm()               <-- |
   |                                    | --> #SCM_BROADCAST_SEGMENT_REQ              --> |
   |                                    | <-- #ScmBroadcastSegmentCfm()               <-- |
   |                                    | --> #SCM_BROADCAST_SEGMENT_REQ              --> |
   |                                    | <-- #ScmBroadcastSegmentCfm()               <-- |
   | <-- #SCM_BROADCAST_MESSAGE_CFM <-- |                                                 |
   | --> #ScmBroadcastDisable()     --> |                                                 |
   |                                    | --> #SCM_BROADCAST_TRANSPORT_UNREGISTER_REQ --> |
   |                                    | <-- #ScmBroadcastTransportUnRegisterCfm()   <-- |
   | <-- #SCM_BROADCAST_DISABLE_CFM <-- |                                                 |

  Typical usage for a receiver role would be:-

  App                               Library                                        Transport
   | --> #ScmReceiverInit()        --> |                                                |
   | <-- #SCM_RECEIVER_INIT_CFM    <-- |                                                |
   | --> #ScmReceiverEnable()      --> |                                                |
   |                                   | --> #SCM_RECEIVER_TRANSPORT_REGISTER_REQ   --> |
   |                                   | <-- #ScmReceiverTransportRegisterCfm()     <-- |
   | <-- #SCM_RECEIVER_ENABLE_CFM  <-- |                                                |
   |                                   | <-- #ScmReceiverSegmentInd()               <-- |
   |                                   | <-- #ScmReceiverSegmentInd()               <-- |
   |                                   | <-- #ScmReceiverSegmentInd()               <-- |
   |                                   | <-- #ScmReceiverSegmentInd()               <-- |
   | <-- #SCM_RECEIVER_MESSAGE_IND <-- |                                                |
   |                                   | <-- #ScmReceiverSegmentExpiredInd()        <-- |
   |                                   | <-- #ScmReceiverSegmentExpiredInd()        <-- |
   |                                   | <-- #ScmReceiverSegmentExpiredInd()        <-- |
   |                                   | <-- #ScmReceiverSegmentExpiredInd()        <-- |
   | --> #ScmReceiverDisable()     --> |                                                |
   |                                   | --> #SCM_RECEIVER_TRANSPORT_UNREGISTER_REQ --> |
   |                                   | <-- #ScmReceiverTransportUnRegisterCfm()   <-- |
   | <-- #SCM_RECEIVER_DISABLE_CFM <-- |                                                |

*/

#ifndef SCM_H_
#define SCM_H_

#include <library.h>
#include <message.h>
#include <bdaddr.h>
#include <sink.h>
#include <source.h>
#include <connection.h>

/****************************************************************************
 * Typedefs and Enumerations
 ****************************************************************************/

/*! Opaque reference to an instance of the SCM broadcast library. */
struct __SCMB;
typedef struct __SCMB SCMB;

/*! Opaque reference to an instance of the SCM receiver library. */
struct __SCMR;
typedef struct __SCMR SCMR;

/*! Opaque handle to SCM broadcast request */
typedef uint16 SCM_HANDLE;

/*! SCM library status codes.
 */
typedef enum
{
    /*! The requested operation was successful. */
    scm_success,

    /*! Failed to initialise a SCM library instance. */
    scm_init_failed,

    /*! SCM request failed */
    scm_failed
} scm_status;

/****************************************************************************
 * Defines
 ****************************************************************************/

 /*! Amount of data in a message segment */
#define SCM_SEGMENT_DATA_SIZE   (3)

/****************************************************************************
 * Message Interface
 ****************************************************************************/

/* Define our own message base if we don't get one from library.h */
#ifndef SCM_MESSAGE_BASE
#define SCM_MESSAGE_BASE      0x7F00
#endif

/*! Messages sent by the SCM library to the task registered with
    #ScmBroadcastInit(), typically a VM application. 
*/
typedef enum
{
    /*! Library initialisation confirmation message.
        Defined in #SCM_BROADCAST_INIT_CFM_T. */
    SCM_BROADCAST_INIT_CFM = SCM_MESSAGE_BASE,
    SCM_BROADCAST_TRANSPORT_REGISTER_REQ,
    SCM_BROADCAST_TRANSPORT_UNREGISTER_REQ,
    SCM_BROADCAST_ENABLE_CFM,
    SCM_BROADCAST_DISABLE_CFM,
    SCM_BROADCAST_SEGMENT_REQ,
    SCM_BROADCAST_MESSAGE_CFM,

    SCM_RECEIVER_INIT_CFM,
    SCM_RECEIVER_TRANSPORT_REGISTER_REQ,
    SCM_RECEIVER_TRANSPORT_UNREGISTER_REQ,
    SCM_RECEIVER_ENABLE_CFM,
    SCM_RECEIVER_DISABLE_CFM,
    SCM_RECEIVER_MESSAGE_IND,

    SCM_MESSAGE_TOP
} ScmMessageId;

/*! Message sent in response to #ScmBroadcastInit() to confirm status
    of library initialisation.
 
    Possible SCM_BROADCAST_INIT_CFM_T::status values are:-
        - #scm_success
        - #scm_init_failed
 */
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMB *scm;

    /*! Status of the library initialisation. */
    scm_status status;
} SCM_BROADCAST_INIT_CFM_T;

/*! Message sent by the SCM library to the transport_task provided in
  #ScmBroadcastEnable(ENABLE). The message allows the SCM library to register
  its existance with the transport task.
 */
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMB *scm;
} SCM_BROADCAST_TRANSPORT_REGISTER_REQ_T;

/*! Message sent by the SCM library to the transport_task upon receiving
    #ScmBroadcastEnable(DISABLE) call.
*/
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMB *scm;
} SCM_BROADCAST_TRANSPORT_UNREGISTER_REQ_T;

/*! Message sent to the client task to confirm the result of the enable
    operation requested with #ScmBroadcastEnable().
*/
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMB *scm;

    /*! Result of enable operation. */
    scm_status status;
} SCM_BROADCAST_ENABLE_CFM_T;

/*! Message sent to the client task to confirm the result of the disable
    operation requested with #ScmBroadcastDisable().
*/
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMB *scm;

    /*! Result of disable operation. */
    scm_status status;
} SCM_BROADCAST_DISABLE_CFM_T;

/*! Message sent as confirmation that message has been broadcast */
typedef struct
{
    SCMB *scm;

    /*! Message handle, matches that returned by #ScmBroadcastMessageReq() */
    SCM_HANDLE handle;

    /*! Original message data that was broadcast */
    const uint8 *data;

    /*! Size of data that was broadcast */
    uint16 size_data;   
} SCM_BROADCAST_MESSAGE_CFM_T;

/*! Message sent in response to #ScmReceiverInit() to confirm status
    of library initialisation.
 
    Possible SCM_RECEIVER_INIT_CFM_T::status values are:-
        - #scm_success
        - #scm_init_failed
 */
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMR *scm;

    /*! Status of the library initialisation. */
    scm_status status;
} SCM_RECEIVER_INIT_CFM_T;

/*! Message sent by the SCM library to the transport_task provided in
  #ScmReceiverEnable(). The message allows the SCM library to register
  its existance with the transport task.
 */
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMR *scm;
} SCM_RECEIVER_TRANSPORT_REGISTER_REQ_T;

/*! Message sent by the SCM library to the transport_task upon receiving
    #ScmBroadcastDisable() call.
*/
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMR *scm;
} SCM_RECEIVER_TRANSPORT_UNREGISTER_REQ_T;

/*! Message sent to the client task to confirm the result of the enable
    operation requested with #ScmReceiverEnable().
*/
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMR *scm;

    /*! Result of enable operation. */
    scm_status status;
} SCM_RECEIVER_ENABLE_CFM_T;

/*! Message sent to the client task to confirm the result of the disable
    operation requested with #ScmReceiverDisable().
*/
typedef struct
{
    /*! Pointer to the instance of the SCM library
        created by the initialisation operation. */
    SCMR *scm;

    /*! Result of disable operation. */
    scm_status status;
} SCM_RECEIVER_DISABLE_CFM_T;


/*! Message sent to application on reception of a complete SCM message */
typedef struct
{
    /*! Pointer to the instance of the SCM library. */
    SCMR *scm;

    uint8 *data;
    uint16 size_data;
} SCM_RECEIVER_MESSAGE_IND_T;

/*! Message sent to application to request transmission of message segment */
typedef struct
{
    /*! Pointer to the instance of the SCM library. */
    SCMB *scm;

    /*! Segment header */
    uint8 header;

    /*! Segment data */
    uint8 data[SCM_SEGMENT_DATA_SIZE];

    /*! Number of transmission required for this segment */
    uint16 num_transmissions;
} SCM_BROADCAST_SEGMENT_REQ_T;


/****************************************************************************
 * Functions
 ****************************************************************************/
/*!
    @brief Create an instance of the SCM broadcast library.

    @param app_task [IN] Application task to which messages are sent.
*/
void ScmBroadcastInit(Task app_task);

/*!
    @brief Create an instance of the SCM receiver library.

    @param app_task [IN] Application task to which messages are sent.
*/
void ScmReceiverInit(Task app_task);

/*!
    @brief Confirm receipt of a #SCM_BROADCAST_TRANSPORT_REGISTER_REQ message.

    @param scm [IN] SCM broadcaster instance.
    @param transport_task [IN] The transport task.
*/
void ScmBroadcastTransportRegisterCfm(SCMB *scm, Task transport_task);

/*!
    @brief Confirm receipt of a #SCM_BROADCAST_TRANSPORT_UNREGISTER_REQ message.

    @param scm [IN] SCM broadcaster instance.
*/
void ScmBroadcastTransportUnRegisterCfm(SCMB *scm);

/*!
    @brief Confirm receipt of a #SCM_RECEIVER_TRANSPORT_REGISTER_REQ message.

    @param scm [IN] SCM receiver instance.
    @param transport_task [IN] The transport task.
*/
void ScmReceiverTransportRegisterCfm(SCMR *scm, Task transport_task);

/*!
    @brief Confirm receipt of a #SCM_RECEIVER_TRANSPORT_UNREGISTER_REQ message.

    @param scm [IN] SCM receiver instance.
*/
void ScmReceiverTransportUnRegisterCfm(SCMR *scm);

/*!
   @brief Destroy an instance of SCM broadcaster library.

   @param scm [IN] SCM broadcaster instance.
*/
void ScmBroadcastDestroy(SCMB *scm);

/*!
   @brief Destroy an instance of SCM receiver library.

   @param scm [IN] SCM receiver instance.
*/
void ScmReceiverDestroy(SCMR *scm);

/*!
    @brief Broadcast a message

    @param scm [IN] SCM broadcaster instance.
    @param data [IN] Message contents as an array of octets.  Caller retains ownership of memory.
    @param size_data [IN] Size of data in octets.
    @param num_transmissions [IN] Number of transmissions for this segment.

    @return uintptr_t Message handle.
*/
SCM_HANDLE ScmBroadcastMessageReq(SCMB *scm, const uint8 *data, uint16 size_data, uint16 num_transmissions);

/*!
    @brief Confirm segment broadcast

    @param scm [IN] SCM broadcaster instance.
    @param header [IN] Segment header.
    @param num_transmissions [IN] Number of transmissions remaining for this segment, 0 if segment transmission complete.
*/
void ScmBroadcastSegmentCfm(SCMB *scm, uint16 header, uint16 num_transmissions);

/*!
    @brief Pass received segment to SCM receiver for re-assembly.

    @param scm [IN] SCM receiver instance.
    @param header [IN] Segment header.
    @param data [IN] Segment data, #SCM_SEGMENT_DATA_SIZE octets.
*/
void ScmReceiverSegmentInd(SCMR *scm, uint16 header, const uint8 *data);

/*!
    @brief Indication that received segment has expired.

    @param scm [IN] SCM receiver instance.
    @param header [IN] Segment header.
*/
void ScmReceiverSegmentExpiredInd(SCMR *scm, uint16 header);

/*!
    @brief Reset receiver state, clear any partial messages.
*/
void ScmReceiverReset(SCMR *scm);
   
/*!
    @brief Enable broadcasting SCM messages.

    @param scm [IN] SCM broadcaster instance.
    @param transport_task [IN] Task to which segments are sent for transmission.

*/
void ScmBroadcastEnable(SCMB *scm, Task transport_task);

/*!
    @brief Disable broadcasting SCM messages.

    @param scm [IN] SCM broadcaster instance.
*/
void ScmBroadcastDisable(SCMB *scm);

/*!
    @brief Enable receiving SCM messages.

    Will result in registering the SCMR instance with the transport_task.

    @param scm [IN] SCM broadcaster instance.
    @param transport_task [IN] Task to which segments are sent for transmission.

*/
void ScmReceiverEnable(SCMR *scm, Task transport_task);

/*!
    @brief Disable receiving SCM messages.

    Will result in unregistering the transport_task.

    @param scm [IN] SCM broadcaster instance.
*/
void ScmReceiverDisable(SCMR *scm);

/*!
    @brief Determine if the SCM library still has messages in progress.
    
    @param scm [IN] SCM broadcaster instance.

    @return bool TRUE if SCM library still has messages outstanding.
*/
bool ScmBroadcastIsBusy(SCMB *scm);

#endif
