/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

/* CSR WARNING: DON'T DO ANYTHING TOO CRAZY TO THIS FILE */
/* it has to be parsed by devHost/kalimba/Makefile as well as the C compiler */

/*!
   @file kalimba_standard_messages.h

   @brief
      The messages passed between the kalimba libraries and the VM
      application.

      Messages passed between MCU and kalimba consist of a 16-bit ID and up to
      four 16-bit arguments.

      If the top-bit is set (0x8000 - 0xFFFF) the message was sent by,
      or is routed to, the BlueCore firmware. Those messages are
      defined in kalimba_messsages.h, which is supplied as a kalimba
      include file but not as a VM include file (since no such
      messages will reach the VM code.)

      kalimba_standard_messages.h lists the standard messages which
      pass between kalimba and the VM application. Those are typically
      sent by the CSR supplied DSP libraries or sample VM
      applications. All messages used here are in the range 0x7000 -
      0x7FFF.  Any customer specific messages must use numbers less
      than this.

      In the descriptions of each message, the four arguments are
      referred to by number (0..3) after their name, for example
      period (1) means that the period parameter is the second of the
      four parameters,
*/

#ifndef KALIMBA_STANDARD_MESSAGES_INCLUDED
#define KALIMBA_STANDARD_MESSAGES_INCLUDED


/* ****************************************************************************
// ****************************************************************************
//                      KALIMBA <--> VM APPLICATION MESSAGES
// ****************************************************************************
// **************************************************************************** */


/*!
  MESSAGE:  Go     (VM APPLICATION --> KALIMBA)

  @brief
   Indicates that the VM application has done all of its streamconnects
   and hence the dsp can start reading/writing data to its ports.
*/
#define   KALIMBA_MSG_GO                         0x7000

/*!
  MESSAGE:  SBC decoder convert to mono     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the sbc decoder to convert stereo streams to mono.

  @param mono (0), 1 means convert to mono, 0 means leave as stereo
*/
#define   KALIMBA_MSG_SBCDEC_CONVERT_TO_MONO     0x7010

/*!
  MESSAGE:  SBC encoder set bitpool     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the sbc encoder what bitpool value to encode using and hence
   the dsp can start reading/writing data to its ports.

  @param bitpool (0), the bitpool to use (valid range is 2..250.)
*/
#define   KALIMBA_MSG_SBCENC_SET_BITPOOL         0x7020

/*!
  MESSAGE:  SBC encoder set parameters     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the sbc encoder what encoder parameters to use.

  @param param (0)
       the format is as follows (see SBC spec)
                  bit 8:    force word (16 bit) aligned packets
                  bit 6-7:  sampling frequency,
                  bit 4-5:  blocks,
                  bit 2-3:  channel_mode,
                  bit 1:    allocation method,
                  bit 0:    subbands
*/
#define   KALIMBA_MSG_SBCENC_SET_PARAMS          0x7021

/*!
  MESSAGE:  MP3 decoder convert to mono     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the mp3 decoder to convert stereo streams to mono.

  @param mono (0), 1 means convert to mono, 0 means leave as stereo
*/
#define   KALIMBA_MSG_MP3DEC_CONVERT_TO_MONO     0x7030

/*!
  MESSAGE:  AAC decoder convert to mono     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the aac decoder to convert stereo streams to mono.

  @param mono (0), 1 means convert to mono, 0 means leave as stereo
*/
#define   KALIMBA_MSG_AACDEC_CONVERT_TO_MONO     0x7040


/*!
  MESSAGE:  AAC decoder set file type     (VM APPLICATION --> KALIMBA)

  @brief
   Tells the aac decoder the type of file it is being sent

  @param file_type (0), 0 means mp4, 1 adts, 2 latm
*/
#define   KALIMBA_MSG_AACDEC_SET_FILE_TYPE       0x7041


/*!
  MESSAGE:  Cook decoder set parameters   (VM APPLICATION --> KALIMBA)

  @brief Tells the cook decoder the parameters it needs to be able to decode

  @param param (0), NumRegions(bits0-5), NumChannels(bits8:9), NumSamples/256(bits10:12)
  @param frame_bits (1), value of FrameBits
  @param start_region (2), value of StartRegion
  @param quat_bits (3), value of QuantBits
*/
#define   KALIMBA_MSG_COOKDEC_SET_PARAMS         0x7050


/*!
  MESSAGE:  Cook decoder set sample rate  (VM APPLICATION --> KALIMBA)

  @brief Tells the cook decoder the sample rate of the current stream

  @param sample_rate (0), Sample Rate (in Hz - unsigned 16bits)
*/
#define   KALIMBA_MSG_COOKDEC_SAMPLE_RATE        0x7051


/*!
  MESSAGE:  Clock mismatch rate on Source device     (KALIMBA --> VM APPLICATION)

  @brief
   Tells the VM the clock mismatch rate for a Source device so an application can store the rate per device.

  @param clock mismatch rate (0)
*/
#define KALIMBA_MSG_SOURCE_CLOCK_MISMATCH_RATE   0x7070


/*!
  MESSAGE:  Pulse LED control    (VM APPLICATION --> KALIMBA)

  @brief
   Tells the pulse_led libary which PIO to pulse and at what rate

  @param mask (0), the PIO mask to use (that is, which PIO lines to modify)
  @param period (1), the period of the pulsing (in units of 0.35 secs)
*/
#define   KALIMBA_MSG_PULSE_LED                  0x7100


/*!
  MESSAGE:  Play new dtmf tone                    (VM APPLICATION --> KALIMBA)

  @brief
    Tells the dtmf library to play a new dtmf tone.

  @param tone to play (0)
  @param duration of tone in samples (1)
  @param duration of silence following the tone in ms (2)
*/
#define    KALIMBA_MSG_DTMF_NEW_TONE_MESSAGE_ID   0x7150


/*!
  MESSAGE:  Silence and clip detect initialise channel
                                                 (VM APPLICATION --> KALIMBA)

  @brief
    Tells the silence and clip detect operator to initalise an instance
    (structure) with silence and clipping limits.

  @param instance number (0)
  @param duration of silence in seconds (1) - period of silence before sending message
  @param clip limit (2)                     - signal level required to indicate clipping
*/
#define   KALIMBA_MSG_CBOPS_SILENCE_CLIP_DETECT_INITIALISE_ID        0x7200



/*!
  MESSAGE:  Silence and clip detect clipping detected
                                                 (KALIMBA --> VM APPLICATION)

  @brief
    Message to the VM to indicate clipping has been detected

*/
#define   KALIMBA_MSG_CBOPS_SILENCE_CLIP_DETECT_CLIP_DETECTED_ID     0x7201


/*!
  MESSAGE:  Silence and clip detect silence detected
                                                 (KALIMBA --> VM APPLICATION)

  @brief
    Message to the VM to indicate silence has been detected

*/
#define   KALIMBA_MSG_CBOPS_SILENCE_CLIP_DETECT_SILENCE_DETECTED_ID  0x7202


/*!
  MESSAGE:  Graphix driver control
                                                 (VM APPLICATION --> KALIMBA)

  @brief
    Tells the DSP graphic driver library to do something.

*/
#define   KALIMBA_MSG_GRAPHIX_DRIVER_FROM_VM_MSG_ID                  0x7400

#endif
