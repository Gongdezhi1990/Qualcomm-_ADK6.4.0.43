/****************************************************************************

        Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.

FILE
        ringtone_notes.h 

DESCRIPTION
        Ringtone note macros

*/

#ifndef __APP_RINGTONE_NOTE_H__
#define __APP_RINGTONE_NOTE_H__
 

/*!
    @file ringtone_notes.h

    @brief The ringtone note macros.

The valid notes that can be used with the RINGTONE_NOTE() and
RINGTONE_NOTE_TIE() macros are defined with names like
#RINGTONE_NOTE_C5. The macros take just the note name, for example C5.

The note names consist of the note (A-G) followed by an optional sharp
or flat (S or F) followed by an octave number (0-9). C5 is middle C.
Octaves start at C that is, the note below C5 is B4. At the moment,
only black notes have the sharp or flat reference, that is FS5 is a
valid note, but FF5 isn't (use E5).

The frequency output for note n is given by f = 440 * 2^((n-69)/12).
That is, note 69 in MIDI terms, the A above middle C, is defined to be
440 Hz. Each MIDI note corresponds to one semi-tone. There are 12
semitones in an octave. An octave jump represents a doubling of
frequency. The ringtone library uses an even tempered scale (the ratio
of frequencies of adjacent notes is the same.)

A rest (silence) can be inserted with #RINGTONE_NOTE_REST.

The valid durations that can be used with the RINGTONE_NOTE() and
RINGTONE_NOTE_TIE() macros are specified with names like
#RINGTONE_NOTE_CROTCHET. Similarly, the macros take just the duration
name, for example CROTCHET.
*/

/*!
   \name Note defines

   First argument to RINGTONE_NOTE() and RINGTONE_NOTE_TIE().
 */
/*! \{ */
#define RINGTONE_NOTE_C0   (0<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS0  (1<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF0  (1<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D0   (2<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS0  (3<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF0  (3<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E0   (4<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F0   (5<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS0  (6<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF0  (6<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G0   (7<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS0  (8<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF0  (8<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A0   (9<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS0  (10<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF0  (10<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B0   (11<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C1   (12<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS1  (13<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF1  (13<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D1   (14<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS1  (15<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF1  (15<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E1   (16<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F1   (17<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS1  (18<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF1  (18<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G1   (19<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS1  (20<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF1  (20<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A1   (21<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS1  (22<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF1  (22<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B1   (23<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C2   (24<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS2  (25<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF2  (25<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D2   (26<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS2  (27<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF2  (27<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E2   (28<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F2   (29<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS2  (30<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF2  (30<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G2   (31<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS2  (32<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF2  (32<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A2   (33<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS2  (34<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF2  (34<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B2   (35<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C3   (36<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS3  (37<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF3  (37<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D3   (38<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS3  (39<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF3  (39<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E3   (40<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F3   (41<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS3  (42<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF3  (42<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G3   (43<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS3  (44<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF3  (44<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A3   (45<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS3  (46<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF3  (46<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B3   (47<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C4   (48<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS4  (49<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF4  (49<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D4   (50<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS4  (51<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF4  (51<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E4   (52<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F4   (53<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS4  (54<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF4  (54<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G4   (55<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS4  (56<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF4  (56<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A4   (57<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS4  (58<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF4  (58<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B4   (59<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C5   (60<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS5  (61<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF5  (61<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D5   (62<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS5  (63<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF5  (63<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E5   (64<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F5   (65<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS5  (66<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF5  (66<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G5   (67<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS5  (68<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF5  (68<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A5   (69<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS5  (70<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF5  (70<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B5   (71<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C6   (72<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS6  (73<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF6  (73<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D6   (74<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS6  (75<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF6  (75<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E6   (76<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F6   (77<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS6  (78<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF6  (78<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G6   (79<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS6  (80<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF6  (80<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A6   (81<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS6  (82<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF6  (82<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B6   (83<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C7   (84<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS7  (85<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF7  (85<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D7   (86<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS7  (87<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF7  (87<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E7   (88<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F7   (89<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS7  (90<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF7  (90<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G7   (91<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS7  (92<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF7  (92<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A7   (93<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS7  (94<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF7  (94<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B7   (95<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C8   (96<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS8  (97<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF8  (97<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D8   (98<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS8  (99<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF8  (99<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E8   (100<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F8   (101<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS8  (102<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF8  (102<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G8   (103<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS8  (104<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF8  (104<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A8   (105<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS8  (106<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF8  (106<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B8   (107<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_C9   (108<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_CS9  (109<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DF9  (109<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_D9   (110<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_DS9  (111<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_EF9  (111<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_E9   (112<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_F9   (113<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_FS9  (114<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GF9  (114<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_G9   (115<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_GS9  (116<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AF9  (116<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_A9   (117<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_AS9  (118<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_BF9  (118<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
#define RINGTONE_NOTE_B9   (119<<RINGTONE_SEQ_NOTE_PITCH_POS)        /*!< Used to set note pitch. */
/* We hit 8 kHz here so it's pointless to put any more notes in */
/*! \} */


