/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#ifndef AAC_MEM_HEADER_INCLUDED
#define AAC_MEM_HEADER_INCLUDED

     //sqrt(3)/2 used by pns decode
    .CONST  $aac.SQRT_THREE_OVER_TWO                  0.8660254038;

    // size of the huffman tables once unpacked
    .CONST  $aac.HUFFMAN_bookscl_SIZE                 120;
    .CONST  $aac.HUFFMAN_book1_SIZE                   80;
    .CONST  $aac.HUFFMAN_book2_SIZE                   80;
    .CONST  $aac.HUFFMAN_book3_SIZE                   80;
    .CONST  $aac.HUFFMAN_book4_SIZE                   80;
    .CONST  $aac.HUFFMAN_book5_SIZE                   80;
    .CONST  $aac.HUFFMAN_book6_SIZE                   80;
    .CONST  $aac.HUFFMAN_book7_SIZE                   64;
    .CONST  $aac.HUFFMAN_book8_SIZE                   64;
    .CONST  $aac.HUFFMAN_book9_SIZE                   168;
    .CONST  $aac.HUFFMAN_book10_SIZE                  168;
    .CONST  $aac.HUFFMAN_book11_SIZE                  288;

    .CONST  $aac.FFT_TWIDDLE_SIZE                     512;


    .CONST  $aac.mem.NUM_BYTES_AVAILABLE                                                  0*ADDR_PER_WORD;
    .CONST  $aac.mem.CONVERT_MONO_TO_STEREO_FIELD                                         1*ADDR_PER_WORD;
    //file formats
    .CONST  $aac.mem.READ_FRAME_FUNCTION                                                  2*ADDR_PER_WORD;
    .CONST  $aac.mem.SKIP_FUNCTION                                                        3*ADDR_PER_WORD;
    .CONST  $aac.mem.skip_amount_ms                                                       4*ADDR_PER_WORD;
    .CONST  $aac.mem.skip_amount_ls                                                       5*ADDR_PER_WORD;
    //header info
    .CONST  $aac.mem.FRAME_UNDERFLOW                                                      6*ADDR_PER_WORD;
    .CONST  $aac.mem.FRAME_VERSION                                                        7*ADDR_PER_WORD;
    .CONST  $aac.mem.FRAME_LENGTH                                                         8*ADDR_PER_WORD;
    .CONST  $aac.mem.NO_RAW_DATA_BLOCKS_IN_FRAME                                          9*ADDR_PER_WORD;
    .CONST  $aac.mem.PROTECTION_ABSENT                                                    10*ADDR_PER_WORD;
    .CONST  $aac.mem.ID3_SKIP_NUM_BYTES                                                   11*ADDR_PER_WORD;
    .CONST  $aac.mem.FRAME_CORRUPT                                                        12*ADDR_PER_WORD;
    .CONST  $aac.mem.POSSIBLE_FRAME_CORRUPTION                                            13*ADDR_PER_WORD;
    //DEBUG_AACDEC
    .CONST  $aac.mem.FRAME_CORRUPT_ERRORS                                                 14*ADDR_PER_WORD;
    .CONST  $aac.mem.LOSTSYNC_ERRORS                                                      15*ADDR_PER_WORD;
    .CONST  $aac.mem.FRAME_COUNT                                                          16*ADDR_PER_WORD;
    .CONST  $aac.mem.FRAME_GARBAGE_ERRORS                                                 17*ADDR_PER_WORD;
    //Lookup table to slightly speedup generating a bit mask
    .CONST  $aac.mem.GET_BITPOS                                                           18*ADDR_PER_WORD;
    .CONST  $aac.mem.READ_BIT_COUNT                                                       19*ADDR_PER_WORD;
    .CONST  $aac.mem.FRAME_NUM_BITS_AVAIL                                                 20*ADDR_PER_WORD;
    .CONST  $aac.mem.GETBITS_SAVED_I0                                                     21*ADDR_PER_WORD;
    .CONST  $aac.mem.GETBITS_SAVED_L0                                                     22*ADDR_PER_WORD;
    .CONST  $aac.mem.GETBITS_SAVED_BITPOS                                                 23*ADDR_PER_WORD;
    .CONST  $aac.mem.SF_INDEX_FIELD                                                       24*ADDR_PER_WORD;
    //channel configuration
    .CONST  $aac.mem.CHANNEL_CONFIGURATION_FIELD                                          25*ADDR_PER_WORD;
    //object type
    .CONST  $aac.mem.AUDIO_OBJECT_TYPE_FIELD                                              26*ADDR_PER_WORD;
    .CONST  $aac.mem.EXTENSION_AUDIO_OBJECT_TYPE_FIELD                                    27*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_PRESENT_FIELD                                                    28*ADDR_PER_WORD;
    //latm header fields
    .CONST  $aac.mem.latm.AUDIO_MUX_VERSION                                               29*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.AUDIO_MUX_VERSION_A                                             30*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.MUX_SLOT_LENGTH_BYTES                                           31*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.CURRENT_SUBFRAME                                                32*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.NUM_SUBFRAMES                                                   33*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.PREVBITPOS                                                      34*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.TARABUFFERFULLNESSS                                             35*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.ASC_LEN                                                         36*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.LATM_BUFFER_FULLNESS                                            37*ADDR_PER_WORD;
    .CONST  $aac.mem.latm.OTHER_DATA_LEN_BITS                                             38*ADDR_PER_WORD;
    /*MP4 data*/
    .CONST  $aac.mem.channel_count                                                        39*ADDR_PER_WORD;
    .CONST  $aac.mem.MP4_MOOV_ATOM_SIZE_MS                                                40*ADDR_PER_WORD;
    .CONST  $aac.mem.MP4_MOOV_ATOM_SIZE_LS                                                41*ADDR_PER_WORD;
    .CONST  $aac.mem.MP4_SEQUENCE_FLAGS_INITIALISED                                       42*ADDR_PER_WORD;
    .CONST  $aac.mem.MP4_DISCARD_AMOUNT_MS                                                43*ADDR_PER_WORD;
    .CONST  $aac.mem.MP4_DISCARD_AMOUNT_LS                                                44*ADDR_PER_WORD;
    .CONST  $aac.mem.FOUND_FIRST_MDAT                                                     45*ADDR_PER_WORD;
    .CONST  $aac.mem.FOUND_MOOV                                                           46*ADDR_PER_WORD;
    // calculated using STSZ/STZ2 atom (either product/sum of 32 bit values)
    // 64 bits. only 16 bits of mdat_size[0] are used
    .CONST  $aac.mem.MDAT_SIZE                                                            47*ADDR_PER_WORD; // [3]
    .CONST  $aac.mem.SAMPLE_COUNT                                                         50*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.MDAT_PROCESSED                                                       52*ADDR_PER_WORD;
    .CONST  $aac.mem.TEMP_BIT_COUNT                                                       53*ADDR_PER_WORD; // used to calc bytes read from mdat
    //Fast Forward/Fast Rewind related parameters
    //position of STSZ in the file
    .CONST  $aac.mem.STSZ_OFFSET                                                          54*ADDR_PER_WORD; // [2]
    //position of STSS in the file
    .CONST  $aac.mem.STSS_OFFSET                                                          56*ADDR_PER_WORD; // [2]
    //size of mdat to skip for fast fwd/rewind
    .CONST  $aac.mem.FF_REW_SKIP_AMOUNT                                                   58*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.MP4_FRAME_COUNT                                                      60*ADDR_PER_WORD;
    // status of mp4 FF/REW
    .CONST  $aac.mem.MP4_FF_REW_STATUS                                                    61*ADDR_PER_WORD;
    // offset of mdat from begining of the file
    .CONST  $aac.mem.MDAT_OFFSET                                                          62*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.MP4_FILE_OFFSET                                                      64*ADDR_PER_WORD; // [2]
    //Num samples to fast forward / rewind (if negative)
    .CONST  $aac.mem.FAST_FWD_SAMPLES_MS                                                  66*ADDR_PER_WORD;
    .CONST  $aac.mem.FAST_FWD_SAMPLES_LS                                                  67*ADDR_PER_WORD;
    .CONST  $aac.mem.AVG_BIT_RATE                                                         68*ADDR_PER_WORD;
    //flags to indicate whether in a particular routine incase frame_underflow occurs
    //and need to resume with it when more data available
    .CONST  $aac.mem.MP4_DECODING_STARTED                                                 69*ADDR_PER_WORD;
    .CONST  $aac.mem.MP4_HEADER_PARSED                                                    70*ADDR_PER_WORD;
    .CONST  $aac.mem.MP4_IN_MOOV                                                          71*ADDR_PER_WORD;
    .CONST  $aac.mem.MP4_IN_DISCARD_ATOM_DATA                                             72*ADDR_PER_WORD;
    //LTP data
    .CONST  $aac.mem.MDCT_INFORMATION                                                     73*ADDR_PER_WORD; // [5] $aacdec.mdct.STRUC_SIZE
    .CONST  $aac.mem.IMDCT_INFO                                                           78*ADDR_PER_WORD; // [5] $aacdec.imdct.STRUC_SIZE
     //HUFFMAN data
    .CONST  $aac.mem.AMOUNT_UNPACKED                                                      83*ADDR_PER_WORD;
    //ICS INFO data
    .CONST  $aac.mem.COMMON_WINDOW                                                        84*ADDR_PER_WORD;
    .CONST  $aac.mem.CURRENT_ICS_PTR                                                      85*ADDR_PER_WORD;
    .CONST  $aac.mem.CURRENT_SPEC_PTR                                                     86*ADDR_PER_WORD;
    .CONST  $aac.mem.CURRENT_CHANNEL                                                      87*ADDR_PER_WORD;
    .CONST  $aac.mem.ICS_LEFT                                                             88*ADDR_PER_WORD;  // [42] ics.STRUC_SIZE
    .CONST  $aac.mem.ICS_RIGHT                                                            130*ADDR_PER_WORD; // [42] ics.STRUC_SIZE
    //MISC data
    // currently we only allow decoding of stereo (1 CPE) and mono (1 SCE)
    .CONST  $aac.mem.NUM_SCEs                                                             172*ADDR_PER_WORD;
    .CONST  $aac.mem.NUM_CPEs                                                             173*ADDR_PER_WORD;
    //temporary data used internally by functions
    .CONST  $aac.mem.TMP                                                                  174*ADDR_PER_WORD; // [40]
    //copy of the pointer to the codec structure
    .CONST  $aac.mem.codec_struc                                                          214*ADDR_PER_WORD;
    //Flag indicating if ics_ics_info has been called successfully
    .CONST  $aac.mem.ICS_INFO_DONE_FIELD                                                  215*ADDR_PER_WORD;
    // spec must be scaled up by factor 2 either in tns_encdec or in overlap_add.
    // spec_blksigndet holds the amount of headroom in the spec variable
    // (calculated using BLKSIGNDET) and whether the scaling has been done yet.
    // spec_blksigndet = [blksigndet, upscaled]
    .CONST  $aac.mem.LEFT_SPEC_BLKSIGNDET                                                 216*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.RIGHT_SPEC_BLKSIGNDET                                                218*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR                                          220*ADDR_PER_WORD;
    .CONST  $aac.mem.TEMP_I5                                                              221*ADDR_PER_WORD;
    .CONST  $aac.mem.saved_I0                                                             222*ADDR_PER_WORD;
    .CONST  $aac.mem.saved_bitpos                                                         223*ADDR_PER_WORD;

    // Frame Buffer pointers
    .CONST  $aac.mem.FRAME_MEM_POOL_END_PTR                                               224*ADDR_PER_WORD;
    .CONST  $aac.mem.TMP_MEM_POOL_END_PTR                                                 225*ADDR_PER_WORD;
    .CONST  $aac.mem.BUF_LEFT_PTR                                                         226*ADDR_PER_WORD;
    .CONST  $aac.mem.BUF_RIGHT_PTR                                                        227*ADDR_PER_WORD;
    .CONST  $aac.mem.OVERLAP_ADD_LEFT_PTR                                                 228*ADDR_PER_WORD;
    .CONST  $aac.mem.OVERLAP_ADD_RIGHT_PTR                                                229*ADDR_PER_WORD;
    .CONST  $aac.mem.FRAME_MEM_POOL_PTR                                                   230*ADDR_PER_WORD;
    .CONST  $aac.mem.TMP_MEM_POOL_PTR                                                     231*ADDR_PER_WORD;
    .CONST  $aac.mem.TNS_FIR_INPUT_HISTORY_PTR                                            232*ADDR_PER_WORD;
    .CONST  $aac.mem.BITMASK_LOOKUP_FIELD                                                 233*ADDR_PER_WORD;
    .CONST  $aac.mem.WRITE_BYTEPOS                                                        234*ADDR_PER_WORD;
    .CONST  $aac.mem.PNS_RAND_NUM                                                         235*ADDR_PER_WORD;
    .CONST  $aac.mem.PREVIOUS_WINDOW_SHAPE                                                236*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.PREVIOUS_WINDOW_SEQUENCE                                             238*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SWB_OFFSET_FIELD                                                     240*ADDR_PER_WORD; // [52]
    .CONST  $aac.mem.TWO2QTRX_LOOKUP_FIELD                                                292*ADDR_PER_WORD;
    .CONST  $aac.mem.X43_LOOKUP1_FIELD                                                    293*ADDR_PER_WORD;
    .CONST  $aac.mem.X43_LOOKUP2_FIELD                                                    294*ADDR_PER_WORD;
    .CONST  $aac.mem.X43_LOOKUP32_FIELD                                                   295*ADDR_PER_WORD;
    .CONST  $aac.mem.TNS_MAX_SFB_LONG_TABLE_FIELD                                         296*ADDR_PER_WORD;
    .CONST  $aac.mem.TNS_LOOKUP_COEFS_FIELD                                               297*ADDR_PER_WORD;
    .CONST  $aac.mem.SAMPLE_RATE_TAGS_FIELD                                               298*ADDR_PER_WORD;
    .CONST  $aac.mem.LTP_COEFS_FIELD                                                      299*ADDR_PER_WORD;
    .CONST  $aac.mem.SIN2048_COEFS_FIELD                                                  300*ADDR_PER_WORD;
    .CONST  $aac.mem.SIN256_COEFS_FIELD                                                   301*ADDR_PER_WORD;
    .CONST  $aac.mem.KAISER2048_COEFS_FIELD                                               302*ADDR_PER_WORD;
    .CONST  $aac.mem.KAISER256_COEFS_FIELD                                                303*ADDR_PER_WORD;
    .CONST  $aac.mem.HUFFMAN_OFFSETS_FIELD                                                304*ADDR_PER_WORD;
    .CONST  $aac.mem.SYNTATIC_ELEMENT_FUNC_TABLE                                          305*ADDR_PER_WORD; // [8]
    .CONST  $aac.mem.READ_FRAME_FUNC_TABLE                                                313*ADDR_PER_WORD; // [3]
    .CONST  $aac.mem.mp4_ff_rew_state                                                     316*ADDR_PER_WORD;
    .CONST  $aac.mem.moov_size_lo                                                         317*ADDR_PER_WORD;
    .CONST  $aac.mem.moov_size_hi                                                         318*ADDR_PER_WORD;
    .CONST  $aac.mem.huffman_cb_table                                                     319*ADDR_PER_WORD; // [12]
    .CONST  $aac.mem.huffman_packed_list                                                  331*ADDR_PER_WORD; // [12]
    .CONST  $aac.mem.huffman_table_sizes_div_2                                            343*ADDR_PER_WORD; // [12]
    .CONST  $aac.mem.set_mp4_file_type_message_struc                                      355*ADDR_PER_WORD; // [4] $message.STRUC_SIZE
    .CONST  $aac.mem.saved_current_subframe                                               359*ADDR_PER_WORD;
    .CONST  $aac.mem.sin_const_ptr                                                        360*ADDR_PER_WORD;
    .CONST  $aac.mem.sin_const_iptr                                                       361*ADDR_PER_WORD;
    .CONST  $aac.mem.FFT_TABLE_INIT_DONE_FIELD                                            362*ADDR_PER_WORD;
