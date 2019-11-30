#ifndef __AUDIO_CLOCK_H__
#define __AUDIO_CLOCK_H__
#include <app/audio/audio_if.h>

/*! file  @brief Audio DSP Clock configuration 
** 
*/
      
#if TRAPSET_WAKE_ON_AUDIO

/**
 *  \brief Configures audio-DSP clock in different operation modes.
 * Application can configure audio-DSP clock in below operation modes:  
 * 1) Active mode - Application can put audio-DSP in active mode by calling 
 * OperatorFrameworkEnable(MAIN_PROCESSOR_ON). 
 * 2) Low-power mode - Application can put audio-DSP in low-power mode by first 
 * calling OperatorFrameworkTriggerNotificationStart() and then switching off 
 * main processor by using OperatorFrameworkEnable(MAIN_PROCESSOR_OFF). 
 * 3) Trigger Mode - If audio-DSP is configured to operate in low-power mode, 
 * it needs a different clock configuration during transistion from low-power to
 * active mode. 
 * The default clock frequency in different modes can be obtained by 
 * using AudioDspGetClock().
 * @note
 * 1) The application must power on audio-DSP by using 
 * OperatorFrameworkEnable(MAIN_PROCESSOR_ON) before using this API.
 * 2) The clock configuration will reset to default settings if application 
 * decides to power-off audio-DSP without enabling trigger notification in 
 * low-power mode. Check OperatorFrameworkTriggerNotificationStart documentation
 * for more details.
 * 3) The active mode clock configuration must be selected carefully in order to
 * run the audio use case with optimum power consumption.
 * 4) The application must stop all operators while changing the active mode 
 * clock configuration to avoid undesirable audio output. 
 *         
 *  \param clock_config  Pointer to audio-dsp clock configuration 
 *  \return TRUE if clock configuration request has succeeded, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
bool AudioDspClockConfigure(audio_dsp_clock_configuration * clock_config);

/**
 *  \brief Read audio-DSP clock values in different operation modes.
 * Application can read audio-DSP clock values in different operation modes
 * by using this API. 
 *         
 *  \param dsp_clock  Pointer to location where clock values will be  
 *             written if read succeeds 
 *  \return TRUE if clock values have been read successfully, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
bool AudioDspGetClock(audio_dsp_clock * dsp_clock);
#endif /* TRAPSET_WAKE_ON_AUDIO */
#endif
