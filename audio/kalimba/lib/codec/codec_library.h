// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef CODEC_STREAM_HEADER_INCLUDED
#define CODEC_STREAM_HEADER_INCLUDED

#include "core_library.h"
#include "codec_constants.h"

// pull in selected stuff only if Kymera build
#ifdef KYMERA
#include "cbuffer_asm.h"
#endif

#if defined(PATCH_LIBS)
   #include "patch_library.h"
#endif

   // general frame encoder/decode constants
   .CONST        $codec.NORMAL_DECODE                                         CODEC_NORMAL_DECODE;
   .CONST        $codec.NO_OUTPUT_DECODE                                      CODEC_NO_OUTPUT;
   .CONST        $codec.GOBBLE_DECODE                                         CODEC_GOBBLE;

   .CONST        $codec.SUCCESS                                               CODEC_SUCCESS;
   .CONST        $codec.NOT_ENOUGH_INPUT_DATA                                 CODEC_NOT_ENOUGH_INPUT_DATA;
   .CONST        $codec.NOT_ENOUGH_OUTPUT_SPACE                               CODEC_NOT_ENOUGH_OUTPUT_SPACE;
   .CONST        $codec.ERROR                                                 CODEC_ERROR;
   .CONST        $codec.EOF                                                   CODEC_EOF;
   .CONST        $codec.FRAME_CORRUPT                                         CODEC_FRAME_CORRUPT;

   .CONST        $codec.ENCODER_OUT_BUFFER_FIELD                              0*ADDR_PER_WORD;
   .CONST        $codec.ENCODER_IN_LEFT_BUFFER_FIELD                          1*ADDR_PER_WORD;
   .CONST        $codec.ENCODER_IN_RIGHT_BUFFER_FIELD                         2*ADDR_PER_WORD;
   .CONST        $codec.ENCODER_MODE_FIELD                                    3*ADDR_PER_WORD;
   .CONST        $codec.ENCODER_DATA_OBJECT_FIELD                             4*ADDR_PER_WORD;
   .CONST        $codec.ENCODER_STRUC_SIZE                                    5;

   .CONST        $codec.DECODER_IN_BUFFER_FIELD                               0*ADDR_PER_WORD;
   .CONST        $codec.DECODER_OUT_LEFT_BUFFER_FIELD                         1*ADDR_PER_WORD;
   .CONST        $codec.DECODER_OUT_RIGHT_BUFFER_FIELD                        2*ADDR_PER_WORD;
   .CONST        $codec.DECODER_MODE_FIELD                                    3*ADDR_PER_WORD;
   .CONST        $codec.DECODER_NUM_OUTPUT_SAMPLES_FIELD                      4*ADDR_PER_WORD;
   .CONST        $codec.DECODER_NUM_INPUT_OCTETS_CONSUMED_FIELD               5*ADDR_PER_WORD;
   .CONST        $codec.DECODER_DATA_OBJECT_FIELD                             6*ADDR_PER_WORD;
   .CONST        $codec.DECODER_STRUC_SIZE                                    7;

   .CONST        $codec.DECRYPTER_IN_BUFFER_FIELD                             0*ADDR_PER_WORD;
   .CONST        $codec.DECRYPTER_OUT_BUFFER_FIELD                            1*ADDR_PER_WORD;
   .CONST        $codec.DECRYPTER_EXTRA_STRUC_PTR_FIELD                       2*ADDR_PER_WORD;
   .CONST        $codec.DECRYPTER_STRUC_SIZE                                  3;

   .CONST        $codec.STREAM_CAN_IDLE                                       0;
   .CONST        $codec.STREAM_DONT_IDLE                                      1;

   // Codec commands -- to be passed to codecs' main function
   .CONST        $codec.FRAME_DECODE                                          0;
   .CONST        $codec.INIT_DECODER                                          1;
   .CONST        $codec.RESET_DECODER                                         2;
   .CONST        $codec.SILENCE_DECODER                                       3;
   .CONST        $codec.SUSPEND_DECODER                                       4;
   .CONST        $codec.RESUME_DECODER                                        5;
   .CONST        $codec.STORE_BOUNDARY_SNAPSHOT                               6;
   .CONST        $codec.RESTORE_BOUNDARY_SNAPSHOT                             7;
   .CONST        $codec.FAST_SKIP                                             8;
   .CONST        $codec.SET_SKIP_FUNCTION                                     9;
   .CONST        $codec.SET_AVERAGE_BITRATE                                   10;
   .CONST        $codec.SET_FILE_TYPE                                         11;
   .CONST        $codec.DECRYPTER_SET_DECODER                                 12;
   .CONST        $codec.DECRYPTER_SET_PARAMETERS                              13;




   .CONST        $codec.stream_encode.ADDR_FIELD                             0*ADDR_PER_WORD;
   .CONST        $codec.stream_encode.RESET_ADDR_FIELD                       1*ADDR_PER_WORD;
   .CONST        $codec.stream_encode.ENCODER_STRUC_FIELD                    $codec.stream_encode.RESET_ADDR_FIELD + 1*ADDR_PER_WORD;
   // encoder data structure is located here
   .CONST        $codec.stream_encode.STATE_FIELD                            $codec.stream_encode.ENCODER_STRUC_FIELD + $codec.ENCODER_STRUC_SIZE*ADDR_PER_WORD;
   .CONST        $codec.stream_encode.MEDIA_HDR_ADDR_FIELD                   $codec.stream_encode.STATE_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.stream_encode.STRUC_SIZE                             $codec.stream_encode.MEDIA_HDR_ADDR_FIELD/ADDR_PER_WORD + 1;

   // inlining the encoder structure soon to be obsoleted
   .CONST        $codec.stream_encode.OUT_BUFFER_FIELD                       $codec.stream_encode.ENCODER_STRUC_FIELD + $codec.ENCODER_OUT_BUFFER_FIELD;
   .CONST        $codec.stream_encode.IN_LEFT_BUFFER_FIELD                   $codec.stream_encode.ENCODER_STRUC_FIELD + $codec.ENCODER_IN_LEFT_BUFFER_FIELD;
   .CONST        $codec.stream_encode.IN_RIGHT_BUFFER_FIELD                  $codec.stream_encode.ENCODER_STRUC_FIELD + $codec.ENCODER_IN_RIGHT_BUFFER_FIELD;
   .CONST        $codec.stream_encode.MODE_FIELD                             $codec.stream_encode.ENCODER_STRUC_FIELD + $codec.ENCODER_MODE_FIELD;
   // end of inlined encoder structure

    // Pay load header 1 byte for SBC
    // Pay load header 3 bytes for MP3
    // don't include 4 byte L2CAP header as this is outside the ATU
   .CONST        $codec.SBC_RTP_AND_PAYLOAD_HEADER_SIZE                     13;
   .CONST        $codec.MP3_RTP_AND_PAYLOAD_HEADER_SIZE                     16;
   .CONST        $codec.APTX_PAYLOAD_HEADER_SIZE                             0;

    // Data structure for Media header (RTP header + Payload header)
   .CONST        $codec.media_header.WRITE_HDR_FIELD                        0*ADDR_PER_WORD;
   .CONST        $codec.media_header.RTP_SEQUENCE_NUM_FIELD                 1*ADDR_PER_WORD;
   .CONST        $codec.media_header.RTP_TIMESTAMP_MS_FIELD                 2*ADDR_PER_WORD;
   .CONST        $codec.media_header.RTP_TIMESTAMP_LS_FIELD                 3*ADDR_PER_WORD;
   .CONST        $codec.media_header.FRAME_COUNT_FIELD                      4*ADDR_PER_WORD;
   .CONST        $codec.media_header.SAMPLES_PER_FRAME                      5*ADDR_PER_WORD;
   .CONST        $codec.media_header.PUT_BITS_ADDR_FIELD                    6*ADDR_PER_WORD;
   .CONST        $codec.media_header.GET_ENCODED_FRAME_INFO                 7*ADDR_PER_WORD;
   .CONST        $codec.media_header.NUM_FRAMES_FIELD                       8*ADDR_PER_WORD;
   .CONST        $codec.media_header.L2CAP_ATU_SIZE_FIELD                   9*ADDR_PER_WORD;
   .CONST        $codec.media_header.SHUNT_FRAME_BYTE_COUNT_FIELD           10*ADDR_PER_WORD;
   .CONST        $codec.media_header.RTP_AND_PAYLOAD_HEADER_SIZE_FIELD      11*ADDR_PER_WORD;
   .CONST        $codec.media_header.FASTSTREAM_ENABLED_FIELD               12*ADDR_PER_WORD;
   .CONST        $codec.media_header.MEDIA_HDR_STRUC_SIZE                   $codec.media_header.FASTSTREAM_ENABLED_FIELD/ADDR_PER_WORD + 1;

   .CONST        $codec.stream_encode.STATE_STOPPED                          0;
   .CONST        $codec.stream_encode.STATE_STREAMING                        1;
   .CONST        $codec.stream_encode.STATE_MASK                             1;

   .CONST        $codec.stream_decode.ADDR_FIELD                             0*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.RESET_ADDR_FIELD                       1*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.SILENCE_ADDR_FIELD                     2*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.DECODER_STRUC_FIELD                    $codec.stream_decode.SILENCE_ADDR_FIELD + ADDR_PER_WORD;
   // decoder data structue is inlined here
   .CONST        $codec.stream_decode.COMFORT_NOISE_GAIN_FIELD               $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 0)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.GOOD_WORKING_BUFLEVEL_FIELD            $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 1)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.POORLINK_DETECT_BUFLEVEL_FIELD         $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 2)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.POORLINK_PERIOD_FIELD                  $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 3)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.PLAYING_ZERO_DATARATE_PERIOD_FIELD     $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 4)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.BUFFERING_ZERO_DATARATE_PERIOD_FIELD   $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 5)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.POORLINK_DETECT_TABLE_ADDR_FIELD       $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 6)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.POORLINK_DETECT_TABLE_SIZE_FIELD       $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 7)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.WARP_RATE_HIGH_COEF_FIELD              $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 8)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.WARP_RATE_LOW_COEF_FIELD               $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 9)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.WARP_RATE_TRANSITION_LEVEL_FIELD       $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 10)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.POORLINK_DETECT_TABLE_OFFSET_FIELD     $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 11)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.RAND_LAST_VAL_FIELD                    $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 12)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.STATE_FIELD                            $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 13)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.POORLINK_ENDTIME_FIELD                 $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 14)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.ZERO_DATARATE_ENDTIME_FIELD            $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 15)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.PLAYING_STARTTIME_FIELD                $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 16)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.PREV_WRITE_PTR_FIELD                   $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 17)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.FAST_AVERAGE_BUFLEVEL_FIELD            $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 18)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.SLOW_AVERAGE_BUFLEVEL_FIELD            $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 19)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.SLOW_AVERAGE_SAMPLE_TIME_FIELD         $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 20)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.WARP_TARGET_FIELD                      $codec.stream_decode.DECODER_STRUC_FIELD + ($codec.DECODER_STRUC_SIZE + 21)*ADDR_PER_WORD;
   .CONST        $codec.stream_decode.STRUC_SIZE                             $codec.stream_decode.WARP_TARGET_FIELD/ADDR_PER_WORD + 1;

    // inlined decoder structure - soon to be obsoleted
   .CONST        $codec.stream_decode.IN_BUFFER_FIELD                        $codec.stream_decode.DECODER_STRUC_FIELD + $codec.DECODER_IN_BUFFER_FIELD;
   .CONST        $codec.stream_decode.OUT_LEFT_BUFFER_FIELD                  $codec.stream_decode.DECODER_STRUC_FIELD + $codec.DECODER_OUT_LEFT_BUFFER_FIELD;
   .CONST        $codec.stream_decode.OUT_RIGHT_BUFFER_FIELD                 $codec.stream_decode.DECODER_STRUC_FIELD + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD;
   .CONST        $codec.stream_decode.MODE_FIELD                             $codec.stream_decode.DECODER_STRUC_FIELD + $codec.DECODER_MODE_FIELD;
   .CONST        $codec.stream_decode.NUM_OUTPUT_SAMPLES_FIELD               $codec.stream_decode.DECODER_STRUC_FIELD + $codec.DECODER_NUM_OUTPUT_SAMPLES_FIELD;
   // end of inlined decoder structure


   .CONST        $codec.stream_decode.STATE_BUFFERING                        0;
   .CONST        $codec.stream_decode.STATE_PLAYING                          1;
   .CONST        $codec.stream_decode.STATE_MASK                             1;

   .CONST        $codec.stream_decode_sync.CONDITION_READ_WALL_CLOCK         2;
   .CONST        $codec.stream_decode_sync.CONDITION_DECODE_FRAME            4;
   .CONST        $codec.stream_decode_sync.CONDITION_INSERT_SAMPLES          8;

   .CONST        $codec.stream_decode.CONDITION_POORLINK                     2;
   .CONST        $codec.stream_decode.CONDITION_DETECT_POORLINK_ENABLE       4;
   .CONST        $codec.stream_decode.CONDITION_ZERO_DATARATE                8;
   .CONST        $codec.stream_decode.CONDITION_STOPPING                    16;

   .CONST        $codec.stream_decode.FAST_AVERAGE_SHIFT_CONST              -6;
   .CONST        $codec.stream_decode.SLOW_AVERAGE_SHIFT_CONST              -2;
   .CONST        $codec.stream_decode.SLOW_AVERAGE_SAMPLE_PERIOD         50000;








   .CONST        $codec.stream_relay.GET_FRAME_INFO_ADDR_FIELD                0*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.IN_BUFFER_FIELD                          1*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.LOCAL_RELAY_BUFFER_FIELD                 2*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.REMOTE_RELAY_BUFFER_FIELD                3*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.BUFFER_DELAY_IN_US_FIELD                 4*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.WARP_RATE_COEF_FIELD                     5*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.NUM_AUDIO_SAMPLES_CONSUMED_PNTR_FIELD    6*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.WALL_CLOCK_STRUC_PNTR_FIELD              7*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.NUM_MICRO_SECS_PER_1024_SAMPLES_FIELD    8*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.PREVIOUS_AUDIO_FRAME_LENGTH_FIELD        9*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.SLOW_AVERAGE_BUFLEVEL_FIELD              10*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.SLOW_AVERAGE_SAMPLE_TIME_FIELD           11*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.STATE_FIELD                              12*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.TIME_TO_PLAY_FIELD                       13*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.NUM_AUDIO_SAMPLES_IN_BUFFER_FIELD        14*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.GOOD_WORKING_BUFLEVEL_FIELD              15*ADDR_PER_WORD;
   .CONST        $codec.stream_relay.STRUC_SIZE                               16;

   .CONST        $codec.stream_relay.SLOW_AVERAGE_SAMPLE_PERIOD               50000;
   .CONST        $codec.stream_relay.SLOW_AVERAGE_SHIFT_CONST                 -2;

   .CONST        $codec.stream_relay.STATE_BUFFERING                          0;
   .CONST        $codec.stream_relay.STATE_PLAYING                            1;



   .CONST        $codec.stream_decode_sync.ADDR_FIELD                               0*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.RESET_ADDR_FIELD                         1*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.SILENCE_ADDR_FIELD                       2*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.GET_FRAME_INFO_ADDR_FIELD                3*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.LEFT_DAC_PORT_FIELD                      4*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.RIGHT_DAC_PORT_FIELD                     5*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.IN_BUFFER_FIELD                          6*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.OUT_LEFT_BUFFER_FIELD                    7*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.OUT_RIGHT_BUFFER_FIELD                   8*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.MODE_FIELD                               9*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.NUM_OUTPUT_SAMPLES_FIELD                 10*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.COMFORT_NOISE_GAIN_FIELD                 11*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.DELAY_TO_WARP_COEF_FIELD                 12*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.MAX_PLAY_TIME_OFFSET_IN_US_FIELD         13*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.GENERATE_SILENCE_THRESHOLD_FIELD         14*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.AMOUNT_OF_SILENCE_TO_GENERATE_FIELD      15*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.CURRENT_MODE_FIELD                       16*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.DECODER_MODE_FIELD                       17*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.FRAME_PLAY_TIME_FIELD                    18*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.NUM_MICRO_SECS_PER_1024_SAMPLES_FIELD    19*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.NUM_SAMPLES_PER_US_FIELD                 20*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.PADDING_BYTE_FIELD                       21*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.RAND_LAST_VAL_FIELD                      22*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.REQUESTED_WARP_FIELD                     23*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.STREAMING_FIELD                          24*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.AUDIO_SAMPLES_CONSUMED_FIELD             25*ADDR_PER_WORD;
