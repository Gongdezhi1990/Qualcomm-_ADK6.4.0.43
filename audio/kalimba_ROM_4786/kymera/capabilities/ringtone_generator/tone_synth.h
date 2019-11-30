/**
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * \defgroup tone_synth
 * \file  tone_synth.h
 * \ingroup capabilities
 *
 * Tone synthesizer interface header file. <br>
 *
 */


#ifndef TONE_SYNTH_TONE_SYNTH_H
#define TONE_SYNTH_TONE_SYNTH_H

#include "types.h"

/****************************************************************************
Public Macro Definitions
*/

/** Define to enable replacing the synthesizer */
#define TONE_SYNTH_EXTENSIBLE

#ifdef TONE_SYNTH_EXTENSIBLE

#define TONE_SYNTH_INIT(SD) \
    do { \
        if ((SD)->synth != NULL) \
        { \
            (SD)->synth->init(SD); \
        } \
        else \
        { \
            tone_synth_init(SD); \
        } \
    } while(0)

#define TONE_SYNTH_SET_NOTE(SD,P,LN,LD,R,T)   \
    do { \
        if ((SD)->synth != NULL) \
        { \
            (SD)->synth->set_note((SD),(P),(LN),(LD),(R),(T)); \
        } \
        else \
        { \
            tone_synth_set_note((SD),(P),(LN),(LD),(R),(T)); \
        } \
    } while(0)

#define TONE_SYNTH_SET_TEMPO(SD,N) \
    do { \
        if ((SD)->synth != NULL) \
        { \
            (SD)->synth->set_tempo((SD),(N)); \
        } \
        else \
        { \
            tone_synth_set_tempo((SD),(N)); \
        } \
    } while(0)

#define TONE_SYNTH_TONE_PLAY(SD,N) \
    ( \
        ((SD)->synth != NULL) ? \
        (SD)->synth->tone_play((SD),(N)) : \
        tone_synth_tone_play((SD),(N)) \
    )

#else /* TONE_SYNTH_EXTENSIBLE */
#define TONE_SYNTH_INIT(SD)         tone_synth_init(SD)
#define TONE_SYNTH_SET_NOTE(SD,N)   tone_synth_set_note(SD,N)
#define TONE_SYNTH_SET_TEMPO(SD,N)  tone_synth_set_tempo(SD,N)
#define TONE_SYNTH_TONE_PLAY(SD)    tone_synth_tone_play(SD,N)
#endif /* TONE_SYNTH_EXTENSIBLE */

#define TONE_SYNTH_MAX_VOLUME       (255)

/****************************************************************************
Public Type Definitions
*/

typedef enum
{
    tone_synth_tone_sine,           /*!< Sine Wave */
    tone_synth_tone_square,         /*!< Square Wave */
    tone_synth_tone_saw,            /*!< Saw-Tooth Wave */
    tone_synth_tone_triangle,       /*!< Triangle Wave */
    tone_synth_tone_triangle2,      /*!< Triangle Wave (assymetric) */
    tone_synth_tone_clipped_sine,   /*!< Clipped Sine Wave */
    tone_synth_tone_plucked         /*!< Simulates a plucked instrument */
}
tone_synth_tone_type;

#ifdef TONE_SYNTH_EXTENSIBLE
typedef uint32 tone_synth_duration;
typedef uint32 tone_synth_phase;
typedef uint32 tone_synth_decay;
#else
typedef uint16 tone_synth_duration;
typedef uint16 tone_synth_phase;
typedef uint16 tone_synth_decay;
#endif

typedef void (*tone_synth_output_writer)(void*, int);

typedef struct TONE_SYNTH_INSTRUMENT_INFO
{
    /** Waveform/timbre to produce */
    tone_synth_tone_type type;

    /** Volume parameter in the range 0..255.
     * 0 is muted, 255 is the maximum.
     * The scale is defined by the synthesizer, currently it is linear.
     */
    uint16              volume;

    tone_synth_duration tempo;      /**< SEMIBREVE(whole note) duration in no. of samples.*/

    /** Decay rate of a volume as a fraction of note duration(no of sample or secs).
     * Volume of note decays to 0 in ((note duration)* decay / 16)
     * Example decay =16(0x01) means volume becomes 0 in (note duration)*16/16 = note duration
       i.e. at end of note, volume is 0.*/
    uint16 decay;
} TONE_SYNTH_INSTRUMENT_INFO;

typedef struct TONE_SYNTH_TONE_STATE
{
    tone_synth_phase    step;       /**< fraction of a waveform step per sample period */
    tone_synth_duration duration;   /**< in sample periods */
    tone_synth_decay    decay;      /**< Change in volume per 256 sample periods */

    tone_synth_phase    accumulator; /**< Fractions of waveform left over */
    uint32              volume;     /**< current volume */
    tone_synth_duration time;       /**< Elapsed time in sample periods */
}TONE_SYNTH_TONE_STATE;

typedef struct TONE_SYNTH_DATA TONE_SYNTH_DATA;