#ifndef AACDEC_SBR_ADDITIONS
    .CONST  $aac.mem.STRUC_SIZE                                                           363;
#else
    .CONST  $aac.mem.SBR_limiter_band_g_boost_mantissa                                    363*ADDR_PER_WORD; // [6]
    .CONST  $aac.mem.SBR_limiter_band_g_boost_exponent                                    369*ADDR_PER_WORD; // [6]
    .CONST  $aac.mem.SBR_in_synth                                                         375*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_in_synth_loops                                                   376*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_dct_dst                                                          377*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_fft_pointer_struct                                               378*ADDR_PER_WORD; // [3] [$fft.STRUC_SIZE];
    .CONST  $aac.mem.SBR_x_input_buffer_write_pointers                                    381*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_v_cbuffer_struc_address                                          383*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_v_left_cbuffer_struc                                             385*ADDR_PER_WORD; // [14] [max of $cbuffer.STRUC_SIZE]
    .CONST  $aac.mem.SBR_v_right_cbuffer_struc                                            399*ADDR_PER_WORD; // [14] [max of $cbuffer.STRUC_SIZE]
    .CONST  $aac.mem.SBR_limiter_bands_compare_ptr                                        413*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_phi_re_sin_ptr                                                   414*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_phi_im_sin_ptr                                                   415*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_h_smooth_ptr                                                     416*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_goal_sb_tab_ptr                                                  417*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_log2Table_ptr                                                    418*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_fscale_gt_zero_temp_1_ptr                                        419*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_fscale_gt_zero_temp_2_ptr                                        420*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_startMinTable_ptr                                                421*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_stopMinTable_ptr                                                 422*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_offsetIndexTable_ptr                                             423*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_E_pan_tab_ptr                                                    424*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_one_over_x_ptr                                                   425*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_pow2_table_ptr                                                   426*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_qmf_stop_channel_offset_ptr                                      427*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_offset_ptr                                                       428*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_bs_frame_class                                                   429*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_bs_num_env                                                       431*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_bs_pointer                                                       433*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_bs_invf_mode                                                     435*ADDR_PER_WORD; // [10]
    .CONST  $aac.mem.SBR_bs_add_harmonic_flag                                             445*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_t_E                                                              447*ADDR_PER_WORD; // [12]
    .CONST  $aac.mem.SBR_t_Q                                                              459*ADDR_PER_WORD; // [6]
    .CONST  $aac.mem.SBR_E_orig_mantissa_base_ptr                                         465*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_Q_orig                                                           467*ADDR_PER_WORD; // [20]
    .CONST  $aac.mem.SBR_Q_orig2                                                          487*ADDR_PER_WORD; // [20]
    .CONST  $aac.mem.SBR_bs_transient_position                                            507*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_x_real_ptr                                                       509*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_x_imag_ptr                                                       510*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_v_buffer_right_ptr                                               511*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_X_sbr_other_real_ptr                                             512*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_X_sbr_other_imag_ptr                                             513*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_synth_temp_ptr                                                   514*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_synthesis_post_process_real_ptr                                  515*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_synthesis_post_process_imag_ptr                                  516*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_qmf_filterbank_window_ptr                                        517*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_synthesis_pre_process_real_ptr                                   518*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_synthesis_pre_process_imag_ptr                                   519*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_est_curr_env_one_over_div_ptr                                    520*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_x_input_buffer_left_ptr                                          521*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_x_input_buffer_right_ptr                                         522*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_dct4_64_table_ptr                                                523*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_temp_1_ptr                                                       524*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_temp_2_ptr                                                       525*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_temp_3_ptr                                                       526*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_temp_4_ptr                                                       527*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_info_ptr                                                         528*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_x_input_buffer_write_base_pointers                               529*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.SBR_numTimeSlotsRate                                                 531*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_numTimeSlotsRate_adjusted                                        532*ADDR_PER_WORD; // has the +6 adjustment in non ELD mode
    .CONST  $aac.mem.SBR_X_2env_real_ptr                                                  533*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_X_2env_imag_ptr                                                  534*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_X_curr_real_ptr                                                  535*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_X_curr_imag_ptr                                                  536*ADDR_PER_WORD;
    .CONST  $aac.mem.SBR_v_buffer_left_ptr                                                537*ADDR_PER_WORD;

