/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_csr_aptx_acl_sprint.h

DESCRIPTION

*/

#ifndef A2DP_CODEC_CSR_APTX_ACL_SPRINT_H_
#define A2DP_CODEC_CSR_APTX_ACL_SPRINT_H_

#ifndef A2DP_SBC_ONLY

/*************************************************************************

  Default parameters for the aptX low latency caps exchange.
   
*/
#define A2DP_APTX_LOWLATENCY_TCL_LSB        0xB4  /* Target codec level = 180 (0x0b4) */
#define A2DP_APTX_LOWLATENCY_TCL_MSB        0x00
#define A2DP_APTX_LOWLATENCY_ICL_LSB        0x7C  /* Initial codec level = 380 (0x17C) */
#define A2DP_APTX_LOWLATENCY_ICL_MSB        0x01
#define A2DP_APTX_LOWLATENCY_MAX_RATE       0x32  /* 50 (/10000) = 0.005 SRA rate */
#define A2DP_APTX_LOWLATENCY_AVG_TIME       0x01  /* SRA Averaging time = 1s */
#define A2DP_APTX_LOWLATENCY_GWBL_LSB       0xB4  /* Good working buffer level = 180 (0x0b4) */
#define A2DP_APTX_LOWLATENCY_GWBL_MSB       0x00

/*************************************************************************
NAME    
     selectOptimalCsrAptxAclSprintCapsSink
    
DESCRIPTION
  Selects the optimal configuration for apt-X playback by setting a single 
	bit in each field of the passed caps structure.

	Note that the priority of selecting features is a
	design decision and not specified by the A2DP profiles.
   
*/
void selectOptimalCsrAptxAclSprintCapsSink(const uint8 *local_caps, uint8 *remote_caps);


/*************************************************************************
NAME    
     selectOptimalCsrAptxAclSprintCapsSource
    
DESCRIPTION
  Selects the optimal configuration for apt-X playback by setting a single 
	bit in each field of the passed caps structure.

	Note that the priority of selecting features is a
	design decision and not specified by the A2DP profiles.
   
*/
void selectOptimalCsrAptxAclSprintCapsSource(const uint8 *local_caps, uint8 *remote_caps);


/*************************************************************************
NAME    
     getCsrAptxAclSprintConfigSettings
    
DESCRIPTION    
	Return the codec configuration settings (rate and channel mode) for the physical codec based
	on the A2DP codec negotiated settings.
*/
void getCsrAptxAclSprintConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

#endif  /* A2DP_SBC_ONLY */

#endif  /* A2DP_CODEC_CSR_APTX_ACL_SPRINT_H_ */
