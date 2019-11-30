/******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_connect.c

DESCRIPTION
    Plugin that implements multichannel audio by utilising multiple hardware
    audio outputs (onboard DAC, I2S, etc...).
*/


/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <stdlib.h>
#include <panic.h>
#include <pio.h>
#include <print.h>
#include <stream.h>
#include <stdlib.h>
#include <sink.h>
#include <string.h>
#include <transform.h>
#include <audio.h>
#include <audio_config.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "audio_i2s_common.h"
#include "audio_plugin_if.h"
#include "audio_output.h"
#include "audio_output_private.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Error Return Values */
#define AUDIO_HARDWARE_NONE     (audio_hardware)(0xFFFF)
#define AUDIO_INSTANCE_NONE     (audio_instance)(0xFFFF)
#define AUDIO_CHANNEL_NONE      (audio_channel)(0xFFFF)

#define SAMPLE_RATE_88200       ((uint32) 88200)
#define SAMPLE_RATE_96KHZ       (96000)
#define SAMPLE_RATE_192KHZ      (192000)

/*============================================================================*
 *  Private Data Types
 *============================================================================*/



/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Multichannel audio configuration data */
const audio_output_config_t* config = NULL;
/* Global state variable */
static audio_output_state_t state;


/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

static void audioOutputResetSources(void);
static bool audioOutputValidateConnectParams(const audio_output_params_t* params);
static void audioOutputSetupI2sIfEnabled(const audio_output_params_t* params);
static void enableMclkIfRequired(bool enable);
static bool connect(audio_output_params_t* params);
static bool i2sOutputEnabled(unsigned disable_mask);
static void configureSink(Sink audio_sink,
                          audio_output_t audio_out,
                          const audio_output_params_t* params);
static Sink audioOutputGetSinkIfMapped(audio_output_t audio_out);
static audio_hardware getAudioHardwareType(audio_output_t audio_out);
static audio_instance getAudioHardwareInstance(audio_output_t audio_out);
static audio_channel getAudioHardwareChannel(audio_output_t audio_out);
static bool isDSPPresent(void);

/*============================================================================*
 * Public Function Implementations
 *============================================================================*/

/******************************************************************************/
bool AudioOutputInit(const audio_output_config_t* const conf)
{
    if (conf == NULL)
    {
        /* Passed in pointer was NULL, return error. */
        PRINT(("AUDIO OUT: Failed to initialise - NULL config\n"));
        return FALSE;
    }

    /* Store pointer for later access. */
    config = conf;

    /* Ensure all source mappings are cleared */
    audioOutputResetSources();

    /* Initialise global state variables. */
    memset(&state, 0, sizeof(audio_output_state_t));

    /* Initialise internal PCM hardware. */
    if (!audioOutputPcmInitHardware(config))
    {
        /* Failed to perform required initialisation, return error. */
        PRINT(("AUDIO OUT: Failed to initialise - PCM hardware\n"));
        return FALSE;
    }

    PRINT(("AUDIO OUT: Initialised successfully\n"));

    return TRUE;
}

/******************************************************************************/
bool AudioOutputAddSource(Source source, audio_output_t output)
{
    if(state.sources == NULL)
        state.sources = calloc(audio_output_max, sizeof(source));

    PanicNull(state.sources);

    if(state.sources[output] != NULL)
        return FALSE;

    state.sources[output] = source;

    return TRUE;
}

/******************************************************************************/
bool AudioOutputConnect(audio_output_params_t* params)
{
    bool connected = FALSE;

    if(audioOutputValidateConnectParams(params))
    {
        /* Connect the DSP sources to their appropriate hardware outputs. */
    	connected = connect(params);

        if (connected)
        {
            PRINT(("AUDIO OUT: Connect DSP successful, sample rate: %lu\n", params->sample_rate));

            /* Update state variables */
            state.params = *params;
            state.connected = TRUE;
        }
        else
        {
            PRINT(("AUDIO OUT: Connect DSP failed - Connecting sinks error\n"));

            /* Reset mappings only if unsuccessful, otherwise reset on disconnect. */
            audioOutputResetSources();
        }
    }

    return connected;
}

