/*
Copyright (c) 2004 - 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    module responsible for tone generation and playback
*/

#include "sink_tones.h"
#include "sink_audio_prompts.h" 
#include "sink_events.h"
#include "sink_volume.h"
#include "sink_audio.h"
#include "sink_slc.h"
#include "sink_states.h"
#include "sink_statemanager.h"
#include "sink_pio.h"
#include "sink_multi_channel.h"
#include "sink_event_queue.h"
#include "sink_hfp_data.h"
#include "gain_utils.h"
#include "sink_config.h"
#include "sink_audio_routing.h"
#include "sink_usb.h"
#include "sink_wired.h"
#include "sink_fm.h"
#include "sink_malloc_debug.h"

/* Include config store and definition headers */
#include "config_definition.h"
#include "sink_tones_config_def.h"
#include <config_store.h>

#include <stddef.h>
#include <csrtypes.h>
#include <audio.h>
#include <audio_config.h>

/* setting Tone volume to maximum */
#define MAX_TONE_VOLUME (255)

#define USE_CURRENT_TONE_VOLUME (0)

#ifdef DEBUG_TONES
    #define TONE_DEBUG(x) DEBUG(x)
#else
    #define TONE_DEBUG(x)
#endif

/****************************************************************************
VARIABLES
*/
/***********************************************************************************/

/* Sink tones */

/***********************************************************************************/

/* Soft stop on tones
 *
 * A rest following a note will cause the note to fade out to zero over 64 samples.
 * The firmware tone generator runs at 8kHz, so 64 samples takes 8ms.
 *
 * To avoid a 'pop' noise at the end of any notes, a rest of at least 8ms should
 * be included. Rest length should not be too long to avoid large silences after
 * the completed tones.
 *
 * The following table shows the maximum tempo for each note duration that
 * allows at least 8ms duration. The shortest note duration, for a given tempo,
 * that is at least 8ms long should be used.
 *
 * For example: for tempo of 300, SEMIQUAVER_TRIPLET should be selected.
 *      RINGTONE_TEMPO(300),
 *      RINGTONE_NOTE(G5,   CROTCHET),
 *      RINGTONE_NOTE(A5,   CROTCHET),
 *      RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),  <- rest that allows fade out
 *      RINGTONE_END
 *
 *  Tempo   | Duration > 8ms
 * --------------------------------------
 * <= 7500  | SEMIBREVE
 * <= 3750  | MINIM
 * <= 2500  | MINIM_TRIPLET
 * <= 1875  | CROTCHET
 * <= 1250  | CROTCHET_TRIPLET
 * <= 937   | QUAVER
 * <= 625   | QUAVER_TRIPLET
 * <= 468   | SEMIQUAVER
 * <= 312   | SEMIQUAVER_TRIPLET
 * <= 234   | DEMISEMIQUAVER
 * <= 156   | DEMISEMIQUAVER_TRIPLET
 * <= 117   | HEMIDEMISEMIQUAVER
 * <= 78    | HEMIDEMISEMIQUAVER_TRIPLET
 *
 */

/****************************************************************/
/*
    SIMPLE TONES ( B - Q )
 */
/****************************************************************/


 /*Vol Min*/
