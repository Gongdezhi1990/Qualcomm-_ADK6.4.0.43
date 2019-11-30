// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef LATENCY_HEADER_INCLUDED
#define LATENCY_HEADER_INCLUDED

   // structure for PCM latency
   .CONST  $pcm_latency.BUFFERS_LIST_FIELD                                 0;  // (input) pointer to a null terminated list of (cbuffer, 1/fs, warp) triplets
   .CONST  $pcm_latency.SAMPLES_LIST_FIELD                                 1;  // (input) pointer to a null terminated list of (samples, 1/fs, warp) triplets
   .CONST  $pcm_latency.STRUC_SIZE                                         2;

   // structure for calculating encoded latency
   .CONST  $encoded_latency.PCM_LATENCY_STRUCT_FIELD                       0;    // (input) structure for calculating PCM latency ($pcm_latency structure)
   .CONST  $encoded_latency.CODEC_PACKETS_INFO_CBUFFER_FIELD               1;    // (input) cbuffer holding packets boundary info for coded data(pairs of (arrival_time, boundary)
   .CONST  $encoded_latency.CODEC_CBUFFER_FIELD                            2;    // (input) codec cbuffer data
   .CONST  $encoded_latency.DECODED_CBUFFER_FIELD                          3;    // (input) decoded output cbuffer data
   .CONST  $encoded_latency.DECODER_INV_SAMPLE_RATE_FIELD                  4;    // (input) inverse sample rate of decoder output
   .CONST  $encoded_latency.CURRENT_WARP_RATE_FIELD                        5;    // (input) optional current warp rate used in the system
   .CONST  $encoded_latency.SEARCH_MIN_LEN_FIELD                           6;    // if NZ, report minimum value over last N calc
   .CONST  $encoded_latency.LATENCY_MEASUREMENT_LAST_CODED_RD_ADDR_FIELD   7;    // (state) last read pointer of codec cbuffer processed
   .CONST  $encoded_latency.LATENCY_MEASUREMENT_LAST_DECODED_WR_ADDR_FIELD 8;    // (state) last write pointer of decoded cbuffer processed
   .CONST  $encoded_latency.TOTAL_LATENCY_US_FIELD                         9;    // (output) computed latency
   .CONST  $encoded_latency.HIST_LATENCY_US_FIELD                          10;   // (state) history to calculate minimum
   .CONST  $encoded_latency.STRUC_SIZE                                     14;

   #define $latency.PCM_LATENCY_SHIFT  7
   #define $latency.INV_FS(fs) (15625.0/2.0/fs)                                 // calc inverse of fs suitable for latency structures
#endif
