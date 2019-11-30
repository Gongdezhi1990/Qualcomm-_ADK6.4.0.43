############################################################################
# CONFIDENTIAL
# Project Kymera
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Kalimba libraries
# DESCRIPTION:  Library makefile.
#
############################################################################


#########################################################################
# Define root directory (relative so we can be installed anywhere)
#########################################################################

BUILD_ROOT = ../../kymera/build
include $(BUILD_ROOT)/roots.mkf

#########################################################################
# Enter source files and directories and header directories here.
#
# Makerules will add the standard interface paths
#########################################################################

# All assembly source

S_SRC += buffer_interleave.asm


GEN_HDRS = $(AUTOGEN_ROOT)/patches/buffer_interleave_patch_ids.txt

PATCH_SRC += buffer_interleave.asm


$(GEN_HDRS) : $(PATCH_SRC) $(PATCH_GEN_SCRIPT)
# Make a patches subdirectory to hold all of temporarily generated stuff
	$(info $(shell $(MKDIR_P) $(call ospath,$(AUTOGEN_ROOT)/patches) $(STDERR_DEV_NULL)))
# Generate the patch ids for this component
	$(PYTHON) $(PATCH_GEN_SCRIPT) -o $(call ospath,$(AUTOGEN_ROOT)/patches/buffer_interleave_patch_ids.txt) $(PATCH_SRC)


#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = buffer_interleave

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
