/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file opmgr_private.h
 * \ingroup opmgr
 *
 */

#ifndef OPMGR_PRIVATE_H
#define OPMGR_PRIVATE_H

/****************************************************************************
Include Files
*/

#include "opmgr/opmgr.h"
#include "pmalloc/pl_malloc.h"
#include "string.h"
#include "platform/pl_trace.h"
#include "platform/pl_assert.h"
#include "platform/pl_intrinsics.h"
#include "pl_timers/pl_timers.h"
#include "assert.h"
#include "panic/panic.h"
#include "opmgr_types.h"
#include "patch/patch.h"
#include "fault/fault.h"
#include "opmgr/opmgr_for_adaptors.h"
#include "opmgr/opmgr_for_stream.h"
#include "stream/stream_for_opmgr.h"
#include "adaptor/adaptor.h"
#include "audio_log/audio_log.h"
#ifdef INSTALL_CAP_DOWNLOAD_MGR
#include "opmgr/opmgr_for_cap_download.h"
#include "cap_download_mgr.h"
#endif
#ifdef INSTALL_PIO_SUPPORT
#include "pio.h"
#endif
#include "sys_events.h"
#include "hal_dm_sections.h"

#include "opmsg_prim.h"
#include "cap_id_prim.h"

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#include "kip_mgr/kip_mgr.h"
#include "opmgr/opmgr_kip.h"
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */

/****************************************************************************
Private Type Declarations
*/
/**
 * Store a single operator ID, or a pointer to a list of them
 */
typedef union
{
    unsigned *op_list;
    unsigned id;
} OPERATOR_ID_INFO;

/**
 * data object for keeping track of requests that implicate multiple operators
 */
typedef struct
{
    void *callback;
    unsigned num_ops;
    unsigned cur_index;
    OPERATOR_ID_INFO id_info;
} MULTI_OP_REQ_DATA;

/** Function pointer prototype of pre-processing functions. */
typedef bool (*preproc_function)(struct OPERATOR_DATA *op_data);



/****************************************************************************
Private Constant Declarations
*/



/****************************************************************************
Private Macro Declarations
*/
/** Convert an internal opid into an external opid */
#define INT_TO_EXT_OPID(id) (STREAM_EP_OP_BIT | (id << STREAM_EP_OPID_POSN))
/** Convert an internal opid into an external sink endpoint */

#define INT_TO_EXT_SINK(id) (STREAM_EP_OP_SINK | (id << STREAM_EP_OPID_POSN))

/* Number of parallel start/stop/reset/destroy accmds that we support */
/* Maximum is 32, so that it fits in 5 bits in the connection id      */
#define NUM_AGGREGATES      1

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#define OPMGR_ON_SAME_CORE KIP_ON_SAME_CORE
#define OPMGR_KIP_FREE_REQ_KEYS() opmgr_kip_free_req_keys()
#else
#define OPMGR_ON_SAME_CORE(x) TRUE
#define OPMGR_KIP_FREE_REQ_KEYS()
#endif 

 /**  OP_STD_UNSOLICITED_MSG struct length */
#define UNSOLICITED_MSG_HEADER_SIZE    4
/****************************************************************************
Private Variable Declarations
*/

/** Pointer to head of operators list */
extern OPERATOR_DATA* oplist_head;

/** Pointer to head of 'remote' operators list - this is only for P0 in dual core case.
 */
#ifdef INSTALL_DUAL_CORE_SUPPORT
extern OPERATOR_DATA* remote_oplist_head;
#endif /* INSTALL_DUAL_CORE_SUPPORT */

/** Table of capability data pointers */
extern const CAPABILITY_DATA* const capability_data_table[];

#ifdef DESKTOP_TEST_BUILD
extern bool test_failed;
#endif /* DESKTOP_TEST_BUILD */

/* Download capability database */
#ifdef INSTALL_CAP_DOWNLOAD_MGR
extern DOWNLOAD_CAP_DATA_DB** cap_download_data_list;
#endif

/****************************************************************************
Private Function Declarations
*/

/**
 * \brief Tell streams to kill the operator endpoints before the operator is destroyed.
 *        It is possible that return value and/or more arguments would be needed in future.
 *
 * \param  op_data Pointer to the operator data for the operator.
 *
 */
extern bool opmgr_destroy_op_endpoints(OPERATOR_DATA* op_data);

/**
 * \brief Returns pointer to capability data for a certain cap ID.
 *
 * \param  cap_id The id of the capability
 *
 * \return  A pointer to a constant cap data structure, if not found NULL is returned.
 */