#ifndef KYMERA
   .CONST        $codec.stream_decode_sync.WALL_CLOCK_STRUC_FIELD                   26*ADDR_PER_WORD;
   .CONST        $codec.stream_decode_sync.STRUC_SIZE                               $codec.stream_decode_sync.WALL_CLOCK_STRUC_FIELD/ADDR_PER_WORD + $wall_clock.STRUC_SIZE;
#else
    // There is no wall clock structure on KYMERA, and $wall_clock.get_time doesn't exist.
    // Any attempt to use the old-style wallclock is doomed.
   .CONST        $codec.stream_decode_sync.STRUC_SIZE                               $codec.stream_decode_sync.AUDIO_SAMPLES_CONSUMED_FIELD/ADDR_PER_WORD + 1;
#endif

   .CONST        $codec.stream_decode_sync.STREAMING_STOPPED                        0;
   .CONST        $codec.stream_decode_sync.STREAMING_PLAYING                        1;



   .CONST        $codec.av_encode.ADDR_FIELD                                  0*ADDR_PER_WORD;
   .CONST        $codec.av_encode.RESET_ADDR_FIELD                            $codec.av_encode.ADDR_FIELD + 1*ADDR_PER_WORD;//If not define then no reset during stall
   // beginning of nested encoder structure
   .CONST        $codec.av_encode.ENCODER_STRUC_FIELD                         $codec.av_encode.RESET_ADDR_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_encode.OUT_BUFFER_FIELD                            $codec.av_encode.ENCODER_STRUC_FIELD;
   .CONST        $codec.av_encode.IN_LEFT_BUFFER_FIELD                        $codec.av_encode.OUT_BUFFER_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_encode.IN_RIGHT_BUFFER_FIELD                       $codec.av_encode.IN_LEFT_BUFFER_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_encode.MODE_FIELD                                  $codec.av_encode.IN_RIGHT_BUFFER_FIELD + 1*ADDR_PER_WORD;
    // added field for data object pointer
   .CONST        $codec.av_encode.ENCODER_DATA_OBJECT_FIELD                   $codec.av_encode.MODE_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_encode.BUFFERING_THRESHOLD_FIELD                   $codec.av_encode.ENCODER_DATA_OBJECT_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_encode.STALL_COUNTER_FIELD                         $codec.av_encode.BUFFERING_THRESHOLD_FIELD + 1*ADDR_PER_WORD;
   // size of the av_encode structure
   .CONST        $codec.av_encode.STRUC_SIZE                                  $codec.av_encode.STALL_COUNTER_FIELD/ADDR_PER_WORD + 1;


   .CONST        $codec.av_decode.ADDR_FIELD                                  0*ADDR_PER_WORD;
   .CONST        $codec.av_decode.RESET_ADDR_FIELD                            $codec.av_decode.ADDR_FIELD + 1*ADDR_PER_WORD; //If not define then no reset during stall
   .CONST        $codec.av_decode.SILENCE_ADDR_FIELD                          $codec.av_decode.RESET_ADDR_FIELD + 1*ADDR_PER_WORD;
   // beginning of nested decoder structure
   .CONST        $codec.av_decode.DECODER_STRUC_FIELD                         $codec.av_decode.SILENCE_ADDR_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.IN_BUFFER_FIELD                             $codec.av_decode.DECODER_STRUC_FIELD;
   .CONST        $codec.av_decode.OUT_LEFT_BUFFER_FIELD                       $codec.av_decode.IN_BUFFER_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.OUT_RIGHT_BUFFER_FIELD                      $codec.av_decode.OUT_LEFT_BUFFER_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.MODE_FIELD                                  $codec.av_decode.OUT_RIGHT_BUFFER_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.DECODER_NUM_OUTPUT_SAMPLES_FIELD            $codec.av_decode.MODE_FIELD + 1*ADDR_PER_WORD;
    // added field for data object pointer
   .CONST        $codec.av_decode.DECODER_DATA_OBJECT_FIELD                   $codec.av_decode.DECODER_NUM_OUTPUT_SAMPLES_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.BUFFERING_THRESHOLD_FIELD                   $codec.av_decode.DECODER_DATA_OBJECT_FIELD + 1*ADDR_PER_WORD;   // not used
   .CONST        $codec.av_decode.DECODE_TO_STALL_THRESHOLD                   $codec.av_decode.BUFFERING_THRESHOLD_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.STALL_BUFFER_LEVEL_FIELD                    $codec.av_decode.DECODE_TO_STALL_THRESHOLD + 1*ADDR_PER_WORD; //In fraction of codec buffer size
   .CONST        $codec.av_decode.NORMAL_BUFFER_LEVEL_FIELD                   $codec.av_decode.STALL_BUFFER_LEVEL_FIELD + 1*ADDR_PER_WORD;  //In fraction of codec buffer size (not used)
   .CONST        $codec.av_decode.CODEC_PURGE_ENABLE_FIELD                    $codec.av_decode.NORMAL_BUFFER_LEVEL_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.MASTER_RESET_FUNC_FIELD                     $codec.av_decode.CODEC_PURGE_ENABLE_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.CODEC_PREV_WADDR_FIELD                      $codec.av_decode.MASTER_RESET_FUNC_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.PAUSE_TIMER_FIELD                           $codec.av_decode.CODEC_PREV_WADDR_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.CURRENT_RUNNING_MODE_FIELD                  $codec.av_decode.PAUSE_TIMER_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.CAN_SWITCH_FIELD                            $codec.av_decode.CURRENT_RUNNING_MODE_FIELD + 1*ADDR_PER_WORD;
   .CONST        $codec.av_decode.PREV_TIME_FIELD                             $codec.av_decode.CAN_SWITCH_FIELD + 1*ADDR_PER_WORD;
    // size of the av_decode structure
   .CONST        $codec.av_decode.STRUC_SIZE                                  $codec.av_decode.PREV_TIME_FIELD/ADDR_PER_WORD + 1;

   .CONST        $codec.av_decode.PAUSE_TO_CLEAR_THRESHOLD                   150; // in ms
   .CONST        $codec.av_decode.STALL_TO_DECODE_THRESHOLD                   70; // in ms
   .CONST        $codec.av_encode.STALL_THRESHOLD                             10;

   .CONST        $codec.pre_post_proc.PRE_PROC_FUNC_ADDR_FIELD                0*ADDR_PER_WORD;
   .CONST        $codec.pre_post_proc.PRE_PROC_FUNC_DATA1_FIELD               1*ADDR_PER_WORD;
   .CONST        $codec.pre_post_proc.PRE_PROC_FUNC_DATA2_FIELD               2*ADDR_PER_WORD;
   .CONST        $codec.pre_post_proc.POST_PROC_FUNC_ADDR_FIELD               3*ADDR_PER_WORD;
   .CONST        $codec.pre_post_proc.POST_PROC_FUNC_DATA1_FIELD              4*ADDR_PER_WORD;
   .CONST        $codec.pre_post_proc.POST_PROC_FUNC_DATA2_FIELD              5*ADDR_PER_WORD;
   .CONST        $codec.pre_post_proc.STRUC_SIZE                              6;

   .CONST        $codec.stream_encode.mux.NEXT_SINK_HANDLER_FIELD            0*ADDR_PER_WORD;   // Must be zero offset
   .CONST        $codec.stream_encode.mux.STATE_FIELD                        1*ADDR_PER_WORD;
   .CONST        $codec.stream_encode.mux.PREV_READ_PNTR_FIELD               2*ADDR_PER_WORD;
   .CONST        $codec.stream_encode.mux.AMOUNT_LEFT_TO_DISCARD_FIELD       3*ADDR_PER_WORD;  // Used only with L2CAP
   .CONST        $codec.stream_encode.mux.TERMINAL_FIELD                     4*ADDR_PER_WORD;  // The terminal number of the operator
   .CONST        $codec.stream_encode.mux.PORT_FIELD                         5*ADDR_PER_WORD;  // The port this terminal is connected to
   .CONST        $codec.stream_encode.mux.BYTE_ALIGNMENT_FIELD               6*ADDR_PER_WORD;  // Keeps track of byte alignment
   .CONST        $codec.stream_encode.mux.CBUFFER_STRUC_FIELD                7*ADDR_PER_WORD;
   .CONST        $codec.stream_encode.mux.STRUC_SIZE                         $codec.stream_encode.mux.CBUFFER_STRUC_FIELD/ADDR_PER_WORD + $cbuffer.STRUC_SIZE;

#endif

