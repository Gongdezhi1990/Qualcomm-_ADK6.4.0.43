/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup HAL Hardware Abstraction Layer
 * \file  hal.h
 *
 * Public header file for HAL functions.
 * Currently just initialisation
 * Likely to get split between functional areas later.
 * \ingroup HAL
 *
 */

/****************************************************************************
Include Files
*/

#include "hal_macros.h"
#include "io_defs.h"

#ifndef HAL_ALIAS_H
#define HAL_ALIAS_H

#define hal_get_reg_proc_pio_status() hal_get_reg_audio_sys_pio_status()
#define hal_get_reg_proc_pio_status2() hal_get_reg_audio_sys_pio_status2()

#ifdef CHIP_STRE
#define hal_set_audio_ana_ref_micbias_en(x) hal_set_audio_di_ref_micbias_en(x)
#define hal_get_audio_ana_ref_micbias_en(x) hal_get_audio_di_ref_micbias_en(x)
#define hal_set_reg_enable_private_ram(x) hal_set_reg_enable_fast_private_ram(x)
#define hal_set_audio_ana_adc_ch1_gain_sel(x) hal_set_audio_di_li1_selgain(x)
#define hal_set_audio_ana_adc_ch2_gain_sel(x) hal_set_audio_di_li2_selgain(x)
#if CHIP_BUILD_VER == CHIP_MINOR_VERSION_d00
/* The hal_set_kcodec_config_dsm_?_sel and hal_set_clkgen_audio_pwm_dac_?_en
   macros are not available on D00.
   Have them do nothing instead of littering the source with "ifdefs". */
#define KCODEC_DSM_SEL_1B_PWM_DAC 0
#define KCODEC_DSM_SEL_2B_PWM_DAC 0
#define hal_set_kcodec_config_dsm_a_sel(x)
#define hal_set_kcodec_config_dsm_b_sel(x)
#define hal_set_clkgen_audio_pwm_dac_a_en(x)
#define hal_set_clkgen_audio_pwm_dac_b_en(x)
#endif
#endif /* CHIP_STRE */

#ifdef CHIP_AMBER
#define hal_set_clkgen_audio_enables_pcm0_en(x) hal_set_clkgen_audio_pcm1_en(x)
#define hal_set_audio_enables_pcm0_en(x) hal_set_audio_enables_pcm_en(x)
#define hal_set_audio_enables_pcm0_in_en(x) hal_set_audio_enables_pcm_in_en(x)
#define hal_set_audio_enables_pcm0_out_en(x) hal_set_audio_enables_pcm_out_en(x)
#define hal_get_audio_enables_pcm0_en()  hal_get_audio_enables_pcm_en()
#define hal_get_audio_enables_pcm0_in_en()  hal_get_audio_enables_pcm_in_en()
#define hal_get_audio_enables_pcm0_out_en()  hal_get_audio_enables_pcm_out_en()

#define AUDIO_ENABLES_PCM0_IN_EN AUDIO_ENABLES_PCM_IN_EN
#define AUDIO_ENABLES_PCM0_OUT_EN AUDIO_ENABLES_PCM_OUT_EN

#else
#define hal_set_clkgen_audio_enables_pcm0_en(x) hal_set_clkgen_audio_pcm0_en(x)
#endif /* CHIP_AMBER */

/* Add macros for other PCM instances, if they exist */
#ifdef INSTALL_AUDIO_INTERFACE_PCM
#if NUMBER_PCM_INSTANCES > 1
#define hal_set_clkgen_audio_enables_pcm1_en(x) hal_set_clkgen_audio_pcm1_en(x)
#endif
#if NUMBER_PCM_INSTANCES > 2
#define hal_set_clkgen_audio_enables_pcm2_en(x) hal_set_clkgen_audio_pcm2_en(x)
#endif
#if NUMBER_PCM_INSTANCES > 3
#define hal_set_clkgen_audio_enables_pcm3_en(x) hal_set_clkgen_audio_epcm0_en(x)
#define hal_set_audio_enables_pcm3_en(x) hal_set_audio_enables_epcm0_en(x)
#define hal_set_audio_enables_pcm3_in_en(x) hal_set_audio_enables_epcm0_in_en(x)
#define hal_set_audio_enables_pcm3_out_en(x) hal_set_audio_enables_epcm0_out_en(x)
#define hal_get_audio_enables_pcm3_en()  hal_get_audio_enables_epcm0_en()
#define hal_get_audio_enables_pcm3_in_en()  hal_get_audio_enables_epcm0_in_en()
#define hal_get_audio_enables_pcm3_out_en()  hal_get_audio_enables_epcm0_out_en()
#endif
#endif /* INSTALL_AUDIO_INTERFACE_PCM */

#ifdef HAVE_CRESCENDO_AUDIO_MUX
#define  hal_set_reg_proc_pio_drive(x) hal_set_reg_audio_sys_proc_pio_drive(x)
#define  hal_get_reg_proc_pio_drive() hal_get_reg_audio_sys_proc_pio_drive()

#define  hal_set_reg_proc_pio_drive2(x) hal_set_reg_audio_sys_proc_pio_drive2(x)
#define  hal_get_reg_proc_pio_drive2() hal_get_reg_audio_sys_proc_pio_drive2()

