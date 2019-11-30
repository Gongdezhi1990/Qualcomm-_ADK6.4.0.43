/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup patch Patch
 *
 * \file patch.h
 * \ingroup patch
 *
 * Public header file for the patch component of the audio subsystem
 *
 * \section sec1 Contains
 *
 *       patch_loader<br>
 *
 */

#ifndef PATCH_H
#define PATCH_H

/****************************************************************************
Include Files
*/

#include "types.h"

#ifdef HYDRA_SW_PATCH_SIMPLE
#ifndef HYDRA_SW_PATCH_DISABLE

/* We only support the simple form of Hydra patches */

#ifdef INCLUDE_PATCHES
#include "isp_router/isp_router_hydra_sw_patch.h"
#include "ssccp/ssccp_hydra_sw_patch.h"
#include "submsg/submsg_hydra_sw_patch.h"
#include "subreport/subreport_hydra_sw_patch.h"
#include "subres/subres_hydra_sw_patch.h"
#include "subserv/subserv_hydra_sw_patch.h"
#include "subsleep/subsleep_hydra_sw_patch.h"
#include "utils/utils_hydra_sw_patch.h"
#include "mib/mib_hydra_sw_patch.h"
#include "sub_host_wake/sub_host_wake_hydra_sw_patch.h"
#include "conman/conman_hydra_sw_patch.h"
#endif /* INCLUDE_PATCHES */


#define HYDRA_SW_PATCH_POINT(p) patch_fn(p)
#define HYDRA_SW_PATCH_POINT_RET(p) patch_fn(p)

#define HYDRA_SW_PATCH_FN(m, p) PATCH_INDEX p
#define HYDRA_SW_PATCH_FN_RET(m, p) PATCH_INDEX p
#define HYDRA_SW_PATCH_FN_ARG(m, p) PATCH_INDEX p
#define HYDRA_SW_PATCH_FN_RET_ARG(m, p) PATCH_INDEX p

#endif /* HYDRA_SW_PATCH_DISABLE */

#define PATCH_INDEX(p) patch_fn_##p##_index,

#endif /* HYDRA_SW_PATCH_SIMPLE */


/****************************************************************************
Public Type Declarations
*/

#ifdef INCLUDE_PATCHES
/*
 * To create a new patch vector, add it into this enum, leave the last line
 * at the end. All the entries must be of the form patch_fn_xxx_index where
 * xxx is the name of the patch vector. To call a patch vector, just say
 * patch_fn(xxx) making sure you've first #included this file.
 *
 * If a patch point is instantiated in multiple places in the code, its name
 * must end in _shared_patchpoint in keeping with the BlueCore approach
 * (e.g., patch_fn_fsm_shared_patchpoint_index).
 * See <http://wiki/BlueCorePatchSystem#Shared_software_patch_points>.
 */

