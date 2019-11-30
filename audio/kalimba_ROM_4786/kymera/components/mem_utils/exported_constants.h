/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup exp_const Exported Constants
 * \ingroup mem_utils
 *
 * \file  
 * \ingroup exp_const
 *
 */
#ifndef EXPORTED_CONSTANTS_H
#define EXPORTED_CONSTANTS_H

#include "sched_oxygen/sched_oxygen.h"
#include "const_data/const_data.h"

#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT

/** 
 * Reserve the requested external_constant 
 *
 * \param source        Pointer to the constant table in ROM, which may be an export stub
 * \param operator_id   Operator ID (which should be system unique)
 *
 * \return FALSE if the table is known to be unavailable. TRUE otherwise.
 */
extern bool external_constant_reserve( void *source , unsigned operator_id);

/** 
 * Release a reservation on the requested external_constant 
 *
 * \param source        Pointer to the constant table in ROM, which may be an export stub
 * \param operator_id   Operator ID (which should be system unique)
 */
extern void external_constant_release( void *source , unsigned operator_id);

/** 
 * Check if the constant table required is accessible
 *
 * This should only be called \b after a call to external_constant_reserve,
 * If the constant is not available then a callback should be requested using
 * external_constant_callback_when_available.
 *
 * \param source        Pointer to the constant table in ROM, which may be an export stub
 * \param operator_id   Operator ID (which should be system unique)
 *
 * \return TRUE if the table has not been exported, or has been loaded, 
 *          is available in RAM and notes this operator as a user
 */
extern bool is_external_constant_available( void *source , unsigned operator_id);

/**
 * Queue a message to be sent when the constants previously reserved are available
 *
 * Note that the message is sent regardless of success. If the exported table happens
 * to be unavailable then it is expected that there will be a subsequent call to
 * external_constant_reserve which will return FALSE.
 *
 * \param source    Pointer to the constant table in ROM, which may be an export stub
 * \param q         ID of queue to send message to
 * \param cmdId     Command ID of message to send
 * \param cmdMsg    Message to send
 * \param rinfo     The routing info for the message
 */
extern void external_constant_callback_when_available(void *source,qid q,uint16 cmdId,void *cmdMsg,tRoutingInfo *rinfo);

/**
 * Update the passed descriptor to point to the equivalent table in RAM (if needed)
 *
 * If the table is not expprted, then the descriptor is not updated as there 
 * is no need.
 *
 * \param start Data descriptor point to the start of a dynamic table in ROM/SQIF
 *
 * \return TRUE if the table is not exported, or is available in RAM. FALSE otherwise.
 */
extern bool external_constant_map_descriptor(const_data_descriptor * start);

/**
 * Function to handle KIP messages for the exported constant code, as received 
 * by P0.
 *
 * The message is only valid for the life of the function call.
 *
 * \param length    length of the kip message
 * \param msg       pointer to the kip message received
 */
extern void external_constant_handle_kip_p0(unsigned length,uint16 *msg);

/**
 * Function to handle KIP messages for the exported constant code, as received 
 * on processors other than P0.
 *
 * The message is only valid for the life of the function call.
 *
 * \param length    length of the kip message
 * \param msg       pointer to the kip message received
 */
extern void external_constant_handle_kip_px(unsigned length,uint16 *msg);

#else /* INSTALL_CAPABILITY_CONSTANT_EXPORT */

#define external_constant_reserve(x,y) TRUE
#define external_constant_release(x,y) ((void)0)
#define is_external_constant_available(x) TRUE
#define external_constant_callback_when_available(s,q,cmdId,cmdMsg,rinfo) ((void)0)
#define external_constant_map_descriptor(x) (TRUE)

#endif

#endif /* EXPORTED_CONSTANTS_H */
