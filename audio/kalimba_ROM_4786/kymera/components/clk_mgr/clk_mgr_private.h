/*************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/
/**
 * \defgroup clk_mgr clock Manager
 *
 * \file clk_mgr_private.h
 * \ingroup clk_mgr
 *
 * Private definitions for ps_router
 */
#ifdef INSTALL_CLK_MGR
#ifndef _CLK_MGR_PRIVATE_H_
#define _CLK_MGR_PRIVATE_H_

/****************************************************************************
Include Files
*/
#include "status_prim.h"
#include "hydra_log/hydra_log.h"
#include "pmalloc/pl_malloc.h"
#include "patch/patch.h"
#include "pl_assert.h"
#include "accmd_prim.h"
#include "clk_mgr.h"

/*cpu clock ccp request configurations
 * Forced to uint16 to match the definition of the CCP*/
typedef enum
{
CPU_CLOCK_SELECT_XTAL,
CPU_CLOCK_SELECT_PLL,
CPU_CLOCK_SELECT_PLL_TURBO,
CPU_CLOCK_SELECT_XTAL_DIV2,
CPU_CLOCK_SELECT_XTAL_DIV4,
CPU_CLOCK_SELECT_XTAL_DIV8,
CPU_CLOCK_SELECT_XTAL_DIV16,
CPU_CLOCK_SELECT_QUERY = 0xff,
CPU_CLOCK_SELECT_UNSPECIFIED, /* Used as ERROR code when CPU clock is invalid */
} CPU_CLOCK_SELECT;

/*Engine clock ccp request configurations
 * Forced to uint16 to match the definition of the CCP*/
typedef enum
{
ENG_CLOCK_SELECT_XTAL,
ENG_CLOCK_SELECT_XTAL_DIV2,
ENG_CLOCK_SELECT_XTAL_DIV4 ,
ENG_CLOCK_SELECT_XTAL_DIV8,
ENG_CLOCK_SELECT_XTAL_DIV16,
ENG_CLOCK_SELECT_OFF = 0xfe,
ENG_CLOCK_SELECT_QUERY,
ENG_CLOCK_SELECT_UNSPECIFIED, /* Used as ERROR code when ENG clock is invalid */
} ENG_CLOCK_SELECT;

/**
 * Definition of callback type for retrieving cpu and engine clock configuration
 */
typedef void (*CLK_MGR_CCP_RSP_HANDLER)(CLK_FREQ_MHZ cpu_clock,
        CLK_FREQ_MHZ engine_clock);

/*clk_mgr clock status and mode
 *  cpu_clk_freq_mhz = root clock comes from curator
 *  eng_clk_freq_mhz = engine clock comes from curator
 *  kcodec_power_mode = KCODEC configured power mode
 *  kcodec_div = kcodec divider. It is used for error prediction if apps tries
 *  kcodec_clients = Resource manager treats kcodec and digimic as different
 *                  resource. We need to keep track here for now
 *  to change the clock without tearing the graph
 *  */
typedef struct clk_status
{
    OP_MODE mode;
    CLK_FREQ_MHZ cpu_clk_freq_mhz;
    CLK_FREQ_MHZ eng_clk_freq_mhz;
    ACCMD_POWER_SAVE_MODES kcodec_power_mode;
    uint16 kcodec_div;
} CLK_STATUS;

/*clk_mgr module status and mode*/
typedef struct
{
    CLK_FREQ_MHZ op_mode_clk[NUMBER_OF_MODES]; /* array idx = mode val= freq */
    CLK_STATUS current_mode;      /* Running clock configurations for the chip*/
    ACCMD_POWER_SAVE_MODES kcodec_power_mode_req; /* An outstanding PM request */
    void *priv_data;                   /* Clients private data */
    CLK_MGR_CBACK clk_mgr_cback;       /* Clients callback */
    uint16 kcodec_clients;  /* kcodec_clients = Resource manager Does not
                               support shared resources. Clk_mgr will keep
                               track of this for now */
} CLK_MGR_STATUS;

/*clk_mgr module status and mode*/
typedef struct
{
    unsigned con_id;
    CLK_MGR_ACCMD_CBACK clk_mgr_accmd_cback;
} CLK_MGR_ACCMD_DATA;

/**
 * \brief This is an intermediate callback. Accmd callback prototype is
 * different from the clk_mgr callback.
 *
 * \param accmd_cback pointer to accmd callback
 * \param status of clock transition
 */
extern void clk_mgr_accmd_cback(void *priv, unsigned status);

/**
 * \brief Convert an ACCMD power mode request to audio clock frequency.
 *  Audio clock frequency may or may not be the same as the engine clock.
 *
 * \return Audio clock frequency in MHz
 */
extern CLK_FREQ_MHZ clk_mgr_accmd_power_mode_to_freq(void);

/**
 * \brief Convert an ACCMD configuration to clock frequency requested
 *
 * \param accmd_mode ACCMD  cpu configuration mode
 * \param cpu_mode CPU power mode
 *
 *
 * \return CPU frequency in MHz
 */
extern CLK_FREQ_MHZ clk_mgr_accmd_cpu_mode_req_to_freq(uint16 accmd_mode, OP_MODE cpu_mode);

/**
 * \brief This function will return the required CCP value in order to
 * achieve the required frequency
 *
 * \param freq_mhz clock frequency in MHz
 *
 * \return CCP value for the desired Audio frequency
 */
extern ENG_CLOCK_SELECT clk_mgr_eng_freq_to_ccp(CLK_FREQ_MHZ freq_mhz);

/**
 * \brief This function will return the required CCP value in order to
 * achieve the required frequency
 *
 * \param freq_mhz clock frequency in MHz
 *
 * \return CCP value for the desired CPU frequency
 */
extern CPU_CLOCK_SELECT clk_mgr_cpu_freq_to_ccp(CLK_FREQ_MHZ freq_mhz);

/**
 * \brief Send a clock change request to the Curator.
 * If both CPU and engine clock are configured wit h AOV, they have
 * to be configured with the same value. Otherwise curator will overwrite the
 * configuration and return identical clocks for both engine and cpu
 *
 * \param cpu_clock CPU clock configuration can be
 * (XTAL, PLL, PLL TURBO or AOV -16/8/4/2).
 * \param engine_clock  Engine clock configuration can be
 * * (XTAL, or AOV -16/8/4/2).
 *
 * \return True if the PDU was successfully unpacked.
 * (Unexpected PDUs will raise an assertion.)
 */
extern bool clk_mgr_variant_req(CPU_CLOCK_SELECT cpu_clock,
        ENG_CLOCK_SELECT engine_clock, CLK_MGR_CCP_RSP_HANDLER callback);

/**
 * \brief checks if the requested Audio power save mode is supported
 *
 * \return True if the mode is supported
 */
bool clk_mgr_accmd_is_valid_power_mode(ACCMD_POWER_SAVE_MODES power_mode);

/*Macro Definitions*/

/* Default kcodec divider for stre */
#define DEFAULT_CODEC_DIV   4

#endif /*_CLK_MGR_PRIVATE_H_*/
#endif /* INSTALL_CLK_MGR */