/*!
    Insert a rest rather than a note.
*/
#define REST_NOTE_PITCH     127
#define RINGTONE_NOTE_REST  (REST_NOTE_PITCH<<RINGTONE_SEQ_NOTE_PITCH_POS)

/*!
   \name British note duration names

   Second argument to RINGTONE_NOTE() and RINGTONE_NOTE_TIE().
*/
/*! \{ */
#define RINGTONE_NOTE_SEMIBREVE                  ( 1<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_MINIM                      ( 2<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_CROTCHET                   ( 4<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_QUAVER                     ( 8<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_SEMIQUAVER                 (16<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_DEMISEMIQUAVER             (32<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_HEMIDEMISEMIQUAVER         (64<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */

#define RINGTONE_NOTE_MINIM_TRIPLET              ( 3<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_CROTCHET_TRIPLET           ( 6<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_QUAVER_TRIPLET             (12<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_SEMIQUAVER_TRIPLET         (24<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_DEMISEMIQUAVER_TRIPLET     (48<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_HEMIDEMISEMIQUAVER_TRIPLET (96<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
/*! \} */


/*!
   \name American note duration names

   Second argument to RINGTONE_NOTE() and RINGTONE_NOTE_TIE().
*/
/*! \{ */
#define RINGTONE_NOTE_WHOLENOTE                  ( 1<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_HALFNOTE                   ( 2<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_QUARTERNOTE                ( 4<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_EIGHTHNOTE                 ( 8<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_SIXTEENTHNOTE              (16<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_THIRTYSECONDNOTE           (32<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_SIXTYFOURTHNOTE            (64<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */

#define RINGTONE_NOTE_HALFNOTE_TRIPLET           ( 3<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_QUARTERNOTE_TRIPLET        ( 6<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_EIGHTHNOTE_TRIPLET         (12<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_SIXTEENTHNOTE_TRIPLET      (24<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_THIRTYSECONDNOTE_TRIPLET   (48<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
#define RINGTONE_NOTE_SIXTYFOURTHNOTE_TRIPLET    (96<<RINGTONE_SEQ_NOTE_LENGTH_POS)  /*!< Used to set note duration. */
/*! \} */

/*!
        Constructs a #ringtone_note that plays note \e n for duration
        \e d. For example:

        \code
    RINGTONE_NOTE(C5, CROTCHET)     Play a middle C
        \endcode

        The available notes and durations are defined in this file.
*/
#define RINGTONE_NOTE(n, d) \
    ((ringtone_note) (RINGTONE_SEQ_NOTE | RINGTONE_NOTE_##n | RINGTONE_NOTE_##d))

/*!
    The RINGTONE_NOTE_TIE macro is very similar to the #RINGTONE_NOTE macro, but it
    plays 'tied' notes.  This means that the pitch of the note apears to change
    (as opposed to a new note starting), and the volume continues to decay from the
    previous note. Please read the description of #RINGTONE_DECAY for important
    information about using tied notes.
 */
#define RINGTONE_NOTE_TIE(n, d) RINGTONE_NOTE(n | RINGTONE_SEQ_NOTE_TIE_MASK, d)


#endif    /* __APP_RINGTONE_NOTE_H__ */
