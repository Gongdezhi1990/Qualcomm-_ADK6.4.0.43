/****************************************************************************
 * Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
* \defgroup anc_tuning
* \file  anc_tuning_defs.h
* \ingroup capabilities
*
* ANC_TUNING capability private header file. <br>
*
 */

#ifndef _ANC_TUNING_PRIVATE_H_
#define _ANC_TUNING_PRIVATE_H_
/******************************************************************************
Include Files
*/
#include "anc_tuning.h"
#include "op_msg_utilities.h"
#include "anc_tuning_gen_c.h"
#include "opmgr/opmgr_endpoint_override.h"

/****************************************************************************
 Private Constant Definitions
 */

/** default block size for this operator's terminals */
#define ANC_TUNING_DEFAULT_BLOCK_SIZE                   1

/** total number of supported terminals for capability */
#define ANC_TUNING_MAX_SINKS          8
#define ANC_TUNING_MAX_SOURCES        4

#define ANC_TUNING_SINK_USB_LEFT      0 /*can be any other backend device. USB used in the ANC tuning graph*/
#define ANC_TUNING_SINK_USB_RIGHT     1 
#define ANC_TUNING_SINK_FBMON_LEFT    2 /*reserve slots for FBMON tap. Always connected.*/   
#define ANC_TUNING_SINK_FBMON_RIGHT   3    
#define ANC_TUNING_SINK_MIC1_LEFT     4 /* must be connected to internal ADC. Analog or digital */
#define ANC_TUNING_SINK_MIC1_RIGHT    5
#define ANC_TUNING_SINK_MIC2_LEFT     6      
#define ANC_TUNING_SINK_MIC2_RIGHT    7

#define ANC_TUNING_SOURCE_USB_LEFT    0 /* can be any other backend device. USB used in the tuning graph */
#define ANC_TUNING_SOURCE_USB_RIGHT   1
#define ANC_TUNING_SOURCE_DAC_LEFT    2 /* must be connected to internal DAC */
#define ANC_TUNING_SOURCE_DAC_RIGHT   3

#define USB_SINK_MASK       0x3
/* 2R | 2L | 1R | 1L | FR | FL | UR | UL */ 
#define ANC_SINK_MASK_1MIC  0x14   /* 0 0 0 1 0 1 0 0 */
#define ANC_SINK_MASK_2MIC  0x54   /* 0 1 0 1 0 1 0 0 */

/* Macros to mask the select/enable for FB tune */
#define ACCMD_ANC_CONTROL_EN_0_FBTUNEOUT0_SEL_MASK      0x00010000
#define ACCMD_ANC_CONTROL_EN_0_FBTUNEOUT1_SEL_MASK      0x00020000
#define ACCMD_ANC_CONTROL_EN_1_FBTUNEOUT0_SEL_MASK      0x00040000
#define ACCMD_ANC_CONTROL_EN_1_FBTUNEOUT1_SEL_MASK      0x00080000
#define ACCMD_ANC_CONTROL_EN_2_FBTUNEOUT0_SEL_MASK      0x00100000
#define ACCMD_ANC_CONTROL_EN_2_FBTUNEOUT1_SEL_MASK      0x00200000
#define ACCMD_ANC_CONTROL_EN_3_FBTUNEOUT0_SEL_MASK      0x00400000
#define ACCMD_ANC_CONTROL_EN_3_FBTUNEOUT1_SEL_MASK      0x00800000
#define ACCMD_ANC_CONTROL_EN_4_FBTUNEOUT0_SEL_MASK      0x01000000
#define ACCMD_ANC_CONTROL_EN_4_FBTUNEOUT1_SEL_MASK      0x02000000
#define ACCMD_ANC_CONTROL_EN_5_FBTUNEOUT0_SEL_MASK      0x04000000
#define ACCMD_ANC_CONTROL_EN_5_FBTUNEOUT1_SEL_MASK      0x08000000

/****************************************************************************
Public Type Declarations
*/

typedef struct anc_sink
{
    struct anc_sink *next;
    OVERRIDE_EP_HANDLE ep_handle;
    tCbuffer *buffer;
    unsigned *buffer_start;
    unsigned *read_ptr;
    unsigned buffer_size;

}anc_sink_t;


typedef struct anc_source
{
    struct anc_source *next;
    OVERRIDE_EP_HANDLE ep_handle;
    tCbuffer *buffer;
    anc_sink_t *sink;
    unsigned sink_index;
    unsigned peak;

}anc_source_t;


/* 
    Must manualy verify this structure matches with instance parameters
    in ANC_TUNING_PARAMETERS. We take this shortcut because CPS can only
    generate one struct per XML.
*/