#ifndef AACDEC_PS_ADDITIONS
    .CONST  $aac.mem.STRUC_SIZE                                                           538;
#else
    .CONST  $aac.mem.PS_num_sub_subbands_per_hybrid_qmf_subband_ptr                       538*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_hybrid_qmf_sub_subband_offset_ptr                                 539*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_hybrid_type_b_fir_filter_coefficients_ptr                         540*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_nr_par_table_ptr                                                  541*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_nr_ipdopd_par_tab_ptr                                             542*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_num_env_tab_ptr                                                   543*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_map_freq_bands_to_20_par_bands_table_ptr                          544*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_map_freq_bands_to_20_par_bands_x_num_samples_per_frame_table_ptr  545*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_frequency_border_table_20_par_bands_ptr                           546*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_iid_coarse_resolution_scale_factor_table_ptr                      547*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_iid_fine_resolution_scale_factor_table_ptr                        548*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_cos_alpha_table_ptr                                               549*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_sin_alpha_table_ptr                                               550*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_alpha_angle_table_ptr                                             551*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_present                                                           552*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_X_hybrid_real_address                                             553*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.PS_X_hybrid_imag_address                                             555*ADDR_PER_WORD; // [2]
    .CONST  $aac.mem.PS_hybrid_type_a_ifft_struc                                          557*ADDR_PER_WORD; // [3]
    .CONST  $aac.mem.PS_iid_index_prev                                                    560*ADDR_PER_WORD; // [34]
    .CONST  $aac.mem.PS_icc_index_prev                                                    594*ADDR_PER_WORD; // [34]
    .CONST  $aac.mem.PS_h11_previous_envelope                                             628*ADDR_PER_WORD; // [22]
    .CONST  $aac.mem.PS_h21_previous_envelope                                             650*ADDR_PER_WORD; // [22]
    .CONST  $aac.mem.PS_h12_previous_envelope                                             672*ADDR_PER_WORD; // [22]
    .CONST  $aac.mem.PS_h22_previous_envelope                                             694*ADDR_PER_WORD; // [22]
    .CONST  $aac.mem.PS_time_history_real                                                 716*ADDR_PER_WORD; // [36]
    .CONST  $aac.mem.PS_time_history_imag                                                 752*ADDR_PER_WORD; // [36]
    .CONST  $aac.mem.PS_power_peak_decay_nrg_prev                                         788*ADDR_PER_WORD; // [40] // block of 20 mantissas, then block of 20 exponents
    .CONST  $aac.mem.PS_power_smoothed_peak_decay_diff_nrg_prev                           828*ADDR_PER_WORD; // [40] // block of 20 mantissas, then block of 20 exponents
    .CONST  $aac.mem.PS_smoothed_input_power_prev                                         868*ADDR_PER_WORD; // [40] // block of 20 mantissas, then block of 20 exponents
    .CONST  $aac.mem.PS_prev_frame_last_two_hybrid_samples_real                           908*ADDR_PER_WORD; // [40]
    .CONST  $aac.mem.PS_prev_frame_last_two_hybrid_samples_imag                           948*ADDR_PER_WORD; // [40]
    .CONST  $aac.mem.PS_prev_frame_last_two_qmf_samples_real                              988*ADDR_PER_WORD; // [40]
    .CONST  $aac.mem.PS_prev_frame_last_two_qmf_samples_imag                             1028*ADDR_PER_WORD; // [40]
    .CONST  $aac.mem.PS_short_delay_band_buffer_real                                     1068*ADDR_PER_WORD; // [30]
    .CONST  $aac.mem.PS_short_delay_band_buffer_imag                                     1098*ADDR_PER_WORD; // [30]
    .CONST  $aac.mem.PS_info_ptr                                                         1128*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_fmp_remains_ptr                                                  1129*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_hybrid_allpass_feedback_buffer_ptr                               1130*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_qmf_allpass_feedback_buffer_ptr                                  1131*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_hybrid_type_a_fir_filter_input_buffer_ptr                        1132*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_long_delay_band_buffer_real_ptr                                  1133*ADDR_PER_WORD;
    .CONST  $aac.mem.PS_long_delay_band_buffer_imag_ptr                                  1134*ADDR_PER_WORD;


