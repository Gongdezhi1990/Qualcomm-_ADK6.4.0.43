/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file sra_c.h
 * \ingroup audio_proc
 *
 */

#ifndef _SRA_C_H_
#define _SRA_C_H_
/****************************************************************************
Include Files
*/

#include "types.h"
#include "buffer/cbuffer_c.h"

/****************************************************************************
Public Constant Declarations
*/

/* Definitions for SRA filter coefficients table
 * Currently we can only build with one of these defined
 * Ideally the filter would be configurable at run time
 * See B-183792
 */
#ifdef SRA_VERY_HIGH_QUALITY_COEFFS
#define SRA_COEFFS_VERY_HIGH_SIZE 18
#define SRA_COEFFS_SIZE SRA_COEFFS_VERY_HIGH_SIZE
#endif
#ifdef SRA_HIGH_QUALITY_COEFFS
#define SRA_COEFFS_HIGH_SIZE 12
#define SRA_COEFFS_SIZE SRA_COEFFS_HIGH_SIZE
#endif
#ifdef SRA_NORMAL_QUALITY_COEFFS
#define SRA_COEFFS_NORMAL_SIZE 8
#define SRA_COEFFS_SIZE SRA_COEFFS_NORMAL_SIZE
#endif
/****************************************************************************
Public Type Declarations
*/

/** Channel-specific parameter block, there is one for each channel */
typedef struct sra_hist_params{
    /** Pointer to buffer for channel's historical data */
    unsigned *hist_buf;
    /** The start address of hist_buf */
    unsigned *hist_buf_start;
} sra_hist_params;

/** Structure of the rate_adjustment and shift parameters */
typedef struct sra_params{
    /** The amount to left shift by (-ve gives a right shift) */
    int shift_amount;
    /** Pointer to the filter coefficients table */
    int *filter_coeffs;

    /** Where to obtain the target resampled rate */
    unsigned *sra_target_rate_addr;

    /* The size of the history buffer */
    unsigned filter_coeffs_size;

    /** The current SRA rate */
    unsigned sra_current_rate;

    /** Some internal state */
    unsigned rf;
    unsigned prev_short_samples;
}sra_params;

/** SRA structure used by the API exposed by this library. */
typedef struct sra_n_channel{
    sra_params sra;

    sra_hist_params hist_buffers[];
} sra_n_channel;


#ifdef SRA_NORMAL_QUALITY_COEFFS
/** The normal quality filter coefficients table used by the operator. */
extern int sra_coeffs_normal[];
#endif
#ifdef SRA_HIGH_QUALITY_COEFFS
/** The high quality filter coefficients table used by the operator. */
extern int sra_coeffs_high[];
#endif
#ifdef SRA_VERY_HIGH_QUALITY_COEFFS
/** The very high quality filter coefficients table used by the operator. */
extern int sra_coeffs_very_high[];
#endif

/****************************************************************************
Public Function Declarations
*/
/**
 * \brief Performs a software rate adjustment algorithm on a set of channels.
 *
 * \param in_bufs Array of pointers to input buffers to perform the sra algorithm on
 * \param out_bufs Array of pointers to output buffers where the sra result is written
 * \param num_channels The number of channels supplied to operate on
 * \param samples_to_process The number of samples to consume from each channel
 * \param params Structure containing the working data for the SRA algorithm
 */
extern void sra_resample(tCbuffer **in_bufs, tCbuffer **out_bufs, unsigned num_channels, unsigned samples_to_process, sra_params *params);

/**
 * \brief Creates an software rate adjustment object.
 *
 * \param num_channels The number of channels the sra needs to operate on
 * \param quality The shared memory id of the coefficient table to use
 * \param rate_val_addr The address to read the amount of adjustment to apply from.
 * The pointed to value should be in fractional representation.
 * \param shift_amt The amount of shift to apply during the operation
 *
 * \return Pointer to the newly created SRA object, NULL if creation failed.
 */
extern sra_n_channel* sra_create(unsigned num_channels, unsigned quality, unsigned *rate_val_addr, int shift_amt);

/**
 * \brief Destroys a software rate adjustment object.
 *
 * \param sra The SRA object to destroy.
 */
extern void sra_destroy(sra_n_channel *sra);

/**
 * \brief Helper function for getting a pointer to the rate adjust
 * filter coeffs. This copies the coeffs into shared DM2 Ram if needed. This
 * function must be used in conjunction with sra_release_coeffs to
 * ensure memory is freed as appropriate.
 *
 * \param coeffs_table_id The shared memory ID that relates to the filter
 * quality to use for this instance of the algorithm.
 *
 * \return pointer to a shared table containing the requested coeffs. NULL if
 * the request failed.
 */
extern int *sra_get_coeffs(unsigned coeffs_table_id);

/**
 * \brief Helper function for releasing the rate adjust filter coefficient
 * table when it is no longer in use.
 *
 * \param coeffs_table Pointer to the table that the instance is no longer using
 */
extern void sra_release_coeffs(int *coeffs_table);

#endif /* _SRA_C_H_ */
