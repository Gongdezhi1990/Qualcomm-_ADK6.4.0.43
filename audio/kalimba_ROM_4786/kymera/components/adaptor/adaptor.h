/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup adaptor Adaptors to kymera interface
 *
 * \file adaptor.h
 * \ingroup adaptor
 *
 */

#ifndef _ADAPTOR_H_
#define _ADAPTOR_H_

#include "types.h"
#include "stream_prim.h"
#include "status_prim.h"

/* TODO: with multiple transports, later condition this & similar per platform. Ideally, though,
 * the client ID definitions for OBPM communication will be divorced from transport-related
 * information and this "legacy" can be changed.
 */


#include "spi/spi_for_adaptors.h"

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#include "adaptor/kip/kip_msg_adaptor.h"
#endif

#ifdef CHIP_BASE_BC7
#include "bc_comms_for_adaptors.h"
#endif


/****************************************************************************
Public Constant Declarations
*/

#if defined(CHIP_BASE_HYDRA) || defined(CHIP_BASE_NAPIER)
#include "accmd_prim.h"
#define STREAM_INFO_KEY_AUDIO_SAMPLE_RATE ACCMD_INFO_KEY_AUDIO_SAMPLE_RATE
#define STREAM_INFO_KEY_AUDIO_LOCALLY_CLOCKED ACCMD_INFO_KEY_AUDIO_LOCALLY_CLOCKED
#define STREAM_INFO_KEY_AUDIO_SAMPLE_PERIOD_DEVIATION ACCMD_INFO_KEY_AUDIO_SAMPLE_PERIOD_DEVIATION
#define STREAM_INFO_KEY_ENDPOINT_EXISTS ACCMD_INFO_KEY_ENDPOINT_EXISTS
#elif defined(CHIP_BASE_BC7)
#define STREAM_INFO_KEY_AUDIO_SAMPLE_RATE   0
#elif defined(CHIP_BASE_A7DA_KAS)
#define STREAM_INFO_KEY_AUDIO_SAMPLE_RATE   0
#else

#error "Adaptor not implemented for this chip"

#endif

#define ADAPTOR_ANY_SIZE 1

/* TODO: routing for any OBPM is for now as per existing (transport-related) client ID.
 * Once multiple transports come into being, rationalise this and divorce client ID from transport.
 */
#define RESPOND_TO_OBPM                     RESPOND_TO_SPI

/****************************************************************************
Public Macro Definitions
*/
/* Constants for connection ID fields */
/** The number of bits to shift the sender/reciever id in the connection id field */
#define RECEIVER_SENDER_ID_SHIFT    8
/** Mask for extracting the receiver id */
#define RECEIVER_ID_MASK            0xFF00
/** Mask for extracting the sender id */
#define SENDER_ID_MASK              0x00FF

/** Mask off upper 8 bits of routing information */
#define ROUTING_VALUE_MASK          SENDER_ID_MASK
/** Used for constructing the connection id from routing info */
#define PACK_CON_ID(send_id, recv_id) \
    ((((recv_id) & ROUTING_VALUE_MASK) << RECEIVER_SENDER_ID_SHIFT) | \
    ((send_id) & ROUTING_VALUE_MASK))

/** Used for extracting the sender id from a connection id */
#define GET_CON_ID_SEND_ID(con_id) ((con_id) & SENDER_ID_MASK)
/** Used for extracting the receiver id from a connection id */
#define GET_CON_ID_RECV_ID(con_id) \
    (((con_id) & RECEIVER_ID_MASK) >> RECEIVER_SENDER_ID_SHIFT)

/** Reverse connection ID - this can be optimised if we know in the end nothing
 *  else is encoded in it. */
#define REVERSE_CONNECTION_ID(con_id) \
        ( (((con_id) & SENDER_ID_MASK) << RECEIVER_SENDER_ID_SHIFT | \
          (((con_id) & RECEIVER_ID_MASK) >> RECEIVER_SENDER_ID_SHIFT)) )


/** Value representing and invalid connection. N.B. This is a theoretically
 * valid value although impossible. */
#define INVALID_CON_ID  (SENDER_ID_MASK | RECEIVER_ID_MASK)

/** Macros used to prepare lists received over transports that are 16-bit wide (such as
 * ACCMD & OBPM) for use in platform independent modules. On platforms where uint16 is
 * not same as unsigned, this involves allocating temporary array, which needs to be
 * freed after use.
 * Also provide protection against zero allocation and freeing of unallocated memory
 */

