/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016 - 2018

@file

    Simple application demonstrating the chaining of audio
    capabilities (operators).

    The application is set up for use on the development
    board, using the LCD and buttons.

    Audio from the Line In and Mic-2 is merged, on dev boards with
    a separate mic input.
    Passed through a volume control.
    Then split and sent out of both the Line Out and the headphone
    jack, on boards with separate outputs.

    Control of operators while running is demonstrated by use of
    the buttons.

    Switch 1 & 2 control the volume
    Switch 3 controls mute

*/

#include <stdio.h>
#include <vmtypes.h>
#include <os.h>
#include <operator.h>
#include <panic.h>
#include <sink.h>
#include <source.h>
#include <stream.h>
#include <vmal.h>
#include <message.h>
#include <operator.h>

#include <cap_id_prim.h>
#include <opmsg_prim.h>

#include <dev_board_lcd.h>
#include <dev_board_buttons.h>

#if defined(H13672v2)
    #define DEVBOARD_HAS_HEADPHONE  1
    #define DEVBOARD_HAS_MICROPHONE 1
#elif defined(CE446)
    #define DEVBOARD_HAS_HEADPHONE  1
    #define DEVBOARD_HAS_MICROPHONE 1
#elif defined(CF376_CE692H3)
    #define DEVBOARD_HAS_HEADPHONE  1
    #define DEVBOARD_HAS_MICROPHONE 1	
#elif defined(CF376_CE826)
#elif defined(CF376_CF212)
#elif defined(CF376_CF429)
#elif defined(CF376_CF440)
#elif defined(QCC5127_AA_DEV_BRD_R2_AA)
#elif defined(QCC5127_AB_DEV_BRD_R2_AA)
#elif defined(CF133)
#elif defined(CG437)
#elif defined(CF376_CG724)
#else
    #error HW Variant not supported
#endif

#define DAC_SAMPLE_RATE         48000

/* The gain can be changed in the mixer as well as in the
 * volume control. If this is set to true, then the volume
 * and mute are implemented using the volume control -
 * otherwise the gain is adjusted in the mixer, and mute sets
 * a low gain in the mixer.
 */
#define DEMO_VOLUME_CONTROL     TRUE

/* Create defines for volume manipulation.
 * Gain or attenuation is performed in DB, which are scaled
 * by 60. i.e. -60 is attenuate by 1 db
 *
 * Attenuation has a maximum value of 0, whereas gain can
 * be positive.
 */
#define GAIN_DB(x)      ((int32)(60l * (x)))

#define MIN_GAIN        GAIN_DB(-60)
#define MAX_GAIN        GAIN_DB(6)
#define INITIAL_OPERATOR_GAIN GAIN_DB(0)
#define GAIN_STEP       GAIN_DB(5)

/* The control points for the volume control capability are
 * not available in a header file. Take these from
 * Kymera Capability Library User Guide.
 */
typedef enum volume_control_id {
    MUTE_ID             = 0x0002,
    TRIM_GAIN1          = 0x0010,
    MASTER_GAIN         = 0x0021
} volume_control_id;

/* The format of SET_CONTROL messages is different to that
 * inferred in the header file. A control message consists
 * of a variable length list of control values, rather than
 * a single control.
 *
 * We only set 1 at a time, so define a structure for that.
 */
typedef struct volume_set_control
{
    uint16          message_id; /* OPMSG_COMMON_ID_SET_CONTROL */
    uint16          number_of_controls;
    uint16          control_id; /* type volume_control_id */
    uint16          value_msw;
    uint16          value_lsw;
} volume_set_control;


/* Prototypes for local functions */
static void button_handler(uint8 button_id);
static void setup_operators(void);

static void configureMixer(Operator operator);
static void configureVolumeControl(Operator operator);
static void send_volume_control_message(volume_control_id control,uint32 value);

static void adjust_gain(void);
static void set_mute(bool muted);

