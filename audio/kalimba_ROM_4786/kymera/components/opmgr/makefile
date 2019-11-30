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
# MODULE:       Opmgr
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

H_PATH += $(call myabspath,../../../lib/audio_fadeout/interface)

C_SRC =         opmgr.c
C_SRC+=         opmgr_task.c
C_SRC+=         opmgr_msg_handler.c
C_SRC+=         opmgr_op.c
C_SRC+=         opmgr_if.c
C_SRC+=         opmgr_endpoint_override.c
# Pull in module for both P0 (BUILD_DUAL_CORE) and for Px (BUILD_SECONDARY_CORE)
# Currently both P0- and P1-side OpMgr have some KIP functions in this module.
C_SRC += $(if $(or $(findstring $(BUILD_SECONDARY_CORE),true), $(findstring $(BUILD_DUAL_CORE),true)), opmgr_kip.c,)
GEN_ASM_HDRS += opmgr_for_ops.h
GEN_ASM_DEFS += OPERATOR_DATA
GEN_ASM_DEFS += CAPABILITY_DATA

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = opmgr

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

