/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_audio_a7da_kas.c
 * \ingroup stream
 *
 * stream audio type file. <br>
 * This file contains stream functions for audio endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_audio_get_endpoint <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "stream_endpoint_audio.h" /* For protected access audio ep base class */
#include "kascmd_prim.h"           /* need the types for configuration keys */
#include "kas_audio.h"             /* A7DA audio type definitions */
#include "kas_audio_endpoint.h"
#include "stream_common_a7da_kas.h"
#include "opmgr/opmgr_endpoint_override.h"
#include "kas_kalimba_regs.h"
#include "pmu_audio.h"
#include "kas_keyhole.h"
#include "iacc.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/


/****************************************************************************
Private Macro Declarations
*/
/** Macro pairs to enable use of kalimba memory mapped register definitions without
 *  generating warnings */
#define POSTFIX(a, p) a ## p
#define UINT(aDDR) POSTFIX(aDDR,u)
#define ULNG(aDDR) POSTFIX(aDDR,ul)

#define RATE_RECALC_PERIOD (SECOND*3)

#define CODEC_WARP_ADJUST_SHIFT (6)
#define CODEC_WARP_ADJUST_MAX   (262143)
#define CODEC_WARP_ADJUST_MIN   (-262144)

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

static bool audio_stop (ENDPOINT *ep);
static bool audio_start (ENDPOINT *ep, KICK_OBJECT *ko);
static bool audio_configure (ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool audio_get_config (ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result);
static int get_audio_rate(ENDPOINT *ep);
static void adjust_audio_rate_swep(ENDPOINT *ep, int32 adjust_val);
static void adjust_audio_rate_iacc_snk(ENDPOINT *ep, int32 adjust_val);
static void adjust_audio_rate_iacc_src(ENDPOINT *ep, int32 adjust_val);
static unsigned get_kcodec_gain_from_db60(int gain_db60);
static void adjust_audio_rate_null(ENDPOINT *ep, int32 adjust_val);
extern unsigned dB60toLinearQ5(int db);

DEFINE_ENDPOINT_FUNCTIONS (audio_functions, audio_common_close, audio_common_connect,
                           audio_common_disconnect, audio_common_buffer_details,
                           audio_kick, stream_sched_kick_dummy,
                           audio_start, audio_stop,
                           audio_configure, audio_get_config,
                           audio_common_get_timing, sync_endpoints);



/****************************************************************************
Private Function Definitions
*/

/****************************************************************************
Global Variables
*/

/**
* On-KAS storage for the KCODEC_WARP registers to preserve values and reduce
* amount of reading through the keyhole */
typedef struct
{
       uint16 warp_update_shd; /* shd value - shadows writes through keyhole */
       uint16 warp_update_bak; /* bak values used to back-up at stream start and restore at end */
       uint16 adc_a_warp_bak;
       uint16 adc_b_warp_bak;
       uint16 dac_a_warp_bak;
       uint16 dac_b_warp_bak;
       uint16 dac_c_warp_bak;
       uint16 dac_d_warp_bak;

       /* Stored warp values for caching & warp correction */
       int hw_warp_dac_adjust_prev;
       int hw_warp_adc_adjust_prev;

       uint16 adc_a_gain_bak;       /* ADC gain A backup register */
       uint16 adc_b_gain_bak;       /* ADC gain B backup register */
       bool   adc_dirty_flag:1;     /* ADC gain dirty flag (detects if the registers need backup or not) */

} kcodec_reg_local;

static kcodec_reg_local kcodec_reg_store;

/**
 * On-KAS copy of the INTCODECCTL_TX_RX_EN register to eliminate un-necessary keyhole
 * accesses to KCODEC_WARP registers */
static uint16 intcodecctl_tx_rx_en;

/****************************************************************************
Public Function Definitions
*/
/****************************************************************************
 *
 * stream_audio_get_endpoint
 *
 */
ENDPOINT *stream_audio_get_endpoint(unsigned int con_id,
                                    ENDPOINT_DIRECTION dir,
                                    unsigned int hardware,
                                    unsigned num_params,
                                    unsigned *params,
                                    bool *pending)
{
    if (num_params != A7DA_AUDIO_PARAMS_NUM)
    {
        return NULL;
    }

    int instance  = params[A7DA_AUDIO_PARAMS_INSTANCE];
    int channel   = params[A7DA_AUDIO_PARAMS_CHANNEL];
    int master_id = params[A7DA_AUDIO_PARAMS_MASTER_ID];
    int num_ch    = params[A7DA_AUDIO_PARAMS_NUM_CHANNELS];
    uint32 cb_addr = ((uint32)params[A7DA_AUDIO_PARAMS_CB_ADDR_MSW]<<16) |
                        params[A7DA_AUDIO_PARAMS_CB_ADDR_LSW];

    /* First go and find a stream on the same audio hardware, if not found
     * then create a new stream. */
    uint24 key = create_stream_key(hardware, instance, channel);
    ENDPOINT *endpoint = a7da_get_endpoint_from_parameters(key, cb_addr, dir,
                                                  &endpoint_audio_functions);
    unsigned buffer_size;

    if(!endpoint)
    {
        if ((endpoint = STREAM_NEW_ENDPOINT(audio, key, dir, con_id)) == NULL)
        {
            return NULL;
        }

        /* Initialise endpoint default values */
        endpoint->state.audio.running = FALSE;
        endpoint->state.audio.head_of_sync = endpoint;
        endpoint->state.audio.nep_in_sync = NULL;
        endpoint->can_be_closed = TRUE;
        endpoint->can_be_destroyed = FALSE;
        /* Audio endpoints are always at the end of a chain */
        endpoint->is_real = TRUE;

        /* If number of channels is greater than allowed, fail */
        if(num_ch > NR_MAX_SYNCED_EPS)
        {
            stream_destroy_endpoint(endpoint);
            return NULL;
        }

        /* Creates master (or slave/logical) endpoint */
        if (master_id == 0)
        {
            /* Create the port descriptor and fill the default values */
            endpoint->state.audio.port = a7da_audio_common_get_port(endpoint, dir,
                                            hardware, instance, num_ch, cb_addr);
            if (endpoint->state.audio.port == NULL)
            {
                stream_destroy_endpoint(endpoint);
                /* TODO: notify the adaptor somehow that we ran out of memory */
                return NULL;
            }
            /* Populate the cbops structure */
            if(!a7da_audio_common_create_cbops(endpoint,num_ch))
            {
                a7da_audio_common_destroy_port(endpoint->state.audio.port);
                stream_destroy_endpoint(endpoint);
                return NULL;
            }

        }
        else
        {
            ENDPOINT *master = stream_endpoint_from_extern_id(master_id);
            /* Take the port descriptor and cbops from the master endpoint */
            endpoint->state.audio.port = master->state.audio.port;
            endpoint->cbops = master->cbops;
        }

        /* Get the buffer size */
        if (endpoint->state.audio.port->interleaved)
        {
            buffer_size = a7da_audio_common_get_buffer_size(endpoint)/num_ch;
        }
        else
        {
            buffer_size = a7da_audio_common_get_buffer_size(endpoint);
        }

        /* Initially set the processing block size to the buffer size */
        endpoint->state.audio.block_size = buffer_size;
    }

    /* Initialise ratematch enactment fn_ptr to a safe value */
    endpoint->state.audio.rm_enacting_fn = adjust_audio_rate_null;

    return endpoint;
}


/****************************************************************************
Private Function Definitions
*/

/* ******************************* Helper functions ************************************ */

/**
 * Collect data for rate calculation
 * NOTE: not used, rm data is ether collected at interrupt or retrieved from the HW rate counters
 */
uint24 get_rm_data(ENDPOINT *ep)
{
    return DELTA_SAMPLES_NOT_COMPUTED;
}

/**
 * Process data collected by get_rm_data
 */
void process_rm_data(ENDPOINT *endpoint,
                     unsigned num_cbops_read, unsigned num_cbops_written)
{
}

/**
 *  Calculate audio rate
 */
static int get_audio_rate(ENDPOINT *ep)
{
    long delta_time;
    long num_samples;
    long rate;
    unsigned curr_time = (unsigned) hal_get_time();
    a7da_audio_port* port = ep->state.audio.port;

    /* If the endpoint isn't running or is locally clocked then there is nothing to compensate for,
     * so indicate that the rate is perfect. */
    if (!ep->state.audio.running)
    {
        return RM_PERFECT_RATE;
    }

    /* Only calculate rate measurement if a certain period elapsed
       to avoid unnecessary consecutive calculations */
    if (curr_time - port->rm_period_start_time > RATE_RECALC_PERIOD)
    {
        /* Get the time between the period measurement start */
        if (port->rc_clk_src_addr)
        {
            /* Get the rate count value and reset the register */
            delta_time = curr_time - port->rm_period_start_time;
            num_samples = *(port->rc_clk_src_addr);
            *(port->rc_clk_src_addr) = 0;
            port->rm_period_start_time = curr_time;
        }
        else
        {
            delta_time  = port->rm_delta_time;
            num_samples = ((long)port->channel_block_size*port->rm_kick_count);
            /* reset kick counter, note: this will also trigger the reset of the RM period */
            port->rm_delta_time = 0;
        }

        rate = ((delta_time * port->sample_rate) / (num_samples));
        rate = (rate << (STREAM_RATEMATCHING_FIX_POINT_SHIFT))/SECOND;
        rate = (2 << STREAM_RATEMATCHING_FIX_POINT_SHIFT) - rate;

        /* Save the measured rate */
        port->rm_new_rate = 1;
        port->rm_meas_rate = (unsigned)rate;
    }
    return (int)(port->rm_meas_rate);
}

/**
 * Initialises Rate matching
 */
static void kas_audio_rm_init(ENDPOINT *ep)
{
    kas_audio_device dev = GET_INSTANCE_FROM_AUDIO_EP_KEY(ep->key);
    a7da_audio_port* port = ep->state.audio.port;
    port->rm_kick_count = 0;
    port->rm_delta_time = 0;
    port->rm_period_start_time = (unsigned) hal_get_time();
    port->rm_meas_rate = 1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT;
    port->rc_clk_src_addr = NULL;

    switch (port->rm_support)
    {
        case RATEMATCHING_SUPPORT_SW:
        {
            uint24 rc_enable_mask = 0;

            /* Initialise the rate matching features. */
            ep->state.audio.rm_adjust_amount = 0;
            ep->state.audio.rm_diff = 0;
            switch(dev)
            {   /* Get the address and enable mask bit for the rate counter register to be used */
                case AC97:
                    port->rc_clk_src_addr = (uint24*) UINT(KAS_RATE_COUNTER_0);
                    rc_enable_mask = 1;
                    break;
                case USP0:
#ifdef A7DA_KAS_ENABLE_USP_RATE_COUNTERS
                    port->rc_clk_src_addr = (uint24*) UINT(KAS_RATE_COUNTER_1);
                    rc_enable_mask = 1<<(UINT(KAS_RATE_COUNTER_1) - UINT(KAS_RATE_COUNTER_0));
                    break;
#endif  /* A7DA_KAS_ENABLE_USP_RATE_COUNTERS */
                case USP1:
#ifdef A7DA_KAS_ENABLE_USP_RATE_COUNTERS
                    port->rc_clk_src_addr = (uint24*) UINT(KAS_RATE_COUNTER_2);
                    rc_enable_mask = 1<<(UINT(KAS_RATE_COUNTER_2) - UINT(KAS_RATE_COUNTER_0));
#endif  /* A7DA_KAS_ENABLE_USP_RATE_COUNTERS */
                    break;
                case USP2:
#ifdef A7DA_KAS_ENABLE_USP_RATE_COUNTERS
                    port->rc_clk_src_addr = (uint24*) UINT(KAS_RATE_COUNTER_3);
                    rc_enable_mask = 1<<(UINT(KAS_RATE_COUNTER_3) - UINT(KAS_RATE_COUNTER_0));
#endif  /* A7DA_KAS_ENABLE_USP_RATE_COUNTERS */
                    break;
                case I2S1:
                    port->rc_clk_src_addr = (uint24*) UINT(KAS_RATE_COUNTER_4);
                    rc_enable_mask = 1<<(UINT(KAS_RATE_COUNTER_4) - UINT(KAS_RATE_COUNTER_0));
                    break;
                case USP3:
#ifdef A7DA_KAS_ENABLE_USP_RATE_COUNTERS
                    port->rc_clk_src_addr = (uint24*) UINT(KAS_RATE_COUNTER_5);
                    rc_enable_mask = 1<<(UINT(KAS_RATE_COUNTER_5) - UINT(KAS_RATE_COUNTER_0));
#endif  /* A7DA_KAS_ENABLE_USP_RATE_COUNTERS */
                    break;
                default:
                    panic_diatribe(PANIC_AUDIO_INVALID_DEVICE_ID, dev);
                    break;
            }
            /* Initialise the rm-enacting function pointer */
            ep->state.audio.rm_enacting_fn = adjust_audio_rate_swep;
            /* enable rate counter register */
            *(uint24*) UINT(KAS_RATE_COUNTER_ENABLE) |= rc_enable_mask;
            port->rc_enable_mask = rc_enable_mask;
            if (port->rc_clk_src_addr)
            {
                *port->rc_clk_src_addr = 0; /*initialise rate counter register*/
            }
            break;
        }
        case RATEMATCHING_SUPPORT_HW:
        {
            uint32 tmp;
            /* Get on-KAS copy of the INTCODECCTL_TX_RX_EN register */
            keyhole_read_32b_addr(INTCODECCTL_TX_RX_EN, &tmp);
            intcodecctl_tx_rx_en = (uint16)tmp;

            switch(dev)
            {   /* Hardware specific stuff */
                case IACC:
                    keyhole_read_32b_addr(ULNG(KCODEC_WARP_UPDATE), &tmp);
                    kcodec_reg_store.warp_update_bak = (uint16)tmp;
                    kcodec_reg_store.warp_update_shd = (uint16)tmp;
                    if(ep->direction == SOURCE)
                    {
                        /* Back-up the warp registers */
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__ADC0_EN__MASK)
                        {
                            keyhole_read_32b_addr(ULNG(KCODEC_ADC_A_WARP), &tmp);
                            kcodec_reg_store.adc_a_warp_bak = (uint16)tmp;
                        }
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__ADC1_EN__MASK)
                        {
                            keyhole_read_32b_addr(ULNG(KCODEC_ADC_B_WARP), &tmp);
                            kcodec_reg_store.adc_b_warp_bak = (uint16)tmp;
                        }
                        /* Initialise the rm-enacting function pointer */
                        ep->state.audio.rm_enacting_fn = adjust_audio_rate_iacc_src;
                        /* reset the warp value cache */
                        kcodec_reg_store.hw_warp_adc_adjust_prev = 0;
                    }
                    else
                    {
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC0_EN__MASK)
                        {
                            keyhole_read_32b_addr(ULNG(KCODEC_DAC_A_WARP), &tmp);
                            kcodec_reg_store.dac_a_warp_bak = (uint16)tmp;
                        }
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC1_EN__MASK)
                        {
                            keyhole_read_32b_addr(ULNG(KCODEC_DAC_B_WARP), &tmp);
                            kcodec_reg_store.dac_b_warp_bak = (uint16)tmp;
                        }
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC2_EN__MASK)
                        {
                            keyhole_read_32b_addr(ULNG(KCODEC_DAC_C_WARP), &tmp);
                            kcodec_reg_store.dac_c_warp_bak = (uint16)tmp;
                        }
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC3_EN__MASK)
                        {
                            keyhole_read_32b_addr(ULNG(KCODEC_DAC_D_WARP), &tmp);
                            kcodec_reg_store.dac_d_warp_bak = (uint16)tmp;
                        }
                        /* Initialise the rm-enacting function pointer */
                        ep->state.audio.rm_enacting_fn = adjust_audio_rate_iacc_snk;
                        /* reset the warp value cache */
                        kcodec_reg_store.hw_warp_dac_adjust_prev = 0;
                    }
                    break;
                default:
                    panic_diatribe(PANIC_AUDIO_INVALID_DEVICE_ID, dev);
                    break;
            }

            break;
        }
        default:
            break;
    }
}

