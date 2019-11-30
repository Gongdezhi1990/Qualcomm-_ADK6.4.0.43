/**
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
* \file  aec_reference_latency.c
* \ingroup  capabilities
*
*  AEC Reference
*
*/

/****************************************************************************
Include Files
*/

#include "pmalloc/pl_malloc.h"
#include "aec_reference_latency.h"




/****************************************************************************
Public Function Declarations
*/

cbops_op* create_mic_latency_op(unsigned idx,aec_latency_common *common)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof(cbops_op)+sizeof(latency_op));

    if(op)
    {
        latency_op *params = (latency_op*)op->parameter_area_start;
        
        /* Setup Operator func table */
        op->function_vector = cbops_mic_latency_table;

        /* Setup cbop param struct header info */

        params->index  = idx;
        params->common = common;
    }

    return(op);
}

cbops_op* create_speaker_latency_op(unsigned idx,aec_latency_common *common)
{
    // cbop param struct size (header plus cbop-specific parameters)
    cbops_op *op = (cbops_op*)xzpmalloc(sizeof(cbops_op)+sizeof(latency_op));

    if(op)
    {
        latency_op *params = (latency_op*)op->parameter_area_start;
        
        /* Setup Operator func table */
        op->function_vector = cbops_speaker_latency_table;

        /* Setup cbop param struct header info */

        params->index  = idx;
        params->common = common;
    }

    return(op);
}

