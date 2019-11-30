/**
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 * 
 * \file  timestamp_reframe.h
 *
 * \ingroup ttp
 * 
 * Public header file for the timestamp reframe module.
 */

#ifndef TIMESTAMP_REFRAME_H
#define TIMESTAMP_REFRAME_H

/*****************************************************************************
Include Files
*/
#include "types.h"
#include "buffer/buffer_metadata.h"

/****************************************************************************
Public Type Declarations
*/

/**
 * Enumeration which represent the status of the reframe module.
 */
typedef enum
{
    /**
     * Until valid no valid (void or timestamped) tag is found the reframe module is in
     * startup mode.
     */
    REFRAME_STATE_STARTUP,
    /**
     * Void tags are generated.
     */
    REFRAME_STATE_VOID_TAGS,

    /**
     * Timestamped tags are generated.
     */
    REFRAME_STATE_TIMESTAMPED_TAGS
} reframe_state;

struct REFRAME_STRUCT
{
    /**
     * The metadata buffer for the .
     */
    tCbuffer *metadata_buff;

    /**
     * The sample rate in which the samples are consumed works.
     */
    unsigned sample_rate;

    /**
     * Reframed tag length. This is a constant value set during
     * the init.
     */
    unsigned desired_tag_length;

    /**
     * Structure holding information about the generated tag.
     */
    struct {

        /**
         * The length of the generated tag. This could be different than desired_tag_legth
         * due to the void tags.
         */
        unsigned length;

        /**
         * The index of the generated tag.
         */
        unsigned index;

        /**
         * The timestamp of the generated tag.
         */
        unsigned timestamp;

        /**
         * TTP Sample period adjustment, fractional difference from nominal
         */
        int sp_adjust;

        /**
         * Error offset pointer. Could be NULL if the tag has no offset.
         */
        unsigned err_offset_id;

        /**
         * True if the next tag is void one. False, otherwise.
         */
        bool is_void;
    } tag;

    /**
     * Structure holding information about the last.
     */
    struct {
        /**
         * The length of the previous tag. This is zero for the first tag and when
         * transitioning from void to timestamped tags.
         */
        unsigned length;

        /**
         * octets_consumed.
         */
        unsigned octets_consumed;
    }prev_tag;

    /**
     * The current state of the timestamp reframe.
     */
    reframe_state state;

    /**
     * The read index from which the next timestamp/void tag should be searched for.
     */
    unsigned tag_read_index;
};
typedef struct REFRAME_STRUCT REFRAME;



struct BUFFER_TIMESTAMP_REFRAME_STRUCT
{
    /**
     * The output buffer for the reframe library.
     */
    tCbuffer *out_buff;

    /**
     * The last tag index copied to the output buffer.
     */
    unsigned current_tag_index;

    /**
     * The reframe module.
     */
    REFRAME rf;
};
typedef struct BUFFER_TIMESTAMP_REFRAME_STRUCT BUFFER_TIMESTAMP_REFRAME;


/****************************************************************************
Function Declarations for the timed playback module reframing.
*/

/**
 * \brief Initialise the reframe instance
 *
 * \param reframe - Pointer to the reframe instance
 * \param metadata_buff - The metadata buffer.
 * \param reframe_period - The timestamp reframing period in samples.
 * \param sample_rate - The sample rate in which the samples are consumed.
 *
 * \return True if the initialisation was successful, false otherwise.
 */
extern void reframe_init(REFRAME *reframe, tCbuffer *metadata_buff,
        unsigned reframe_period, unsigned sample_rate);

/**
 * \brief Checks the available tags in the input buffer.
 *
 * \param reframe - Pointer to the reframe instance
 */
extern void reframe_check_tags(REFRAME *reframe);

/**
 * \brief Returns true if there is an available generated tag for the consumer.
 *
 * \param reframe - Pointer to the reframe instance
 *
 * \return True if there is a tag available.
 */
extern bool reframe_tag_available(REFRAME *reframe);

/**
 * \brief Returns the generated tag  index.
 *
 * \param reframe - Pointer to the reframe instance
 *
 * \return The tag index.
 */
extern unsigned reframe_tag_index(REFRAME *reframe);

/**
 * \brief Returns true if the next tag available is a void tag.
 *
 * Note: Void tags should be ignored in the sra calculation.
 *
 * \param reframe - Pointer to the reframe instance
 *
 * \return True the next tag is a void tag.
 */
extern bool reframe_tag_is_void(REFRAME *reframe);

/**
 * \brief Returns the generated tag length.
 *
 * \param reframe - Pointer to the reframe instance
 *
 * \return The tag length.
 */
extern unsigned reframe_tag_length(REFRAME *reframe);

/**
 * \brief Returns the generated tag playback time which is the timestamp plus the error
 *       offset.
 *
 * NOTE
 *  Before calling this function, the user must make sure that there is available tag by
 *  calling the reframe_tag_available function.
 *
 * \param reframe - Pointer to the reframe instance
 *
 * \return The tag playback time.
 */
extern unsigned reframe_tag_playback_time(REFRAME *reframe);

/**
 * \brief Returns the generated tag timestamp.
 *
 * NOTE
 *  Before calling this function, the user must make sure that there is available tag by
 *  calling the reframe_tag_available function.
 *
 * \param reframe - Pointer to the reframe instance
 *
 * \return The tag timestamp.
 */
extern unsigned reframe_tag_timestamp(REFRAME *reframe);

/**
 * \brief Returns the pointer to the generated tag error offset which is the last
 *       tag error offset read from the metadata buffer.
 *
 * \param reframe - Pointer to the reframe instance
 *
 * \return Pointer to the generated tag error offset.
 */
extern int * reframe_tag_error_offset(REFRAME *reframe);

/**
 * \brief Returns the sample period adjustment for the current tag.
 *
 * \param reframe - Pointer to the reframe instance
 *
 * \return The tag's sample period adjustment.
 */
extern unsigned reframe_sp_adjust(REFRAME *reframe);
/**
 * \brief Consumes data from the last generated tag.
 *
 * \param reframe - Pointer to the reframe instance
 * \param consumed_octets - The number of octets consumed from the tag.
 *
 * \return True the the consuming is correct, False otherwise.
 */
extern void reframe_consume(REFRAME *reframe, unsigned consumed_octets);

#endif /* TIMESTAMP_REFRAME_H */