/**
 * Restores registers used by rate-matching
 */
static void kas_audio_rm_clr(ENDPOINT *ep)
{
    kas_audio_device dev = GET_INSTANCE_FROM_AUDIO_EP_KEY(ep->key);

    switch (ep->state.audio.port->rm_support)
    {
        case RATEMATCHING_SUPPORT_SW:
        {
            *(uint24*) UINT(KAS_RATE_COUNTER_ENABLE) &= ~ep->state.audio.port->rc_enable_mask;
            break;
        }
        case RATEMATCHING_SUPPORT_HW:
        {
            switch(dev)
            {   /* Hardware specific stuff */
                case IACC:
                    keyhole_write_32b_addr(ULNG(KCODEC_WARP_UPDATE), kcodec_reg_store.warp_update_bak);
                    /* Restore the warp registers */
                    if(ep->direction == SOURCE)
                    {
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__ADC0_EN__MASK)
                        {
                            keyhole_write_32b_addr(ULNG(KCODEC_ADC_A_WARP), kcodec_reg_store.adc_a_warp_bak);
                        }
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__ADC1_EN__MASK)
                        {
                            keyhole_write_32b_addr(ULNG(KCODEC_ADC_B_WARP), kcodec_reg_store.adc_b_warp_bak);
                        }
                    }
                    else
                    {
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC0_EN__MASK)
                        {
                            keyhole_write_32b_addr(ULNG(KCODEC_DAC_A_WARP), kcodec_reg_store.dac_a_warp_bak);
                        }
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC1_EN__MASK)
                        {
                            keyhole_write_32b_addr(ULNG(KCODEC_DAC_B_WARP), kcodec_reg_store.dac_b_warp_bak);
                        }
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC2_EN__MASK)
                        {
                            keyhole_write_32b_addr(ULNG(KCODEC_DAC_C_WARP), kcodec_reg_store.dac_c_warp_bak);
                        }
                        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC3_EN__MASK)
                        {
                            keyhole_write_32b_addr(ULNG(KCODEC_DAC_D_WARP), kcodec_reg_store.dac_d_warp_bak);
                        }
                    }
                    break;
                default:
                    panic_diatribe(PANIC_AUDIO_INVALID_DEVICE_ID, dev);
                    break;
            }
            break;
        }
        default:
            break;
    }
}

