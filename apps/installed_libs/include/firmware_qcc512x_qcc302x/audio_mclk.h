#ifndef __AUDIO_MCLK_H__
#define __AUDIO_MCLK_H__
/** \file */
#if TRAPSET_AUDIO_MCLK

/**
 *  \brief Configure the audio master clock (MCLK). 
 *  \param use_external TRUE if the MCLK is supplied externally, FALSE if generated internally,
 *             which uses a dedicated clock generator that consumes extra power
 *  when in use. 
 *  \param frequency If use_external is TRUE, this specifies the nominal
 *             frequency of the input in Hz. If zero, the frequency of the input
 *             will be measured each time MCLK is required (when
 *  Source/SinkMasterClockEnable() is
 *             called). If the measured frequency is not within 1% of the required
 *  frequency (configured
 *             by STREAM_{PCM, I2S}_SYNC_RATE and STREAM_{PCM,
 *  I2S}_MASTER_MCLK_MULT), enabling MCLK
 *             will fail.
 *             Ignored if use_external is FALSE. 
 *  \return TRUE if configuration succeeded, FALSE otherwise. MCLK cannot be reconfigured
 *  while in use. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio_mclk
 */
bool AudioMasterClockConfigure(bool use_external, uint32 frequency);

/**
 *  \brief Output or use the audio master clock (MCLK) for a source interface. 
 * It is only valid to call this function for an audio source whose CLK_SOURCE has
 * been set to MCLK. Interfaces for which this can be done are \#AUDIO_HARDWARE_I2S
 * and \#AUDIO_HARDWARE_PCM.
 * For such a source, this function must be called with one of the ENABLE values
 * before the source is taken into use (with StreamConnect()), and source use must
 * stop (Source disconnected) before the MCLK can be DISABLEd.
 * The master clock used is as configured with AudioMasterClockConfigure() (all
 * interfaces use the same master clock signal and configuration).
 * Once this function has been called with one of the ENABLE values, it is not 
 * possible to reconfigure the source in any way (with SourceConfigure()) or to
 * reconfigure the MCLK (with AudioMasterClockConfigure()) until it is called
 * again with \#MCLK_DISABLE.
 * This function does not itself cause any activity on audio interface pins (other
 * than the MCLK pin, if configured) or cause audio to start flowing. It does
 * cause the internal MPLL to be enabled (if configured) or the external MCLK
 * signal's frequency to be measured (if configured).
 * If AudioMasterClockConfigure() is set to generate an MCLK signal internally,
 * the frequency of any MCLK output signal will be the interface sample rate 
 * (\#STREAM_I2S_SYNC_RATE) multipled by \#STREAM_I2S_MASTER_MCLK_MULT (or similar 
 * for a PCM interface).
 *         
 *  \param source The source to configure MCLK use on. 
 *  \param enable The MCLK enable state. 
 *  \return TRUE if the clock was enabled/disabled, FALSE if it failed. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio_mclk
 */
bool SourceMasterClockEnable(Source source, vm_mclk_enable enable);

/**
 *  \brief Output or use the audio master clock (MCLK) for a sink interface. 
 * It is only valid to call this function for an audio sink whose CLK_SOURCE has
 * been set to MCLK. Interfaces for which this can be done are \#AUDIO_HARDWARE_I2S,
 * \#AUDIO_HARDWARE_PCM and \#AUDIO_HARDWARE_SPDIF.
 * For such a sink, this function must be called with one of the ENABLE values
 * before the sink is taken into use (with StreamConnect()), and sink use must stop
 * (Sink disconnected) before the MCLK can be DISABLEd.
 * The master clock used is as configured with AudioMasterClockConfigure() (all
 * interfaces use the same master clock signal and configuration).
 * Once this function has been called with one of the ENABLE values, it is not 
 * possible to reconfigure the source in any way (with SinkConfigure()) or to
 * reconfigure the MCLK (with AudioMasterClockConfigure()) until it is called
 * again with \#MCLK_DISABLE.
 * This function does not itself cause any activity on audio interface pins (other
 * than the MCLK pin, if configured) or cause audio to start flowing. It does
 * cause the internal MPLL to be enabled (if configured) or the external MCLK
 * signal's frequency to be measured (if configured).
 * If AudioMasterClockConfigure() is set to generate an MCLK signal internally,
 * the frequency of any MCLK output signal will be the interface sample rate 
 * (\#STREAM_I2S_SYNC_RATE) multiplied by \#STREAM_I2S_MASTER_MCLK_MULT (or similar 
 * for a PCM interface).
 *         
 *  \param sink The sink to configure MCLK use on. 
 *  \param enable The MCLK enable state. 
 *  \return TRUE if the clock was enabled/disabled, FALSE if it failed. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio_mclk
 */
bool SinkMasterClockEnable(Sink sink, vm_mclk_enable enable);
#endif /* TRAPSET_AUDIO_MCLK */
#endif
