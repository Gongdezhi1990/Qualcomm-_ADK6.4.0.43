############################################################################
# CONFIDENTIAL
# Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.
# Unit 400, Cambridge Science Park,
# Milton Road, Cambridge, CB4 0WH.  UK
# Tel: +44 1223 692000  Fax: +44 1223 692001
#
############################################################################
#
# COMPONENT:    makefile
# MODULE:       
# DESCRIPTION:  Top-level makefile.
#               This makefile generates build definitions, runs an 
#               autogeneration phase, then builds the final executable by
#               invoking makefile_exe.
#
#               Top-level makefile can be for audio CPU firmware or for
#               test applications. All top level makefiles invoke same
#               makefile_exe and MUST specify build root and final target/
#               output files. Top level makefile can also optionally add 
#               source files, source and header paths to be included in the 
#               build.
#               
############################################################################

ALL_GOALS_EXCEPT_HELP = all debug dox clean verbose
.PHONY: $(ALL_GOALS_EXCEPT_HELP) help

# Report the machine we're building on
ifneq ($(strip $(COMPUTERNAME)),)
$(info $(shell echo Building on $(COMPUTERNAME)))
else
$(info $(shell echo Building on $(HOSTNAME)))
endif

#########################################################################
# Define root directory (relative so we can be installed anywhere)
# Top level makefiles MUST define BUILD_ROOT. Absolute path is preferred 
# for build root.
#########################################################################

BUILD_ROOT = $(CURDIR)

include $(BUILD_ROOT)/roots.mkf

#########################################################################
# Verify KCC version is valid
#########################################################################
KCC_VERIFY:= $(shell $(PYTHON) $(BUILD_ROOT)/verify_kcc_version.py $(OSTYPE))
ifneq (True, $(KCC_VERIFY))
$(error $(KCC_VERIFY))
endif

#########################################################################
# Include goals.mkf ASAP after defining BUILD_ROOT.
# This validates the targets supplied on the 'make' command line.
#########################################################################
include $(BUILD_ROOT)/goals.mkf

#########################################################################
# Create new log file for errors
export ERR_LOGFILE := $(call ospath,$(OUTPUT_ROOT)/__errorsLog.txt)

#########################################################################
# Build config-specific make file and pick up definitions from it
#########################################################################

ifneq ($(MAKECMDGOALS), help) # This won't get built for make help
ifeq ($(strip $(CONFIG)), )
$(error CONFIG must be specified. Type 'make help' for more information)
endif

ifneq ($(strip $(BUILD_EXTRA_DEFS)), )
    CONFIG_EXTRA_DEFS_OPT = -c '$(BUILD_EXTRA_DEFS)'
endif

$(shell $(MKDIR_P) $(OUTPUT_ROOT) $(STDERR_DEV_NULL))

debug: $(OUTPUT_ROOT)/$(CONFIG).mkf

.PHONY: $(ERR_LOGFILE)

$(ERR_LOGFILE) :
# Create new error file
	@echo "" > $(ERR_LOGFILE)   

# The include of the $(OUTPUT_ROOT)/$(CONFIG).mkf won't work if this is a direct rule
# this is why is dependent on the $(ERR_LOGFILE).
$(OUTPUT_ROOT)/$(CONFIG).mkf: $(ERR_LOGFILE) $(BUILD_ROOT)/gen_config.py
	$(info ***** Parsing config files *****)
	$(PYTHON) $(BUILD_ROOT)/gen_config.py -o $(OUTPUT_ROOT) -k $(KYMERA_ROOT) -f config.$(CONFIG)

-include $(OUTPUT_ROOT)/$(CONFIG).mkf
endif

TARGET_MAKEFILE = $(call myabspath,$(CURDIR)/makefile)

LIBS += $(addprefix $(KYMERA_ROOT)/,$(CFG_LIBS))

COMPILE_EXTRA_DEFS += $(addprefix -DINSTALL_OPERATOR_,$(CFG_CAPABILITIES))

