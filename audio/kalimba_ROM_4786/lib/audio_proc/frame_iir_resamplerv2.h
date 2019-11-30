/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef FRAME_IIR_RESAMPLETR_H
#define FRAME_IIR_RESAMPLETR_H

#include "iir_resamplerv2_common.h"

/* frame processing parameters data structure */
typedef struct iir_resampler_op{
    void    *input_frame_ptr;      /**< Input stream pointer*/
    void    *output_frame_ptr;     /**< Output stream pointer */
    
    iir_resamplerv2_common    common;
    iir_resamplerv2_channel   channel;

    /* Working Data */
    unsigned    working[];        /**< filter history buffers  */
}frame_iir_resampler_op;



#endif