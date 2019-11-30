############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Kalimba cbops library
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

H_PATH += ./operators
S_PATH += ./operators
S_PATH += ./log_linear_cbops
C_PATH += ./operators
C_PATH += ./log_linear_cbops


# Convert C Header to assembly
GEN_ASM_HDRS += cbops_c.h
GEN_ASM_HDRS += cbops_insert_op.h
GEN_ASM_HDRS += cbops_mixer_op.h
GEN_ASM_HDRS += cbops_port_wrap_op.h
GEN_ASM_HDRS += cbops_sidetone_filter.h
GEN_ASM_HDRS += cbops_sidetone_mix_op.h
GEN_ASM_HDRS += cbops_rate_monitor_op.h
GEN_ASM_HDRS += cbops_iir_resamplerv2_op.h
GEN_ASM_HDRS += cbops_underrun_comp_op.h
GEN_ASM_HDRS += cbops_discard_op.h
GEN_ASM_HDRS += cbops_dc_remove_c.h
GEN_ASM_HDRS += cbops_rate_adjustment_and_shift_c.h
GEN_ASM_HDRS += cbops_shift_c.h
GEN_ASM_HDRS += cbops_sink_overflow_disgard_op.h
GEN_ASM_HDRS += cbops_mute_c.h

GEN_ASM_DEFS += cbops_buffer
GEN_ASM_DEFS += cbops_op
GEN_ASM_DEFS += cbops_graph
GEN_ASM_DEFS += cbops_functions
GEN_ASM_DEFS += cbops_shift
GEN_ASM_DEFS += cbops_dc_remove
GEN_ASM_DEFS += cbops_mute
GEN_ASM_DEFS += cbops_rate_adjustment_and_shift
GEN_ASM_DEFS += sra_params
GEN_ASM_DEFS += sra_hist_params
GEN_ASM_DEFS += discard
GEN_ASM_DEFS += insert_op
GEN_ASM_DEFS += port_wrap_op
GEN_ASM_DEFS += iir_resampler_op
GEN_ASM_DEFS += iir_resampler_common
GEN_ASM_DEFS += iir_resampler_channel
GEN_ASM_DEFS += mixer_op
GEN_ASM_DEFS += sidetone_filter_op
GEN_ASM_DEFS += sidetone_params
GEN_ASM_DEFS += peq
GEN_ASM_DEFS += sink_overflow_disgard
GEN_ASM_DEFS += rate_monitor_op
GEN_ASM_DEFS += st_mix
GEN_ASM_DEFS += underrun_comp

GEN_ASM_DEFS += CBOPS_IO

# All assembly source
S_SRC += cbops.asm
S_SRC += cbops_dc_remove.asm
S_SRC += cbops_copy_op.asm
S_SRC += cbops_rate_adjustment_and_shift.asm
S_SRC += cbops_shift.asm
S_SRC += lin2log.asm
S_SRC += log2lin.asm
S_SRC += log_linear_cbops.asm
S_SRC += cbops_insert_operator.asm
S_SRC += cbops_mixer_operator.asm
S_SRC += cbops_port_wrap_operator.asm
S_SRC += cbops_sidetone_filter_operator.asm
S_SRC += cbops_sidetone_mix_operator.asm
S_SRC += cbops_rate_monitor_operator.asm
S_SRC += cbops_iir_resamplev2_operator.asm
S_SRC += cbops_sink_overflow_disgard.asm
S_SRC += cbops_underrun_comp.asm
S_SRC += cbops_discard.asm
S_SRC += cbops_mute.asm

# All C source
C_SRC += cbops_support.c
C_SRC += cbops_rate_adjustment_and_shift_c.c
C_SRC += cbops_insert_op.c
C_SRC += cbops_mixer_op.c
C_SRC += cbops_port_wrap_op.c
C_SRC += cbops_sidetone_filter.c
C_SRC += cbops_sidetone_mix_op.c
C_SRC += cbops_shift_op.c
C_SRC += cbops_rate_monitor_op.c
C_SRC += cbops_dc_remove_op.c
C_SRC += cbops_copy_op_c.c
C_SRC += log_linear_cbops_c.c
C_SRC += cbops_iir_resamplerv2_op.c
C_SRC += cbops_underrun_comp_op.c
C_SRC += cbops_discard_op.c
C_SRC += cbops_sink_overflow_disgard_op.c
C_SRC += cbops_mute_c.c


#########################################################################
# Patching
PATCH_SRC += cbops.asm
PATCH_SRC += operators/cbops_sidetone_filter_operator.asm
PATCH_SRC += operators/cbops_rate_adjustment_and_shift.asm
PATCH_SRC += operators/cbops_underrun_comp.asm
PATCH_SRC += operators/cbops_discard.asm
PATCH_SRC += operators/cbops_mixer_operator.asm
PATCH_SRC += operators/cbops_iir_resamplev2_operator.asm
PATCH_SRC += operators/cbops_sink_overflow_disgard.asm
PATCH_SRC += operators/cbops_sidetone_mix.asm
PATCH_SRC += operators/cbops_rate_monitor_operator.asm


GEN_PIDS = $(PATCH_DIR)/cbops_patch_ids.txt

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = cbops

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
