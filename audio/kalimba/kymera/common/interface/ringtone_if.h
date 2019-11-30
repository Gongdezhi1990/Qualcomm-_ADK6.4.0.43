/****************************************************************************

        Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.

FILE
        ringtone_if.h  -  Ringtone control library

CONTAINS
        The GLOBAL definitions for the RINGTONE subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.

*/

/*!
    @file ringtone_if.h

    @brief The global definitions for the ringtone subsystem.

\section audio Audio

BlueCore chips are provided with an advanced audio subsystem, and this can
be accessed by user code running on the VM.  The audio system allows for the
playing of tones of any pitch, or for the playing of samples.  In this way
maximum flexibility is maintained.

The audio subsystem is accessed by creating a stream from either an array of 
ringtone_notes, or a sample in the file system, and then connecting this stream 
to an Audio sink stream.
  
For example:

\code
StreamConnect(StreamRingtoneSource(ringtone), StreamAudioSink(audio_hardware, audio_instance, audio_channel));

StreamConnect(StreamFileSource(FileFind(FILE_ROOT, "sample", sizeof("sample")-1)), StreamAudioSink(audio_hardware, audio_instance, audio_channel));
\endcode

\subsection playing_ringtones Playing Ringtones

When playing ringtones from an array of #ringtone_note elements, each element
array can specify either a note value or a command.  Macros are defined
to build these arrays.

Notes are specified with either the RINGTONE_NOTE(note, duration) or
RINGTONE_NOTE_TIE(note, duration) macros.  The parameters to these two macros 
are the same, and their operation is very similar.  The \e note parameter specifies
the pitch of the note that will be played; the macro contains token-pasting, so we
only need to specify the last few characters of the note value (i.e., C3). The
\e duration parameter gives the duration of the note in musical terms.

\code
    RINGTONE_NOTE(C5, CROTCHET)     Play a middle C

    RINGTONE_NOTE(D7, SEMIBREVE)    Play a D two octaves up, for longer

    RINGTONE_NOTE(REST, SEMIQUAVER) Play nothing for a short time
\endcode

The available non-note commands are listed in the #ringtone_note documentation.
The most important of these is #RINGTONE_END, which must appear at the end
of every sequence.

So to play a simple ringtone we first declare the #ringtone_note array as
following (It is highly recommended to insert a rest note before RINGTONE_END
to avoid pop noise at end of ring tone):

\code
static const ringtone_note ringtone[] =
{
    RINGTONE_TIMBRE(sine),
    RINGTONE_NOTE(C5, MINIM),
    RINGTONE_TIMBRE(square),
    RINGTONE_NOTE(C5, MINIM),
    RINGTONE_TIMBRE(saw),
    RINGTONE_NOTE(C5, MINIM),
    RINGTONE_TIMBRE(triangle),
    RINGTONE_NOTE(C5, MINIM),
    RINGTONE_TIMBRE(triangle2),
    RINGTONE_NOTE(C5, MINIM),
    RINGTONE_TIMBRE(clipped_sine),
    RINGTONE_NOTE(C5, MINIM),
    RINGTONE_TIMBRE(plucked),
    RINGTONE_NOTE(C5, MINIM),
    RINGTONE_NOTE(REST, MINIM),
    RINGTONE_END
};
\endcode

and then connect it to a suitable Audio sink.

\code
StreamConnect(StreamRingtoneSource(ringtone), StreamAudioSink(audio_hardware, audio_instance, audio_channel));
\endcode

There may be a short gap in the sequence between moving from one note or
sample from the next. The precise lengths of these gaps may vary from one
build to the next and between multiple renditions of the same sequence.

\subsection playing_samples Playing samples

Applications can play samples by using the flash file system as follows.

 - Ensure the sample is in the correct format for the codec being used (e.g. 8/13/16bit).
 - Place the sample in the flash file system by adding it to the 'image' directory
   of the project.
 - Use FileFind() to get the index of the file. 
\code
        FILE_INDEX index = FileFind(FILE_ROOT, "sample", sizeof("sample")-1))
\endcode
 - Use StreamFileSource() to get a Source from the file index.  
\code
        Source file_src = StreamFileSource(index);
\endcode
 - Use StreamConnect() to connect the source to the desired Audio sink. 
\code
        StreamConnect(file_src, StreamAudioSink(audio_hardware, audio_instance, audio_channel));
\endcode

*/

