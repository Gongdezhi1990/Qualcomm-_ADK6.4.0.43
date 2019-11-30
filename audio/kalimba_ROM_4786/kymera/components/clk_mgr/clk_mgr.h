/*************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/
/**
 * \defgroup clk_mgr clock Manager
 *
 * \file clk_mgr.h
 * \ingroup clk_mgr
 *
 * Public definitions for clk_mgr
 */
#ifdef INSTALL_CLK_MGR
#ifndef _CLK_MGR_H_
#define _CLK_MGR_H_

/****************************************************************************
Include Files
*/
#include "hydra_types.h"

/* CPU Power modes for Stre chip */
typedef enum
{
    LOW_POWER,
    WAKE_ON,
    ACTIVE,
    OP_MODE_N /*Do not use! keeps track of count*/
} OP_MODE;

/* clock frequency
 * Forced to uint16 to match curator CCP response*/
typedef enum
{
    CLK_OFF,
    FREQ_1MHZ = 1,
    FREQ_2MHZ = 2,
    FREQ_4MHZ = 4,
    FREQ_8MHZ = 8,
    FREQ_16MHZ = 16,
    FREQ_32MHZ = 32,
    FREQ_80MHZ = 80,
    FREQ_120MHZ = 120,
    FREQ_UNSPECIFIED = 0x7fff /* error code when input/response is invalid */
} CLK_FREQ_MHZ;

/**
 * Definition of callback type. Return the CPU configuration status to
 * the client module
 *
 * \param priv Private caller data
 * \param status Audio status code
 *
 */
typedef void (*CLK_MGR_CBACK)(void *priv, unsigned status);

/**
 * Definition of callback type. Accmd callback prototype
 *
 * \param con_id The undecoded PDU
 * \param status Audio status code
 */
typedef bool (*CLK_MGR_ACCMD_CBACK)(unsigned con_id, unsigned status);

/**
 * \brief Handle a clock change response from the Curator.
 *
 * This function will only be called from ssccp_router
 *
 * \param pdu The undecoded PDU
 * \param pdu_len_words Its length
 *
 * \return True if the PDU was successfully unpacked.
 */
extern bool clk_mgr_variant_rsp_handler(const uint16 *pdu,
        uint16 pdu_len_words);

/**
 *
 * \brief Registers clock preference for the various operation modes.
 * For explicit usage by accmd_audio_interface
 *
 * \param con_id  Connection/client ID
 * \param low_power_mode_clk Accmd configuration parameter for low power clock
 *  preference
 * \param wake_on_mode_clk Accmd configuration parameter for wake on clock
 *  preference
 * \param active_mode_clk Accmd configuration parameter for active clock
 *  preference
 * \param callback Callback to return the configuration status
 *
 * \return Audio Status code
 */
extern void clk_mgr_accmd_cpu_mode_req(unsigned con_id, uint16 low_power_mode_clk,
        uint16 wake_on_mode_clk, uint16 active_mode_clk,
        CLK_MGR_ACCMD_CBACK callback);

/**
 *
 * \brief Registers clock preference for the audio/kcodec clock.
 * For explicit usage by accmd_audio_interface
 *
 * \param con_id  Connection/client ID
 * \param power_mode Accmd power mode requested configuration
 * \param callback Callback to return the configuration status
 *
 * \return Audio Status code
 */  
void clk_mgr_accmd_audio_mode_req(unsigned con_id, ACCMD_POWER_SAVE_MODES power_mode,
        CLK_MGR_ACCMD_CBACK accmd_cback);

/**
 * \brief Configures Audio CPU clock according to the requested mode.
 *
 * This function will call the callback before return when the requested mode
 * is the same as the current mode
 *
 * \param mode Requested mode
 * \param priv Private caller data
 * \param callback Callback to be called after the reception of the CCP
 *  response
 *
 * \return True if success.
 */
extern bool clk_mgr_set_mode(OP_MODE mode, void *priv, CLK_MGR_CBACK callback);

/**
 * \brief Configures Audio engine and KCODEC clock according to the requested
 * configuration.
 *
 * Audio power mode will be requested by accmd_clock_power_save_mode.
 * clk_mgr_aud_on will try to provide the requested by the apps power
 * configuration. This may not be possible if CPU clk configuration uses
 * different divider. In such cases, Audio will provide the next lowest
 * possible power configuration.
 *
 * \param priv Private caller data
 * \param callback Callback to be called after the reception of the CCP
 *  response
 *
 * \return True if  success.
 */
extern bool clk_mgr_aud_on(void *priv, CLK_MGR_CBACK callback);

/**
 * \brief Turns engine/Kcodec clock off
 *
 * \param priv Private caller data
 * \param callback Callback to be called after the reception of the CCP
 *  response
 *
 * \return True if  success.
 */
extern bool clk_mgr_aud_off(void *priv, CLK_MGR_CBACK callback);

/**
 * \brief Returns Audio CPU clock frequency.
 *
 * \return  Audio CPU clock frequency.
 */
extern CLK_FREQ_MHZ clk_mgr_get_current_cpu_freq(void);

/**
 * \brief Returns Audio CPU frequency for the requested mode. If the requested
 *  mode is not the current mode, the returned frequency will not be the same as
 *  the running Audio cpu clock.
 *
 * \param mode
 *
 * \return  Audio CPU clock frequency.
 */
extern CLK_FREQ_MHZ clk_mgr_get_mode_freq(OP_MODE mode);

/**
 * \brief Returns active operation mode for the clock manager.
 *
 * \return  clm_mgr operation mode.
 */
extern OP_MODE clk_mgr_get_current_mode(void);

/**
 * \brief Returns the current configuration for the audio/kcodec clock.
 *
 * \return  Running frequency of kcodec clock
 */
extern CLK_FREQ_MHZ clk_mgr_get_aud_clk(void);

/**
 * \brief Returns the current configuration for the engine clock.
 *
 * \return  Running frequency of Engine clock
 */
extern CLK_FREQ_MHZ clk_mgr_get_eng_clk(void);

/**
 * \brief Returns active operation mode for the clock manager.
 *
 * \return  clm_mgr operation mode.
 */
ACCMD_POWER_SAVE_MODES clk_mgr_get_power_mode(void);

/**
 * \brief Changes the MUX divider to support SPDIF 192 KHz 24 Bit Mode
 *
 * \return  True if successful
 */
extern bool clk_mgr_spdif_192KHz_support_en(void);

/**
 * \brief Changes the MUX divider to default value
 *
 * \return  True if successful
 */
extern bool clk_mgr_spdif_192KHz_support_disable(void);

/**
 * \brief Initialises default values for clock manager data
 */
extern void clk_mgr_init(void);

/*Macro Definitions*/

/*Number of operation modes*/
#define NUMBER_OF_MODES   OP_MODE_N

#endif /*_CLK_MGR_H_*/
#endif /* INSTALL_CLK_MGR */
