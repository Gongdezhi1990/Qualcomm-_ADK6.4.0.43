// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************
#ifndef AUDIO_PEQ_C_HEADER_INCLUDED
#define AUDIO_PEQ_C_HEADER_INCLUDED

/*****************************************************************************
Include Files
*/
#include "mem_utils/memory_table.h"
#include "mem_utils/shared_memory_ids.h"

/****************************************************************************
Private Const Declarations
*/

/* peq data object size, in AU */
#define PEQ_BASE_STRUC_SIZE (sizeof(t_peq_object))
#define SH_PEQ_OBJECT_SIZE(x)     (PEQ_BASE_STRUC_SIZE + sizeof(unsigned)*2*((x)+1) )
#define HQ_PEQ_OBJECT_SIZE(x)     (PEQ_BASE_STRUC_SIZE + sizeof(unsigned)*(2+((x)*3)) )
#define DH_PEQ_OBJECT_SIZE(x)     (PEQ_BASE_STRUC_SIZE + sizeof(unsigned)*4*((x)+1) )

/* "coefficient params" object size */
#define PEQ_PARAMS_OBJECT_SIZE(x)     ( sizeof(unsigned) * ( 6*(x) + 3) )



#define PEQ_HEADROOM_SHIFTS           0;
#define HQ_PEQ_HEADROOM_SHIFTS        0;
#define DH_PEQ_HEADROOM_SHIFTS        0;

/* Maximum number of stages supported */
#define PEQ_MAX_STAGES                           10
/* Number of Stage Mask */
#define PEQ_NUM_STAGES_MASK          0xFF;


/****************************************************************************
Private Type Definitions
*/


// PEQ "coefficient" Parameters

   // PEQ config word: metadata(upper bits) + number of stages (LSBs)
   // Number of stage at lower LSBs should be masked out before any use
   // Use $audio_proc.peq.const.NUM_STAGES_MASK for masking
   
typedef struct  t_coeffs_scales{  
   unsigned int b2;
   unsigned int b1;
   unsigned int b0;
   unsigned int a2;
   unsigned int a1;
   unsigned int scale;
} t_coeffs_scales;
   
typedef struct t_peq_params{
   unsigned int num_stages;
   unsigned int gain_exponent;
   unsigned int gain_mantisa;
   t_coeffs_scales coeffs_scales[];
} t_peq_params;

// PEQ Data Structure    
typedef struct  t_peq_object {
   void* input_buffer;
   void* output_buffer;
   unsigned int max_stages;
   t_peq_params* params_ptr;
   unsigned int* delayline_start_addr;
   unsigned int* delayline_addr;
   unsigned int* coeff_ptr;
   unsigned int num_stages;
   unsigned int delayline_size;
   unsigned int coeffs_size;
   unsigned int headroom_bits;
   unsigned historybuf[];
} t_peq_object;
  

#endif // AUDIO_PEQ_C_HEADER_INCLUDED