typedef struct _tag_ANC_INST_PARAMS
{
    unsigned OFFSET_ANC_USECASE;
    unsigned OFFSET_FF_A_MIC_SENSITIVITY;
    unsigned OFFSET_FF_B_MIC_SENSITIVITY;
    unsigned OFFSET_FF_A_FE_GAIN;
    unsigned OFFSET_FF_B_FE_GAIN;
    unsigned OFFSET_SPKR_RECEIVE_SENSITIVITY;
    unsigned OFFSET_SPKR_RECEIVER_IMPEDANCE;
    unsigned OFFSET_SPKR_RECEIVER_PA_GAIN;
    unsigned OFFSET_FF_A_ENABLE;
    unsigned OFFSET_FF_B_ENABLE;
    unsigned OFFSET_FB_ENABLE;
    unsigned OFFSET_FFA_IN_ENABLE;
    unsigned OFFSET_FFB_IN_ENABLE;
    unsigned OFFSET_FF_A_INPUT_DEVICE;
    unsigned OFFSET_FF_B_INPUT_DEVICE;
    unsigned OFFSET_FB_MON;
    unsigned OFFSET_FF_OUT_ENABLE;
    unsigned OFFSET_SMLPF_ENABLE;
    unsigned OFFSET_FF_FLEX_ENABLE;
    unsigned OFFSET_FF_A_GAIN_ENABLE;
    unsigned OFFSET_FF_B_GAIN_ENABLE;
    unsigned OFFSET_FB_GAIN_ENABLE;
    unsigned OFFSET_FF_A_DCFLT_ENABLE;
    unsigned OFFSET_FF_B_DCFLT_ENABLE;
    unsigned OFFSET_DMIC_X2_FF_A_ENABLE;
    unsigned OFFSET_DMIC_X2_FF_B_ENABLE;
    unsigned OFFSET_ANC_FF_A_SHIFT;
    unsigned OFFSET_ANC_FF_B_SHIFT;
    unsigned OFFSET_ANC_FB_SHIFT;
    unsigned OFFSET_ANC_FF_A_COEFF[15];
    unsigned OFFSET_ANC_FF_A_GAIN_SCALE;
    unsigned OFFSET_ANC_FF_A_GAIN_SCALE_DEFAULT;
    unsigned OFFSET_ANC_FF_A_GAIN;
    unsigned OFFSET_ANC_FF_B_COEFF[15];
    unsigned OFFSET_ANC_FF_B_GAIN_SCALE;
    unsigned OFFSET_ANC_FF_B_GAIN_SCALE_DEFAULT;
    unsigned OFFSET_ANC_FF_B_GAIN;
    unsigned OFFSET_ANC_FB_COEFF[15];
    unsigned OFFSET_ANC_FB_GAIN_SCALE;
    unsigned OFFSET_ANC_FB_GAIN_SCALE_DEFAULT;
    unsigned OFFSET_ANC_FB_GAIN;
    unsigned OFFSET_ANC_FF_A_LPF_SHIFT0;
    unsigned OFFSET_ANC_FF_A_LPF_SHIFT1;
    unsigned OFFSET_ANC_FF_B_LPF_SHIFT0;
    unsigned OFFSET_ANC_FF_B_LPF_SHIFT1;
    unsigned OFFSET_ANC_FB_LPF_SHIFT0;
    unsigned OFFSET_ANC_FB_LPF_SHIFT1;
    unsigned OFFSET_FF_A_DCFLT_SHIFT;
    unsigned OFFSET_FF_B_DCFLT_SHIFT;
    unsigned OFFSET_SM_LPF_SHIFT;
}ANC_INST_PARAMS;

/* capability-specific extra operator data */
typedef struct anc_tuning_exop
{
    /* Common Parameter System (CPS) data */
    CPS_PARAM_DEF params_def;                         /* CPS control block */
    ANC_TUNING_PARAMETERS anc_tuning_cap_params;      /* Current Parameters */
    unsigned ReInitFlag;
    anc_sink_t sinks[ANC_TUNING_MAX_SINKS];
    anc_source_t sources[ANC_TUNING_MAX_SOURCES];
    anc_sink_t *first_sink;
    anc_source_t *first_source;
    unsigned connected_sinks;
    unsigned connected_sources;
    bool is_stereo;   /* set by config message - before connect/start*/
    bool is_two_mic;  /* set by config message - before connect/start*/
    bool connect_change;
    uint16 coeffs[15];
    unsigned fb_mon[2];

} ANC_TUNING_OP_DATA;

/*****************************************************************************
Private Function Definitions
*/



#endif /* _ANC_TUNING_PRIVATE_H_ */

