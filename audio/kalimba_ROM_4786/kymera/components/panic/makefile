############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
# Unit 400, Cambridge Science Park,
# Milton Road, Cambridge, CB4 0WH.  UK
# Tel: +44 1223 692000  Fax: +44 1223 692001
#
############################################################################
#
# COMPONENT:    panic
# MODULE:       
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

C_SRC = panic.c

GEN_ASM_HDRS += panicids.h

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = panic

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