/* Local variables to store state of the values we control */
static int32 master_volume_gain = INITIAL_OPERATOR_GAIN;
static bool master_volume_mute = FALSE;

/* The operators */
static Operator mixer_line_mic;
static Operator volume_control;
static Operator splitter_line_headset;


/*!
 * \brief The application entry point.
 *
 * All applications should follow the basics of initialising the
 * operating system and finishing by calling MessageLoop(), which
 * never returns.
 *
 * As the initialisation of this demo is straightforward, it is
 * embedded in the function rather than being a separate, init,
 * function.
 */
void main(void)
{
    OsInit();

    /* Power on audio subsystem. This is necessary to claim
     * (StreamAudioSource/Sink) and configure (Source/SinkConfigure)
     * audio interfaces, and create and configure audio operators.
     */
    OperatorFrameworkEnable(1);

    dev_board_lcd_initialise();

    /* Write an initial message to LCD */
    dev_board_lcd_write(0, "QTIL DevBoard");
    dev_board_lcd_write(1, "Complex. Init....");

    setup_operators();

    /* Now display the application name */
    dev_board_lcd_write(1, "DevBrd Complex");

    /* start handling buttons */
    dev_board_initialise_buttons(button_handler);

    MessageLoop();
}

/*!
 * \brief Create the media graph.
 *
 * This function configures the audio interfaces used in this
 * demonstration application, creates the 3 operators used by
 * the example, and connects them all together.
 *
 * All calls are protected by Panic macros, so in the event
 * of a failure, the system will halt.
 */
static void setup_operators(void)
{
    /* Get the main input endpoints (analogue ADC interface)
     * On CSRA681xx, AUDIO_INSTANCE_0 corresponds to the LINE_IN inputs.
     * On QCC512x/QCC30xx, it corresponds to the AUDIO_MIC/LINEIN inputs.
     */
    Source source_line_inL = PanicNull(StreamAudioSource(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_A));
    Source source_line_inR = PanicNull(StreamAudioSource(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_B));
    
    /* ...and configure the sample rate and gain for each channel.
     * (For the codec interfaces, these must be configured per-channel.
     * For some other interfaces, they need only be configured per-instance.
     * See the Audio API document for details.)
     */
    PanicFalse(SourceConfigure(source_line_inL, STREAM_CODEC_INPUT_RATE, DAC_SAMPLE_RATE));
    PanicFalse(SourceConfigure(source_line_inR, STREAM_CODEC_INPUT_RATE, DAC_SAMPLE_RATE));
    /* Unity gain (0dB) */
    PanicFalse(SourceConfigure(source_line_inL, STREAM_CODEC_INPUT_GAIN, 9));
    PanicFalse(SourceConfigure(source_line_inR, STREAM_CODEC_INPUT_GAIN, 9));

    /* As well as ensuring the two channels' audio stays in sync, this
     * reduces the CPU load in the audio subsystem.
     */
    PanicFalse(SourceSynchronise(source_line_inL, source_line_inR));

#ifdef DEVBOARD_HAS_MICROPHONE
    /* On CSRA681xx, AUDIO_INSTANCE_1 corresponds to the MIC inputs. */
    Source source_mic = PanicNull(StreamAudioSource(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_1, AUDIO_CHANNEL_A));
    PanicFalse(SourceConfigure(source_mic, STREAM_CODEC_INPUT_RATE, DAC_SAMPLE_RATE));
    /* Unity gain (0dB) */
    PanicFalse(SourceConfigure(source_mic, STREAM_CODEC_INPUT_GAIN, 9));
    /* Provide a mic bias voltage for the microphone to work */
    PanicFalse(MicbiasConfigure(MIC_BIAS_0, MIC_BIAS_ENABLE, MIC_BIAS_FORCE_ON));
#endif

    /* Get the main output endpoints (analogue DAC interface)
     * On CSRA681xx, AUDIO_INSTANCE_0 corresponds to the LINE_OUT outputs.
     * On QCC512x/QCC30xx, it corresponds to the AUDIO_HP/SPK outputs.
     */
    Sink sink_line_outL = PanicNull(StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_A));
    Sink sink_line_outR = PanicNull(StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_B));

    PanicFalse(SinkConfigure(sink_line_outL, STREAM_CODEC_OUTPUT_RATE, DAC_SAMPLE_RATE));
    PanicFalse(SinkConfigure(sink_line_outR, STREAM_CODEC_OUTPUT_RATE, DAC_SAMPLE_RATE));
    /* Unity gain (0dB) */
    PanicFalse(SinkConfigure(sink_line_outL, STREAM_CODEC_OUTPUT_GAIN, 15));
    PanicFalse(SinkConfigure(sink_line_outR, STREAM_CODEC_OUTPUT_GAIN, 15));

    /* Again, necessary for synchronised stereo audio and reduced CPU load. */
    PanicFalse(SinkSynchronise(sink_line_outL, sink_line_outR));