/**
 * \brief Ratematch enactment function - used as a safety placeholder during stream initialisation.
 *
 * \param ep pointer to the endpoint
 * \param adjust_val amount of ratematch adjustment required
 */
static void adjust_audio_rate_null(ENDPOINT *ep, int32 adjust_val)
{
    /* No action, just return. */
}

/**
 * \brief Ratematch enactment function - used where enactment is by software (SRA).
 *
 * \param ep pointer to the endpoint
 * \param adjust_val amount of ratematch adjustment required
 */
static void adjust_audio_rate_swep(ENDPOINT *ep, int32 adjust_val)
{
    /* For the sake of reducing the MIPS burden no safety checks just do it. */
    ep->state.audio.rm_adjust_amount = (unsigned int)adjust_val;
}

/**
 * \brief Ratematch enactment function - used where enactment is by a hardware SOURCE.
 *
 * \param ep pointer to the endpoint
 * \param adjust_val amount of ratematch adjustment required
 */
static void adjust_audio_rate_iacc_src(ENDPOINT *ep, int32 adjust_val)
{

    /* Do not adjust rate until the measurement has been updated */
    if(!ep->state.audio.port->rm_new_rate)
    {
         return;
    }

    /* Determine the actual bitfield required for the warp adjustment. The HW warp has +-3.125% adjustment available,
     * the adjust_val is for a wider range (and is sign-extended in a 32-bit word). The upshot is that (for the 32-bit
     * input representation), we need to arithmetic-shift "adjust_val" to give a 13-bit warp value in the LSBs.
     * Further info is in CS-236709-SP-E-Marco_Audio_Analog_APB_Register_Map
     * Note: the new warp value must be corrected by adding the current warp to the new warp setting, it is
     *       a rough approximation to: w_corr = 1 - (1-w_prev)*(1-w_new).
     */
    int use_val = kcodec_reg_store.hw_warp_adc_adjust_prev + (int)adjust_val;

    /* Valid adjustment range is based on the HW warp bitfield mapped onto a 32-bit signed int */
    /* If the adjustment required is outside of the available 13-bit range trim to the maximum available adjustment */
    if (use_val > CODEC_WARP_ADJUST_MAX)
    {
        // TODO: add fault/log message (e.g. AUDIO_EP_RATEMATCH_ADJUSTMENT outside available range)
        use_val = CODEC_WARP_ADJUST_MAX;
    }
    else if (use_val < CODEC_WARP_ADJUST_MIN)
    {
        // TODO: add fault/log message (e.g. AUDIO_EP_RATEMATCH_ADJUSTMENT outside available range)
        use_val = CODEC_WARP_ADJUST_MIN;
    }

    /* Only take the latency-hit of keyhole access if the actual bitfield (adjust_val) has changed */
    if(use_val != kcodec_reg_store.hw_warp_adc_adjust_prev)
    {
        uint24 warp_update = (uint24)kcodec_reg_store.warp_update_shd
                             & ULNG(KCODEC_WARP_UPDATE__KCODEC_ADC_WARP_UPDATE_EN__INV_MASK);
        kcodec_reg_store.warp_update_shd = warp_update;
        keyhole_write_32b_addr(ULNG(KCODEC_WARP_UPDATE), warp_update);
        kcodec_reg_store.hw_warp_adc_adjust_prev = use_val;
        use_val >>= CODEC_WARP_ADJUST_SHIFT;
        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__ADC0_EN__MASK)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_ADC_A_WARP), use_val);
        }
        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__ADC1_EN__MASK)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_ADC_B_WARP), use_val);
        }
        warp_update = (uint24) kcodec_reg_store.warp_update_shd | KCODEC_WARP_UPDATE__KCODEC_ADC_WARP_UPDATE_EN__MASK;
        keyhole_write_32b_addr(ULNG(KCODEC_WARP_UPDATE), warp_update);
        kcodec_reg_store.warp_update_shd = warp_update;
        /* Initiate new measurement */
        ep->state.audio.port->rm_delta_time  = 0;
        ep->state.audio.port->rm_new_rate    = 0;
    }
}