#ifndef AACDEC_ELD_ADDITIONS
    .CONST  $aac.mem.STRUC_SIZE                                                          1135;
#else
    .CONST  $aac.mem.ELD_temp_u_ptr                                                      1135*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_synthesis_temp1_ptr                                             1136*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_synthesis_temp2_ptr                                             1137*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_synthesis_temp3_ptr                                             1138*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_synthesis_g_w_buffer_ptr                                        1139*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_frame_length_flag                                               1140*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sbr_sampling_rate                                               1141*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sbr_crc_flag                                                    1142*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_delay_shift                                                     1143*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sbr_numTimeSlots                                                1144*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_twiddle_tab_ptr                                                 1145*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_win_coefs_ptr                                                   1146*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sin_const_ptr                                                   1147*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sbr_temp_5_ptr                                                  1148*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sbr_temp_6_ptr                                                  1149*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sbr_temp_7_ptr                                                  1150*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sbr_temp_8_ptr                                                  1151*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_dct4_pre_cos_ptr                                                1152*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_dct4_pre_sin_ptr                                                1153*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_dct4_post_cos_ptr                                               1154*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_dct4_post_sin_ptr                                               1155*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_dct4_pre_cos_synthesis_ptr                                      1156*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_dct4_pre_sin_synthesis_ptr                                      1157*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_dct4_post_cos_synthesis_ptr                                     1158*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_dct4_post_sin_synthesis_ptr                                     1159*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_qmf_filterbank_window_ptr                                       1160*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_qmf_filterbank_window_synthesis_ptr                             1161*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_sbr_present                                                     1162*ADDR_PER_WORD;
    .CONST  $aac.mem.ELD_ifft_re_ptr                                                     1163*ADDR_PER_WORD;

    .CONST  $aac.mem.STRUC_SIZE                                                          1164;
