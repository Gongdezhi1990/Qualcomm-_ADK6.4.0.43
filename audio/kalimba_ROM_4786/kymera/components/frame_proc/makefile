############################################################################
# CONFIDENTIAL
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
# Unit 400, Cambridge Science Park,
# Milton Road, Cambridge, CB4 0WH.  UK
# Tel: +44 1223 692000  Fax: +44 1223 692001
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       frame_proc
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
# Makerules will add the standard paths
#########################################################################

S_SRC += frame_proc.asm

PATCH_DIR = $(AUTOGEN_ROOT)/patches
PATCH_SRC = frame_proc.asm

GEN_PIDS = $(PATCH_DIR)/frame_proc_ids.txt

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = frame_proc

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

