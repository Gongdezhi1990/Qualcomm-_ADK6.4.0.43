############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
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

C_SRC += sbc_malloc_tables.c

# All assembly source
S_SRC+= analysis_subband_filter.asm \
		calc_bit_allocation.asm \
		calc_frame_length.asm \
		calc_scale_factors.asm \
		crc_calc.asm \
		encoder_set_parameters.asm \
		find_sync.asm \
		frame_decode.asm \
		frame_encode.asm \
		get_encoded_frame_info.asm \
		get_frame_info.asm \
		getbits.asm \
		global_variables_decoder.asm \
		global_variables_encdec.asm \
		global_variables_encoder.asm \
		init_decoder.asm \
		init_encoder.asm \
		joint_stereo_decode.asm \
		joint_stereo_encode.asm \
		putbits.asm \
		quantize_samples.asm \
		read_audio_samples.asm \
		read_frame_header.asm \
		read_padding_bits.asm \
		read_scale_factors.asm \
		reset_decoder.asm \
		reset_encoder.asm \
		sample_reconstruction.asm \
		sbc_api.asm \
		silence_decoder.asm \
		synthesis_subband_filter.asm \
		write_audio_samples.asm \
		write_frame_header.asm \
		write_padding_bits.asm \
		wbs_sco_decode.asm \
		wbs_sco_encode.asm \
		write_scale_factors.asm \
		initialise_tables.asm

GEN_PIDS = $(PATCH_DIR)/sbc_patch_ids.txt

PATCH_SRC += $(S_SRC)

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = sbc

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
