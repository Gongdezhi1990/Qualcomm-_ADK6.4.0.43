// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Filter Bank Library
//
// DESCRIPTION:
//    This library provides the filter bank analysis and synthesis API functions
//    for for various configurations according to different frame size(64/60, 128/120),
//    filter bank proto window size (128 or 256), and point real FFT (128 or 256).
//
//    This library provides the following API functions:
//       - $filter_bank.one_channel.analysis.initialize:
//       - $filter_bank.one_channel.analysis.process:
//       - $filter_bank.one_channel.synthesis.initialize:
//       - $filter_bank.one_channel.synthesis.process:
//       - $filter_bank.two_channel.analysis.initialize:
//       - $filter_bank.two_channel.analysis.process:
//       - $filter_bank.two_channel.synthesis.initialize:
//       - $filter_bank.two_channel.synthesis.process:
//       - $M.filter_bank.cepstrum128.Process
//
// HISTORY:
//    06-02-03 Fri wms 1.0.0: initial version
//    06-05-22 Mon wms 1.1.0: added some one channel subroutines
//    07-10-16 Tue mcv 1.2.0: added two channel analysis for 64/128/128 config
//    08-01-10 Thu wms 1.3.0: utilizing mathlib fft
//    08-02-12 Tue wms 1.3.1: added two channel synthesis for 64/128/128 config
//    08-03-12 Wed wms 2.0.0: fft data object
//    08-04-03 Thu wms 2.0.1: fft data object user defined bit-reversed cbuffer
//    11-02-22 Thu wms 2.0.2: static memory removed
//    11-10-25 Tue wms 3.0.0: general api
//    12-03-27 Tue wms 3.1.0: 60/240/128 and 120/480/256 config
//    12-04-25 Wed wms 3.2.0: data memory loading and sharing
// *****************************************************************************

#ifndef _FILTER_BANK_LIB_H
#define	_FILTER_BANK_LIB_H

// Library Build Version
.CONST	$FILTER_BANK_VERSION			0x030200;		//3.2.0



//***************************************************************************
//  Filter Bank configuration parameters
//***************************************************************************
   .CONST   $M.filter_bank.Parameters.FRAME32                  32;
   .CONST   $M.filter_bank.Parameters.FRAME60                  60;
   .CONST   $M.filter_bank.Parameters.FRAME64                  64;
   .CONST   $M.filter_bank.Parameters.FRAME120                 120;
   .CONST   $M.filter_bank.Parameters.FRAME128                 128;

   .CONST   $M.filter_bank.Parameters.PROTO120                 120;
   .CONST   $M.filter_bank.Parameters.PROTO128                 128;
   .CONST   $M.filter_bank.Parameters.PROTO240                 240;
   .CONST   $M.filter_bank.Parameters.PROTO256                 256;
   .CONST   $M.filter_bank.Parameters.PROTO480                 480;
   .CONST   $M.filter_bank.Parameters.PROTO512                 512;

   .CONST   $M.filter_bank.Parameters.FFT128_SCALE             6;
   .CONST   $M.filter_bank.Parameters.FFT256_SCALE             7;

   .CONST   $M.filter_bank.Parameters.FFT128_NUM_BIN           65;
   .CONST   $M.filter_bank.Parameters.FFT256_NUM_BIN           129;

   .CONST   $M.filter_bank.Parameters.FFT128_BUFFLEN           64;  // arch4: in words
   .CONST   $M.filter_bank.Parameters.FFT256_BUFFLEN           128; // arch4: in words

   .CONST   $M.filter_bank.Parameters.SPLIT_TABLELEN           63;  // arch4: in words

//***************************************************************************
// Analysis/Synthesis Data Stream Default Data Q-format:
//    0: Q1.23, data upshifted to the upper part of 24 bits
//    8: Q9.15, data stay at lowest 16 bits, or
//    x: Q(x+1).(23-x)
//***************************************************************************

// if COMPACT_HISTORYBUF flag is set, history will be converted between Q1.23
// format (framebuf) and Q1.15 format (historybuf) so set Q_DAT accordingly
#ifdef FB_USES_16BIT_HISTORY
   .CONST   $M.filter_bank.Parameters.Q_DAT_IN                 8;
   .CONST   $M.filter_bank.Parameters.Q_DAT_OUT                8;
#else
   .CONST   $M.filter_bank.Parameters.Q_DAT_IN                 0;
   .CONST   $M.filter_bank.Parameters.Q_DAT_OUT                0;
#endif

//***************************************************************************
// FFT/IFFT default extra scaling: results are left-shifted by scaling factor
//***************************************************************************
   .CONST   $M.filter_bank.Parameters.FFT_EXTRA_SCALE          0;
   .CONST   $M.filter_bank.Parameters.IFFT_EXTRA_SCALE         0;


