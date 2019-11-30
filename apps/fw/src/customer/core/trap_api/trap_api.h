/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Public header for firmware interfaces to the trap API implementation,
 * primarily the IPC receive handlers
 */

#ifndef TRAP_API_H_
#define TRAP_API_H_

#include "hydra/hydra_types.h"
#include "ipc/ipc_msg_types.h"
#include "trap_api/message.h"
#include <sink.h>

/**
 * Pass the given message to the given firmware message channel handler task,
 * applying filtering as required
 *
 * \note This function is essentially equivalent to "vm_send_message_filtered"
 * in vm_message.c in the BC VM
 *
 * @param msg_type Firmware message channel ID
 * @param id Message int
 * @param message Message body
 * @param allow_duplicates Whether to filter the task's queue for duplicates
 *
 * \ingroup trap_api_impl
 */
extern void trap_api_send_message_filtered(IPC_MSG_TYPE msg_type,
                                           uint16 id,
                                           void *message,
                                           bool allow_duplicates);

/**
 * Pass the given message to the given Sink or Source firmware message
 * handler task, applying filtering as required
 *
 * @param stream_id ID of the sink or source to post the message to
 * @param id Message int
 * @param message Message body
 * @param allow_duplicates Whether to filter the task's queue for duplicates
 * @return TRUE if the message was handled; FALSE if not (because it is for a
 * high-priority task, but the IPC is blocked on an atomic response, so the
 * task handler isn't allowed to run)
 *
 * \ingroup trap_api_impl
 */
extern bool trap_api_send_sink_source_message_filtered(uint16 stream_id,
                                                       uint16 id,
                                                       void *message,
                                                       bool allow_duplicates);

/**
 * Pass the given message to the given firmware handler task, applying filtering
 * as required
 *
 * @param task Firmware handler task
 * @param id Message int
 * @param message Message body
 * @param allow_duplicates Whether to filter the task's queue for duplicates
 *
 * \ingroup trap_api_impl
 */
extern void trap_api_send_message_to_task_filtered(Task task,
                                                   uint16 id,
                                                   void *message,
                                                   bool allow_duplicates);

/**
 * Pass the give Bluestack primitive up as a message to the appropriate handler
 * @param protocol The Bluestack protocol ID (see "Primitive segmentation" in
 * bluetooth.h
 * @param prim The primitive
 */
extern void trap_api_send_bluestack_message(uint16 protocol, void *prim);

/**
 * Type definiton for supported IPC PIO operation.
 */
typedef enum pio_ipc_op
{
    ACQUIRE = 0,
    RELEASE,
    SET_OWNER,
    GET_OWNER,
    SET_PULLEN,
    GET_PULLEN,
    SET_PULLDIR,
    GET_PULLDIR,
    SET_PULLSTR,
    GET_PULLSTR,
    GET_UNUSED,
    SET_PIO_MUX,
    GET_PIO_MUX,
    SET_PAD_MUX,
    GET_PAD_MUX,
    SET_DRIVE_STRENGTH,
    GET_DRIVE_STRENGTH,
    SET_STICKY,
    GET_STICKY,
    SET_SLEW,
    GET_SLEW,
    SET_XIO_MODE,
    GET_XIO_MODE,
    SET_DRIVE_ENABLE,
    GET_DRIVE_ENABLE,
    SET_DRIVE,
    GET_DRIVE,
    SET_FUNC_BITSERIAL,
    SET_FUNC_UART
} pio_ipc_op;

/**
 * Main function for doing an IPC PIO operation. This is used to alter PIO
 * settings which are not available to Apps P1.
 * \param op IPC PIO operation.
 * \param bank PIO bank index.
 * \param mask PIO mask.
 * \param value Mask representing a parameter for each PIO. The actual
 * parameter depends on the type of operation chosen.
 * \return Mask indicating a pass or fail status for each PIO (0 - Pass,
 * 1 - Fail).
 */
uint32 pio_ipc(pio_ipc_op op, uint16 bank, uint32 mask, uint32 value);

#ifndef DESKTOP_TEST_BUILD

/**
 * Pass the given character for logging to hydra_logging mechanism.
 *
 * Note: the declaration needs to be removed when we upgrade to KCC 48!
 *
 * @param c character to log
 *
 * \ingroup trap_api_impl
 */
extern void putchar(uint16 c);
#endif /* !DESKTOP_TEST_BUILD */

#endif /* TRAP_API_H_ */
