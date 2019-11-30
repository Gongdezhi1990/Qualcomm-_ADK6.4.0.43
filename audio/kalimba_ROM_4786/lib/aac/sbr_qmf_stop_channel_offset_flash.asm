// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_SBR_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.sbr_qmf_stop_channel_rows_packed
//
// DESCRIPTION:
//    - Packed up qmf_stop_channel_rows that will go into flash
//
// *****************************************************************************

// SBR QMF Stop channel offset table row 0
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_96000[]  =
      0, 2, 4, 6, 8, 11, 14, 18, 22, 26, 31, 37, 44, 51;
// SBR QMF Stop channel offset table row 1
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_88200[]  =
      0, 2, 4, 6, 8, 11, 14, 18, 22, 26, 31, 36, 42, 49;
// SBR QMF Stop channel offset table row 2
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_64000[]  =
      0, 2, 4, 6, 8, 11, 14, 17, 21, 25, 29, 34, 39, 44;
// SBR QMF Stop channel offset table row 3
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_48000[]  =
      0, 2, 4, 6, 8, 11, 14, 17, 20, 24, 28, 33, 38, 43;
// SBR QMF Stop channel offset table row 4
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_44100[]  =
      0, 2, 4, 6, 8, 11, 14, 17, 20, 24, 28, 32, 36, 41;
// SBR QMF Stop channel offset table rows 5 & 6
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_24000_to_32000[]  =
      0, 2, 4, 6, 8, 10, 12, 14, 17, 20, 23, 26, 29, 32;
// SBR QMF Stop channel offset table row 7
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_22050[]  =
      0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 20, 23, 26, 29;
// SBR QMF Stop channel offset table row 8
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_16000[]  =
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16;
// SBR QMF Stop channel offset table row 9
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_12000[]  =
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;
// SBR QMF Stop channel offset table row 10
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_11025[]  =
      0, -1, -2, -3, -4, -5, -6, -6, -6, -6, -6, -6, -6, -6;
// SBR QMF Stop channel offset table row 11
.VAR/DMCONST_WINDOWED16 sbr_qmf_stop_channel_offset_8000[]  =
      0, -3, -6, -9, -12, -15, -18, -20, -22, -24, -26, -28, -30, -32;


// SBR offset table row 0
.VAR/DMCONST_WINDOWED16 sbr_offset_fs_sbr_16000[]  =
      -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7;
// SBR offset table row 1
.VAR/DMCONST_WINDOWED16 sbr_offset_fs_sbr_22050[]  =
      -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13;
// SBR offset table row 2
.VAR/DMCONST_WINDOWED16 sbr_offset_fs_sbr_24000[]  =
      -5, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16;
// SBR offset table row 3
.VAR/DMCONST_WINDOWED16 sbr_offset_fs_sbr_32000[]  =
      -6, -4, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16;
// SBR offset table row 4
.VAR/DMCONST_WINDOWED16 sbr_offset_fs_sbr_44100_to_64000[]  =
      -4, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20;
// SBR offset table row 5
.VAR/DMCONST_WINDOWED16 sbr_offset_fs_sbr_gt_64000[]  =
      -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20, 24;

#endif