/******************************************************************************/
bool AudioOutputConnectStereoSource(Source source_left,
                                    Source source_right,
                                    audio_output_params_t* params)
{
    if (!state.connected)
    {
        /* Reset any previous mappings */
        audioOutputResetSources();

        if(!AudioOutputAddSource(source_left, audio_output_primary_left))
            return FALSE;

        if(!AudioOutputAddSource(source_right, audio_output_primary_right))
            return FALSE;
    }

    return AudioOutputConnect(params);
}

/******************************************************************************/
bool AudioOutputConnectSource(Source source,
                              audio_output_t output,
                              audio_output_params_t* params)
{
    if (!state.connected)
    {
        /* Reset any previous mappings */
        audioOutputResetSources();

        if(!AudioOutputAddSource(source, output))
            return FALSE;
    }

    return AudioOutputConnect(params);
}

/******************************************************************************/
bool AudioOutputDisconnect(void)
{
    audio_output_t audio_out;  /* Loop variable */
    bool at_least_one_mapping = FALSE;

    if (config == NULL)
    {
        /* Plugin is not yet initialised, return error. */
        PRINT(("AUDIO OUT: Disconnect failed - Not initialised\n"));
        return FALSE;
    }

    if (!state.connected || state.sources == NULL)
    {
        /* Not connected yet, return error. */
        PRINT(("AUDIO OUT: Disconnect failed - Not connected\n"));
        return FALSE;
    }

    /* Disconnect each output in turn. */
    forEachOutput(audio_out)
    {
        Sink audio_sink = audioOutputGetSinkIfMapped(audio_out);

        if (audio_sink != (Sink)NULL)
        {
            at_least_one_mapping = TRUE;
            StreamDisconnect(0, audio_sink);
        }
    }

    if (!at_least_one_mapping)
    {
        /* No mappings in config, return error. */
        PRINT(("AUDIO OUT: Disconnect failed - No enabled mappings\n"));
        return FALSE;
    }

    if (state.i2s_devices_active)
    {
        /* Disable MCLK */
        enableMclkIfRequired(FALSE);

        /* Shut down I2S devices */
        PRINT(("AUDIO OUT: Shutting down I2S devices...\n"));
        AudioShutdownI2SDevice();
        state.i2s_devices_active = FALSE;
    }

    forEachOutput(audio_out)
    {
        Sink audio_sink = audioOutputGetSinkIfMapped(audio_out);

        if (audio_sink != (Sink)NULL)
        {
            PanicFalse(SinkClose(audio_sink));

            audioOutputCheckUseCase(FALSE, config->mapping[audio_out].endpoint.type);
        }
    }

    /* Update state variables */
    state.connected = FALSE;

    /* Reset mappings */
    audioOutputResetSources();

    PRINT(("AUDIO OUT: Disconnect successful\n"));
    return TRUE;
}

/******************************************************************************/
Sink AudioOutputGetAudioSink(void)
{
    audio_output_t audio_out;

    if (state.sources != NULL)
    {
        /* Return the first valid audio sink */
        forEachOutput(audio_out)
        {
            Sink audio_sink = audioOutputGetSinkIfMapped(audio_out);

            if(audio_sink)
                return audio_sink;
        }
    }

    return (Sink)NULL;
}

/******************************************************************************/
bool AudioOutputConfigRequiresI2s(void)
{
    audio_output_t audio_out;  /* Loop variable */

    if (config == NULL)
    {
        /* Plugin is not initialised, so no I2S required. */
        return FALSE;
    }

    /* Loop through and check each output mapping. */
    forEachOutput(audio_out)
    {
        if (config->mapping[audio_out].endpoint.type == audio_output_type_i2s)
        {
            /* I2S output found, no need to continue. */
            return TRUE;
        }
    }

    /* No I2S outputs detected. */
    return FALSE;
}

/******************************************************************************/
bool AudioOutputI2sActive(void)
{
    if (config == NULL)
    {
        /* Plugin is not initialised, so I2S can't be. */
        return FALSE;
    }

    return state.i2s_devices_active;
}

