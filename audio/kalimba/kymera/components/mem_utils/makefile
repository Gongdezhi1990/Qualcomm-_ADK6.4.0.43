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
# MODULE:       mem_utils
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

C_SRC += shared_memory.c
C_SRC += scratch_memory.c
C_SRC += memory_table.c
C_SRC += dynloader.c
C_SRC += exported_constants.c exported_constants_file_xfer.c
C_SRC += reserved_ram.c

GEN_ASM_HDRS += shared_memory_ids.h
GEN_ASM_HDRS += exported_constant_files.h

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = mem_utils

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