#ifdef DEVBOARD_HAS_HEADPHONE
    /* On CSRA681xx, AUDIO_INSTANCE_1 corresponds to the AUDIO_HP (headphone) outputs. */
    Sink sink_headphoneL = PanicNull(StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_1, AUDIO_CHANNEL_A));
    Sink sink_headphoneR = PanicNull(StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_1, AUDIO_CHANNEL_B));

    PanicFalse(SinkConfigure(sink_headphoneL, STREAM_CODEC_OUTPUT_RATE, DAC_SAMPLE_RATE));
    PanicFalse(SinkConfigure(sink_headphoneR, STREAM_CODEC_OUTPUT_RATE, DAC_SAMPLE_RATE));
    /* Unity gain (0dB) */
    PanicFalse(SinkConfigure(sink_headphoneL, STREAM_CODEC_OUTPUT_GAIN, 15));
    PanicFalse(SinkConfigure(sink_headphoneR, STREAM_CODEC_OUTPUT_GAIN, 15));

    /* Again, necessary for synchronised stereo audio and reduced CPU load. */
    PanicFalse(SinkSynchronise(sink_headphoneL, sink_headphoneR));
#endif

    /* Now instantiate the operators for processing the audio */
    mixer_line_mic = PanicZero(VmalOperatorCreate(CAP_ID_MIXER));
    volume_control = PanicZero(VmalOperatorCreate(CAP_ID_VOL_CTRL_VOL));
    splitter_line_headset = PanicZero(VmalOperatorCreate(CAP_ID_SPLITTER));

    /* Configure the operators */
    configureMixer(mixer_line_mic);
    configureVolumeControl(volume_control);

    /* And connect everything */
    /* ...line_in to the mixer operator */
    PanicNull(StreamConnect(source_line_inL,
                                StreamSinkFromOperatorTerminal(mixer_line_mic, 0)));
    PanicNull(StreamConnect(source_line_inR,
                                StreamSinkFromOperatorTerminal(mixer_line_mic, 1)));
#ifdef DEVBOARD_HAS_MICROPHONE
    /* ...mic to the mixer operator */
    PanicNull(StreamConnect(source_mic,
                                StreamSinkFromOperatorTerminal(mixer_line_mic, 2)));
#endif

    /* ...mixer operator to volume control operator */
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(mixer_line_mic,0),
                                StreamSinkFromOperatorTerminal(volume_control,0)));
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(mixer_line_mic,1),
                                StreamSinkFromOperatorTerminal(volume_control,2)));

    /* ...volume control operator to splitter operator */
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(volume_control,0),
                                StreamSinkFromOperatorTerminal(splitter_line_headset,0)));
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(volume_control,1),
                                StreamSinkFromOperatorTerminal(splitter_line_headset,1)));

    /* ...splitter operator to line out */
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(splitter_line_headset,0),
                                sink_line_outL));
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(splitter_line_headset,2),
                                sink_line_outR));

