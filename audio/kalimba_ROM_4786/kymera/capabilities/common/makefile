############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
# Unit 400, Cambridge Science Park,
# Milton Road, Cambridge, CB4 0WH.  UK
# Tel: +44 1223 692000  Fax: +44 1223 692001
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       common
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

C_SRC =         capability_common.c
C_SRC +=        op_channel_list.c
C_SRC +=        op_msg_helpers.c
C_SRC +=        ttp_utilities.c
S_SRC +=        op_msg_utilities.asm
S_SRC +=        gain_conversion.asm

GEN_ASM_HDRS += op_channel_list.h


PATCH_DIR = $(AUTOGEN_ROOT)/patches

PATCH_SRC += op_msg_utilities.asm
PATCH_SRC += gain_conversion.asm


GEN_PIDS = $(PATCH_DIR)/capability_common_patch_ids.txt

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = common

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

