/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_c.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_C_H_
#define _CBOPS_C_H_

#include "operators/cbops_copy_op_c.h"
#include "operators/cbops_shift_c.h"
#include "operators/cbops_dc_remove_c.h"
#include "operators/cbops_rate_adjustment_and_shift_c.h"
#include "log_linear_cbops/log_linear_cbops_c.h"
#include "operators/cbops_iir_resamplerv2_op.h"
#include "operators/cbops_sidetone_filter.h"
#include "operators/cbops_underrun_comp_op.h"
#include "operators/cbops_discard_op.h"
#include "operators/cbops_sink_overflow_disgard_op.h"
#include "buffer.h"

/****************************************************************************
Public Constant Declarations
*/
#define CBOPS_NO_MORE_OPERATORS_PTR           ((cbops_op*)NULL)
#define CBOPS_BUFFER_NOT_SUPPLIED   (-1)

/* Maximum number of channels - for now until ever deeding it differently, same number of max in/out channels are considered */
#define CBOPS_MAX_NR_CHANNELS       8
#define CBOPS_MAX_COPY_SIZE         512 /* the equivalent of asm $cbops.MAX_COPY_SIZE in cbops.h */

/****************************************************************************
Public Type Declarations
*/
typedef enum cbops_buffer_type
{
   CBOPS_IO_SINK     = 0x0001,
   CBOPS_IO_SOURCE   = 0x0002,
   CBOPS_IO_INTERNAL = 0x0004,
   CBOPS_IO_SHARED   = 0x8000
}cbops_buffer_type;

typedef struct cbops_buffer
{
   cbops_buffer_type  type;
   tCbuffer          *buffer;
   void              *base;
   unsigned          size;
   void              *rw_ptr;
   unsigned          *transfer_ptr;       // Pointer to shared amount to transfer - muli-channel optimisation
   unsigned          transfer_amount;     // Amount to transfer
}cbops_buffer;

typedef struct cbops_functions
{
   void *reset;
   void *amount_to_use;
   void *process;
}cbops_functions;

/** The "header" of cbop parameter structure - this is followed by cbop-specific parameters */
typedef struct cbops_param_hdr
{
    /** Pointer to operator parameters */
    void *operator_data_ptr;
    /** Number of input channels at creation time */
    unsigned nr_inputs;
    /** Number of output channels at creation time */
    unsigned nr_outputs;
    /** Table of input, and then output channel indexes in buffer info table.
     *  This table has always nr_inputs + nr_outputs entries.
     *  This table may have zero length if there are zero in & out channels (see certain
     *  ops like HW rate monitor).
     */
    unsigned index_table[];
} cbops_param_hdr;


typedef struct cbops_op
{
   /** The next operator in the chain */
    struct cbops_op *prev_operator_addr;
    /** The next operator in the chain */
    struct cbops_op *next_operator_addr;
    /** The function table of this cbop operator */
    void *function_vector;

    /** cbop operator specific data fields - it begins with a cbops_param_hdr structure */
    unsigned parameter_area_start[];
} cbops_op;


/**
 * cbops_graph structure
 *
 * The cbops_graph structure describes a CBOPs graph

 */
typedef struct cbops_graph
{
    /** linked list of operators */
    cbops_op   *first;
    cbops_op   *last;
    /** Graph Support data*/
    unsigned         num_io;             /* number of buffers */
    unsigned        *override_data;        /* cbops override operator data*/
    cbops_functions  override_funcs;     /* cbops override functions */
    unsigned         refresh_buffers;    /* Signal buffer reset required */
    unsigned         force_update;       /* Signal processing must occur */
    /** Graph IO Table */
    cbops_buffer   buffers[];
}cbops_graph;

/****************************************************************************
Public Macro Declarations
*/


/* Pointer to where cbop parameters begin in cbops_op - this is where
 * cbops_param_hdr begins.
 */
#define CBOPS_OPERATOR_DATA_PTR(op)           (&((op)->parameter_area_start[0]))

/* Size of cbop parameter structure header: nr ins, nr outs, indexes for ins & outs */
#define CBOPS_PARAM_HDR_SIZE(ni, no)          (sizeof(cbops_param_hdr) + (ni + no)*sizeof(unsigned))


