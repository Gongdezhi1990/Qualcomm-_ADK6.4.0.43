/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
        ringtone_if.h  -  Ringtone control library

CONTAINS
        The GLOBAL definitions for the RINGTONE subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef    __APP_RINGTONE_H__
#define    __APP_RINGTONE_H__
 

#include "app/ringtone/ringtone_notes.h"


/*
  What we need is some completely opaque type that will give an error if
  dereferenced. It must be able to hold a uint16 and a uint16 * and must
  be a basic type (ie. not a union). 
*/
/*!
  The \e ringtone_note type is defined so that it is possible to store either 
  sixteen bit numbers or pointers.

  Notes are specified with either the RINGTONE_NOTE(note, duration) or
  RINGTONE_NOTE_TIE(note, duration) macros.

  There are also several non-note commands that can be specified. These are
  #RINGTONE_END, #RINGTONE_TEMPO, #RINGTONE_VOLUME, #RINGTONE_TIMBRE,
  and #RINGTONE_DECAY.
*/
typedef struct this_struct_doesnt_exist *ringtone_note;

/*!
    The waveform to use for tone generation. Set using the #RINGTONE_TIMBRE 
macro. */
typedef enum ringtone_tone_type_enum {
    ringtone_tone_sine,             /*!< Sine Wave */
    ringtone_tone_square,           /*!< Square Wave */
    ringtone_tone_saw,              /*!< Saw-Tooth Wave */
    ringtone_tone_triangle,         /*!< Triangle Wave */
    ringtone_tone_triangle2,        /*!< Triangle Wave (assymetric) */
    ringtone_tone_clipped_sine,     /*!< Clipped Sine Wave */
    ringtone_tone_plucked           /*!< Simulates a plucked instrument */
} ringtone_tone_type;

/* These defines should not be used directly by application code.*/
#ifndef DO_NOT_DOCUMENT

#define RINGTONE_SEQ_CONTROL_MASK        0x8000
#define RINGTONE_SEQ_NOTE            0x0000
#define RINGTONE_SEQ_CONTROL        0x8000
#define RINGTONE_SEQ_NOTE_TIE_MASK        0x4000
#define RINGTONE_SEQ_NOTE_PITCH_MASK    0x3f80
#define RINGTONE_SEQ_NOTE_PITCH_POS    7
#define RINGTONE_SEQ_NOTE_LENGTH_MASK    0x007f
#define RINGTONE_SEQ_NOTE_LENGTH_POS    0
#define RINGTONE_SEQ_CONTROL_COMMAND_MASK  0x7000
#define RINGTONE_SEQ_CONTROL_COMMAND_POS   12

#define RINGTONE_SEQ_END    (RINGTONE_SEQ_CONTROL|(0<<RINGTONE_SEQ_CONTROL_COMMAND_POS))
#define RINGTONE_SEQ_TEMPO  (RINGTONE_SEQ_CONTROL|(1<<RINGTONE_SEQ_CONTROL_COMMAND_POS))
#define RINGTONE_SEQ_VOLUME (RINGTONE_SEQ_CONTROL|(2<<RINGTONE_SEQ_CONTROL_COMMAND_POS))
#define RINGTONE_SEQ_TIMBRE (RINGTONE_SEQ_CONTROL|(3<<RINGTONE_SEQ_CONTROL_COMMAND_POS))
#define RINGTONE_SEQ_DECAY  (RINGTONE_SEQ_CONTROL|(4<<RINGTONE_SEQ_CONTROL_COMMAND_POS))

#define RINGTONE_SEQ_TEMPO_MASK        0x0fff
#define RINGTONE_SEQ_VOLUME_MASK        0x00ff
#define RINGTONE_SEQ_TIMBRE_MASK        0x00ff
#define RINGTONE_SEQ_DECAY_RATE_MASK    0x00ff

#endif /*end of DO_NOT_DOCUMENT*/

/*!
\name Ringtone commands

These are the commands that can be put in a #ringtone_note.
*/

/*! \{ */

/*!
    This command must be given at the end of a sequence.  It causes the sequence
    to be stopped.  All sequences must end this way.
*/
#define RINGTONE_END      ((ringtone_note) RINGTONE_SEQ_END)

/*!
    This command is used to change the tempo of the sequence.  A twelve-bit value
    is given as the new tempo in crotchets (quarter notes) per minute. The
    default is 120.
*/
#define RINGTONE_TEMPO(t) ((ringtone_note) (RINGTONE_SEQ_TEMPO | (t)))

/*!
    This command sets a new volume.  All notes played after this command are
    played with the specified volume, that is, currently playing notes are not
    affected. The default is maximum volume.
*/
#define RINGTONE_VOLUME(v) ((ringtone_note) (RINGTONE_SEQ_VOLUME | (v)))

/*!
        This command sets the timbre, or waveform, of the notes.  The
        available timbres are defined by #ringtone_tone_type. The default
        timbre is #ringtone_tone_sine.
*/
#define RINGTONE_TIMBRE(t) \
    ((ringtone_note) (RINGTONE_SEQ_TIMBRE | (uint16) ringtone_tone_##t))

/*!
    This sets the volume decay rate.  As each tone is played, its volume decreases
    with a variable rate.  Low values for this parameter cause notes to decay
    very quickly, whereas high values cause the notes to continue with an almost
    constant volume.

    The parameter to this command is a fixed point number with four bits to the
    right of the binary point giving the time taken for the note to decay to zero
    volume relative to the length of the note played. A value of 0x10 (meaning 1.0) 
    will cause each note to reach zero volume just as the next note starts. A value of 
    0x08 (meaning 0.5) will cause each note to reach zero halfway through its duration 
    giving a staccato feel. A value of 0x20 (meaning 2.0) will cause each note to reach 
    half its initial volume when the next note starts.

    When using tied notes, note that the decay length is set from the duration
    of the first note. This means that a decay value of 0x10 (1.0) will cause
    tied notes to be silent as the volume will have decayed to zero during the
    first note.

    The default for the decay is 0x20 (2.0) which allows two notes of the same
    length to be tied together.
*/
#define RINGTONE_DECAY(d) ((ringtone_note) (RINGTONE_SEQ_DECAY | (d)))

/*! \} */

#endif    /* __APP_RINGTONE_H__ */
