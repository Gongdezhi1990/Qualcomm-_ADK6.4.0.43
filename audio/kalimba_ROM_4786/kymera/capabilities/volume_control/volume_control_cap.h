/**
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
* \file  volume_control_cap_c.h
* \ingroup  capabilities
*
*  Volume Control
*
*/

#ifndef VOL_CTRL_CAP_C_H
#define VOL_CTRL_CAP_C_H

#include "vol_ctrl_gen_c.h"
#include "buffer/cbuffer_c.h"
#include "volume/shared_volume_control.h"
#include "op_msg_utilities.h"
#include "opmgr/opmgr_for_ops.h"
#include "pl_timers/pl_timers.h"

/* Capability Version */
#define VOL_CTRL_CAP_VERSION_MINOR            2

/* Aux Channel States */
typedef enum
{
   AUX_STATE_NO_AUX = 0,
   AUX_STATE_START_AUX,
   AUX_STATE_IN_AUX,
   AUX_STATE_END_AUX
}vol_ctlr_aux_state;

/* Parameter Access structures */

typedef struct vol_ctrl_aux_params
{
   unsigned aux_scale;                        /**< Scaling applied to AUX channel */
   unsigned atk_tc;                           /**< hold time ramping in to AUX audio  (0.01/msec) fractional */
   unsigned dec_tc;                           /**< hold time ramping out of AUX audio (0.01/msec) fractional */
}vol_ctrl_aux_params_t;

/* priority AUX mixing only supports 4 AUX channels 
   aux_routing is 5 bits per priority level.
   The entries aree in decreasing priority

   bit4:     Priority Level is valid
   bit3:     Mute AUX for channel
   bit2..0:  AUX Channel Index
*/
typedef struct vol_ctrl_chan_params
{
    unsigned aux_routing;                                       /**< priority list for mixing AUX channel(s) */
    unsigned prim_scale[VOL_CTRL_CONSTANT_AUX_NUM_PRIORITIES];  /**< fractional attenuation when mixing aux */
}vol_ctrl_chan_params_t;


/* Volume Conrtol Gains */
typedef struct vol_ctrl_gains
{
   unsigned master_gain;                                        /**< Overall gain for audio */
   unsigned mute;
   unsigned auxiliary_gain[VOL_CTRL_CONSTANT_NUM_CHANNELS];     /**< Volume for Aux stream */
   unsigned channel_trims[VOL_CTRL_CONSTANT_NUM_CHANNELS];      /**< Per channel Trim*/
}vol_ctrl_gains_t;


/* Volume Control Channels */

typedef struct vol_ctrl_aux_channel
{
   tCbuffer *buffer;
   vol_ctlr_aux_state state;
   unsigned           transition;
   unsigned           advance_buffer; 
   
}vol_ctrl_aux_channel_t;

typedef struct vol_ctrl_channel
{
   int       chan_idx;       /**< index for stream source buffer */
   
   unsigned aux_mix_gain;   /**< mix ratio for aux audio  Q.XX */
   unsigned prim_mix_gain;  /**< mix ratio for channel audio Q.XX */
   
   unsigned limit_gain_log2;     /**< Attenutaion to prevent saturation */
   unsigned limiter_gain_linear; /**< Attenutaion applied for saturation prevention Q.XX*/
   unsigned last_peak;           /**< Last peak measurement Q.XX */
   
   unsigned channel_gain;   /**< Q5.xx channel gain */
   unsigned last_volume;    /**< last digital gain applied to channel*/                            

   tCbuffer *aux_buffer;  
}vol_ctrl_channel_t;

/* Time constants for adjustments */
typedef struct vol_time_constants
{
   unsigned num_words;
   unsigned vol_tc;
   unsigned sat_tc;
   unsigned sat_tcp5;
   unsigned period;
}vol_time_constants_t;

/**
 * Extended data stucture for Volume Control capability  
 */
typedef struct vol_ctrl_data
{
   tCbuffer *input_buffer[VOL_CTRL_CONSTANT_NUM_CHANNELS];          /**< Pointer to Sink Terminals  */
   tCbuffer *output_buffer[VOL_CTRL_CONSTANT_NUM_CHANNELS];         /**< Pointer to Source Terminals  */
   tCbuffer *wait_on_space_buffer;
   tCbuffer *wait_on_data_buffer;

#ifdef INSTALL_METADATA
    tCbuffer *metadata_ip_buffer;   /** The input buffer with metadata to transport from */
    tCbuffer *metadata_op_buffer;   /** The output buffer with metadata to transport to */
#endif /* INSTALL_METADATA */
   unsigned num_channels;                                           /**< number of active channels */
   unsigned touched_sink;                                           /**< touched for active channels */
   unsigned touched_src;                                            /**< touched for active channels */
   vol_ctrl_channel_t *channels;
   
   vol_ctrl_aux_channel_t  aux_channel[VOL_CTRL_CONSTANT_NUM_CHANNELS];    /**< Auxilliary Channels  */
   
   unsigned sample_rate;   

   unsigned Ovr_Control;                                    /**< obpm control overides  */
   unsigned post_gain;                                      /**< gain applied outside of DSP control  */
   vol_ctrl_gains_t host_vol;
   vol_ctrl_gains_t obpm_vol;
   vol_ctrl_gains_t *lpvols;

   SHARED_VOLUME_CONTROL_OBJ *shared_volume_ptr;         /**< NDVC noise level */

   VOL_CTRL_PARAMETERS       parameters;
   /** additionally used fields */
   unsigned ReInitFlag;
   CPS_PARAM_DEF parms_def;
   
   vol_time_constants_t tc;

   unsigned aux_connected;
   unsigned aux_active;
   unsigned aux_state;				/**< AUX current usage */
   unsigned aux_in_use;
   unsigned aux_buff_size;
   bool     aux_pending;
   bool     used_all_input;
   tTimerId pending_timer;
   
   unsigned vol_initialised;

   unsigned mute_period;                               /**< Mute period in msec */
   unsigned cur_mute_gain;
   int      mute_increment;
   
   bool stream_based;                                   /**< Main channels are sync'd */

   unsigned reserved_1;     /**< For extension */
   unsigned reserved_2;
} vol_ctrl_data_t;

/****************************************************************************
Private Function Definitions
 */
extern bool setup_processing(vol_ctrl_data_t   *op_extra_data);
extern void destroy_processing(vol_ctrl_data_t   *op_extra_data);

/* Receive capability handler functions declarations */
extern bool vol_ctlr_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool vol_ctlr_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool vol_ctlr_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool vol_ctlr_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool vol_ctlr_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool vol_ctlr_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool vol_ctlr_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Data processing function */
extern void vol_ctlr_timer_task(void *kick_object);
extern void vol_ctlr_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);

/* Operator message handlers */
extern bool vol_ctlr_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vol_ctlr_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vol_ctlr_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vol_ctlr_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vol_ctlr_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool vol_ctlr_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vol_ctlr_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool vol_ctlr_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vol_ctlr_opmsg_data_stream_based(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

#endif  /* VOL_CTRL_CAP_C_H */


