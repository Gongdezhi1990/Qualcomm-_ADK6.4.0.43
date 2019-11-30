############################################################################
# CONFIDENTIAL
# Copyright (c) 2018 - 2018 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       download_support_lib
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

S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_extmem_lock.asm, )) 
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_extmem_unlock.asm, )) 
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_extmem_get_mode.asm, )) 
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_extmem_enable.asm, )) 
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_extmem_enable_with_retry.asm, )) 

C_SRC += extmem.c

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = extmem

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