/** Used for getting the memory allocation size required for the specified multi-channel cbop operator.
 *  NOTE: the operator's create() function is to then take into account what extras are needed to be allocated
 *  on top of what below macro calculates.
 *  Example: a single channel dc removal cbop will arrive at the right size with below macro as there is one
 *  channel-specific algorithm parameter in its param struct definition. If it was created with more than one
 *  channel, then it has to add the size of the extra channels' parameters (in dc removal case, N-1 extra
 *  integers, as one channel is accounted for already).
 */
#define sizeof_cbops_op(op_name, ni, no) \
             (sizeof(cbops_op) + sizeof(op_name) + CBOPS_PARAM_HDR_SIZE(ni, no))

/* Pointer to cbop-specific parameter start (this is after cbops_param_hdr structure) */
#define CBOPS_PARAM_PTR(op, op_name) \
             ((op_name*)(((cbops_param_hdr*)CBOPS_OPERATOR_DATA_PTR(op))->operator_data_ptr))




/****************************************************************************
Public Function Declarations
*/
/**
 * Initialise buffer tables.
 *
 * Initialises the buffer tables when a chain is created.
 *
 * \param head pointer to the head of the cbops chain structure.
 * \param fw_obj framework internal object pointer.
 * \return none.
 */
extern void cbops_reinit_buffers(cbops_graph *graph);

/**
 * Executes cbops chain / graph.
 *
 * This function executes the cbops graph.
 *
 * \param graph pointer to cbops_graph object describing the graph.
 * \param max limit on the amount of data in samples to process.
 */
extern void cbops_process_data(cbops_graph* graph, unsigned max_amount);

/**
 * \brief Retrieves the amount of data written by a cbops graph run.
 *
 * \param index The index of a buffer for the graph
 *
 * \return The amount of data consumed/produced in buffer
 * cbops_process_data was called
 */
extern unsigned cbops_get_amount(cbops_graph* graph, unsigned index);

/**
 * Accessor functions
 *
 * Get or Set one of next items of private data obfuscated in io[] area of the head
 *      number of inputs / outputs
 *      pointer to I/O or scratch buffer
 *    for a chain
 *
 *  Caution. The accessors are low-level, fast access functions, assuming the head
 *          structure is valid, there are no checks.
 *
 * \param head pointer to the head of the cbops chain structure.
 * \return none or requested item for this chain
 */
extern unsigned cbops_get_num_inputs(cbops_graph *head);
extern unsigned cbops_get_num_outputs(cbops_graph *head);

/**
 * dynamically create a cbops graph
 *
 *
 * \param number of io buffers in the graph
 * \return pointer to allocated graph
 */
cbops_graph* cbops_alloc_graph(unsigned num_io);

/**
 * destroy a cbops graph that was dynamically created
 *
 * \param pointer to graph
 * \return none
 */
void destroy_graph(cbops_graph *graph);

/**
 * destroy all operators in a graph
 *
 * \param pointer to graph
 * \return none
 */
void cbops_free_operators(cbops_graph *graph);

/**
 * associate an input buffer with a graph
 *
 * \param pointer to graph
 * \param buffer index in graph
 * \param linked buffer index in graph
 * \param pointer to buffer
 * \return NONE
 */
void cbops_set_input_io_buffer(cbops_graph *graph,unsigned index,unsigned share_index,tCbuffer *buf);

/**
 * associate an output buffer with a graph
 *
 * \param pointer to graph
 * \param buffer index in graph
 * \param linked buffer index in graph
 * \param pointer to buffer
 * \return NONE
 */

void cbops_set_output_io_buffer(cbops_graph *graph,unsigned index,unsigned share_index,tCbuffer *buf);

/**
 * associate an internal buffer with a graph
 *
 * \param pointer to graph
 * \param buffer index in graph
 * \param linked buffer index in graph
 * \param pointer to buffer
 * \return NONE
 */
void cbops_set_internal_io_buffer(cbops_graph *graph,unsigned index,unsigned share_index,tCbuffer *buf);


/**
 * append an operator to a graph
 *
 * \param pointer to graph
 * \param pointer to operator
 * \return NONE
 */
