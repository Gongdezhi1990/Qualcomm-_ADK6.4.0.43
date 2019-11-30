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

S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_audio_pios_get_inputs.asm, )) 
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_audio_pios_set_outputs.asm, )) 
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true),dsl_audio_pios_set_direction.asm, )) 

C_SRC += $(if $(findstring $(KALSIM_CONFIG), True), audio_pios.c)

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = audio_pios

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