/******************************************************************************/
bool AudioOutput24BitOutputEnabled(void)
{
    if (config == NULL)
    {
        /* Plugin is not initialised. */
        return FALSE;
    }
    return (config->output_resolution_mode == audio_output_24_bit) ? TRUE: FALSE;
}
/******************************************************************************/
uint32 AudioOutputGetSampleRate(const audio_output_params_t* params, unsigned disable_mask)
{
    /* If an I2S output is to be used then return the re-sampling frequency */
    if(i2sOutputEnabled(disable_mask) && !params->disable_resample)
    {
        return AudioI2SGetOutputResamplingFrequencyForI2s(params->sample_rate);
    }
    /* Return the sample_rate from params */
    return params->sample_rate;
}

/******************************************************************************/
AUDIO_OUTPUT_TYPE_T AudioOutputGetOutputType(audio_output_t audio_out)
{
    if ((config == NULL) || (audio_out >= audio_output_max))
    {
        return OUTPUT_INTERFACE_TYPE_NONE;
    }

    switch (config->mapping[audio_out].endpoint.type)
    {
        case audio_output_type_dac:
            return OUTPUT_INTERFACE_TYPE_DAC;

        case audio_output_type_i2s:
            return OUTPUT_INTERFACE_TYPE_I2S;

        case audio_output_type_spdif:
            return OUTPUT_INTERFACE_TYPE_SPDIF;

        case audio_output_type_bt:
            /* Handled by specific decoder plugin, fall through. */
        case audio_output_type_none:
        default:
            /* No mapping or unsupported output type. */
            return OUTPUT_INTERFACE_TYPE_NONE;
    }
}

bool AudioOutputIsOutputMapped(audio_output_t audio_out)
{
    if (config == NULL || audio_out >= audio_output_max)
    {
        return FALSE;
    }

    if(config->mapping[audio_out].endpoint.type != audio_output_type_none)
    {
        return TRUE;
    }

    return FALSE;
}

/******************************************************************************/
bool AudioOutputGetDspOutputTypesMsg(AUDIO_OUTPUT_TYPES_T* msg)
{
    audio_output_t audio_out;    /* Loop variable */

    if (config == NULL || msg == NULL)
    {
        return FALSE;
    }

    /* Loop through each output mapping. */
    forEachOutput(audio_out)
    {
        msg->out_type[audio_out] = AudioOutputGetOutputType(audio_out);
    }

    return TRUE;
}


/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/******************************************************************************
NAME:
    audioOutputResetSources

DESCRIPTION:
    Reset list of mapped sources
*/
static void audioOutputResetSources(void)
{
    if(state.sources)
    {
        free(state.sources);
        state.sources = NULL;
    }
}

/******************************************************************************
NAME:
    audioOutputValidateConnectParams

DESCRIPTION:
    Check that parameters passed to connect function are valid

PARAMETERS:
    params      Connection parameters containing configuration data.

RETURNS:
    TRUE if parameters were okay and library has been configured correctly,
    otherwise FALSE.
*/
static bool audioOutputValidateConnectParams(const audio_output_params_t* params)
{
    if (config == NULL || params == NULL || state.sources == NULL)
    {
        PRINT(("AUDIO OUT: Connect failed - Invalid parameters\n"));
        return FALSE;
    }

    if (state.connected)
    {
        PRINT(("AUDIO OUT: Connect failed - Already Connected\n"));
        return FALSE;
    }

    if (params->sample_rate == 0)
    {
        PRINT(("AUDIO OUT: Connect failed - Invalid sample rate\n"));
        return FALSE;
    }
    return TRUE;
}

/******************************************************************************
NAME:
    audioOutputSetupI2sIfEnabled

DESCRIPTION:
    Power on external I2S DAC(s) if required

PARAMETERS:
    params      Connection parameters containing configuration data.
*/
static void audioOutputSetupI2sIfEnabled(const audio_output_params_t* params)
{
    /* Carry out I2S-specific set up. */
    if (i2sOutputEnabled(0))
    {
        /* Check if we need to initialise I2S devices. */
        if (!state.i2s_devices_active)
        {
            PRINT(("AUDIO OUT: Initialising I2S devices...\n"));

            /* Initialise the I2S device hardware. */
            AudioI2SInitialiseDevice(params->sample_rate);

            /* Only needs to happen once. */
            state.i2s_devices_active = TRUE;
        }
    }
}