void cbops_append_operator_to_graph(cbops_graph *graph,cbops_op *op);

/**
 * pre-pend an operator to a graph
 *
 * \param pointer to graph
 * \param pointer to operator
 * \return NONE
 */
void cbops_prepend_operator_to_graph(cbops_graph *graph,cbops_op *op);


/**
 * set the override operator for the graph
 *
 * \param pointer to graph
 * \param pointer to operator
 * \return NONE
 */
void cbops_set_override_operator(cbops_graph *graph,cbops_op *op);

/**
 * Set input / output channel buffer indexes
 *
 * \param op pointer to cbop
 * \param channel number of the input/output channel (0,...)
 * \param idx  index in buffer table
 * \return TRUE/FALSE depending on success
 */
extern bool cbops_set_input_idx(cbops_op* op, unsigned channel, unsigned idx);
extern bool cbops_set_output_idx(cbops_op* op, unsigned channel, unsigned idx);


/**
 * Signal buffer change
 *
 * \param op pointer to cbop
 */
static inline void cbops_reshresh_buffers(cbops_graph *graph)
{
   graph->refresh_buffers = TRUE;
}


/**
 * Create and fill an index table with default indexes.
 *
 * \param nr_ins   Number of input channels at creation time.
 * \param nr_outs  Number of output channels at creation time.
 */
extern unsigned* create_default_indexes(unsigned nr_io);

/**
 * Fill the channel number & channel index-related header part of cbop param struct (multi-channel)
 *
 * \param op       Pointer to cbop param structure
 * \param nr_ins   Number of input channels at creation time.
 * \param nr_outs  Number of output channels at creation time.
 * \param input_idx     Pointer to input channel indexes.
 * \param output_idx    Pointer to output channel indexes.
 */
extern void* cbops_populate_param_hdr(cbops_op* op,
                                     unsigned nr_ins, unsigned nr_outs,
                                     unsigned* input_idx, unsigned* output_idx);

/* cbops operator specific support */

/**
 * create a G.711 loglinear operator (multi-channel)
 *
 * \param nr_channels   Number of channels at creation time.
 * \param input_idx     Pointer to input channel indexes.
 * \param output_idx    Pointer to output channel indexes.
 * \param mapping_func  Address of the conversion function (u-law or A-law).
 * \return pointer to operator
 */
cbops_op* create_g711_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx, void* mapping_func);

/**
 * create a copy operator (multi-channel)
 *
 * \param nr_channels   Number of channels at creation time.
 * \param input_idx     Pointer to input channel indexes.
 * \param output_idx    Pointer to output channel indexes.
 * \return pointer to operator
 */
cbops_op* create_copy_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx);

/**
 * create a DC offset removal operator (multi-channel)
 *
 * \param nr_channels   Number of channels at creation time.
 * \param input_idx     Pointer to input channel indexes.
 * \param output_idx    Pointer to output channel indexes.
 * \return pointer to operator
 */
cbops_op* create_dc_remove_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx);

/**
 * Create underrun compensation cbop (multi-channel). It must be at end of the chain,
 * it works only on "final" output buffers. It uses and tweaks endpoint-level parameters, too,
 * that are produced / used by RM (rate matching) calculations.
 *
 * \param nr_ins   Number of input channels at creation time, must match entire chain inputs.
 * \param nr_outs  Number of output channels at creation time.
 * \param input_idx       Pointer to input channel indexes (for consistency and future proofing - values are not used)
 * \param output_idx      Pointer to output channel indexes.
 * \param rm_diff_ptr     Pointer to rm_diff adjustment value used by owner entity, too.
 * \param block_size      Block size of owner endpoint (sink). Equates to data amount per kick period.
 * \param total_inserts_ptr   Pointer to sum total of inserted samples (discards will try to reduce this)
 * \param data_block_size_ptr Pointer to estimated data blockiness arriving to the chain
 * \param delta_samples_ptr   Pointer to sample amount delta, calculated by endpoint-level RM functions
 * \param insertion_vals_ptr  Pointer to a vector of nr_outs values used for insertion.
 *                            These may be last copied values on those channels, or just some constant values.
 *                            If NULL, then insertions are done with zero value.
 * \param sync_start_ptr      Pointer to data started state's flag.
 *
 * \return pointer to operator
 */
