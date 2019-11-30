/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
 ****************************************************************************/
/**
 * \file cbops_mgr.c
 * \ingroup cbops_mgr
 *
 * This file contains the functions that handle cbops manager stored in endpoint.
 *
 * \section sec1 Contains:
 *     cbops_mgr_create
 *     cbops_mgr_append
 *     cbops_mgr_remove
 *     cbops_mgr_destroy
 *     inline cbops_mgr_process_data
 *     cbops_mgr_connect
 *     cbops_mgr_disconnect
 */

/****************************************************************************
Include Files
*/
#include "cbops_mgr.h"
#include "cbops_flags.h"
#include "pmalloc/pl_malloc.h"
#include "panic/panic.h"
#include "mem_utils/shared_memory_ids.h"
#include "platform/pl_assert.h"
#include "patch/patch.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Macros
*/
/* Dual core support enabled restricts running cbops
 * on input buffer that could be P1's buffer with no P0 write
 * access. This requires CBOPS to run inplace on the output buffer
 * instead of input buffer. This will not be possible on Bluecore
 * platforms where the output is MMU ports.
 */
#ifdef AVOID_CBOPS_INPLACE_ON_INPUT_BUFFER
#define SHUFFLED_OPS { CBOPS_DC_REMOVE, CBOPS_SHIFT, CBOPS_RATEADJUST | CBOPS_U_LAW | CBOPS_A_LAW }
#define INPLACE_ON_OUTPUT_BUFFER TRUE
#else
#define SHUFFLED_OPS { CBOPS_U_LAW | CBOPS_A_LAW | CBOPS_RATEADJUST, CBOPS_SHIFT, CBOPS_DC_REMOVE }
#define INPLACE_ON_OUTPUT_BUFFER FALSE
#endif

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Creates a cbops_mgr chain of cbops that are an input to the DSP.
 *
 * \param cbops_flags The set of flags that defines the cbops required in the
 * chain.
 * \param head The cbops head structure that contains the cbops chain list.
 * \param vals cbops creation params
 *
 * \return TRUE if the chain is successfully created. FALSE on a failure.
 */
static bool create_in_chain(unsigned int cbops_flags,unsigned nr_chans,cbops_graph *head, CBOP_VALS *vals)
{
    unsigned *idxs;
    cbops_op *new_op;

    patch_fn_shared(cbops_mgr_create);
    if((idxs = create_default_indexes(2*nr_chans)) == NULL)
    {
        return FALSE;
    }

    /* If it is not simply a copy, create the cbops in the order that they should
     * logically be applied on the input. The copy is performed on the first
     * step, the rest of the chain runs in place.
     *
     * Currently the order is:
     *
     * A-LAW/U-LAW conversion
     * DC REMOVE
     * SHIFT (RATEADJUST & SHIFT single operator used if both requested)
     */

    if (cbops_flags != CBOPS_COPY_ONLY)
    {
        unsigned masked_flags = cbops_flags & (CBOPS_U_LAW | CBOPS_A_LAW);
        /* It can't be both MU and A law encoding so if it tries both panic */
        if (masked_flags == (CBOPS_U_LAW | CBOPS_A_LAW))
        {
            panic_diatribe(PANIC_AUDIO_INVALID_CBOP_COMBINATION, cbops_flags);
        }
        else if (masked_flags == CBOPS_U_LAW)
        {
            new_op = create_g711_op(1, &idxs[0], &idxs[nr_chans], (void *)&g711_ulaw2linear);

            if (NULL == new_op)
            {
                pfree(idxs);
                return FALSE;
            }
            cbops_append_operator_to_graph(head,new_op);
        }
        else if (masked_flags == CBOPS_A_LAW)
        {
            new_op = create_g711_op(1, &idxs[0], &idxs[nr_chans], (void *)&g711_alaw2linear);

            if (NULL == new_op)
            {
                pfree(idxs);
                return FALSE;
            }
            cbops_append_operator_to_graph(head,new_op);
        }

        if ((cbops_flags & CBOPS_RATEADJUST) != 0)
        {
            unsigned shift_amount;

            /* If this is not the first operation then panic */
            if(head->first != NULL)
            {
                /* This cbop can't run in place so panic if we end up here. */
                panic_diatribe(PANIC_AUDIO_INVALID_CBOP_COMBINATION, cbops_flags);
            }

            /* If a shift is required lets use this cbop to do it. The flag
             * is cleared after this so that the shift cbop isn't created too.
             */
            if ((cbops_flags & CBOPS_SHIFT) != 0)
            {
                shift_amount = 8;
                cbops_flags &= ~CBOPS_SHIFT;
            }
            else
            {
                shift_amount = 0;
            }
            new_op = create_sw_rate_adj_op(nr_chans, &idxs[0], &idxs[nr_chans],
                                           CBOPS_RATEADJUST_COEFFS,
                                           vals->rate_adjustment_amount, shift_amount);
            if (NULL == new_op)
            {
                pfree(idxs);
                return FALSE;
            }

            cbops_append_operator_to_graph(head,new_op);
        }

        if ((cbops_flags & CBOPS_SHIFT) != 0)
        {
            /* If this is the first operation then copy from input to output
             * buffer. If it isn't then run in place. */
            unsigned *src_idxs = (head->first==NULL) ? &idxs[0] : &idxs[nr_chans];

            new_op = (cbops_op*)create_shift_op(nr_chans,src_idxs, &idxs[nr_chans], 8);

            if (NULL == new_op)
            {
                pfree(idxs);
                return FALSE;
            }

            cbops_append_operator_to_graph(head,new_op);
        }

        if ((cbops_flags & CBOPS_DC_REMOVE) != 0)
        {
            /* If this is not the first operation then run in place (input idx "n" set to output idx "n",
             * which is in turn nr of channels + "n").
             */
            unsigned *src_idxs = (head->first==NULL) ? &idxs[0] : &idxs[nr_chans];

            new_op = create_dc_remove_op(nr_chans,src_idxs, &idxs[nr_chans]);

            if (NULL == new_op)
            {
                pfree(idxs);
                return FALSE;
            }
            cbops_append_operator_to_graph(head,new_op);
        }
    }
    else
    {
        new_op = create_copy_op(nr_chans, &idxs[0], &idxs[nr_chans]);

        if (NULL == new_op)
        {
            pfree(idxs);
            return FALSE;
        }
        cbops_append_operator_to_graph(head,new_op);
    }

    pfree(idxs);
    return TRUE;
}


