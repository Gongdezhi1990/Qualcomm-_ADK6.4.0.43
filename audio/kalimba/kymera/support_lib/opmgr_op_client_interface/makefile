############################################################################
# CONFIDENTIAL
# Copyright (c) 2018 - 2018 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       opmgr_op_client_interface
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

S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true), dsl_opmgr_op_client_send_message.asm, ))
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true), dsl_opmgr_op_client_start_operator.asm, ))
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true), dsl_opmgr_op_client_stop_operator.asm, ))
S_SRC += $(if $(findstring $(KALSIM_CONFIG), True), ,$(if $(findstring $(CAPABILITY_DOWNLOAD_BUILD), true), dsl_opmgr_op_client_reset_operator.asm, ))

C_SRC += $(if $(findstring $(KALSIM_CONFIG), True), opmgr_op_client_interface.c)

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = opmgr_op_client_interface

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf

