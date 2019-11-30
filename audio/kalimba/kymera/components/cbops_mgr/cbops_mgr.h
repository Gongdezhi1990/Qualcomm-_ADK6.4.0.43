/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup cbops_mgr Cbops Manager
 *
 * \file cbops_mgr.h
 * \ingroup cbops_mgr
 *
 */

#ifndef _CBOPS_MGR_H_
#define _CBOPS_MGR_H_

/****************************************************************************
Include Files
*/
#include "stream/stream.h"
#include "buffer.h"
#include "cbops/cbops_c.h"
#include "sections.h"

/****************************************************************************
Public Constant Declarations
*/
#define CBOPS_SCRATCH_SIZE  512     /* to match $cbops.MAX_COPY_SIZE in lib/cbops/cbops.h */

/****************************************************************************
Public Type Declarations
*/


/**
 * cbops_mgr structure
 *
 * The cbops_mgr structure describes a CBOPs graph,
 * relies on cbops_graph defined in lib/cbops;
 */
typedef struct cbops_mgr
{
    /** Encapsulated cbops_graph object */
    cbops_graph *graph;
    /** Pointer to a table of required cbops for the cbops chain (specified as a 'flags' bit-field). */
    unsigned req_ops;
    /** Direction of the cbops chain */
    ENDPOINT_DIRECTION dir;
} cbops_mgr;


/* Cbops creation parameters for various operators - TODO: could be abstracted even more... but it's
 * specific to endpoint-owned chains.
 */
typedef struct
{
    /** Pointer to RM amount */
    unsigned *rate_adjustment_amount;
    /** Pointer to endpoint block size */
    unsigned *block_size_ptr;
    /** RM headroom as set by endpoints */
    unsigned rm_headroom;
    /** Shift amount */
    unsigned shift_amount;
    /** Pointer to RM diff amount calculated by endpoint */
    int      *rm_diff_ptr;
    /** Estimated arriving data lump size at endpoint level */
    unsigned *data_block_size_ptr;
    /** Pointer to delta samples calculated by endpoint level RM */
    unsigned *delta_samples_ptr;
    /** Pointer to total inserts count */
    unsigned *total_inserts_ptr;
    /** Pointer to vector of values used for insertion on each channel - if NULL, silence is used */
    unsigned *insertion_vals_ptr;
    /** Pointer to synchronised data start flag */
    bool     *sync_started_ptr;
} CBOP_VALS;


/****************************************************************************
Public Constant Declarations
*/


/****************************************************************************
Public Function Declarations
*/

/**
 * Creates a new cbops_mgr object.
 *
 * This function creates the cbops_mgr
 *
 * \param dir direction of the stream.
 * \param initial_cbops_flags for the cbops mgr.
 *
 * \return pointer to allocated object if successful (NULL otherwise).
 */
extern cbops_mgr* cbops_mgr_create(ENDPOINT_DIRECTION dir, unsigned initial_cbops_flags);

/**
 * Appends new CBOP operators as described by bit-field cbops_flags.
 *
 * This function tears down and re-creates the cbops chain.
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object previously allocated (create)
 * \param cbops_flags to update
 * \param nr_chans number of channels
 * \param source_buf needed to recreate the cbops_chain
 * \param sink_buf needed to recreate the cbops_chain
 * \param vals Pointer to configuration values
 * \param force_chain_update if TRUE, chain update happens irrespective of the buffer information being complete
 * \return boolean indicating if the operation was successful.
 */
extern bool cbops_mgr_append(cbops_mgr* cbops_mgr_obj, unsigned cbops_flags, unsigned nr_chans,
                             tCbuffer *source_buf[], tCbuffer *sink_buf[],
                             CBOP_VALS *vals, bool force_chain_update);

/**
 * Removes CBOP operators as described by bit-field cbops_flags.
 *
 * This function tears down and re-creates the cbops chain according to the remaining flags.
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object previously allocated (create)
 * \param cbops_flags to update
 * \param nr_chans number of channels
 * \param source_buf needed to recreate the cbops_chain
 * \param sink_buf needed to recreate the cbops_chain
 * \param force_chain_update if TRUE, chain update happens irrespective of the buffer information being complete
 * \return boolean indicating if the operation was successful.
 */
extern bool cbops_mgr_remove(cbops_mgr* cbops_mgr_obj, unsigned cbops_flags, unsigned nr_chans,
                             tCbuffer *source_buf[], tCbuffer *sink_buf[], bool force_chain_update);

/**
 * Returns the cbops flags which describes the cbops operators of the cbops manager.
 *
 * This function returns all the implemented cbops operations in form of flags.
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object previously allocated (create)
 * \return unsigned containing the the cbops flags.
 */
extern unsigned cbops_get_flags(cbops_mgr* cbops_mgr_obj);

/**
 * Destroys completely the cbops structure pointed by ep.
 *
 * This function frees the whole cbops linked list and then the cbops_mgr object
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object previously allocated (create)
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_destroy(cbops_mgr* cbops_mgr_obj);

/**
 * Executes cbops chain / graph.
 *
 * This function executes the previously created cbops chain.
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object previously allocated (create)
 * \param max_amount max amount of data in samples to process.
 */
INLINE_SECTION static inline void cbops_mgr_process_data(cbops_mgr* cbops_mgr_obj, unsigned max_amount)
{
    cbops_process_data(cbops_mgr_obj->graph, max_amount);
}

/**
 * Obtains source/sink buffers and creates a new cbops chain.
 *
 * This function receives source/sink buffers when the endpoint has been connected
 * and creates cbops_chain for these buffers according to the cbops_flags stored in
 * cbops_mgr.
 *
 * \param cbops_mgr_obj pointer to the cbops manager.
 * \param nr_chans number of channels
 * \param source_buf source buffers.
 * \param sink_buf sink buffers.
 * \param vals Creation/connection-time parameters for the cbops in the chain
 * \return indicates if the operation was successful.
 */
bool cbops_mgr_connect(cbops_mgr* cbops_mgr_obj, unsigned nr_chans, tCbuffer *source_buf[], tCbuffer *sink_buf[], CBOP_VALS *vals);

/**
 * Destroys the cbops chain stored in the endpoint received.
 *
 * This function frees the linked list stored in head.
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object previously allocated (create)
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_disconnect(cbops_mgr* cbops_mgr_obj);

/**
 * Re-initialises the cbops buffer tables based on the cbuffers at the point of call,
 * initialises also the data amounts calculated previously.
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_buffer_reinit(cbops_mgr* cbops_mgr_obj);

/**
 * Updates the cbops buffer tables based on the cbuffers at the point of call.
 * It does not initialise calculated amounts.
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_update_buffer_info(cbops_mgr* cbops_mgr_obj);

/**
 * \brief Return how much data was actually written into the sink buffer the
 * last time cbops_mgr_process_data was called.
 *
 * \param cbops_mgr_obj pointer to cbops_mgr object previously allocated (create)
 * \return The amount of data written on the last kick.
 */
INLINE_SECTION static inline unsigned cbops_mgr_amount_written(cbops_mgr* cbops_mgr_obj)
{
    return cbops_get_amount(cbops_mgr_obj->graph,cbops_mgr_obj->graph->num_io>>1);
}


/**
 * \brief Set a channel to "not used" in the cbops chain (for all operators and framework).
 *        This NULLs the cbuffer ptrs and the channel index entries for the given channel
 *
 * \param cbops    pointer to cbops_mgr object
 * \param channel  Channel number to set to unused (0...N-1, N is the number of (equal) input and
 *                 output channels. This can be further generalised if ever needed by cbops_mgr-owned
 *                 chains.
 * \return TRUE/FALSE indicates if the operation was successful.
 */
extern bool cbops_mgr_set_unused_channel(cbops_mgr* cbops_mgr_obj, unsigned channel);



/**
 * Obtains source/sink buffer information.
 *
 * \param cbops_mgr_obj pointer to the cbops manager.
 * \param nr_chans number of channels
 * \param in_buffs source buffers.
 * \param outbuffs sink buffers.
 * \param vals Creation/connection-time parameters for the cbops in the chain
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_get_buffer_info(cbops_mgr* cbops_mgr_obj, unsigned nr_chans, tCbuffer **in_buffs, tCbuffer **outbuffs);

/**
 * Allocate source/sink buffer information.
 *
 * \param cbops_mgr_obj pointer to the cbops manager.
 * \param nr_chans number of channels
 * \param in_buffs source buffers array that gets allocated.
 * \param outbuffs sink buffers array that gets allocated.
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_alloc_buffer_info(cbops_mgr* cbops_mgr_obj, unsigned *nr_chans, tCbuffer ***in_buffs, tCbuffer ***outbuffs);

/**
 * Frees the source/sink buffer information
 *
 * \param in_buffs source buffers array returned by cbops_mgr_get_buffer_info call
 * \param out_buffs sink buffers array returned by cbops_mgr_get_bufer_info call
 */
extern void cbops_mgr_free_buffer_info(tCbuffer **in_buffs, tCbuffer **out_buffs);

/**
 * Get source/sink buffer information for a certain channel.
 *
 * \param cbops_mgr_obj pointer to the cbops manager.
 * \param channel  the number of the channel (as input-to-output channel, considering equal number of inputs and outputs)
 * \param in_buff source buffer
 * \param out_buff sink buffer
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_get_channel_buffers(cbops_mgr* cbops_mgr_obj, unsigned channel, tCbuffer **in_buff, tCbuffer **out_buff);

/**
 * Connect source/sink buffer information for a certain channel.
 *
 * \param cbops_mgr_obj pointer to the cbops manager.
 * \param channel  the number of the channel (as input-to-output channel, considering equal number of inputs and outputs)
 * \param in_buff source buffer
 * \param out_buff sink buffer
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_connect_channel(cbops_mgr* cbops_mgr_obj, unsigned channel,
                               tCbuffer *source_buf, tCbuffer *sink_buf);

/**
 * Default channel indexes for a given channel.
 *
 * \param cbops_mgr_obj pointer to the cbops manager.
 * \param channel  the number of the channel (as input-to-output channel, considering equal number of inputs and outputs)
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_default_channel_idx(cbops_mgr* cbops_mgr_obj, unsigned channel);

/**
 * Retrieve some alg parameters from the cbops chain.
 *
 * \param cbops_mgr_obj pointer to the cbops manager.
 * \param vals pointer to a CBOP_VALS struct of parameters that will get filled based on cbop chain's params
 * \return indicates if the operation was successful.
 */
extern bool cbops_mgr_get_op_vals(cbops_mgr* cbops_mgr_obj, CBOP_VALS* vals);

/**
 * \brief Utility function for configuring the passthrough mode of the rateadjust and shift
 * cbop in a cbops_mgr graph.
 *
 * \param cbops_mgr_obj The cbops_mgr containing the graph to configure the mode of
 * \param enable Boolean representing the state to set the passthrough mode to.
 *
 * \return Indicates if the cbop operator was found and configured
 */
extern bool cbops_mgr_rateadjust_passthrough_mode(cbops_mgr *cbops_mgr_obj, bool enable);

/**
 * \brief Find a cbop in the chain that has a matching function table.
 *
 * \param head        Pointer to cbops chain head
 * \param func_table  Pointer to function table of a certain type of operator
 * \return Pointer to found cbop (or NULL if not found)
 */
extern cbops_op* find_cbops_op(cbops_graph* head, unsigned func_table[]);


#endif /*_CBOPS_MGR_H_*/
