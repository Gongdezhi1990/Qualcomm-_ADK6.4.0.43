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

#include "connection_id.h"

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


/* Macros used to prepare lists received over transports that are 16-bit wide (such as
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

#define ADAPTOR_PS_SHUTDOWN_COMPLETE_MSG_WORD_SIZE (0)

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
