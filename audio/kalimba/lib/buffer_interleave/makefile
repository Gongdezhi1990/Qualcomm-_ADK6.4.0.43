############################################################################
# CONFIDENTIAL
# Project Kymera
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Kalimba libraries
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

# All assembly source

S_SRC += buffer_interleave.asm


GEN_PIDS = $(PATCH_DIR)/buffer_interleave_patch_ids.txt

PATCH_SRC += buffer_interleave.asm


#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = buffer_interleave

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