#if (ADDR_PER_WORD == 1)

typedef const uint16 *UINT16_LISTPTR;

#define ADAPTOR_UNPACK_UINT16_LIST_TO_UNSIGNED(p,c) (unsigned*)(p)
#define ADAPTOR_PACK_UNSIGNED_LIST_TO_UINT16(p, c) (UINT16_LISTPTR)(p)
#define ADAPTOR_FREE_LIST(p) (void)0

#else

typedef uint16 *UINT16_LISTPTR;

#define ADAPTOR_UNPACK_UINT16_LIST_TO_UNSIGNED(p,c) adaptor_unpack_list_to_unsigned((p),(c))
#define ADAPTOR_PACK_UNSIGNED_LIST_TO_UINT16(p, c) adaptor_pack_list_to_uint16(NULL, (p), (c))
#define ADAPTOR_FREE_LIST(p) pfree(p)

#endif


/* Processor ID is top 3 bits and client id is last 5 bits of receiver id
 * and sender id
 *
 *    15 - 13      12 - 8         |  7 - 5       |  4 - 0          |
 *  ----------------------------------------------------------------
 *  |RCV PROC ID | RECV CLIENT ID | SEND PROC ID | SENDER CLIENT ID|
 *  ----------------------------------------------------------------
 */
/** Constants for extracting processor and client IDs from sender or receiver IDs. 5 LSBs are client ID,
 * 3 MSBs are processor ID.
 */
#define CONID_PROCESSOR_ID_SHIFT          5
#define CONID_PROCESSOR_ID_MASK           0x0007
#define CONID_CLIENT_ID_MASK              0x001F
#define CONID_PACKED_CLIENT_ID_MASK       0x1F1F
#define CONID_PACKED_RECV_PROC_ID_MASK    0xE000

/** Used for building a sender OR receiver ID from client ID and processor ID */
#define PACK_SEND_RECV_ID(proc_id, client_id) \
        ((((proc_id) & CONID_PROCESSOR_ID_MASK) << CONID_PROCESSOR_ID_SHIFT) | \
          ((client_id) & CONID_CLIENT_ID_MASK))

/* Get client ID from sender OR receiver ID */
#define GET_SEND_RECV_ID_CLIENT_ID(id)  ((id) & CONID_CLIENT_ID_MASK)

/** Get processor ID from a sender or receiver ID */

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)

#define GET_RECV_PROC_ID(conid) \
            (IPC_PROCESSOR_ID_NUM)((GET_CON_ID_RECV_ID(conid) >> \
                                    CONID_PROCESSOR_ID_SHIFT) & CONID_PROCESSOR_ID_MASK)
#define GET_SEND_PROC_ID(conid) \
            (IPC_PROCESSOR_ID_NUM)((GET_CON_ID_SEND_ID(conid) >> \
                                    CONID_PROCESSOR_ID_SHIFT) & CONID_PROCESSOR_ID_MASK)


#else

#define GET_RECV_PROC_ID(conid) 0
#define GET_SEND_PROC_ID(conid) 0

#endif

/** Used for extracting the receiver id from a internal connection id */
#define GET_EXT_CON_ID_RECV_ID(con_id) \
    (((con_id) >> RECEIVER_SENDER_ID_SHIFT) & CONID_CLIENT_ID_MASK)

#define UNPACK_REVERSE_CONID(conid) REVERSE_CONNECTION_ID(GET_UNPACKED_CONID(conid))
#define GET_UNPACKED_CONID(conid) (((uint16)(conid)) & CONID_PACKED_CLIENT_ID_MASK)

#define PACK_CONID_PROCID( conid, procid) \
            (uint16)(GET_UNPACKED_CONID(conid)| \
            ((uint16)((procid) & CONID_PROCESSOR_ID_MASK) \
             << (RECEIVER_SENDER_ID_SHIFT + CONID_PROCESSOR_ID_SHIFT)) | \
            ((uint16)(hal_get_reg_processor_id()  & CONID_PROCESSOR_ID_MASK) \
            <<  CONID_PROCESSOR_ID_SHIFT))


/****************************************************************************
Public Type Definitions
*/

/**
 * Adapter Message IDs
 */