/******************************************************************************
NAME:
    enableMclkIfRequired

DESCRIPTION:
    Enables or disables the MCLK signal for an output, if required. MCLK only
    needs to be enabled once for each audio hardware instance, and shouldn't
    be disabled until all output channels of that instance have disconnected.

PARAMETERS:
    audio_out   Output to configure the MCLK for.
    enable      TRUE to enable MCLK output, FALSE to disable.
*/
static void enableMclkIfRequired(bool enable)
{
    audio_output_hardware_instance_t instance;

    for (instance = audio_output_hardware_instance_0;
         instance < audio_output_hardware_instance_max;
         instance++)
    {
        audio_output_t audio_out;

        forEachOutput(audio_out)
        {
            if (config->mapping[audio_out].endpoint.instance == instance &&
                config->mapping[audio_out].endpoint.type == audio_output_type_i2s)
            {
                Sink audio_sink = audioOutputGetSinkIfMapped(audio_out);

                if (audio_sink)
                {
                    AudioI2SEnableMasterClockIfRequired(audio_sink, enable);
                    break;  /* Instance found, no need to continue */
                }
            }
        }
    }
}

/******************************************************************************
NAME:
    connect

DESCRIPTION:
    Connects sinks for all output channels to the sources specified in the
    supplied parameters.

PARAMETERS:
    sources     Pointer to an array of audio_output_max sources.
    params      Connection parameters containing configuration data.

RETURNS:
    The sink that was successfully configured, NULL otherwise.
*/
static bool connect(audio_output_params_t* params)
{
    audio_output_t audio_out;  /* Loop variable */
    Sink audio_sink;
    Sink last_valid_sink = (Sink)NULL;

    params->sample_rate = AudioOutputGetSampleRate(params, 0);

    audioOutputSetupI2sIfEnabled(params);

    /* Configure and sync the sink for each output. */
    forEachOutput(audio_out)
    {
        audio_sink = audioOutputGetSinkIfMapped(audio_out);

        if (audio_sink == (Sink)NULL)
        {
            /* No mapping or sink failed to open. Move on to the next. */
            continue;
        }

        configureSink(audio_sink, audio_out, params);

        if (last_valid_sink)
        {
            PanicFalse(SinkSynchronise(last_valid_sink, audio_sink));
        }

        audioOutputCheckUseCase(TRUE, config->mapping[audio_out].endpoint.type);

        last_valid_sink = audio_sink;
    }

    /* Check if there are any sinks to connect before proceeding. */
    if (last_valid_sink == (Sink)NULL)
    {
        /* No mappings in config, all disabled, or all failed, return error. */
        PRINT(("AUDIO OUT: Connect failed - No sinks to connect\n"));
        return FALSE;
    }

    /* Enable MCLK */
    enableMclkIfRequired(TRUE);

    /* Connect up each valid sink to its source and un-mute. Must happen after
     * synchronisation, hence the need for a second loop and duplicate checks.
     */
    forEachOutput(audio_out)
    {
        audio_sink = audioOutputGetSinkIfMapped(audio_out);

        if (audio_sink == (Sink)NULL)
        {
            /* No mapping or sink failed to open. Move on to the next. */
            continue;
        }

        switch(params->transform)
        {
            case audio_output_tansform_adpcm:
                PanicFalse(TransformStart(TransformAdpcmDecode(state.sources[audio_out], audio_sink)));
            break;

            case audio_output_tansform_connect_and_dispose:
                PanicFalse(StreamConnectAndDispose(state.sources[audio_out], audio_sink));
            break;

            case audio_output_tansform_connect:
            default:
                PanicNull(StreamConnect(state.sources[audio_out], audio_sink));
            break;
        }
    }

    return TRUE;
}

