############################################################################
# CONFIDENTIAL
# Copyright 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Rate
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

C_SRC += rate_compare.c
C_SRC += rate_conv.c
C_SRC += rate_match.c
C_SRC += rate_measure.c
C_SRC += rate_measure_metadata.c
C_SRC += rate_metadata.c
C_SRC += rate_pid.c
C_SRC += rate_platform.c
C_SRC += rate_ts_filter.c

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = rate

#########################################################################
# Build tools extra switches
#########################################################################

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