cbops_op* create_underrun_comp_op(unsigned nr_ins, unsigned nr_outs,
                                  unsigned* input_idx, unsigned* output_idx,
                                  int* rm_diff_ptr, unsigned *block_size_ptr,
                                  unsigned* total_inserts_ptr, unsigned *data_block_size_ptr,
                                  unsigned *delta_samples_ptr, unsigned* insertion_vals_ptr,
                                  bool *sync_start_ptr);

/**
 * Get key parameters from the cbop instance, which are used for inter-component communication, too.
 *
 * \param op                  Pointer to cbop param structure
 * \param rm_diff_ptr         Pointer to pointer to rm_diff adjustment value used by owner entity, too.
 * \param block_size_ptr      Pointer to block size of owner endpoint (sink). Equates to data amount per kick period.
 * \param total_inserts_ptr   Pointer to pointer to sum total of inserted samples (discards will try to reduce this)
 * \param data_block_size_ptr Pointer to pointer to estimated data blockiness arriving to the chain
 * \param delta_samples_ptr   Pointer to pointer to sample amount delta, calculated by endpoint-level RM functions
 * \param insertion_vals_ptr  Pointer to pointer to a vector of nr_outs values used for insertion.
 *                            These may be last copied values on those channels, or just some constant values.
 *                            If NULL, then insertions are done with zero value.
 * \param sync_start_ptr      Pointer to data started state's flag.
 *
 * \return TRUE/FALSE depending on success
 */
bool get_underrun_comp_op_vals(cbops_op *op, int **rm_diff_ptr, unsigned** block_size_ptr,
                               unsigned **total_inserts_ptr, unsigned **data_block_size_ptr,
                               unsigned **delta_samples_ptr, unsigned **insertion_vals_ptr,
                               bool **sync_start_ptr);

/**
 * Create input data discard cbop (multi-channel). It must be at start of the chain,
 * it works only on input buffers. It uses and tweaks endpoint-level parameters, too,
 * that are produced / used by RM (rate matching) calculations and the underrun compensator cbop.
 * It essentially has zero outputs, only ever works on the input buffers located at the input of the
 * owner entity (sink endpoint).
 *
 * \param nr_ins              Number of input channels at creation time, must match entire chain's inputs.
 * \param input_idx           Pointer to input channel indexes (for consistency and future proofing - values are not used)
 * \param block_size_ptr      Pointer to block size of owner endpoint (sink). Equates to data amount per kick period.
 * \param rm_headroom         RM headroom amount
 * \param total_inserts_ptr   Pointer to sum total of inserted samples (discards will try to reduce this)
 * \param data_block_size_ptr Pointer to estimated data blockiness arriving to the chain
 * \param sync_start_ptr      Pointer to data started state's flag.
 *
 * \return pointer to operator
 */
cbops_op* create_discard_op(unsigned nr_ins, unsigned* input_idx,
                            unsigned *block_size_ptr, unsigned rm_headroom,
                            unsigned* total_inserts_ptr, unsigned *data_block_size_ptr,
                            bool *sync_start_ptr);

/**
 * Get key parameters from the cbop instance, which are used for inter-component communication, too.
 *
 * \param op                  Pointer to cbop param structure
 * \param block_size_ptr      Pointer to pointer to block size of owner endpoint (sink).
 *                            Equates to data amount per kick period.
 * \param rm_headroom         Pointer to RM headroom amount
 * \param total_inserts_ptr   Pointer to pointer to sum total of inserted samples (discards will try to reduce this)
 * \param data_block_size_ptr Pointer to pointer to  estimated data blockiness arriving to the chain
 * \param sync_start_ptr      Pointer to data started state's flag.
 *
 * \return TRUE/FALSE depending on success
 */
bool get_discard_op_vals(cbops_op *op,
                         unsigned **block_size_ptr, unsigned *rm_headroom,
                         unsigned **total_inserts_ptr, unsigned **data_block_size_ptr,
                         bool **sync_start_ptr);