extern const CAPABILITY_DATA* opmgr_lookup_cap_data_for_cap_id(unsigned cap_id);

/**
 * \brief Adds a new capability to the downloadable capabilities list.
 *
 * \param  cap_data The capability to store
 *
 * \return  True if successfully added, or False otherwise
 */
#ifdef INSTALL_CAP_DOWNLOAD_MGR
extern bool opmgr_add_cap_download_data(CAPABILITY_DATA* cap_data);
#endif

/**
 * \brief Remove an existing capability from the downloadable capabilities list.
 *
 * \param  cap_id The capability id to remove
 *
 * \return  True if successfully added, or False otherwise
 */
#ifdef INSTALL_CAP_DOWNLOAD_MGR
extern bool opmgr_remove_cap_download_data(unsigned cap_id);
#endif

/**
 * \brief Stores information about an operator request whilst it is being serviced
 * THIS FUNCTION SHOULD ONLY BE CALLED FROM THE BACKGROUND.
 *
 * Note: This function only fails if malloc fails.
 *
 * \param client_id The ID of the client making the request
 * \param op_id The ID of the operator an action is requested from
 * \param data void pointer to whatever data structure the user needs to store
 * to handle the response message from the operator when it is received.
 *
 * \return TRUE if the task was stored, FALSE if it couldn't be.
 */
extern bool opmgr_store_in_progress_task(unsigned client_id, unsigned op_id,
                                         void *data);

/**
 * \brief Retrieves the callback associated with a request that was sent to an
 * operator. THIS FUNCTION SHOULD ONLY BE CALLED FROM THE BACKGROUND.
 *
 * \param client_id The id of the client that made the request
 * \param op_id The ID of the operator the action was requested from
 *
 * \return A void pointer to the callback function that was stored, NULL if not
 * found.
 */
extern void *opmgr_retrieve_in_progress_task(unsigned client_id, unsigned op_id);

/**
 * \brief Gets the operator structure for the operator with id
 *        on the local core.
 * 
 * \param  id the internal id of the operator.
 * 
 * \return pointer to the operator structure, NULL if not found.
 */
extern OPERATOR_DATA* get_op_data_from_id(unsigned int id);

/**
 * \brief Gets the operator structure for the operator with id
 *        on any core in a multi-core configuration.
 * 
 * \param  id the internal id of the operator.
 * 
 * \return pointer to the operator structure, NULL if not found.
 */
extern OPERATOR_DATA* get_anycore_op_data_from_id(unsigned int id);

/**
 * \brief    Remove the operator data from the operator list
 *
 * \param    id  operator id
 */
extern void remove_op_data_from_list(unsigned int id, OPERATOR_DATA** op_list);

/**
 * \brief Extracts the terminal id of the operator that the endpoint refers to.
 * Any information about direction is lost.
 *
 * \param  opidep the endpoint id of an operator endpoint.
 *
 * \return  the terminal number on the operator the endpoint refers to.
 */
extern unsigned int get_terminal_from_opidep(unsigned int opidep);

/**
 * \brief This function is used for determining whether an operator endpoint is
 * a source.
 *
 * \param  opidep the endpoint id of an operator endpoint.
 *
 * \return  TRUE if opidep is a source, FALSE if a sink.
 */
extern bool get_is_source_from_opidep(unsigned int opidep);

/**
 * \brief This function is used for determining whether an operator endpoint is
 * a sink.
 *
 * \param  opidep the endpoint id of an operator endpoint.
 *
 * \return  TRUE if opidep is a sink, FALSE if a source.
 */
extern bool get_is_sink_from_opidep(unsigned int opidep);

/**
 * \brief This function is used for converting an internal operator id into an
 * external one.
 *
 * \param  opid the id of an operator.
 *
 * \return  The external id of the operator known to the host.
 */
extern unsigned int get_ext_opid_from_int(unsigned int opid);

/**
 * \brief Called when a message is sent to an operator. This looks up the
 * correct function to handle the message that was received from the capabilities
 * handler table, calls it and sends a response message to OpMgr.
 *
 * \param  msg_data pointer to a pointer to the OPERATOR_DATA of the operator. The
 * type is void ** as this is a generic scheduler message handler.
 */
extern void opmgr_operator_task_handler(void **msg_data);

/**
 * \brief Called when a background interrupt is sent to an operator. This is a
 * kick and sets off the data processing function of the operator.
 *
 * \param  bg_data pointer to a pointer to the OPERATOR_DATA of the operator. The
 * type is void ** as this is a generic background interrupt handler.
 */
extern void opmgr_operator_bgint_handler(void **bg_data);

#endif /* OPMGR_PRIVATE_H */