#ifndef    __APP_RINGTONE_H__
#define    __APP_RINGTONE_H__
 

#include "ringtone_notes.h"


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

#define RINGTONE_SEQ_CONTROL_MASK           0x8000
#define RINGTONE_SEQ_NOTE                   0x0000
#define RINGTONE_SEQ_CONTROL               0x8000
#define RINGTONE_SEQ_NOTE_TIE_MASK           0x4000
#define RINGTONE_SEQ_NOTE_PITCH_MASK       0x3f80
#define RINGTONE_SEQ_NOTE_PITCH_POS           7
#define RINGTONE_SEQ_NOTE_LENGTH_MASK       0x007f
#define RINGTONE_SEQ_NOTE_LENGTH_POS       0
#define RINGTONE_SEQ_CONTROL_COMMAND_MASK  0x7000
#define RINGTONE_SEQ_CONTROL_COMMAND_POS   12

#define RINGTONE_SEQ_END    (RINGTONE_SEQ_CONTROL|(0<<RINGTONE_SEQ_CONTROL_COMMAND_POS))
#define RINGTONE_SEQ_TEMPO  (RINGTONE_SEQ_CONTROL|(1<<RINGTONE_SEQ_CONTROL_COMMAND_POS))
#define RINGTONE_SEQ_VOLUME (RINGTONE_SEQ_CONTROL|(2<<RINGTONE_SEQ_CONTROL_COMMAND_POS))
#define RINGTONE_SEQ_TIMBRE (RINGTONE_SEQ_CONTROL|(3<<RINGTONE_SEQ_CONTROL_COMMAND_POS))
#define RINGTONE_SEQ_DECAY  (RINGTONE_SEQ_CONTROL|(4<<RINGTONE_SEQ_CONTROL_COMMAND_POS))

#define RINGTONE_SEQ_TEMPO_MASK              0x0fff
#define RINGTONE_SEQ_VOLUME_MASK          0x00ff
#define RINGTONE_SEQ_TIMBRE_MASK          0x00ff
#define RINGTONE_SEQ_DECAY_RATE_MASK      0x00ff

#endif /*end of DO_NOT_DOCUMENT*/

/*!
\name Ringtone commands

These are the commands that can be put in a #ringtone_note.
*/

/*! \{ */

/*!
    This command must be given at the end of a sequence.  It causes the sequence
    to be stopped. All sequences must end this way. It is highly
        recommended to insert a rest note before RINGTONE_END to avoid pop noise
        at end of ring tone.
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
    half its initial volume when the next note starts. A value of 0 is not valid.

    When using tied notes, note that the decay length is set from the duration
    of the first note. This means that a decay value of 0x10 (1.0) will cause
    tied notes to be silent as the volume will have decayed to zero during the
    first note.

    The default for the decay is 0x20 (2.0) which allows two notes of the same
    length to be tied together.
*/
#define RINGTONE_DECAY(d) ((ringtone_note) (RINGTONE_SEQ_DECAY | (d)))
/*!
    Alternative to #RINGTONE_DECAY meaning 'no decay'; notes stay at their
    initial volume until the end of the defined period.
    (This is an extension relative to BlueCore's StreamRingtoneSource().)
*/
#define RINGTONE_DECAY_NONE ((ringtone_note) (RINGTONE_SEQ_DECAY | 0))

/*! \} */

#endif    /* __APP_RINGTONE_H__ */