enum patch_index {
    patch_fn_accmd_shared_patchpoint_index,
#ifdef INSTALL_AUDIO_MODULE
    patch_fn_audio_hwm_shared_patchpoint_index,
    patch_fn_audio_vsm_shared_patchpoint_index,
    patch_fn_audio_vsm_activate_shared_patchpoint_index,
    patch_fn_audio_vsm_deactivate_shared_patchpoint_index,
    patch_fn_audio_vsm_sync_shared_patchpoint_index,
    patch_fn_audio_vsm_chn_map_shared_patchpoint_index,
#ifdef INSTALL_AUDIO_RESOURCE_ROUTING
    patch_fn_audio_res_mgr_shared_patchpoint_index,
#endif
#ifdef INSTALL_CODEC
    patch_fn_audio_rate_matching_shared_patchpoint_index,
#endif /* INSTALL CODEC */
#ifdef INSTALL_AUDIO_INTERFACE_PCM
    patch_fn_hal_audio_pcm_shared_patchpoint_index,
    patch_fn_hal_audio_pcm_common_shared_patchpoint_index,
#ifdef INSTALL_AUDIO_INTERFACE_I2S
    patch_fn_hal_audio_i2s_shared_patchpoint_index,
#endif /* INSTALL_AUDIO_INTERFACE_I2S */
#ifdef INSTALL_AUDIO_INTERFACE_SPDIF
    patch_fn_hal_audio_spdif_shared_patchpoint_index,
#endif /* INSTALL_AUDIO_INTERFACE_SPDIF */
#endif /* INSTALL_AUDIO_INTERFACE_PCM */
#ifdef INSTALL_CODEC
    patch_fn_hal_audio_codec_shared_patchpoint_index,
#endif /* INSTALL_CODEC */
#if defined(INSTALL_CODEC)||defined(INSTALL_DIGITAL_MIC)||defined(INSTALL_AUDIO_INTERFACE_PWM)
    patch_fn_hal_audio_warp_shared_patchpoint_index,
#endif
    patch_fn_hal_audio_slot_init_index,
    patch_fn_stream_audio_hydra_shared_patchpoint_index,
    patch_fn_stream_audio_hydra_get_rm_data_index,
#endif /* INSTALL_AUDIO_MODULE */
    patch_fn_install_patches_index,
    patch_fn_sssm_ss_start_firmware_index,
    patch_fn_stream_shared_patchpoint_index,
    patch_fn_stream_connect_shared_patchpoint_index,
    patch_fn_stream_schedule_timers_shared_patchpoint_index,
    patch_fn_stream_ratematch_decision_index,
    patch_fn_stream_new_endpoint_index,
    patch_fn_stream_in_place_shared_patchpoint_index,
    patch_fn_stream_if_shared_patchpoint_index,
    patch_fn_stream_operator_shared_patchpoint_index,
    patch_fn_stream_timestamped_shared_patchpoint_index,
    patch_fn_stream_sco_shared_patchpoint_index,
    patch_fn_stream_shadow_shared_patchpoint_index,
    patch_fn_stream_ratematch_shared_patchpoint_index,
    patch_fn_stream_audio_shared_patchpoint_index,
#ifdef INSTALL_OPERATOR_AEC_REFERENCE
    patch_fn_aec_reference_shared_patchpoint_index,
    patch_fn_aec_reference_run_shared_patchpoint_index,
#endif
    patch_fn_sps_service_shared_patchpoint_index,
    patch_fn_timers_create_shared_patchpoint_index,
    patch_fn_timers_cancel_shared_patchpoint_index,
    patch_fn_timers_service_shared_patchpoint_index,
    patch_fn_pl_dynamic_tasks_shared_patchpoint_index,
    patch_fn_pl_msgs_shared_patchpoint_index,
    patch_fn_pl_bgint_shared_patchpoint_index,
    patch_fn_pl_context_switch_index,
    patch_fn_panic_diatribe_index,
    patch_fn_opmgr_shared_patchpoint_index,
    patch_fn_opmgr_bgint_patchpoint_index,
    patch_fn_cbuffer_create_shared_patchpoint_index,
    patch_fn_hydra_cbuff_create_shared_patchpoint_index,
    patch_fn_hydra_cbuff_modify_shared_patchpoint_index,
    patch_fn_hydra_cbuff_remote_shared_patchpoint_index,
    patch_fn_hydra_cbuff_offset_shared_patchpoint_index,
    patch_fn_hydra_cbuff_sample_shared_patchpoint_index,
    patch_fn_mmu_buffer_create_shared_patchpoint_index,
    patch_fn_mmu_buffer_readwrite_shared_patchpoint_index,
    patch_fn_octet_buffer_shared_patchpoint_index,
    patch_fn_cbops_mgr_create_shared_patchpoint_index,
    patch_fn_cbops_mgr_run_shared_patchpoint_index,
    patch_fn_cbops_lib_shared_patchpoint_index,
    patch_fn_sssm_init_operational_shared_patchpoint_index,
    patch_fn_sco_mgr_and_impl_shared_patchpoint_index,
#ifdef INSTALL_AUDIO_DATA_SERVICE
    patch_fn_audio_data_service_shared_patchpoint_index,
    patch_fn_audio_data_service_meta_shared_patchpoint_index,
    patch_fn_audio_data_service_kick_shared_patchpoint_index,
#endif
    patch_fn_lmgr_shared_patchpoint_index,
    patch_fn_lmgr_aux_msg_handler_index,
#ifdef INSTALL_METADATA
    patch_fn_buff_metadata_shared_patchpoint_index,
    patch_fn_metadata_append_index,
    patch_fn_metadata_remove_index,
#endif
#ifdef INSTALL_TTP
    patch_fn_ttp_gen_shared_patchpoint_index,
    patch_fn_ttp_utils_populate_tag_index,
    patch_fn_timed_playback_shared_patchpoint_index,
    patch_fn_timestamp_reframe_shared_patchpoint_index,
#endif
#ifdef INSTALL_CAP_DOWNLOAD_MGR
    patch_fn_cap_download_debug_shared_patchpoint_index,
    patch_fn_cap_download_mgr_kcs_shared_patchpoint_index,
    patch_fn_cap_download_mgr_logic_other_shared_patchpoint_index,
    patch_fn_cap_download_mgr_logic_state_shared_patchpoint_index,
    patch_fn_cap_download_mgr_logic_event_shared_patchpoint_index,
    patch_fn_cap_download_mgr_shared_patchpoint_index,
    patch_fn_cap_download_task_shared_patchpoint_index,
    patch_fn_cap_download_util_shared_patchpoint_index,
    patch_fn_cap_download_util_write_shared_patchpoint_index,
    patch_fn_cap_download_util_relocate_shared_patchpoint_index,
    patch_fn_cap_download_mgr_parser_other_1_shared_patchpoint_index,
    patch_fn_cap_download_mgr_parser_other_2_shared_patchpoint_index,
    patch_fn_cap_download_mgr_parser_state_1_shared_patchpoint_index,
    patch_fn_cap_download_mgr_parser_state_2_shared_patchpoint_index,
    patch_fn_cap_download_mgr_parser_state_process_mem_index,
    patch_fn_cap_download_mgr_parser_state_reloc_index,
    patch_fn_cap_download_mgr_parser_state_kdc_info_index,
    patch_fn_heap_alloc_pm_init_shared_patchpoint_index,
    patch_fn_heap_alloc_pm_heap_alloc_internal_pm_index,
    patch_fn_heap_alloc_pm_shared_patchpoint_index,
    patch_fn_heap_alloc_pm_heap_free_pm_index,
    patch_fn_malloc_pm_shared_patchpoint_index,
    patch_fn_capability_database_init_download_list_index,
#ifdef INSTALL_HYDRA
    patch_fn_cap_download_mgr_fetch_hydra_shared_patchpoint_index,
    patch_fn_cap_download_interface_shared_patchpoint_index,
#endif
#endif /* INSTALL_CAP_DOWNLOAD_MGR */
#ifdef INSTALL_A2DP
    patch_fn_stream_a2dp_shared_patchpoint_index,
    patch_fn_stream_a2dp_kick_shared_patchpoint_index,
#endif /* INSTALL_A2DP */
#if defined(INSTALL_HYDRA) && defined(INSTALL_FILE)
    patch_fn_stream_file_shared_patchpoint_index,
    patch_fn_stream_file_kick_index,
#endif /* defined(INSTALL_HYDRA) && defined(INSTALL_FILE) */
#ifdef INSTALL_MCLK_SUPPORT
    patch_fn_audio_mclk_mgr_shared_patchpoint_index,
#endif
#ifdef INSTALL_USB_AUDIO
    patch_fn_stream_usb_audio_hydra_shared_patchpoint_index,
#endif
#ifdef INSTALL_SPDIF
    patch_fn_stream_spdif_hydra_shared_patchpoint_index,
#endif
#ifdef INSTALL_PS_FOR_ACCMD
    patch_fn_ps_sar_shared_patchpoint_index,
    patch_fn_ps_router_shared_patchpoint_index,
#endif
    patch_fn_mem_utils_shared_patchpoint_index,
#ifdef INSTALL_OPERATOR_XOVER
    patch_fn_xover_wrapper_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_VSE
    patch_fn_vse_wrapper_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_PEQ
    patch_fn_peq_wrapper_shared_patchpoint_index,
    patch_fn_peq_wrapper_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_MIXER
    patch_fn_mixer_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_IIR_RESAMPLER
    patch_fn_iir_resampler_shared_patchpoint_index,
    patch_fn_iir_resampler_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_DBE
    patch_fn_dbe_wrapper_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_COMPANDER
    patch_fn_compander_wrapper_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_CELT_ENCODE
    patch_fn_celt_encode_wrapper_shared_patchpoint_index,
#endif
#ifdef INSTALL_OPERATOR_CELT_DECODE
    patch_fn_celt_decode_wrapper_shared_patchpoint_index,
    patch_fn_celt_decode_wrapper_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_CHANNEL_MIXER
    patch_fn_channel_mixer_process_data_index,
#endif
    patch_fn_common_set_fadeout_state_index,
    patch_fn_common_send_unsolicited_message_index,
    patch_fn_op_channel_list_connect_index,
    patch_fn_op_channel_list_disconnect_index,
    patch_fn_op_channel_list_num_connected_index,
    patch_fn_op_channel_list_free_list_index,
    patch_fn_cpsInitParameters_index,
    patch_fn_cpsGetParameterMsgHandler_index,
    patch_fn_cpsGetDefaultsMsgHandler_index,
    patch_fn_cpsSetParameterMsgHandler_index,
    patch_fn_cpsSetParameterFromPsStore_index,
    patch_fn_cpsGetUcidMsgHandler_index,
    patch_fn_cpsSetUcidMsgHandler_index,
    patch_fn_cps_control_resp_helper_index,
    patch_fn_cps_response_set_result_index,
    patch_fn_common_obpm_response_helper_index,
    patch_fn_common_obpm_status_helper_index,
#ifdef INSTALL_OPERATOR_DELAY
    patch_fn_delay_wrapper_process_data_index,
#endif
    patch_fn_encoder_process_data_index,
#ifdef INSTALL_OPERATOR_RINGTONE_GENERATOR
    patch_fn_ringtone_generator_shared_patchpoint_index,
    patch_fn_ringtone_generator_process_data_index,
#endif
    patch_fn_base_multi_channel_shared_patchpoint_index,
#ifdef INSTALL_METADATA
    patch_fn_base_multi_channel_metadata_transfer_index,
    patch_fn_base_multi_channel_metadata_shared_patchpoint_index,
#endif
#ifdef INSTALL_OPERATOR_RTP_DECODE
    patch_fn_rtp_decode_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_SPLITTER
    patch_fn_splitter_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_USB_AUDIO
    patch_fn_usb_audio_process_data_index,
#endif
#ifdef INSTALL_OPERATOR_RESAMPLER
    patch_fn_resampler_wrapper_shared_patchpoint_index,
    patch_fn_resampler_wrapper_process_data_index,
#endif
    patch_fn_sbc_decode_process_data_index,
    patch_fn_wbs_encode_process_data_index,
    patch_fn_wbs_decode_process_data_index,
    patch_fn_wbs_decode_data_processing_shared_patchpoint_index,
    patch_fn_read_packet_metadata_index,
    patch_fn_analyse_sco_metadata_index,
    patch_fn_fake_packet_index,
    patch_fn_sco_fw_update_expected_timestamp_index,
    patch_fn_sco_fw_check_bad_kick_threshold_index,
    patch_fn_sco_common_connect_index,
    patch_fn_sco_common_disconnect_index,
    patch_fn_sco_common_rcv_initialise_index,
    patch_fn_sco_common_rcv_reset_working_data_index,
    patch_fn_sco_common_rcv_set_from_air_info_helper_index,
 #ifdef INSTALL_PLC100
    patch_fn_sco_common_rcv_destroy_plc_data_index,
    patch_fn_sco_common_rcv_force_plc_off_helper_index,
#endif
    patch_fn_sco_common_rcv_frame_counts_helper_index,
    patch_fn_sco_common_get_data_format_index,
    patch_fn_sco_common_get_sched_info_index,
    patch_fn_sco_rcv_process_data_index,
    patch_fn_sco_send_process_data_index,
#ifdef INSTALL_SPDIF
    patch_fn_spdif_decode_process_data_index,
    patch_fn_spdif_decode_shared_patchpoint_index,
#endif
#if defined (INSTALL_OPERATOR_APTX_CLASSIC_DECODE) || \
    defined (INSTALL_OPERATOR_APTX_LOW_LATENCY_DECODE) || \
    defined (INSTALL_OPERATOR_APTX_SHUNT_DECODER)
    patch_fn_aptx_decode_process_data_index,
    patch_fn_aptx_decode_shared_patchpoint_index,
#endif
#if defined (INSTALL_OPERATOR_AAC_DECODE) || defined (INSTALL_OPERATOR_AAC_SHUNT_DECODER)
    patch_fn_aac_decode_index,
#endif
#if defined(INSTALL_OPERATOR_CVC_HEADSET_1MIC)     || \
    defined(INSTALL_OPERATOR_CVC_HEADSET_2MIC_MONO)     || \
    defined(INSTALL_OPERATOR_CVC_HEADSET_BINAURAL) || \
    defined(INSTALL_OPERATOR_CVC_AUTO_1MIC)        || \
    defined(INSTALL_OPERATOR_CVC_AUTO_2MIC)        || \
    defined(INSTALL_OPERATOR_CVC_SPEAKER_1MIC)     || \
    defined(INSTALL_OPERATOR_CVC_SPEAKER_2MIC)     || \
    defined(INSTALL_OPERATOR_CVC_SPEAKER_3MIC)     || \
    defined(INSTALL_OPERATOR_CVC_SPEAKER_4MIC)
    patch_fn_cvc_send_wrapper_shared_patchpoint_index,
    patch_fn_cvc_send_process_data_patch_index,
    patch_fn_cvc_send_opmsg_obpm_set_control_patch_index,
    patch_fn_cvc_send_opmsg_obpm_set_params_patch_index,
    patch_fn_cvc_send_opmsg_obpm_get_status_patch_index,
#endif
#ifdef INSTALL_OPERATOR_CVC_RECEIVE
    patch_fn_cvc_receive_wrapper_shared_patchpoint_index,
    patch_fn_cvc_receive_process_data_patch_index,
    patch_fn_cvc_receive_opmsg_obpm_set_control_patch_index,
    patch_fn_cvc_receive_opmsg_obpm_set_params_patch_index,
#endif
#ifdef INSTALL_OPERATOR_VOLUME
    patch_fn_volume_control_wrapper_shared_patchpoint_index,
    patch_fn_volume_control_process_data_patch_index,
    patch_fn_volume_control_adjust_amount_index,
    patch_fn_volume_control_aux_check_index,
    patch_fn_volume_control_opmsg_obpm_set_control_patch_index,
    patch_fn_volume_control_opmsg_obpm_set_params_patch_index,
#endif
#ifdef INSTALL_OPERATOR_SOURCE_SYNC
    patch_fn_src_sync_shared_patchpoint_index,
    patch_fn_src_sync_rate_adjust_shared_patchpoint_index,
    patch_fn_src_sync_route_copy_index,
    patch_fn_src_sync_route_discard_index,
    patch_fn_src_sync_route_silence_index,
#endif
#ifdef INSTALL_OPERATOR_VAD
    patch_fn_vad_capability_shared_patchpoint_index,
    patch_fn_vad_process_data_index,
    patch_fn_vad_opmsg_obpm_set_params_index,
    patch_fn_vad_configure_mode_change_index,
#endif /* INSTALL_OPERATOR_VAD */
#ifdef INSTALL_OPERATOR_SVA
    patch_fn_sva_capability_shared_patchpoint_index,
    patch_fn_sva_lib_shared_patchpoint_index,
    patch_fn_sva_process_data_index,
    patch_fn_sva_opmsg_obpm_set_params_index,
    patch_fn_sva_configure_mode_change_index,
    patch_fn_sva_trigger_phrase_load_index,
#endif /* INSTALL_OPERATOR_SVA */
#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    patch_fn_audio_kymera_ipc_shared_patchpoint_index,
    patch_fn_audio_kymera_ipc_msg_handler_index,
    patch_fn_audio_kymera_ipc_sig_handler_index,
    patch_fn_audio_kymera_ipc_task_handler_index,
    patch_fn_kip_shared_patchpoint_index,
    patch_fn_stream_kip_shared_patchpoint_index,
#ifdef INSTALL_METADATA_DUALCORE
    patch_fn_kip_metadata_shared_patchpoint_index,
#endif /* INSTALL_METADATA_DUALCORE */
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */
#ifdef INSTALL_CBUFFER_EX
    patch_fn_cbuffer_copy_16bit_be_zero_shift_shared_patchpoint_index,
#endif /* INSTALL_CBUFFER_EX */
#ifdef INSTALL_PM_HEAP_SHARE
    patch_fn_patch_loader_init_malloc_pm_index,
#endif /* INSTALL_PM_HEAP_SHARE */

#ifdef INSTALL_OBPM_ADAPTOR
    patch_fn_obpm_shared_patchpoint_index,
    patch_fn_obpm_callbacks_shared_patchpoint_index,
#endif /* INSTALL_OBPM_ADAPTOR */

#ifdef INSTALL_SPI_INTERFACE
    patch_fn_spi_shared_patchpoint_index,
#endif /* INSTALL_SPI_INTERFACE */

#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
    patch_fn_exp_const_file_id_index,
    patch_fn_exp_const_shared_patchpoint_index,
#endif /* INSTALL_CAPABILITY_CONSTANT_EXPORT */
    /* Add new patchpoint enumerators HERE */

#ifdef INSTALL_AOV
    patch_fn_aov_task_shared_patchpoint_index,
    patch_fn_wake_on_voice_service_shared_patchpoint_index,
#endif /* INSTALL_AOV */
#ifdef INSTALL_FILE_MGR
    patch_fn_file_mgr_shared_patchpoint_index,
    patch_fn_file_mgr_interface_shared_patchpoint_index,
#endif

#ifdef INSTALL_CLK_MGR
    patch_fn_clk_mgr_shared_patchpoint_index,
    patch_fn_audio_mic_bias_shared_patchpoint_index,
#endif /* INSTALL_CLK_MGR */

#ifdef INSTALL_UNINTERRUPTABLE_ANC
    patch_fn_audio_anc_shared_patchpoint_index,
    patch_fn_hal_audio_anc_shared_patchpoint_index,
    patch_fn_stream_anc_shared_patchpoint_index,
#endif /* INSTALL_UNINTERRUPTABLE_ANC */

#ifdef HAVE_SIDE_TONE_HARDWARE
    patch_fn_audio_sidetone_shared_patchpoint_index,
    patch_fn_hal_audio_sidetone_shared_patchpoint_index,
#endif /* HAVE_SIDE_TONE_HARDWARE */

