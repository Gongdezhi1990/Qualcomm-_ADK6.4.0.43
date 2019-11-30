############################################################################
# CONFIDENTIAL
# Project Audio-CPU
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
# Unit 400, Cambridge Science Park,
# Milton Road, Cambridge, CB4 0WH.  UK
# Tel: +44 1223 692000  Fax: +44 1223 692001
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Kalimba core libraries
# DESCRIPTION:  Library makefile.
#
############################################################################


#########################################################################
# Define root directory (relative so we can be installed anywhere)
#########################################################################

BUILD_ROOT = ../../kymera/build
include $(BUILD_ROOT)/roots.mkf

#########################################################################
# Enter source files and directories and header directories here.
#
# Makerules will add the standard interface paths
#########################################################################


S_PATH += ${KALIMBA_LIB_ROOT}/architecture

H_PATH += ${KALIMBA_LIB_ROOT}/architecture

ASM_ARCHITECTURE = ../../architecture/$(CHIP_NAME).asm

C_SRC += aac_malloc_tables.c

# All assembly source
S_SRC+= aac_ff_rew.asm \
	aacdec_api.asm \
	adts_read_frame.asm \
	apply_scalefactors_and_dequantize.asm \
	audio_mux_element.asm \
	audio_specific_config.asm \
	byte_align.asm \
	calc_sfb_and_wingroup.asm \
	corruption.asm \
	decode_cpe.asm \
	decode_fil.asm \
	decode_sce.asm \
	decoder_state.asm \
	discard_dse.asm \
	filterbank.asm \
	filterbank_analysis_ltp.asm \
	frame_decode.asm \
	ga_specific_config.asm \
	getbits.asm \
	showbits.asm \
	global_variables.asm \
	huffman_decode.asm \
	huffman_unpack.asm \
	ics_info.asm \
	imdct.asm \
	individual_channel_stream.asm \
	init_decoder.asm \
	is_decode.asm \
	latm_get_value.asm \
	latm_read_frame.asm \
	ltp_data.asm \
	ltp_decode.asm \
	ltp_reconstruction.asm \
	mdct.asm \
	mem_alloc.asm \
	mp4_discard_atom_data.asm \
	mp4_ff_rew.asm \
	mp4_moov_routine.asm \
	mp4_read_atom_header.asm \
	mp4_read_frame.asm \
	mp4_sequence.asm \
	ms_decode.asm \
	overlap_add.asm \
	payload_length_info.asm \
	payload_mux.asm \
	pns_decode.asm \
	program_element_config.asm \
	pulse_data.asm \
	pulse_decode.asm \
	raw_data_block.asm \
	reconstruct_channels.asm \
	reorder_spec.asm \
	reset_decoder.asm \
	samples_in_packet.asm \
	scalefactor_data.asm \
	section_data.asm \
	silence_decoder.asm \
	spectral_data.asm \
	stream_mux_config.asm \
	tns_data.asm \
	tns_encdec.asm \
	windowing.asm \
	init_static_decoder.asm \
	init_tables.asm \
	sbr_global_variables.asm \
	sbr_extension_data.asm \
	sbr_header.asm \
	sbr_reset.asm \
	sbr_calc_tables.asm \
	sbr_fband_tables.asm \
	sbr_fmaster_table_calc_fscale_gt_zero.asm \
	sbr_fmaster_table_calc_fscale_eq_zero.asm \
	sbr_single_channel_element.asm \
	sbr_channel_pair_element.asm \
	sbr_dtdf.asm \
	sbr_invf.asm \
	sbr_grid.asm \
	sbr_huff_dec.asm \
	sbr_allocate_and_unpack_from_flash.asm \
	sbr_read_one_word_from_flash.asm \
	sbr_envelope.asm \
	sbr_noise.asm \
	sbr_bubble_sort.asm \
	sbr_envelope_time_border_vector.asm \
	sbr_envelope_noise_border_vector.asm \
	sbr_middle_border.asm \
	sbr_extract_envelope_data.asm \
	sbr_extract_noise_floor_data.asm \
	sbr_envelope_noise_dequantisation.asm \
	sbr_envelope_noise_dequantisation_coupling_mode.asm \
	sbr_read_qdiv_tables.asm \
	sbr_fp_mult_frac.asm \
	sbr_hf_generation.asm \
	sbr_calc_chirp_factors.asm \
	sbr_patch_construction.asm \
	sbr_auto_correlation_opt.asm \
	sbr_prediction_coeff.asm \
	sbr_limiter_frequency_table.asm \
	sbr_hf_adjustment.asm \
	sbr_estimate_current_envelope.asm \
	sbr_calculate_gain.asm \
	sbr_calculate_limiter_band_boost_coefficients.asm \
	sbr_get_s_mapped.asm \
	sbr_hf_assembly.asm \
	sbr_hf_assembly_initialise_outer_loop_iteration.asm \
	sbr_hf_assembly_save_persistent_gain_signal_envelopes.asm \
	sbr_hf_assembly_initialise_signal_gain_and_component_loop.asm \
	sbr_hf_assembly_calc_gain_filters_smoothing_mode.asm \
	sbr_analysis_filterbank.asm \
	sbr_analysis_dct_kernel.asm \
	sbr_construct_x_matrix.asm \
	sbr_synthesis_filterbank_combined.asm \
	sbr_synthesis_construct_v.asm \
	sbr_synthesis_downsampled_construct_v.asm \
	sbr_save_prev_data.asm \
	sbr_wrap_last_thfgen_envelopes.asm \
	sbr_swap_channels.asm \
	eld_global_variables.asm \
	eld_specific_config.asm \
	eld_raw_data_block.asm \
	eld_decode_sce.asm \
	eld_decode_cpe.asm \
	eld_individual_channel_stream.asm \
	eld_imdct480.asm \
	eld_sbr_grid.asm \
	eld_sbr_block.asm \
	eld_sbr_data.asm \
	eld_overlap_add.asm \
	eld_windowing.asm \
	ps_global_variables.asm \
	ps_data.asm \
	ps_extension.asm \
	ps_huff_dec.asm \
	ps_data_decode.asm \
	ps_hybrid_analysis.asm \
	ps_hybrid_type_a_fir_filter.asm \
	ps_hybrid_type_b_fir_filter.asm \
	ps_hybrid_synthesis.asm \
	ps_delta_decode.asm \
	ps_map_34_parameters_to_20.asm \
	ps_decorrelate.asm \
	ps_stereo_mixing.asm \
	ps_transient_detection.asm \
	ps_initialise_transient_detection_for_hybrid_freq_bins_flash.asm \
	ps_initialise_transient_detection_for_qmf_freq_bins_flash.asm \
	ps_initialise_decorrelation_for_hybrid_freq_bins_flash.asm \
	ps_initialise_decorrelation_for_qmf_freq_bins_flash.asm \
	ps_initialise_stereo_mixing_for_hybrid_freq_bins_flash.asm \
	ps_initialise_stereo_mixing_for_qmf_freq_bins_flash.asm


GEN_PIDS = $(PATCH_DIR)/aac_patch_ids.txt

PATCH_SRC += $(S_SRC)

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = aac

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
