############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
# Unit 400, Cambridge Science Park,
# Milton Road, Cambridge, CB4 0WH.  UK
# Tel: +44 1223 692000  Fax: +44 1223 692001
#
############################################################################
#
# COMPONENT:    const_data
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


# Previously: C_SRC += const_data_$(CHIP_ARCH).c
C_SRC += $(if $(findstring $(CHIP_ARCH), hydra), const_data_hydra.c)
C_SRC += $(if $(findstring $(CHIP_ARCH), napier), const_data_hydra.c)
C_SRC += $(if $(findstring $(CHIP_ARCH), a7da_kas), const_data_a7da_kas.c)
C_SRC += $(if $(findstring $(CHIP_ARCH), bluecore), const_data_bluecore.c)

# Previously: S_SRC +=    memory_$(CHIP_ARCH).asm
S_SRC += $(if $(findstring $(CHIP_ARCH), hydra), memory_hydra.asm)
S_SRC += $(if $(findstring $(CHIP_ARCH), napier), memory_hydra.asm)
S_SRC += $(if $(findstring $(CHIP_ARCH), a7da_kas), memory_a7da_kas.asm)
S_SRC += $(if $(findstring $(CHIP_ARCH), bluecore), memory_bluecore.asm)

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = const_data

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