//***************************************************************************
//  Filter Bank configuration object structure
//***************************************************************************
   .CONST $M.filter_bank.config.FRAME              0*ADDR_PER_WORD;  // in words (samples)
   .CONST $M.filter_bank.config.PROTO              1*ADDR_PER_WORD;  // in words (samples)
   .CONST $M.filter_bank.config.ZEROPADDED_PROTO   2*ADDR_PER_WORD;  // in words (samples)
   .CONST $M.filter_bank.config.FFT_SCALE          3*ADDR_PER_WORD;
   .CONST $M.filter_bank.config.PTR_PROTO          4*ADDR_PER_WORD;
   .CONST $M.filter_bank.config.STRUCT_SIZE        5;

//********************************************************************
// Filter Bank FFT configuration Data Structure
// To be used by API functions as input in r7
//
// Note on the sizes of FFT buffers:
//    - for one channel application, number of FFT bins
//    - for two channel application, 2 times of number of FFT bins
//    - for various application combinations, use maximum size
// These buffers are FFT temporary buffers, after FFT, they are freed
//********************************************************************

   // @DATA_OBJECT FBFFT_DATAOBJECT

   // @DOC_FIELD_TEXT Internal, user don't care
   .CONST   $M.filter_bank.fft.NUM_POINTS_FIELD      0*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT FFT temporary buffer(1), DM1
   // @DOC_FIELD_TEXT FFT Required size: 64 for FFT128, 128 for FFT256, double if two channel
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.fft.REAL_ADDR_FIELD       1*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT FFT temporary buffer(2), DM2
   // @DOC_FIELD_TEXT FFT Required size: 64 for FFT128, 128 for FFT256, double if two channel
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.fft.IMAG_ADDR_FIELD       2*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT FFT temporary buffer(3), DM2, must be CIRCULAR
   // @DOC_FIELD_TEXT FFT Required size: 64 for FFT128, 128 for FFT256, double if two channel
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.fft.CBUF_ADDR_FIELD       3*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Pointer to filter bank split cos table
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.fft.PTR_FFTSPLIT          4*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT FFT extra scaling
   // @DOC_FIELD_FORMAT Integer
   .CONST   $M.filter_bank.fft.FFT_EXTRA_SCALE       5*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT IFFT extra scaling
   // @DOC_FIELD_FORMAT Integer
   .CONST   $M.filter_bank.fft.IFFT_EXTRA_SCALE      6*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT FFT input data Q scaling, internal, default 0
   // @DOC_FIELD_FORMAT Integer
   .CONST   $M.filter_bank.fft.Q_DAT_IN              7*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT IFFT output data Q scaling, internal, default 0
   // @DOC_FIELD_FORMAT Integer
   .CONST   $M.filter_bank.fft.Q_DAT_OUT             8*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Size of the fft configuration data structure
   .CONST   $M.filter_bank.fft.STRUC_SIZE            9;

   // @END  DATA_OBJECT FBFFT_DATAOBJECT

//********************************************************************
// Filter Bank Data Structure Element Offset Definitions
//  for one channel processing
//
// The application using this set of library must include a data block
//  of size ONE_CHNL_BLOCK_SIZE
//
//  The associated parameters should be initialized as defined below.
//********************************************************************

   // @DATA_OBJECT FB1CH_DATAOBJECT

   // @DOC_FIELD_TEXT Configuration object
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CONFIG_OBJECT    0*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Time Domain Input(Anlysis)/Output(Synthesis) Frame Buffer Pointer
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_PTR_FRAME        1*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Pointer to History Buffer DM2
   // @DOC_FIELD_FORMAT Pointer
   .CONST  $M.filter_bank.Parameters.OFFSET_PTR_HISTORY       2*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Output(Analysis) Normalized Exponent
   // @DOC_FIELD_FORMAT Integer
   .CONST   $M.filter_bank.Parameters.OFFSET_BEXP             3*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Input(Synthesis) Normalized Exponent
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_PTR_BEXP         3*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Pointer to IFFT Input(Synthesis)/ FFT output (Analysis) Frequency Object
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_PTR_FFT_FREQ_OBJ 4*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT pointer into reference delay stream structure
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_DELAY_STREAM_PTR 5*ADDR_PER_WORD;
   // Flag for clearing DC and Nyquist bin
   .CONST   $fbs.ZDN_FIELD    $M.filter_bank.Parameters.OFFSET_DELAY_STREAM_PTR;
   // @DOC_FIELD_TEXT pointer into reference delay parameter
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_DELAY_PARAM_PTR  6*ADDR_PER_WORD;
   // Flag for IFFT saturation protection:
   //          0  - Don't do SP
   //    negative - Unconditional SP
   //    positive - Conditional SP (based on internal saturation test)
   .CONST   $fbs.SP_FIELD  $M.filter_bank.Parameters.OFFSET_DELAY_PARAM_PTR;
   // @DOC_FIELD_TEXT Size of the block
   .CONST   $M.filter_bank.Parameters.ONE_CHNL_BLOCK_SIZE     7;
   // @END  DATA_OBJECT FB1CH_DATAOBJECT

