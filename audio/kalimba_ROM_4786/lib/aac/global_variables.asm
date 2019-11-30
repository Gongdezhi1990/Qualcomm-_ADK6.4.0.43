// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.variables
//
// DESCRIPTION:
//    Variables
//
// *****************************************************************************
.MODULE $aacdec;
   .DATASEGMENT DM;

   // Lookup table to slightly speedup generating a bit mask
   .VAR/DMCONST  bitmask_lookup[17] =
                 0x000000,  0x000001,  0x000003,  0x000007,  0x00000F,  0x00001F,  0x00003F,
                 0x00007F,  0x0000FF,  0x0001FF,  0x0003FF,  0x0007FF,  0x000FFF,  0x001FFF,
                 0x003FFF,  0x007FFF,  0x00FFFF;


   .VAR/DMCONST  sin_const_mdct[4] = 0.9999952938,  0.0030679567,  0.9999999264,  0.0003834951;

   .VAR/DMCONST  sin_const_imdct[18] = 0.9996988186,  0.0245412285,  0.9999952938,  0.0030679567,
                                       0.9999999264,  0.0003834951,  0.9227011283,  0.3855160538,
                                       0.9237327073,  0.3830377075,  0.7049340803,  0.7092728264,
                                       0.7068355571,  0.7073779012,  0.3798472089,  0.9250492407,
                                       0.3823291008,  0.9240262218;


   // **************************************************************************
   //                          DEQUANTIZATION data
   // **************************************************************************
  .VAR/DMCONST  two2qtrx_lookup[4] = 0.500000000,     // 2^(-4/4)
                                     0.594603557,     // 2^(-3/4)
                                     0.707106781,     // 2^(-2/4)
                                     0.840896415;     // 2^(-1/4)

  .VAR/DMCONST  x43_lookup1[36] =
          19,                  17,                  16,                  15,
          13,                  12,                  11,                  9,
          8,                  -0.035037674009800,  -0.073515585158020,  -0.058341562747955,
         -0.046293357852846,  -0.073446875438094,  -0.058232603594661,  -0.046120746526867,
         -0.072901102714241,  -0.057373376097530,   0.279823091812432,   0.384265203494579,
          0.305011543445289,   0.242119652219117,   0.384442062117159,   0.305292369332165,
          0.242565686348826,   0.385859712492675,   0.307547663804144,   0.560124327428639,
          0.954673318192363,   0.757699610199779,   0.601346732582897,   0.954451961908489,
          0.757348355371505,   0.600789554417133,   0.952685629483312,   0.754552931990474;

   .VAR/DMCONST  x43_lookup2[36]=
          0,                   19,                  16,                  15,
          14,                  12,                  11,                  10,
          8,                   0,                  -0.029032050166279,  -0.092162329237908,
         -0.073135506361723,  -0.058025822043419,  -0.092040891759098,  -0.072943015489727,
         -0.057721149176359,  -0.091079273726791,   0,                   0.076640653889626,
          0.243330279365182,   0.193149421829730,   0.153331456240267,   0.243489523883909,
          0.193402307573706,   0.153733205981553,   0.244766994379461,   0,
          0.267386461142451,   0.848879184108227,   0.673724243883044,   0.534685116261244,
          0.848600490484387,   0.673282076604664,   0.533983958419412,   0.846379224210978;


   .VAR/DMCONST x43_lookup32_flash[64] =
          0,            1,            2,            3,            3,            4,            4,            4,
          5,            5,            5,            5,            5,            5,            6,            6,
          6,            6,            6,            6,            6,            6,            6,            7,
          7,            7,            7,            7,            7,            7,            7,            7,
          0.0000000000, 0.5000000000, 0.6299605248, 0.5408435888, 0.7937005260, 0.5343674831, 0.6814202224, 0.8369073924,
          0.5000000000, 0.5850235755, 0.6732608406, 0.7644931562, 0.8585356819, 0.9552297168, 0.5272186203, 0.5780184548,
          0.6299605248, 0.6829966726, 0.7370835170, 0.7921817396, 0.8482555053, 0.9052719953, 0.9632010199, 0.5110073481,
          0.5408435888, 0.5710972147, 0.6017570137, 0.6328125000, 0.6642538374, 0.6960717808, 0.7282576184, 0.7608031267;





   // **************************************************************************
   //                               TNS data
   // **************************************************************************

   // TNS structure
   .VAR/DMCONST tns_max_sfb_long_table[9] =
          0x000028, 0x00002A, 0x000033, 0x00002E, 0x00002E, 0x00002A, 0x00002A, 0x00002A, 0x000027;

   .VAR/DMCONST tns_lookup_coefs[24] =
                          0,   0.433883739117558,   0.781831482468030,   0.974927912181824,
         -0.984807753012208,  -0.866025403784439,  -0.642787609686539,  -0.342020143325669,
                          0,   0.207911690817759,   0.406736643075800,   0.587785252292473,
          0.743144825477394,   0.866025403784439,   0.951056516295154,   0.994521895368273,
         -0.995734176295034,  -0.961825643172819,  -0.895163291355062,  -0.798017227280239,
         -0.673695643646557,  -0.526432162877356,  -0.361241666187153,  -0.183749517816570;




   .VAR/DMCONST sample_rate_tags[12] =
         0x017700, 0x015888, 0x00FA00, 0x00BB80, 0x00AC44, 0x007D00,
         0x005DC0, 0x005622, 0x003E80, 0x002EE0, 0x002B11, 0x001F40;


   // **************************************************************************
   //                              LTP data
   // **************************************************************************
   .VAR/DMCONST ltp_coefs[8] =
         0.142707228660584,   0.174154043197632,   0.203251004219055,   0.227825999259949,
         0.246224999427795,   0.266973495483398,   0.298650264739990,   0.342383265495300;




   // **************************************************************************
   //                        WINDOWING data
   // **************************************************************************

   // Rotation matrix =   [ cos(pi/N)   -sin(pi/N)
   //                       sin(pi/N)    cos(pi/N) ]
   // just store cos(pi/N) and sin(pi/N)
   //
   // Starting vector =   [ cos(pi/N/2)
   //                       sin(pi/N/2) ]
   // N=2048

   .VAR/DMCONST sin2048_coefs[4] =  0.9999997059,  0.0007669903,  0.9999988235,  0.0015339802;
   .VAR/DMCONST sin256_coefs[4] =   0.9999811753,  0.0061358846,  0.9999247018,  0.0122715383;


   //  Kaiser window polynomial fit coefficients: generated with kaiser_generate.m
   .VAR/DMCONST  kaiser2048_coefs[36] =
       +0.0003828354,  +0.0224643906,  +0.0523265891,  +0.1343142927,  -0.0299551922,  +210,
       +0.1145978775,  +0.3115340074,  +0.2592987506,  -0.0108654896,  -0.0631660178,  +210,
       +0.5100967816,  +0.5758919995,  -0.0008818654,  -0.2141262944,  +0.0651965208,  +92,  // split at halfway point
       +0.5100967816,  +0.5758919995,  -0.0008818654,  -0.2141262944,  +0.0651965208,  +98,
       +0.8692721937,  +0.3290540106,  -0.2648256370,  +0.0320950729,  +0.0353308851,  +180,
       +0.9894981150,  +0.0517153546,  -0.0990314628,  +0.0869802760,  -0.0293442242,  +234;

   .VAR/DMCONST kaiser256_coefs[36] =
       +0.0000241752,  +0.0028029329,  +0.0073619623,  +0.0600559776,  +0.0457888540,  +26,
       +0.0592348859,  +0.2295280147,  +0.3056280595,  +0.1657036557,  -0.1665639444,  +28,
       +0.5075518468,  +0.6991855142,  +0.0209393097,  -0.4259068627,  +0.1667795857,  +10, // split at halfway point
       +0.5075518468,  +0.6991855142,  +0.0209393097,  -0.4259068627,  +0.1667795857,  +14,
       +0.9168486198,  +0.2972068181,  -0.3854267702,  +0.1926917615,  -0.0170969141,  +23,
       +0.9982646973,  +0.0125103377,  -0.0342462435,  +0.0414374941,  -0.0185043712,  +27;

   // **************************************************************************
   //                            HUFFMAN data
   // **************************************************************************

   // include the packed tables from another file
   #include "huffman_tables_packed.asm"

   .VAR/DMCONST huffman_offsets[12] = 0, -1, -1, 0, 0, -4, -4, 0, 0, 0, 0, 0;



   // **************************************************************************
   //                          MEMORY POOL data
   // **************************************************************************

