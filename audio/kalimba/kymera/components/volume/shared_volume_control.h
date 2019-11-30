/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup Volume
 *
 * \file  shared_volume_control.h
 * 
 * Common system volume control shared object
 * \ingroup volume
 */

#ifndef SHARED_VOLUME_CONTROL_H
#define SHARED_VOLUME_CONTROL_H
/*********************************************************
  TODO. rationalize shared volume information as defined in the volume Capability
        requirements and design document
  
  ndvc_noise_level - set by the cVc Send NDVC module as a volume
                     increase in 3dB steps.   It is used by the
                     cVc Receive Adaptive Eq module and by the
                     volume control capability in applying digital
                     gain.   Default value = 0
                     
  ndvc_filter_sum_lpdnz - set by the cVc Send NDVC module as the noise estimate.
                    It is used for Sidetone mode switching in the AEC Reference 
                    capability.
                    
                    
  current_volume_level - Is a 0-15 volume level.   It is used in the
                    cVc Send Capabilities for headset configurations to
                    switch to low volume (reduced processing) mode
  
  
  
**********************************************************/


typedef struct _shared_volume{ 
    unsigned  ndvc_noise_level;         /**< NDVC volume adjustment in 3dB steps */
    unsigned  ndvc_filter_sum_lpdnz;    /**< Noise Estimate from NDVC */
    unsigned  current_volume_level;     /**< Volume Level (dB/60) */
    unsigned  inv_post_gain;            /**< Inverse post gain (linear) */
    /* TODO - add in additional volume information as per Volume capability */

}SHARED_VOLUME_CONTROL_OBJ;


SHARED_VOLUME_CONTROL_OBJ* allocate_shared_volume_cntrl(void);
void release_shared_volume_cntrl(SHARED_VOLUME_CONTROL_OBJ *vol);


#endif /* SHARED_VOLUME_CONTROL_H */