#ifdef TONE_SYNTH_EXTENSIBLE
typedef struct TONE_SYNTH_FUNCTIONS
{
    /** Reset the synthesizer */
    void (*init)(TONE_SYNTH_DATA *synth_data);

    /** Prepare to play a note */
    void (*set_note)(TONE_SYNTH_DATA *synth_data, unsigned pitch_index,
            unsigned length_num, unsigned length_den,
            bool is_rest, bool is_tied);

    /** Update the tempo */
    void (*set_tempo)(TONE_SYNTH_DATA *synth_data, unsigned tempo);

    /** Generate a number of samples */
    bool (*tone_play)(TONE_SYNTH_DATA *synth_data, unsigned num_of_samples);

} TONE_SYNTH_FUNCTIONS;
#endif /* TONE_SYNTH_EXTENSIBLE */

struct TONE_SYNTH_DATA
{
    /**
     * The sample rate of the generated tone.
     */
    unsigned                    tone_sample_rate;

    /** Callback to write generated samples */
    tone_synth_output_writer    writer_fn;

    /** Caller data for the output callback */
    void*                       writer_data;

    /** Parameters which take effect at the next tone setup
     * and are not modified by the synthesizer core
     */
    TONE_SYNTH_INSTRUMENT_INFO  tone_info;

#ifdef TONE_SYNTH_EXTENSIBLE
    /** Hooks for replacing the synthesizer */
    TONE_SYNTH_FUNCTIONS        *synth;
#endif /* TONE_SYNTH_EXTENSIBLE */

    /** \note There are inline accessors to fields up to this point,
     * so changes to the layout above create incompatibility
     */

    /** Parameters which are updated at each tone start
     * and during synthesis
     */
    TONE_SYNTH_TONE_STATE       tone_data;
};

/****************************************************************************
Public Function Declarations
*/

/**
* \brief    Initialise a tone
*
* \param    synth_data  The tone synthesizer context
*/
extern void tone_synth_init(TONE_SYNTH_DATA *synth_data);


/**
 * \brief   set up a note
 *
 * \param   synth_data  The tone synthesizer context
 * \param   pitch_index Pitch as a semitone index, with 0 = C0, up to 119
 * \param   length_num  Numerator of length relative to whole note
 * \param   length_den  Denominator of length relative to whole note
 * \param   is_rest     Produce a rest
 * \param   is_tied     Note is tied to previous note, i.e. decay continues
 */
extern void tone_synth_set_note(TONE_SYNTH_DATA *synth_data, unsigned pitch_index,
                                unsigned length_num, unsigned length_den,
                                bool is_rest, bool is_tied);

/**
 * \brief   process a tempo parameter
 *
 * \param   synth_data  The tone synthesizer context
 * \param   n       The tempo parameter, in crotchets (quarters) per minute
 */
extern void tone_synth_set_tempo(TONE_SYNTH_DATA *synth_data, unsigned tempo);

/**
* \brief    return next n samples of tone audio
*
* \param    synth_data  The tone synthesizer context
* \param    samples     Samples of tone audio the function generates each time
*
* \return   TRUE        More samples will be generated
*           FALSE       No more samples of tone audio
*/
extern bool tone_synth_tone_play(TONE_SYNTH_DATA *synth_data, unsigned samples);

/****************************************************************************
Public Inline Function Definitions
*/

/**
 * \brief   Accessor to set the sample rate
 * \param   synth_data  The tone synthesizer context
 * \param   sample_rate Sample rate in Hz
 */
static inline void tone_synth_set_sample_rate(TONE_SYNTH_DATA *synth_data, unsigned sample_rate)
{
    synth_data->tone_sample_rate = sample_rate;
}

/**
 * \brief   Accessor to set the tone type (timbre)
 * \param   synth_data  The tone synthesizer context
 * \param   tone_type Index of a tone type
 */
static inline void tone_synth_set_tone_type(TONE_SYNTH_DATA* synth_data, tone_synth_tone_type tone_type)
{
    synth_data->tone_info.type = tone_type;
}

/**
 * \brief   Accessor to set the volume
 * \param   synth_data  The tone synthesizer context
 * \param   volume Volume on a scale from 0 (mute) to 255 (maximum)
 */
static inline void tone_synth_set_volume(TONE_SYNTH_DATA* synth_data, unsigned volume)
{
    synth_data->tone_info.volume = volume;
}

/**
 * \brief   Accessor to set the decay
 * \param   synth_data  The tone synthesizer context
 * \param   decay Volume of note decays to 0 in ((note duration)* decay / 16)
 *              Example decay=16 means volume becomes 0 in (note duration)*16/16 = note duration
 *              i.e. at end of note, volume is 0.
 */
static inline void tone_synth_set_decay(TONE_SYNTH_DATA* synth_data, unsigned decay)
{
    synth_data->tone_info.decay = decay;
}

/**
 * \brief   Accessor to set the output callback
 * \param   synth_data  The tone synthesizer context
 * \param   output_cb Function to call for each sample produced
 * \param   output_data Callback context for the output function
 */
static inline void tone_synth_set_output(TONE_SYNTH_DATA* synth_data,
                                         tone_synth_output_writer writer_fn, void* writer_data)
{
    synth_data->writer_fn = writer_fn;
    synth_data->writer_data = writer_data;
}

#endif /* TONE_SYNTH_TONE_SYNTH_H */
