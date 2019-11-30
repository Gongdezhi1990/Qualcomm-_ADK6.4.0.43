/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup kip adaptor
 * \ingroup adaptor
 *
 * \file kip_msg_adaptor.h
 * \ingroup kip
 *
 *
 */

#ifndef _KIP_ADAPTOR_H_
#define _KIP_ADAPTOR_H_

#include "types.h"
#include "kip_mgr/kip_mgr.h"

/****************************************************************************
Public Macros
*/

#define KIP_CONID_SEND_CLIENT_MASK    0xF800
#define KIP_CONID_RECV_CLIENT_MASK    0x00F8


/****************************************************************************
Public Functions
*/
/**
 * \brief    Register KIP REQ information to be used when KIP RESP comes back.
 *
 * \param    sender_id  The sender part of connection id in the request
 * \param    msg_id     The request message id
 * \param    seq_nr     The sequence number
 * \param    context    Context ( it can be a callback or data )
 */
extern bool kip_record_in_progress_req( unsigned con_id, unsigned msg_id,
                                        uint16 seq_nr, void *context);

/**
 * \brief    Retrieve the request information when the response comes back.
 *           We may only pass the receiver part of con_id,to be then matched up with
 *           receiver part of RESP con_id along with the msg_id
 *
 * \param    sender_id  The sender part of connection id in the request
 * \param    msg_id     The request message id
 * \param    *seq_nr    The sequence number to return
 *
 * \return void*       Context
 */

extern void* kip_retrieve_in_progress_req(unsigned con_id,  unsigned msg_id,
                                           uint16* seq_nr);
extern void kip_adaptor_init(void);

/* Function for sending KIP message - NOTE: the connection ID is inside payload. */
extern bool kip_adaptor_send_message( unsigned conid, unsigned msg_id,
                                      unsigned length, unsigned *msg_data,
                                      void* context);

/**
 * Helper function for sending variable length key-value system messages to P1
 * @param  key_value_pairs An array of uint16 key-value pairs
 * @param  num_pairs       Number of key-value pairs in the KIP message
 * @return                 Returns false if memory allocation for sending the
 *                         message fails or if called from the wrong processor.
 */
extern bool kip_adaptor_send_system_key_value_pairs(uint32* key_value_pairs,
                                                    uint16 num_pairs);


/*
 * Functions registered as received message notification handler.
 * One for P0, to receive KIP response messages from secondary processor.
 * One for Px, to receive KIP request messages from P0.
 */
extern void kip_adaptor_msg_handler_p0(uint16 channel_id, ipc_msg* msg);
extern void kip_adaptor_msg_handler_px(uint16 channel_id, ipc_msg* msg);

#ifdef AUDIO_SECOND_CORE
bool kip_send_msg_lic_mgr_query(unsigned length, unsigned* payload);
#endif

#ifdef INSTALL_DUAL_CORE_SUPPORT
bool kip_reply_if_kip_request_pending(unsigned length, unsigned* payload);
void kip_configure_pending_query(void);
#endif

#endif /* _KIP_ADAPTOR_H_ */