/**
 * \brief Ratematch enactment function - used where enactment is by a hardware SINK.
 *
 * \param ep pointer to the endpoint
 * \param adjust_val amount of ratematch adjustment required
 */
static void adjust_audio_rate_iacc_snk(ENDPOINT *ep, int32 adjust_val)
{
    /* Do not adjust rate until the measurement has been updated */
    if(!ep->state.audio.port->rm_new_rate)
    {
         return;
    }

    /* Determine the actual bitfield required for the warp adjustment. The HW warp has +-3.125% adjustment available,
     * the adjust_val is for a wider range (and is sign-extended in a 32-bit word). The upshot is that (for the 32-bit
     * input representation), we need to arithmetic-shift "adjust_val" to give a 13-bit warp value in the LSBs.
     * NOTE: according to HW specs, the DAC sample rate is *divided* by the factor (1 - warp_reg_value/2^17),
     * therefore the provided adjustment value should be negated.
     * Further info is in CS-236709-SP-E-Marco_Audio_Analog_APB_Register_Map
     * Note: the new warp value must be corrected by adding the current warp to the new warp setting, it is
     *       a rough approximation to: w_corr = 1 - (1-w_prev)*(1-w_new).
     */
    int use_val = kcodec_reg_store.hw_warp_dac_adjust_prev - (int)adjust_val;

    /* Valid adjustment range is based on the HW warp bitfield mapped onto a 32-bit signed int */
    /* If the adjustment required is outside of the available 13-bit range trim to the maximum available adjustment */
    if (use_val > CODEC_WARP_ADJUST_MAX)
    {
       use_val = CODEC_WARP_ADJUST_MAX;
    }
    else if (use_val < CODEC_WARP_ADJUST_MIN)
    {
        use_val = CODEC_WARP_ADJUST_MIN;
    }

    /* Only take the latency-hit of keyhole access if the actual bitfield (adjust_val) has changed */
    if(use_val != kcodec_reg_store.hw_warp_dac_adjust_prev)
    {
        uint24 warp_update = (uint24)kcodec_reg_store.warp_update_shd
                             & ULNG(KCODEC_WARP_UPDATE__KCODEC_DAC_WARP_UPDATE_EN__INV_MASK);
        kcodec_reg_store.warp_update_shd = warp_update;
        keyhole_write_32b_addr(ULNG(KCODEC_WARP_UPDATE), warp_update);

        kcodec_reg_store.hw_warp_dac_adjust_prev = use_val;
        use_val >>= CODEC_WARP_ADJUST_SHIFT;

        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC0_EN__MASK)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_DAC_A_WARP), use_val);
        }
        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC1_EN__MASK)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_DAC_B_WARP), use_val);
        }
        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC2_EN__MASK)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_DAC_C_WARP), use_val);
        }
        if (intcodecctl_tx_rx_en & INTCODECCTL_TX_RX_EN__DAC3_EN__MASK)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_DAC_D_WARP), use_val);
        }
        warp_update = (uint24) kcodec_reg_store.warp_update_shd | KCODEC_WARP_UPDATE__KCODEC_DAC_WARP_UPDATE_EN__MASK;
        keyhole_write_32b_addr(ULNG(KCODEC_WARP_UPDATE), warp_update);
        kcodec_reg_store.warp_update_shd = warp_update;

        /* Signal a new measurement */
        ep->state.audio.port->rm_delta_time  = 0;
        ep->state.audio.port->rm_new_rate    = 0;
    }
}

