/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file stream_endpoint_spdif.h
 * \ingroup stream
 *
 */

#ifndef STREAM_ENDPOINT_SPDIF_H_INCLUDED
#define STREAM_ENDPOINT_SPDIF_H_INCLUDED

/* define masks for the supported rates */
typedef enum
{
    SPSRM_48KHZ = 1<<0,
    SPSRM_44K1HZ = 1<<1,
    SPSRM_32KHZ = 1<<2,
    SPSRM_96KHZ = 1<<3,
    SPSRM_88K2HZ = 1<<4,
    SPSRM_192KHZ = 1<<5,
    SPSRM_176K4HZ = 1<<6

}SPDIF_SUPPORTED_RATE_MASK;

#define panic_spdif_invalid_param(reason, arg) panic_diatribe(PANIC_SPDIF_ENDPOINT_INVALID_PARAM, (arg<<8)|reason)

/*
 * the amount of silence generated when stream is paused
 * (gets invalid) is configurable and can be indefinite
 */
#define SPDIF_UNLIMITED_SILENCE_DURING_PAUSE (-1)

/* enum defines pause state,
 * values are used in asm functions too  */
enum {
    /* ep isn't in pause mode */
    SPEPS_NOT_IN_PAUSE = 0,

    /* ep is in pause but no silence
     * is being inserted
     */
    SPEPS_PAUSE_IDLE = 1,

    /* ep is in pause state and
     * silence is being inserted */
    SPEPS_ACTIVE_PAUSE = 2

}SPDIF_EP_PAUSE_STATE;

/* parameter to pass to panic function
 * when an argument looks wrong in the API
 * function called or a message received
 */
typedef enum
{
    SIPR_SPDIF_EP_NOT_FOUND,
    SIPR_INVALID_PORT_ID,
    SIPR_UNEXPECTED_CHANNEL_ORDER,
    SIPR_NOT_MATCHING_CHANNEL_ORDER,
    SIPR_UEXPECTED_NUM_PORTS,
    SIPR_SPDIF_EP_ACT_REQUEST_FAILED,
    SIRP_SPDIF_EP_BAD_STATE,
    SIRP_SPDIF_EP_BAD_MESSAGE
}SPDIF_INVALID_PARAM_REASON;

/**
 * spdif_detect_sample_rate
 * \brief measures the incoming sample rate,
 *
 * \param new_data number of new samples received, for interleave config
 *        it should be half of new samples
 * \param spdif_str spdif structure *
 */
extern void spdif_detect_sample_rate(unsigned int new_data, void *spdif_str);

/**
 * spdif_copy_raw_data
 * \brief copies spdif data into output buffers
 * \param sink_buf output buffer to write output samples into
 * \param source_buff input buffer to read samples from
 * \param amount_to_read number of samples to read from source buffer,
 *        samples that cannot be written to output buffer due to lack
 *        of space will be discarded. For interleaved config this must be
 *        an even number.
 * \param amount_to_write number of samples to write into sink buffer,
 *        if not enough input silence will be written . For interleaved
 *        config this must be an even number.
 * \param shift_amount amount to shift input samples before writing to output buffer.
 */
extern void spdif_copy_raw_data (tCbuffer *sink_buf, tCbuffer *source_buff, unsigned int amount_to_read ,
                          unsigned int amount_to_write, unsigned shift_amount);
/**
 * spdif_handle_pause_state
 * \brief handles the pause state for spdif rx endpoint
 * \param silence_duration duration of silence (in microseconds) to insert once pause detected,
 *        negative value will be interpreted as indefinite amount of silence to insert, and it will
 *        only ended when the interface becomes active again.
 * \param spdif_str pointer to spdif structure
 * \active shows the validity of the input
 * \sample_rate last valid sample rate, silence is generated at last valid input rate
 * \return number of silence samples that needs to be generated.
 */
extern unsigned spdif_handle_pause_state(int silence_duration, void *spdif_str, unsigned active, unsigned sample_rate);

#endif /* #ifndef STREAM_ENDPOINT_SPDIF_H_INCLUDED */

