/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup audio fadeout
 * \file  audio_fadeout_asm.h
 * \ingroup common
 *
 * ASM header file for audio fadeout <br>
 *
 */

/* until some XML-based common header generation is solved: */

/** indexes of fields in FADEOUT_PARAMS structure (as per C definition) */
.const $audio.fade_out.FADEOUT_COUNTER_FIELD                 0 * ADDR_PER_WORD;
.const $audio.fade_out.FADEOUT_STATE_FIELD                   1 * ADDR_PER_WORD;
.const $audio.fade_out.FADEOUT_FLUSH_COUNT_FIELD             2 * ADDR_PER_WORD;

/** states of the fade-out machinery (matching the C definitions) */
.const $audio.fade_out.NOT_RUNNING_STATE                     0;
.const $audio.fade_out.RUNNING_STATE                         1;
.const $audio.fade_out.FLUSHING_STATE                        2;
.const $audio.fade_out.END_STATE                             3;