/*
 *  \brief  This function will return the gain in the format accepted
 *          by the KCODEC "FINE" GAIN register, i.e. U4.5 representation,
 *          gain range from 16.0 (+24dB) to 1/32 (-30dB) in 1/32 steps
 *  \param  gain_db60: gain value in 1/60 dB, useful range is between
 *                     -30dB (-1806) to +24dB (+1441).
 */
static unsigned get_kcodec_gain_from_db60(int gain_db60)
{
    if (gain_db60 > 1441)       /* 24.02dB HW imposed upper limit */
    {
        return 0x81FF;
    }
    else if (gain_db60 < -1806) /* -30.10dB HW imposed lower value limit */
    {
        return 0x8000;
    }

    /* Valid range: calculate linear gain and convert to KCODEC FINE register (9bit) */
    return 0x8000 | (dB60toLinearQ5(gain_db60)>>14);
}

/* ********************************** API functions ************************************* */

/*
 * \brief configure an audio endpoint with a key and value pair
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value value to which the key is to be configured
 *
 * \return Whether the request succeeded.
 *
 */
static bool audio_configure(ENDPOINT *endpoint, unsigned int key, uint32 value)
{
    kas_audio_device dev = GET_INSTANCE_FROM_AUDIO_EP_KEY(endpoint->key);
    switch(key)
    {
    case EP_BLOCK_SIZE:
        /* Use block size and sample rate to calculate kick period */
        endpoint->state.audio.block_size = (unsigned int)value;
        return TRUE;
    case EP_DATA_FORMAT:
    case KASCMD_CONFIG_KEY_AUDIO_DATA_FORMAT:
        return audio_common_set_data_format(endpoint, (AUDIO_DATA_FORMAT)value);
    case KASCMD_CONFIG_KEY_AUDIO_SAMPLE_RATE:
        endpoint->state.audio.port->sample_rate = (unsigned) value;
        return TRUE;
    case KASCMD_CONFIG_KEY_DRAM_PACKING_FORMAT:
        endpoint->state.audio.port->word_format = (unsigned) value;
        return TRUE;
    case KASCMD_CONFIG_KEY_INTERLEAVING_MODE:
        //endpoint->state.audio.port->interleaved = (bool) value;
        return TRUE;
    case KASCMD_CONFIG_KEY_CLOCK_MASTER_SLAVE:
        endpoint->state.audio.port->locally_clocked = (bool) value;
        return TRUE;
    case EP_OVERRIDE_ENDPOINT:
        /* Set the logical value of override flag. */
        endpoint->state.audio.is_overridden = (bool)value;
        /* Depending on the direction of the endpoint, the source/sink bufffer is
         * already set from the endpoint creation. */
        return TRUE;
    case EP_CBOPS_PARAMETERS:
    {
        bool retval;
        CBOPS_PARAMETERS *parameters = (CBOPS_PARAMETERS *)(uintptr_t) value;

        if (parameters)
        {
            /* cbops_mgr should not be updated when endpoint is running. */
            if (!endpoint->is_enabled &&  endpoint->cbops && \
                opmgr_override_pass_cbops_parameters(parameters, endpoint->cbops,
                          endpoint->state.audio.source_buf, endpoint->state.audio.sink_buf))
            {
                retval = TRUE;
            }
            else
            {
                retval = FALSE;
            }
        }
        else
        {
            /* Panic can return in unit test*/
            retval = FALSE;
            panic_diatribe(PANIC_AUDIO_STREAM_INVALID_CONFIGURE_KEY,
                                                            endpoint->id);
        }

        free_cbops_parameters(parameters);
        return retval;
    }
    case EP_SET_INPUT_GAIN:
    {
        if ((dev!=IACC) || (endpoint->direction!=SOURCE))
        {
            return FALSE;
        }

        uint32 gain_reg = (uint32) get_kcodec_gain_from_db60((signed int)value);
        if      (GET_CHANNEL_FROM_AUDIO_EP_KEY(endpoint->key)==0)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_ADC_A_GAIN), gain_reg);
        }
        else if (GET_CHANNEL_FROM_AUDIO_EP_KEY(endpoint->key)==1)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_ADC_B_GAIN), gain_reg);
        }
        kcodec_reg_store.adc_dirty_flag = TRUE;
        return TRUE;
    }
    case EP_SET_OUTPUT_GAIN:
    {
    /* Currently, output gain is handled solely by ALSA so we should return FALSE. */
    #ifdef TBD_A7DA_KAS_KCODEC_ENABLE_OUTPUT_GAIN_CHANGE
        if ((dev!=IACC) || (endpoint->direction!=SINK))
        {
            return FALSE;
        }

        uint32 gain_reg = (uint32) get_kcodec_gain_from_db60((signed int)value);

        /* NOTE: currently all channels are written by default
          (similarly behaviour seen on ALSAmixer), however this
          could be optimised if not all channels are used. */
        keyhole_write_32b_addr(ULNG(KCODEC_DAC_A_GAIN), gain_reg);
        keyhole_write_32b_addr(ULNG(KCODEC_DAC_B_GAIN), gain_reg);
        keyhole_write_32b_addr(ULNG(KCODEC_DAC_C_GAIN), gain_reg);
        keyhole_write_32b_addr(ULNG(KCODEC_DAC_D_GAIN), gain_reg);
        return TRUE;
    #else
        return FALSE;
    #endif /*TBD_A7DA_KAS_KCODEC_ENABLE_OUTPUT_GAIN_CHANGE*/
    }
    case EP_RATEMATCH_ADJUSTMENT:
        L4_DBG_MSG3("AUDIO_EP_RATEMATCH_ADJUSTMENT ep: %06p fn_addr: %06p value: %06x", endpoint,
                    endpoint->state.audio.rm_enacting_fn, value);
        /* adjust_audio_rate - using a function pointer reduces in-function conditionals */
        endpoint->state.audio.rm_enacting_fn(endpoint->state.audio.head_of_sync, (uint32)value);
        return TRUE;
    case EP_RATEMATCH_ENACTING:
    {
        /* Configure non-hardware enacting endpoint to use CBOPS SRA etc */
        value = (endpoint->state.audio.port->rm_support == RATEMATCHING_SUPPORT_HW) ? 0 : value;
        return audio_configure_rm_enacting(endpoint, value);
    }
    default:
        return FALSE;
    }
}

