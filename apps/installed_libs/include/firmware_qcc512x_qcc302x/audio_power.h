#ifndef __AUDIO_POWER_H__
#define __AUDIO_POWER_H__
#include <app/audio/audio_if.h>

/*! file  @brief Audio power save mode configuration 
** 
*/
      
#if TRAPSET_WAKE_ON_AUDIO

/**
 *  \brief Configure audio power save mode.
 *         
 *  \param mode  Power save mode to set 
 *  \return TRUE if power save mode has been set successfully, FALSE otherwise.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
bool AudioPowerSaveModeSet(audio_power_save_mode mode);

/**
 *  \brief Read audio power save mode in use.
 *         
 *  \return Current power save mode, 0 if get request failed.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_wake_on_audio
 */
audio_power_save_mode AudioPowerSaveModeGet(void );
#endif /* TRAPSET_WAKE_ON_AUDIO */
#endif