#define  hal_set_reg_proc_pio_drive_enable(x) hal_set_reg_audio_sys_proc_pio_drive_enable(x)
#define  hal_get_reg_proc_pio_drive_enable() hal_get_reg_audio_sys_proc_pio_drive_enable()

#define  hal_set_reg_proc_pio_drive_enable2(x) hal_set_reg_audio_sys_proc_pio_drive_enable2(x)
#define  hal_get_reg_proc_pio_drive_enable2() hal_get_reg_audio_sys_proc_pio_drive_enable2()



#else /* HAVE_CRESCENDO_AUDIO_MUX */

#define  hal_set_reg_proc_pio_drive(x) hal_set_reg_audio_sys_pio_drive(x)
#define  hal_get_reg_proc_pio_drive() hal_get_reg_audio_sys_pio_drive()

#define  hal_set_reg_proc_pio_drive2(x) hal_set_reg_audio_sys_pio_drive2(x)
#define  hal_get_reg_proc_pio_drive2() hal_get_reg_audio_sys_pio_drive2()

#define  hal_set_reg_proc_pio_drive_enable(x) hal_set_reg_audio_sys_pio_drive_enable(x)
#define  hal_get_reg_proc_pio_drive_enable() hal_get_reg_audio_sys_pio_drive_enable()

#define  hal_set_reg_proc_pio_drive_enable2(x) hal_set_reg_audio_sys_pio_drive_enable2(x)
#define  hal_get_reg_proc_pio_drive_enable2() hal_get_reg_audio_sys_pio_drive_enable2()

#endif /* HAVE_CRESCENDO_AUDIO_MUX */

#ifdef CHIP_AMBER
#define hal_set_clkgen_tbus_adptrs_en(x) hal_set_clkgen_adptrs_en(x)
#define hal_set_audio_in_event_type_buffer_error_mask hal_set_audio_in_event_type_buffer_error
#define hal_set_audio_in_event_type_fifo_ovrflow_mask hal_set_audio_in_event_type_fifo_ovrflow
#define hal_set_audio_out_event_type_buffer_error_mask hal_set_audio_out_event_type_buffer_error
#define hal_set_audio_out_event_type_fifo_undrflow_mask hal_set_audio_out_event_type_fifo_undrflow

#define hal_set_audio_fifo_config_fifo_service_fshn hal_set_audio_config2_fifo_service_fshn
#define hal_set_audio_fifo_config_in_fifo_empty_depth hal_set_audio_config2_fifo_in_empty_depth
#define hal_set_audio_fifo_config_out_fifo_depth hal_set_audio_config2_fifo_out_depth
#define hal_set_audio_fifo_config_out_fifo_fill_depth hal_set_audio_config2_fifo_out_fill_depth
#define hal_set_audio_fifo_config_buffer_set_limit hal_set_audio_config2_mmu_flush_limit

#define AUDIO_MUX_EVENT       AUDIO_EVENT
#define BAC_LOCAL_READ_ADPTR  AUDIO_STREAM_READ_ADPTR
#define BAC_LOCAL_WRITE_ADPTR AUDIO_STREAM_WRITE_ADPTR
#define EXCEPTION_TYPE_PM_PROG_REGION   EXCEPTION_TYPE_PM_MAPPED_CACHE_EXECUTE

typedef int_sources_list int_source;

#else /* CHIP_AMBER */


#endif /* CHIP_AMBER */

#ifdef CHIP_BASE_CRESCENDO
#endif /* CHIP_BASE_CRESCENDO */

#if defined(CHIP_GORDON)
#define INT_SOURCE_LOW_PRI_SW0            INT_SOURCE_INT_EVENT_SW0_LSB_POSN
#define INT_SOURCE_LOW_PRI_SW1            INT_SOURCE_INT_EVENT_SW1_LSB_POSN
#define INT_SOURCE_LOW_PRI_SW2            INT_SOURCE_INT_EVENT_SW2_LSB_POSN
#define INT_SOURCE_LOW_PRI_SW3            INT_SOURCE_INT_EVENT_SW3_LSB_POSN

#define INT_SOURCE_LOW_PRI_TIMER2         INTERRUPTS_INT_EVENT_TIMER2_LSB_POSN
#endif /* CHIP_GORDON */

#if defined (CHIP_BASE_CRESCENDO) || defined(CHIP_NAPIER)
#define CHIP_INT_SOURCE_SW0 INT_SOURCE_SW0
#else
#define CHIP_INT_SOURCE_SW0 INT_SOURCE_LOW_PRI_SW0
#endif

#if defined (CHIP_AMBER)
#define hal_get_doloop_cache_config_doloop_en hal_get_cache_config_doloop_en
#define hal_set_doloop_cache_config_doloop_en hal_set_cache_config_doloop_en
#endif

#if defined (CHIP_AMBER)
/* PREFETCH_FLUSH was defined as a 1 bit register without any fields previously */
#define hal_set_prefetch_flush(x) hal_set_reg_prefetch_flush(x)
#endif

#endif /* HAL_ALIAS_H */