/*
 * \brief get audio endpoint configuration
 *
 * \param *endpoint pointer to the endpoint to be configured
 * \param key denoting what is being configured
 * \param value pointer to a value which is populated with the current value
 *
 * \return Whether the request succeeded.
 *
 */
static bool audio_get_config(ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    kas_audio_device dev = GET_INSTANCE_FROM_AUDIO_EP_KEY(endpoint->key);
    switch(key)
    {
        case EP_BLOCK_SIZE:
            /* Use block size and sample rate to calculate kick period */
            result->u.value = endpoint->state.audio.block_size;
            return TRUE;
        case EP_DATA_FORMAT:
        case KASCMD_CONFIG_KEY_AUDIO_DATA_FORMAT:
            result->u.value = (uint32)audio_get_data_format(endpoint);
            return TRUE;
        case STREAM_INFO_KEY_AUDIO_SAMPLE_RATE:
        case KASCMD_CONFIG_KEY_AUDIO_SAMPLE_RATE:
            result->u.value = endpoint->state.audio.port->sample_rate;
            return TRUE;
        case KASCMD_CONFIG_KEY_DRAM_PACKING_FORMAT:
            result->u.value = endpoint->state.audio.port->word_format;
            return TRUE;
        case KASCMD_CONFIG_KEY_INTERLEAVING_MODE:
            result->u.value = endpoint->state.audio.port->interleaved;
            return TRUE;
        case KASCMD_CONFIG_KEY_CLOCK_MASTER_SLAVE:
            result->u.value = endpoint->state.audio.port->locally_clocked;
            return TRUE;
        case EP_CBOPS_PARAMETERS:
        {
            CBOPS_PARAMETERS *parameters;
            unsigned cbops_flags = cbops_get_flags(endpoint->cbops);
            parameters = create_cbops_parameters(cbops_flags, EMPTY_FLAG);

            if (!parameters)
            {
                return FALSE;
            }

            if (cbops_flags & CBOPS_SHIFT)
            {
                if(endpoint->direction == SOURCE)
                {
                    if (!cbops_parameters_set_shift_amount(parameters, 8))
                    {
                        return FALSE;
                    }
                }
                else
                {
                    if (!cbops_parameters_set_shift_amount(parameters, -8))
                    {
                        return FALSE;
                    }
                }
            }
            result->u.value = (uint32)(uintptr_t) parameters;
            return TRUE;
        }
        case EP_RATEMATCH_RATE:
        {
            /* This is the code to do the measured audio rate */
            result->u.value = get_audio_rate(endpoint->state.audio.head_of_sync);
            return TRUE;
        }
        case EP_RATEMATCH_MEASUREMENT:
        {
            /* This is the code to do the measured audio rate */
            result->u.rm_meas.sp_deviation =
                    STREAM_RATEMATCHING_RATE_TO_FRAC(
                            get_audio_rate(endpoint->state.audio.head_of_sync));
            result->u.rm_meas.measurement.valid = FALSE;
            return TRUE;
        }
        case EP_RATEMATCH_ABILITY:
        {
            switch (dev)
            {
                /* Deliberate fall-through in switch cases */
                case AC97:
                case USP0:
                case USP1:
                case USP2:
                case USP3:
                    result->u.value = (uint32)RATEMATCHING_SUPPORT_SW;
                    endpoint->state.audio.port->rm_support = RATEMATCHING_SUPPORT_SW;
                    return TRUE;
                case I2S1:
                    result->u.value = (uint32)RATEMATCHING_SUPPORT_SW;
                    endpoint->state.audio.port->rm_support = RATEMATCHING_SUPPORT_SW;
                    return TRUE;
                case IACC:
                    result->u.value = (uint32)RATEMATCHING_SUPPORT_HW;
                    endpoint->state.audio.port->rm_support = RATEMATCHING_SUPPORT_HW;
                    return TRUE;
                default:
                    result->u.value = (uint32)RATEMATCHING_SUPPORT_NONE;
                    endpoint->state.audio.port->rm_support = RATEMATCHING_SUPPORT_NONE;
                    return FALSE;
            }
        }

        default:
            return FALSE;
    }
}

