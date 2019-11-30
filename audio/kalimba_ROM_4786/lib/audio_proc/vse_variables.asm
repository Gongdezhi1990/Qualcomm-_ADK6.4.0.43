// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#include "vse.h"

// *****************************************************************************
// MODULE:
//    $audio_proc.vse.variables
//
// DESCRIPTION:
//    Variables to be used in the VSE module
//
// *****************************************************************************
.MODULE $audio_proc.vse;
   .DATASEGMENT DM;

   .VAR/DMCONST $theta_virtual_60_ipsi_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.964525804116248 , -0.822629020581239 , 0.787154824697487;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_60_contra_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.368807529217085 , -0.226910745682077 , 0.787154824697487;    // b0 , b1 , a1    
   .VAR/DMCONST $theta_virtual_60_ipsi_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.973484848484848 , -0.867424242424242 , 0.840909090909091;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_60_contra_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.359848484848485 , -0.253787878787879 , 0.840909090909091;    // b0 , b1 , a1 
   .VAR/DMCONST $theta_virtual_60_ipsi_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.975480734863107 , -0.877403674315534 , 0.852884409178640;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_60_contra_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.357852598470227 , -0.259775537922654 , 0.852884409178640;    // b0 , b1 , a1 
   .VAR/DMCONST $theta_virtual_60_ipsi_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.986193293885602 , -0.930966469428008 , 0.917159763313609;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_60_contra_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.347140039447732 , -0.291913214990138 , 0.917159763313609;    // b0 , b1 , a1 
   .VAR/DMCONST $theta_virtual_60_ipsi_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.987272255000185 , -0.936361275000928 , 0.923633530001113;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_60_contra_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.346061078333148 , -0.295150098333890 , 0.923633530001113;    // b0 , b1 , a1 
   
   .VAR/DMCONST $theta_virtual_120_ipsi_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.950571105161553 , -0.836512279105868 , 0.787154824697487;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_120_contra_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.121182820137198 , -0.007123994081514 , 0.787154824697487;    // b0 , b1 , a1    
   .VAR/DMCONST $theta_virtual_120_ipsi_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.963044327416802 , -0.877791174268038 , 0.840909090909091;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_120_contra_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.108709597881949 , -0.023456444733185 , 0.840909090909091;    // b0 , b1 , a1  
   .VAR/DMCONST $theta_virtual_120_ipsi_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.965823098608012 , -0.886987242640294 , 0.852884409178640;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_120_contra_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.105930826690739 , -0.027094970723021 , 0.852884409178640;    // b0 , b1 , a1  
   .VAR/DMCONST $theta_virtual_120_ipsi_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.980737650217820 , -0.936345475797162 , 0.917159763313609;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_120_contra_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.091016275080931,  -0.046624100660273 , 0.917159763313609;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_120_ipsi_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.982239832954423 , -0.941316800963165 , 0.923633530001113;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_120_contra_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.089514092344328 , -0.048591060353070 , 0.923633530001113;    // b0 , b1 , a1  
   
   .VAR/DMCONST $theta_virtual_180_ipsi_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.946788706174372,  -0.840366118523115,  0.787154824697487;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_contra_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.053211293825628,   0.053211293825628,  0.787154824697487;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_ipsi_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.960227272727273,  -0.880681818181818,  0.840909090909091;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_contra_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.039772727272727,   0.039772727272727,  0.840909090909091;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_ipsi_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.963221102294660,  -0.889663306883980,  0.852884409178640;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_contra_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.036778897705340,   0.036778897705340,  0.852884409178640;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_ipsi_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.979289940828402,  -0.937869822485207,  0.917159763313609;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_contra_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.020710059171597,   0.020710059171597,  0.917159763313609;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_ipsi_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]      =  0.980908382500278,  -0.942725147500835,  0.923633530001113;    // b0 , b1 , a1
   .VAR/DMCONST $theta_virtual_180_contra_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.019091617499722,   0.019091617499722,  0.923633530001113;    // b0 , b1 , a1
   
   .VAR/DMCONST $theta_speaker_6_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]     =  0.955323139889622, -0.751987218709268, 0.796664078819646;        // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_6_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]     =  0.954041103096848, -0.802261836695077, 0.848220733598229;        // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_6_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]     =  0.953755960839316, -0.813443589161046, 0.859687628321731;        // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_6_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]     =  0.952228419225652, -0.873345591597492, 0.921117172371840;        // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_6_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]     =  0.952074837537262, -0.879368243019778, 0.927293405482516;        // b0 , b1 , a1 
   
   .VAR/DMCONST $theta_speaker_10_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.927746712108737, -0.730280300533623, 0.802533588424886;        // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_10_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.925734864900434, -0.778458863666274, 0.852723998765840;        // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_10_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.925287855599427, -0.789163586043088, 0.863875730443662;        // b0 , b1 , a1
   .VAR/DMCONST $theta_speaker_10_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.922895971657086, -0.846443050928097, 0.923547079271011;        // b0 , b1 , a1     
   .VAR/DMCONST $theta_speaker_10_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.922655748152651, -0.852195785642051, 0.929540037489400;        // b0 , b1 , a1 
   
   .VAR/DMCONST $theta_speaker_12_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.914574804609625,  -0.719911969995228,  0.805337165385603;      // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_12_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.912230891459199,  -0.767103249636145,  0.854872358176946;      // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_12_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.911710352432527,  -0.777583545276466,  0.865873192843939;      // b0 , b1 , a1
   .VAR/DMCONST $theta_speaker_12_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.908926573556061,  -0.833630881072127,  0.924704307516066;      // b0 , b1 , a1        
   .VAR/DMCONST $theta_speaker_12_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.908647135419853,  -0.839256961213239,  0.930609825793385;      // b0 , b1 , a1
   
   .VAR/DMCONST $theta_speaker_14_32000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.901794821246021,  -0.709852144431013,  0.808057323184992;      // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_14_44100_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.899138919850380,  -0.756094091692365,  0.856955171841985;      // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_14_48000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.898549370342517,  -0.766358748842417,  0.867809378499900;      // b0 , b1 , a1 
   .VAR/DMCONST $theta_speaker_14_88200_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.895398234456066,  -0.821223232785149,  0.925824998329083;      // b0 , b1 , a1    
   .VAR/DMCONST $theta_speaker_14_96000_coeff[$BIN_SYNTH_FILTER_COEFF_SIZE]    =  0.895082079048239,  -0.826727820312060,  0.931645741263822;      // b0 , b1 , a1 
   
   .VAR/DMCONST $dcb_coeffs_32000[3] = 0.998040349837247,  -0.998040349837247,  0.996080699674494;   // b0 , b1 , a1 
   .VAR/DMCONST $dcb_coeffs_44100[3] = 0.998577267514563,  -0.998577267514563,  0.997154535029127;   // b0 , b1 , a1 
   .VAR/DMCONST $dcb_coeffs_48000[3] = 0.998692713548301,  -0.998692713548301,  0.997385427096602;   // b0 , b1 , a1 
   .VAR/DMCONST $dcb_coeffs_88200[3] = 0.999288127716120,  -0.999288127716120,  0.998576255432240;   // b0 , b1 , a1 
   .VAR/DMCONST $dcb_coeffs_96000[3] = 0.999345929525232,  -0.999345929525232,  0.998691859050465;   // b0 , b1 , a1 
   
   .VAR/DMCONST $itf_coeffs_32000[2] = 0.469720637916390,  0.472964775891320;  // b0 , a1 
   .VAR/DMCONST $itf_coeffs_44100[2] = 0.361764251491776,  0.594093833719490;  // b0 , a1 
   .VAR/DMCONST $itf_coeffs_48000[2] = 0.337035490353857,  0.621839960068261;  // b0 , a1 
   .VAR/DMCONST $itf_coeffs_88200[2] = 0.198501055858478,  0.777278152128363;  // b0 , a1
   .VAR/DMCONST $itf_coeffs_96000[2] = 0.183903890883865,  0.793656440610369;  // b0 , a1 
   
   .VAR/DMCONST $lsf_coeffs_32000[5] = 0.504070762181798,  -0.980154334996137, 0.476839819323771,  0.980342947702851,  -0.480721968798854;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   .VAR/DMCONST $lsf_coeffs_44100[5] = 0.502950758747826,  -0.985636139580999, 0.483085672111731,  0.985735974781005,  -0.485936595659551;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   .VAR/DMCONST $lsf_coeffs_48000[5] = 0.502710394119551,  -0.986810494385746, 0.484438371493580,  0.986894861389104,  -0.487064398609773;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   .VAR/DMCONST $lsf_coeffs_88200[5] = 0.501473296248415,  -0.992842679107185, 0.491470161786758,  0.992867814015116,  -0.492918323127242;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   .VAR/DMCONST $lsf_coeffs_96000[5] = 0.501353433291104,  -0.993426067086503, 0.492157750308749,  0.993447295688247,  -0.493489954998109;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   
   .VAR/DMCONST $peak_coeffs_32000[5] = 0.778293135816175, -0.340715369243681, -0.337528890311464, 0.340715369243681,   0.059235754495289;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   .VAR/DMCONST $peak_coeffs_44100[5] = 0.743284552022244, -0.448871966055242, -0.232169840580267, 0.448871966055242,  -0.011114711441976;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   .VAR/DMCONST $peak_coeffs_48000[5] = 0.733576647094008, -0.475901255550590, -0.202953701928664, 0.475901255550590,  -0.030622945165343;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   .VAR/DMCONST $peak_coeffs_88200[5] = 0.664436495453904, -0.648856837057400,  0.005125000081983, 0.648856837057400,  -0.169561495535887;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   .VAR/DMCONST $peak_coeffs_96000[5] = 0.655417291632438, -0.669721340656182,  0.032268478651555, 0.669721340656182,  -0.187685770283993;     // b0/2 , b1/2 , b2/2 , a1/2 , a2/2
   
   .VAR/DMCONST $gdelay_32000[3] = 9,16,22;      // delay values @ 60,120,180
   .VAR/DMCONST $gdelay_44100[3] = 12,22,30;     // delay values @ 60,120,180
   .VAR/DMCONST $gdelay_48000[3] = 13,24,32;     // delay values @ 60,120,180
   .VAR/DMCONST $gdelay_88200[3] = 24,44,59;     // delay values @ 60,120,180
   .VAR/DMCONST $gdelay_96000[3] = 26,48,65;     // delay values @ 60,120,180
   
   .VAR/DMCONST $ipsi_coeffs_32000[3]           = &$theta_virtual_60_ipsi_32000_coeff , &$theta_virtual_120_ipsi_32000_coeff, &$theta_virtual_180_ipsi_32000_coeff;
   .VAR/DMCONST $contra_coeffs_32000[3]         = &$theta_virtual_60_contra_32000_coeff , &$theta_virtual_120_contra_32000_coeff, &$theta_virtual_180_contra_32000_coeff;
   .VAR/DMCONST $theta_speaker_coeffs_32000[4]  = &$theta_speaker_6_32000_coeff , &$theta_speaker_10_32000_coeff, &$theta_speaker_12_32000_coeff, &$theta_speaker_14_32000_coeff;
   .VAR/DMCONST $all_params_32000[8]            = &$ipsi_coeffs_32000, &$contra_coeffs_32000 ,&$theta_speaker_coeffs_32000 , &$gdelay_32000 , &$dcb_coeffs_32000 , &$itf_coeffs_32000, &$lsf_coeffs_32000 , &$peak_coeffs_32000;
   
   .VAR/DMCONST $ipsi_coeffs_44100[3]           = &$theta_virtual_60_ipsi_44100_coeff , &$theta_virtual_120_ipsi_44100_coeff, &$theta_virtual_180_ipsi_44100_coeff;
   .VAR/DMCONST $contra_coeffs_44100[3]         = &$theta_virtual_60_contra_44100_coeff , &$theta_virtual_120_contra_44100_coeff, &$theta_virtual_180_contra_44100_coeff;
   .VAR/DMCONST $theta_speaker_coeffs_44100[4]  = &$theta_speaker_6_44100_coeff , &$theta_speaker_10_44100_coeff, &$theta_speaker_12_44100_coeff, &$theta_speaker_14_44100_coeff;
   .VAR/DMCONST $all_params_44100[8]            = &$ipsi_coeffs_44100, &$contra_coeffs_44100 ,&$theta_speaker_coeffs_44100 , &$gdelay_44100 , &$dcb_coeffs_44100 , &$itf_coeffs_44100, &$lsf_coeffs_44100 , &$peak_coeffs_44100;
   
   .VAR/DMCONST $ipsi_coeffs_48000[3]           = &$theta_virtual_60_ipsi_48000_coeff , &$theta_virtual_120_ipsi_48000_coeff, &$theta_virtual_180_ipsi_48000_coeff;
   .VAR/DMCONST $contra_coeffs_48000[3]         = &$theta_virtual_60_contra_48000_coeff , &$theta_virtual_120_contra_48000_coeff, &$theta_virtual_180_contra_48000_coeff;
   .VAR/DMCONST $theta_speaker_coeffs_48000[4]  = &$theta_speaker_6_48000_coeff , &$theta_speaker_10_48000_coeff, &$theta_speaker_12_48000_coeff, &$theta_speaker_14_48000_coeff;
   .VAR/DMCONST $all_params_48000[8]            = &$ipsi_coeffs_48000, &$contra_coeffs_48000 ,&$theta_speaker_coeffs_48000 , &$gdelay_48000 , &$dcb_coeffs_48000 , &$itf_coeffs_48000, &$lsf_coeffs_48000 , &$peak_coeffs_48000;
   
   .VAR/DMCONST $ipsi_coeffs_88200[3]           = &$theta_virtual_60_ipsi_88200_coeff , &$theta_virtual_120_ipsi_88200_coeff, &$theta_virtual_180_ipsi_88200_coeff;
   .VAR/DMCONST $contra_coeffs_88200[3]         = &$theta_virtual_60_contra_88200_coeff , &$theta_virtual_120_contra_88200_coeff, &$theta_virtual_180_contra_88200_coeff;
   .VAR/DMCONST $theta_speaker_coeffs_88200[4]  = &$theta_speaker_6_88200_coeff , &$theta_speaker_10_88200_coeff, &$theta_speaker_12_88200_coeff, &$theta_speaker_14_88200_coeff;
   .VAR/DMCONST $all_params_88200[8]            = &$ipsi_coeffs_88200, &$contra_coeffs_88200 ,&$theta_speaker_coeffs_88200 , &$gdelay_88200 , &$dcb_coeffs_88200 , &$itf_coeffs_88200, &$lsf_coeffs_88200 , &$peak_coeffs_88200;
   
   .VAR/DMCONST $ipsi_coeffs_96000[3]           = &$theta_virtual_60_ipsi_96000_coeff , &$theta_virtual_120_ipsi_96000_coeff, &$theta_virtual_180_ipsi_96000_coeff;
   .VAR/DMCONST $contra_coeffs_96000[3]         = &$theta_virtual_60_contra_96000_coeff , &$theta_virtual_120_contra_96000_coeff, &$theta_virtual_180_contra_96000_coeff;
   .VAR/DMCONST $theta_speaker_coeffs_96000[4]  = &$theta_speaker_6_96000_coeff , &$theta_speaker_10_96000_coeff, &$theta_speaker_12_96000_coeff, &$theta_speaker_14_96000_coeff;
   .VAR/DMCONST $all_params_96000[8]            = &$ipsi_coeffs_96000, &$contra_coeffs_96000 ,&$theta_speaker_coeffs_96000 , &$gdelay_96000 , &$dcb_coeffs_96000 , &$itf_coeffs_96000, &$lsf_coeffs_96000 , &$peak_coeffs_96000;
   
   .VAR/DMCONST $all_params[$SAMPLE_RATES_SUPPORTED_COUNT] = &$all_params_32000 , &$all_params_44100 , &$all_params_48000 , &$all_params_88200 , &$all_params_96000;

.ENDMODULE;   