#endif // AACDEC_ELD_ADDITIONS
#endif // AACDEC_PS_ADDITIONS
#endif // AACDEC_SBR_ADDITIONS

    .CONST           $aac.dummy_decode.CODEC_STRUC                                       0*ADDR_PER_WORD;    // I
    .CONST           $aac.dummy_decode.IN_CBUFFER                                        1*ADDR_PER_WORD;    // I
    .CONST           $aac.dummy_decode.PAYLOAD_SIZE                                      2*ADDR_PER_WORD;    // I
    .CONST           $aac.dummy_decode.FRAME_DEC_STRUC                                   3*ADDR_PER_WORD;    // O
    .CONST           $aac.dummy_decode.GET_BITPOS                                        4*ADDR_PER_WORD;    // I/O
    .CONST           $aac.dummy_decode.CBUFF_CLONE                                       5*ADDR_PER_WORD;    // Status
    .CONST           $aac.dummy_decode.PAYLOAD_LEFT                                      6*ADDR_PER_WORD;    // Status
    .CONST           $aac.dummy_decode.STRUC_SIZE                                        7;                  // SIZE in words

    // ASM definition of RTP_FRAME_DECODE_DATA structure
    .CONST           $aac.frame_decode_data.valid                                        0*ADDR_PER_WORD;
    .CONST           $aac.frame_decode_data.frame_length                                 1*ADDR_PER_WORD;
    .CONST           $aac.frame_decode_data.frame_samples                                2*ADDR_PER_WORD;
    .CONST           $aac.frame_decode_data.nr_of_frames                                 3*ADDR_PER_WORD;
    .CONST           $aac.frame_decode_data.STRUC_SIZE                                   4;                  // SIZE in words

#endif