static const ringtone_note tone_B[] =
{
    RINGTONE_TEMPO(200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5,   CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};
 /*Vol 2*/
static const ringtone_note tone_C[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C6, SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};
 /*Vol 3*/
static const ringtone_note tone_D[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6, SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};
 /*Vol 4*/
static const ringtone_note tone_E[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G6, SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};
 /*Vol 5*/
static const ringtone_note tone_F[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(AS6, SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};
 /*Vol 6*/
static const ringtone_note tone_G[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C7, SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};
 /*Vol 7*/
static const ringtone_note tone_H[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(D7, SEMIBREVE),

    RINGTONE_END
};
 /*Vol 8*/
static const ringtone_note tone_I[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E7, SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};
 /*Vol 9*/
static const ringtone_note tone_J[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(FS7, SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};
 /*Vol 10*/
static const ringtone_note tone_K[] =
{
    RINGTONE_TEMPO(200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(GS7, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};


/*4 note rising scale*/
static const ringtone_note tone_L[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(GS4, CROTCHET),
    RINGTONE_NOTE(REST,DEMISEMIQUAVER),
    RINGTONE_NOTE(DS5, CROTCHET),
    RINGTONE_NOTE(REST,DEMISEMIQUAVER),
    RINGTONE_NOTE(GS5, CROTCHET),
    RINGTONE_NOTE(REST,DEMISEMIQUAVER),
    RINGTONE_NOTE(C6, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
};
/*4 note falling scale*/
static const ringtone_note tone_M[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(REST,DEMISEMIQUAVER),
    RINGTONE_NOTE(GS5, CROTCHET),
    RINGTONE_NOTE(REST,DEMISEMIQUAVER),
    RINGTONE_NOTE(DS5, CROTCHET),
    RINGTONE_NOTE(REST,DEMISEMIQUAVER),
    RINGTONE_NOTE(GS4, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
} ;
/*2 tone rising scale*/
static const ringtone_note tone_N[] =
{
    RINGTONE_TEMPO(150), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6, QUAVER),
    RINGTONE_NOTE(REST,DEMISEMIQUAVER),
    RINGTONE_NOTE(A6, QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER_TRIPLET),
    RINGTONE_END
};
/*2 tone falling scale*/
static const ringtone_note tone_O[] =
{
    RINGTONE_TEMPO(150), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(A6, QUAVER),
    RINGTONE_NOTE(REST,DEMISEMIQUAVER),
    RINGTONE_NOTE(E6, QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER_TRIPLET),
    RINGTONE_END
};
/*long low followed by short high*/
static const ringtone_note tone_P[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5, SEMIBREVE),
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_NOTE(G7, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};
/*connection*/
static const ringtone_note tone_Q[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(GS7 , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};

/*power*/
static const ringtone_note tone_R[] =
{
    RINGTONE_TEMPO(120), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(GS7 , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER_TRIPLET),
    RINGTONE_END
};


/*error*/
static const ringtone_note tone_S[] =
{
    RINGTONE_TEMPO(120), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5 , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER_TRIPLET),
    RINGTONE_END
};

/*short cfm*/
static const ringtone_note tone_T[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G6 , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};

/*long cfm*/
static const ringtone_note tone_U[] =
{
    RINGTONE_TEMPO(150), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G6 , MINIM),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER_TRIPLET),
    RINGTONE_END
};


/*Batt Low*/
static const ringtone_note tone_V[] =
{
    RINGTONE_TEMPO(120), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G6 , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER_TRIPLET),
    RINGTONE_END
};

/*Pair removal*/
static const ringtone_note tone_W[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5 , MINIM_TRIPLET),
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_NOTE(G5 , MINIM_TRIPLET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*Double*/
static const ringtone_note tone_X[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G6 , MINIM_TRIPLET),
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_NOTE(G6 , MINIM_TRIPLET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};



/****************************************************************/
/*
    OTHER TONES
*/
/****************************************************************/


static const ringtone_note battery_low_tone[] =
{
    RINGTONE_TEMPO(240), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6, QUAVER),
    RINGTONE_NOTE(AS6, QUAVER),
    RINGTONE_NOTE(E6, QUAVER),
    RINGTONE_NOTE(AS6, QUAVER),
    RINGTONE_NOTE(E6, QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};

static const ringtone_note low_500ms_tone[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_TEMPO(120), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),
    RINGTONE_NOTE(G5, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

static const ringtone_note mute_reminder_tone[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_TEMPO(120), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),
    RINGTONE_NOTE(G5, CROTCHET),
    RINGTONE_NOTE(REST, CROTCHET),
    RINGTONE_NOTE(G5, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};


/****************************************************************/
/*
    RING TONES
*/
/****************************************************************/

static const ringtone_note ring_twilight[] =
{
    RINGTONE_TEMPO(180), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E7, QUAVER),
    RINGTONE_NOTE(F7, QUAVER),
    RINGTONE_NOTE(E7, QUAVER),
    RINGTONE_NOTE(C7, QUAVER),
    RINGTONE_NOTE(E7, QUAVER),
    RINGTONE_NOTE(F7, QUAVER),
    RINGTONE_NOTE(E7, QUAVER),
    RINGTONE_NOTE(C7, QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};




static const ringtone_note ring_g_s_t_q[] =
{
    RINGTONE_TEMPO(230), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE  (G7, CROTCHET),
    RINGTONE_NOTE  (G7, CROTCHET),
    RINGTONE_NOTE  (A7, CROTCHET),
    RINGTONE_NOTE  (FS7, CROTCHET),
    RINGTONE_NOTE_TIE (FS7, QUAVER),
    RINGTONE_NOTE  (G7, QUAVER),
    RINGTONE_NOTE  (A7, CROTCHET),
    RINGTONE_NOTE  (B7, CROTCHET),
    RINGTONE_NOTE  (B7, CROTCHET),
    RINGTONE_NOTE  (C8, CROTCHET),
    RINGTONE_NOTE  (B7, CROTCHET),
    RINGTONE_NOTE_TIE (B7, QUAVER),
    RINGTONE_NOTE  (A7, QUAVER),
    RINGTONE_NOTE  (G7, CROTCHET),
    RINGTONE_NOTE  (A7, CROTCHET),
    RINGTONE_NOTE  (G7, CROTCHET),
    RINGTONE_NOTE  (FS7, CROTCHET),
    RINGTONE_NOTE  (G7, MINIM),
    RINGTONE_NOTE_TIE (G7, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};



static const ringtone_note ring_valkyries[] =
{
    RINGTONE_TEMPO(240), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E5,QUAVER),
    RINGTONE_NOTE_TIE(E5,SEMIQUAVER),
    RINGTONE_NOTE(D5,SEMIQUAVER),
    RINGTONE_NOTE(E5,QUAVER),
    RINGTONE_NOTE(G5,CROTCHET),
    RINGTONE_NOTE_TIE(G5,QUAVER),
    RINGTONE_NOTE(E5,CROTCHET),
    RINGTONE_NOTE_TIE(E5,QUAVER),
    RINGTONE_NOTE(G5,QUAVER),
    RINGTONE_NOTE_TIE(G5,SEMIQUAVER),
    RINGTONE_NOTE(F5,SEMIQUAVER),
    RINGTONE_NOTE(G5,QUAVER),
    RINGTONE_NOTE(B5,CROTCHET),
    RINGTONE_NOTE_TIE(B5,QUAVER),
    RINGTONE_NOTE(G5,CROTCHET),
    RINGTONE_NOTE_TIE(G5,QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};


static const ringtone_note ring_greensleeves[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(F6,CROTCHET),
    RINGTONE_NOTE(AF6,MINIM),
    RINGTONE_NOTE(BF6,CROTCHET),
    RINGTONE_NOTE(C7,CROTCHET),
    RINGTONE_NOTE_TIE(C7,QUAVER),
    RINGTONE_NOTE(DF7,QUAVER),
    RINGTONE_NOTE(C7,CROTCHET),
    RINGTONE_NOTE(BF6,MINIM),
    RINGTONE_NOTE(G6,CROTCHET),
    RINGTONE_NOTE(EF6,CROTCHET),
    RINGTONE_NOTE_TIE(EF6,QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
};



static const ringtone_note ring_two_beeps[] =
{
    RINGTONE_TEMPO(300), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B5,SEMIQUAVER),
    RINGTONE_NOTE(G5,SEMIQUAVER),
    RINGTONE_NOTE(B5,SEMIQUAVER),
    RINGTONE_NOTE(G5,SEMIQUAVER),
    RINGTONE_NOTE(B5,SEMIQUAVER),
    RINGTONE_NOTE(REST,QUAVER),
    RINGTONE_NOTE(B5,SEMIQUAVER),
    RINGTONE_NOTE(G5,SEMIQUAVER),
    RINGTONE_NOTE(B5,SEMIQUAVER),
    RINGTONE_NOTE(G5,SEMIQUAVER),
    RINGTONE_NOTE(B5,SEMIQUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};




static const ringtone_note ring_fairground[] =
{
    RINGTONE_TEMPO(300), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6,CROTCHET),
    RINGTONE_NOTE(D6,CROTCHET),
    RINGTONE_NOTE(DF6,QUAVER),
    RINGTONE_NOTE(D6,QUAVER),
    RINGTONE_NOTE(DF6,QUAVER),
    RINGTONE_NOTE(C6,QUAVER),
    RINGTONE_NOTE(DF6,CROTCHET),
    RINGTONE_NOTE(C6,CROTCHET),
    RINGTONE_NOTE(B5,CROTCHET),
    RINGTONE_NOTE(C6,CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};



static const ringtone_note ring_counterpoint[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6,CROTCHET),
    RINGTONE_NOTE(D6,CROTCHET),
    RINGTONE_NOTE(E6,CROTCHET),
    RINGTONE_NOTE(DF6,CROTCHET),
    RINGTONE_NOTE(E6,CROTCHET),
    RINGTONE_NOTE(C6,CROTCHET),
    RINGTONE_NOTE(E6,CROTCHET),
    RINGTONE_NOTE(B5,CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
};


static const ringtone_note ring_major_arp[] =
{
    RINGTONE_TEMPO(220), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E5,QUAVER),
    RINGTONE_NOTE(AF5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(E6,QUAVER),
    RINGTONE_NOTE(GS6,QUAVER),
    RINGTONE_NOTE(B6,QUAVER),
    RINGTONE_NOTE(E7,QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};


static const ringtone_note ring_major_scale[] =
{
    RINGTONE_TEMPO(300), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6,QUAVER),
    RINGTONE_NOTE(FS6,QUAVER),
    RINGTONE_NOTE(GS6,QUAVER),
    RINGTONE_NOTE(A6,QUAVER),
    RINGTONE_NOTE(B6,QUAVER),
    RINGTONE_NOTE(CS7,QUAVER),
    RINGTONE_NOTE(DS7,QUAVER),
    RINGTONE_NOTE(E7,QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};

static const ringtone_note ring_minor_arp[] =
{
    RINGTONE_TEMPO(220), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E5,QUAVER),
    RINGTONE_NOTE(G5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(E6,QUAVER),
    RINGTONE_NOTE(G6,QUAVER),
    RINGTONE_NOTE(B6,QUAVER),
    RINGTONE_NOTE(E7,QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};




static const ringtone_note ring_harmonic_minor_scale[] =
{
    RINGTONE_TEMPO(300), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E5,QUAVER),
    RINGTONE_NOTE(FS5,QUAVER),
    RINGTONE_NOTE(G5,QUAVER),
    RINGTONE_NOTE(A5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(C6,QUAVER),
    RINGTONE_NOTE(DS6,QUAVER),
    RINGTONE_NOTE(E6,QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};



static const ringtone_note ring_more_than_a_feeling[] =
{
    RINGTONE_TEMPO(200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(D6,QUAVER),
    RINGTONE_NOTE(A6,QUAVER),
    RINGTONE_NOTE(D7,QUAVER),
    RINGTONE_NOTE(G7,CROTCHET),
    RINGTONE_NOTE(A6,QUAVER),
    RINGTONE_NOTE(FS7,CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};




static const ringtone_note ring_eight_beeps[] =
{
    RINGTONE_TEMPO(200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};



static const ringtone_note ring_four_beeps[] =
{
    RINGTONE_TEMPO(180), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),
    RINGTONE_NOTE(B5,QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};




static const ringtone_note ring_bumble_bee[] =
{
    RINGTONE_TEMPO(70), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E7,DEMISEMIQUAVER),

    RINGTONE_NOTE(EF7,DEMISEMIQUAVER),

    RINGTONE_NOTE(D7,DEMISEMIQUAVER),

    RINGTONE_NOTE(DF7,DEMISEMIQUAVER),

    RINGTONE_NOTE(EF7,DEMISEMIQUAVER),

    RINGTONE_NOTE(D7,DEMISEMIQUAVER),

    RINGTONE_NOTE(DF7,DEMISEMIQUAVER),

    RINGTONE_NOTE(C7,DEMISEMIQUAVER),

    RINGTONE_NOTE(D7,DEMISEMIQUAVER),

    RINGTONE_NOTE(DF7,DEMISEMIQUAVER),

    RINGTONE_NOTE(C7,DEMISEMIQUAVER),

    RINGTONE_NOTE(B6,DEMISEMIQUAVER),

    RINGTONE_NOTE(DF7,DEMISEMIQUAVER),

    RINGTONE_NOTE(C7,DEMISEMIQUAVER),

    RINGTONE_NOTE(B6,DEMISEMIQUAVER),

    RINGTONE_NOTE(BF6,DEMISEMIQUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, HEMIDEMISEMIQUAVER_TRIPLET),
    RINGTONE_END
};


static const ringtone_note ring_aug_scale[] =
{
    RINGTONE_TEMPO(280), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6,CROTCHET),
    RINGTONE_NOTE(CS7,CROTCHET),
    RINGTONE_NOTE(F7,CROTCHET),
    RINGTONE_NOTE(A7,CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};



static const ringtone_note ring_dim_scale[] =
{
    RINGTONE_TEMPO(280), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6,CROTCHET),
    RINGTONE_NOTE(G6,CROTCHET),
    RINGTONE_NOTE(AS6,CROTCHET),
    RINGTONE_NOTE(DF7,CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};


static const ringtone_note CustomRingTone[] =
{
    /* A sample custom ring tone */
    RINGTONE_TEMPO(280), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),
    RINGTONE_NOTE(E6,CROTCHET),
    RINGTONE_NOTE(G6,CROTCHET),
    RINGTONE_NOTE(AS6,CROTCHET),
    RINGTONE_NOTE(DF7,CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};

/**********************************************************************/

/*4 low to high 38ms*/
static const ringtone_note tone_A1[] =
{
    RINGTONE_TEMPO(1600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5 , CROTCHET),
    RINGTONE_NOTE(D6 , CROTCHET),
    RINGTONE_NOTE(G6 , CROTCHET),
    RINGTONE_NOTE(B5 , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET),
    RINGTONE_END
};
/*4 high to low 38ms*/
static const ringtone_note tone_A2[] =
{
    RINGTONE_TEMPO(1600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B5 , CROTCHET),
    RINGTONE_NOTE(G6 , CROTCHET),
    RINGTONE_NOTE(D6 , CROTCHET),
    RINGTONE_NOTE(G5 , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET),
    RINGTONE_END
};

/*short high low 25ms**/
/*no-network-service tone - 880 Hz for 100ms*/
static const ringtone_note tone_A3[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),
    RINGTONE_NOTE(B5, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/*series of 5 high tones 94 ms*/
static const ringtone_note tone_A4[] =
{
    RINGTONE_TEMPO(640), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B6 , CROTCHET),
    RINGTONE_NOTE(B6 , CROTCHET),
    RINGTONE_NOTE(B6 , CROTCHET),
    RINGTONE_NOTE(B6 , CROTCHET),
    RINGTONE_NOTE(B6 , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*low to high 100ms tones 50ms pause*/
static const ringtone_note tone_A5[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(DS7  , CROTCHET),
    RINGTONE_NOTE(REST , QUAVER),
    RINGTONE_NOTE(G7   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/*high to low 100ms tones 50ms pause*/
static const ringtone_note tone_A6[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G7   , CROTCHET),
    RINGTONE_NOTE(REST , QUAVER),
    RINGTONE_NOTE(DS7  , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/***************************************************************************/
/*S-Tones*/
    /*short v high 2400hz*/
static const ringtone_note tone_S1[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(DS8   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};

/*series(12) of high / higher beeps*/
static const ringtone_note tone_S2[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B6   , CROTCHET),
    RINGTONE_NOTE(G8   , CROTCHET),
    RINGTONE_NOTE(B6   , CROTCHET),
    RINGTONE_NOTE(G8   , CROTCHET),
    RINGTONE_NOTE(B6   , CROTCHET),
    RINGTONE_NOTE(G8   , CROTCHET),
    RINGTONE_NOTE(B6   , CROTCHET),
    RINGTONE_NOTE(G8   , CROTCHET),
    RINGTONE_NOTE(B6   , CROTCHET),
    RINGTONE_NOTE(G8   , CROTCHET),
    RINGTONE_NOTE(B6   , CROTCHET),
    RINGTONE_NOTE(G8   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};
    /*short middle (987)  */
static const ringtone_note tone_S3[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B6  , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};

    /*series of 5 high beeps with pause*/
static const ringtone_note tone_S4[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C6  , CROTCHET),
    RINGTONE_NOTE(REST  , CROTCHET),

    RINGTONE_NOTE(C6  , CROTCHET),
    RINGTONE_NOTE(REST  , CROTCHET),

    RINGTONE_NOTE(C6  , CROTCHET),
    RINGTONE_NOTE(REST  , CROTCHET),

    RINGTONE_NOTE(C6  , CROTCHET),
    RINGTONE_NOTE(REST  , CROTCHET),

    RINGTONE_NOTE(C6  , CROTCHET),
    RINGTONE_NOTE(REST  , CROTCHET),

    /* Tone soft stop (covered by final crotchet rest). Duration must be > 8ms*/
    RINGTONE_END
};

    /*Low / High beep  */
static const ringtone_note tone_S5[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B6  , CROTCHET),
    RINGTONE_NOTE(REST  , CROTCHET),
    RINGTONE_NOTE(DS8  , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};
    /* High / Low beep  */
static const ringtone_note tone_S6[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(DS8  , CROTCHET),
    RINGTONE_NOTE(REST  , CROTCHET),
    RINGTONE_NOTE(B6  , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};
/***************************************************************/
/*New tones for Q2 2007*/

/*Short Middle Tone, 100ms D6*/
static const ringtone_note tone_Q21[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(D6  , MINIM),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
};
/*4 Low to High Long at Top, 100ms G5 D6 G6 300ms B6 */
static const ringtone_note tone_Q22[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5  , CROTCHET_TRIPLET),
    RINGTONE_NOTE(REST  , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(D6  , CROTCHET_TRIPLET),
    RINGTONE_NOTE(REST  , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(G6  , CROTCHET_TRIPLET),
    RINGTONE_NOTE(REST  , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(B6  , MINIM),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
};

/*4 High to Low Long at Bottom, 100ms B6 G6 D6 300ms G5 */
static const ringtone_note tone_Q23[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B6  , CROTCHET_TRIPLET),
    RINGTONE_NOTE(REST  , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(G6  , CROTCHET_TRIPLET),
    RINGTONE_NOTE(REST  , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(D6  , CROTCHET_TRIPLET),
    RINGTONE_NOTE(REST  , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(G5  , MINIM),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
};

/*3 Note Descending, 300ms G6 D6 G5*/
static const ringtone_note tone_Q24[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G6  , MINIM),
    RINGTONE_NOTE(REST  , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(D6  , MINIM),
    RINGTONE_NOTE(REST  , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(G5  , MINIM),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
};

/*5th Down Medium-Long, 300ms D6 500ms G5*/
static const ringtone_note tone_Q25[] =
{
    RINGTONE_TEMPO(200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(D6  , CROTCHET),
    RINGTONE_NOTE(REST  , DEMISEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(G5  , MINIM_TRIPLET),
    RINGTONE_NOTE_TIE(G5  , QUAVER_TRIPLET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, DEMISEMIQUAVER),
    RINGTONE_END
};

/*5th Down Short-Short, 100ms D6 G5*/
static const ringtone_note tone_Q26[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(D6  , SEMIBREVE),
    RINGTONE_NOTE(REST  , CROTCHET),
    RINGTONE_NOTE(G5  , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};

/*5th Up Short-Short, 100ms G5 D6*/
static const ringtone_note tone_Q27[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5  , SEMIBREVE),
    RINGTONE_NOTE(REST  , CROTCHET),
    RINGTONE_NOTE(D6  , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};

/*2 Half Second Low Tones, 500ms G5 REST G5*/
static const ringtone_note tone_Q28[] =
{
    RINGTONE_TEMPO(480), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5  , SEMIBREVE),
    RINGTONE_NOTE(REST  , QUAVER_TRIPLET),
    RINGTONE_NOTE(G5  , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/*Short High Tone, 100ms G7*/
static const ringtone_note tone_Q29[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G7  , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};

/*Long Low Tone, 400ms G5*/
static const ringtone_note tone_Q2A[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(REST  , SEMIQUAVER),
    RINGTONE_NOTE(G5  , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/*Double Low Tones, 200ms G5 REST G5*/
static const ringtone_note tone_Q2B[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5  , MINIM),
    RINGTONE_NOTE(REST  , CROTCHET),
    RINGTONE_NOTE(G5  , MINIM),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/*G Major Arp Up, 125ms G6 B6 D7 G7*/

static const ringtone_note tone_Q2C[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G6  , MINIM_TRIPLET),
    RINGTONE_NOTE(B6  , MINIM_TRIPLET),
    RINGTONE_NOTE(D7  , MINIM_TRIPLET),
    RINGTONE_NOTE(G7  , MINIM_TRIPLET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/*G Major Arp Down, 125ms G7 D7 B6 G6*/

static const ringtone_note tone_Q2D[] =
{
    RINGTONE_TEMPO(600), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G7  , MINIM_TRIPLET),
    RINGTONE_NOTE(D7  , MINIM_TRIPLET),
    RINGTONE_NOTE(B6  , MINIM_TRIPLET),
    RINGTONE_NOTE(G6  , MINIM_TRIPLET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/*Fairly Short High Tone, 150ms G7*/
static const ringtone_note tone_Q2E[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G7  , MINIM),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*Fairly Short Low Tone, 150ms G5*/
static const ringtone_note tone_Q2F[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5  , MINIM),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*Medium Length Low Tone, 200ms G5*/
static const ringtone_note tone_Q210[] =
{
    RINGTONE_TEMPO(300), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5  , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};


  /*Short Low-High, 80ms G5 G7*/
static const ringtone_note tone_Q211[] =
{
    RINGTONE_TEMPO(300), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5  , SEMIBREVE),
    RINGTONE_NOTE(REST  , SEMIBREVE),
    RINGTONE_NOTE(G7  , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};

/*Double Octave Toggle Ring Tone, 50ms G7 G5 G7 G5 G7 G5 Repeat after 250ms*/
static const ringtone_note tone_Q212[] =
{
    RINGTONE_TEMPO(100), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G7  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G5  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G7  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G5  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G7  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G5  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(REST  , SEMIQUAVER  ),
    RINGTONE_NOTE_TIE(REST , SEMIQUAVER_TRIPLET),
    RINGTONE_NOTE(G7  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G5  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G7  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G5  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G7  , DEMISEMIQUAVER_TRIPLET  ),
    RINGTONE_NOTE(G5  , DEMISEMIQUAVER_TRIPLET  ),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, HEMIDEMISEMIQUAVER),
    RINGTONE_END
};


/*Low Low High, 100ms G5 G5 G7*/
static const ringtone_note tone_Q213[] =
{
    RINGTONE_TEMPO(2400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G5  , SEMIBREVE),
    RINGTONE_NOTE(G5  , SEMIBREVE),
    RINGTONE_NOTE(G7  , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_END
};

/*Double Low Tones (long), 300ms G5 G5*/
static const ringtone_note tone_Q214[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(G6 , SEMIBREVE),
    RINGTONE_NOTE(REST, MINIM_TRIPLET),
    RINGTONE_NOTE(G6 , SEMIBREVE),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*Connected 48mS C6 D6 A6 */
static const ringtone_note tone_Q31[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(D6, CROTCHET),
    RINGTONE_NOTE(A6, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};

/*Connected 48mS A6 D6 C6 */
static const ringtone_note tone_Q32[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(A6, CROTCHET),
    RINGTONE_NOTE(D6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};

/*confirmation tone 68mS C5 E5  */
static const ringtone_note tone_Q33[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C5, CROTCHET),
    RINGTONE_NOTE(E5, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*error tone 2 68mS A5 REST A5 */
static const ringtone_note tone_Q34[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(A5, CROTCHET),
    RINGTONE_NOTE(REST, CROTCHET),
    RINGTONE_NOTE(A5, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*low batt 2 125mS E6 F5 */
static const ringtone_note tone_Q35[] =
{
    RINGTONE_TEMPO(480), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(F5, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER_TRIPLET),
    RINGTONE_END
};

/*mute off 2 48mS A5 A4 */
static const ringtone_note tone_Q36[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(A5, CROTCHET),
    RINGTONE_NOTE(A4, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};

/*mute on 2 48mS A4 A5 */
static const ringtone_note tone_Q37[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(A4, CROTCHET),
    RINGTONE_NOTE(A5, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};

/*single tone 2 48mS A5 */
static const ringtone_note tone_Q38[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(A5, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};

/*shutdown tone 2 408mS A5, 136mS A4  */
static const ringtone_note tone_Q39[] =
{
    RINGTONE_TEMPO(300), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(A5, MINIM),
    RINGTONE_NOTE(A4, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER_TRIPLET),
    RINGTONE_END
};

/*phone ring 48mS C6 E6 C6 E6 C6 E6 C6 E6 C6 E6 C6 REST C6 E6 C6 E6 C6 E6 C6 E6 C6 E6 C6 */
static const ringtone_note tone_Q3A[] =
{
    RINGTONE_TEMPO(1200), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(REST, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),
    RINGTONE_NOTE(E6, CROTCHET),
    RINGTONE_NOTE(C6, CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, CROTCHET_TRIPLET),
    RINGTONE_END
};

/*power up tone 83mS C5 10mS REST 83mS E5 10mS REST 83mS G5 10mS REST 124mS B5 498mS REST 372mS C6 */
static const ringtone_note tone_Q3B[] =
{
    RINGTONE_TEMPO(400), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C5   , QUAVER),
    RINGTONE_NOTE(REST , DEMISEMIQUAVER),
    RINGTONE_NOTE(E5   , QUAVER),
    RINGTONE_NOTE(REST , DEMISEMIQUAVER),
    RINGTONE_NOTE(G5   , QUAVER),
    RINGTONE_NOTE(REST , DEMISEMIQUAVER),
    RINGTONE_NOTE(B5   , CROTCHET),
    RINGTONE_NOTE(REST , SEMIBREVE),
    RINGTONE_NOTE(C6   , MINIM),
    RINGTONE_NOTE_TIE(C6, QUAVER),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_END
};

/*vol 7/0 68mS A5 REST A5*/
static const ringtone_note tone_Q3C[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(A5   , CROTCHET),
    RINGTONE_NOTE(REST   , CROTCHET),
    RINGTONE_NOTE(A5   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*vol 6 68mS DS6*/
static const ringtone_note tone_Q3D[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(DS6   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*vol 5 68mS D6*/
static const ringtone_note tone_Q3E[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(D6   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*vol 4 68mS CS6*/
static const ringtone_note tone_Q3F[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(CS6   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*vol 3 68mS C6 */
static const ringtone_note tone_Q40[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(C6   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*vol 2 68mS B5*/
static const ringtone_note tone_Q41[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(B5   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/*vol 1 68mS AS5*/
static const ringtone_note tone_Q42[] =
{
    RINGTONE_TEMPO(800), RINGTONE_VOLUME(MAX_TONE_VOLUME), RINGTONE_TIMBRE(sine),

    RINGTONE_NOTE(AS5   , CROTCHET),

    /* Tone soft stop. Duration must be > 8ms*/
    RINGTONE_NOTE(REST, QUAVER),
    RINGTONE_END
};

/***************************************************************************/
/*
    The Tone Array
*/
/*************************************************************************/

/* This must make use of all of the defined tones - requires the extra space first */
static const ringtone_note * const gFixedTones[] =
{
/*1*/    tone_B  ,  /*Vol 0*/
/*2*/    tone_C  ,  /*Vol 1*/
/*3*/    tone_D  ,  /*Vol 2*/
/*4*/    tone_E  ,  /*Vol 3*/

/*5*/    tone_F  ,  /*Vol 4*/
/*6*/    tone_G  ,  /*Vol 5*/
/*7*/    tone_H  ,  /*Vol 6*/
/*8*/    tone_I  ,  /*Vol 7*/

/*9*/    tone_J  ,  /*Vol 8*/
/*A*/    tone_K  ,  /*Vol 9*/
/*b*/    tone_L  ,  /*Vol 10*/
/*c*/    tone_M  ,  /*4 note rise*/

/*d*/    tone_N  ,  /*2 note ris*/
/*e*/    tone_O  ,  /*2 note fall*/
/*f*/    tone_P  ,  /*long low short high*/
/*10*/   tone_Q  ,  /*conn  */

/*11*/   tone_R  ,  /*power*/
/*12*/   tone_S  ,  /*error*/
/*13*/   tone_T  ,  /*short cfm*/
/*14*/   tone_U  ,  /*long CFM*/

/*15*/   tone_V  ,  /*Low Batt*/
/*16*/   tone_W  ,  /*pair removal*/
/*17*/   tone_X  ,  /*double*/
/*18*/   battery_low_tone    ,

/*19*/   low_500ms_tone      ,
/*1a*/   mute_reminder_tone  ,
/*1b*/   ring_twilight       ,
/*1c*/   ring_g_s_t_q        ,

/*1d*/   ring_valkyries      ,
/*1e*/   ring_greensleeves   ,
/*1f*/   ring_two_beeps      ,
/*20*/   ring_fairground     ,

/*21*/   ring_counterpoint   ,
/*22*/   ring_major_arp      ,
/*23*/   ring_major_scale    ,
/*24*/   ring_minor_arp      ,

/*25*/   ring_harmonic_minor_scale,
/*26*/   ring_more_than_a_feeling ,
/*27*/   ring_eight_beeps         ,
/*28*/   ring_four_beeps          ,

/*29*/   ring_bumble_bee ,
/*2a*/   ring_aug_scale  ,
/*2b*/   ring_dim_scale  ,

/*2c*/   CustomRingTone  , /*this is a custom ring tone that can be defined if required.*/
/*2d*/   tone_A1         ,   /*4 low to high 38ms*/
/*2e*/   tone_A2         ,   /*4 high to low 38ms*/
/*2f*/   tone_A3         ,   /*880 Hz for 100ms*/

/*30*/   tone_A4         ,   /*series of 5 high tones 94 ms*/
/*31*/   tone_A5         ,   /*low to high 100ms tones 50ms pause*/
/*32*/   tone_A6         ,   /*high to low 100ms tones 50ms pause*/
/*33*/   tone_S1         ,   /*short v high 2400hz*/

/*34*/   tone_S2         ,   /*series(12) of high / higher beeps*/
/*35*/   tone_S3         ,   /*short middle (987) **/
/*36*/   tone_S4         ,   /*series of 5 high beeps with pause*/
/*37*/   tone_S5         ,   /*Low High*/

/*38*/   tone_S6         ,   /*High Low*/
/*39*/     tone_Q21         ,   /*Short Middle Tone, 100ms D6*/
/*3a*/     tone_Q22         ,   /*4 Low to High Long at Top, 100ms G5 D6 G6 300ms B6 */
/*3b*/     tone_Q23         ,   /*4 High to Low Long at Bottom, 100ms B6 G6 D6 300ms G5 */

/*3c*/   tone_Q24         ,     /*3 Note Descending, 300ms G6 D6 G5*/
/*3d*/     tone_Q25         ,     /*5th Down Medium-Long, 300ms D6 500ms G5*/
/*3e*/     tone_Q26         ,     /*5th Down Short-Short, 100ms D6 G5*/
/*3f*/     tone_Q27         ,      /*5th Up Short-Short, 100ms G5 D6*/

/*40*/   tone_Q28         ,   /*2 Half Second Low Tones, 500ms G5 REST G5*/
/*41*/   tone_Q29         ,   /*Short High Tone, 100ms G7*/
/*42*/     tone_Q2A         ,   /*Long Low Tone, 400ms G5*/
/*43*/   tone_Q2B        ,   /*Double Low Tones, 200ms G5 REST G5*/

/*44*/   tone_Q2C        ,   /*G Major Arp Up, 125ms G6 B6 D7 G7*/
/*45*/   tone_Q2D        ,   /*G Major Arp Down, 125ms G7 D7 B6 G6*/
/*46*/   tone_Q2E        ,   /*Fairly Short High Tone, 150ms G7*/
/*47*/   tone_Q2F         ,   /*Fairly Short Low Tone, 150ms G5*/

/*48*/   tone_Q210       ,   /*Medium Length Low Tone, 200ms G5*/
/*49*/   tone_Q211       ,   /*Short Low-High, 80ms G5 G7*/
/*4A*/   tone_Q212       ,   /*Double Octave Toggle Ring Tone, 50ms G7 G5 G7 G5 G7 G5 Repeat after 250ms*/
/*4B*/   tone_Q213         ,     /*Low Low High, 100ms G5 G5 G7*/

/*4c*/     tone_Q214         ,   /*Double Low Tones (long), 300ms G5 G5*/
/*4d*/     tone_Q31         ,   /*Connected 48mS C6 D6 A6 */
/*4e*/     tone_Q32         ,   /*Disconnected 48mS A6 D6 C6 */
/*4f*/     tone_Q33         ,   /*confirmation tone 68mS C5 E5 */

/*50*/     tone_Q34         ,   /*error tone 2 68mS A5 REST A5 */
/*51*/     tone_Q35         ,   /*low batt 2 125mS E6 F5 */
/*52*/     tone_Q36         ,   /*mute off 2 48mS A5 A4 */
/*53*/     tone_Q37         ,   /*mute on 2 48mS A4 A5 */

/*54*/     tone_Q38         ,   /*single tone 48mS A5*/
/*55*/     tone_Q39         ,   /*shutdown tone 408mS A5 136mS A4 */
/*56*/     tone_Q3A         ,   /*phone ring 48mS C6 E6 C6 E6 C6 E6 C6 E6 C6 E6 C6 REST C6 E6 C6 E6 C6 E6 C6 E6 C6 E6 C6*/
/*57*/   tone_Q3B        ,   /*power up tone 83mS C5 10mS REST 83mS E5 10mS REST 83mS G5 10mS REST 124mS B5 498mS REST 372mS C6 */

/*58*/   tone_Q3C        ,   /*vol lev 0/7 */
/*59*/   tone_Q3D        ,   /*vol lev 6 */
/*5a*/   tone_Q3E        ,   /*vol lev 5 */
/*5b*/   tone_Q3F        ,   /*vol lev 4 */

/*5c*/   tone_Q40        ,   /*vol lev 3 */
/*5d*/   tone_Q41        ,   /*vol lev 2 */
/*5e*/   tone_Q42        ,    /*vol lev 1 */

} ;

#define NUM_FIXED_TONES (sizeof(gFixedTones) / sizeof(gFixedTones[0]))

/* Tone index 0 means no tone */
#define NO_TONE 0

/* Maximum number of user-defined tones that are supported. */
#define MAX_NUM_USER_TONES 8

/* Maximum length of a user-defined tone. This limit is to limit the maximum
   size of the config storage (e.g. ps key). It is not set by the hardware. */
#define MAX_USER_TONE_LENGTH 32


typedef struct __sink_tone_global_data_t
{
   sink_tone_writeable_config_def_t *event_tones;
   configtone_rdata_config_def_t *user_tones;

   uint16 num_tones;            /* The number of event->tone mappings in the tone config array. */
   uint16 user_tones_count;    /* The number of elements in the user tones array. */
}sink_tone_global_data_t;

/* global data */
static sink_tone_global_data_t gConfigTonesType;
#define TONES_GDATA gConfigTonesType

/****************************************************************************
NAME
      sinkToneGetNumTones

DESCRIPTION
     Get the number of tones configured

RETURNS
      void
*/
static uint16 sinkToneGetNumTones(void)
{
    return TONES_GDATA.num_tones;
}

/****************************************************************************
NAME
      sinkToneConfigInitEventTones

DESCRIPTION
      Configure an event tone only if one is defined

RETURNS
      void
*/
static void sinkToneConfigInitEventTones(void)
{
    uint16 size;

    size = configManagerGetReadOnlyConfig(SINK_TONE_WRITEABLE_CONFIG_BLK_ID, (const void **)&TONES_GDATA.event_tones);
    if(!size)
    {
        TONE_DEBUG(("TONES: sinkToneConfigInitEventTones():  Error In reading event tone type Config"));
        Panic();
    }

    TONES_GDATA.num_tones = ((size * sizeof(uint16)) / sizeof(TONES_GDATA.event_tones->gEventTones[0]));

    TONE_DEBUG(("INIT: Tones size %d and num_tones configured is %d \n", size, TONES_GDATA.num_tones));

    /* Do not release the config block on purpose because it is read at runtime. */
}

/****************************************************************************
NAME
      sinkToneConfigInitUserDefinedTones

DESCRIPTION
      Attempt to read the user configured tones, if data exists it will be in the following format:

    uint16 offset in array to user tone 1,
    uint16 offset in array to user tone ....,
    uint16 offset in array to user tone 8,
    uint16[] user tone data

    To play a particular tone it can be access via gVariableTones, e.g. to access tone 1

    TONES_GDATA->gVariableTones[0] + (uint16)*theSink.audioData.gConfigTones->gVariableTones[0]

    or to access tone 2

    TONES_GDATA->gVariableTones[0] + (uint16)*theSink.audioData.gConfigTones->gVariableTones[1]

    and so on

RETURNS
      void
*/
static void sinkToneConfigInitUserDefinedTones(void)
{
    uint16 size;

    size = configManagerGetReadOnlyConfig(CONFIGTONE_RDATA_CONFIG_BLK_ID, (const void **)&TONES_GDATA.user_tones);

    TONES_GDATA.user_tones_count = ((size * sizeof(uint16)) / sizeof(TONES_GDATA.user_tones->gVariableTones[0]));

    TONE_DEBUG(("INIT: User tones size %d\n", TONES_GDATA.user_tones_count));

    /* Do not release the config block on purpose because it is read at runtime. */
}

/****************************************************************************
NAME
 TonesPlayEvent

DESCRIPTION
 function to indaicate an event by playing its associated tone uses underlying
    tones playback

RETURNS
 void

*/
bool sinkTonesInit(void)
{
    memset(&TONES_GDATA, 0, sizeof(TONES_GDATA));

    sinkToneConfigInitEventTones();
    sinkToneConfigInitUserDefinedTones();

    return TRUE;
}

/****************************************************************************
NAME
   sinkTonesCanQueueEventTones

DESCRIPTION
   Function to get event tones.

RETURNS
   TRUE or FALSE

*/
bool sinkTonesCanQueueEventTones(void)
{
     bool ret_val = FALSE;
     sink_tone_readonly_config_def_t *tones_r_configdata = NULL;

     if (configManagerGetReadOnlyConfig(SINK_TONE_READONLY_CONFIG_BLK_ID, (const void **)&tones_r_configdata))
     {
        ret_val = tones_r_configdata->QueueEventTones;
        configManagerReleaseConfig(SINK_TONE_READONLY_CONFIG_BLK_ID);
     }
     return (ret_val) ? TRUE : FALSE;
}

/****************************************************************************
NAME
   sinkTonesCanQueueVolumeTones

DESCRIPTION
   Function to get volume tones.

RETURNS
   TRUE or FALSE

*/
bool sinkTonesCanQueueVolumeTones(void)
{
     bool ret_val = FALSE;
     sink_tone_readonly_config_def_t *tones_r_configdata = NULL;

     if (configManagerGetReadOnlyConfig(SINK_TONE_READONLY_CONFIG_BLK_ID, (const void **)&tones_r_configdata))
     {
        ret_val = tones_r_configdata->QueueVolumeTones;
        configManagerReleaseConfig(SINK_TONE_READONLY_CONFIG_BLK_ID);
     }
     return (ret_val) ? TRUE : FALSE;
}
/****************************************************************************
NAME
   sinkTonesIsPlayLocalVolumeTone

DESCRIPTION
   Function to get PlayLocalVolumeTone
RETURNS
   TRUE or FALSE

*/
bool sinkTonesIsPlayLocalVolumeTone(void)
{
    bool ret_val = FALSE;
    sink_tone_readonly_config_def_t *tones_r_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINK_TONE_READONLY_CONFIG_BLK_ID, (const void **)&tones_r_configdata))
    {
      ret_val = tones_r_configdata->PlayLocalVolumeTone;
      configManagerReleaseConfig(SINK_TONE_READONLY_CONFIG_BLK_ID);
    }
    return (ret_val) ? TRUE : FALSE;
}
/****************************************************************************
NAME
   sinkTonesIsPlayTonesAtFixedVolume

DESCRIPTION
   Function to get PlayTonesAtFixedVolume
RETURNS
   TRUE or FALSE

*/
bool sinkTonesIsPlayTonesAtFixedVolume(void)
{
    bool ret_val = FALSE;
    sink_tone_readonly_config_def_t *tones_r_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINK_TONE_READONLY_CONFIG_BLK_ID, (const void **)&tones_r_configdata))
    {
      ret_val = tones_r_configdata->PlayTonesAtFixedVolume;
      configManagerReleaseConfig(SINK_TONE_READONLY_CONFIG_BLK_ID);
    }
    return (ret_val) ? TRUE : FALSE;
}

/****************************************************************************
NAME
   sinkTonesGetFixedToneVolumeLevel

DESCRIPTION
   Function to get fixed tone volume level.

RETURNS
   uint16

*/
uint16 sinkTonesGetFixedToneVolumeLevel(void)
{
    uint16 ret_val = 0;
    sink_tone_readonly_config_def_t *tones_r_configdata = NULL;

    if (configManagerGetReadOnlyConfig(SINK_TONE_READONLY_CONFIG_BLK_ID, (const void **)&tones_r_configdata))
    {
        ret_val = tones_r_configdata->FixedToneVolumeLevel;
        configManagerReleaseConfig(SINK_TONE_READONLY_CONFIG_BLK_ID);
    }

    return ret_val;
}

bool sinkTonesSetFixedToneVolumeLevel(uint16 volume_level)
{
    sink_tone_readonly_config_def_t *tones_r_configdata = NULL;

    if (configManagerGetWriteableConfig(SINK_TONE_READONLY_CONFIG_BLK_ID, (void **)&tones_r_configdata, 0))
    {
        tones_r_configdata->FixedToneVolumeLevel = volume_level;
        configManagerUpdateWriteableConfig(SINK_TONE_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
DESCRIPTION
    Store tone volume settings for use by audio plugins
*/
void TonesInitSettings(bool fixed_volume, unsigned fixed_volume_level)
{
    AudioConfigSetTonesAreFixedVolume(fixed_volume);
    AudioConfigSetToneVolume(VolumeConvertDACGainToDB(fixed_volume_level));
}

/****************************************************************************
NAME
    TonesGetToneVolume

DESCRIPTION
      Works out the correct volume to play tones or Audio Prompts at

RETURNS
    void
*/
uint16 TonesGetToneVolume(void)
{
    uint16 volumeIndex;
  
    TONE_DEBUG(("TONE: PlayTonesAtFixedVolume : [%d]\n",sinkTonesIsPlayTonesAtFixedVolume()));
    /* if play at fixed volume selected */
    if (sinkTonesIsPlayTonesAtFixedVolume())
    {
        /* volume level is already in raw gain units, no need to use volume mapping table */
        volumeIndex = sinkTonesGetFixedToneVolumeLevel() ;
    }
    /* check for play tone at default level */
    else
    {
        /* default volume level unless changed by other options */
        volumeIndex = sinkHfpDataGetDefaultVolume() ;

        switch(sinkAudioRouteAvailable())
        {
            case audio_route_a2dp_primary:
                volumeIndex = SinkA2dpGetMainVolume(a2dp_primary);
                TONE_DEBUG(("TONE: GetVolume : A2DP primary\n"));
                break;
            case audio_route_a2dp_secondary:
                volumeIndex = SinkA2dpGetMainVolume(a2dp_secondary);
                TONE_DEBUG(("TONE: GetVolume : A2DP secondary\n"));
                break;
            case audio_route_hfp_primary:
                TONE_DEBUG(("TONE: GetVolume : HFP primary\n"));
                volumeIndex = sinkHfpDataGetAudioSMVolumeLevel(PROFILE_INDEX(hfp_primary_link));
                volumeIndex =  sinkVolumeGetVolumeMappingforCVC(volumeIndex);
                break;
            case audio_route_hfp_secondary:
                TONE_DEBUG(("TONE: GetVolume : HFP secondary\n"));
                volumeIndex = sinkHfpDataGetAudioSMVolumeLevel(PROFILE_INDEX(hfp_secondary_link));
                volumeIndex =  sinkVolumeGetVolumeMappingforCVC(volumeIndex);
                break;
            case audio_route_usb:
                volumeIndex = sinkUsbGetUsbVolumeIndex();
                TONE_DEBUG(("TONE: GetVolume : USB\n"));
             break;
            case audio_route_analog:
                TONE_DEBUG(("TONE: GetVolume : Analog\n"));
                volumeIndex = SinkWiredGetAnalogMainVolume();
                break;
            case audio_route_i2s:
                TONE_DEBUG(("TONE: GetVolume : I2S\n"));
                volumeIndex = SinkWiredGetI2sMainVolume();
                break;
            case audio_route_spdif:
                TONE_DEBUG(("TONE: GetVolume : Spdif\n"));
                volumeIndex = SinkWiredGetSpdifMainVolume();
                break;
            case audio_route_fm:
                TONE_DEBUG(("TONE: GetVolume : FM\n"));
                volumeIndex = sinkFmGetFmMainVolume();
                break;
           case audio_route_none:
           default:
                TONE_DEBUG(("TONE: GetVolume : nothing routed\n"));
                {
                    /* Get the link to change volume on */
                    hfp_link_priority priority = audioGetLinkPriority(TRUE);
                    /* Get current volume for this profile */
                    uint16 lOldVol = sinkHfpDataGetAudioSMVolumeLevel(PROFILE_INDEX(priority));
                    /* Obtain current volume level */
                    volumeIndex  = sinkVolumeGetVolumeMappingforCVC(lOldVol);
                }
                break;
        }
    
    }
    TONE_DEBUG(("TONE: GetVolume [%d]\n", volumeIndex));
    return volumeIndex;
}

/****************************************************************************
NAME
    TonesPlayTone

DESCRIPTION
      Plays back the tone given by the ringtone_note index

RETURNS
    void
*/
void TonesPlayTone(uint16 pTone, bool pCanQueue)
{
    int16 tone_volume = USE_CURRENT_TONE_VOLUME;

    TONE_DEBUG(("TONE Play sinth [%x]\n", pTone));

    /* ensure tone is valid (non zero) before playing */
    if(pTone == NO_TONE)
        return;

    /* Check if tone is within fixed tone range */
    if(pTone <= NUM_FIXED_TONES)
    {
       AudioPlayTone ( gFixedTones [ pTone  - 1 ],
                       pCanQueue,
                       tone_volume,
                       sinkAudioGetPluginFeatures()) ;
    }
    /* if the tone index is beyond that of the fixed tones, check for the prescence of
       a user defined tone, there are 8 user defineable tones available at the end of the
       fixed tones list */
    else
    {
        TONE_DEBUG(("TONE Play sinth which is larger than 5e is [%x] - [%d]\n", pTone, pTone-1-NUM_FIXED_TONES));

         /* check to see if there are any configured user defined tones and then check to see
            if there is a tone available at the index (0 to 7) requested */
        if((TONES_GDATA.user_tones) && (&TONES_GDATA.user_tones->gVariableTones[0] &&
           TONES_GDATA.user_tones->gVariableTones[pTone-1-NUM_FIXED_TONES]))
        {

            /* audio tone is located at 'start of data + an offset' into the array of data,
               the first 8 words of data in gVariableTones are offsets into the data array
               for user tones 0 to 7 */
            AudioPlayTone ( (const ringtone_note *)(&TONES_GDATA.user_tones->gVariableTones[0] +
                                                 (uint16)(unsigned)TONES_GDATA.user_tones->gVariableTones[pTone-1-NUM_FIXED_TONES]),
                            pCanQueue,
                            tone_volume,
                            sinkAudioGetPluginFeatures()) ;

        }
    }
}

/****************************************************************************
NAME
    ToneTerminate

DESCRIPTION
      function to terminate a ring tone prematurely.

RETURNS

*/
void ToneTerminate ( void )
{
    AudioPromptCancelNumString();
    AudioStopToneAndPrompt(!SinkAudioPromptsGetDisableAudioPromptTerminate());
}

/****************************************************************************
NAME
    TonesGetToneVolumeInDb

DESCRIPTION
    Works out the correct volume to play tones or Audio Prompts at in dB/60

RETURNS
    Volume in dB/60
*/
int16 TonesGetToneVolumeInDb(audio_output_group_t group)
{
    int16  tone_steps = 0;
    uint16 tone_system = TonesGetToneVolume();
    volume_group_config_t gain_utils_vol;

    sinkVolumeGetGroupConfig(group, &gain_utils_vol);

    tone_steps = (tone_system * gain_utils_vol.no_of_steps) / VOLUME_NUM_VOICE_STEPS;

    return VolumeConvertStepsToDB(tone_steps, &gain_utils_vol);
}


/****************************************************************************
NAME    
    IsToneConfiguredForEvent
    
DESCRIPTION
    Checks if audio tone is configured for the given sink event.
 
PARAMETERS
    sink_event    - incoming sink event

RETURNS
    TRUE if audio tone is configured for the sink event
    FALSE if audio tone is not configured for the sink event
*/
bool IsToneConfiguredForEvent(sinkEvents_t sink_event)
{
    uint16 tone_count;
    tone_config_type_t *tone_config;
    uint16 i = 0 ;

    tone_count = sinkToneGetNumTones();
    for (i = 0; i < tone_count; i++)
    {
        tone_config = &TONES_GDATA.event_tones->gEventTones[i];

        if (tone_config->event == sink_event)
        {
            /* Tone config found */
            return TRUE;
        }
    }
    return FALSE;
}

/****************************************************************************
NAME
    sinkTonesGetToneForEvent

DESCRIPTION
    Helper function to lookup the tone config for the given sink event.

PARAMS
    event        Sink event to get teh tone config for.
    tone_config  Pointer to where matched tone config struct will be placed.

RETURNS
    TRUE if matching tone config found, FALSE otherwise.
*/
static bool sinkTonesGetToneForEvent(sinkEvents_t event, tone_config_type_t *tone_config)
{
    uint16 i;

    for (i = 0; i < TONES_GDATA.num_tones; i++)
    {
        if (TONES_GDATA.event_tones->gEventTones[i].event == event)
        {
            *tone_config = TONES_GDATA.event_tones->gEventTones[i];
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************
NAME    
    AudioTonePlayEvent
    
DESCRIPTION
    Plyas audio tone attached to the incoming sink event.
 
PARAMETERS
    sink_event           - incoming sink event
    event_can_be_queued  - Flag which indicate if this event
                           is to be played imidiately or not

RETURNS
    void
*/
void AudioTonePlayEvent(sinkEvents_t sink_event, bool event_can_be_queued)
{
    uint16 tone_count;
    tone_config_type_t *tone_config;
    uint16 i = 0 ;

    tone_count = sinkToneGetNumTones();
    for (i = 0; i < tone_count; i++)
    {
        tone_config = &TONES_GDATA.event_tones->gEventTones[i];

        if (tone_config->event == sink_event )
        {
            /* turn on audio amp */
             enableAudioActivePio();
            /* check event as tone queueing not allowed on mute and ring tones */
            if(event_can_be_queued == FALSE)
            {
                /* check whether to play mute reminder tone at default volume level, never queue mute reminders to
                   protect against the case that the tone is longer than the mute reminder timer */
                TonesPlayTone (tone_config->tone, event_can_be_queued);
            }
            else
            {
                TonesPlayTone (tone_config->tone, event_can_be_queued );
            }

            /* turn amp off if audio is inactive */
            disableAudioActivePioWhenAudioNotBusy();

        }
    }
}

bool SinkTonesGetToneConfig(uint16 event, tone_config_type *tone_config)
{
    tone_config_type_t config;

    if (sinkTonesGetToneForEvent(event, &config))
    {
        tone_config->event = event;
        tone_config->tone = config.tone;
        return TRUE;
    }

    return FALSE;
}

bool SinkTonesSetToneConfig(const tone_config_type *tone_config)
{
    uint16 i;
    bool ret = FALSE;
    uint16 count = TONES_GDATA.num_tones;

    /* Try to find index of event in current config block */
    for (i = 0; i < count; i++)
    {
        if (TONES_GDATA.event_tones->gEventTones[i].event == tone_config->event)
            break;
    }

    /* If not found, increase the number of config items. */
    if (i == count)
        count++;

    /* Re-open the config block as writeable (and one element bigger if necessary). */
    configManagerReleaseConfig(SINK_TONE_WRITEABLE_CONFIG_BLK_ID);
    if (configManagerGetWriteableConfig(SINK_TONE_WRITEABLE_CONFIG_BLK_ID, (void **)&TONES_GDATA.event_tones, (count * sizeof(tone_config_type_t))))
    {
        /* Write new config at the index found earlier and store in config_store. */
        TONES_GDATA.event_tones->gEventTones[i].event = tone_config->event;
        TONES_GDATA.event_tones->gEventTones[i].tone = tone_config->tone;
        configManagerUpdateWriteableConfig(SINK_TONE_WRITEABLE_CONFIG_BLK_ID);
        ret = TRUE;
    }

    /* Re-initialise the tones config for this module. */
    sinkToneConfigInitEventTones();

    return ret;
}

/******************************************************************************
NAME
    sinkTonesGetUserToneLength

DESCRIPTION
    Calculate the length of a user-defined tone by parsing it until
    RINGTONE_END is reached.

    The returned length includes the final element with RINGTONE_END in it.

    The given tone is invalid if a RINGTONE_END is not found before
    MAX_USER_TONE_LENGTH elements have been parsed.

PARAMS
    tone Tone to calculate the length of.

RETURNS
    The number of ringtone_note elements in the tone including the final
    RINGTONE_END, or 0 if it the tone is invalid.
*/
static uint16 sinkTonesGetToneLength(const ringtone_note *tone)
{
    uint16 *note;
    uint16 length = 0;
    
    if (!tone)
        return 0;

    note = (uint16 *)tone;

    while (*note++ != (unsigned)RINGTONE_END)
    {
        length++;

        if (length > MAX_USER_TONE_LENGTH)
        {
            /* Tone is too long, or perhaps doesn't have a RINGTONE_END. */
            return 0;
        }
    }

    /* Add 1 on for the final RINGTONE_END */
    return (length + 1);
}

bool SinkTonesGetUserTone(uint16 id, const ringtone_note **tone, uint16 *note_count)
{
    uint16 offset;
    uint16 count = 0;
    uint16 *note;
    uint16 *end;
    const ringtone_note *config_tone;


    if ((id > MAX_NUM_USER_TONES) || (!TONES_GDATA.user_tones))
        return FALSE;

    offset = TONES_GDATA.user_tones->gVariableTones[id];

    /* if the offset is 0 then this user-defined tone does not exist. */
    if (offset == 0)
        return FALSE;

    config_tone = (const ringtone_note *)(&TONES_GDATA.user_tones->gVariableTones[offset]);

    /* Calculate the number of notes in the tone, excluding the initial
       tempo, volume, timbre and decay values. */
    note = (uint16 *)config_tone;
    end = (uint16 *)(&TONES_GDATA.user_tones->gVariableTones[TONES_GDATA.user_tones_count]);
    while (*note != (unsigned)RINGTONE_END)
    {
        if ((*note & RINGTONE_SEQ_CONTROL_MASK) == 0)
            count++;

        note++;

        /* Check we haven't gone past the end of the user-defined tones array. */
        if (note >= end)
        {
            return FALSE;
        }
    }
    
    *tone = config_tone;
    *note_count = count;
    
    return TRUE;
}


bool SinkTonesSetUserTone(uint16 id, const ringtone_note *tone, uint16 len)
{
    uint16 i;
    uint16 index;
    uint16 size;
    uint16 src_len = 0;
    uint16 *src;
    uint16 *buffer;
    uint16 offset;

    if (id >= MAX_NUM_USER_TONES)
        return FALSE;

    /* Calculate the required size for the new config array. */
    if (TONES_GDATA.user_tones_count < MAX_NUM_USER_TONES)
    {
        /* No user tones currently allocated, so allow space for the 
           offset table plus the new user tone. */
        size = (MAX_NUM_USER_TONES + len);
    }
    else
    {
        index = TONES_GDATA.user_tones->gVariableTones[id];
        if (index)
        {
            src = (uint16 *)&TONES_GDATA.user_tones->gVariableTones[index];
            src_len = sinkTonesGetToneLength((const ringtone_note*)src);
        }
        size = (TONES_GDATA.user_tones_count - src_len + len);
    }

    /* Allocate a temporary buffer to work with. */
    buffer = mallocZDebugNoPanic(size * sizeof(uint16));
    if (!buffer)
        return FALSE;

    /* Copy the contents from the existing config data and insert the new
       user-defined tone when we get to it. */
    offset = MAX_NUM_USER_TONES;
    for (i = 0; i < MAX_NUM_USER_TONES; i++)
    {
        if (id == i)
        {
            src = (uint16 *)tone;
            src_len = len;
        }
        else
        {
            /* There may be no user tones in the config, in which case
               TONES_GDATA.user_tones->gVariableTones will be NULL. */
            if (TONES_GDATA.user_tones->gVariableTones)
                index = TONES_GDATA.user_tones->gVariableTones[i];
            else
                index = 0;
            
            if (index)
            {
                src = (uint16 *)&TONES_GDATA.user_tones->gVariableTones[index];
                src_len = sinkTonesGetToneLength((const ringtone_note*)src);
            }
            else
            {
                src = NULL;
                src_len = 0;
            }
        }
 
        if (src)
        {
            buffer[i] = offset;
            memcpy(&buffer[offset], src, (src_len * sizeof(uint16)));
            offset += src_len;
        }
        else
        {
            buffer[i] = 0;
        }
    }

    /* Copy the modified user tones data into the config block. We need to
       re-open the config as writeable, and possibly a different size, to do this. */
    configManagerReleaseConfig(CONFIGTONE_RDATA_CONFIG_BLK_ID);
    configManagerGetWriteableConfig(CONFIGTONE_RDATA_CONFIG_BLK_ID, (void **)&TONES_GDATA.user_tones, size);

    memcpy(&TONES_GDATA.user_tones->gVariableTones[0], buffer, size);
    TONES_GDATA.user_tones_count = (size / sizeof(TONES_GDATA.user_tones->gVariableTones[0]));

    ConfigStoreWriteConfig(CONFIGTONE_RDATA_CONFIG_BLK_ID);

    free(buffer);
    
    return TRUE;
}

uint16 SinkTonesUserToneNumMax(void)
{
    return MAX_NUM_USER_TONES;
}