    patch_fn_rate_lib_shared_patchpoint_index,

#ifdef HYDRA_SW_PATCH_SIMPLE
#ifndef HYDRA_SW_PATCH_DISABLE
    ISP_ROUTER_HYDRA_SW_PATCH_FN(x)
    MIB_HYDRA_SW_PATCH_FN(x)
    SSCCP_HYDRA_SW_PATCH_FN(x)
    SUB_HOST_WAKE_HYDRA_SW_PATCH_FN(x)
    SUBMSG_HYDRA_SW_PATCH_FN(x)
    SUBREPORT_HYDRA_SW_PATCH_FN(x)
    SUBRES_HYDRA_SW_PATCH_FN(x)
    SUBSERV_HYDRA_SW_PATCH_FN(x)
    SUBSLEEP_HYDRA_SW_PATCH_FN(x)
    UTILS_HYDRA_SW_PATCH_FN(x)
    CONMAN_HYDRA_SW_PATCH_FN(x)
#endif  /* HYDRA_SW_PATCH_DISABLE */
#endif /* HYDRA_SW_PATCH_SIMPLE */
    patch_fn_comms_interface_shared_patchpoint_index,

    sw_patch_table_size    /* One more than the last patchpoint index */
};

#endif /* INCLUDE_PATCHES */


