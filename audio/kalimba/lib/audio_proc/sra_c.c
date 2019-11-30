/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/****************************************************************************
Include Files
*/
#include "sra_c.h"
#include "sra_shared_constants.h"
#include "mem_utils/shared_memory.h"
#include "mem_utils/shared_memory_ids.h"
#include "pmalloc/pmalloc.h"
#include "platform/pl_fractional.h"
#include "string.h"

/****************************************************************************
Public Function Definitions
*/
/*
 * sra_get_coeffs
 */
int *sra_get_coeffs(unsigned coeffs_table_id)
{
    bool new_alloc;
    unsigned length;
    int *coeffs_table, *table_source;

    switch (coeffs_table_id)
    {
#ifdef SRA_NORMAL_QUALITY_COEFFS
        case SW_RA_NORMAL_QUALITY_COEFFS:
        {
            /* Table length is 10.5 * filter length */
            length = (SRA_COEFFS_NORMAL_SIZE * UPRATE_AMT) >> 1;
            table_source = sra_coeffs_normal;
            break;
        }
#endif /* SRA_NORMAL_QUALITY_COEFFS */
#ifdef SRA_HIGH_QUALITY_COEFFS
        case SW_RA_HIGH_QUALITY_COEFFS:
        {
            /* Table length is 10.5 * filter length */
            length = (SRA_COEFFS_HIGH_SIZE * UPRATE_AMT) >> 1;
            table_source = sra_coeffs_high;
            break;
        }
#endif /*SRA_HIGH_QUALITY_COEFFS */
#ifdef SRA_VERY_HIGH_QUALITY_COEFFS
        case SW_RA_VERY_HIGH_QUALITY_COEFFS:
        {
            /* Table length is 10.5 * filter length */
            length = (SRA_COEFFS_VERY_HIGH_SIZE * UPRATE_AMT) >> 1;
            table_source = sra_coeffs_very_high;
            break;
        }
#endif /* SRA_VERY_HIGH_QUALITY_COEFFS*/
        default:
        {
            /* If the table isn't recognised bale out. */
            return NULL;
        }
    }

    coeffs_table = shared_malloc(length * sizeof(int), MALLOC_PREFERENCE_DM2,
                                    coeffs_table_id, &new_alloc);

    /* If there was enough DM2 RAM and it was a new allocation the table needs
     * populating. */
    if (coeffs_table != NULL && new_alloc)
    {
        memcpy(coeffs_table, table_source, length * sizeof(int));
    }

    return coeffs_table;
}

/*
 * sra_release_coeffs
 */
void sra_release_coeffs(int *coeffs_table)
{
    shared_free(coeffs_table);
}

/*
 * sra_create
 */
sra_n_channel* sra_create(unsigned num_channels, unsigned quality, unsigned *rate_val_addr, int shift_amt)
{
    sra_n_channel *sra;
    unsigned       hist_len, channel;
    int            *lp_coeffs;

    /* Get Filter Quality - TODO: This doesn't really work at the moment, as underneath
     * there has only been a build-time switching of the filters & filtering. It needs
     * significant changes inside historic sra_resample asm function.
     */
    switch(quality)
    {
#ifdef SRA_NORMAL_QUALITY_COEFFS
        case SW_RA_NORMAL_QUALITY_COEFFS:
            hist_len = SRA_COEFFS_NORMAL_SIZE;
            break;
#endif
#ifdef SRA_HIGH_QUALITY_COEFFS
        case SW_RA_HIGH_QUALITY_COEFFS:
            hist_len = SRA_COEFFS_HIGH_SIZE;
            break;
#endif
#ifdef SRA_VERY_HIGH_QUALITY_COEFFS
        case SW_RA_VERY_HIGH_QUALITY_COEFFS:
            hist_len = SRA_COEFFS_VERY_HIGH_SIZE;
            break;
#endif
        default:
            return NULL;
    }

    /* Get Shared Coefficients */
    lp_coeffs = sra_get_coeffs(quality);

    if(lp_coeffs == NULL)
    {
        return NULL;
    }

    /* The history buffer needs to be in DM1 as the coefficients are in DM2 */
    sra = xzppmalloc(sizeof(sra_n_channel) + num_channels*(sizeof(sra_hist_params) +
                                        hist_len*sizeof(unsigned)), MALLOC_PREFERENCE_DM1);
    if(sra == NULL)
    {
        /* Release Shared Coefficients */
        sra_release_coeffs(lp_coeffs);
    }
    else
    {
        /* Memory was zero initialised so only set non-zero parameters*/
        sra->sra.filter_coeffs_size  = hist_len;
        sra->sra.filter_coeffs = lp_coeffs;
        sra->sra.shift_amount = shift_amt;
        sra->sra.sra_target_rate_addr = rate_val_addr;

        /* the rf should be initialised to -1.0 (asm) */
        sra->sra.rf = FRACTIONAL(-1.0);

        /* Fill channel-specific history buffer information. The history buffers were contiguously
         * allocated after the sra_n_channel struct, thus they follow each other after the
         * channel-specific parameters.
         */
        for(channel=0; channel < num_channels; channel++)
        {
            sra->hist_buffers[channel].hist_buf = sra->hist_buffers[channel].hist_buf_start =
                                    ((unsigned*)&(sra->hist_buffers[0])) +
                                    num_channels*sizeof(sra_hist_params)/sizeof(unsigned) +
                                    channel*hist_len;
        }
    }

    return sra;
}

/*
 * sra_destroy
 */
void sra_destroy(sra_n_channel *sra)
{
    if (sra->sra.filter_coeffs)
    {
        sra_release_coeffs(sra->sra.filter_coeffs);
    }

    pfree(sra);
}