/**
 * \brief Stops the kick interrupt source that was started by this endpoint.
 *
 * \param ep Pointer to the endpoint this function is being called on.
 *
 * \return TRUE/FALSE success or failure
 */
static bool audio_stop (ENDPOINT *ep)
{
    a7da_audio_port* port = ep->state.audio.port;
    bool ep_head_of_sync = IS_ENDPOINT_HEAD_OF_SYNC(ep);

    if(!ep->state.audio.running)
    {
        /* The kick source is already stopped */
        return FALSE;
    }

    ep->state.audio.running = FALSE;

    if (ep_head_of_sync)
    {
        remote_cbuf_init(&port->cb_desc_local);
        kas_audio_device dev = GET_INSTANCE_FROM_AUDIO_EP_KEY(ep->key);
        kas_audio_stop(dev, ep->direction);

        /* uninitialise rate-matching */
        kas_audio_rm_clr(ep);

        if ((ep->direction==SOURCE) && (dev==IACC) && kcodec_reg_store.adc_dirty_flag)
        {
            keyhole_write_32b_addr(ULNG(KCODEC_ADC_A_GAIN), (uint32)kcodec_reg_store.adc_a_gain_bak);
            keyhole_write_32b_addr(ULNG(KCODEC_ADC_B_GAIN), (uint32)kcodec_reg_store.adc_b_gain_bak);
        }

    }

    return TRUE;
}