/****************************************************************************
Public Macro Declarations
*/

#ifdef INCLUDE_PATCHES

typedef void (*patch_func_ptr)(void);

#define patch_fn(x) do { \
    enum patch_index pi = patch_fn_##x##_index; \
    patch_func_ptr patch_func = (patch_func_ptr)(uintptr_t)patch_fn_table[pi]; \
    if (patch_func != NULL) \
    {\
        patch_func();\
    }\
} while(0)


#define patch_fn_shared(x) patch_fn(x##_shared_patchpoint)
#else
#define patch_fn(x) ((void) 0)
#define patch_fn_shared(x) ((void) 0)
#endif


/****************************************************************************
Public Data Declarations
*/

#ifdef INCLUDE_PATCHES

extern const void *patch_fn_table[];


/****************************************************************************
Public Type Declarations
*/

/**
 * \brief  Typedef of callback function to be called when the audio subsystem
 * patching operation has been completed.
 */
typedef void (*patch_complete_cb)(void);


/****************************************************************************
Public Function Declarations
*/

/**
 * \brief  Loads and installs the patch points for the audio subsystem
 *
 * \param  cb Function to call on completion of patch loading
 *
 * This function is called during the boot sequence by the subsystem state
 * manager. It will attempt to load in and install the default audio subsystem
 * patch file suitable for the current ROM version. On completion it will call
 * the function supplied via the cb parameter.
 */
void patch_loader(patch_complete_cb cb);

#endif /* INCLUDE_PATCHES */

#endif /* PATCH_H */
