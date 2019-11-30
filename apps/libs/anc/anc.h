/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file   anc.h
@brief  Header file for the ANC library.

        This file provides documentation for the ANC library API.
*/

#ifndef ANC_H_
#define ANC_H_

#include <audio_plugin_common.h>
#include <audio_plugin_music_variants.h>

#include <csrtypes.h>
#include <app/audio/audio_if.h>

/*! @brief The ANC Mode of operation when ANC functionality is enabled.

    Used by the VM application to set different ANC modes and also get the
    current mode of operation.
 */
typedef enum
{
    anc_mode_1,
    anc_mode_2,
    anc_mode_3,
    anc_mode_4,
    anc_mode_5,
    anc_mode_6,
    anc_mode_7,
    anc_mode_8,
    anc_mode_9,
    anc_mode_10
} anc_mode_t;

typedef enum
{
    all_disabled                    = 0x00,
    feed_forward_left               = 0x01,
    feed_forward_right              = 0x02,
    feed_back_left                  = 0x04,
    feed_back_right                 = 0x08,

    feed_forward_mode               = (feed_forward_left | feed_forward_right),
    feed_forward_mode_left_only     = feed_forward_left,
    feed_forward_mode_right_only    = feed_forward_right,

    feed_back_mode                  = (feed_back_left | feed_back_right),
    feed_back_mode_left_only        = feed_back_left,
    feed_back_mode_right_only       = feed_back_right,

    hybrid_mode                     = (feed_forward_left | feed_forward_right | feed_back_left | feed_back_right),
    hybrid_mode_left_only           = (feed_forward_left | feed_back_left),
    hybrid_mode_right_only          = (feed_forward_right | feed_back_right)
} anc_path_enable;


/*! @brief The ANC Microphones to be used by the library.

    Used by the VM application to provide the ANC library with all the required
    information about the microphones to use.
 */
typedef struct
{
    audio_mic_params feed_forward_left;
    audio_mic_params feed_forward_right;
    audio_mic_params feed_back_left;
    audio_mic_params feed_back_right;
    anc_path_enable enabled_mics;
    uint16 mic_gain_step_size;
} anc_mic_params_t;

/*! @brief The minimum sidetone gain value that can be set using the library.
 */
#define ANC_SIDETONE_GAIN_MIN 0

/*! @brief The maximum sidetone gain value that can be set using the library.
 */
#define ANC_SIDETONE_GAIN_MAX 15

/*! @brief The gain value corresponding to mute.
 */
#define ANC_SIDETONE_GAIN_MUTE ANC_SIDETONE_GAIN_MIN

/*!
    @brief Initialise the ANC library. This function must be called, and return
           indicating success, before any of the other library API functions can
           be called.

    @param mic_params The microphones to use for ANC functionality.
    @param init_mode The ANC mode at initialisation
    @param init_gain The gain at initialisation

    @return TRUE indicating the library was successfully initialised otherwise FALSE.
*/
bool AncInit(anc_mic_params_t *mic_params, anc_mode_t init_mode, unsigned init_gain);


#ifdef HOSTED_TEST_ENVIRONMENT
/*!
    @brief free the memory allocated for the ANC library data. 

    @return TRUE indicating the memory allocated for library was successfully freed doesn't return FALSE.  
*/    
bool AncLibraryTestReset(void);
#endif
             
/*!
    @brief Enable or Disable the ANC functionality. If enabled then the ANC will
           start operating in the last set ANC mode. To ensure no audio artefacts,
           the ANC functionality should not be enabled or disabled while audio is
           being routed to the DACs.

    @param mic_params The microphones to use for ANC functionality.

    @return TRUE indicating ANC was successfully enabled or disabled otherwise FALSE.
*/
bool AncEnable(bool enable);


/*!
    @brief Query the current state of the ANC functionality.

    @return TRUE if ANC is currently enabled otherwise FALSE.
*/
bool AncIsEnabled(void);


/*!
    @brief Set the ANC operating mode. To ensure no audio artefacts, the ANC mode
           should not be changed while audio is being routed to the DACs.

    @param anc_mode_t The ANC mode to set.

    @return TRUE indicating the ANC mode was successfully changed otherwise FALSE.
*/
bool AncSetMode(anc_mode_t mode);

/*!
    @brief Set the ANC sidetone gain. The ANC sidetone gain must not be less 
           than ANC_SIDETONE_GAIN_MIN or greater than ANC_SIDETONE_GAIN_MAX. 
           If the ANC functionality is enabled then the ANC sidetone gain will 
           be applied immediately otherwise it will be applied the next time 
           ANC is enabled.

    @param gain The ANC sidetone gain to set.

    @return TRUE indicating the ANC sidetone gain was successfully changed
            otherwise FALSE.
*/
bool AncSetSidetoneGain(uint16 gain);

/*!
    @brief Cycle through fine tune gains based on step size configured.
           If the ANC functionality is enabled then the fine tune gain will
           be applied immediately otherwise it will be ignored.

    @return TRUE indicating the fine tune gain was successfully changed
            otherwise FALSE.
*/
bool AncCycleFineTuneGain(void);

/*!
    @brief Returns the current ANC mic config.

    @return Pointer to current anc_mic_params_t stored in the ANC library
*/
anc_mic_params_t * AncGetAncMicParams(void);

/*!
    @brief Check if audio disconnection on ANC state change is required.

    @return TRUE if audio disconnection is required, otherwise FALSE.
*/
bool AncIsAudioDisconnectRequiredOnStateChange(void);

#endif