/**
 * create a SW rate adjustment operator (multi-channel)
 *
 * \param nr_channels   Number of channels at creation time.
 * \param input_idx     Pointer to input channel indexes.
 * \param output_idx    Pointer to output channel indexes.
 * \param quality       ID for quality level (across all channels)
 * \param rate_val_addr pointer to rate adjustment variable (across all channels)
 * \param shift_amt     pow2 shift amount applied to output (across all channels)
 * \return pointer to operator
 */
cbops_op* create_sw_rate_adj_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx,
                                      unsigned quality, unsigned *rate_val_addr, int shift_amt);

/**
 * destroy a SW rate adjustment operator (multi-channel)
 *
 * \param op   pointer to operator
 */
void destroy_sw_rate_adj_op(cbops_op *op);


/**
 * \brief Enable/Disable passthrough mode of a rateadjust and shift cbop
 *
 * \param op The rateadjust and shift cbop
 * \param enable boolean indicating whether to enable or disable passthrough mode
 */
extern void cbops_rateadjust_passthrough_mode(cbops_op *op, bool enable);

/**
 * \brief Set the rate of a rate_adjust cbop.
 *
 * \param pointer to operator
 * \param rate to adjust to (fraction of nominal rate)
 * \return NONE
 */
extern void cbops_sra_set_rate_adjust(cbops_op *op, unsigned nr_channels, unsigned rate);

/**
 * \brief Get the current rate of a rate_adjust cbops op.
 *
 * \param pointer to operator
 * \return current rate for the cbops rate adjust operator
 */
extern unsigned cbops_sra_get_current_rate_adjust(const cbops_op *op);

/**
 * \brief Resets the sw rate_adjust cbop.
 *
 * \param pointer to operator
 * \return NONE
 */
extern void cbops_sra_reset(cbops_op *op, unsigned nr_channels);

/**
 * \brief Retrieve the phase of the next output sample
 * relative to the input samples
 *
 * \param pointer to operator
 * \return A fractional value from -1 to 0
 */
extern int cbops_sra_get_phase(const cbops_op *op);

/**
 * create resampler operator (multi-channel).
 * It acts on a chorus of channels in tandem, applying same resampling to all of them.
 *
 * \param nr_channels   Number of channels at creation time.
 * \param input_idx     Pointer to input channel indexes.
 * \param output_idx    Pointer to output channel indexes.
 * \param input sample rate
 * \param output sample rate
 * \param size of interstage buffer
 * \param pointer to interstage buffer
 * \param pow2 scale of output
 * \param double precision flag   1:on, 0:off
 * \param low mips flag           1:on, 0:off
 * \return pointer to operator
 */
cbops_op* create_iir_resamplerv2_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx,
                                          unsigned in_rate, unsigned out_rate,
                                          unsigned inter_stage_size, unsigned *inter_stage,
                                          int shift, unsigned dbl_precision, unsigned low_mips);
/**
 * destroy resampler operator
 *
 * \param pointer to operator
 * \return NONE
 */
void      destroy_iir_resamplerv2_op(cbops_op *op);

/**
 * create a sample insert  HW rate adjustment rate monitor operator
 *
 * \param nr_channels   Number of channels at creation time.
 * \param input_idx     Pointer to input channel indexes.
 * \param output_idx    Pointer to output channel indexes.
 * \param threshold     Comparison threshold amount.
 * \return pointer to operator
 */
cbops_op* create_insert_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx, unsigned threshold);

/**
 * create mix operator
 * NOTE: As it stands, it specifically does 2-to-1 downmixing. This can be generalised as/when needed to N-to-1 downmixing,
 * at which point it would take arrays of indexes.
 *
 * \param input1 index
 * \param input2 index
 * \param output index
 * \param shift pow2 scalling applied to output
 * \param mix_ratio  fraction mix ratio for channels
 * \return pointer to operator
 */
cbops_op* create_mixer_op(unsigned input1_idx,unsigned input2_idx,unsigned output_idx,unsigned shift,unsigned mix_ratio);

/**
 * create external IO latency control and wrap protection operation.
 * NOTE: Due to its particularities, it can be modelled as a cbop with no channels or indexes, it just receives
 * some buffer pointers that may not be at all in framework object's buffer info table.
 *
 * \param nr_channels Number of output buffer pointers it receives.
 * \param out_bufs    Pointer to some output buffers
 * \param latency threshold
 * \return pointer to operator
 */
