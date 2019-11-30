/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_sidetone_filter.h
 *
 * \ingroup cbops
 *
 * Apply filtering and gain related to the sidetone
 * operation.
 */
#ifndef CBOPS_SIDETONE_FILTER_H
#define CBOPS_SIDETONE_FILTER_H

/****************************************************************************
Public Type Declarations
*/

/* Structure for PEQ feature of operator */
typedef struct peq{
    unsigned input_addr;        /**< Internal use  */
    unsigned output_addr;       /**< Internal use  */
    unsigned max_stages;        /**< maximum PEQ stages supported  */
    unsigned *params;           /**< pointer to PEQ configuration */
    /* Remainder set by initialize function */
    unsigned *delayline_addr;   /**< Internal use  */
    unsigned *coeffs_addr;      /**< Internal use  */
    unsigned num_stages;        /**< Internal use  */
    unsigned delayline_size;    /**< Internal use  */
    unsigned coeff_size;        /**< Internal use  */
}peq_op;

/* sidetone filter configuration parameters */
typedef struct sidetone_params{
    unsigned st_clip_point;         /**< clip point for sidetone */
    unsigned st_adjust_limit;       /**< gain limit for sidetone */
    unsigned stf_switch;            /**< Filter switch mode*/
    unsigned stf_noise_low_thres;   /**< lower noise threshold for filter switch */
    unsigned stf_noise_high_thres;  /**< upper noise threshold for filter switch */
    unsigned stf_gain_exp;          /**< sidetone gain pow2 scale  */
    unsigned stf_gain_mantisa;      /**< sidetone gain mantisa */
}cbops_sidetone_params;

/* sidetone cbops operator parameter structure */
typedef struct sidetone_filter_op{
    cbops_sidetone_params *params; /**< Pointer to parameters  */

    unsigned apply_filter;      /**< Flag to enable/disable filter  */
    unsigned apply_gain;        /**< Flag to enable/disable sidetone  */
    int      inv_dac_gain;      /**< Scaling to match pos volume  */

    unsigned current_gain;      /**< Internal use */
    unsigned peak_level;        /**< Peak magnitude of sidetone.  Status */

    /* Filter data */
    peq_op   peq;               /**< PEQ config   */
    unsigned peq_data[];        /**< PEQ working data appended to structure  */
}cbops_sidetone_filter_op;

/****************************************************************************
Public Variable Definitions
*/

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_sidetone_filter_table[];

/****************************************************************************
Public Function Definitions
*/

#endif // CBOPS_SIDETONE_FILTER_H