/*
----------------------------------- Worst case memory pool usage -------------------------------


!!!!!!!!!!!!!!!!! NOTE THE PS INFORMATION IN HERE IS BASED UPON PRE CHANGELIST 184699 FILES !!!!!!!!!!!!!!!!

frame mem pool:                                                initial  max
----------------

   ltp_data.asm
      allocate left ltp structure                              0        42
      allocate right ltp structure                             42       84

   decode_cpe.asm
      allocate ms_used array                                   84       136

   huffman_unpack.asm
      possibly allocate scalefactor huffman tables                 120 words
      (only if spectral tables also unpacked)

   tns_data.asm
      allocate left tns structure                              136      240
      allocate right tns structure                             240      344

   huffman_unpack.asm
      possibly allocate and free spectral huffman tables           1232 words allocated and freed

   sbr_envelope.asm
      allocate memory for sbr_E_envelope at end of pool        344      664
      allocate and free two arrays for huffman tables          664      904

   sbr_noise.asm
      allocate and free two arrays for huffman tables          664      904

   sbr_envelope.asm
      allocate and free two arrays for huffman tables          664      904

   sbr_noise.asm
      allocate and free two arrays for huffman tables          664      904

   ps_data.asm
      allocate and free arrays for huffman tables              664      840

   tns_encdec.asm
      allocate memory for lpc coefficients                     664      688

   tns_encdec.asm
      allocate memory for lpc coefficients                     688      712

   tns_encdec.asm
      allocate memory for lpc coefficients                     712      736

   tns_encdec.asm
      allocate memory for lpc coefficients                     736      760

   FREE ALL (except sbr_E_envelope)

   sbr_limiter_frequency_table.asm
      allocate memory for sbr_F_table_lim                      320      468
      allocate and free limTable array                         468      568
      allocate and free patchBorders array                     468      532

   sbr_estimate_currant_envelope.asm
      allocate memory for sbr_E_curr                           468      713

   sbr_calculate_gain.asm
      allocate memory for lim_boost mantissas                  713      1268
      allocate and free memory for lim_boost exponents         1268     1379

   sbr_hf_assembly.asm
      free lim_boost mantissas and sbr_E_curr                  1268     1268

   ps_hybrid_analysis
      allocate and free something                              468      ????

   ps_decorrelate.asm
      allocate and free P array                                468      1748

   sbr_estimate_currant_envelope.asm
      allocate memory for sbr_E_curr                           468      713

   sbr_calculate_gain.asm
      allocate memory for lim_boost mantissas                  713      1268
      allocate and free memory for lim_boost exponents         1268     1379

   Free sbr_E_envelope memory                                  1268     1268
      (now sbr_E_orig_mantissa)

   sbr_hf_assembly.asm
      free lim_boost mantissas and sbr_E_curr                  948      948

   reconstruct_channels
      free  sbr_F_table_lim                                    148      148

   FREE ALL


   TOTALS:
   All ram - AAC                  440
   All ram - AAC+SBR             1379
   All ram - AAC+SBR+PS          1748
   Spectral data in flash        1576 (= 344 + 1232)
   Scalefactor data in flash     1696 (= 344 + 1232 + 120)

   Note: sbr_E_envelope is stored without using the allocate/free framework.
      This is why it can be kept passed 'free all's and freed at any point


tmp mem pool:                                                  pre      max
----------------

   program_element_config.asm
      allocate and free program_element_config array           0        6

   calc_sfb_and_wingroup.asm
      allocate left sect_sfb_offset array                      0        128

   section_data.asm
      allocate left sect_cb array                              128      248
      allocate left sect_start array                           248      368
      allocate left sect_end array                             368      488
      allocate left sfb_cb array                               488      608

   scalefactor_data.asm
      allocate left scalefactor array                          608      728

   pulse_data
      allocate left pulse_struc array                          728      740

   calc_sfb_and_wingroup.asm
      allocate right sect_sfb_offset array                     740      868

   section_data.asm
      allocate right sect_cb array                             868      988
      allocate right sect_start array                          988      1108
      allocate right sect_end array                            1108     1228
      allocate right sfb_cb array                              1228     1348

   scalefactor_data.asm
      allocate right scalefactor array                         1348     1468

   pulse_data.asm
      allocate right pulse_struc array                         1468     1480

   sbr_fmaster_table_calc_fscale_eq_zero.asm
      allocate and free sbr_cDk array                          1480     1608
      (same for _gt_zero.asm)

   sbr_envelope.asm
      allocate memory for sbr_E_envelope in middle of pool     1480     1800

   sbr_noise.asm
      allocate left sbr_Q_envelope                             1800     1810

   sbr_noise.asm
      allocate right sbr_Q_envelope                            1810     1820

   reorder_spec.asm
      allocate and free left temp array                        1820     2844

   reorder_spec.asm
      allocate and free right temp array                       1820     2844

   FREE ALL (except sbr_E_envelope)

   filterbank.asm
      allocate and free buf_left array                         320      1344

   filterbanks.asm
      allocate and free buf_right array                        320      1344

   FREE ALL (except sbr_E_envelope)

   sbr_analysis_filterbank.asm
      allocate X_sbr_shared                                    320      2368

   sbr_hf_assembly.asm
      allocate and free memory for filt arrays                 2368     2466

   ps_hybrid_type_a_filter.asm
      allocate and free temp_real array                        2368     2376

   ps_decorrelate.asm
      allocate and free gain_transient_ratio array             2368     3008

   Free X_sbr_shared                                           2368     2368

   FREE ALL (except sbr_E_envelope)

   sbr_analysis_filterbank.asm
      allocate X_sbr_shared                                    320      2368

   Free sbr_E_envelope memory                                  2368     2368
      (now sbr_E_orig_mantissa)

   sbr_hf_assembly.asm
      allocate and free memory for filt arrays                 2048     2146

   Free X_sbr_shared                                           2048     2048

   FREE ALL


   TOTALS:

   AAC            2504
   AAC+SBR        2844
   AAC+SBR+PS     3008

   Note: sbr_E_envelope is stored without using the allocate/free framework.
      This is why it can be kept passed 'free all's and freed at any point

*/

.ENDMODULE;
