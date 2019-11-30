/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file dsl_cbops_support.c
 * \ingroup cbops
 *
 * This file contains (extra) helper functions in addition to those in cbops_support.c
 */
/****************************************************************************
Include Files
 */
#include "cbops_c.h"
#include "pmalloc/pl_malloc.h"

static void destroy_operator(cbops_op *op)
{
    if(op->function_vector == (void*)cbops_rate_adjust_table)
    {
         destroy_sw_rate_adj_op((cbops_op*)op);
    }
    else if(op->function_vector == (void*)cbops_iir_resampler_table)
    {
         destroy_iir_resamplerv2_op(op);
    }
    else
    {
        pfree(op);
    }
}

void cbops_remove_operator_from_graph(cbops_graph *graph,cbops_op *op)
{
    cbops_op   *prev_op = op->prev_operator_addr;
    cbops_op   *next_op = op->next_operator_addr;

   if(graph->first == op)
   {
       /* we are removing the first operator in the graph,
        * next op will become new first
        */
       graph->first = next_op;
       if(next_op == NULL)
       {
           /* this op was the only op in the graph
            * now graph is empty
            */
           graph->last = NULL;
       }
       else
       {
           /* there is still op remaing,
            * just config the new first
            */
           next_op->prev_operator_addr = NULL;
       }
   }
   else if(next_op == NULL)
   {
       /* This isn't the only operator in the graph,
        * but it is the last one in chain
        */
       prev_op->next_operator_addr = NULL;
       graph->last = prev_op;
   }
   else
   {
       /* easy part, the op is neither first
        * nor last
        */
       prev_op->next_operator_addr = next_op;
       next_op->prev_operator_addr = prev_op;
   }

   /* op removed from the graph, now
    * destroy the operator
    */
   destroy_operator(op);
}


void cbops_insert_operator_into_graph(cbops_graph *graph,cbops_op *op, cbops_op *after)
{
    if(after->next_operator_addr == NULL)
    {
        /* if this is the last operator in the graph
         * then append it to end of graph
         */
        cbops_append_operator_to_graph(graph, op);
    }
    else
    {
        /* put the operator between after and after->next */
        op->prev_operator_addr = after;
        op->next_operator_addr = after->next_operator_addr;
        after->next_operator_addr->prev_operator_addr = op;        
        after->next_operator_addr = op;
    }
}

void cbops_unset_buffer(cbops_graph *graph,unsigned index)
{
    cbops_buffer *buffer = &graph->buffers[index];
    if(buffer != NULL)
    {
        /* make the buffer index unused */
        buffer->type = 0;
        buffer->buffer = NULL;
        buffer->transfer_ptr = NULL;

        /* Buffer change, signal refresh */
        cbops_reshresh_buffers(graph);
    }
}
