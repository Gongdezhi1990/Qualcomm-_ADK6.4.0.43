/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016 - 2018

@file

    Simple application demonstrating simple stereo loopback using the
    BASIC_PASS (passthrough) Kymera capability.

    The application is set up for use on the development
    board and uses the LCD.

    Audio from Line In is looped back to Line Out, with 0dB
    of gain.
*/

#include <stdio.h>
#include <vmtypes.h>
#include <os.h>
#include <panic.h>
#include <sink.h>
#include <source.h>
#include <stream.h>
#include <vmal.h>
#include <operator.h>
#include <hydra_macros.h>

#include <cap_id_prim.h>
#include <opmsg_prim.h>

#include <dev_board_lcd.h>
#include <dev_board_audio.h>

#define DAC_SAMPLE_RATE         48000

/* Create defines for volume manipulation.
 * The passthrough gain is the log2 of the required linear gain in Q6N format.
 * Convert a dB gain to Q6N as follows: 2^(32-6) * gain_db / 20log(2)
 * This can be simplified to a scaling of 2^26 / 20log2 = 67108864 / 6.0206
 */
#define GAIN_DB_TO_Q6N_SF (11146541)
#define GAIN_DB(x)      ((int32)(GAIN_DB_TO_Q6N_SF * (x)))

/* Operator applies unity gain (0dB) */
/* This can be adjusted while the application is running with the QACT
 * tuning tool. */
#define INITIAL_OPERATOR_GAIN    GAIN_DB(0)

/* Local function */
static void setup_loopback(void);

/* The operators */
static Operator passthrough;


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
    dev_board_audio_initialise();

    /* Write an initial message to LCD */
    dev_board_lcd_write(0, "QTIL DevBoard");
    dev_board_lcd_write(1, "Loopback. Init....");

    setup_loopback();

    /* Now display the application name */
    dev_board_lcd_write(1, "DevBrd Loopback");

    MessageLoop();
}

/*!
 * \brief Create the media graph.
 *
 * This function configures the audio interfaces used in this
 * demonstration application, creates the passthrough operator,
 * and connects them all together.
 *
 * All calls are protected by Panic macros, so in the event
 * of a failure, the system will halt.
 */
static void setup_loopback(void)
{
    /* Get the input endpoints (analogue ADC interface)
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

    /* Get the output endpoints (analogue DAC interface)
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

    /* Now instantiate the BASIC_PASS ("basic passthrough") capability as
     * an operator, to copy the audio. */
    passthrough = PanicZero(VmalOperatorCreate(CAP_ID_BASIC_PASS));

    /* Configure the operator.
     * Using the information from the Kymera Capability Library User Guide.
     * (Could also use OperatorsStandardSetParameters() as a higher-level
     * interface to set the gain parameter, rather than formatting the
     * message manually.)
     */
    uint16 set_gain[] = { OPMSG_COMMON_ID_SET_PARAMS, 1, 1, 1,
                          UINT32_MSW(INITIAL_OPERATOR_GAIN),
                          UINT32_LSW(INITIAL_OPERATOR_GAIN) };
    PanicZero(VmalOperatorMessage(passthrough, set_gain,
                                  sizeof(set_gain)/sizeof(set_gain[0]),
                                  NULL, 0));

    /* And connect everything */
    /* ...line_in to the passthrough operator */
    PanicNull(StreamConnect(source_line_inL,
                                StreamSinkFromOperatorTerminal(passthrough, 0)));
    PanicNull(StreamConnect(source_line_inR,
                                StreamSinkFromOperatorTerminal(passthrough, 1)));

    /* ...and passthrough operator to line out */
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(passthrough,0),
                                sink_line_outL));
    PanicNull(StreamConnect(StreamSourceFromOperatorTerminal(passthrough,1),
                                sink_line_outR));

    /* Finally start the operator.
     * After this call, you should hear any audio signal on LINE_IN audio
     * looped through to LINE_OUT.
     */
    PanicFalse(OperatorStartMultiple(1,&passthrough,NULL));
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
