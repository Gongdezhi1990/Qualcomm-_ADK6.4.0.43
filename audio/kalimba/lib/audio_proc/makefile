############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Kalimba audio_proc library
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

C_SRC+= sra_c.c

# All assembly source
S_SRC+= cmpd100.asm
S_SRC+= delay.asm
S_SRC+= mute_control.asm
S_SRC+= peak_monitor.asm
S_SRC+= peq.asm
S_SRC+= hq_peq.asm
S_SRC+= dh_peq.asm
S_SRC+= peq_coeff_gen.asm 
S_SRC+= sra.asm
S_SRC+= stereo_3d_enhancement.asm
S_SRC+= stereo_copy.asm
S_SRC+= stream_gain.asm
S_SRC+= stream_mixer.asm
S_SRC+= mic_grouping.asm
S_SRC+= dbe.asm
S_SRC+= dbe_variables.asm
S_SRC+= vse.asm
S_SRC+= vse_variables.asm
S_SRC+= xover.asm
S_SRC+= compander.asm
S_SRC+= xover_variables.asm 


S_SRC += iir_resamplev2_common.asm
S_SRC += frame_iir_resamplev2.asm

# Convert C Header to assembly
GEN_ASM_HDRS += iir_resamplerv2_common.h
GEN_ASM_HDRS += frame_iir_resamplerv2.h
GEN_ASM_HDRS += sra_c.h

# Pack shared coefficients
DYN_FILES  += iir_resamplev2_coefs.dyn

GEN_PIDS = $(PATCH_DIR)/audio_proc_patch_ids.txt

PATCH_SRC += $(S_SRC)

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = audio_proc

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
