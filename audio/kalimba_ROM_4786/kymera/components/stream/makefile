############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Boot
# DESCRIPTION:  Library makefile.
#
############################################################################


#########################################################################
# Define root directory (relative so we can be installed anywhere)
#########################################################################

BUILD_ROOT = ../../build
include $(BUILD_ROOT)/roots.mkf


#########################################################################
# Enter source files and directories and header directories here.
#
# Makerules will add the standard interface paths
#########################################################################

C_SRC =  stream.c
C_SRC += stream_if.c
C_SRC += stream_audio_common.c
C_SRC += stream_connect.c
C_SRC += stream_operator.c
C_SRC += stream_schedule_timers.c
C_SRC += stream_kick_obj.c
C_SRC += stream_ratematch_mgr.c
C_SRC += stream_inplace_mgr.c
C_SRC += stream_anc.c
C_SRC += stream_unit_test.c

# Previously: C_SRC +=        stream_audio_$(CHIP_ARCH).c
C_SRC += $(if $(findstring $(CHIP_ARCH), $(filter hydra napier, $(strip $(CHIP_ARCH)))), stream_audio_hydra.c,)
C_SRC += $(if $(findstring $(CHIP_ARCH), a7da_kas), stream_audio_a7da_kas.c,)
C_SRC += $(if $(findstring $(CHIP_ARCH), bluecore), stream_audio_bluecore.c,)

C_SRC += $(if $(BUILD_SPDIF), stream_spdif_$(CHIP_ARCH).c,)
S_SRC += $(if $(BUILD_SPDIF), stream_spdif_ep.asm,)
C_SRC += $(if $(findstring $(CHIP_ARCH), $(filter hydra napier, $(strip $(CHIP_ARCH)))), stream_monitor_interrupt.c,)
C_SRC += $(if $(BUILD_SCO), stream_sco_common.c,)
C_SRC += $(if $(BUILD_SCO), stream_sco_$(CHIP_ARCH).c,)
C_SRC += $(if $(BUILD_A2DP), stream_a2dp_common.c)
C_SRC += $(if $(BUILD_A2DP), stream_a2dp_$(CHIP_ARCH).c,)
C_SRC += $(if $(BUILD_FILE), stream_file_$(CHIP_ARCH).c,)
C_SRC += $(if $(BUILD_SIMULATION_FILE), stream_simulation_file_$(CHIP_ARCH).c,)
C_SRC += $(if $(BUILD_TIMESTAMPED), stream_timestamped_$(CHIP_ARCH).c,)
C_SRC += $(if $(findstring $(CHIP_ARCH), hydra), stream_raw_buffer_hydra.c,)
C_SRC += $(if $(findstring $(CHIP_ARCH), hydra), stream_tester_hydra.c,)
C_SRC += $(if $(findstring $(CHIP_ARCH), a7da_kas), stream_common_a7da_kas.c,)
C_SRC += $(if $(or $(findstring $(BUILD_SECONDARY_CORE),true), $(findstring $(BUILD_DUAL_CORE),true)), stream_shadow.c,)
C_SRC += $(if $(or $(findstring $(BUILD_SECONDARY_CORE),true), $(findstring $(BUILD_DUAL_CORE),true)), stream_kip.c,)
C_SRC += $(if $(BUILD_USB_AUDIO), stream_usb_audio_$(CHIP_ARCH).c,)
S_SRC += $(if $(BUILD_USB_AUDIO), stream_usb_audio_ep.asm,)

# patch points for asm files used in the stream component
# NOTE: autogen for patch IDs is run unconditionally for any build
#       I couldn't get it conditionally working!
PATCH_SRC += stream_spdif_ep.asm
PATCH_SRC += stream_usb_audio_ep.asm
PATCH_DIR = $(AUTOGEN_ROOT)/patches
GEN_PIDS = $(PATCH_DIR)/stream_patch_ids.txt

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = stream

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
