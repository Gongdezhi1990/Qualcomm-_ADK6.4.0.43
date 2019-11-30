/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file ps_for_adaptors.h
 * \ingroup ps
 *
 * Persistent Storage API for adaptors.
 */

#ifndef PS_FOR_ADAPTORS_H
#define PS_FOR_ADAPTORS_H

#include "ps_common.h"

/** API functions - NOTE: some may be unsupported on certain platform(s). */

/**
 * \brief Signals a PS shutdown.
 *
 * \param conn_id The connection ID (sender and recipient ID codes).
 * \param cback Pointer to callback function.
 */
extern void ps_signal_shutdown(unsigned conn_id, PS_SHUTDOWN_CALLBACK cback);

#ifdef INSTALL_PS_FOR_ACCMD
/**
 * \brief Function called for each response to the request created
 *        by the fragmentation of ps_read_entry.
 *
 * \param success Indicate whether the operation was successful or not.
 * \param total_size Total size of the block of memory from which
 *        the payload in this message has been extracted.            
 * \param payload_size The number of uint16 present in *payload.
 * \param payload The data present in the message.
 */
extern void ps_read_resp(bool success, unsigned total_size,
                         unsigned payload_size, uint16 *payload);

/**
 * \brief Function called for each response to the request created
 *        by the fragmentation of ps_write_entry.
 *
 * \param success Indicate whether the operation was successful or not.
 */
extern void ps_write_resp(bool success);

/**
 * \brief Tell PS which connection to send messages on.
 *
 * \param conn_id The connection ID (sender and recipient ID codes).
 */
extern void ps_register(unsigned conn_id);

extern void ps_kip_read_req(unsigned msg_length, uint16* msg_data);
extern void ps_kip_write_req(unsigned msg_length, uint16* msg_data);
extern void ps_kip_delete_req(unsigned msg_length, uint16* msg_data);
extern void ps_kip_shutdown_req(unsigned msg_length, uint16* msg_data);
extern void ps_kip_shutdown_complete_req(unsigned msg_length, uint16* msg_data);
extern void ps_kip_read_res(unsigned msg_length, uint16* msg_data);
extern void ps_kip_write_res(unsigned msg_length, uint16* msg_data);

#endif

#endif /* PS_FOR_ADAPTORS_H */
