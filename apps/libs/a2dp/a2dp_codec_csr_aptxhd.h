/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_csr_aptxhd.h

DESCRIPTION
    This file contains aptX-HD specific code.
    
*/

#ifndef A2DP_CODEC_CSR_APTXHD_H_
#define A2DP_CODEC_CSR_APTXHD_H_

#ifndef A2DP_SBC_ONLY

/*************************************************************************
NAME    
     selectOptimalCsrAptxHDCapsSink
    
DESCRIPTION
    Selects the optimal configuration for aptX-HD playback by setting a single 
    bit in each field of the passed caps structure.

    Note that the priority of selecting features is a
    design decision and not specified by the A2DP profiles.
   
*/
void selectOptimalCsrAptxHDCapsSink(const uint8 *local_caps, uint8 *remote_caps);


/*************************************************************************
NAME    
     selectOptimalCsrAptxHDCapsSource
    
DESCRIPTION
    Selects the optimal configuration for aptX-HD playback by setting a single 
    bit in each field of the passed caps structure.

    Note that the priority of selecting features is a
    design decision and not specified by the A2DP profiles.
   
*/
void selectOptimalCsrAptxHDCapsSource(const uint8 *local_caps, uint8 *remote_caps);


/*************************************************************************
NAME    
     getCsrAptxHDConfigSettings
    
DESCRIPTION    
    Return the codec configuration settings (rate and channel mode) for the physical codec based
    on the A2DP codec negotiated settings.
*/
void getCsrAptxHDConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

#endif  /* A2DP_SBC_ONLY */

#endif  /* A2DP_CODEC_CSR_APTXHD_H_ */
