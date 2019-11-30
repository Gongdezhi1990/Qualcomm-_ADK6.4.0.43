/*******************************************************************************
 * Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.
*******************************************************************************/

/**
 * \file  stream_anc.c
 * \ingroup stream
 *
 * Control the ANC audio HW <br>
 * This file contains the stream ANC operator shim functions. <br>
 *
 *  \note This file contains the operator access functions for Active Noise Cancellation (ANC).
 *  It consists of several shim functions that provide an interface between instances of
 *  (downloaded) Kymera capabilities and the ANC HAL layer (without which access would not
 *  be possible).
 *
 */

#ifdef INSTALL_AUDIO_MODULE

#ifdef INSTALL_UNINTERRUPTABLE_ANC

#if !defined(HAVE_SIDE_TONE_HARDWARE) || !defined(HAVE_ANC_HARDWARE)
    #error "HAVE_SIDE_TONE_HARDWARE and HAVE_ANC_HARDWARE must be defined "
           "when INSTALL_UNINTERRUPTABLE_ANC is defined"
#endif /* !defined(HAVE_SIDE_TONE_HARDWARE) || !defined(HAVE_ANC_HARDWARE) */

/*******************************************************************************
Include Files
*/

#include "hal_audio_anc.h"
#include "hal_audio.h"
#include "patch/patch.h"
#include "stream_for_opmgr.h"

/*******************************************************************************
 * Private macros/consts
 */

/*******************************************************************************
Private Function Declarations
*/

/*******************************************************************************
Private Function Definitions
*/

/*******************************************************************************
Public Function Definitions
*/

/**
 * \brief Configure the ANC DC filters and small low-pass filters.
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param shift Shift used to configure filter.
 *
 */
void stream_anc_set_anc_filter_shift(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 shift)
{
    patch_fn_shared(stream_anc);

    hal_audio_set_anc_filter_shift(anc_instance, path_id, shift);
}

/**
 * \brief Configure the ANC path gains.
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param gain Gain applied to filter (8 bit linear gain value).
 *
 */
void stream_anc_set_anc_gain(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 gain)
{
    patch_fn_shared(stream_anc);

    hal_audio_set_anc_gain(anc_instance, path_id, gain);
}

/**
 * \brief Configure the ANC path gain shift (gain exponent).
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param shift Gain shift applied to filter (4 bit shift value).
 *
 */
void stream_anc_set_anc_gain_shift(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 shift)
{
    patch_fn_shared(stream_anc);

    hal_audio_set_anc_gain_shift(anc_instance, path_id, shift);
}

/**
 * \brief Configure the ANC gain enable (used in Adaptive ANC)
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param enable Enable/disable adaptive operation.
 *
 */
void stream_anc_set_anc_gain_enable(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 enable)
{
    patch_fn_shared(stream_anc);

    hal_audio_set_anc_gain_enable(anc_instance, path_id, enable);
}

/**
 * \brief Configure ANC miscellaneous hardware options
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param en_and_mask Combined bit field of enables and selection mask for ANC hardware control options.
 *
 */
void stream_anc_set_anc_control(ACCMD_ANC_INSTANCE anc_instance, uint32 en_and_mask)
{
    uint32 bit_enable;
    uint32 bit_select;

    patch_fn_shared(stream_anc);

    bit_enable = en_and_mask & ((1 << ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT) - 1);
    bit_select = (en_and_mask >> ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT);

    hal_audio_set_anc_control(anc_instance, bit_enable, bit_select);
}

/**
 * \brief Configure an ANC IIR filter (sets the coefficients)
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param num_coeffs Number of coefficients.
 * \param coeffs Pointer to an array of IIR coefficients.
 *
 */
void stream_anc_set_anc_iir_coeffs(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 num_coeffs, const uint16 *coeffs)
{
    patch_fn_shared(stream_anc);

    hal_audio_anc_set_anc_iir_coeffs(anc_instance, path_id, num_coeffs, coeffs);
}

/**
 * \brief Select the currently active IIR coefficient set
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param coeff_set coefficient set 0: Foreground, 1: background.
 *
 * \note  Coefficients for the FFA, FFB and FB IIR filters are banked
 *        (LPF shift coefficients are not banked)
 */
void stream_anc_select_active_iir_coeffs(ACCMD_ANC_INSTANCE anc_instance, hal_anc_bank_select_type coeff_set)
{
    patch_fn_shared(stream_anc);

    hal_select_active_iir_coeffs(anc_instance, coeff_set);
}

/**
 * \brief Copy the foreground coefficient set to the background coefficient set
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 *
 * \note  Coefficients for the FFA, FFB and FB IIR filters are banked
 *        (LPF shift coefficients are not banked)
 */
void stream_anc_update_background_iir_coeffs(ACCMD_ANC_INSTANCE anc_instance)
{
    patch_fn_shared(stream_anc);

    hal_update_background_iir_coeffs(anc_instance);
}

/**
 * \brief Select the currently active gain set
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param gain_set gain set 0: Foreground, 1: background.
 *
 * \note: Gains for the FFA, FFB, and FB LPF are shadowed
 *        (but gain shifts are not)
 */
void stream_anc_select_active_gains(ACCMD_ANC_INSTANCE anc_instance, hal_anc_bank_select_type gain_set)
{
    patch_fn_shared(stream_anc);

    hal_select_active_gains(anc_instance, gain_set);
}

/**
 * \brief Copy the foreground gain set to the background gain set
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 *
 * \note  Gains for the FFA, FFB, and FB LPF are shadowed
 *        (but gain shifts are not)
 */
void stream_anc_update_background_gains(ACCMD_ANC_INSTANCE anc_instance)
{
    patch_fn_shared(stream_anc);

    hal_update_background_gains(anc_instance);
}

/**
 * \brief Configure an ANC LPF filter (sets the LPF coefficients)
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param shift1 Coefficient 1 expressed as a shift.
 * \param shift2 Coefficient 2 expressed as a shift.
 *
 */
void stream_anc_set_anc_lpf_coeffs(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 shift1, uint16 shift2)
{
    patch_fn_shared(stream_anc);

    hal_audio_anc_set_anc_lpf_coeffs(anc_instance, path_id, shift1, shift2);
}

/**
 * \brief Place-holder function for patching purposes.
 *
 * \param ptr Pointer to function parameters (to be used as required)
 *
 */
void stream_anc_user1(void *ptr)
{
    patch_fn_shared(stream_anc);
}

/**
 * \brief Place-holder function for patching purposes.
 *
 * \param ptr Pointer to function parameters (to be used as required)
 *
 */
void stream_anc_user2(void *ptr)
{
    patch_fn_shared(stream_anc);
}

#endif /* INSTALL_UNINTERRUPTABLE_ANC */

#endif /* INSTALL_AUDIO_MODULE */
