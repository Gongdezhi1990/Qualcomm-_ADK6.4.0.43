#ifndef __MICBIAS_H__
#define __MICBIAS_H__
#include <app/mic_bias/mic_bias_if.h>

/*! file  @brief Traps to control the Microphone bias hardware */

#if TRAPSET_MICBIAS

/**
 *  \brief Control of Microphone bias hardware 
 *  \param mic Which microphone bias pin to configure 
 *  \param key Which aspect of the microphone bias to configure 
 *  \param value Which value to use
 *  \return TRUE if the "mic" parameter is valid and the input value is in valid range,
 *  else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_micbias
 */
bool MicbiasConfigure(mic_bias_id mic, mic_bias_config_key key, uint16 value);
#endif /* TRAPSET_MICBIAS */
#endif
