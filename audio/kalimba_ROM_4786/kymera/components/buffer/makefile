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
# MODULE:       Buffer
# DESCRIPTION:  Library makefile.
#
############################################################################


#########################################################################
# Define root directory (relative so we can be installed anywhere)
#########################################################################

BUILD_ROOT = ../../build
include $(BUILD_ROOT)/roots.mkf

#########################################################################
# Patching
#########################################################################

PATCH_DIR = $(AUTOGEN_ROOT)/patches
PATCH_SRC += cbuffer_asm.asm
PATCH_SRC += $(if $(findstring $(CHIP_ARCH), hydra), hydra/mmu_buff_asm.asm)
PATCH_SRC += $(if $(findstring $(CHIP_ARCH), napier), hydra/mmu_buff_asm.asm)
PATCH_SRC += cbuffer_ex_asm.asm
PATCH_SRC += $(if $(findstring $(CHIP_ARCH), hydra), hydra/mmu_buff_ex_asm.asm)
PATCH_SRC += $(if $(findstring $(CHIP_ARCH), napier), hydra/mmu_buff_ex_asm.asm)

GEN_PIDS = $(PATCH_DIR)/buffer_asm_patch_ids.txt

#########################################################################
# Enter source files and directories and header directories here.
#
# Makerules will add the standard interface paths
#########################################################################

# C files we include in all builds
C_SRC += cbuffer.c
C_SRC += octet_buffer.c
C_SRC += buffer_metadata.c
C_SRC += buffer_metadata_eof.c
C_SRC += cbuffer_16bit_be_zero_shift_ex.c
C_SRC += $(if $(findstring $(BUILD_METADATA_DUALCORE), true), buffer_metadata_kip.c, )

ifneq ($(TEST_BUILD), gcc)
# Assembler files we need for a normal kalimba build
S_SRC += cbuffer_asm.asm
S_SRC += octet_buffer_asm.asm
S_SRC += cbuffer_ex_asm.asm

# choose a target
S_PATH += $(if $(findstring $(CHIP_ARCH), bluecore), $(call myabspath,./bc), )
S_PATH += $(if $(findstring $(CHIP_ARCH), hydra), $(call myabspath,./hydra), )
S_PATH += $(if $(findstring $(CHIP_ARCH), napier), $(call myabspath,./hydra), )
S_SRC += $(if $(findstring $(CHIP_ARCH), bluecore), mmu_buff_asm.asm,)
S_SRC += $(if $(findstring $(CHIP_ARCH), hydra), mmu_buff_asm.asm,)
S_SRC += $(if $(findstring $(CHIP_ARCH), napier), mmu_buff_asm.asm,)
S_SRC += $(if $(findstring $(CHIP_ARCH), hydra), mmu_buff_ex_asm.asm,)
S_SRC += $(if $(findstring $(CHIP_ARCH), napier), mmu_buff_ex_asm.asm,)

# build either the BC specific...
C_PATH += $(if $(findstring $(CHIP_ARCH), bluecore), $(call myabspath,./bc), )
C_SRC += $(if $(findstring $(CHIP_ARCH), bluecore), cbuff.c,)
# ... or build the Hydra specific
C_PATH += $(if $(findstring $(CHIP_ARCH), hydra), $(call myabspath,./hydra), )
C_SRC += $(if $(findstring $(CHIP_ARCH), hydra), cbuff.c,)
C_SRC += $(if $(findstring $(CHIP_ARCH), hydra), mmu_buff.c,)
# build for A7DA_KAS specific
C_PATH += $(if $(findstring $(CHIP_ARCH), a7da_kas), $(call myabspath,./a7da_kas), )
C_SRC += $(if $(findstring $(CHIP_ARCH), a7da_kas), cbuff.c,)

# build for NAPIER specific
C_PATH += $(if $(findstring $(CHIP_ARCH), napier), $(call myabspath,./hydra), )
C_SRC += $(if $(findstring $(CHIP_ARCH), napier), cbuff.c,)
C_SRC += $(if $(findstring $(CHIP_ARCH), napier), mmu_buff.c,)

else # gcc
# Fake buffer code for compiling with gcc.
C_PATH += ./test
C_SRC += host_cbuffer_stubs.c
C_SRC += $(if $(findstring $(CHIP_ARCH), hydra), host_mmu_buffer_stubs.c,)
endif

# path for platform specific private headers
H_PATH += $(if $(findstring $(CHIP_ARCH), hydra), $(call myabspath,./hydra), )
H_PATH += $(if $(findstring $(CHIP_ARCH), napier), $(call myabspath,./hydra), )

# generate asm header.
GEN_ASM_HDRS += octet_buffer.h

#########################################################################
# Enter final target file here (only 1 target should be specified)
#########################################################################

TARGET_EXE_ROOT =
TARGET_LIB_ROOT = buffer

#########################################################################
# Include the standard definitions and rules
#########################################################################

include $(BUILD_ROOT)/makerules.mkf
