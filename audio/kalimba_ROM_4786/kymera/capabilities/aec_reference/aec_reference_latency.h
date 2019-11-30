/**
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
 * \defgroup aec_reference_cap
 *
 * \file  aec_reference_latency.h
 * 
 * AEC Reference Capability
 * \ingroup capabilities
 */
#ifndef AEC_REFERENCE_LATENCY_H
#define AEC_REFERENCE_LATENCY_H
#include "cbops/cbops_c.h"

/****************************************************************************
Public Type Declarations
*/

typedef struct aec_latency_common
{ 
	unsigned jitter;				/**< latency threshold in samples */
	unsigned block_size;      /**< frame period in samples */
	unsigned block_sync;      /**< signal that mic path has a frame */
	unsigned mic_data;        /**< current amount of data in mic */
	unsigned speaker_data;    /**< current amount of data in speaker */
	unsigned ref_delay;       /**< amount by which reference precedes mic */
	
   unsigned frm_count;       /**< Running count of frame processed */
   unsigned rm_adjustment;   /** applied rate adjustment */ 

#ifdef AEC_REFERENCE_LATENCY_DEBUG
	unsigned speaker_drops;
	unsigned speaker_inserts;
	unsigned speaker_delay;
#endif
	
}aec_latency_common;

typedef struct latency_op{
    unsigned    index;          /**< index of mic master channel */
    unsigned    available;      /**< current available space in buffer */
    aec_latency_common *common; /**< pointer to aec latency structure */
}latency_op;

/****************************************************************************
Public Function Definitions
*/

cbops_op* create_mic_latency_op(unsigned idx,aec_latency_common *common);
cbops_op* create_speaker_latency_op(unsigned idx,aec_latency_common *common);

void aec_ref_purge_mics(cbops_graph *mic_graph,unsigned num_mics);
int aecref_calc_ref_rate(unsigned mic_rt,int mic_ra,unsigned spkr_rt,int spkr_ra);

/****************************************************************************
Public Variable Definitions
*/

extern unsigned cbops_mic_latency_table[];
extern unsigned cbops_speaker_latency_table[];

/** The address of the function vector table. This is aliased in ASM */

#endif	// AEC_REFERENCE_LATENCY_H