/**
 * \brief Starts a kick interrupt source based off this audio endpoint.
 *
 * \param ep pointer to the endpoint which is responsible for scheduling
 * the kick.
 * \param ko pointer to the KICK_OBJECT that recieved an interrupt and called
 * this function.
 *
 * \return TRUE/FALSE success or failure
 */
static bool audio_start (ENDPOINT *ep, KICK_OBJECT *ko)
{
    a7da_audio_port *port = ep->state.audio.port;

    /* If we are already running then don't do anything */
    if(ep->state.audio.running)
    {
        return TRUE;
    }

    if (IS_ENDPOINT_HEAD_OF_SYNC(ep))
    {
        kas_remote_cbuffer *cb = &port->cb_desc_local;
        remote_cbuf_init(cb);

        /* Initialise rate-matching */
        kas_audio_rm_init(ep);

        kas_audio_device dev = GET_INSTANCE_FROM_AUDIO_EP_KEY(ep->key);
        /* Start audio devices */
        if(kas_audio_start(port, dev, ep->direction) == FALSE)
        {
            // Todo: should this be true for this specific case?
            // N.B. although we return true the state is not running.
            return TRUE;
        }
        /* Advance HW pointer to half-buffer initially */
        remote_cbuf_advance_hw_ptr(cb);

        if (ep->state.audio.port->ilv_buf)
        {   /* Reset the interleaving buffer */
            cbuffer_empty_buffer(ep->state.audio.port->ilv_buf);
        }

        /* ADC gain register backup values since they are used by ALSA,
            so we have to restore them after use.  */
        if ((ep->direction==SOURCE) && (dev==IACC))
        {
            uint32 reg_value;
            keyhole_read_32b_addr(ULNG(KCODEC_ADC_A_GAIN), &reg_value);
            kcodec_reg_store.adc_a_gain_bak = (uint16)reg_value;
            keyhole_read_32b_addr(ULNG(KCODEC_ADC_B_GAIN), &reg_value);
            kcodec_reg_store.adc_b_gain_bak = (uint16)reg_value;
            kcodec_reg_store.adc_dirty_flag = FALSE;
        }

    }

    /* Reset the channel buffers */
    if(ep->state.audio.source_buf != NULL)
    {
        cbuffer_empty_buffer(ep->state.audio.source_buf);
    }
    if(ep->state.audio.sink_buf != NULL)
    {
        cbuffer_empty_buffer(ep->state.audio.sink_buf);
        if (ep->direction==SINK)
        {
            /* Add a block of latency to the sink to prevent initial underruns */
            cbuffer_block_fill(ep->state.audio.sink_buf, ep->state.audio.block_size, 0);
        }
    }
    if(!ep->state.audio.is_overridden)
    {
        cbops_mgr_buffer_reinit(ep->cbops);
    }
    return ep->state.audio.running = TRUE;
}
