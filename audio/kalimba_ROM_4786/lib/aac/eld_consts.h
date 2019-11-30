// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

#ifndef ELD_CONSTS_HEADER_INCLUDED
#define ELD_CONSTS_HEADER_INCLUDED

   // AAC ELD constants
   .CONST $aacdec.ELDEXT_TERM                   0;

   .CONST $aacdec.NO_DELAY                      0;
   .CONST $aacdec.DEFAULT_DELAY_480             120;
   .CONST $aacdec.DEFAULT_DELAY_512             128;

   // number of zeros removed from the start of the window tables (to save memory)
   .CONST $aacdec.WINDOWING_ZEROS_480           120;
   .CONST $aacdec.WINDOWING_ZEROS_512           128;

   // size of window to be applied for 512|480 frame
   .CONST $aacdec.WIN_480_LD_SIZE               (1920 - $aacdec.WINDOWING_ZEROS_480);
   .CONST $aacdec.WIN_512_LD_SIZE               (2048 - $aacdec.WINDOWING_ZEROS_512);

   // size of twiddle factor table for 480 frame size
   .CONST $aacdec.TWIDDLE_TABLE_SIZE            61;

   // frame size (spectral/time domain)
   .CONST $aacdec.FRAME_SIZE_512                512;
   .CONST $aacdec.FRAME_SIZE_480                480;

   // LATM & LOAS constants
   .CONST $aacdec.LATM_SYNC_WORD                0x2B7;

   // byte alignment ON/OFF switch
   .CONST $aacdec.BYTE_ALIGN_ON                 1;
   .CONST $aacdec.BYTE_ALIGN_OFF                0;

   // FRAME_SIZE_480 constants
   .CONST $aacdec.IFFT_Q1_BOUNDARY              60;
   .CONST $aacdec.IFFT_Q2_BOUNDARY              120;
   .CONST $aacdec.IFFT_Q3_BOUNDARY              180;
   .CONST $aacdec.IFFT_NUM_POINTS               240;

   .CONST $aacdec.IFFT_OUTPUT_OFFSET            720*ADDR_PER_WORD;

   .CONST $aacdec.POWER_OF_2_IFFT_SIZE          16;
   .CONST $aacdec.POWER_OF_2_IFFT_SIZE_MASK     ($aacdec.POWER_OF_2_IFFT_SIZE - 1);
   .CONST $aacdec.NON_POWER_OF_2_DFT_SIZE       15;

#endif
