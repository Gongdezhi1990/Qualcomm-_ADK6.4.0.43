/*************************************************************************
 * Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/
/**
 * \defgroup clk_mgr Clock Manager
 *
 * \file clk_mgr_for_adaptors.h
 * \ingroup clk_mgr
 *
 * Definitions included directly by adaptors.
 */
#ifndef CLK_MGR_FOR_ADAPTORS_H
#define CLK_MGR_FOR_ADAPTORS_H

/****************************************************************************
Include Files
*/

/* Note: This type must be kept in line with its counterpart
   on the adaptor side. */
typedef enum
{
    CLK_MGR_POWER_SAVE_MODES_PM_QUERY = 0,
    CLK_MGR_POWER_SAVE_MODES_PM1 = 1,
    CLK_MGR_POWER_SAVE_MODES_PM2 = 2,
    CLK_MGR_POWER_SAVE_MODES_PM3 = 3,
    CLK_MGR_POWER_SAVE_MODES_PM4 = 4,
    CLK_MGR_POWER_SAVE_MODES_PM5 = 5
} CLK_MGR_POWER_SAVE_MODES;

/* Note: This type must be kept in line with its counterpart
   on the adaptor side. */
typedef enum
{
    CLK_MGR_CPU_CLK_NO_CHANGE = 0,
    CLK_MGR_CPU_CLK_EXT_LP_CLOCK = 1,
    CLK_MGR_CPU_CLK_VERY_LP_CLOCK = 2,
    CLK_MGR_CPU_CLK_LP_CLOCK = 3,
    CLK_MGR_CPU_CLK_VERY_SLOW_CLOCK = 4,
    CLK_MGR_CPU_CLK_SLOW_CLOCK = 5,
    CLK_MGR_CPU_CLK_BASE_CLOCK = 6,
    CLK_MGR_CPU_CLK_TURBO = 7,
    CLK_MGR_CPU_CLK_TURBO_PLUS = 8
} CLK_MGR_CPU_CLK;

typedef struct
{
    CLK_FREQ_MHZ low_power_mode_clk;
    CLK_FREQ_MHZ wake_on_mode_clk;
    CLK_FREQ_MHZ active_mode_clk;
} CLK_MGR_CPU_CLKS;

typedef bool (*CLK_MGR_ADAPTOR_AUDIO_MODE_CALLBACK)(unsigned conidx,
                                                    STATUS_KYMERA status,
                                                    CLK_MGR_POWER_SAVE_MODES power_save_mode);

typedef bool (*CLK_MGR_ADAPTOR_CPU_MODE_CALLBACK)(unsigned conidx,
                                                  STATUS_KYMERA status,
                                                  CLK_MGR_CPU_CLKS *cpu_mode);

/**
 * \brief Registers clock preference for the audio/kcodec clock.
 *        For explicit usage by accmd_audio_interface
 *
 * \param con_id     Connection/client ID
 * \param power_mode Kcodec power mode requested configuration
 * \param callback   Callback to return the configuration status
 */
extern void clk_mgr_adaptor_audio_mode_req(unsigned con_id,
                                           CLK_MGR_POWER_SAVE_MODES power_mode,
                                           CLK_MGR_ADAPTOR_AUDIO_MODE_CALLBACK callback);

/**
 * \brief Registers clock preference for the various operation modes.
 * For explicit usage by adaptors.
 *
 * \param con_id             Connection/client ID
 * \param low_power_mode_clk CPU power mode to be used in low power.
 * \param wake_on_mode_clk   CPU power mode to be used while wakin up.
 * \param active_mode_clk    CPU power mode when the subsystem is active.
 * \param callback           Function to call to report the result.
 */
extern void clk_mgr_adaptor_cpu_mode_req(unsigned con_id,
                                         CLK_MGR_CPU_CLK low_power_mode_clk,
                                         CLK_MGR_CPU_CLK wake_on_mode_clk,
                                         CLK_MGR_CPU_CLK active_mode_clk,
                                         CLK_MGR_ADAPTOR_CPU_MODE_CALLBACK callback);

/**
 * \brief Returns TRUE if the kcodec block clock is enabled.
 *
 * \return  TRUE if enabled
 */
extern bool clk_mgr_kcodec_is_clocked(void);

#endif /* CLK_MGR_FOR_ADAPTORS_H */
