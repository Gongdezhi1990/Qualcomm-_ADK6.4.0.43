/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup cvc_send
 * \file  cvc_send.h
 * \ingroup capabilities
 *
 * CVC send operator public header file. <br>
 *
 */


#ifndef __CVC_SEND_H__
#define __CVC_SEND_H__

#include "opmgr/opmgr_for_ops.h"


/* Capability structure references */
#ifdef INSTALL_OPERATOR_CVC_HEADSET_1MIC
   extern const CAPABILITY_DATA cvc_send_1mic_nb_hs_cap_data;
   extern const CAPABILITY_DATA cvc_send_1mic_wb_hs_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_1mic_uwb_hs_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_1mic_swb_hs_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_1mic_fb_hs_cap_data;
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_2MIC_MONO
   extern const CAPABILITY_DATA cvc_send_2mic_hs_mono_nb_cap_data;
   extern const CAPABILITY_DATA cvc_send_2mic_hs_mono_wb_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_2mic_hs_mono_uwb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_2mic_hs_mono_swb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_2mic_hs_mono_fb_cap_data;
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_BINAURAL
   extern const CAPABILITY_DATA cvc_send_2mic_hs_binaural_nb_cap_data;
   extern const CAPABILITY_DATA cvc_send_2mic_hs_binaural_wb_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_2mic_hs_binaural_uwb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_2mic_hs_binaural_swb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_2mic_hs_binaural_fb_cap_data;
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_AUTO_1MIC
   extern const CAPABILITY_DATA cvc_send_1mic_nb_auto_cap_data;
   extern const CAPABILITY_DATA cvc_send_1mic_wb_auto_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_1mic_uwb_auto_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_1mic_swb_auto_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_1mic_fb_auto_cap_data;
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_AUTO_2MIC
   extern const CAPABILITY_DATA cvc_send_2mic_nb_auto_cap_data;
   extern const CAPABILITY_DATA cvc_send_2mic_wb_auto_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_2mic_uwb_auto_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_2mic_swb_auto_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_2mic_fb_auto_cap_data;
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_1MIC
   extern const CAPABILITY_DATA cvc_send_1mic_speaker_nb_cap_data;
   extern const CAPABILITY_DATA cvc_send_1mic_speaker_wb_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_1mic_speaker_uwb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_1mic_speaker_swb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_1mic_speaker_fb_cap_data;
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_2MIC
   extern const CAPABILITY_DATA cvc_send_2mic_speaker_nb_cap_data;
   extern const CAPABILITY_DATA cvc_send_2mic_speaker_wb_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_2mic_speaker_uwb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_2mic_speaker_swb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_2mic_speaker_fb_cap_data;
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_3MIC
   extern const CAPABILITY_DATA cvc_send_3mic_speaker_nb_cap_data;
   extern const CAPABILITY_DATA cvc_send_3mic_speaker_wb_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_3mic_speaker_uwb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_3mic_speaker_swb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_3mic_speaker_fb_cap_data;
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_4MIC
   extern const CAPABILITY_DATA cvc_send_4mic_speaker_nb_cap_data;
   extern const CAPABILITY_DATA cvc_send_4mic_speaker_wb_cap_data;
#ifdef INSTALL_OPERATOR_CVC_24K
   extern const CAPABILITY_DATA cvc_send_4mic_speaker_uwb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
   extern const CAPABILITY_DATA cvc_send_4mic_speaker_swb_cap_data;
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
   extern const CAPABILITY_DATA cvc_send_4mic_speaker_fb_cap_data;
#endif
#endif

#endif /* __CVC_SEND_H__ */
