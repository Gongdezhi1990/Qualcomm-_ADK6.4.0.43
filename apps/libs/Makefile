# Top level makefile to build the application libraries
# Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.

dirs := $(filter-out $(patsubst %/,%,$(sort $(dir $(wildcard */BROKEN)))),$(patsubst %/,%,$(sort $(dir $(wildcard */*.h)))))
# LIBS can be passed in the command line, but if not then build them all
LIBS ?= $(dirs)

# PRIVATE_LIBS can be set in the project, these libraries will be made private
# i.e. scrambled by kalscramble. They're given a .pa extension and live in a separate folder
PRIVATE_LIBS ?=
PRIVATE_LIBS_SEED ?=

dirs_warning_free =

DEFS += -DGATT_DBI_LIB

# Get the location of the libs directory being built by getting the Makefile
# path (but replace the word Makefile with nothing)
LIBRARIES_DIR := $(subst Makefile,,$(abspath $(lastword $(MAKEFILE_LIST))))

# $(CONFIGS) will look like "CONF1,CONF2,etc"
# Replace commas with spaces and " with nothing 
# in the $(CONFIGS) to create $(CONFIG_LIST)
comma := ,
empty :=
space := $(empty) $(empty)

ifneq (,$(CONFIGS))
CONFIG_LIST := $(subst ",,$(subst $(comma),$(space),$(CONFIGS)))
endif

# If no $(CONFIGS) were supplied at command line 
# set CONFIG_LIST to default
CONFIG_LIST ?=default

# Debug output
# $(1) = debug output
ifneq (,$(SCRIPT_DEBUG))
define print
$(warning $(1))
endef
endif

# Fail build if configuration is not default and makefile 
# $(1).mak does not exist
define fail_if_no_config
$(if $(strip $(filter-out default,$(1))),\
    $(if $(wildcard $(LIBRARIES_DIR)config/$(1).mak), , \
        $(error $(1).mak not found in config)\
     ),\
 )
endef

define get_build_config
# Get information from build configuration file
# $(1) = build configuration name

# Don't enable any CONFIG_FEATURES by default
CONFIG_FEATURES:=

# Set some defaults (prevents bleed through from 
# previous runs for other configurations)
override LIBRARY_VERSION:=$(BASE_LIBRARY_VERSION)
override dirs:= $(BASE_DIRS)
override SUPPORTED_EXECUTION_MODES:=$(BASE_EXECUTION_MODES)

# If no build configuration was provided then use 
# current value of LIBRARY_VERSION and don't try to
# load any CONFIG_FEATURES
ifneq (default,$(1))

# Set LIBRARY_VERSION to base_config
override LIBRARY_VERSION:=$(BASE_LIBRARY_VERSION)_$(1)

endif

# Prevent bleed through of config settings
CONFIG_EXECUTION_MODES:=
CONFIG_DIRS_FILTER:=
CONFIG_DIRS_NOT_WARNING_FREE:=

# Now read the config file
include config/$(1).mak

endef

define set_execution_modes_and_dirs
# Do nothing if this is the default configuration
#ifneq (default,$(1))

# Always use execution modes if specified in the config
ifneq (,$(CONFIG_EXECUTION_MODES))
override SUPPORTED_EXECUTION_MODES:=$(CONFIG_EXECUTION_MODES)
endif

# Always use dirs if specified in the config
ifneq (,$(CONFIG_DIRS_FILTER))
override dirs:=$(filter-out $(CONFIG_DIRS_FILTER), $(dirs))
endif

override dirs_warning_free:=$(dirs)
ifneq (FALSE,$(EXTRA_WARNINGS))
  override dirs_warning_free:=
else
  ifneq (,$(CONFIG_DIRS_NOT_WARNING_FREE))
    override dirs_warning_free:=$$(filter-out $(CONFIG_DIRS_NOT_WARNING_FREE), $(dirs))
  endif
endif

#endif
endef

# It's all high-level magic from here on in
# Pay no attention to the man behind the curtain.

# Generate lib<library> targets for building selected libraries only
define add_libbuild
dirs += $(1)
lib$(1) :: install
endef

libtargets := $(filter $(foreach l,$(dirs),lib$(l)),$(MAKECMDGOALS))

ifneq (,$(libtargets))
dirs = 
dirs_warning_free = 
endif

$(foreach addlib,$(libtargets),$(eval $(call add_libbuild,$(patsubst lib%,%,$(addlib)))))

# Check if we've been told to ship broken libraries, and refuse
broken := $(wildcard $(addsuffix /BROKEN,$(dirs)))
ifneq (,$(broken))
$(error Refusing to install libraries marked as broken: $(patsubst %/,%,$(dir $(broken))))
endif

build :: dbi no_ble msg_host header uuids

install :: build helpers

# We support two scenarios:
# 1) Building using Heracles/ubuild: this sets MAKEFILE_RULES_DIR to point at Makefile.rules
# 2) Building in the traditional way with a single BLUELAB installation
# Note: BLUELAB must be defined in any case so that Makefile.rules does the 
# right thing for the libs build
ifdef MAKEFILE_RULES_DIR
    BLUELAB = $(MAKEFILE_RULES_DIR)
endif
include $(BLUELAB)/Makefile.rules

ifneq (TRUE, $(DISABLE_SHIM))
install-shim :: install
endif

# Stash information that is overwritten later
BASE_LIBRARY_VERSION:=$(LIBRARY_VERSION)
BASE_EXECUTION_MODES:=$(SUPPORTED_EXECUTION_MODES)
BASE_DIRS:=$(dirs)

define built_from
# Record the source tree, so switching to a new library set replaces files
# $(1) = $(LIBRARY_VERSION)
$(shell $(mkdir) $(dir $(call profiles_inc_dir,$(1))/built_from))
$(shell $(recordflags) $(call profiles_inc_dir,$(1))/built_from $(shell $(pwd)))
endef

define get_aliases_and_variants
# $(1) = directory containing source, whose name is usually the library name and header name (e.g. a2dp)
# $(2) = $(CONFIG_FEATURES) for this build

ALIAS:= $(1)
#set ALIAS by default to dir name, so that if there are no aliases the foreach loop will still happen once
VARIANTS:=

# Make sure each library starts with blank feature CFLAGS
$(foreach f, $(2), \
  CFLAGS_$(f):= 
)

# This includes the VARIANTS file in the directory, which may adjust the CFLAGS and define variants included in the build
-include $(1)/VARIANTS

# Include the default variant here 
VARIANTS+=default

endef

define get_suffix
# Convert a variant name into a suffix
# $(1) = variant name (e.g. debug)
# Prepend an _ (so debug becomes _debug) except for default which has an empty suffix
suffix:=$(filter-out _default,_$(strip $(1)))

endef

ifdef BUILDOUTPUT_PATH
define make_dirs
$(foreach v,$(3), \
  $(eval $(call get_suffix, $(v))) \
  $(eval $(call get_helpers,$(2),$(LIBRARY_VERSION))) \
  $(shell $(mkdir) $(1)/$(2)/$(v)/$(LIBRARY_VERSION)) \
)
endef

endif

# need to use LIBRARY_VERSION in variable names (in place of configuration too)

define header_rules
# $(1) = directory containing source, whose name is usually the library name and header name (e.g. a2dp)
# $(2) = library alias (e.g. a2dp when the dir is actually called ad2p_new)
# $(3) = $(LIBRARY_VERSION)

hdr_$(1) := $(call profiles_inc_dir,$(3))/$(2).h
dbi_$(1) := $$(patsubst $(1)/%_db.dbi,$(call profiles_inc_dir,$(3))/%_db.dbi,$$(wildcard $(1)/*_db.dbi))
uuids_$(1) := $$(patsubst $(1)/%_uuids.h,$(call profiles_inc_dir,$(3))/%_uuids.h,$$(wildcard $(1)/*_uuids.h))
no_ble_$(1) := $$(patsubst $(1)/%_no_ble.h,$$(call profiles_inc_dir,$(3))/%_no_ble.h,$$(wildcard $(1)/$(1)_no_ble.h))
msg_host_$(1) := $$(patsubst $(1)/%_msg_host.h,$$(call profiles_inc_dir,$(3))/%_msg_host.h,$$(wildcard $(1)/$(1)_msg_host.h))
par_$(1) := $$(addsuffix .c,$$(basename $$(wildcard $(1)/*.parse)))
src_$(1)_$(3) := $$(par_$(1)) $$(filter-out $$(par_$(1)) $(1)/main.c,$$(wildcard $(1)/*.c))
ifeq (TRUE, $(DISABLE_SHIM))
src_$(1)_$(3) := $$(filter-out $(1)/$(1)_shim.c, $$(src_$(1)_$(3)))
endif
all_srcs += $$(src_$(1)_$(3))
all_hdrs += $$(hdr_$(1))

header :: $$(hdr_$(1)) $$(patsubst %.c,%.h,$$(par_$(1)))

$$(hdr_$(1)) : $(1)/$(2).h
	$$(mkdir) $$(dir $$@)
	$$(copyfile) $$^ $$@
    
no_ble :: $$(no_ble_$(1))

$$(no_ble_$(1)) : $(1)/$(2)_no_ble.h
	$$(mkdir) $$(dir $$@)
	$$(copyfile) $$^ $$@    

msg_host :: $$(msg_host_$(1))

$$(msg_host_$(1)) : $(1)/$(2)_msg_host.h
	$$(mkdir) $$(dir $$@)
	$$(copyfile) $$^ $$@    

dbi :: $$(dbi_$(2))

$$(dbi_$(1)) : $(1)/*_db.dbi
	$$(mkdir) $$(dir $$@)
	$$(copyfile) $$^ $$@

uuids :: $$(uuids_$(2))

$$(uuids_$(1)) : $(1)/*_uuids.h
	$$(mkdir) $$(dir $$@)
	$$(copyfile) $$^ $$@

ifneq (TRUE, $(DISABLE_SHIM))
shim_$(1) := $$(patsubst $(1)/%.h,$$(call profiles_inc_dir,$(3))/%.h,$$(wildcard $(1)/$(1)_shim.h))

# Internal CSR target for shim code for library tests
install :: $$(shim_$(1))

$$(shim_$(1)) : $(1)/$(1)_shim.h
	$$(mkdir) $$(dir $$@)
	$$(copyfile) $$^ $$@
endif

ifneq (,$$(par_$(1)))
.PRECIOUS : $$(par_$(1))

clean ::
	$$(del) $$(par_$(1)) $$(addsuffix .h, $$(basename $$(par_$(1))))
endif

endef # header_rules

define get_features_src
# Generate a list of source files for a lib that are specific to a given feature
# $(1) = directory containing source, whose name is usually the library name (e.g. a2dp)
# $(2) = $(CONFIG_FEATURES) for this build

# For each config feature X, create a list of C files in /lib/X.
$(foreach f, $(2), \
  SRC_$(1)_$(f) = $(wildcard $(1)/$(f)/*.c)
)

endef

define replace_src
# $(1) = directory containing source, whose name is usually the library name (e.g. a2dp)
# $(2) = variant (e.g. debug)
# $(3) = $(LIBRARY_VERSION)
# $(4) = $(CONFIG_FEATURES) instance
src_$(1)_$(3) = $(filter-out $(subst $(1)/$(4)/,$(1)/,$(SRC_$(1)_$(4))),$(src_$(1)_$(3)))
src_$(1)_$(3) += $(SRC_$(1)_$(4))
INC_$(1)_$(2)_$(3) += -I$(LIBRARIES_DIR)$(1)/$(4)
endef

define variant_rules
# $(1) = directory containing source, whose name is usually the library name (e.g. a2dp)
# $(2) = variant (e.g. debug)
# $(3) = variant with underscore prepended (or empty for default variant)
# $(4) = $(LIBRARY_VERSION)
# $(5) = $(CONFIG_FEATURES) for this build

# Check that a (not empty) CONFIG_FEATURES has a CONFIG_ (prefix)
ifneq (,$(5))
  ifeq (,$(findstring CONFIG_,$(5)))
    $$(warning **** CONFIG_FEATURES [$(CONFIG_FEATURES)] DOES NOT HAVE CONFIG_ PREFIX)
  endif
endif

# Make CFLAGS unique across libraries
CFLAGS_$(1)_$(2)_$(4) := $(CFLAGS$(3))

# If this library is in the list of directories to be warning clean, add errors fatal
ifeq (,$(filter-out $(dirs_warning_free), $(1)))
  CFLAGS_$(1)_$(2)_$(4) += -Werror
endif

# Stop them bleeding across into other libraries
CFLAGS$(3) :=

# For a given feature X add in CFLAGS_X to the CFLAGS
$(foreach f, $(5), \
  CFLAGS_$(1)_$(2)_$(4) += $(CFLAGS_$(f))
)

# Filter out files with names matching feature source files from /lib from src_$(1)_$(4) 
# and add in the feature version from /lib/X e.g. For feature X in lib, /lib/X/file.c 
# will remove lib/file.c from the build (if it exists) and replace it with file /lib/X/file.c.
# Finally add -I lib/X to include any feature specific headers
$(foreach f, $(5), $(eval $(call replace_src,$(1),$(2),$(4),$(f))))

# Make sure we also include headers from the lib directory
INC_$(1)_$(2)_$(4) += -I$(LIBRARIES_DIR)$(1)

# Add the -I options to CFLAGS
CFLAGS_$(1)_$(2)_$(4) += $$(INC_$(1)_$(2)_$(4))

endef # variant_rules

MAKE_MAJOR_VERSION = $(firstword $(subst ., ,$(MAKE_VERSION)))
MAKE_MAJOR_VERSION_IS_4 = $(filter 4, $(firstword $(sort $(MAKE_MAJOR_VERSION) 4)))

ifndef BUILDOUTPUT_PATH
define object_rules
# $(1) = directory containing source, whose name is usually the library name (e.g. a2dp)
# $(2) = variant (e.g. debug)
# $(3) = variant with underscore prepended (or empty for default variant)
# $(4) = execution mode
# $(5) = library alias (e.g. a2dp when the dir is actually called ad2p_new)
# $(6) = $(LIBRARY_VERSION)
# $(7) = configuration from $(CONFIG_LIST)

obj_$(1)_$(2)_$(6)_$(4) := $$(patsubst $(1)/%.c,$(1)/$(2)/$(6)/$(4)/%.o,$$(src_$(1)_$(6)))

ifneq (,$$(obj_$(1)_$(2)_$(6)_$(4)))

dep_$(1)_$(2)_$(6)_$(4) := $$(call dependencies, $$(obj_$(1)_$(2)_$(6)_$(4)))
lib_$(1)_$(2)_$(6)_$(4) := $$(call lib_dir,$(4),$(6))/lib$(5)$(3).a

ifeq ($(5), $(filter $(5), $(PRIVATE_LIBS)))
private_lib_$(1)_$(2)_$(6)_$(4) := $$(call private_lib_dir,$(6))/lib$(5)$(3).pa
obj_$(1)_$(2)_$(6)_$(4) := $$(obj_$(1)_$(2)_$(6)_$(4):.o=.po)
private_lib_symbols_$(1)_$(2)_$(6)_$(4) := $(1)/$(1).symbols

CFLAGS_$(1)_$(2)_$(6) += $(COMPILE_AS_PRIVATE_FLAGS)

$$(private_lib_$(1)_$(2)_$(6)_$(4)) : $$(lib_$(1)_$(2)_$(6)_$(4)) | $$(private_lib_symbols_$(1)_$(2)_$(6)_$(4))
	$$(info Making private library from: $$(notdir $$^))
	$$(info Keeping public symbols from: $$|)
	-$$(mkdir) $$(dir $$@)
	$$(KSCRM) -o $$@ -s $$(PRIVATE_LIBS_SEED) -i $$| $$^
	$$(RM) $$^
	$$(info Generated private library is at: $$@)
endif

build :: $$(obj_$(1)_$(2)_$(6)_$(4))

clean ::
	$$(del) $$(lib_$(1)_$(2)_$(6)_$(4))
	$$(del) $$(obj_$(1)_$(2)_$(6)_$(4))
	$$(del) $$(dep_$(1)_$(2)_$(6)_$(4))
	$$(del) $$(lib_$(1)_$(2)_$(6)_$(4).a=.pa)
	$$(del) $$(obj_$(1)_$(2)_$(6)_$(4).o=.po)
	$$(del) $(1)/$(2)/$(6)/$(4)/depend/flags.compile

install :: $$(lib_$(1)_$(2)_$(6)_$(4)) $$(private_lib_$(1)_$(2)_$(6)_$(4))

$$(lib_$(1)_$(2)_$(6)_$(4)) : $$(obj_$(1)_$(2)_$(6)_$(4))
	-$$(mkdir) $$(dir $$@)
	$$(ar) cr $$@ $$^

$$(obj_$(1)_$(2)_$(6)_$(4)) : $$(hdr_$(1))

# If the dependency files don't exist, we need to force compilation of
# the _db.h files or else parallel make might get ahead of itself
ifeq (,$$(MAKE_MAJOR_VERSION_IS_4))
# Something weird happens with make 3.81 and these force_gattdb targets.  But
# we don't need them in the builds we still use 3.81 for, i.e. the XAP builds,
# so just disable the mechanism in all cases.
FORCE_GATTDB_$(1) :=
else
ifeq (,$$(strip $$(foreach f,$$(dep_$(1)_$(2)_$(6)_$(4)),$$(wildcard $$f))))
FORCE_GATTDB_$(1) := force_gattdb_$(1)
else
FORCE_GATTDB_$(1) :=
endif
endif

# Make sure the config .mak file is included when docompile/$(compileflags) are used
$(1)/$(2)/$(6)/$(4)/%.o : $(1)/%.c $(1)/$(2)/$(6)/$(4)/depend/flags.compile  $$(FORCE_GATTDB_$(1))
	$$(eval $$(call get_build_config,$(7)))
	$$(call docompile,$$(CFLAGS_$(1)_$(2)_$(6)),$(4))

# Rename .o files into .po when used in a private library
# They need the .po extension so that the linker knows to put them
# in the private sections
%.po : %.o
	$$(rename) $$< $$@

-include $$(dep_$(1)_$(2)_$(6)_$(4)) $(1)/$(2)/$(6)/$(4)/dummy.force

$(1)/$(2)/$(6)/$(4)/dummy.force :
	$$(eval $$(call get_build_config,$(7)))
	@$$(mkdir) $(1)/$(2)/$(6)/$(4)/depend
	@$$(recordflags) $(1)/$(2)/$(6)/$(4)/depend/flags.compile $$(compileflags) $$(4) $$(CFLAGS_$(1)_$(2)_$(6))
endif

endef # object_rules

else
# BUILDOUTPUT_PATH

define object_rules
# $(1) = directory containing source, whose name is usually the library name (e.g. a2dp)
# $(2) = variant (e.g. debug)
# $(3) = variant with underscore prepended (or empty for default variant)
# $(4) = execution mode
# $(5) = library alias (e.g. a2dp when the dir is actually called ad2p_new)
# $(6) = $(LIBRARY_VERSION)
# $(7) = configuration from $(CONFIG_LIST)
# $(8) = optional output location

obj_$(1)_$(2)_$(6)_$(4) := $$(patsubst $(1)/%.c,$(8)/$(1)/$(2)/$(6)/$(4)/%.o,$$(src_$(1)_$(6)))

ifneq (,$$(obj_$(1)_$(2)_$(6)_$(4)))

dep_$(1)_$(2)_$(6)_$(4) := $$(call dependencies, $$(obj_$(1)_$(2)_$(6)_$(4)))
lib_$(1)_$(2)_$(6)_$(4) := $$(call lib_dir,$(4),$(6))/lib$(5)$(3).a

ifeq ($(5), $(filter $(5), $(PRIVATE_LIBS)))
private_lib_symbols_$(1)_$(2)_$(6)_$(4) := $(1)/$(1).symbols

CFLAGS_$(1)_$(2)_$(6) += $(COMPILE_AS_PRIVATE_FLAGS)

$$(private_lib_$(1)_$(2)_$(6)_$(4)) : $$(lib_$(1)_$(2)_$(6)_$(4)) | $$(private_lib_symbols_$(1)_$(2)_$(6)_$(4))
	$$(info Making private library from: $$(notdir $$^))
	$$(info Keeping public symbols from: $$|)
	-$$(mkdir) $$(dir $$@)
	$$(KSCRM) -o $$@ -s $$(PRIVATE_LIBS_SEED) -i $$| $$^
	$$(RM) $$^
	$$(info Generated private library is at: $$@)
endif

build :: $$(obj_$(1)_$(2)_$(6)_$(4))

clean ::
	$$(del) $$(lib_$(1)_$(2)_$(6)_$(4))
	$$(del) $$(obj_$(1)_$(2)_$(6)_$(4))
	$$(del) $$(dep_$(1)_$(2)_$(6)_$(4))
	$$(del) $$(lib_$(1)_$(2)_$(6)_$(4).a=.pa)
	$$(del) $$(obj_$(1)_$(2)_$(6)_$(4).o=.po)
	$$(del) $(8)/$(1)/$(2)/$(6)/$(4)/depend/flags.compile

install :: $$(lib_$(1)_$(2)_$(6)_$(4)) $$(private_lib_$(1)_$(2)_$(6)_$(4))

$$(lib_$(1)_$(2)_$(6)_$(4)) : $$(obj_$(1)_$(2)_$(6)_$(4))
	-$$(mkdir) $$(dir $$@)
	$$(ar) cr $$@ $$^

$$(obj_$(1)_$(2)_$(6)_$(4)) : $$(hdr_$(1))

# If the dependency files don't exist, we need to force compilation of
# the _db.h files or else parallel make might get ahead of itself
ifeq (,$$(MAKE_MAJOR_VERSION_IS_4))
# Something weird happens with make 3.81 and these force_gattdb targets.  But
# we don't need them in the builds we still use 3.81 for, i.e. the XAP builds,
# so just disable the mechanism in all cases.
FORCE_GATTDB_$(1) :=
else
ifeq (,$$(strip $$(foreach f,$$(dep_$(1)_$(2)_$(6)_$(4)),$$(wildcard $$f))))
FORCE_GATTDB_$(1) := force_gattdb_$(1)
else
FORCE_GATTDB_$(1) :=
endif
endif

# Make sure the config .mak file is included when docompile/$(compileflags) are used
$(8)/$(1)/$(2)/$(6)/$(4)/%.o : $(1)/%.c $(8)/$(1)/$(2)/$(6)/$(4)/depend/flags.compile  $$(FORCE_GATTDB_$(1))
	$$(eval $$(call get_build_config,$(7)))
	$$(call docompile,$$(CFLAGS_$(1)_$(2)_$(6)),$(4))

# Rename .o files into .po when used in a private library
# They need the .po extension so that the linker knows to put them
# in the private sections
%.po : %.o
	$$(rename) $$< $$@

-include $$(dep_$(1)_$(2)_$(6)_$(4)) $(1)/$(2)/$(6)/$(4)/dummy.force

$(1)/$(2)/$(6)/$(4)/dummy.force :
	$$(eval $$(call get_build_config,$(7)))
	@$$(mkdir) $(8)/$(1)/$(2)/$(6)/$(4)/depend
	@$$(recordflags) $(8)/$(1)/$(2)/$(6)/$(4)/depend/flags.compile $$(compileflags) $$(4) $$(CFLAGS_$(1)_$(2)_$(6))
endif

endef # object_rules
endif

define gattdb_rules
# $(1) = directory containing source, whose name is usually the library name (e.g. gatt_battery_server)
#
dbh_$(1) := $$(patsubst $(1)/%.dbi,$(1)/%.h,$$(wildcard $(1)/*.dbi))

# Rule to force recompilation of the .h files from .dbi files
force_gattdb_$(1) : $$(dbh_$(1))
.PHONY: force_gattdb_$(1)

ifneq (,$$(dbh_$(1)))

build :: $$(dbh_$(1))

# Dependencies are emitted by KCC on Windows using backslashes, and make doesn't 
# recognise a rule to create $(1)/%.h as a satisfying a dependency on $(1)\%.h.
# This expands to a set of rules, one per dbi .h file making the backslash 
# form of the file pathname depend on the forwardslash form.
$$(eval $$(foreach db_hdr, $$(dbh_$(1)), $$(subst /,\,$$(db_hdr)) : $$(db_hdr)))

$(1)/%.h : $(1)/%.dbi_
	$$(call dogattdbi,$(1))

$(1)/%.dbi_ $(1)/depend/%db.d : $(1)/%.dbi
	$$(call dogattdbi_pre,$(1))

clean_dbh ::
	$$(del) $$(dbh_$(1))

endif

endef # gattdb_rules


define get_helpers
# $(1) - library name
# $(2) - $(LIBRARY_VERSION)

helper_directory := $(call profiles_inc_dir,$(2)/helpers)

src_dir_$(1)  := $(1)/helpers
dest_dir_$(1) := $(helper_directory)/$(1)

helpers_needed_$(1) := $$(patsubst $$(src_dir_$(1))/%,%,$$(wildcard $$(src_dir_$(1))/*))
helpers_exist_$(1)  := $$(patsubst $$(dest_dir_$(1))/%,%,$$(wildcard $$(dest_dir_$(1))/*))
helpers_to_add_$(1) := $$(addprefix $$(dest_dir_$(1))/,$$(helpers_needed_$(1)))

helpers_to_remove += $$(addprefix $$(dest_dir_$(1))/,$$(filter-out $$(helpers_needed_$(1)),$$(helpers_exist_$(1))))

helpers :: $$(helpers_to_add_$(1))

$$(helpers_to_add_$(1)) : $$(dest_dir_$(1))/% : $(1)/helpers/%
	$(mkdir) $$(dir $$@)
	$(copyfile) $$^ $$@

endef #get_helpers

helpers :: | cleanup

cleanup ::
	$(del) $(helpers_to_remove)

define process_variant
# $(1) = library name (e.g. a2dp)
# $(2) = $(LIBRARY_VERSION)
# $(3) = $(CONFIG_FEATURES) for this build
# $(4) = variant (e.g. debug)
# $(5) = library alias
# $(6) = config

  ifneq ($(filter $(1), $(LIBS)),)
    $$(eval $$(call get_suffix, $(4)))
    $$(call print,$(1)$$(suffix))
    $$(eval $$(call variant_rules,$(1),$(4),$$(suffix),$(2),$(3)))
    $$(foreach mode,$(SUPPORTED_EXECUTION_MODES), \
      $$(eval $$(call get_suffix, $(4))) \
      $$(eval $$(call object_rules,$(1),$(4),$$(suffix),$$(mode),$(5),$(2),$(6),$(BUILDOUTPUT_PATH))) \
    )
  endif
endef

$(foreach c,$(CONFIG_LIST), \
  $(eval $(call fail_if_no_config,$(c))) \
  $(eval $(call get_build_config,$(c))) \
  $(call print,Loaded Configuration $(c)) \
  $(call print,Including config features $(CONFIG_FEATURES)) \
  $(call print,Building Variants) \
  $(eval $(call built_from,$(LIBRARY_VERSION))) \
  $(eval $(call set_execution_modes_and_dirs,$(c))) \
  $(foreach d,$(dirs), \
    $(eval $(call get_aliases_and_variants,$(d),$(CONFIG_FEATURES))) \
    $(eval $(call get_features_src,$(d),$(CONFIG_FEATURES))) \
    $(foreach a,$(ALIAS), \
      $(eval $(call header_rules,$(d),$(a),$(LIBRARY_VERSION))) \
      $(eval $(call gattdb_rules,$(d),)) \
      $(eval $(call get_helpers,$(d),$(LIBRARY_VERSION))) \
      $(eval $(call make_dirs,$(BUILDOUTPUT_PATH),$(d),$(VARIANTS))) \
      $(foreach v,$(VARIANTS), \
        $(eval $(call process_variant,$(d),$(LIBRARY_VERSION),$(CONFIG_FEATURES),$(v),$(a),$(c))) \
      ) \
    ) \
  ) \
)

doc_hdrs := $(sort $(wildcard $(addprefix $(firmware_inc_dir)/,*.h */*.h */*/*.h) $(all_hdrs)))

doc_header_stamp_file = $(BLUELAB)/../doc/reference/header_files

$(shell $(mkdir) $(dir $(doc_header_stamp_file)))
$(shell $(recordflags) $(doc_header_stamp_file) $(doc_hdrs))

doxygen : $(BLUELAB)/../doc/reference/html/index.html

$(BLUELAB)/../doc/reference/html/index.html : $(doc_hdrs) $(doc_header_stamp_file)
	$(MAKE) -C $(BLUELAB)/doc

# CSR internal target, requires PC-Lint 8.0

export BLUELAB

lint : header $(all_srcs)
	$(lint) -i../../../tools/pclint $(lintflags) $(all_srcs)
