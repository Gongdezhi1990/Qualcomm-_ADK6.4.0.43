#ifndef __AUDIO_PWM_H__
#define __AUDIO_PWM_H__
/** \file */
#if TRAPSET_AUDIO_PWM

/**
 *  \brief Enables/disables power PIO of the external audio pwm
 *         amplifier.
 *         
 * Note that the external amplifier's power PIO
 *         (and assert PIO) must be configured via AudioPwmPios MIB key.
 *         
 *  \param enable  Enables (TRUE) or disables (FALSE) power to the
 *             external audio amplifier. 
 *  \return TRUE if firmware enables/disables the external
 *           audio amplifier's power PIO, otherwise FALSE. A failure here only
 *           indicates that power PIO level has not been changed. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio_pwm
 */
bool AudioPwmPowerEnable(bool enable);

/**
 *  \brief Mute/unmute the audio pwm signal. 
 * Note that the external amplifier's power PIO
 *         (and assert PIO) must be configured via AudioPwmPios MIB key.
 *         
 *  \param enable  Mute (TRUE) or unmute (FALSE) the audio pwm signal. 
 *             
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_audio_pwm
 */
void AudioPwmMute(bool enable);
#endif /* TRAPSET_AUDIO_PWM */
#endif