/**
 * \brief Creates a cbops_mgr chain of cbops that are an output to the DSP.
 *
 * \param cbops_flags The set of flags that defines the cbops required in the
 * chain.
 * \param head The cbops head structure that contains the cbops chain list.
 * \param vals Cbops creation params
 *
 * \return TRUE if the chain is successfully created. FALSE on a failure.
 */
static bool create_out_chain(unsigned int cbops_flags,unsigned nr_chans,cbops_graph *head, CBOP_VALS *vals)
{
    unsigned chan, i;
    unsigned *idxs;
    cbops_op *temp;

    patch_fn_shared(cbops_mgr_create);
    if((idxs = create_default_indexes(2*nr_chans)) == NULL)
    {
        return FALSE;
    }


    /* If it is not simply a copy, create the cbops in the order that they should
     * logically be applied on the output. The copy is performed as the last
     * step, everything else runs in place on the input.
     *
     * Currently the BlueCore order is:
     *
     * Discard
     * DC REMOVE
     * SHIFT (RATEADJUST & SHIFT single operator used if both requested)
     * A-LAW/U-LAW conversion/RATEADJUST
     * Underrun comp.
     *
     * and the hydra order is
     * Discard
     * RATEADJUST / A-LAW/U-LAW conversion
     * DC REMOVE
     * Underrun comp.
     *
     */

    if (cbops_flags != CBOPS_COPY_ONLY)
    {
        unsigned reordered_ops[] = SHUFFLED_OPS;
        bool copy_in_place = FALSE;
        unsigned  shift_amount = 0;
        unsigned masked_flags = cbops_flags & (CBOPS_U_LAW | CBOPS_A_LAW);
        if ((cbops_flags & CBOPS_UNDERRUN) != 0)
        {
            temp = create_underrun_comp_op( nr_chans, nr_chans, &idxs[0],
                                            &idxs[nr_chans],
                                            vals->rm_diff_ptr,
                                            vals->block_size_ptr,
                                            vals->total_inserts_ptr,
                                            vals->data_block_size_ptr,
                                            vals->delta_samples_ptr,
                                            vals->insertion_vals_ptr,
                                            vals->sync_started_ptr);

            /* Putting together the chain in reverse, last op first so insert
             * at the head of the list.
             */
            if (NULL == temp)
            {
                pfree(idxs);
                return FALSE;
            }

            /* If this is not the last operation then scream. */
            if (head->last != NULL)
            {
                panic_diatribe(PANIC_AUDIO_INVALID_CBOP_COMBINATION, cbops_flags);
            }

            /* Putting together the chain in reverse, last op first so insert
             * at the head of the list.
             */
            cbops_prepend_operator_to_graph(head,temp);
        }


        /* It can't be both MU and A law encoding so if it tries both panic */
        if ((masked_flags == (CBOPS_U_LAW | CBOPS_A_LAW)) ||
            (((cbops_flags & CBOPS_RATEADJUST) != 0) && (masked_flags != 0)))
        {
            panic_diatribe(PANIC_AUDIO_INVALID_CBOP_COMBINATION, cbops_flags);
        }

        /* Rate adjustment or a-law/u-law does the copy */
        if(((cbops_flags & CBOPS_RATEADJUST) != 0) || (masked_flags != 0))
        {
            copy_in_place = TRUE;
        }

        if (((cbops_flags & CBOPS_RATEADJUST) != 0) &&
            ((cbops_flags & CBOPS_SHIFT) != 0))
        {
            /* If a shift is required lets use this cbop to do it. The flag
             * is cleared after this so that the shift cbop isn't created too.
             */
            shift_amount = -8;
            cbops_flags &= ~CBOPS_SHIFT;
        }

        for( i = 0; i< sizeof(reordered_ops)/sizeof(unsigned) ; i++)
        {
            /* in case we wanted to do something for individual ops */
            patch_fn_shared(cbops_mgr_create);
            
            switch( reordered_ops[i] & cbops_flags)
            {
                case CBOPS_U_LAW:
                {
                    temp = create_g711_op(nr_chans, &idxs[0], &idxs[nr_chans],(void *)&g711_linear2ulaw);

                    if (NULL == temp)
                    {
                        pfree(idxs);
                        return FALSE;
                    }

                    cbops_prepend_operator_to_graph(head,temp);
                    break;
                }

                case CBOPS_A_LAW:
                {
                    temp = create_g711_op(nr_chans, &idxs[0], &idxs[nr_chans],(void *)&g711_linear2alaw);

                    if (NULL == temp)
                    {
                        pfree(idxs);
                        return FALSE;
                    }

                    cbops_prepend_operator_to_graph(head,temp);
                    break;
                }

                case CBOPS_RATEADJUST:
                {
                    temp = create_sw_rate_adj_op(nr_chans, &idxs[0], &idxs[nr_chans],
                                                 CBOPS_RATEADJUST_COEFFS,
                                                 vals->rate_adjustment_amount, shift_amount);

                    if (NULL == temp)
                    {
                        pfree(idxs);
                        return FALSE;
                    }
                    /* Putting together the chain in reverse, last op first so insert
                     * at the head of the list.
                     */
                    cbops_prepend_operator_to_graph(head,temp);
                    break;
                }
                /* If some other cbop is going to do the shift as part of it's operation
                 * then the flag will get unset by the cbop that will do the shift. That
                 * way we don't end up with the operation twice. */
                case  CBOPS_SHIFT:
                {
                    /* If this is not going to be the last operation then run in place, but only if there is
                     * something in chain that is copying - if this is only followed by underrun comp. op.,
                     * then shift can't be done in-place (someone must copy something...).
                     * If it does work in place, then output idx "n" (nr channels + "n") is set to input idx "n".
                     */
                    unsigned *dest_idxs = (head->last!=NULL) && masked_flags ? &idxs[0] : &idxs[nr_chans];

                    /* It sets shift amount so vals->shift_amount is irrelevant at this lifecycle phase.
                     */
                    temp = create_shift_op(nr_chans, &idxs[0],dest_idxs, -8);

                    if (NULL == temp)
                    {
                        pfree(idxs);
                        return FALSE;
                    }
                    /* If this is not going to be the last operation then
                     * run in place, but only if there is
                     * something in chain that is copying - if this is only
                     * followed by underrun comp. op.,
                     * then shift can't be done in-place (someone must copy
                     * something...).
                     * If it does work in place, then output idx "n" (nr channels +
                     * "n") is set to input idx "n".
                     */
                    if(INPLACE_ON_OUTPUT_BUFFER && copy_in_place)
                    {
                        for(chan = 0; chan < nr_chans; chan++)
                        {
                            cbops_set_input_idx(temp, chan,
                                                idxs[chan+ nr_chans]);
                        }
                    }
                    else if(head->first != NULL && masked_flags)
                    {
                        for(chan = 0; chan < nr_chans; chan++)
                        {
                            cbops_set_output_idx(temp, chan, idxs[chan]);
                        }
                    }
                    /* Putting together the chain in reverse, last op first so insert
                     * at the head of the list. */
                    cbops_prepend_operator_to_graph(head,temp);
                    break;
                }

                case CBOPS_DC_REMOVE:
                {
                    /* Putting together the chain in reverse, last op first
                     * so insert at the head of the list. */

                    temp = create_dc_remove_op(nr_chans, &idxs[0], &idxs[nr_chans]);
                    if (NULL == temp)
                    {
                        pfree(idxs);
                        return FALSE;
                    }

                    /* If this is not the last operation and there are
                     * other operators in chain that
                     * perform copying, then dc_remove can run in place., so
                     * output idx "n" (nr channels + "n") is set to input idx "n"
                     * However: if there are only non-copying operators in
                     * the chain (e.g. underrun
                     * compensation op and discard op), then dc_remove must
                     * actually move data from input buffer to output buffer,
                     * otherwise soothing silence will be heard...
                     */
                    if(INPLACE_ON_OUTPUT_BUFFER && copy_in_place)
                    {
                        for(chan = 0; chan < nr_chans; chan++)
                        {
                            cbops_set_input_idx(temp, chan,
                                                idxs[chan+ nr_chans]);
                        }
                    }
                    else if((head->first != NULL)
                            && (copy_in_place ||  ((cbops_flags & CBOPS_SHIFT) != 0)))
                    {
                        for(chan = 0; chan < nr_chans; chan++)
                        {
                            cbops_set_output_idx(temp, chan, idxs[chan]);
                        }
                    }
                    /* Putting together the chain in reverse, last op first so insert
                     * at the head of the list. */
                    cbops_prepend_operator_to_graph(head,temp);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        /* A special note: endpoint may have set us up such that there are NO operators, except underrun and discard
         * is created. This may happen in mad cases when e.g. data format makes endpoint decide that there is only
         * copying to be done - however, cbops as it stands has no dedicated flag for copy only - latter is deduced from
         * an absence of flags...
         * So without a nice cbops overhaul: TODO_COPY_ONLY_FLAGGING: based on the endpoint hint that there is only and only
         * underrun and discard, we must insert here at least a measly copy operator - otherwise catch-22, there is nobody
         * to really copy but COPY_ONLY "flag" (i.e. absence of all flags) can not be tested against.
         * For this TODO, cbops really should have a clear signal that it copies only - and then also endpoints to explicitly
         * state this. Otherwise the sign being the absence of sign is just too postmodern...
         */
        if ((cbops_flags & (~(CBOPS_UNDERRUN | CBOPS_DISCARD))) == CBOPS_COPY_ONLY)
        {
            temp = create_copy_op(nr_chans, &idxs[0], &idxs[nr_chans]);

            if (NULL == temp)
            {
                pfree(idxs);
                return FALSE;
            }

            cbops_prepend_operator_to_graph(head,temp);
        }

        if ((cbops_flags & CBOPS_DISCARD) != 0)		/* TODDO SP */
        {
            temp = create_discard_op(nr_chans, &idxs[0], vals->block_size_ptr, vals->rm_headroom,
                                     vals->total_inserts_ptr, vals->data_block_size_ptr,
                                     vals->sync_started_ptr);

            /* Putting together the chain in reverse, last op first so insert
             * at the head of the list.   This is always the first operator in a chain
             */
            if (NULL == temp)
            {
                pfree(idxs);
                return FALSE;
            }

            cbops_prepend_operator_to_graph(head,temp);
        }
    }
    else
    {
        /* If purely copying, no discarding or undderrun handling
         * for now. It may be added, if endpoints ever want to use such barebones cbop chain AND still
         * do proper underrun/discard logic.
         */
        temp = create_copy_op(nr_chans, &idxs[0], &idxs[nr_chans]);

        if (NULL == temp)
        {
            pfree(idxs);
            return FALSE;
        }
        cbops_prepend_operator_to_graph(head,temp);
    }

    pfree(idxs);
    return TRUE;
}

/****************************************************************************
Public Function Definitions
*/
cbops_mgr* cbops_mgr_create(ENDPOINT_DIRECTION dir, unsigned initial_cbops_flags)
{
    cbops_mgr *cbops_mgr_obj = (cbops_mgr *)xpmalloc(sizeof(cbops_mgr));
    patch_fn_shared(cbops_mgr_create);
    if(!cbops_mgr_obj)
    {
        return NULL;
    }
    /* populate cbops_mgr object */
    if(dir == SOURCE)
    {
        /* We may have by default the "mandatory" cbops flagged in the chain. However, it is only
         * there for sink endpoints, and in actual reality sources will only do pure copy if nothing else
         * is instantiated. However, endpoints at the moment of calling this may not know it yet and they pass
         * us the direction as a clue to use...
         */
        initial_cbops_flags &= (~(CBOPS_UNDERRUN | CBOPS_DISCARD));
    }
    cbops_mgr_obj->graph = NULL;
    cbops_mgr_obj->req_ops = initial_cbops_flags;
    cbops_mgr_obj->dir = dir;

    return cbops_mgr_obj;
}


/* cbops_mgr_append */
bool cbops_mgr_append(cbops_mgr* cbops_mgr_obj, unsigned cbops_flags, unsigned nr_chans,
                      tCbuffer *source_buf[], tCbuffer *sink_buf[], CBOP_VALS* vals, bool force_chain_update)
{
    patch_fn_shared(cbops_mgr_create);
    if ((cbops_mgr_obj->req_ops & cbops_flags) == cbops_flags)
    {
        /* All already set, nothing to do */
        return TRUE;
    }

    cbops_mgr_obj->req_ops  |= cbops_flags;

    if (cbops_flags != CBOPS_RATEADJUST)
    {
        vals->rate_adjustment_amount = NULL;
    }

    /* For CBOPS_SHIFT, the shift amount is set independently later, or caller already set it */

    /* Depending on platform and timeline of steps taken by Stream (for sync, set data format and connect),
     * one may or may not have proper buffer information at this point in time. Guessing that timeline of steps
     * taken several layers above is not recommended, not even possible for some scenario. Therefore check if those
     * layers have hinted to force chain re-creation irrespective of buffer tables having been fully completed.
     * Otherwise we may exit without having changed the chain itself, and horrid processing may result.
     */
    if(!force_chain_update)
    {
        /* Do not create cbops chain if sink/source cbuffers are not present. */
        bool buffs_present = (source_buf != NULL) && (sink_buf != NULL);
        unsigned chan = 0;
        while((chan < nr_chans) && (buffs_present))
        {
            buffs_present = ((source_buf[chan] != NULL) && (sink_buf[chan] != NULL));
            chan++;
        }

        if(!buffs_present)
        {
            /* cbops_chain has not been created but the flags have been updated */
            return TRUE;
        }
    }

    /* if the endpoint is connected recreate the cbops_chain every time the flags
     * are updated. */
    return cbops_mgr_connect(cbops_mgr_obj, nr_chans, source_buf, sink_buf, vals);
}


/* cbops_mgr_remove */
bool cbops_mgr_remove(cbops_mgr* cbops_mgr_obj, unsigned cbops_flags, unsigned nr_chans,
                      tCbuffer *source_buf[], tCbuffer *sink_buf[], bool force_chain_update)
{
    CBOP_VALS vals;

    if ((cbops_mgr_obj->req_ops  & cbops_flags) == 0)
    {
        /* Not there, nothing to do */
        return TRUE;
    }

    /* The old cbop_vals should be preserved but only rateadjust currently and
     * that is always last appended and first removed. */
    vals.rate_adjustment_amount = NULL;

    /* If we are not removing discard op - which is always the very first in the endpoint-owned chain! */
    if(((cbops_flags & CBOPS_DISCARD) == 0) && (SINK == cbops_mgr_obj->dir))
    {

        if(cbops_mgr_obj->graph != NULL)
        {
            get_discard_op_vals(cbops_mgr_obj->graph->first,
                                &vals.block_size_ptr, &vals.rm_headroom,
                                &vals.total_inserts_ptr, &vals.data_block_size_ptr,
                                &vals.sync_started_ptr);
        }
    }

    /* If we are not removing underrun comp. op - in case neither discard nor underrun cbop is
     * being removed, then some vals are read twice but they must match and also last in the chain
     * (underrun comp op) is the one dictating them. This op is always very last in the chain!
     */
    if(((cbops_flags & CBOPS_UNDERRUN) == 0) && (SINK == cbops_mgr_obj->dir))
    {
        if(cbops_mgr_obj->graph != NULL)
        {
            /* Snapshot relevant params from the current op */
            get_underrun_comp_op_vals(cbops_mgr_obj->graph->last, &vals.rm_diff_ptr, &vals.block_size_ptr,
                                      &vals.total_inserts_ptr, &vals.data_block_size_ptr,
                                      &vals.delta_samples_ptr, &vals.insertion_vals_ptr,
                                      &vals.sync_started_ptr);
        }
    }

    cbops_mgr_obj->req_ops  &= ~cbops_flags;

    /* Depending on platform and timeline of steps taken by Stream (for sync, set data format and connect),
     * one may or may not have proper buffer information at this point in time. Guessing that timeline of steps
     * taken several layers above is not recommended, not even possible for some scenario. Therefore check if those
     * layers have hinted to force chain re-creation irrespective of buffer tables having been fully completed.
     * Otherwise we may exit without having changed the chain itself, and horrid processing may result.
     */
    if(!force_chain_update)
    {
        /* Do not create cbops chain if sink/source cbuffers are not present. */
        bool buffs_present = (source_buf != NULL) && (sink_buf != NULL);
        unsigned chan = 0;
        while((chan < nr_chans) && (buffs_present))
        {
            buffs_present = ((source_buf[chan] != NULL) && (sink_buf[chan] != NULL));
            chan++;
        }

        if(!buffs_present)
        {
            /* cbops_chain has not been created but the flags have been updated */
            return TRUE;
        }
    }

    /* if the endpoint is connected recreate the cbops_chain every time the flags
     * are updated. */
    return cbops_mgr_connect(cbops_mgr_obj, nr_chans, source_buf, sink_buf, &vals);
}

/* cbops_get_flags */
unsigned cbops_get_flags(cbops_mgr* cbops_mgr_obj)
{
    return cbops_mgr_obj->req_ops;
}

/* cbops_mgr_disconnect */
bool cbops_mgr_disconnect(cbops_mgr* cbops_mgr_obj)
{
    /* Free the cbops chain and then the cbops head structure that contains them. */
    /* If there isn't a head object then there is no work to do */
    if (cbops_mgr_obj->graph != NULL)
    {
        destroy_graph(cbops_mgr_obj->graph);
        cbops_mgr_obj->graph = NULL;
    }
    return TRUE;
}

/* cbops_mgr_destroy */
bool cbops_mgr_destroy(cbops_mgr* cbops_mgr_obj)
{
    cbops_mgr_disconnect(cbops_mgr_obj);
    /* Deallocate cbops_mgr structure */
    pfree(cbops_mgr_obj);
    return TRUE;
}


/* cbops_mgr_connect */
bool cbops_mgr_connect(cbops_mgr* cbops_mgr_obj, unsigned nr_chans, tCbuffer *source_buf[], tCbuffer *sink_buf[], CBOP_VALS *vals)
{
    cbops_graph *cbops_hd_obj;
    bool retval;
    unsigned chan;
    patch_fn_shared(cbops_mgr_create);
    if (NULL == cbops_mgr_obj->graph)
    {
        /* Allocate new graph */
        cbops_hd_obj = cbops_alloc_graph(nr_chans*2);
        if(!cbops_hd_obj)
        {
            return FALSE;
        }
        cbops_mgr_obj->graph = cbops_hd_obj;
    }
    else
    {
        /* graph exists, so reuse it */
        cbops_hd_obj = cbops_mgr_obj->graph;

        /* Cleanup old graph */
        cbops_free_operators(cbops_hd_obj);
    }

    /* Setup IO Buffers */
    for(chan = 0; chan < nr_chans; chan++)
    {
        /* Setup IO in graph, Inputs followed by outputs */
        cbops_set_input_io_buffer(cbops_hd_obj,chan,0,source_buf[chan]);
        cbops_set_output_io_buffer(cbops_hd_obj,chan+nr_chans,nr_chans,sink_buf[chan]);
    }

    /* create the chain depending on direction */
    if (SOURCE == cbops_mgr_obj->dir)
    {
        retval = create_in_chain(cbops_mgr_obj->req_ops, nr_chans,cbops_hd_obj, vals);
    }
    else
    {
        retval = create_out_chain(cbops_mgr_obj->req_ops, nr_chans,cbops_hd_obj, vals);
    }

    if(!retval)
    {
        destroy_graph(cbops_hd_obj);
        cbops_mgr_obj->graph = NULL;
        return FALSE;
    }
    else
    {
        cbops_mgr_buffer_reinit(cbops_mgr_obj);
        return TRUE;
    }
}


/* "Connect" a certain channel with buffer info and indexes.
 */
bool cbops_mgr_connect_channel(cbops_mgr* cbops_mgr_obj, unsigned channel,
                               tCbuffer *source_buf, tCbuffer *sink_buf)
{
    cbops_graph *head = cbops_mgr_obj->graph;
    unsigned num_chans = head->num_io/2;
    patch_fn_shared(cbops_mgr_create);

    if (cbops_mgr_obj == NULL)
    {
        return FALSE;
    }

    if(channel >= num_chans)
    {
        return FALSE;
    }

    /* Set the cbuffer info in the head obj */
    cbops_set_input_io_buffer(head,channel,0,source_buf);
    cbops_set_output_io_buffer(head,channel+num_chans,num_chans,sink_buf);

    /* Update the "unpacked" fw_obj buffer info based on the head's cbuffer info */
    cbops_mgr_buffer_reinit(cbops_mgr_obj);

    return TRUE;
}

/* cbops_mgr_buffer_reinit */
bool cbops_mgr_buffer_reinit(cbops_mgr* cbops_mgr_obj)
{
    patch_fn_shared(cbops_mgr_run);
    if(cbops_mgr_obj && cbops_mgr_obj->graph)
    {
        cbops_reshresh_buffers(cbops_mgr_obj->graph);
    }
    return TRUE;
}


/* Set both input and output cbops_buffer for this channel to NULL
 */
bool cbops_mgr_set_unused_channel(cbops_mgr* cbops_mgr_obj, unsigned channel)
{
    cbops_graph *head = cbops_mgr_obj->graph;
    unsigned num_chans = head->num_io/2;

    patch_fn_shared(cbops_mgr_create);

    if(channel >= num_chans)
    {
        return FALSE;
    }

    /* Set the channel as disconnected.   cbops will handle the operators */
    cbops_set_input_io_buffer(head,channel,0,NULL);
    cbops_set_output_io_buffer(head,channel+num_chans,num_chans,NULL);

    return TRUE;
}

/* Get cbuffer ptrs for in/out channels based on what head has at this point. The tables have to be
 * pre-allocated for the right number of channels.
 */
bool cbops_mgr_get_buffer_info(cbops_mgr* cbops_mgr_obj, unsigned nr_chans, tCbuffer **in_buffs, tCbuffer **out_buffs)
{
    unsigned chan;
    cbops_graph *head;

    patch_fn_shared(cbops_mgr_run);
    if((in_buffs == NULL) || (out_buffs) == NULL)
    {
        return FALSE;
    }

    head =  cbops_mgr_obj->graph;

    /* If cbops graph is not yet set up, the buffer information is NULL - and later gets set up */
    if(head == NULL)
    {
        for(chan = 0; chan < nr_chans; chan++)
        {
            in_buffs[chan] = NULL;
            out_buffs[chan] = NULL;
        }
    }
    else
    {
        cbops_buffer *buffers  = head->buffers;
        unsigned     num_chans = head->num_io/2;

        for(chan = 0; chan < nr_chans; chan++)
        {
            in_buffs[chan]  = buffers[chan].buffer;
            out_buffs[chan] = buffers[chan+num_chans].buffer;
        }
    }

    return TRUE;
}

/* Get in/out channel buffer info for a given channel number */
/* TODO: For foreseeable future, cbops_mgr_owned chains have equal in/out channel numbers.
 */
bool cbops_mgr_get_channel_buffers(cbops_mgr* cbops_mgr_obj, unsigned channel, tCbuffer **in_buff, tCbuffer **out_buff)
{
    cbops_graph *head;

    patch_fn_shared(cbops_mgr_run);
    if((cbops_mgr_obj == NULL) || (in_buff == NULL) || (out_buff == NULL))
    {
        return FALSE;
    }

    head =  cbops_mgr_obj->graph;

    /* If cbops graph is not yet set up, the buffer information is NULL - and later gets set up */
    if(head == NULL)
    {
        *in_buff = NULL;
        *out_buff = NULL;
    }
    else
    {
        cbops_buffer *buffers = head->buffers;
        unsigned num_chans = head->num_io/2;
        /* Get the buffer info */
        *in_buff  = buffers[channel].buffer;
        *out_buff = buffers[channel+num_chans].buffer;
    }

    return TRUE;
}

/* Allocates cbuffer ptr tables for the cbops chain's number of in/out channels. */
/* TODO_DIFFERENT_IN_OUT_CHANNEL_COUNT: For foreseeable future, cbops_mgr_owned chains have equal in/out channel numbers.
 * Can generalise this if/when needed, as cbops themselves can take different numbers.
 */
bool cbops_mgr_alloc_buffer_info(cbops_mgr* cbops_mgr_obj, unsigned *nr_chans, tCbuffer ***in_buffs, tCbuffer ***out_buffs)
{
    patch_fn_shared(cbops_mgr_create);
    if(cbops_mgr_obj == NULL)
    {
        return FALSE;
    }

    /* see if the cbops graph structure has been created */
    if(cbops_mgr_obj->graph != NULL)
    {
        *nr_chans = cbops_mgr_obj->graph->num_io/2;

        /* Allocate in_buffs and out_buffs as a single block */
        *in_buffs = xpnewn(*nr_chans * 2, tCbuffer*);

        if(*in_buffs == NULL)
        {
            *out_buffs = NULL;
            return FALSE;
        }

        *out_buffs = &((*in_buffs)[*nr_chans]);
    }
    else
    {
        /* graph hasn't been created yet - so zero channels for now, it will be all set up later */
        *nr_chans = 0;
        *in_buffs = NULL;
        *out_buffs = NULL;
    }

    return TRUE;
}

void cbops_mgr_free_buffer_info(tCbuffer **in_buffs, tCbuffer **out_buffs)
{
    pdelete(in_buffs);
}

/* Retrieve whatever parameters the chain uses  */
bool cbops_mgr_get_op_vals(cbops_mgr* cbops_mgr_obj, CBOP_VALS* vals)
{
    cbops_graph *head = cbops_mgr_obj->graph;
    unsigned cbops_flags = cbops_mgr_obj->req_ops;

    patch_fn_shared(cbops_mgr_run);

    if(head == NULL)
    {
        return FALSE;
    }

    if (cbops_flags != CBOPS_RATEADJUST)
    {
        /* In and out channel numbers also considered to be equal for now. */
        cbops_op* op = find_cbops_op(head, cbops_rate_adjust_table);

        cbops_rate_adjustment_and_shift *params = CBOPS_PARAM_PTR(op, cbops_rate_adjustment_and_shift);

        vals->shift_amount = params->sra.shift_amount;
        vals->rate_adjustment_amount = params->sra.sra_target_rate_addr;
    }

    /* For CBOPS_SHIFT, the shift amount is set independently later, or caller already set it */
    if (cbops_flags != CBOPS_SHIFT)
    {
        /* In and out channel numbers
         * also considered to be equal for now.
         */
        cbops_op* op = find_cbops_op(head, cbops_shift_table);

        cbops_shift *params = CBOPS_PARAM_PTR(op, cbops_shift);
        vals->shift_amount = params->shift_amount;
    }

    /* If we are not adding discard op - which is always the very first in the endpoint-owned chain! */
    if(cbops_flags != CBOPS_DISCARD)
    {
        cbops_op* op = find_cbops_op(head, cbops_discard_table);

        if(op != NULL)
        {
            get_discard_op_vals(op,
                                &vals->block_size_ptr, &vals->rm_headroom,
                                &vals->total_inserts_ptr, &vals->data_block_size_ptr,
                                &vals->sync_started_ptr);
        }
    }

    /* If we are not adding underrun comp. op - This op is always very last in the chain! */
    if(cbops_flags != CBOPS_UNDERRUN)
    {
        cbops_op* op = find_cbops_op(cbops_mgr_obj->graph, cbops_underrun_comp_table);

        /* Snapshot relevant params from the current op */
        get_underrun_comp_op_vals(op, &vals->rm_diff_ptr, &vals->block_size_ptr,
                                  &vals->total_inserts_ptr, &vals->data_block_size_ptr,
                                  &vals->delta_samples_ptr, &vals->insertion_vals_ptr,
                                  &vals->sync_started_ptr);
    }

    return TRUE;
}

bool cbops_mgr_rateadjust_passthrough_mode(cbops_mgr *cbops_mgr_obj, bool enable)
{
    cbops_op *rm;
    patch_fn_shared(cbops_mgr_run);
    /* There has to be a cbops_mgr object and a graph defined for there to be
     * a rm cbop in the graph. */
    if (cbops_mgr_obj == NULL || cbops_mgr_obj->graph == NULL)
    {
        return FALSE;
    }

    rm = find_cbops_op(cbops_mgr_obj->graph, cbops_rate_adjust_table);

    /* If there was not RM cbop in the graph then the operation wasn't possible */
    if (rm == NULL)
    {
        return FALSE;
    }

    cbops_rateadjust_passthrough_mode(rm, enable);
    return TRUE;
}

/**
 * \brief Find a cbop in the chain that has a matching function table.
 *
 * \param head        Pointer to cbops chain head
 * \param func_table  Pointer to function table of a certain type of operator
 * \return Pointer to found cbop (or NULL if not found)
 */
cbops_op* find_cbops_op(cbops_graph* head, unsigned func_table[])
{
    cbops_op* temp;

    patch_fn_shared(cbops_mgr_run);
    for(temp = head->first; temp != NULL; temp = temp->next_operator_addr)
    {
        if(temp->function_vector == func_table)
        {
            return temp;
        }
    }

    return NULL;
}