/******************************************************************************
NAME:
    i2sOutputEnabled

DESCRIPTION:
    Checks whether any output has been configured to use the I2S hardware and
    has not been disabled in the supplied bitmask. Used to determine whether
    any I2S devices need to be initialised for a specific connection request.

PARAMETERS:
    disable_mask    Bitmask of outputs (audio_output_t) to ignore.

RETURNS:
    Whether I2S is going to be used. TRUE = yes, FALSE = no.
*/
static bool i2sOutputEnabled(unsigned disable_mask)
{
    audio_output_t audio_out;  /* Loop variable */

    /* Loop through and check each output mapping. */
    forEachOutput(audio_out)
    {
        if (config->mapping[audio_out].endpoint.type == audio_output_type_i2s)
        {
            if (state.sources != NULL &&
                state.sources[audio_out] == NULL)
            {
                /* No mapped I2S output, skip to next channel. */
                continue;
            }

            if (!(AudioOutputGetMask(audio_out) & disable_mask))
            {
                /* Enabled I2S output found, no need to continue. */
                return TRUE;
            }
        }
    }

    /* No mapped/enabled I2S outputs detected. */
    return FALSE;
}

/******************************************************************************
NAME:
    isDSPPresent

DESCRIPTION:
    Check if the DSP is loaded

PARAMETERS:
    NULL

RETURNS:
    Returns TRUE if DSP is loaded, FALSE otherwise.
*/
static bool isDSPPresent(void)
{
    DSP_STATUS_INFO_T status = GetCurrentDspStatus();
    if(status != DSP_NOT_LOADED)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static uint32 getDacOutputSampleRate(uint32 music_manager_rate)
{
    return (AudioConfigGetDacOutputResamplingRate()
                        ? AudioConfigGetDacOutputResamplingRate() : music_manager_rate);
}

/******************************************************************************
NAME:
    configureSink

DESCRIPTION:
    Configures the sink mapped to an output with the required parameters.

PARAMETERS:
    audio_sink  The audio sink to configure
    audio_out   Output to configure the mapped hardware output of.
    params      Parameters to configure.

RETURNS:
    The sink that was successfully configured, NULL otherwise.
*/
static void configureSink(Sink audio_sink,
                          audio_output_t audio_out,
                          const audio_output_params_t *params)
{
    uint16 bit_resolution = RESOLUTION_MODE_16BIT;

    /* Configure output rate */
    switch(config->mapping[audio_out].endpoint.type)
    {
        case audio_output_type_i2s:
            AudioI2SConfigureSink(audio_sink, params->sample_rate);
        break;

        case audio_output_type_spdif:
            PanicFalse(SinkConfigure(audio_sink, STREAM_SPDIF_OUTPUT_RATE, params->sample_rate));
        break;

        case audio_output_type_dac:
        {
            /* Enabling short FIR when running DAC at 88.2 Khz and long FIR for other sampling rates */
            uint32 mode = (params->sample_rate == SAMPLE_RATE_88200) ? CODEC_OUTPUT_INTERP_MODE_SHORT_FIR : CODEC_OUTPUT_INTERP_MODE_LONG_FIR;

            PanicFalse(SinkConfigure(audio_sink, STREAM_CODEC_OUTPUT_RATE, getDacOutputSampleRate(params->sample_rate)));
            PanicFalse(SinkConfigure(audio_sink, STREAM_CODEC_OUTPUT_INTERP_FILTER_MODE, mode));
        }
        break;

        default:
            /* Should never get here as audio_sink would be NULL */
            Panic();
        break;
    }

    /* 24bit resolution supported only when playing through DSP
        (standlone prompts/tones still works only on 16 bit)*/
    if((config->output_resolution_mode == audio_output_24_bit) && isDSPPresent())
    {
        bit_resolution = RESOLUTION_MODE_24BIT ;
    }
    PanicFalse(SinkConfigure(audio_sink, STREAM_AUDIO_SAMPLE_SIZE, bit_resolution));
}

/******************************************************************************
NAME:
    audioOutputGetSink

DESCRIPTION:
    Gets the sink for a specified output channel, if a mapping exists.

PARAMETERS:
    audio_out     Output to get the sink for.

RETURNS:
    The sink, or NULL if there was an error.
*/
Sink audioOutputGetSink(audio_output_t audio_out)
{
    return StreamAudioSink(getAudioHardwareType(audio_out),
                           getAudioHardwareInstance(audio_out),
                           getAudioHardwareChannel(audio_out));
}

/******************************************************************************
NAME:
    audioOutputGetSinkIfMapped

DESCRIPTION:
    Gets the sink for a specified output channel, but only if there is a Source
    mapping for it.

PARAMETERS:
    audio_out     Output to get the sink for.

RETURNS:
    The sink, or NULL if there was no source mapping.
*/
static Sink audioOutputGetSinkIfMapped(audio_output_t audio_out)
{
    if (state.sources[audio_out] != NULL)
    {
        return audioOutputGetSink(audio_out);
    }
    return NULL;
}

/******************************************************************************
NAME:
    getAudioHardwareType

DESCRIPTION:
    Gets the hardware type of an audio sink mapped to an output. Must only be
    called after successful initialisation (i.e. 'config' is valid).

PARAMETERS:
    audio_out     Output to get the hardware type of.

RETURNS:
    The hardware type, or AUDIO_HARDWARE_NONE if there was an error.
*/
static audio_hardware getAudioHardwareType(audio_output_t audio_out)
{
    switch (config->mapping[audio_out].endpoint.type)
    {
        case audio_output_type_dac:
            return AUDIO_HARDWARE_CODEC;

        case audio_output_type_i2s:
            return AUDIO_HARDWARE_I2S;

        case audio_output_type_spdif:
            return AUDIO_HARDWARE_SPDIF;

        case audio_output_type_none:
        default:
            /* No mapping or unsupported output type, return error. */
            return AUDIO_HARDWARE_NONE;
    }
}

/******************************************************************************
NAME:
    getAudioHardwareInstance

DESCRIPTION:
    Gets the hardware instance of an audio sink mapped to an output. Must only
    be called after successful initialisation (i.e. 'config' is valid).

PARAMETERS:
    audio_out     Output to get the hardware instance for.

RETURNS:
    The hardware instance, or AUDIO_INSTANCE_NONE if there was an error.
*/
static audio_instance getAudioHardwareInstance(audio_output_t audio_out)
{
    switch (config->mapping[audio_out].endpoint.instance)
    {
        case audio_output_hardware_instance_0:
            return AUDIO_INSTANCE_0;

        case audio_output_hardware_instance_1:
            return AUDIO_INSTANCE_1;

        case audio_output_hardware_instance_2:
            return AUDIO_INSTANCE_2;

        default:
            /* Control should never reach here, return error. */
            return AUDIO_INSTANCE_NONE;
    }
}

/******************************************************************************
NAME:
    getAudioHardwareChannel

DESCRIPTION:
    Gets the hardware channel of an audio sink mapped to an output. Must only
    be called after successful initialisation (i.e. 'config' is valid).

PARAMETERS:
    audio_out     Output to get the hardware channel for.

RETURNS:
    The hardware channel, or AUDIO_CHANNEL_NONE if there was an error.
*/
static audio_channel getAudioHardwareChannel(audio_output_t audio_out)
{
    /* Channel depends on hardware type */
    switch (config->mapping[audio_out].endpoint.type)
    {
        case audio_output_type_dac:
        {
            switch (config->mapping[audio_out].endpoint.channel)
            {
                case audio_output_channel_a:
                    return AUDIO_CHANNEL_A;

                case audio_output_channel_b:
                    return AUDIO_CHANNEL_B;

                default:
                break;
            }
        }
        break;

        case audio_output_type_i2s:
        {
            switch (config->mapping[audio_out].endpoint.channel)
            {
                case audio_output_channel_a:
                    return AUDIO_CHANNEL_SLOT_0;

                case audio_output_channel_b:
                    return AUDIO_CHANNEL_SLOT_1;

                default:
                break;
            }
        }
        break;

        case audio_output_type_spdif:
        {
            switch (config->mapping[audio_out].endpoint.channel)
            {
                case audio_output_channel_a:
                    return SPDIF_CHANNEL_A;

                case audio_output_channel_b:
                    return SPDIF_CHANNEL_B;

                default:
                break;
            }
        }
        break;

        case audio_output_type_none:
        default:
        break;
    }

    /* No mapping or unsupported output type, return error. */
    return AUDIO_CHANNEL_NONE;
}
