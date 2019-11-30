# Copyright (c) 2018 Qualcomm Technologies International, Ltd.
#   
BUILDOUTPUT_PATH?=..
ifeq ($(DEVKIT_ROOT),)
	PYTHON=python
else
	PYTHON=$(DEVKIT_ROOT)/tools/python27/python.exe
endif
KCC_DIR?=$(DEVKIT_ROOT)/tools/kcc
CC=$(KCC_DIR)/bin/kcc
AR=$(KCC_DIR)/bin/kar
ifeq ($(OS),Windows_NT)
	RMDIR=rmdir /S /Q
	FIXPATH=$(subst /,\,$1)
	MKDIR=-if not exist $(call FIXPATH,$1) mkdir $(call FIXPATH,$1)
else
	RMDIR=rm -rf
	FIXPATH=$1
	MKDIR=mkdir -p $1
endif
MAKE_DIR=$(call MKDIR,${@D})
C_SOURCE=core/appcmd/appcmd.c core/buffer/buf_init_handle.c core/buffer/buf_raw_read_map_16bit.c core/buffer/buf_raw_read_map_16bit_be.c core/buffer/buf_raw_read_map_16bit_save_state.c core/buffer/buf_raw_read_map_8bit.c core/buffer/buf_raw_read_map_8bit_save_state.c core/buffer/buf_raw_read_unmap.c core/buffer/buf_raw_read_update_restore_state.c core/buffer/buf_raw_read_write_map_16bit.c core/buffer/buf_raw_read_write_map_16bit_be.c core/buffer/buf_raw_read_write_map_16bit_save_state.c core/buffer/buf_raw_read_write_map_8bit.c core/buffer/buf_raw_read_write_map_8bit_save_state.c core/buffer/buf_raw_read_write_unmap.c core/buffer/buf_raw_update_tail_free.c core/buffer/buf_raw_write_map_16bit.c core/buffer/buf_raw_write_map_16bit_be.c core/buffer/buf_raw_write_map_16bit_save_state.c core/buffer/buf_raw_write_map_8bit.c core/buffer/buf_raw_write_map_8bit_save_state.c core/buffer/buf_raw_write_only_map_16bit.c core/buffer/buf_raw_write_only_map_16bit_be.c core/buffer/buf_raw_write_only_map_8bit.c core/buffer/buf_raw_write_only_map_8bit_save_state.c core/buffer/buf_raw_write_only_unmap.c core/buffer/buf_raw_write_unmap.c core/buffer/buf_raw_write_update_restore_state.c core/buffer/buffer_msg.c core/buffer/buffer_msg_ptr_access.c core/cache/cache.c core/dorm/dorm_config.c core/dorm/dorm_get_kip_flags.c core/dorm/dorm_kalimba.c core/excep/excep.c core/excep/excep_test.c core/fault/fault.c core/fault/fault_appcmd.c core/fault/fault_comms.c core/fault/fault_db.c core/hal/hal_bitserial.c core/hal/hal_data_conv.c core/hal/hal_data_conv_access.c core/hal/hal_data_conv_cal.c core/hal/hal_delay_us.c core/hal/hal_led.c core/hydra_log/hydra_log_firm.c core/hydra_log/hydra_log_soft.c core/id/id.c core/int/configure_interrupt.c core/int/configure_sw_interrupt.c core/int/configure_sw_interrupt_raw.c core/int/generate_sw_interrupt.c core/int/init_int.c core/int/swint_demux.c core/int/unconfigure_sw_interrupt.c core/ipc/ipc_bluestack.c core/ipc/ipc_deep_sleep.c core/ipc/ipc_fault_panic.c core/ipc/ipc_init.c core/ipc/ipc_malloc.c core/ipc/ipc_memory_access.c core/ipc/ipc_mmu.c core/ipc/ipc_pio.c core/ipc/ipc_recv.c core/ipc/ipc_sched.c core/ipc/ipc_sd_mmc.c core/ipc/ipc_send.c core/ipc/ipc_stream.c core/ipc/ipc_test.c core/ipc/ipc_test_traps.c core/ipc/ipc_test_tunnel.c core/ipc/ipc_trap_api.c core/ipc/ipc_uart.c core/itime_kal/itime_kal.c core/kal_utils/kal_utils.c core/led/led.c core/led/led_appcmd.c core/led_cfg/led_cfg.c core/led_cfg/led_cfg_utils.c core/longtimer/get_deci_time.c core/longtimer/get_milli_time.c core/longtimer/get_second_time.c core/longtimer/longtimer.c core/memprot/memprot.c core/optim/mempack.c core/optim/udiv3216.c core/panic/panic.c core/panic/panic_comms.c core/panic/panic_on_assert.c core/pio/init_pio.c core/pio/pio_get_levels_mask.c core/pio/pio_set_directions_mask.c core/pio/pio_set_internal_owners_mask.c core/pio/pio_set_levels_mask.c core/piodebounce/piodebounce.c core/pioint/pioint_configure.c core/pioint/pioint_init.c core/pl_timers/pl_timers.c core/pmalloc/init_pmalloc.c core/pmalloc/pcopy.c core/pmalloc/pfree.c core/pmalloc/pfree_set_free_list_ptr.c core/pmalloc/pmalloc.c core/pmalloc/pmalloc_available.c core/pmalloc/pmalloc_config.c core/pmalloc/pmalloc_debug_check_block.c core/pmalloc/pmalloc_debug_validate_free_list.c core/pmalloc/pmalloc_debug_validate_pool_control.c core/pmalloc/pmalloc_set_monitor_limits.c core/pmalloc/prealloc.c core/pmalloc/prightsize.c core/pmalloc/psizeof.c core/pmalloc/xpcopy.c core/pmalloc/xpmalloc.c core/pmalloc/xpmalloc_buffer.c core/pmalloc/xprealloc.c core/pmalloc/xzpmalloc.c core/pmalloc/zpmalloc.c core/sched_oxygen/sched_oxygen.c core/sched_oxygen/sched_oxygen_cancel.c core/slt/slt_entry.c customer/core/init/init.c customer/core/trap_api/trap_api_audio.c customer/core/trap_api/trap_api_bdaddr.c customer/core/trap_api/trap_api_bitserial.c customer/core/trap_api/trap_api_bluestack.c customer/core/trap_api/trap_api_capacitive_sensor.c customer/core/trap_api/trap_api_charger.c customer/core/trap_api/trap_api_core.c customer/core/trap_api/trap_api_core_pio.c customer/core/trap_api/trap_api_core_util.c customer/core/trap_api/trap_api_csb.c customer/core/trap_api/trap_api_extra.c customer/core/trap_api/trap_api_file.c customer/core/trap_api/trap_api_led.c customer/core/trap_api/trap_api_message.c customer/core/trap_api/trap_api_message_log.c customer/core/trap_api/trap_api_operator.c customer/core/trap_api/trap_api_psu.c customer/core/trap_api/trap_api_sd_mmc.c customer/core/trap_api/trap_api_stream.c customer/core/trap_api/trap_api_test_support.c customer/core/trap_api/trap_api_uart.c gen/core/trap_version/trap_version_supported.c gen/customer/core/trap_api/gen/trap_api_ipc_glue.c
H_SOURCE=../../common/interface/app/adc/adc_if.h ../../common/interface/app/audio/audio_if.h ../../common/interface/app/bitserial/bitserial_if.h ../../common/interface/app/bluestack/att_prim.h ../../common/interface/app/bluestack/bluetooth.h ../../common/interface/app/bluestack/dm_prim.h ../../common/interface/app/bluestack/hci.h ../../common/interface/app/bluestack/l2cap_prim.h ../../common/interface/app/bluestack/rfcomm_prim.h ../../common/interface/app/bluestack/sdc_prim.h ../../common/interface/app/bluestack/sds_prim.h ../../common/interface/app/bluestack/types.h ../../common/interface/app/capacitive_sensor/capacitive_sensor_if.h ../../common/interface/app/charger/charger_if.h ../../common/interface/app/dormant/dormant_if.h ../../common/interface/app/feature/feature_if.h ../../common/interface/app/file/file_if.h ../../common/interface/app/image_upgrade/image_upgrade_if.h ../../common/interface/app/infrared/infrared_if.h ../../common/interface/app/lcd/lcd_if.h ../../common/interface/app/led/led_if.h ../../common/interface/app/message/subsystem_if.h ../../common/interface/app/message/system_message.h ../../common/interface/app/mic_bias/mic_bias_if.h ../../common/interface/app/operator/operator_if.h ../../common/interface/app/partition/partition_if.h ../../common/interface/app/pio/pio_if.h ../../common/interface/app/ps/ps_if.h ../../common/interface/app/psu/psu_if.h ../../common/interface/app/ringtone/ringtone_if.h ../../common/interface/app/ringtone/ringtone_notes.h ../../common/interface/app/sd_mmc/sd_mmc_if.h ../../common/interface/app/status/status_if.h ../../common/interface/app/stream/stream_if.h ../../common/interface/app/uart/uart_if.h ../../common/interface/app/usb/usb_hub_if.h ../../common/interface/app/usb/usb_if.h ../../common/interface/app/vm/vm_if.h ../../common/interface/app/voltsense/voltsense_if.h ../../common/interface/gen/k32/appcmd_prim.h ../../common/interface/gen/k32/test_tunnel_prim.h ../../common/interface/slt/apps_fingerprint.h ../../common/interface/slt/apps_slt_ids.h bt/bluestack_if/bluestack_if.h bt/bt/bluestack_types.h bt/bt/bt_faultids.h bt/bt/bt_panicids.h core/appcmd/appcmd.h core/appcmd/appcmd_private.h core/appcmd/appcmd_sched.h core/bigint/bigint.h core/bigint/bigint_imp.h core/buffer/buffer.h core/buffer/buffer_msg.h core/buffer/buffer_private.h core/cache/cache.h core/dorm/dorm.h core/dorm/dorm_private.h core/excep/excep.h core/excep/excep_private.h core/fault/fault.h core/fault/fault_appcmd.h core/fault/fault_itime.h core/fault/fault_private.h core/fault/fault_sched.h core/hal/aura/d01/hal/hal_macros.h core/hal/hal.h core/hal/hal_bitserial.h core/hal/hal_cross_cpu_registers.h core/hal/hal_data_conv.h core/hal/hal_data_conv_access.h core/hal/hal_led.h core/hal/hal_macros.h core/hal/hal_registers.h core/hal/hal_transaction_types.h core/hal/halauxio.h core/hal/halint.h core/hal/haltime.h core/hydra/hydra.h core/hydra/hydra_faultids.h core/hydra/hydra_macros.h core/hydra/hydra_panicids.h core/hydra/hydra_patch.h core/hydra/hydra_trb.h core/hydra/hydra_types.h core/hydra_log/hydra_log.h core/hydra_log/hydra_log_disabled.h core/hydra_log/hydra_log_firm.h core/hydra_log/hydra_log_firm_modules.h core/hydra_log/hydra_log_soft.h core/id/id.h core/id/id_slt_entry.h core/include/bits.h core/include/chip.h core/include/dwarf_constants.h core/include/faultids.h core/include/hal_utils.h core/include/kaldwarfregnums.h core/include/macros.h core/include/memory_map.h core/include/panicids.h core/include/patch.h core/include/types.h core/include_fw/assert.h core/include_fw/hal_macros_divert.h core/int/int.h core/int/int_private.h core/int/swint.h core/int/swint_private.h core/io/aura/d01/io/io_defs.h core/io/aura/d01/io/io_map.h core/io/io.h core/io/io_defs.h core/io/io_map.h core/io/io_slt_entry.h core/ipc/ipc.h core/ipc/ipc_msg_types.h core/ipc/ipc_prim.h core/ipc/ipc_private.h core/ipc/ipc_sched.h core/itime/itime.h core/itime_kal/itime_kal.h core/itime_kal/itime_kal_private.h core/kal_utils/kal_utils.h core/led/led.h core/led/led_appcmd.h core/led/led_private.h core/led/led_sched.h core/led_cfg/led_cfg.h core/led_cfg/led_cfg_private.h core/longtimer/longtimer.h core/longtimer/longtimer_private.h core/memprot/memprot.h core/mmu/memmap.h core/mmu/mmu.h core/mmu/mmu_proc_port.h core/optim/optim.h core/optim/optim_private.h core/panic/panic.h core/panic/panic_private.h core/pio/pio.h core/pio/pio_private.h core/pio_cfg/pio_cfg.h core/piodebounce/piodebounce.h core/piodebounce/piodebounce_private.h core/piodebounce/piodebounce_sched.h core/pioint/pioint.h core/pioint/pioint_private.h core/pl_timers/pl_timers.h core/pl_timers/pl_timers_private.h core/pmalloc/pmalloc.h core/pmalloc/pmalloc_config_P1.h core/pmalloc/pmalloc_debug.h core/pmalloc/pmalloc_private.h core/pmalloc/pmalloc_trace.h core/sched/runlevels.h core/sched/sched.h core/sched_oxygen/sched_oxygen.h core/sched_oxygen/sched_oxygen_priority.h core/sched_oxygen/sched_oxygen_private.h core/slt/slt.h core/slt/slt_private.h core/timed_event/rtime.h core/timed_event/rtime_types.h core/timed_event/timed_event.h core/timed_event_oxygen/timed_event_oxygen.h core/trap_version/trap_version.h core/trap_version/trap_version_slt_entry.h core/utils/utils.h core/utils/utils_bit.h core/utils/utils_bitarray.h core/utils/utils_bits_and_bobs.h core/utils/utils_event.h core/utils/utils_fault_panic.h core/utils/utils_fsm.h core/utils/utils_geometry.h core/utils/utils_jobq.h core/utils/utils_patch.h core/utils/utils_set.h core/utils/utils_sll.h core/utils/utils_strdup.h customer/core/init/init.h customer/core/init/init_private.h customer/core/portability/portability.h customer/core/trap_api/csrtypes.h customer/core/trap_api/panicdefs.h customer/core/trap_api/trap_api.h customer/core/trap_api/trap_api_private.h customer/core/trap_api/trap_api_sched.h gen/build_defs.h gen/core/hydra_log/hydra_log_subsystems.h gen/core/ipc/gen/ipc_trap_api_prims.h gen/core/ipc/gen/ipc_trap_api_signals.h gen/core/itime_kal/itime_subsystems.h gen/core/sched_oxygen/bg_int_subsystem.h gen/core/sched_oxygen/sched_subsystem.h gen/core/slt/slt_data_subsystems.h gen/core/slt/slt_entry_subsystems.h gen/customer/core/trap_api/adc.h gen/customer/core/trap_api/api.h gen/customer/core/trap_api/audio_anc.h gen/customer/core/trap_api/audio_clock.h gen/customer/core/trap_api/audio_mclk.h gen/customer/core/trap_api/audio_power.h gen/customer/core/trap_api/audio_pwm.h gen/customer/core/trap_api/bdaddr_.h gen/customer/core/trap_api/bitserial_api.h gen/customer/core/trap_api/boot.h gen/customer/core/trap_api/capacitivesensor.h gen/customer/core/trap_api/charger.h gen/customer/core/trap_api/codec_.h gen/customer/core/trap_api/crypto.h gen/customer/core/trap_api/csb.h gen/customer/core/trap_api/csb_.h gen/customer/core/trap_api/dormant.h gen/customer/core/trap_api/energy.h gen/customer/core/trap_api/feature.h gen/customer/core/trap_api/file.h gen/customer/core/trap_api/font.h gen/customer/core/trap_api/host.h gen/customer/core/trap_api/i2c.h gen/customer/core/trap_api/imageupgrade.h gen/customer/core/trap_api/infrared.h gen/customer/core/trap_api/inquiry.h gen/customer/core/trap_api/kalimba.h gen/customer/core/trap_api/lcd.h gen/customer/core/trap_api/led.h gen/customer/core/trap_api/loader.h gen/customer/core/trap_api/message.h gen/customer/core/trap_api/message_.h gen/customer/core/trap_api/micbias.h gen/customer/core/trap_api/native.h gen/customer/core/trap_api/nfc.h gen/customer/core/trap_api/operator.h gen/customer/core/trap_api/operator_.h gen/customer/core/trap_api/os.h gen/customer/core/trap_api/otp.h gen/customer/core/trap_api/panic.h gen/customer/core/trap_api/partition.h gen/customer/core/trap_api/pio.h gen/customer/core/trap_api/ps.h gen/customer/core/trap_api/psu.h gen/customer/core/trap_api/sdmmc.h gen/customer/core/trap_api/sink.h gen/customer/core/trap_api/sink_.h gen/customer/core/trap_api/source.h gen/customer/core/trap_api/source_.h gen/customer/core/trap_api/sram.h gen/customer/core/trap_api/status.h gen/customer/core/trap_api/stream.h gen/customer/core/trap_api/test.h gen/customer/core/trap_api/transform.h gen/customer/core/trap_api/transform_.h gen/customer/core/trap_api/usb.h gen/customer/core/trap_api/usb_hub.h gen/customer/core/trap_api/util.h gen/customer/core/trap_api/vm.h gen/customer/core/trap_api/vm_.h gen/customer/core/trap_api/voltsense.h nfc/nfc/nfc_faultids.h nfc/nfc/nfc_panicids.h
ASM_SOURCE=core/appcmd/appcmd_call_function.asm core/crt/crt0.asm core/crt/crt0_rst_maxim.asm core/int/interrupt.asm core/int/interrupt_inc.asm core/io/aura/d01/io/io_defs.asm core/io/aura/d01/io/io_map.asm core/io/io_defs.asm core/kal_utils/kal_utils_asm.asm core/optim/uint64_divmod31_opt.asm core/pmalloc/pmalloc_trace_pc.asm core/slt/slt_header.asm
CHIP_TYPE=qcc512x_qcc302x
INSTALL_DIR=$(BUILDOUTPUT_PATH)/../installed_libs
LIB_DIR=$(INSTALL_DIR)/lib/os/$(CHIP_TYPE)
ADK_INTERFACE=$(INSTALL_DIR)/include
ADK_INTERFACE_FW=$(ADK_INTERFACE)/firmware_$(CHIP_TYPE)
ADK_INTERFACE_STD=$(ADK_INTERFACE)/standard
TRAPSETS_HEADER=$(ADK_INTERFACE_FW)/trapsets.h
MAKE_TOOLS_DIR=../tools/make
BUILD_DIR=$(BUILDOUTPUT_PATH)/build/aura_p1_d01_lib
INCPATHS=core/include_fw ../../common/interface ../../common/interface/app customer/core customer/apps customer/libs gen/customer/core customer/core customer/apps customer/libs gen/customer/core core core/include gen/core bt customer/core/trap_api gen/customer/core/trap_api customer/core gen/customer/core customer/core/trap_api gen/customer/core/trap_api customer/core gen/customer/core .
INCLUDE_PATHS=$(foreach inc,$(INCPATHS),-I$(inc))
ASM_INCLUDE_PATHS=$(foreach inc,$(INCPATHS),-A$(inc))
PREINCPATHS=gen/build_defs.h
PREINCLUDES=$(foreach pre,$(PREINCPATHS),-preinclude $(pre))
ASM_PREINCLUDES=$(foreach pre,$(PREINCPATHS),-Xa -include -Xa $(pre))
C_OBJECTS=$(patsubst %.c, $(BUILD_DIR)/%.o,$(C_SOURCE))
ASM_OBJECTS=$(patsubst %.asm, $(BUILD_DIR)/%.o,$(ASM_SOURCE))
CHIP_NAME_OPT=-kqcc512x_app
WARNING_OPTS=-Wall -WAccuracyLoss -WnoPointerUnaligned -WnoConstantTest
CFLAGS=-g -minim $(CHIP_NAME_OPT) -Xc -o2 -Xa -apply-b179745-workaround $(WARNING_OPTS)
CFLAGS+=$(PREINCLUDES) $(INCLUDE_PATHS)
CFLAGS+=$(ASM_PREINCLUDES) $(ASM_INCLUDE_PATHS)
all: $(ADK_INTERFACE) $(LIB_DIR)/lib$(CHIP_TYPE).a $(LIB_DIR)/libusb_early_init.a
$(ADK_INTERFACE): $(TRAPSETS_HEADER) $(ADK_INTERFACE_FW)/app/adc/adc_if.h $(ADK_INTERFACE_FW)/app/audio/audio_if.h $(ADK_INTERFACE_FW)/app/bitserial/bitserial_if.h $(ADK_INTERFACE_FW)/app/bluestack/att_prim.h $(ADK_INTERFACE_FW)/app/bluestack/bluetooth.h $(ADK_INTERFACE_FW)/app/bluestack/dm_prim.h $(ADK_INTERFACE_FW)/app/bluestack/hci.h $(ADK_INTERFACE_FW)/app/bluestack/l2cap_prim.h $(ADK_INTERFACE_FW)/app/bluestack/rfcomm_prim.h $(ADK_INTERFACE_FW)/app/bluestack/sdc_prim.h $(ADK_INTERFACE_FW)/app/bluestack/sds_prim.h $(ADK_INTERFACE_FW)/app/bluestack/types.h $(ADK_INTERFACE_FW)/app/capacitive_sensor/capacitive_sensor_if.h $(ADK_INTERFACE_FW)/app/charger/charger_if.h $(ADK_INTERFACE_FW)/app/dormant/dormant_if.h $(ADK_INTERFACE_FW)/app/feature/feature_if.h $(ADK_INTERFACE_FW)/app/file/file_if.h $(ADK_INTERFACE_FW)/app/image_upgrade/image_upgrade_if.h $(ADK_INTERFACE_FW)/app/infrared/infrared_if.h $(ADK_INTERFACE_FW)/app/lcd/lcd_if.h $(ADK_INTERFACE_FW)/app/led/led_if.h $(ADK_INTERFACE_FW)/app/message/subsystem_if.h $(ADK_INTERFACE_FW)/app/message/system_message.h $(ADK_INTERFACE_FW)/app/mic_bias/mic_bias_if.h $(ADK_INTERFACE_FW)/app/operator/operator_if.h $(ADK_INTERFACE_FW)/app/partition/partition_if.h $(ADK_INTERFACE_FW)/app/pio/pio_if.h $(ADK_INTERFACE_FW)/app/ps/ps_if.h $(ADK_INTERFACE_FW)/app/psu/psu_if.h $(ADK_INTERFACE_FW)/app/ringtone/ringtone_if.h $(ADK_INTERFACE_FW)/app/ringtone/ringtone_notes.h $(ADK_INTERFACE_FW)/app/sd_mmc/sd_mmc_if.h $(ADK_INTERFACE_FW)/app/status/status_if.h $(ADK_INTERFACE_FW)/app/stream/stream_if.h $(ADK_INTERFACE_FW)/app/uart/uart_if.h $(ADK_INTERFACE_FW)/app/usb/usb_hub_if.h $(ADK_INTERFACE_FW)/app/usb/usb_if.h $(ADK_INTERFACE_FW)/app/vm/vm_if.h $(ADK_INTERFACE_FW)/app/voltsense/voltsense_if.h $(ADK_INTERFACE_FW)/hydra_macros.h $(ADK_INTERFACE_STD)/hydra_types.h $(ADK_INTERFACE_FW)/hydra_log.h $(ADK_INTERFACE_FW)/hydra_log_disabled.h $(ADK_INTERFACE_FW)/hydra_log_firm.h $(ADK_INTERFACE_FW)/hydra_log_firm_modules.h $(ADK_INTERFACE_FW)/hydra_log_soft.h $(ADK_INTERFACE_STD)/macros.h $(ADK_INTERFACE_STD)/types.h $(ADK_INTERFACE_FW)/pmalloc.h $(ADK_INTERFACE_FW)/pmalloc_trace.h $(ADK_INTERFACE_STD)/csrtypes.h $(ADK_INTERFACE_FW)/hydra_log_subsystems.h $(ADK_INTERFACE_FW)/adc.h $(ADK_INTERFACE_FW)/api.h $(ADK_INTERFACE_FW)/audio_anc.h $(ADK_INTERFACE_FW)/audio_clock.h $(ADK_INTERFACE_FW)/audio_mclk.h $(ADK_INTERFACE_FW)/audio_power.h $(ADK_INTERFACE_FW)/audio_pwm.h $(ADK_INTERFACE_FW)/bdaddr_.h $(ADK_INTERFACE_FW)/bitserial_api.h $(ADK_INTERFACE_FW)/boot.h $(ADK_INTERFACE_FW)/capacitivesensor.h $(ADK_INTERFACE_FW)/charger.h $(ADK_INTERFACE_FW)/codec_.h $(ADK_INTERFACE_FW)/crypto.h $(ADK_INTERFACE_FW)/csb.h $(ADK_INTERFACE_FW)/csb_.h $(ADK_INTERFACE_FW)/dormant.h $(ADK_INTERFACE_FW)/energy.h $(ADK_INTERFACE_FW)/feature.h $(ADK_INTERFACE_FW)/file.h $(ADK_INTERFACE_FW)/font.h $(ADK_INTERFACE_FW)/host.h $(ADK_INTERFACE_FW)/i2c.h $(ADK_INTERFACE_FW)/imageupgrade.h $(ADK_INTERFACE_FW)/infrared.h $(ADK_INTERFACE_FW)/inquiry.h $(ADK_INTERFACE_FW)/kalimba.h $(ADK_INTERFACE_FW)/lcd.h $(ADK_INTERFACE_FW)/led.h $(ADK_INTERFACE_FW)/loader.h $(ADK_INTERFACE_FW)/message.h $(ADK_INTERFACE_FW)/message_.h $(ADK_INTERFACE_FW)/micbias.h $(ADK_INTERFACE_FW)/native.h $(ADK_INTERFACE_FW)/nfc.h $(ADK_INTERFACE_FW)/operator.h $(ADK_INTERFACE_FW)/operator_.h $(ADK_INTERFACE_FW)/os.h $(ADK_INTERFACE_FW)/otp.h $(ADK_INTERFACE_FW)/panic.h $(ADK_INTERFACE_FW)/partition.h $(ADK_INTERFACE_FW)/pio.h $(ADK_INTERFACE_FW)/ps.h $(ADK_INTERFACE_FW)/psu.h $(ADK_INTERFACE_FW)/sdmmc.h $(ADK_INTERFACE_FW)/sink.h $(ADK_INTERFACE_FW)/sink_.h $(ADK_INTERFACE_FW)/source.h $(ADK_INTERFACE_FW)/source_.h $(ADK_INTERFACE_FW)/sram.h $(ADK_INTERFACE_FW)/status.h $(ADK_INTERFACE_FW)/stream.h $(ADK_INTERFACE_FW)/test.h $(ADK_INTERFACE_FW)/transform.h $(ADK_INTERFACE_FW)/transform_.h $(ADK_INTERFACE_FW)/usb.h $(ADK_INTERFACE_FW)/usb_hub.h $(ADK_INTERFACE_FW)/util.h $(ADK_INTERFACE_FW)/vm.h $(ADK_INTERFACE_FW)/vm_.h $(ADK_INTERFACE_FW)/voltsense.h
$(TRAPSETS_HEADER): gen/build_defs.h
	$(PYTHON) $(MAKE_TOOLS_DIR)/create_trapsets_header.py $< $@
