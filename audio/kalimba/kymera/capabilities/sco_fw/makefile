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
# MODULE:       sco_fw
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

C_SRC =         sco_common_funcs.c \
				sco_fw_c.c 

GEN_ASM_HDRS += sco_struct.h
GEN_ASM_DEFS += SCO_TERMINAL_BUFFERS
GEN_ASM_DEFS += SCO_RCV_PARAMS
GEN_ASM_DEFS += SCO_COMMON_RCV_METADATA_HIST
GEN_ASM_DEFS += SCO_COMMON_RCV_OP_DATA
GEN_ASM_DEFS += sco_metadata

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = sco_fw

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

