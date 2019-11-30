############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Kalimba math library
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
S_SRC += math_library.asm
S_SRC += math_library_c_stubs.asm

GEN_HDRS = $(AUTOGEN_ROOT)/patches/math_patch_ids.txt

PATCH_SRC += fft.asm
PATCH_SRC += fft_twiddle_alloc.asm
PATCH_SRC += math_library_c_stubs.asm

$(GEN_HDRS) : $(PATCH_SRC) $(PATCH_GEN_SCRIPT)
# Make a patches subdirectory to hold all of temporarily generated stuff
	$(info $(shell $(MKDIR_P) $(call ospath,$(AUTOGEN_ROOT)/patches) $(STDERR_DEV_NULL)))
# Generate the patch ids for this component
	$(PYTHON) $(PATCH_GEN_SCRIPT) -o $(call ospath,$(AUTOGEN_ROOT)/patches/math_patch_ids.txt) $(PATCH_SRC)

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = math

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
