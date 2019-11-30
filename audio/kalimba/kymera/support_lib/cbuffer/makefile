############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       CBuffer
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

# C files we include in all builds
C_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),cbuffer_ex.c, ))

# Assembler files we need for a normal kalimba build
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True),cbuffer_ex_asm_kalsim.asm ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_cbuffer_ex_asm.asm, cbuffer_ex_asm.asm))

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = cbuffer

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
