$(info ############################################################)
$(info ############################################################)
$(info ##                Running config_build.mak                ##)
$(info ############################################################)

# Determine whether we are in a hosted environment or an ADK build
# and calculate the path to the source or executable respectively of
# the tool config_build.

CURRENT_DIR = $(CURDIR)

SRC_DIR?=.

ifndef PYTHON_TOOLS
CURRENT_DIR_PATH = $(CURRENT_DIR:\=/)
# Try local branch
PYTHON_TOOLS = $(strip $(wildcard $(firstword $(subst /vm/,/ vm/,$(CURRENT_DIR_PATH)))tools/BuildConfigScripts/src))
endif
ifeq ($(PYTHON_TOOLS),)
# Local branch dir didn't work... next look in the ADK
PYTHON_TOOLS = $(strip $(wildcard $(firstword $(subst /apps/,/ apps/,$(CURRENT_DIR_PATH)))tools/bin))
endif
ifeq ($(PYTHON_TOOLS),)
# If all else fails look in the DEVKIT
ifdef DEVKIT_ROOT
DEVKIT_ROOT_PATH = $(DEVKIT_ROOT:\=/)
PYTHON_TOOLS = $(strip $(wildcard $(DEVKIT_ROOT_PATH)/tools/bin))
endif
endif

ifndef PYTHON_TOOLS
  $(error Unknown Environment)
endif

PYTHON_TOOLS_LOCATION = $(subst \,/,$(PYTHON_TOOLS))
$(info $(PYTHON_TOOLS_LOCATION))

ifdef DEVKIT_ROOT
PYTHON=$(DEVKIT_ROOT)/tools/Python27/python
else
PYTHON=python
endif

RUN_CONFIG_BUILD_SRC:=$(PYTHON) $(PYTHON_TOOLS_LOCATION)/moduledef2configdef.py
RUN_CONFIG_BUILD_EXE:=$(PYTHON_TOOLS_LOCATION)/config_build.exe

CONFIG_BUILD_EXE_CMD:=$(if $(findstring src, $(PYTHON_TOOLS_LOCATION)),$(RUN_CONFIG_BUILD_SRC),$(RUN_CONFIG_BUILD_EXE))

#Name for the config definition files
CONFIG_DEFINITION=config_definition

#Name for the dir containing the module config definition files
MODULE_DEFINITION_DIR=$(SRC_DIR)/module_configurations

#First SQIF partition from the project (not necessarily the first
#in the .ptn).  This holds the application and zipped config.
FIRST_SQIF_PARTITION=$(word 1, $(SQIF_PARTITIONS))

#Temp variables used to determine dependent modules
COMMON_IAS:=
INPUT_MGR:=

#All core modules xml config files should be listed here
USED_MODULES=sink_a2dp_module_def.xml
USED_MODULES+=sink_hfp_data_def.xml
USED_MODULES+=sink_pio_module_def.xml
USED_MODULES+=sink_private_data_module_def.xml
USED_MODULES+=sink_button_module_def.xml
USED_MODULES+=sink_inquiry_module_def.xml
USED_MODULES+=sink_multi_channel_module_def.xml
USED_MODULES+=sink_device_id_module_def.xml
USED_MODULES+=sink_dut_module_def.xml
USED_MODULES+=sink_volume_module_def.xml
USED_MODULES+=sink_audio_routing_module_def.xml
USED_MODULES+=sink_audio_prompts_module_def.xml
USED_MODULES+=sink_tones_module_def.xml
USED_MODULES+=sink_led_module_def.xml
USED_MODULES+=sink_dsp_capabilities_def.xml

# Check for additional modules that are included in project
ifneq (,$(findstring ENABLE_BROADCAST_AUDIO,$(DEFS)))
    USED_MODULES+=sink_broadcast_audio_module_def.xml
endif
ifneq (,$(findstring ENABLE_AVRCP,$(DEFS)))
    USED_MODULES+=sink_avrcp_module_def.xml
endif
ifneq (,$(findstring GATT_HID_CLIENT,$(DEFS)))
    USED_MODULES+=sink_gatt_hid_remote_control_module_def.xml
    INPUT_MGR:=True
endif
ifneq (,$(findstring ENABLE_IR_REMOTE,$(DEFS)))
    USED_MODULES+=sink_ir_remote_control_module_def.xml
    INPUT_MGR:=True
endif
ifdef INPUT_MGR
    USED_MODULES+=sink_inputmanager_module_def.xml
endif
ifneq (,$(findstring GATT_ENABLED,$(DEFS)))
    USED_MODULES+=sink_ble_module_def.xml
    ifneq (,$(findstring CUSTOM_BLE_ADVERTISING_ENABLED,$(DEFS)))
        USED_MODULES+=sink_ble_advertising_module_def.xml
    endif
endif
ifneq (,$(findstring GATT_ANCS_CLIENT,$(DEFS)))
    USED_MODULES+=sink_gatt_client_ancs_def.xml
endif
ifneq (,$(findstring GATT_IAS_SERVER,$(DEFS)))
    USED_MODULES+=sink_gatt_server_ias_def.xml
    COMMON_IAS:=True
endif
ifneq (,$(findstring GATT_IAS_CLIENT,$(DEFS)))
    COMMON_IAS:=True
endif
ifdef COMMON_IAS
    USED_MODULES+= sink_gatt_common_ias_def.xml
endif
ifneq (,$(findstring GATT_LLS_SERVER,$(DEFS)))
    USED_MODULES+=sink_gatt_server_lls_def.xml
endif
ifneq (,$(findstring GATT_HRS_CLIENT,$(DEFS)))
    USED_MODULES+=sink_gatt_client_hrs_def.xml
endif
ifneq (,$(findstring GATT_DIS_SERVER,$(DEFS)))
    USED_MODULES+=sink_gatt_server_dis_def.xml
endif
ifneq (,$(findstring ENABLE_PBAP,$(DEFS)))
    USED_MODULES+=sink_pbap_module_def.xml
endif
ifneq (,$(findstring ENABLE_SPEECH_RECOGNITION,$(DEFS)))
    USED_MODULES+=sink_speech_rec_module_def.xml
endif
ifneq (,$(findstring ENABLE_GAIA,$(DEFS)))
    USED_MODULES+=sink_gaia_module_def.xml
endif
ifneq (,$(findstring ENABLE_PEER,$(DEFS)))
    USED_MODULES+=sink_peer_module_def.xml
endif
ifneq (,$(findstring ENABLE_FM,$(DEFS)))
    USED_MODULES+=sink_fm_module_def.xml
endif
ifneq (,$(findstring ENABLE_WIRED,$(DEFS)))
    USED_MODULES+=sink_wired_module_def.xml
endif
ifneq (,$(findstring ENABLE_PARTYMODE,$(DEFS)))
    USED_MODULES+=sink_partymode_def.xml
endif
ifneq (,$(findstring ENABLE_SUBWOOFER,$(DEFS)))
    USED_MODULES+=sink_swat_module_def.xml
endif
ifneq (,$(findstring ENABLE_USB,$(DEFS)))
    USED_MODULES+=sink_usb_module_def.xml
    ifneq (,$(findstring ENABLE_CHARGER_V2,$(DEFS)))
        ifneq (,$(findstring ENABLE_SMB1352,$(DEFS)))
            USED_MODULES+=sink_usb_smb1352_charger_def.xml
            USED_MODULES+=sink_usb_smb1352_proprietary_charger_def.xml
        else
            USED_MODULES+=sink_usb_onchip_charger_def.xml
            USED_MODULES+=sink_usb_onchip_proprietary_charger_def.xml
        endif
    else
        USED_MODULES+=sink_usb_charger_v1_def.xml
    endif
endif
ifneq (,$(findstring ENABLE_BATTERY_OPERATION,$(DEFS)))
    USED_MODULES+=sink_power_manager_module_def.xml
    ifneq (,$(findstring ENABLE_CHARGER_V2,$(DEFS)))
        USED_MODULES+=sink_powermanager_pmu_monitor_disabled_def.xml
        USED_MODULES+=sink_powermanager_onchip_vref_def.xml
        USED_MODULES+=sink_powermanager_onchip_vbat_def.xml
        ifneq (,$(findstring ENABLE_SMB1352,$(DEFS)))
            USED_MODULES+=sink_powermanager_bc12_disabled_def.xml
            USED_MODULES+=sink_powermanager_smb1352_vthm_def.xml
            USED_MODULES+=sink_powermanager_smb1352_vchg_def.xml
            USED_MODULES+=sink_powermanager_smb1352_charger_configuration_def.xml
        else
            USED_MODULES+=sink_powermanager_bc12_enabled_def.xml
            USED_MODULES+=sink_powermanager_onchip_vthm_def.xml
            USED_MODULES+=sink_powermanager_onchip_vchg_def.xml
            USED_MODULES+=sink_powermanager_onchip_charger_configuration_def.xml
        endif
    else
        USED_MODULES+=sink_powermanager_pmu_monitor_enabled_def.xml
        USED_MODULES+=sink_powermanager_bc12_disabled_def.xml
        USED_MODULES+=sink_powermanager_onchip_vref_v1_def.xml
        USED_MODULES+=sink_powermanager_onchip_vbat_v1_def.xml
        USED_MODULES+=sink_powermanager_onchip_vthm_v1_def.xml
        USED_MODULES+=sink_powermanager_onchip_vchg_v1_def.xml
        USED_MODULES+=sink_powermanager_onchip_charger_configuration_v1_def.xml
    endif
endif
ifneq (,$(findstring ENABLE_ANC,$(DEFS)))
    USED_MODULES+=sink_anc_def.xml
endif
ifneq (,$(findstring ENABLE_BREDR_SC,$(DEFS)))
    USED_MODULES+=sink_sc_module_def.xml
endif
ifneq (,$(findstring ENABLE_IAP2,$(DEFS)))
    USED_MODULES+=sink_accessory_module_def.xml
endif
ifneq (,$(findstring ENABLE_DORMANT_SUPPORT,$(DEFS)))
    USED_MODULES+=sink_low_power_mode_def.xml
endif
ifneq (,$(findstring ENABLE_UPGRADE,$(DEFS)))
    ifneq (,$(findstring ENABLE_UPGRADE_PARTITIONS_CONFIG,$(DEFS)))
        USED_MODULES+=sink_upgrade_with_partitions_module_def.xml
    else
        USED_MODULES+=sink_upgrade_module_def.xml
    endif
endif
ifneq (,$(findstring ENABLE_DISPLAY,$(DEFS)))
    USED_MODULES+=sink_display_module_def.xml
endif
ifneq (,$(findstring ENABLE_AUDIO_CLOCK,$(DEFS)))
    USED_MODULES+=sink_audio_clock_def.xml
endif
ifneq (,$(findstring ENABLE_AUDIO_TTP,$(DEFS)))
    USED_MODULES+=sink_audio_ttp_def.xml
endif
ifneq (,$(findstring ENABLE_DOWNLOADABLE_CAPS,$(DEFS)))
    USED_MODULES+=sink_downloadable_capabilities_def.xml
    USED_MODULES+=sink_configure_dsp_capability_ids_def.xml
endif
ifneq (,$(findstring ENABLE_STANDALONE_RATE_ADJUSTMENT,$(DEFS)))
    USED_MODULES+=sink_audio_ra_def.xml
endif
ifneq (,$(findstring SBC_ENCODER_CONFORMANCE,$(DEFS)))
    USED_MODULES+=sink_sbc_module_def.xml
endif
ifneq (,$(findstring ACTIVITY_MONITORING,$(DEFS)))
    USED_MODULES+=sink_activity_monitoring_def.xml
endif
ifneq (,$(findstring ENABLE_AMA,$(DEFS)))
    USED_MODULES+=sink_ama_module_def.xml
endif
ifneq (,$(findstring ENABLE_BISTO,$(DEFS)))
    USED_MODULES += sink_bisto_module_def.xml
endif
ifneq (,$(findstring ENABLE_FAST_PAIR,$(DEFS)))
    USED_MODULES+=sink_fast_pair_module_def.xml
endif

# Prepend the dir containing the module config defintions
MODULE_DEFINITIONS= $(USED_MODULES:%.xml=$(MODULE_DEFINITION_DIR)/%.xml)

# Get Python to print out a space-separated list of the Name attributes of all
# the root nodes of the module definitions we're using.  These are then easily
# translated into the names of the config headers that moduledef2configdef.py
# creates.
# Implementation note: this could be done with slightly less Python using a
# foreach loop in Make.  But that means multiple invocations of Python, which is
# noticeably slower.
CONFIG_HEADERS = $(patsubst %, $(SRC_DIR)/%_config_def.h,$(shell $(PYTHON) -c \
'from xml.etree import cElementTree as ET; import sys; print " ".join([ET.parse(m).getroot().attrib["HeaderName" if "HeaderName" in ET.parse(m).getroot().attrib.keys() else "Name"].lower() for m in sys.argv[1:]])' \
$(MODULE_DEFINITIONS)))

ifeq (,$(HW_VARIANT))
    $(error Hardware Variant must be set in project properties)
endif

ifeq (,$(SW_VARIANT))
    $(error Software Variant must be set in project properties)
endif

ifneq (,$(findstring HYDRACORE,$(DEFS)))
    PLATFORM=HYDRACORE
else
    PLATFORM=BLUECORE
endif

INPUTS+=$(SRC_DIR)/$(CONFIG_DEFINITION).c

# Make the config_build target phony so that is always run.
# The script itself checks if any of the generated files differ from the
# exisiting files and will not overwrite them if they are the same.
.PHONY : config_build

$(SRC_DIR)/$(CONFIG_DEFINITION).c $(SRC_DIR)/$(CONFIG_DEFINITION).h $(CONFIG_HEADERS) : config_build

# Note: The first 7 user PS keys are reserved for persistent data stored at runtime - see sink_configmanager.h
config_build : $(SRC_DIR)/global_config.xml $(MODULE_DEFINITIONS)
	$(CONFIG_BUILD_EXE_CMD) -s $(PLATFORM) -g $(SRC_DIR)/global_config.xml -ocd $(CONFIG_DEFINITION) -o $(SRC_DIR)/ $(MODULE_DEFINITIONS) -vvv -psk 8-49,150-199 -pm $(SRC_DIR)/pskey_map.xml -hw $(HW_VARIANT) -sw $(SW_VARIANT) -l $(SRC_DIR)/config_build.log
ifeq (HYDRACORE,$(PLATFORM))
	@$(copyfile) $(SRC_DIR)/$(CONFIG_DEFINITION).gz $(SRC_DIR)/$(CHIP_TYPE)/customer_ro_filesystem/$(CONFIG_DEFINITION).gz
else ifneq (,$(FIRST_SQIF_PARTITION))
	$(mkdir) $(SRC_DIR)/$(FIRST_SQIF_PARTITION)
	$(copyfile) $(SRC_DIR)/$(CONFIG_DEFINITION).gz $(SRC_DIR)/$(FIRST_SQIF_PARTITION)/$(CONFIG_DEFINITION).gz
endif

config_clean :
	$(del) $(SRC_DIR)/*config_def.h
	$(del) $(SRC_DIR)/$(CONFIG_DEFINITION).*
ifeq (HYDRACORE,$(PLATFORM))
	$(del) $(SRC_DIR)/$(CHIP_TYPE)/customer_ro_filesystem/$(CONFIG_DEFINITION).gz
else ifeq (BLUECORE,$(PLATFORM))
	$(del) $(SRC_DIR)/image/$(CONFIG_DEFINITION).gz
endif

ifeq (HYDRACORE,$(PLATFORM)) # HYDRACORE uses single colon rules for these targets.

build: config_build

clean: config_clean

else # BLUECORE uses double colon rules for these targets.

build:: config_build

clean:: config_clean

# Config definition file
image/$(CONFIG_DEFINITION).gz : $(CONFIG_DEFINITION).gz
	$(copyfile) $< $@

image.fs : image/$(CONFIG_DEFINITION).gz

endif