$(ADK_INTERFACE_FW)/app/adc/adc_if.h: ../../common/interface/app/adc/adc_if.h
$(ADK_INTERFACE_FW)/app/audio/audio_if.h: ../../common/interface/app/audio/audio_if.h
$(ADK_INTERFACE_FW)/app/bitserial/bitserial_if.h: ../../common/interface/app/bitserial/bitserial_if.h
$(ADK_INTERFACE_FW)/app/bluestack/att_prim.h: ../../common/interface/app/bluestack/att_prim.h
$(ADK_INTERFACE_FW)/app/bluestack/bluetooth.h: ../../common/interface/app/bluestack/bluetooth.h
$(ADK_INTERFACE_FW)/app/bluestack/dm_prim.h: ../../common/interface/app/bluestack/dm_prim.h
$(ADK_INTERFACE_FW)/app/bluestack/hci.h: ../../common/interface/app/bluestack/hci.h
$(ADK_INTERFACE_FW)/app/bluestack/l2cap_prim.h: ../../common/interface/app/bluestack/l2cap_prim.h
$(ADK_INTERFACE_FW)/app/bluestack/rfcomm_prim.h: ../../common/interface/app/bluestack/rfcomm_prim.h
$(ADK_INTERFACE_FW)/app/bluestack/sdc_prim.h: ../../common/interface/app/bluestack/sdc_prim.h
$(ADK_INTERFACE_FW)/app/bluestack/sds_prim.h: ../../common/interface/app/bluestack/sds_prim.h
$(ADK_INTERFACE_FW)/app/bluestack/types.h: ../../common/interface/app/bluestack/types.h
$(ADK_INTERFACE_FW)/app/capacitive_sensor/capacitive_sensor_if.h: ../../common/interface/app/capacitive_sensor/capacitive_sensor_if.h
$(ADK_INTERFACE_FW)/app/charger/charger_if.h: ../../common/interface/app/charger/charger_if.h
$(ADK_INTERFACE_FW)/app/dormant/dormant_if.h: ../../common/interface/app/dormant/dormant_if.h
$(ADK_INTERFACE_FW)/app/feature/feature_if.h: ../../common/interface/app/feature/feature_if.h
$(ADK_INTERFACE_FW)/app/file/file_if.h: ../../common/interface/app/file/file_if.h
$(ADK_INTERFACE_FW)/app/image_upgrade/image_upgrade_if.h: ../../common/interface/app/image_upgrade/image_upgrade_if.h
$(ADK_INTERFACE_FW)/app/infrared/infrared_if.h: ../../common/interface/app/infrared/infrared_if.h
$(ADK_INTERFACE_FW)/app/lcd/lcd_if.h: ../../common/interface/app/lcd/lcd_if.h
$(ADK_INTERFACE_FW)/app/led/led_if.h: ../../common/interface/app/led/led_if.h
$(ADK_INTERFACE_FW)/app/message/subsystem_if.h: ../../common/interface/app/message/subsystem_if.h
$(ADK_INTERFACE_FW)/app/message/system_message.h: ../../common/interface/app/message/system_message.h
$(ADK_INTERFACE_FW)/app/mic_bias/mic_bias_if.h: ../../common/interface/app/mic_bias/mic_bias_if.h
$(ADK_INTERFACE_FW)/app/operator/operator_if.h: ../../common/interface/app/operator/operator_if.h
$(ADK_INTERFACE_FW)/app/partition/partition_if.h: ../../common/interface/app/partition/partition_if.h
$(ADK_INTERFACE_FW)/app/pio/pio_if.h: ../../common/interface/app/pio/pio_if.h
$(ADK_INTERFACE_FW)/app/ps/ps_if.h: ../../common/interface/app/ps/ps_if.h
$(ADK_INTERFACE_FW)/app/psu/psu_if.h: ../../common/interface/app/psu/psu_if.h
$(ADK_INTERFACE_FW)/app/ringtone/ringtone_if.h: ../../common/interface/app/ringtone/ringtone_if.h
$(ADK_INTERFACE_FW)/app/ringtone/ringtone_notes.h: ../../common/interface/app/ringtone/ringtone_notes.h
$(ADK_INTERFACE_FW)/app/sd_mmc/sd_mmc_if.h: ../../common/interface/app/sd_mmc/sd_mmc_if.h
$(ADK_INTERFACE_FW)/app/status/status_if.h: ../../common/interface/app/status/status_if.h
$(ADK_INTERFACE_FW)/app/stream/stream_if.h: ../../common/interface/app/stream/stream_if.h
$(ADK_INTERFACE_FW)/app/uart/uart_if.h: ../../common/interface/app/uart/uart_if.h
$(ADK_INTERFACE_FW)/app/usb/usb_hub_if.h: ../../common/interface/app/usb/usb_hub_if.h
$(ADK_INTERFACE_FW)/app/usb/usb_if.h: ../../common/interface/app/usb/usb_if.h
$(ADK_INTERFACE_FW)/app/vm/vm_if.h: ../../common/interface/app/vm/vm_if.h
$(ADK_INTERFACE_FW)/app/voltsense/voltsense_if.h: ../../common/interface/app/voltsense/voltsense_if.h
$(ADK_INTERFACE_FW)/hydra_macros.h: core/hydra/hydra_macros.h
$(ADK_INTERFACE_STD)/hydra_types.h: core/hydra/hydra_types.h
$(ADK_INTERFACE_FW)/hydra_log.h: core/hydra_log/hydra_log.h
$(ADK_INTERFACE_FW)/hydra_log_disabled.h: core/hydra_log/hydra_log_disabled.h
$(ADK_INTERFACE_FW)/hydra_log_firm.h: core/hydra_log/hydra_log_firm.h
$(ADK_INTERFACE_FW)/hydra_log_firm_modules.h: core/hydra_log/hydra_log_firm_modules.h
$(ADK_INTERFACE_FW)/hydra_log_soft.h: core/hydra_log/hydra_log_soft.h
$(ADK_INTERFACE_STD)/macros.h: core/include/macros.h
$(ADK_INTERFACE_STD)/types.h: core/include/types.h
$(ADK_INTERFACE_FW)/pmalloc.h: core/pmalloc/pmalloc.h
$(ADK_INTERFACE_FW)/pmalloc_trace.h: core/pmalloc/pmalloc_trace.h
$(ADK_INTERFACE_STD)/csrtypes.h: customer/core/trap_api/csrtypes.h
$(ADK_INTERFACE_FW)/hydra_log_subsystems.h: gen/core/hydra_log/hydra_log_subsystems.h
$(ADK_INTERFACE_FW)/adc.h: gen/customer/core/trap_api/adc.h
$(ADK_INTERFACE_FW)/api.h: gen/customer/core/trap_api/api.h
$(ADK_INTERFACE_FW)/audio_anc.h: gen/customer/core/trap_api/audio_anc.h
$(ADK_INTERFACE_FW)/audio_clock.h: gen/customer/core/trap_api/audio_clock.h
$(ADK_INTERFACE_FW)/audio_mclk.h: gen/customer/core/trap_api/audio_mclk.h
$(ADK_INTERFACE_FW)/audio_power.h: gen/customer/core/trap_api/audio_power.h
$(ADK_INTERFACE_FW)/audio_pwm.h: gen/customer/core/trap_api/audio_pwm.h
$(ADK_INTERFACE_FW)/bdaddr_.h: gen/customer/core/trap_api/bdaddr_.h
$(ADK_INTERFACE_FW)/bitserial_api.h: gen/customer/core/trap_api/bitserial_api.h
$(ADK_INTERFACE_FW)/boot.h: gen/customer/core/trap_api/boot.h
$(ADK_INTERFACE_FW)/capacitivesensor.h: gen/customer/core/trap_api/capacitivesensor.h
$(ADK_INTERFACE_FW)/charger.h: gen/customer/core/trap_api/charger.h
$(ADK_INTERFACE_FW)/codec_.h: gen/customer/core/trap_api/codec_.h
$(ADK_INTERFACE_FW)/crypto.h: gen/customer/core/trap_api/crypto.h
$(ADK_INTERFACE_FW)/csb.h: gen/customer/core/trap_api/csb.h
$(ADK_INTERFACE_FW)/csb_.h: gen/customer/core/trap_api/csb_.h
$(ADK_INTERFACE_FW)/dormant.h: gen/customer/core/trap_api/dormant.h
$(ADK_INTERFACE_FW)/energy.h: gen/customer/core/trap_api/energy.h
$(ADK_INTERFACE_FW)/feature.h: gen/customer/core/trap_api/feature.h
$(ADK_INTERFACE_FW)/file.h: gen/customer/core/trap_api/file.h
$(ADK_INTERFACE_FW)/font.h: gen/customer/core/trap_api/font.h
$(ADK_INTERFACE_FW)/host.h: gen/customer/core/trap_api/host.h
$(ADK_INTERFACE_FW)/i2c.h: gen/customer/core/trap_api/i2c.h
$(ADK_INTERFACE_FW)/imageupgrade.h: gen/customer/core/trap_api/imageupgrade.h
$(ADK_INTERFACE_FW)/infrared.h: gen/customer/core/trap_api/infrared.h
$(ADK_INTERFACE_FW)/inquiry.h: gen/customer/core/trap_api/inquiry.h
$(ADK_INTERFACE_FW)/kalimba.h: gen/customer/core/trap_api/kalimba.h
$(ADK_INTERFACE_FW)/lcd.h: gen/customer/core/trap_api/lcd.h
$(ADK_INTERFACE_FW)/led.h: gen/customer/core/trap_api/led.h
$(ADK_INTERFACE_FW)/loader.h: gen/customer/core/trap_api/loader.h
$(ADK_INTERFACE_FW)/message.h: gen/customer/core/trap_api/message.h
$(ADK_INTERFACE_FW)/message_.h: gen/customer/core/trap_api/message_.h
$(ADK_INTERFACE_FW)/micbias.h: gen/customer/core/trap_api/micbias.h
$(ADK_INTERFACE_FW)/native.h: gen/customer/core/trap_api/native.h
$(ADK_INTERFACE_FW)/nfc.h: gen/customer/core/trap_api/nfc.h
$(ADK_INTERFACE_FW)/operator.h: gen/customer/core/trap_api/operator.h
$(ADK_INTERFACE_FW)/operator_.h: gen/customer/core/trap_api/operator_.h
$(ADK_INTERFACE_FW)/os.h: gen/customer/core/trap_api/os.h
$(ADK_INTERFACE_FW)/otp.h: gen/customer/core/trap_api/otp.h
$(ADK_INTERFACE_FW)/panic.h: gen/customer/core/trap_api/panic.h
$(ADK_INTERFACE_FW)/partition.h: gen/customer/core/trap_api/partition.h
$(ADK_INTERFACE_FW)/pio.h: gen/customer/core/trap_api/pio.h
$(ADK_INTERFACE_FW)/ps.h: gen/customer/core/trap_api/ps.h
$(ADK_INTERFACE_FW)/psu.h: gen/customer/core/trap_api/psu.h
$(ADK_INTERFACE_FW)/sdmmc.h: gen/customer/core/trap_api/sdmmc.h
$(ADK_INTERFACE_FW)/sink.h: gen/customer/core/trap_api/sink.h
$(ADK_INTERFACE_FW)/sink_.h: gen/customer/core/trap_api/sink_.h
$(ADK_INTERFACE_FW)/source.h: gen/customer/core/trap_api/source.h
$(ADK_INTERFACE_FW)/source_.h: gen/customer/core/trap_api/source_.h
$(ADK_INTERFACE_FW)/sram.h: gen/customer/core/trap_api/sram.h
$(ADK_INTERFACE_FW)/status.h: gen/customer/core/trap_api/status.h
$(ADK_INTERFACE_FW)/stream.h: gen/customer/core/trap_api/stream.h
$(ADK_INTERFACE_FW)/test.h: gen/customer/core/trap_api/test.h
$(ADK_INTERFACE_FW)/transform.h: gen/customer/core/trap_api/transform.h
$(ADK_INTERFACE_FW)/transform_.h: gen/customer/core/trap_api/transform_.h
$(ADK_INTERFACE_FW)/usb.h: gen/customer/core/trap_api/usb.h
$(ADK_INTERFACE_FW)/usb_hub.h: gen/customer/core/trap_api/usb_hub.h
$(ADK_INTERFACE_FW)/util.h: gen/customer/core/trap_api/util.h
$(ADK_INTERFACE_FW)/vm.h: gen/customer/core/trap_api/vm.h
$(ADK_INTERFACE_FW)/vm_.h: gen/customer/core/trap_api/vm_.h
$(ADK_INTERFACE_FW)/voltsense.h: gen/customer/core/trap_api/voltsense.h
$(ADK_INTERFACE_FW)/app/adc/adc_if.h $(ADK_INTERFACE_FW)/app/audio/audio_if.h $(ADK_INTERFACE_FW)/app/bitserial/bitserial_if.h $(ADK_INTERFACE_FW)/app/bluestack/att_prim.h $(ADK_INTERFACE_FW)/app/bluestack/bluetooth.h $(ADK_INTERFACE_FW)/app/bluestack/dm_prim.h $(ADK_INTERFACE_FW)/app/bluestack/hci.h $(ADK_INTERFACE_FW)/app/bluestack/l2cap_prim.h $(ADK_INTERFACE_FW)/app/bluestack/rfcomm_prim.h $(ADK_INTERFACE_FW)/app/bluestack/sdc_prim.h $(ADK_INTERFACE_FW)/app/bluestack/sds_prim.h $(ADK_INTERFACE_FW)/app/bluestack/types.h $(ADK_INTERFACE_FW)/app/capacitive_sensor/capacitive_sensor_if.h $(ADK_INTERFACE_FW)/app/charger/charger_if.h $(ADK_INTERFACE_FW)/app/dormant/dormant_if.h $(ADK_INTERFACE_FW)/app/feature/feature_if.h $(ADK_INTERFACE_FW)/app/file/file_if.h $(ADK_INTERFACE_FW)/app/image_upgrade/image_upgrade_if.h $(ADK_INTERFACE_FW)/app/infrared/infrared_if.h $(ADK_INTERFACE_FW)/app/lcd/lcd_if.h $(ADK_INTERFACE_FW)/app/led/led_if.h $(ADK_INTERFACE_FW)/app/message/subsystem_if.h $(ADK_INTERFACE_FW)/app/message/system_message.h $(ADK_INTERFACE_FW)/app/mic_bias/mic_bias_if.h $(ADK_INTERFACE_FW)/app/operator/operator_if.h $(ADK_INTERFACE_FW)/app/partition/partition_if.h $(ADK_INTERFACE_FW)/app/pio/pio_if.h $(ADK_INTERFACE_FW)/app/ps/ps_if.h $(ADK_INTERFACE_FW)/app/psu/psu_if.h $(ADK_INTERFACE_FW)/app/ringtone/ringtone_if.h $(ADK_INTERFACE_FW)/app/ringtone/ringtone_notes.h $(ADK_INTERFACE_FW)/app/sd_mmc/sd_mmc_if.h $(ADK_INTERFACE_FW)/app/status/status_if.h $(ADK_INTERFACE_FW)/app/stream/stream_if.h $(ADK_INTERFACE_FW)/app/uart/uart_if.h $(ADK_INTERFACE_FW)/app/usb/usb_hub_if.h $(ADK_INTERFACE_FW)/app/usb/usb_if.h $(ADK_INTERFACE_FW)/app/vm/vm_if.h $(ADK_INTERFACE_FW)/app/voltsense/voltsense_if.h $(ADK_INTERFACE_FW)/hydra_macros.h $(ADK_INTERFACE_STD)/hydra_types.h $(ADK_INTERFACE_FW)/hydra_log.h $(ADK_INTERFACE_FW)/hydra_log_disabled.h $(ADK_INTERFACE_FW)/hydra_log_firm.h $(ADK_INTERFACE_FW)/hydra_log_firm_modules.h $(ADK_INTERFACE_FW)/hydra_log_soft.h $(ADK_INTERFACE_STD)/macros.h $(ADK_INTERFACE_STD)/types.h $(ADK_INTERFACE_FW)/pmalloc.h $(ADK_INTERFACE_FW)/pmalloc_trace.h $(ADK_INTERFACE_STD)/csrtypes.h $(ADK_INTERFACE_FW)/hydra_log_subsystems.h $(ADK_INTERFACE_FW)/adc.h $(ADK_INTERFACE_FW)/api.h $(ADK_INTERFACE_FW)/audio_anc.h $(ADK_INTERFACE_FW)/audio_clock.h $(ADK_INTERFACE_FW)/audio_mclk.h $(ADK_INTERFACE_FW)/audio_power.h $(ADK_INTERFACE_FW)/audio_pwm.h $(ADK_INTERFACE_FW)/bdaddr_.h $(ADK_INTERFACE_FW)/bitserial_api.h $(ADK_INTERFACE_FW)/boot.h $(ADK_INTERFACE_FW)/capacitivesensor.h $(ADK_INTERFACE_FW)/charger.h $(ADK_INTERFACE_FW)/codec_.h $(ADK_INTERFACE_FW)/crypto.h $(ADK_INTERFACE_FW)/csb.h $(ADK_INTERFACE_FW)/csb_.h $(ADK_INTERFACE_FW)/dormant.h $(ADK_INTERFACE_FW)/energy.h $(ADK_INTERFACE_FW)/feature.h $(ADK_INTERFACE_FW)/file.h $(ADK_INTERFACE_FW)/font.h $(ADK_INTERFACE_FW)/host.h $(ADK_INTERFACE_FW)/i2c.h $(ADK_INTERFACE_FW)/imageupgrade.h $(ADK_INTERFACE_FW)/infrared.h $(ADK_INTERFACE_FW)/inquiry.h $(ADK_INTERFACE_FW)/kalimba.h $(ADK_INTERFACE_FW)/lcd.h $(ADK_INTERFACE_FW)/led.h $(ADK_INTERFACE_FW)/loader.h $(ADK_INTERFACE_FW)/message.h $(ADK_INTERFACE_FW)/message_.h $(ADK_INTERFACE_FW)/micbias.h $(ADK_INTERFACE_FW)/native.h $(ADK_INTERFACE_FW)/nfc.h $(ADK_INTERFACE_FW)/operator.h $(ADK_INTERFACE_FW)/operator_.h $(ADK_INTERFACE_FW)/os.h $(ADK_INTERFACE_FW)/otp.h $(ADK_INTERFACE_FW)/panic.h $(ADK_INTERFACE_FW)/partition.h $(ADK_INTERFACE_FW)/pio.h $(ADK_INTERFACE_FW)/ps.h $(ADK_INTERFACE_FW)/psu.h $(ADK_INTERFACE_FW)/sdmmc.h $(ADK_INTERFACE_FW)/sink.h $(ADK_INTERFACE_FW)/sink_.h $(ADK_INTERFACE_FW)/source.h $(ADK_INTERFACE_FW)/source_.h $(ADK_INTERFACE_FW)/sram.h $(ADK_INTERFACE_FW)/status.h $(ADK_INTERFACE_FW)/stream.h $(ADK_INTERFACE_FW)/test.h $(ADK_INTERFACE_FW)/transform.h $(ADK_INTERFACE_FW)/transform_.h $(ADK_INTERFACE_FW)/usb.h $(ADK_INTERFACE_FW)/usb_hub.h $(ADK_INTERFACE_FW)/util.h $(ADK_INTERFACE_FW)/vm.h $(ADK_INTERFACE_FW)/vm_.h $(ADK_INTERFACE_FW)/voltsense.h:
	$(PYTHON) $(MAKE_TOOLS_DIR)/install_adk_header.py $< $@
$(LIB_DIR)/lib$(CHIP_TYPE).a: $(C_OBJECTS) $(ASM_OBJECTS)
	$(MAKE_DIR)
	$(AR) ru $@ $^
$(LIB_DIR)/libusb_early_init.a: $(BUILD_DIR)/customer/core/init/init_usb_early_init.o
	$(MAKE_DIR)
	$(AR) ru $@ $^
$(BUILD_DIR)/%.o: %.c $(H_SOURCE)
	$(MAKE_DIR)
	$(info Compiling $<)
	$(CC) $(CFLAGS) -o $@ -c $<
$(BUILD_DIR)/%.o: %.asm $(H_SOURCE)
	$(MAKE_DIR)
	$(info Compiling $<)
	$(CC) $(CFLAGS) -o $@ -c $<
.PHONY: clean
clean:
	-$(RMDIR) $(call FIXPATH,$(BUILDOUTPUT_PATH)/build)
	-$(RMDIR) $(call FIXPATH,$(LIB_DIR))
