/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  base_op.h
 * \ingroup capabilities
 *
 * Base operator public header file. <br>
 *
 */

#ifndef BASE_OP_H
#define BASE_OP_H


/* Default block size, 1 basically means "continuous" i.e. no blockiness
 *
 * NOTE: NB base_Op would not anyway do data processing as such, and anybody deriving a
 * capability from base_op MUST redefine these defaults with the best suited specific values
 * for that capability!
 */
#define BASE_OP_BLOCK_SIZE          1

/* Default version length that is returned by operators (two 16-bit words, with major and minor
 * revision number). Some capability code may wish to customise it in order to return some extra information.
 */
#define BASE_OP_VERSION_LENGTH      2

/* Handler function declarations - assumption at the moment is that, as base_op gets re-used partially by other operators, */
/* these functions are published in this header. Other operators' handler functions will stay private unless some common */
/* and re-used functionality is provided - e.g. fade-out could be such case. */
extern bool base_op_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_stop(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern OP_SCHED_INFO_RSP* base_op_get_sched_info_ex(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id);
extern bool base_op_set_block_size(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool base_op_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern void base_op_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);

/* Operator message handlers */
extern bool base_op_opmsg_get_capability_version(OPERATOR_DATA *op_data, void *message_data, unsigned int *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Some "helper" functions that any operator can use to perform some common sequences of operations. */

/**
 * \brief Check whether all terminals are connected. NOTE that this will only return TRUE if
 *        all terminals as specified by capability data (max number of sources and sinks) are connected.
 *        Capability code that allows a subset of terminals connected should not use this helper function.
 *
 * \param  op_data  Pointer to the operator structure.
 *
 * \return TRUE if yes, FALSE for no.
 */
extern bool base_op_are_all_terminals_connected(OPERATOR_DATA* op_data);

/**
 * \brief Checks whether the terminal ID is valid for a certain operator.
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  terminal_id  Numeric ID of the operator terminal.
 *
 * \return TRUE if yes, FALSE for no.
 */
extern bool base_op_is_terminal_valid(OPERATOR_DATA* op_data, unsigned terminal_id);

/**
 * \brief Build a simple "standard" response message.
 *
 * \param  status  A status ID that is to be sent in the message.
 * \param  id  Message ID
 * \param  response_data  Pointer to the pointer to message that is allocated in the function.
 *
 * \return TRUE if yes, FALSE for no.
 */
extern bool base_op_build_std_response(STATUS_KYMERA status, unsigned id, void **response_data);

/**
 * \brief Build a simple "standard" response message.
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  status  A status ID that is to be sent in the message.
 * \param  response_data  Pointer to the pointer to message that is allocated in the function.
 *
 * \return TRUE if yes, FALSE for no.
 */
extern bool base_op_build_std_response_ex(OPERATOR_DATA* op_data, STATUS_KYMERA status, void **response_data);

/**
 * \brief Extract a pointer to capability-specific per-instance data
 *        from the generic operator data structure.
 *
 * \param  op_data    Pointer to the operator structure.
 *
 * \return The pointer to the operator private structure.
 */
extern void* base_op_get_instance_data(OPERATOR_DATA *op_data);

/**
 * \brief Return the capability id of a given operator.
 *        This is useful for functions shared by different
 *        capabilities.
 *
 * \param  op_data    Pointer to the operator structure.
 *
 * \return The capability id of the operator being queried
 */
extern CAP_ID base_op_get_cap_id(OPERATOR_DATA *op_data);

/**
 * \brief Return the internal operator id of a given operator.
 *
 * \param  op_data    Pointer to the operator structure.
 *
 * \return The internal id of the operator being queried
 */
extern INT_OP_ID base_op_get_int_op_id(OPERATOR_DATA *op_data);

/**
 * \brief Return the external operator id of a given operator.
 *        For capabilities, this is mostly useful for debug
 *        messages.
 *
 * \param  op_data    Pointer to the operator structure.
 *
 * \return The internal id of the operator being queried
 */
extern EXT_OP_ID base_op_get_ext_op_id(OPERATOR_DATA *op_data);

/**
 * \brief Make a copy of an operator data.
 *        This is useful for callbacks for which the operator
 *        might have been destroyed.
 *
 * \param  op_data    Pointer to the operator structure.
 *
 * \return A copy of the operator data
 */
extern OPERATOR_DATA *base_op_clone_operator_data(OPERATOR_DATA *op_data);

#ifdef PROFILER_ON
extern void base_op_profiler_start(OPERATOR_DATA *op_data);
extern void base_op_profiler_stop(OPERATOR_DATA *op_data);
extern void base_op_profiler_add_kick(OPERATOR_DATA *op_data);
#else
#define base_op_profiler_start(x)
#define base_op_profiler_stop(x)
#define base_op_profiler_add_kick(x)
#endif

/**
 * \brief Ensure the framework will not call an operator
 *        processing function. Its effect is similar to
 *        receiving a "stop" message but the relevant
 *        handling function is not called.
 *
 * \param  op_data    Pointer to the operator structure.
 *
 * \note For capabilities that need to lock a section, use
 *       the functions "opmgr_op_suspend_processing" and
 *       "opmgr_op_resume_processing" instead.
 */
extern void base_op_stop_operator(OPERATOR_DATA *op_data);

/**
 * \brief Returns the capability data associated with 
 *        an operator. This is useful for capabilities that
 *        allow other capabilities to derived from them.
 *        (e.g. "base_multi_chan_op" and "encoder").
 *
 * \param  op_data    Pointer to the operator structure.
 *
 * \note Files that provide multiple capabilities like
 *       "basic_passthrough" usually just need "base_op_get_cap_id".
 */
extern const CAPABILITY_DATA* base_op_get_cap_data(OPERATOR_DATA *op_data);

/**
 * \brief Returns the class extra data associated with 
 *        an operator. This is useful for capabilities that
 *        allow other capabilities to derived from them.
 *        (e.g. "base_multi_chan_op" and "encoder").
 *
 * \param  op_data    Pointer to the operator structure.
 *
 * \note This data is defined by the base class. The derived classes
 *       get this information via "base_op_get_instance_data".
 */
extern void* base_op_get_class_ext(OPERATOR_DATA *op_data);

/**
 * \brief Set the class extra data associated with 
 *        an operator.
 *
 * \param  op_data    Pointer to the operator structure.
 * \param  class_data Pointer to the class data to be linked.
 *
 * \note See "base_op_get_class_ext" for more information.
 */
extern void base_op_set_class_ext(OPERATOR_DATA *op_data, void *class_data);

/**
 * \brief Change the status of an already allocated "standard" response message.
 *
 * \param  response_data Pointer to the pointer to message that is allocated in the function.
 * \param  new_status  A status ID that is to be sent in the message.
 */
inline static void base_op_change_response_status(void **response_data, STATUS_KYMERA new_status)
{
    (((OP_STD_RSP *)(*response_data))->status = new_status);
}

/* Define a macro to declare the mapping between a capability identifier
   and the type used by the capability-specific per-instance data.
   The information is stored in the ELF and used by ACAT. */
#if !defined(__GNUC__)
#define GLOBAL_DEBUG_STRING_ATTR _Pragma("datasection GLOBAL_DEBUG_STRINGS")
#define MAP_INSTANCE_DATA(CAPID,CAPTYPE) \
    GLOBAL_DEBUG_STRING_ATTR const char ACAT_INSTANCE_TYPE_##CAPID[] = "" #CAPTYPE "";
#else
#define MAP_INSTANCE_DATA(CAPID,CAPTYPE)
#endif

#endif /* BASE_OP_H */
