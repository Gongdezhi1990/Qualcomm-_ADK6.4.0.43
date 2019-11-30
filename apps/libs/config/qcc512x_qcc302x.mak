# Pull in the Kymera build configuration
-include config/hydracore.mak

CONFIG_DIRS_FILTER := $(CONFIG_DIRS_FILTER) nfc_api nfc_cl 
# audio_input_i2s csr_i2s_audio_plugin audio_output csr_cvc_common_plugin

TEMP := $(CONFIG_DIRS_FILTER)
CONFIG_DIRS_FILTER = $(filter-out anc, $(TEMP))
