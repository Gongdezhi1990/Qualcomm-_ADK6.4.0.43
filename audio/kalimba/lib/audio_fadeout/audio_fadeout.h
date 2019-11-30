/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup audio fadeout
 * \file  audio_fadeout.h
 * \ingroup common
 *
 * Header file for audio fadeout <br>
 *
 */

#ifndef _AUDIO_FADEOUT_H_
#define _AUDIO_FADEOUT_H_

/*****************************************************************************
Include Files
*/
#include "buffer/cbuffer_c.h"
#include "types.h"
/*****************************************************************************
Public Type Declarations
*/
/** Fadeout states */
typedef enum
{
    NOT_RUNNING_STATE,
    RUNNING_STATE,
    FLUSHING_STATE,
    END_STATE
} FADEOUT_STATE;

/** Fade-out parameters */
typedef struct
{
    unsigned fadeout_counter;
    FADEOUT_STATE fadeout_state;
    unsigned fadeout_flush_count;
} FADEOUT_PARAMS;

/*****************************************************************************
Public Function Definitions
*/
/* common fadeout function wrapper */
extern bool mono_cbuffer_fadeout(tCbuffer *cbuff, unsigned samples_to_process,
        FADEOUT_PARAMS* fadeout_params);

#endif /* _AUDIO_FADEOUT_H_ */
