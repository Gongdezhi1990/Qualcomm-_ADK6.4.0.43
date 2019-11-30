/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_csr_tws.h
    
DESCRIPTION
	
*/

#ifndef A2DP_CSR_TWS_H_
#define A2DP_CSR_TWS_H_

#ifndef A2DP_SBC_ONLY

/*************************************************************************/
void selectOptimalCsrTwsSbcCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalCsrTwsSbcCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalCsrTwsMp3CapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalCsrTwsMp3CapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalCsrTwsAacCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalCsrTwsAacCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*************************************************************************/
void selectOptimalCsrTwsAptxCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalCsrTwsAptxCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalTwsAptxAdCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalTwsAptxAdCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*************************************************************************/
void selectOptimalCsrTwsPlusAptxCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalCsrTwsPlusAptxCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*************************************************************************/
void selectOptimalCsrTwsPlusSbcCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void selectOptimalCsrTwsPlusSbcCapsSource(const uint8 *local_codec_caps, uint8 *remote_codec_caps);

/*****************************************************************************/
void getCsrTwsSbcConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

/*****************************************************************************/
void getCsrTwsMp3ConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

/*****************************************************************************/
void getCsrTwsAacConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

/*****************************************************************************/
void getCsrTwsAptxConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

/*****************************************************************************/
void getTwsAptxAdConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

/*****************************************************************************/
void getCsrTwsPlusAptxConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

/*****************************************************************************/
void getCsrTwsPlusSbcConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

#endif  /* A2DP_SBC_ONLY */

#endif  /* A2DP_CSR_TWS_H_ */
