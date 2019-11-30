/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_dsp_capabilities.c

DESCRIPTION
    Initialises the dsp capabilities configuration data for custom operator library.

*/


#include "sink_debug.h"
#include "sink_configmanager.h"
#include "sink_dsp_capabilities_config_def.h"
#include "custom_operator.h"
#include "sink_dsp_capabilities.h"

#ifdef DEBUG_DSP_CAPABILITIES
#define DSP_CAPABILITIES_DEBUG(x) DEBUG(x)
#else
#define DSP_CAPABILITIES(x)
#endif

static bool offloadAptxDecoderEnabled(void);
static bool offloadCeltEncoderEnabled(void);


void sinkDspCapabilitiesInit(void)
{
    if(offloadAptxDecoderEnabled())
        CustomOperatorReplaceProcessorId(capability_id_aptx_decoder,OPERATOR_PROCESSOR_ID_1);
    
    if(offloadCeltEncoderEnabled())
        CustomOperatorReplaceProcessorId(capability_id_celt_encoder,OPERATOR_PROCESSOR_ID_1);
}


/*************************************************************************
NAME
    offloadAptxDecoderEnabled

DESCRIPTION
    Check the status of aptX decoder offload enable configuration bit.

RETURNS
    TRUE if aptX offload enabled, FALSE otherwise

**************************************************************************/
static bool offloadAptxDecoderEnabled(void)
{
    dsp_caps_config_def_t *dsp_caps_config_data = NULL;
    bool enabled = FALSE;

    if(configManagerGetReadOnlyConfig(DSP_CAPS_CONFIG_BLK_ID, (const void **)&dsp_caps_config_data))
    {
        enabled = dsp_caps_config_data->OffloadAptxDecoder;
        configManagerReleaseConfig(DSP_CAPS_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME
    offloadCeltEncoderEnabled

DESCRIPTION
    Check the status of Celt encoder offload enable configuration bit.

RETURNS
    TRUE if aptX offload enabled, FALSE otherwise

**************************************************************************/
static bool offloadCeltEncoderEnabled(void)
{
    dsp_caps_config_def_t *dsp_caps_config_data = NULL;
    bool enabled = FALSE;

    if(configManagerGetReadOnlyConfig(DSP_CAPS_CONFIG_BLK_ID, (const void **)&dsp_caps_config_data))
    {
        enabled = dsp_caps_config_data->OffloadCeltEncoder;
        configManagerReleaseConfig(DSP_CAPS_CONFIG_BLK_ID);
    }
    return enabled;
}