cbops_op* create_port_wrap_op(unsigned nr_out_bufs, unsigned out_bufs[], unsigned threshold);

/**
 * create copy shift operator (multi-channel)
 *
 * \param nr_channels Number of channels at creation time.
 * \param input_idx   Pointer to input channel indexes.
 * \param output_idx  Pointer to output channel indexes.
 * \param shift_amt   Shift amount
 * \return pointer to operator
 */
cbops_op* create_shift_op(unsigned nr_channels, unsigned* input_idx, unsigned* output_idx, int shift_amt);

/**
 * create sidetone filter operator (fits into multi-channel model, but works on single channel always)
 *
 * \param input index (it only ever works on one channel)
 * \param output index (it only ever works on one channel)
 * \param maximum support filter stages
 * \param pointer to sidetone parameters
 * \param pointer to filter parameters
 * \return pointer to operator
 */
cbops_op* create_sidetone_filter_op(unsigned input_idx, unsigned output_idx,
                                          unsigned max_stages, cbops_sidetone_params *st_params,
                                          unsigned *peq_params);
/**
 * reset the sidetone filter
 *
 * \param pointer to operator
 * \return NONE
 */
void      initialize_sidetone_filter_op(cbops_op *op);

/**
 * update the sidetone mode
 *
 * \param pointer to operator
 * \param mode flags
 * \param current measured noise level for noise switch of mode
 * \return NONE
 */
void      update_sidetone_filter_op(cbops_op *op,unsigned enable,unsigned noise_level);

/**
 * create sidetone mix operator (fits into multi-channel model, but works on single channel always)
 *
 * \param input index (it only ever works on one channel)
 * \param output index (it only ever works on one channel)
 * \param pointer to sidetone source buffer
 * \param threshold for sidetone latency
 * \return pointer to operator
 */
cbops_op* create_sidetone_mix_op(unsigned input_idx, unsigned output_idx,unsigned st_in_idx, unsigned threshold);

/**
 * create rate monitor operator (multi-channel model, but it acts as a cbop with zero in & out channels)
 *
 * \param clocks per second
 * \return pointer to operator
 */
cbops_op* create_rate_monitor_op(unsigned clk_per_sec,unsigned idx);

/**
 * reset SW rate monitor operator
 *
 * \param pointer to operator
 * \param expected sample rate
 * \param HW=true/SW=false select
 * \param measurement period in msec
 * \return NONE
 */
void rate_monitor_op_initiatlize(cbops_op *op, unsigned target,bool bHwSw,unsigned meas_period_msec);

/**
 * get rate monitor operator computed rate adjustment
 *
 * \param pointer to operator
 * \param direction 0=source,1=sink
 * \return rate adjustment
 */
int rate_monitor_op_get_rate(cbops_op *op,unsigned dir);
/**
 * rate_monitor_op_restart
 * \param pointer to operator
 * \return NONE
 */

void rate_monitor_op_restart (cbops_op *op);

/**
 * rate_monitor_op_is_complete
 * \param pointer to operator
 * \return measurement complete indication
 */

int rate_monitor_op_is_complete (cbops_op *op);

/**
 * destroy an interleave operator (multi-channel)
 *
 * \param op   pointer to operator
 */
void      destroy_interleave_op(cbops_op *op);


/**
 * create operator to drop data at sink if insufficient space
 *
 * \param nr_channels   Number of channels at creation time.
 * \param output_idx    Pointer to output channel indexes.
 * \param min_space     Comparison threshold amount.
 * \return pointer to operator
 */
cbops_op* create_sink_overflow_disgard_op(unsigned nr_channels,unsigned* output_idx,unsigned min_space);

/**
 * return number of samples dropped by operator
 *
 * \param pointer to operator
 * \return number of drops
 */
unsigned get_sink_overflow_disgard_drops(cbops_op *op);

void rate_monitor_simple_reset(cbops_op *op);

unsigned get_cbops_insert_op_inserts(cbops_op *op);

unsigned get_rate_monitor_last_acc(cbops_op *op);

#endif      /* _CBOPS_C_H_ */