#ifdef DEVBOARD_HAS_HEADPHONE
    /* ...splitter operator's other outputs to headphone */
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(splitter_line_headset,1),
                                sink_headphoneL));
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(splitter_line_headset,3),
                                sink_headphoneR));
#endif

    /* Then start all of the operators at once.
     * After this call, you should hear any audio signal from LINE_IN
     * (mixed with the mic if supported by your dev board) looped through
     * to LINE_OUT (and also to the headphones if supported by your
     * dev board).
     * */
    Operator all_ops[] = {mixer_line_mic,
                          splitter_line_headset,
                          volume_control};

    PanicFalse(OperatorStartMultiple(sizeof(all_ops)/sizeof(all_ops[0]),all_ops,NULL));
}

/*!
 * \brief handler called when a button/switch on the
 * development button has been pressed.
 *
 * This can be extended to change the handling of buttons.
 *
 * \param button_id The button pressed (starts at 1)
 */
static void button_handler(uint8 button_id)
{
    switch(button_id)
    {
    case 1:
        master_volume_gain += -GAIN_STEP;
        adjust_gain();
        break;

    case 2:
        master_volume_gain += GAIN_STEP;
        adjust_gain();
        break;

    case 3:
        master_volume_mute = !master_volume_mute;
        set_mute(master_volume_mute);
        break;

    default:
        break;
    }


}

/*!
 * \brief Configure the mixer operator
 *
 * The number of channels for each stream is set, as well
 * as the sample rate, the primary stream and the gain.
 *
 * \param operator The mixer operator
 */
static void configureMixer(Operator operator)
{
    OPMSG_MIXER_SET_STREAM_CHANNELS set_channels;
    OPMSG_MIXER_SET_PRIMARY_STREAM set_primary;
    OPMSG_COMMON_MSG_SET_SAMPLE_RATE set_rate;
    OPMSG_MIXER_SET_STREAM_GAINS set_gains;

    OPMSG_MIXER_SET_STREAM_CHANNELS_PACK(&set_channels,OPMSG_MIXER_ID_SET_STREAM_CHANNELS,2,1,0);
    PanicFalse(VmalOperatorMessage(operator, &set_channels, SIZEOF_OPERATOR_MESSAGE(set_channels), NULL, 0));

    OPMSG_MIXER_SET_STREAM_GAINS_PACK(&set_gains,OPMSG_MIXER_ID_SET_STREAM_GAINS,INITIAL_OPERATOR_GAIN,INITIAL_OPERATOR_GAIN,0);
    PanicFalse(VmalOperatorMessage(operator, &set_gains, SIZEOF_OPERATOR_MESSAGE(set_gains), NULL, 0));

    OPMSG_MIXER_SET_PRIMARY_STREAM_PACK(&set_primary,OPMSG_MIXER_ID_SET_PRIMARY_STREAM,1);
    PanicFalse(VmalOperatorMessage(operator, &set_primary, SIZEOF_OPERATOR_MESSAGE(set_primary), NULL, 0));

    OPMSG_COMMON_MSG_SET_SAMPLE_RATE_PACK(&set_rate,OPMSG_COMMON_SET_SAMPLE_RATE,DAC_SAMPLE_RATE/25);
    PanicFalse(VmalOperatorMessage(operator, &set_rate, SIZEOF_OPERATOR_MESSAGE(set_rate), NULL, 0));
}

/*!
 * \brief Configure the volume control operator
 *
 * The sample rate is set as well as the rate at which volume
 * changes are effected.
 *
 * \param operator The volume operator
 */