//********************************************************************
// Filter Bank Data Structure Element Offset Definitions
//  for two channels bunddled processing
//
// The application using this set of library must include a data block
//  of size TWO_CHNL_BLOCK_SIZE
//
//  The associated parameters should be initialized as defined below.
//********************************************************************

   // @DATA_OBJECT FB2CH_DATAOBJECT

   // @DOC_FIELD_TEXT Configuration object
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CONFIG_OBJECT        0*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (1) Time Domain Input(Anlysis)/Output(Synthesis) Frame Buffer Pointer
   // @DOC_FIELD_TEXT pointer to circular or linear buffer in DM1
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH1_PTR_FRAME        1*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (2) Time Domain Input(Anlysis)/Output(Synthesis) Frame Buffer Pointer
   // @DOC_FIELD_TEXT pointer to circular or linear buffer in DM2
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH2_PTR_FRAME        2*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (1) Pointer to History Buffer in DM1
   // @DOC_FIELD_FORMAT Pointer
   .CONST  $M.filter_bank.Parameters.OFFSET_CH1_PTR_HISTORY       3*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (2) Pointer to History Buffer in DM2
   // @DOC_FIELD_FORMAT Pointer
   .CONST  $M.filter_bank.Parameters.OFFSET_CH2_PTR_HISTORY       4*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (1) Output(Analysis) Normallized Exponent
   // @DOC_FIELD_FORMAT Integer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH1_BEXP             5*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (1) Input(Synthesis) Normallized Exponent
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH1_PTR_BEXP         5*ADDR_PER_WORD;
   // @DOC_FIELD_FORMAT Integer
   // @DOC_FIELD_TEXT Channel (2) Output(Analysis) Normallized Exponent
   .CONST   $M.filter_bank.Parameters.OFFSET_CH2_BEXP             6*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (2) Input(Synthesis) Normallized Exponent
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH2_PTR_BEXP         6*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (1) Pointer to IFFT Input(Synthesis)/ FFT output (Analysis)
   // @DOC_FIELD_TEXT Frequency Object (DM1)
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH1_PTR_FFT_FREQ_OBJ 7*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT Channel (2) Pointer to IFFT Input(Synthesis)/ FFT output (Analysis)
   // @DOC_FIELD_TEXT Frequency Object (DM1)
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH2_PTR_FFT_FREQ_OBJ 8*ADDR_PER_WORD;

   // @DOC_FIELD_TEXT pointer to linear buffer in DM2
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH2_DELAY_PTR        9*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT base address of reference delay buffer
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH2_DELAY_BASE       10*ADDR_PER_WORD;

   // @DOC_FIELD_TEXT pointer into reference delay buffer
   // @DOC_FIELD_FORMAT Pointer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH2_DELAY_BUF_PTR    11*ADDR_PER_WORD;
   // @DOC_FIELD_TEXT length of reference delay bufferin addr (bytes on arch4) set to LENGTH(ref_delay_buffer)
   // @DOC_FIELD_FORMAT Integer
   .CONST   $M.filter_bank.Parameters.OFFSET_CH2_DELAY_BUF_LEN    12*ADDR_PER_WORD;

   // @DOC_FIELD_TEXT Size of the block
   .CONST   $M.filter_bank.Parameters.TWO_CHNL_BLOCK_SIZE         13;
   // @END  DATA_OBJECT FB2CH_DATAOBJECT