AUTOGEN_ONLY_LIBS += $(addprefix $(KYMERA_ROOT)/,$(AUTOGEN_CFG_LIBS))


#########################################################################
# Top level makefile can also optionally add source files, source and header 
# paths to be included in the build.
# Enter source files (C_SRC) and (C_PATH) directories and (H_PATH) header 
# directories here.
#
# Makerules will add the standard paths
#########################################################################

#########################################################################
# The final target files MUST be set by the top level makefile and passed
# as a parameter to makefile_exe. Final target can EITHER be an exe file
# OR library and is specified as TGT_EXE or TGT_LIB.
# NB at this stage we expect the target always to be EXE.
#########################################################################
TGT_EXE = kymera_$(CHIP_NAME)
TGT_LIB = 

#########################################################################
# Include generic make rules.
#########################################################################
# NB This includes the rule for 'make help'.
include $(BUILD_ROOT)/makerules_common.mkf

#########################################################################
# Top-level rules for "clean" target
# Individual components are cleaned via makerules_src.mkf
# Note protection against undefined path names if cleaning all files
#########################################################################
clean:
	-@$(RM_F) $(OUTPUT_ROOT)/$(CONFIG).mkf
	-@$(RM_F) $(OUTPUT_ROOT)/builddefs.mkf
ifneq ($(strip $(AUTOGEN_ROOT)),)
	-@$(RM_RF) $(AUTOGEN_ROOT)/*
endif
ifneq ($(strip $(DEBUG_EXTCONST_ROOT)),)
	-@$(RM_F) $(call ospath,$(DEBUG_EXTCONST_ROOT)/*)
endif

#########################################################################
# Autogeneration phase: run 'make autogen' for all libs and top-level exe
#
# This is designed such that we could, if we wanted, separate the 
# autogeneration of headers from that of source files. To do that you 
# would build with different targets: autogen_hdr and autogen_src.
#########################################################################
LDIRS:=			$(dir $(LIBS) $(AUTOGEN_ONLY_LIBS))
AUTOGEN_LIB:=		${LDIRS:%=%_autogen}
AUTOGEN_PRE_EXE:=	${TGT_EXE:%=%_autogen_pre_exe}
AUTOGEN_POST_EXE:=	${TGT_EXE:%=%_autogen_post_exe}

ifneq ($(strip $(AUTOGEN_DEPS)),)
# Parse any autogen dependencies
# Because the list can contain multiple entries, the result of the text substitution is output to a file
# which is then subsequently included by this makefile.
# (It isn't possible to directly expand the list variable to multiple lines)

SUBST_COLON := $(subst :,/_autogen:$(KYMERA_ROOT)/,$(AUTOGEN_DEPS))
SUFFIX_EACH_WORD := $(addsuffix /_autogen,$(SUBST_COLON))
AUTOGEN_DEP_TMP := $(addprefix $(KYMERA_ROOT)/, $(SUFFIX_EACH_WORD))

$(shell $(RM) $(OUTPUT_ROOT)/autogen_deps.mkf $(STDERR_DEV_NULL))
$(foreach dep, $(AUTOGEN_DEP_TMP), $(info $(shell echo $(dep) >> $(OUTPUT_ROOT)/autogen_deps.mkf)))
include $(OUTPUT_ROOT)/autogen_deps.mkf
endif

.PHONY:		$(AUTOGEN_LIB) $(AUTOGEN_PRE_EXE) $(AUTOGEN_POST_EXE) DO_BOILERPLATE


DO_BOILERPLATE:
ifneq ($(findstring verbose, $(MAKECMDGOALS)), )
	$(info ----------------------------------------------------)
	$(info Making $(call myabspath,$(CURDIR)/make) $(MAKEFLAGS) $(MAKECMDGOALS))
	$(info Build environment:)
	@echo SHELL $(shell ver)
	$(info MAKE: $(MAKE))
	$(info COMPILE: $(COMPILE))
	$(PYTHON) -V
	$(info ----------------------------------------------------)
else
#Do nothing
	@$(TRUE)
endif

# Per-component autogen
$(AUTOGEN_LIB): $(AUTOGEN_PRE_EXE)
ifeq ($(findstring clean, $(MAKECMDGOALS)), )
	$(info Autogenerating $@...)
	$(MAKE) -C $(@D) autogen RELEASE_BUILD='${RELEASE_BUILD}' REL_EXT_BUILD='${REL_EXT_BUILD}'
else
#Don't bother autogenerating if all we want to do is clean.
	@$(TRUE)
endif

# Top-level autogen BEFORE the per-component stuff
$(AUTOGEN_PRE_EXE): DO_BOILERPLATE
ifeq ($(findstring clean, $(MAKECMDGOALS)), )
	$(info Autogenerating components for $@...)
	${MAKE} -f $(BUILD_ROOT)/makefile_exe autogen_pre_exe TGT_EXE='${TGT_EXE}' TGT_LIB='${TGT_LIB}' BUILD_ROOT='$(BUILD_ROOT)' CONFIG='${CONFIG}' GEN_EXTRA_TASK_DIRS='$(GEN_EXTRA_TASK_DIRS)'
else
#Don't bother autogenerating if all we want to do is clean.
	@$(TRUE)
endif

# Top-level autogen AFTER the per-component stuff
$(AUTOGEN_POST_EXE): $(AUTOGEN_LIB)
ifeq ($(findstring clean, $(MAKECMDGOALS)), )
	$(info Autogenerating components for $@...)
	${MAKE} -f $(BUILD_ROOT)/makefile_exe autogen_post_exe TGT_EXE='${TGT_EXE}' TGT_LIB='${TGT_LIB}' BUILD_ROOT='$(BUILD_ROOT)' CONFIG='${CONFIG}' GEN_EXTRA_TASK_DIRS='$(GEN_EXTRA_TASK_DIRS)'
else
	$(info Not autogenerating - all we want to do is clean.)
endif


#########################################################################
# Make sure autogeneration is done, then kick off the main build.
# The first step is to create the err_logfile used by cleanup which is created
# here so it happens only once
# makefile_exe takes the following parameters
#   TGT_EXE - Target EXE name. Empty if TGT_LIB is specified
#   TGT_LIB - Target LIB name. Empty if TGT_EXE is specified
#   INIT_C_PATH - Initial value for C_PATH.
#   INIT_H_PATH - Initial value for H_PATH.
#   INIT_C_SRC - Initial value for C_SRC.
#   INIT_S_SRC - Initial value for S_SRC.
# In addition makefile_exe requires top level makefile to specify build
# root, config, compile flags etc.
#########################################################################
.PHONY:		MAKE_ALL_TARGETS_ONCE
$(ALL_GOALS_EXCEPT_HELP):	MAKE_ALL_TARGETS_ONCE

MAKE_ALL_TARGETS_ONCE:	$(MAKE_DEPS) $(AUTOGEN_POST_EXE)
# Since we pass on all the MAKECMDGOALS to makefile_exe, we only need to execute
# this rule once for all targets.
	$(info Autogeneration phase finished. Starting make proper...)
	${MAKE} -f $(BUILD_ROOT)/makefile_exe $(MAKECMDGOALS) TGT_EXE='${TGT_EXE}' TGT_LIB='${TGT_LIB}' INIT_C_PATH='${C_PATH}' INIT_H_PATH='${H_PATH}' INIT_S_PATH='$(S_PATH)' INIT_C_SRC='${C_SRC}' INIT_S_SRC='${S_SRC}' BUILD_ROOT='$(BUILD_ROOT)' CONFIG='${CONFIG}' COMPILE_EXTRA_DEFS='${COMPILE_EXTRA_DEFS}' ASSEMBLE_EXTRA_DEFS='${ASSEMBLE_EXTRA_DEFS}' LINK_EXTRA_FLAGS='${LINK_EXTRA_FLAGS}' PRINT_DEFINES='${PRINT_DEFINES}' TARGET_MAKEFILE='${TARGET_MAKEFILE}'

