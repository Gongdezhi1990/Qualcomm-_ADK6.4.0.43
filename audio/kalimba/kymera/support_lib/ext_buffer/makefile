############################################################################
# CONFIDENTIAL
# Copyright (c) 2018 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       ext_buffer
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

# Kalsim builds and non-downloadable code will use a set of 
# ASM & C files, while downloadable capabilities built for
# HW will use a different set.

S_SRC += $(if $(findstring $(KALSIM_CONFIG), True),ext_buffer_asm.asm ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),ext_buffer.asm, ext_buffer_asm.asm))


TARGET_EXE_ROOT =
TARGET_LIB_ROOT = ext_buffer

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
