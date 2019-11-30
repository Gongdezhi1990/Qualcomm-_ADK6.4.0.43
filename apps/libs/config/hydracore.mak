# Define features to build
CONFIG_FEATURES:=CONFIG_HYDRACORE CONFIG_I2S_MCLK CONFIG_HAVE_PIO_BANKS CONFIG_CHANNEL_SELECTION CONFIG_PHY_PREFERENCES

# Set the lib directories to exclude in the build (if unset all libraries are included)
# NOTE: Removing a lib directory from the following list implies the need of the item in the hydracore build,
# therefore MUST be added in chip-type-specific project e.g. libs_csra68105.x2p.
CONFIG_DIRS_FILTER=aghfp config config_test csr_a2dp_decoder_common_plugin csr_a2dp_encoder_common_plugin \
csr_voice_prompts_plugin csr_voice_assistant_plugin csr_ag_audio_plugin csr_common_example_plugin csr_dut_audio_plugin \
csr_subwoofer_plugin debongle display_example_plugin display_plugin_cns10010 firmware_mock fm_plugin_if \
fm_rx_api fm_rx_plugin leds_flash leds_manager leds_manager_if leds_rom obex_parse swat \
 wbs csr_cvc_common_plugin \
broadcast_msg_interface broadcast_status_msg_structures erasure_code_common erasure_code_input_stats \
csr_broadcast_audio_plugin csr_broadcast_receiver_plugin

# Pull in the Kymera build configuration
path := $(subst hydracore.mak,,$(abspath $(lastword $(MAKEFILE_LIST))))
include $(path)/kymera.mak
-include $(path)/fastpair.mak