static void configureVolumeControl(Operator operator)
{
    OPMSG_COMMON_MSG_SET_SAMPLE_RATE set_rate;
    OPMSG_COMMON_SET_PARAMS params;

    OPMSG_COMMON_MSG_SET_SAMPLE_RATE_PACK(&set_rate,OPMSG_COMMON_SET_SAMPLE_RATE,DAC_SAMPLE_RATE/25);
    PanicFalse(VmalOperatorMessage(operator, &set_rate, SIZEOF_OPERATOR_MESSAGE(set_rate), NULL, 0));

    /* Set the speed at which volume will change */
    OPMSG_COMMON_SET_PARAMS_PACK(&params,OPMSG_COMMON_ID_SET_PARAMS,1,68,1,UINT32_MSW(1),UINT32_LSW(1));
    PanicFalse(VmalOperatorMessage(operator, &params, SIZEOF_OPERATOR_MESSAGE(params), NULL, 0));

    send_volume_control_message(MASTER_GAIN,INITIAL_OPERATOR_GAIN);
}

/*!
 * \brief Send a SET_CONTROL message to the volume operator
 *
 * \param control_id The control to be updated
 * \param value      The new value
 */
static void send_volume_control_message(volume_control_id control_id,uint32 value)
{
    volume_set_control set = { OPMSG_COMMON_ID_SET_CONTROL, 1, control_id, value>>16, value & 0xFFFF};

    PanicFalse(VmalOperatorMessage(volume_control, &set, SIZEOF_OPERATOR_MESSAGE(set), NULL, 0));
}

/*!
 * \brief Helper function to adjust the volume.
 *
 * If DEMO_VOLUME_CONTROL is specified then this uses the
 * volume operator to make the adjustment, otherwise it
 * demonstrates the ability to vary the volume in the mixer,
 * adjusting the gain on the microphone (stream 2) by less
 * than that on line in (stream 1).
 *
 * The gain value is updated on the LCD.
 */
static void adjust_gain(void)
{
    char buf[30];

    if (master_volume_gain > MAX_GAIN)
    {
        master_volume_gain = MAX_GAIN;
    }
    else if (master_volume_gain < MIN_GAIN)
    {
        master_volume_gain = MIN_GAIN;
    }

#if DEMO_VOLUME_CONTROL
    send_volume_control_message(MASTER_GAIN,master_volume_gain);
#else
    OPMSG_MIXER_SET_STREAM_GAINS set_gains;
    int32 mic_gain = master_volume_gain / 2;

    OPMSG_MIXER_SET_STREAM_GAINS_PACK(&set_gains,OPMSG_MIXER_ID_SET_STREAM_GAINS,master_volume_gain,mic_gain,0);
    PanicFalse(VmalOperatorMessage(mixer_line_mic, &set_gains, SIZEOF_OPERATOR_MESSAGE(set_gains), NULL, 0));
#endif

    sprintf(buf,"Gain %04x %ddB   ",master_volume_gain&0xFFFF,master_volume_gain/60);
    dev_board_lcd_write(1,buf);
}

/*!
 * \brief Helper function to mute/unmute audio
 *
 * This always uses the volume control for the mute.
 *
 * The display is updated with the gain and mute state.
 *
 * \param muted Whether to mute audio.
 */
static void set_mute(bool muted)
{
    char buf[30];

    /*When muted is true then it will mute the system
      and when it is false it will unmute the system*/
    send_volume_control_message(MUTE_ID,muted);

    sprintf(buf,"Gain %04x %s   ",master_volume_gain&0xFFFF,muted?"MUTED":"UNMUTE");
    dev_board_lcd_write(1,buf);
}

/* Define pmalloc pools. This definition will be merged at run time with the
   base definitions from Hydra OS - see 'pmalloc_config_P1.h'. */

#include <pmalloc.h>

_Pragma ("unitsuppress Unused")

_Pragma ("datasection apppool")

static const pmalloc_pool_config app_pools[] =
{
    {   4, 14 },
    {   8, 25 },
    {  12, 17 },
    {  16,  7 },
    {  20, 22 },
    {  24, 15 },
    {  28, 55 },
    {  32, 21 },
    {  36, 19 },
    {  40, 10 },
    {  56,  9 },
    {  64,  7 },
    {  80,  9 },
    { 120, 16 },
    { 140,  4 },
    { 180,  6 },
    { 220,  3 },
    { 288,  1 }, 
    { 512,  1 },
    { 692,  3 }    
};
