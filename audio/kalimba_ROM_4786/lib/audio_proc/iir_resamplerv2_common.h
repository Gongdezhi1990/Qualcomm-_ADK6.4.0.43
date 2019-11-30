/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/



#ifndef COMMON_IIR_RESAMPLER_H
#define COMMON_IIR_RESAMPLER_H


#define IIR_RESAMPLEV2_IIR_BUFFER_SIZE      19
#define IIR_RESAMPLEV2_FIR_BUFFER_SIZE      10  


/* Constants for reampler functions */
typedef enum{
   iir_1stStage_none=0,
   iir_1stStage_upsample,
   iir_2ndStage_upsample,
   iir_2ndStage_downsample
}iir_function_types;

/* Constants for IIR Stage Functions */
typedef enum{
      iir_19_s2 = 0,
      iir_19_s3,
      iir_19_s4,
      iir_19_s5,
      iir_15_s3,
      iir_15_s2,
      iir_9_s2
}iir_stage_types;


/* Stage Descriptor for resample configuration */
typedef struct iir_resampler_stage_def{
    void    *func_ptr1;         /**< main filter function*/
    unsigned FIR_Size;          /**< Size of FIR component of filter  */
    unsigned IIR_Size;          /**< Size of IIR component of filter */
    unsigned Rout;              /**< polyphase increment */
    unsigned input_scale;       /**< Pow2 scaling of input to filter  */
    unsigned output_scale;      /**< Pow2 scaling of output of filter  */
    unsigned *FirFilter;        /**< Pointer to FIR filter polyphase coefficients  */
    unsigned FracRatio;         /**< resampling ratio (fractional) */
    unsigned IntRatio;          /**< resampling ratio (integrer) */
    void     *IIRFunction;      /**< IIR filter function */
    unsigned iir_coeffs[];      /**< IIR filter coefficients */
}iir_resampler_stage_definition;

/* Descriptor for resample configuration */
typedef struct iir_resampler_def{
   unsigned int_ratio;          /**< resampling ratio (integer) all stages*/
   unsigned frac_ratio;         /**< resampling ratio (fractional)all stages */
   
   unsigned int_ratio_s1;       /**< resampling ratio (integer) 1st stages*/
   unsigned frac_ratio_s1;      /**< resampling ratio (fractional)1st stages */
   
   unsigned stage1;             /**< 1st stage enabled */
   
   /* remainder of structure varies based on specific resampling
      this describes.   The full structure definition is not applicable
      to a C structure definition.   This structure is processed in assembly. */
}iir_resampler_definition;


typedef struct iir_resampler_common{
    void       *filter;             /**< pointer to resampler descriptor */
    unsigned    input_scale;        /**< Pow2 scaling of input.  scales to Q7.16  */
    unsigned    output_scale;       /**< Pow2 scaling of output.  scales from Q7.16  */
    /* Buffer between Stages */
    unsigned    *intermediate_ptr;  /**< Pointer to interstage scratch buffer  */
    unsigned    intermediate_size;  /**< Size of interstage buffer  */
    /* Reset Flags (Set to NULL) */
    unsigned    reset_flag;       /**< resampler reset flag.  0 to reset  */
    unsigned    dbl_precission;   /**< Use double-precission in iir stages.  0 for single  */
}iir_resamplerv2_common;

typedef struct iir_resampler_channel{
    /* 1st Stage */
    unsigned    partial1;           /**< Stage 1 state info  */
    unsigned    smpl_count1;
    unsigned    *fir1_ptr;
    unsigned    *iir1_ptr;
    /* 2nd Stage */
    unsigned    partial2;          /**< Stage 2 state info  */
    unsigned    smpl_count2;
    unsigned    *fir2_ptr;
    unsigned    *iir2_ptr;    
}iir_resamplerv2_channel;


/****************************************************************************
Public Function Definitions
*/

/**
 * compute required buffer sizes to append to the operator data for the
 *  resampling configuration
 *
 * \param config - pointer to resampler configuration
 * \param bDoublePrec - non-zero if iir is double precission
 * \return buffer sizes (arch4: in words)
 */
unsigned  iir_resamplerv2_get_buffer_sizes(void *config,int bDoublePrec);

/**
 * get the resampling configuration for the desired sample rates. if the
 *  low_mips flag is set, this function will check if a low_mips version of the
 *  specified resampling configuration exists. if yes, the low_mips version will
 *  be used instead of the standard config
 *
 * \param in_rate - input sample rate
 * \param out_rate - output sample rate
 * \param low_mips - low mips flag
 * \return pointer to configuration shared variable
 */
void*     iir_resamplerv2_allocate_config_by_rate(unsigned in_rate,unsigned out_rate,unsigned low_mips);

/**
 * get the shared variable ID for the desired sample rates. returns 0 if the 
 *  requested resampling configuration does not exist
 *
 * \param in_rate - input sample rate
 * \param out_rate - output sample rate
 * \param low_mips - low mips flag
 * \return shared variable ID
 */
unsigned iir_resamplerv2_get_id_from_rate(unsigned in_rate,unsigned out_rate,unsigned low_mips);

/**
 * get the resampling configuration based on shared variable ID for
 *  the desired configuration
 *
 * \param id - shared variable ID
 * \return pointer to configuration shared variable
 */
void*     iir_resamplerv2_allocate_config_by_id(unsigned id);

/**
 * associated a resampling configuration with the cbops operator and
 *  signal the operator to reset
 *
 * \param params - pointer to operator data
 * \param lpconfig - pointer to resampling configuration
 * \return NONE
 */
void      iir_resamplerv2_set_config(iir_resamplerv2_common *params,void *lpconfig);

/**
 * release a sample rate configuration shared variable
 *
 * \param lpconfig - pointer to resampling configuration
 * \return NONE
 */
void      iir_resamplerv2_release_config(void *lpconfig);
#endif

