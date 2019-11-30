############################################################################
# CONFIDENTIAL
# Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
# Unit 400, Cambridge Science Park,
# Milton Road, Cambridge, CB4 0WH.  UK
# Tel: +44 1223 692000  Fax: +44 1223 692001
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       Platform
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

C_SRC +=	pl_trace_mask.c
C_SRC +=	pl_error.c
C_SRC +=	profiler_utils.c

S_PATH += $(call myabspath,./$(CHIP_NAME))
S_SRC =		reset.asm
S_SRC +=	crt0.asm
S_SRC +=	error.asm
S_SRC +=	interrupt.asm
S_SRC +=	stack.asm
S_SRC +=	pl_memory.asm
S_SRC +=	circular_buffer_copy.asm
S_SRC +=	kalsim.asm
S_SRC +=	pl_intrinsics_asm.asm
S_SRC +=	profiler.asm

# Files for secondary core build
S_SRC +=        $(if $(BUILD_SECONDARY_CORE), crt_p1.asm,)

# If not building for target include a small set of C functions for compatibility.
# asm files above are ignored in GCC builds
C_SRC += $(if $(findstring $(TARGET_COMPILER), gcc), pl_intrinsics.c,)
C_SRC += $(if $(findstring $(TARGET_COMPILER), gcc), pl_interrupt.c,)

PATCH_SRC += profiler.asm
GEN_PIDS = $(PATCH_DIR)/platform_patch_ids.txt

GEN_ASM_HDRS += pl_trace_masks.h
GEN_ASM_HDRS += profiler_c.h

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = platform

#########################################################################
# Build tools extra switches
#########################################################################

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
