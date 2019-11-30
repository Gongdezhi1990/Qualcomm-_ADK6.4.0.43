# This defines the config features that will be built into libraries
CONFIG_FEATURES:=CONFIG_LATENCY CONFIG_CRYPTO CONFIG_CHANNEL_SELECTION

# This sets the execution modes that will be supported by the build (vm, native or assisted)
CONFIG_EXECUTION_MODES:=assisted

# Pull in the non-Kymera build configuration
path = $(dir $(lastword $(MAKEFILE_LIST)))
include $(path)/no_kymera.mak