// *****************************************************************************
// API configuration objects:
// The following list is the available filter bank configurations, it is to be
// declared in the 'OFFSET_CONFIG_OBJECT' field in both analasys and synthesis
// data structure.
//    - $filter_bank.config.one_channel.frame64_proto128_fft128
//    - $filter_bank.config.two_channel.frame64_proto128_fft128
//    - $filter_bank.config.one_channel.frame128_proto256_fft256
//    - $filter_bank.config.two_channel.frame128_proto256_fft256
//    - $filter_bank.config.one_channel.frame64_proto256_fft128
//    - $filter_bank.config.two_channel.frame64_proto256_fft128
//    - $filter_bank.config.one_channel.frame128_proto512_fft256
//    - $filter_bank.config.two_channel.frame128_proto512_fft256
//    - $filter_bank.config.one_channel.frame60_proto120_fft128
//    - $filter_bank.config.two_channel.frame60_proto120_fft128
//    - $filter_bank.config.one_channel.frame120_proto240_fft256
//    - $filter_bank.config.two_channel.frame120_proto240_fft256
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $filter_bank.one_channel.analysis.initialize
//
// DESCRIPTION:
//    Initialize filter bank analysis history buffers for one channel
//
// MODIFICATIONS:
//    11-10-25 Tue wms - initial version
//
// INPUTS:
//    r7 = fft data object
//    r8 = &$filter_bank_analysis_data_block;
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $filter_bank.one_channel.analysis.process
//
// DESCRIPTION:
//    One Channel Filter Bank Analysis Operation.
//
//    After this function being executed, the caller will have the following
//    things available (as defined in data block(r8)):
//        (1) Input stream buffer pointer is updated
//        (2) BExp value is updated
//        (3) FFT values (FFT_real and FFT_imag) are calculated
//            from time domain signal inputs.
//
// MODIFICATIONS:
//    11-10-25 Tue wms - initial version
//
// INPUTS:
//    r7 = fft data object
//    r8 = &$filter_bank_analysis_data_block;
//
// OUTPUTS:
//    None. See description for information.
//
// TRASHED REGISTERS:
//    Everything
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $filter_bank.one_channel.synthesis.initialize
//
// DESCRIPTION:
//    Initialize filter bank synthesis history buffers for one channel
//
// MODIFICATIONS:
//    11-10-25 Tue wms - initial version
//
// INPUTS:
//    r7 = fft data object
//    r8 = &$filter_bank_synthesis_data_block;
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $filter_bank.one_channel.synthesis.process
//
// DESCRIPTION:
//    One Channel Filter Bank Synthesis Operation.
//
//    After this function being executed, the caller will have the following
//    things available, as defined in data block(r8):
//        (1) Time domain signal are transferred back from frequency domain,
//            and are put to output stream.
//        (2) Output stream buffer pointer is updated
//
// MODIFICATIONS:
//    11-10-25 Tue wms - initial version
//
// INPUTS:
//    r7 = fft data object
//    r8 = &$filter_bank_synthesis_data_block;
//
// OUTPUTS:
//    None. See description for information.
//
// TRASHED REGISTERS:
//    Everything
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $filter_bank.two_channel.analysis.initialize
//
// DESCRIPTION:
//    Initialize filter bank analysis history buffers for two channels
//
// MODIFICATIONS:
//    11-10-25 Tue wms - initial version
//
// INPUTS:
//    r7 = fft data object
//    r8 = &$filter_bank_analysis_data_block;
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $filter_bank.two_channel.analysis.process
//
// DESCRIPTION:
//    Two Channel Filter Bank Analysis Operation.
//
//    After this function being executed, the caller will have the following
//    things available (as defined in data block(r8)):
//        (1) Input stream buffer pointer is updated
//        (2) BExp value is updated
//        (3) FFT values (FFT_real and FFT_imag) are calculated
//            from time domain signal inputs.
//
// MODIFICATIONS:
//    11-10-25 Tue wms - initial version
//
// INPUTS:
//    r7 = fft data object
//    r8 = &$filter_bank_analysis_data_block;
//
// OUTPUTS:
//    None. See description for information.
//
// TRASHED REGISTERS:
//    Everything
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $filter_bank.two_channel.synthesis.initialize
//
// DESCRIPTION:
//    Initialize filter bank synthesis history buffers for two channels
//
// MODIFICATIONS:
//    11-10-25 Tue wms - initial version
//
// INPUTS:
//    r7 = fft data object
//    r8 = &$filter_bank_analysis_data_block;
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************


// *****************************************************************************
// MODULE:
//    $filter_bank.two_channel.synthesis.process
//
// DESCRIPTION:
//    Two Channel Filter Bank Synthesis Operation.
//
//    After this function being executed, the caller will have the following
//    things available for the two channels defined in data block(r8):
//        (1) Time domain signal are transferred back from frequency domain,
//            and are put to output stream.
//        (2) Output stream buffer pointers are updated
//
// MODIFICATIONS:
//    11-10-25 Tue wms - initial version
//
// INPUTS:
//    r7 = fft data object
//    r8 = &$filter_bank_analysis_data_block;
//
// OUTPUTS:
//    None. See description for information.
//
// TRASHED REGISTERS:
//    Everything
//
// CPU USAGE:
//
// NOTES:
// *****************************************************************************

#endif   //_FILTER_BANK_LIB_H