typedef enum
{
	AMSGID_FROM_OPERATOR        = 0x0000,
	AMSGID_PS_ENTRY_READ        = 0x0001,
	AMSGID_PS_ENTRY_WRITE       = 0x0002,
	AMSGID_PS_ENTRY_DELETE      = 0x0003,
	AMSGID_PS_SHUTDOWN_COMPLETE = 0x0004,
	AMSGID_UNACHIEVABLE_LATENCY = 0x0005,
} ADAPTOR_MSGID;

typedef struct
{
	unsigned key_id;
	unsigned offset;
} ADAPTOR_PS_ENTRY_READ_MSG;

typedef struct
{
	unsigned key_id;
	unsigned total_len;
	unsigned offset;
	unsigned data[ADAPTOR_ANY_SIZE];
} ADAPTOR_PS_ENTRY_WRITE_MSG;
#define ADAPTOR_PS_ENTRY_WRITE_MSG_DATA_WORD_OFFSET (3)
#define ADAPTOR_PS_ENTRY_WRITE_MSG_WORD_SIZE (4)

typedef struct
{
	unsigned key_id;
} ADAPTOR_PS_ENTRY_DELETE_MSG;

typedef struct
{
    uint32 heap_size;
    uint32 heap_current;
    uint32 heap_min;
    uint32 pool_size;
    uint32 pool_current;
    uint32 pool_min;
} ADAPTOR_GET_MEM_USAGE;

/* Generic adaptor callback. Status takes one of the values from status_prim.h */
typedef bool (*ADAPTOR_GENERIC_CALLBACK)(unsigned conidx, unsigned status, void *parameters);

/****************************************************************************
Public Function Definitions
*/

/**
 * \brief  Send message function.
 *
 * \param  conn_id the connection ID (sender and recipient ID codes)
 * \param  msg_id ID of the message
 * \param  msg_length length of payload
 * \param  msg_data pointer to the message payload
 *
 * \return success/fail (true/false).
 */
extern bool adaptor_send_message(unsigned conn_id, ADAPTOR_MSGID msg_id, unsigned msg_length, unsigned* msg_data);

 /**
  * \brief Unpack a list of parameters in a message received over 16-bit wide transport
  * for use in a platform independent module
  *
  * \param pparam Pointer to the start of the parameter list within the message
  *
  * \param count  Number of elements in the list
  *
  * \return Unpacked list, which needs to be freed after use or NULL if count is 0
  */
unsigned *adaptor_unpack_list_to_unsigned(uint16* pparam, unsigned count);

 /**
  * \brief converts an unsigned array into uint16 array.
  *  Allocates the destination uint16 array if required.
  *
  * \param pdest pointer to the destination uint16 array.
  *  Can be NULL in which case, the destination array
  *  is allocated on a non-zero count.
  *
  * \param psrc pointer to the source unsigned array.
  *
  * \param count number of elements in the source list.
  *
  * \return pointer to the destination uint16 array, if allocated.
  */
uint16* adaptor_pack_list_to_uint16(uint16* ppdu, const unsigned* psrc, unsigned count);

#if !defined(NONSECURE_PROCESSING)
/**
 * \brief  Send licence query function. This query may be sent as dedicated system message or something
 *         else, e.g. unsolicited message from special system operator etc. - depending on platform.
 *         Response is handled in platform-specific manner in the various adaptors.
 *
 * \param  length  Length of payload
 * \param  payload Pointer to the payload
 *
 * \return success/fail (true/false).
 */
bool adaptor_send_licence_query(unsigned length, unsigned* payload);

#ifndef CHIP_A7DA_KAS
/**
 * \brief  Temporary logging function. TODO: can be removed after integration trials on non-A7DA.
 *
 * \param  length   Product ID length in 16-bit words
 * \param  prod_id  Pointer to (non-A7DA) product ID words, in MSW to LSW order
 */
void log_security_error(unsigned length, uint16* prod_id);
#endif

#ifdef SECURITY_TEST
/* Only pulled in for specific unit test build(s) when sending is hijacked by test harness */
extern bool test_send_licence_query(unsigned length, unsigned* payload);
#endif /* SECURITY_TEST */

#endif /* !NONSECURE_PROCESSING */

#ifdef TIMED_PLAYBACK_MODE
void accmd_tplay_cback(unsigned con_id, unsigned ep_id, unsigned data);
#endif

#endif /* _ADAPTOR_H_ */
