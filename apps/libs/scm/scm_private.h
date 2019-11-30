/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file scm_private.h
  @brief Internal interface within the SCM library.
*/

#ifndef SCM_PRIVATE_H_
#define SCM_PRIVATE_H_

#include "scm.h"

/****************************************************************************
 * Defines
 ****************************************************************************/

/* Control debug generation */
#ifdef SCM_DEBUG_LIB
#include <stdio.h>
#define SCM_DEBUG(x)  printf x
#else
#define SCM_DEBUG(x)
#endif

/* Message macros */
#define MAKE_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T);
#define MAKE_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);
#define COPY_MESSAGE(src, dst) *(dst) = *(src);
#define COPY_MESSAGE_WITH_LEN(TYPE, LEN, src, dst) memmove((dst), (src), sizeof(TYPE##_T) + LEN);

/*! Maximum number of simultaneous segments that can be broadcast */
#define SCM_MAX_SEGMENTS (8)

/*! Unassigned/invalid sequence number */
#define SCM_SEQ_NUM_INVALID (0xFF)

/*! Number of sequence numbers in protocol */
#define SCM_NUM_SEQ_NUMS    (16)

#define SCM_INTERNAL_MSG_BASE (0x0000)
typedef enum
{
    SCM_INTERNAL_BROADCAST_MSG = SCM_INTERNAL_MSG_BASE,
    SCM_INTERNAL_KICK_REQ,
    SCM_INTERNAL_SEGMENT_CFM,
    SCM_INTERNAL_RESET_REQ,
    SCM_INTERNAL_SEQ_ADD_TO_POOL_BASE_IND,
    SCM_INTERNAL_SEQ_ADD_TO_POOL_TOP_IND = SCM_INTERNAL_SEQ_ADD_TO_POOL_BASE_IND + (SCM_NUM_SEQ_NUMS - 1),
    SCM_INTERNAL_MSG_TOP
} SCM_INTERNAL_MSG;

typedef struct SCM_INTERNAL_BROADCAST_MSG
{
    struct SCM_INTERNAL_BROADCAST_MSG *next;
    uint16 data_size;
    const uint8 *data;
    uint8 sequence_number;      /* 0xFF if sequence number hasn't yet been allocated */
    uint16 num_transmissions;
    uint16 segment_pending_set; /* Set when segment is sent to DSP for transmission */
    uint16 segment_sent_set;    /* Set when segment has been transmitted by DSP */
    SCM_HANDLE handle;
} SCM_INTERNAL_BROADCAST_MSG_T;

typedef struct SCM_INTERNAL_RECEIVE_MSG
{
    struct SCM_INTERNAL_RECEIVE_MSG *next;
    uint16 segment_received_set;
    uint16 segment_expired_set;
    uint16 data_size;
    uint8 *data;
    uint8 sequence_number;
} SCM_INTERNAL_RECEIVE_MSG_T;

struct __SCMB
{
    /*! Task for this instance of the broadcast library */
    TaskData    lib_task;

    /*! Application to which messages for this instance will be sent */
    Task        client_task;

    /*! Task to send segment for transmission to */
    Task        transport_task;

    /*! List of messages to be broadcast */
    SCM_INTERNAL_BROADCAST_MSG_T *msg_list;

    /*! Last allocated sequence number */
    unsigned alloc_seq_num:4;

    /*! Pool of available sequence numbers (1 bit per sequence number) */
    uint16 seq_num_pool;
    
    uint16 segments_pending;

    /*! Flag to keep track of pending internal tasks of the library */
    uint32 pending_internal_tasks;

    /*! Handle for next message, 16 bit number, with bit 15 set to avoid 0x0000 */
    SCM_HANDLE next_handle;
};

struct __SCMR
{
    /*! Task for this instance of the broadcast library */
    TaskData        lib_task;

    /*! Application to which messages for this instance will be sent */
    Task            client_task;

    /*! Task to send segment for transmission to */
    Task        transport_task;

    /*! List of messages being received */
    SCM_INTERNAL_RECEIVE_MSG_T *msg_list;
};

#endif /* BROADCAST_PRIVATE_H_ */


